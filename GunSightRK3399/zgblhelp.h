#ifndef ZGBLHELP_H
#define ZGBLHELP_H

#include <QString>
#include <QImage>
#include <QMutex>
#include <QWidget>
#include <QRect>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#define CAP_IMG_SIZE_W  720
#define CAP_IMG_SIZE_H  480

#define TRACK_BOX_W 100
#define TRACK_BOX_H 100

#define DUMP_CAM_INFO2FILE  1

#define THREAD_SCHEDULE_MS  10//100ms.

enum
{
    STATE_TRACKING_STOP=0,
    STATE_TRACKING_DRAWOBJ,
    STATE_TRACKING_START,
    STATE_TRACKING_FAILED,
};
class ZGblHelp
{
public:
    ZGblHelp();

public:
    bool m_bExitFlag;
public:
    qint32 m_nTrackingState;
public:
    bool m_bWrYuv2File;
public:
    //box size.
    qint32 m_nBoxX;
    qint32 m_nBoxY;
    qint32 m_nBoxWidth;
    qint32 m_nBoxHeight;

    QRectF m_rectTracked;

public:
    QWidget *m_mainWidget;
public:
    QString ZGetTipsByState(qint32 nState);

public:
    //CAM image to screen size scale.
    qreal m_fImg2ScreenWScale;
    qreal m_fImg2ScreenHScale;
    qreal ZMapImgX2ScreenX(qreal x);
    qreal ZMapImgY2ScreenY(qreal y);
    qreal ZMapImgWidth2ScreenWidth(qreal width);
    qreal ZMapImgHeight2ScreenHeight(qreal height);

    qreal ZMapScreenX2ImgX(qreal x);
    qreal ZMapScreenY2ImgY(qreal y);
    qreal ZMapScreenWidth2ImgWidth(qreal width);
    qreal ZMapScreenHeight2ImgHeight(qreal height);
};
extern ZGblHelp g_GblHelp;

extern cv::Mat QImage2cvMat(const QImage &img);
extern QImage cvMat2QImage(const cv::Mat &mat);
#endif // ZGBLHELP_H
