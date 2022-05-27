#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H


extern "C" {
#include <libavutil/avutil.h>
}

typedef struct {
    const char *filename = nullptr;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void h264Decode(const char *in, VideoDecodeSpec &out);
};

#endif // FFMPEGUTIL_H
