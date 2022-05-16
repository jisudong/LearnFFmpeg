#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#include <QThread>

class RecordThread : public QThread
{
    Q_OBJECT

private:
    void run();

public:
    explicit RecordThread(QObject *parent = nullptr);
    ~RecordThread();
signals:
    void timeChanged(unsigned long long ms);
};

#endif // RECORDTHREAD_H
