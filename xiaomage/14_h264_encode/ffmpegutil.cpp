#include "ffmpegutil.h"

#include <QFile>
#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

FFmpegUtil::FFmpegUtil()
{

}

static int check_pix_fmt(AVCodec *codec, AVPixelFormat pixFmt) {
    const enum AVPixelFormat *p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pixFmt) {
            return 1;
        }
        p++;
    }
    return 0;
}

static int encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, QFile &file) {
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_send_frame error:" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == 0) {
            file.write((char *) pkt->data, pkt->size);
            av_packet_unref(pkt);
        } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "avcodec_receive_packet error:" << errbuf;
            return ret;
        }
    }
}


void FFmpegUtil::h264Encode(VideoEncodeSpec &in, const char *out) {
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    QFile inFile(in.filename), outFile(out);
    int imageSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1);
    int len = 0;
    int ret = 0;

    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        qDebug() << "avcodec_find_decoder_by_name error";
        return;
    }

    if (!check_pix_fmt(codec, in.pixFmt)) {
       qDebug() << "unsupport format:" << in.pixFmt;
       return;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context error";
        return;
    }
    ctx->width = in.width;
    ctx->height = in.height;
    ctx->pix_fmt = in.pixFmt;
    ctx->time_base = {1, in.fps};

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        qDebug() << "avcodec_open error:" << errbuf;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }
    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    frame->pts = 0;

    ret = av_image_alloc(frame->data, frame->linesize, in.width, in.height, in.pixFmt, 1);
    if (ret < 0) {
        char errbuf[1024];
        qDebug() << "av_image_alloc error:" << errbuf;
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << in.filename;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out;
        goto end;
    }

    while ((len = inFile.read((char *) frame->data[0], imageSize)) > 0) {

        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }

        frame->pts++;
    }

    encode(ctx, frame, pkt, outFile);

end:
    inFile.close();
    outFile.close();
    if (frame) {
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
    }
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
}
