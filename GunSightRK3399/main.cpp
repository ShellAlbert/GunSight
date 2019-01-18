#include "zmainui.h"
#include <QApplication>
#include "zringbuffer.h"
#include "zgblhelp.h"
#include "zcapthread.h"
#include "zkeydetthread.h"
#include "ztrackthread.h"
#include <QScreen>
#include <QFontDatabase>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSize sizeScreen=app.primaryScreen()->size();
    qDebug()<<"Screen size:"<<sizeScreen;
    g_GblHelp.m_fImg2ScreenWScale=sizeScreen.width()*1.0/CAP_IMG_SIZE_W;
    g_GblHelp.m_fImg2ScreenHScale=sizeScreen.height()*1.0/CAP_IMG_SIZE_H;
    qDebug()<<"Img2Screen scale:"<<g_GblHelp.m_fImg2ScreenWScale<<","<<g_GblHelp.m_fImg2ScreenHScale;

    //load font.
    qint32 nFontId=QFontDatabase::addApplicationFont("wqy-zenhei.ttc");
    if(nFontId>=0)
    {
        //qDebug()<<"FontId="<<nFontId;
        QString strZenHei=QFontDatabase::applicationFontFamilies(nFontId).at(0);
        if(!strZenHei.isEmpty())
        {
            //qDebug()<<"wqy-zenhei="<<strZenHei;
            QFont fontApp(strZenHei);
            app.setFont(fontApp);
        }
    }


    qDebug()<<"Camera Image Size:"<<CAP_IMG_SIZE_W<<"*"<<CAP_IMG_SIZE_H;
    qDebug()<<"Tracking Box Size:"<<TRACK_BOX_W<<"*"<<TRACK_BOX_H;

    ZRingBuffer *rb=new ZRingBuffer(15,CAP_IMG_SIZE_W*CAP_IMG_SIZE_H*3);
    ZCapThread *cap=new ZCapThread("/dev/video0",CAP_IMG_SIZE_W,CAP_IMG_SIZE_H,20);
    cap->ZBindRingBuffer(rb);
    ZTrackThread *track=new ZTrackThread;
    track->ZBindRingBuffer(rb);

    ZMainUI *ui=new ZMainUI;
    g_GblHelp.m_mainWidget=ui;
    ZKeyDetThread *key=new ZKeyDetThread;


    QObject::connect(cap,SIGNAL(ZSigNewImgArrived(QImage)),ui,SLOT(ZSlotUptImg(QImage)));
    QObject::connect(track,SIGNAL(ZSigNewInitBox(QImage)),ui,SLOT(ZSlotUptBoxImg(QImage)));
    QObject::connect(track,SIGNAL(ZSigNewTracking()),ui,SLOT(ZSlotUptProcessed()));

    ui->showFullScreen();
    cap->start();
    track->start();
    key->start();
    qint32 nRet=app.exec();

    delete cap;
    delete ui;
    delete rb;
    delete key;
    system("echo 32 >/sys/class/gpio/unexport");
    return nRet;
}
