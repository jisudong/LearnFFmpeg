#include "ffmpegutil.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#define DATA_SIZE 4096

FFmpegUtil::FFmpegUtil()
{

}

static int decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, QFile &file) {
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_send_packet error:" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "avcodec_receive_frame error:" << errbuf;
            return ret;
        }

        file.write((char *)frame->data[0], frame->linesize[0] * ctx->height);
        file.write((char *)frame->data[1], frame->linesize[1] * ctx->height >> 1);
        file.write((char *)frame->data[2], frame->linesize[2] * ctx->height >> 1);
    }
}

void FFmpegUtil::h264Decode(const char *in, VideoDecodeSpec &out) {

    QFile inFile(in), outFile(out.filename);
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    AVCodecParserContext *parserCtx = nullptr;

    char inDataArray[DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = inDataArray;
    int inLen = 0;
    int isEnd = 0;
    int ret = 0;

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qDebug() << "avcodec_find_decoder error";
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

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_open error:" << errbuf;
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

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << in;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    do {
        inLen = inFile.read(inDataArray, DATA_SIZE);
        inData = inDataArray;
        isEnd = !inLen;

        while (inLen > 0 || isEnd) {
            ret = av_parser_parse2(parserCtx, ctx,
                                    &pkt->data, &pkt->size,
                                    (uint8_t *) inData, inLen,
                                    AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
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

            if (isEnd) {
                break;
            }
        }
    } while (!isEnd);

    decode(ctx, nullptr, frame, outFile);

    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    out.fps = ctx->framerate.num;

end:
    inFile.close();
    outFile.close();
    av_frame_free(&frame);
    av_packet_free(&pkt);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
}
