/*****************************************************************************
*    Copyright (C)2018 Tongshi Corporation. All Rights Reserved.
*
*    File:    MxL241.c
*
*    Description:    Source file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20180125       Yanxuewen      Ver 0.1        Create file.

*****************************************************************************/

#include "MxL241.h"

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
UINT8 Mxl241_Init(void)
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

  /***** 初始化 MxL2401 *************/

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
  MxL241SF_XtalSetting.XtalClkOutEnable = MXL_DISABLE;//关闭clkout
  MxL241SF_XtalSetting.XtalClkOutGain =  0x0;
  MxL241SF_XtalSetting.LoopThruEnable = MXL_DISABLE;//关闭loop

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
  MxL241SF_MpegOut.SerialOrPar = MPEG_DATA_SERIAL;//MPEG_DATA_PARALLEL;
  MxL241SF_MpegOut.MpegValidPol = MPEG_ACTIVE_LOW;//MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;//

  MxL241SF_MpegOut.MpegSyncPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
  MxL241SF_MpegOut.MpegClkFreq = MPEG_CLK_57MHz;//MPEG_CLK_28_5MHz;//MPEG_CLK_7_125MHz;
  MxL241SF_MpegOut.MpegClkSource = MPEG_CLK_EXTERNAL;//MPEG_CLK_INTERNAL;
  MxL241SF_MpegOut.MpegSyncPulseWidth=MPEG_SYNC_WIDTH_BIT;
  MxL241SF_MpegOut.LsbOrMsbFirst = MPEG_SERIAL_LSB_1ST;
  MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&MxL241SF_MpegOut);

  delay_ms(1);

  //Mxl241_tune_channel(251000000,6875,64);

  return 0;
}
SINT32 dbm;
unsigned char Mxl241_tune_channel(signed long tune_channel_FchanHz, unsigned long tuner_channel_SymbolRate, unsigned short tune_channel_QAM)
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
UINT8 Mxl241_GetSNR(void)
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
UINT8 MxL241_GetLock(void)
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

void MxL241_Start_Abort(UINT16 flag)
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
SINT32 Mxl241_Get_RxPower(void)
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

void delay_ms(UINT16 ms)
{
    while(ms>0)
    {
    usleep(1000);
    ms--;
    }
}
