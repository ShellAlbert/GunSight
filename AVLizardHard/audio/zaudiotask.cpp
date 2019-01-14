#include "zaudiotask.h"
#include "../zgblpara.h"
#include <QApplication>
ZAudioTask::ZAudioTask(QObject *parent):QObject(parent)
{
    //exit timer check.
    this->m_timerExit=NULL;

    //ring buffer.
    this->m_rbNoise=NULL;
    this->m_rbPlay=NULL;
    this->m_rbTx=NULL;

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
qint32 ZAudioTask::ZBindWaveFormQueueBefore(ZRingBuffer *rbWave)
{
    this->m_rbWaveBefore=rbWave;
    return 0;
}
qint32 ZAudioTask::ZBindWaveFormQueueAfter(ZRingBuffer *rbWave)
{
    this->m_rbWaveAfter=rbWave;
    return 0;
}
qint32 ZAudioTask::ZStartTask()
{
    this->m_timerExit=new QTimer;
    QObject::connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotTimeout()));

    //noise queue for capture thread -> noise cut thread.
    this->m_rbNoise=new ZRingBuffer(MAX_AUDIO_RING_BUFFER,BLOCK_SIZE);

    //play queue for noise cut thread -> play thread.
    this->m_rbPlay=new ZRingBuffer(MAX_AUDIO_RING_BUFFER,BLOCK_SIZE);

    //tx queue for noise cut thread -> tcp tx thread.
    this->m_rbTx=new ZRingBuffer(MAX_AUDIO_RING_BUFFER,BLOCK_SIZE);

    //Audio Capture --noise queue-->  Noise Cut --play queue--> Local Play.
    //                                          -- tx queue --> Tcp Tx.
    this->m_capThread=new ZAudioCaptureThread(gGblPara.m_audio.m_capCardName,false);
    QObject::connect(this->m_capThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //create noise cut thread.
    this->m_cutThread=new ZNoiseCutThread;
    this->m_cutThread->ZBindWaveFormQueue(this->m_rbWaveBefore,this->m_rbWaveAfter);
    QObject::connect(this->m_cutThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //create playback thread.
    this->m_playThread=new ZAudioPlayThread(gGblPara.m_audio.m_playCardName);
    QObject::connect(this->m_playThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //tcp tx thread.
    this->m_txThread=new ZAudioTxThread;
    QObject::connect(this->m_txThread,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotHelpThreads2Exit()));

    //start thread.
    this->m_capThread->ZStartThread(this->m_rbNoise);
    this->m_cutThread->ZStartThread(this->m_rbNoise,this->m_rbPlay,this->m_rbTx);
    this->m_playThread->ZStartThread(this->m_rbPlay);
    this->m_txThread->ZStartThread(this->m_rbTx);
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
        //tell other working threads to exit.
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
    //如果检测到全局请求退出标志，则清空所有队列，唤醒所有子线程。
    if(gGblPara.m_bGblRst2Exit)
    {
        //如果CapThread没有退出，可能noise queue队列满,则模拟NoiseCut线程出队一个音频帧，解除阻塞.
        if(!this->m_capThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for audio capture thread.";
            if(this->m_rbNoise->ZGetValidNum()==MAX_AUDIO_RING_BUFFER)
            {
                qint8 *pMuteData=new qint8[BLOCK_SIZE];
                if(this->m_rbNoise->ZGetElement(pMuteData,BLOCK_SIZE)<0)//出列数据无用，所以这里并不真取。
                {
                    qDebug()<<"<Exiting>:timeout to get data from noise queue.";
                }
                delete [] pMuteData;
                pMuteData=NULL;
            }
        }

        //如果NoiseCut没有退出，可能noise queue队列空，则模拟CapThread入队一个静音数据，解除阻塞。
        //也有可能是play queue满造成的阻塞，则模拟play thread取出一帧数据，解除阻塞。
        if(!this->m_cutThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for noise cut thread.";
            if(this->m_rbNoise->ZGetValidNum()==0)
            {
                qint8 *pMuteData=new qint8[BLOCK_SIZE];
                memset(pMuteData,0,BLOCK_SIZE);
                if(this->m_rbNoise->ZPutElement(pMuteData,BLOCK_SIZE)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put data to noise queue.";
                }
                delete  [] pMuteData;
                pMuteData=NULL;
            }
            if(this->m_rbPlay->ZGetValidNum()==MAX_AUDIO_RING_BUFFER)
            {
                qint8 *pMuteData=new qint8[BLOCK_SIZE];
                memset(pMuteData,0,BLOCK_SIZE);
                if(this->m_rbPlay->ZGetElement(pMuteData,BLOCK_SIZE)<0)//出列数据无用，所以这里并不真取。
                {
                    qDebug()<<"Exiting>:timeout to get data from play queue.";
                }
                delete  [] pMuteData;
                pMuteData=NULL;
            }
        }

        //如果play thread没有退出，可能play queue队列空，则模拟noise cut投入一个静音数据，解除阻塞。
        if(!this->m_playThread->ZIsExitCleanup())
        {
            qDebug()<<"<Exiting>:waiting for audio play thread.";
            if(this->m_rbPlay->ZGetValidNum()==0)
            {
                qint8 *pMuteData=new qint8[BLOCK_SIZE];
                memset(pMuteData,0,BLOCK_SIZE);
                if(this->m_rbPlay->ZPutElement(pMuteData,BLOCK_SIZE)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put data to play queue.";
                }
                delete  [] pMuteData;
                pMuteData=NULL;
            }
        }

        //如果TxThread没有退出，可能tx queue队列空，则模拟noise cut投入一个静音数据，解除阻塞。
        if(!this->m_txThread->ZIsExitCleanup())
        {
            qDebug()<<"wait for audio tx thread";
            qint8 *pMuteData=new qint8[BLOCK_SIZE];
            memset(pMuteData,0,BLOCK_SIZE);
            if(this->m_rbTx->ZPutElement(pMuteData,BLOCK_SIZE)<0)
            {
                qDebug()<<"<Exiting>:timeout to put data to tx queue.";
            }
            delete  [] pMuteData;
            pMuteData=NULL;
        }

        //当所有的子线程都退出时，则音频任务退出。
        if(this->ZIsExitCleanup())
        {
            this->m_timerExit->stop();
            emit this->ZSigAudioTaskExited();
        }
    }
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
