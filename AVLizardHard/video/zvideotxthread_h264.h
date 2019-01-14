#ifndef ZVIDEOTXTHREADH264_H
#define ZVIDEOTXTHREADH264_H

#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#include <zringbuffer.h>
class ZVideoTxThreadHard264 : public QThread
{
    Q_OBJECT
public:
    ZVideoTxThreadHard264(qint32 nTcpPort,qint32 nTcpPort2);
    qint32 ZBindQueue(ZRingBuffer *rbYUVMain,ZRingBuffer *rbYUVAux);
    qint32 ZStartThread();
    qint32 ZStopThread();
    bool ZIsExitCleanup();
signals:
    void ZSigThreadFinished();

protected:
    void run();
private:
    ZRingBuffer *m_rbYUVMain;
    ZRingBuffer *m_rbYUVAux;
    bool m_bExitFlag;
    bool m_bCleanup;
    qint32 m_nTcpPort;
    qint32 m_nTcpPort2;
};

#endif // ZVIDEOTXTHREADH264_H
