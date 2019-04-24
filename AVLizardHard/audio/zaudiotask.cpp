#include "zaudiotask.h"
#include "../zgblpara.h"
#include <QApplication>
ZAudioTask::ZAudioTask(QObject *parent):QObject(parent)
{
    //exit timer check.
    this->m_timerExit=NULL;

    //capture-process-playback/tcp tx threads.
    this->m_capThread=NULL;
    this->m_cutThread=NULL;
    this->m_playThread=NULL;
    this->m_txThread=NULL;
}

ZAudioTask::~ZAudioTask()
{
    delete this->m_timerExit;

    //capture-process-playback mode.
    if(this->m_capThread!=NULL)
    {
        delete this->m_capThread;
    }
    if(this->m_cutThread!=NULL)
    {
        delete this->m_cutThread;
    }
    if(this->m_playThread!=NULL)
    {
        delete this->m_playThread;
    }
    if(this->m_txThread!=NULL)
    {
        delete this->m_txThread;
    }
}
qint32 ZAudioTask::ZStartTask()
{
    this->m_timerExit=new QTimer;
    QObject::connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotTimeout()));

    //create FIFOs.
    //capture to noise queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2NsFIFO[i]=new QByteArray;
        this->m_Cap2NsFIFO[i]->resize(PERIOD_SIZE);
        this->m_Cap2NsFIFOFree.enqueue(this->m_Cap2NsFIFO[i]);
    }
    //noise to playback fifo.
    for(qint32 i=0;i<5;i++)
    {
        this->m_Ns2PbFIFO[i]=new QByteArray;
        this->m_Ns2PbFIFO[i]->resize(PERIOD_SIZE);
        this->m_Ns2PbFIFOFree.enqueue(this->m_Ns2PbFIFO[i]);
    }
    //noise to tx fifo.
    for(qint32 i=0;i<5;i++)
    {
        this->m_Ns2TxFIFO[i]=new QByteArray;
        this->m_Ns2TxFIFO[i]->resize(PERIOD_SIZE);
        this->m_Ns2TxFIFOFree.enqueue(this->m_Ns2TxFIFO[i]);
    }

    //Audio Capture --noise queue-->  Noise Cut --play queue--> Local Play.
    //                                          -- tx queue --> Tcp Tx.
    this->m_capThread=new ZAudioCaptureThread(gGblPara.m_audio.m_capCardName,false);
    this->m_capThread->ZBindFIFO(&this->m_Cap2NsFIFOFree,&this->m_Cap2NsFIFOUsed,&this->m_Cap2NsFIFOMutex,&this->m_condCap2NsFIFOEmpty,&this->m_condCap2NsFIFOFull);
    QObject::connect(this->m_capThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //create noise cut thread.
    this->m_cutThread=new ZNoiseCutThread;
    this->m_cutThread->ZBindInFIFO(&this->m_Cap2NsFIFOFree,&this->m_Cap2NsFIFOUsed,&this->m_Cap2NsFIFOMutex,&this->m_condCap2NsFIFOEmpty,&this->m_condCap2NsFIFOFull);
    this->m_cutThread->ZBindOut1FIFO(&this->m_Ns2PbFIFOFree,&this->m_Ns2PbFIFOUsed,&this->m_Ns2PbFIFOMutex,&this->m_condNs2PbFIFOEmpty,&this->m_condNs2PbFIFOFull);
    this->m_cutThread->ZBindOut2FIFO(&this->m_Ns2TxFIFOFree,&this->m_Ns2TxFIFOUsed,&this->m_Ns2TxFIFOMutex,&this->m_condNs2TxFIFOEmpty,&this->m_condNs2TxFIFOFull);
    QObject::connect(this->m_cutThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //create playback thread.
    this->m_playThread=new ZAudioPlayThread(gGblPara.m_audio.m_playCardName);
    this->m_playThread->ZBindFIFO(&this->m_Ns2PbFIFOFree,&this->m_Ns2PbFIFOUsed,&this->m_Ns2PbFIFOMutex,&this->m_condNs2PbFIFOEmpty,&this->m_condNs2PbFIFOFull);
    QObject::connect(this->m_playThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //tcp tx thread.
    this->m_txThread=new ZAudioTxThread;
    this->m_txThread->ZBindFIFO(&this->m_Ns2TxFIFOFree,&this->m_Ns2TxFIFOUsed,&this->m_Ns2TxFIFOMutex,&this->m_condNs2TxFIFOEmpty,&this->m_condNs2TxFIFOFull);
    QObject::connect(this->m_txThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //start thread.
    this->m_capThread->ZStartThread();
    this->m_cutThread->ZStartThread();
    this->m_playThread->ZStartThread();
    this->m_txThread->ZStartThread();
    return 0;
}
ZNoiseCutThread* ZAudioTask::ZGetNoiseCutThread()
{
    return this->m_cutThread;
}
void ZAudioTask::ZSlotHelpThreads2Exit()
{
    if(!this->m_timerExit->isActive())
    {
        //tell other working threads to exit event-loop exec().
        this->m_capThread->exit(0);
        this->m_cutThread->exit(0);
        this->m_playThread->exit(0);
        this->m_txThread->exit(0);

        //start check timer.
        this->m_timerExit->start(1000);
    }
}
void ZAudioTask::ZSlotTimeout()
{
#if 1
    if(gGblPara.m_bGblRst2Exit)
    {
        //if CapThread doesnot exit,maybe Cap2NsFreeQueue is empty to cause thread blocks.
        //here we move a element from Cap2NsUsedQueue to Cap2NsFreeQueue to unblock.
        if(!this->m_capThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for AudioCaptureThread.";
            this->m_Cap2NsFIFOMutex.lock();
            if(!this->m_Cap2NsFIFOUsed.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2NsFIFOUsed.dequeue();
                this->m_Cap2NsFIFOFree.enqueue(elementHelp);
                this->m_condCap2NsFIFOEmpty.wakeAll();
            }
            this->m_Cap2NsFIFOMutex.unlock();
        }

        //if NoiseCutThread doesnot exit,maybe Cap2NsUsedQueue is empty to cause thread blocks,
        //or Ns2PbFreeQueue is empty to cause thread blocks,
        //or Ns2TxFreeQueue is empty to cause thread blocks.
        //here we move a element from Cap2NsFreeQueue to Cap2NsUsedQueue to unblock.
        //here we move a element from Ns2PbUsedQueue to Ns2PbFreeQueue to unblock.
        //here we move a element from Ns2TxUsedQueue to Ns2TxFreeQueue to unblock.
        if(!this->m_cutThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for NoiseCutThread.";
            this->m_Cap2NsFIFOMutex.lock();
            if(!this->m_Cap2NsFIFOFree.isEmpty())
            {
                QByteArray *elementHelp=this->m_Cap2NsFIFOFree.dequeue();
                this->m_Cap2NsFIFOUsed.enqueue(elementHelp);
                this->m_condCap2NsFIFOFull.wakeAll();
            }
            this->m_Cap2NsFIFOMutex.unlock();

            this->m_Ns2PbFIFOMutex.lock();
            if(!this->m_Ns2PbFIFOUsed.isEmpty())
            {
                QByteArray *elementHelp=this->m_Ns2PbFIFOUsed.dequeue();
                this->m_Ns2PbFIFOFree.enqueue(elementHelp);
                this->m_condNs2PbFIFOEmpty.wakeAll();
            }
            this->m_Ns2PbFIFOMutex.unlock();

            this->m_Ns2TxFIFOMutex.lock();
            if(!this->m_Ns2TxFIFOUsed.isEmpty())
            {
                QByteArray *elementHelp=this->m_Ns2TxFIFOUsed.dequeue();
                this->m_Ns2TxFIFOFree.enqueue(elementHelp);
                this->m_condNs2TxFIFOEmpty.wakeAll();
            }
            this->m_Ns2TxFIFOMutex.unlock();
        }

        //if playThread doesnot exit,maybe Ns2PbUsedQueue is empty to cause thread blocks.
        //here we move a element from Ns2PbFreeQueue to Ns2PbUsedQueue to unblock.
        if(!this->m_playThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for AudioPlayThread.";
            this->m_Ns2PbFIFOMutex.lock();
            if(!this->m_Ns2PbFIFOFree.isEmpty())
            {
                QByteArray *elementHelp=this->m_Ns2PbFIFOFree.dequeue();
                this->m_Ns2PbFIFOUsed.enqueue(elementHelp);
                this->m_condNs2PbFIFOFull.wakeAll();
            }
            this->m_Ns2PbFIFOMutex.unlock();
        }

        //if TxThread doesnot exit,maybe Ns2TxUsedQueue is empty to cause thread blocks.
        //here we move a element from Ns2TxFreeQueue to Ns2TxUsedQueue to unblock.
        if(!this->m_txThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for AudioTxThread.";
            this->m_Ns2TxFIFOMutex.lock();
            if(!this->m_Ns2TxFIFOFree.isEmpty())
            {
                QByteArray *elementHelp=this->m_Ns2TxFIFOFree.dequeue();
                this->m_Ns2TxFIFOUsed.enqueue(elementHelp);
                this->m_condNs2TxFIFOFull.wakeAll();
            }
            this->m_Ns2TxFIFOMutex.unlock();
        }

        //当所有的子线程都退出时，则音频任务退出。
        if(this->ZIsExitCleanup())
        {
            this->m_timerExit->stop();
            emit this->ZSigAudioTaskExited();
        }
    }
#endif
}
bool ZAudioTask::ZIsExitCleanup()
{
    bool bAllCleanup=true;
    if(!this->m_capThread->ZIsExitCleanup())
    {
        bAllCleanup=false;
    }
    if(!this->m_cutThread->ZIsExitCleanup())
    {
        bAllCleanup=false;
    }
    if(!this->m_playThread->ZIsExitCleanup())
    {
        bAllCleanup=false;
    }
    if(!this->m_txThread->ZIsExitCleanup())
    {
        bAllCleanup=false;
    }
    return bAllCleanup;
}
