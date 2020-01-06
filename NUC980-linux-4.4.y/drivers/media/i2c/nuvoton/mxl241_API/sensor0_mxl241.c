/*
 * Copyright (c) 2018 Nuvoton Technology Corporation.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */


#include "MxL241.h"


//#define DISALBE_READ_ID

static struct nuvoton_vin_sensor cap0_mxl241;

//struct OV_RegValue {
//	__u16   uRegAddr;
//	__u8    uValue;
//};

MXL_RESET_CFG_T               MxL241SF_Reset;
MXL_OVERWRITE_DEFAULT_CFG_T   MxL241SF_OverwriteDefault;
MXL_DEV_INFO_T                MxL241SF_DeviceInfo;
MXL_XTAL_CFG_T                MxL241SF_XtalSetting;
MXL_DEMOD_LOCK_STATUS_T       MxL241SF_LockStatus;
MXL_SYMBOL_RATE_T             MxL241SF_SymbolRate;
MXL_AGC_T                     MxL241SF_Agc;
MXL_AGC_CTRL_SPEED_T          MxL241SF_Agc_Speed;
MXL_TOP_MASTER_CFG_T          MxL241SF_PowerUpTuner;
MXL_ANNEX_CFG_T               MxL241SF_AnnexType;
MXL_PWR_MODE_CFG_T            MxL241SF_PwrMode;
MXL_MPEG_OUT_CFG_T            MxL241SF_MpegOut;
MXL_CHAN_DEPENDENT_T          MxL241SF_ChanDependent;
MXL_ADCIQFLIP_CFG_T           MxL241SF_AdcIpFlip;
MXL_INTR_CFG_T                MxL241SF_INTER;
MXL_RF_TUNE_CFG_T             MxL241SF_ChanTune;

/*****************************************************************************
* UINT8 mxl241_Init(void)
* Description: mxl241 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: UINT8
*****************************************************************************/
static UINT8 Mxl241_Init(void)
{
//UINT16 Data,i=0;
// UINT32 loop;
//#ifdef __MXL_INTEGER_CALC_STATISTICS__
//  SINT64 tmpRealNum1;
//  SINT64 tmpRealNum2;
//#else
//  REAL32 tmpRealNum1;
//  REAL32 tmpRealNum2;
//  #endif
  MXL_STATUS status;

  /***** ³õÊ¼»¯ MxL2401 *************/

//  GPIO_InitTypeDef GPIO_InitStruct = {0};  /*Configure LOCK GPIO pins : PC10*/
//  GPIO_InitStruct.Pin = MxL_INT_GPIO_PIN;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//  HAL_GPIO_Init(MxL_INT_GPIO, &GPIO_InitStruct);

  // 1. Do SW Reset
  MxL241SF_Reset.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  status = MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&MxL241SF_Reset);

  // 2. Overwrite default
  MxL241SF_OverwriteDefault.I2cSlaveAddr = MXL241_CHIP_ADRRESS;// 96;
  status = MxLWare_API_ConfigDevice(MXL_DEV_OVERWRITE_DEFAULT_CFG, (void*)&MxL241SF_OverwriteDefault);

  // 3. Read Back Device id and version
  MxL241SF_DeviceInfo.I2cSlaveAddr = MXL241_CHIP_ADRRESS;
  status = MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, (void*)&MxL241SF_DeviceInfo);
  if (status == MXL_TRUE)
  {
    //libc_printf("MxL241SF : DevId = 0x%x, Version = 0x%x \n\n", MxL241SF_DeviceInfo. DevId, MxL241SF_DeviceInfo. DevVer);
  }

  // 4. XTAL and Clock out setting
  MxL241SF_XtalSetting.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_XtalSetting.XtalEnable = MXL_ENABLE;
  MxL241SF_XtalSetting.DigXtalFreq = XTAL_48MHz;//XTAL_48MHz;
  MxL241SF_XtalSetting.XtalBiasCurrent = 1;
  MxL241SF_XtalSetting.XtalCap = 10; // 10pF
  MxL241SF_XtalSetting.XtalClkOutEnable = MXL_DISABLE;//¹Ø±Õclkout
  MxL241SF_XtalSetting.XtalClkOutGain =  0x0;
  MxL241SF_XtalSetting.LoopThruEnable = MXL_DISABLE;//¹Ø±Õloop

  status = MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, (void*)&MxL241SF_XtalSetting);

  // 5. AGC configuration
  MxL241SF_Agc.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_Agc.FreezeAgcGainWord = MXL_NO_FREEZE;
  MxLWare_API_ConfigTuner(MXL_TUNER_AGC_SETTINGS_CFG, (void*)&MxL241SF_Agc);
  // AGC LOCK SPEED configuration
  MxL241SF_Agc_Speed.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_Agc_Speed.AgcSpeedMode = NORMAL_AGC_LOCK;
  status = MxLWare_API_ConfigTuner(MXL_TUNER_AGC_LOCK_SPEED_CFG, (void*)&MxL241SF_Agc_Speed);

  // 6. Power Up Tuner
  MxL241SF_PowerUpTuner.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 96;
  MxL241SF_PowerUpTuner.TopMasterEnable = MXL_ENABLE;
  status = MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, (void*)&MxL241SF_PowerUpTuner);

  delay_ms(1);

  // 7. MPEG out setting
  MxL241SF_MpegOut.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_MpegOut.SerialOrPar = MPEG_DATA_PARALLEL;
  MxL241SF_MpegOut.MpegValidPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;//
  MxL241SF_MpegOut.MpegSyncPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkFreq = MPEG_CLK_7_125MHz;
  MxL241SF_MpegOut.MpegClkSource = MPEG_CLK_INTERNAL;
 // MxL241SF_MpegOut.MpegSyncPulseWidth=MPEG_SYNC_WIDTH_BIT;
  MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&MxL241SF_MpegOut);

  delay_ms(1);

  //Mxl241_tune_channel(251000000,6875,64);

  return 0;
}
SINT32 dbm;

/*****************************************************************************
* UINT8 Mxl241_GetSNR(UINT16 qam)
*
* This function returns an approximate estimation of the SNR from the NIM
*  The Eb No is calculated using the SNR from the NIM, using the formula:
*     Eb ~     13312- M_SNR_H
*     -- =    ----------------  dB.
*     NO           683
*
* Return Value: UINT8 SNR
*****************************************************************************/
static UINT8 Mxl241_GetSNR(void)
{
  MXL_STATUS status;
  MXL_DEMOD_SNR_INFO_T  MxL241_SnrPtr;
  MxL241_SnrPtr.I2cSlaveAddr = MXL241_CHIP_ADRRESS;
  status = MxLWare_API_GetDemodStatus(MXL_DEMOD_SNR_REQ, (void*)&MxL241_SnrPtr);
  if(status ==  MXL_FALSE)
    return 0;
  else
    return (UINT8)(MxL241_SnrPtr.SNR);
}



/***********************************************
   Get lock status
   returned 1 when locked;0 when unlocked
************************************************/
static UINT8 MxL241_GetLock(void)
{
    MXL_STATUS status;
  //  QAM_Lock
    MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
    status = MxLWare_API_GetDemodStatus(MXL_DEMOD_QAM_LOCK_REQ, (void*)&MxL241SF_LockStatus);

    delay_ms(10);

  if((status==1) || (MxL241SF_LockStatus.Status != MXL_LOCKED))
      return 0;
   //  FEC Lock
    MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;

    status = MxLWare_API_GetDemodStatus(MXL_DEMOD_FEC_LOCK_REQ, (void*)&MxL241SF_LockStatus);

  if((status==1) || (MxL241SF_LockStatus.Status != MXL_LOCKED))
      return 0;

  // MXL_DEMOD_MPEG_LOCK

   MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
   status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&MxL241SF_LockStatus);

   if((status==1) || (MxL241SF_LockStatus.Status != MXL_LOCKED))
      return 0;
  return 1;
//  if(HAL_GPIO_ReadPin(MxL_INT_GPIO, MxL_INT_GPIO_PIN) == GPIO_PIN_RESET)
//    return 0;
//  else
//    return 1;
}


/***********************************************
   Start_0r_Abort the mxl241
   flag:1 for start;0 for abort
************************************************/

static void MxL241_Start_Abort(UINT16 flag)
{

Ctrl_WriteRegister(MXL241_CHIP_ADRRESS, 0x0013, flag);

}
/***********************************************
   FUNCTION NAME : Mxl241_Get_RxPower - MXL_TUNER_RF_RX_PWR_REQ
--| DESCRIPTION   : This function returns RF input power in dBm, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
   Return Value: SINT32 dbm
************************************************/
static SINT32 Mxl241_Get_RxPower(void)
{
  MXL_STATUS status;
  MXL_TUNER_RX_PWR_T  MxL241_Rx_PWR_Ptr;
  MxL241_Rx_PWR_Ptr.I2cSlaveAddr = MXL241_CHIP_ADRRESS;
  status = MxLWare_API_GetTunerStatus(MXL_TUNER_RF_RX_PWR_REQ, (void*)&MxL241_Rx_PWR_Ptr);
  if(status ==  MXL_FALSE)
    return 0;
  else
    return (SINT32)(MxL241_Rx_PWR_Ptr.RxPwr);

}
static unsigned char Mxl241_tune_channel(signed long tune_channel_FchanHz, unsigned long tuner_channel_SymbolRate, unsigned short tune_channel_QAM)
{

  MXL_STATUS status;
  UINT8 loop;

   // Symbol rate
  MxL241SF_SymbolRate.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_SymbolRate.SymbolType = SYM_TYPE_USER_DEFINED_J83A;//SYM_TYPE_J83A;//;
  MxL241SF_SymbolRate.SymbolRate =((REAL32)(tuner_channel_SymbolRate)/1000);//6.89;//(REAL32)(((REAL32)(sym))/100.0);//(REAL32)6.952;
  status = MxLWare_API_ConfigDemod(MXL_DEMOD_SYMBOL_RATE_CFG, (void*)&MxL241SF_SymbolRate);

  delay_ms(1);

  // 9. Config Annex Type
  MxL241SF_AnnexType.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  MxL241SF_AnnexType.AnnexType = ANNEX_A;
  MxL241SF_AnnexType.AutoDetectQamType = MXL_DISABLE;//MXL_ENABLE;
  MxL241SF_AnnexType.AutoDetectMode = MXL_ENABLE;
  //MxL241SF_AnnexType.QamType=MXL_QAM256;
  switch(tune_channel_QAM)
	{
	 case 16: MxL241SF_AnnexType.QamType=MXL_QAM16;break;
	 case 32: MxL241SF_AnnexType.QamType=MXL_QAM32;break;
	 case 64: MxL241SF_AnnexType.QamType=MXL_QAM64;break;
	 case 128: MxL241SF_AnnexType.QamType=MXL_QAM128;break;
	 case 256: MxL241SF_AnnexType.QamType=MXL_QAM256;break;
	 default: MxL241SF_AnnexType.QamType=MXL_QAM64; break;
	}
  status = MxLWare_API_ConfigDemod(MXL_DEMOD_ANNEX_QAM_TYPE_CFG, (void*)&MxL241SF_AnnexType);

  // 10. Do MiscSettings if needed
  // 11. Tune RF with channel frequency and bandwidth
  MxL241SF_ChanTune.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_ChanTune.BandWidth = 8;//8;                     // 8MHz
  MxL241SF_ChanTune.Frequency = tune_channel_FchanHz;         // UINT:HZ
  status = MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_TUNE_CFG, (void*)&MxL241SF_ChanTune );

  delay_ms(30);

  // 12. Enable I/Q path flip
  MxL241SF_AdcIpFlip.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 96;
  MxL241SF_AdcIpFlip.AdcIqFlip = MXL_ENABLE;
  status = MxLWare_API_ConfigDemod(MXL_DEMOD_ADC_IQ_FLIP_CFG, (void*)&MxL241SF_AdcIpFlip);

  /* Please do not forget to put time delay at least 25ms */
    delay_ms(30);


  // 13. Channel dependent setting
  MxL241SF_ChanDependent.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_ChanDependent.ChanDependentCfg = MXL_ENABLE;
  status = MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG, (void*)&MxL241SF_ChanDependent);

  /* Please do not forget to put time delay at least 300ms */
  //delay_ms(300);
  // 14. Wait QAM_LOCK
  loop = 0;
  while (loop < 10) //  This number is experimental to monitor statistics.
  	{
    	  MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;

    	  status = MxLWare_API_GetDemodStatus(MXL_DEMOD_QAM_LOCK_REQ, (void*)&MxL241SF_LockStatus);
          if (MxL241SF_LockStatus.Status== 0)break;

  	  delay_ms(10);
    	  loop++;
 	 }
  if(status==1)
      return 0;
   // 15. Wait FEC Lock
  loop = 0;
  while (loop < 10) //  This number is experimental to monitor statistics.
  	{
    	  MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;

    	  status = MxLWare_API_GetDemodStatus(MXL_DEMOD_FEC_LOCK_REQ, (void*)&MxL241SF_LockStatus);
          if (MxL241SF_LockStatus.Status== 0) break;

  	  delay_ms(10);
          loop++;
 	 }
  if(status==1)
      return 0;

  //16. MXL_DEMOD_MPEG_LOCK
  loop = 0;
  while (loop < 10) //  This number is experimental to monitor statistics.
        {
    	  MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
          status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&MxL241SF_LockStatus);
          if (MxL241SF_LockStatus.Status== 0)break;

	  delay_ms(10); // This delay is also experimental, it can be anything.
    	  loop++;
  	}
   if(status==1)
      return 0;
//  MxL241SF_PwrMode.I2cSlaveAddr = MXL241_CHIP_ADRRESS;
//  MxL241SF_PwrMode.PowerMode = STANDBY_ON;
//  MxLWare_API_ConfigDevice(MXL_DEV_POWER_MODE_CFG, (void*)&MxL241SF_PwrMode);
  //dbm = Mxl241_Get_RxPower();
  delay_ms(1000);
  return MxL241_GetLock();
}


/************  I2C  *****************/
struct i2c_client *save_client0;
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
	.infmtord = (INORD_YUYV | INFMT_RGB565 | INTYPE_CCIR601),
	.polarity = (VSP_LO | HSP_HI | PCLKP_LO),
	.cropstart = ( 0 | 0<<16 ), /*( Vertical | Horizontal<<16 ) */
	.cropcap = {
		.bounds = {
			.left = 0,
			.top = 0,
			.width = 320,//.width = 640,
			.height = 240,//.height = 480,
		},
		.defrect = {
			.left = 0,
			.top = 0,
			.width = 400,//800,
			.height = 240,//480,
		},
	},
	.pix_format  = {
		.width = 320,//640,
		.height = 240,//480,
		.pixelformat = V4L2_PIX_FMT_GREY,//V4L2_PIX_FMT_YUYV,
		.priv = 16,
		.colorspace = V4L2_COLORSPACE_RAW ,//V4L2_COLORSPACE_JPEG,
	},
};

int nuvoton_vin0_probe_mxl241(struct nuvoton_vin_device* cam)
{
	int ret = 0;

	ENTRY();

	nuvoton_vin0_attach_sensor(cam, &cap0_mxl241);

	// if i2c module isn't loaded at this time
	if(!sensor0_inited)
		return -1;

	printk("Mxl241 initialization...\n");

	Mxl241_Init();

	printk("Mxl241 initialization successful!\n");

	printk("Mxl241 tunner to 235MHz!\n");

	Mxl241_tune_channel(235, 6875, 64);

	printk("Mxl241 tunner successful!\n");

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
