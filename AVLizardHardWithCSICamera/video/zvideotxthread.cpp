#include "zvideotxthread.h"
#include <zgblpara.h>
#include <sys/socket.h>
#include <iostream>
#include <QFile>
extern "C"
{
#include <x264.h>
#include <x264_config.h>
}

#define YUYV_CHUNK_SIZE  (640*480*2) //614400.

int m_yuv422_to_yuv420(const char *in,char *out,int width,int height)
{
    char *y=out;
    char *u=out+width*height;
    char *v=out+width*height+width*height/4;
    //YU YV YU YV
    int yuv422Len=width*height*2;
    int yIndex=0;
    int uIndex=0;
    int vIndex=0;
    int is_u=1;
    for(int i=0;i<yuv422Len;i+=2)
    {
        *(y+(yIndex++))=*(in+i);
    }
    for(int i=0;i<height;i+=2)
    {
        int baseH=i*width*2;
        for(int j=baseH+1;j<baseH+width*2;j+=2)
        {
            if(is_u)
            {
                *(u+(uIndex++))=*(in+j);
                is_u=0;
            }else{
                *(v+(vIndex++))=*(in+j);
                is_u=1;
            }
        }
    }
    return 0;
}
#ifdef USE_HARD_H264_ENCODER
typedef struct{
    GstElement *_pipeline;
    GstElement *_src;
    GstElement *_enc;
    GstElement *_sink;
    GMainLoop *_loop;
    guint _sourceId;
    GstBus *_bus;

    ZRingBuffer *_rbYUV;
    qint8 *_yuv422;
    QTcpSocket *_tcpSocket;
}gst_app_t;
static gst_app_t gApp;

static gboolean read_data(gst_app_t *app)
{
    GstBuffer *buffer;
    GstMapInfo map;
    GstFlowReturn ret;
    qint32 nYUV422Len=0;
    app->_rbYUV->m_semaUsed->acquire();//已用信号量减1.
    nYUV422Len=app->_rbYUV->ZGetElement(app->_yuv422,BUFSIZE_1MB);
    app->_rbYUV->m_semaFree->release();//空闲信号量加1.
    if(nYUV422Len<=0)
    {
        qDebug()<<"<error>:h264 enc thread get yuv data length is not right.";
        return FALSE;
    }
    qDebug()<<"get yuv from queue:"<<nYUV422Len;


    //y=640*480
    //u=640*480*0.25
    //v=640*480*0.25

    //create a new empty buffer.
    buffer=gst_buffer_new_and_alloc(YUYV_CHUNK_SIZE);
    if(gst_buffer_map(buffer,&map,GST_MAP_WRITE)!=TRUE)
    {
        qDebug()<<"failed to gst_buffer_map in read_data.";
        return FALSE;
    }
    //convert yuv422 to yuv420.
    m_yuv422_to_yuv420((const char*)app->_yuv422,(char*)map.data,640,480);
    map.size=nYUV422Len;
    //push the buffer into the appsrc.
    g_signal_emit_by_name(app->_src,"push-buffer",buffer,&ret);

    gst_buffer_unmap(buffer,&map);
    gst_buffer_unref(buffer);

    if(ret!=GST_FLOW_OK)
    {
        qDebug()<<"error push-buffer";
        g_main_loop_quit(app->_loop);
        return FALSE;
    }
    return TRUE;
}
static void _AppSrc_needData(GstElement*src,guint arg0,gpointer user_data)
{
    Q_UNUSED(src);
    Q_UNUSED(arg0);
    gst_app_t *gst_app=(gst_app_t*)user_data;
    if(gst_app->_sourceId==0)
    {
        qDebug()<<"start feeding";
        gst_app->_sourceId=g_idle_add((GSourceFunc)read_data,gst_app);
    }
}
static void _AppSrc_enoughData(GstElement* object,gpointer user_data)
{
    Q_UNUSED(object);
    gst_app_t *gst_app=(gst_app_t*)user_data;
    if(gst_app->_sourceId!=0)
    {
        qDebug()<<"stop feeding";
        g_source_remove(gst_app->_sourceId);
        gst_app->_sourceId=0;
    }
}

//this function called when the appsink has received a buffer from GStreamer pipeline.
static GstFlowReturn _AppSink_newSample(GstElement* object,gpointer user_data)
{
    Q_UNUSED(object);
    gst_app_t *gst_app=(gst_app_t*)user_data;
    GstSample *sample;
    GstFlowReturn ret;
    //retrieve the buffer.
    g_signal_emit_by_name(gst_app->_sink,"pull-sample",&sample,&ret);
    if(sample)
    {
        qDebug()<<"get new data";
        GstBuffer *buffer=gst_sample_get_buffer(sample);
        qDebug()<<"size:"<<gst_buffer_get_size(buffer);
#if 0
        GstMapInfo info;
        if(gst_buffer_map(buffer,&info,GST_MAP_READ)==TRUE)
        {
            //confirm that correct buffer was received.
            //g_print("appsink get:%d,%s\n",info.size,info.data);
            g_print("appsink get:%d\n",info.size);

            if(info.size>0)
            {
                QByteArray baH264PktLen=qint32ToQByteArray(info.size);
                if(gst_app->_tcpSocket->write(baH264PktLen)<0)
                {
                    qDebug()<<"<Error>:socket write error,break it.";
                    g_main_loop_quit(gst_app->_loop);
                }
                if(gst_app->_tcpSocket->write((const char*)info.data,info.size)<0)
                {
                    qDebug()<<"<Error>:socket write error,break it.";
                    g_main_loop_quit(gst_app->_loop);
                }
                gst_app->_tcpSocket->waitForBytesWritten(100);
            }
            gst_buffer_unmap(buffer,&info);
        }
        //gst_buffer_unref((GstBuffer*)sample);
        gst_sample_unref(sample);
#endif
    }else{
        qDebug()<<"get sample is NULL.";
    }

    return GST_FLOW_OK;
}
static gboolean bus_message(GstBus *bus,GstMessage *message,gpointer user_data)
{
    Q_UNUSED(bus);
    gst_app_t *gst_app=(gst_app_t*)user_data;
    GST_DEBUG("got message %s",gst_message_type_get_name(GST_MESSAGE_TYPE(message)));
    switch(GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
    {
        GError *err=NULL;
        gchar *dbg_info=NULL;
        gst_message_parse_error(message,&err,&dbg_info);
        g_printerr("Error from element %s:%s\n",GST_OBJECT_NAME(message->src),err->message);
        g_printerr("Debugging info:%s\n",(dbg_info)?dbg_info:"none");
        g_error_free(err);
        g_free(dbg_info);
        g_main_loop_quit(gst_app->_loop);
    }
        break;
    case GST_MESSAGE_EOS:
        g_main_loop_quit(gst_app->_loop);
        break;
    default:
        break;
    }
    return TRUE;
}
#endif

#if 0
#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtplibraryversion.h"
using namespace jrtplib;
//  RTP:
//  -----------------------------------------------------------------
//  0               8              16              24              32
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | V |P|X|  CC   |M|     PT      |               SN              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                           Timestamp                           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                             SSRC                              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                             CSRC                              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                             ...                               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//  NALU:
//  -----------------
//  0               8
//  +-+-+-+-+-+-+-+-+
//  |F|NRI|   Type  |
//  +-+-+-+-+-+-+-+-+

#define MAXLEN  (RTP_DEFAULTPACKETSIZE - 100)

void checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
        exit(-1);
    }
}

static int findStartCode1(unsigned char *buf)
{
    if(buf[0]!=0 || buf[1]!=0 || buf[2]!=1)                // 判断是否为0x000001,如果是返回1
        return 0;
    else
        return 1;
}
static int findStartCode2(unsigned char *buf)
{
    if(buf[0]!=0 || buf[1]!=0 || buf[2]!=0 || buf[3]!=1)   // 判断是否为0x00000001,如果是返回1
        return 0;
    else
        return 1;
}
void naluPrintf(unsigned char *buf, unsigned int len, unsigned char type, unsigned int count)
{
    unsigned int i=0;

    printf("NALU type=%d num=%d len=%d : \n", type, count, len);

#if 0
    for(i=0; i<len; i++)
    {
        printf(" %02X", buf[i]);
        if(i%32 == 31)
            printf("\n");
    }
    printf("\n");
#endif
}
void rtpPrintf(unsigned char *buf, unsigned int len)
{
    unsigned int i=0;

    printf("RTP len=%d : \n", len);

    for(i=0; i<len; i++)
    {
        printf(" %02X", buf[i]);
        if(i%32 == 31)
            printf("\n");
    }

    printf("\n");
}
#endif

ZVideoTxThreadSoft264::ZVideoTxThreadSoft264(qint32 nTcpPort)
{
    this->m_rbYUV=NULL;
    this->m_bExitFlag=false;
    this->m_bCleanup=true;
    this->m_nTcpPort=nTcpPort;
}
qint32 ZVideoTxThreadSoft264::ZBindQueue(ZRingBuffer *rbYUV)
{
    this->m_rbYUV=rbYUV;
    return 0;
}
qint32 ZVideoTxThreadSoft264::ZStartThread()
{
    if(NULL==this->m_rbYUV)
    {
        qDebug()<<"<Error>:VideoTxThread,no bind yuv queue,can not start.";
        return -1;
    }
    this->m_bExitFlag=false;
    this->start();
    return 0;
}
qint32 ZVideoTxThreadSoft264::ZStopThread()
{
    this->m_bExitFlag=true;
    return 0;
}
bool ZVideoTxThreadSoft264::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
void ZVideoTxThreadSoft264::run()
{
#if 0
    RTPSession session;
    qDebug()<<"JRTP version:"<<RTPLibraryVersion::GetVersion().GetVersionString().c_str();
    RTPUDPv4TransmissionParams transParam;
    transParam.SetPortbase(1234);


    RTPSessionParams sessionPara;
    // IMPORTANT: The local timestamp unit MUST be set, otherwise
    //            RTCP Sender Report info will be calculated wrong
    // In this case, we'll be sending 10 samples each second, so we'll
    // put the timestamp unit to (1.0/10.0)
    sessionPara.SetOwnTimestampUnit(1.0/10.0);
    sessionPara.SetAcceptOwnPackets(true);
    int status=session.Create(sessionPara,&transParam);
    checkerror(status);

    //set the destination ip & port.
    RTPIPv4Address addr(inet_addr("127.0.0.1"),ntohl(1200));
    status=session.AddDestination(addr);
    checkerror(status);

    session.SetDefaultPayloadType(96);//96 for h264 type.
    session.SetDefaultMark(false);
    session.SetDefaultTimestampIncrement(90000.0/10.0);
    RTPTime delay(0.040);
    RTPTime startTime=RTPTime::CurrentTime();
#endif

#ifdef USE_HARD_H264_ENCODER
    gst_app_t *gst_app=&gApp;
    gst_init(NULL,NULL);
    char szTemp[64];

    //make sure the name is unique in GStreamer.
    //appsrc.
    sprintf(szTemp,"AVLizardAppSrc%d",this->m_nTcpPort);
    gst_app->_src=gst_element_factory_make("appsrc",szTemp);
    //mpph264enc.
    sprintf(szTemp,"AVLizardH264Enc%d",this->m_nTcpPort);
    gst_app->_enc=gst_element_factory_make("mpph264enc",szTemp);
    //appsink.
    sprintf(szTemp,"AVLizardAppSink%d",this->m_nTcpPort);
    gst_app->_sink=gst_element_factory_make("appsink",szTemp);
    //pipeline.
    sprintf(szTemp,"AVLizardPipeline%d",this->m_nTcpPort);
    gst_app->_pipeline=gst_pipeline_new(szTemp);

    g_assert(gst_app->_src);
    g_assert(gst_app->_enc);
    g_assert(gst_app->_sink);
    g_assert(gst_app->_pipeline);


    //set caps before link.
    //this make sure the appsrc has same pad caps compared to mpph264enc.
    g_object_set(G_OBJECT(gst_app->_src),"caps",///<
                 gst_caps_new_simple("video/x-raw",///<
                                     "format",G_TYPE_STRING,"I420",///<
                                     "width",G_TYPE_INT,640,///<
                                     "height",G_TYPE_INT,480,///<
                                     "framerate",GST_TYPE_FRACTION,15,1,NULL),///<
                 NULL);

    //this make sure the appsink has same pad caps compared to mpph264enc.
    g_object_set(G_OBJECT(gst_app->_sink),"caps",///<
                 gst_caps_new_simple("video/x-h264",///<
                                     "width",G_TYPE_INT,640,///<
                                     "height",G_TYPE_INT,480,///<
                                     "framerate",GST_TYPE_FRACTION,15,1,///<
                                     "stream-format",G_TYPE_STRING,"byte-stream",///<
                                     "alignment",G_TYPE_STRING,"au",///<
                                     "profile",G_TYPE_STRING,"high",NULL),///<
                 NULL);

    //must add elements to pipeline before linking them.
    gst_bin_add_many(GST_BIN(gst_app->_pipeline),gst_app->_src,gst_app->_enc,gst_app->_sink,NULL);
    if(gst_element_link_many(gst_app->_src,gst_app->_enc,gst_app->_sink,NULL)!=TRUE)
    {
        qDebug()<<"gst elements <appsrc-mpph264enc-appsink> link failed!";
        return;
    }
    qDebug()<<"gst elements link okay.";


    sprintf(szTemp,"%d",YUYV_CHUNK_SIZE);
    //set appsrc.
    g_object_set(G_OBJECT(gst_app->_src),///<
                 "emit-signals",TRUE,///<
                 "is-live",TRUE,///<
                 "blocksize",szTemp,///<
                 "stream-type",0,///<
                 "format",GST_FORMAT_TIME,NULL);
    g_signal_connect(gst_app->_src,"need-data",G_CALLBACK(_AppSrc_needData),gst_app);
    g_signal_connect(gst_app->_src,"enough-data",G_CALLBACK(_AppSrc_enoughData),gst_app);


    //set appsink.
    g_object_set(G_OBJECT(gst_app->_sink),///<
                 "emit-signals",TRUE,NULL);
    g_signal_connect(gst_app->_sink,"new-sample",G_CALLBACK(_AppSink_newSample),gst_app);

    //add monitor bus.
    gst_app->_bus=gst_pipeline_get_bus(GST_PIPELINE(gst_app->_pipeline));
    g_assert(gst_app->_bus);
    gst_bus_add_watch(gst_app->_bus,(GstBusFunc)bus_message,gst_app);

#else
    int iNal=0;
    x264_t *pHandle=NULL;
    x264_nal_t *pNals=NULL;
    x264_param_t *pParam=(x264_param_t*)malloc(sizeof(x264_param_t));
    x264_picture_t *pPicIn=(x264_picture_t*)malloc(sizeof(x264_picture_t));

    //static const char * const x264_tune_names[] = { "film", "animation", "grain", "stillimage", "psnr", "ssim", "fastdecode", "zerolatency", 0 };
    //    if(x264_param_default_preset(pParam,"zerolatency", NULL)<0)
    //    {
    //        qDebug()<<"<error>: error at x264_param_default_preset().";
    //        return;
    //    }

    //set default parameters.
    //x264_param_default(pParam);
    x264_param_default_preset(pParam,"ultrafast","zerolatency");

    pParam->i_csp=X264_CSP_I420;
    pParam->b_vfr_input = 0;
    pParam->b_repeat_headers = 1;
    pParam->b_annexb = 1;

    pParam->i_threads=X264_SYNC_LOOKAHEAD_AUTO;

    //set width*height.
    pParam->i_width=640;
    pParam->i_height=480;

    pParam->i_frame_total=0;
    pParam->i_keyint_max=10;
    pParam->rc.i_lookahead=0;
    pParam->i_bframe=0;

    pParam->b_open_gop=0;
    pParam->i_bframe_pyramid=0;
    pParam->i_bframe_adaptive=X264_B_ADAPT_TRELLIS;
    pParam->i_fps_num=15;
    pParam->i_fps_den=1;


    //static const char * const x264_profile_names[] = { "baseline", "main", "high", "high10", "high422", "high444", 0 };
    if(x264_param_apply_profile(pParam,x264_profile_names[4])<0)
    {
        qDebug()<<"<error>:error at x264_param_apply_profile().";
        return;
    }

    ////////////////////////////////////////////
    pHandle=x264_encoder_open(pParam);
    if(pHandle==NULL)
    {
        qDebug()<<"<error>:error at x264_encoder_open().";
        return;
    }

    /* yuyv 4:2:2 packed */
    //allocate a picture in.//
    //    pPicIn->img.i_csp=X264_CSP_YUYV;
    //    pPicIn->img.i_plane=1;
    if(x264_picture_alloc(pPicIn,X264_CSP_I420,640,480)<0)
    {
        qDebug()<<"<error>:error at x264_picture_alloc().";
        return;
    }


    long int pts=0;
    char *pEncodeBuffer=new char[BUFSIZE_1MB];

    //    QFile fileH264(QString("yantai.h264"));
    //    if(fileH264.open(QIODevice::WriteOnly))
    //    {

    //    }
    char *pYUV422Buffer=new char[BUFSIZE_1MB];
    char *yuv420pBuffer=new char[BUFSIZE_1MB];
    bool bIPBFlag=true;

    //    char *txBuffer=new char[BUFSIZE_1MB];
#endif

    qDebug()<<"<MainLoop>:VideoTxThread starts ["<<this->m_nTcpPort<<"].";
    this->m_bCleanup=false;

    //bind main img tx thread to cpu core 4.
    //bind aux img tx thread to cpu core 5.
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(4,&cpuSet);
    if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
    {
        qDebug()<<"<Error>:failed to bind MainImgTx thread to cpu core 4.";
    }else{
        qDebug()<<"<Info>:success to bind MainImgTx thread to cpu core 4.";
    }


    while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
    {
        QTcpServer *tcpServer=new QTcpServer;
        int on=1;
        int sockFd=tcpServer->socketDescriptor();
        setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
        if(!tcpServer->listen(QHostAddress::Any,this->m_nTcpPort))
        {
            qDebug()<<"<Error>:VideoTx tcp server error listen on port"<<this->m_nTcpPort;
            delete tcpServer;
            break;
        }
        //wait until get a new connection.
        while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
        {
            //qDebug()<<"wait for tcp connection";
            if(tcpServer->waitForNewConnection(1000*10))
            {
                break;
            }
        }
        if(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
        {
            QTcpSocket *tcpSocket=tcpServer->nextPendingConnection();
            if(NULL==tcpSocket)
            {
                qDebug()<<"<Error>: failed to get next pending connection.";
            }else{
                //qDebug()<<"new connection,close tcp server.";
                //客户端连接上后，就判断服务监听端，这样只允许一个tcp连接.
                tcpServer->close();
                //设置连接标志，这样编码器线程就会开始工作.
                switch(this->m_nTcpPort)
                {
                case TCP_PORT_VIDEO:
                    gGblPara.m_bVideoTcpConnected=true;
                    break;
                case TCP_PORT_VIDEO2:
                    gGblPara.m_bVideoTcpConnected2=true;
                    break;
                default:
                    break;
                }

                //向客户端发送音频数据包.
#if 0
                MPP_RET ret = MPP_OK;
                MpiEncTestCmd  cmd_ctx;
                MpiEncTestCmd *cmd=&cmd_ctx;
                memset((void*)cmd, 0, sizeof(*cmd));
                //parse out.
                cmd->format=MPP_FMT_YUV422_YUYV; //YUV.
                cmd->debug=0;
                cmd->width=640;
                cmd->height=480;
                cmd->type=MPP_VIDEO_CodingAVC;//H264.
                cmd->num_frames=1;//input max number of frames.

                //enable debug output.
                mpp_env_set_u32("mpi_debug", cmd->debug);

                MpiEncTestData *p=mpp_calloc(MpiEncTestData, 1);
                if (!p)
                {
                    qDebug()<<"failed to calloc memory for MpiEncTestData";
                    return;
                }
                // get paramter from cmd
                p->width        = cmd->width;
                p->height       = cmd->height;
                p->hor_stride   = MPP_ALIGN(cmd->width, 16);
                p->ver_stride   = MPP_ALIGN(cmd->height, 16);
                p->fmt          = cmd->format;
                p->type         = cmd->type;
                if (cmd->type == MPP_VIDEO_CodingMJPEG)
                {
                    cmd->num_frames = 1;
                }
                p->num_frames   = cmd->num_frames;

                // update resource parameter
                if (p->fmt <= MPP_FMT_YUV420SP_VU)
                {
                    p->frame_size = p->hor_stride * p->ver_stride * 3 / 2;
                }else if (p->fmt <= MPP_FMT_YUV422_UYVY) {
                    // NOTE: yuyv and uyvy need to double stride
                    p->hor_stride *= 2;
                    p->frame_size = p->hor_stride * p->ver_stride;
                } else{
                    p->frame_size = p->hor_stride * p->ver_stride * 4;
                }
                p->packet_size  = p->width * p->height;

                //initial buffer.
                ret = mpp_buffer_get(NULL, &p->frm_buf, p->frame_size);
                if (ret)
                {
                    qDebug()<<"failed to get buffer for input frame ret "<<ret;
                    return;
                }

                qDebug()<<"mpi_enc_test encoder test start w:"<<p->width<<" h:"<<p->height<<"type:"<<p->type;

                // encoder demo
                ret = mpp_create(&p->ctx, &p->mpi);
                if (ret)
                {
                    qDebug()<<"mpp_create failed ret "<<ret;
                    return;
                }

                ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
                if (ret)
                {
                    qDebug()<<"mpp_init failed ret "<<ret;
                    return;
                }

                ret = test_mpp_setup(p);
                if (ret)
                {
                    qDebug()<<"test mpp setup failed ret "<<ret;
                    return;
                }

                //                ret = test_mpp_run(p);
                //                if (ret)
                //                {
                //                    qDebug()<<"test mpp run failed ret "<<ret;
                //                    return -1;
                //                }
                while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
                {
                    qint32 nYUV422Len=0;
                    if((nYUV422Len=this->m_rbYUV->ZGetElement((qint8*)pYUV422Buffer,BUFSIZE_1MB))<=0)
                    {
                        qDebug()<<"<Error>:h264 enc thread get yuv data length is not right.";
                        break;
                    }

                    MppApi *mpi;
                    MppCtx ctx;
                    mpi = p->mpi;
                    ctx = p->ctx;

                    if (p->type == MPP_VIDEO_CodingAVC)
                    {
                        MppPacket packet = NULL;
                        ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
                        if (ret)
                        {
                            qDebug()<<"mpi control enc get extra info failed";
                        }

                        /* get and write sps/pps for H.264 */
                        if (packet)
                        {
                            void *ptr   = mpp_packet_get_pos(packet);
                            size_t len  = mpp_packet_get_length(packet);

                            qDebug()<<"get sps/pps okay:"<<len;
                            //            if (p->fp_output)
                            //                fwrite(ptr, 1, len, p->fp_output);
                            packet = NULL;
                        }
                    }

                    //do encode.
                    MppFrame frame = NULL;
                    MppPacket packet = NULL;
                    void *buf = mpp_buffer_get_ptr(p->frm_buf);

                    //                    if (p->fp_input)
                    //                    {
                    //                        ret = read_yuv_image(buf, p->fp_input, p->width, p->height,p->hor_stride, p->ver_stride, p->fmt);
                    //                        if (ret == MPP_NOK  || feof(p->fp_input))
                    //                        {
                    //                            mpp_log("found last frame. feof %d\n", feof(p->fp_input));
                    //                            p->frm_eos = 1;
                    //                        } else if (ret == MPP_ERR_VALUE)
                    //                        {
                    //                            goto RET;
                    //                        }
                    //                    } else {
                    //                        ret = fill_yuv_image(buf, p->width, p->height, p->hor_stride,p->ver_stride, p->fmt, p->frame_count);
                    //                        if (ret)
                    //                        {
                    //                            goto RET;
                    //                        }
                    //                    }
                    void *buf_y = buf;
                    void *buf_u = buf_y + p->hor_stride * p->ver_stride; // NOTE: diff from gen_yuv_image
                    void *buf_v = buf_u + p->hor_stride * p->ver_stride / 4; // NOTE: diff from gen_yuv_image

                    memcpy(buf_y,pYUV422Buffer,307200);
                    memcpy(buf_u,pYUV422Buffer+307200,76800);
                    memcpy(buf_v,pYUV422Buffer+384000,76800);

                    ret = mpp_frame_init(&frame);
                    if (ret)
                    {
                        mpp_err_f("mpp_frame_init failed\n");
                    }

                    mpp_frame_set_width(frame, p->width);
                    mpp_frame_set_height(frame, p->height);
                    mpp_frame_set_hor_stride(frame, p->hor_stride);
                    mpp_frame_set_ver_stride(frame, p->ver_stride);
                    mpp_frame_set_fmt(frame, p->fmt);
                    mpp_frame_set_buffer(frame, p->frm_buf);
                    mpp_frame_set_eos(frame, p->frm_eos);

                    ret = mpi->encode_put_frame(ctx, frame);
                    if (ret) {
                        qDebug()<<"mpp encode put frame failed\n";
                    }

                    ret = mpi->encode_get_packet(ctx, &packet);
                    if (ret) {
                        qDebug()<<"mpp encode get packet failed\n";
                    }

                    if (packet)
                    {
                        // write packet to file here
                        void *ptr   = mpp_packet_get_pos(packet);
                        size_t len  = mpp_packet_get_length(packet);

                        p->pkt_eos = mpp_packet_get_eos(packet);

                        //            if (p->fp_output)
                        //                fwrite(ptr, 1, len, p->fp_output);

                        qDebug()<<"encode one pkt okay.";


                        mpp_packet_deinit(&packet);

                        mpp_log_f("encoded frame %d size %d\n", p->frame_count, len);
                        p->stream_size += len;
                        p->frame_count++;

                        if (p->pkt_eos)
                        {
                            mpp_log("found last packet\n");
                            mpp_assert(p->frm_eos);
                        }
                    }

                    if (p->num_frames && p->frame_count >= p->num_frames)
                    {
                        mpp_log_f("encode max %d frames", p->frame_count);
                    }
                    qDebug()<<"encode okay";
                }

                ret = p->mpi->reset(p->ctx);
                if (ret)
                {
                    qDebug()<<"mpi->reset failed";
                    return;
                }


                if (p->ctx)
                {
                    mpp_destroy(p->ctx);
                    p->ctx = NULL;
                }

                if (p->frm_buf)
                {
                    mpp_buffer_put(p->frm_buf);
                    p->frm_buf = NULL;
                }

                if (MPP_OK == ret)
                {
                    mpp_log("mpi_enc_test success total frame %d bps %lld\n",
                            p->frame_count, (RK_U64)((p->stream_size * 8 * p->fps) / p->frame_count));
                }else{
                    mpp_err("mpi_enc_test failed ret %d\n", ret);
                }

                MPP_FREE(p);
                mpp_env_set_u32("mpi_debug", 0x0);

#endif
                int fileIndex=0;
                while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
                {
                    //1.fetch data from yuv queue and encode to h264 I/P frames.
                    x264_picture_t picOut;
                    int nEncodeBytes=0;
                    qint32 nYUV422Len=0;
                    if((nYUV422Len=this->m_rbYUV->ZGetElement((qint8*)pYUV422Buffer,BUFSIZE_1MB))<=0)
                    {
                        qDebug()<<"<Error>:h264 enc thread get yuv data length is not right.";
                        break;
                    }


                    //                    //write yuv to file.
                    //                    QFile file(QString("yuv-%1-%2.yuv").arg(this->m_nTcpPort).arg(fileIndex));
                    //                    if(file.open(QIODevice::WriteOnly))
                    //                    {
                    //                        file.write(pYUV422Buffer,nYUV422Len);
                    //                        file.close();
                    //                    }
                    //                    fileIndex++;
                    //                    if(fileIndex>10)
                    //                    {
                    //                        break;
                    //                       exit(0);
                    //                    }
                    //2.do h264 encode.
                    int yuv420Length=640*480*1.5;
                    //y=640*480
                    //u=640*480*0.25
                    //v=640*480*0.25
                    m_yuv422_to_yuv420(pYUV422Buffer,yuv420pBuffer,640,480);

                    char *y=(char*)pPicIn->img.plane[0];
                    char *u=(char*)pPicIn->img.plane[1];
                    char *v=(char*)pPicIn->img.plane[2];

                    memcpy(y,yuv420pBuffer,307200);
                    memcpy(u,yuv420pBuffer+307200,76800);
                    memcpy(v,yuv420pBuffer+384000,76800);

                    //一个I帧一个P帧，这样节省传输带宽.
                    //IP,IP,IP,IP....
                    if(bIPBFlag)
                    {
                        pPicIn->i_type=X264_TYPE_KEYFRAME;
                    }else{
                        pPicIn->i_type=X264_TYPE_P;
                    }
                    bIPBFlag=!bIPBFlag;
                    //pPicIn->i_type=X264_TYPE_P;//
                    //pPicIn->i_type=X264_TYPE_IDR;
                    //pPicIn->i_type=X264_TYPE_KEYFRAME;
                    //pPicIn->i_type=X264_TYPE_B;
                    //pPicIn->i_type=X264_TYPE_AUTO;

                    //i_pts参数需要递增，否则会出现警告:x264[warning]:non-strictly-monotonic PTS.
                    pPicIn->i_pts=pts++;
                    //进行编码.
                    qint32 ret=x264_encoder_encode(pHandle,&pNals,&iNal,pPicIn,&picOut);
                    if(ret<0)
                    {
                        qDebug()<<"<error>:x264 encode failed.";
                        continue;
                    }
                    for(int j=0;j<iNal;j++)
                    {
                        /* Size of payload (including any padding) in bytes. */
                        //int     i_payload;
                        /* If param->b_annexb is set, Annex-B bytestream with startcode.
                             * Otherwise, startcode is replaced with a 4-byte size.
                             * This size is the size used in mp4/similar muxing; it is equal to i_payload-4 */
                        //uint8_t *p_payload;
                        //fwrite(pNals[j].p_payload,1,pNals[j].i_payload,fp);
                        //qDebug()<<"encode size:"<<pNals[j].i_payload;
                        memcpy(pEncodeBuffer+nEncodeBytes,pNals[j].p_payload,pNals[j].i_payload);
                        nEncodeBytes+=pNals[j].i_payload;
                    }

                    //flush encode when not all I frames.
#if 0
                    while(1)
                    {
                        if(0==x264_encoder_encode(pHandle,&pNals,&iNal,NULL,&picOut))
                        {
                            break;
                        }
                        for(int j=0;j<iNal;j++)
                        {
                            memcpy(pEncodeBuffer+nEncodeBytes,pNals[j].p_payload,pNals[j].i_payload);
                            nEncodeBytes+=pNals[j].i_payload;
                        }
                    }
#endif
                    //qDebug()<<"encode bytes:"<<nEncodeBytes;

                    //2.Audio Packet format: pkt len + pkt data.
                    if(nEncodeBytes>0)
                    {
                        QByteArray baH264PktLen=qint32ToQByteArray(nEncodeBytes);
                        if(tcpSocket->write(baH264PktLen)<0)
                        {
                            qDebug()<<"<Error>:socket write error,break it.";
                            break;
                        }
                        if(tcpSocket->write(pEncodeBuffer,nEncodeBytes)<0)
                        {
                            qDebug()<<"<Error>:socket write error,break it.";
                            break;
                        }
                        tcpSocket->waitForBytesWritten(100);
                    }
                    this->usleep(VIDEO_THREAD_SCHEDULE_US);
                }

                //设置连接标志，这样编码器线程就会停止工作.
                switch(this->m_nTcpPort)
                {
                case TCP_PORT_VIDEO:
                    gGblPara.m_bVideoTcpConnected=false;
                    break;
                case TCP_PORT_VIDEO2:
                    gGblPara.m_bVideoTcpConnected2=false;
                    break;
                default:
                    break;
                }
            }
        }
        delete tcpServer;
    }



    qDebug()<<"<MainLoop>:VideoTxThread ends ["<<this->m_nTcpPort<<"].";
    //此处设置本线程退出标志.
    //同时设置全局请求退出标志，请求其他线程退出.
    //gGblPara.m_bVideoTxThreadExitFlag=true;
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}
