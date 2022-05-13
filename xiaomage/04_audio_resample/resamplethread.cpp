#include "resamplethread.h"
#include "ffmpegutil.h"
#include <QDebug>

extern "C" {
#include <libavformat/avformat.h>
}

ResampleThread::ResampleThread(QObject *parent) : QThread(parent)
{
    connect(this, &ResampleThread::finished, this, &ResampleThread::deleteLater);
}

ResampleThread::~ResampleThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "线程安全退出";
}

void ResampleThread::run() {

    AudioResampleSpec inSpec;
    inSpec.filename = "/Users/jisudong/Documents/44100_s16le_2.pcm";
    inSpec.sampleRate = 44100;
    inSpec.sampleFormat = AV_SAMPLE_FMT_S16;
    inSpec.chLayout = AV_CH_LAYOUT_STEREO;

    AudioResampleSpec outSpec;
    outSpec.filename = "/Users/jisudong/Documents/48000_f32le_1.pcm";
    outSpec.sampleRate = 48000;
    outSpec.sampleFormat = AV_SAMPLE_FMT_FLT;
    outSpec.chLayout = AV_CH_LAYOUT_MONO;

    FFmpegUtil::resample(inSpec, outSpec);
}
