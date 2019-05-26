#ifndef ZMAINTASK_H
#define ZMAINTASK_H

#include <QObject>
#include <QTimer>
#include <audio/zaudiotask.h>
#include <video/zvideotask.h>
#include <video/zkeydetthread.h>
#include <forward/ztcp2uartforwardthread.h>
#include <ctl/zctlthread.h>
#include <zavui.h>
#include <zringbuffer.h>
#include <QVector>
class ZMainTask:public QObject
{
    Q_OBJECT
public:
    ZMainTask();
    ~ZMainTask();
    qint32 ZStartTask();
private slots:
    void ZSlotSubThreadsFinished();
    void ZSlotChkAllExitFlags();
//    void ZSlotFwdImgMatchedSet2Ctl(const ZImgMatchedSet &set);
private:
    ZTcp2UartForwardThread *m_tcp2Uart;
    ZJsonThread *m_ctlJson;
    ZAudioTask *m_audio;
    ZVideoTask *m_video;
    ZAVUI *m_ui;
    ZKeyDetThread *m_keyDet;
private:
    QTimer *m_timerExit;
private:
//    QVector<ZImgMatchedSet> m_vecImgMatched;
};
#endif // ZMAINTASK_H
