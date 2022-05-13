#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

extern "C" {
#include <libavformat/avformat.h>
}

typedef struct {
    const char *filename;
    int sampleRate;
    AVSampleFormat sampleFormat;
    int chLayout;
} AudioResampleSpec;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void resample(AudioResampleSpec &in, AudioResampleSpec &out);
};

#endif // FFMPEGUTIL_H
