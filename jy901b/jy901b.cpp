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
#define I2C_REG_ADDR_GXOFFSET       0x0b
#define I2C_REG_ADDR_GYOFFSET       0x0c
#define I2C_REG_ADDR_GZOFFSET       0x0d
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

#define I2C_REG_ADDR_IICADDR        0x1a
class jy901b
{
public:
    jy901b(const char *i2c_dev);
    ~jy901b();

    int doInit();
    int doClean();
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
    this->m_i2cdev=i2c_dev;
}
jy901b::~jy901b()
{
}
int jy901b::doInit()
{
    this->m_fd=open(i2c_dev,O_RDWR);
    if(this->m_fd<0)
    {
        printf("<error>:failed to open i2c dev:%s!\n",i2c_dev);
        return -1;
    }
    if(ioctl(this->m_fd,I2C_SLAVE,I2C_ADDR_JY901B)<0)
    {
        printf("<error>:failed to set i2c slave address!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
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
    args.data=data;
    if(ioctl(this->m_fd,I2C_SMBUS,&args)<0)
    {
        printf("<error>:failed to ioctl()!\n");
        return -1;
    }
    return 0;
}
int main(void)
{
    jy901b dev1("/dev/i2c-7");
    if(dev1.doInit()<0)
    {
        return -1;
    }
    dev1.doClean();
    return 0;
}
