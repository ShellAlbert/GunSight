#ifndef ZAUDIOTASK_H
#define ZAUDIOTASK_H
#include "../zgblpara.h"
#include <QWidget>
#include <audio/zaudiocapturethread.h>
#include <audio/znoisecutthread.h>
#include <audio/zaudioplaythread.h>
#include <audio/zaudiotxthread.h>
#include <QTimer>

class ZAudioTask : public QObject
{
    Q_OBJECT
public:
     ZAudioTask(QObject *parent = 0);
    ~ZAudioTask();
     qint32 ZStartTask();

     ZNoiseCutThread* ZGetNoiseCutThread();
     bool ZIsExitCleanup();
signals:
     void ZSigAudioTaskExited();
private slots:
    void ZSlotHelpThreads2Exit();
    void ZSlotTimeout();
private:
    //Audio Capture --noise queue-->  Noise Cut --play queue--> Local Play.
    //                                          -- tx queue --> Tcp Tx.
    ZAudioCaptureThread *m_capThread;
    ZNoiseCutThread *m_cutThread;
    ZAudioPlayThread *m_playThread;
    ZAudioTxThread *m_txThread;
private:
    QTimer *m_timerExit;
private:
    //capture to noise queue(fifo).
    QByteArray* m_Cap2NsFIFO[5];
    QQueue<QByteArray*> m_Cap2NsFIFOFree;
    QQueue<QByteArray*> m_Cap2NsFIFOUsed;
    QMutex m_Cap2NsFIFOMutex;
    QWaitCondition m_condCap2NsFIFOFull;
    QWaitCondition m_condCap2NsFIFOEmpty;
    //noise to playback fifo.
    QByteArray* m_Ns2PbFIFO[5];
    QQueue<QByteArray*> m_Ns2PbFIFOFree;
    QQueue<QByteArray*> m_Ns2PbFIFOUsed;
    QMutex m_Ns2PbFIFOMutex;
    QWaitCondition m_condNs2PbFIFOFull;
    QWaitCondition m_condNs2PbFIFOEmpty;
    //noise to tx fifo.
    QByteArray* m_Ns2TxFIFO[5];
    QQueue<QByteArray*> m_Ns2TxFIFOFree;
    QQueue<QByteArray*> m_Ns2TxFIFOUsed;
    QMutex m_Ns2TxFIFOMutex;
    QWaitCondition m_condNs2TxFIFOFull;
    QWaitCondition m_condNs2TxFIFOEmpty;
};

#endif // ZAUDIOTASK_H
