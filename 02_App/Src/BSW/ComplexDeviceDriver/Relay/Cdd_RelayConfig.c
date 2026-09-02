/******************************************************************************
* File Name          : Cdd_RelayConfig.c
* Description        : Code for the driver of relay
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
#include "Cdd_RelayConfig.h"
#include "Mcal_Port.h"
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
static void CddRelayCfg_CtrlPort0SwitchOn(void);
static void CddRelayCfg_HoldPort0SwitchOn(void);
static void CddRelayCfg_CtrlPort0SwitchOff(void);
static void CddRelayCfg_CtrlPort0ShortCutOff(void);
static void CddRelayCfg_CtrlPort0ShortCutOn(void);
static uint8_t CddRelayCfg_GetPort0ShortCutStatus(void);
static uint8_t CddRelayCfg_GetPort0RelayStatus(void);
static uint8_t CddRelayCfg_GetPort0RelayAdhesionStatus(void);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const CddRelayOpsConfig_Struct c_stCddRelayOpsConfigTable[SYSCFG_CFG_GUN_NUM] = 
{
    [0] =
    { 
        .pFuncCtrlSwitchOn = CddRelayCfg_CtrlPort0SwitchOn,
        .pFuncCtrlSwitchOff = CddRelayCfg_CtrlPort0SwitchOff,
        .pFuncHoldSwitchOn = CddRelayCfg_HoldPort0SwitchOn,
        .pFuncGetSwitchStatus = CddRelayCfg_GetPort0RelayStatus,
        .pFuncGetRelayAdhesionStatus = CddRelayCfg_GetPort0RelayAdhesionStatus,
        .pFuncCtrlShortCutOn = CddRelayCfg_CtrlPort0ShortCutOn,
        .pFuncCtrlShortCutOff = CddRelayCfg_CtrlPort0ShortCutOff,
        .pFuncGetShortCutStatus = CddRelayCfg_GetPort0ShortCutStatus,
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddRelayCfg_CtrlPort0SwitchOn(void)
{
    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_HIGH);
}

static void CddRelayCfg_HoldPort0SwitchOn(void)
{
    McalPWM_SetSingleDuty(eMcalPWMOCChannel_Relay, 500);
    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_PWM);
}

static void CddRelayCfg_CtrlPort0SwitchOff(void)
{
    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_LOW);
}

static uint8_t CddRelayCfg_GetPort0RelayStatus(void)
{  
    CddRelayState_Enum eRet = eCddRelayState_Off;

    eRet = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC7_OutBack1)) ? \
    eCddRelayState_On : eCddRelayState_Off);
    
    return (uint8_t)eRet;
}

static uint8_t CddRelayCfg_GetPort0RelayAdhesionStatus(void)
{  
    uint8_t ret = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC6_RelayAdhesion)) ? TRUE : FALSE);

    return ret;
}

static void CddRelayCfg_CtrlPort0ShortCutOff(void)
{
    McalPort_ResetPin(eMcalPortPinChanel_PA8_ShortCutEn);
}

static void CddRelayCfg_CtrlPort0ShortCutOn(void)
{
    McalPort_SetPin(eMcalPortPinChanel_PA8_ShortCutEn);
}

static uint8_t CddRelayCfg_GetPort0ShortCutStatus(void)
{
    uint16_t adcData[CDDRELAY_CFG_ADC_BUFF_POINT] = {0};
    uint16_t averageAdcData = 0;
    float shortCutVol = 0.0;
    uint8_t ret = TRUE;

    McalADC_GetChannelData(eMcalADCChanel_ShortCut, adcData, CDDRELAY_CFG_ADC_BUFF_POINT);
    
    averageAdcData = Common_MedianU16Filter(adcData, CDDRELAY_CFG_ADC_BUFF_POINT, CDDRELAY_CFG_ADC_BUFF_POINT / 2);
    shortCutVol = averageAdcData * 100 / 4096.0 * 3.3;

    if (shortCutVol < CDDRELAY_CFG_SHORTCUT_UPPER_LIMIT && shortCutVol > CDDRELAY_CFG_SHORTCUT_LOWER_LIMIT)
    {
        ret = FALSE;
    }

    return ret;
}

















