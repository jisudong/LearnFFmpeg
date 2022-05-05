#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>

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
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;

    int ret = avformat_open_input(&ctx, DEVICE_NAME,fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf,sizeof (errbuf));
        qDebug() << "打开上下文失败" << errbuf;
        return;
    }

    QFile file(FILE_NAME);

    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "打开文件失败" << FILE_NAME;

        avformat_close_input(&ctx);
        return;
    }

    int count = 50;

    AVPacket pkt;

    while (count-- > 0 && av_read_frame(ctx, &pkt) == 0) {
        file.write((const char *)pkt.data, pkt.size);
    }

    file.close();

    avformat_close_input(&ctx);



}

