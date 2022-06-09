#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videoplayer.h"
#include "videoslider.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_muteButton_clicked();

    void on_openFileButton_clicked();

    void on_timeSlider_valueChanged(int value);

    void on_volumeSlider_valueChanged(int value);

    void on_player_stateChanged(VideoPlayer::State state);

    void on_player_timeChanged(VideoPlayer *player);

    void on_player_initFinished(VideoPlayer *player);

    void on_player_playFailed(VideoPlayer *player);

    void on_slider_clicked(VideoSlider *slider);


private:
    Ui::MainWindow *ui;
    VideoPlayer *_player = nullptr;

    QString getTimeText(int value);
};
#endif // MAINWINDOW_H
