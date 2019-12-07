
#include <sys/ioctl.h> /*IO指令流函数，如cmd等，除了打开函数之外，其他的函数定义*/
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "include/i2c/smbus.h"
#include "Mxl_i2c.h"


MXL_STATUS Mxl_i2c_write(UINT8 I2cSlaveAddr, UINT8* Write_Cmd, UINT8 len)
{
    /* Using I2C Write, equivalent of
         i2c_smbus_write_word_data(file, reg, 0x6543) */

    if (write(fp_i2c, Write_Cmd, len) != len)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("i2c transaction write failed!");
        return MXL_FALSE;
    }
    return MXL_TRUE;
}

MXL_STATUS Mxl_i2c_read(UINT8 I2cSlaveAddr, UINT8* Read_Data, UINT8 len)
{
    /* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
    if (read(fp_i2c, Read_Data, len) != len)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("i2c transaction write failed!");
        return MXL_FALSE;
    }
    return MXL_TRUE;
}
