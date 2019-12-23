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
#include "MxL241.h"

#include "MxL241SF_OEM_Drv.h"

//extern ID f_MXL241_IIC_Sema_ID;
extern struct i2c_client *save_client0;

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
 int ret;
 #if 1

  UINT8 Write_Cmd[4];

	Write_Cmd[0] = (RegAddr>>8)&0xFF;
	Write_Cmd[1] = (RegAddr)&0xFF;
	Write_Cmd[2] = (RegData>>8)&0xFF;
   	Write_Cmd[3] = (RegData)&0xFF;


   	ret = i2c_master_send(save_client0, Write_Cmd, 4);
	if (ret < 4) {
		dev_err(&save_client0->dev, "%s: i2c write error, reg: 0x%x\n",
		        __func__, RegAddr);
		//return ret < 0 ? ret : -EIO;
		return MXL_FALSE;
	}
#endif

    return MXL_TRUE;
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
  int ret;
  // OEM should implement I2C read protocol that complies with MxL241SF I2C
  // format.
#if 1
	UINT8   Read_Cmd[4];
    	UINT8	Read_data[2];
// osal_semaphore_capture(f_MXL241_IIC_Sema_ID,TMO_FEVR);
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
		//return ret < 0 ? ret : -EIO;
	return MXL_FALSE;
	}
	else

     *DataPtr=(Read_data[0]<<8)|Read_data[1];
#endif
return MXL_TRUE;
}

