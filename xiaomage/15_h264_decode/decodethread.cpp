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
    VideoDecodeSpec spec;
    spec.filename = "/Users/jisudong/Documents/decode.yuv";

    FFmpegUtil::h264Decode("/Users/jisudong/Documents/Titanic.h264", spec);
}
