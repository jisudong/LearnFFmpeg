#include "playthread.h"

#include <QDebug>
#include <SDL2/SDL.h>

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
    SDL_Surface *surface = nullptr;
    SDL_Texture *texture = nullptr;

    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        qDebug() << "SDL_Init error:" << SDL_GetError();
        return;
    }

    surface = SDL_LoadBMP("/Users/jisudong/Documents/in.bmp");
    if (!surface) {
        qDebug() << "SDL_LoadBMP" << SDL_GetError();
        SDL_Quit();
        return;
    }

    window = SDL_CreateWindow("显示bmp图片", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              surface->w, surface->h, SDL_WINDOW_SHOWN);
    if (!window) {
        qDebug() << "SDL_CreateWindow error:" << SDL_GetError();
        SDL_FreeSurface(surface);
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
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        qDebug() << "SDL_CreateTextureFromSurface error:" << SDL_GetError();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }

    ret = SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    if (ret < 0) {
        qDebug() << "SDL_SetRenderDrawColor error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }

    ret = SDL_RenderClear(renderer);
    if (ret < 0) {
        qDebug() << "SDL_SetRenderDrawColor error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }

    ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    if (ret < 0) {
        qDebug() << "SDL_SetRenderDrawColor error:" << SDL_GetError();
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(2000);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_FreeSurface(surface);
    SDL_Quit();
}
