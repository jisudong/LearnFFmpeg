#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>

class DecodeThread : public QThread
{
    Q_OBJECT
private:
    void run();
public:
    explicit DecodeThread(QObject *parent = nullptr);
    ~DecodeThread();
signals:

};

#endif // DECODETHREAD_H
