#include "ffmpegutil.h"

#include <QFile>
#include <QDebug>

FFmpegUtil::FFmpegUtil()
{

}

void FFmpegUtil::pcm2wav(WAVHeader &header, const char *pcm, const char *wav) {
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign >> 3;
    QFile pcmFile(pcm);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "打开文件失败" << pcm;
        return;
    }
    header.dataChunkSize = pcmFile.size();
    header.riffChunkSize = header.dataChunkSize + sizeof (WAVHeader)
                            - sizeof (header.riffChunkId) - sizeof (header.riffChunkSize);

    QFile wavFile(wav);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "打开文件失败" << wav;
        pcmFile.close();
        return;
    }

    wavFile.write((const char *)&header, sizeof (header));

    char buf[1024];
    int size = 0;
    while ((size = pcmFile.read(buf, sizeof (buf))) > 0) {
        wavFile.write(buf, size);
    }

    pcmFile.close();
    wavFile.close();
}
