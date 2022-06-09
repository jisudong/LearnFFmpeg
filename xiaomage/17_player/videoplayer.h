#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <list>
#include "condmutex.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        Stopped,
        Playing,
        Paused
    } State;

    typedef enum {
        Min = 0,
        Max = 100
    } Volume;

    typedef struct {
        int width;
        int height;
        AVPixelFormat pixFmt;
        int size;
    } VideoSwsSpec;

    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    void play();
    void pause();
    void stop();
    bool isPlaying();
    State getState();
    void setFilename(QString &filename);
    int getDuration();
    int getTime();
    void setTime(int seekTime);
    void setVolume(int volume);
    int getVolume();
    void setMute(bool mute);
    bool isMute();

signals:
    void stateChanged(State state);
    void timeChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);
    void videoDecoded(VideoPlayer *player, uint8_t *data, VideoSwsSpec &spec);

private:
    typedef struct {
        int sampleRate;
        AVSampleFormat sampleFmt;
        int chLayout;
        int channels;
        int bytesPerSampleFrame;
    } AudioSwrSpec;

    State _state = Stopped;
    AVFormatContext *_fmtCtx = nullptr;
    bool _fmttCtxCanFree = false;
    int _volume = Max;
    bool _mute = false;
    char _filename[512];
    int _seekTime = -1;

    AVCodecContext *_aCodecCtx = nullptr;
    AVStream *_aStream = nullptr;
    std::list<AVPacket> _aPktList;
    CondMutex _aMutex;
    SwrContext *_aSwrCtx = nullptr;
    AudioSwrSpec _aSwrInSpec, _aSwrOutSpec;
    AVFrame *_aSwrInFrame = nullptr;
    AVFrame *_aSwrOutFrame = nullptr;
    int _aSwrOutIdx = 0;
    int _aSwrOutSize = 0;
    double _aTime = 0;
    bool _aCanFree = false;
    int _aSeekTime = -1;
    bool _hasAudio = false;

    AVCodecContext *_vCodecCtx = nullptr;
    AVStream *_vStream = nullptr;
    AVFrame *_vSwsInFrame = nullptr;
    AVFrame *_vSwsOutFrame = nullptr;
    SwsContext *_vSwsCtx = nullptr;
    VideoSwsSpec _vSwsOutSpec;
    std::list<AVPacket> _vPktList;
    CondMutex _vMutex;
    double _vTime = 0;
    bool _vCanfree = false;
    int _vSeekTime = -1;
    bool _hasVideo = false;


    int initAudio();
    int initSwr();
    int initSDL();
    void addAudioPkt(AVPacket &pkt);
    void clearAudioPktList();
    static void sdlAudioCallbackFunc(void *userdata, Uint8 *stream, int len);
    void sdlAudioCallback(Uint8 *stream, int len);
    int decodeAudio();

    int initVideo();
    int initSws();
    void addVideoPkt(AVPacket &pkt);
    void clearVideoPktList();
    void decodeVideo();

    void setState(State state);
    void readFile();
    int initDecoder(AVCodecContext **ctx, AVStream **stream, AVMediaType type);
    void free();
    void freeAudio();
    void freeVideo();
    void fatalError();
};

#endif // VIDEOPLAYER_H
