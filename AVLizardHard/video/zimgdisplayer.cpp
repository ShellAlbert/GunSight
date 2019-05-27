#include "zimgdisplayer.h"
#include <QPainter>
#include <QDebug>
#include <QDateTime>
ZImgDisplayer::ZImgDisplayer(qint32 nCenterX,qint32 nCenterY,bool bMainCamera,QWidget *parent) : QWidget(parent)
{
    this->m_nCenterX=nCenterX;
    this->m_nCenterY=nCenterY;
    this->m_colorRect=QColor(0,255,0);
    this->m_bMainCamera=bMainCamera;
    this->m_nTotalFrames=0;
    this->m_nTotalTime=0;
    //remember current timestamp in millsecond.
    this->m_nLastTsMsec=QDateTime::currentMSecsSinceEpoch();
}
ZImgDisplayer::~ZImgDisplayer()
{
}
void ZImgDisplayer::ZSetSensitiveRect(QRect rect)
{
    this->m_rectSensitive=rect;
}
void ZImgDisplayer::ZSetPaintParameters(QColor colorRect)
{
    this->m_colorRect=colorRect;
}
QSize ZImgDisplayer::sizeHint() const
{
    return QSize(IMG_SCALED_W,IMG_SCALED_H);
}

void ZImgDisplayer::ZSlotFlushImg(const QImage &img)
{
    this->m_img=img;

    //calculate the fps.
    this->m_nTotalFrames++;
    qint64 nNowTsMsec=QDateTime::currentMSecsSinceEpoch();
    this->m_nTotalTime+=(nNowTsMsec-this->m_nLastTsMsec);
    this->m_nLastTsMsec=nNowTsMsec;
    //qDebug()<<"totalfrm:"<<this->m_nTotalFrames<<",time:"<<this->m_nTotalTime;
    qint64 nTotalMsec=this->m_nTotalTime/1000;
    if(nTotalMsec>0)
    {
        this->m_nCAMFps=this->m_nTotalFrames/nTotalMsec;
    }else{
        this->m_nCAMFps=0;
    }
    this->update();
}
void ZImgDisplayer::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
     QPainter painter(this);
     if(this->m_img.isNull())
     {
         painter.fillRect(QRectF(0,0,this->width(),this->height()),Qt::black);
         QPen pen(Qt::green,4);
         painter.setPen(pen);
         painter.drawLine(QPointF(0,0),QPointF(this->width(),this->height()));
         painter.drawLine(QPointF(this->width(),0),QPointF(0,this->height()));
         return;
     }
     //1.draw everything on image.
     QPainter painterImg(&this->m_img);
     painterImg.setRenderHints(QPainter::Antialiasing);
     //set font & pen.
     QFont tFont=painterImg.font();
     tFont.setPointSize(16);
     painterImg.setFont(tFont);
     painterImg.setPen(QPen(Qt::green,3,Qt::SolidLine));
     //translate (0,0) to calibrate (x,y).
     painterImg.save();
     painterImg.translate(this->m_nCenterX,this->m_nCenterY);
     painterImg.setPen(QPen(Qt::blue,4,Qt::SolidLine));
     //if we are not in ImgPro state,draw a circle to indicate center.
     if(gGblPara.m_bJsonImgPro)
     {
     }else{
         painterImg.drawEllipse(QPoint(0,0),30,30);
     }

     QPoint pt1,pt2,pt3,pt4,pt5;
     pt1=QPoint(60,0);
     pt2=QPoint(pt1.x()+80,pt1.y()-10);
     pt3=QPoint(pt2.x()+100,pt2.y());
     pt4=QPoint(pt3.x(),pt1.y()+10);
     pt5=QPoint(pt2.x(),pt1.y()+10);
     QPainterPath path1;
     path1.moveTo(pt1);
     path1.lineTo(pt2);
     path1.lineTo(pt3);
     path1.lineTo(pt4);
     path1.lineTo(pt5);
     path1.lineTo(pt1);

     painterImg.rotate(45.0);
     painterImg.fillPath(path1,QBrush(Qt::blue));
     painterImg.rotate(90.0);
     painterImg.fillPath(path1,QBrush(Qt::blue));
     painterImg.rotate(90.0);
     painterImg.fillPath(path1,QBrush(Qt::blue));
     painterImg.rotate(90.0);
     painterImg.fillPath(path1,QBrush(Qt::blue));
     painterImg.restore();

     //draw the template rect & matched rect.
     if(gGblPara.m_bJsonImgPro)
     {
         if(this->m_bMainCamera)
         {
             //cut a template by calibrate (x1,y1) center to size(200x200) in main img.
             QRect rect(this->m_nCenterX-100,this->m_nCenterY-100,200,200);
             painterImg.drawRect(rect);
             //qDebug()<<this->m_img.width()<<this->m_img.height()<<rect;
         }else{
             //draw a template by calibrate(x2,y2) center to size (200,200) in aux img.
             QRect rect(this->m_nCenterX-100,this->m_nCenterY-100,200,200);
             painterImg.drawRect(rect);

             //draw the TrackingRectangle.
             QPen penRect(Qt::red,5);
             painterImg.setPen(penRect);
             painterImg.drawRect(this->m_rectSensitive);

             //draw a line from template rect center to tracking rect center.
             painterImg.drawLine(QPoint(rect.center()),QPoint(this->m_rectSensitive.center()));
         }
     }
     //2.draw image to UI.
     QRectF rectIMG(0,0,this->width(),this->height());
     painter.drawImage(rectIMG,this->m_img);

     //draw some others message.
     QString camInfo;
     if(this->m_bMainCamera)
     {
         camInfo.append("Main\n");
     }else{
         camInfo.append("Aux\n");
     }
     camInfo.append(tr("%1*%2\n").arg(this->m_img.width()).arg(this->m_img.height()));
     if(this->m_bMainCamera)
     {
         camInfo.append(tr("%1/%2fps\n").arg(gGblPara.m_video.m_nCam1Fps).arg(this->m_nCAMFps));
     }else{
         camInfo.append(tr("%1/%2fps\n").arg(gGblPara.m_video.m_nCam2Fps).arg(this->m_nCAMFps));
     }
     painter.setPen(QPen(Qt::green,2,Qt::SolidLine));
     qint32 nHeight=painter.fontMetrics().height();
     qint32 nWidth=painter.fontMetrics().width(camInfo);

     QRect rectCamInfo(0,0,nWidth*2,nHeight*3);
     painter.drawText(rectCamInfo,camInfo);

     if(this->m_bMainCamera)
     {
         qint32 nHeight=painter.fontMetrics().height();
         qint32 nWidth=painter.fontMetrics().width(gGblPara.m_video.m_Cam1ID);
         QRect rectCamID(0,this->height()-nHeight,nWidth+10,nHeight);
         painter.drawText(rectCamID,gGblPara.m_video.m_Cam1ID);
     }else{
         qint32 nHeight=painter.fontMetrics().height();
         qint32 nWidth=painter.fontMetrics().width(gGblPara.m_video.m_Cam2ID);
         QRect rectCamID(0,this->height()-nHeight,nWidth+10,nHeight);
         painter.drawText(rectCamID,gGblPara.m_video.m_Cam2ID);
     }

     //draw the current time.
     QString strDT=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
     qint32 nDTWidth=painter.fontMetrics().width(strDT);
     QRect rectDT(this->width()-nDTWidth-10,this->height()-nHeight-2,nDTWidth+10,nHeight);
     painter.drawText(rectDT,strDT);
}
