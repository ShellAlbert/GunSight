#include "zaudioplaythread.h"
#include "../zgblpara.h"
#include <sys/time.h>
#include <QDebug>
#include <QDateTime>
ZAudioPlayThread::ZAudioPlayThread(QString playCardName)
{
    this->m_playCardName=playCardName;
    this->m_bCleanup=true;
}
ZAudioPlayThread::~ZAudioPlayThread()
{

}
void ZAudioPlayThread::ZBindFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
               QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueue=freeQueue;
    this->m_usedQueue=usedQueue;
    this->m_mutex=mutex;
    this->m_condQueueEmpty=condQueueEmpty;
    this->m_condQueueFull=condQueueFull;
}
qint32 ZAudioPlayThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZAudioPlayThread::ZStopThread()
{
    return 0;
}
bool ZAudioPlayThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}

void ZAudioPlayThread::run()
{
    // Input and output driver variables
    snd_pcm_hw_params_t *hwparams;
    int periods=PERIODS;
    snd_pcm_uframes_t periodSize=PERIOD_SIZE;

    // Now we can open the PCM device:
    /* Open PCM. The last parameter of this function is the mode. */
    /* If this is set to 0, the standard mode is used. Possible   */
    /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */
    /* If SND_PCM_NONBLOCK is used, read / write access to the    */
    /* PCM device will return immediately. If SND_PCM_ASYNC is    */
    /* specified, SIGIO will be emitted whenever a period has     */
    /* been completely processed by the soundcard.                */
    if(snd_pcm_open(&this->m_pcmHandle,(char*)gGblPara.m_audio.m_playCardName.toStdString().c_str(),SND_PCM_STREAM_PLAYBACK,0)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_open():"<<gGblPara.m_audio.m_playCardName;
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    snd_pcm_hw_params_alloca(&hwparams);

    /*
        Before we can write PCM data to the soundcard,
        we have to specify access type, sample format, sample rate, number of channels,
        number of periods and period size.
        First, we initialize the hwparams structure with the full configuration space of the soundcard.
    */
    /* Init hwparams with full configuration space */
    if(snd_pcm_hw_params_any(this->m_pcmHandle,hwparams)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_any().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    /*
    A frame is equivalent of one sample being played,
    irrespective of the number of channels or the number of bits. e.g.
    1 frame of a Stereo 48khz 16bit PCM stream is 4 bytes.
    1 frame of a 5.1 48khz 16bit PCM stream is 12 bytes.
    A period is the number of frames in between each hardware interrupt. The poll() will return once a period.
    The buffer is a ring buffer. The buffer size always has to be greater than one period size.
    Commonly this is 2*period size, but some hardware can do 8 periods per buffer.
    It is also possible for the buffer size to not be an integer multiple of the period size.
    Now, if the hardware has been set to 48000Hz , 2 periods, of 1024 frames each,
    making a buffer size of 2048 frames. The hardware will interrupt 2 times per buffer.
    ALSA will endeavor to keep the buffer as full as possible.
    Once the first period of samples has been played,
    the third period of samples is transfered into the space the first one occupied
    while the second period of samples is being played. (normal ring buffer behaviour).
*/

    /*
    The access type specifies the way in which multichannel data is stored in the buffer.
    For INTERLEAVED access, each frame in the buffer contains the consecutive sample data for the channels.
    For 16 Bit stereo data,
    this means that the buffer contains alternating words of sample data for the left and right channel.
    For NONINTERLEAVED access,
    each period contains first all sample data for the first channel followed by the sample data
    for the second channel and so on.
*/

    /* Set access type. This can be either    */
    /* SND_PCM_ACCESS_RW_INTERLEAVED or       */
    /* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
    /* There are also access types for MMAPed */
    /* access, but this is beyond the scope   */
    /* of this introduction.                  */
    if(snd_pcm_hw_params_set_access(this->m_pcmHandle,hwparams,SND_PCM_ACCESS_RW_INTERLEAVED)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_access().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    /* Set sample format */
    if(snd_pcm_hw_params_set_format(this->m_pcmHandle,hwparams,SND_PCM_FORMAT_S16_LE)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_format().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */
    unsigned int nRealSampleRate=SAMPLE_RATE;
    if(snd_pcm_hw_params_set_rate_near(this->m_pcmHandle,hwparams,&nRealSampleRate,0u)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_rate_near().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    if(nRealSampleRate!=SAMPLE_RATE)
    {
        qDebug()<<"<Warning>:Audio PlayThread,the rate "<<SAMPLE_RATE<<" Hz is not supported by hardware.";
        qDebug()<<"<Warning>:Using "<<nRealSampleRate<<" instead.";
    }

    /* Set number of channels */
    if(snd_pcm_hw_params_set_channels(this->m_pcmHandle,hwparams,CHANNELS_NUM)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_channels().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    /* Set number of periods. Periods used to be called fragments. */
    /* Number of periods, See http://www.alsa-project.org/main/index.php/FramesPeriods */
    unsigned int request_periods=periods;
    int dir=0;
    //	Restrict a configuration space to contain only one periods count
    if(snd_pcm_hw_params_set_periods_near(this->m_pcmHandle,hwparams,&request_periods,&dir)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_periods_near().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    if(request_periods!=periods)
    {
        qDebug("<Warning>:Audio PlayThread,requested %d periods,but recieved %d.\n", request_periods, periods);
    }

    /*
        The unit of the buffersize depends on the function.
        Sometimes it is given in bytes, sometimes the number of frames has to be specified.
        One frame is the sample data vector for all channels.
        For 16 Bit stereo data, one frame has a length of four bytes.
    */
    snd_pcm_uframes_t bufferSizeInFrames=(periods*periodSize)>>2;
    snd_pcm_uframes_t bufferSizeInFrames2=bufferSizeInFrames;
    if(snd_pcm_hw_params_set_buffer_size_near(this->m_pcmHandle,hwparams,&bufferSizeInFrames)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params_set_buffer_size_near().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    if(bufferSizeInFrames!=bufferSizeInFrames2)
    {
        qDebug()<<"request buffer size:"<<bufferSizeInFrames2<<",really:"<<bufferSizeInFrames;
    }
    /*
        If your hardware does not support a buffersize of 2^n,
        you can use the function snd_pcm_hw_params_set_buffer_size_near.
        This works similar to snd_pcm_hw_params_set_rate_near.
        Now we apply the configuration to the PCM device pointed to by pcm_handle.
        This will also prepare the PCM device.
    */
    /* Apply HW parameter settings to PCM device and prepare device*/
    if(snd_pcm_hw_params(this->m_pcmHandle,hwparams)<0)
    {
        qDebug()<<"<Error>:Audio PlayThread,error at snd_pcm_hw_params().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }

    //the main loop.
    qDebug()<<"<MainLoop>:PlaybackThread starts.";
    this->m_bCleanup=false;
    //bind thread to cpu core.
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3,&cpuSet);
    if(0!=pthread_setaffinity_np((int)this->currentThreadId(),sizeof(cpu_set_t),&cpuSet))
    {
        qDebug()<<"<Error>:failed to bind AudioPlay thread to cpu core 3.";
    }else{
        qDebug()<<"<Info>:success to bind AudioPlay thread to cpu core 3.";
    }
    //这里不能使用事件循环，还是需要用队列的方式
    //因为声卡播放需要时间，如果使用signal/slot会导致采样数据太快，
    //而声卡缓冲区内的数据未消耗干净，所以填充数据失败，导致丢帧现象.
    //enter event-loop until exit() is called.
    //this->exec();

    while(!gGblPara.m_bGblRst2Exit)
    {
        //get a data buffer from fifo.
        this->m_mutex->lock();
        while(this->m_usedQueue->isEmpty())
        {//timeout 5s to check exit flag.
            if(!this->m_condQueueFull->wait(this->m_mutex,5000))
            {
                this->m_mutex->unlock();
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
        QByteArray *bufferIn=this->m_usedQueue->dequeue();
        this->m_mutex->unlock();
        //qDebug()<<"play one frame";
        while(1)
        {
            qint32 ret=snd_pcm_writei(this->m_pcmHandle,bufferIn->data(),PERIOD_SIZE>>2);
            if(ret==-EPIPE)
            {
                snd_pcm_prepare(this->m_pcmHandle);
                qDebug()<<QDateTime::currentDateTime()<<",<Playback>:Buffer Underrun\n";
                continue;
            }else{
                break;
            }
        }
        this->m_mutex->lock();
        this->m_freeQueue->enqueue(bufferIn);
        this->m_condQueueEmpty->wakeAll();
        this->m_mutex->unlock();
    }


    //do some clean here.
    /*
        If we want to stop playback, we can either use snd_pcm_drop or snd_pcm_drain.
        The first function will immediately stop the playback and drop pending frames.
        The latter function will stop after pending frames have been played.
    */
    /* Stop PCM device and drop pending frames */
    snd_pcm_drop(this->m_pcmHandle);

    /* Stop PCM device after pending frames have been played */
    //snd_pcm_drain(pcmHandle);

    qDebug()<<"<MainLoop>:PlaybackThread ends.";
    //set global request to exit flag to help other thread to exit.
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}
quint64 ZAudioPlayThread::ZGetTimestamp()
{
    struct timeval now;
    gettimeofday(&now,NULL);
    return now.tv_usec+(quint64)now.tv_sec*1000000;
}
