#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <SDL2/SDL.h>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();

private:
    Ui::MainWindow *ui;
    QWidget *_widget;
    int _timerId = 0;
    QFile _file;

    SDL_Window *_window = nullptr;
    SDL_Renderer *_renderer = nullptr;
    SDL_Texture *_texture = nullptr;

    void timerEvent(QTimerEvent *event);
};
#endif // MAINWINDOW_H
