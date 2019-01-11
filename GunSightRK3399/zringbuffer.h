#ifndef ZRINGBUFFERBYTEARRAY_H
#define ZRINGBUFFERBYTEARRAY_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QSemaphore>
#include <QMutex>
#include <QWaitCondition>
class ZRingBufferElement
{
public:
    qint8 *m_pData;
    qint32 m_nLen;
};
class ZRingBuffer:public QObject
{
    Q_OBJECT
public:
    ZRingBuffer(qint32 nMaxElement,qint32 nEachBytes);
    ~ZRingBuffer();
    bool ZIsFull();
    bool ZIsEmpty();

    qint32 ZPutElement(const qint8 *buffer,qint32 len);
    qint32 ZTryPutElement(const qint8 *buffer,qint32 len);

    qint32 ZGetElement(qint8 *buffer,qint32 len);
    qint32 ZTryGetElement(qint8 *buffer,qint32 len);

    qint32 ZGetValidNum();

private:
    QVector<ZRingBufferElement*> *m_vec;
    qint32 m_nMaxElement;
    qint32 nEachBytes;
    qint32 m_nValidElement;
private:
    //read & write pos.
    qint32 m_nRdPos;
    qint32 m_nWrPos;
private:
    QMutex m_mutex;
    QWaitCondition m_wcRBNotFull;
    QWaitCondition m_wcRBNotEmpty;
};

#endif // ZRINGBUFFERBYTEARRAY_H
