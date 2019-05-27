#ifndef ZIMGDISPLAYER_H
#define ZIMGDISPLAYER_H

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QToolButton>
#include <QTimer>
#include <QQueue>
#include <QSemaphore>
#include <zgblpara.h>
#include <zringbuffer.h>
#define IMG_SCALED_W    gGblPara.m_widthCAM1
#define IMG_SCALED_H    gGblPara.m_heightCAM1
class ZImgDisplayer : public QWidget
{
    Q_OBJECT
public:
    explicit ZImgDisplayer(qint32 nCenterX,qint32 nCenterY,bool bMainCamera=false,QWidget *parent = nullptr);
    ~ZImgDisplayer();
    void ZSetSensitiveRect(QRect rect);
    void ZSetPaintParameters(QColor colorRect);
protected:
    QSize sizeHint() const;
signals:

public slots:
    void ZSlotFlushImg(const QImage &img);
protected:
    void paintEvent(QPaintEvent *e);

private:
    QImage m_img;
    qint32 m_nCenterX,m_nCenterY;
    QRect m_rectSensitive;
    QColor m_colorRect;
    //camera parameters.
    QString m_camID;
    qint32 m_nCamWidth,m_nCamHeight,m_nCAMFps;

    //AM i the main camera?
    bool m_bMainCamera;
private:
    //QRect m_rectSensitive;
    qint32 m_nRatio;
    bool m_bStretchFlag;
private:
    //calculate the fps.
    qint64 m_nTotalTime;
    qint64 m_nTotalFrames;
    qint64 m_nLastTsMsec;//in millsecond.
};

#endif // ZIMGDISPLAYER_H
