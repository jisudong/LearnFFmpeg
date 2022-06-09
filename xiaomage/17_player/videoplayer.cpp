#include "videoplayer.h"

#include <QDebug>
#include <thread>

#define AUDIO_MAX_PKT_SIZE 1000
#define VIDEO_MAX_PKT_SIZE 500


VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error:" << SDL_GetError();
        emit playFailed(this);
        return;
    }
}

VideoPlayer::~VideoPlayer() {

    disconnect();
    stop();
    SDL_Quit();

}

void VideoPlayer::play() {
    if (_state == Playing) return;

    if (_state == Stopped) {
        std::thread([this]() {
            readFile();
        }).detach();
    } else {
        setState(Playing);
    }
}

void VideoPlayer::pause() {
    if (_state != Playing) return;

    setState(Paused);
}

void VideoPlayer::stop() {
    if (_state == Stopped) return;

    _state = Stopped;
    free();
    emit stateChanged(_state);
}

bool VideoPlayer::isPlaying() {
    return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState() {
    return _state;
}

void VideoPlayer::setFilename(QString &filename) {
   const char *name = filename.toUtf8().data();
   memcpy(_filename, name, strlen(name) + 1);
}

int VideoPlayer::getDuration() {
    return _fmtCtx ? round(_fmtCtx->duration * av_q2d(AV_TIME_BASE_Q)) : 0;
}

int VideoPlayer::getTime() {
    return round(_aTime);
}

void VideoPlayer::setTime(int seekTime) {
    _seekTime = seekTime;
}

void VideoPlayer::setVolume(int volume) {
    _volume = volume;
}

int VideoPlayer::getVolume() {
    return _volume;
}

void VideoPlayer::setMute(bool mute) {
    _mute = mute;
}

bool VideoPlayer::isMute() {
    return _mute;
}


void VideoPlayer::readFile() {
    qDebug() << _filename;
    int ret = avformat_open_input(&_fmtCtx, _filename, nullptr, nullptr);
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
        return;;
    }

    av_dump_format(_fmtCtx, 0, _filename, 0);
    fflush(stderr);

    _hasAudio = initAudio() >= 0;
    _hasVideo = initVideo() >= 0;
    if (!_hasAudio && !_hasVideo) {
        fatalError();
        return;;
    }

    emit initFinished(this);

    setState(Playing);

    SDL_PauseAudio(0);

    std::thread([this]() {
        decodeVideo();
    }).detach();

    AVPacket pkt;
    while (_state != Stopped) {
        if (_seekTime >= 0) {
            int streamIdx;
            if (_hasAudio) {
                streamIdx = _aStream->index;
            } else {
                streamIdx = _vStream->index;
            }
            AVRational timebase = _fmtCtx->streams[streamIdx]->time_base;
            int64_t ts = _seekTime / av_q2d(timebase);
            ret = av_seek_frame(_fmtCtx, streamIdx, ts, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                qDebug() << "seek失败";
                _seekTime = -1;
            } else {
                clearAudioPktList();
                clearVideoPktList();
                _vSeekTime = _seekTime;
                _aSeekTime = _seekTime;
                _seekTime = -1;
                _aTime = 0;
                _vTime = 0;
            }
        }

        int vsize = _vPktList.size();
        int asize = _aPktList.size();

        if (vsize >= VIDEO_MAX_PKT_SIZE || asize >= AUDIO_MAX_PKT_SIZE) {
            continue;
        }

        ret = av_read_frame(_fmtCtx, &pkt);
        if (ret == 0) {
            if (pkt.stream_index == _aStream->index) {
                addAudioPkt(pkt);
            } else if (pkt.stream_index == _vStream->index) {
                addVideoPkt(pkt);
            } else {
                av_packet_unref(&pkt);
            }
        } else if (ret == AVERROR_EOF) {
            if (vsize == 0 && asize == 0) {
                _fmttCtxCanFree = true;
                break;
            }
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error:" << errbuf;
            continue;
        }
    }

    if (_fmttCtxCanFree) {
        stop();
    } else {
        _fmttCtxCanFree = true;
    }
}

int VideoPlayer::initDecoder(AVCodecContext **ctx, AVStream **stream, AVMediaType type) {
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "av_find_best_stream error:" << errbuf;
        return ret;
    }

    int streamIdx = ret;
    *stream = _fmtCtx->streams[streamIdx];
    if (!*stream) {
        qDebug() << "stream not found!";
        return -1;
    }

    AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found!";
        return -1;
    }

    *ctx = avcodec_alloc_context3(decoder);
    if (!*ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    ret = avcodec_parameters_to_context(*ctx, (*stream)->codecpar);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_parameters_to_context error:" << errbuf;
        return ret;
    }

    ret = avcodec_open2(*ctx, decoder, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avcodec_open2 error:" << errbuf;
        return ret;
    }

    return 0;
}

void VideoPlayer::setState(State state) {
    if (_state == state) return;

    _state = state;

    emit stateChanged(_state);
}

void VideoPlayer::free() {
    while (_hasAudio && !_aCanFree);
    while (_hasVideo && !_vCanfree);
    while (!_fmttCtxCanFree);

    avformat_close_input(&_fmtCtx);
    _fmttCtxCanFree = false;
    _seekTime = -1;
    freeAudio();
    freeVideo();
}

void VideoPlayer::fatalError() {
    _state = Playing;
    stop();
    emit playFailed(this);
}
