#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include "audiothread.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
}

#ifdef Q_OS_MAC
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":1"
    #define FILE_NAME "/Users/jisudong/Documents/out.pcm"
#else

#endif

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
    if (!_audioThread) {
        _audioThread = new Audiothread(this);
        _audioThread->start();
        connect(_audioThread, &Audiothread::finished, [this]() {
            _audioThread = nullptr;
            ui->audioButton->setText("开始录音");
        });

        ui->audioButton->setText("结束录音");
    } else {
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->audioButton->setText("开始录音");
    }
}

