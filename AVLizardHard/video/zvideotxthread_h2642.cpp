#include "zvideotxthread_h2642.h"
#include <zgblpara.h>
#include <sys/socket.h>
#include <iostream>
#include <QFile>

//for rk3399 hard encoder.
#define MODULE_TAG "AVLizard_h264_2"
extern "C"
{
#include <string.h>
#include "rk_mpi.h"

#include "mpp_env.h"
#include "mpp_mem.h"
#include "mpp_log.h"
#include "mpp_time.h"
#include "mpp_common.h"
#include "utils.h"
#include "vpu_api.h"
}

typedef struct {
    MppCodingType   type;
    RK_U32          width;
    RK_U32          height;
    MppFrameFormat  format;
    RK_U32          debug;
    RK_U32          num_frames;
} MpiEncTestCmd2;

typedef struct {
    // global flow control flag
    RK_U32 frm_eos;
    RK_U32 pkt_eos;
    RK_U32 frame_count;
    RK_U64 stream_size;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBuffer frm_buf;
    MppEncSeiMode sei_mode;

    // paramter for resource malloc
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    RK_U32 num_frames;

    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;

    // rate control runtime parameter
    RK_S32 gop;
    RK_S32 fps;
    RK_S32 bps;
} MpiEncTestData2;
//#define YUYV_CHUNK_SIZE  (640*480*2) //614400.

ZHardEncTx2Thread::ZHardEncTx2Thread(qint32 nTcpPort)
{
    this->m_bCleanup=true;
    this->m_nTcpPort=nTcpPort;
}
//aux capture -> encTxThread.
void ZHardEncTx2Thread::ZBindInFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
               QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    //in fifo.(aux capture -> encTxThread).
    this->m_freeQueueIn=freeQueue;
    this->m_usedQueueIn=usedQueue;
    this->m_mutexIn=mutex;
    this->m_condQueueEmptyIn=condQueueEmpty;
    this->m_condQueueFullIn=condQueueFull;
}
qint32 ZHardEncTx2Thread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZHardEncTx2Thread::ZStopThread()
{
    this->quit();
    this->wait(1000);
    return 0;
}
bool ZHardEncTx2Thread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}

void ZHardEncTx2Thread::run()
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(5,&cpuSet);
    if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
    {
        qDebug()<<"<Error>:failed to bind ZVideoTxThreadHard264 thread to cpu core 5.";
    }else{
        qDebug()<<"<Info>:success to bind ZVideoTxThreadHard264 thread to cpu core 5.";
    }

    char *pSpsPps=new char[BUFSIZE_1MB];
    qint32 nSpsPspLen=0;

    MPP_RET ret = MPP_OK;
    MpiEncTestCmd2  cmd_ctx;
    MpiEncTestCmd2 *cmd=&cmd_ctx;
    memset((void*)cmd, 0, sizeof(*cmd));

    //prepare parameters.
    cmd->debug=0;//0:disable dbg output,1:enable dbg output.
    cmd->width=gGblPara.m_widthCAM1;
    cmd->height=gGblPara.m_heightCAM1;
    cmd->format=MPP_FMT_YUV422_YUYV;
    cmd->type=MPP_VIDEO_CodingAVC;
    cmd->num_frames=1;
    ////////////////////////////////////////////////////////
    mpp_log("cmd parse result:\n");
    mpp_log("width      : %d\n", cmd->width);
    mpp_log("height     : %d\n", cmd->height);
    mpp_log("format     : %d\n", cmd->format);
    mpp_log("type       : %d\n", cmd->type);
    mpp_log("debug flag : %x\n", cmd->debug);
    ///////////////////////////////////////////////
    mpp_env_set_u32("mpi_debug", cmd->debug);
    ////////////////////////////////////////////////
    MpiEncTestData2 *p=NULL;
    p=mpp_calloc(MpiEncTestData2, 1);
    if(!p)
    {
        qDebug()<<"<Error>:create MpiEncTestData failed.";
        return;
    }

    // get paramter from cmd
    p->width=cmd->width;
    p->height=cmd->height;
    p->hor_stride=MPP_ALIGN(cmd->width,16);
    p->ver_stride=MPP_ALIGN(cmd->height,16);
    p->fmt=cmd->format;
    p->type=cmd->type;
    p->num_frames=cmd->num_frames;

    // update resource parameter
    if(p->fmt<=MPP_FMT_YUV420SP_VU)
    {
        p->frame_size=p->hor_stride * p->ver_stride * 3 / 2;
    }
    else if(p->fmt< MPP_FMT_YUV422_UYVY)
    {
        // NOTE: yuyv and uyvy need to double stride
        p->hor_stride *= 2;
        p->frame_size=p->hor_stride * p->ver_stride;
    } else{
        p->frame_size = p->hor_stride*p->ver_stride * 4;
    }
    p->packet_size=p->width*p->height;

    mpp_log("hor_stride      : %d\n", p->hor_stride);
    mpp_log("ver_stride     : %d\n", p->ver_stride);
    mpp_log("frame_size     : %d\n", p->frame_size);
    mpp_log("packet_size     : %d\n", p->packet_size);

    //    ret = test_ctx_init(&p, cmd);
    //    if (ret)
    //    {
    //        mpp_err_f("test data init failed ret %d\n", ret);
    //        return -1;
    //    }
    /////////////////////////////////////////////
    ret = mpp_buffer_get(NULL, &p->frm_buf, p->frame_size);
    if (ret)
    {
        qDebug("failed to get buffer for input frame ret %d\n", ret);
        return;
    }

    qDebug("mpi_enc_test encoder test start w %d h %d type %d\n",p->width, p->height, p->type);

    // encoder demo
    ret = mpp_create(&p->ctx, &p->mpi);
    if (ret)
    {
        qDebug("mpp_create failed ret %d\n", ret);
        return;
    }

    ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
    if (ret)
    {
        qDebug("mpp_init failed ret %d\n", ret);
        return;
    }

    MppApi *mpi;
    MppCtx ctx;
    MppEncCodecCfg *codec_cfg;
    MppEncPrepCfg *prep_cfg;
    MppEncRcCfg *rc_cfg;

    mpi = p->mpi;
    ctx = p->ctx;
    codec_cfg = &p->codec_cfg;
    prep_cfg = &p->prep_cfg;
    rc_cfg = &p->rc_cfg;

    /* setup default parameter */
    //p->fps = gGblPara.m_fpsCAM1*2;
    p->fps = 5;//gGblPara.m_fpsCAM1;
    p->gop = 1;
    //p->bps = p->width * p->height / 8 * p->fps;
    p->bps = p->width * p->height / 8 * p->fps*10;

    prep_cfg->change = MPP_ENC_PREP_CFG_CHANGE_INPUT |MPP_ENC_PREP_CFG_CHANGE_ROTATION |MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = p->width;
    prep_cfg->height        = p->height;
    prep_cfg->hor_stride    = p->hor_stride;
    prep_cfg->ver_stride    = p->ver_stride;
    prep_cfg->format        = p->fmt;
    prep_cfg->rotation      = MPP_ENC_ROT_0;
    ret = mpi->control(ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret)
    {
        qDebug("mpi control enc set prep cfg failed ret %d\n", ret);
        return;
    }

    rc_cfg->change  = MPP_ENC_RC_CFG_CHANGE_ALL;
    rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    rc_cfg->quality = MPP_ENC_RC_QUALITY_MEDIUM;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR)
    {
        /* constant bitrate has very small bps range of 1/16 bps */
        rc_cfg->bps_target   = p->bps;
        rc_cfg->bps_max      = p->bps * 17 / 16;
        rc_cfg->bps_min      = p->bps * 15 / 16;
    } else if (rc_cfg->rc_mode ==  MPP_ENC_RC_MODE_VBR)
    {
        if (rc_cfg->quality == MPP_ENC_RC_QUALITY_CQP)
        {
            /* constant QP does not have bps */
            rc_cfg->bps_target   = -1;
            rc_cfg->bps_max      = -1;
            rc_cfg->bps_min      = -1;
        } else {
            /* variable bitrate has large bps range */
            rc_cfg->bps_target   = p->bps;
            rc_cfg->bps_max      = p->bps * 17 / 16;
            rc_cfg->bps_min      = p->bps * 1 / 16;
        }
    }

    /* fix input / output frame rate */
    rc_cfg->fps_in_flex      = 0;
    rc_cfg->fps_in_num       = p->fps;
    rc_cfg->fps_in_denorm    = 1;
    rc_cfg->fps_out_flex     = 0;
    rc_cfg->fps_out_num      = p->fps;
    rc_cfg->fps_out_denorm   = 1;

    rc_cfg->gop              = p->gop;
    rc_cfg->skip_cnt         = 0;

    qDebug("mpi_enc_test bps %d fps %d gop %d\n",rc_cfg->bps_target, rc_cfg->fps_out_num, rc_cfg->gop);
    ret = mpi->control(ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret)
    {
        qDebug("mpi control enc set rc cfg failed ret %d\n", ret);
        return;
    }

    codec_cfg->coding = p->type;
    switch (codec_cfg->coding)
    {
    case MPP_VIDEO_CodingAVC :
    {
        codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
            * H.264 profile_idc parameter
            * 66  - Baseline profile
            * 77  - Main profile
            * 100 - High profile
            */
        codec_cfg->h264.profile=100;
        /*
            * H.264 level_idc parameter
            * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
            * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
            * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
            * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
            * 50 / 51 / 52         - 4K@30fps
            */
        codec_cfg->h264.level    = 40;
        codec_cfg->h264.entropy_coding_mode  = 1;
        codec_cfg->h264.cabac_init_idc  = 0;
        codec_cfg->h264.transform8x8_mode = 1;
    }
        break;
    case MPP_VIDEO_CodingMJPEG :
    {
        codec_cfg->jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
        codec_cfg->jpeg.quant   = 10;
    }
        break;
    case MPP_VIDEO_CodingVP8 :
    case MPP_VIDEO_CodingHEVC :
    default :
    {
        qDebug("support encoder coding type %d\n", codec_cfg->coding);
    }
        break;
    }
    ret = mpi->control(ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret)
    {
        qDebug("mpi control enc set codec cfg failed ret %d\n", ret);
        return;
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret)
    {
        qDebug("mpi control enc set sei cfg failed ret %d\n", ret);
        return;
    }

    if (p->type == MPP_VIDEO_CodingAVC)
    {
        MppPacket packet = NULL;
        ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret)
        {
            qDebug("mpi control enc get extra info failed\n");
            return;
        }

        /* get and write sps/pps for H.264 */
        if (packet)
        {
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            //save sps/pps here.
            memcpy(pSpsPps,ptr,len);
            nSpsPspLen=len;
            qDebug()<<"save sps/pps to buffer,len:"<<len;
            packet = NULL;
        }
    }


    ////////////////////////////////////////////
    char *pYUV422Buffer=new char[FIFO_SIZE];
    qDebug()<<"<MainLoop>:AuxVideoTxThread starts ["<<this->m_nTcpPort<<"].";
    this->m_bCleanup=false;

    //write encode h264 frames to local file for debugging.
#if 0
    QFile fileH2642("zsy2.h264");
    fileH2642.open(QIODevice::WriteOnly);
#endif

    while(!gGblPara.m_bGblRst2Exit)
    {
#if 1
        //tcp video aux camera.
        QTcpServer *tcpServerAux=new QTcpServer;
        int on2=1;
        int sockFd2=tcpServerAux->socketDescriptor();
        setsockopt(sockFd2,SOL_SOCKET,SO_REUSEADDR,&on2,sizeof(on2));
        if(!tcpServerAux->listen(QHostAddress::Any,this->m_nTcpPort))
        {
            qDebug()<<"<Error>:VideoTx tcp server error listen on port"<<this->m_nTcpPort;
            delete tcpServerAux;
            break;
        }

        //reset.
        gGblPara.m_nTx6805Cnt=0;

        //wait until get a new connection.
        while(!gGblPara.m_bGblRst2Exit)
        {
            //qDebug()<<"wait for tcp connection";
            if(tcpServerAux->waitForNewConnection(100))
            {
                break;
            }
        }

        //do working in tcp socket.
        QTcpSocket *tcpSocketAux=tcpServerAux->nextPendingConnection();
        if(NULL==tcpSocketAux)
        {
            qDebug()<<"<Error>: failed to get next pending connection.";
            delete tcpServerAux;
            tcpServerAux=NULL;
            continue;
        }
        //客户端连接上后，就判断服务监听端，这样只允许一个tcp连接.
        tcpServerAux->close();
        //设置连接标志，这样编码器线程就会开始工作.
        gGblPara.m_bVideoTcpConnected2=true;
        qDebug()<<"aux video connect okay.";


        //向客户端发送sps/pps.send sps/pps to main.
        QByteArray baSpsPpsLen=qint32ToQByteArray(nSpsPspLen);
        if(tcpSocketAux->write(baSpsPpsLen)<0)
        {
            qDebug()<<"<Error>:failed to tx sps/pps len,break socket.";
            delete tcpServerAux;
            break;
        }
        if(tcpSocketAux->write(pSpsPps,nSpsPspLen)<0)
        {
            qDebug()<<"<Error>:failed to tx sps/pps,break socket.";
            delete tcpServerAux;
            break;
        }

        tcpSocketAux->waitForBytesWritten(1000);
#endif

        //write encode h264 frames to local file for debugging.
#if 0
        fileH2642.write(pSpsPps,nSpsPspLen);
#endif

        //fetch yuv from queue and do encode & tx.
        while(!gGblPara.m_bGblRst2Exit)
        {

            ////////////////////////////////encode aux camera///////////////////////
            //1.fetch data from yuv queue and encode to h264 I/P frames.
            this->m_mutexIn->lock();
            while(this->m_usedQueueIn->isEmpty())//timeout 5s to check exit flag.
            {
                if(!this->m_condQueueFullIn->wait(this->m_mutexIn,5000))
                {
                    this->m_mutexIn->unlock();
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
            QByteArray *bufferIn=this->m_usedQueueIn->dequeue();
            memcpy(pYUV422Buffer,bufferIn->data(),bufferIn->size());
            this->m_freeQueueIn->enqueue(bufferIn);
            this->m_condQueueEmptyIn->wakeAll();
            this->m_mutexIn->unlock();

            //execute encode job.
            MppFrame frame = NULL;
            ret=mpp_frame_init(&frame);
            if(ret)
            {
                qDebug("mpp_frame_init failed\n");
                break;
            }
            mpp_frame_set_width(frame, p->width);
            mpp_frame_set_height(frame, p->height);
            mpp_frame_set_hor_stride(frame, p->hor_stride);
            mpp_frame_set_ver_stride(frame, p->ver_stride);
            mpp_frame_set_fmt(frame, p->fmt);
            mpp_frame_set_buffer(frame, p->frm_buf);
            p->frm_eos=0;//never reach end of streaming.
            mpp_frame_set_eos(frame, p->frm_eos);

            void *buf = mpp_buffer_get_ptr(p->frm_buf);
            //read yuv to mpp buffer.
            void *buf_y = buf;
            void *buf_u = buf_y + p->hor_stride * p->ver_stride; // NOTE: diff from gen_yuv_image
            void *buf_v = buf_u + p->hor_stride * p->ver_stride / 4; // NOTE: diff from gen_yuv_image
            //qDebug("w:%d,h:%d,hor_stride:%d,ver_stride:%d\n",p->width,p->height,p->hor_stride,p->ver_stride);

            //read yuv to mpp buffer.
            for(quint32 row=0,offset=0;row<p->height;row++)
            {
                memcpy((char*)buf_y+row*p->hor_stride,pYUV422Buffer+offset,p->width*2);
                offset+=p->width*2;
            }

            //this function works in block-mode.
            ret=mpi->encode_put_frame(ctx,frame);
            if(ret)
            {
                qDebug("<Error>:mpp encode put frame failed,reset it!\n");
                ret=p->mpi->reset(p->ctx);
                if(ret)
                {
                    qDebug()<<"<Error>:mpi->reset failed.";
                }
                continue;
            }
            MppPacket packet = NULL;
            ret = mpi->encode_get_packet(ctx,&packet);
            if(ret)
            {
                qDebug("<Error>:mpp encode get packet failed\n");
                continue;
            }
            if(packet)
            {
                // write packet to file here
                void *pH264Data2=mpp_packet_get_pos(packet);
                size_t nH264DataLen2=mpp_packet_get_length(packet);
                p->pkt_eos=mpp_packet_get_eos(packet);
                p->stream_size+=nH264DataLen2;
                p->frame_count++;
                //qDebug("aux h264 encoded frame %d size %d\n",p->frame_count,nH264DataLen2);

#if 0
                //write encode h264 frames to local file for debugging.
                fileH2642.write((const char*)pH264Data2,nH264DataLen2);
                fileH2642.flush();
#endif
#if 1
                //tx out.
                QByteArray baH264PktLen2=qint32ToQByteArray(nH264DataLen2);
                if(tcpSocketAux->write(baH264PktLen2)!=baH264PktLen2.size())
                {
                    qDebug()<<"<Error>:failed to tx h264 pkt len,break socket.";
                    mpp_packet_deinit(&packet);
                    break;
                }
                if(tcpSocketAux->write((const char*)pH264Data2,nH264DataLen2)!=nH264DataLen2)
                {
                    qDebug()<<"<Error>:failed to tx h264 data,break socket.";
                    mpp_packet_deinit(&packet);
                    break;
                }
                if(!tcpSocketAux->waitForBytesWritten(SOCKET_TX_TIMEOUT))
                {
                    qDebug()<<"tx aux failed:"<<tcpSocketAux->errorString();
                    mpp_packet_deinit(&packet);
                    break;
                }
                gGblPara.m_nTx6805Cnt++;
#endif
                mpp_packet_deinit(&packet);
            }
            this->usleep(VIDEO_THREAD_SCHEDULE_US);
        }
        tcpSocketAux->close();
        //clean here.
        //设置连接标志，这样编码器线程就会停止工作.
        gGblPara.m_bVideoTcpConnected2=false;
#if 1
        delete tcpServerAux;
        tcpServerAux=NULL;
#endif
    }

    delete [] pSpsPps;
    delete [] pYUV422Buffer;
    //clear memory after encode finishes.
    ret=p->mpi->reset(p->ctx);
    if(ret)
    {
        qDebug()<<"<Error>:mpi->reset failed.";
    }
    if(p->ctx)
    {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }

    if(p->frm_buf)
    {
        mpp_buffer_put(p->frm_buf);
        p->frm_buf = NULL;
    }
    free(p);
    this->ZDoCleanBeforeExit();
    return;
}
void ZHardEncTx2Thread::ZDoCleanBeforeExit()
{
    //set global request exit flag to notify other threads to exit.
    gGblPara.m_bGblRst2Exit=true;
    this->m_bCleanup=true;
    emit this->ZSigFinished();
    qDebug()<<"<MainLoop>:AuxVideoTxThread ends ["<<this->m_nTcpPort<<"].";
}
