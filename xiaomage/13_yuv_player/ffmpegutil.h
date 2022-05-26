#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

extern "C" {
#include <libavutil/avutil.h>
}

typedef struct {
    char *pixels;
    int width;
    int height;
    AVPixelFormat format;
} RawVideoFrame;


typedef struct {
    const char *filename = nullptr;
    int width;
    int height;
    AVPixelFormat format;
} RawVideoFile;


class FFmpegUtil
{
public:
    FFmpegUtil();

    static void convertVideo(RawVideoFrame &in, RawVideoFrame &out);
    static void convertVideo(RawVideoFile &in, RawVideoFile &out);
};

#endif // FFMPEGUTIL_H
