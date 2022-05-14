
#include "ffmpegutil.h"

#include <QFile>
#include <QDebug>

extern "C" {
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

FFmpegUtil::FFmpegUtil()
{

}

void FFmpegUtil::resample(AudioResampleSpec &in, AudioResampleSpec &out) {

    QFile inFile(in.filename);
    QFile outFile(out.filename);

    uint8_t **inData = nullptr;
    int inLinesize = 0;
    int inChannels = av_get_channel_layout_nb_channels(in.chLayout);
    int inBytesPerSample = inChannels * av_get_bytes_per_sample(in.sampleFormat);
    int inSamples = 1024;

    uint8_t **outData = nullptr;
    int outLinesize = 0;
    int outChannels = av_get_channel_layout_nb_channels(out.chLayout);
    int outBytesPerSample = outChannels * av_get_bytes_per_sample(out.sampleFormat);
    int outSamples = av_rescale_rnd(inSamples, out.sampleRate, in.sampleRate, AV_ROUND_UP);

    int ret = 0;
    int len = 0;

    SwrContext *ctx = swr_alloc_set_opts(nullptr, out.chLayout, out.sampleFormat, out.sampleRate,
                                         in.chLayout, in.sampleFormat, in.sampleRate, 0, nullptr);
    if (!ctx) {
        qDebug() << "swr_alloc_set_opts error";
        return;
    }

    ret = swr_init(ctx);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "swr_init error:" << errbuf;
        goto end;
    }

    ret = av_samples_alloc_array_and_samples(&inData, &inLinesize, inChannels,
                                             inSamples, in.sampleFormat, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }

    ret = av_samples_alloc_array_and_samples(&outData, &outLinesize, outChannels,
                                             outSamples, out.sampleFormat, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "文件打开失败：" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败：" << out.filename;
        goto end;
    }

    while ((len = inFile.read((char *)inData[0], inLinesize)) > 0) {
        inSamples = len / inBytesPerSample;
        ret = swr_convert(ctx, outData, outSamples, (const uint8_t **) inData, inSamples);
        if (ret < 0) {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "swr_convert error:" << errbuf;
            goto end;
        }

//        int size = av_samples_get_buffer_size(nullptr, outChannels, ret, out.sampleFormat, 1);
//        outFile.write((char *) outData[0], size);

        outFile.write((char *) outData[0], ret * outBytesPerSample);
    }

    // 输出缓冲区的残留样本（已经重采样的）
    while ((ret = swr_convert(ctx, outData, outSamples, nullptr, 0)) > 0) {
        outFile.write((char *) outData[0], ret * outBytesPerSample);
    }

end:
    inFile.close();
    outFile.close();

    swr_free(&ctx);

    if (inData) {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    if (outData) {
        av_freep(&outData[0]);
    }
    av_freep(&outData);
}
