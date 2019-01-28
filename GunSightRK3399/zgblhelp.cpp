#include "zgblhelp.h"
#include <QObject>
#include <QDebug>
ZGblHelp::ZGblHelp()
{
    this->m_bExitFlag=false;

    this->m_nTrackingState=STATE_TRACKING_STOP;

    this->m_bWrYuv2File=false;

    this->m_nBoxWidth=TRACK_BOX_W;
    this->m_nBoxHeight=TRACK_BOX_H;
    this->m_nBoxX=(CAP_IMG_SIZE_W-this->m_nBoxWidth)/2;
    this->m_nBoxY=(CAP_IMG_SIZE_H-this->m_nBoxHeight)/2;

    for(qint32 i=0;i<4;i++)
    {
        this->m_bFlashIndicator[i]=false;
    }
    this->m_nObjDistance=0;
    this->m_nWindSpeed=0;
}
qreal ZGblHelp::ZMapImgX2ScreenX(qreal x)
{
    return (x*this->m_fImg2ScreenWScale);
}
qreal ZGblHelp::ZMapImgY2ScreenY(qreal y)
{
    return (y*this->m_fImg2ScreenHScale);
}
qreal ZGblHelp::ZMapImgWidth2ScreenWidth(qreal width)
{
    return (width*this->m_fImg2ScreenWScale);
}
qreal ZGblHelp::ZMapImgHeight2ScreenHeight(qreal height)
{
    return (height*this->m_fImg2ScreenHScale);
}

qreal ZGblHelp::ZMapScreenX2ImgX(qreal x)
{
    return (x/this->m_fImg2ScreenWScale);
}
qreal ZGblHelp::ZMapScreenY2ImgY(qreal y)
{
    return (y/this->m_fImg2ScreenHScale);
}
qreal ZGblHelp::ZMapScreenWidth2ImgWidth(qreal width)
{
    return (width/this->m_fImg2ScreenWScale);
}
qreal ZGblHelp::ZMapScreenHeight2ImgHeight(qreal height)
{
    return (height/this->m_fImg2ScreenHScale);
}
QString ZGblHelp::ZGetTipsByState(qint32 nState)
{
    QString strTips;
    switch(nState)
    {
    case STATE_TRACKING_STOP:
        strTips=QObject::tr("移动镜头使物件进入矩形框区域内\n按键触发追踪");
        break;
    case STATE_TRACKING_START:
        strTips=QObject::tr("正在追踪...\n按键取消追踪");
        break;
    case STATE_TRACKING_FAILED:
        strTips=QObject::tr("追踪丢失");
        break;
    default:
        break;
    }
    return strTips;
}
ZGblHelp g_GblHelp;
using namespace cv;
cv::Mat QImage2cvMat(const QImage &img)
{
    cv::Mat mat;
    //qDebug()<<"format:"<<img.format();
    switch(img.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat=cv::Mat(img.height(),img.width(),CV_8UC4,(void*)img.constBits(),img.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat=cv::Mat(img.height(),img.width(),CV_8UC3,(void*)img.constBits(),img.bytesPerLine());
        cv::cvtColor(mat,mat,CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat=cv::Mat(img.height(),img.width(),CV_8UC1,(void*)img.constBits(),img.bytesPerLine());
        break;
    default:
        break;
    }
    return mat;
}
QImage cvMat2QImage(const cv::Mat &mat)
{
    //8-bit unsigned ,number of channels=1.
    if(mat.type()==CV_8UC1)
    {
        QImage img(mat.cols,mat.rows,QImage::Format_Indexed8);
        //set the color table
        //used to translate colour indexes to qRgb values.
        img.setColorCount(256);
        for(qint32 i=0;i<256;i++)
        {
            img.setColor(i,qRgb(i,i,i));
        }
        //copy input mat.
        uchar *pSrc=mat.data;
        for(qint32 row=0;row<mat.rows;row++)
        {
            uchar *pDst=img.scanLine(row);
            memcpy(pDst,pSrc,mat.cols);
            pSrc+=mat.step;
        }
        return img;
    }else if(mat.type()==CV_8UC3)
    {
        //8-bits unsigned,number of channels=3.
        const uchar *pSrc=(const uchar*)mat.data;
        QImage img(pSrc,mat.cols,mat.rows,mat.step,QImage::Format_RGB888);
        return img.rgbSwapped();
    }else if(mat.type()==CV_8UC4)
    {
        const uchar *pSrc=(const uchar*)mat.data;
        QImage img(pSrc,mat.cols,mat.rows,mat.step,QImage::Format_ARGB32);
        return img.copy();
    }else{
        qDebug()<<"<Error>:failed to convert cvMat to QImage!";
        return QImage();
    }
}
