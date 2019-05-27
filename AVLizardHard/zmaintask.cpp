#include "zmaintask.h"

ZMainTask::ZMainTask()
{
    this->m_tcp2Uart=NULL;
    this->m_ctlJson=NULL;
    this->m_audio=NULL;
    this->m_video=NULL;
    this->m_ui=NULL;
}
ZMainTask::~ZMainTask()
{
    delete this->m_timerExit;
    //wait until tcp2Uart thread ends.
    this->m_tcp2Uart->quit();
    this->m_tcp2Uart->wait();
    delete this->m_tcp2Uart;
    //wait until ctlJson thread ends.
    this->m_ctlJson->quit();
    this->m_ctlJson->wait();
    delete this->m_ctlJson;
    //wait until audio task clean up.
    while(!this->m_audio->ZIsExitCleanup());
    delete this->m_audio;
    //wait until video task clean up.
    while(!this->m_video->ZIsExitCleanup());
    delete this->m_video;
    delete this->m_ui;
}
qint32 ZMainTask::ZStartTask()
{
    this->m_timerExit=new QTimer;
    QObject::connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotChkAllExitFlags()));

    //start Android(tcp) <--> STM32(uart) forward task.
    this->m_tcp2Uart=new ZTcp2UartForwardThread;
    QObject::connect(this->m_tcp2Uart,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));
    this->m_tcp2Uart->ZStartThread();

    //ctl thread for Video/Audio.
    this->m_ctlJson=new ZJsonThread(0);
    QObject::connect(this->m_ctlJson,SIGNAL(ZSigThreadExited()),this,SLOT(ZSlotSubThreadsExited()));
    this->m_ctlJson->ZStartThread();

    //audio task.
    this->m_audio=new ZAudioTask;
    QObject::connect(this->m_audio,SIGNAL(ZSigAudioTaskExited()),this,SLOT(ZSlotSubThreadsExited()));
    if(this->m_audio->ZStartTask()<0)
    {
        qDebug()<<"<Error>:failed to start audio task.";
        return -1;
    }

    //video task.
    this->m_video=new ZVideoTask;
    QObject::connect(this->m_video,SIGNAL(ZSigVideoTaskExited()),this,SLOT(ZSlotSubThreadsExited()));
    if(this->m_video->ZDoInit()<0)
    {
        qDebug()<<"<Error>:failed to init video task.";
        return -1;
    }
    if(this->m_video->ZStartTask()<0)
    {
        qDebug()<<"<Error>:failed to start video task!";
        return -1;
    }

    //AV UI.
    this->m_ui=new ZAVUI;

    //use signal-slot event to notify local UI flush.
    QObject::connect(this->m_video->ZGetImgCapThread(0),SIGNAL(ZSigNewImgArrived(QImage)),this->m_ui->ZGetImgDisp(0),SLOT(ZSlotFlushImg(QImage)),Qt::AutoConnection);
    QObject::connect(this->m_video->ZGetImgCapThread(1),SIGNAL(ZSigNewImgArrived(QImage)),this->m_ui->ZGetImgDisp(1),SLOT(ZSlotFlushImg(QImage)),Qt::AutoConnection);

    //use signal-slot event to notify UI to flush new image process set.
    QObject::connect(this->m_video->ZGetImgProcessThread(),SIGNAL(ZSigNewMatchedSetArrived(ZImgMatchedSet)),this->m_ui,SLOT(ZSlotFlushMatchedSet(ZImgMatchedSet)),Qt::AutoConnection);
    QObject::connect(this->m_video->ZGetImgProcessThread(),SIGNAL(ZSigSSIMImgSimilarity(qint32)),this->m_ui,SLOT(ZSlotSSIMImgSimilarity(qint32)),Qt::AutoConnection);

    this->m_ui->showMaximized();

    //start key detect thread.
    gGblPara.m_mainUI=this->m_ui;
    this->m_keyDet=new ZKeyDetThread;
    this->m_keyDet->start();
    return 0;
}
void ZMainTask::ZSlotSubThreadsExited()
{
    if(!this->m_timerExit->isActive())
    {
        this->m_timerExit->start(1000);
    }
}
void ZMainTask::ZSlotChkAllExitFlags()
{
    qDebug()<<"<Waiting>:main task waiting working threads...";
    if(!this->m_tcp2Uart->ZIsExitCleanup())
    {
        qDebug()<<"<Exit>:wait for tcp2uart thread.";
        return;
    }
    //    if(!this->m_ctlJson->ZIsExitCleanup())
    //    {
    //        qDebug()<<"<Exit>:wait for ctlJson thread.";
    //        return;
    //    }
    if(!this->m_audio->ZIsExitCleanup())
    {
        qDebug()<<"<Exit>:wait for audio task.";
        return;
    }
    if(!this->m_video->ZIsExitCleanup())
    {
        qDebug()<<"<Exit>:wait for m_video task.";
        return;
    }

    this->m_timerExit->stop();
    qApp->exit(0);
}
