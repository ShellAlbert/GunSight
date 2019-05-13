#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <alsa/asoundlib.h>

//这里我们使用32KHz的采样率，双声道，采样位数24bit
//则有bps=2*24bit*32000=1536000bits/s=192000Bytes
//这里我们设置period为4，即1秒发生4次中断，则中断间隔为1s/4=250ms.
//则每次中断发生时，我们至少需要填充192000Bytes/(1s/250ms)=48000Bytes
//#define     BLOCK_SIZE        48000	// Number of bytes

//这里我们设置period为10，即1秒发生10次中断，则中断间隔为1s/10=100ms.
//则每次中断发生时，我们至少需要填充192000Bytes/(1s/100ms)=19200Bytes
#define     BLOCK_SIZE      48000
#define     ALSA_PERIOD     20

//* The sample rate of the audio codec **
#define     SAMPLE_RATE      32000

//frame帧是播放样本的一个计量单位，由通道数和比特数决定。
//立体声32KHz 24-bit的PCM，那么一帧的大小就是6字节(2 Channels*24-bit=48bit/8bit=6 bytes)
#define CHANNELS_NUM    2
#define BYTES_PER_FRAME 6

//how to play in rk3399-linux board.
//aplay -D plughw:CARD=realtekrt5651co,DEV=0 -c 2 -r 32000 -f S24_3LE -t raw cap.pcm
static int g_ExitFlag=0;
void gSigInt(int signo)
{
    g_ExitFlag=1;
}
int main(void)
{
    int ret,i;
    int nPcmLen=0;
    char pcmBuffer[BLOCK_SIZE+1024];
    int fd=open("/dev/fpga-i2s",O_RDONLY);
    snd_pcm_t *pcmHandle;
    if(fd<0)
    {
        printf("failed to open fpga-i2s.\n");
        return -1;
    }
    unsigned int nSampleRate=SAMPLE_RATE;
    // Input and output driver variables
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_uframes_t pcmFrames;

    // Now we can open the PCM device:
    /* Open PCM. The last parameter of this function is the mode. */
    /* If this is set to 0, the standard mode is used. Possible   */
    /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */
    /* If SND_PCM_NONBLOCK is used, read / write access to the    */
    /* PCM device will return immediately. If SND_PCM_ASYNC is    */
    /* specified, SIGIO will be emitted whenever a period has     */
    /* been completely processed by the soundcard.                */
    if(snd_pcm_open(&pcmHandle,"plughw:CARD=rockchiprt5640c,DEV=0",SND_PCM_STREAM_PLAYBACK,0)<0)
    {
        printf("error:failed to open playback device.\n");
        return -1;
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
    if(snd_pcm_hw_params_any(pcmHandle,hwparams)<0)
    {
        printf("error:failed to init hwparams.\n");
        return -1;
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
    if(snd_pcm_hw_params_set_access(pcmHandle,hwparams,SND_PCM_ACCESS_RW_INTERLEAVED)<0)
    {
        printf("error:failed to set hwparams.\n");
        return -1;
    }

    /* Set sample format */
    //SND_PCM_FORMAT_S24_LE:指的是4字节数据，但只有3字节有效，最高位全为无效的0
    //SND_PCM_FORMAT_S24_3LE:指的是3字节数据，都有效
    if(snd_pcm_hw_params_set_format(pcmHandle,hwparams,SND_PCM_FORMAT_S24_3LE)<0)
    {
        printf("error:failed to set format.\n");
        return -1;
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */
    unsigned int nRealSampleRate=nSampleRate;
    if(snd_pcm_hw_params_set_rate_near(pcmHandle,hwparams,&nRealSampleRate,0u)<0)
    {
        printf("error:failed to set sample Rate.\n");
        return -1;
    }
    if(nRealSampleRate!=nSampleRate)
    {
        printf("<Warning>:using %d to replace %d.\n",nRealSampleRate,nSampleRate);
    }

    /* Set number of channels */
    if(snd_pcm_hw_params_set_channels(pcmHandle,hwparams,CHANNELS_NUM)<0)
    {
        printf("error:failed to set channel number.\n");
        return -1;
    }

    /* Set number of periods. Periods used to be called fragments. */
    /* Number of periods, See http://www.alsa-project.org/main/index.php/FramesPeriods */
    unsigned int periods=ALSA_PERIOD;
    unsigned int request_periods;
    int dir=0;
    request_periods=periods;
    //	Restrict a configuration space to contain only one periods count
    if(snd_pcm_hw_params_set_periods_near(pcmHandle,hwparams,&periods,&dir)<0)
    {
        printf("error:failed to set periods.\n");
        return -1;
    }
    if(request_periods!=periods)
    {
        printf("<Warning>:using %d to replace %d periods.\n",request_periods,periods);
    }

    /*
            The unit of the buffersize depends on the function.
            Sometimes it is given in bytes, sometimes the number of frames has to be specified.
            One frame is the sample data vector for all channels.
            For 16 Bit stereo data, one frame has a length of four bytes.
        */
    if(snd_pcm_hw_params_set_buffer_size_near(pcmHandle,hwparams,&pcmFrames)<0)
    {
        printf("error:failed to set buffer size.\n");
        return -1;
    }

    /*
            If your hardware does not support a buffersize of 2^n,
            you can use the function snd_pcm_hw_params_set_buffer_size_near.
            This works similar to snd_pcm_hw_params_set_rate_near.
            Now we apply the configuration to the PCM device pointed to by pcm_handle.
            This will also prepare the PCM device.
        */
    /* Apply HW parameter settings to PCM device and prepare device*/
    if(snd_pcm_hw_params(pcmHandle,hwparams)<0)
    {
        printf("error:failed to set hwparams.\n");
        return -1;
    }

    signal(SIGINT,gSigInt);
    signal(SIGABRT,gSigInt);
    printf("capturing & playing....\n");
#if 0
    nPcmLen=0;
    while(!g_ExitFlag)
    {
        //capture.
        ret=read(fd,pcmBuffer+nPcmLen,sizeof(pcmBuffer)-nPcmLen);
        //printf("read %d bytes:\n",ret);
        if(ret>0)
        {
            //add new data to pcm buffer.
            nPcmLen+=ret;
            if(nPcmLen>=BLOCK_SIZE)
            {
                int nRemainingBytes=nPcmLen-BLOCK_SIZE;
                snd_pcm_uframes_t pcmFrames=BLOCK_SIZE/BYTES_PER_FRAME;
                while(snd_pcm_writei(pcmHandle,pcmBuffer,pcmFrames)<0)
                {
                    snd_pcm_prepare(pcmHandle);
                    //qDebug("<playback>:Buffer Underrun");
                }
                //reset length.
                nPcmLen=nRemainingBytes;
                if(nRemainingBytes>0)
                {
                    memmove(pcmBuffer,pcmBuffer+BLOCK_SIZE,nRemainingBytes);
                }
            }
        }else if(0==ret)
        {
            printf("<warning>:timeout to read pcm data!\n");
            break;
        }else if(ret<0)
        {
            printf("<error>:failed to read pcm data!\n");
            break;
        }
    }
#endif
    while(!g_ExitFlag)
    {
        //capture.
        ret=read(fd,pcmBuffer,sizeof(pcmBuffer));
        if(ret>0)
        {
            snd_pcm_uframes_t pcmFrames=ret/BYTES_PER_FRAME;
            while(snd_pcm_writei(pcmHandle,pcmBuffer,pcmFrames)<0)
            {
                snd_pcm_prepare(pcmHandle);
                //qDebug("<playback>:Buffer Underrun");
            }
        }else if(0==ret)
        {
             printf("<warning>:timeout to read pcm data!\n");
        }else if(ret<0){
            printf("<error>:failed to read pcm data!\n");
            break;
        }
    }
    snd_pcm_drop(pcmHandle);
    close(fd);
    printf("done.\n");
    return 0;
}
