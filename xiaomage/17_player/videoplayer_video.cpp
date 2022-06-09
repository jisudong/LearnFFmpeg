#include <videoplayer.h>
#include <QDebug>
#include <thread>

extern "C" {
#include <libavutil/imgutils.h>
}

int VideoPlayer::initVideo() {
    int ret = initDecoder(&_vCodecCtx, &_vStream, AVMEDIA_TYPE_VIDEO);
    if (ret < 0) {
        return ret;
    }

    ret = initSws();
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int VideoPlayer::initSws() {
    int w = _vCodecCtx->width;
    int h = _vCodecCtx->height;

    _vSwsOutSpec.width = w >> 4 << 4;
    _vSwsOutSpec.height = h >> 4 << 4;
    _vSwsOutSpec.pixFmt = AV_PIX_FMT_RGB24;
    _vSwsOutSpec.size = av_image_get_buffer_size(_vSwsOutSpec.pixFmt,
                                                 _vSwsOutSpec.width,
                                                 _vSwsOutSpec.height, 1);

    _vSwsCtx = sws_getContext(w, h, _vCodecCtx->pix_fmt,
                              _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.pixFmt,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!_vSwsCtx) {
        qDebug() << "sws_getContext error";
        return -1;
    }

    _vSwsInFrame = av_frame_alloc();
    if (!_vSwsInFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    _vSwsOutFrame = av_frame_alloc();
    if (!_vSwsOutFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    int ret = av_image_alloc(_vSwsOutFrame->data, _vSwsOutFrame->linesize,
                             _vSwsOutSpec.width, _vSwsOutSpec.height,
                             _vSwsOutSpec.pixFmt, 1);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "swr_init error:" << errbuf;
        return ret;
    }

    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt) {
    _vMutex.lock();
    _vPktList.push_back(pkt);
    _vMutex.signal();
    _vMutex.unlock();
}

void VideoPlayer::clearVideoPktList() {
    _vMutex.lock();
    for (AVPacket &pkt : _vPktList) {
        av_packet_unref(&pkt);
    }
    _vPktList.clear();
    _vMutex.unlock();
}

void VideoPlayer::freeVideo() {
    clearVideoPktList();
    avcodec_free_context(&_vCodecCtx);
    av_frame_free(&_vSwsInFrame);
    if (_vSwsOutFrame) {
        av_freep(&_vSwsOutFrame->data[0]);
        av_frame_free(&_vSwsOutFrame);
    }
    sws_freeContext(_vSwsCtx);
    _vSwsCtx = nullptr;
    _vStream = nullptr;
    _vTime = 0;
    _vCanfree = false;
    _vSeekTime = -1;
}

void VideoPlayer::decodeVideo() {
    while (true) {
        if (_state == Paused && _vSeekTime == -1) {
            continue;
        }

        if (_state == Stopped) {
            _vCanfree = true;
            break;
        }

        _vMutex.lock();

        if (_vPktList.empty()) {
            _vMutex.unlock();
            continue;
        }

        AVPacket pkt = _vPktList.front();
        _vPktList.pop_front();
        _vMutex.unlock();

        if (pkt.dts != AV_NOPTS_VALUE) {
            _vTime = av_q2d(_vStream->time_base) * pkt.dts;
        }

        int ret = avcodec_send_packet(_vCodecCtx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "avcodec_send_packet error:" << errbuf;
            continue;;
        }

        while (true) {
            ret = avcodec_receive_frame(_vCodecCtx, _vSwsInFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                char errbuf[1024];
                av_strerror(ret, errbuf, sizeof (errbuf));
                qDebug() << "avcodec_receive_frame error:" << errbuf;
                break;
            }

            if (_vSeekTime >= 0) {
                if (_vTime < _vSeekTime) {
                    continue;
                } else {
                    _vSeekTime = -1;
                }
            }

            sws_scale(_vSwsCtx, _vSwsInFrame->data, _vSwsInFrame->linesize, 0,
                      _vCodecCtx->height, _vSwsOutFrame->data, _vSwsOutFrame->linesize);

            if (_hasAudio) {
                while (_vTime > _aTime && _state == Playing) {
                    SDL_Delay(2);
                }
            } else {
                // 没有音频的情况
            }

            uint8_t *data = (uint8_t *) av_malloc(_vSwsOutSpec.size);
            memcpy(data, _vSwsOutFrame->data[0], _vSwsOutSpec.size);
            emit videoDecoded(this, data, _vSwsOutSpec);
        }
    }
}
