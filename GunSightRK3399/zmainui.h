#ifndef ZMAINUI_H
#define ZMAINUI_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPaintEvent>
#include <QImage>
#include <QTimer>
class ZMainUI : public QWidget
{
    Q_OBJECT

public:
    ZMainUI(QWidget *parent = 0);
    ~ZMainUI();

public slots:
    void ZSlotUptImg(const QImage &img);
    void ZSlotUptTrackBoxImg(const QImage &img);
    void ZSlotUptProcessed();
protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
private slots:
    void ZSlotTimeout();
private:
    quint64 m_nFrmCnt;
    quint64 m_nFrmCaptured;
    quint64 m_nFrmProcessed;
    QImage m_img;
    QImage m_imgTrackBox;

    QLabel *m_llObjDistance;
//private:
//    QPoint m_ptStart;
//    QPoint m_ptEnd;
private:
    bool m_bDrawRect;
private:
    QTimer *m_timer;
};

#endif // ZMAINUI_H
