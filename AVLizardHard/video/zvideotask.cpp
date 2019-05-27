#include "zvideotask.h"
#include <video/zfiltecamdev.h>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QDir>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/videodev2.h>
ZVideoTask::ZVideoTask(QObject *parent) : QObject(parent)
{
    for(qint32 i=0;i<3;i++)
    {
        this->m_capThread[i]=NULL;
    }
    this->m_process=NULL;
    this->m_encTxThread=NULL;
    this->m_encTx2Thread=NULL;
    this->m_timerExit=new QTimer;
    QObject::connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotChkAllExitFlags()));
}
ZVideoTask::~ZVideoTask()
{
    delete this->m_timerExit;

    for(qint32 i=0;i<3;i++)
    {
        delete this->m_capThread[i];
    }
    delete this->m_process;
    delete this->m_encTxThread;
    delete this->m_encTx2Thread;

    //clean FIFOs.
    //main capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        delete this->m_Cap2EncFIFOMain[i];
    }
    this->m_Cap2EncFIFOFreeMain.clear();
    this->m_Cap2EncFIFOUsedMain.clear();
    //aux capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        delete this->m_Cap2EncFIFOAux[i];
    }
    this->m_Cap2EncFIFOFreeAux.clear();
    this->m_Cap2EncFIFOUsedAux.clear();

    //main capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        delete this->m_Cap2ProFIFOMain[i];
    }
    this->m_Cap2ProFIFOFreeMain.clear();
    this->m_Cap2ProFIFOUsedMain.clear();
    //aux capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        delete this->m_Cap2ProFIFOAux[i];
    }
    this->m_Cap2ProFIFOFreeAux.clear();
    this->m_Cap2ProFIFOUsedAux.clear();
}
qint32 ZVideoTask::ZDoInit()
{
    //需要排除的设备节点列表.
    QStringList extractNode;
    //extractNode.append("video0");
    //    extractNode.append("video1");
    //    extractNode.append("video16");
    //    extractNode.append("video17");

    //列出/dev目录下所有的videoX设备节点.
    QStringList nodeNameList;
    QDir dir("/dev");
    QStringList fileList=dir.entryList(QDir::System);
    for(qint32 i=0;i<fileList.size();i++)
    {
        QString nodeName=fileList.at(i);
        if(nodeName.startsWith("video"))
        {
            if(!extractNode.contains(nodeName))
            {
                nodeNameList.append(nodeName);
            }
        }
    }

    if(nodeNameList.size()<3)
    {
        qDebug()<<"<Error>:No 3 cameras found at least.";
        return -1;
    }

    //比对配置文件ini设置的CamID来决定哪一个是主摄像头，哪一个是主摄像头Ex,哪一个是辅摄像头.
    bool bMainIdChk=false,bMainExIdChk=false,bAuxIdChk=false;
    QFile fileUSBBusId("usb_bus_id.txt");
    fileUSBBusId.open(QIODevice::WriteOnly);
    for(qint32 i=0;i<nodeNameList.size();i++)
    {
        struct v4l2_capability cap;
        QString nodeDev="/dev/"+nodeNameList.at(i);
        int fd=open(nodeDev.toStdString().c_str(),O_RDWR);
        if(ioctl(fd,VIDIOC_QUERYCAP,&cap)<0)
        {
            qDebug()<<"<Error>:failed to query capability "<<nodeDev;
            return -1;
        }
        qDebug()<<"<Info>:"<<nodeDev<<","<<QString((char*)cap.bus_info);
        fileUSBBusId.write(QString(nodeDev+QString((char*)cap.bus_info)).toUtf8()+"\n");
        if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam1ID)
        {
            bMainIdChk=true;
        }else if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam1IDEx)
        {
            bMainExIdChk=true;
        }else if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam2ID)
        {
            bAuxIdChk=true;
        }
        gGblPara.m_video.m_mapID2Fd.insert(QString((char*)cap.bus_info),nodeDev);
        close(fd);
    }
    fileUSBBusId.close();
    if(!(bMainIdChk && bMainExIdChk && bAuxIdChk))
    {
        qDebug()<<"<Error>:config file USB IDs do not match real USB IDs!";
        return -1;
    }

    //create FIFOs.
    //main capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_Cap2EncFIFOMain[i]=new QByteArray;
        this->m_Cap2EncFIFOMain[i]->resize(FIFO_SIZE);
        this->m_Cap2EncFIFOFreeMain.enqueue(this->m_Cap2EncFIFOMain[i]);
    }
    this->m_Cap2EncFIFOUsedMain.clear();
    //aux capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_Cap2EncFIFOAux[i]=new QByteArray;
        this->m_Cap2EncFIFOAux[i]->resize(FIFO_SIZE);
        this->m_Cap2EncFIFOFreeAux.enqueue(this->m_Cap2EncFIFOAux[i]);
    }
    this->m_Cap2EncFIFOUsedAux.clear();

    //main capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_Cap2ProFIFOMain[i]=new QByteArray;
        this->m_Cap2ProFIFOMain[i]->resize(FIFO_SIZE);
        this->m_Cap2ProFIFOFreeMain.enqueue(this->m_Cap2ProFIFOMain[i]);
    }
    this->m_Cap2ProFIFOUsedMain.clear();
    //aux capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_Cap2ProFIFOAux[i]=new QByteArray;
        this->m_Cap2ProFIFOAux[i]->resize(FIFO_SIZE);
        this->m_Cap2ProFIFOFreeAux.enqueue(this->m_Cap2ProFIFOAux[i]);
    }
    this->m_Cap2ProFIFOUsedAux.clear();



    //create capture thread.
    this->m_capThread[0]=new ZImgCapThread(gGblPara.m_video.m_Cam1ID,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM1_ID_Main);//Main Camera.
    this->m_capThread[1]=new ZImgCapThread(gGblPara.m_video.m_Cam2ID,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM2_ID_Aux);//Aux Camera.
    this->m_capThread[2]=new ZImgCapThread(gGblPara.m_video.m_Cam1IDEx,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM3_ID_MainEx);//MainEx Camera.

    this->m_capThread[0]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_capThread[0]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);
    QObject::connect(this->m_capThread[0],SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    this->m_capThread[1]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeAux,&this->m_Cap2ProFIFOUsedAux,&this->m_Cap2ProFIFOMutexAux,&this->m_condCap2ProFIFOEmptyAux,&this->m_condCap2ProFIFOFullAux);
    this->m_capThread[1]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeAux,&this->m_Cap2EncFIFOUsedAux,&this->m_Cap2EncFIFOMutexAux,&this->m_condCap2EncFIFOEmptyAux,&this->m_condCap2EncFIFOFullAux);
    QObject::connect(this->m_capThread[1],SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    this->m_capThread[2]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_capThread[2]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);
    QObject::connect(this->m_capThread[2],SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));


    this->m_encTxThread=new ZHardEncTxThread(TCP_PORT_VIDEO);
    this->m_encTxThread->ZBindInFIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);
    QObject::connect(this->m_encTxThread,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    //create video tx thread.
    this->m_encTx2Thread=new ZHardEncTx2Thread(TCP_PORT_VIDEO2);
    this->m_encTx2Thread->ZBindInFIFO(&this->m_Cap2EncFIFOFreeAux,&this->m_Cap2EncFIFOUsedAux,&this->m_Cap2EncFIFOMutexAux,&this->m_condCap2EncFIFOEmptyAux,&this->m_condCap2EncFIFOFullAux);
    QObject::connect(this->m_encTx2Thread,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    //create image process thread.
    this->m_process=new ZImgProcessThread;
    this->m_process->ZBindIn1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_process->ZBindIn2FIFO(&this->m_Cap2ProFIFOFreeAux,&this->m_Cap2ProFIFOUsedAux,&this->m_Cap2ProFIFOMutexAux,&this->m_condCap2ProFIFOEmptyAux,&this->m_condCap2ProFIFOFullAux);
    QObject::connect(this->m_process,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    return 0;
}
ZImgCapThread* ZVideoTask::ZGetImgCapThread(qint32 index)
{
    ZImgCapThread *capThread=NULL;
    switch(index)
    {
    case 0:
        capThread=this->m_capThread[0];
        break;
    case 1:
        capThread=this->m_capThread[1];
        break;
    default:
        break;
    }
    return capThread;
}
ZImgProcessThread* ZVideoTask::ZGetImgProcessThread()
{
    return this->m_process;
}
qint32 ZVideoTask::ZStartTask()
{
    this->m_encTxThread->ZStartThread();
    this->m_encTx2Thread->ZStartThread();
    for(qint32 i=0;i<3;i++)
    {
        this->m_capThread[i]->ZStartThread();
    }
    this->m_process->ZStartThread();
    return 0;
}
void ZVideoTask::ZSlotSubThreadsFinished()
{
    if(!this->m_timerExit->isActive())
    {
        //notify all working threads to exit.
        for(qint32 i=0;i<3;i++)
        {
            this->m_capThread[i]->ZStopThread();
        }
        this->m_process->ZStopThread();
        this->m_encTxThread->ZStopThread();
        this->m_encTx2Thread->ZStopThread();

        //start timer to help unblocking the queue empty or full.
        this->m_timerExit->start(1000);
    }
}
void ZVideoTask::ZSlotChkAllExitFlags()
{
    if(gGblPara.m_bGblRst2Exit)
    {
        //if CapThread[0] doesnot exit,maybe m_Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or m_Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from m_Cap2ProFIFOUsedMain to m_Cap2ProFIFOFreeMain to unblock.
        if(!this->m_capThread[0]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainVideoCaptureThread...";
            this->m_Cap2ProFIFOMutexMain.lock();
            if(!this->m_Cap2ProFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedMain.dequeue();
                this->m_Cap2ProFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2ProFIFOMutexMain.unlock();

            this->m_Cap2EncFIFOMutexMain.lock();
            if(!this->m_Cap2EncFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedMain.dequeue();
                this->m_Cap2EncFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2EncFIFOMutexMain.unlock();
        }
        //if CapThread[1] doesnot exit,maybe m_Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or m_Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from m_Cap2ProFIFOUsedMain to m_Cap2ProFIFOFreeMain to unblock.
        if(!this->m_capThread[1]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for AuxVideoCaptureThread...";
            this->m_Cap2ProFIFOMutexAux.lock();
            if(!this->m_Cap2ProFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedAux.dequeue();
                this->m_Cap2ProFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2ProFIFOMutexAux.unlock();

            this->m_Cap2EncFIFOMutexAux.lock();
            if(!this->m_Cap2EncFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedAux.dequeue();
                this->m_Cap2EncFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2EncFIFOMutexAux.unlock();
        }
        //if CapThread[2] doesnot exit,maybe m_Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or m_Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from m_Cap2ProFIFOUsedMain to m_Cap2ProFIFOFreeMain to unblock.
        if(!this->m_capThread[2]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainVideoCaptureThread...";
            this->m_Cap2ProFIFOMutexMain.lock();
            if(!this->m_Cap2ProFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedMain.dequeue();
                this->m_Cap2ProFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2ProFIFOMutexMain.unlock();

            this->m_Cap2EncFIFOMutexMain.lock();
            if(!this->m_Cap2EncFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedMain.dequeue();
                this->m_Cap2EncFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2EncFIFOMutexMain.unlock();
        }

        //if m_encTxThread doesnot exit,maybe m_Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from m_Cap2EncFIFOUsedMain to m_Cap2EncFIFOFreeMain to unblock.
        if(!this->m_encTxThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainEncTxThread...";
            this->m_Cap2EncFIFOMutexMain.lock();
            if(!this->m_Cap2EncFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedMain.dequeue();
                this->m_Cap2EncFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2EncFIFOMutexMain.unlock();
        }

        //if m_encTx2Thread doesnot exit,maybe m_Cap2EncFIFOFreeAux is empty to cause thread blocks.
        //here we move a element from m_Cap2EncFIFOUsedAux to m_Cap2EncFIFOFreeAux to unblock.
        if(!this->m_encTx2Thread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainEncTx2Thread...";
            this->m_Cap2EncFIFOMutexAux.lock();
            if(!this->m_Cap2EncFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedAux.dequeue();
                this->m_Cap2EncFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2EncFIFOMutexAux.unlock();
        }

        //if m_process doesnot exit,maybe m_Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or m_Cap2ProFIFOUsedAux is empty to cause thread blocks.
        //here we move a element from m_Cap2ProFIFOUsedMain to m_Cap2ProFIFOFreeMain to unblock.
        //here we move a element from m_Cap2ProFIFOUsedAux to m_Cap2ProFIFOFreeAux to unblock.
        if(!this->m_process->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for ProcessThread...";
            this->m_Cap2ProFIFOMutexMain.lock();
            if(!this->m_Cap2ProFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedMain.dequeue();
                this->m_Cap2ProFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2ProFIFOMutexMain.unlock();

            this->m_Cap2ProFIFOMutexAux.lock();
            if(!this->m_Cap2ProFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedAux.dequeue();
                this->m_Cap2ProFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2ProFIFOMutexAux.unlock();
        }

        //video task exit after all sub threads exited.
        if(this->ZIsExitCleanup())
        {
            this->m_timerExit->stop();
            emit this->ZSigVideoTaskExited();
        }
    }
}
bool ZVideoTask::ZIsExitCleanup()
{
    bool bCleanup=true;
    if(!this->m_capThread[0]->ZIsExitCleanup() || !this->m_capThread[1]->ZIsExitCleanup() || !this->m_capThread[2]->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    if(!this->m_process->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    if(!this->m_encTxThread->ZIsExitCleanup() || !this->m_encTx2Thread->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    return bCleanup;
}
