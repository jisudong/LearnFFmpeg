#include "decodethread.h"

#include <QDebug>
#include "ffmpegutil.h"

DecodeThread::DecodeThread(QObject *parent) : QThread(parent)
{
    connect(this, &DecodeThread::finished, this, &DecodeThread::deleteLater);
}

DecodeThread::~DecodeThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程安全退出";
}

void DecodeThread::run() {
    AudioDecodeSpec spec;
    spec.filename = "/Users/jisudong/Documents/out.pcm";

    FFmpegUtil::aacDecode("/Users/jisudong/Documents/in.aac", spec);

    qDebug() << spec.sampleRate;
    qDebug() << av_get_sample_fmt_name(spec.sampleFmt);
    qDebug() << av_get_channel_layout_nb_channels(spec.chLayout);

}
