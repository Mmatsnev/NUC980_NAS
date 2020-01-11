/*
 * Copyright (c) 2018 Nuvoton Technology Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>

#include <mach/mfp.h>
#include <linux/platform_data/spi-nuc980.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/irq.h>

#include <mach/hardware.h>
#include <mach/regs-gcr.h>

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <mach/regs-pdma.h>
#include <mach/regs-clock.h>

#include <linux/platform_data/dma-nuc980.h>

/* spi registers offset */
#define REG_CTL		0x00
#define REG_CLKDIV	0x04
#define REG_SSCTL	0x08
#define REG_PDMACTL	0x0C
#define REG_FIFOCTL	0x10
#define REG_STATUS	0x14
#define REG_TX		0x20
#define REG_RX		0x30

/* spi register bit */
#define UNITIEN		(0x01 << 17)
#define TXNEG		(0x01 << 2)
#define RXNEG		(0x01 << 1)
#define SLAVE		(0x01 << 18)
#define LSB		(0x01 << 13)
#define SELECTLEV	(0x01 << 2)
#define SELECTPOL	(0x01 << 3)
#define SSACTIEN	(0x01 << 12)
#define SELECTSLAVE0	0x01
#define SELECTSLAVE1	0x02
#define SPIEN		0x01
#define TXTHIEN		(0x01 << 3)
#define RXTHIEN		(0x01 << 2)
#define SSINAIEN	(0x01 << 13)
#define SSACTIF		(0x01 << 2)
#define SSINAIF		(0x01 << 3)
#define RXTHIF		(0x01 << 10)
#define TXTHIF		(0x01 << 18)
#define TXUFIF		(0x01 << 19)


#define SPIx_TX NUC980_PA_SPI2 + 0x20
#define SPIx_RX NUC980_PA_SPI2 + 0x30
#define PDMA_SPIx_TX PDMA_SPI2_TX
#define PDMA_SPIx_RX PDMA_SPI2_RX


#ifdef CONFIG_SPI_NUC980_SPI1
# error Do not enable CONFIG_SPI_NUC980_SPI1 and CONFIG_NUC980_SPI1_SLAVE at the same time!
#endif

#define ENTRY()                 printk("[%-20s] : Enter...\n", __FUNCTION__)
#define LEAVE()                 printk("[%-20s] : Leave...\n", __FUNCTION__)

static int is_spidev = 0;
/* DMA */
static u8 dummy_buf[4096];

static volatile int spi1_slave_done_state=0;
static DECLARE_WAIT_QUEUE_HEAD(spi1_slave_done);

struct ts_spi_slave {
u8 *rx_buf;
u32 len;
};

struct nuc980_ip_dma {
	struct dma_chan                 *chan_rx;
	struct dma_chan                 *chan_tx;
	struct scatterlist              sgrx;
	struct scatterlist              sgtx;
	struct dma_async_tx_descriptor  *rxdesc;
	struct dma_async_tx_descriptor  *txdesc;
	struct dma_slave_config slave_config;
};

static struct nuc980_ip_dma dma;
struct nuc980_mem_alloc spi1_src_mem_p;
struct nuc980_mem_alloc spi1_dest_mem_p;

struct nuc980_dma_done  spi1_dma_slave_done;

//
//static volatile int slave_done_state=0;
//static DECLARE_WAIT_QUEUE_HEAD(slave_done);
//
//static int SPI1_SlaveDataLen = 188;
//static int SPI1_SlaveData[188];
//static int TransmittedCnt = 0;
//static int InTransmitted = 0;
//static int Transmit_done = 0;

struct nuc980_spi {
	struct spi_bitbang	bitbang;
	struct completion	done;
	void __iomem		*regs;
	int			irq;
	unsigned int 		len;
	unsigned int		count;
	const void		*tx;
	void			*rx;
	struct clk		*clk;
	struct resource		*ioarea;
	struct spi_master	*master;
	struct spi_device	*curdev;
	struct device		*dev;
	struct nuc980_spi_info *pdata;
	spinlock_t		lock;
	struct resource		*res;
};

struct TS_DTMB_SPI_DATA {

    struct mutex lock;
    struct miscdevice ts_dtmb_misc;
};

struct TS_DTMB_SPI_DATA *ts_dtmb_data_dev;

//dma callback function
static void spi1_nuc980_slave_dma_callback(void *arg)
{
	struct nuc980_dma_done *done = arg;

	done->done = true;
	spi1_slave_done_state = 1;
	wake_up_interruptible(&spi1_slave_done);
	return;
}

static inline struct nuc980_spi1 *to_hw(struct spi_device *sdev) {
	return spi_master_get_devdata(sdev->master);
}

static inline void nuc980_slave_select(struct spi_device *spi, unsigned int ssr)
{
	struct nuc980_spi *hw = (struct nuc980_spi *)to_hw(spi);
	unsigned int val;
	unsigned int cs = spi->mode & SPI_CS_HIGH ? 1 : 0;
	unsigned long flags;

	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_SSCTL);

	if (!cs)
		val &= ~SELECTLEV;
	else
		val |= SELECTLEV;

	if(spi->chip_select == 0) {
		if (!ssr)
			val &= ~SELECTSLAVE0;
		else
			val |= SELECTSLAVE0;
	} else {
		if (!ssr)
			val &= ~SELECTSLAVE1;
		else
			val |= SELECTSLAVE1;
	}

	__raw_writel(val, hw->regs + REG_SSCTL);

	spin_unlock_irqrestore(&hw->lock, flags);

}

static inline void nuc980_spi1_chipsel(struct spi_device *spi, int value)
{
    ENTRY();

	switch (value) {
	case BITBANG_CS_INACTIVE:
		nuc980_slave_select(spi, 0);
		break;

	case BITBANG_CS_ACTIVE:
		nuc980_slave_select(spi, 1);
		break;
	}
	    LEAVE();
}

static inline void nuc980_spi1_setup_txbitlen(struct nuc980_spi *hw,
                unsigned int txbitlen)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);
	val &= ~0x1f00;
	if(txbitlen != 32)
		val |= (txbitlen << 8);

	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);

}

//static int nuc980_spi1_txrx(struct spi_device *spi, struct ts_spi_slave *t)
static int nuc980_spi1_txrx(struct nuc980_spi *hw, struct ts_spi_slave *t)
{
//	struct nuc980_spi *hw = (struct nuc980_spi *)to_hw(spi);

	struct nuc980_ip_dma *pdma=&dma;
	struct nuc980_dma_config dma_crx,dma_ctx;
	dma_cookie_t            cookie;

	__raw_writel(__raw_readl(hw->regs + REG_FIFOCTL) | 0x3, hw->regs + REG_FIFOCTL); //CWWeng : RXRST & TXRST
	while (__raw_readl(hw->regs + REG_STATUS) & (1<<23)); //TXRXRST

	__raw_writel(__raw_readl(hw->regs + REG_PDMACTL)&~(0x3), hw->regs + REG_PDMACTL); //Disable SPIx TX/RX PDMA


	if (t->rx_buf) {
		/* prepare the RX dma transfer */
		sg_init_table(&pdma->sgrx, 1);
		pdma->slave_config.src_addr = SPIx_RX;
		if (!is_spidev && !(t->len % 4) && !(((int)t->rx_buf) % 4)) {
			__raw_writel((__raw_readl(hw->regs + REG_CTL)&~(0x1F00))|(0x80000), hw->regs + REG_CTL);//32 bits,byte reorder
			pdma->slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
			pdma->sgrx.length=t->len/4;
		} else {
			pdma->slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
			pdma->sgrx.length=t->len;
		}
		pdma->slave_config.src_maxburst = 1;
		pdma->slave_config.direction = DMA_DEV_TO_MEM;
		pdma->slave_config.device_fc = false;
		dmaengine_slave_config(pdma->chan_rx,&(pdma->slave_config));

		pdma->sgrx.dma_address = dma_map_single(hw->dev,
		                                        (void *)t->rx_buf,
		                                        t->len, DMA_FROM_DEVICE);
		if (dma_mapping_error(hw->dev, pdma->sgrx.dma_address)) {
			dev_err(hw->dev, "rx dma map error\n");
		}

		dma_crx.reqsel = PDMA_SPIx_RX;
		dma_crx.timeout_counter = 0;
		dma_crx.timeout_prescaler = 0;
		dma_crx.en_sc = 0;
		pdma->rxdesc=pdma->chan_rx->device->device_prep_slave_sg(pdma->chan_rx,
		                &pdma->sgrx,
		                1,
		                DMA_FROM_DEVICE,
		                DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
		                (void *)&dma_crx); //PDMA Request Source Select
		if (!pdma->rxdesc) {
			printk("pdma->rxdesc=NULL\n");
			BUG();
		}
		spi1_dma_slave_done.done = false;
		pdma->rxdesc->callback = spi1_nuc980_slave_dma_callback;
		pdma->rxdesc->callback_param = &spi1_dma_slave_done;
		cookie = pdma->rxdesc->tx_submit(pdma->rxdesc);
		if (dma_submit_error(cookie)) {
			printk("rx cookie=%d\n",cookie);
			BUG();
		}
	}

	/* prepare the TX dma transfer */
//	sg_init_table(&pdma->sgtx, 1);
//	pdma->slave_config.dst_addr = SPIx_TX;
//	if (!is_spidev && !(t->len % 4) && !(((int)t->tx_buf) % 4)) {
//		__raw_writel((__raw_readl(hw->regs + REG_CTL)&~(0x1F00))|(0x80000), hw->regs + REG_CTL);//32 bits,byte reorder
//		pdma->slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
//		pdma->sgtx.length=t->len/4;
//	} else {
//		pdma->slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
//		pdma->sgtx.length=t->len;
//	}
//	pdma->slave_config.dst_maxburst = 1;
//	pdma->slave_config.direction = DMA_MEM_TO_DEV;
//	dmaengine_slave_config(pdma->chan_tx,&(pdma->slave_config));
//	if (t->tx_buf) {
//		pdma->sgtx.dma_address = dma_map_single(hw->dev,
//		                                        (void *)t->tx_buf,
//		                                        t->len, DMA_TO_DEVICE);
//		if (dma_mapping_error(hw->dev, pdma->sgtx.dma_address)) {
//			dev_err(hw->dev, "tx dma map error\n");
//		}
//	} else {
//		pdma->sgtx.dma_address=virt_to_phys(dummy_buf);
//	}
//
//	dma_ctx.reqsel = PDMA_SPIx_TX;
//	dma_ctx.timeout_counter = 0;
//	dma_ctx.timeout_prescaler = 0;
//	dma_ctx.en_sc = 0;
//	pdma->txdesc=pdma->chan_tx->device->device_prep_slave_sg(pdma->chan_tx,
//	                &pdma->sgtx,
//	                1,
//	                DMA_TO_DEVICE,
//	                DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
//	                (void *)&dma_ctx);
//	if (!pdma->txdesc) {
//		printk("pdma->txdex=NULL\n");
//		BUG();
//	}
//
//	if (!t->rx_buf) {
//		pdma->txdesc->callback = spi1_nuc980_slave_dma_callback;
//		pdma->txdesc->callback_param = &spi1_dma_slave_done;
//	} else {
//		pdma->txdesc->callback = NULL;
//		pdma->txdesc->callback_param = NULL;
//	}
//
//	cookie = pdma->txdesc->tx_submit(pdma->txdesc);
//	if (dma_submit_error(cookie)) {
//		printk("tx cookie=%d\n",cookie);
//		BUG();
//	}

//	if (t->rx_buf)
//		__raw_writel(__raw_readl(hw->regs + REG_PDMACTL)|(0x3), hw->regs + REG_PDMACTL); //Enable SPIx TX/RX PDMA
//	else
//		__raw_writel(__raw_readl(hw->regs + REG_PDMACTL)|(0x1), hw->regs + REG_PDMACTL); //Enable SPIx TX PDMA
    if (t->rx_buf)
    __raw_writel(__raw_readl(hw->regs + REG_PDMACTL)|(0x2), hw->regs + REG_PDMACTL); //Enable SPIx RX PDMA

	wait_event_interruptible(spi1_slave_done, (spi1_slave_done_state != 0));
	spi1_slave_done_state=0;

//    int p_num;
//    for(p_num = 0;p_num < 188;p_num++)
//    {
//        if(p_num % 10 == 0)
//            printk("[0x%02x] \n", *(t->rx_buf + p_num));
//        else
//            printk("[0x%02x] \n", *(t->rx_buf + p_num));
//    }
	while(__raw_readl(hw->regs + REG_STATUS) & 1); //wait busy

	/* unmap buffers if mapped above */
	if (t->rx_buf)
		dma_unmap_single(hw->dev, pdma->sgrx.dma_address, t->len,
		                 DMA_FROM_DEVICE);
//	if (t->tx_buf)
//		dma_unmap_single(hw->dev, pdma->sgtx.dma_address, t->len,
//		                 DMA_TO_DEVICE);

	__raw_writel(((__raw_readl(hw->regs + REG_CTL) & ~(0x81F00))|0x800), hw->regs + REG_CTL); //restore to 8 bits, no byte reorder



	return t->len;
}

static inline void nuc980_set_clock_polarity(struct nuc980_spi *hw, unsigned int polarity)
{
	unsigned int val;
	unsigned long flags;
    ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	if (polarity)
		val |= SELECTPOL;
	else
		val &= ~SELECTPOL;
	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);
	    LEAVE();
}

static inline void nuc980_tx_edge(struct nuc980_spi *hw, unsigned int edge)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	if (edge)
		val |= TXNEG;
	else
		val &= ~TXNEG;
	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);
	LEAVE();
}

static inline void nuc980_rx_edge(struct nuc980_spi *hw, unsigned int edge)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	if (edge)
		val |= RXNEG;
	else
		val &= ~RXNEG;
	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);
	LEAVE();
}

static inline void nuc980_send_first(struct nuc980_spi *hw, unsigned int lsb)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	if (lsb)
		val |= LSB;
	else
		val &= ~LSB;
	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);
LEAVE();
}

static inline void nuc980_enable_slave(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	val |= SLAVE;

	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);

}

static inline void nuc980_set_sleep(struct nuc980_spi *hw, unsigned int sleep)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_CTL);

	val &= ~(0x0f << 4);

	if (sleep)
		val |= (sleep << 4);

	__raw_writel(val, hw->regs + REG_CTL);

	spin_unlock_irqrestore(&hw->lock, flags);
	LEAVE();
}


static inline void nuc980_set_divider(struct nuc980_spi *hw)
{

	__raw_writel(hw->pdata->divider, hw->regs + REG_CLKDIV);

}

static int nuc980_spi1_update_state(struct spi_device *spi,
                                    struct spi_transfer *t)
{
	struct nuc980_spi *hw = (struct nuc980_spi *)to_hw(spi);
	unsigned int clk;
	unsigned int div;
	unsigned int bpw;
	unsigned int hz;
	unsigned char spimode;
ENTRY();
	bpw = t ? t->bits_per_word : spi->bits_per_word;
	hz  = t ? t->speed_hz : spi->max_speed_hz;

	if(hw->pdata->txbitlen != bpw)
		hw->pdata->txbitlen = bpw;

	if(hw->pdata->hz != hz) {
		clk = clk_get_rate(hw->clk);
		div = DIV_ROUND_UP(clk, hz) - 1;
		hw->pdata->hz = hz;
		hw->pdata->divider = div;
	}

	//Mode 0: CPOL=0, CPHA=0; active high
	//Mode 1: CPOL=0, CPHA=1 ;active low
	//Mode 2: CPOL=1, CPHA=0 ;active low
	//Mode 3: POL=1, CPHA=1;active high
	if (spi->mode & SPI_CPOL)
		hw->pdata->clkpol = 1;
	else
		hw->pdata->clkpol = 0;

	spimode = spi->mode & 0xff; //remove dual/quad bit

	if ((spimode == SPI_MODE_0) || (spimode == SPI_MODE_3)) {
		hw->pdata->txneg = 1;
		hw->pdata->rxneg = 0;
	} else {
		hw->pdata->txneg = 0;
		hw->pdata->rxneg = 1;
	}

	if (spi->mode & SPI_LSB_FIRST)
		hw->pdata->lsb = 1;
	else
		hw->pdata->lsb = 0;
LEAVE();
	return 0;
}

static int nuc980_spi1_setupxfer(struct spi_device *spi,
                                 struct spi_transfer *t)
{
	struct nuc980_spi *hw = (struct nuc980_spi *)to_hw(spi);
	int ret;
ENTRY();
	ret = nuc980_spi1_update_state(spi, t);
	if (ret)
		return ret;

	nuc980_spi1_setup_txbitlen(hw, hw->pdata->txbitlen);
	nuc980_tx_edge(hw, hw->pdata->txneg);
	nuc980_rx_edge(hw, hw->pdata->rxneg);
	nuc980_set_clock_polarity(hw, hw->pdata->clkpol);
	nuc980_send_first(hw, hw->pdata->lsb);
	nuc980_set_divider(hw);
LEAVE();
	return 0;
}

static int nuc980_spi1_setup(struct spi_device *spi)
{
	struct nuc980_spi *hw = (struct nuc980_spi *)to_hw(spi);
	int ret;
ENTRY();
	ret = nuc980_spi1_update_state(spi, NULL);
	if (ret)
		return ret;

	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		nuc980_set_divider(hw);
		nuc980_slave_select(spi, 0);
	}
	spin_unlock(&hw->bitbang.lock);
LEAVE();
	return 0;
}

static inline void nuc980_enable_rxth_int(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_FIFOCTL);

	val = (val & ~0x7000000) | 0x0000000; /* set RXTH = 0 */
//	val = (val & ~0x7000000) | 0x4000000; /* set RXTH = 4 */
	val |= RXTHIEN; /* enable RXTHIEN */

	__raw_writel(val, hw->regs + REG_FIFOCTL);

    spin_unlock_irqrestore(&hw->lock, flags);



}

static inline void nuc980_disable_rxth_int(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_FIFOCTL);

	val &= ~RXTHIEN; /* disable RXTHIEN */

	__raw_writel(val, hw->regs + REG_FIFOCTL);

	spin_unlock_irqrestore(&hw->lock, flags);


}

static inline void nuc980_enable_txth_int(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_FIFOCTL);

	val = (val & ~0x70000000) | 0x30000000; /* set TXTH = 3 */
	val |= TXTHIEN; /* enable TXTHIEN */

	__raw_writel(val, hw->regs + REG_FIFOCTL);

	spin_unlock_irqrestore(&hw->lock, flags);

LEAVE();

}

static inline void nuc980_disable_txth_int(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_FIFOCTL);

	val &= ~TXTHIEN; /* disable TXTHIEN */

	__raw_writel(val, hw->regs + REG_FIFOCTL);

	spin_unlock_irqrestore(&hw->lock, flags);
LEAVE();
}

static inline void nuc980_enable_ssinact_int(struct nuc980_spi *hw)
{
	unsigned int val;
	unsigned long flags;
ENTRY();
	spin_lock_irqsave(&hw->lock, flags);

	val = __raw_readl(hw->regs + REG_SSCTL);

	val |= SSINAIEN; /* enable SSINAIEN */

	__raw_writel(val, hw->regs + REG_SSCTL);

	spin_unlock_irqrestore(&hw->lock, flags);
	LEAVE();
}

////static irqreturn_t nuc980_spi1_irq(int irq, void *dev)
//{
//	struct nuc980_spi *hw = dev;
//	unsigned int val, status;
//
//	status = __raw_readl(hw->regs + REG_STATUS);
//	__raw_writel(status, hw->regs + REG_STATUS);
////printk("SPI1_STATUS = [0x%08x] \n", __raw_readl(hw->regs + REG_STATUS));
//	if (status & RXTHIF) {
////	    printk("SPI1_irq= RXTHIF ");
////		if (InTransmitted == 0) {
//////			nuc980_disable_rxth_int(hw);
////
////
////			InTransmitted = 1;
////		}
//		//	    slave_done_state = 1;
//	    Transmit_done = 1;
//		slave_done_state = 1;
//		wake_up_interruptible(&slave_done);
//		nuc980_disable_rxth_int(hw);
//	}
////	if (status & TXTHIF) {
////	    printk("SPI1_irq= TXTHIF);
////		while(!(__raw_readl(hw->regs + REG_STATUS) & 0x20000)) {//TXFULL
////			__raw_writel(SPI1_SlaveData[TransmittedCnt++], hw->regs + REG_TX);
////			if (TransmittedCnt >= SPI1_SlaveDataLen) {
////				nuc980_disable_txth_int(hw);
////				InTransmitted = 0;
////				TransmittedCnt = 0;
////				break;
////			}
////		}
////	}
////	if (status & SSINAIF) {
////	    slave_done_state = 1;
////	    Transmit_done = 1;
////		wake_up_interruptible(&slave_done);
////		nuc980_disable_rxth_int(hw);
////		/* Check if transmition complete */
//////		printk("SPI1_irq= SSINAIF);
//////		if (InTransmitted == 1) {
////////			int i;
////////			printk("Master pull SS high, but slave TX doesn't complete!\n");
////////			__raw_writel(__raw_readl(hw->regs + REG_FIFOCTL) | 0x3, hw->regs + REG_FIFOCTL); //TXRX reset
////////			while (__raw_readl(hw->regs + REG_STATUS) & (1<<23)); //TXRXRST
////////			nuc980_enable_rxth_int(hw);
////////			InTransmitted = 0;
////////			TransmittedCnt = 0;
////////			for (i = 0; i < SPI1_SlaveDataLen; i++)
////////				SPI1_SlaveData[i] = 0;
//////        Transmit_done = 1;
//////        InTransmitted = 0;
//////        nuc980_disable_rxth_int(hw);
////////        wake_up_interruptible(&slave_done);
//////		}
////	}
//
//	return IRQ_HANDLED;
//}


static void nuc980_init_spi(struct nuc980_spi *hw)
{
    ENTRY();
	clk_prepare(hw->clk);
	clk_enable(hw->clk);

	spin_lock_init(&hw->lock);

	nuc980_tx_edge(hw, hw->pdata->txneg);
	nuc980_rx_edge(hw, hw->pdata->rxneg);
	nuc980_send_first(hw, hw->pdata->lsb);
	nuc980_set_sleep(hw, hw->pdata->sleep);
	nuc980_spi1_setup_txbitlen(hw, hw->pdata->txbitlen);
	nuc980_set_clock_polarity(hw, hw->pdata->clkpol);
	nuc980_set_divider(hw);
	nuc980_enable_slave(hw);
//	nuc980_enable_rxth_int(hw);
//	nuc980_enable_ssinact_int(hw);
	LEAVE();
}

#ifdef CONFIG_OF
static struct nuc980_spi_info *nuc980_spi1_parse_dt(struct device *dev) {
	struct nuc980_spi_info *sci;
	u32 temp;

	sci = devm_kzalloc(dev, sizeof(*sci), GFP_KERNEL);
	if (!sci) {
		dev_err(dev, "memory allocation for spi_info failed\n");
		return ERR_PTR(-ENOMEM);
	}

	if (of_property_read_u32(dev->of_node, "num_cs", &temp)) {
		dev_warn(dev, "can't get num_cs from dt\n");
		sci->num_cs = 2;
	} else {
		sci->num_cs = temp;
	}

	if (of_property_read_u32(dev->of_node, "lsb", &temp)) {
		dev_warn(dev, "can't get lsb from dt\n");
		sci->lsb = 0;
	} else {
		sci->lsb = temp;
	}

	if (of_property_read_u32(dev->of_node, "txneg", &temp)) {
		dev_warn(dev, "can't get txneg from dt\n");
		sci->txneg = 1;
	} else {
		sci->txneg = temp;
	}

	if (of_property_read_u32(dev->of_node, "clkpol", &temp)) {
		dev_warn(dev, "can't get clkpol from dt\n");
		sci->clkpol = 0;
	} else {
		sci->clkpol = temp;
	}

	if (of_property_read_u32(dev->of_node, "rxneg", &temp)) {
		dev_warn(dev, "can't get rxneg from dt\n");
		sci->rxneg = 0;
	} else {
		sci->rxneg = temp;
	}

	if (of_property_read_u32(dev->of_node, "divider", &temp)) {
		dev_warn(dev, "can't get divider from dt\n");
		sci->divider = 4;
	} else {
		sci->divider = temp;
	}

	if (of_property_read_u32(dev->of_node, "sleep", &temp)) {
		dev_warn(dev, "can't get sleep from dt\n");
		sci->sleep = 0;
	} else {
		sci->sleep = temp;
	}

	if (of_property_read_u32(dev->of_node, "txbitlen", &temp)) {
		dev_warn(dev, "can't get txbitlen from dt\n");
		sci->txbitlen = 8;
	} else {
		sci->txbitlen = temp;
	}

	if (of_property_read_u32(dev->of_node, "bus_num", &temp)) {
		dev_warn(dev, "can't get bus_num from dt\n");
		sci->bus_num = 0;
	} else {
		sci->bus_num = temp;
	}

	return sci;
}
#else
static struct nuc980_spi_info *nuc980_spi1_parse_dt(struct device *dev) {
	return dev->platform_data;
}
#endif

/* In Thread, only prepare data and enable TXTHIEN.
   The data will be transmitted in IRQ
 */
//static int SPI1_Slave_Thread_TXRX(struct nuc980_spi *hw)
//{
//	unsigned int rx;
//	unsigned long flags;
//     int i =0;
//
//	while(1) {
//
//		wait_event_interruptible(slave_done, (slave_done_state != 0));
//		slave_done_state = 0;
//		rx = __raw_readl(hw->regs + REG_RX);
//		printk("Receive [0x%08x] \n", rx);
////
////        if(TransmittedCnt < 188)
////        {
////
////            SPI1_SlaveData[TransmittedCnt++] = rx;
////        }
////        else
////        {
////            nuc980_disable_rxth_int(hw);
////            Transmit_done = 1;
////        }
//
////printk("TransmittedCnt = %d]\n", TransmittedCnt);
//        if(Transmit_done == 1)
//        {
////            printk("TransmittedCnt = %d]\n", TransmittedCnt);
////            for( i=0;i < TransmittedCnt ;++i)
////            {
////                printk("[0x%02x]\t ", SPI1_SlaveData[i]);
////
////            }
//
//
//        printk(" \n");
//        TransmittedCnt = 0;
//        InTransmitted = 0;
//		slave_done_state = 0;
//		nuc980_enable_rxth_int(hw);
//        }
////		switch (rx) {
////		case 0xbb:
////			for (i = 0; i < SPI1_SlaveDataLen; i++)
////				SPI1_SlaveData[i] = i;
////
////			nuc980_enable_txth_int(hw);
////			break;
////		default:
////			break;
////		}
//
//
//	}
//
//	return 0;
//}
 struct ts_spi_slave	*t  ;
 struct nuc980_spi *hw;

static int TS_DTMB_SPI_open(struct inode *inode, struct file *file)
{
    printk(KERN_EMERG "TS_DTMB_SPI open\n");
    return 0;
}

static int TS_DTMB_SPI_release(struct inode *inode, struct file *file)
{
    printk(KERN_EMERG "TS_DTMB_SPI release\n");
    return 0;
}

static int TS_DTMB_SPI_read(struct file *filp, char __user *buf, size_t count, loff_t *fops)
{
    //printk(KERN_EMERG "TS_DTMB_SPI read\n");

    nuc980_spi1_txrx(hw, t);

    if(copy_to_user(buf,t->rx_buf,count))
	{
		return -EFAULT;
	}

    return 0;
}
/*ops结构体，存储相关的操作函数*/
static struct file_operations TS_DTMB_SPI_ops = {
    .owner					= THIS_MODULE,
    .open					= TS_DTMB_SPI_open,
    .release				= TS_DTMB_SPI_release,
    .read                   = TS_DTMB_SPI_read,

};


static int nuc980_spi1_slave_probe(struct platform_device *pdev)
{
    struct nuc980_ip_dma *pdma=&dma;
	dma_cap_mask_t mask;

	struct spi_master *master;
	int err = 0;
	struct pinctrl *p;

    /* Zero out the capability mask then initialize it for a slave channel that is
	 * private.
	 */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);
	dma_cap_set(DMA_PRIVATE, mask);

	/* Request the DMA channel from the DMA engine and then use the device from
	 * the channel for the proxy channel also.
	 */
	pdma->chan_rx = dma_request_channel(mask, NULL, NULL);
	if (!pdma->chan_rx) {
		printk("RX DMA channel request error\n");
		return -1;
	}
	pdma->chan_rx->private=(void *)1;
	printk("RX %s: %s module removed\n",__func__, dma_chan_name(pdma->chan_rx));

//	pdma->chan_tx = dma_request_channel(mask, NULL, NULL);
//	if (!pdma->chan_tx) {
//		printk("TX DMA channel request error\n");
//		return -1;
//	}
//	pdma->chan_tx->private=(void *)1;
//	printk("TX %s: %s module removed\n",__func__, dma_chan_name(pdma->chan_tx));

	master = spi_alloc_master(&pdev->dev, sizeof(struct nuc980_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	hw->master = spi_master_get(master);
	hw->pdata = nuc980_spi1_parse_dt(&pdev->dev);
	hw->dev = &pdev->dev;
    hw->curdev = to_spi_device(hw->dev);

	if (hw->pdata == NULL) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_pdata;
	}

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);
	master->mode_bits          = (SPI_MODE_0 | SPI_LSB_FIRST |  SPI_CPHA | SPI_CPOL);//(SPI_MODE_0 | SPI_CS_HIGH | SPI_LSB_FIRST | SPI_CPHA | SPI_CPOL);
	master->dev.of_node        = pdev->dev.of_node;
	master->num_chipselect     = hw->pdata->num_cs;
	master->bus_num            = hw->pdata->bus_num;
	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = nuc980_spi1_setupxfer;
	hw->bitbang.chipselect     = nuc980_spi1_chipsel;
	hw->bitbang.master->setup  = nuc980_spi1_setup;

//    nuc980_spi1_setupxfer(hw->curdev);
//    nuc980_spi1_chipsel(hw->curdev);
//    nuc980_spi1_setup(hw->curdev);

    hw->pdata->clkpol = 1;
    hw->pdata->txneg = 1;
    hw->pdata->rxneg = 0;

	//spimode = spi->mode & 0xff; //remove dual/quad bit

	hw->pdata->lsb = 1;


	hw->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (hw->res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_pdata;
	}

#if defined(CONFIG_OF)
	hw->regs = devm_ioremap_resource(&pdev->dev, hw->res);
#else
	hw->ioarea = request_mem_region(hw->res->start,
	                                resource_size(hw->res), pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_pdata;
	}

	hw->regs = ioremap(hw->res->start, resource_size(hw->res));
	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_iomap;
	}
#endif

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_irq;
	}

//	err = request_irq(hw->irq, nuc980_spi1_irq, 0, pdev->name, hw);
//	if (err) {
//		dev_err(&pdev->dev, "Cannot claim IRQ\n");
//		goto err_irq;
//	}

	hw->clk = clk_get(NULL, "spi1_eclk");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_clk;
	}
	clk_prepare(hw->clk);
	clk_enable(hw->clk);

	hw->clk = clk_get(NULL, "spi1");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_clk;
	}

	clk_prepare(hw->clk);
	clk_enable(hw->clk);

#if defined(CONFIG_OF)
	p = devm_pinctrl_get_select_default(&pdev->dev);
#else
	/*
#if defined(CONFIG_SPI_NUC980_P0_NORMAL) && !defined(CONFIG_SPI_NUC980_P0_SS1)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1");
#elif defined(CONFIG_SPI_NUC980_P0_NORMAL) && defined(CONFIG_SPI_NUC980_P0_SS1_PB0)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1-ss1-PB");
#elif defined(CONFIG_SPI_NUC980_P0_NORMAL) && defined(CONFIG_SPI_NUC980_P0_SS1_PH12)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1-ss1-PH");
#elif defined(CONFIG_SPI_NUC980_P0_QUAD) && !defined(CONFIG_SPI_NUC980_P0_SS1)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1-quad");
#elif defined(CONFIG_SPI_NUC980_P0_QUAD) && defined(CONFIG_SPI_NUC980_P0_SS1_PB0)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1-quad-ss1-PB");
#elif defined(CONFIG_SPI_NUC980_P0_QUAD) && defined(CONFIG_SPI_NUC980_P0_SS1_PH12)
		p = devm_pinctrl_get_select(&pdev->dev, "spi1-quad-ss1-PB");
#endif
	*/
	/*TODO : use pinctrl*/
	//__raw_writel((__raw_readl(REG_MFP_GPB_H) & ~0xFFFF0) | 0x55550, REG_MFP_GPB_H);
	__raw_writel((__raw_readl(REG_MFP_GPG_H) & ~0x0FFFF000) | 0x02222000, REG_MFP_GPG_H);//yanxuewen modify
#endif
	if(IS_ERR(p)) {
		dev_err(&pdev->dev, "unable to reserve spi pin by mode\n");
		err = PTR_ERR(p);
		goto err_register;
	}
//    hw->pdata->divider = 0;
//    hw->pdata->txbitlen = 32;
	nuc980_init_spi(hw);

	__raw_writel(__raw_readl(hw->regs + REG_FIFOCTL) | 0x3, hw->regs + REG_FIFOCTL); //CWWeng : RXRST & TXRST
	while (__raw_readl(hw->regs + REG_STATUS) & (1<<23)); //TXRXRST

	__raw_writel(__raw_readl(hw->regs + REG_CTL) | SPIEN, hw->regs + REG_CTL); /* enable SPI */
	while ((__raw_readl(hw->regs + REG_STATUS) & (1<<15)) == 0); //SPIENSTS



    printk("SPI1_CTL = [0x%08x] \n", __raw_readl(hw->regs + REG_CTL));
    printk("SPI1_CLKDIV = [0x%08x] \n", __raw_readl(hw->regs + REG_CLKDIV));
    printk("SPI1_SSCTL = [0x%08x] \n", __raw_readl(hw->regs + REG_SSCTL));
    printk("SPI1_PDMACTL = [0x%08x] \n", __raw_readl(hw->regs + REG_PDMACTL));
    printk("SPI1_FIFOCTL = [0x%08x] \n", __raw_readl(hw->regs + REG_FIFOCTL));
    printk("SPI1_STATUS = [0x%08x] \n", __raw_readl(hw->regs + REG_STATUS));
    printk("SPI1_TX = [0x%08x] \n", __raw_readl(hw->regs + REG_TX));
    printk("SPI1_RX = [0x%08x] \n", __raw_readl(hw->regs + REG_RX));

    printk("CLK_HCLKEN=0x%08x\n",__raw_readl(CLK_BA + 0x010));
    printk("REG_CLK_DIV0=0x%08x\n",__raw_readl(CLK_BA + 0x020));
    printk("REG_CLK_DIV2=0x%08x\n",__raw_readl(CLK_BA + 0x028));
    printk("REG_CLK_APLLCON=0x%08x\n",__raw_readl(CLK_BA + 0x060));
    printk("REG_CLK_UPLLCON=0x%08x\n",__raw_readl(CLK_BA + 0x064));

//    kthread_run(SPI1_Slave_Thread_TXRX, hw, "SPI1_SLAVE_THread_TXRX");

    t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (!t)
	{
	    printk("open/ENOMEM/t\n");
	    return -ENOMEM;
	}

    if (!t->rx_buf) {
		t->rx_buf = kmalloc(188, GFP_KERNEL);
		printk("malloc rx_buf success.\n");
		if (!t->rx_buf) {
			printk("open/ENOMEM\n");
		}
    }
    else
    {
      printk("t->rxbuf is not null\n");
    }

    t->len		= 188;

    ts_dtmb_data_dev = kzalloc(sizeof(struct TS_DTMB_SPI_DATA), GFP_KERNEL);
	if (!ts_dtmb_data_dev)
		return -ENOMEM;

    mutex_init(&ts_dtmb_data_dev->lock);

    ts_dtmb_data_dev->ts_dtmb_misc.fops = &TS_DTMB_SPI_ops;
    ts_dtmb_data_dev->ts_dtmb_misc.minor = 199;
    ts_dtmb_data_dev->ts_dtmb_misc.name = "TS_STMB_SPI";
    misc_register(&ts_dtmb_data_dev->ts_dtmb_misc);


//    while(1)
//    {
//        nuc980_spi1_txrx(hw, t);
//    }

//    err = spi_bitbang_start(&hw->bitbang);
//	if (err) {
//		dev_err(&pdev->dev, "Failed to register SPI master\n");
//		goto err_register;
//	}

	return 0;

err_register:
	clk_disable(hw->clk);
	clk_put(hw->clk);
err_clk:
	free_irq(hw->irq, hw);
err_irq:
	iounmap(hw->regs);
#ifndef CONFIG_OF
err_iomap:
	release_mem_region(hw->res->start, resource_size(hw->res));
	kfree(hw->ioarea);
#endif
err_pdata:
	spi_master_put(hw->master);

err_nomem:
	return err;
}

static int nuc980_spi1_slave_remove(struct platform_device *dev)
{
	struct nuc980_spi *hw = platform_get_drvdata(dev);

	free_irq(hw->irq, hw);
	platform_set_drvdata(dev, NULL);
	spi_bitbang_stop(&hw->bitbang);

	clk_disable(hw->clk);
	clk_put(hw->clk);

	iounmap(hw->regs);

#ifndef CONFIG_OF
	release_mem_region(hw->res->start, resource_size(hw->res));
	kfree(hw->ioarea);
#else
	kfree(hw->pdata);
#endif

	spi_master_put(hw->master);

	misc_deregister(&ts_dtmb_data_dev->ts_dtmb_misc);
	return 0;
}

#ifdef CONFIG_PM
static int nuc980_spi1_suspend(struct device *dev)
{
	struct nuc980_spi *hw = dev_get_drvdata(dev);

	while(__raw_readl(hw->regs + REG_CTL) & 0x1)
		msleep(1);

	// disable interrupt
	__raw_writel((__raw_readl(hw->regs + REG_CTL) & ~0x20000), hw->regs + REG_CTL);

	return 0;
}

static int nuc980_spi1_resume(struct device *dev)
{
	struct nuc980_spi *hw = dev_get_drvdata(dev);

	// enable interrupt
	__raw_writel((__raw_readl(hw->regs + REG_CTL) | 0x20000), hw->regs + REG_CTL);

	return 0;
}

static const struct dev_pm_ops nuc980_spi1_pmops = {
	.suspend    = nuc980_spi1_suspend,
	.resume     = nuc980_spi1_resume,
};

#define NUC980_SPI1_PMOPS (&nuc980_spi1_pmops)

#else
#define NUC980_SPI1_PMOPS NULL
#endif

#if defined(CONFIG_OF)
static const struct of_device_id nuc980_spi1_slave_of_match[] = {
	{   .compatible = "nuvoton,nuc980-spi1-slave" },
	{	},
};
MODULE_DEVICE_TABLE(of, nuc980_spi1_of_match);
#endif

static struct platform_driver nuc980_spi1_slave_driver = {
	.probe      = nuc980_spi1_slave_probe,
	.remove     = nuc980_spi1_slave_remove,
	.driver     = {
		.name   = "nuc980-spi1-slave",
		.owner  = THIS_MODULE,
		.pm	= NUC980_SPI1_PMOPS,
#if defined(CONFIG_OF)
		.of_match_table = of_match_ptr(nuc980_spi1_slave_of_match),
#endif
	},
};
module_platform_driver(nuc980_spi1_slave_driver);

MODULE_DESCRIPTION("nuc980 spi1 slave driver!");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:nuc980-spi1-slave");


