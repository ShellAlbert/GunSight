#ifndef ZVIDEOTXTHREADH264_H
#define ZVIDEOTXTHREADH264_H

#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWaitCondition>
class ZHardEncTxThread : public QThread
{
    Q_OBJECT
public:
    ZHardEncTxThread(qint32 nTcpPort);
    //main capture -> encTxThread.
    void ZBindInFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);
    qint32 ZStartThread();
    qint32 ZStopThread();
    bool ZIsExitCleanup();
private:
    void ZDoCleanBeforeExit();
signals:
    void ZSigFinished();

protected:
    void run();
private:
    bool m_bCleanup;
    qint32 m_nTcpPort;
private:
    //in fifo.(main capture -> encTxThread).
    QQueue<QByteArray*> *m_freeQueueIn;
    QQueue<QByteArray*> *m_usedQueueIn;
    QMutex *m_mutexIn;
    QWaitCondition *m_condQueueEmptyIn;
    QWaitCondition *m_condQueueFullIn;
};

#endif // ZVIDEOTXTHREADH264_H
