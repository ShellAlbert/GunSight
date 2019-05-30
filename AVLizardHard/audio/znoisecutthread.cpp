#include "znoisecutthread.h"
#include "../zgblpara.h"
#include <QDebug>
#include <QFile>
#include "audio/libns.h"
#define FRAME_SIZE_SHIFT 2
#define FRAME_SIZE (120<<FRAME_SIZE_SHIFT)

void f32_to_s16(int16_t *pOut,const float *pIn,size_t sampleCount)
{
    if(pOut==NULL || pIn==NULL)
    {
        return;
    }
    for(size_t i=0;i<sampleCount;i++)
    {
        *pOut++=(short)pIn[i];
    }
}
void s16_to_f32(float *pOut,const int16_t *pIn,size_t sampleCount)
{
    if(pOut==NULL || pIn==NULL)
    {
        return;
    }
    for(size_t i=0;i<sampleCount;i++)
    {
        *pOut++=pIn[i];
    }
}
ZNoiseCutThread::ZNoiseCutThread()
{
    this->m_bCleanup=true;
}
void ZNoiseCutThread::ZBindInFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueIn=freeQueue;
    this->m_usedQueueIn=usedQueue;
    this->m_mutexIn=mutex;
    this->m_condQueueEmptyIn=condQueueEmpty;
    this->m_condQueueFullIn=condQueueFull;
}
void ZNoiseCutThread::ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                    QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut1=freeQueue;
    this->m_usedQueueOut1=usedQueue;
    this->m_mutexOut1=mutex;
    this->m_condQueueEmptyOut1=condQueueEmpty;
    this->m_condQueueFullOut1=condQueueFull;
}
void ZNoiseCutThread::ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                    QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut2=freeQueue;
    this->m_usedQueueOut2=usedQueue;
    this->m_mutexOut2=mutex;
    this->m_condQueueEmptyOut2=condQueueEmpty;
    this->m_condQueueFullOut2=condQueueFull;
}

qint32 ZNoiseCutThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZNoiseCutThread::ZStopThread()
{
    return 0;
}
bool ZNoiseCutThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
void ZNoiseCutThread::run()
{
    //initial libns.
//    if(ns_init()<0)
//    {
//        qDebug()<<"<Error>:failed to init libns.";
//        //set global request to exit flag to cause other threads to exit.
//        gGblPara.m_bGblRst2Exit=true;
//        return;
//    }

    //LogMMSE.
    X_INT16 *pBuffer;
    X_FLOAT32 *OutBuf;
    NoiseSupStructX *NSX;
    this->m_logMMSE=(LOGMMSE_VAR *)malloc(sizeof(LOGMMSE_VAR));
    logMMSE_Init(this->m_logMMSE);
    this->m_SigIn=(X_INT16*)malloc(sizeof(X_INT16)*FRAME_LEN);
    this->m_SigOut=(X_INT16*)malloc(sizeof(X_INT16)*FRAME_LEN);
    this->m_OutBuf=(X_FLOAT32 *)malloc(sizeof(X_FLOAT32)*FRAME_SHIFT);

    //RNNoise.
//    this->m_st=rnnoise_create();
//    if(this->m_st==NULL)
//    {
//        qDebug()<<"<Error>:NoiseCut,error at rnnoise_create().";
//        //set global request to exit flag to cause other threads to exit.
//        gGblPara.m_bGblRst2Exit=true;
//        return;
//    }

    //WebRTC.
    //use this variable to control webRtc noiseSuppression grade.
    //valid range is 0,1,2.default is 0.
    qint32 nWebRtcNsPolicy=gGblPara.m_audio.m_nWebRtcNsPolicy;
    if(0!=WebRtcNs_Create(&this->m_pNS_inst))
    {
        qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_Create().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    //webRtc only supports 8KHz,16KHz,32KHz.
    //CAUTION HERE: it does not support 48khz!!!
    if(0!=WebRtcNs_Init(this->m_pNS_inst,/*32000*/16000))
    {
        qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_Init().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    if(0!=WebRtcNs_set_policy(this->m_pNS_inst,nWebRtcNsPolicy))
    {
        qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_set_policy().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    //auto gain control.
    WebRtcAgc_Create(&this->m_agcHandle);
    int minLevel = 0;
    int maxLevel = 255;
    //使用自动增益放大的音量比较有限，所以此处使用固定增益模式.
    int agcMode  = kAgcModeFixedDigital;
    WebRtcAgc_Init(this->m_agcHandle, minLevel, maxLevel, agcMode,8000);
    //1.compressionGaindB,在Fixed模式下，该值越大声音越大.
    //2.targetLevelDbfs,表示相对于Full Scale的下降值，0表示full scale,该值越小声音越大.
    WebRtcAgc_config_t agcConfig;
    //compressionGaindB.
    //Set the maximum gain the digital compression stage many apply in dB.
    //A higher number corresponds to greater compression.
    //while a value of 0 will leave the signal uncompressed.
    //value range: limited to [0,90].

    //targetLevelDbfs.
    //According the description of webrtc:
    //change of this parameter will set the target peak level of the AGC in dBFs.
    //the conversion is to use positive values.
    //For instance,passing in a value of 3 corresponds to -3 dBFs or a target level 3dB below full-scale.
    //value range: limited to [0,31].

    //limiterEnable.
    //When enabled,the compression stage will hard limit the signal to the target level.
    //otherwise,the signal will be compressed but not limited above the target level.

    agcConfig.compressionGaindB=gGblPara.m_audio.m_nGaindB;
    agcConfig.targetLevelDbfs=3;//dBFs表示相对于full scale的下降值,0表示full scale.3dB below full-scale.
    agcConfig.limiterEnable=1;
    WebRtcAgc_set_config(this->m_agcHandle,agcConfig);
    WebRtcSpl_ResetResample48khzTo16khz(&m_state4816);
    WebRtcSpl_ResetResample16khzTo48khz(&m_state1648);

    int frameSize=160;//80;
    int len=frameSize*sizeof(short);
    this->m_pDataIn=(short*)malloc(frameSize*sizeof(short));
    this->m_pDataOut=(short*)malloc(frameSize*sizeof(short));

    //这个变量用于控制通过json动态调整增益.
    qint32 nGaindBShadow=gGblPara.m_audio.m_nGaindB;

    //bevis.
    //    qint32 nBevisRemaingBytes=0;
    //    _WINDNSManager bevis;
    //    bevis.vp_init();
    //    //de-noise grade:0/1/2/3.
    //    bevis.vp_setwindnsmode(gGblPara.m_audio.m_nBevisGrade);

    //    char *pRNNoiseBuffer=new char[FRAME_SIZE];
    //    qint32 nRNNoiseBufLen=0;
    //    //脏音频数据（未经过降噪处理的原始采样数据）.
    //    QByteArray *baPCMData=new QByteArray(BLOCK_SIZE,0);
    //    qint32 nPCMDataLen=0;

    this->m_pcm16k=new char[PERIOD_SIZE];
    if(NULL==this->m_pcm16k)
    {
        qDebug()<<"<Error>:failed to allocate buffer form pcm16k!";
        return;
    }

    qDebug()<<"<MainLoop>:Audio NoiseSuppressThread starts.";
    this->m_bCleanup=false;
    //bind thread to cpu cores.
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(2,&cpuSet);
    if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
    {
        qDebug()<<"<Error>:failed to bind AudioNoiseCut thread to cpu core 2.";
    }else{
        qDebug()<<"<Info>:success to bind AudioNoiseCut thread to cpu core 2.";
    }

    //for libns.
    qint32 nLibNsModeShadow=gGblPara.m_audio.m_nRNNoiseView;
    ns_init(0);//0~5.
    while(!gGblPara.m_bGblRst2Exit)
    {
#if 1
        //WebRTC降噪级别被json修改了，这里重新初始化WebRTC.
        if(nWebRtcNsPolicy!=gGblPara.m_audio.m_nWebRtcNsPolicy)
        {
            WebRtcNs_Free(this->m_pNS_inst);
            WebRtcAgc_Free(this->m_agcHandle);
            this->m_pNS_inst=NULL;
            this->m_agcHandle=NULL;

            //update flag.
            nWebRtcNsPolicy=gGblPara.m_audio.m_nWebRtcNsPolicy;
            //re-init webRTC.
            if(0!=WebRtcNs_Create(&this->m_pNS_inst))
            {
                qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_Create().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            if(0!=WebRtcNs_Init(this->m_pNS_inst,/*32000*/16000))
            {
                qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_Init().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            if(0!=WebRtcNs_set_policy(this->m_pNS_inst,nWebRtcNsPolicy))
            {
                qDebug()<<"<Error>:NoiseCut,error at WebRtcNs_set_policy().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            //auto gain control.
            WebRtcAgc_Create(&this->m_agcHandle);
            int minLevel = 0;
            int maxLevel = 255;
            //使用自动增益放大的音量比较有限，所以此处使用固定增益模式.
            int agcMode  = kAgcModeFixedDigital;
            WebRtcAgc_Init(this->m_agcHandle, minLevel, maxLevel, agcMode,8000);
            //1.compressionGaindB,在Fixed模式下，该值越大声音越大.
            //2.targetLevelDbfs,表示相对于Full Scale的下降值，0表示full scale,该值越小声音越大.
            WebRtcAgc_config_t agcConfig;
            //compressionGaindB.
            //Set the maximum gain the digital compression stage many apply in dB.
            //A higher number corresponds to greater compression.
            //while a value of 0 will leave the signal uncompressed.
            //value range: limited to [0,90].

            //targetLevelDbfs.
            //According the description of webrtc:
            //change of this parameter will set the target peak level of the AGC in dBFs.
            //the conversion is to use positive values.
            //For instance,passing in a value of 3 corresponds to -3 dBFs or a target level 3dB below full-scale.
            //value range: limited to [0,31].

            //limiterEnable.
            //When enabled,the compression stage will hard limit the signal to the target level.
            //otherwise,the signal will be compressed but not limited above the target level.

            agcConfig.compressionGaindB=nGaindBShadow;
            agcConfig.targetLevelDbfs=3;//dBFs表示相对于full scale的下降值,0表示full scale.3dB below full-scale.
            agcConfig.limiterEnable=1;
            WebRtcAgc_set_config(this->m_agcHandle,agcConfig);

            WebRtcSpl_ResetResample48khzTo16khz(&m_state4816);
            WebRtcSpl_ResetResample16khzTo48khz(&m_state1648);
        }
#endif

#if 1
        //动态增益变量肯定被json协议修改了,我们重新调整动态增益的相关设置。
        if(nGaindBShadow!=gGblPara.m_audio.m_nGaindB)
        {
            //重新初始化.
            WebRtcAgc_Create(&this->m_agcHandle);
            WebRtcAgc_Init(this->m_agcHandle,0,255,kAgcModeFixedDigital,8000);
            WebRtcAgc_config_t agcConfig;
            agcConfig.compressionGaindB=gGblPara.m_audio.m_nGaindB;
            agcConfig.targetLevelDbfs=3;
            agcConfig.limiterEnable=1;
            WebRtcAgc_set_config(this->m_agcHandle,agcConfig);

            //update flag.
            nGaindBShadow=gGblPara.m_audio.m_nGaindB;
            qDebug()<<"noise cut update dgain to "<<nGaindBShadow;
        }

#endif

        //get a data buffer from fifo.
        this->m_mutexIn->lock();
        while(this->m_usedQueueIn->isEmpty())
        {//timeout 5s to check exit flag.
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
        QByteArray *pcmIn=this->m_usedQueueIn->dequeue();
        this->m_mutexIn->unlock();
        //qDebug()<<"noisecut,get one frame";

        //noise cut processing by different algorithm.
        //qDebug()<<"noisecut:size:"<<pcmIn->size();
        switch(gGblPara.m_audio.m_nDeNoiseMethod)
        {
        case 0:
            //qDebug()<<"DeNoise:disabled";
            break;
        case 1:
            //qDebug()<<"DeNoise:RNNoise Enabled";
            if(nLibNsModeShadow!=gGblPara.m_audio.m_nRNNoiseView)
            {
                ns_uninit();
                nLibNsModeShadow=gGblPara.m_audio.m_nRNNoiseView;
                ns_init(nLibNsModeShadow);
            }
            this->ZCutNoiseByRNNoise(pcmIn);
            break;
        case 2:
            //qDebug()<<"DeNoise:WebRTC Enabled";
            this->ZCutNoiseByWebRTC(pcmIn);
            break;
        case 3:
            //qDebug()<<"DeNoise:Bevis Enabled";
            this->ZCutNoiseByBevis(pcmIn);
            break;
        case 4:
            //logMMSE.
            //this->ZCutNoiseByLogMMSE(pcmIn);
            ns_processing(pcmIn->data(),pcmIn->size());
            break;
        default:
            break;
        }


        if(gGblPara.m_audio.m_nGaindB>0)
        {
            this->ZDGainByWebRTC(pcmIn);
        }

        //move data from IN fifo to OUT1 fifo.
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
        QByteArray *bufferOut1=this->m_freeQueueOut1->dequeue();
        this->m_mutexOut1->unlock();
        //move data from one fifo to another fifo.
        memcpy(bufferOut1->data(),pcmIn->data(),PERIOD_SIZE);

        this->m_mutexOut1->lock();
        this->m_usedQueueOut1->enqueue(bufferOut1);
        this->m_condQueueFullOut1->wakeAll();
        this->m_mutexOut1->unlock();

        //qDebug()<<"copy data from in fifo to out1 fifo";
        this->m_mutexIn->lock();
        this->m_freeQueueIn->enqueue(pcmIn);
        this->m_condQueueEmptyIn->wakeAll();
        this->m_mutexIn->unlock();

        //move data from IN fifo to OUT2 fifo.
        //当有客户端连接上时，才将采集到的数据放到发送队列.
        //如果TCP传输速度慢，则会导致发送队列满，所以这里使用try防止本地阻塞，可能造成音频丢帧现象。
        if(gGblPara.m_audio.m_bAudioTcpConnected)
        {
            if(this->m_mutexOut2->tryLock())
            {
                while(this->m_freeQueueOut2->isEmpty())
                {
                    if(!this->m_condQueueEmptyOut2->wait(this->m_mutexOut2,5000))
                    {
                        this->m_mutexOut2->unlock();
                        if(gGblPara.m_bGblRst2Exit)
                        {
                            return;
                        }
                    }
                }
                QByteArray *bufferOut2=this->m_freeQueueOut2->dequeue();
                //move data from one fifo to another fifo.
                memcpy(bufferOut2->data(),pcmIn->data(),PERIOD_SIZE);
                this->m_usedQueueOut2->enqueue(bufferOut2);
                this->m_condQueueFullOut2->wakeAll();
                this->m_mutexOut2->unlock();
            }
        }


    }

    //rnnoise_destroy(this->m_st);
    WebRtcNs_Free(this->m_pNS_inst);
    WebRtcAgc_Free(this->m_agcHandle);
    delete [] this->m_pcm16k;
    //    bevis.vp_uninit();
    //    delete [] pFilterBuffer;

    //uninit libns.
    ns_uninit();

    qDebug()<<"<MainLoop>:NoiseSuppressThread ends.";
    //set global request to exit flag to help other thread to exit.
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}

qint32 ZNoiseCutThread::ZCutNoiseByRNNoise(QByteArray *baPCM)
{
    //because original pcm data is 48000 bytes.
    //libns only process 960 bytes each time.
    //so 48000/960=50.
    qint32 nOffset=0;
    for(qint32 i=0;i<50;i++)
    {
        char *pPcmAudio=baPCM->data()+nOffset;
        ns_processing(pPcmAudio,960);
        nOffset+=960;
    }
    return 0;
}

//WebRTC.
qint32 ZNoiseCutThread::ZCutNoiseByWebRTC(QByteArray *baPCM)
{
#if 1
    //48khz downsample to 16khz,so data is decrease 3.
    //we dump data from baPCM to m_pcm16k.
    qint32 nOffset16k=0;
    //downsample 48khz to 16khz.48000/(480*2)=50.
    qint32 nLoopNum1=baPCM->size()/(480*sizeof(int16_t));
    for(qint32 i=0;i<nLoopNum1;i++)
    {

        int16_t tmpIn[480];//48khz,480=10ms.
        int16_t tmpOut[160];//16khz,160=10ms.
        int tmpMem[960];
        memcpy(tmpIn,baPCM->data()+i*sizeof(tmpIn),sizeof(tmpIn));
        WebRtcSpl_Resample48khzTo16khz(tmpIn,tmpOut,&m_state4816,tmpMem);
        //save data.
        memcpy(this->m_pcm16k+nOffset16k,tmpOut,sizeof(tmpOut));
        nOffset16k+=sizeof(tmpOut);
    }
    //qDebug()<<"48khz-16khz okay,bytes:"<<nOffset16k;
#endif
    ////////////////////////////////////////////////////////////
#if 1
    int i;
    //    char *pcmData=baPCM->data();
    //    qint32 nPCMDataLen=baPCM->size();
    char *pcmData=this->m_pcm16k;
    qint32 nPCMDataLen=nOffset16k;
    int  filter_state1[6],filter_state12[6];
    int  Synthesis_state1[6],Synthesis_state12[6];

    memset(filter_state1,0,sizeof(filter_state1));
    memset(filter_state12,0,sizeof(filter_state12));
    memset(Synthesis_state1,0,sizeof(Synthesis_state1));
    memset(Synthesis_state12,0,sizeof(Synthesis_state12));

    for(i=0;i<nPCMDataLen;i+=320)
    {
        if((nPCMDataLen-i)>=320)
        {
            short shInL[160]={0},shInH[160]={0};
            short shOutL[160]={0},shOutH[160]={0};
            memcpy(shInL,(char*)(pcmData+i),160*sizeof(short));
            if (0==WebRtcNs_Process(this->m_pNS_inst,shInL,shInH,shOutL,shOutH))
            {
                memcpy(pcmData+i,shOutL,160*sizeof(short));
            }
        }
    }
#endif

#if 1
    //processing finished,we dump data from m_pcm16k to baPCM.
    //16000/(160*2)=50.
    qint32 xTmpOffset=0;
    qint32 nLoopNum2=nOffset16k/(sizeof(int16_t)*160);
    for(qint32 i=0;i<nLoopNum2;i++)
    {

        int16_t xTmpIn[160];
        int16_t xTmpOut[480];
        int xTmpMem[960];
        memcpy(xTmpIn,this->m_pcm16k+i*sizeof(xTmpIn),sizeof(xTmpIn));
        WebRtcSpl_Resample16khzTo48khz(xTmpIn,xTmpOut,&m_state1648,xTmpMem);
        //save data.
        memcpy(baPCM->data()+xTmpOffset,xTmpOut,sizeof(xTmpOut));
        xTmpOffset+=sizeof(xTmpOut);
    }
    //qDebug()<<"16khz-48khz okay,bytes:"<<xTmpOffset;
#endif

    return 0;
}
qint32 ZNoiseCutThread::ZCutNoiseByBevis(QByteArray *baPCM)
{
#if 0
    //新的数据帧的地址及大小.
    //char *pPcmData=baPCM.data();
    //qint32 nPcmBytes=baPCM.size();

    //如果上一帧有遗留的尾巴数据，则从新帧中复制出部分数据与之前遗留数据拼帧处理.
    if(nBevisRemaingBytes>0 && nBevisRemaingBytes<FRAME_LEN)
    {
        qint32 nNeedBytes=FRAME_LEN-nBevisRemaingBytes;
        memcpy(pFilterBuffer+nBevisRemaingBytes,pPcmData,nNeedBytes);
        nBevisRemaingBytes+=nNeedBytes;

        //update.
        pPcmData+=nNeedBytes;
        nPcmBytes-=nNeedBytes;
    }
    if(nBevisRemaingBytes>=FRAME_LEN)
    {
        int16_t *pSrc=(int16_t*)(pFilterBuffer);
        char pDst[FRAME_LEN];
        bevis.vp_process(pSrc,(int16_t*)pDst,2,FRAME_LEN);

        //                QByteArray baPCM(pDst,FRAME_LEN);
        //                this->m_semaFreeClear->acquire();//空闲信号量减1.
        //                this->m_queueClear->enqueue(baPCM);
        //                this->m_semaUsedClear->release();//已用信号量加1.

        //reset.
        nBevisRemaingBytes=0;
    }
    //the Bevis algorithm can only process 512 bytes once.
    //so here we split PCM data to multiple blocks.
    //and process each block then combine the result.
    qint32 nBlocks=nPcmBytes/FRAME_LEN;
    qint32 nRemaingBytes=nPcmBytes%FRAME_LEN;
    //qint32 nPadding=FRAME_LEN-nRemaingBytes;
    //qDebug()<<"total:"<<baPCMData.size()<<",blocks:"<<nBlocks<<",remain bytes:"<<nRemaingBytes<<",padding="<<nPadding;
    for(qint32 i=0;i<nBlocks;i++)
    {
        char *pSrc=pPcmData+FRAME_LEN*i;
        char pDst[FRAME_LEN];
        bevis.vp_process((int16_t*)pSrc,(int16_t*)pDst,2,FRAME_LEN);

        //                QByteArray baPCM(pDst,FRAME_LEN);
        //                this->m_semaFreeClear->acquire();//空闲信号量减1.
        //                this->m_queueClear->enqueue(baPCM);
        //                this->m_semaUsedClear->release();//已用信号量加1.
    }
    if(nRemaingBytes>0)
    {
        char *pSrc=pPcmData+FRAME_LEN*nBlocks;
        memcpy(pFilterBuffer+nBevisRemaingBytes,pSrc,nRemaingBytes);
        nBevisRemaingBytes+=nRemaingBytes;
    }
#endif
    return 0;
}
qint32 ZNoiseCutThread::ZCutNoiseByLogMMSE(QByteArray *baPCM)
{
    //48khz downsample to 16khz,so data is decrease 3.
    QByteArray baPCM16k;
    baPCM16k.resize(baPCM->size()/3);
    qint32 nOffset16k=0;
    //downsample 48khz to 16khz.
    qint32 nLoopNum1=baPCM->size()/(480*sizeof(int16_t));
    for(qint32 i=0;i<nLoopNum1;i++)
    {

        int16_t tmpIn[480];//48khz,480=10ms.
        int16_t tmpOut[160];//16khz,160=10ms.
        int tmpMem[960];
        memcpy(tmpIn,baPCM->data()+i*sizeof(tmpIn),sizeof(tmpIn));
        WebRtcSpl_Resample48khzTo16khz(tmpIn,tmpOut,&m_state4816,tmpMem);
        //save data.
        memcpy(baPCM16k.data()+nOffset16k,tmpOut,sizeof(tmpOut));
        nOffset16k+=sizeof(tmpOut);
    }

    //now PCM block size is 48000/3=16000.
    static int preProcessNum=7;
    qint32 nLogMMSEBlkSize=sizeof(X_INT16)*FRAME_LEN;
    //16000/(2*160)=50.
    qint32 nLoopNum=baPCM16k.size()/nLogMMSEBlkSize;
    for(qint32 i=0;i<nLoopNum;i++)
    {
        if(preProcessNum>0)
        { //we use the first 7 blocks to estimate noise.
            preProcessNum--;
            memcpy(this->m_SigIn,baPCM16k.data()+i*nLogMMSEBlkSize,nLogMMSEBlkSize);
            noise_estimate(this->m_SigIn,this->m_logMMSE);
        }else{
            //the others we do denoise.
            memcpy(this->m_SigIn,baPCM16k.data()+i*nLogMMSEBlkSize,nLogMMSEBlkSize);
            logMMSE_denosie(this->m_SigIn,this->m_OutBuf,this->m_logMMSE);

            X_INT16 *pBuffer=this->m_SigOut;
            for(qint32 j=0;j<FRAME_SHIFT;j++)
                pBuffer[j]=(X_INT16)this->m_OutBuf[j];

            logMMSE_denosie(this->m_SigIn+FRAME_SHIFT,this->m_OutBuf,this->m_logMMSE);

            pBuffer=this->m_SigOut+FRAME_SHIFT;
            for(qint32 k=0;k<FRAME_SHIFT;k++)
                pBuffer[k]=(X_INT16)this->m_OutBuf[k];

            //fwrite(SigOut, sizeof(X_INT16), FRAME_LEN, fp_out);
            memcpy(baPCM16k.data()+i*nLogMMSEBlkSize,this->m_SigOut,nLogMMSEBlkSize);
        }
    }

    //16000/(160*2)=50.
    qint32 xTmpOffset=0;
    qint32 nLoopNum2=baPCM16k.size()/(sizeof(int16_t)*160);
    for(qint32 i=0;i<nLoopNum2;i++)
    {

        int16_t xTmpIn[160];
        int16_t xTmpOut[480];
        int xTmpMem[960];
        memcpy(xTmpIn,baPCM16k.data()+i*sizeof(xTmpIn),sizeof(xTmpIn));
        WebRtcSpl_Resample16khzTo48khz(xTmpIn,xTmpOut,&m_state1648,xTmpMem);
        //save data.
        memcpy(baPCM->data()+xTmpOffset,xTmpOut,sizeof(xTmpOut));
        xTmpOffset+=sizeof(xTmpOut);
    }
    return 0;
}
//auto gain control.
qint32 ZNoiseCutThread::ZDGainByWebRTC(QByteArray *baPCM)
{
    int frameSize=160;//80;
    int len=frameSize*sizeof(short);

    int micLevelIn=0;
    int micLevelOut=0;

    char *pcmData=(char*)baPCM->data();
    int nRemaingBytes=baPCM->size();
    int nProcessedBytes=0;

    while(nRemaingBytes>=len)
    {
        //prepare data.
        memcpy(this->m_pDataIn,pcmData+nProcessedBytes,len);

        int inMicLevel=micLevelOut;
        int outMicLevel=0;
        uint8_t saturationWarning;
        int nAgcRet = WebRtcAgc_Process(this->m_agcHandle,this->m_pDataIn,NULL,frameSize,this->m_pDataOut,NULL,inMicLevel,&outMicLevel,0,&saturationWarning);
        if(nAgcRet!=0)
        {
            qDebug()<<"<Error>:error at WebRtcAgc_Process().";
            return -1;
        }
        micLevelIn=outMicLevel;

        //copy data out.
        memcpy(pcmData+nProcessedBytes,this->m_pDataOut,len);

        //update the processed and remaing bytes.
        nProcessedBytes+=len;
        nRemaingBytes-=len;
    }
    return 0;
}
