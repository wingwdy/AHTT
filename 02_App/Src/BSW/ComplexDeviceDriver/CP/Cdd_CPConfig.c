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
static float CddCPCfg_GetPort0CPVol(void);
static void CddCPCfg_SetPort0PwmDuty(uint16_t duty);


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const CddCPOpsConfig_Struct c_stCddCPOpsConfigTable[SYSCFG_CFG_GUN_NUM] = 
{
    [0] = 
    {
        .pFuncGetCpVol = CddCPCfg_GetPort0CPVol,
        .pFunSetPwmDuty = CddCPCfg_SetPort0PwmDuty,
    },
};

const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterGB[CDDCP_CFG_VOLSTATE_COUNT] = 
{
    [eCddCPVolState_Ground] = {     0,      0,   2000,   2000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_Ground },
    [eCddCPVolState_6V] =     {  4000,   4200,   7800,   8000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_6V  },
    [eCddCPVolState_9V] =     {  7800,   8000,  10800,  11000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_9V  },
    [eCddCPVolState_12V] =    { 10800,  11000,  13000,  13000,   CDDCP_CFG_GB_FILERCNT,      eCddCPVolState_12V },
};

const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterQB[CDDCP_CFG_VOLSTATE_COUNT] = 
{
    [eCddCPVolState_Ground] = {     0,      0,   2000,   2000,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_Ground  },
    [eCddCPVolState_6V] =     {  3000,   3000,   7800,   8000,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_6V  },
    [eCddCPVolState_9V] =     {  7800,   8000,  10800,  11000,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_9V  },
    [eCddCPVolState_12V] =    { 10800,  11000,  13000,  13000,   CDDCP_CFG_QB_FILERCNT,      eCddCPVolState_12V },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static float CddCPCfg_GetPort0CPVol(void)
{
    uint16_t adcData[CDDCP_CFG_ADC_BUFF_POINT] = {0};
    uint16_t averageAdcData = 0;
    float cpVol = 0.0;

    McalADC_GetChannelData(eMcalADCChanel_CP, adcData, CDDCP_CFG_ADC_BUFF_POINT);
    
    averageAdcData = Common_MedianU16Filter(adcData, CDDCP_CFG_ADC_BUFF_POINT, CDDCP_CFG_ADC_BUFF_POINT / 2);
    cpVol = (averageAdcData / 4096.0 * 3.3) * 8.32439 - 13.2805;
    return cpVol;
}

static void CddCPCfg_SetPort0PwmDuty(uint16_t duty)
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
        McalPWM_SetSingleDuty(eMcalPWMOCChannel_CPDetect, duty / 2);
        McalPWM_SetOutputMode(eMcalPWMOCChannel_CP, MCALPWM_MODE_FORCE_PWM);
    }
}


















