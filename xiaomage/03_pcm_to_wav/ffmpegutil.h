#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

#include <stdint.h>

typedef struct {
    uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'};
    uint32_t riffChunkSize;
    uint8_t format[4] = {'W', 'A', 'V', 'E'};
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};
    uint32_t dataChunkSize;
} WAVHeader;

class FFmpegUtil
{
public:
    FFmpegUtil();

    static void pcm2wav(WAVHeader &header, const char *pcm, const char *wav);
};

#endif // FFMPEGUTIL_H
