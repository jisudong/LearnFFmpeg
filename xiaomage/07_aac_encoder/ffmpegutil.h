#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

extern "C" {
#include <libavformat/avformat.h>
}

typedef struct {
    const char *filename = nullptr;
    int sampleRate = 0;
    AVSampleFormat sampleFmt;
    int chLayout = 0;
} AudioEncodeSpec;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void aacEncode(AudioEncodeSpec &in, const char *out);
};

#endif // FFMPEGUTIL_H
