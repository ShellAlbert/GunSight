#include "ztrackthread.h"
#include "zgblhelp.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <QDebug>
#include <QDateTime>
#include <CSK_Tracker.h>

#include "CppMT/CMT.h"
#include "CppMT/gui.h"

//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>


using namespace std;
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
    ( std::ostringstream() << std::dec << x ) ).str()

ZTrackThread::ZTrackThread()
{
    this->m_bInitBox=false;
}
void ZTrackThread::ZBindRingBuffer(ZRingBuffer *rb)
{
    this->m_rb=rb;
}
void ZTrackThread::run()
{
    qint32 nBufSize=CAP_IMG_SIZE_W*CAP_IMG_SIZE_H*3;
    char *pRGBBuffer=new char[nBufSize];
    char *pBackupBuffer=new char[nBufSize];

#if 0
    cv::Ptr<cv::Tracker> tracker;
    while(!g_GblHelp.m_bExitFlag)
    {
        if(STATE_TRACKING_START==g_GblHelp.m_nTrackingState)
        {
            //1.fetch RGB data from ring buffer.
            if(this->m_rb->ZGetElement((qint8*)pRGBBuffer,nBufSize)<0)
            {
                qDebug()<<"<Warning>:process,failed to get RGB from RGB queue.";
                continue;
            }
            //2.build a cvMat from plain RGB buffer.
            cv::Mat matFrm=cv::Mat(CAP_IMG_SIZE_H,CAP_IMG_SIZE_W,CV_8UC3,pRGBBuffer);

            if(this->m_bInitBox)
            {
                qint64 tStart=QDateTime::currentDateTime().toMSecsSinceEpoch();
                // Update the tracking result
                cv::Rect2d boxNewLocate;
                bool result=tracker->update(matFrm,boxNewLocate);
                if(result)
                {
                    qDebug("new locate:(%.2f,%.2f,%.2f,%.2f)\n",boxNewLocate.x,boxNewLocate.y,boxNewLocate.width,boxNewLocate.height);
                    g_GblHelp.m_rectTracked=QRect(boxNewLocate.x,boxNewLocate.y,boxNewLocate.width,boxNewLocate.height);
                    g_GblHelp.m_nTrackingState=STATE_TRACKING_START;
                }else{
                    // Tracking failure detected.
                    qDebug()<<"Tracking failure detected.";
                    //                    tracker->~Tracker();
                    //                    tracker=NULL;
                    ////                    this->m_bInitBox=false;
                    ////                    g_GblHelp.m_nTrackingState=STATE_TRACKING_FAILED;
                    //                    this->usleep(1000*500);//500ms.
                    //                    cv::Mat matFrmBackup=cv::Mat(CAP_IMG_SIZE_H,CAP_IMG_SIZE_W,CV_8UC3,pBackupBuffer);
                    //                    tracker=cv::TrackerMOSSE::create();
                    //                    cv::Rect2d box(g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
                    //                    if(tracker->init(matFrmBackup,box))
                    //                    {
                    //                        qDebug()<<"ReInit tracker success.";
                    //                    }else{
                    //                        qDebug()<<"ReInit tracker failed.";
                    //                        break;
                    //                    }
                }
                qint64 tEnd=QDateTime::currentDateTime().toMSecsSinceEpoch();
                qDebug()<<"cost (ms):"<<(tEnd-tStart);
                emit this->ZSigNewTracking();
            }else{
                tracker=cv::TrackerMOSSE::create();
                //tracker=cv::TrackerKCF::create();CSRT
                qDebug()<<"track:cvMat size:"<<matFrm.cols<<","<<matFrm.rows;
                qDebug("track:box(%d,%d,%d,%d)\n",g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
                cv::Rect2d box(g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
                if(tracker->init(matFrm,box))
                {
                    qDebug()<<"Init tracker success.";
                }else{
                    qDebug()<<"Init tracker failed.";
                    break;
                }
                this->m_bInitBox=true;
                cv::Mat matBox=cv::Mat(matFrm,box);
                QImage imgBox=cvMat2QImage(matBox);
                emit this->ZSigNewInitBox(imgBox);

                //save the feature frames.
                memcpy(pBackupBuffer,pRGBBuffer,nBufSize);
            }
            this->usleep(THREAD_SCHEDULE_MS);
        }else{
            if(this->m_bInitBox)
            {
                qDebug()<<"Tracking cancel.";
                tracker->~Tracker();
                tracker=NULL;
                this->sleep(1);
                this->m_bInitBox=false;
            }
        }
        this->usleep(THREAD_SCHEDULE_MS*2);
    }
#endif

#if 1
    // Create a tracker
    CSK_Tracker tracker;
    // Define initial bounding box
    //Point ptCenter;
    //Rect2d bbox;
//    Size target_size = Size(60, 60);
    cv::Rect2d box;
    Point ptCenter;
    cv::Size target_size;
    while(!g_GblHelp.m_bExitFlag)
    {
        if(STATE_TRACKING_START==g_GblHelp.m_nTrackingState)
        {
            //1.fetch RGB data from ring buffer.
            if(this->m_rb->ZGetElement((qint8*)pRGBBuffer,nBufSize)<0)
            {
                qDebug()<<"<Warning>:process,failed to get RGB from RGB queue.";
                continue;
            }
            //2.build a cvMat from plain RGB buffer.
            cv::Mat matFrm=cv::Mat(CAP_IMG_SIZE_H,CAP_IMG_SIZE_W,CV_8UC3,pRGBBuffer);

            if(this->m_bInitBox)
            {
                //qint64 tStart=QDateTime::currentDateTime().toMSecsSinceEpoch();

                // Update the tracking result.
                bool result = tracker.tracker_update(matFrm,ptCenter,target_size);
                if(result)
                {
                    box.x = ptCenter.x - target_size.width/2;
                    box.y = ptCenter.y - target_size.height/2;
                    box.width = target_size.width;
                    box.height = target_size.height;


                    //to avoid reaching the boundary to cause openCV fault.
                    if((box.x+box.width)>matFrm.cols)
                    {
                        box.width=matFrm.cols-box.x-2;
                    }
                    if((box.y+box.height)>matFrm.rows)
                    {
                        box.height=matFrm.rows-box.y-2;
                    }
                    //display the tracked section img.
//                    cv::Mat matBox=cv::Mat(matFrm,box);
//                    QImage imgBox=cvMat2QImage(matBox);
//                    emit this->ZSigNewInitBox(imgBox);

                    g_GblHelp.m_rectTracked.setX(g_GblHelp.ZMapImgX2ScreenX(box.x));
                    g_GblHelp.m_rectTracked.setY(g_GblHelp.ZMapImgY2ScreenY(box.y));
                    g_GblHelp.m_rectTracked.setWidth(g_GblHelp.ZMapImgWidth2ScreenWidth(box.width));
                    g_GblHelp.m_rectTracked.setHeight(g_GblHelp.ZMapImgHeight2ScreenHeight(box.height));


//                    qDebug("tracked:(%.2f,%.2f,%.2f,%.2f)->(%.2f,%.2f,%.2f,%.2f)\n",box.x,box.y,box.width,box.height,///<
//                           g_GblHelp.m_rectTracked.x(),g_GblHelp.m_rectTracked.y(),g_GblHelp.m_rectTracked.width(),g_GblHelp.m_rectTracked.height());
                }else{
                    // Tracking failure detected.
                    qDebug()<<"Tracking failure detected.";
//                    g_GblHelp.m_nTrackingState=STATE_TRACKING_FAILED;
                }
                //qint64 tEnd=QDateTime::currentDateTime().toMSecsSinceEpoch();
                //qDebug()<<"cost (ms):"<<(tEnd-tStart);
                emit this->ZSigNewTracking();
            }else{

//                qDebug()<<"track:cvMat size:"<<matFrm.cols<<","<<matFrm.rows;
//                qDebug("track:box(%d,%d,%d,%d)\n",g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
//                cv::Rect2d box(g_GblHelp.m_nBoxX,g_GblHelp.m_nBoxY,g_GblHelp.m_nBoxWidth,g_GblHelp.m_nBoxHeight);
//                ptCenter.x = matFrm.cols/2;
//                ptCenter.y = matFrm.rows/2;
//                bbox.x = ptCenter.x - target_size.width/2;
//                bbox.y = ptCenter.y - target_size.height/2;
//                bbox.width = target_size.width;
//                bbox.height = target_size.height;

                //init tracker.
//                cv::Rect2d box;
                box.x=g_GblHelp.ZMapScreenX2ImgX(g_GblHelp.m_nBoxX);
                box.y=g_GblHelp.ZMapScreenY2ImgY(g_GblHelp.m_nBoxY);
                box.width=g_GblHelp.ZMapScreenWidth2ImgWidth(g_GblHelp.m_nBoxWidth);
                box.height=g_GblHelp.ZMapScreenHeight2ImgHeight(g_GblHelp.m_nBoxHeight);
//                Point ptCenter;
                ptCenter.x=box.x+box.width/2;
                ptCenter.y=box.y+box.height/2;
                target_size.width=box.width;
                target_size.height=box.height;
                tracker.tracker_init(matFrm,ptCenter,target_size);
                this->m_bInitBox=true;

                qDebug("init tracker:(%.2f,%.2f,%.2f,%.2f),center(%d,%d)\n",box.x,box.y,box.width,box.height,ptCenter.x,ptCenter.y);

                //display the tracked section img.
                cv::Mat matBox=cv::Mat(matFrm,box);
                QImage imgBox=cvMat2QImage(matBox);
                emit this->ZSigNewInitBox(imgBox);
            }
            this->usleep(THREAD_SCHEDULE_MS);
        }else{
            if(this->m_bInitBox)
            {
                qDebug()<<"Tracking cancel.";
                this->sleep(1);
                this->m_bInitBox=false;
            }
        }
        this->usleep(THREAD_SCHEDULE_MS*2);
    }
#endif

#if 0
    //Set up logging
    FILELog::ReportingLevel()=logINFO;
    Output2FILE::Stream()=stdout; //Log to stdout

    //Create a CMT object
    cmt::CMT cmt;
    bool bBypassThisFrame=false;
    while(!g_GblHelp.m_bExitFlag)
    {
        bBypassThisFrame=!bBypassThisFrame;
        if(STATE_TRACKING_START==g_GblHelp.m_nTrackingState)
        {
            //1.fetch RGB data from ring buffer.
            if(this->m_rb->ZGetElement((qint8*)pRGBBuffer,nBufSize)<0)
            {
                qDebug()<<"<Warning>:process,failed to get RGB from RGB queue.";
                continue;
            }

            if(bBypassThisFrame)
            {
                continue;
            }


            //2.build a cvMat from plain RGB buffer.
            cv::Mat matFrm=cv::Mat(CAP_IMG_SIZE_H,CAP_IMG_SIZE_W,CV_8UC3,pRGBBuffer);

            if(this->m_bInitBox)
            {
                qint64 tStart=QDateTime::currentDateTime().toMSecsSinceEpoch();

                Mat matFrmGray;
                if (matFrm.channels() > 1)
                {
                    cv::cvtColor(matFrm, matFrmGray, CV_BGR2GRAY);
                } else {
                    matFrmGray = matFrm;
                }

                //Let CMT process the frame
                cmt.processFrame(matFrmGray);
                //get the active points.
                g_GblHelp.m_vecActivePoints.clear();
                for(size_t i=0;i<cmt.points_active.size();i++)
                {
                     g_GblHelp.m_vecActivePoints.append(QPointF(cmt.points_active[i].x,cmt.points_active[i].y));
                }
                //get the four lines.
                Point2f vertices[4];
                cmt.bb_rot.points(vertices);
                for(int i=0;i<4;i++)
                {
                    g_GblHelp.m_lines[i].setP1(QPointF(vertices[i].x,vertices[i].y));
                    g_GblHelp.m_lines[i].setP2(QPointF(vertices[(i+1)%4].x,vertices[(i+1)%4].y));
                }

                // Update the tracking result
//                cv::Rect2d boxNewLocate;
//                bool result=tracker->update(matFrm,boxNewLocate);
//                if(result)
//                {
//                    qDebug("new locate:(%.2f,%.2f,%.2f,%.2f)\n",boxNewLocate.x,boxNewLocate.y,boxNewLocate.width,boxNewLocate.height);
//                    g_GblHelp.m_rectTracked=QRect(boxNewLocate.x,boxNewLocate.y,boxNewLocate.width,boxNewLocate.height);
//                    g_GblHelp.m_nTrackingState=STATE_TRACKING_START;
//                }else{
//                    // Tracking failure detected.
//                    qDebug()<<"Tracking failure detected.";
//                }
                qint64 tEnd=QDateTime::currentDateTime().toMSecsSinceEpoch();
                qDebug()<<"cost (ms):"<<(tEnd-tStart);
                emit this->ZSigNewTracking();
            }else{
                //Convert im0 to grayscale
                Mat matFrmGray;
                if (matFrm.channels() > 1) {
                    cv::cvtColor(matFrm,matFrmGray,CV_BGR2GRAY);
                } else {
                    matFrmGray=matFrm;
                }

                cv::Rect rect;
                rect.x=g_GblHelp.ZMapScreenX2ImgX(g_GblHelp.m_nBoxX);
                rect.y=g_GblHelp.ZMapScreenY2ImgY(g_GblHelp.m_nBoxY);
                rect.width=g_GblHelp.ZMapScreenWidth2ImgWidth(g_GblHelp.m_nBoxWidth);
                rect.height=g_GblHelp.ZMapScreenHeight2ImgHeight(g_GblHelp.m_nBoxHeight);
                //Initialize CMT
                cmt.initialize(matFrmGray, rect);


                this->m_bInitBox=true;
                cv::Mat matBox=cv::Mat(matFrm,rect);
                QImage imgBox=cvMat2QImage(matBox);
                emit this->ZSigNewInitBox(imgBox);

                //save the feature frames.
                //memcpy(pBackupBuffer,pRGBBuffer,nBufSize);
            }
            this->usleep(THREAD_SCHEDULE_MS);
        }else{
            if(this->m_bInitBox)
            {
                qDebug()<<"Tracking cancel.";
                this->sleep(1);
                this->m_bInitBox=false;
            }
        }
        this->usleep(THREAD_SCHEDULE_MS*2);
    }
#endif
    delete [] pRGBBuffer;
    pRGBBuffer=NULL;
    delete [] pBackupBuffer;
    pBackupBuffer=NULL;
    return;
}
