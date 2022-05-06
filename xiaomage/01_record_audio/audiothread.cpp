#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

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

Audiothread::Audiothread(QObject *parent) : QThread(parent)
{
    connect(this, &Audiothread::finished, this, &Audiothread::deleteLater);
}

Audiothread::~Audiothread() {
    disconnect();

    requestInterruption();
    quit();
    wait();

    qDebug() << this << "析构（内存被回收）";
}

void showSpec(AVFormatContext *ctx) {
    AVStream *stream = ctx->streams[0];
    AVCodecParameters *param = stream->codecpar;
    qDebug() << "声道数：" << param->channels;
    qDebug() << "采样率：" << param->sample_rate;
    qDebug() << "采样格式：" << param->format;
    qDebug() << "每一个样本的一个声道占用字节数：" << av_get_bytes_per_sample((AVSampleFormat)param->format);
}

void Audiothread::run() {
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf,sizeof(errbuf));
        qDebug() << "打开上下文失败" << errbuf;
        return;
    }

    showSpec(ctx);

    QString filename = FILE_PATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "打开文件失败" << filename;

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
            av_strerror(ret, errbuf, sizeof(errbuf));
            qDebug() << "录音错误：" << errbuf;

            av_packet_unref(pkt);
            break;
        }
    }

    av_packet_free(&pkt);
    file.close();
    avformat_close_input(&ctx);
}
