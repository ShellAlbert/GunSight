#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define I2C_ADDR_JY901B 0x50
#define I2C_REG_ADDR_BAUD           0x04
//Ax,Ay,Az zero offset.
#define I2C_REG_ADDR_AXOFFSET       0x05
#define I2C_REG_ADDR_AYOFFSET       0x06
#define I2C_REG_ADDR_AZOFFSET       0x07
//Gx,Gy,Gz zero offset.
#define I2C_REG_ADDR_GXOFFSET       0x08
#define I2C_REG_ADDR_GYOFFSET       0x09
#define I2C_REG_ADDR_GZOFFSET       0x0a
//Hx,Hy,Hz zero offset.
#define I2C_REG_ADDR_HXOFFSET       0x0b
#define I2C_REG_ADDR_HYOFFSET       0x0c
#define I2C_REG_ADDR_HZOFFSET       0x0d
//Ax,Ay,Ax.
#define I2C_REG_ADDR_AX             0x34
#define I2C_REG_ADDR_AY             0x35
#define I2C_REG_ADDR_AZ             0x36
//Gx,Gy,Gz.
#define I2C_REG_ADDR_GX             0x37
#define I2C_REG_ADDR_GY             0x38
#define I2C_REG_ADDR_GZ             0x39
//Hx,Hy,Hz.
#define I2C_REG_ADDR_HX             0x3a
#define I2C_REG_ADDR_HY             0x3b
#define I2C_REG_ADDR_HZ             0x3c
//Roll,Pitch,Yaw.
#define I2C_REG_ADDR_POLL           0x3d
#define I2C_REG_ADDR_PITCH          0x3e
#define I2C_REG_ADDR_YAW            0x3f
//pressure.
#define I2C_REG_ADDR_PRESSURE_L     0x45
#define I2C_REG_ADDR_PRESSURE_H     0x46

#define I2C_REG_ADDR_IICADDR        0x1a

//port mode.
typedef enum
{
    Analog_In=0x00,
    Digital_In=0x01,
    Digital_Out_High=0x02,
    Digital_Out_Low=0x03,
    Digital_Out_Pwm=0x04,
}PortMode;
class jy901b
{
public:
    jy901b(const char *i2c_dev);
    ~jy901b();

    int doInit();
    int doClean();
public:
    //get Ax,Ay,Az Offset.
    int getAxAyAzOffset(int *axOffset,int *ayOffset,int *azOffset);
    //get Ax,Ay,Az.
    int getAxAyAz(float *ax,float *ay,float *az);

    //get Gx,Gy,Gz Offset.
    int getGxGyGzOffset(int *gxOffset,int *gyOffset,int *gzOffset);
    //get Gx,Gy,Gz.
    int getGxGyGz(float *gx,float *gy,float *gz);

    //get Hx,Hy,Hz Offset.
    int getHxHyHzOffset(int *hxOffset,int *hyOffset,int *hzOffset);
    //get Hx,Hy,Hz.
    int getHxHyHz(float *hx,float *hy,float *hz);

    //get Roll,Pitch,Yaw.
    int getRollPitchYaw(float *roll,float *pitch,float *yaw);

    //get Pressure.
    int getPressure(int *pressure);
    //get Longtude  High and Low.
    int getLongitudeHL(int *high,int *low);
    //get Latitude High and Low.
    int getLatitude(int *high,int *low);

    //set D0~D3 port mode.
    int setD0123Mode(int port,PortMode mode);
private:
    //byte interface.
    int i2c_smbus_read_byte_data(char reg_addr,char *reg_value);
    int i2c_smbus_write_byte_data(char reg_addr,char reg_value);
    //word interface.
    int i2c_smbus_read_word_data(char reg_addr,int *reg_value);
    int i2c_smbus_write_word_data(char reg_addr,int reg_value);
    //block interface.
    int i2c_smbus_read_block_data(char reg_addr,char *reg_value);
    int i2c_smbus_write_block_data(char reg_addr,char *reg_value,int len);
private:
    char *m_i2cdev;
    int m_fd;
};
jy901b::jy901b(const char *i2c_dev)
{
    this->m_i2cdev=(char*)i2c_dev;
}
jy901b::~jy901b()
{
}
int jy901b::doInit()
{
    this->m_fd=open(this->m_i2cdev,O_RDWR);
    if(this->m_fd<0)
    {
        printf("<jy901b>:failed to open i2c dev:%s!\n",this->m_i2cdev);
        return -1;
    }
    if(ioctl(this->m_fd,I2C_SLAVE,I2C_ADDR_JY901B)<0)
    {
        printf("<jy901b>:failed to set i2c slave address!\n");
        return -1;
    }
    return 0;
}
int jy901b::doClean()
{
    if(this->m_fd>0)
    {
        close(this->m_fd);
    }
    return 0;
}
int jy901b::i2c_smbus_read_byte_data(char reg_addr,char *reg_value)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    args.read_write=I2C_SMBUS_READ;
    args.command=reg_addr;
    args.size=I2C_SMBUS_BYTE_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_read_byte_data!\n");
        return -1;
    }
    *reg_value=0xFF&data.byte;
    return 0;
}
int jy901b::i2c_smbus_write_byte_data(char reg_addr,char reg_value)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    data.byte=reg_value;

    args.read_write=I2C_SMBUS_WRITE;
    args.command=reg_addr;
    args.size=I2C_SMBUS_BYTE_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_write_byte_data!\n");
        return -1;
    }
    return 0;
}
int jy901b::i2c_smbus_read_word_data(char reg_addr,int *reg_value)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    args.read_write=I2C_SMBUS_READ;
    args.command=reg_addr;
    args.size=I2C_SMBUS_WORD_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_read_word_data!\n");
        return -1;
    }
    *reg_value=0xFFFF&data.word;
    return 0;
}
int jy901b::i2c_smbus_write_word_data(char reg_addr,int reg_value)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    data.word=reg_value&0xFFFF;

    args.read_write=I2C_SMBUS_WRITE;
    args.command=reg_addr;
    args.size=I2C_SMBUS_WORD_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_write_word_data!\n");
        return -1;
    }
    return 0;
}
int jy901b::i2c_smbus_read_block_data(char reg_addr,char *reg_value)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    args.read_write=I2C_SMBUS_READ;
    args.command=reg_addr;
    args.size=I2C_SMBUS_BLOCK_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_read_block_data!\n");
        return -1;
    }
    for(int i=1;i<=data.block[0];i++)
    {
        reg_value[i-1]=data.block[i];
    }
    return data.block[0];
}
int jy901b::i2c_smbus_write_block_data(char reg_addr,char *reg_value,int len)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    if(len>32)
    {
        len=32;
    }
    for(int i=1;i<=len;i++)
    {
        data.block[i]=reg_value[i-1];
    }
    data.block[0]=len;

    args.read_write=I2C_SMBUS_WRITE;
    args.command=reg_addr;
    args.size=I2C_SMBUS_BLOCK_DATA;
    args.data=&data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<jy901b>:failed to ioctl i2c_smbus_write_block_data!\n");
        return -1;
    }
    return 0;
}
//get Ax,Ay,Az Offset.
int jy901b::getAxAyAzOffset(int *axOffset,int *ayOffset,int *azOffset)
{

}
//get Ax,Ay,Az.
int jy901b::getAxAyAz(float *ax,float *ay,float *az)
{
    int nAx,nAy,nAz;
    int nAxSwp=0,nAySwp=0,nAzSwp=0;
    if(NULL==ax || NULL==ay || NULL==az)
    {
        printf("<jy901b>:null parameter.");
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_AX,&nAx)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_AY,&nAy)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_AZ,&nAz)<0)
    {
        return -1;
    }
    nAxSwp=((nAx&0xFF)<<8)|((nAx&0xFF00)>>8);
    nAySwp=((nAy&0xFF)<<8)|((nAy&0xFF00)>>8);
    nAzSwp=((nAz&0xFF)<<8)|((nAz&0xFF00)>>8);
    printf("......Ax:%x(%x),Ay:%x(%x),Az:%x(%x)\n",nAx,nAxSwp,nAy,nAySwp,nAz,nAzSwp);

    *ax=nAxSwp/32768.0*16.0*9.8;
    *ay=nAySwp/32768.0*16.0*9.8;
    *az=nAzSwp/32768.0*16.0*9.8;
    //printf("AxAyAz:%.2f,%.2f,%.2f\n",*ax,*ay,*az);
    return 0;
}

//get Gx,Gy,Gz Offset.
int jy901b::getGxGyGzOffset(int *gxOffset,int *gyOffset,int *gzOffset)
{

}
//get Gx,Gy,Gz.
int jy901b::getGxGyGz(float *gx,float *gy,float *gz)
{
    int nGx,nGy,nGz;
    int nGxSwp=0,nGySwp=0,nGzSwp=0;
    if(NULL==gx || NULL==gy || NULL==gz)
    {
        printf("<jy901b>:null parameter.");
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_GX,&nGx)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_GY,&nGy)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_GZ,&nGz)<0)
    {
        return -1;
    }
    nGxSwp=((nGx&0xFF)<<8)|((nGx&0xFF00)>>8);
    nGySwp=((nGy&0xFF)<<8)|((nGy&0xFF00)>>8);
    nGzSwp=((nGz&0xFF)<<8)|((nGz&0xFF00)>>8);
    printf("......Gx:%x(%x),Gy:%x(%x),Gz:%x(%x)\n",nGx,nGxSwp,nGy,nGySwp,nGz,nGzSwp);

    *gx=nGxSwp/32768.0*2000.0;
    *gy=nGySwp/32768.0*2000.0;
    *gz=nGzSwp/32768.0*2000.0;
    //printf("GxGyGz:%.2f,%.2f,%.2f\n",*gx,*gy,*gz);
    return 0;
}

//get Hx,Hy,Hz Offset.
int jy901b::getHxHyHzOffset(int *hxOffset,int *hyOffset,int *hzOffset)
{

}
//get Hx,Hy,Hz.
int jy901b::getHxHyHz(float *hx,float *hy,float *hz)
{
    int nHx,nHy,nHz;
    int nHxSwp=0,nHySwp=0,nHzSwp=0;
    if(NULL==hx || NULL==hy || NULL==hz)
    {
        printf("<jy901b>:null parameter.");
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_HX,&nHx)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_HY,&nHy)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_HZ,&nHz)<0)
    {
        return -1;
    }
    nHxSwp=((nHx&0xFF)<<8)|((nHx&0xFF00)>>8);
    nHySwp=((nHy&0xFF)<<8)|((nHy&0xFF00)>>8);
    nHzSwp=((nHz&0xFF)<<8)|((nHz&0xFF00)>>8);
    printf("......Hx:%x(%x),Hy:%x(%x),Hz:%x(%x)\n",nHx,nHxSwp,nHy,nHySwp,nHz,nHzSwp);

    *hx=nHxSwp/100.0;
    *hy=nHySwp/100.0;
    *hz=nHzSwp/100.0;
    //printf("HxHyHz:%.2f,%.2f,%.2f\n",*hx,*hy,*hz);
    return 0;
}

//get Roll,Pitch,Yaw.
int jy901b::getRollPitchYaw(float *roll,float *pitch,float *yaw)
{
    int nRoll,nPitch,nYaw;
    int nRollSwp=0,nPitchSwp=0,nYawSwp=0;
    if(NULL==roll || NULL==pitch || NULL==yaw)
    {
        printf("<jy901b>:null parameter.");
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_POLL,&nRoll)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_PITCH,&nPitch)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_YAW,&nYaw)<0)
    {
        return -1;
    }
    nRollSwp=((nRoll&0xFF)<<8)|((nRoll&0xFF00)>>8);
    nPitchSwp=((nPitch&0xFF)<<8)|((nPitch&0xFF00)>>8);
    nYawSwp=((nYaw&0xFF)<<8)|((nYaw&0xFF00)>>8);
    printf("......roll:%x(%x),pitch:%x(%x),yaw:%x(%x)\n",nRoll,nRollSwp,nPitch,nPitchSwp,nYaw,nYawSwp);

    *roll=nRollSwp/32768.0*180.0;
    *pitch=nPitchSwp/32768.0*180.0;
    *yaw=nYawSwp/32768.0*180.0;
    //printf("roll:%.2f,%.2f,%.2f\n",*roll,*pitch,*yaw);
    return 0;
}

//get Pressure.
int jy901b::getPressure(int *pressure)
{
    int nPressureL,nPressureH;
    int nPressureLSwp,nPressureHSwp;
    if(NULL==pressure)
    {
        printf("<jy901b>:null parameter.");
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_PRESSURE_L,&nPressureL)<0)
    {
        return -1;
    }
    if(this->i2c_smbus_read_word_data(I2C_REG_ADDR_PRESSURE_H,&nPressureH)<0)
    {
        return -1;
    }
    nPressureLSwp=((nPressureL&0xFF)<<8)|((nPressureL&0xFF00)>>8);
    nPressureHSwp=((nPressureH&0xFF)<<8)|((nPressureH&0xFF00)>>8);
    *pressure=(nPressureHSwp<<16)|nPressureLSwp;
    return 0;
}
//get Longtude  High and Low.
int jy901b::getLongitudeHL(int *high,int *low)
{

}
//get Latitude High and Low.
int jy901b::getLatitude(int *high,int *low)
{
    if(NULL==high || NULL==low)
    {
        return -1;
    }

    return 0;
}
//set D0~D3 port mode.
int jy901b::setD0123Mode(int port,PortMode mode)
{
    char reg_addr;
    int reg_value;
    if(!(port>=0 && port<=3))
    {
        printf("<jy901b>:invalid port number,setD0123Mode().");
        return -1;
    }

    switch(port)
    {
    case 0:
        reg_addr=0x0e;
        break;
    case 1:
        reg_addr=0x0f;
        break;
    case 2:
        reg_addr=0x10;
        break;
    case 3:
        reg_addr=0x11;
        break;
    default:
        break;
    }

    switch(mode)
    {
    case Analog_In:
        break;
    case Digital_In:
        break;
    case Digital_Out_High:
        reg_value=0x0002;
        break;
    case Digital_Out_Low:
        reg_value=0x0003;
        break;
    case Digital_Out_Pwm:
        break;
    default:
        break;
    }
    return this->i2c_smbus_write_word_data(reg_addr,reg_value);
}
int g_ExitFlag=0;
void g_SigIntHandler(int signo)
{
    g_ExitFlag=1;
}
int main(void)
{
    jy901b dev1("/dev/i2c-7");
    if(dev1.doInit()<0)
    {
        return -1;
    }
    signal(SIGINT,g_SigIntHandler);
    while(!g_ExitFlag)
    {

        dev1.setD0123Mode(1,Digital_Out_High);
        sleep(1);
        //加速度输出.
        float ax,ay,az;
        dev1.getAxAyAz(&ax,&ay,&az);
        printf("Ax:%.2f,Ay:%.2f,Az:%.2f\n",ax,ay,az);

        //角速度输出.
        float gx,gy,gz;
        dev1.getGxGyGz(&gx,&gy,&gz);
        printf("Gx:%.2f,Gy:%.2f,Gz:%.2f\n",gx,gy,gz);

        //角度输出.
        float roll,pitch,yaw;
        dev1.getRollPitchYaw(&roll,&pitch,&yaw);
        printf("Roll:%.2f,Pitch:%.2f,Yaw:%.2f\n",roll,pitch,yaw);

        //磁场输出.
        float hx,hy,hz;
        dev1.getHxHyHz(&hx,&hy,&hz);
        printf("Hx:%.2f,Hy:%.2f,Hz:%.2f\n",hx,hy,hz);

        //压力输出.
        int pressure;
        dev1.getPressure(&pressure);
        printf("Pressure:%d (pa)\n",pressure);

        dev1.setD0123Mode(1,Digital_Out_Low);
        sleep(3);
    }
    dev1.doClean();
    return 0;
}
