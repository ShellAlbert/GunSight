#include "zvideotask.h"
#include <video/zfiltecamdev.h>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QDir>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/videodev2.h>
ZVideoTask::ZVideoTask(QObject *parent) : QObject(parent)
{
    for(qint32 i=0;i<2;i++)
    {
        this->m_rbProcess[i]=NULL;
        this->m_rbYUV[i]=NULL;
        this->m_rbH264[i]=NULL;

        this->m_capThread[i]=NULL;
    }
    this->m_process=NULL;//图像处理线程.
    this->m_videoTxThreadSoft=NULL;
    this->m_videoTxThreadHard=NULL;
    this->m_timerExit=new QTimer;
    QObject::connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotChkAllExitFlags()));
}
ZVideoTask::~ZVideoTask()
{
    delete this->m_timerExit;
    delete this->m_process;

    for(qint32 i=0;i<2;i++)
    {
        delete this->m_capThread[i];
    }
    delete this->m_videoTxThreadSoft;
    delete this->m_videoTxThreadHard;

    for(qint32 i=0;i<2;i++)
    {
        delete this->m_rbProcess[i];
        delete this->m_rbYUV[i];
        delete this->m_rbH264[i];
    }
}
qint32 ZVideoTask::ZDoInit()
{
    //需要排除的设备节点列表.
    QStringList extractNode;
    //extractNode.append("video0");
    //    extractNode.append("video1");
    //    extractNode.append("video16");
    //    extractNode.append("video17");

    //列出/dev目录下所有的videoX设备节点.
    QStringList nodeNameList;
    QDir dir("/dev");
    QStringList fileList=dir.entryList(QDir::System);
    for(qint32 i=0;i<fileList.size();i++)
    {
        QString nodeName=fileList.at(i);
        if(nodeName.startsWith("video"))
        {
            if(!extractNode.contains(nodeName))
            {
                nodeNameList.append(nodeName);
            }
        }
    }

    if(nodeNameList.size()<3)
    {
        qDebug()<<"<Error>:No 3 cameras found at least.";
        return -1;
    }

    //比对配置文件ini设置的CamID来决定哪一个是主摄像头，哪一个是主摄像头Ex,哪一个是辅摄像头.
    bool bMainIdChk=false,bMainExIdChk=false,bAuxIdChk=false;
    QFile fileUSBBusId("usb_bus_id.txt");
    fileUSBBusId.open(QIODevice::WriteOnly);
    for(qint32 i=0;i<nodeNameList.size();i++)
    {
        struct v4l2_capability cap;
        QString nodeDev="/dev/"+nodeNameList.at(i);
        int fd=open(nodeDev.toStdString().c_str(),O_RDWR);
        if(ioctl(fd,VIDIOC_QUERYCAP,&cap)<0)
        {
            qDebug()<<"<Error>:failed to query capability "<<nodeDev;
            return -1;
        }
        qDebug()<<"<Info>:"<<nodeDev<<","<<QString((char*)cap.bus_info);
        fileUSBBusId.write(QString(nodeDev+QString((char*)cap.bus_info)).toUtf8()+"\n");
        if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam1ID)
        {
            bMainIdChk=true;
        }else if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam1IDEx)
        {
            bMainExIdChk=true;
        }else if(QString((char*)cap.bus_info)==gGblPara.m_video.m_Cam2ID)
        {
            bAuxIdChk=true;
        }
        gGblPara.m_video.m_mapID2Fd.insert(QString((char*)cap.bus_info),nodeDev);
        close(fd);
    }
    fileUSBBusId.close();
    if(!(bMainIdChk && bMainExIdChk && bAuxIdChk))
    {
        qDebug()<<"<Error>:config file USB IDs do not match real USB IDs!";
        return -1;
    }
    for(qint32 i=0;i<2;i++)
    {
        //main/aux capture thread to img process thread queue.
        this->m_rbProcess[i]=new ZRingBuffer(MAX_VIDEO_RING_BUFFER,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*2);
        //main/aux cap thread to h264 enc thread.
        this->m_rbYUV[i]=new ZRingBuffer(MAX_VIDEO_RING_BUFFER,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*2);
    }
    //create capture thread.
    this->m_capThread[0]=new ZImgCapThread(gGblPara.m_video.m_Cam1ID,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM1_ID_Main);//Main Camera.
    this->m_capThread[1]=new ZImgCapThread(gGblPara.m_video.m_Cam2ID,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM2_ID_Aux);//Aux Camera.
    this->m_capThread[2]=new ZImgCapThread(gGblPara.m_video.m_Cam1IDEx,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,gGblPara.m_fpsCAM1,CAM3_ID_MainEx);//MainEx Camera.
    QObject::connect(this->m_capThread[0],SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));
    QObject::connect(this->m_capThread[1],SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));
    QObject::connect(this->m_capThread[2],SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));
    this->m_capThread[0]->ZBindProcessQueue(this->m_rbProcess[0]);
    this->m_capThread[1]->ZBindProcessQueue(this->m_rbProcess[1]);
    this->m_capThread[2]->ZBindProcessQueue(this->m_rbProcess[0]);
    //MainEx camera does not bind process queue.
    this->m_capThread[0]->ZBindYUVQueue(this->m_rbYUV[0]);//main camera.
    this->m_capThread[1]->ZBindYUVQueue(this->m_rbYUV[1]);//aux camera.
    this->m_capThread[2]->ZBindYUVQueue(this->m_rbYUV[0]);//mainEx camera.


    this->m_videoTxThreadSoft=new ZVideoTxThreadHard264(TCP_PORT_VIDEO,TCP_PORT_VIDEO2);
    this->m_videoTxThreadSoft->ZBindQueue(this->m_rbYUV[0],this->m_rbYUV[1]);
    QObject::connect(this->m_videoTxThreadSoft,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));

    //create video tx thread.
    this->m_videoTxThreadHard=new ZVideoTxThreadHard2642(TCP_PORT_VIDEO,TCP_PORT_VIDEO2);
    this->m_videoTxThreadHard->ZBindQueue(this->m_rbYUV[0],this->m_rbYUV[1]);
    QObject::connect(this->m_videoTxThreadHard,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));

    //create image process thread.
    this->m_process=new ZImgProcessThread;
    QObject::connect(this->m_process,SIGNAL(ZSigThreadFinished()),this,SLOT(ZSlotSubThreadsExited()));
    this->m_process->ZBindMainAuxImgQueue(this->m_rbProcess[0],this->m_rbProcess[1]);
    return 0;
}
ZImgCapThread* ZVideoTask::ZGetImgCapThread(qint32 index)
{
    ZImgCapThread *capThread=NULL;
    switch(index)
    {
    case 0:
        capThread=this->m_capThread[0];
        break;
    case 1:
        capThread=this->m_capThread[1];
        break;
    default:
        break;
    }
    return capThread;
}
ZImgProcessThread* ZVideoTask::ZGetImgProcessThread()
{
    return this->m_process;
}
qint32 ZVideoTask::ZStartTask()
{
    this->m_videoTxThreadSoft->ZStartThread();
    this->m_videoTxThreadHard->ZStartThread();
    for(qint32 i=0;i<3;i++)
    {
        this->m_capThread[i]->ZStartThread();
    }
    this->m_process->ZStartThread();
    return 0;
}
void ZVideoTask::ZSlotSubThreadsExited()
{
    if(!this->m_timerExit->isActive())
    {
        //tell all working threads to exit.
        this->m_capThread[0]->exit(0);
        this->m_capThread[1]->exit(0);
        this->m_capThread[2]->exit(0);
        this->m_process->exit(0);
        //this->m_videoTxThread->exit(0);
        this->m_videoTxThreadHard->exit(0);

        //start timer to help unblocking the queue empty or full.
        this->m_timerExit->start(1000);
    }
}
void ZVideoTask::ZSlotChkAllExitFlags()
{
    if(gGblPara.m_bGblRst2Exit)
    {
        //采集任务针对队列的操作使用的try。

        //如果图像处理线程没有退出，可能process queue队列空，则模拟ImgCapThread投入一个空的图像数据，解除阻塞。
        if(!this->m_process->ZIsExitCleanup())
        {
            if(this->m_rbProcess[0]->ZGetValidNum()==0)
            {
                qint8 *pRGBEmpty=new qint8[gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3];
                memset(pRGBEmpty,0,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3);
                //因为图像是RGB888的，所以总字节数=width*height*3.
                if(this->m_rbProcess[0]->ZPutElement((qint8*)pRGBEmpty,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put RGB to process queue 0.";
                }
                delete [] pRGBEmpty;
            }
            if(this->m_rbProcess[1]->ZGetValidNum()==0)
            {
                qint8 *pRGBEmpty=new qint8[gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3];
                memset(pRGBEmpty,0,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3);
                //因为图像是RGB888的，所以总字节数=width*height*3.
                if(this->m_rbProcess[1]->ZPutElement(pRGBEmpty,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put RGB to process queue 1.";
                }
                delete [] pRGBEmpty;
            }
        }

        //如果发送线程没有退出，可能yuv queue队列为空，模拟ImgCapThread投入一帧空数据，解除阻塞。
        if(this->m_videoTxThreadHard->ZIsExitCleanup())
        {
            if(this->m_rbYUV[0]->ZGetValidNum()==0)
            {
                qint8 *pYUVEmpty=new qint8[gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3];
                memset(pYUVEmpty,0,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3);
                if(this->m_rbYUV[0]->ZPutElement(pYUVEmpty,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put yuv data to yuv queue 0.";
                }
                delete [] pYUVEmpty;
            }
        }
        if(this->m_videoTxThreadSoft->ZIsExitCleanup())
        {
            if(this->m_rbYUV[1]->ZGetValidNum()==0)
            {
                qint8 *pYUVEmpty=new qint8[gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3];
                memset(pYUVEmpty,0,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3);
                if(this->m_rbYUV[1]->ZPutElement(pYUVEmpty,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3)<0)
                {
                    qDebug()<<"<Exiting>:timeout to put yuv data to yuv queue 1.";
                }
                delete [] pYUVEmpty;
            }
        }

        //当所有的子线程都退出时，则视频任务退出。
        if(this->ZIsExitCleanup())
        {
            this->m_timerExit->stop();
            emit this->ZSigVideoTaskExited();
        }
    }
}
bool ZVideoTask::ZIsExitCleanup()
{
    bool bCleanup=true;
    if(!this->m_capThread[0]->ZIsExitCleanup() || !this->m_capThread[1]->ZIsExitCleanup() || !this->m_capThread[2]->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    if(!this->m_process->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    if(!this->m_videoTxThreadHard->ZIsExitCleanup() || !this->m_videoTxThreadSoft->ZIsExitCleanup())
    {
        bCleanup=false;
    }
    return bCleanup;
}
