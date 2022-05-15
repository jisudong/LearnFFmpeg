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

};

#endif // RECORDTHREAD_H
