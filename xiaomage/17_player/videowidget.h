#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include "videoplayer.h"

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

public slots:
    void on_player_videoDecoded(VideoPlayer *player, uint8_t *data, VideoPlayer::VideoSwsSpec &spec);
    void on_player_stateChanged(VideoPlayer::State state);

private:
    QImage *_image = nullptr;
    QRect _rect;
    void paintEvent(QPaintEvent *event) override;
    void freeImage();

signals:

};

#endif // VIDEOWIDGET_H
