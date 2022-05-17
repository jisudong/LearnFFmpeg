#include "encodethread.h"

#include <QDebug>
#include "ffmpegutil.h"

EncodeThread::EncodeThread(QObject *parent) : QThread(parent)
{
    connect(this, &EncodeThread::finished, this, &EncodeThread::deleteLater);
}

EncodeThread::~EncodeThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程安全结束";
}

void EncodeThread::run() {
    AudioEncodeSpec in;
    in.filename = "/Users/jisudong/Documents/in.pcm";
    in.sampleRate = 44100;
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.chLayout = AV_CH_LAYOUT_STEREO;
    FFmpegUtil::aacEncode(in, "/Users/jisudong/Documents/out.aac");
}
