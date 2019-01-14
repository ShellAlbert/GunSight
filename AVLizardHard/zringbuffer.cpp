#include "zringbuffer.h"
#include <QDebug>
ZRingBuffer::ZRingBuffer(qint32 nMaxElement,qint32 nEachBytes)
{
    this->m_vec=new QVector<ZRingBufferElement*>;
    for(qint32 i=0;i<nMaxElement;i++)
    {
        ZRingBufferElement *element=new ZRingBufferElement;
        element->m_pData=new qint8[nEachBytes];
        element->m_nLen=0;
        this->m_vec->append(element);
    }
    this->m_nMaxElement=nMaxElement;
    this->m_nValidElement=0;
    this->m_nRdPos=0;
    this->m_nWrPos=0;
}
ZRingBuffer::~ZRingBuffer()
{
    while(!this->m_vec->isEmpty())
    {
        ZRingBufferElement *element=this->m_vec->takeFirst();
        delete [] element->m_pData;
        delete element;
    }
    delete this->m_vec;
}
//写入时判断队列是否为满.
bool ZRingBuffer::ZIsFull()
{
    if(this->m_nValidElement==this->m_nMaxElement)
    {
        return true;
    }else{
        return false;
    }
}
//读取时判断队列是否为空.
bool ZRingBuffer::ZIsEmpty()
{
    if(0==this->m_nValidElement)
    {
        return true;
    }else{
        return false;
    }
}

qint32 ZRingBuffer::ZPutElement(const qint8 *buffer,qint32 len)
{
    this->m_mutex.lock();
    //如果队列满则等待,此处超时10s后失败.
    while(this->ZIsFull())
    {
        if(!this->m_wcRBNotFull.wait(&this->m_mutex,1000*10))
        {
            this->m_mutex.unlock();
            return -1;
        }
    }

    //写入数据.
    ZRingBufferElement *element=this->m_vec->at(this->m_nWrPos);
    memcpy(element->m_pData,buffer,len);
    element->m_nLen=len;

    //调整写指针.
    //wrPos始终指向下一个要写入的地址.
    this->m_nWrPos++;
    if(this->m_nWrPos==this->m_nMaxElement)
    {
        this->m_nWrPos=0;
    }
    //放入一个元素，有效个数加1.
    this->m_nValidElement++;

    //唤醒因队列空而引起的等待线程.
    this->m_wcRBNotEmpty.wakeAll();
    this->m_mutex.unlock();

    return 0;
}
qint32 ZRingBuffer::ZTryPutElement(const qint8 *buffer,qint32 len)
{
    this->m_mutex.lock();
    //如果队列满则立即返回,防止阻塞.
    if(this->ZIsFull())
    {
        this->m_mutex.unlock();
        return -1;
    }

    //写入数据.
    ZRingBufferElement *element=this->m_vec->at(this->m_nWrPos);
    memcpy(element->m_pData,buffer,len);
    element->m_nLen=len;

    //调整写指针.
    //wrPos始终指向下一个要写入的地址.
    this->m_nWrPos++;
    if(this->m_nWrPos==this->m_nMaxElement)
    {
        this->m_nWrPos=0;
    }
    //放入一个元素，有效个数加1.
    this->m_nValidElement++;

    //唤醒因队列空而引起的等待线程.
    this->m_wcRBNotEmpty.wakeAll();
    this->m_mutex.unlock();

    return 0;
}
qint32 ZRingBuffer::ZGetElement(qint8 *buffer,qint32 len)
{
    this->m_mutex.lock();
    //如果队列空则等待,此处超时10s后失败.
    while(this->ZIsEmpty())
    {
        if(!this->m_wcRBNotEmpty.wait(&this->m_mutex,1000*10))
        {
            qDebug()<<"<Warning>:timeout to wait for ring buffer has elements.";
            this->m_mutex.unlock();
            return -1;
        }
    }

    ZRingBufferElement *element=this->m_vec->at(this->m_nRdPos);
    if(element->m_nLen>len)
    {
        qDebug()<<"<RingBuffer>:get element buffer is not big enough.";
        this->m_mutex.unlock();
        return -1;
    }
    //rdPos始终指定当前可读取的地址.
    memcpy(buffer,element->m_pData,element->m_nLen);

    //调整读指针.
    this->m_nRdPos++;
    if(this->m_nRdPos==this->m_nMaxElement)
    {
        this->m_nRdPos=0;
    }

    //取出一个元素，有效个数减1.
    this->m_nValidElement--;

    //唤醒因队列满而引起等待的线程.
    this->m_wcRBNotFull.wakeAll();
    this->m_mutex.unlock();

    return element->m_nLen;
}
qint32 ZRingBuffer::ZTryGetElement(qint8 *buffer,qint32 len)
{
    this->m_mutex.lock();
    //如果队列空则立即返回，防止阻塞.
    if(this->ZIsEmpty())
    {
        this->m_mutex.unlock();
        return -1;
    }

    ZRingBufferElement *element=this->m_vec->at(this->m_nRdPos);
    if(element->m_nLen>len)
    {
        qDebug()<<"<RingBuffer>:get element buffer is not big enough.";
        this->m_mutex.unlock();
        return -1;
    }
    //rdPos始终指定当前可读取的地址.
    memcpy(buffer,element->m_pData,element->m_nLen);

    //调整读指针.
    this->m_nRdPos++;
    if(this->m_nRdPos==this->m_nMaxElement)
    {
        this->m_nRdPos=0;
    }

    //取出一个元素，有效个数减1.
    this->m_nValidElement--;

    //唤醒因队列满而引起等待的线程.
    this->m_wcRBNotFull.wakeAll();
    this->m_mutex.unlock();

    return element->m_nLen;
}
qint32 ZRingBuffer::ZGetValidNum()
{
    this->m_mutex.lock();
    qint32 nRet=this->m_nValidElement;
    this->m_mutex.unlock();
    return nRet;
}
