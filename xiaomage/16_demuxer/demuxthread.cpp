#include "demuxthread.h"

#include <QDebug>
#include "demuxer.h"

DemuxThread::DemuxThread(QObject *parent) : QThread(parent)
{
    connect(this, &DemuxThread::finished, this, &DemuxThread::deleteLater);
}

DemuxThread::~DemuxThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程安全退出";
}

void DemuxThread::run() {
    AudioDecodeSpec aout;
    aout.filename = "/Users/jisudong/Documents/Titanic_demuxer.pcm";

    VideoDecodeSpec vout;
    vout.filename = "/Users/jisudong/Documents/Titanic_demuxer.yuv";

    Demuxer().demux("/Users/jisudong/Documents/Titanic.mkv", aout, vout);

    qDebug() << aout.sampleRate << av_get_channel_layout_nb_channels(aout.chLayout)
             << av_get_sample_fmt_name(aout.sampleFmt);
    qDebug() << vout.width << vout.height << vout.fps << av_get_pix_fmt_name(vout.pixFmt);
}
