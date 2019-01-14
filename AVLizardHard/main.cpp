#include "zgblpara.h"
#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <signal.h>
#include <QMetaType>
#include "zmaintask.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

Q_DECLARE_METATYPE(ZImgMatchedSet)

void gSIGHandler(int sigNo)
{
    switch(sigNo)
    {
    case SIGINT:
    case SIGKILL:
    case SIGTERM:
        qDebug()<<"prepare to exit...";
        gGblPara.m_bGblRst2Exit=true;
        break;
    default:
        break;
    }
}


//AppName: AVLizard
//capture audio with ALSA,encode with opus.
//capture video with V4L2,encode with h264.

//RK3399,2 big cores,4 small cores.
//core 0,big:
//core 1,big: ImgProc.
//core 2,small:MainCamCap,AudioCap,NoiseCut,
//core 3,small:AuxCamCap,AudioPlay,AudioTx.
//core 4,small:MainVideoTx,
//core 5,small:AuxVideoTx,
int main(int argc,char **argv)
{
    //运行时先使用--DoNotCmpCamId来跳开USB CamId比对代码
    //这样就会在运行目录下生成cam info文件，从里面找出usb bus info信息
    //将这个bus info字符串复制到AVLizard.ini的id中，用于区别主从摄像头
    //usb-fe3c0000.usb-1
    //usb-fe380000.usb-1

    qRegisterMetaType<ZImgMatchedSet>("ZImgMatchedSet");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    QApplication app(argc,argv);
    //parse audio command line arguments.
    QCommandLineOption opMode("mode","0:capture to file,1:realtime process,default is 1.","runMode","1");
    QCommandLineOption opInRate("inRate","specified the capture input sample rate,default is 48000.","inRate","48000");
    QCommandLineOption opOutRate("outRate","specified the playback output sample rate,default is 48000.","outRate","48000");
    QCommandLineOption opChannels("channels","specified the channel number for Capture & Playback,1 for Mono,2 for Stereo,default is 2.","channels","2");
    QCommandLineOption opDeNoise("denoise","de-noise ctrl,0:off,1:RNNoise,2:WebRTC,3:Bevis,default is 0.","denoise","0");
    QCommandLineOption opGaindB("gaindB","compression gain dB range [0,90],default is 0.","gaindB","0");
    QCommandLineOption opBevisGrade("grade","Bevis:noise reduction grade,range:1~4,default is 1.","grade","1");
    //parse video command line arguments.
    QCommandLineOption opDebug("debug","enable debug mode,output more messages.");
    QCommandLineOption opDoNotCmpCamId("DoNotCmpCamId","do not compare camera IDs,used to get cam info at first time run.");
    QCommandLineOption opDumpCamInfo("camInfo","dump camera parameters to file then exit.");
    QCommandLineOption opCapture("capLog","print capture logs.");
    QCommandLineOption opTransfer("tx2PC","enable transfer h264 stream to PC.");
    QCommandLineOption opSpeed("speed","monitor transfer speed.");//-s transfer speed monitor.
    QCommandLineOption opUART("diffXYT","ouput diff XYT to UART.(x,y) and cost time.");//-y hex uart data.
    QCommandLineOption opXMode("xMode","enable xMode to matchTemplate.");//-x xMode,resize image to 1/2 before matchTemplate.
    QCommandLineOption opFMode("fMode","enable fMode to matchTemplate.");//-f fMode,use feature extractor.
    ///////////////////////////////////////////////////////////////
    QCommandLineOption opHelp("help","print this help message.");//-h help.
    QCommandLineParser cmdLineParser;
    //audio.
    cmdLineParser.addOption(opMode);
    cmdLineParser.addOption(opInRate);
    cmdLineParser.addOption(opOutRate);
    cmdLineParser.addOption(opChannels);
    cmdLineParser.addOption(opDeNoise);
    cmdLineParser.addOption(opGaindB);
    cmdLineParser.addOption(opBevisGrade);
    //video.
    cmdLineParser.addOption(opDebug);
    cmdLineParser.addOption(opDoNotCmpCamId);
    cmdLineParser.addOption(opDumpCamInfo);
    cmdLineParser.addOption(opCapture);
    cmdLineParser.addOption(opTransfer);
    cmdLineParser.addOption(opSpeed);
    cmdLineParser.addOption(opUART);
    cmdLineParser.addOption(opXMode);
    cmdLineParser.addOption(opFMode);
    cmdLineParser.addOption(opHelp);
    cmdLineParser.process(app);

    /////////////////////////////
    if(cmdLineParser.isSet(opHelp))
    {
        cmdLineParser.showHelp(0);
        return 0;
    }
    //audio process.
    if(cmdLineParser.isSet(opMode))
    {
        gGblPara.m_audio.m_runMode=cmdLineParser.value("mode").toInt();
    }
    if(cmdLineParser.isSet(opInRate))
    {
        gGblPara.m_audio.m_nCaptureSampleRate=cmdLineParser.value("inRate").toInt();
    }
    if(cmdLineParser.isSet(opOutRate))
    {
        gGblPara.m_audio.m_nPlaybackSampleRate=cmdLineParser.value("outRate").toInt();
    }
    if(cmdLineParser.isSet(opChannels))
    {
        gGblPara.m_audio.m_nChannelNum=cmdLineParser.value("channels").toInt();
    }
    if(cmdLineParser.isSet(opDeNoise))
    {
        gGblPara.m_audio.m_nDeNoiseMethod=cmdLineParser.value("denoise").toInt();
    }
    if(cmdLineParser.isSet(opGaindB))
    {
        gGblPara.m_audio.m_nGaindB=cmdLineParser.value("gaindB").toInt();
    }
    if(cmdLineParser.isSet(opBevisGrade))
    {
        gGblPara.m_audio.m_nBevisGrade=cmdLineParser.value("grade").toInt();
    }

    //video process.
    if(cmdLineParser.isSet(opDebug))
    {
        gGblPara.m_bDebugMode=true;
    }
    if(cmdLineParser.isSet(opDoNotCmpCamId))
    {
        gGblPara.m_video.m_bDoNotCmpCamId=true;
    }
    if(cmdLineParser.isSet(opDumpCamInfo))
    {
        gGblPara.m_bDumpCamInfo2File=true;
    }
    if(cmdLineParser.isSet(opCapture))
    {
        gGblPara.m_bCaptureLog=true;
    }
    if(cmdLineParser.isSet(opSpeed))
    {
        gGblPara.m_bTransferSpeedMonitor=true;
    }
    if(cmdLineParser.isSet(opTransfer))
    {
        gGblPara.m_bTransfer2PC=true;
    }
    if(cmdLineParser.isSet(opUART))
    {
        gGblPara.m_bDumpUART=true;
    }
    if(cmdLineParser.isSet(opXMode))
    {
        gGblPara.m_bXMode=true;
    }
    if(cmdLineParser.isSet(opFMode))
    {
        gGblPara.m_bFMode=true;
    }

    //这里作一个优先级判断，若xMode和fMode同时启动，则只启动fMode
    if(gGblPara.m_bXMode && gGblPara.m_bFMode)
    {
        gGblPara.m_bXMode=false;
    }

    //read config file.
    QFile fileIni("AVLizard.ini");
    if(!fileIni.exists())
    {
        qDebug()<<"<error>:AVLizard.ini config file missed!";
        qDebug()<<"<error>:a template file was generated!";
        qDebug()<<"<error>:please modify it by manual and run again!";
        gGblPara.resetCfgFile();
        return 0;
    }
    gGblPara.readCfgFile();

    //这里我们确保设置的2个摄像头的分辨率必须一致！
    if((gGblPara.m_widthCAM1 != gGblPara.m_widthCAM2) ||(gGblPara.m_heightCAM1!=gGblPara.m_heightCAM2))
    {
        qDebug()<<"<Error>:the resolution of two cameras are not same.";
        qDebug()<<"<Error>: CAM1("<<gGblPara.m_widthCAM1<<"*"<<gGblPara.m_heightCAM1<<") CAM2("<<gGblPara.m_widthCAM2<<"*"<<gGblPara.m_heightCAM2<<")";
        return -1;
    }

    ////////////////////////////////////////////////////////
    qDebug()<<"AVLizard Version:"<<APP_VERSION<<" Build on"<<__DATE__<<" "<<__TIME__;
    qDebug()<<"CPU cores:"<<sysconf(_SC_NPROCESSORS_CONF);
    //print out audio related settings.
    qDebug()<<"############# Audio #############";
    qDebug()<<"Capture:"<<gGblPara.m_audio.m_capCardName<<","<<gGblPara.m_audio.m_nCaptureSampleRate<<",Channels:"<<gGblPara.m_audio.m_nChannelNum;
    qDebug()<<"Playback:"<<gGblPara.m_audio.m_playCardName<<","<<gGblPara.m_audio.m_nPlaybackSampleRate<<",Channels:"<<gGblPara.m_audio.m_nChannelNum;
    switch(gGblPara.m_audio.m_nDeNoiseMethod)
    {
    case 0:
        qDebug()<<"DeNoise:Disabled.";
        break;
    case 1:
        qDebug()<<"DeNoise:RNNoise Enabled.";
        break;
    case 2:
        qDebug()<<"DeNoise:WebRTC Enabled.";
        break;
    case 3:
        qDebug()<<"DeNoise:Bevis Enabled,grade "<<gGblPara.m_audio.m_nBevisGrade<<".";
        break;
    default:
        break;
    }
    if(gGblPara.m_audio.m_nGaindB>0)
    {
        if(!(gGblPara.m_audio.m_nGaindB>=0 && gGblPara.m_audio.m_nGaindB<=90))
        {
            gGblPara.m_audio.m_nGaindB=10;
            qDebug()<<"Error gain dB,fix to 10dB.";
        }
        qDebug()<<"Compression Gain dB:Enalbed,"<<gGblPara.m_audio.m_nGaindB;
    }else{
        qDebug()<<"Compression Gain dB:Disabled.";
    }

    //print out video related settings.
    qDebug()<<"############# Video #############";
    if(gGblPara.m_bXMode)
    {
        qDebug()<<"XMode Enabled.";
    }else if(gGblPara.m_bFMode){
        qDebug()<<"FMode Enabled.";
    }

    //write pid to file.
    if(gGblPara.writePid2File()<0)
    {
        return -1;
    }

    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    //load skin file.
    QFile fileSkin(":/skin/skin.qss");
    if(fileSkin.open(QIODevice::ReadOnly))
    {
        QByteArray baQss=fileSkin.readAll();
        app.setStyleSheet(baQss);
        fileSkin.close();
    }

    //start main task.
    ZMainTask *mainTask=new ZMainTask;
    if(mainTask->ZStartTask()<0)
    {
        return -1;
    }

    //enter event-loop.
    int ret=app.exec();

    //free memory.
    delete mainTask;

    return ret;
}

/*
#/bin/bash

#author:ShellAlbert.
#date:July 19,2018.
#this script is used to check the wifi hotspot by periodly.
#if the hotspot is not exist then restart it.
#

#export libraries.
LIB_QT=$PWD/libqt
LIB_OPENCV=$PWD/libopencv
LIB_ALSA=$PWD/libalsa
LIB_OPUS=$PWD/libopus
LIB_RNNOISE=$PWD/librnnoise
LIB_X264=$PWD/libx264
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_QT:$LIB_OPENCV:$LIB_ALSA:$LIB_OPUS:$LIB_RNNOISE:$LIB_X264


#start function.
function startHotspot()
{
nmcli connection add type wifi ifname '*' con-name AVLizard autoconnect no ssid "AVLizard(ZSYDbg)"
nmcli connection modify AVLizard 802-11-wireless.mode ap 802-11-wireless.band bg ipv4.method shared
nmcli connection modify AVLizard 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk 12345678
nmcli connection up AVLizard
return 0
}

#stop function.
function stopHotspot()
{
nmcli connection delete AVLizard
#    nmcli device disconnect wlan0
return 0
}

#the main entry.
#make sure we run this with root priority.
if [ `whoami` != "root" ];then
echo "<error>: latch me with root priority please."
exit -1
fi

#the main loop.
while true
do
    #make sure we have 2 cameras at least.
    CAM_NUM=`ls -l /dev/video* | wc -l`
    if [ $CAM_NUM -lt 2 ];then
        echo "<error>: I need 2 cameras at least."
    sleep 5
    fi

    #check wifi hotspot.
    LIVESTR=`nmcli connection show -active | grep AVLizard`
    if [ -z "$LIVESTR" ];then
        echo "<error>:cannot detected AVLizard hotspot,restart it."
        stopHotspot
        sleep 5
        startHotspot
    else
        echo "<okay>:wifi hotspot okay."
    fi


    #check the video app.
    if [ -f "/tmp/AVLizard.pid" ];then
        PID=`cat /tmp/AVLizard.pid`
        kill -0 $PID
        if [ $? -eq 0 ];then
            echo "<okay>:lizard pid detect okay."
        else
            echo "<error>:lizard pid detect error! schedule it."
            taskset -c 2,3,4,5 ./AVLizard.bin  --DoNotCmpCamId  &
        fi
    else
        echo "<error>:lizard pid file not exist! schedule it."
        taskset 2,3,4,5 ./AVLizard.bin  --DoNotCmpCamId &
    fi

    #check periodly every 5 seconds.
    sleep 30
done

#the end of file.

 */
