#ifndef ZVIDEOTXTHREAD_H
#define ZVIDEOTXTHREAD_H

#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#include <zringbuffer.h>
class ZVideoTxThreadSoft264: public QThread
{
    Q_OBJECT
public:
    ZVideoTxThreadSoft264(qint32 nTcpPort);
    qint32 ZBindQueue(ZRingBuffer *rbYUV);
    qint32 ZStartThread();
    qint32 ZStopThread();
    bool ZIsExitCleanup();
signals:
    void ZSigThreadFinished();

protected:
    void run();
private:
    ZRingBuffer *m_rbYUV;
    bool m_bExitFlag;
    bool m_bCleanup;
    qint32 m_nTcpPort;
};

#endif // ZVIDEOTXTHREAD_H
