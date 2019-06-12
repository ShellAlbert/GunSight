//in order to use our own fpga-i2s driver.
//modify the dts file.
//arch/arm64/boot/dts/rockchip/rk3399.dtsi.
//change the following line.
// i2s0:i2s@ff880000{
// compatible="rockchip,rk3399-i2s","rockchip,rk3066-i2s";
// to new text
// i2s0:i2s@ff880000{
// compatible="rockchip,fpga-i2s";

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <sound/pcm_params.h>
#include <sound/dmaengine_pcm.h>

#define DRV_NAME "rockchip,fpga-i2s"
//1s data size=(32000*3*2)=192000B //32khz,24bit,2channel.
#define PCM_BPS     (32000*3*2)
//2^23=8388608=8.3MB/192000B=43.69.
#define FIFO_SIZE   8388608//(2^23)

#ifndef BIT
#define BIT(x)		(1 << (x))
#endif

/*
 * TXCR
 * transmit operation control register
*/
#define I2S_TXCR_RCNT_SHIFT	17
#define I2S_TXCR_RCNT_MASK	(0x3f << I2S_TXCR_RCNT_SHIFT)
#define I2S_TXCR_CSR_SHIFT	15
#define I2S_TXCR_CSR(x)		(x << I2S_TXCR_CSR_SHIFT)
#define I2S_TXCR_CSR_MASK	(3 << I2S_TXCR_CSR_SHIFT)
#define I2S_TXCR_HWT		BIT(14)
#define I2S_TXCR_SJM_SHIFT	12
#define I2S_TXCR_SJM_R		(0 << I2S_TXCR_SJM_SHIFT)
#define I2S_TXCR_SJM_L		(1 << I2S_TXCR_SJM_SHIFT)
#define I2S_TXCR_FBM_SHIFT	11
#define I2S_TXCR_FBM_MSB	(0 << I2S_TXCR_FBM_SHIFT)
#define I2S_TXCR_FBM_LSB	(1 << I2S_TXCR_FBM_SHIFT)
#define I2S_TXCR_IBM_SHIFT	9
#define I2S_TXCR_IBM_NORMAL	(0 << I2S_TXCR_IBM_SHIFT)
#define I2S_TXCR_IBM_LSJM	(1 << I2S_TXCR_IBM_SHIFT)
#define I2S_TXCR_IBM_RSJM	(2 << I2S_TXCR_IBM_SHIFT)
#define I2S_TXCR_IBM_MASK	(3 << I2S_TXCR_IBM_SHIFT)
#define I2S_TXCR_PBM_SHIFT	7
#define I2S_TXCR_PBM_MODE(x)	(x << I2S_TXCR_PBM_SHIFT)
#define I2S_TXCR_PBM_MASK	(3 << I2S_TXCR_PBM_SHIFT)
#define I2S_TXCR_TFS_SHIFT	5
#define I2S_TXCR_TFS_I2S	(0 << I2S_TXCR_TFS_SHIFT)
#define I2S_TXCR_TFS_PCM	(1 << I2S_TXCR_TFS_SHIFT)
#define I2S_TXCR_TFS_MASK	(1 << I2S_TXCR_TFS_SHIFT)
#define I2S_TXCR_VDW_SHIFT	0
#define I2S_TXCR_VDW(x)		((x - 1) << I2S_TXCR_VDW_SHIFT)
#define I2S_TXCR_VDW_MASK	(0x1f << I2S_TXCR_VDW_SHIFT)

/*
 * RXCR
 * receive operation control register
*/
#define I2S_RXCR_CSR_SHIFT	15
#define I2S_RXCR_CSR(x)		(x << I2S_RXCR_CSR_SHIFT)
#define I2S_RXCR_CSR_MASK	(3 << I2S_RXCR_CSR_SHIFT)
#define I2S_RXCR_HWT		BIT(14)
#define I2S_RXCR_SJM_SHIFT	12
#define I2S_RXCR_SJM_R		(0 << I2S_RXCR_SJM_SHIFT)
#define I2S_RXCR_SJM_L		(1 << I2S_RXCR_SJM_SHIFT)
#define I2S_RXCR_FBM_SHIFT	11
#define I2S_RXCR_FBM_MSB	(0 << I2S_RXCR_FBM_SHIFT)
#define I2S_RXCR_FBM_LSB	(1 << I2S_RXCR_FBM_SHIFT)
#define I2S_RXCR_IBM_SHIFT	9
#define I2S_RXCR_IBM_NORMAL	(0 << I2S_RXCR_IBM_SHIFT)
#define I2S_RXCR_IBM_LSJM	(1 << I2S_RXCR_IBM_SHIFT)
#define I2S_RXCR_IBM_RSJM	(2 << I2S_RXCR_IBM_SHIFT)
#define I2S_RXCR_IBM_MASK	(3 << I2S_RXCR_IBM_SHIFT)
#define I2S_RXCR_PBM_SHIFT	7
#define I2S_RXCR_PBM_MODE(x)	(x << I2S_RXCR_PBM_SHIFT)
#define I2S_RXCR_PBM_MASK	(3 << I2S_RXCR_PBM_SHIFT)
#define I2S_RXCR_TFS_SHIFT	5
#define I2S_RXCR_TFS_I2S	(0 << I2S_RXCR_TFS_SHIFT)
#define I2S_RXCR_TFS_PCM	(1 << I2S_RXCR_TFS_SHIFT)
#define I2S_RXCR_TFS_MASK	(1 << I2S_RXCR_TFS_SHIFT)
#define I2S_RXCR_VDW_SHIFT	0
#define I2S_RXCR_VDW(x)		((x - 1) << I2S_RXCR_VDW_SHIFT)
#define I2S_RXCR_VDW_MASK	(0x1f << I2S_RXCR_VDW_SHIFT)

/*
 * CKR
 * clock generation register
*/
#define I2S_CKR_TRCM_SHIFT	28
#define I2S_CKR_TRCM(x)	(x << I2S_CKR_TRCM_SHIFT)
#define I2S_CKR_TRCM_TXRX	(0 << I2S_CKR_TRCM_SHIFT)
#define I2S_CKR_TRCM_TXONLY	(1 << I2S_CKR_TRCM_SHIFT)
#define I2S_CKR_TRCM_RXONLY	(2 << I2S_CKR_TRCM_SHIFT)
#define I2S_CKR_TRCM_MASK	(3 << I2S_CKR_TRCM_SHIFT)
#define I2S_CKR_MSS_SHIFT	27
#define I2S_CKR_MSS_MASTER	(0 << I2S_CKR_MSS_SHIFT)
#define I2S_CKR_MSS_SLAVE	(1 << I2S_CKR_MSS_SHIFT)
#define I2S_CKR_MSS_MASK	(1 << I2S_CKR_MSS_SHIFT)
#define I2S_CKR_CKP_SHIFT	26
#define I2S_CKR_CKP_NEG		(0 << I2S_CKR_CKP_SHIFT)
#define I2S_CKR_CKP_POS		(1 << I2S_CKR_CKP_SHIFT)
#define I2S_CKR_CKP_MASK	(1 << I2S_CKR_CKP_SHIFT)
#define I2S_CKR_RLP_SHIFT	25
#define I2S_CKR_RLP_NORMAL	(0 << I2S_CKR_RLP_SHIFT)
#define I2S_CKR_RLP_OPPSITE	(1 << I2S_CKR_RLP_SHIFT)
#define I2S_CKR_TLP_SHIFT	24
#define I2S_CKR_TLP_NORMAL	(0 << I2S_CKR_TLP_SHIFT)
#define I2S_CKR_TLP_OPPSITE	(1 << I2S_CKR_TLP_SHIFT)
#define I2S_CKR_MDIV_SHIFT	16
#define I2S_CKR_MDIV(x)		((x - 1) << I2S_CKR_MDIV_SHIFT)
#define I2S_CKR_MDIV_MASK	(0xff << I2S_CKR_MDIV_SHIFT)
#define I2S_CKR_RSD_SHIFT	8
#define I2S_CKR_RSD(x)		((x - 1) << I2S_CKR_RSD_SHIFT)
#define I2S_CKR_RSD_MASK	(0xff << I2S_CKR_RSD_SHIFT)
#define I2S_CKR_TSD_SHIFT	0
#define I2S_CKR_TSD(x)		((x - 1) << I2S_CKR_TSD_SHIFT)
#define I2S_CKR_TSD_MASK	(0xff << I2S_CKR_TSD_SHIFT)

/*
 * FIFOLR
 * FIFO level register
*/
#define I2S_FIFOLR_RFL_SHIFT	24
#define I2S_FIFOLR_RFL_MASK	(0x3f << I2S_FIFOLR_RFL_SHIFT)
#define I2S_FIFOLR_TFL3_SHIFT	18
#define I2S_FIFOLR_TFL3_MASK	(0x3f << I2S_FIFOLR_TFL3_SHIFT)
#define I2S_FIFOLR_TFL2_SHIFT	12
#define I2S_FIFOLR_TFL2_MASK	(0x3f << I2S_FIFOLR_TFL2_SHIFT)
#define I2S_FIFOLR_TFL1_SHIFT	6
#define I2S_FIFOLR_TFL1_MASK	(0x3f << I2S_FIFOLR_TFL1_SHIFT)
#define I2S_FIFOLR_TFL0_SHIFT	0
#define I2S_FIFOLR_TFL0_MASK	(0x3f << I2S_FIFOLR_TFL0_SHIFT)

/*
 * DMACR
 * DMA control register
*/
#define I2S_DMACR_RDE_SHIFT	24
#define I2S_DMACR_RDE_DISABLE	(0 << I2S_DMACR_RDE_SHIFT)
#define I2S_DMACR_RDE_ENABLE	(1 << I2S_DMACR_RDE_SHIFT)
#define I2S_DMACR_RDL_SHIFT	16
#define I2S_DMACR_RDL(x)	((x - 1) << I2S_DMACR_RDL_SHIFT)
#define I2S_DMACR_RDL_MASK	(0x1f << I2S_DMACR_RDL_SHIFT)
#define I2S_DMACR_TDE_SHIFT	8
#define I2S_DMACR_TDE_DISABLE	(0 << I2S_DMACR_TDE_SHIFT)
#define I2S_DMACR_TDE_ENABLE	(1 << I2S_DMACR_TDE_SHIFT)
#define I2S_DMACR_TDL_SHIFT	0
#define I2S_DMACR_TDL(x)	((x) << I2S_DMACR_TDL_SHIFT)
#define I2S_DMACR_TDL_MASK	(0x1f << I2S_DMACR_TDL_SHIFT)

/*
 * INTCR
 * interrupt control register
*/
#define I2S_INTCR_RFT_SHIFT	20
#define I2S_INTCR_RFT(x)	((x - 1) << I2S_INTCR_RFT_SHIFT)
#define I2S_INTCR_RXOIC		BIT(18)
#define I2S_INTCR_RXOIE_SHIFT	17
#define I2S_INTCR_RXOIE_DISABLE	(0 << I2S_INTCR_RXOIE_SHIFT)
#define I2S_INTCR_RXOIE_ENABLE	(1 << I2S_INTCR_RXOIE_SHIFT)
#define I2S_INTCR_RXFIE_SHIFT	16
#define I2S_INTCR_RXFIE_DISABLE	(0 << I2S_INTCR_RXFIE_SHIFT)
#define I2S_INTCR_RXFIE_ENABLE	(1 << I2S_INTCR_RXFIE_SHIFT)
#define I2S_INTCR_TFT_SHIFT	4
#define I2S_INTCR_TFT(x)	((x - 1) << I2S_INTCR_TFT_SHIFT)
#define I2S_INTCR_TFT_MASK	(0x1f << I2S_INTCR_TFT_SHIFT)
#define I2S_INTCR_TXUIC		BIT(2)
#define I2S_INTCR_TXUIE_SHIFT	1
#define I2S_INTCR_TXUIE_DISABLE	(0 << I2S_INTCR_TXUIE_SHIFT)
#define I2S_INTCR_TXUIE_ENABLE	(1 << I2S_INTCR_TXUIE_SHIFT)
#define I2S_INTCR_TXEIE_SHIFT	0
#define I2S_INTCR_TXEIE_DISABLE	(0 << I2S_INTCR_TXEIE_SHIFT)
#define I2S_INTCR_TXEIE_ENABLE	(1 << I2S_INTCR_TXEIE_SHIFT)

/*
 * INTSR
 * interrupt status register
*/
#define I2S_INTSR_TXEIE_SHIFT	0
#define I2S_INTSR_TXEIE_DISABLE	(0 << I2S_INTSR_TXEIE_SHIFT)
#define I2S_INTSR_TXEIE_ENABLE	(1 << I2S_INTSR_TXEIE_SHIFT)
#define I2S_INTSR_RXOI_SHIFT	17
#define I2S_INTSR_RXOI_INA	(0 << I2S_INTSR_RXOI_SHIFT)
#define I2S_INTSR_RXOI_ACT	(1 << I2S_INTSR_RXOI_SHIFT)
#define I2S_INTSR_RXFI_SHIFT	16
#define I2S_INTSR_RXFI_INA	(0 << I2S_INTSR_RXFI_SHIFT)
#define I2S_INTSR_RXFI_ACT	(1 << I2S_INTSR_RXFI_SHIFT)
#define I2S_INTSR_TXUI_SHIFT	1
#define I2S_INTSR_TXUI_INA	(0 << I2S_INTSR_TXUI_SHIFT)
#define I2S_INTSR_TXUI_ACT	(1 << I2S_INTSR_TXUI_SHIFT)
#define I2S_INTSR_TXEI_SHIFT	0
#define I2S_INTSR_TXEI_INA	(0 << I2S_INTSR_TXEI_SHIFT)
#define I2S_INTSR_TXEI_ACT	(1 << I2S_INTSR_TXEI_SHIFT)

/*
 * XFER
 * Transfer start register
*/
#define I2S_XFER_RXS_SHIFT	1
#define I2S_XFER_RXS_STOP	(0 << I2S_XFER_RXS_SHIFT)
#define I2S_XFER_RXS_START	(1 << I2S_XFER_RXS_SHIFT)
#define I2S_XFER_TXS_SHIFT	0
#define I2S_XFER_TXS_STOP	(0 << I2S_XFER_TXS_SHIFT)
#define I2S_XFER_TXS_START	(1 << I2S_XFER_TXS_SHIFT)

/*
 * CLR
 * clear SCLK domain logic register
*/
#define I2S_CLR_RXC	BIT(1)
#define I2S_CLR_TXC	BIT(0)

/*
 * TXDR
 * Transimt FIFO data register, write only.
*/
#define I2S_TXDR_MASK	(0xff)

/*
 * RXDR
 * Receive FIFO data register, write only.
*/
#define I2S_RXDR_MASK	(0xff)

/* Clock divider id */
enum {
    ROCKCHIP_DIV_MCLK = 0,
    ROCKCHIP_DIV_BCLK,
};

/* channel select */
#define I2S_CSR_SHIFT	15
#define I2S_CHN_2	(0 << I2S_CSR_SHIFT)
#define I2S_CHN_4	(1 << I2S_CSR_SHIFT)
#define I2S_CHN_6	(2 << I2S_CSR_SHIFT)
#define I2S_CHN_8	(3 << I2S_CSR_SHIFT)

/* I2S REGS */
#define I2S_TXCR	(0x0000)
#define I2S_RXCR	(0x0004)
#define I2S_CKR		(0x0008)
#define I2S_TXFIFOLR	(0x000c)
#define I2S_DMACR	(0x0010)
#define I2S_INTCR	(0x0014)
#define I2S_INTSR	(0x0018)
#define I2S_XFER	(0x001c)
#define I2S_CLR		(0x0020)
#define I2S_TXDR	(0x0024)
#define I2S_RXDR	(0x0028)
#define I2S_RXFIFOLR    (0x002c)

/* io direction cfg register */
#define I2S_IO_DIRECTION_MASK	(7)
#define I2S_IO_8CH_OUT_2CH_IN	(0)
#define I2S_IO_6CH_OUT_4CH_IN	(4)
#define I2S_IO_4CH_OUT_6CH_IN	(6)
#define I2S_IO_2CH_OUT_8CH_IN	(7)
///////////////////////////////////////////////////////
struct rk_i2s_pins {
    u32 reg_offset;
    u32 shift;
};
struct fpga_i2s_dev {
    struct device *dev;

    struct clk *hclk;
    struct clk *mclk;


    /*
 * Used to indicate the tx/rx status.
 * I2S controller hopes to start the tx and rx together,
 * also to stop them when they are both try to stop.
*/
    bool tx_start;
    bool rx_start;
    bool is_master_mode;
    const struct fpga_i2s_pins *pins;
    unsigned int bclk_fs;

    //register virutal address.
    void __iomem *paddr_regs;
    void __iomem *paddr_i2s_txcr;
    void __iomem *paddr_i2s_rxcr;
    void __iomem *paddr_i2s_ckr;
    void __iomem *paddr_i2s_txfifolr;
    void __iomem *paddr_i2s_dmacr;
    void __iomem *paddr_i2s_intcr;
    void __iomem *paddr_i2s_intsr;
    void __iomem *paddr_i2s_xfer;
    void __iomem *paddr_i2s_clr;
    void __iomem *paddr_i2s_txdr;
    void __iomem *paddr_i2s_rxdr;
    void __iomem *paddr_i2s_rxfifolr;

    //irq.
    int irq;
    struct kfifo fifo;
    //spinlock_t lock;
    struct mutex mutex;
    void *pcm_buffer;

    //wait queue.
    wait_queue_head_t wait_queue;
    bool brx_okay;

    //kernel thread.
    struct task_struct *gRdPcmTask;

    //dma.
    struct snd_dmaengine_dai_dma_data capture_dma_data;
    struct snd_dmaengine_dai_dma_data playback_dma_data;
};
static int devno_major;
static struct fpga_i2s_dev *g_i2s;
static int gRdPcmThread(void *arg)
{
    struct fpga_i2s_dev *i2s=(struct fpga_i2s_dev*)arg;
    while(!kthread_should_stop())
    {
        //set_current_state(TASK_UNINTERRUPTIBLE);

        //read finish,reset flag.
        i2s->brx_okay=false;
        //re-enable irq.
        enable_irq(i2s->irq);

        //wait until irq happened.
        if(0==wait_event_interruptible(i2s->wait_queue,i2s->brx_okay))
        {
            unsigned int nfifo,nfifo0;
            //read number of valid data entries.
            nfifo=ioread32(i2s->paddr_i2s_rxfifolr);
            nfifo0=(nfifo>>0)&0x3F;
            //printk(KERN_INFO DRV_NAME ":rx fifo0 %d\n",nfifo);

            //read data from RxFIFO.
            while(nfifo0-->0)
            {
                unsigned int pcm;
                unsigned char bytes[3];
                //volatile unsigned char newbytes;
                pcm=ioread32(i2s->paddr_i2s_rxdr);
                //pcm&=0x00FFFFFF;//only 24bits are valid.
                //printk(KERN_INFO DRV_NAME ":%0x\n",pcm);
                //0x12345600
                //Big-endian.
                bytes[0]=(unsigned char)((pcm&0xFF000000)>>24);//12
                bytes[1]=(unsigned char)((pcm&0xFF0000)>>16);//34
                bytes[2]=(unsigned char)((pcm&0xFF00)>>8);//56
                //little endian.
                //bytes[2]=(unsigned char)((pcm&0xFF000000)>>24);//12
                //bytes[1]=(unsigned char)((pcm&0xFF0000)>>16);//34
                //bytes[0]=(unsigned char)((pcm&0xFF00)>>8);//56

                //spin_lock(&i2s->lock);
                mutex_lock(&i2s->mutex);
                //overwrite last 3 bytes.
                //printk(KERN_INFO "fifo size:%d\n",kfifo_size(&i2s->fifo));
                //printk(KERN_INFO "fifo free:%d\n",kfifo_avail(&i2s->fifo));
                if(kfifo_avail(&i2s->fifo)<sizeof(bytes))
                {
                    int nCopied;
                    unsigned char nonvalid[3];
                    nCopied=kfifo_out(&i2s->fifo,nonvalid,sizeof(nonvalid));
                    printk(KERN_INFO "overwrite first 3 bytes!\n");
                }
                kfifo_in(&i2s->fifo,bytes,sizeof(bytes));
                //printk(KERN_INFO "fifo data:%d\n",kfifo_len(&i2s->fifo));
                //spin_unlock(&i2s->lock);
                mutex_unlock(&i2s->mutex);
            }
        }
    }
    return 0;
}

static irqreturn_t fpga_i2s_rx_irq(int irq, void *data)
{
    volatile unsigned int val;
    struct fpga_i2s_dev *i2s=(struct fpga_i2s_dev*)data;
    //disalbe irq & wake up wait queue.
    disable_irq_nosync(i2s->irq);
    //printk(KERN_INFO "irq happened fpga-i2s\n");

    //1.I2S_INTSR to detect which IRQ happened.
    val=ioread32(i2s->paddr_i2s_intsr);
    if(val&I2S_INTSR_RXOI_ACT)//RX overrun interrupt.
    {
        unsigned int val2;
        //write 1 to clear Rx overrun interrupt.
        val2=ioread32(i2s->paddr_i2s_intcr);
        val2=val2|I2S_INTCR_RXOIC;
        iowrite32(val2,i2s->paddr_i2s_intcr);
        //printk(KERN_INFO DRV_NAME "Rx overrun interrupt\n");
    }
    if(val&I2S_INTSR_RXFI_ACT)//RX full interrupt.
    {
        //read data from RxFIFO to clear.
        //printk(KERN_INFO DRV_NAME "Rx full interrupt\n");
    }
    if(val&I2S_INTSR_TXUI_ACT)//Tx underrun interrupt.
    {
        //write 1 to clear Tx underrun interrupt.
        volatile unsigned int val2;
        val2=ioread32(i2s->paddr_i2s_intcr);
        val2=val2|I2S_INTCR_TXUIC;
        iowrite32(val2,i2s->paddr_i2s_intcr);
        //printk(KERN_INFO DRV_NAME "Tx underrun interrupt\n");
    }
    if(val&I2S_INTSR_TXEI_ACT)//Tx empty interrupt.
    {
        //write data to TxFIFO to clear.
        //printk(KERN_INFO DRV_NAME "Tx empty interrupt\n");
    }

    //set flag.
    i2s->brx_okay=true;
    wake_up_interruptible(&i2s->wait_queue);

    return IRQ_HANDLED;
}
static int fpga_i2s_open(struct inode *inode,struct file*filp)
{
    volatile unsigned int val;
    struct fpga_i2s_dev *i2s=g_i2s;

    //tx&rx will be on together.the hard core hopes that.
    val=ioread32(i2s->paddr_i2s_xfer);
    val=val|I2S_XFER_RXS_START;//start RX transfer.
    //val=val|I2S_XFER_TXS_START;//start TX transfer.
    iowrite32(val,i2s->paddr_i2s_xfer);

    filp->private_data=i2s;

    i2s->gRdPcmTask=kthread_create(gRdPcmThread,(void*)g_i2s,"gRdPcmThread");
    if(IS_ERR(i2s->gRdPcmTask))
    {
        printk(KERN_INFO DRV_NAME ":failed to create kthread.\n");
        i2s->gRdPcmTask=NULL;
        return -1;
    }
    wake_up_process(i2s->gRdPcmTask);
    printk(KERN_INFO DRV_NAME ":open to enable tx/rx.\n");
    return 0;
}
static int fpga_i2s_release(struct inode *inode,struct file*filp)
{
    volatile unsigned int val;
    //tx&rx will be off together.the hard core hopes that.
    val=ioread32(g_i2s->paddr_i2s_xfer);
    val=val&(~(1<<I2S_XFER_RXS_SHIFT)|I2S_XFER_RXS_STOP);//stop RX transfer.
    iowrite32(val,g_i2s->paddr_i2s_xfer);

    if(g_i2s->gRdPcmTask)
    {
        kthread_stop(g_i2s->gRdPcmTask);
        //set flag to help kernel thread to exit.
        g_i2s->brx_okay=true;
        wake_up_interruptible(&g_i2s->wait_queue);
        g_i2s->gRdPcmTask=NULL;
    }
    disable_irq_nosync(g_i2s->irq);
    printk(KERN_INFO DRV_NAME ":release to disable tx/rx.\n");
    return 0;
}
//32khz sample rate,24-bit data width,2-channel
//so bps=32000*3bytes*2channels=192000.
static ssize_t fpga_i2s_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos)
{
    int nCopied=0;
    struct fpga_i2s_dev *i2s=filp->private_data;
    if(count<=0)
    {
        printk(KERN_INFO DRV_NAME ":read buffer is not enough!(%d)\n",(int)count);
        return -1;
    }

    //spin_lock(&i2s->lock);
    mutex_lock(&i2s->mutex);
    if(kfifo_to_user(&i2s->fifo,(void*)buf,count,&nCopied))
    {
        printk(KERN_INFO DRV_NAME ":kfifo_to_user failed!\n");
        //spin_unlock(&i2s->lock);
        mutex_unlock(&i2s->mutex);
        return -1;
    }
    if(nCopied!=count)
    {
        //printk(KERN_INFO DRV_NAME ":kfifo_to_user less/more data! (%d/%d)\n",count,nCopied);
        //spin_unlock(&i2s->lock);
        mutex_unlock(&i2s->mutex);
        return nCopied;
    }
    //spin_unlock(&i2s->lock);
    mutex_unlock(&i2s->mutex);
    return PCM_BPS;
}
static struct file_operations fpga_i2s_fops=
{
    .owner=THIS_MODULE,
    .open=fpga_i2s_open,
    .release=fpga_i2s_release,
    .read=fpga_i2s_read,
};
////////////////////////////////////////////////////
#define GRF_BASE_ADDR 0xFF770000
static int fpga_i2s_probe(struct platform_device *pdev)
{
    struct fpga_i2s_dev *i2s;
    struct resource *res;
    volatile unsigned int val;
    int ret;

    unsigned long *pbase=ioremap(GRF_BASE_ADDR+0x0e020,4);
    printk(KERN_ERR "val=%x\n",ioread32(pbase));
    //iowrite32(0xffff5555,pbase);
    //printk("val=%x\n",ioread32(pbase));
    iounmap(pbase);

    i2s = devm_kzalloc(&pdev->dev, sizeof(*i2s), GFP_KERNEL);
    if (!i2s) {
        dev_err(&pdev->dev, "Can't allocate fpga_i2s_dev\n");
        return -ENOMEM;
    }
    i2s->dev = &pdev->dev;

    //initial wait queue.
    init_waitqueue_head(&i2s->wait_queue);

    /* try to prepare related clocks */
    i2s->hclk = devm_clk_get(&pdev->dev, "i2s_hclk");
    if (IS_ERR(i2s->hclk)) {
        dev_err(&pdev->dev, "Can't retrieve i2s bus clock\n");
        return PTR_ERR(i2s->hclk);
    }
    ret=clk_prepare_enable(i2s->hclk);
    if(ret)
    {
        dev_err(i2s->dev, "hclock enable failed %d\n", ret);
        return ret;
    }
    ///////////////////////////////////////////////////////////////
    i2s->mclk = devm_clk_get(&pdev->dev, "i2s_clk");
    if (IS_ERR(i2s->mclk)) {
        dev_err(&pdev->dev, "Can't retI2S_CKR_TLP_SHIFTrieve i2s master clock\n");
        return PTR_ERR(i2s->mclk);
    }
    ret = clk_prepare_enable(i2s->mclk);
    if (ret) {
        dev_err(&pdev->dev, "clock enable failed %d\n", ret);
        return PTR_ERR(i2s->mclk);
    }
    /////////////////////////////////////////////////////////////////
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    i2s->paddr_regs = devm_ioremap_resource(&pdev->dev, res);
    if(IS_ERR(i2s->paddr_regs))
    {
        return PTR_ERR(i2s->paddr_regs);
    }
    /////////////////////////////////////////////////
    i2s->irq=platform_get_irq(pdev,0);
    printk(KERN_INFO "get irq:%d\n",i2s->irq);
    ret=devm_request_irq(&pdev->dev,i2s->irq,&fpga_i2s_rx_irq, 0, DRV_NAME,i2s);
    if(ret)
    {
        dev_err(&pdev->dev,"Error requesting IRQ!\n");
        return -1;
    }

    ////////////////////////////////////////////////////////
    //fetch each register's address.
    i2s->paddr_i2s_txcr=i2s->paddr_regs+I2S_TXCR;
    i2s->paddr_i2s_rxcr=i2s->paddr_regs+I2S_RXCR;
    i2s->paddr_i2s_ckr=i2s->paddr_regs+I2S_CKR;
    i2s->paddr_i2s_txfifolr=i2s->paddr_regs+I2S_TXFIFOLR;
    i2s->paddr_i2s_dmacr=i2s->paddr_regs+I2S_DMACR;
    i2s->paddr_i2s_intcr=i2s->paddr_regs+I2S_INTCR;
    i2s->paddr_i2s_intsr=i2s->paddr_regs+I2S_INTSR;
    i2s->paddr_i2s_xfer=i2s->paddr_regs+I2S_XFER;
    i2s->paddr_i2s_clr=i2s->paddr_regs+I2S_CLR;
    i2s->paddr_i2s_txdr=i2s->paddr_regs+I2S_TXDR;
    i2s->paddr_i2s_rxdr=i2s->paddr_regs+I2S_RXDR;
    i2s->paddr_i2s_rxfifolr=i2s->paddr_regs+I2S_RXFIFOLR;
    /////////////////////////////////////////////////
    dev_set_drvdata(&pdev->dev, i2s);
    ///////////////////////////////////////////////
    //config i2s registers.
    //1.stop Rx/Tx transfer.
    iowrite32(0,i2s->paddr_i2s_xfer);
    //2.clear all rx&tx logic.
    val=ioread32(i2s->paddr_i2s_clr);
    val=val|I2S_CLR_RXC;
    val=val|I2S_CLR_TXC;
    printk("I2S_CLR:%x\n",val);
    iowrite32(val,i2s->paddr_i2s_clr);
    printk("before self-clear!\n");
    //wait until self-clear okay.
    while(1)
    {
        val=ioread32(i2s->paddr_i2s_clr);
        if((val&I2S_CLR_RXC) || (val&I2S_CLR_TXC))
        {
            udelay(1000);
            continue;
        }else{
            //two bits are cleared by inter logic.
            break;
        }
    }
    
    //3.config I2S_RXCR.
    val=ioread32(i2s->paddr_i2s_rxcr);
    val=val&(~I2S_RXCR_CSR_MASK);//2'b00:two channel.
    //HWT:only valid when VDW select 16bit data.
    val=val|(0x1<<12);//SJM,right-justified.
    val=val&(~I2S_RXCR_FBM_MSB);//MSB.
    //val=val|I2S_RXCR_FBM_LSB;//LSB.
    val=(val&(~I2S_RXCR_IBM_MASK))|I2S_RXCR_IBM_NORMAL;//I2S Bus Mode=I2S normal.
    //val&=(~I2S_RXCR_PBM_MASK)|I2S_RXCR_PBM_MODE(1);//PCM Bus Mode=PCM delay 1 mode.
    //val&=(~I2S_RXCR_TFS_MASK|I2S_RXCR_TFS_I2S);//transfer format select:i2s.
    val=(val&(~I2S_RXCR_VDW_MASK))|I2S_RXCR_VDW(24);//24-bit valid data width.
    printk(KERN_INFO DRV_NAME "I2S_RXCR:%x\n",val);
    iowrite32(val,i2s->paddr_i2s_rxcr);

    //3.config I2S_CKR.
    val=ioread32(i2s->paddr_i2s_ckr);
    val=(val&(~I2S_CKR_TRCM_MASK))|I2S_CKR_TRCM_TXRX;//tx_lrck/rx_lrcr for Tx/Rx.
    val=(val&(~I2S_CKR_MSS_MASK))|I2S_CKR_MSS_SLAVE;//slave mode,SCLK input.
    val=(val&(~I2S_CKR_CKP_MASK))|I2S_CKR_CKP_NEG;//0:sample at posedge clk,drive at negedge clk.
    val=(val&(~(1<<I2S_CKR_RLP_SHIFT)))|I2S_CKR_RLP_NORMAL;//i2s normal,low for left,high for right channel.
    //val&=(~(1<<I2S_CKR_TLP_SHIFT))|I2S_CKR_TLP_NORMAL;//Transmit lrck polarity.
    val=(val&(~I2S_CKR_MDIV_MASK))|I2S_CKR_MDIV(64);//mclk divider. Fmclk=256*Ftxsclk.
    val=(val&(~I2S_CKR_RSD_MASK))|I2S_CKR_RSD(64);//receive sclk divider=Fsclk/Frxlrck.(2.048MHz/32kHz/=64).
    val=(val&(~I2S_CKR_TSD_MASK))|I2S_CKR_TSD(64);//transmit sclk divider=Fsclk/Ftxlrck.(255:256fs).
    printk(KERN_INFO DRV_NAME "I2S_CKR:%x\n",val);
    iowrite32(val,i2s->paddr_i2s_ckr);

    //4.disable DMA,here we use IRQ to fetch data.
    val=ioread32(i2s->paddr_i2s_dmacr);
    val=(val&(~(1<<I2S_DMACR_RDE_SHIFT)))|I2S_DMACR_RDE_DISABLE;//Receive DMA disabled.
    //Receive Data Level,we donot care.
    //This bit field controls the level at which a DMA request is made by the receive logic.
    val=(val&(~(1<<I2S_DMACR_TDE_SHIFT)))|I2S_DMACR_TDE_DISABLE;//Transmit DMA disabled.
    //Transmit Data Level,we donot care.
    //This bit field controls the level at which a DMA request is made by the transmit logic.
    printk(KERN_INFO DRV_NAME "I2S_DMACR:%x\n",val);
    iowrite32(val,i2s->paddr_i2s_dmacr);

    //5.config I2S_INTCR,enable rx interrupt.
    val=ioread32(i2s->paddr_i2s_intcr);
    //Receive FIFO threshold.
    //the receive FIFO full interrupt is triggered.
    val=(val&(~I2S_INTCR_RFT(32)))|I2S_INTCR_RFT(16);
    val=val|I2S_INTCR_RXOIC;//write 1 to clear RX overrun interrupt.
    val=val|I2S_INTCR_RXOIE_ENABLE;//enable RX overrun interrupt.
    val=val|I2S_INTCR_RXFIE_ENABLE;//enable RX full interrupt.

    //Transmit FIFO threshold.
    //the transmit FIFO empty interrupt is triggered.
    val=val&(~I2S_INTCR_TFT(32)|I2S_INTCR_TFT(32));
    val=val|I2S_INTCR_TXUIC;//write 1 to clear Tx underrun interrupt.
    val=val&(~(1<<I2S_INTCR_TXUIE_SHIFT)|I2S_INTCR_TXUIE_DISABLE);//disable Tx underrun interrupt.
    val=val&(~(1<<I2S_INTCR_TXEIE_SHIFT)|I2S_INTCR_TXEIE_DISABLE);//disable Tx empty interrupt.
    printk(KERN_INFO DRV_NAME "I2S_INTCR:%x\n",val);
    iowrite32(val,i2s->paddr_i2s_intcr);
    
    ///////////////////////////////////////////////
    //dynamic allocated device major number.
    devno_major=register_chrdev(0,DRV_NAME,&fpga_i2s_fops);
    if(devno_major<0)
    {
        printk(KERN_ERR DRV_NAME ":failed to allocate major number!\n");
        return -1;
    }
    printk(KERN_ERR DRV_NAME ":allocate device no (%d,0)!\n",devno_major);
    ///////////////////////////////////////////////
    i2s->pcm_buffer=vmalloc(FIFO_SIZE);
    if(NULL==i2s->pcm_buffer)
    {
        printk(KERN_ERR DRV_NAME ":failed to allocate pcm buffer.\n");
        return -1;
    }
    if(kfifo_init(&i2s->fifo,i2s->pcm_buffer,FIFO_SIZE))
    {
        printk(KERN_ERR DRV_NAME ":failed to allocate fifo.\n");
        return -1;
    }
    //    if(kfifo_alloc(&i2s->fifo,FIFO_SIZE,GFP_KERNEL))
    //    {
    //        printk(KERN_ERR DRV_NAME ":failed to allocate fifo.\n");
    //        return -1;
    //    }
    //spin_lock_init(&i2s->lock);
    mutex_init(&i2s->mutex);

    //use global pointer to tract it.
    g_i2s=i2s;
    printk(KERN_INFO DRV_NAME ":module loaded.\n");

    return 0;
}

static int fpga_i2s_remove(struct platform_device *pdev)
{
    struct fpga_i2s_dev *i2s = dev_get_drvdata(&pdev->dev);

    devm_free_irq(&pdev->dev,i2s->irq,i2s);

    unregister_chrdev(devno_major,DRV_NAME);

    clk_disable_unprepare(i2s->mclk);
    clk_disable_unprepare(i2s->hclk);

    devm_ioport_unmap(&pdev->dev,i2s->paddr_regs);

    //kfifo_free(&i2s->fifo);
    vfree(i2s->pcm_buffer);

    printk(KERN_INFO DRV_NAME ":module unloaded.\n");
    return 0;
}
////////////////////////////////////////////////////////////////
static const struct of_device_id fpga_i2s_match[]={
{.compatible="rockchip,fpga-i2s",},
};
static struct platform_driver fpga_i2s_driver = {
    .probe = fpga_i2s_probe,
    .remove = fpga_i2s_remove,
    .driver = {
        .name = DRV_NAME,
        .of_match_table=of_match_ptr(fpga_i2s_match),
    },
};
module_platform_driver(fpga_i2s_driver);
//////////////////////////////////////////////////////////////
MODULE_DESCRIPTION("RK3399 (NanoPCT4) FPGA-I2S Driver Module");
MODULE_AUTHOR("shell.albert <1811543668@qq.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
