#include "mainwindow.h"

#include <QApplication>
//#include "ffmpegutil.h"

int main(int argc, char *argv[])
{
//    RawVideoFile in = {
//        "/Users/jisudong/Documents/Titanic.yuv",
//        640,
//        272,
//        AV_PIX_FMT_YUV420P
//    };
//    RawVideoFile out = {
//        "/Users/jisudong/Documents/Titanic_out.yuv",
//        500,
//        400,
//        AV_PIX_FMT_YUV444P
//    };
//    FFmpegUtil::convertVideo(in, out);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
