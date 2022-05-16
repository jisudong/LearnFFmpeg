#include "recordthread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#define FMT_NAME "avfoundation"
#define DEVICE_NAME ":1"
#define FILEPATH "/Users/jisudong/Documents/"

typedef struct {
    uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'};
    uint32_t riffChunkSize;
    uint8_t format[4] = {'W', 'A', 'V', 'E'};
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};
    uint32_t dataChunkSize;
} WAVHeader;

RecordThread::RecordThread(QObject *parent) : QThread(parent)
{
    connect(this, &RecordThread::finished, this, &RecordThread::deleteLater);
}

RecordThread::~RecordThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程安全退出";
}

void RecordThread::run() {
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "av_find_input_format error:" << FMT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avformat_open_input" << errbuf;
        return;
    }

    QString filename = FILEPATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".wav";
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << filename;
        avformat_close_input(&ctx);
        return;
    }

    AVStream *stream = ctx->streams[0];
    AVCodecParameters *param = stream->codecpar;
    WAVHeader header;
    header.dataChunkSize = 0;
    header.numChannels = param->channels;
    header.sampleRate = param->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(param->codec_id);
    header.blockAlign = header.numChannels * header.bitsPerSample >> 3;
    header.byteRate = header.blockAlign * header.sampleRate;
    if (param->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = 3;
    }

    file.write((char *) &header, sizeof (WAVHeader));

    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) {
            file.write((char *) pkt->data, pkt->size);
            header.dataChunkSize += pkt->size;

            unsigned long long ms = 1000 * header.dataChunkSize / header.byteRate;
            emit timeChanged(ms);

        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error:" << errbuf;
            break;
        }
        av_packet_unref(pkt);
    }

    file.seek(sizeof (WAVHeader) - sizeof (header.dataChunkSize));
    file.write((char *) &header.dataChunkSize, sizeof (header.dataChunkSize));

    header.riffChunkSize = header.dataChunkSize + sizeof (WAVHeader)
                            - sizeof (header.riffChunkId) - sizeof (header.riffChunkSize);
    file.seek(sizeof (header.riffChunkId));
    file.write((char *) &header.riffChunkSize, sizeof (header.riffChunkSize));

    file.close();
    av_packet_free(&pkt);
    avformat_close_input(&ctx);
}
