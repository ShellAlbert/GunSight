#include "zkeydetthread.h"
#include "zgblhelp.h"
#include <QFile>
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>
ZKeyDetThread::ZKeyDetThread()
{

}
void ZKeyDetThread::run()
{
    //check Menu key inode.
    //Menu key:GPIO1_A0=1*32+0=32.
    if(!QFile::exists("/sys/class/gpio/gpio32/value"))
    {
        system("chmod 777 /sys/class/gpio/export");
        system("echo 32 >/sys/class/gpio/export");
        system("chmod 777 /sys/class/gpio/gpio32/direction");
        system("echo in >/sys/class/gpio/gpio32/direction");
    }

    QString preKey="0\n";
    while(!g_GblHelp.m_bExitFlag)
    {

        QFile fileKey("/sys/class/gpio/gpio32/value");
        if(!fileKey.open(QIODevice::ReadOnly))
        {
            qDebug()<<"<ERROR>:[ZKeyDet],failed to open gpio key:"<<fileKey.errorString();
            break;
        }

        QString curKey=fileKey.readAll();
        //qDebug()<<"read key:"<<curKey;
        fileKey.close();
        //check previous value and current value.
        if(preKey=="0\n" && curKey=="1\n")
        {
            qDebug()<<"key pressed";
            QKeyEvent *event=new QKeyEvent(QEvent::KeyPress,Qt::Key_F1,Qt::NoModifier);
            QCoreApplication::postEvent(g_GblHelp.m_mainWidget,event);
        }else if(preKey=="1\n" && curKey=="0\n")
        {
            qDebug()<<"key released";
            QKeyEvent *event=new QKeyEvent(QEvent::KeyRelease,Qt::Key_F1,Qt::NoModifier);
            QCoreApplication::postEvent(g_GblHelp.m_mainWidget,event);
        }
        preKey=curKey;
        this->usleep(1000*200);//200ms.
    }
    //set global exit flag to cause other threads to exit normally.
    g_GblHelp.m_bExitFlag=true;
    return;
}
