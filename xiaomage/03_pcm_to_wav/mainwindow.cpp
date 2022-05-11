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


void MainWindow::on_audioButton_clicked()
{
    if (_audioThread) {
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->audioButton->setText("开始录音");
    } else {
        _audioThread = new AudioThread(this);
        _audioThread->start();
        connect(_audioThread, &AudioThread::finished, [this]() {
            _audioThread = nullptr;
            ui->audioButton->setText("开始录音");
        });

        ui->audioButton->setText("结束录音");
    }
}

