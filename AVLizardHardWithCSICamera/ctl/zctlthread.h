#ifndef ZCTLTHREAD_H
#define ZCTLTHREAD_H

#include <QThread>
#include <QJsonDocument>
#include <QVector>
#include <QTcpSocket>
#include <QTcpServer>
#include "zgblpara.h"
class ZJsonThread:public QThread
{
    Q_OBJECT
public:
    ZJsonThread(QObject *parent=nullptr);
    qint32 ZStartThread();
signals:
    void ZSigThreadExited();
protected:
    void run();
private:
    qint32 ZScanRecvBuffer();
    QByteArray ZParseJson(const QJsonDocument &jsonDoc);
private:
    QTcpSocket *m_tcpSocket;
private:
    qint64 m_nJsonLen;
    qint8 *m_recvBuffer;
};
#endif // ZCTLTHREAD_H
