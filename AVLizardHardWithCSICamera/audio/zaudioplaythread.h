#ifndef ZAUDIOPLAYTHREAD_H
#define ZAUDIOPLAYTHREAD_H

#include <QThread>
#include <QTimer>
#include <QQueue>
#include <QSemaphore>
#include <QWaitCondition>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
class ZAudioPlayThread : public QThread
{
    Q_OBJECT
public:
    ZAudioPlayThread(QString playCardName);
    ~ZAudioPlayThread();
    void ZBindFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);
    qint32 ZStartThread();
    qint32 ZStopThread();
    bool ZIsExitCleanup();
protected:
    void run();
signals:
    void ZSigThreadFinished();
private:
    quint64 ZGetTimestamp();
private:
    bool m_bCleanup;
    QTimer *m_timerPlay;
private:
    QString m_playCardName;
    snd_pcm_t *m_pcmHandle;
private:
    QQueue<QByteArray*> *m_freeQueue;
    QQueue<QByteArray*> *m_usedQueue;

    QMutex *m_mutex;
    QWaitCondition *m_condQueueEmpty;
    QWaitCondition *m_condQueueFull;
};

#endif // ZAUDIOPLAYTHREAD_H
