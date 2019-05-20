#ifndef ZVIDEOTASK_H
#define ZVIDEOTASK_H

#include <QObject>
#include <QPaintEvent>
#include <QImage>
#include <QtWidgets/QWidget>
#include <QLabel>
#include <QQueue>
#include <QByteArray>
#include <QSemaphore>
#include <QProgressBar>

#include <video/zimgcapthread.h>
#include <video/zimgprocessthread.h>
#include <video/zimgdisplayer.h>
#include <video/zh264encthread.h>
#include <video/zvideotxthread.h>
#include <video/zvideotxthread_h264.h>
#include <video/zvideotxthread_h2642.h>
#include "zgblpara.h"

class ZVideoTask : public QObject
{
    Q_OBJECT
public:
    explicit ZVideoTask(QObject *parent = nullptr);
    ~ZVideoTask();
    qint32 ZDoInit();
    qint32 ZStartTask();

    bool ZIsExitCleanup();

    ZImgCapThread* ZGetImgCapThread(qint32 index);
    ZImgProcessThread* ZGetImgProcessThread();
signals:
    void ZSigVideoTaskExited();
private slots:
    void ZSlotSubThreadsExited();
    void ZSlotChkAllExitFlags();
private:
    ZImgCapThread *m_capThread[3];//主/辅摄像头Main/Aux图像采集线程+MainEx.
    ZImgProcessThread *m_process;//图像处理线程.
    ZVideoTxThreadHard264 *m_encTxThread;//h264 soft encode图像编码&TCP传输线程.
    ZVideoTxThreadHard2642 *m_encTx2Thread;//h264 hard encode.

    //Main/Aux CaptureThread put QImage to this queue.
    //ImgProcessThread get QImage from this queue.
    ZRingBuffer *m_rbProcess[2];

    //Main CaptureThread put yuv to this queue.
    //H264 EncodeThread get yuv from this queue.
    ZRingBuffer *m_rbYUV[2];

    //H264EncThread put h264 frame to this queue.
    //TcpTxThread get data from this queue.
    ZRingBuffer *m_rbH264[2];


    //main capture to h264 encoder queue(fifo).
    QByteArray* m_Cap2EncFIFOMain[5];
    QQueue<QByteArray*> m_Cap2EncFIFOFreeMain;
    QQueue<QByteArray*> m_Cap2EncFIFOUsedMain;
    QMutex m_Cap2EncFIFOMutexMain;
    QWaitCondition m_condCap2EncFIFOFullMain;
    QWaitCondition m_condCap2EncFIFOEmptyMain;

    //aux capture to h264 encoder queue(fifo).
    QByteArray* m_Cap2EncFIFOAux[5];
    QQueue<QByteArray*> m_Cap2EncFIFOFreeAux;
    QQueue<QByteArray*> m_Cap2EncFIFOUsedAux;
    QMutex m_Cap2EncFIFOMutexAux;
    QWaitCondition m_condCap2EncFIFOFullAux;
    QWaitCondition m_condCap2EncFIFOEmptyAux;

    //main capture to ImgProcess queue(fifo).
    QByteArray* m_Cap2ProFIFOMain[5];
    QQueue<QByteArray*> m_Cap2ProFIFOFreeMain;
    QQueue<QByteArray*> m_Cap2ProFIFOUsedMain;
    QMutex m_Cap2ProFIFOMutexMain;
    QWaitCondition m_condCap2ProFIFOFullMain;
    QWaitCondition m_condCap2ProFIFOEmptyMain;
    //aux capture to ImgProcess queue(fifo).
    QByteArray* m_Cap2ProFIFOAux[5];
    QQueue<QByteArray*> m_Cap2ProFIFOFreeAux;
    QQueue<QByteArray*> m_Cap2ProFIFOUsedAux;
    QMutex m_Cap2ProFIFOMutexAux;
    QWaitCondition m_condCap2ProFIFOFullAux;
    QWaitCondition m_condCap2ProFIFOEmptyAux;
private:
    QTimer *m_timerExit;
};

#endif // ZVIDEOTASK_H
