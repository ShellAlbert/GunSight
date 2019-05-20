#include "zaudiotxthread.h"
#include "../zgblpara.h"
#include <opus/opus.h>
#include <opus/opus_multistream.h>
#include <opus/opus_defines.h>
#include <opus/opus_types.h>
#include <sys/types.h>
#include <sys/socket.h>

#define AUDIO_ENC_OPUS  1
ZAudioTxThread::ZAudioTxThread()
{
    this->m_bExitFlag=false;
    this->m_bCleanup=true;
}
void ZAudioTxThread::ZBindFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                               QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueue=freeQueue;
    this->m_usedQueue=usedQueue;
    this->m_mutex=mutex;
    this->m_condQueueEmpty=condQueueEmpty;
    this->m_condQueueFull=condQueueFull;
}
qint32 ZAudioTxThread::ZStartThread()
{
    this->m_bExitFlag=false;
    this->start();
    return 0;
}
qint32 ZAudioTxThread::ZStopThread()
{
    this->m_bExitFlag=true;
    return 0;
}
bool ZAudioTxThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
void ZAudioTxThread::run()
{
#ifdef AUDIO_ENC_OPUS
    char *pEncBuffer=new char[PERIOD_SIZE];
    char *pOpusTails=new char[OPUS_BLKFRM_SIZEx2];
    qint32 nOpusTailsLen=0;
    int err;
    OpusMSEncoder *encoder;
#endif
    //48KHz sample rate, 2 channels.
    /** Allocates and initializes a multistream encoder state.
      * Call opus_multistream_encoder_destroy() to release
      * this object when finished.

      * @param coupled_streams <tt>int</tt>: Number of coupled (2 channel) streams
      *                                      to encode.
      *                                      This must be no larger than the total
      *                                      number of streams.
      *                                      Additionally, The total number of
      *                                      encoded channels (<code>streams +
      *                                      coupled_streams</code>) must be no
      *                                      more than the number of input channels.
      * @param[in] mapping <code>const unsigned char[channels]</code>: Mapping from
      *                    encoded channels to input channels, as described in
      *                    @ref opus_multistream. As an extra constraint, the
      *                    multistream encoder does not allow encoding coupled
      *                    streams for which one channel is unused since this
      *                    is never a good idea.
      * @param application <tt>int</tt>: The target encoder application.
      *                                  This must be one of the following:
      * <dl>
      * <dt>#OPUS_APPLICATION_VOIP</dt>
      * <dd>Process signal for improved speech intelligibility.</dd>
      * <dt>#OPUS_APPLICATION_AUDIO</dt>
      * <dd>Favor faithfulness to the original input.</dd>
      * <dt>#OPUS_APPLICATION_RESTRICTED_LOWDELAY</dt>
      * <dd>Configure the minimum possible coding delay by disabling certain modes
      * of operation.</dd>
      * </dl>
      * @param[out] error <tt>int *</tt>: Returns #OPUS_OK on success, or an error
      *                                   code (see @ref opus_errorcodes) on
      *                                   failure.
      */
    //1.Sampling rate of the input signal (in Hz).48000.
    //2.Number of channels in the input signal.2 channels.
    //3.The total number of streams to encode from the input.This must be no more than the number of channels. 0.
    //4.
    /*
     *       opus_int32 Fs,
      int channels,
      int mapping_family,
      int *streams,
      int *coupled_streams,
      unsigned char *mapping,
      int application,
      int *error
      */
#ifdef AUDIO_ENC_OPUS
    int mapping_family=0;
    int streams=1;
    int coupled_streams=1;
    unsigned char stream_map[255];
    encoder=opus_multistream_surround_encoder_create(SAMPLE_RATE,CHANNELS_NUM,mapping_family,&streams,&coupled_streams,stream_map,OPUS_APPLICATION_AUDIO,&err);
    if(err!=OPUS_OK || encoder==NULL)
    {
        qDebug()<<"<Error>:error at create opus encode "<<opus_strerror(err);
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
#endif
#if 0
    encoder=opus_encoder_create(SAMPLE_RATE,CHANNELS_NUM,OPUS_APPLICATION_AUDIO,&err);
    if(err!=OPUS_OK || encoder==NULL)
    {
        qDebug()<<"<error>:error at opus_encoder_create().";
        return;
    }

    if(opus_encoder_ctl(encoder,OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND))!=OPUS_OK)
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_BITRATE(OPUS_AUTO))<0)
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE))<0)
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_VBR(1))<0)//0:CBR,1:VBR.
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_VBR_CONSTRAINT(1))<0)//0:Unconstrained VBR,1:Constrained VBR.
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_COMPLEXITY(5))<0)//range:0~10.
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_FORCE_CHANNELS(CHANNELS_NUM))<0)//1:Forced Mono,2:Forced Stereo.
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_APPLICATION(OPUS_APPLICATION_VOIP))<0)
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_INBAND_FEC(1))<0)//0:disabled,1:enabled.
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
    if(opus_encoder_ctl(encoder,OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_60_MS))<0)
    {
        qDebug()<<"<error>:error at opus_encoder_ctl().";
        return;
    }
#endif
    /** Encodes an Opus frame.
      * @param [in] st <tt>OpusEncoder*</tt>: Encoder state
      * @param [in] pcm <tt>opus_int16*</tt>: Input signal (interleaved if 2 channels). length is frame_size*channels*sizeof(opus_int16)
      * @param [in] frame_size <tt>int</tt>: Number of samples per channel in the
      *                                      input signal.
      *                                      This must be an Opus frame size for
      *                                      the encoder's sampling rate.
      *                                      For example, at 48 kHz the permitted
      *                                      values are 120, 240, 480, 960, 1920,
      *                                      and 2880.
      *                                      Passing in a duration of less than
      *                                      10 ms (480 samples at 48 kHz) will
      *                                      prevent the encoder from using the LPC
      *                                      or hybrid modes.
      * @param [out] data <tt>unsigned char*</tt>: Output payload.
      *                                            This must contain storage for at
      *                                            least \a max_data_bytes.
      * @param [in] max_data_bytes <tt>opus_int32</tt>: Size of the allocated
      *                                                 memory for the output
      *                                                 payload. This may be
      *                                                 used to impose an upper limit on
      *                                                 the instant bitrate, but should
      *                                                 not be used as the only bitrate
      *                                                 control. Use #OPUS_SET_BITRATE to
      *                                                 control the bitrate.
      * @returns The length of the encoded packet (in bytes) on success or a
      *          negative error code (see @ref opus_errorcodes) on failure.
      */
    //hold the pcm data.
    char *pPCMBuffer=new char[PERIOD_SIZE];
    qint32 nPCMLen=0;

    char *txBuffer=new char[PERIOD_SIZE];
    qDebug()<<"<MainLoop>:AudioTxThread starts.";
    this->m_bCleanup=false;
    //bind thread to cpu core.
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3,&cpuSet);
    if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
    {
        qDebug()<<"<Error>:failed to bind AudioTx thread to cpu core 3.";
    }else{
        qDebug()<<"<Info>:success to bind AudioTx thread to cpu core 3.";
    }

    while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
    {
        QTcpServer *tcpServer=new QTcpServer;
        int on=1;
        int sockFd=tcpServer->socketDescriptor();
        setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
        if(!tcpServer->listen(QHostAddress::Any,TCP_PORT_AUDIO))
        {
            qDebug()<<"<Error>: audio tx tcp server error listen on port"<<TCP_PORT_AUDIO;
            delete tcpServer;
            //set global request to exit flag to cause other threads to exit.
            gGblPara.m_bGblRst2Exit=true;
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
                qDebug()<<"<Error>:AudioTxThread,failed to get next pending connection.";
            }else{
                //qDebug()<<"new connection,close tcp server.";
                //客户端连接上后，就判断服务监听端，这样只允许一个tcp连接.
                tcpServer->close();
                //设置连接标志，这样编码器线程就会开始工作.
                gGblPara.m_audio.m_bAudioTcpConnected=true;
                //qDebug()<<"audio connected.";

                //向客户端发送音频数据包.
                while(!gGblPara.m_bGblRst2Exit && !this->m_bExitFlag)
                {
                    //main loop exit flag.
                    bool bSocketBreaking=false;

                    //1.fetch data from clear queue and enc to opus.
                    //                    if((nPCMLen=this->m_rbTx->ZGetElement((qint8*)pPCMBuffer,BLOCK_SIZE))<0)
                    //                    {
                    //                        qDebug()<<"<Error>:timeout to get opus data from encode queue.";
                    //                        continue;
                    //                    }
                    this->m_mutex->lock();
                    while(this->m_usedQueue->isEmpty())
                    {//timeout 5s to check exit flag.
                        if(!this->m_condQueueFull->wait(this->m_mutex,5000))
                        {
                            this->m_mutex->unlock();
                            if(gGblPara.m_bGblRst2Exit)
                            {
                                goto ExceptHandler;
                            }
                        }
                    }
                    QByteArray *pcmNs=this->m_usedQueue->dequeue();
                    nPCMLen=PERIOD_SIZE;
                    this->m_mutex->unlock();


#ifdef AUDIO_ENC_OPUS
                    //encode pcm to opus.
                    qint32 nNewPCMBytes=nPCMLen;
                    qint32 nPaddingBytes=0;
                    //如果小尾巴缓冲区有数据，则从新数据中复制一段至小尾巴缓冲区，拼成完整一帧，然后编码。
                    if(nOpusTailsLen>0)
                    {
                        //我们还需要多少数据才能拼成一个完整的帧.
                        nPaddingBytes=OPUS_BLKFRM_SIZEx2-nOpusTailsLen;
                        if(nPaddingBytes>=0)
                        {
                            //将新的PCM数据复制一段至小尾巴缓冲区拼成一帧.
                            memcpy(pOpusTails+nOpusTailsLen,pcmNs->data(),nPaddingBytes);

                            //执行编码操作.
                            //48000Hz,2 Channels.
                            //frame_size is per channel size.so here is 2880.
                            //but because we are 2 channels.so the length of pcm data should be 2880*2.
                            //frameSize in 16 bit sample here we choose 2880,so the input pcm buffer size is 2880*sizeof(opus_int16) for mono channel.
                            //but we are using two channels,so here is 2880*sizeof(opus_int16)*2.
                            qint32 nBytes=opus_multistream_encode(encoder,(const opus_int16*)pOpusTails,OPUS_SAMPLE_FRMSIZE,(unsigned char*)pEncBuffer,PERIOD_SIZE);
                            //qint32 nBytes=opus_multistream_encode_float(encoder,(const float *)pOpusTails,OPUS_SAMPLE_FRMSIZE,(unsigned char*)pEncBuffer,BLOCK_SIZE);
                            if(nBytes<0)
                            {
                                qDebug()<<"<Error>:PCMEncThread,error at opus_encode(),"<<opus_strerror(nBytes);
                            }else if(nBytes==0)
                            {
                                qDebug()<<"<Warning>:AudioTx,opus encode bytes is zero.";
                            }else if(nBytes>0){
                                //qDebug()<<"<opus>:tail encode okay,pcm="<<OPUS_SAMPLE_FRMSIZE<<",ret="<<nBytes;
                                //新的PCM数据量将减少.
                                nNewPCMBytes-=nPaddingBytes;
                                nOpusTailsLen+=nPaddingBytes;

                                //qDebug()<<"we need "<<nPaddingBytes<<",new pcm remaings:"<<nNewPCMBytes;

                                //after encode,send it to client.
                                //Audio Packet format: pkt len + pkt data.
                                QByteArray baOpusPktLen=qint32ToQByteArray(nBytes);
                                //qDebug("%d:%02x %02x %02x %02x\n",baOpusData.size(),(uchar)baOpusPktLen.at(0),(uchar)baOpusPktLen.at(1),(uchar)baOpusPktLen.at(2),(uchar)baOpusPktLen.at(3));
                                if(tcpSocket->write(baOpusPktLen)<0)
                                {
                                    qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                    bSocketBreaking=true;
                                }
                                if(tcpSocket->write(pEncBuffer,nBytes)<0)
                                {
                                    qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                    bSocketBreaking=true;
                                }
                                if(!tcpSocket->waitForBytesWritten(1000))
                                {
                                    qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                    bSocketBreaking=true;
                                }
                            }
                        }
                        //reset.
                        nOpusTailsLen=0;
                    }
                    //处理完上一次遗留的小尾巴数据了，现在开始正式数据新的PCM数据，但此处要跳过之前已经处理过的数据.
                    qint32 nBlocks=nNewPCMBytes/OPUS_BLKFRM_SIZEx2;//(OPUS_BLKFRM_SIZE*CHANNELS_NUM*sizeof(opus_int16));
                    qint32 nRemainBytes=nNewPCMBytes%OPUS_BLKFRM_SIZEx2;//(OPUS_BLKFRM_SIZE*CHANNELS_NUM*sizeof(opus_int16));

                    //qDebug()<<"blocks="<<nBlocks<<",remaing bytes:"<<nRemainBytes;
                    qint32 nOffsetIndex=0;
                    for(qint32 i=0;i<nBlocks;i++)
                    {
                        //Input signal (interleaved if 2 channels). length is frame_size*channels*sizeof(opus_int16).
                        //Number of samples per channel in the input signal.
                        //
                        //pcmData输入数据(双声道即为交错存储模式),长度应该=frame_size*channels*sizeof(opus_int16),采样个数×通道数×16（采样位数）

                        //输入音频数据的每个声道的采样数量，这一定是一个opus框架编码器采样率的大小
                        //例如，当采样率48KHz时，采样数量允许的数值为120,240,480,960,1920,2880.
                        //传递一个持续时间少于10ms的音频数据(480个样本48KHz）编码器将不会使用LPC或混合模式.

                        //关于采样率与采样数量的关系
                        //opus_encode()/opus_encode_float()在调用时必须使用一帧的音频数据（2.5ms,5ms,10ms,20ms,40ms,60ms).
                        //如果采样率为48KHz，则有如下：
                        //采样频率Fm=48KHz
                        //采样间隔T=1/Fm=1/48000=1/48ms
                        //当T0=2.5ms时，则有采样数量N=T0/T=2.5ms/(1/48ms)=120.
                        //当T0=5.0ms时，则有采样数量N=T0/T=5.0ms/(1/48ms)=240.
                        //当T0=10.0ms时，则有采样数量N=T0/T=10.0ms/(1/48ms)=480.
                        //当T0=20.0ms时，则有采样数量N=T0/T=20.0ms/(1/48ms)=960.
                        //当T0=40.0ms时，则有采样数量N=T0/T=40.0ms/(1/48ms)=1920.
                        //当T0=60.0ms时，则有采样数量N=T0/T=60.0ms/(1/48ms)=2880.

                        const opus_int16 *pcmData=(int16_t*)(pcmNs->data()+nPaddingBytes+nOffsetIndex);
                        qint32 nBytes=opus_multistream_encode(encoder,pcmData,OPUS_SAMPLE_FRMSIZE,(unsigned char*)pEncBuffer,PERIOD_SIZE);
                        //qint32 nBytes=opus_multistream_encode_float(encoder,(const float *)pcmData,OPUS_SAMPLE_FRMSIZE,(unsigned char*)pEncBuffer,BLOCK_SIZE);
                        if(nBytes<0)
                        {
                            qDebug()<<"<Error>:PCMEncThread,error at opus_encode(),"<<opus_strerror(nBytes);
                        }else if(nBytes==0)
                        {
                            qDebug()<<"<Warning>:AudioTx,opus encode bytes is zero.";
                        }else if(nBytes>0){
                            //qDebug()<<"<opus>:encode okay,pcm="<<OPUS_SAMPLE_FRMSIZE<<",opus="<<nBytes;

                            //after encode,send it to client.
                            //Audio Packet format: pkt len + pkt data.
                            QByteArray baOpusPktLen=qint32ToQByteArray(nBytes);
                            //qDebug("%d:%02x %02x %02x %02x\n",baOpusData.size(),(uchar)baOpusPktLen.at(0),(uchar)baOpusPktLen.at(1),(uchar)baOpusPktLen.at(2),(uchar)baOpusPktLen.at(3));
                            if(tcpSocket->write(baOpusPktLen)<0)
                            {
                                qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                bSocketBreaking=true;
                            }
                            if(tcpSocket->write(pEncBuffer,nBytes)<0)
                            {
                                qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                bSocketBreaking=true;
                            }
                            if(!tcpSocket->waitForBytesWritten(1000))
                            {
                                qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                                bSocketBreaking=true;
                            }
                        }
                        nOffsetIndex+=OPUS_BLKFRM_SIZEx2;//OPUS_BLKFRM_SIZE*CHANNELS_NUM*sizeof(opus_int16);
                        //qDebug()<<"offsetIndex="<<nOffsetIndex;
                    }
                    //如果存在遗留字节则当作小尾巴处理，复制到尾巴缓冲区作为下一帧数据的头部数据
                    if(nRemainBytes>0)
                    {
                        char *pTailBytes=(char*)(pcmNs->data()+nPaddingBytes+nOffsetIndex);
                        memcpy(pOpusTails,pTailBytes,nRemainBytes);
                        nOpusTailsLen=nRemainBytes;
                        //qDebug()<<"hold remaing "<<nRemainBytes;
                    }
#else
                    QByteArray baPCMPktLen=qint32ToQByteArray(nPCMLen);
                    //qDebug("%d:%02x %02x %02x %02x\n",baOpusData.size(),(uchar)baOpusPktLen.at(0),(uchar)baOpusPktLen.at(1),(uchar)baOpusPktLen.at(2),(uchar)baOpusPktLen.at(3));
                    if(tcpSocket->write(baPCMPktLen)<0)
                    {
                        qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                        bSocketBreaking=true;
                    }
                    if(tcpSocket->write(pPCMBuffer,nPCMLen)<0)
                    {
                        qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                        bSocketBreaking=true;
                    }
                    if(!tcpSocket->waitForBytesWritten(1000))
                    {
                        qDebug()<<"<Error>:AudioTxThread,socket write error,break it.";
                        bSocketBreaking=true;
                    }
#endif

                    this->m_mutex->lock();
                    this->m_freeQueue->enqueue(pcmNs);
                    this->m_condQueueEmpty->wakeAll();
                    this->m_mutex->unlock();

                    if(bSocketBreaking)
                    {
                        break;
                    }
                    //                    this->usleep(AUDIO_THREAD_SCHEDULE_US);
                }
ExceptHandler:
                //设置连接标志，这样编码器线程就会停止工作.
                gGblPara.m_audio.m_bAudioTcpConnected=false;
                //qDebug()<<"audio disconnected.";
            }
        }
        delete tcpServer;
    }
    delete [] txBuffer;
    qDebug()<<"<MainLoop>:AudioTxThread ends.";
    //set global request to exit flag to help other thread to exit.
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}

