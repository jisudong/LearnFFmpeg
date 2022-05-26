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
} VideoEncodeSpec;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void h264Encode(VideoEncodeSpec &in, const char *out);
};

#endif // FFMPEGUTIL_H
