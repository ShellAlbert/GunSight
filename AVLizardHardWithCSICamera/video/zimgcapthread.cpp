#include "zimgcapthread.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <fcntl.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <video/yuv2rgb.h>

ZImgCapThread::ZImgCapThread(QString devFile,CAM_ID_TYPE_E camIdType)
{
    this->m_devFile=devFile;
    this->m_nCamIdType=camIdType;
    this->m_bCleanup=true;

    this->m_nTotalFrames=0;
    this->m_nTotalTime=0;
    //remember current timestamp in millsecond.
    this->m_nLastTsMsec=QDateTime::currentMSecsSinceEpoch();
}
ZImgCapThread::~ZImgCapThread()
{

}
//capture -> processing.
void ZImgCapThread::ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut1=freeQueue;
    this->m_usedQueueOut1=usedQueue;
    this->m_mutexOut1=mutex;
    this->m_condQueueEmptyOut1=condQueueEmpty;
    this->m_condQueueFullOut1=condQueueFull;
}

//capture -> h264 encoder.
void ZImgCapThread::ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut2=freeQueue;
    this->m_usedQueueOut2=usedQueue;
    this->m_mutexOut2=mutex;
    this->m_condQueueEmptyOut2=condQueueEmpty;
    this->m_condQueueFullOut2=condQueueFull;
}
qint32 ZImgCapThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZImgCapThread::ZStopThread()
{
    this->quit();
    this->wait(1000);
    return 0;
}
QString ZImgCapThread::ZGetDevName()
{
    return this->m_devFile;
}
void ZImgCapThread::doCleanBeforeExit()
{
    //set global request to exit flag to notify other threads.
    gGblPara.m_bGblRst2Exit=true;
    this->m_bCleanup=true;
    emit this->ZSigFinished();
}
void ZImgCapThread::run()
{
    bool bTimeOutRetryFlag;
timeout_retry:
    bTimeOutRetryFlag=false;
    if(this->m_nCamIdType==CAM3_ID_MainEx)
    {
        while(!gGblPara.m_bGblRst2Exit)
        {
            this->sleep(1);
        }
        return;
    }
    //bind main/main ex img cap thread to cpu core 2.
    //bind aux img cap thread to cpu core 3.
    if(this->m_nCamIdType==CAM1_ID_Main || this->m_nCamIdType==CAM3_ID_MainEx)
    {
        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(2,&cpuSet);
        if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
        {
            qDebug()<<"<Error>:failed to bind MainImgCap thread to cpu core 2.";
        }else{
            qDebug()<<"<Info>:success to bind MainImgCap thread to cpu core 2.";
        }
    }else{
        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(3,&cpuSet);
        if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
        {
            qDebug()<<"<Error>:failed to bind AuxImgCap thread to cpu core 3.";
        }else{
            qDebug()<<"<Info>:success to bind AuxImgCap thread to cpu core 3.";
        }
    }

    //create file to save camera infomation.
    QFile fileInfo;
    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        fileInfo.setFileName("Cam1.main.info");
        //this->sleep(1);
        break;
    case CAM2_ID_Aux:
        fileInfo.setFileName("Cam2.aux.info");
        //this->usleep(1000*500);
        break;
    case CAM3_ID_MainEx:
        fileInfo.setFileName("Cam3.mainEx.info");
        break;
    }
    if(!fileInfo.open(QIODevice::WriteOnly))
    {
        qDebug()<<"<Error>:failed to create cam info file,"<<fileInfo.fileName();
        this->doCleanBeforeExit();
        return;
    }

    //malloc memory for YUV2RGB convert.
    //YUV420P,1920*1080*2.
    //RGB888,1920*1080*3.
    qint32 nSingleImgSize=gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*sizeof(char);
    unsigned char *pRGBBuffer=(unsigned char*)malloc(nSingleImgSize);
    if(NULL==pRGBBuffer)
    {
        qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to allocate RGB buffer.";
        this->doCleanBeforeExit();
        return;
    }

    //open device file.
    qint32 fd=open(this->m_devFile.toStdString().c_str(),O_RDWR|O_NONBLOCK,0);
    if(fd<0)
    {
        qDebug()<<"<Error>:failed to open device file,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }
    //query capabilities.
    struct v4l2_capability cap;
    if(ioctl(fd,VIDIOC_QUERYCAP,&cap)<0)
    {
        qDebug()<<"<Error>:failed to query capability,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    //write capability to file.
    fileInfo.write(this->m_devFile.toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.driver).toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.card).toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.bus_info).toLocal8Bit()+"\n");

    //check capture&streaming ability.
    if(!((cap.capabilities&V4L2_CAP_VIDEO_CAPTURE)&&(cap.capabilities&V4L2_CAP_STREAMING)))
    {
        qDebug()<<"<Error>:CAM has no video streaming capture ability,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        gGblPara.m_video.m_Cam1ID=QString((char*)cap.card);
        break;
    case CAM2_ID_Aux:
        gGblPara.m_video.m_Cam2ID=QString((char*)cap.card);
        break;
    default:
        break;
    }

    //enumrate format description.
    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc,0,sizeof(fmtdesc));
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtdesc.index=0;
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
    {
        char *fmtName=(char*)fmtdesc.description;
        fileInfo.write(QString("%1:%2\n").arg(fmtdesc.index).arg(fmtName).toLatin1());
        fmtdesc.index++;
    }
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd,VIDIOC_G_FMT,&fmt);
    fileInfo.write(QString("%1*%2,color space:%3\n").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height).arg(fmt.fmt.pix.colorspace).toLatin1());

    //request the uvc driver to allocate buffers.
    struct v4l2_requestbuffers reqBuf;
    reqBuf.count=4;
    reqBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuf.memory=V4L2_MEMORY_MMAP;
    if(ioctl(fd,VIDIOC_REQBUFS,&reqBuf)<0)
    {
        qDebug()<<"<Error>:failed to request uvc buffer,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    IMGBufferStruct* pIMGBuffer;
    pIMGBuffer=(IMGBufferStruct*)malloc(reqBuf.count*sizeof(IMGBufferStruct));
    if(NULL==pIMGBuffer)
    {
        qDebug()<<"<Error>:failed to calloc buffer,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }
    //query buffer & do mmap.
    for(quint32 i=0;i<reqBuf.count;i++)
    {
        struct v4l2_buffer buf;
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=i;
        if(ioctl(fd,VIDIOC_QUERYBUF,&buf)<0)
        {
            qDebug()<<"<Error>:failed to query buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
        pIMGBuffer[i].nLength=buf.length;
        pIMGBuffer[i].pStart=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
        if(MAP_FAILED==pIMGBuffer[i].pStart)
        {
            qDebug()<<"<Error>:failed to mmap buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
    }
    //start capturing.
    for(quint32 i=0;i<reqBuf.count;i++)
    {
        //queue buffer.
        struct v4l2_buffer buf;
        memset(&buf,0,sizeof(buf));
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=i;
        if(ioctl(fd,VIDIOC_QBUF,&buf)<0)
        {
            qDebug()<<"<Error>:failed to query buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
    }
    //stream on.
    v4l2_buf_type type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMON,&type)<0)
    {
        qDebug()<<"<Error>:failed to stream on,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    fileInfo.close();

    //working loop.
    while(!gGblPara.m_bGblRst2Exit)
    {
#if 0
        //只有当有客户端连接时才开始采集图像.
        switch(this->m_nCamIdType)
        {
        case CAM1_ID_Main:
        case CAM3_ID_MainEx:
            if(!(gGblPara.m_bVideoTcpConnected))
            {
                this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                continue;
            }
            break;
        case CAM2_ID_Aux:
            if(!gGblPara.m_bVideoTcpConnected2)
            {
                this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                continue;
            }
            break;
        default:
            break;
        }
#endif
        qint32 nLen;
        quint8 *pYUVData;
        qint32 nBufIndex;
        ///////////////////////
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        struct timeval tv;
        tv.tv_sec=1;
        tv.tv_usec=0;//1000*10;//10ms.
        int ret=select(fd+1,&fds,NULL,NULL,&tv);
        if(ret<0)
        {
            qDebug()<<"<Error>:select() cam device error,"<<this->m_devFile;
            break;
        }else if(0==ret){
            qDebug()<<"<Warning>:select() timeout for cam "<<this->m_devFile;
            //continue;
            bTimeOutRetryFlag=true;
            break;
        }else{
            struct v4l2_buffer getBuf;
            memset(&getBuf,0,sizeof(getBuf));
            getBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
            getBuf.memory=V4L2_MEMORY_MMAP;
            if(ioctl(fd,VIDIOC_DQBUF,&getBuf)<0)
            {
                qDebug()<<"<Error>:failed to DQBUF,"<<this->m_devFile;
                break;
            }
            pYUVData=(quint8*)pIMGBuffer[getBuf.index].pStart;
            nLen=pIMGBuffer[getBuf.index].nLength;
            //remember the index.
            nBufIndex=getBuf.index;
        }


#if 0
        //dump yuv420sp to file to check by YUView tool.
        static qint32 nYUVIndex=0;
        QString fileName=QString("%1.%2.yuv").arg(this->m_devFile).arg(nYUVIndex++);
        QFile fileYUV(fileName);
        if(fileYUV.open(QIODevice::WriteOnly))
        {
            fileYUV.write((const char*)pYUVData,nLen);
            fileYUV.close();
            qDebug()<<fileName<<"generated.";
        }else{
            qDebug()<<fileName<<"failed to open.";
        }
#endif

#if 1
        //only m_bJonImgPro flag is set by Android Json protocol.
        //or m_bJsonFlushUIImg flag is set (flush local UI ).
        //here do YUV2RGB convert.
        if(gGblPara.m_bJsonImgPro || gGblPara.m_bJsonFlushUIImg)
        {
            //software YUV2RGB cost 46ms.
            //            struct timeval tStart,tEnd;
            //            unsigned long nCostMs;
            //            gettimeofday(&tStart,NULL);
            nv21_to_rgb(pRGBBuffer,pYUVData,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);
            //            gettimeofday(&tEnd,NULL);
            //            nCostMs=1000000*(tEnd.tv_sec-tStart.tv_sec)+(tEnd.tv_usec-tStart.tv_usec);
            //            nCostMs/=1000;
            //            qDebug()<<this->m_devFile<<"YU2RGB(ms):"<<nCostMs;
            //if flush local UI or not.
            if(gGblPara.m_bJsonFlushUIImg)
            {
                //bytesPerLine=1080*3.
                QImage imgNew((uchar*)pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_widthCAM1*3,QImage::Format_RGB888);
                emit this->ZSigNewImgArrived(imgNew);
            }

            if(gGblPara.m_bJsonImgPro)
            {
                //1.put yuv data to img process fifo.
                this->m_mutexOut1->lock();
                while(this->m_freeQueueOut1->isEmpty())//timeout 5s to check exit flag.
                {
                    if(!this->m_condQueueEmptyOut1->wait(this->m_mutexOut1,5000))
                    {
                        this->m_mutexOut1->unlock();
                        if(gGblPara.m_bGblRst2Exit)
                        {
                            break;
                        }
                    }
                }
                if(gGblPara.m_bGblRst2Exit)
                {
                    break;
                }

                QByteArray *pcmBuffer1=this->m_freeQueueOut1->dequeue();
                memcpy(pcmBuffer1->data(),pRGBBuffer,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*sizeof(char));
                this->m_usedQueueOut1->enqueue(pcmBuffer1);
                this->m_condQueueFullOut1->wakeAll();
                this->m_mutexOut1->unlock();
            }
        }
#endif

#if 1
        //2.put yuv data to h264 encode fifo.
        //get a free buffer from fifo.
        this->m_mutexOut2->lock();
        while(this->m_freeQueueOut2->isEmpty())//timeout 5s to check exit flag.
        {
            if(!this->m_condQueueEmptyOut2->wait(this->m_mutexOut2,5000))
            {
                this->m_mutexOut2->unlock();
                if(gGblPara.m_bGblRst2Exit)
                {
                    break;
                }
            }
        }
        if(gGblPara.m_bGblRst2Exit)
        {
            break;
        }

        QByteArray *pcmBuffer2=this->m_freeQueueOut2->dequeue();
        memcpy(pcmBuffer2->data(),pYUVData,nLen);
        this->m_usedQueueOut2->enqueue(pcmBuffer2);
        this->m_condQueueFullOut2->wakeAll();
        this->m_mutexOut2->unlock();
#endif
        //free a buffer for device.
        struct v4l2_buffer putBuf;
        memset(&putBuf,0,sizeof(putBuf));
        putBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        putBuf.memory=V4L2_MEMORY_MMAP;
        putBuf.index=nBufIndex;
        if(ioctl(fd,VIDIOC_QBUF,&putBuf)<0)
        {
            qDebug()<<"<Error>:failed to DQBUF,"<<this->m_devFile;
            break;
        }

        //calculate the fps.
        this->m_nTotalFrames++;
        qint64 nNowTsMsec=QDateTime::currentMSecsSinceEpoch();
        this->m_nTotalTime+=(nNowTsMsec-this->m_nLastTsMsec);
        this->m_nLastTsMsec=nNowTsMsec;
        qint64 nTotalMsec=this->m_nTotalTime/1000;

        //reduce CPU heavy load.
        switch(this->m_nCamIdType)
        {
        case CAM1_ID_Main:
            if(nTotalMsec>0)
            {
                gGblPara.m_video.m_nCam1Fps=this->m_nTotalFrames/nTotalMsec;
            }else{
                gGblPara.m_video.m_nCam1Fps=0;
            }
            //this->usleep(1000*10);
            //qDebug()<<"Main fps:"<<gGblPara.m_video.m_nCam1Fps;
            break;
        case CAM2_ID_Aux:
            if(nTotalMsec>0)
            {
                gGblPara.m_video.m_nCam2Fps=this->m_nTotalFrames/nTotalMsec;
            }else{
                gGblPara.m_video.m_nCam2Fps=0;
            }
            //this->usleep(1000*15);
            //qDebug()<<"Aux fps:"<<gGblPara.m_video.m_nCam2Fps;
            break;
        case CAM3_ID_MainEx:
            //this->usleep(1000*20);
            break;
        default:
            break;
        }
        //qDebug()<<QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")<<this->m_devFile;
    }

    //stop capture.
    type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMOFF,&type)<0)
    {
        qDebug()<<"<Error>:failed to stream off,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    for(quint32 i=0;i<reqBuf.count;i++)
    {
        munmap(pIMGBuffer[i].pStart,pIMGBuffer[i].nLength);
    }
    free(pIMGBuffer);
    free(pRGBBuffer);
    close(fd);
    if(bTimeOutRetryFlag)
    {
        qDebug()<<this->m_devFile<<",timeout reset!";
        goto timeout_retry;
    }
    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainCamera capture ends "<<this->m_devFile<<".";
        break;
    case CAM2_ID_Aux:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"AuxCamera capture ends "<<this->m_devFile<<".";
        break;
    case CAM3_ID_MainEx:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainExCamera capture ends "<<this->m_devFile<<".";
        break;
    default:
        break;
    }
    this->doCleanBeforeExit();
    return;
}
bool ZImgCapThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
