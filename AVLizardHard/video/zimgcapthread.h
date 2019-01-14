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

    qint32 ZBindProcessQueue(ZRingBuffer *rbProcess);
    qint32 ZBindYUVQueue(ZRingBuffer *rbYUV);

    qint32 ZStartThread();
    qint32 ZStopThread();

    qint32 ZGetCAMImgFps();

    QString ZGetDevName();

    bool ZIsExitCleanup();
signals:
    void ZSigNewImgArrived(QImage img);
    void ZSigMsg(const QString &msg,const qint32 &type);
    void ZSigThreadFinished();
    void ZSigCAMIDFind(QString camID);
protected:
    void run();
private:
    QString m_devUsbId;
    qint32 m_nPreWidth,m_nPreHeight,m_nPreFps;
    ZCAMDevice *m_cam;
    CAM_ID_TYPE_E m_nCamIdType;
private:
    //capture image to process queue.
    ZRingBuffer *m_rbProcess;
    //capture yuv to yuv queue.
    ZRingBuffer *m_rbYUV;
private:
    bool m_bExitFlag;
    bool m_bCleanup;
};
#endif // ZIMGCAPTHREAD_H
