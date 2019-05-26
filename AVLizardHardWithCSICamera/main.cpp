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
    qRegisterMetaType<ZImgMatchedSet>("ZImgMatchedSet");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    QApplication app(argc,argv);

    qDebug()<<"AVLizard Version:"<<APP_VERSION<<" Build on"<<__DATE__<<" "<<__TIME__;
    qDebug()<<"CPU cores:"<<sysconf(_SC_NPROCESSORS_CONF);

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

    //VEYE.290 CSI Camera only support 1920x1080/30fps.
    gGblPara.m_widthCAM1=gGblPara.m_widthCAM2=1920;
    gGblPara.m_heightCAM1=gGblPara.m_heightCAM2=1080;
    qDebug()<<"CAM1/CAM2 resolution : ("<<gGblPara.m_widthCAM1<<"*"<<gGblPara.m_heightCAM1<<")";


    qDebug()<<"Capture:"<<gGblPara.m_audio.m_capCardName<<","<<gGblPara.m_audio.m_nCaptureSampleRate<<",Channels:"<<gGblPara.m_audio.m_nChannelNum;
    qDebug()<<"Playback:"<<gGblPara.m_audio.m_playCardName<<","<<gGblPara.m_audio.m_nPlaybackSampleRate<<",Channels:"<<gGblPara.m_audio.m_nChannelNum;
    //DeNoise is disabled at startup.
    gGblPara.m_audio.m_nDeNoiseMethod=0;
    qDebug()<<"DeNoise: Disabled.";
    //audio gain dB is 0.
    gGblPara.m_audio.m_nGaindB=0;
    qDebug()<<"Compression Gain dB: 0, range [0,90].";

    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    //write pid to file.
    if(gGblPara.writePid2File()<0)
    {
        return -1;
    }

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
#this script is used to check the wifi hotspot periodly.
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
