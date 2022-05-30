#ifndef DEMUXER_H
#define DEMUXER_H

#include <QFile>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

typedef struct {
    const char *filename = nullptr;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} AudioDecodeSpec;

typedef struct {
    const char *filename = nullptr;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;

class Demuxer
{
public:
    Demuxer();

    void demux(const char *filename, AudioDecodeSpec &aout, VideoDecodeSpec &vout);

private:
    AVFormatContext *_fmtCtx = nullptr;
    AVCodecContext *_aCodecCtx = nullptr;
    AVCodecContext *_vCodecCtx = nullptr;
    AVFrame *_frame = nullptr;
    int _aStreamIdx = 0;
    int _vStreamIdx = 0;
    QFile _aFile, _vFile;
    AudioDecodeSpec *_aout = nullptr;
    VideoDecodeSpec *_vout = nullptr;
    uint8_t *_imgBuf[4] = { nullptr };
    int _imgLinesize[4] = { 0 };
    int _imgSize = 0;
    int _sampleFrameSize = 0;
    int _sampleSize = 0;

    int initVideoInfo();
    int initAudioInfo();
    int initDecoder(AVCodecContext **ctx, int *streamIdx, AVMediaType type);
    int decode(AVCodecContext *ctx, AVPacket *pkt, void (Demuxer::*func)());
    void writeVideoFrame();
    void writeAudioFrame();
};

#endif // DEMUXER_H
