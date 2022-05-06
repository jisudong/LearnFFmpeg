#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

class Audiothread : public QThread
{
    Q_OBJECT
private:
    void run();

public:
    explicit Audiothread(QObject *parent = nullptr);
    ~Audiothread();


signals:

};

#endif // AUDIOTHREAD_H
