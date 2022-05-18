#include "ffmpegutil.h"

#include <QFile>
#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define IN_DATA_SIZE 20480
#define REFILL_THRESH 4096

FFmpegUtil::FFmpegUtil()
{

}

int decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, QFile &outfile) {
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0){
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "avcodec_send_packet error" << errbuf;
            return ret;
        }

        outfile.write((char *) frame->data[0], frame->linesize[0]);
    }

}

void FFmpegUtil::aacDecode(const char *in, AudioDecodeSpec &out) {
    QFile inFile(in);
    QFile outFile(out.filename);

    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    AVCodecParserContext *parserCtx = nullptr;

    int ret = 0;
    int inLen = 0;
    int isEnd = 0;

    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = inDataArray;

    codec = avcodec_find_decoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "decode not found";
        return;
    }

    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        qDebug() << "av_parser_init error";
        return;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_open error:" << errbuf;
        goto end;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << in;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    inLen = inFile.read(inDataArray, IN_DATA_SIZE);
    while (inLen > 0) {
        ret = av_parser_parse2(parserCtx, ctx, &pkt->data, &pkt->size,
                                (uint8_t *) inData, inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_parser_parse error:" << errbuf;
            goto end;
        }

        inData += ret;
        inLen -= ret;

        if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
            goto end;
        }

        if (inLen < REFILL_THRESH && !isEnd) {
            memmove(inDataArray, inData, inLen);
            inData = inDataArray;
            int len = inFile.read(inData + inLen, IN_DATA_SIZE - inLen);
            if (len > 0) {
                inLen += len;
            } else {
                isEnd = 1;
            }
        }
    }

    decode(ctx, nullptr, frame, outFile);

    out.sampleRate = ctx->sample_rate;
    out.sampleFmt = ctx->sample_fmt;
    out.chLayout = ctx->channel_layout;

end:
    inFile.close();
    outFile.close();
    av_parser_close(parserCtx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
}
