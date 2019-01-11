#ifndef ZTRACKTHREAD_H
#define ZTRACKTHREAD_H

#include <QThread>
#include "zringbuffer.h"
class ZTrackThread : public QThread
{
    Q_OBJECT
public:
    ZTrackThread();
    void ZBindRingBuffer(ZRingBuffer *rb);
protected:
    void run();
signals:
    void ZSigNewTracking();
    void ZSigNewInitBox(const QImage &img);
private:
    ZRingBuffer *m_rb;
    bool m_bInitBox;
};

#endif // ZTRACKTHREAD_H
