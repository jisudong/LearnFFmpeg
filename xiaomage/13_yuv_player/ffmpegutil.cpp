#include "ffmpegutil.h"

#include <QDebug>
#include <QFile>


extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

FFmpegUtil::FFmpegUtil()
{

}

void FFmpegUtil::convertVideo(RawVideoFrame &in, RawVideoFrame &out) {
    uint8_t *inData[4];
    uint8_t *outData[4];
    int inStrides[4];
    int outStrides[4];
    int inFrameSize = 0;
    int outFrameSize = 0;
    int ret;

    SwsContext *ctx = sws_getContext(in.width, in.height, in.format,
                                     out.width, out.height, out.format,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        return;
    }

    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        sws_freeContext(ctx);
        return;
    }

    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        av_freep(&inData[0]);
        sws_freeContext(ctx);
        return;
    }

    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    memcpy(inData[0], in.pixels, inFrameSize);

    sws_scale(ctx, inData, inStrides, 0, in.height, outData, outStrides);

    out.pixels = (char *) malloc(outFrameSize);
    memcpy(out.pixels, outData[0], outFrameSize);

    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}

void FFmpegUtil::convertVideo(RawVideoFile &in, RawVideoFile &out) {

    uint8_t *inData[4];
    uint8_t *outData[4];
    int inStrides[4];
    int outStrides[4];
    int inFrameSize = 0;
    int outFrameSize = 0;
    int ret;

    SwsContext *ctx = sws_getContext(in.width, in.height, in.format,
                                     out.width, out.height, out.format,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        return;
    }

    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        sws_freeContext(ctx);
        return;
    }

    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        av_freep(&inData[0]);
        sws_freeContext(ctx);
        return;
    }

    ret = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        av_freep(&inData[0]);
        av_freep(&outData[0]);
        sws_freeContext(ctx);
        return;
    }
    inFrameSize = ret;

    ret = av_image_get_buffer_size(out.format, out.width, out.height, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error" << errbuf;

        av_freep(&inData[0]);
        av_freep(&outData[0]);
        sws_freeContext(ctx);
        return;
    }
    outFrameSize = ret;

    QFile inFile(in.filename);
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << in.filename;

        av_freep(&inData[0]);
        av_freep(&outData[0]);
        sws_freeContext(ctx);
        return;
    }

    QFile outFile(out.filename);
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out.filename;

        inFile.close();
        av_freep(&inData[0]);
        av_freep(&outData[0]);
        sws_freeContext(ctx);
        return;
    }

    while (inFile.read((char *) inData[0], inFrameSize) == inFrameSize) {
        sws_scale(ctx, inData, inStrides, 0, in.height, outData, outStrides);
        outFile.write((char *) outData[0], outFrameSize);
    }

    inFile.close();
    outFile.close();
    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}
