#ifndef ZNOISECUTTHREAD_H
#define ZNOISECUTTHREAD_H

#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <zringbuffer.h>
extern "C"
{
#include <rnnoise.h>
}
#include <audio/webrtc/signal_processing_library.h>
#include <audio/webrtc/noise_suppression_x.h>
#include <audio/webrtc/noise_suppression.h>
#include <audio/webrtc/gain_control.h>
//#include <audio/bevis/WindNSManager.h>
class ZNoiseCutThread : public QThread
{
    Q_OBJECT
public:
    ZNoiseCutThread();
    qint32 ZBindWaveFormQueue(ZRingBuffer *rbWaveBefore,ZRingBuffer *rbWaveAfter);
    qint32 ZStartThread(ZRingBuffer *m_rbNoise,ZRingBuffer *m_rbPlay,ZRingBuffer *m_rbTx);
    qint32 ZStopThread();
    bool ZIsExitCleanup();
signals:
    void ZSigThreadFinished();
    void ZSigNewWaveBeforeArrived(const QByteArray &baPCM);
    void ZSigNewWaveAfterArrived(const QByteArray &baPCM);
protected:
    void run();
private:
    ZRingBuffer *m_rbNoise;
    ZRingBuffer *m_rbPlay;
    ZRingBuffer *m_rbTx;
private:
    //波形显示队列，降噪算法处理之前与处理之后波形比对
    ZRingBuffer *m_rbWaveBefore;
    ZRingBuffer *m_rbWaveAfter;
private:
    bool m_bCleanup;

private:
    //RNNoise.
    DenoiseState *m_st;
    qint32 ZCutNoiseByRNNoise(QByteArray &baPCM);

    //WebRTC.
    NsHandle *m_pNS_inst;
    qint32 ZCutNoiseByWebRTC(QByteArray &baPCM);

    //auto gain control.
    void *m_agcHandle;
    qint32 ZDGainByWebRTC(QByteArray &baPCM);
    //Bevis.
//    _WINDNSManager m_bevis;
    qint32 ZCutNoiseByBevis(QByteArray &baPCM);
private:
    short *m_pDataIn;
    short *m_pDataOut;
};

#endif // ZNOISECUTTHREAD_H
