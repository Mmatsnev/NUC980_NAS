/*****************************************************************************
*    Copyright (C)2018 TongShi Corporation. All Rights Reserved.
*
*    File:    Mxl241.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20180125       Yanxuewen      Ver 0.1       Create file.

*****************************************************************************/

#ifndef __MXL241_H__
#define __MXL241_H__

#include <linux/delay.h>
#include <linux/module.h>
#include "../nuc980_cap.h"

#include "MaxLinearDataTypes.h"
#include "MxL241SF_PhyCtrlApi.h"
#include "MxL241SF_PhyCfg.h"
#include "MxL241SF_OEM_Drv.h"

//ID f_MXL241_IIC_Sema_ID = OSAL_INVALID_ID;

#define QAM_ONLY   0
#define QAM_SOC	1

#define MXL241_GET_BYTE(i)             (*(volatile UINT8 *)(i))
#define MXL241_SET_BYTE(i,d)          (*(volatile UINT8 *)(i)) = (d)

#define MXL241_QAM_ONLY_I2C_BASE_ADDR  	99


#define SWITCH_NIM_MXL241_DEBUG	0

#define MXL241_ALL_LOCK					0x3F

#define delay_ms	mdelay

//void delay_ms(UINT16 ms);

#endif	/* __MXL241_H__ */




