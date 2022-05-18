#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

extern "C" {
#include <libavformat/avformat.h>
}

typedef struct {
    const char *filename;
    int sampleRate;
    int chLayout;
    AVSampleFormat sampleFmt;
} AudioDecodeSpec;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void aacDecode(const char *in, AudioDecodeSpec &out);
};

#endif // FFMPEGUTIL_H
