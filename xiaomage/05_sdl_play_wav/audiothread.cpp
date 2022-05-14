#include "audiothread.h"

#include <SDL2/SDL.h>
#include <QDebug>

#define FILENAME "/Users/jisudong/Documents/in.wav"

typedef struct {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;

AudioThread::AudioThread(QObject *parent) : QThread(parent)
{
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "线程安全退出";
}

void pull_audio_data(void *userdata, Uint8 * stream, int len) {

    memset(stream, 0, len);
    AudioBuffer *buffer = (AudioBuffer *)userdata;
    if (buffer->len <= 0) return;
    buffer->pullLen = len > buffer->len ? buffer->len : len;
    SDL_MixAudio(stream, buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void AudioThread::run() {

    int ret = SDL_Init(SDL_INIT_AUDIO);
    if (ret < 0) {
        qDebug() << "SDL_Init error:" << SDL_GetError();
        return;
    }

    SDL_AudioSpec spec;
    Uint8 *data = nullptr;
    Uint32 len;
    if (!SDL_LoadWAV(FILENAME, &spec, &data, &len)) {
        qDebug() << "SDL_LoadWAV error:" << SDL_GetError();
        SDL_Quit();
        return;
    }

    AudioBuffer buffer;
    buffer.len = len;
    buffer.data = data;

    spec.samples = 1024;
    spec.callback = pull_audio_data;
    spec.userdata = &buffer;

    ret = SDL_OpenAudio(&spec, nullptr);
    if (ret < 0) {
        qDebug() << "SDL_OpenAudio error:" << SDL_GetError();
        SDL_Quit();
        return;
    }

    SDL_PauseAudio(0);

    int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
    int bytesPerSample = (sampleSize * spec.channels) >> 3;
    while (!isInterruptionRequested()) {

        if (buffer.len <= 0) {
            // 剩余样本数量
            int samples = buffer.pullLen / bytesPerSample;
            int ms = samples * 1000 / spec.freq;
            SDL_Delay(ms);
            break;
        }
    }

    SDL_FreeWAV(data);
    SDL_CloseAudio();
    SDL_Quit();
}
