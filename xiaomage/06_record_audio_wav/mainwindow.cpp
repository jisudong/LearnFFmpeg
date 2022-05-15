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


void MainWindow::on_recordButton_clicked()
{
    if (_recordThread) {
        _recordThread->requestInterruption();
        _recordThread = nullptr;
        ui->recordButton->setText("开始录音");
    } else {
        _recordThread = new RecordThread(this);
        _recordThread->start();
        connect(_recordThread, &RecordThread::finished, [this]() {
            _recordThread = nullptr;
            ui->recordButton->setText("开始录音");
        });
        ui->recordButton->setText("停止录音");
    }
}

