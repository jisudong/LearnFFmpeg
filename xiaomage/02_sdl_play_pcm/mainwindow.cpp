#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_playButton_clicked()
{
    if (_playThread) {
        _playThread->requestInterruption();
        _playThread = nullptr;
        ui->playButton->setText("开始播放");
    } else {
        _playThread = new PlayThread(this);
        _playThread->start();
        connect(_playThread, &PlayThread::finished, [this]() {
            _playThread = nullptr;
            ui->playButton->setText("开始播放");
        });

        ui->playButton->setText("停止播放");
    }
}

