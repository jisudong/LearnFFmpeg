#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>
#include <QFile>

extern "C" {
#include <libavutil/avutil.h>
}

typedef struct {
    const char *filename;
    int width;
    int height;
    AVPixelFormat format;
    int fps;
} Yuv;

class YuvPlayer : public QWidget
{
    Q_OBJECT
public:
    typedef enum {
        Stopped = 0,
        Playing,
        Paused,
        Finished
    } State;

    explicit YuvPlayer(QWidget *parent = nullptr);
    ~YuvPlayer();

    void stop();
    void play();
    void pause();

    void setYuv(Yuv &yuv);
    State getState();
    bool isPlaying();

signals:
    void stateChanged();

private:
    Yuv _yuv;
    int _timerId = 0;
    int _interval = 0;
    State _state = Stopped;
    int _imageSize = 0;
    QRect _dstRect;
    QFile *_file = nullptr;
    QImage *_currentImage = nullptr;

    void setState(State state);
    void stopTimer();
    void closeFile();
    void freeCurrentImage();

    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);
};

#endif // YUVPLAYER_H
