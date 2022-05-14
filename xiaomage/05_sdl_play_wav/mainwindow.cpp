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
    if (_audioThread) {
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->playButton->setText("播放");
    } else {
        _audioThread = new AudioThread(this);
        _audioThread->start();
        connect(_audioThread, &AudioThread::finished, [this]() {
            _audioThread = nullptr;
            ui->playButton->setText("播放");
        });
        ui->playButton->setText("停止");
    }
}

