/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    nim_mxl241.c
*
*    Description:    Source file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20070524       Penghui      Ver 0.1       Create file.
*	2.20071022	   Penghui      Ver 0.2       Delete the redundant code
*	3.20080418		Joey		Ver 1.0		Fromal release for mxl241B V1.0
*	4.20100822		Joey		Ver 1.14. 	Optimize CR parameter for easy lock.
*****************************************************************************/
//#include <sys_config.h>
//#include <retcode.h>
//#include <types.h>
//#include <osal/osal.h> 
//#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
//#include <hal/hal_gpio.h>
//#include <bus/i2c/i2c.h>
//#include <hld/hld_dev.h>
//#include <hld/nim/nim_dev.h>
//#include <hld/nim/nim.h>
//
//#include <hld/nim/nim_tuner.h>		
//#include <bus/tsi/tsi.h>

#include "MaxLinearDataTypes.h"
#include "nim_mxl241.h"
#include "MxL241SF_OEM_Drv.h"
#include "i2c_master.h"

/* Need to modify here for different perpose */
//===============

#ifdef TUNER_I2C_BYPASS
#define	I2C_BYPASS
#endif
//===============

/* Name for the tuner, the last character must be Number for index */
static char nim_mxl241_name[HLD_MAX_NAME_SIZE] = "NIM_QAM_mxl241";
static struct mxl241_Lock_Info mxl241_CurChannelInfo;
static OSAL_ID dem_mxl241_mutex_id = OSAL_INVALID_ID;
static OSAL_ID dem_mxl241_task_id = OSAL_INVALID_ID;

static UINT32 BER_COUNTS=0;
static UINT32 PER_COUNTS=0;
static BOOL    BER_VALID=FALSE;
//joey 20080504. add in ber&per ready count.
static UINT32 acc_ber_cnt = 0;

static UINT16 aci_delfreq =  0;
static BOOL channel_change_en = FALSE;

//joey 20080504. For ACI and max level AGC patch.
static UINT8 if_sml_val1; //for strong signal
static UINT8 if_def_val2;//for aci signal

static UINT16 rf_sml_th1;
static UINT16 rf_big_th2;

//joey 20080512. for RF-agc enable switch.
static BOOL rf_agc_en = TRUE;

//joey 20080504. move here for global usage.
//joey 20080422. for init freq offset usage.
static INT16 init_foffset = 150;

static OSAL_ID l_mxl241_sema_id;

ID  	nim_mxl241_flag = OSAL_INVALID_ID;
#define	NIM_mxl241_FLAG_ENABLE			0x00000100
#define NIM_mxl241_SCAN_END                        	0x00000001
#define NIM_mxl241_CHECKING				0x00000002



//#define MXL241_PRINTF(...)  //libc_printf
//=============================================================
static UINT32 dem_i2c_id = I2C_TYPE_SCB0;
INT32 nim_mxl241_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{

#if 0
	UINT8 chip_adr;
	INT32 bRet;

	chip_adr = 0xC0;
	
	osal_mutex_lock(dem_mxl241_mutex_id, OSAL_WAIT_FOREVER_TIME);
	bRet = i2c_write(dem_i2c_id, chip_adr, &bMemAdr, 1);
	if(bRet == SUCCESS)
		bRet = i2c_read(dem_i2c_id, chip_adr, pData, bLen);
	osal_mutex_unlock(dem_mxl241_mutex_id);
	return bRet;
#else
	*pData=0;
	return 0;
#endif
}
INT32 nim_mxl241_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{

#if 0
	UINT8 chip_adr;
	INT32 i,bRet;

	chip_adr = 0xC0;

	UINT8 bTemp[bLen+1];

	for(i = 1;i < bLen+1;i++)
		bTemp[i] = pData[i-1];
	bTemp[0] = bMemAdr;

	osal_mutex_lock(dem_mxl241_mutex_id, OSAL_WAIT_FOREVER_TIME);
	bRet = i2c_write(dem_i2c_id, chip_adr, bTemp, bLen+1);
	osal_mutex_unlock(dem_mxl241_mutex_id);
	return bRet;
#else
	return 0;
#endif
}
/**********************
void nim_mxl241_reg_dump(void)
{
	UINT16 Data,i=0;

#if 1
	
	//while(1)
	{	
		Data=0;
		Ctrl_ReadRegister(MXL241_CHIP_ADRRESS, 0x0017, &Data);
		osal_task_sleep(1);
		//libc_printf("Reg[ID]=%x \n",Data);


		Data=0x0014;
		Ctrl_WriteRegister(MXL241_CHIP_ADRRESS, 0x0006, Data);
		Data=0;
		Ctrl_ReadRegister(MXL241_CHIP_ADRRESS, 0x0006, &Data);
		osal_task_sleep(100);
		//libc_printf("Reg[LT]=%x \n",Data);

		Data=0;
		Ctrl_ReadRegister(MXL241_CHIP_ADRRESS, 0x0027, &Data);
		osal_task_sleep(1);
		//libc_printf("Reg[IQ]=%x \n",Data);

	}
#else	
		for(i=0x0000;i<0xffff;i++){
		Data=0;
		Ctrl_ReadRegister(MXL241_CHIP_ADRRESS, i, &Data);
		osal_task_sleep(1);
		//libc_printf("Reg[%x]=%x \n",i,Data);
	}	
#endif
	
}

void nim_mxl241_dump_all(void)
{
		UINT16 Data,i=0;

		for(i=0x0000;i<0xffff;i++){
		Data=0;
		Ctrl_ReadRegister(MXL241_CHIP_ADRRESS, i, &Data);
		osal_task_sleep(1);
		//libc_printf("Reg[%x]=%x \n",i,Data);
		}

	
}
******************************************************/

/*****************************************************************************
* INT32 nim_mxl241_open(struct nim_device *dev)
* Description: mxl241 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_open(struct nim_device *dev)
{
	UINT16 Data,i=0;
	 UINT32 loop;
#ifdef __MXL_INTEGER_CALC_STATISTICS__
  SINT64 tmpRealNum1;
  SINT64 tmpRealNum2;
#else
  REAL32 tmpRealNum1;
  REAL32 tmpRealNum2;
  #endif
  MXL_STATUS status;
  MXL_RESET_CFG_T               MxL241SF_Reset;
  MXL_OVERWRITE_DEFAULT_CFG_T   MxL241SF_OverwriteDefault;
  MXL_DEV_INFO_T                MxL241SF_DeviceInfo;
  MXL_XTAL_CFG_T                MxL241SF_XtalSetting;
  MXL_DEMOD_LOCK_STATUS_T       MxL241SF_LockStatus;
  MXL_SYMBOL_RATE_T             MxL241SF_SymbolRate;
  MXL_AGC_T                     MxL241SF_Agc;
  MXL_TOP_MASTER_CFG_T          MxL241SF_PowerUpTuner;
  MXL_ANNEX_CFG_T               MxL241SF_AnnexType;
  MXL_PWR_MODE_CFG_T            MxL241SF_PwrMode;
  MXL_MPEG_OUT_CFG_T            MxL241SF_MpegOut;
  MXL_CHAN_DEPENDENT_T          MxL241SF_ChanDependent;
  MXL_ADCIQFLIP_CFG_T           MxL241SF_AdcIpFlip;

	struct QAM_TUNER_CONFIG_DATA TunerConfig= ((struct nim_mxl241_private*)dev->priv)->tuner_config_data;
	struct QAM_TUNER_CONFIG_EXT TunerConfigext= ((struct nim_mxl241_private*)dev->priv)->tuner_config_ext;

	dem_mxl241_mutex_id = osal_mutex_create();
	if (dem_mxl241_mutex_id == OSAL_INVALID_ID)
	{
		//MXL241_PRINTF("nim_mxl241_open: Create mutex failed!\n");
		return ERR_FAILUE;
	}



    	l_mxl241_sema_id = osal_semaphore_create(1);
	if (l_mxl241_sema_id == OSAL_INVALID_ID)
    	{
       	 //MXL241_PRINTF("Create l_mxl241_sema_id  semaphore fail!\n");
        	return ERR_FAILUE;
    	}

	nim_mxl241_flag = osal_flag_create(NIM_mxl241_FLAG_ENABLE);
	if (nim_mxl241_flag==OSAL_INVALID_ID)
	{	
		 //MXL241_PRINTF("Create nim_mxl241_flag  semaphore fail!\n");
		return ERR_FAILUE;
	}

  //nim_mxl241_reg_dump();
  // 1. Do SW Reset
  MxL241SF_Reset.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  status = MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&MxL241SF_Reset);

  // 2. Overwrite default
  MxL241SF_OverwriteDefault.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 96;
  status = MxLWare_API_ConfigDevice(MXL_DEV_OVERWRITE_DEFAULT_CFG, (void*)&MxL241SF_OverwriteDefault);

  // 3. Read Back Device id and version
  status = MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, (void*)&MxL241SF_DeviceInfo);
  //if (status == MXL_TRUE) 
  //{
    //libc_printf("MxL241SF : DevId = 0x%x, Version = 0x%x \n\n", MxL241SF_DeviceInfo. DevId, MxL241SF_DeviceInfo. DevVer);
  //}

  // 4. XTAL and Clock out setting
  MxL241SF_XtalSetting.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_XtalSetting.XtalEnable = MXL_ENABLE;
  MxL241SF_XtalSetting.DigXtalFreq = XTAL_48MHz;//XTAL_48MHz;
  MxL241SF_XtalSetting.XtalBiasCurrent = 1;
  MxL241SF_XtalSetting.XtalCap = 10; // 10pF
  MxL241SF_XtalSetting.XtalClkOutEnable = MXL_DISABLE;//¹Ø±Õclkout
  MxL241SF_XtalSetting.XtalClkOutGain =  0x0; 
  MxL241SF_XtalSetting.LoopThruEnable = MXL_DISABLE;//¹Ø±Õloop

  MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, (void*)&MxL241SF_XtalSetting);

  // 5. AGC configuration
 MxL241SF_Agc.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
 MxL241SF_Agc.FreezeAgcGainWord = MXL_NO_FREEZE;

  MxLWare_API_ConfigTuner(MXL_TUNER_AGC_SETTINGS_CFG, (void*)&MxL241SF_Agc);

    // 7. MPEG out setting
  MxL241SF_MpegOut.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//96;
  MxL241SF_MpegOut.SerialOrPar = MPEG_DATA_PARALLEL;
  MxL241SF_MpegOut.MpegValidPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;//
  MxL241SF_MpegOut.MpegSyncPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkFreq = MPEG_CLK_4_75MHz;
  MxL241SF_MpegOut.MpegClkSource = MPEG_CLK_INTERNAL;
 // MxL241SF_MpegOut.MpegSyncPulseWidth=MPEG_SYNC_WIDTH_BIT;

  MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&MxL241SF_MpegOut);


  // 6. Power Up Tuner
  MxL241SF_PowerUpTuner.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 99;
  MxL241SF_PowerUpTuner.TopMasterEnable = MXL_ENABLE;

  MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, (void*)&MxL241SF_PowerUpTuner);

  osal_task_sleep(1);

	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_mxl241_close(struct nim_device *dev)
* Description: mxl241 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_close(struct nim_device *dev)
{
	UINT8 Data = 0;
	INT32 ret = SUCCESS;

	//mxl241
	MXL_PWR_MODE_CFG_T            MxL241SF_PwrMode;

	// Do power control If needed 
 	MxL241SF_PwrMode.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_PwrMode.PowerMode = STANDBY_ON;

  	//MxLWare_API_ConfigDevice(MXL_DEV_POWER_MODE_CFG, (void*)&MxL241SF_PwrMode) ;


	return ret;
}


/*****************************************************************************
* INT32 nim_mxl241_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
*
*  device input/output operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 cmd
*  Parameter3: UINT32 param
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	INT32 rtn;
	switch( cmd )
	{
	case NIM_DRIVER_READ_QPSK_BER:
	    rtn =  nim_mxl241_get_BER(dev, (UINT32 *)param);
	    break;
	case NIM_DRIVER_READ_RSUB:
	    rtn =  nim_mxl241_get_PER(dev, (UINT32 *)param);
	    break;
	default:
	    rtn = SUCCESS;
	    break;
	}
	return rtn;
}
static INT32 nim_mxl241_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list)
{
	INT32 rtn;
	switch( cmd )
	{
	case NIM_DRIVER_AUTO_SCAN:			/* Do AutoScan Procedure */
		rtn = SUCCESS;
		break;
	case NIM_DRIVER_CHANNEL_CHANGE:		/* Do Channel Change */
		rtn = nim_mxl241_channel_change(dev, (struct NIM_Channel_Change *) (param_list));
		break;
	case NIM_DRIVER_QUICK_CHANNEL_CHANGE:	/* Do Quick Channel Change without waiting lock */
		rtn = nim_mxl241_channel_change(dev, (struct NIM_Channel_Change *) (param_list));	
		break;
	case NIM_DRIVER_CHANNEL_SEARCH:	/* Do Channel Search */
		rtn= SUCCESS;
		break;
	case NIM_DRIVER_GET_RF_LEVEL:
		rtn = nim_mxl241_get_RF_Level(dev, (UINT16 *)param_list);
		break;
	case NIM_DRIVER_GET_CN_VALUE:
		rtn = nim_mxl241_get_CN_value(dev, (UINT16 *)param_list);
		break;
	case NIM_DRIVER_GET_BER_VALUE:
		rtn = nim_mxl241_get_BER(dev, (UINT32 *)param_list);
		break;
	case NIM_DRIVER_SET_PERF_LEVEL:
		rtn = SUCCESS;//nim_mxl241_set_perf_level(dev, (UINT32)param_list);
		break;
	default:
		rtn = SUCCESS;
	       break;
	}

	return rtn;
}



static INT32 nim_mxl241_channel_search(struct nim_device *dev, UINT32 freq)
{
	return SUCCESS;
}

static INT32 nim_mxl241_get_lock(struct nim_device *dev, UINT8 *lock)
{
	UINT8 data;	
	MXL_STATUS status = MXL_TRUE;
	MXL_DEMOD_LOCK_STATUS_T       MxL241SF_LockStatus;
	OSAL_ER	result;
       UINT32	flgptn;

	result = osal_flag_wait(&flgptn,nim_mxl241_flag, NIM_mxl241_SCAN_END, OSAL_TWF_ANDW,0);
	//libc_printf("lock result = %d\r\n",result);
	if(OSAL_E_OK!=result)
	{
        	//libc_printf("Get LOCK Fail, Wait ChannelChg Complete!\n");
		*lock=0;
	}  
	else
	{
		MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;   
    		status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&MxL241SF_LockStatus);
    		if (MxL241SF_LockStatus.Status == 1)
		{
			*lock = 1;
		}
		else
		{
			*lock = 0;
		}
	}
	return SUCCESS;
}

static INT32 nim_mxl241_get_freq(struct nim_device *dev, UINT32 *freq)
{
	INT32 freq_off;
	UINT8 data[2];


	//*freq = (mxl241_CurChannelInfo.Frequency - init_foffset)/10;
	*freq = mxl241_CurChannelInfo.Frequency/10;

	//MXL241_PRINTF("*freq is %d !\n", *freq);
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_mxl241_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Read mxl241 symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate			: Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
static INT32 nim_mxl241_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
	UINT8 data[2];
	UINT32 rtp_sym;

	*sym_rate =mxl241_CurChannelInfo.SymbolRate;

	return SUCCESS;
}

static INT32 nim_mxl241_get_qam_order(struct nim_device *dev, UINT8 *qam_order)//fec
{
	UINT8 data;

	//nim_mxl241_read(NIM_mxl241_FSM19, &data, 1);

	*qam_order =mxl241_CurChannelInfo.Modulation;// (UINT8)(data&0x0f);

	return SUCCESS;
}

static INT32 nim_mxl241_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr )
{
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_mxl241_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_AGC(struct nim_device *dev, UINT8 *agc)
{
	UINT8 data[4];
	UINT16 temp;
	UINT8 if_agc_gain, tun_agc_gain;
	UINT8 RF_EN, IF_EN;

	//nim_mxl241_read(NIM_mxl241_AGC6,data,4);
	UINT8	RF_AGC_MAX_priv=data[0];
	UINT8	RF_AGC_MIN_priv=data[1] ;
	UINT8	IF_AGC_MAX_priv=data[2];
	UINT8	IF_AGC_MIN_priv=data[3];
	
	//nim_mxl241_read(NIM_mxl241_MONITOR1, data, 1);
	*agc = 100;	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl241_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*  The Eb No is calculated using the SNR from the NIM, using the formula:
*     Eb ~     13312- M_SNR_H
*     -- =    ----------------  dB.
*     NO           683
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_SNR(struct nim_device *dev, UINT8 *snr)
{
	//UINT8 data[2];
	UINT32 rpt_power=0;
	
		*snr = 100;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl241_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel)
*
*  This function will access the NIM to determine the RF level feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *RfLevel
*  Real_RF_level and RfLevel relation is : RfLevel = -(Real_RF_level * 10)
*  eg.  if Real_RF_level = -30.2dBm then , RfLevel = -(Real_RF_level * 10) = 302
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel)
{
	UINT8 data[3];
	UINT16 Temp16, if_agc_gain,rf_agc_gain;
	UINT32 Temp32=0;

#if 0
	nim_mxl241_read(NIM_mxl241_AGC11, data, 3);
	Temp16 = (UINT16)(((data[1]&0x03)<<8) | data[0]);
	if_agc_gain = (UINT16)((Temp16+0x200)&0x3FF);
	Temp16 = (UINT16)(((data[2]&0x0f)<<6) | ((data[1]>>2)&0x3f));
	rf_agc_gain = (UINT16)((Temp16+0x200)&0x3FF);  

#if (SYS_TUNER_MODULE ==DCT7044)
	if (if_agc_gain > 208)
		Temp32 = (UINT32)((179 * if_agc_gain + 30205 + 50 )/100) ; 
	else if (if_agc_gain > 142)
		Temp32 = (UINT32)((159 * if_agc_gain + 34309 + 50)/100);
	else if (if_agc_gain > 114)
		Temp32 = (UINT32) ((114 * if_agc_gain + 40671 + 50)/100);
	else if (if_agc_gain > 91 )
		Temp32 = (UINT32 )((52 * if_agc_gain + 47752 + 50)/100);
	else if ((if_agc_gain <= 91) && (rf_agc_gain >967 ))
		Temp32 = (UINT32) 510;
	else if (rf_agc_gain > 967 )
		Temp32 = (UINT32)((13 * rf_agc_gain + 38793 + 50)/100);
	else if (rf_agc_gain > 940)
		Temp32 = (UINT32 )((26 * rf_agc_gain + 26030 + 50)/100);
	else if (rf_agc_gain > 914 )
		Temp32 = (UINT32 )((58 * rf_agc_gain - 3831 + 50)/100);
	else if (rf_agc_gain > 878 )
		Temp32 = (UINT32)(( 75 * rf_agc_gain - 19650 + 50)/100);
	else if (rf_agc_gain > 806)
		Temp32 = (UINT32)(( 83 * rf_agc_gain - 26967 + 50)/100);
	else if (rf_agc_gain > 767)
		Temp32 = (UINT32)((67 * rf_agc_gain - 13533 + 50)/100);
	else if (rf_agc_gain > 694)
		Temp32 = (UINT32)((49 * rf_agc_gain - 225 + 50)/100);
	else if (rf_agc_gain > 532)
		Temp32 = (UINT32) ((28 * rf_agc_gain + 14294 + 50)/100);
	else if(rf_agc_gain > 387 )
		Temp32 = (UINT32)(( 21 * rf_agc_gain + 18026 + 50)/100);
	else if (rf_agc_gain > 315)
		Temp32 = (UINT32)(( 33 * rf_agc_gain + 13400 + 50)/100);
	else if (rf_agc_gain > 245)
		Temp32 = (UINT32)((64 * rf_agc_gain+ 3650 + 50)/100);
	else if (rf_agc_gain > 201)
		Temp32 = (UINT32)((141 * rf_agc_gain - 15123 + 50)/100);
	else if (rf_agc_gain > 155)
		Temp32 = (UINT32)((570 * rf_agc_gain - 101283 + 50)/100);
	else 
		Temp32 = 0;
#elif (SYS_TUNER_MODULE ==DCT70701)
	if (if_agc_gain > 721)
		Temp32 = 770;
	else if (if_agc_gain > 704)
		Temp32 = (UINT32)((176 * if_agc_gain - 49896)/100);
	else if (if_agc_gain > 685)
		Temp32 = (UINT32)((211 * if_agc_gain - 74544)/100);
	else if (if_agc_gain > 667)
		Temp32 = (UINT32)((167 * if_agc_gain - 44395)/100);
	else if (if_agc_gain > 650)
		Temp32 = (UINT32)((294 * if_agc_gain - 129098)/100);
	else if (if_agc_gain > 628)
		Temp32 = (UINT32)((227 * if_agc_gain - 85550)/100);
	else if (if_agc_gain > 608)
		Temp32 = (UINT32)((250 * if_agc_gain - 100000)/100);
	else if (if_agc_gain > 583)
		Temp32 = (UINT32)((200 * if_agc_gain - 69600)/100);
	else
		Temp32 = 470;
#elif (SYS_TUNER_MODULE ==TDCCG0X1F)
	if (if_agc_gain > 456)
		Temp32 = 850;
	else if (if_agc_gain > 430)
		Temp32 = (UINT32)((222 * if_agc_gain - 16460)/100);
	else if (if_agc_gain > 407)
		Temp32 = (UINT32)((217 * if_agc_gain - 14319)/100);
	else if (if_agc_gain > 373)
		Temp32 = (UINT32)((205 * if_agc_gain - 9465)/100);
	else if (if_agc_gain > 350)
		Temp32 = (UINT32)((434 * if_agc_gain - 94878)/100);
	else if (if_agc_gain > 341)
		Temp32 = (UINT32)((1111 * if_agc_gain - 331851)/100);
	else
		Temp32 = 460;
#endif
#endif	

	Temp32 = 460;
	*RfLevel = (UINT16)Temp32;

	return SUCCESS;	

}

/*****************************************************************************
* INT32 nim_mxl241_get_CN_value(struct nim_device *dev, UINT16 *CNValue)
*
*  This function will access the NIM to determine the C/N  feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *CNValue
*  Real_CN_value and CNValue relation is : CNValue = Real_CN_value * 10
*  eg.  if Real_CN_value = 28.3dB then , CNValue = Real_RF_level * 10 = 283
* Return Value: INT32
*****************************************************************************/

static INT32 nim_mxl241_get_CN_value(struct nim_device *dev, UINT16 *CNValue)
{
	UINT16 EQ_MSE;
	UINT8 data[2];
	UINT8 qam_order;
	UINT16 const_modules;
	UINT16 log_cstmod_x100_add_100;//  100 * log10(const_modules) + 100
	UINT16 SNR_offset = 25;
	UINT16 cnr = 0;

#if 0
	nim_mxl241_read(NIM_mxl241_SNR_MONI1, data, 2);
	EQ_MSE = (UINT16)(data[1]<<8 | data[0]);

	qam_order = mxl241_CurChannelInfo.Modulation;
	switch (qam_order)
	{
		case 4:const_modules = 40960; log_cstmod_x100_add_100 = 561;break;
		case 5:const_modules = 20480; log_cstmod_x100_add_100 = 531;break;
		case 6:const_modules = 43008; log_cstmod_x100_add_100 = 563;break;
		case 7:const_modules = 20992; log_cstmod_x100_add_100 = 532;break;
		case 8:const_modules = 43520; log_cstmod_x100_add_100 = 564;break;
		default:const_modules = 43008; log_cstmod_x100_add_100 = 563;break;
	}
	/*
	SNR = 10*log10(const_modules/EQ_MSE);
		= 10*log10(const_modules)-10*log10(EQ_MSE);
		
	SNR10 = 10 * SNR
		= 100 * log10(const_modules) - 100*log10(EQ_MSE)
		= 100 * log10(const_modules) + 100  - 100*log10(EQ_MSE *10);
		= log_cstmod_x100_add_100 - Log10Times100_L(EQ_MSE*10);
	*/

	cnr = log_cstmod_x100_add_100 - Log10Times100_L(EQ_MSE*10) - SNR_offset;
#endif
	*CNValue = 30;//cnr;

	return SUCCESS;	
	
}



/*****************************************************************************
* INT32 nim_mxl241_get_PER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_PER(struct nim_device *dev, UINT32 *RsUbc)
{
	*RsUbc = PER_COUNTS;
	return SUCCESS;
}

static INT32 nim_mxl241_get_BER(struct nim_device *dev, UINT32 *err_count)
{
	MXL_DEMOD_BER_INFO_T BerInfoPtr;
	MXL_STATUS status = MXL_TRUE;

	*err_count = BER_COUNTS;
#if 0
	BerInfoPtr.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99; 
	status=MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_UNCODED_BER_CER_REQ, (void*)&BerInfoPtr);
	//libc_printf("ber=%d \n",BerInfoPtr.BER);
#endif
	return SUCCESS;
}

INT32 nim_mxl241_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{

	struct nim_device *dev;
	struct nim_mxl241_private *priv_mem;

       /* Attatch the private Tuner Configuration */

       /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_mxl241_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		//MXL241_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	priv_mem = (struct nim_mxl241_private *)MALLOC(sizeof(struct nim_mxl241_private));	
	if ((void*)priv_mem == NULL)
	{
		dev_free(dev);
		//MXL241_PRINTF("Alloc nim device prive memory error!/n");
		return ERR_NO_MEM;
	}
	MEMSET((void*)priv_mem, 0, sizeof(struct nim_mxl241_private));

	/* tuner configuration function */
	MEMCPY((void*)&(priv_mem->tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
       MEMCPY((void*)&(priv_mem->tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
	priv_mem->nim_Tuner_Init = ptrQAM_Tuner->nim_Tuner_Init;
	priv_mem->nim_Tuner_Control = ptrQAM_Tuner->nim_Tuner_Control;
	priv_mem->nim_Tuner_Status = ptrQAM_Tuner->nim_Tuner_Status;

	f_MXL241_IIC_Sema_ID=osal_semaphore_create(1);
	if(f_MXL241_IIC_Sema_ID == OSAL_INVALID_ID)
	{
		//libc_printf("f_MXL241_IIC_Sema_ID Create fail\n");
	}

    #ifdef I2C_BYPASS
    dem_i2c_id = ptrQAM_Tuner->tuner_config_ext.i2c_type_id;
    #endif
	
	dev->priv = (void*)priv_mem;
	/* Function point init */
	dev->base_addr = MXL241_QAM_ONLY_I2C_BASE_ADDR;	//0x40
	dev->init = nim_mxl241_attach;
	dev->open = nim_mxl241_open;
	dev->stop = nim_mxl241_close;
	dev->do_ioctl = nim_mxl241_ioctl;
	dev->do_ioctl_ext = nim_mxl241_ioctl_ext;
	dev->get_lock = nim_mxl241_get_lock;
	dev->get_freq = nim_mxl241_get_freq;
	dev->get_FEC = nim_mxl241_get_qam_order;	
	dev->get_SNR = nim_mxl241_get_SNR;
	dev->get_sym = nim_mxl241_get_symbol_rate;
	dev->get_BER = nim_mxl241_get_BER;
	dev->get_AGC = nim_mxl241_get_AGC;
	dev->get_fft_result = nim_mxl241_get_fft_result;
	dev->channel_search = nim_mxl241_channel_search;
	
	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		MXL241_PRINTF("Error: Register nim device error!\n");
		FREE(priv_mem);
		dev_free(dev);
		return ERR_NO_DEV;
	}
	/* Tuner Initial */
	if (((struct nim_mxl241_private*)dev->priv)->nim_Tuner_Init != NULL)	
	{
		if (((struct nim_mxl241_private*)dev->priv)->nim_Tuner_Init(&(((struct nim_mxl241_private*)dev->priv)->tuner_id), &(ptrQAM_Tuner->tuner_config_ext)) != SUCCESS)
		{
			//MXL241_PRINTF("Error: Init Tuner Failure!\n");
			return ERR_NO_DEV;
		}
	}
	return SUCCESS;
}



/*****************************************************************************
* INT32 nim_mxl241_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
* Description: mxl241 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq				: Frequence
*  Parameter3: UINT32 sym				: Symbol rate
*  Parameter4: UINT8 fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change)
{
	struct nim_mxl241_private *dev_priv;	
	INT32 rtn;
	INT32 result;
   	UINT8 lock = 0, Tuner_Retry = 0;
	UINT8 data,data1;
	UINT8 try_time;
	UINT16 cons_time;
	static UINT8 if_agc_min_ch = 0;
	
	UINT32	freq = pstChl_Change->freq;
	UINT32	sym = pstChl_Change->sym;
	UINT8	fec   = pstChl_Change->modulation;
	UINT16 Data;
	UINT8 lockget;

	//mxl241
	UINT32 loop;
	#ifdef __MXL_INTEGER_CALC_STATISTICS__
  	SINT64 tmpRealNum1;
  	SINT64 tmpRealNum2;
	#else
  	REAL32 tmpRealNum1;
  	REAL32 tmpRealNum2;
	#endif
 	 MXL_STATUS status;
  	MXL_RF_TUNE_CFG_T             MxL241SF_ChanTune;
  	MXL_DEMOD_LOCK_STATUS_T       MxL241SF_LockStatus;
  	MXL_SYMBOL_RATE_T             MxL241SF_SymbolRate;
  	MXL_AGC_T                     MxL241SF_Agc;
  	MXL_TOP_MASTER_CFG_T          MxL241SF_PowerUpTuner;
 	MXL_ANNEX_CFG_T               MxL241SF_AnnexType;
  	MXL_PWR_MODE_CFG_T            MxL241SF_PwrMode;
  	MXL_MPEG_OUT_CFG_T            MxL241SF_MpegOut;
  	MXL_ADCIQFLIP_CFG_T           MxL241SF_AdcIpFlip;
  	MXL_INTERDEPTH_INFO_T         MxL241SF_InterleaverInfo;
  	MXL_DEMOD_EQUALFILTER_INFO_T  MxL241SF_EqualizerInfo;
  	MXL_DEMOD_TIMINGOFFSET_INFO_T MxL241SF_TimingOffsetInfo;
  	MXL_TUNER_RX_PWR_T            MxL241SF_RfPowerReadback;
  	MXL_CHAN_DEPENDENT_T          MxL241SF_ChanDependent;
	//
	MXL_RESET_CFG_T               MxL241SF_Reset;
  	MXL_OVERWRITE_DEFAULT_CFG_T   MxL241SF_OverwriteDefault;
  	MXL_DEV_INFO_T                MxL241SF_DeviceInfo;
 	MXL_XTAL_CFG_T                MxL241SF_XtalSetting;


	//osal_semaphore_capture(l_mxl241_sema_id, TMO_FEVR);
	 osal_flag_clear(nim_mxl241_flag,NIM_mxl241_SCAN_END);	

	nim_mxl241_get_lock(dev, &lockget);
	
	// Add a comparametion, when the the new parameter and the existed parameter is the same ,
	// then return success directly without set it to the tuner and demod.
	// For the request of Mark_Li 2007/11/07
	if ((((freq - mxl241_CurChannelInfo.Frequency/10)<= 40)||\
		((mxl241_CurChannelInfo.Frequency/10 - freq)<= 40))&&\
	    ((sym - mxl241_CurChannelInfo.SymbolRate <=40)||(mxl241_CurChannelInfo.SymbolRate - sym <=40))&&\
	    (mxl241_CurChannelInfo.Modulation == fec)&&(lockget ==1))
	{

		mxl241_CurChannelInfo.Frequency = freq/10;
		mxl241_CurChannelInfo.SymbolRate = sym;
		mxl241_CurChannelInfo.Modulation = fec;
		//MXL241_PRINTF("MpegClkFreq=%d\n",MxL241SF_MpegOut.MpegClkFreq);("return CG : lockget=%d ,freq=%d sym=%d ,fec=%d \n",lockget,freq,sym,fec);
		//osal_semaphore_release(l_mxl241_sema_id);
		osal_flag_set(nim_mxl241_flag,NIM_mxl241_SCAN_END);
	   	return SUCCESS;
	
	}

	// 1. Do SW Reset
  	MxL241SF_Reset.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	status = MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&MxL241SF_Reset);

  	// 2. Overwrite default
  	MxL241SF_OverwriteDefault.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 99;
  	status = MxLWare_API_ConfigDevice(MXL_DEV_OVERWRITE_DEFAULT_CFG, (void*)&MxL241SF_OverwriteDefault);

  	// 3. Read Back Device id and version
  	status = MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, (void*)&MxL241SF_DeviceInfo);
  	if (status == MXL_TRUE) 
  	{
    		//MXL241_PRINTF("MxL241SF : DevId = 0x%x, Version = 0x%x \n\n", MxL241SF_DeviceInfo. DevId, MxL241SF_DeviceInfo. DevVer);
  	}

  	// 4. XTAL and Clock out setting
  	MxL241SF_XtalSetting.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_XtalSetting.XtalEnable = MXL_ENABLE;
  	MxL241SF_XtalSetting.DigXtalFreq = XTAL_24MHz;
  	MxL241SF_XtalSetting.XtalBiasCurrent = 1;
  	MxL241SF_XtalSetting.XtalCap = 10; // 10pF
  	MxL241SF_XtalSetting.XtalClkOutEnable = MXL_ENABLE;
  	MxL241SF_XtalSetting.XtalClkOutGain =  0xa; 
  	MxL241SF_XtalSetting.LoopThruEnable = MXL_ENABLE;

  	MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, (void*)&MxL241SF_XtalSetting);

  	// 5. AGC configuration
  	MxL241SF_Agc.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_Agc.FreezeAgcGainWord = MXL_NO_FREEZE;

  	MxLWare_API_ConfigTuner(MXL_TUNER_AGC_SETTINGS_CFG, (void*)&MxL241SF_Agc);

  	// 6. Power Up Tuner
  	MxL241SF_PowerUpTuner.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 99;
  	MxL241SF_PowerUpTuner.TopMasterEnable = MXL_ENABLE;

    	osal_task_sleep(1);

  	MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, (void*)&MxL241SF_PowerUpTuner);
  
  	// 7. MPEG out setting
  	MxL241SF_MpegOut.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_MpegOut.SerialOrPar = MPEG_DATA_PARALLEL;
  	MxL241SF_MpegOut.MpegValidPol = MPEG_ACTIVE_HIGH;
  	MxL241SF_MpegOut.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;//;
  	MxL241SF_MpegOut.MpegSyncPol = MPEG_ACTIVE_HIGH;
  	MxL241SF_MpegOut.MpegClkFreq = MPEG_CLK_4_75MHz;
	MxL241SF_MpegOut.MpegClkSource = MPEG_CLK_INTERNAL;
	//MXL241_PRINTF("MpegClkFreq=%d\n",MxL241SF_MpegOut.MpegClkFreq);

  	MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&MxL241SF_MpegOut);

	//Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
	freq = freq*10; //KHZ

	// To watch the input parameter	
	char  *char_qam;
	
	switch(fec)
	{
		case 4: char_qam = "16QAM";break;
		case 5: char_qam = "32QAM";break;
		case 6: char_qam = "64QAM";break;
		case 7: char_qam = "128QAM";break;
		case 8: char_qam = "256QAM";break;
		default: char_qam = "NONE"; break;
	}

	//libc_printf("%s,freq = %d, sym = %d, fec = %s \n",__FUNCTION__, freq,sym,char_qam);


	dev_priv = (struct nim_mxl241_private *)dev->priv;

	mxl241_CurChannelInfo.Frequency = freq;
	mxl241_CurChannelInfo.SymbolRate = sym;
	mxl241_CurChannelInfo.Modulation = fec;

 	// 8. Symbol rate
  	MxL241SF_SymbolRate.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_SymbolRate.SymbolType = SYM_TYPE_USER_DEFINED_J83A;//SYM_TYPE_J83A;//;
 	// sym=sym/10;
  	MxL241SF_SymbolRate.SymbolRate =((REAL32)(sym)/1000);//6.89;//(REAL32)(((REAL32)(sym))/100.0);//(REAL32)6.952;
 	// libc_printf("SymbolRate=%f \n",MxL241SF_SymbolRate.SymbolRate);
   	//MXL241_PRINTF("SymbolRate=%d \n",(UINT32)MxL241SF_SymbolRate.SymbolRate);
  	MxLWare_API_ConfigDemod(MXL_DEMOD_SYMBOL_RATE_CFG, (void*)&MxL241SF_SymbolRate);
	
 	 //  Wait(1ms);
  	osal_task_sleep(1);

  	// 9. Config Annex Type
  	MxL241SF_AnnexType.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_AnnexType.AnnexType = ANNEX_A;

  	MxL241SF_AnnexType.AutoDetectQamType = MXL_DISABLE;//MXL_ENABLE;
  	MxL241SF_AnnexType.AutoDetectMode = MXL_ENABLE;
  	//MxL241SF_AnnexType.QamType=MXL_QAM256;
  
	switch(fec)
	{
		case 4: MxL241SF_AnnexType.QamType=MXL_QAM16;break;
		case 5: MxL241SF_AnnexType.QamType=MXL_QAM32;break;
		case 6: MxL241SF_AnnexType.QamType=MXL_QAM64;break;
		case 7: MxL241SF_AnnexType.QamType=MXL_QAM128;break;
		case 8: MxL241SF_AnnexType.QamType=MXL_QAM256;break;
		default: MxL241SF_AnnexType.QamType=MXL_QAM64; break;
	}

	MxLWare_API_ConfigDemod(MXL_DEMOD_ANNEX_QAM_TYPE_CFG, (void*)&MxL241SF_AnnexType);

  	// 10. Do MiscSettings if needed
  	// 11. Tune RF with channel frequency and bandwidth
  	MxL241SF_ChanTune.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
  	MxL241SF_ChanTune.BandWidth = 8;//8;                     // 8MHz
  	MxL241SF_ChanTune.Frequency = freq*1000;;         // UINT:HZ
  	MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_TUNE_CFG, (void*)&MxL241SF_ChanTune );

      	osal_task_sleep(30);

  	// 12. Enable I/Q path flip
    	MxL241SF_AdcIpFlip.I2cSlaveAddr =MXL241_CHIP_ADRRESS;// 99; 
    	MxL241SF_AdcIpFlip.AdcIqFlip = MXL_ENABLE;
    	MxLWare_API_ConfigDemod(MXL_DEMOD_ADC_IQ_FLIP_CFG, (void*)&MxL241SF_AdcIpFlip);  

  	/* Please do not forget to put time delay at least 25ms */
  	// Wait 25ms
   	 osal_task_sleep(25);

  	// 13. Channel dependent setting
  	MxL241SF_ChanDependent.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
 	MxL241SF_ChanDependent.ChanDependentCfg = MXL_DISABLE;
  	MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG, (void*)&MxL241SF_ChanDependent);

  	/* Please do not forget to put time delay at least 300ms */
  	// Wait 300ms
    	//osal_task_sleep(300); 

	  // 14. Wait QAM_LOCK
  	loop = 0; 
 	 while (loop < 10) //  This number is experimental to monitor statistics.
  	{
    		MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;
   
    		status = MxLWare_API_GetDemodStatus(MXL_DEMOD_QAM_LOCK_REQ, (void*)&MxL241SF_LockStatus);
   		 if (MxL241SF_LockStatus.Status== 1)
    		{
       		//MXL241_PRINTF("QAM_LOCK : QAM_LOCK status = 0x%x\n", MxL241SF_LockStatus.Status); 
	  		 break;
    		}
		//Wait(1000); // This delay is also experimental, it can be anything.
  		osal_task_sleep(10);
    		loop++;
 	 }
  	//MXL241_PRINTF("QAM_LOCK status = 0x%x\n", MxL241SF_LockStatus.Status); 

	//if(status==0)
		//return 0;

	 // 15. Wait FEC Lock
  	loop = 0; 
  	while (loop < 10) //  This number is experimental to monitor statistics.
  	{
    		MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;   
    		status = MxLWare_API_GetDemodStatus(MXL_DEMOD_FEC_LOCK_REQ, (void*)&MxL241SF_LockStatus);
    		if (MxL241SF_LockStatus.Status== 1)
    		{
       		//MXL241_PRINTF("FEC Lock  : FEC Lock status = 0x%x\n", MxL241SF_LockStatus.Status); 
	   		break;
    		}
	  
		// Wait(1000); // This delay is also experimental, it can be anything.
  		osal_task_sleep(10);
    		loop++;
 	 }
  	//MXL241_PRINTF(" FEC Lock status = 0x%x\n", MxL241SF_LockStatus.Status); 

	//if(status==0)
		//return 0;

	//16. MXL_DEMOD_MPEG_LOCK
  	loop = 0; 
  	while (loop < 10) //  This number is experimental to monitor statistics.
  	{
    		MxL241SF_LockStatus.I2cSlaveAddr = MXL241_CHIP_ADRRESS;//99;   
    		status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&MxL241SF_LockStatus);
    		if (MxL241SF_LockStatus.Status== 1)
    		{
      			 //MXL241_PRINTF("MPEG_LOCK : MPEG_LOCK status = 0x%x\n", MxL241SF_LockStatus.Status); 
	     		break;
    		}
	
	//Wait(1000); // This delay is also experimental, it can be anything.
  	osal_task_sleep(10);
    	loop++;
  	}
  	//MXL241_PRINTF("MPEG_LOCK status = 0x%x\n", MxL241SF_LockStatus.Status); 	

	//osal_semaphore_release(l_mxl241_sema_id);
	osal_flag_set(nim_mxl241_flag,NIM_mxl241_SCAN_END);
	return rtn;
}


/*============================================================================*/
/**
* \fn UINT32 Log10Times100( UINT32 x)
* \brief Compute: 100*log10(x)
* \param x 32 bits
* \return 100*log10(x)
*
* 100*log10(x)
* = 100*(log2(x)/log2(10)))
* = (100*(2^15)*log2(x))/((2^15)*log2(10))
* = ((200*(2^15)*log2(x))/((2^15)*log2(10)))/2
* = ((200*(2^15)*(log2(x/y)+log2(y)))/((2^15)*log2(10)))/2
* = ((200*(2^15)*log2(x/y))+(200*(2^15)*log2(y)))/((2^15)*log2(10)))/2
*
* where y = 2^k and 1<= (x/y) < 2
*/

UINT32 Log10Times100_L( UINT32 x)
{
   static const UINT8 scale=15;
   static const UINT8 indexWidth=5;
   /*
   log2lut[n] = (1<<scale) * 200 * log2( 1.0 + ( (1.0/(1<<INDEXWIDTH)) * n ))
   0 <= n < ((1<<INDEXWIDTH)+1)
   */

   static const UINT32 log2lut[] = {
      0, 290941,  573196,  847269,1113620, 1372674, 1624818, 
      1870412, 2109788, 2343253, 2571091, 2793569,3010931, 
      3223408, 3431216, 3634553, 3833610, 4028562, 4219576, 
      4406807, 4590402, 4770499, 4947231, 5120719, 5291081, 
      5458428, 5622864, 5784489, 5943398,  6099680, 6253421, 
      6404702,  6553600  };
	  
   UINT8  i = 0;
   UINT32 y = 0;
   UINT32 d = 0;
   UINT32 k = 0;
   UINT32 r = 0;

   if (x==0) return (0);

   /* Scale x (normalize) */
   /* computing y in log(x/y) = log(x) - log(y) */
   if ( (x & (((UINT32)(-1))<<(scale+1)) ) == 0 )
   {
      for (k = scale; k>0 ; k--)
      {
        if (x & (((UINT32)1)<<scale)) break;
        x <<= 1;
      }
   } else {
      for (k = scale; k<31 ; k++)
      {
        if ((x & (((UINT32)(-1))<<(scale+1)))==0) break;
        x >>= 1;
      }
   }
   /*
     Now x has binary point between bit[scale] and bit[scale-1]
     and 1.0 <= x < 2.0 */

   /* correction for divison: log(x) = log(x/y)+log(y) */
   y = k * ( ( ((UINT32)1) << scale ) * 200 );

   /* remove integer part */
   x &= ((((UINT32)1) << scale)-1);
   /* get index */
   i = (UINT8) (x >> (scale -indexWidth));
   /* compute delta (x-a) */
   d = x & ((((UINT32)1) << (scale-indexWidth))-1);
   /* compute log, multiplication ( d* (.. )) must be within range ! */
   y += log2lut[i] + (( d*( log2lut[i+1]-log2lut[i] ))>>(scale-indexWidth));
   /* Conver to log10() */
   y /= 108853; /* (log2(10) << scale) */
   r = (y>>1);
   /* rounding */
   if (y&((UINT32)1)) r++;

   return (r);

}


