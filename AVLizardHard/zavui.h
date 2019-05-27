#ifndef ZAVUI_H
#define ZAVUI_H
#include <QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QPaintEvent>
#include <QImage>
#include <QtWidgets/QWidget>
#include <QLabel>
#include <QQueue>
#include <QByteArray>
#include <QSemaphore>
#include <QSlider>
#include <QProgressBar>
#include <video/zimgdisplayer.h>
#include "zgblpara.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QtCharts>
#include "xyseriesiodevice.h"
#include <zringbuffer.h>
class ZAVUI : public QFrame
{
    Q_OBJECT
public:
    ZAVUI(QWidget *parent=nullptr);
    ~ZAVUI();
    qint32 ZBindWaveFormQueueBefore(ZRingBuffer *rbWaveBefore);
    qint32 ZBindWaveFormQueueAfter(ZRingBuffer *rbWaveAfter);

    ZImgDisplayer *ZGetImgDisp(qint32 index);
private slots:
    void ZSlotNoiseViewApply();
    void ZSlotChangeDGain(qint32 val);
    void ZSlotSSIMImgSimilarity(qint32 nVal);
    void ZSlot1sTimeout();
public slots:
    void ZSlotFlushWaveBefore(const QByteArray &baPCM);
    void ZSlotFlushWaveAfter(const QByteArray &baPCM);
    void ZSlotFlushMatchedSet(const ZImgMatchedSet &imgProSet);
private:
    void ZUpdateMatchBar(QProgressBar *pBar,qint32 nVal);
protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
private:
    //像素坐标差值及算法消耗时间.
    QLabel *m_llDiffXY;
    //程序累计运行时间.
    QLabel *m_llRunSec;
    QTimer *m_timer;
    //算法当前状态，开启/关闭.
    QLabel *m_llState;
    //透传数据统计Android(tcp) <--> STM32(uart).
    QLabel *m_llForward;
    //音频客户端是否连接/视频客户端是否连接.
    QLabel *m_llAVClients;
    //layout.
    QVBoxLayout *m_vLayoutInfo;


    //the bottom layout.
    //disp0,progress bar,disp1.
    ZImgDisplayer *m_disp[2];
    QProgressBar *m_SSIMMatchBar;
    //DGain progress Bar.
    QProgressBar *m_barDGain;
    QSlider *m_sliderDGain;
    qint32 m_nDGainShadow;

    QLabel *m_llTxCnt;
    QHBoxLayout *m_hLayout;
    QVBoxLayout *m_vLayout;

    QVBoxLayout *m_vLayoutWavForm;
    QHBoxLayout *m_hLayoutWavFormAndInfo;
private:
    QLineSeries *m_seriesBefore;
    XYSeriesIODevice *m_deviceBefore;
    QChartView *m_chartViewBefore;
    QChart *m_chartBefore;

    //denoise view.
    QToolButton *m_tbNoiseView[7];
    QHBoxLayout *m_hLayoutNoiseView;

    QLineSeries *m_seriesAfter;
    XYSeriesIODevice *m_deviceAfter;
    QChartView *m_chartViewAfter;
    QChart *m_chartAfter;
private:
    //波形显示队列，降噪算法处理之前与处理之后波形比对.
    ZRingBuffer *m_rbWaveBefore;
    ZRingBuffer *m_rbWaveAfter;
};

#endif // ZAVUI_H
