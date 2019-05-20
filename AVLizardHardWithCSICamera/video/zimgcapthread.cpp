#include "zimgcapthread.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <fcntl.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

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

ZImgCapThread::ZImgCapThread(QString devFile,CAM_ID_TYPE_E camIdType)
{
    this->m_devFile=devFile;
    this->m_nCamIdType=camIdType;
    this->m_bCleanup=true;
}
ZImgCapThread::~ZImgCapThread()
{

}
//capture -> processing.
void ZImgCapThread::ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut1=freeQueue;
    this->m_usedQueueOut1=usedQueue;
    this->m_mutexOut1=mutex;
    this->m_condQueueEmptyOut1=condQueueEmpty;
    this->m_condQueueFullOut1=condQueueFull;
}

//capture -> h264 encoder.
void ZImgCapThread::ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut2=freeQueue;
    this->m_usedQueueOut2=usedQueue;
    this->m_mutexOut2=mutex;
    this->m_condQueueEmptyOut2=condQueueEmpty;
    this->m_condQueueFullOut2=condQueueFull;
}
qint32 ZImgCapThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZImgCapThread::ZStopThread()
{
    this->quit();
    this->wait(1000);
    return 0;
}
QString ZImgCapThread::ZGetDevName()
{
    return this->m_devFile;
}
void ZImgCapThread::doCleanBeforeExit()
{
    //set global request to exit flag to notify other threads.
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
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

    //create file to save camera infomation.
    QFile fileInfo;
    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        fileInfo.setFileName("Cam1.main.info");
        break;
    case CAM2_ID_Aux:
        fileInfo.setFileName("Cam2.aux.info");
        break;
    case CAM3_ID_MainEx:
        fileInfo.setFileName("Cam3.mainEx.info");
        break;
    }
    if(!fileInfo.open(QIODevice::WriteOnly))
    {
        qDebug()<<"<Error>:failed to create cam info file,"<<fileInfo.fileName();
        this->doCleanBeforeExit();
        return;
    }

    //malloc memory for YUV2RGB convert.
    //YUV420P,1920*1080*2.
    //RGB888,1920*1080*3.
    qint32 nSingleImgSize=gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*sizeof(char);
    unsigned char *pRGBBuffer=(unsigned char*)malloc(nSingleImgSize);
    if(NULL==pRGBBuffer)
    {
        qDebug()<<"<Error>:"<<_CURRENT_DATETIME_<<"ImgCapThread,failed to allocate RGB buffer.";
        this->doCleanBeforeExit();
        return;
    }

    //open device file.
    qint32 fd=open(this->m_devFile.toStdString().c_str(),O_RDWR|O_NONBLOCK,0);
    if(fd<0)
    {
        qDebug()<<"<Error>:failed to open device file,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }
    //query capabilities.
    struct v4l2_capability cap;
    if(ioctl(fd,VIDIOC_QUERYCAP,&cap)<0)
    {
        qDebug()<<"<Error>:failed to query capability,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    //write capability to file.
    fileInfo.write(this->m_devFile.toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.driver).toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.card).toLocal8Bit()+"\n");
    fileInfo.write(QString((char*)cap.bus_info).toLocal8Bit()+"\n");

    //check capture&streaming ability.
    if(!((cap.capabilities&V4L2_CAP_VIDEO_CAPTURE)&&(cap.capabilities & V4L2_CAP_STREAMING)))
    {
        qDebug()<<"<Error>:CAM has no video streaming capture ability,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    //enumrate format description.
    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc,0,sizeof(fmtdesc));
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtdesc.index=0;
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
    {
        char *fmtName=(char*)fmtdesc.description;
        fileInfo.write(QString("%1:%2\n").arg(fmtdesc.index).arg(fmtName).toLatin1());
        fmtdesc.index++;
    }
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd,VIDIOC_G_FMT,&fmt);
    fileInfo.write(QString("%1*%2,color space:%3\n").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height).arg(fmt.fmt.pix.colorspace).toLatin1());

    //request the uvc driver to allocate buffers.
    struct v4l2_requestbuffers reqBuf;
    reqBuf.count=4;
    reqBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuf.memory=V4L2_MEMORY_MMAP;
    if(ioctl(fd,VIDIOC_REQBUFS,&reqBuf)<0)
    {
        qDebug()<<"<Error>:failed to request uvc buffer,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    IMGBufferStruct* pIMGBuffer;
    pIMGBuffer=(IMGBufferStruct*)malloc(reqBuf.count*sizeof(IMGBufferStruct));
    if(NULL==pIMGBuffer)
    {
        qDebug()<<"<Error>:failed to calloc buffer,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }
    //query buffer & do mmap.
    for(quint32 i=0;i<reqBuf.count;i++)
    {
        struct v4l2_buffer buf;
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=i;
        if(ioctl(fd,VIDIOC_QUERYBUF,&buf)<0)
        {
            qDebug()<<"<Error>:failed to query buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
        pIMGBuffer[i].nLength=buf.length;
        pIMGBuffer[i].pStart=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
        if(MAP_FAILED==pIMGBuffer[i].pStart)
        {
            qDebug()<<"<Error>:failed to mmap buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
    }
    //start capturing.
    for(quint32 i=0;i<reqBuf.count;i++)
    {
        //queue buffer.
        struct v4l2_buffer buf;
        memset(&buf,0,sizeof(buf));
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=i;
        if(ioctl(fd,VIDIOC_QBUF,&buf)<0)
        {
            qDebug()<<"<Error>:failed to query buffer,"<<this->m_devFile;
            this->doCleanBeforeExit();
            return;
        }
    }
    //stream on.
    v4l2_buf_type type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMON,&type)<0)
    {
        qDebug()<<"<Error>:failed to stream on,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }
    //working loop.
    while(!gGblPara.m_bGblRst2Exit)
    {
        //只有当有客户端连接时才开始采集图像.
        switch(this->m_nCamIdType)
        {
        case CAM1_ID_Main:
        case CAM3_ID_MainEx:
            if(!(gGblPara.m_bVideoTcpConnected))
            {
                this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                continue;
            }
            break;
        case CAM2_ID_Aux:
            if(!gGblPara.m_bVideoTcpConnected2)
            {
                this->usleep(VIDEO_THREAD_SCHEDULE_US*10);
                continue;
            }
            break;
        default:
            break;
        }

        qint32 nLen;
        quint8 *pYUVData;
        qint32 nBufIndex;
        ///////////////////////
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        struct timeval tv;
        tv.tv_sec=0;
        tv.tv_usec=1000*10;//10ms.
        int ret=select(fd+1,&fds,NULL,NULL,&tv);
        if(ret<0)
        {
            qDebug()<<"<Error>:select() cam device error,"<<this->m_devFile;
            break;
        }else if(0==ret){
            //qDebug()<<"<Warning>:select() timeout for cam device.";
            continue;
        }else{
            struct v4l2_buffer getBuf;
            memset(&getBuf,0,sizeof(getBuf));
            getBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
            getBuf.memory=V4L2_MEMORY_MMAP;
            if(ioctl(fd,VIDIOC_DQBUF,&getBuf)<0)
            {
                qDebug()<<"<Error>:failed to DQBUF,"<<this->m_devFile;
                break;
            }
            pYUVData=(quint8*)pIMGBuffer[getBuf.index].pStart;
            nLen=pIMGBuffer[getBuf.index].nLength;
            //remember the index.
            nBufIndex=getBuf.index;
        }


        //只有当开启ImgPro标志位被置位时（通过Android Json协议).
        //采集线程才将图像扔入process队列，从而唤醒图像处理线程工作.
        if(gGblPara.m_bJsonImgPro)
        {
            //这段代码将YUV420P转换为RGB888太耗CPU了。
            //使用top查看能达到80%以上的CPU占用率。
            //此处需要优化。
            //convert yuv to RGB.
            //YUYVToRGB_table(pYUVData,pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);
            //YU YV YU YV .....
            convert_yuv_to_rgb_buffer(pYUVData,pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);

            //1.put yuv data to img process fifo.
            this->m_mutexOut1->lock();
            while(this->m_freeQueueOut1->isEmpty())
            {//timeout 5s to check exit flag.
                if(!this->m_condQueueEmptyOut1->wait(this->m_mutexOut1,5000))
                {
                    this->m_mutexOut1->unlock();
                    if(gGblPara.m_bGblRst2Exit)
                    {
                        break;
                    }
                }
            }
            if(gGblPara.m_bGblRst2Exit)
            {
                break;
            }

            QByteArray *pcmBuffer1=this->m_freeQueueOut1->dequeue();
            memcpy(pcmBuffer1->data(),pRGBBuffer,gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3*sizeof(char));
            this->m_usedQueueOut1->enqueue(pcmBuffer1);
            this->m_condQueueFullOut1->wakeAll();
            this->m_mutexOut1->unlock();
        }

        //2.put yuv data to h264 encode fifo.
        //get a free buffer from fifo.
        this->m_mutexOut2->lock();
        while(this->m_freeQueueOut2->isEmpty())
        {//timeout 5s to check exit flag.
            if(!this->m_condQueueEmptyOut2->wait(this->m_mutexOut2,5000))
            {
                this->m_mutexOut2->unlock();
                if(gGblPara.m_bGblRst2Exit)
                {
                    break;
                }
            }
        }
        if(gGblPara.m_bGblRst2Exit)
        {
            break;
        }

        QByteArray *pcmBuffer2=this->m_freeQueueOut2->dequeue();
        memcpy(pcmBuffer2->data(),pYUVData,nLen);
        this->m_usedQueueOut2->enqueue(pcmBuffer2);
        this->m_condQueueFullOut2->wakeAll();
        this->m_mutexOut2->unlock();

        //free a buffer for device.
        struct v4l2_buffer putBuf;
        memset(&putBuf,0,sizeof(putBuf));
        putBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        putBuf.memory=V4L2_MEMORY_MMAP;
        putBuf.index=nBufIndex;
        if(ioctl(fd,VIDIOC_QBUF,&putBuf)<0)
        {
            qDebug()<<"<Error>:failed to DQBUF,"<<this->m_devFile;
            break;
        }
    }

    //stop capture.
    type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMOFF,&type)<0)
    {
        qDebug()<<"<Error>:failed to stream off,"<<this->m_devFile;
        this->doCleanBeforeExit();
        return;
    }

    for(quint32 i=0;i<reqBuf.count;i++)
    {
        munmap(pIMGBuffer[i].pStart,pIMGBuffer[i].nLength);
    }
    free(pIMGBuffer);
    free(pRGBBuffer);
    close(fd);
    //此处设置本线程退出标志.
    //同时设置全局请求退出标志，请求其他线程退出.
    switch(this->m_nCamIdType)
    {
    case CAM1_ID_Main:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainCamera capture ends "<<this->m_devFile<<".";
        break;
    case CAM2_ID_Aux:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"AuxCamera capture ends "<<this->m_devFile<<".";
        break;
    case CAM3_ID_MainEx:
        qDebug()<<"<MainLoop>:"<<_CURRENT_DATETIME_<<"MainExCamera capture ends "<<this->m_devFile<<".";
        break;
    default:
        break;
    }
    this->doCleanBeforeExit();
    return;
}
bool ZImgCapThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
