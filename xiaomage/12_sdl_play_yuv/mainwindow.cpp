#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>


#define FILENAME "/Users/jisudong/Documents/Titanic.yuv"
#define WIDTH 640
#define HEIGHT 272

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _widget = new QWidget(this);
    _widget->setGeometry(100, 100, WIDTH, HEIGHT);

    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        qDebug() << "SDL_Init error:" << SDL_GetError();
        return;
    }

    _window = SDL_CreateWindowFrom((void *) _widget->winId());
    if (!_window) {
        qDebug() << "SDL_CreateWindowFrom error:" << SDL_GetError();
        SDL_Quit();
        return;
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
    }
    if (!_renderer) {
        qDebug() << "SDL_CreateRenderer error:" << SDL_GetError();
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return;
    }

    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                 WIDTH, HEIGHT);
    if (!_texture) {
        qDebug() << "SDL_CreateTexture error:" << SDL_GetError();
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return;
    }

    _file.setFileName(FILENAME);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << SDL_GetError();
        SDL_DestroyTexture(_texture);
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_playButton_clicked()
{
    _timerId = startTimer(41);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    int imagesize = WIDTH * HEIGHT * 1.5;
    char data[imagesize];
    int ret = 0;
    if (_file.read(data, imagesize) > 0) {
        ret = SDL_UpdateTexture(_texture, nullptr, data, WIDTH);
        if (ret < 0) {
            qDebug() << "SDL_UpdateTexture error:" << SDL_GetError();
            SDL_DestroyTexture(_texture);
            SDL_DestroyRenderer(_renderer);
            SDL_DestroyWindow(_window);
            SDL_Quit();
            return;
        }

        ret = SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        if (ret < 0) {
            qDebug() << "SDL_SetRenderDrawColor error:" << SDL_GetError();
            SDL_DestroyTexture(_texture);
            SDL_DestroyRenderer(_renderer);
            SDL_DestroyWindow(_window);
            SDL_Quit();
            return;
        }

        ret = SDL_RenderClear(_renderer);
        if (ret < 0) {
            qDebug() << "SDL_RenderClear error:" << SDL_GetError();
            SDL_DestroyTexture(_texture);
            SDL_DestroyRenderer(_renderer);
            SDL_DestroyWindow(_window);
            SDL_Quit();
            return;
        }

        SDL_Rect rect = {100, 100, WIDTH, HEIGHT};
        ret = SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
        if (ret < 0) {
            qDebug() << "SDL_RenderCopy error:" << SDL_GetError();
            SDL_DestroyTexture(_texture);
            SDL_DestroyRenderer(_renderer);
            SDL_DestroyWindow(_window);
            SDL_Quit();
            return;
        }
        SDL_RenderPresent(_renderer);
    } else {
        killTimer(_timerId);
    }
}

