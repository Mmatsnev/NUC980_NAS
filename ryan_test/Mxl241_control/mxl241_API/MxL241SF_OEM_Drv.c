/*******************************************************************************
 *
 * FILE NAME          : MxL241SF_OEM_Drv.c
 *
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : 7/30/2009
 *
 * DESCRIPTION        : This file contains I2C driver functins that OEM should
 *                      implement for MxL241SF APIs
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "MxL241SF_OEM_Drv.h"

//extern ID f_MXL241_IIC_Sema_ID;
/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_WriteRegister
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C write operation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/
MXL_STATUS Ctrl_WriteRegister(UINT8 I2cSlaveAddr, UINT16 RegAddr, UINT16 RegData)
{

 MXL_STATUS status = MXL_TRUE;

 #if 1

  UINT8 Write_Cmd[4];

 //osal_semaphore_capture(f_MXL241_IIC_Sema_ID,TMO_FEVR);
	Write_Cmd[0] = (RegAddr>>8)&0xFF;
	Write_Cmd[1] = (RegAddr)&0xFF;
	Write_Cmd[2] = (RegData>>8)&0xFF;
   	Write_Cmd[3] = (RegData)&0xFF;

   status=Mxl_i2c_write(MXL241_CHIP_ADRRESS, Write_Cmd, 4);
  if(status!=MXL_TRUE)
  {
    printf("MxL Reg Write : status %d : Addr[ 0x%x]=  0x%x\n", status,RegAddr, RegData);

  }
	//osal_task_sleep(10);
	//osal_semaphore_release(f_MXL241_IIC_Sema_ID);
#endif
//  if(value!=0)
//   return MXL_FALSE;
//  else
//   return MXL_TRUE;
    return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ReadRegister
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C read operation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ReadRegister(UINT8 I2cSlaveAddr, UINT16 RegAddr, UINT16 *DataPtr)
{
  MXL_STATUS status = MXL_TRUE;
  // OEM should implement I2C read protocol that complies with MxL241SF I2C
  // format.
#if 1
	UINT8 Read_Cmd[4];
    UINT8	Read_data[2];
// osal_semaphore_capture(f_MXL241_IIC_Sema_ID,TMO_FEVR);
	/* read step 1. accroding to mxl5007 driver API user guide. */
	Read_Cmd[0] = 0xFF;
	Read_Cmd[1] = 0xFB;
	Read_Cmd[2] = (RegAddr>>8)&0xFF;//ADD_HIGH
	Read_Cmd[3] = RegAddr&0xFF;//ADD_LOW

	status=Mxl_i2c_write(MXL241_CHIP_ADRRESS, Read_Cmd, 4);
	delay_ms(1);
	status=Mxl_i2c_read(MXL241_CHIP_ADRRESS, Read_data, 2);

   *DataPtr=(Read_data[0]<<8)|Read_data[1];
    if(status != 0)
    {
        printf("MxL Reg Read : status  %d  : Addr [ 0x%x]= 0x%x\n",status, RegAddr, *DataPtr);
    }

 //osal_semaphore_release(f_MXL241_IIC_Sema_ID);
   #endif
return status;
}

