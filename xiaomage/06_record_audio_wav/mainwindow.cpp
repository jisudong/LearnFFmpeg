#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    on_time_changed(0);
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
        connect(_recordThread, &RecordThread::timeChanged, this, &MainWindow::on_time_changed);
        connect(_recordThread, &RecordThread::finished, [this]() {
            _recordThread = nullptr;
            ui->recordButton->setText("开始录音");
        });
        ui->recordButton->setText("停止录音");
    }
}

void MainWindow::on_time_changed(unsigned long long ms) {
    QTime time(0, 0, 0, 0);
    QString text = time.addMSecs(ms).toString("mm:ss.z");
    ui->timeLable->setText(text.left(7));
}

