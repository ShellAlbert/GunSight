#include "zmainui.h"
#include <QPainter>
#include <QDebug>
#include "zgblhelp.h"
ZMainUI::ZMainUI(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    this->m_llTitle=new QLabel(g_GblHelp.ZGetTipsByState(STATE_TRACKING_STOP));
    this->m_llTitle->setStyleSheet("QLabel{background-color:yellow;color:red;font-size:40px;}");
    this->m_llTitle->setFixedWidth(800);
    this->m_llTitle->setScaledContents(true);
    this->m_llTitle->setAlignment(Qt::AlignCenter);


    this->m_vLayoutTbLeft=new QVBoxLayout;
    this->m_vLayoutTbLeft->setSpacing(20);
    this->m_vLayoutTbLeft->addStretch(1);
    for(qint32 i=0;i<5;i++)
    {
        QFont fontNow=this->font();
        fontNow.setPixelSize(40);
        this->m_tbLeft[i]=new QToolButton;
        this->m_tbLeft[i]->setFont(fontNow);
        switch(i)
        {
        case 0:
            this->m_tbLeft[i]->setText(tr("校准"));
            break;
        case 1:
            this->m_tbLeft[i]->setText(tr("设置"));
            break;
        case 2:
            this->m_tbLeft[i]->setText(tr("颜色"));
            break;
        case 3:
            this->m_tbLeft[i]->setText(tr("调整"));
            break;
        case 4:
            this->m_tbLeft[i]->setText(tr("输出"));
            break;
        default:
            break;
        }
        this->m_vLayoutTbLeft->addWidget(this->m_tbLeft[i]);
    }
    this->m_vLayoutTbLeft->addStretch(1);

    this->m_llNum=new QLabel(tr("0/0"));
    this->m_llNum->setStyleSheet("QLabel{color:yellow;font-size:100px;}");
    this->m_hLayoutCenter=new QHBoxLayout;
    this->m_hLayoutCenter->addLayout(this->m_vLayoutTbLeft);
    this->m_hLayoutCenter->addStretch(1);
    this->m_hLayoutCenter->addWidget(this->m_llNum);


    this->m_hLayoutTips=new QHBoxLayout;
    this->m_hLayoutTips->setSpacing(10);
    for(qint32 i=0;i<5;i++)
    {
        this->m_llTips[i]=new QLabel;
        switch(i)
        {
        case 0:
            this->m_llTips[i]->setText(tr("95B-1式"));
            this->m_llTips[i]->setStyleSheet("QLabel{color:blue;font-size:60px;}");
            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
            break;
        case 1:
            this->m_llTips[i]->setText(tr("弹种1"));
            this->m_llTips[i]->setStyleSheet("QLabel{color:red;font-size:60px;}");
            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
            break;
        case 2:
            this->m_llTips[i]->setText(tr("白热"));
            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
            break;
        case 3:
            this->m_llTips[i]->setText(tr("0"));
            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
            this->m_hLayoutTips->addWidget(this->m_llTips[i]);
            break;
        case 4:
            this->m_llTips[i]->setText(tr("(0,0)"));
            this->m_llTips[i]->setStyleSheet("QLabel{color:yellow;font-size:60px;}");
            break;
        default:
            break;
        }

    }
    this->m_hLayoutTips->addStretch(1);
    this->m_hLayoutTips->addWidget(this->m_llTips[4]);

    this->m_vLayout=new QVBoxLayout;
    this->m_vLayout->setContentsMargins(10,50,10,10);
    this->m_vLayout->addWidget(this->m_llTitle,0,Qt::AlignCenter);
    this->m_vLayout->addStretch(1);
    this->m_vLayout->addLayout(this->m_hLayoutCenter);
    this->m_vLayout->addStretch(1);
    this->m_vLayout->addLayout(this->m_hLayoutTips);
    this->setLayout(this->m_vLayout);
    this->m_nFrmCnt=0;

    this->m_nFrmCaptured=0;
    this->m_nFrmProcessed=0;

    this->m_bDrawRect=false;
}

ZMainUI::~ZMainUI()
{
    delete this->m_llTitle;

    for(qint32 i=0;i<5;i++)
    {
        delete this->m_tbLeft[i];
    }
    delete this->m_vLayoutTbLeft;
    delete this->m_llNum;
    delete this->m_hLayoutCenter;

    for(qint32 i=0;i<5;i++)
    {
        delete this->m_llTips[i];
    }
    delete this->m_hLayoutTips;
    delete this->m_vLayout;
}
void ZMainUI::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    //draw the image.
    p.drawImage(QRect(0,0,this->width(),this->height()),this->m_img);

    //draw the center cross marker.
    QPoint ptCenter(this->width()/2,this->height()/2);
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


    //draw the bottom part.
    qint32 nBottomLongerThanTop=100;
    qint32 nBtmPartHeight=100;
    qint32 nBtmPartWidth=this->width()/3;
    QPointF ptBtmCenter(this->width()/2,this->height());
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

    switch(g_GblHelp.m_nTrackingState)
    {
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
        p.setPen(QPen(Qt::green,6));
        //CSK.
        QRectF rectBox(g_GblHelp.m_rectTracked.x(),g_GblHelp.m_rectTracked.y(),g_GblHelp.m_rectTracked.width(),g_GblHelp.m_rectTracked.height());
        p.drawRect(rectBox);
#if 0
        for(qint32 i=0;i<g_GblHelp.m_vecActivePoints.size();i++)
        {
            qreal nX=g_GblHelp.m_vecActivePoints.at(i).x();
            qreal nY=g_GblHelp.m_vecActivePoints.at(i).y();
            p.drawEllipse(QPointF(g_GblHelp.ZMapImgX2ScreenX(nX),g_GblHelp.ZMapImgY2ScreenY(nY)),2,2);
        }
        for(qint32 i=0;i<4;i++)
        {
            QPointF pt1,pt2;
            pt1.setX(g_GblHelp.ZMapImgX2ScreenX(g_GblHelp.m_lines[i].x1()));
            pt1.setY(g_GblHelp.ZMapImgX2ScreenX(g_GblHelp.m_lines[i].y1()));
            pt2.setX(g_GblHelp.ZMapImgX2ScreenX(g_GblHelp.m_lines[i].x2()));
            pt2.setY(g_GblHelp.ZMapImgX2ScreenX(g_GblHelp.m_lines[i].y2()));
            p.drawLine(pt1,pt2);
        }
#endif
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

    //draw 4 rectangles on four corner to indicate image size.
    p.fillRect(QRect(0,0,20,20),Qt::yellow);
    p.fillRect(QRect(this->width()-20,0,20,20),Qt::yellow);
    p.fillRect(QRect(0,this->height()-20,20,20),Qt::yellow);
    p.fillRect(QRect(this->width()-20,this->height()-20,20,20),Qt::yellow);

    //draw box image.
    p.drawImage(QRect(0,0,300,300),this->m_imgBox);
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
        this->m_llTitle->setText(g_GblHelp.ZGetTipsByState(g_GblHelp.m_nTrackingState));
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
    if(g_GblHelp.m_nTrackingState==STATE_TRACKING_START)
    {
        this->m_nFrmCaptured++;
        this->m_llNum->setText(tr("%1/%2").arg(this->m_nFrmCaptured).arg(this->m_nFrmProcessed));
    }

    this->m_llTips[3]->setText(QString::number(this->m_nFrmCnt++,10));
    this->m_llTips[4]->setText(tr("(%1,%2)").arg(g_GblHelp.m_rectTracked.x()).arg(g_GblHelp.m_rectTracked.y()));
    this->m_img=img;
    this->update();
}
void ZMainUI::ZSlotUptBoxImg(const QImage &img)
{
    this->m_imgBox=img;
}
void ZMainUI::ZSlotUptProcessed()
{
    this->m_nFrmProcessed++;
}
