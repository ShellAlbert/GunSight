#ifndef ZNOISECUTTHREAD_H
#define ZNOISECUTTHREAD_H

#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QWaitCondition>
#include <audio/webrtc/signal_processing_library.h>
#include <audio/webrtc/noise_suppression_x.h>
#include <audio/webrtc/noise_suppression.h>
#include <audio/webrtc/gain_control.h>
//#include <audio/bevis/WindNSManager.h>
extern "C"
{
#include "audio/LogMMSE/type.h"
}
class ZNoiseCutThread : public QThread
{
    Q_OBJECT
public:
    ZNoiseCutThread();
    void ZBindInFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);
    void ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);
    void ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                   QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull);
    qint32 ZStartThread();
    qint32 ZStopThread();
    bool ZIsExitCleanup();
signals:
    void ZSigThreadFinished();
protected:
    void run();
private:
    bool m_bCleanup;

private:
    //RNNoise.
    //DenoiseState *m_st;
    qint32 ZCutNoiseByRNNoise(QByteArray *baPCM);

    //WebRTC.
    NsHandle *m_pNS_inst;
    qint32 ZCutNoiseByWebRTC(QByteArray *baPCM);

    //auto gain control.
    void *m_agcHandle;
    qint32 ZDGainByWebRTC(QByteArray *baPCM);
    //Bevis.
//    _WINDNSManager m_bevis;
    qint32 ZCutNoiseByBevis(QByteArray *baPCM);
    //logMMSE.
    LOGMMSE_VAR *m_logMMSE;
    X_INT16 *m_SigIn;
    X_INT16 *m_SigOut;
    X_FLOAT32 *m_OutBuf;
    qint32 ZCutNoiseByLogMMSE(QByteArray *baPCM);

    //for 48khz downsampling 16khz buffer.
    char *m_pcm16k;
private:
    short *m_pDataIn;
    short *m_pDataOut;

private:
    //in fifo.(capture -> noise cut)
    QQueue<QByteArray*> *m_freeQueueIn;
    QQueue<QByteArray*> *m_usedQueueIn;

    QMutex *m_mutexIn;
    QWaitCondition *m_condQueueEmptyIn;
    QWaitCondition *m_condQueueFullIn;

    //out1 fifo.(noise cut -> playback).
    QQueue<QByteArray*> *m_freeQueueOut1;
    QQueue<QByteArray*> *m_usedQueueOut1;

    QMutex *m_mutexOut1;
    QWaitCondition *m_condQueueEmptyOut1;
    QWaitCondition *m_condQueueFullOut1;

    //out2 fifo.(noise cut -> tx).
    QQueue<QByteArray*> *m_freeQueueOut2;
    QQueue<QByteArray*> *m_usedQueueOut2;

    QMutex *m_mutexOut2;
    QWaitCondition *m_condQueueEmptyOut2;
    QWaitCondition *m_condQueueFullOut2;

private:
    WebRtcSpl_State48khzTo16khz m_state4816;
    WebRtcSpl_State16khzTo48khz m_state1648;
};

#endif // ZNOISECUTTHREAD_H
