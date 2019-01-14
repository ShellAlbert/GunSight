#include "ztcp2uartforwardthread.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <sys/socket.h>
#include <zgblpara.h>
#include <QtSerialPort/QSerialPort>
ZTcp2UartForwardThread::ZTcp2UartForwardThread()
{
    this->m_bExitFlag=false;
    this->m_bCleanup=false;
}
qint32 ZTcp2UartForwardThread::ZStartThread()
{
    this->m_bExitFlag=false;
    this->start();
    return 0;
}
qint32 ZTcp2UartForwardThread::ZStopThread()
{
    this->m_bExitFlag=true;
    return 0;
}
void ZTcp2UartForwardThread::run()
{
    QSerialPort *uart=new QSerialPort;
    uart->setPortName(gGblPara.m_uartName);
    uart->setBaudRate(QSerialPort::Baud115200);
    uart->setDataBits(QSerialPort::Data8);
    uart->setParity(QSerialPort::NoParity);
    uart->setStopBits(QSerialPort::OneStop);
    uart->setFlowControl(QSerialPort::NoFlowControl);
    if(!uart->open(QIODevice::ReadWrite))
    {
        qDebug()<<"<error>:Tcp2Uart cannot open uart device.";
        //此处设置全局请求退出标志,请求其他线程退出.
        gGblPara.m_bGblRst2Exit=true;
        delete uart;
        this->m_bCleanup=true;
        return;
    }

    qDebug()<<"<MainLoop>:Tcp2UartThread starts.";
    this->m_bCleanup=false;
    while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
    {
        QTcpServer *tcpServer=new QTcpServer;
        int on=1;
        int sockFd=tcpServer->socketDescriptor();
        setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
        if(!tcpServer->listen(QHostAddress::Any,TCP_PORT_FORWARD))
        {
            qDebug()<<"<error>: Tcp2Uart server listen error on port:"<<TCP_PORT_FORWARD;
            this->sleep(3);
            delete tcpServer;
            continue;
        }
        qDebug()<<"<Tcp2Uart>: listen on tcp "<<TCP_PORT_FORWARD;
        //wait until get a new connection.
        while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
        {
            //qDebug()<<"wait for tcp connection";
            if(tcpServer->waitForNewConnection(1000*2))
            {
                break;
            }
        }

        QTcpSocket *tcpSocket=tcpServer->nextPendingConnection();
        if(tcpSocket)
        {
            //客户端连接上后，就判断服务监听端，这样只允许一个tcp连接.
            tcpServer->close();

            //设置连接标志.
            gGblPara.m_bTcp2UartConnected=true;
            qDebug()<<"Tcp2Uart connected.";

            while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
            {
//                jsonObj.insert("ImgMatched",///<
//                               QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")///<
//                               .arg(gGblPara.m_video.m_rectTemplate.x()).arg(gGblPara.m_video.m_rectTemplate.y())///<
//                               .arg(gGblPara.m_video.m_rectTemplate.width()).arg(gGblPara.m_video.m_rectTemplate.height())///<
//                               .arg(gGblPara.m_video.m_rectMatched.x()).arg(gGblPara.m_video.m_rectMatched.y())///<
//                               .arg(gGblPara.m_video.m_rectMatched.width()).arg(gGblPara.m_video.m_rectMatched.height())///<
//                               .arg(gGblPara.m_video.m_nDiffX).arg(gGblPara.m_video.m_nDiffY).arg(gGblPara.m_video.m_nCostMs));
                //只有开启图像匹配功能时才tx diffXY to uart.
                if(gGblPara.m_bJsonImgPro)
                {
                    qint16 nDiffX=gGblPara.m_video.m_nDiffX;
                    qint16 nDiffY=gGblPara.m_video.m_nDiffY;
                    QByteArray baCmd;
                    baCmd.resize(12);
                    baCmd[0]=0x5A;//frame head.
                    baCmd[1]=0xA5;
                    baCmd[2]=0x0C;//frame length,12 bytes.
                    baCmd[3]=0x02;//frame type.
                    baCmd[4]=0x01;//end point.
                    //data section.
                    baCmd[5]=0x23;
                    baCmd[6]=0x04;
                    baCmd[7]=(qint8)((nDiffX>>8)&0xFF);//diff X.
                    baCmd[8]=(qint8)(nDiffX&0xFF);
                    baCmd[9]=(qint8)((nDiffY>>8)&0xFF);//diff Y.
                    baCmd[10]=(qint8)(nDiffY&0xFF);
                    baCmd[11]=gGblPara.ZDoCheckSum((uint8_t*)baCmd.data(),11);//checksum.
                    uart->write(baCmd);
                    if(!uart->waitForBytesWritten(SOCKET_TX_TIMEOUT))
                    {
                        qDebug()<<"<error>:Tcp2Uart failed to tx DiffXY "<<uart->errorString();
                        break;
                    }
                }
                //read data from tcp and write it to uart.
                if(tcpSocket->waitForReadyRead(50))//100ms.
                {
                    QByteArray baFromTcp=tcpSocket->readAll();
                    //qDebug()<<"from tcp:"<<baFromTcp;
                    uart->write(baFromTcp);
                    if(!uart->waitForBytesWritten(1000))
                    {
                        qDebug()<<"<error>:Tcp2Uart failed to forward tcp to uart "<<uart->errorString();
                        break;
                    }
                    gGblPara.m_nTcp2UartBytes+=baFromTcp.size();
                }
                //read data from uart and write data to tcp.
                if(uart->waitForReadyRead(50))//100ms.
                {
                    QByteArray baFromUart=uart->readAll();
                    //qDebug()<<"from uart:"<<baFromUart;
                    tcpSocket->write(baFromUart);
                    if(!tcpSocket->waitForBytesWritten(1000))
                    {
                        qDebug()<<"<error>:Tcp2Uart failed to forward uart to tcp:"<<tcpSocket->errorString();
                        break;
                    }
                    gGblPara.m_nUart2TcpBytes+=baFromUart.size();
                }

                //check tcp socket state.
                if(tcpSocket->state()!=QAbstractSocket::ConnectedState)
                {
                    qDebug()<<"<Warning>:Tcp2Uart socket broken.";
                    break;
                }
            }

            //设置连接标志.
            gGblPara.m_bTcp2UartConnected=false;
            qDebug()<<"Tcp2Uart disconnected.";
        }
        delete tcpServer;
        tcpServer=NULL;
    }
    //退出主循环,做清理工作.
    uart->close();
    delete uart;
    qDebug()<<"<MainLoop>:Tcp2UartThread ends.";
    //此处设置本线程退出标志.
    //同时设置全局请求退出标志，请求其他线程退出.
    gGblPara.m_bTcp2UartThreadExitFlag=true;
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}
bool ZTcp2UartForwardThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
