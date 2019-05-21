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
    this->m_process=NULL;//图像处理线程.
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
    for(qint32 i=0;i<5;i++)
    {
        delete this->m_Cap2EncFIFOMain[i];
    }
    this->m_Cap2EncFIFOFreeMain.clear();
    this->m_Cap2EncFIFOUsedMain.clear();
    //aux capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        delete this->m_Cap2EncFIFOAux[i];
    }
    this->m_Cap2EncFIFOFreeAux.clear();
    this->m_Cap2EncFIFOUsedAux.clear();

    //main capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        delete this->m_Cap2ProFIFOMain[i];
    }
    this->m_Cap2ProFIFOFreeMain.clear();
    this->m_Cap2ProFIFOUsedMain.clear();
    //aux capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        delete this->m_Cap2ProFIFOAux[i];
    }
    this->m_Cap2ProFIFOFreeAux.clear();
    this->m_Cap2ProFIFOUsedAux.clear();
}
qint32 ZVideoTask::ZDoInit()
{
    //find out how many videoX device file we have.
    QStringList devFileList;
    QDir dir("/dev");
    QStringList allFileList=dir.entryList(QDir::System);
    for(qint32 i=0;i<allFileList.size();i++)
    {
        QString devFile=allFileList.at(i);
        if(devFile.startsWith("video"))
        {
            devFileList.append(devFile);
        }
    }
    qDebug()<<devFileList;
    if(devFileList.size()<3)
    {
        qDebug()<<"<Error>:Greater than 3 cameras are needed for run this app.";
        qDebug()<<"<Error>:Please check camera connections and run again!";
        return -1;
    }

    //filte the device file /dev/videoXX to find out which supports VideoCaptureStreaming ability.
    QStringList validDevFileList;
    for(qint32 i=0;i<devFileList.size();i++)
    {
        QString devFileName=QString("/dev/")+devFileList.at(i);
        int fd=open(devFileName.toLatin1().data(),O_RDWR);
        if(fd<0)
        {
            qDebug()<<"<Error>:failed to open device file "<<devFileName;
            continue;
        }
        struct v4l2_capability cap;
        ioctl(fd,VIDIOC_QUERYCAP,&cap);
        qDebug()<<devFileName<<",driver:"<<cap.driver<<",card:"<<cap.card<<",bus:"<<cap.bus_info;
        if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && (cap.capabilities & V4L2_CAP_STREAMING))
        {
            validDevFileList.append(devFileName);
        }else{
            qDebug()<<"<Warning>:"<<devFileName<<"has no capability to capture &streaming.";
        }
    }
    qDebug()<<validDevFileList;
    if(validDevFileList.size()<3)
    {
        qDebug()<<"<Error>:Less than 3 cameras support video capture & streaming.";
        qDebug()<<"<Error>:So this app cannot run correctly! please check cameras!";
        return -1;
    }


    //main capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2EncFIFOMain[i]=new QByteArray;
        this->m_Cap2EncFIFOMain[i]->resize(MIPI_CSI2_YUV_SIZE);
        this->m_Cap2EncFIFOFreeMain.enqueue(this->m_Cap2EncFIFOMain[i]);
    }
    this->m_Cap2EncFIFOUsedMain.clear();
    //aux capture to h264 encoder queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2EncFIFOAux[i]=new QByteArray;
        this->m_Cap2EncFIFOAux[i]->resize(MIPI_CSI2_YUV_SIZE);
        this->m_Cap2EncFIFOFreeAux.enqueue(this->m_Cap2EncFIFOAux[i]);
    }
    this->m_Cap2EncFIFOUsedAux.clear();

    //main capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2ProFIFOMain[i]=new QByteArray;
        this->m_Cap2ProFIFOMain[i]->resize(MIPI_CSI2_YUV_SIZE);
        this->m_Cap2ProFIFOFreeMain.enqueue(this->m_Cap2ProFIFOMain[i]);
    }
    this->m_Cap2ProFIFOUsedMain.clear();
    //aux capture to ImgProcess queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2ProFIFOAux[i]=new QByteArray;
        this->m_Cap2ProFIFOAux[i]->resize(MIPI_CSI2_YUV_SIZE);
        this->m_Cap2ProFIFOFreeAux.enqueue(this->m_Cap2ProFIFOAux[i]);
    }
    this->m_Cap2ProFIFOUsedAux.clear();

    //create capture threads & bind FIFOs.
    this->m_capThread[0]=new ZImgCapThread(validDevFileList.at(0),CAM1_ID_Main);
    this->m_capThread[0]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_capThread[0]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);
    QObject::connect(this->m_capThread[0],SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    this->m_capThread[1]=new ZImgCapThread(validDevFileList.at(1),CAM2_ID_Aux);
    this->m_capThread[1]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeAux,&this->m_Cap2ProFIFOUsedAux,&this->m_Cap2ProFIFOMutexAux,&this->m_condCap2ProFIFOEmptyAux,&this->m_condCap2ProFIFOFullAux);
    this->m_capThread[1]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeAux,&this->m_Cap2EncFIFOUsedAux,&this->m_Cap2EncFIFOMutexAux,&this->m_condCap2EncFIFOEmptyAux,&this->m_condCap2EncFIFOFullAux);
    QObject::connect(this->m_capThread[1],SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    this->m_capThread[2]=new ZImgCapThread(validDevFileList.at(2),CAM3_ID_MainEx);
    this->m_capThread[2]->ZBindOut1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_capThread[2]->ZBindOut2FIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);

    //create process thread & bind FIFO.
    this->m_process=new ZImgProcessThread;
    this->m_process->ZBindIn1FIFO(&this->m_Cap2ProFIFOFreeMain,&this->m_Cap2ProFIFOUsedMain,&this->m_Cap2ProFIFOMutexMain,&this->m_condCap2ProFIFOEmptyMain,&this->m_condCap2ProFIFOFullMain);
    this->m_process->ZBindIn2FIFO(&this->m_Cap2ProFIFOFreeAux,&this->m_Cap2ProFIFOUsedAux,&this->m_Cap2ProFIFOMutexAux,&this->m_condCap2ProFIFOEmptyAux,&this->m_condCap2ProFIFOFullAux);
    QObject::connect(this->m_process,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    //create encode/tx thread & bind FIFO.
    this->m_encTxThread=new ZHardEncTxThread(TCP_PORT_VIDEO);
    this->m_encTxThread->ZBindInFIFO(&this->m_Cap2EncFIFOFreeMain,&this->m_Cap2EncFIFOUsedMain,&this->m_Cap2EncFIFOMutexMain,&this->m_condCap2EncFIFOEmptyMain,&this->m_condCap2EncFIFOFullMain);
    QObject::connect(this->m_encTxThread,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

    this->m_encTx2Thread=new ZHardEncTx2Thread(TCP_PORT_VIDEO2);
    this->m_encTx2Thread->ZBindInFIFO(&this->m_Cap2EncFIFOFreeAux,&this->m_Cap2EncFIFOUsedAux,&this->m_Cap2EncFIFOMutexAux,&this->m_condCap2EncFIFOEmptyAux,&this->m_condCap2EncFIFOFullAux);
    QObject::connect(this->m_encTx2Thread,SIGNAL(ZSigFinished()),this,SLOT(ZSlotSubThreadsFinished()));

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
        this->m_capThread[0]->exit(0);
        this->m_capThread[1]->exit(0);
        this->m_capThread[2]->exit(0);
        this->m_process->exit(0);
        this->m_encTxThread->exit(0);
        this->m_encTx2Thread->exit(0);

        //start timer to help unblocking the queue empty or full.
        this->m_timerExit->start(1000);
    }
}
void ZVideoTask::ZSlotChkAllExitFlags()
{
    if(gGblPara.m_bGblRst2Exit)
    {
        //if MainCapThread doesnot exit,maybe Cap2ProFreeQueueMain is empty to cause thread blocks.
        //or Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from Cap2ProUsedQueueMain to Cap2ProFreeQueueMain to unblock.
        //here we move a element from Cap2EncFIFOUsedMain to Cap2EncFIFOFreeMain to unblock.
        if(!this->m_capThread[0]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainVideoCapThread...";
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

        //if AuxCapThread doesnot exit,maybe Cap2ProFreeQueueMain is empty to cause thread blocks.
        //or Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from Cap2ProUsedQueueMain to Cap2ProFreeQueueMain to unblock.
        //here we move a element from Cap2EncFIFOUsedMain to Cap2EncFIFOFreeMain to unblock.
        if(!this->m_capThread[1]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for AuxVideoCapThread...";
            this->m_Cap2ProFIFOMutexAux.lock();
            if(!this->m_Cap2ProFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2ProFIFOUsedAux.dequeue();
                this->m_Cap2ProFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2ProFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2ProFIFOMutexAux.unlock();

            this->m_Cap2ProFIFOMutexAux.lock();
            if(!this->m_Cap2EncFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedMain.dequeue();
                this->m_Cap2EncFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2ProFIFOMutexAux.unlock();
        }

        //if MainExCapThread doesnot exit,maybe Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from Cap2ProFIFOUsedMain to Cap2ProFIFOFreeMain to unblock.
        //here we move a element from Cap2EncFIFOUsedMain to Cap2EncFIFOFreeMain to unblock.
        if(!this->m_capThread[2]->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for MainExVideoCapThread...";
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

        //if ProcessThread doesnot exit,maybe Cap2ProFIFOFreeMain is empty to cause thread blocks.
        //or m_Cap2ProFIFOFreeAux is empty to cause thread blocks.
        //here we move a element from Cap2ProFIFOUsedMain to Cap2ProFIFOFreeMain to unblock.
        //here we move a element from Cap2ProFIFOUsedAux to m_Cap2ProFIFOFreeAux to unblock.
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

        //if EncTx1Thread doesnot exit,maybe Cap2EncFIFOFreeMain is empty to cause thread blocks.
        //here we move a element from Cap2EncFIFOUsedMain to Cap2EncFIFOFreeMain to unblock.
        if(!this->m_encTxThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for EncTx1Thread...";
            this->m_Cap2EncFIFOMutexMain.lock();
            if(!this->m_Cap2EncFIFOUsedMain.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedMain.dequeue();
                this->m_Cap2EncFIFOFreeMain.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyMain.wakeAll();
            }
            this->m_Cap2EncFIFOMutexMain.unlock();
        }

        //if EncTx2Thread doesnot exit,maybe Cap2EncFIFOFreeAux is empty to cause thread blocks.
        //here we move a element from Cap2EncFIFOUsedAux to Cap2EncFIFOFreeAux to unblock.
        if(!this->m_encTx2Thread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for EncTx2Thread...";
            this->m_Cap2EncFIFOMutexAux.lock();
            if(!this->m_Cap2EncFIFOUsedAux.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2EncFIFOUsedAux.dequeue();
                this->m_Cap2EncFIFOFreeAux.enqueue(elementHelp);
                this->m_condCap2EncFIFOEmptyAux.wakeAll();
            }
            this->m_Cap2EncFIFOMutexAux.unlock();
        }

        //when all threads exited,then video task exit.
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
