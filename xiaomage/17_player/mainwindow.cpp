#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QString>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec&");

    _player = new VideoPlayer();
    connect(_player, &VideoPlayer::stateChanged, this, &MainWindow::on_player_stateChanged);
    connect(_player, &VideoPlayer::initFinished, this, &MainWindow::on_player_initFinished);
    connect(_player, &VideoPlayer::playFailed, this, &MainWindow::on_player_playFailed);
    connect(_player, &VideoPlayer::timeChanged, this, &MainWindow::on_player_timeChanged);

    connect(_player, &VideoPlayer::videoDecoded,
                ui->videoWidget, &VideoWidget::on_player_videoDecoded);
    connect(_player, &VideoPlayer::stateChanged,
                ui->videoWidget, &VideoWidget::on_player_stateChanged);

    connect(ui->timeSlider, &VideoSlider::clicked, this, &MainWindow::on_slider_clicked);

    ui->volumeSlider->setRange(VideoPlayer::Volume::Min, VideoPlayer::Volume::Max);
    ui->volumeSlider->setValue(ui->volumeSlider->maximum() >> 1);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _player;
}


void MainWindow::on_playButton_clicked()
{
    VideoPlayer::State state = _player->getState();
    if (state == VideoPlayer::Playing) {
        _player->pause();
    } else {
        _player->play();
    }
}


void MainWindow::on_stopButton_clicked()
{
    _player->stop();
}


void MainWindow::on_muteButton_clicked()
{
    if (_player->isMute()) {
        _player->setMute(false);
        ui->muteButton->setText("静音");
    } else {
        _player->setMute(true);
        ui->muteButton->setText("开音");
    }
}


void MainWindow::on_openFileButton_clicked()
{
    qDebug() << "on_openFileButton_clicked";
    QString filename = QFileDialog().getOpenFileName(nullptr,
                                                     "/",
                                                     "选择文件",
                                                     "视频文件 (*.mp4 *.mkv *.avi *.mp3 *.aac)");
    if (filename.isEmpty()) return;

    _player->setFilename(filename);
    _player->play();
}


void MainWindow::on_timeSlider_valueChanged(int value)
{
    ui->timeLabel->setText(getTimeText(value));
}


void MainWindow::on_volumeSlider_valueChanged(int value)
{
    ui->volumeLabel->setText(QString("%1").arg(value));
    _player->setVolume(value);
}

void MainWindow::on_player_stateChanged(VideoPlayer::State state) {
    if (state == VideoPlayer::Playing) {
        ui->playButton->setText("暂停");
    } else {
        ui->playButton->setText("播放");
    }

    if (state == VideoPlayer::Stopped) {

        ui->playButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->muteButton->setEnabled(false);
        ui->timeSlider->setEnabled(false);
        ui->volumeSlider->setEnabled(false);

        ui->durationLabel->setText(getTimeText(0));
        ui->timeSlider->setValue(0);
        ui->playWidget->setCurrentWidget(ui->openFilePage);

    } else {

        ui->playButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        ui->muteButton->setEnabled(true);
        ui->timeSlider->setEnabled(true);
        ui->volumeSlider->setEnabled(true);

        ui->playWidget->setCurrentWidget(ui->videoPage);
    }
}

void MainWindow::on_player_timeChanged(VideoPlayer *player) {
    ui->timeSlider->setValue(player->getTime());
}


void MainWindow::on_player_initFinished(VideoPlayer *player) {

    int value = player->getDuration();

    ui->timeSlider->setRange(0, value);
    ui->durationLabel->setText(getTimeText(value));

}

void MainWindow::on_player_playFailed(VideoPlayer *player) {
    QMessageBox::critical(nullptr, "提示", "播放失败！");
}

void MainWindow::on_slider_clicked(VideoSlider *slider) {
    _player->setTime(slider->value());
}

QString MainWindow::getTimeText(int value) {

//    QString h = QString("0%1").arg(value / 3600).right(2);
//    QString m = QString("0%1").arg((value % 3600) / 60).right(2);
//    QString s = QString("0%1").arg(value % 60).right(2);
//    return QString("%1:%2:%3").arg(h).arg(m).arg(s);

    return QString("%1:%2:%3")
            .arg(value / 3600, 2, 10, QLatin1Char('0'))
            .arg((value % 3600) / 60, 2, 10, QLatin1Char('0'))
            .arg(value % 60, 2, 10, QLatin1Char('0'));
}

