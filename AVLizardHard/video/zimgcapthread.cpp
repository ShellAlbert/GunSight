#include "zimgcapthread.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
short redAdjust[] = {
    -161,-160,-159,-158,-157,-156,-155,-153,
    -152,-151,-150,-149,-148,-147,-145,-144,
    -143,-142,-141,-140,-139,-137,-136,-135,
    -134,-133,-132,-131,-129,-128,-127,-126,
    -125,-124,-123,-122,-120,-119,-118,-117,
    -116,-115,-114,-112,-111,-110,-109,-108,
    -107,-106,-104,-103,-102,-101,-100, -99,
    -98, -96, -95, -94, -93, -92, -91, -90,
    -88, -87, -86, -85, -84, -83, -82, -80,
    -79, -78, -77, -76, -75, -74, -72, -71,
    -70, -69, -68, -67, -66, -65, -63, -62,
    -61, -60, -59, -58, -57, -55, -54, -53,
    -52, -51, -50, -49, -47, -46, -45, -44,
    -43, -42, -41, -39, -38, -37, -36, -35,
    -34, -33, -31, -30, -29, -28, -27, -26,
    -25, -23, -22, -21, -20, -19, -18, -17,
    -16, -14, -13, -12, -11, -10,  -9,  -8,
    -6,  -5,  -4,  -3,  -2,  -1,   0,   1,
    2,   3,   4,   5,   6,   7,   9,  10,
    11,  12,  13,  14,  15,  17,  18,  19,
    20,  21,  22,  23,  25,  26,  27,  28,
    29,  30,  31,  33,  34,  35,  36,  37,
    38,  39,  40,  42,  43,  44,  45,  46,
    47,  48,  50,  51,  52,  53,  54,  55,
    56,  58,  59,  60,  61,  62,  63,  64,
    66,  67,  68,  69,  70,  71,  72,  74,
    75,  76,  77,  78,  79,  80,  82,  83,
    84,  85,  86,  87,  88,  90,  91,  92,
    93,  94,  95,  96,  97,  99, 100, 101,
    102, 103, 104, 105, 107, 108, 109, 110,
    111, 112, 113, 115, 116, 117, 118, 119,
    120, 121, 123, 124, 125, 126, 127, 128,
};

short greenAdjust1[] = {
    34,  34,  33,  33,  32,  32,  32,  31,
    31,  30,  30,  30,  29,  29,  28,  28,
    28,  27,  27,  27,  26,  26,  25,  25,
    25,  24,  24,  23,  23,  23,  22,  22,
    21,  21,  21,  20,  20,  19,  19,  19,
    18,  18,  17,  17,  17,  16,  16,  15,
    15,  15,  14,  14,  13,  13,  13,  12,
    12,  12,  11,  11,  10,  10,  10,   9,
    9,   8,   8,   8,   7,   7,   6,   6,
    6,   5,   5,   4,   4,   4,   3,   3,
    2,   2,   2,   1,   1,   0,   0,   0,
    0,   0,  -1,  -1,  -1,  -2,  -2,  -2,
    -3,  -3,  -4,  -4,  -4,  -5,  -5,  -6,
    -6,  -6,  -7,  -7,  -8,  -8,  -8,  -9,
    -9, -10, -10, -10, -11, -11, -12, -12,
    -12, -13, -13, -14, -14, -14, -15, -15,
    -16, -16, -16, -17, -17, -17, -18, -18,
    -19, -19, -19, -20, -20, -21, -21, -21,
    -22, -22, -23, -23, -23, -24, -24, -25,
    -25, -25, -26, -26, -27, -27, -27, -28,
    -28, -29, -29, -29, -30, -30, -30, -31,
    -31, -32, -32, -32, -33, -33, -34, -34,
    -34, -35, -35, -36, -36, -36, -37, -37,
    -38, -38, -38, -39, -39, -40, -40, -40,
    -41, -41, -42, -42, -42, -43, -43, -44,
    -44, -44, -45, -45, -45, -46, -46, -47,
    -47, -47, -48, -48, -49, -49, -49, -50,
    -50, -51, -51, -51, -52, -52, -53, -53,
    -53, -54, -54, -55, -55, -55, -56, -56,
    -57, -57, -57, -58, -58, -59, -59, -59,
    -60, -60, -60, -61, -61, -62, -62, -62,
    -63, -63, -64, -64, -64, -65, -65, -66,
};

short greenAdjust2[] = {
    74,  73,  73,  72,  71,  71,  70,  70,
    69,  69,  68,  67,  67,  66,  66,  65,
    65,  64,  63,  63,  62,  62,  61,  60,
    60,  59,  59,  58,  58,  57,  56,  56,
    55,  55,  54,  53,  53,  52,  52,  51,
    51,  50,  49,  49,  48,  48,  47,  47,
    46,  45,  45,  44,  44,  43,  42,  42,
    41,  41,  40,  40,  39,  38,  38,  37,
    37,  36,  35,  35,  34,  34,  33,  33,
    32,  31,  31,  30,  30,  29,  29,  28,
    27,  27,  26,  26,  25,  24,  24,  23,
    23,  22,  22,  21,  20,  20,  19,  19,
    18,  17,  17,  16,  16,  15,  15,  14,
    13,  13,  12,  12,  11,  11,  10,   9,
    9,   8,   8,   7,   6,   6,   5,   5,
    4,   4,   3,   2,   2,   1,   1,   0,
    0,   0,  -1,  -1,  -2,  -2,  -3,  -4,
    -4,  -5,  -5,  -6,  -6,  -7,  -8,  -8,
    -9,  -9, -10, -11, -11, -12, -12, -13,
    -13, -14, -15, -15, -16, -16, -17, -17,
    -18, -19, -19, -20, -20, -21, -22, -22,
    -23, -23, -24, -24, -25, -26, -26, -27,
    -27, -28, -29, -29, -30, -30, -31, -31,
    -32, -33, -33, -34, -34, -35, -35, -36,
    -37, -37, -38, -38, -39, -40, -40, -41,
    -41, -42, -42, -43, -44, -44, -45, -45,
    -46, -47, -47, -48, -48, -49, -49, -50,
    -51, -51, -52, -52, -53, -53, -54, -55,
    -55, -56, -56, -57, -58, -58, -59, -59,
    -60, -60, -61, -62, -62, -63, -63, -64,
    -65, -65, -66, -66, -67, -67, -68, -69,
    -69, -70, -70, -71, -71, -72, -73, -73,
};

short blueAdjust[] = {
    -276,-274,-272,-270,-267,-265,-263,-261,
    -259,-257,-255,-253,-251,-249,-247,-245,
    -243,-241,-239,-237,-235,-233,-231,-229,
    -227,-225,-223,-221,-219,-217,-215,-213,
    -211,-209,-207,-204,-202,-200,-198,-196,
    -194,-192,-190,-188,-186,-184,-182,-180,
    -178,-176,-174,-172,-170,-168,-166,-164,
    -162,-160,-158,-156,-154,-152,-150,-148,
    -146,-144,-141,-139,-137,-135,-133,-131,
    -129,-127,-125,-123,-121,-119,-117,-115,
    -113,-111,-109,-107,-105,-103,-101, -99,
    -97, -95, -93, -91, -89, -87, -85, -83,
    -81, -78, -76, -74, -72, -70, -68, -66,
    -64, -62, -60, -58, -56, -54, -52, -50,
    -48, -46, -44, -42, -40, -38, -36, -34,
    -32, -30, -28, -26, -24, -22, -20, -18,
    -16, -13, -11,  -9,  -7,  -5,  -3,  -1,
    0,   2,   4,   6,   8,  10,  12,  14,
    16,  18,  20,  22,  24,  26,  28,  30,
    32,  34,  36,  38,  40,  42,  44,  46,
    49,  51,  53,  55,  57,  59,  61,  63,
    65,  67,  69,  71,  73,  75,  77,  79,
    81,  83,  85,  87,  89,  91,  93,  95,
    97,  99, 101, 103, 105, 107, 109, 112,
    114, 116, 118, 120, 122, 124, 126, 128,
    130, 132, 134, 136, 138, 140, 142, 144,
    146, 148, 150, 152, 154, 156, 158, 160,
    162, 164, 166, 168, 170, 172, 175, 177,
    179, 181, 183, 185, 187, 189, 191, 193,
    195, 197, 199, 201, 203, 205, 207, 209,
    211, 213, 215, 217, 219, 221, 223, 225,
    227, 229, 231, 233, 235, 238, 240, 242,
};
//判断范围
unsigned char clip(int val)
{
    if(val > 255)
    {
        return 255;
    }
    else if(val > 0)
    {
        return val;
    }
    else
    {
        return 0;
    }
}
//查表法YUV TO RGB
int YUYVToRGB_table(unsigned char *yuv, unsigned char *rgb, unsigned int width,unsigned int height)
{
    //YU YV YU YV .....
    short y1=0, y2=0, u=0, v=0;
    unsigned char *pYUV = yuv;
    unsigned char *pGRB = rgb;
    int i=0;
    //int y=0,x=0,in=0,y0,out=0;
    int count =width*height/2;
    for(i = 0; i < count; i++)
    {
        y1 = *pYUV++ ;
        u  = *pYUV++ ;
        y2 = *pYUV++ ;
        v  = *pYUV++ ;

        *pGRB++ = clip(y1 + redAdjust[v]);
        *pGRB++ = clip(y1 + greenAdjust1[u] + greenAdjust2[v]);
        *pGRB++ = clip(y1 + blueAdjust[u]);
        *pGRB++ = clip(y2 + redAdjust[v]);
        *pGRB++ = clip(y2 + greenAdjust1[u] + greenAdjust2[v]);
        *pGRB++ = clip(y2 + blueAdjust[u]);
    }
    return 0;
}


static int convert_yuv_to_rgb_pixel(int y, int u, int v)

{

    unsigned int pixel32 = 0;

    unsigned char *pixel = (unsigned char *)&pixel32;

    int r, g, b;

    r = y + (1.370705 * (v-128));

    g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));

    b = y + (1.732446 * (u-128));

    if(r > 255) r = 255;

    if(g > 255) g = 255;

    if(b > 255) b = 255;

    if(r < 0) r = 0;

    if(g < 0) g = 0;

    if(b < 0) b = 0;

    pixel[0] = r * 220 / 256;

    pixel[1] = g * 220 / 256;

    pixel[2] = b * 220 / 256;

    return pixel32;

}



int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)

{

    unsigned int in, out = 0;

    unsigned int pixel_16;

    unsigned char pixel_24[3];

    unsigned int pixel32;

    int y0, u, y1, v;

    for(in = 0; in < width * height * 2; in += 4) {

        pixel_16 = yuv[in + 3] << 24 |

                                  yuv[in + 2] << 16 |

                                                 yuv[in + 1] <<  8 |

                                                                 yuv[in + 0];

        y0 = (pixel_16 & 0x000000ff);

        u  = (pixel_16 & 0x0000ff00) >>  8;

        y1 = (pixel_16 & 0x00ff0000) >> 16;

        v  = (pixel_16 & 0xff000000) >> 24;

        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);

        pixel_24[0] = (pixel32 & 0x000000ff);

        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;

        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

        rgb[out++] = pixel_24[0];

        rgb[out++] = pixel_24[1];

        rgb[out++] = pixel_24[2];

        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);

        pixel_24[0] = (pixel32 & 0x000000ff);

        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;

        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

        rgb[out++] = pixel_24[0];

        rgb[out++] = pixel_24[1];

        rgb[out++] = pixel_24[2];

    }
    return 0;
}

ZImgCapThread::ZImgCapThread(QString devUsbId,qint32 nPreWidth,qint32 nPreHeight,qint32 nPreFps,CAM_ID_TYPE_E camIdType)
{
    this->m_devUsbId=devUsbId;
    this->m_nPreWidth=nPreWidth;
    this->m_nPreHeight=nPreHeight;
    this->m_nPreFps=nPreFps;
    this->m_nCamIdType=camIdType;

    //capture image to process queue.
    this->m_rbProcess=NULL;
    //capture yuv to yuv queue.
    this->m_rbYUV=NULL;

    this->m_bExitFlag=false;
    this->m_bCleanup=true;
}
ZImgCapThread::~ZImgCapThread()
{

}
qint32 ZImgCapThread::ZBindProcessQueue(ZRingBuffer *rbProcess)
{
    this->m_rbProcess=rbProcess;
    return 0;
}
qint32 ZImgCapThread::ZBindYUVQueue(ZRingBuffer *rbYUV)
{
    this->m_rbYUV=rbYUV;
    return 0;
}
qint32 ZImgCapThread::ZStartThread()
{
    if(this->m_rbProcess==NULL)
    {
        qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,no bind process queue,cannot start thread.";
        return -1;
    }
    //check yuv ringbuffer.
    if(this->m_rbYUV==NULL)
    {
        qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,no bind yuv queue,cannot start thread.";
        return -1;
    }
    this->m_bExitFlag=false;
    this->start();
    return 0;
}
qint32 ZImgCapThread::ZStopThread()
{
    this->quit();
    this->m_bExitFlag=true;
    this->wait(1000);
    return 0;
}
qint32 ZImgCapThread::ZGetCAMImgFps()
{
    return this->m_cam->ZGetFrameRate();
}
QString ZImgCapThread::ZGetDevName()
{
    return this->m_devUsbId;
}
void ZImgCapThread::run()
{
    //bind main/main ex img cap thread to cpu core 2.
    //bind aux img cap thread to cpu core 3.
    if(this->m_nCamIdType==CAM1_ID_Main || this->m_nCamIdType==CAM3_ID_MainEx)
    {
        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(2,&cpuSet);
        if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
        {
            qDebug()<<"<Error>:failed to bind MainImgCap thread to cpu core 2.";
        }else{
            qDebug()<<"<Info>:success to bind MainImgCap thread to cpu core 2.";
        }
    }else{
        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(3,&cpuSet);
        if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
        {
            qDebug()<<"<Error>:failed to bind AuxImgCap thread to cpu core 3.";
        }else{
            qDebug()<<"<Info>:success to bind AuxImgCap thread to cpu core 3.";
        }
    }
    //malloc memory.
    qint32 nSingleImgSize=gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*sizeof(char);
    unsigned char *pRGBBuffer=(unsigned char*)malloc(nSingleImgSize);
    if(NULL==pRGBBuffer)
    {
        qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to allocate RGB buffer.";
        gGblPara.m_bGblRst2Exit=true;
        emit this->ZSigThreadFinished();
        this->m_bCleanup=true;
        return;
    }

    QString devName;
    while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
    {
        devName=gGblPara.m_video.m_mapID2Fd.value(this->m_devUsbId);
        if(devName.isEmpty())
        {
            qDebug()<<"<Error>:cannot map usb id to fd "<<this->m_devUsbId;
            break;
        }
        ZCAMDevice *camDev=new ZCAMDevice(devName,this->m_nPreWidth,this->m_nPreHeight,this->m_nPreFps);
        if(camDev->ZOpenCAM()<0)
        {
            qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to open cam device"<<devName;
            break;
        }
        if(camDev->ZInitCAM()<0)
        {
            qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to init cam device"<<devName;
            break;
        }
        if(camDev->ZInitMAP()<0)
        {
            qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to init map device"<<devName;
            break;
        }

        //start capture.
        if(camDev->ZStartCapture()<0)
        {
            qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to start capture"<<devName;
            break;
        }

        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"Camera capture starts"<<devName<<".";
        this->m_bCleanup=false;


        //    QByteArray baYUV;
        //    QFile fileYUV("a.yuv");
        //    if(fileYUV.open(QIODevice::ReadOnly))
        //    {
        //        baYUV=fileYUV.readAll();
        //        fileYUV.close();
        //    }else{
        //        qDebug()<<"open a.yuv failed.";
        //        return;
        //    }

        while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
        {
            qint32 nLen;
            unsigned char *pYUVData;

            //只有当有客户端连接时才开始采集图像.
            if(this->m_nCamIdType==CAM1_ID_Main || this->m_nCamIdType==CAM3_ID_MainEx)
            {
                if(!(gGblPara.m_bVideoTcpConnected))
                {
                    this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                    continue;
                }
            }else{//the aux camera.
                if(!gGblPara.m_bVideoTcpConnected2)
                {
                    this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                    continue;
                }
            }


            //get a yuv frame.
            //返回值:<0错误，0:超时，需要再次调用，>0：成功.
            int ret=camDev->ZGetFrame((void**)&pYUVData,(size_t*)&nLen);
            if(ret<0)
            {
                qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to get yuv data from"<<devName;
                break;
            }else if(0==ret)
            {
                //timeout,retry again.
                //qDebug()<<"<Warning>:"<<_CURRENT_DATETIME_<<"ImgCapThread,select() timeout"<<devName;
                this->usleep(VIDEO_THREAD_SCHEDULE_US);
                continue;
            }else if(ret>0)
            {
                //如果当前视场为小视场，则只采不扔进YUV队列.
                if(this->m_nCamIdType==CAM1_ID_Main || this->m_nCamIdType==CAM3_ID_MainEx)
                {
                    if(this->m_devUsbId!=gGblPara.m_video.m_Cam1IDNow)
                    {
                        camDev->ZUnGetFrame();
                        this->usleep(VIDEO_THREAD_SCHEDULE_US);
                        continue;
                    }
                }
                //qDebug()<<"yuyv len:"<<nLen;
                if(this->m_rbYUV->ZPutElement((qint8*)pYUVData,nLen)<0)
                {
                    qDebug()<<"<Warning>:img cap,timeout to put yuv to yuv queue.";
                }

                if(gGblPara.m_bJsonImgPro || gGblPara.m_bJsonFlushUIImg)
                {
                    //这段代码将YUV420P转换为RGB888太耗CPU了。
                    //使用top查看能达到80%以上的CPU占用率。
                    //此处需要优化。
                    //convert yuv to RGB.
                    //YUYVToRGB_table(pYUVData,pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);
                    //YU YV YU YV .....
                    convert_yuv_to_rgb_buffer(pYUVData,pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);

                    //convert_yuv_to_rgb_buffer(pYUVData,pRGBBuffer,camDev->ZGetImgWidth(),camDev->ZGetImgHeight());
                    //build a RGB888 QImage object.
                    //QImage newQImg((uchar*)pRGBBuffer,camDev->ZGetImgWidth(),camDev->ZGetImgHeight(),QImage::Format_RGB888);
                }
                //free a buffer for device.
                camDev->ZUnGetFrame();

                //只有当开启ImgPro标志位被置位时（通过Android Json协议).
                //采集线程才将图像扔入process队列，从而唤醒图像处理线程工作.
                if(gGblPara.m_bJsonImgPro)
                {
                    if(this->m_rbProcess->ZTryPutElement((qint8*)pRGBBuffer,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3)<0)
                    {
                        if(this->m_nCamIdType==CAM1_ID_Main)
                        {
                            qDebug()<<"<Warning>:cam1_id_main,img cap,failed to put RGB to img matched queue:"<<this->m_rbProcess->ZGetValidNum();
                        }else if(this->m_nCamIdType==CAM2_ID_Aux)
                        {
                            qDebug()<<"<Warning>:cam2_id_aux,img cap,failed to put RGB to img matched queue:"<<this->m_rbProcess->ZGetValidNum();
                        }
                    }
                }

                //只有开启本地UI刷新才将采集到的图像扔入disp队列.
                if(gGblPara.m_bJsonFlushUIImg)
                {
                    QImage newImg((uchar*)pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1,QImage::Format_RGB888);
                    emit this->ZSigNewImgArrived(newImg);
                }
            }
            this->usleep(VIDEO_THREAD_SCHEDULE_US);
        }
        //do some clean. stop camera.
        camDev->ZStopCapture();
        camDev->ZUnInitCAM();
        camDev->ZCloseCAM();
    }
    //free memory.
    free(pRGBBuffer);
    pRGBBuffer=NULL;
    //此处设置本线程退出标志.
    //同时设置全局请求退出标志，请求其他线程退出.
    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainCamera capture ends "<<devName<<".";
        //gGblPara.m_bMainCapThreadExitFlag=true;
        break;
    case CAM2_ID_Aux:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"AuxCamera capture ends "<<devName<<".";
        //gGblPara.m_bAuxCapThreadExitFlag=true;
        break;
    case CAM3_ID_MainEx:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainExCamera capture ends "<<devName<<".";
        //gGblPara.m_bMainExCapThreadExitFlag=true;
        break;
    }
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}
bool ZImgCapThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
