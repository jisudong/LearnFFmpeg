#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "ffmpegutil.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
}

#ifdef Q_OS_MAC
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":1"
    #define FILE_PATH "/Users/jisudong/Documents/"
#else

#endif

AudioThread::AudioThread(QObject *parent) : QThread(parent)
{
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
}

void AudioThread::run() {
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "打开输入设备失败" << errbuf;
        return;
    }

    QString filename = FILE_PATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    QString wavname = filename;
    filename += ".pcm";
    wavname += ".wav";
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;
        avformat_close_input(&ctx);
        return;
    }

    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) {
            file.write((const char *)pkt->data, pkt->size);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "录音错误" << errbuf;
            break;
        }
        av_packet_unref(pkt);
    }

    file.close();
    av_packet_free(&pkt);

    AVStream *stream = ctx->streams[0];
    AVCodecParameters *param = stream->codecpar;

    WAVHeader header;
    header.sampleRate = param->sample_rate;
    header.numChannels = param->channels;
    header.bitsPerSample = av_get_bits_per_sample(param->codec_id);
    if (param->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = 3;
    }
    FFmpegUtil::pcm2wav(header, filename.toUtf8().data(), wavname.toUtf8().data());

    avformat_close_input(&ctx);
}
