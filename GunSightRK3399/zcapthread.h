#ifndef ZCAPTHREAD_H
#define ZCAPTHREAD_H

#include <QThread>
#include <vector>
#include <iostream>
#include <QObject>
#include <QImage>
#include "zringbuffer.h"
typedef struct{
    void *pStart;
    size_t nLength;
}IMGBufferStruct;
using namespace std;

#define V4L2_REQBUF_NUM  4

class ZCapThread:public QThread
{
    Q_OBJECT
public:
    ZCapThread(QString devName,quint32 width,quint32 height,quint32 fps);
    void ZBindRingBuffer(ZRingBuffer *rb);
protected:
    void run();
signals:
    void ZSigNewImgArrived(QImage img);
private:
    QString m_devName;
    quint32 m_nWidth,m_nHeight,m_nFps;
private:
    ZRingBuffer *m_rb;
};

#endif // ZCAPTHREAD_H
