#include "recordthread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
}

#ifdef Q_OS_MAC
#define FMT_NAME "avfoundation"
#define DEVICE_NAME "0"
#define FILENAME "/Users/jisudong/Documents/out.yuv"
#else

#endif

RecordThread::RecordThread(QObject *parent) : QThread(parent)
{
    connect(this, &RecordThread::finished, this, &RecordThread::deleteLater);
}

RecordThread::~RecordThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "线程安全结束";
}

void RecordThread::run() {
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "av_find_input_format error:" << FMT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;
    AVDictionary *options;
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "framerate", "30", 0);

    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avformat_open_input error:" << errbuf;
        return;
    }

    QFile file(FILENAME);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << FILENAME;
        avformat_close_input(&ctx);
        return;
    }

    AVCodecParameters *params = ctx->streams[0]->codecpar;
    int imagesize = av_image_get_buffer_size((AVPixelFormat) params->format,
                                             params->width,
                                             params->height,
                                             1);

    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) {
            file.write((char *) pkt->data, imagesize);
            av_packet_unref(pkt);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error:" << errbuf;
            break;
        }
    }

    file.close();
    av_packet_free(&pkt);
    avformat_close_input(&ctx);
}
