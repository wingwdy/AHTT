/******************************************************************************
* File Name          : Cdd_CPConfig.c
* Description        : Code for Control Pilot
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_CPConfig.h"
#include "Mcal_PWM.h"
#include "Mcal_ADC.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/




/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static float CddCPCfg_GetGun0CPVol(void);
static void CddCPCfg_SetGun0PwmDuty(uint16_t duty);


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const CddCPOpsConfig_Struct c_stCddCPOpsConfigTable[SYSCFG_CFG_GUN_NUM] = 
{
    [0] = 
    {
        .pFuncGetCpVol = CddCPCfg_GetGun0CPVol,
        .pFunSetPwmDuty = CddCPCfg_SetGun0PwmDuty,
    }
};

const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterGB[4] = 
{
    [eCddCPVolState_Ground] = {     0,      1999,   CDDCP_CFG_GB_FILERCNT,  eCddCPVolState_Ground  },
    [eCddCPVolState_6V] =     {  5000,      7000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_6V  },
    [eCddCPVolState_9V] =     {  8000,     10000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_9V  },
    [eCddCPVolState_12V] =    { 11000,     13000,   CDDCP_CFG_GB_FILERCNT,     eCddCPVolState_12V  },
};

const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterQB[4] = 
{
    [eCddCPVolState_Ground] = {     0,      1999,   CDDCP_CFG_QB_FILERCNT,  eCddCPVolState_Ground  },
    [eCddCPVolState_6V]     = {  3000,      7800,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_6V  },
    [eCddCPVolState_9V]     = {  8000,     10000,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_9V  },
    [eCddCPVolState_12V]    = { 11000,     13000,   CDDCP_CFG_QB_FILERCNT,     eCddCPVolState_12V  },
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static float CddCPCfg_GetGun0CPVol(void)
{
    uint16_t adcData[CDDCP_CFG_ADC_BUFF_POINT] = {0};
    uint16_t averageAdcData = 0;
    float cpVol = 0.0;

    McalADC_GetChannelData(eMcalADCChanel_CP, adcData, CDDCP_CFG_ADC_BUFF_POINT);
    averageAdcData = Common_MedianU16Filter(adcData, CDDCP_CFG_ADC_BUFF_POINT, CDDCP_CFG_ADC_BUFF_POINT / 2);

    cpVol = (averageAdcData / 4096.0 * 3.3  - 1.595) / 0.119 - 0.1;
    return cpVol;
}

static void CddCPCfg_SetGun0PwmDuty(uint16_t duty)
{
    if (duty == 0)
    {
        McalPWM_SetOutputMode(eMcalPWMOCChannel_CP, MCALPWM_MODE_FORCE_LOW);

    }
    else if (duty >= 1000)
    {
        McalPWM_SetOutputMode(eMcalPWMOCChannel_CP, MCALPWM_MODE_FORCE_HIGH);
    }
    else
    {
        McalPWM_SetSingleDuty(eMcalPWMOCChannel_CP, duty);
        McalPWM_SetSingleDuty(eMcalPWMOCChannel_CPDetect, duty);
        McalPWM_SetOutputMode(eMcalPWMOCChannel_CP, MCALPWM_MODE_FORCE_PWM);
    }
}


















