#include "zmainui.h"
#include <QPainter>
#include <QTime>
#include <QDebug>
#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include "zgblhelp.h"
ZMainUI::ZMainUI(QWidget *parent)
    : QWidget(parent)
{
    //this->setWindowFlags(Qt::FramelessWindowHint);
    //this->setAttribute(Qt::WA_TranslucentBackground,true);

    //    this->m_llTitle=new QLabel(g_GblHelp.ZGetTipsByState(STATE_TRACKING_STOP));
    //    this->m_llTitle->setStyleSheet("QLabel{background-color:yellow;color:red;font-size:40px;}");
    //    this->m_llTitle->setFixedWidth(800);
    //    this->m_llTitle->setScaledContents(true);
    //    this->m_llTitle->setAlignment(Qt::AlignCenter);


    //    this->m_vLayoutTbLeft=new QVBoxLayout;
    //    this->m_vLayoutTbLeft->setSpacing(20);
    //    this->m_vLayoutTbLeft->addStretch(1);
    //    for(qint32 i=0;i<5;i++)
    //    {
    //        QFont fontNow=this->font();
    //        fontNow.setPixelSize(40);
    //        this->m_tbLeft[i]=new QToolButton;
    //        this->m_tbLeft[i]->setFont(fontNow);
    //        switch(i)
    //        {
    //        case 0:
    //            this->m_tbLeft[i]->setText(tr("校准"));
    //            break;
    //        case 1:
    //            this->m_tbLeft[i]->setText(tr("设置"));
    //            break;
    //        case 2:
    //            this->m_tbLeft[i]->setText(tr("颜色"));
    //            break;
    //        case 3:
    //            this->m_tbLeft[i]->setText(tr("调整"));
    //            break;
    //        case 4:
    //            this->m_tbLeft[i]->setText(tr("输出"));
    //            break;
    //        default:
    //            break;
    //        }
    //        this->m_vLayoutTbLeft->addWidget(this->m_tbLeft[i]);
    //    }
    //    this->m_vLayoutTbLeft->addStretch(1);

    //    this->m_llNum=new QLabel(tr("0/0"));
    //    this->m_llNum->setStyleSheet("QLabel{color:yellow;font-size:100px;}");
    //    this->m_hLayoutCenter=new QHBoxLayout;
    //    this->m_hLayoutCenter->addLayout(this->m_vLayoutTbLeft);
    //    this->m_hLayoutCenter->addStretch(1);
    //    this->m_hLayoutCenter->addWidget(this->m_llNum);


    //    this->m_hLayoutTips=new QHBoxLayout;
    //    this->m_hLayoutTips->setSpacing(10);
    //    for(qint32 i=0;i<5;i++)
    //    {
    //        this->m_llTips[i]=new QLabel;
    //        switch(i)
    //        {
    //        case 0:
    //            this->m_llTips[i]->setText(tr("95B-1式"));
    //            this->m_llTips[i]->setStyleSheet("QLabel{color:blue;font-size:60px;}");
    //            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
    //            break;
    //        case 1:
    //            this->m_llTips[i]->setText(tr("弹种1"));
    //            this->m_llTips[i]->setStyleSheet("QLabel{color:red;font-size:60px;}");
    //            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
    //            break;
    //        case 2:
    //            this->m_llTips[i]->setText(tr("白热"));
    //            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
    //            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
    //            break;
    //        case 3:
    //            this->m_llTips[i]->setText(tr("0"));
    //            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
    //            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
    //            break;
    //        case 4:
    //            this->m_llTips[i]->setText(tr("(0,0)"));
    //            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
    //            break;
    //        default:
    //            break;
    //        }

    //    }
    //    this->m_hLayoutTips->addStretch(1);
    //    this->m_hLayoutTips->addWidget(this->m_llTips[4]);

    //    this->m_vLayout=new QVBoxLayout;
    //    this->m_vLayout->setContentsMargins(10,50,10,10);
    //    this->m_vLayout->addWidget(this->m_llTitle,0,Qt::AlignCenter);
    //    this->m_vLayout->addStretch(1);
    //    this->m_vLayout->addLayout(this->m_hLayoutCenter);
    //    this->m_vLayout->addStretch(1);
    //    this->m_vLayout->addLayout(this->m_hLayoutTips);
    //    this->setLayout(this->m_vLayout);

    this->m_nFrmCnt=0;

    this->m_nFrmCaptured=0;
    this->m_nFrmProcessed=0;

    this->m_bDrawRect=false;

    this->m_llObjDistance=new QLabel(this);
    this->m_llObjDistance->setAlignment(Qt::AlignCenter);
    this->m_llObjDistance->setText(tr("<font size=\"20\" color=\"red\">---</font>"));
    this->m_llObjDistance->setGeometry(QRect(200,0,80,44));
    this->m_timer=new QTimer;
    QObject::connect(this->m_timer,SIGNAL(timeout()),this,SLOT(ZSlotTimeout()));
}

ZMainUI::~ZMainUI()
{
    //    delete this->m_llTitle;

    //    for(qint32 i=0;i<5;i++)
    //    {
    //        delete this->m_tbLeft[i];
    //    }
    //    delete this->m_vLayoutTbLeft;
    //    delete this->m_llNum;
    //    delete this->m_hLayoutCenter;

    //    for(qint32 i=0;i<5;i++)
    //    {
    //        delete this->m_llTips[i];
    //    }
    //    delete this->m_hLayoutTips;
    //    delete this->m_vLayout;
    delete this->m_timer;
    delete this->m_llObjDistance;
}
void ZMainUI::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(this->m_img.isNull())
    {
        return;
    }

    QImage img(800,600,QImage::Format_RGB888);
    QPainter p;
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(0,0,this->m_img);
    p.drawPixmap(0,0,img.width(),img.height(),QPixmap(":/images/background1.png"));

#if 0
    //draw the track box image.
    if(!this->m_imgTrackBox.isNull())
    {
        QRectF rectTrackBox(img.width()/2-100/2,0,100,100);
        p.drawImage(rectTrackBox,this->m_imgTrackBox);
        QBrush oldBrush=p.brush();
        p.setBrush(QBrush(Qt::red));
        p.drawEllipse(rectTrackBox.center(),5,5);
        p.setBrush(oldBrush);
    }
    //////////////////////////////////////////////////////////////////////////////////
    //draw the top part.
    qint32 nBlankFromCenterPixel=120;
    qint32 nTopPartWidth=160;
    qint32 nTopPartHeight=60;
    qint32 nQingXie=50;
    QPointF ptCenterTop(img.width()/2,10);
    qint32 nSmallPartHeight=10;

    //draw the top part (left).
    QPointF ptTopRight1(ptCenterTop.x()-nBlankFromCenterPixel,ptCenterTop.y());
    QPointF ptTopLeft1(ptTopRight1.x()-nTopPartWidth,ptTopRight1.y());
    QPointF ptBottomRigh1(ptTopRight1.x()-nQingXie,ptCenterTop.y()+nTopPartHeight);
    QPointF ptBottomLeft1(ptBottomRigh1.x()-nTopPartWidth,ptBottomRigh1.y());
    QPainterPath pathTopPartLeft;
    pathTopPartLeft.moveTo(ptTopLeft1);
    pathTopPartLeft.lineTo(ptTopRight1);
    pathTopPartLeft.lineTo(ptBottomRigh1);
    pathTopPartLeft.lineTo(ptBottomLeft1);
    pathTopPartLeft.lineTo(ptTopLeft1);
    p.fillPath(pathTopPartLeft,QBrush(QColor(0,0,0,255)));

    {
        QPen nPenBackup=p.pen();
        p.setPen(QPen(Qt::red,2));
        QRectF rectRangeYard(ptTopLeft1.x(),ptTopLeft1.y(),ptBottomRigh1.x()-ptTopLeft1.x(),nTopPartHeight);
        //p.drawRect(rectRangeYard);
        QRectF rectRangeYardTips(ptTopLeft1.x(),ptTopLeft1.y(),rectRangeYard.width()/2,nTopPartHeight);
        QRectF rectRangeYardDigit(ptTopLeft1.x()+rectRangeYardTips.width(),ptTopLeft1.y(),rectRangeYardTips.width(),rectRangeYardTips.height());
        QFont nFont=p.font();
        nFont.setPointSize(14);
        p.setFont(nFont);
        p.setPen(QPen(Qt::white,4));
        p.drawText(rectRangeYardTips,QObject::tr("目标\n距离"));

        p.setPen(QPen(Qt::red,6));
        nFont.setPointSize(14);
        p.setFont(nFont);
        p.drawText(rectRangeYardDigit,QObject::tr("1024"));
        p.setPen(nPenBackup);
    }

    //draw small part (left).
    QPointF ptSmallPartBottomLeft(0,ptBottomLeft1.y());
    QPointF ptSmallPartTopLeft(0,ptSmallPartBottomLeft.y()-nSmallPartHeight);
    QPointF ptSmallPartBottomRight(ptBottomLeft1.x()+20,ptBottomLeft1.y());//here we move by 20 to cover background.
    QPointF ptSmallPartTopRight(ptBottomLeft1.x()+20,ptSmallPartBottomRight.y()-nSmallPartHeight);//here we move by 20 to cover background.
    QPainterPath pathTopSmallPartLeft;
    pathTopSmallPartLeft.moveTo(ptSmallPartTopLeft);
    pathTopSmallPartLeft.lineTo(ptSmallPartTopRight);
    pathTopSmallPartLeft.lineTo(ptSmallPartBottomRight);
    pathTopSmallPartLeft.lineTo(ptSmallPartBottomLeft);
    pathTopSmallPartLeft.lineTo(ptSmallPartTopLeft);
    p.fillPath(pathTopSmallPartLeft,QBrush(QColor(0,0,0,255)));

    //draw top part (right).
    QPointF ptRTopLeft1(ptCenterTop.x()+nBlankFromCenterPixel,ptCenterTop.y());
    QPointF ptRTopRight1(ptRTopLeft1.x()+nTopPartWidth,ptRTopLeft1.y());
    QPointF ptRBottomLeft1(ptRTopLeft1.x()+nQingXie,ptCenterTop.y()+nTopPartHeight);
    QPointF ptRBottomRigh1(ptRBottomLeft1.x()+nTopPartWidth,ptRBottomLeft1.y());
    QPainterPath pathTopPartRight;
    pathTopPartRight.moveTo(ptRTopLeft1);
    pathTopPartRight.lineTo(ptRTopRight1);
    pathTopPartRight.lineTo(ptRBottomRigh1);
    pathTopPartRight.lineTo(ptRBottomLeft1);
    pathTopPartRight.lineTo(ptRTopLeft1);
    p.fillPath(pathTopPartRight,QBrush(QColor(0,0,0,255)));

    {
        //draw small part (right).
        QPointF ptRightBottom(this->width(),ptRBottomRigh1.y());
        QPointF ptLeftBottom(ptRBottomRigh1.x()-20,ptRBottomRigh1.y());//here we move by 20 to cover background.
        QPointF ptLeftTop(ptLeftBottom.x(),ptLeftBottom.y()-nSmallPartHeight);//here we move by 20 to cover background.
        QPointF ptRightTop(ptRightBottom.x(),ptRightBottom.y()-nSmallPartHeight);//here we move by 20 to cover background.
        QPainterPath pathSmallPart;
        pathSmallPart.moveTo(ptLeftTop);
        pathSmallPart.lineTo(ptRightTop);
        pathSmallPart.lineTo(ptRightBottom);
        pathSmallPart.lineTo(ptLeftBottom);
        pathSmallPart.lineTo(ptLeftTop);
        p.fillPath(pathSmallPart,QBrush(QColor(0,0,0,255)));
    }

    {
        QPen nPenBackup=p.pen();
        p.setPen(QPen(Qt::red,2));
        QRectF rectWindSpeed(ptRBottomLeft1.x(),ptRBottomLeft1.y()-nTopPartHeight,ptRTopRight1.x()-ptRBottomLeft1.x(),nTopPartHeight);
        //p.drawRect(rectWindSpeed);
        QRectF rectWindSpeedDigit(rectWindSpeed.x(),rectWindSpeed.y(),rectWindSpeed.width()/2,rectWindSpeed.height());
        QRectF rectWindSpeedTips(rectWindSpeedDigit.x()+rectWindSpeed.width()/2,rectWindSpeed.y(),rectWindSpeed.width()/2,rectWindSpeed.height());

        QFont nFont=p.font();
        nFont.setPointSize(14);
        p.setFont(nFont);

        p.setPen(QPen(Qt::red,6));
        nFont.setPointSize(14);
        p.setFont(nFont);
        p.drawText(rectWindSpeedDigit,QObject::tr("35"));

        p.setPen(QPen(Qt::white,4));
        p.drawText(rectWindSpeedTips,QObject::tr("风速\nMPH"));


        p.setPen(nPenBackup);
    }

    ////////////////////////////////////////////////////////////////////////////
    //draw the bottom part.
    qint32 nBottomLongerThanTop=80;
    qint32 nBtmPartHeight=30;
    qint32 nBtmPartWidth=img.width()/3;
    QPointF ptBtmCenter(img.width()/2,img.height());
    QPointF ptBtmTopLeft(ptBtmCenter.x()-nBtmPartWidth,ptBtmCenter.y()-nBtmPartHeight);
    QPointF ptBtmBottomLeft(ptBtmCenter.x()-nBtmPartWidth-nBottomLongerThanTop,ptBtmCenter.y());
    QPointF ptBtmTopRight(ptBtmCenter.x()+nBtmPartWidth,ptBtmCenter.y()-nBtmPartHeight);
    QPointF ptBtmBottomRight(ptBtmCenter.x()+nBtmPartWidth+nBottomLongerThanTop,ptBtmCenter.y());
    QPainterPath pathBtmPart;
    pathBtmPart.moveTo(ptBtmTopLeft);
    pathBtmPart.lineTo(ptBtmBottomLeft);
    pathBtmPart.lineTo(ptBtmBottomRight);
    pathBtmPart.lineTo(ptBtmTopRight);
    pathBtmPart.lineTo(ptBtmTopLeft);
    p.fillPath(pathBtmPart,QBrush(QColor(0,0,0,128)));
    //draw the bottom part2.
    qint32 nBottomLongerThanTop2=80;
    qint32 nBtmPartHeight2=40;
    qint32 nBtmPartWidth2=img.width()/5;
    QPointF ptBtmTopLeft2(ptBtmCenter.x()-nBtmPartWidth2,ptBtmCenter.y()-nBtmPartHeight2);
    QPointF ptBtmBottomLeft2(ptBtmCenter.x()-nBtmPartWidth2-nBottomLongerThanTop2,ptBtmCenter.y());
    QPointF ptBtmTopRight2(ptBtmCenter.x()+nBtmPartWidth2,ptBtmCenter.y()-nBtmPartHeight2);
    QPointF ptBtmBottomRight2(ptBtmCenter.x()+nBtmPartWidth2+nBottomLongerThanTop2,ptBtmCenter.y());
    QPainterPath pathBtmPart2;
    pathBtmPart2.moveTo(ptBtmTopLeft2);
    pathBtmPart2.lineTo(ptBtmBottomLeft2);
    pathBtmPart2.lineTo(ptBtmBottomRight2);
    pathBtmPart2.lineTo(ptBtmTopRight2);
    pathBtmPart2.lineTo(ptBtmTopLeft2);
    p.fillPath(pathBtmPart2,QBrush(QColor(0,0,0,255)));


    //draw the left center marker.
    {
        QPointF ptMid(0,img.height()/2);
        QPointF ptLeftTop(ptMid.x(),ptMid.y()-5);
        QPointF ptLeftBtm(ptMid.x(),ptMid.y()+5);
        QPointF ptRightTop(ptLeftTop.x()+20,ptLeftTop.y());
        QPointF ptRightBtm(ptLeftBtm.x()+20,ptLeftBtm.y());
        QPointF ptEnd(ptRightTop.x()+10,ptMid.y());
        QPainterPath pathMarker;
        pathMarker.moveTo(ptLeftTop);
        pathMarker.lineTo(ptRightTop);
        pathMarker.lineTo(ptEnd);
        pathMarker.lineTo(ptRightBtm);
        pathMarker.lineTo(ptLeftBtm);
        pathMarker.lineTo(ptLeftTop);
        p.fillPath(pathMarker,QBrush(Qt::blue));
    }
    //draw the right center marker.
    {
        QPointF ptMid(img.width(),img.height()/2);
        QPointF ptRightTop(ptMid.x(),ptMid.y()-5);
        QPointF ptRightBtm(ptMid.x(),ptMid.y()+5);

        QPointF ptLeftTop(ptRightTop.x()-20,ptRightTop.y());
        QPointF ptLeftBtm(ptRightBtm.x()-20,ptRightBtm.y());

        QPointF ptEnd(ptLeftTop.x()-10,ptMid.y());

        QPainterPath pathMarker;
        pathMarker.moveTo(ptEnd);
        pathMarker.lineTo(ptLeftTop);
        pathMarker.lineTo(ptRightTop);
        pathMarker.lineTo(ptRightBtm);
        pathMarker.lineTo(ptLeftBtm);
        pathMarker.lineTo(ptEnd);
        p.fillPath(pathMarker,QBrush(Qt::blue));
    }
#endif
    //draw the right center marker.

    switch(g_GblHelp.m_nTrackingState)
    {
    case  STATE_TRACKING_STOP:
    {
        //in stop state,we draw a rectangle locate around the center point.
        //        QPointF ptCenter(img.width()/2,img.height()/2);
        //        QPointF ptLeftTop(ptCenter.x()-TRACK_BOX_W/2,ptCenter.y()-TRACK_BOX_H/2);
        //        p.setPen(QPen(Qt::green,3));
        //        p.drawRect(QRectF(ptLeftTop,QSizeF(TRACK_BOX_W,TRACK_BOX_H)));
        p.save();
        p.translate(img.width()/2,img.height()/2);
        p.setPen(QPen(Qt::green,4));
        p.drawLine(QPointF(70,-50),QPointF(100,-50));
        p.drawLine(QPointF(100,-50),QPointF(100,-20));
        //
        p.drawLine(QPointF(70,50),QPointF(100,50));
        p.drawLine(QPointF(100,50),QPointF(100,20));
        //
        p.drawLine(QPointF(-70,-50),QPointF(-100,-50));
        p.drawLine(QPointF(-100,-50),QPointF(-100,-20));
        //
        p.drawLine(QPointF(-70,50),QPointF(-100,50));
        p.drawLine(QPointF(-100,50),QPointF(-100,20));
        p.restore();

        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0,255,0,50)));
        QPointF ptCenter(img.width()/2,img.height()/2);
        p.drawEllipse(ptCenter,TRACK_BOX_W/3,TRACK_BOX_H/3);
        p.setBrush(QBrush(QColor(255,0,0)));
        p.drawEllipse(ptCenter,3,3);


        this->m_timer->stop();
        for(qint32 i=0;i<4;i++)
        {
            g_GblHelp.m_bFlashIndicator[i]=false;
        }

        //draw the object distance.
        //        {
        //            p.setPen(QPen(Qt::red,3));
        //            p.setBrush(Qt::NoBrush);
        //            QFont nFontBackup=p.font();
        //            QFont nFont=p.font();
        //            nFont.setPixelSize(30);
        //            p.setFont(nFont);
        //            QString strObjDistance("---");
        //            QRectF rectObjDistance(200,0,80,44);
        //            //p.drawRect(rectObjDistance);
        //            qreal nPosX=rectObjDistance.x()+(rectObjDistance.width()-p.fontMetrics().width(strObjDistance))/2;
        //            qreal nPosY=rectObjDistance.y()+rectObjDistance.height()/2;
        //            //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
        //            p.drawText(QPointF(nPosX,nPosY),strObjDistance);
        //            p.setFont(nFontBackup);
        //        }
        //draw the wind speed.
        {
            p.setPen(QPen(Qt::red,3));
            p.setBrush(Qt::NoBrush);
            QFont nFontBackup=p.font();
            QFont nFont=p.font();
            nFont.setPixelSize(30);
            p.setFont(nFont);
            QString strWindSpeed("---");
            QRectF rectWindSpeed(520,0,80,44);
            //p.drawRect(rectObjDistance);
            qreal nPosX=rectWindSpeed.x()+(rectWindSpeed.width()-p.fontMetrics().width(strWindSpeed))/2;
            qreal nPosY=rectWindSpeed.y()+rectWindSpeed.height()/2;
            //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
            p.drawText(QPointF(nPosX,nPosY),strWindSpeed);
            p.setFont(nFontBackup);
        }
    }
        break;
        //    case STATE_TRACKING_DRAWOBJ:
        //    {
        //        //p.setPen(QPen(Qt::red,6));
        //        //draw a rectangle.
        //        //        qreal fBoxWidth=TRACK_BOX_W*g_GblHelp.m_fImg2ScreenWScale;
        //        //        qreal fBoxHeight=TRACK_BOX_H*g_GblHelp.m_fImg2ScreenHScale;
        //        //        qreal fBoxX=(CAP_IMG_SIZE_W-TRACK_BOX_W)/2*g_GblHelp.m_fImg2ScreenWScale;
        //        //        qreal fBoxY=(CAP_IMG_SIZE_H-TRACK_BOX_H)/2*g_GblHelp.m_fImg2ScreenHScale;
        //        //        QRectF rectBox(fBoxX,fBoxY,fBoxWidth,fBoxHeight);
        //        //        p.drawRect(rectBox);
        //        if(this->m_bDrawRect)
        //        {
        //            QRectF rectBox(g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
        //            p.setPen(QPen(Qt::red,6));
        //            p.drawRect(rectBox);
        //            qDebug()<<"drawBox:"<<rectBox;
        //        }
        //    }
        //        break;
    case STATE_TRACKING_START:
    {
        //draw the cross marker.
        //draw the center cross marker.
        QPoint ptCenter(img.width()/2,img.height()/2);

        QBrush oldBrush=p.brush();
        p.setBrush(QBrush(Qt::red));
        p.drawEllipse(ptCenter,5,5);
        p.setBrush(oldBrush);

        QSizeF size1(40,40);
        QRectF rect1(QPointF(ptCenter.x()-size1.width()/2,ptCenter.y()-size1.height()/2),size1);

        QSizeF size2(100,100);
        QRectF rect2(QPointF(ptCenter.x()-size2.width()/2,ptCenter.y()-size2.height()/2),size2);

        QSizeF size3(200,200);
        QRectF rect3(QPointF(ptCenter.x()-size3.width()/2,ptCenter.y()-size3.height()/2),size3);

        ///////////////////////////////////////////////////////
        p.setPen(QPen(Qt::red,10));
        //draw the left-top part.
        p.drawLine(rect3.topLeft(),rect2.topLeft());
        //draw the right-top part.
        p.drawLine(rect3.topRight(),rect2.topRight());
        //draw the left-bottom part.
        p.drawLine(rect3.bottomLeft(),rect2.bottomLeft());
        //draw the right-bottom part.
        p.drawLine(rect3.bottomRight(),rect2.bottomRight());

        //////////////////////////////////////////////////////////////
        p.setPen(QPen(Qt::red,4));
        //draw the left-top part.
        p.drawLine(rect2.topLeft(),rect1.topLeft());
        //draw the right-top part.
        p.drawLine(rect2.topRight(),rect1.topRight());
        //draw the left-bottom part.
        p.drawLine(rect2.bottomLeft(),rect1.bottomLeft());
        //draw the right-bottom part.
        p.drawLine(rect2.bottomRight(),rect1.bottomRight());

        //draw the tracked object.
        p.setPen(QPen(Qt::green,1));
        //        //CSK.
        //        QRectF rectBox(g_GblHelp.m_rectTracked.x(),g_GblHelp.m_rectTracked.y(),g_GblHelp.m_rectTracked.width(),g_GblHelp.m_rectTracked.height());
        //        p.drawRect(rectBox);
#if 1
        for(qint32 i=0;i<g_GblHelp.m_vecActivePoints.size();i++)
        {
            qreal nX=g_GblHelp.m_vecActivePoints.at(i).x();
            qreal nY=g_GblHelp.m_vecActivePoints.at(i).y();
            p.drawEllipse(QPointF(nX,nY),2,2);
        }
        for(qint32 i=0;i<4;i++)
        {
            QPointF pt1,pt2;
            pt1.setX(g_GblHelp.m_lines[i].x1());
            pt1.setY(g_GblHelp.m_lines[i].y1());
            pt2.setX(g_GblHelp.m_lines[i].x2());
            pt2.setY(g_GblHelp.m_lines[i].y2());
            p.drawLine(pt1,pt2);
        }

#endif
        //draw the object distance.
        //        {
        //            p.setPen(QPen(Qt::red,3));
        //            p.setBrush(Qt::NoBrush);
        //            QFont nFontBackup=p.font();
        //            QFont nFont=p.font();
        //            nFont.setPixelSize(24);
        //            p.setFont(nFont);
        //            QString strObjDistance=QString::number(g_GblHelp.m_nObjDistance,10);
        //            QRectF rectObjDistance(200,0,80,44);
        //            //p.drawRect(rectObjDistance);
        //            qreal nPosX=rectObjDistance.x()+(rectObjDistance.width()-p.fontMetrics().width(strObjDistance))/2;
        //            qreal nPosY=rectObjDistance.y()+rectObjDistance.height()/2;
        //            //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
        //            p.drawText(QPointF(nPosX,nPosY),strObjDistance);
        //            p.setFont(nFontBackup);
        //        }
        this->m_llObjDistance->setText(QString("<font size=\"20\" color=\"red\">%1</font>").arg(g_GblHelp.m_nObjDistance));

        //draw the wind speed.
        {
            p.setPen(QPen(Qt::red,3));
            p.setBrush(Qt::NoBrush);
            QFont nFontBackup=p.font();
            QFont nFont=p.font();
            nFont.setPixelSize(30);
            p.setFont(nFont);
            QString strWindSpeed=QString::number(g_GblHelp.m_nWindSpeed,10);
            QRectF rectWindSpeed(520,0,80,44);
            //p.drawRect(rectObjDistance);
            qreal nPosX=rectWindSpeed.x()+(rectWindSpeed.width()-p.fontMetrics().width(strWindSpeed))/2;
            qreal nPosY=rectWindSpeed.y()+rectWindSpeed.height()/2;
            //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
            p.drawText(QPointF(nPosX,nPosY),strWindSpeed);
            p.setFont(nFontBackup);
        }

        //draw the top human-find indicator.
        if(g_GblHelp.m_bFlashIndicator[0])
        {
            QRectF rectTopIndicator(380,0,40,50);
            QPoint ptTop[3];
            //top middle point.
            ptTop[0].setX(rectTopIndicator.x()+rectTopIndicator.width()/2);
            ptTop[0].setY(rectTopIndicator.y());
            //bottom left point.
            ptTop[1].setX(rectTopIndicator.x());
            ptTop[1].setY(rectTopIndicator.y()+rectTopIndicator.height());
            //bottom righ point.
            ptTop[2].setX(rectTopIndicator.x()+rectTopIndicator.width());
            ptTop[2].setY(rectTopIndicator.y()+rectTopIndicator.height());
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(Qt::red));
            p.drawConvexPolygon(ptTop,3);
            p.drawPixmap(rectTopIndicator.x()+10,rectTopIndicator.y()+20,rectTopIndicator.width()-20,rectTopIndicator.height()-20,QPixmap(":/images/man_top.png"));
        }
        //draw the bottom human-find indicator.
        if(g_GblHelp.m_bFlashIndicator[1])
        {
            QRectF rectBtmIndicator(380,480,40,50);
            QPoint ptBtm[3];
            //top left point.
            ptBtm[0].setX(rectBtmIndicator.x());
            ptBtm[0].setY(rectBtmIndicator.y());
            //top right point.
            ptBtm[1].setX(rectBtmIndicator.x()+rectBtmIndicator.width());
            ptBtm[1].setY(rectBtmIndicator.y());
            //bottom middle point.
            ptBtm[2].setX(rectBtmIndicator.x()+rectBtmIndicator.width()/2);
            ptBtm[2].setY(rectBtmIndicator.y()+rectBtmIndicator.height());
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(Qt::red));
            p.drawConvexPolygon(ptBtm,3);
            p.drawPixmap(rectBtmIndicator.x()+10,rectBtmIndicator.y(),rectBtmIndicator.width()-20,rectBtmIndicator.height()-10,QPixmap(":/images/man_bottom.png"));
        }
        //draw the left human-find indicator.
        if(g_GblHelp.m_bFlashIndicator[2])
        {
            QRectF rectLeftIndicator(0,280,50,40);
            QPoint ptLeft[3];
            //left middle point.
            ptLeft[0].setX(rectLeftIndicator.x());
            ptLeft[0].setY(rectLeftIndicator.y()+rectLeftIndicator.height()/2);
            //right top point.
            ptLeft[1].setX(rectLeftIndicator.x()+rectLeftIndicator.width());
            ptLeft[1].setY(rectLeftIndicator.y());
            //right bottom point.
            ptLeft[2].setX(rectLeftIndicator.x()+rectLeftIndicator.width());
            ptLeft[2].setY(rectLeftIndicator.y()+rectLeftIndicator.height());
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(Qt::red));
            p.drawConvexPolygon(ptLeft,3);
            p.drawPixmap(rectLeftIndicator.x()+20,rectLeftIndicator.y()+10,rectLeftIndicator.width()-20,rectLeftIndicator.height()-20,QPixmap(":/images/man_left.png"));
        }
        //draw the right human-find indicator.
        if(g_GblHelp.m_bFlashIndicator[2])
        {
            QRectF rectRightIndicator(750,280,50,40);
            QPoint ptRight[3];
            //left top point.
            ptRight[0].setX(rectRightIndicator.x());
            ptRight[0].setY(rectRightIndicator.y());
            //left bottom point.
            ptRight[1].setX(rectRightIndicator.x());
            ptRight[1].setY(rectRightIndicator.y()+rectRightIndicator.height());
            //right middle point.
            ptRight[2].setX(rectRightIndicator.x()+rectRightIndicator.width());
            ptRight[2].setY(rectRightIndicator.y()+rectRightIndicator.height()/2);
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(Qt::red));
            p.drawConvexPolygon(ptRight,3);
            p.drawPixmap(rectRightIndicator.x(),rectRightIndicator.y()+10,rectRightIndicator.width()-20,rectRightIndicator.height()-20,QPixmap(":/images/man_right.png"));
        }

        if(!this->m_timer->isActive())
        {
            this->m_timer->start(250);
        }
    }
        break;
    case STATE_TRACKING_FAILED:
    {
        p.setPen(QPen(Qt::red,6));
        QFont nFont=p.font();
        nFont.setPixelSize(100);
        nFont.setPointSize(100);
        p.setFont(nFont);
        qint32 nFontW=p.fontMetrics().width("Lost");
        qint32 nFontH=p.fontMetrics().height();
        p.drawText(QRectF((this->width()-nFontW)/2,(this->height()-nFontH)/2,nFontW,nFontH),"Lost");
    }
        break;
    default:
        break;
    }

    //    //draw 4 rectangles on four corner to indicate image size.
    //    p.fillRect(QRect(0,0,20,20),Qt::yellow);
    //    p.fillRect(QRect(this->width()-20,0,20,20),Qt::yellow);
    //    p.fillRect(QRect(0,this->height()-20,20,20),Qt::yellow);
    //    p.fillRect(QRect(this->width()-20,this->height()-20,20,20),Qt::yellow);

    //    //draw box image.
    //    p.drawImage(QRect(0,0,300,300),this->m_imgBox);

    //    QFont nFont=p.font();
    //    nFont.setPointSize(50);
    //    p.setPen(QPen(Qt::white,4));
    //    QRectF rectTarget(10,10,160,80);
    //    p.drawText(rectTarget,tr("TARGET"));
    //    p.drawRect(rectTarget);

    ////////////////////////////////////////////////////////////////////////////////////
    //draw the right marker.
    p.save();
    p.translate(400,600);
    //draw the current position indicator.
    QPoint ptPosition[3];
    ptPosition[0]=QPoint(-10,-140);
    ptPosition[1]=QPoint(10,-140);
    ptPosition[2]=QPoint(0,-130);
    p.setBrush(QBrush(Qt::blue));
    p.setPen(Qt::NoPen);
    p.drawConvexPolygon(ptPosition,3);
    p.setBrush(Qt::NoBrush);

    //0,(5),10,(15),20,(25),30,(35),40,(45),50.
    //so num gap=10,big gap=5,small gap=5,90 degreen/10=9 degreen.
    for(qint32 i=0;i<10;i++)
    {
        if(i%2==0)
        {
            p.setPen(QPen(Qt::green,3));
            p.drawLine(QPointF(0,-120),QPointF(0,-100));
            p.drawText(QPointF(-5,-80),QString::number(i*10));
        }else{
            p.setPen(QPen(Qt::green,2));
            p.drawLine(QPointF(0,-120),QPointF(0,-110));
        }
        p.rotate(9);
    }
    p.restore();

    //draw the left marker.
    p.save();
    p.translate(400,600);
    for(qint32 i=0;i<10;i++)
    {
        if(i%2==0)
        {
            p.setPen(QPen(Qt::green,3));
            p.drawLine(QPointF(0,-120),QPointF(0,-100));
            p.drawText(QPointF(-5,-80),QString::number(i*10));
        }else{
            p.setPen(QPen(Qt::green,2));
            p.drawLine(QPointF(0,-120),QPointF(0,-110));
        }
        p.rotate(-9);
    }
    p.restore();



    //draw the right-bottom indicator.
    p.save();
    p.translate(400,300);

    //draw the current position indicator.
    QPoint ptPosition2[3];
    ptPosition2[0]=QPoint(340,0);
    ptPosition2[1]=QPoint(350,-10);
    ptPosition2[2]=QPoint(350,10);
    p.setBrush(QBrush(Qt::blue));
    p.setPen(Qt::NoPen);
    p.drawConvexPolygon(ptPosition2,3);
    p.setBrush(Qt::NoBrush);

    p.setPen(QPen(Qt::green,3));
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"0");
    p.rotate(6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"10");
    p.rotate(6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"20");
    p.rotate(6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"30");
    p.rotate(6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"40");
    p.restore();
    //draw the right-top indicator.
    p.save();
    p.translate(400,300);
    p.setPen(QPen(Qt::green,3));
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"0");
    p.rotate(-6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"10");
    p.rotate(-6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"20");
    p.rotate(-6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"30");
    p.rotate(-6);
    p.drawLine(QPointF(330,0),QPointF(320,0));
    p.drawText(QPointF(300,0),"40");
    p.restore();


    //draw the left-bottom indicator.
    p.save();
    p.translate(400,300);

    //draw the current position indicator.
    QPoint ptPosition3[3];
    ptPosition3[0]=QPoint(-340,0);
    ptPosition3[1]=QPoint(-350,-10);
    ptPosition3[2]=QPoint(-350,10);
    p.setBrush(QBrush(Qt::blue));
    p.setPen(Qt::NoPen);
    p.drawConvexPolygon(ptPosition3,3);
    p.setBrush(Qt::NoBrush);

    p.setPen(QPen(Qt::green,3));
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"0");
    p.rotate(-6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"10");
    p.rotate(-6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"20");
    p.rotate(-6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"30");
    p.rotate(-6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"40");
    p.restore();
    //draw the left-top indicator.
    p.save();
    p.translate(400,300);
    p.setPen(QPen(Qt::green,3));
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"0");
    p.rotate(6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"10");
    p.rotate(6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"20");
    p.rotate(6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"30");
    p.rotate(6);
    p.drawLine(QPointF(-330,0),QPointF(-320,0));
    p.drawText(QPointF(-310,0),"40");
    p.restore();

    //temperature.
    p.setPen(QPen(Qt::red,3));
    p.setBrush(Qt::NoBrush);
    QFont nFont=p.font();
    nFont.setPixelSize(18);
    p.setFont(nFont);

    {
        QString strTemp("21.6");
        QRectF rectTemp(230,550,50,40);
        //p.drawRect(rectTemp);
        qreal nPosX=rectTemp.x()+(rectTemp.width()-p.fontMetrics().width(strTemp))/2;
        qreal nPosY=rectTemp.y()+rectTemp.height()/2;
        //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
        p.drawText(QPointF(nPosX,nPosY),strTemp);
    }
    {
        QString strMpa("0.1");
        QRectF rectMpa(520,550,50,40);
        //p.drawRect(rectMpa);
        qreal nPosX=rectMpa.x()+(rectMpa.width()-p.fontMetrics().width(strMpa))/2;
        qreal nPosY=rectMpa.y()+rectMpa.height()/2;
        //qDebug()<<"nPosX:"<<nPosX<<",nPosY:"<<nPosY;
        p.drawText(QPointF(nPosX,nPosY),strMpa);
    }

    p.end();

    QPainter painter(this);
    painter.drawImage(QRect(0,0,this->width(),this->height()),img);
}
#if 1
void ZMainUI::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_F1)
    {
        qDebug()<<"key pressed.";
        //exchange state to next state.
        switch(g_GblHelp.m_nTrackingState)
        {
        case STATE_TRACKING_STOP:
            //reset counter.
            this->m_nFrmCaptured=0;
            this->m_nFrmProcessed=0;

            //enter next state.
            g_GblHelp.m_nTrackingState=STATE_TRACKING_START;
            //generate the distance object animation.
        {
            QPropertyAnimation *animation=new QPropertyAnimation(this->m_llObjDistance,"geometry");
            animation->setDuration(1000);
            animation->setStartValue(QRect(400,300,80,44));
            animation->setEndValue(QRect(200,0,80,44));
            animation->setEasingCurve(QEasingCurve::InOutSine);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
            break;
        case STATE_TRACKING_START:
            //enter next state.
            g_GblHelp.m_nTrackingState=STATE_TRACKING_STOP;
            break;
        case STATE_TRACKING_FAILED:
            //stop when tracking failed.wait for next user trigger.
            //enter stop state.
            g_GblHelp.m_nTrackingState=STATE_TRACKING_STOP;
        default:
            break;
        }
        //        this->m_llTitle->setText(g_GblHelp.ZGetTipsByState(g_GblHelp.m_nTrackingState));
        this->update();
    }
    QWidget::keyPressEvent(event);
}
void ZMainUI::keyReleaseEvent(QKeyEvent *event)
{

    if(event->key()==Qt::Key_F1)
    {
        qDebug()<<"key released.";
    }
    QWidget::keyReleaseEvent(event);
}
#endif

#if 0
void ZMainUI::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
    case Qt::LeftButton:
        //remember the start point.
        this->m_ptStart=event->pos();
        //Stop->DrawObj.
        g_GblHelp.m_nTrackingState=STATE_TRACKING_DRAWOBJ;
        break;
    case Qt::RightButton:
        //stop.
        g_GblHelp.m_nTrackingState=STATE_TRACKING_STOP;
        break;
    default:
        break;
    }
    this->m_llTitle->setText(g_GblHelp.ZGetTipsByState(g_GblHelp.m_nTrackingState));
    QWidget::mousePressEvent(event);
}
void ZMainUI::mouseMoveEvent(QMouseEvent *event)
{
    this->m_ptEnd=event->pos();
    //end point is on the right side of start point.
    if(this->m_ptEnd.x()>this->m_ptStart.x())
    {
        g_GblHelp.m_nBoxX=this->m_ptStart.x();
        g_GblHelp.m_nBoxY=this->m_ptStart.y();
        g_GblHelp.m_nBoxWidth=this->m_ptEnd.x()-this->m_ptStart.x();
        g_GblHelp.m_nBoxHeight=this->m_ptEnd.y()-this->m_ptStart.y();
    }else{
        g_GblHelp.m_nBoxX=this->m_ptEnd.x();
        g_GblHelp.m_nBoxY=this->m_ptEnd.y();
        g_GblHelp.m_nBoxWidth=this->m_ptEnd.x()-this->m_ptStart.x();
        g_GblHelp.m_nBoxHeight=this->m_ptEnd.y()-this->m_ptStart.y();
    }
    this->m_bDrawRect=true;
    this->update();
    QWidget::mouseMoveEvent(event);
}
void ZMainUI::mouseReleaseEvent(QMouseEvent *event)
{
    if(STATE_TRACKING_DRAWOBJ==g_GblHelp.m_nTrackingState)
    {
        this->m_bDrawRect=false;
        this->update();
        //drawobj->start.
        g_GblHelp.m_nTrackingState=STATE_TRACKING_START;
        this->m_llTitle->setText(g_GblHelp.ZGetTipsByState(g_GblHelp.m_nTrackingState));
    }
    QWidget::mouseReleaseEvent(event);
}
#endif
void ZMainUI::ZSlotUptImg(const QImage &img)
{
    //    if(g_GblHelp.m_nTrackingState==STATE_TRACKING_START)
    //    {
    //        this->m_nFrmCaptured++;
    //        this->m_llNum->setText(tr("%1/%2").arg(this->m_nFrmCaptured).arg(this->m_nFrmProcessed));
    //    }

    //    this->m_llTips[3]->setText(QString::number(this->m_nFrmCnt++,10));
    //    this->m_llTips[4]->setText(tr("(%1,%2)").arg(g_GblHelp.m_rectTracked.x()).arg(g_GblHelp.m_rectTracked.y()));
    this->m_img=img;
    this->update();
}
void ZMainUI::ZSlotUptTrackBoxImg(const QImage &img)
{
    this->m_imgTrackBox=img;
}
void ZMainUI::ZSlotUptProcessed()
{
    this->m_nFrmProcessed++;
}
void ZMainUI::ZSlotTimeout()
{
    for(qint32 i=0;i<4;i++)
    {
        g_GblHelp.m_bFlashIndicator[i]=!g_GblHelp.m_bFlashIndicator[i];
    }
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    g_GblHelp.m_nObjDistance=200+qrand()%9;
    g_GblHelp.m_nWindSpeed=10+qrand()%5;
}
