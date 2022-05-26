#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _yuvPlayer = new YuvPlayer(this);
    connect(_yuvPlayer, &YuvPlayer::stateChanged, this, &MainWindow::on_state_changed);

    int w = 500;
    int h = 500;
    int x = (width() - w) >> 1;
    int y = (height() - h) >> 1;
    _yuvPlayer->setGeometry(x, y, w, h);

    Yuv yuv = {
        "/Users/jisudong/Documents/Titanic.yuv",
        640, 272, AV_PIX_FMT_YUV420P, 24
    };
    _yuvPlayer->setYuv(yuv);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_playButton_clicked()
{
    if (_yuvPlayer->isPlaying()) {
        _yuvPlayer->pause();
    } else {
        _yuvPlayer->play();
    }
}


void MainWindow::on_stopButton_clicked()
{
    _yuvPlayer->stop();
    ui->playButton->setText("播放");
}


void MainWindow::on_nextButton_clicked()
{

}

void MainWindow::on_state_changed() {
    if (_yuvPlayer->getState() == YuvPlayer::Playing) {
        ui->playButton->setText("暂停");
    } else {
        ui->playButton->setText("播放");
    }
}

