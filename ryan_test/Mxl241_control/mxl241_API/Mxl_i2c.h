#ifndef MXL_I2C_H_INCLUDED
#define MXL_I2C_H_INCLUDED

#include "MxL241.h"

MXL_STATUS Mxl_i2c_write(UINT8 I2cSlaveAddr, UINT8* Write_Cmd, UINT8 len);

MXL_STATUS Mxl_i2c_read(UINT8 I2cSlaveAddr, UINT8* Read_Data, UINT8 len);

extern int fp_i2c;

#endif // MXL_I2C_H_INCLUDED
