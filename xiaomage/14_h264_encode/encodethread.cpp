#include "encodethread.h"

#include <QDebug>
#include "ffmpegutil.h"

extern "C" {
#include <libavutil/avutil.h>
}

EncodeThread::EncodeThread(QObject *parent) : QThread(parent)
{
    connect(this, &EncodeThread::finished, this, &EncodeThread::deleteLater);
}

EncodeThread::~EncodeThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "线程安全退出";
}

void EncodeThread::run() {
    VideoEncodeSpec spec;
    spec.filename = "/Users/jisudong/Documents/Titanic.yuv";
    spec.width = 640;
    spec.height = 272;
    spec.pixFmt = AV_PIX_FMT_YUV420P;
    spec.fps = 24;

    FFmpegUtil::h264Encode(spec, "/Users/jisudong/Documents/Titanic.h264");
}
