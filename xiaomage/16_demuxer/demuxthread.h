#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include <QThread>

class DemuxThread : public QThread
{
    Q_OBJECT
private:
    void run();

public:
    explicit DemuxThread(QObject *parent = nullptr);
    ~DemuxThread();

signals:

};

#endif // DEMUXTHREAD_H
