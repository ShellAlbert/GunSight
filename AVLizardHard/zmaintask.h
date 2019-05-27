#ifndef ZMAINTASK_H
#define ZMAINTASK_H

#include <QObject>
#include <QTimer>
#include <audio/zaudiotask.h>
#include <video/zvideotask.h>
#include <forward/ztcp2uartforwardthread.h>
#include <ctl/zctlthread.h>
#include <zavui.h>
#include <zringbuffer.h>
#include <QVector>
#include <video/zkeydetthread.h>
class ZMainTask:public QObject
{
    Q_OBJECT
public:
    ZMainTask();
    ~ZMainTask();
    qint32 ZStartTask();
private slots:
    void ZSlotSubThreadsExited();
    void ZSlotChkAllExitFlags();
//    void ZSlotFwdImgMatchedSet2Ctl(const ZImgMatchedSet &set);
private:
    ZTcp2UartForwardThread *m_tcp2Uart;
    ZJsonThread *m_ctlJson;
    ZAudioTask *m_audio;
    ZVideoTask *m_video;
    ZAVUI *m_ui;
private:
    QTimer *m_timerExit;
    ZKeyDetThread *m_keyDet;
private:
//    QVector<ZImgMatchedSet> m_vecImgMatched;
};
#endif // ZMAINTASK_H
