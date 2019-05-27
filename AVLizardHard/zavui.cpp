#include "zavui.h"
#include <QToolTip>
ZAVUI::ZAVUI(QWidget *parent):QFrame(parent)
{
    //未处理过的PCM数据曲线
    this->m_seriesBefore=new QLineSeries;
    this->m_chartBefore=new QChart;
    this->m_chartBefore->setTitle(tr("Original Waveform"));
    this->m_chartBefore->legend()->hide();
    this->m_chartBefore->addSeries(this->m_seriesBefore);
    this->m_chartBefore->createDefaultAxes();
    this->m_chartBefore->axisX()->setRange(0,2000);
    this->m_chartBefore->axisY()->setRange(-1,1);
    this->m_chartBefore->setTheme(QChart::ChartThemeBlueCerulean);

    this->m_chartViewBefore=new QChartView(this->m_chartBefore);
    this->m_chartViewBefore->setRenderHint(QPainter::Antialiasing);
    this->m_deviceBefore=new XYSeriesIODevice(this->m_seriesBefore,this);
    this->m_deviceBefore->open(QIODevice::WriteOnly);

    //经过降噪算法处理过后的PCM数据曲线
    this->m_seriesAfter=new QLineSeries;
    this->m_chartAfter=new QChart;
    this->m_chartAfter->setTitle(tr("Noise Suppression Waveform"));
    this->m_chartAfter->legend()->hide();
    this->m_chartAfter->addSeries(this->m_seriesAfter);
    this->m_chartAfter->createDefaultAxes();
    this->m_chartAfter->axisX()->setRange(0,2000);
    this->m_chartAfter->axisY()->setRange(-1,1);
    this->m_chartAfter->setTheme(QChart::ChartThemeBlueCerulean);

    this->m_chartViewAfter=new QChartView(this->m_chartAfter);
    this->m_chartViewAfter->setRenderHint(QPainter::Antialiasing);
    this->m_deviceAfter=new XYSeriesIODevice(this->m_seriesAfter,this);
    this->m_deviceAfter->open(QIODevice::WriteOnly);

    //noies view change buttons.
    this->m_hLayoutNoiseView=new QHBoxLayout;
    for(qint32 i=0;i<7;i++)
    {
        this->m_tbNoiseView[i]=new QToolButton;
        this->m_tbNoiseView[i]->setStyleSheet("QToolButton{background:#09AF60;font:20px;min-width:100px;}");
        switch(i)
        {
        case 0:
            this->m_tbNoiseView[i]->setText(tr("White"));
            break;
        case 1:
            this->m_tbNoiseView[i]->setText(tr("Pink"));
            break;
        case 2:
            this->m_tbNoiseView[i]->setText(tr("Babble"));
            break;
        case 3:
            this->m_tbNoiseView[i]->setText(tr("Vehicle"));
            break;
        case 4:
            this->m_tbNoiseView[i]->setText(tr("Machine"));
            break;
        case 5:
            this->m_tbNoiseView[i]->setText(tr("Current"));
            break;
        case 6:
            this->m_tbNoiseView[i]->setText(tr("Custom"));
            break;
        default:
            break;
        }
        this->m_hLayoutNoiseView->addWidget(this->m_tbNoiseView[i]);
        QObject::connect(this->m_tbNoiseView[i],SIGNAL(clicked(bool)),this,SLOT(ZSlotNoiseViewApply()));
    }

    //layout wave form.
    this->m_vLayoutWavForm=new QVBoxLayout;
    this->m_vLayoutWavForm->addWidget(this->m_chartViewBefore);
    this->m_vLayoutWavForm->addLayout(this->m_hLayoutNoiseView);
    this->m_vLayoutWavForm->addWidget(this->m_chartViewAfter);

    ///////////////////////////////////////////////
    this->m_llDiffXY=new QLabel;
    this->m_llDiffXY->setAlignment(Qt::AlignCenter);
    this->m_llDiffXY->setText(tr("Diff XYT\n[X:0 Y:0 T:0ms]"));
    this->m_llRunSec=new QLabel;
    this->m_llRunSec->setAlignment(Qt::AlignCenter);
    this->m_llRunSec->setText(tr("Accumulated\n[0H0M0S]"));
    this->m_llState=new QLabel;
    this->m_llState->setAlignment(Qt::AlignCenter);
    this->m_llState->setText(tr("ImgMatch\n[Pause]"));

    //透传数据统计Android(tcp) <--> STM32(uart).
    this->m_llForward=new QLabel;
    this->m_llForward->setAlignment(Qt::AlignCenter);
    this->m_llForward->setText(tr("Forward\n0/0"));
    //音频客户端是否连接/视频客户端是否连接/Tcp2Uart是否连接/控制端是否连接.
    this->m_llAVClients=new QLabel;
    this->m_llAVClients->setAlignment(Qt::AlignCenter);
    this->m_llAVClients->setText(tr("A:X V:X/X\nCTL:X FWD:X"));

    this->m_vLayoutInfo=new QVBoxLayout;
    this->m_vLayoutInfo->addWidget(this->m_llDiffXY);
    this->m_vLayoutInfo->addWidget(this->m_llRunSec);
    this->m_vLayoutInfo->addWidget(this->m_llState);
    this->m_vLayoutInfo->addWidget(this->m_llForward);
    this->m_vLayoutInfo->addWidget(this->m_llAVClients);

#if 1
    this->m_llDiffXY->setStyleSheet("QLabel{background:#FFAF60;font:20px;min-width:100px;}");
    this->m_llRunSec->setStyleSheet("QLabel{background:#9999CC;font:20px;min-width:100px;}");
    this->m_llState->setStyleSheet("QLabel{background:#AFAF61;font:20px;min-width:100px;}");
    this->m_llForward->setStyleSheet("QLabel{background:#0099CC;font:20px;min-width:100px;}");
    this->m_llAVClients->setStyleSheet("QLabel{background:#23AF61;font:20px;min-width:100px;}");
#endif

    //top horizontal layout.
    this->m_hLayoutWavFormAndInfo=new QHBoxLayout;
    this->m_hLayoutWavFormAndInfo->addLayout(this->m_vLayoutWavForm);
    this->m_hLayoutWavFormAndInfo->addLayout(this->m_vLayoutInfo);


    this->m_disp[0]=new ZImgDisplayer(gGblPara.m_calibrateX1,gGblPara.m_calibrateY1,true);//the main camera.
    this->m_disp[1]=new ZImgDisplayer(gGblPara.m_calibrateX2,gGblPara.m_calibrateY2);
    this->m_disp[0]->ZSetPaintParameters(QColor(0,255,0));
    this->m_disp[1]->ZSetPaintParameters(QColor(255,0,0));

    this->m_SSIMMatchBar=new QProgressBar;
    this->m_SSIMMatchBar->setOrientation(Qt::Vertical);
    this->m_SSIMMatchBar->setValue(0);
    this->m_SSIMMatchBar->setRange(0,100);
    this->m_SSIMMatchBar->setStyleSheet("QProgressBar::chunk{background-color:#FF0000}");

    this->m_sliderDGain=new QSlider(Qt::Vertical);
    this->m_sliderDGain->setRange(0,90);
    this->m_sliderDGain->setValue(0);
    this->m_sliderDGain->setTickInterval(1);
    QObject::connect(this->m_sliderDGain,SIGNAL(valueChanged(int)),this,SLOT(ZSlotChangeDGain(qint32)));

    this->m_barDGain=new QProgressBar;
    this->m_barDGain->setOrientation(Qt::Vertical);
    this->m_barDGain->setValue(0);
    this->m_barDGain->setRange(0,90);
    this->m_barDGain->setStyleSheet("QProgressBar::chunk{background-color:#00FF00}");
    this->m_nDGainShadow=0;

    this->m_hLayout=new QHBoxLayout;
    this->m_hLayout->addWidget(this->m_disp[0]);
    this->m_hLayout->addWidget(this->m_barDGain);
    this->m_hLayout->addWidget(this->m_sliderDGain);
    this->m_hLayout->addWidget(this->m_SSIMMatchBar);
    this->m_hLayout->addWidget(this->m_disp[1]);

    this->m_llTxCnt=new QLabel;

    //main layout.
    this->m_vLayout=new QVBoxLayout;
    this->m_vLayout->addWidget(this->m_llTxCnt);
    this->m_vLayout->addLayout(this->m_hLayoutWavFormAndInfo);
    this->m_vLayout->addLayout(this->m_hLayout);
    this->setLayout(this->m_vLayout);

    this->setStyleSheet("QFrame{background-color:#84C1FF;}");

    //accumulated running time.
    this->m_timer=new QTimer;
    QObject::connect(this->m_timer,SIGNAL(timeout()),this,SLOT(ZSlot1sTimeout()));
    this->m_timer->start(1000);
}
ZAVUI::~ZAVUI()
{
    this->m_timer->stop();
    delete this->m_timer;
    //wav form.
    delete this->m_seriesBefore;
    delete this->m_deviceBefore;
    delete this->m_chartBefore;
    delete this->m_chartViewBefore;

    delete this->m_seriesAfter;
    delete this->m_deviceAfter;
    delete this->m_chartAfter;
    delete this->m_chartViewAfter;

    for(qint32 i=0;i<7;i++)
    {
        delete this->m_tbNoiseView[i];
    }
    delete this->m_hLayoutNoiseView;
    delete this->m_vLayoutWavForm;

    //info.
    delete this->m_llDiffXY;
    delete this->m_llRunSec;
    delete this->m_llState;
    delete this->m_llForward;
    delete this->m_llAVClients;
    delete this->m_vLayoutInfo;

    delete this->m_hLayoutWavFormAndInfo;

    delete this->m_sliderDGain;
    delete this->m_barDGain;
    delete this->m_SSIMMatchBar;
    delete this->m_disp[0];
    delete this->m_disp[1];
    delete this->m_hLayout;
    delete this->m_llTxCnt;
    delete this->m_vLayout;
}
void ZAVUI::closeEvent(QCloseEvent *event)
{
    //set global request exit flag.
    gGblPara.m_bGblRst2Exit=true;
    QFrame::closeEvent(event);
}
void ZAVUI::mousePressEvent(QMouseEvent *event)
{
    this->m_chartAfter->setTheme(QChart::ChartThemeBlueCerulean);
    this->m_chartAfter->setTitle("Denosie OFF");
    gGblPara.m_audio.m_nDeNoiseMethod=0;//DeNoise Off.
    for(qint32 i=0;i<7;i++)
    {
        this->m_tbNoiseView[i]->setEnabled(true);
    }
    QFrame::mousePressEvent(event);
}
void ZAVUI::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_F1)
    {
        gGblPara.m_bJsonImgPro=!gGblPara.m_bJsonImgPro;
        if(gGblPara.m_bJsonImgPro)
        {
            qDebug()<<"keyPress to on ImgPro";
        }else{
            qDebug()<<"keyPress to off ImgPro";
        }
    }
    QWidget::keyPressEvent(event);
}
void ZAVUI::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_F1)
    {
        qDebug()<<"keyRelease";
    }
    QWidget::keyReleaseEvent(event);
}
qint32 ZAVUI::ZBindWaveFormQueueBefore(ZRingBuffer *rbWaveBefore)
{
    this->m_rbWaveBefore=rbWaveBefore;
    return 0;
}
qint32 ZAVUI::ZBindWaveFormQueueAfter(ZRingBuffer *rbWaveAfter)
{
    this->m_rbWaveAfter=rbWaveAfter;
    return 0;
}
ZImgDisplayer* ZAVUI::ZGetImgDisp(qint32 index)
{
    return this->m_disp[index];
}
void ZAVUI::ZSlotSSIMImgSimilarity(qint32 nVal)
{
    this->ZUpdateMatchBar(this->m_SSIMMatchBar,nVal);
}
void ZAVUI::ZSlot1sTimeout()
{

    this->m_llTxCnt->setText(tr("6803:%1,6805:%2").arg(gGblPara.m_nTx6803Cnt).arg(gGblPara.m_nTx6805Cnt));

    //accumulated run seocnds.
    gGblPara.m_nAccumulatedSec++;

    //convert sec to h/m/s.
    qint32 nReaningSec=gGblPara.m_nAccumulatedSec;
    qint32 nHour,nMinute,nSecond;
    nHour=nReaningSec/3600;
    nReaningSec=nReaningSec%3600;

    nMinute=nReaningSec/60;
    nReaningSec=nReaningSec%60;

    nSecond=nReaningSec;

    this->m_llRunSec->setText(tr("Accumulated\n%1H%2M%3S").arg(nHour).arg(nMinute).arg(nSecond));

    //ImgPro on/off.
    if(gGblPara.m_bJsonImgPro)
    {
        this->m_llState->setText(tr("ImgMatch\n[Running]"));
    }else{
        this->m_llState->setText(tr("ImgMatch\n[Pause]"));
    }

    //update the andorid(tcp) <--> STM32(uart) bytes.
    this->m_llForward->setText(tr("Forward\n%1/%2").arg(gGblPara.m_nTcp2UartBytes).arg(gGblPara.m_nUart2TcpBytes));

    //update the connected flags.
    QString audioClient("X");
    if(gGblPara.m_audio.m_bAudioTcpConnected)
    {
        audioClient="V";
    }

    QString videoClient("X");
    if(gGblPara.m_bVideoTcpConnected)
    {
        videoClient="V";
    }
    QString videoClient2("X");
    if(gGblPara.m_bVideoTcpConnected2)
    {
        videoClient2="V";
    }

    QString ctlClient("X");
    if(gGblPara.m_bCtlClientConnected)
    {
        ctlClient="V";
    }

    QString forwardClient("X");
    if(gGblPara.m_bTcp2UartConnected)
    {
        forwardClient="V";
    }

    this->m_llAVClients->setText(tr("A:%1 V:%2/%3\nCTL:%4 FWD:%5").arg(audioClient).arg(videoClient).arg(videoClient2).arg(ctlClient).arg(forwardClient));

    //update the DGain bar.
    if(this->m_nDGainShadow!=gGblPara.m_audio.m_nGaindB)
    {
        this->m_sliderDGain->setValue(gGblPara.m_audio.m_nGaindB);
        this->m_barDGain->setValue(gGblPara.m_audio.m_nGaindB);
        this->m_nDGainShadow=gGblPara.m_audio.m_nGaindB;
    }

    //if RNNoise enabled.
    if(gGblPara.m_audio.m_nDeNoiseMethod)
    {
        switch(gGblPara.m_audio.m_nRNNoiseView)
        {
        case 0:
            this->m_chartAfter->setTitle("Denosie White Noise");
            break;
        case 1:
            this->m_chartAfter->setTitle("Denosie Pink Noise");
            break;
        case 2:
            this->m_chartAfter->setTitle("Denosie Babble Noise");
            break;
        case 3:
            this->m_chartAfter->setTitle("Denosie Vehicle Noise");
            break;
        case 4:
            this->m_chartAfter->setTitle("Denosie Machine Noise");
            break;
        case 5:
            this->m_chartAfter->setTitle("Denosie Current Noise");
            break;
        case 6:
            this->m_chartAfter->setTitle("Denosie Custom Noise");
            break;
        default:
            break;
        }
        this->m_chartAfter->setTheme(QChart::ChartThemeBlueIcy);
        for(qint32 i=0;i<7;i++)
        {
            this->m_tbNoiseView[i]->setEnabled(false);
        }
    }else{
        this->m_chartAfter->setTheme(QChart::ChartThemeBlueCerulean);
        this->m_chartAfter->setTitle("Denosie OFF");
        gGblPara.m_audio.m_nDeNoiseMethod=0;//DeNoise Off.
        for(qint32 i=0;i<7;i++)
        {
            this->m_tbNoiseView[i]->setEnabled(true);
        }
    }
}
void ZAVUI::ZUpdateMatchBar(QProgressBar *pBar,qint32 nVal)
{
    pBar->setValue(nVal);
    if(nVal<=100 && nVal>80)
    {
        pBar->setStyleSheet("QProgressBar::chunk{ background-color:#00FF00}");
    }else if(nVal<=80 && nVal>60)
    {
        pBar->setStyleSheet("QProgressBar::chunk{ background-color:#00EE76}");
    }else if(nVal<=60 && nVal>40)
    {
        pBar->setStyleSheet("QProgressBar::chunk{ background-color:#43CD80}");
    }else if(nVal<=40 && nVal>20)
    {
        pBar->setStyleSheet("QProgressBar::chunk{ background-color:#FFAEB9}");
    }else if(nVal<=20 && nVal>=0)
    {
        pBar->setStyleSheet("QProgressBar::chunk{ background-color:#FF0000}");
    }
}
void ZAVUI::ZSlotFlushWaveBefore(const QByteArray &baPCM)
{
    this->m_deviceBefore->write(baPCM);
}
void ZAVUI::ZSlotFlushWaveAfter(const QByteArray &baPCM)
{
    this->m_deviceAfter->write(baPCM);
}
void ZAVUI::ZSlotFlushMatchedSet(const ZImgMatchedSet &imgProSet)
{
    this->m_disp[0]->ZSetSensitiveRect(imgProSet.rectTemplate);
    this->m_disp[1]->ZSetSensitiveRect(imgProSet.rectMatched);
    this->m_llDiffXY->setText(tr("Diff XYT\n[X:%1 Y:%2 T:%3ms]").arg(imgProSet.nDiffX).arg(imgProSet.nDiffY).arg(imgProSet.nCostMs));
}
void ZAVUI::ZSlotChangeDGain(qint32 val)
{
    gGblPara.m_audio.m_nGaindB=val;
}
void ZAVUI::ZSlotNoiseViewApply()
{
    QToolButton *rbSender=qobject_cast<QToolButton*>(this->sender());
    if(rbSender==this->m_tbNoiseView[0])//Cut White Noise.
    {
        this->m_chartAfter->setTitle("Denosie White Noise");

        gGblPara.m_audio.m_nRNNoiseView=0;//Enable White Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[1])//Cut Pink Noise.
    {
        this->m_chartAfter->setTitle("Denosie Pink Noise");

        gGblPara.m_audio.m_nRNNoiseView=1;//Enable Pink Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[2])//Cut Babble Noise.
    {
        this->m_chartAfter->setTitle("Denosie Babble Noise");

        gGblPara.m_audio.m_nRNNoiseView=2;//Enable Babble Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[3])//Cut Vehicle Noise Cut.
    {
        this->m_chartAfter->setTitle("Denosie Vehicle Noise");

        gGblPara.m_audio.m_nRNNoiseView=3;//Enable Vehicle Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[4])//Cut Machine Noise.
    {
        this->m_chartAfter->setTitle("Denosie Machine Noise");

        gGblPara.m_audio.m_nRNNoiseView=4;//Enable Machine Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[5])//Cut Current Noise.
    {
        this->m_chartAfter->setTitle("Denosie Current Noise");

        gGblPara.m_audio.m_nRNNoiseView=5;//Enable Current Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }else if(rbSender==this->m_tbNoiseView[6])//Cut Custom Noise.
    {
        this->m_chartAfter->setTitle("Denosie Custom Noise");

        gGblPara.m_audio.m_nRNNoiseView=6;//Enable Custom Noise Cut.
        gGblPara.m_audio.m_nDeNoiseMethod=1;//Enable RNNoise Method.
    }
    this->m_chartAfter->setTheme(QChart::ChartThemeBlueIcy);
    for(qint32 i=0;i<7;i++)
    {
        this->m_tbNoiseView[i]->setEnabled(false);
    }
}
