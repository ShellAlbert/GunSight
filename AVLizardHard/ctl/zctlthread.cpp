#include "zctlthread.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <sys/socket.h>
#include <zgblpara.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QProcess>

ZJsonThread::ZJsonThread(QObject *parent):QThread(parent)
{
    this->m_tcpSocket=NULL;
    this->m_nJsonLen=0;
}
qint32 ZJsonThread::ZStartThread()
{
    this->start();
    return 0;
}
void ZJsonThread::run()
{
    QTcpServer *tcpServer=new QTcpServer;
    int on=1;
    int sockFd=tcpServer->socketDescriptor();
    setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    this->m_recvBuffer=new qint8[BUFSIZE_1MB];
    while(!gGblPara.m_bGblRst2Exit)
    {
        gGblPara.m_bCtlClientConnected=false;//设置断开标志.

        //listen.
        if(!tcpServer->listen(QHostAddress::Any,TCP_PORT_CTL))
        {
            qDebug()<<"<Error>: Ctl server listen error on port:"<<TCP_PORT_CTL;
            gGblPara.m_bGblRst2Exit=true;
            break;
        }

        //wait until connected.
        while(!gGblPara.m_bGblRst2Exit)
        {
            if(tcpServer->waitForNewConnection(1000*1))//1s.
            {
                gGblPara.m_bCtlClientConnected=true;//设置连接标志.
                break;
            }
        }


        this->m_tcpSocket=tcpServer->nextPendingConnection();
        if(NULL==this->m_tcpSocket)
        {
            qDebug()<<"<Error>:empty next pending connection.";
            continue;
        }

        //同一时刻只允许一个客户端连接，当连接上时，则关闭服务端.
        tcpServer->close();

        //read & write.
        this->m_nJsonLen=0;
        while(!gGblPara.m_bGblRst2Exit)
        {
            if(this->m_tcpSocket->waitForReadyRead(SOCKET_TX_TIMEOUT))//100ms.
            {
                QByteArray baJsonData=this->m_tcpSocket->readAll();
                if(baJsonData.size()<=0)
                {
                    qDebug()<<"<Warning>:not read any json data.";
                    continue;
                }
//                qDebug()<<"----------------read all begin-----------";
//                qDebug()<<baJsonData;
//                qDebug()<<"----------------read all end-----------";
                if(baJsonData.size()>0)
                {
                    memcpy(this->m_recvBuffer+this->m_nJsonLen,baJsonData.data(),baJsonData.size());
                    this->m_nJsonLen+=baJsonData.size();
                    this->m_recvBuffer[this->m_nJsonLen]='\0';
                    this->ZScanRecvBuffer();
                }
            }
            //只有开启图像匹配功能时才上传匹配结果集.
#if 1
            if(gGblPara.m_bJsonImgPro)
            {
                QJsonObject jsonObj;
                jsonObj.insert("ImgMatched",///<
                               QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")///<
                               .arg(gGblPara.m_video.m_rectTemplate.x()).arg(gGblPara.m_video.m_rectTemplate.y())///<
                               .arg(gGblPara.m_video.m_rectTemplate.width()).arg(gGblPara.m_video.m_rectTemplate.height())///<
                               .arg(gGblPara.m_video.m_rectMatched.x()).arg(gGblPara.m_video.m_rectMatched.y())///<
                               .arg(gGblPara.m_video.m_rectMatched.width()).arg(gGblPara.m_video.m_rectMatched.height())///<
                               .arg(gGblPara.m_video.m_nDiffX).arg(gGblPara.m_video.m_nDiffY).arg(gGblPara.m_video.m_nCostMs));
                QJsonDocument jsonDoc;
                jsonDoc.setObject(jsonObj);
                QByteArray baTx=jsonDoc.toJson();
                QByteArray baTxLen=qint32ToQByteArray(baTx.size());
                //qDebug()<<baTx;
                this->m_tcpSocket->write(baTxLen);
                if(!this->m_tcpSocket->waitForBytesWritten(1000))
                {
                    qDebug()<<"<Error>:json tcp socket timeout.reset";
                    break;
                }
                this->m_tcpSocket->write(baTx);
                if(!this->m_tcpSocket->waitForBytesWritten(1000))
                {
                    qDebug()<<"<Error>:json tcp socket timeout.reset";
                    break;
                }
            }
#endif
            if(this->m_tcpSocket->state()!=QAbstractSocket::ConnectedState)
            {
                qDebug()<<"<Error>:tcp socket broken.";
                break;
            }
        }
    }
    delete []this->m_recvBuffer;
    this->m_recvBuffer=NULL;

    tcpServer->close();
    delete tcpServer;
    tcpServer=NULL;

    emit this->ZSigThreadExited();
}

qint32 ZJsonThread::ZScanRecvBuffer()
{
//    QByteArray prt((char*)this->m_recvBuffer,this->m_nJsonLen);
//    qDebug()<<prt;
    qint32 nOffset=0;
    qint32 nRemainingBytes=this->m_nJsonLen;
    while(nRemainingBytes>0)
    {
        //fetch 4 bytes json pkt len.
        //check the remainings bytes is feet or not.
        if(nRemainingBytes<4)//we need more data,check next time.
        {
            break;
        }
        QByteArray baJsonLen((char*)(this->m_recvBuffer+nOffset+0),4);
        //convert to int.
        qint32 nJsonLen=QByteArrayToqint32(baJsonLen);
        if(nJsonLen<0)
        {
            qDebug()<<"<Error>:error convert json pkt len.reset.";
            this->m_nJsonLen=0;
            return -1;
        }
        //qDebug()<<"json len:"<<nJsonLen;
        //fetch n bytes json pkt data.
        //check the remaining bytes is feet or not.
        if(nRemainingBytes<(4+nJsonLen))//we need more data,check next time.
        {
            break;
        }
        QByteArray baJsonData((char*)(this->m_recvBuffer+nOffset+4),nJsonLen);
        //here we use (json pkt len+json pkt data) as a unit.
        //so here we add/sub in a unit.
        nOffset+=(4+nJsonLen);
        nRemainingBytes-=(4+nJsonLen);
        //qDebug()<<baJsonData;
#if 1
        //parse out json.
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(baJsonData,&jsonErr);
        if(jsonErr.error==QJsonParseError::NoError)
        {
            QByteArray baFeedBack=this->ZParseJson(jsonDoc);
            if(baFeedBack.size()>0)
            {
                QByteArray baFeedBackLen=qint32ToQByteArray(baFeedBack.size());
                this->m_tcpSocket->write(baFeedBackLen);
                this->m_tcpSocket->waitForBytesWritten(100);
                this->m_tcpSocket->write(baFeedBack);
                this->m_tcpSocket->waitForBytesWritten(100);
            }
        }
//        qDebug()<<"nRemaingBytes:"<<nRemainingBytes;
#endif
    }
    if(nRemainingBytes>0 && nOffset>0)
    {
        memmove(this->m_recvBuffer,this->m_recvBuffer+nOffset,nRemainingBytes);
        this->m_nJsonLen=nRemainingBytes;
        this->m_recvBuffer[nRemainingBytes]='\0';
//        qDebug()<<"after processed:remaing:"<<this->m_nJsonLen;
//        qDebug()<<QByteArray((const char*)this->m_recvBuffer,this->m_nJsonLen);
    }else{
        //processed all bytes.nRemainingBytes=0,so reset here.
        memset(this->m_recvBuffer,0,BUFSIZE_1MB);
        this->m_nJsonLen=0;
    }
    return 0;
}
QByteArray ZJsonThread::ZParseJson(const QJsonDocument &jsonDoc)
{
    bool bWrCfgFileFlag=false;
    QJsonObject jsonObjFeedBack;
    if(jsonDoc.isObject())
    {
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonObj.contains("ImgPro"))
        {
            QJsonValue val=jsonObj.take("ImgPro");
            if(val.isString())
            {
                QString method=val.toVariant().toString();
                if(method=="on")
                {
                    //设置图像比对开启标志位.
                    //这将引起采集线程向Process队列扔图像数据.
                    //从而ImgProcess线程解除等待开始处理图像.
                    gGblPara.m_bJsonImgPro=true;
                }else if(method=="off")
                {
                    //设置图像比对暂停标志位.
                    //这将引起采集线程不再向Process队列扔图像数据.
                    //使得ImgProcess线程等待信号量从而暂停.
                    gGblPara.m_bJsonImgPro=false;
                }else if(method=="query")
                {
                    //仅用于查询当前状态.
                }
                jsonObjFeedBack.insert("ImgPro",gGblPara.m_bJsonImgPro?"on":"false");
            }
        }
        if(jsonObj.contains("RTC"))
        {
            QJsonValue val=jsonObj.take("RTC");
            if(val.isString())
            {
                QString rtcStr=val.toVariant().toString();
                qDebug()<<"RTC:"<<rtcStr;
                QString cmdSetRtc=QString("date -s %1").arg(rtcStr);
                qDebug()<<"cmdSetRtc:"<<cmdSetRtc;
                QProcess::startDetached(cmdSetRtc);
                QProcess::startDetached("hwclock -w");
                QProcess::startDetached("sync");
                jsonObjFeedBack.insert("RTC",QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
            }
        }
        if(jsonObj.contains("DeNoise"))
        {
            QJsonValue val=jsonObj.take("DeNoise");
            if(val.isString())
            {
                QString deNoise=val.toVariant().toString();
                qDebug()<<"deNoise:"<<deNoise;
                if(deNoise=="off")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=0;
                }else if(deNoise=="Strong")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=1;
                }else if(deNoise=="WebRTC")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=2;
                }else if(deNoise=="Bevis")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=3;
                }else if(deNoise=="mmse")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=4;
                }else if(deNoise=="query")
                {
                    //仅用于查询当前状态.
                }
                switch(gGblPara.m_audio.m_nDeNoiseMethod)
                {
                case 0:
                    jsonObjFeedBack.insert("DeNoise","off");
                    break;
                case 1:
                    jsonObjFeedBack.insert("DeNoise","Strong");
                    break;
                case 2:
                    jsonObjFeedBack.insert("DeNoise","WebRTC");
                    break;
                case 3:
                    jsonObjFeedBack.insert("DeNoise","Bevis");
                    break;
                case 4:
                    jsonObjFeedBack.insert("DeNoise","mmse");
                    break;
                default:
                    break;
                }
            }
        }
        if(jsonObj.contains("BevisGrade"))
        {
            QJsonValue val=jsonObj.take("BevisGrade");
            if(val.isString())
            {
                QString bevisGrade=val.toVariant().toString();
                //qDebug()<<"bevisGrade:"<<bevisGrade;
                if(bevisGrade=="0")
                {
                    gGblPara.m_audio.m_nBevisGrade=1;
                }else if(bevisGrade=="1")
                {
                    gGblPara.m_audio.m_nBevisGrade=2;
                }else if(bevisGrade=="2")
                {
                    gGblPara.m_audio.m_nBevisGrade=3;
                }else if(bevisGrade=="3")
                {
                    gGblPara.m_audio.m_nBevisGrade=4;
                }else if(bevisGrade=="query")
                {
                    //仅用于查询当前状态.
                }
                switch(gGblPara.m_audio.m_nBevisGrade)
                {
                case 1:
                    jsonObjFeedBack.insert("BevisGrade","0");
                    break;
                case 2:
                    jsonObjFeedBack.insert("BevisGrade","1");
                    break;
                case 3:
                    jsonObjFeedBack.insert("BevisGrade","2");
                    break;
                case 4:
                    jsonObjFeedBack.insert("BevisGrade","3");
                    break;
                default:
                    break;
                }
            }
        }
        if(jsonObj.contains("WebRtcGrade"))
        {
            QJsonValue val=jsonObj.take("WebRtcGrade");
            if(val.isString())
            {
                QString webRtcGrade=val.toVariant().toString();
                //qDebug()<<"webRtcGrade:"<<webRtcGrade;
                if(webRtcGrade=="0")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=0;
                }else if(webRtcGrade=="1")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=1;
                }else if(webRtcGrade=="2")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=2;
                }else if(webRtcGrade=="query")
                {
                    //仅用于查询当前状态.
                }
                switch(gGblPara.m_audio.m_nBevisGrade)
                {
                case 0:
                    jsonObjFeedBack.insert("WebRtcGrade","0");
                    break;
                case 1:
                    jsonObjFeedBack.insert("WebRtcGrade","1");
                    break;
                case 2:
                    jsonObjFeedBack.insert("WebRtcGrade","2");
                    break;
                default:
                    break;
                }
            }
        }
        if(jsonObj.contains("DGain"))
        {
            QJsonValue val=jsonObj.take("DGain");
            if(val.isString())
            {
                QString dGain=val.toVariant().toString();
                if(dGain=="query")
                {
                    jsonObjFeedBack.insert("DGain",QString::number(gGblPara.m_audio.m_nGaindB));
                }else{
                    qint32 dGain=val.toVariant().toInt();
                    //qDebug()<<"DGain:"<<dGain;
                    gGblPara.m_audio.m_nGaindB=dGain;
                    jsonObjFeedBack.insert("DGain",QString::number(gGblPara.m_audio.m_nGaindB));
                }
            }
        }
        if(jsonObj.contains("FlushUI"))
        {
            QJsonValue val=jsonObj.take("FlushUI");
            if(val.isString())
            {
                QString flushUI=val.toVariant().toString();
                if(flushUI=="on")
                {
                    gGblPara.m_bJsonFlushUIWav=true;
                    gGblPara.m_bJsonFlushUIImg=true;

                }else if(flushUI=="off")
                {
                    gGblPara.m_bJsonFlushUIWav=false;
                    gGblPara.m_bJsonFlushUIImg=false;
                }else if(flushUI=="query")
                {
                    //仅用于查询当前状态.
                }
                if(gGblPara.m_bJsonFlushUIWav && gGblPara.m_bJsonFlushUIImg)
                {
                    jsonObjFeedBack.insert("FlushUI","on");
                }else{
                    jsonObjFeedBack.insert("FlushUI","off");
                }
            }
        }
        if(jsonObj.contains("Cam1CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam1CenterXY");
            if(val.isString())
            {
                QString cam1CenterXY=val.toVariant().toString();
                if(cam1CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam1CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX1).arg(gGblPara.m_calibrateY1));
                }else{
                    QStringList xyList=cam1CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam1CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calibrateX1=x;
                        gGblPara.m_calibrateY1=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam1CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX1).arg(gGblPara.m_calibrateY1));
                    }
                }
            }
        }
        if(jsonObj.contains("Cam2CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam2CenterXY");
            if(val.isString())
            {
                QString cam2CenterXY=val.toVariant().toString();
                if(cam2CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam2CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX2).arg(gGblPara.m_calibrateY2));
                }else{
                    QStringList xyList=cam2CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam1CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calibrateX2=x;
                        gGblPara.m_calibrateY2=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam2CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX2).arg(gGblPara.m_calibrateY2));
                    }
                }
            }
        }
        if(jsonObj.contains("Cam3CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam3CenterXY");
            if(val.isString())
            {
                QString cam3CenterXY=val.toVariant().toString();
                if(cam3CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam3CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX3).arg(gGblPara.m_calibrateY3));
                }else{
                    QStringList xyList=cam3CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam3CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calibrateX3=x;
                        gGblPara.m_calibrateY3=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam3CenterXY",QString("%1,%2").arg(gGblPara.m_calibrateX3).arg(gGblPara.m_calibrateY3));
                    }
                }
            }
        }
        if(jsonObj.contains("Accumulated"))
        {
            QJsonValue val=jsonObj.take("Accumulated");
            if(val.isString())
            {
                QString str=val.toVariant().toString();
                if(str=="query")
                {
                    jsonObjFeedBack.insert("Accumulated",QString::number(gGblPara.m_nAccumulatedSec));
                }else{
                    qDebug()<<"<Error>:unknow cmd in Accumulated json.";
                }
            }
        }
        if(jsonObj.contains("StrongMode"))
        {
            QJsonValue val=jsonObj.take("StrongMode");
            if(val.isString())
            {
                QString noiseView=val.toVariant().toString();
                if(noiseView=="mode1")
                {
                    gGblPara.m_audio.m_nRNNoiseView=0;
                    jsonObjFeedBack.insert("StrongMode","mode1");
                }else if(noiseView=="mode2")
                {
                    gGblPara.m_audio.m_nRNNoiseView=1;
                    jsonObjFeedBack.insert("StrongMode","mode2");
                }else if(noiseView=="mode3")
                {
                    gGblPara.m_audio.m_nRNNoiseView=2;
                    jsonObjFeedBack.insert("StrongMode","mode3");
                }else if(noiseView=="mode4")
                {
                    gGblPara.m_audio.m_nRNNoiseView=3;
                    jsonObjFeedBack.insert("StrongMode","mode4");
                }else if(noiseView=="mode5")
                {
                    gGblPara.m_audio.m_nRNNoiseView=4;
                    jsonObjFeedBack.insert("StrongMode","mode5");
                }else if(noiseView=="mode6")
                {
                    gGblPara.m_audio.m_nRNNoiseView=5;
                    jsonObjFeedBack.insert("StrongMode","mode6");
                }else if(noiseView=="mode7")
                {
                    gGblPara.m_audio.m_nRNNoiseView=6;
                    jsonObjFeedBack.insert("StrongMode","mode7");
                }else if(noiseView=="mode8")
                {
                    gGblPara.m_audio.m_nRNNoiseView=7;
                    jsonObjFeedBack.insert("StrongMode","mode8");
                }else if(noiseView=="mode9")
                {
                    gGblPara.m_audio.m_nRNNoiseView=8;
                    jsonObjFeedBack.insert("StrongMode","mode9");
                }else if(noiseView=="mode10")
                {
                    gGblPara.m_audio.m_nRNNoiseView=9;
                    jsonObjFeedBack.insert("StrongMode","mode10");
                }
            }
        }
        if(jsonObj.contains("MainCamSw"))
        {
            QJsonValue val=jsonObj.take("MainCamSw");
            if(val.isString())
            {
                QString str=val.toVariant().toString();
                if(str=="sw")
                {
//                    qDebug()<<"json:before:"<<gGblPara.m_video.m_Cam1IDNow;
                    if(gGblPara.m_video.m_Cam1IDNow==gGblPara.m_video.m_Cam1ID)
                    {
                        gGblPara.m_video.m_Cam1IDNow=gGblPara.m_video.m_Cam1IDEx;
                        jsonObjFeedBack.insert("MainCamSw","small");
                    }else{
                        gGblPara.m_video.m_Cam1IDNow=gGblPara.m_video.m_Cam1ID;
                        jsonObjFeedBack.insert("MainCamSw","big");
                    }
//                    //this flag will cause CapThread to reinit.
//                    gGblPara.m_video.m_bMainViewSwFlag=true;
//                    qDebug()<<"json:after:"<<gGblPara.m_video.m_Cam1IDNow;
                }else if(str=="query")
                {
                    if(gGblPara.m_video.m_Cam1IDNow==gGblPara.m_video.m_Cam1ID)
                    {
                        jsonObjFeedBack.insert("MainCamSw","big");
                    }else{
                        jsonObjFeedBack.insert("MainCamSw","small");
                    }
                }else{
                    qDebug()<<"<Error>:unknow cmd in MainCamSw json.";
                }
            }
        }
        if(bWrCfgFileFlag)
        {
            gGblPara.writeCfgFile();
        }
    }else{
        qDebug()<<"<Error>:CtlThread,failed to parse json.";
    }
    QJsonDocument jsonDocFeedBack;
    jsonDocFeedBack.setObject(jsonObjFeedBack);
    QByteArray baFeedBack=jsonDocFeedBack.toJson();
    return baFeedBack;
}
