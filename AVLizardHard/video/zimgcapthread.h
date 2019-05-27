#ifndef ZIMGCAPTHREAD_H
#define ZIMGCAPTHREAD_H

#include <QThread>
#include <QtGui/QImage>
#include "zcamdevice.h"
#include "zgblpara.h"
#include <QTimer>
#include <QQueue>
#include <QByteArray>
#include <QSemaphore>
#include <zringbuffer.h>
#define BYPASS_FIRST_BLACK_FRAMES   100
typedef enum {
    CAM1_ID_Main,
    CAM2_ID_Aux,
    CAM3_ID_MainEx,
}CAM_ID_TYPE_E;
class ZImgCapThread : public QThread
{
    Q_OBJECT
public:
    explicit ZImgCapThread(QString devUsbId,qint32 nPreWidth,qint32 nPreHeight,qint32 nPreFps,CAM_ID_TYPE_E camIdType);
    ~ZImgCapThread();

    //capture -> processing.
    void ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);

    //capture -> h264 encoder.
    void ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);

    qint32 ZStartThread();
    qint32 ZStopThread();

    qint32 ZGetCAMImgFps();

    QString ZGetDevName();

    bool ZIsExitCleanup();
private:
    void ZDoCleanBeforeExit();
signals:
    void ZSigNewImgArrived(QImage img);
    void ZSigMsg(const QString &msg,const qint32 &type);
    void ZSigFinished();
    void ZSigCAMIDFind(QString camID);
protected:
    void run();
private:
    QString m_devUsbId;
    qint32 m_nPreWidth,m_nPreHeight,m_nPreFps;
    ZCAMDevice *m_cam;
    CAM_ID_TYPE_E m_nCamIdType;
private:
    //out1 fifo.(capture -> processing).
    QQueue<QByteArray*> *m_freeQueueOut1;
    QQueue<QByteArray*> *m_usedQueueOut1;

    QMutex *m_mutexOut1;
    QWaitCondition *m_condQueueEmptyOut1;
    QWaitCondition *m_condQueueFullOut1;

    //out2 fifo.(capture -> h264 encoder).
    QQueue<QByteArray*> *m_freeQueueOut2;
    QQueue<QByteArray*> *m_usedQueueOut2;

    QMutex *m_mutexOut2;
    QWaitCondition *m_condQueueEmptyOut2;
    QWaitCondition *m_condQueueFullOut2;
private:
    bool m_bCleanup;
};
#endif // ZIMGCAPTHREAD_H
