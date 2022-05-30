#include "demuxer.h"

#include <QDebug>

Demuxer::Demuxer()
{

}

void Demuxer::demux(const char *filename, AudioDecodeSpec &aout, VideoDecodeSpec &vout) {
    _aout = &aout;
    _vout = &vout;

    AVPacket *pkt = nullptr;

    int ret = 0;

    ret = avformat_open_input(&_fmtCtx, filename, nullptr, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avformat_open_input error:" << errbuf;
        return;
    }

    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avformat_find_stream_info error:" << errbuf;
        goto end;
    }

    av_dump_format(_fmtCtx, 0, filename, 0);
    fflush(stderr);

    ret = initAudioInfo();
    if (ret < 0) {
        goto end;
    }

    ret = initVideoInfo();
    if (ret < 0) {
        goto end;
    }

    _frame = av_frame_alloc();
    if (!_frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    pkt = av_packet_alloc();
    pkt->data = nullptr;
    pkt->size = 0;
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    while (av_read_frame(_fmtCtx, pkt) == 0) {
        if (pkt->stream_index == _aStreamIdx) {
            ret = decode(_aCodecCtx, pkt, &Demuxer::writeAudioFrame);
        } else if (pkt->stream_index == _vStreamIdx) {
            ret = decode(_vCodecCtx, pkt, &Demuxer::writeVideoFrame);
        }

        av_packet_unref(pkt);

        if (ret < 0) {
            goto end;
        }
    }

    decode(_aCodecCtx, nullptr, &Demuxer::writeAudioFrame);
    decode(_vCodecCtx, nullptr, &Demuxer::writeVideoFrame);

end:
    _aFile.close();
    _vFile.close();
    av_frame_free(&_frame);
    av_packet_free(&pkt);
    avformat_close_input(&_fmtCtx);
    avcodec_free_context(&_aCodecCtx);
    avcodec_free_context(&_vCodecCtx);
    av_freep(&_imgBuf[0]);
}

int Demuxer::initAudioInfo() {
    int ret = initDecoder(&_aCodecCtx, &_aStreamIdx, AVMEDIA_TYPE_AUDIO);
    if (ret < 0) {
        return ret;
    }

    _aFile.setFileName(_aout->filename);
    if (!_aFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error";
        return -1;
    }

    _aout->sampleFmt = _aCodecCtx->sample_fmt;
    _aout->sampleRate = _aCodecCtx->sample_rate;
    _aout->chLayout = _aCodecCtx->channel_layout;

    _sampleSize = av_get_bytes_per_sample(_aCodecCtx->sample_fmt);
    _sampleFrameSize = _sampleSize * _aCodecCtx->channels;

    return 0;
}

int Demuxer::initVideoInfo() {
    int ret = initDecoder(&_vCodecCtx, &_vStreamIdx, AVMEDIA_TYPE_VIDEO);
    if (ret < 0) {
        return ret;
    }

    _vFile.setFileName(_vout->filename);
    if (!_vFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error";
        return -1;
    }

    _vout->width = _vCodecCtx->width;
    _vout->height = _vCodecCtx->height;
    _vout->pixFmt = _vCodecCtx->pix_fmt;
    AVStream *stream = _fmtCtx->streams[_vStreamIdx];
    AVRational framerate = av_guess_frame_rate(_fmtCtx, stream, nullptr);
    _vout->fps = framerate.num / framerate.den;

    ret = av_image_alloc(_imgBuf, _imgLinesize, _vout->width, _vout->height, _vout->pixFmt, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error:" << errbuf;
        return ret;
    }

    _imgSize = ret;

    return 0;
}

int Demuxer::initDecoder(AVCodecContext **ctx, int *streamIdx, AVMediaType type) {
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_image_alloc error:" << errbuf;
        return ret;
    }

    *streamIdx = ret;
    AVStream *stream = _fmtCtx->streams[*streamIdx];
    if (!stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        qDebug() << "decode not found";
        return -1;
    }

    *ctx = avcodec_alloc_context3(codec);
    if (!_aCodecCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    ret = avcodec_parameters_to_context(*ctx, stream->codecpar);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_parameters_to_context error:" << errbuf;
        return ret;
    }

    ret = avcodec_open2(*ctx, codec, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_open2 error:" << errbuf;
        return ret;
    }

    return 0;
}

int Demuxer::decode(AVCodecContext *ctx, AVPacket *pkt, void (Demuxer::*func)()) {
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_send_packet error:" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_frame(ctx, _frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        if (ret < 0) {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "avcodec_receive_frame error:" << errbuf;
            return ret;
        }

        // 写文件
        (this->*func)();
    }
}

void Demuxer::writeAudioFrame() {
    if (av_sample_fmt_is_planar(_aout->sampleFmt) ) {
        // 每个声道的样本数
        for (int si = 0; si < _frame->nb_samples; si++) {
            // 有多少个声道
            for (int ci = 0; ci < _aCodecCtx->channels; ci++) {
                char *begin = (char *) (_frame->data[ci] + si * _sampleSize);
                _aFile.write(begin, _sampleSize);
            }
        }
    } else {
        _aFile.write((char *) _frame->data[0], _frame->nb_samples * _sampleFrameSize);
    }
}

void Demuxer::writeVideoFrame() {
    av_image_copy(_imgBuf, _imgLinesize,
                  (const uint8_t **) _frame->data, _frame->linesize,
                  _vout->pixFmt, _vout->width, _vout->height);

    _vFile.write((char *) _imgBuf[0], _imgSize);
}
