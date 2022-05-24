#include "playthread.h"

#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

#define FILENAME "/Users/jisudong/Documents/lena_256x256_yuv420p.yuv"
#define IMG_W 256
#define IMG_H 256

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "线程安全退出";
}

void PlayThread::run() {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    int ret = 0;

    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        qDebug() << "SDL_Init error:" << SDL_GetError();
        return;
    }

    window = SDL_CreateWindow("show yuv", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              IMG_W, IMG_H, SDL_WINDOW_SHOWN);
    if (!window) {
        qDebug() << "SDL_CreateWindow error:" << SDL_GetError();
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (!renderer) {
        qDebug() << "SDL_CreateRenderer error:" << SDL_GetError();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING, IMG_W, IMG_H);
    if (!texture) {
        qDebug() << "SDL_CreateTexture error:" << SDL_GetError();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << FILENAME;
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    ret = SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W);
    if (ret < 0) {
        qDebug() << "SDL_UpdateTexture error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    ret = SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    if (ret < 0) {
        qDebug() << "SDL_SetRenderDrawColor error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    ret = SDL_RenderClear(renderer);
    if (ret < 0) {
        qDebug() << "SDL_RenderClear error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    if (ret < 0) {
        qDebug() << "SDL_RenderCopy error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_RenderPresent(renderer);

    while (!isInterruptionRequested()) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
    }

    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
