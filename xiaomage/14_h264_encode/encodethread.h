#ifndef ENCODETHREAD_H
#define ENCODETHREAD_H

#include <QThread>

class EncodeThread : public QThread
{
    Q_OBJECT
private:
    void run();

public:
    explicit EncodeThread(QObject *parent = nullptr);
    ~EncodeThread();

signals:

};

#endif // ENCODETHREAD_H
