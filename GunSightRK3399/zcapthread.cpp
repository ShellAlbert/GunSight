#include "zcapthread.h"
#include "zgblhelp.h"
#include <QFile>
extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <sys/select.h>
}
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QDateTime>
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
ZCapThread::ZCapThread(QString devName,quint32 width,quint32 height,quint32 fps)
{
    this->m_devName=devName;
    this->m_nWidth=width;
    this->m_nHeight=height;
    this->m_nFps=fps;
}
void ZCapThread::ZBindRingBuffer(ZRingBuffer *rb)
{
    this->m_rb=rb;
}
void ZCapThread::run()
{
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtDesc;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_streamparm streamPara;

    qint32 fd=open(this->m_devName.toStdString().c_str(),O_RDWR|O_NONBLOCK,0);
    if(fd<0)
    {
        qDebug()<<"failed to open device "<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        return;
    }

#ifdef DUMP_CAM_INFO2FILE
    QFile fileCamInfo(QApplication::applicationDirPath()+"/"+this->m_devName.mid(5));
    fileCamInfo.open(QIODevice::WriteOnly);
    fileCamInfo.write(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toLatin1()+"\n");
    fileCamInfo.write(this->m_devName.toLocal8Bit()+"\n");
#endif

    //query capability.
    if(ioctl(fd,VIDIOC_QUERYCAP,&cap)<0)
    {
        qDebug()<<"failed to query cap "<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        qDebug()<<"has no video capture ability "<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        qDebug()<<"has no capture streaming ability "<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    //get supported format.
#ifdef DUMP_CAM_INFO2FILE
    fmtDesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtDesc.index=0;//enumerate all format.
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtDesc)!=-1)
    {
        QString fmtName((char*)fmtDesc.description);
        //qDebug()<<this->m_devName<<fmtName;
        //get all frameSize.
        struct v4l2_frmsizeenum frmSizeEnum;
        frmSizeEnum.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        frmSizeEnum.pixel_format=fmtDesc.pixelformat;
        frmSizeEnum.index=0;//enumerate all frameSize.
        while(ioctl(fd,VIDIOC_ENUM_FRAMESIZES,&frmSizeEnum)!=-1)
        {
            //get all frameRate.
            struct v4l2_frmivalenum frmRateEnum;
            frmRateEnum.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
            frmRateEnum.pixel_format=fmtDesc.pixelformat;
            frmRateEnum.width=frmSizeEnum.discrete.width;
            frmRateEnum.height=frmSizeEnum.discrete.height;
            frmRateEnum.index=0;//enumerate all frame Intervals.(fps).
            while(ioctl(fd,VIDIOC_ENUM_FRAMEINTERVALS,&frmRateEnum)!=-1)
            {
                //dump width*height,fps to file.
                QString fmtInfo;
                fmtInfo+=fmtName+",";
                fmtInfo+=QString::number(frmSizeEnum.discrete.width,10)+"*"+QString::number(frmSizeEnum.discrete.height)+",";
                fmtInfo+=QString::number(frmRateEnum.discrete.denominator/frmRateEnum.discrete.numerator,10)+"fps";
                fmtInfo+="\n";
                fileCamInfo.write(fmtInfo.toLocal8Bit());
                //next frame rate.
                frmRateEnum.index++;
            }
            frmSizeEnum.index++;
        }
        fmtDesc.index++;
    }
#endif

    //get default format.
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_FMT,&fmt)<0)
    {
        qDebug()<<"failed to get default format"<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
#ifdef DUMP_CAM_INFO2FILE
    QString fmtDefault;
    fmtDefault+=QString("Default setting format:\n");
    switch(fmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_YUYV:
        fmtDefault+=QString("YUYV pixel format.\n");
        break;
    case V4L2_PIX_FMT_MJPEG:
        fmtDefault+=QString("MJPEG pixel format.\n");
        break;
    default:
        fmtDefault+=QString("other pixel format.\n");
        break;
    }
    fmtDefault+=QString("%1*%2,image size=%3").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height).arg(fmt.fmt.pix.sizeimage);
    fmtDefault+="\n";
    fileCamInfo.write(fmtDefault.toLocal8Bit());
#endif

    //get default fps.
    memset(&streamPara,0,sizeof(streamPara));
    streamPara.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_PARM,&streamPara)<0)
    {
        qDebug()<<"failed to get default fps"<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
#ifdef DUMP_CAM_INFO2FILE
    QString fpsDefault;
    fpsDefault+=QString("Default fps:%1/%2\n")///<
            .arg(streamPara.parm.output.timeperframe.numerator)///<
            .arg(streamPara.parm.output.timeperframe.denominator);
    fileCamInfo.write(fpsDefault.toLocal8Bit());
#endif

    /* 16  YUV 4:2:2     */
    //set predefined format.
    memset(&fmt,0,sizeof(fmt));
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width=this->m_nWidth;
    fmt.fmt.pix.height=this->m_nHeight;
    fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_MJPEG.
    fmt.fmt.pix.field=V4L2_FIELD_INTERLACED;//V4L2_FIELD_NONE;//V4L2_FIELD_INTERLACED;
    if(ioctl(fd,VIDIOC_S_FMT,&fmt)<0)
    {
        qDebug()<<"failed to set YUV422 format"<<this->m_devName;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
    //set predefined fps.
    streamPara.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamPara.parm.capture.timeperframe.numerator=1;
    streamPara.parm.capture.timeperframe.denominator=this->m_nFps;
    if(ioctl(fd,VIDIOC_S_PARM,&streamPara)<0)
    {
        qDebug()<<"failed to set fps to"<<this->m_nFps;
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
#ifdef DUMP_CAM_INFO2FILE
    QString fmtDefault3;
    fmtDefault3+=QString("Reqeust to set %1*%2,%3 fps").arg(this->m_nWidth).arg(this->m_nHeight).arg(this->m_nFps);
    fmtDefault3+="\n";
    fileCamInfo.write(fmtDefault3.toLocal8Bit());
#endif

    //get format again to verify my setting.
    memset(&fmt,0,sizeof(fmt));
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_FMT,&fmt)<0)
    {
        qDebug()<<"failed to get fmt.";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
#ifdef DUMP_CAM_INFO2FILE
    QString fmtDefault2;
    fmtDefault2+=QString("Readback setting format:\n");
    switch(fmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_YUYV:
        fmtDefault2+=QString("YUYV pixel format.\n");
        break;
    case V4L2_PIX_FMT_MJPEG:
        fmtDefault2+=QString("MJPEG pixel format.\n");
        break;
    default:
        fmtDefault2+=QString("other pixel format.\n");
        break;
    }
    fmtDefault2+=QString("%1*%2,image size=%3").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height).arg(fmt.fmt.pix.sizeimage);
    fmtDefault2+="\n";
    fileCamInfo.write(fmtDefault2.toLocal8Bit());
#endif

    if(this->m_nWidth!=fmt.fmt.pix.width || this->m_nHeight!=fmt.fmt.pix.height)
    {
        qDebug()<<"read back width*size does not equal setting expected!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    //get fps again to verify my setting.
    streamPara.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_PARM,&streamPara)<0)
    {
        qDebug()<<"failed to get paramters.";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
#ifdef DUMP_CAM_INFO2FILE
    QString fpsDefault2;
    fpsDefault2+=QString("Readback fps:%1/%2\n")///<
            .arg(streamPara.parm.output.timeperframe.numerator)///<
            .arg(streamPara.parm.output.timeperframe.denominator);
    fileCamInfo.write(fpsDefault2.toLocal8Bit());
    fileCamInfo.flush();
    fileCamInfo.close();
#endif

    if(streamPara.parm.output.timeperframe.denominator>streamPara.parm.output.timeperframe.numerator)
    {
        quint32 nGetFps=streamPara.parm.output.timeperframe.denominator/streamPara.parm.output.timeperframe.numerator;
        if(this->m_nFps!=nGetFps)
        {
            qDebug()<<"read back fps does not equal setting expected ("<<this->m_nFps<<","<<nGetFps<<").";
            g_GblHelp.m_bExitFlag=true;
            close(fd);
            return;
        }
    }

    //request the uvc driver to allocate buffers.
    struct v4l2_requestbuffers reqBuf;
    reqBuf.count=V4L2_REQBUF_NUM;
    reqBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuf.memory=V4L2_MEMORY_MMAP;
    if(ioctl(fd,VIDIOC_REQBUFS,&reqBuf)<0)
    {
        qDebug()<<"failed to request buffers!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    IMGBufferStruct* pIMGBuffer=(IMGBufferStruct*)malloc(reqBuf.count*sizeof(IMGBufferStruct));
    if(NULL==pIMGBuffer)
    {
        qDebug()<<"failed to callocate buffers!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
    for(quint32 i=0;i<reqBuf.count;i++)
    {
        //query buffer & do mmap.
        struct v4l2_buffer buf;
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=i;
        if(ioctl(fd,VIDIOC_QUERYBUF,&buf)<0)
        {
            qDebug()<<"failed to query buffer!";
            g_GblHelp.m_bExitFlag=true;
            close(fd);
            return;
        }
        pIMGBuffer[i].nLength=buf.length;
        pIMGBuffer[i].pStart=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
        if(MAP_FAILED==pIMGBuffer[i].pStart)
        {
            qDebug()<<"failed to mmap!";
            g_GblHelp.m_bExitFlag=true;
            close(fd);
            return;
        }
    }

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
            qDebug()<<"failed to query buffer!";
            g_GblHelp.m_bExitFlag=true;
            close(fd);
            return;
        }
    }
    //stream on.
    v4l2_buf_type type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMON,&type)<0)
    {
        qDebug()<<"failed to stream on!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    quint8 *pRGBBuffer=(unsigned char*)malloc(this->m_nWidth*this->m_nHeight*3);
    if(NULL==pRGBBuffer)
    {
        qDebug()<<"failed to malloc buffer for RGB!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }
    while(!g_GblHelp.m_bExitFlag)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        struct timeval tv;
        tv.tv_sec=0;
        tv.tv_usec=1000*10;//10ms.
        int ret=select(fd+1,&fds,NULL,NULL,&tv);
        if(ret<0)
        {
            qDebug()<<"<error>:select() cam device error.";
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
                qDebug()<<"<error>:failed to DQBUF";
                break;
            }
            void *pYUV=pIMGBuffer[getBuf.index].pStart;
            qint32 nYUVLen=pIMGBuffer[getBuf.index].nLength;

            //qDebug()<<"get frame len:"<<*nLen;
            if(g_GblHelp.m_bWrYuv2File)
            {
                static quint32 nFileIndex=0;
                //write img to file.
                QString fileName=tr("%1.yuv").arg(nFileIndex);
                QFile fileYUV(fileName);
                if(fileYUV.open(QIODevice::WriteOnly))
                {
                    fileYUV.write((const char*)pYUV,nYUVLen);
                    fileYUV.flush();
                    fileYUV.close();
                    qDebug()<<"YUV file generated "<<fileName;
                }
            }

            //这段代码将YUV420P转换为RGB888太耗CPU了。
            //使用top查看能达到80%以上的CPU占用率。
            //此处需要优化。
            //convert yuv to RGB.
            //YUYVToRGB_table(pYUVData,pRGBBuffer,gGblPara.m_widthCAM1,gGblPara.m_heightCAM1);
            //YU YV YU YV .....
            convert_yuv_to_rgb_buffer((unsigned char*)pYUV,(unsigned char*)pRGBBuffer,this->m_nWidth,this->m_nHeight);

            struct v4l2_buffer putBuf;
            memset(&putBuf,0,sizeof(putBuf));
            putBuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
            putBuf.memory=V4L2_MEMORY_MMAP;
            putBuf.index=getBuf.index;
            if(ioctl(fd,VIDIOC_QBUF,&putBuf)<0)
            {
                qDebug()<<"failed to QBUF!";
                break;
            }


            if(STATE_TRACKING_START==g_GblHelp.m_nTrackingState)
            {
                if(this->m_rb->ZTryPutElement((qint8*)pRGBBuffer,this->m_nWidth*this->m_nHeight*3)<0)
                {
                    qDebug()<<"failed to put RGB to ring buffer.";
                }
            }

            QImage newImg((uchar*)pRGBBuffer,this->m_nWidth,this->m_nHeight,QImage::Format_RGB888);
            emit this->ZSigNewImgArrived(newImg);

            this->usleep(THREAD_SCHEDULE_MS);
        }
    }
    delete [] pRGBBuffer;
    pRGBBuffer=NULL;

    //stream off.
    v4l2_buf_type type2=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_STREAMOFF,&type2)<0)
    {
        qDebug()<<"failed to stream off!";
        g_GblHelp.m_bExitFlag=true;
        close(fd);
        return;
    }

    for(quint32 i=0;i<reqBuf.count;i++)
    {
        if(munmap(pIMGBuffer[i].pStart,pIMGBuffer[i].nLength)<0)
        {
            qDebug()<<"failed to munmap!";
            g_GblHelp.m_bExitFlag=true;
            close(fd);
            return;
        }
    }
    free(pIMGBuffer);
    close(fd);

    g_GblHelp.m_bExitFlag=true;
    return;
}
