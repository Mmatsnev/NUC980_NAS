/*
 * Copyright (c) 2018 Nuvoton Technology Corporation.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>

#include "nuc980_cap.h"

//#define DISALBE_READ_ID

static struct nuvoton_vin_sensor cap0_mxl241;

struct OV_RegValue {
	__u16   uRegAddr;
	__u8    uValue;
};

#define _REG_TABLE_SIZE(nTableName) sizeof(nTableName)/sizeof(struct OV_RegValue)


/************  I2C  *****************/
static struct i2c_client *save_client0;
static char sensor0_inited = 0;

static int sensor0_read_mxl241(u16 RegAddr,u16 *DataPtr)
{
	int ret;
	/* We have 16-bit i2c addresses - care for endianess */
	unsigned char Read_Cmd[4];
	unsigned char Read_data[2];

	/* read step 1. accroding to mxl5007 driver API user guide. */
	Read_Cmd[0] = 0xFF;
	Read_Cmd[1] = 0xFB;
	Read_Cmd[2] = (RegAddr>>8)&0xFF;//ADD_HIGH
	Read_Cmd[3] = RegAddr&0xFF;//ADD_LOW

	ret = i2c_master_send(save_client0, Read_Cmd, 4);
	if (ret < 4) {
		dev_err(&save_client0->dev, "%s: i2c read error, reg: 0x%x\n",
		        __func__, RegAddr);
		return ret < 0 ? ret : -EIO;
	}

	ret = i2c_master_recv(save_client0, Read_data, 2);
	if (ret < 2) {
		dev_err(&save_client0->dev, "%s: i2c read error, reg: 0x%x\n",__func__, RegAddr);
		return ret < 0 ? ret : -EIO;
	}
	*DataPtr=(Read_data[0]<<8)|Read_data[1];
	return 0;
}

static int sensor0_write_mxl241(u16 RegAddr, u16 RegData)
{
	int ret;
	unsigned char Write_Cmd[4];

	Write_Cmd[0] = (RegAddr>>8)&0xFF;
	Write_Cmd[1] = (RegAddr)&0xFF;
	Write_Cmd[2] = (RegData>>8)&0xFF;
   	Write_Cmd[3] = (RegData)&0xFF;

	ret = i2c_master_send(save_client0, Write_Cmd, 4);
	if (ret < 4) {
		dev_err(&save_client0->dev, "%s: i2c write error, reg: 0x%x\n",
		        __func__, RegAddr);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

static int sensor0_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	ENTRY();
    printk("============>sensor0 id=%d, addr %x\n",i2c_adapter_id(client->adapter),client->addr);
	if(i2c_adapter_id(client->adapter) != cap0_mxl241.i2c_id || client->addr != 0x60)
	{
	return -ENODEV;
	}
	sensor0_inited = 1;
	client->flags = I2C_CLIENT_SCCB;
	save_client0 = client;
	LEAVE();
	return 0;
}
static int sensor0_remove(struct i2c_client *client)
{
	ENTRY();
	LEAVE();
	return 0;
}

static int cap0_mxl241_init(struct nuvoton_vin_device* cam)
{
	int err = 0;
	ENTRY();
	LEAVE();
	return err;
}

static struct nuvoton_vin_sensor cap0_mxl241 = {
	.i2c_id = 4,
	.name = "cap0_mxl241",
	.init = &cap0_mxl241_init,
	.infmtord = (INORD_YUYV | INFMT_YCbCr | INTYPE_CCIR601),
	.polarity = (VSP_LO | HSP_LO | PCLKP_HI),
	.cropstart = ( 0 | 0<<16 ), /*( Vertical | Horizontal<<16 ) */
	.cropcap = {
		.bounds = {
			.left = 0,
			.top = 0,
			.width = 640,
			.height = 480,
		},
		.defrect = {
			.left = 0,
			.top = 0,
			.width = 800,
			.height = 480,
		},
	},
	.pix_format  = {
		.width = 640,
		.height = 480,
		.pixelformat = V4L2_PIX_FMT_YUYV,
		.priv = 16,
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
};

int nuvoton_vin0_probe_mxl241(struct nuvoton_vin_device* cam)
{
	int i,ret = 0;
#ifndef DISALBE_READ_ID
	__u8 SensorID[2];
#endif
	struct OV_RegValue *psRegValue;

	ENTRY();

	nuvoton_vin0_attach_sensor(cam, &cap0_mxl241);

	// if i2c module isn't loaded at this time
	if(!sensor0_inited)
		return -1;
printk("has run to mxl241 line 190.\n");

		printk("Stop Tune\n");
		ret = sensor0_write_mxl241(0x0013, 0);
		udelay(200);
		printk("Power down Tuner\n");
		ret = sensor0_write_mxl241(0x0001, 0);
		udelay(200);
		ret = sensor0_write_mxl241(0xFFFF, 0);


	//----------Read sensor id-------------------------------------
//#ifndef DISALBE_READ_ID
//	sensor0_read_nt99141(0x3000,&SensorID[0]);  /* Chip_Version_H 0x14 */
//	sensor0_read_nt99141(0x3001,&SensorID[1]);  /* Chip_Version_L 0x10 */
//	printk("\nSensor Chip_Version_H = 0x%02x(0x14) Chip_Version_L = 0x%02x(0x10)\n", SensorID[0],SensorID[1]);
//#endif
	//-------------------------------------------------------------
	printk("\n");
	if(ret>=0)
		printk("driver i2c initial done\n");
	else
		printk("driver i2c initial fail\n");
	LEAVE();
	return ret;
}

static const struct i2c_device_id sensor0_id[] = {
	{ "cap0_mxl241", 0 },
};
MODULE_DEVICE_TABLE(i2c, sensor0_id);

#ifdef CONFIG_USE_OF
static struct of_device_id cap0_mxl241_match_table[] = {
	{ .compatible = "nuvoton,cap0-mxl241",},
	{},
};
#else
#define cap0_mxl241_match_table NULL
#endif


static struct i2c_driver sensor0_i2c_driver = {
	.driver = {
		.name = "cap0_mxl241",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cap0_mxl241_match_table),
	},
	.probe    = sensor0_probe,
	.remove   = sensor0_remove,
	.id_table = sensor0_id,
};
module_i2c_driver(sensor0_i2c_driver);
