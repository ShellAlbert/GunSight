#ifndef ZMAINUI_H
#define ZMAINUI_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPaintEvent>
#include <QImage>
class ZMainUI : public QWidget
{
    Q_OBJECT

public:
    ZMainUI(QWidget *parent = 0);
    ~ZMainUI();

public slots:
    void ZSlotUptImg(const QImage &img);
    void ZSlotUptBoxImg(const QImage &img);
    void ZSlotUptProcessed();
protected:
    void paintEvent(QPaintEvent *event);
//    void keyPressEvent(QKeyEvent *event);
//    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    QLabel *m_llTitle;

    QToolButton *m_tbLeft[5];
    QVBoxLayout *m_vLayoutTbLeft;
    QLabel *m_llNum;
    QHBoxLayout *m_hLayoutCenter;

    QLabel *m_llTips[5];
    QHBoxLayout *m_hLayoutTips;

    QVBoxLayout *m_vLayout;
private:
    quint64 m_nFrmCnt;
    quint64 m_nFrmCaptured;
    quint64 m_nFrmProcessed;
    QImage m_img;
    QImage m_imgBox;

private:
    QPoint m_ptStart;
    QPoint m_ptEnd;
private:
    bool m_bDrawRect;
};

#endif // ZMAINUI_H
