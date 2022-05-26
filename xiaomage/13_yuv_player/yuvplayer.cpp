#include "yuvplayer.h"

#include <QDebug>
#include <QPainter>
#include <ffmpegutil.h>

extern "C" {
#include <libavutil/imgutils.h>
}

YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

YuvPlayer::~YuvPlayer() {
    stopTimer();
    closeFile();
    freeCurrentImage();
}

void YuvPlayer::play() {
    if (_state == Playing) return;

    _timerId = startTimer(_interval);

    setState(Playing);
}

void YuvPlayer::pause() {
    if (_state != Playing) return;

    stopTimer();
    setState(Paused);
}

void YuvPlayer::stop() {
    if (_state == Stopped) return;

    stopTimer();
    freeCurrentImage();
    update();
    setState(Stopped);
}

void YuvPlayer::setYuv(Yuv &yuv) {
    _yuv = yuv;
    closeFile();

    _file = new QFile(yuv.filename);
    if (!_file->open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << yuv.filename;
        return;
    }

    _interval = 1000 / yuv.fps;

    _imageSize = av_image_get_buffer_size(yuv.format, yuv.width, yuv.height, 1);

    int w = width();
    int h = height();

    int dx = 0;
    int dy = 0;
    int dw = yuv.width;
    int dh = yuv.height;

    if (dw > w || dh > h) {
        if (dw * h > w * dh) {
            dh = w * dh / dw;
            dw = w;
        } else {
            dw = h * dw / dh;
            dh = h;
        }
    }
    dx = (w - dw) >> 1;
    dy = (h - dh) >> 1;

    _dstRect = QRect(dx, dy, dw, dh);
}

void YuvPlayer::setState(State state) {
    if (_state == state) return;

    if (state == Stopped || state == Finished) {
        _file->seek(0);
    }
    _state = state;
    emit stateChanged();
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}

bool YuvPlayer::isPlaying() {
    return _state == Playing;
}

void YuvPlayer::stopTimer() {
    if (_timerId == 0) return;

    killTimer(_timerId);
    _timerId = 0;
}

void YuvPlayer::closeFile() {
    if (!_file) return;

    _file->close();
    delete _file;
    _file = nullptr;
}

void YuvPlayer::freeCurrentImage() {
    if (!_currentImage) return;

    free(_currentImage->bits());
    delete _currentImage;
    _currentImage = nullptr;
}

void YuvPlayer::timerEvent(QTimerEvent *event) {
    char data[_imageSize];
    if (_file->read(data, _imageSize) == _imageSize) {
        RawVideoFrame in = {
            data, _yuv.width, _yuv.height, _yuv.format
        };
        RawVideoFrame out = {
            nullptr, _yuv.width >> 4 << 4, _yuv.height >> 4 << 4, AV_PIX_FMT_RGB24
        };
        FFmpegUtil::convertVideo(in, out);
        freeCurrentImage();
        _currentImage = new QImage((uchar *) out.pixels, out.width, out.height, QImage::Format_RGB888);

        update();
    } else {
        stopTimer();
        setState(Finished);
    }
}

void YuvPlayer::paintEvent(QPaintEvent *event) {
    if (!_currentImage) return;

    QPainter(this).drawImage(_dstRect, *_currentImage);
}
