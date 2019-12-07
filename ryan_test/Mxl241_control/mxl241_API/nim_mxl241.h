/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    nim_mxl241.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20070417       Penghui      Ver 0.1       Create file.
*	2.20080418		Joey		Ver 1.0		Fromal release for MXL241B V1.0
*	4.20100822		Joey		Ver 1.14. 	Optimize CR parameter for easy lock.
*****************************************************************************/

#ifndef __LLD_NIM_MXL241_H__
#define __LLD_NIM_MXL241_H__

//#include <sys_config.h>
//#include <types.h>
//#include <retcode.h>
//#include <hld/nim/nim_dev.h>
//#include <hld/nim/nim_tuner.h>

#include "MaxLinearDataTypes.h"
#include"MxL241SF_PhyCtrlApi.h"

#define MXL241_DEBUG_FLAG	0
#if(MXL241_DEBUG_FLAG)
#define QAM_DEBUG			1
#define TUNER_DEBUG		1
#else
#define QAM_DEBUG			0
#define TUNER_DEBUG		0
#endif
#if(QAM_DEBUG)
	#define MXL241_PRINTF		libc_printf
#else
	#define MXL241_PRINTF(...)		do{}while(0)
#endif
#if(TUNER_DEBUG)
	#define TUNER_PRINTF		libc_printf
#else
	#define TUNER_PRINTF(...)		do{}while(0)
#endif

//ID f_MXL241_IIC_Sema_ID = OSAL_INVALID_ID;

#define QAM_ONLY   0
#define QAM_SOC	1

#define NIM_MXL241_GET_BYTE(i)             (*(volatile UINT8 *)(i))
#define NIM_MXL241_SET_BYTE(i,d)          (*(volatile UINT8 *)(i)) = (d)

#define MXL241_QAM_ONLY_I2C_BASE_ADDR  	99 


#define SWITCH_NIM_MXL241_DEBUG	0

#define NIM_MXL241_ALL_LOCK							0x3F

struct nim_mxl241_private
{

	/* struct for QAM Configuration */
	struct   QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_Tuner_Init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_Tuner_Control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	INT32 (*nim_Tuner_Status)(UINT32 Tun_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	//struct QAM_TUNER_CONFIG_API TUNER_PRIV;	

	UINT32 tuner_id;
};


struct mxl241_Lock_Info
{	
	UINT32	Frequency;
	UINT32	SymbolRate;
	UINT8	Modulation;
};


INT32 nim_mxl241_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_mxl241_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_mxl241_Tuner_Attatch(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner);
INT32 nim_mxl241_attach();

static INT32 nim_mxl241_open(struct nim_device *dev);
static INT32 nim_mxl241_close(struct nim_device *dev);
static INT32 nim_mxl241_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
static INT32 nim_mxl241_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list);
static INT32 nim_mxl241_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change);
static INT32 nim_mxl241_channel_search(struct nim_device *dev, UINT32 freq);

static INT32 nim_mxl241_get_BER(struct nim_device *dev, UINT32 *err_count);
static INT32 nim_mxl241_get_lock(struct nim_device *dev, UINT8 *lock);
static INT32 nim_mxl241_get_freq(struct nim_device *dev, UINT32 *freq);
static INT32 nim_mxl241_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
static INT32 nim_mxl241_get_qam_order(struct nim_device *dev, UINT8 *QAM_order);
static INT32 nim_mxl241_get_AGC(struct nim_device *dev, UINT8 *agc);
static INT32 nim_mxl241_get_SNR(struct nim_device *dev, UINT8 *snr);
static INT32 nim_mxl241_get_PER(struct nim_device *dev, UINT32 *RsUbc);

static INT32 nim_mxl241_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel);
static INT32 nim_mxl241_get_CN_value(struct nim_device *dev, UINT16 *CNValue);

static UINT32 Log10Times100_L( UINT32 x);

#endif	/* __LLD_NIM_MXL241_H__ */




