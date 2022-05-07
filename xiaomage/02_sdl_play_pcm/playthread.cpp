#include "playthread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

#define FILENAME "/Users/jisudong/Documents/in.pcm"

#define SAMPLE_RATE 44100
// 采样格式
#define SAMPLE_FORMAT AUDIO_S16LSB
// 采样大小
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 单个样本字节数
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
// 文件缓冲区大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

struct AudioBuffer {
    int len = 0;
    int pullLen = 0;
    char *data = nullptr;
};

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    disconnect();

    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程析构了";
}


void pull_audio_data(void *userdata, Uint8 * stream, int len) {
    AudioBuffer *buffer = (AudioBuffer *) userdata;
    memset(stream, 0, len);
    if (buffer->len <= 0) return;
    buffer->pullLen = len > buffer->len ? buffer->len : len;
    SDL_MixAudio(stream, (Uint8 *)buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void PlayThread::run() {
    // 初始化Audio子系统
    int ret = SDL_Init(SDL_INIT_AUDIO);
    if (ret < 0) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.channels = CHANNELS;
    spec.format = SAMPLE_FORMAT;
    spec.samples = SAMPLES;
    spec.callback = pull_audio_data;
    AudioBuffer buffer;
    spec.userdata = &buffer;

    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "打开设备失败：" << SDL_GetError();
        SDL_Quit();
        return;
    }

    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "文件打开失败：" << FILENAME;
        SDL_CloseAudio();
        SDL_Quit();
        return;
    }

    SDL_PauseAudio(0);

    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {

        if (buffer.len > 0) continue;

        buffer.len = file.read(data, BUFFER_SIZE);

        // 处理线程退出了还没播放完
        if (buffer.len <= 0) {
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        }

        buffer.data = data;
    }

    file.close();

    // 关闭设备
    SDL_CloseAudio();
    // 关闭所有子系统
    SDL_Quit();
}
