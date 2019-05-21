#ifndef ZIMGCAPTHREAD_H
#define ZIMGCAPTHREAD_H

#include <QThread>
#include <QtGui/QImage>
#include "zgblpara.h"
#include <QTimer>
#include <QQueue>
#include <QByteArray>
#include <QSemaphore>
#include <QWaitCondition>
#define BYPASS_FIRST_BLACK_FRAMES   100
typedef enum {
    CAM1_ID_Main,
    CAM2_ID_Aux,
    CAM3_ID_MainEx,
}CAM_ID_TYPE_E;
typedef struct{
    void *pStart;
    size_t nLength;
}IMGBufferStruct;
class ZImgCapThread : public QThread
{
    Q_OBJECT
public:
    explicit ZImgCapThread(QString devFile,CAM_ID_TYPE_E camIdType);
    ~ZImgCapThread();

    //capture -> processing.
    void ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);

    //capture -> h264 encoder.
    void ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);

    qint32 ZStartThread();
    qint32 ZStopThread();

    QString ZGetDevName();

    bool ZIsExitCleanup();
signals:
    void ZSigNewImgArrived(QImage img);
    void ZSigMsg(const QString &msg,const qint32 &type);
    void ZSigFinished();
    void ZSigCAMIDFind(QString camID);
protected:
    void run();
private:
    void doCleanBeforeExit();
private:
    QString m_devFile;
    CAM_ID_TYPE_E m_nCamIdType;
private:
    bool m_bCleanup;
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
};
#endif // ZIMGCAPTHREAD_H
