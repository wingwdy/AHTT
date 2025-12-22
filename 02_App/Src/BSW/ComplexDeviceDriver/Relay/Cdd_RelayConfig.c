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
static void CddRelayCfg_CtrlSwitchOn(uint8_t port);
static void CddRelayCfg_HoldSwitchOn(uint8_t port);
static void CddRelayCfg_CtrlSwitchOff(uint8_t port);
static void CddRelayCfg_CtrlShortCutOff(uint8_t port);
static void CddRelayCfg_CtrlShortCutOn(uint8_t port);
static uint8_t CddRelayCfg_GetShortCutStatus(uint8_t port);
static uint8_t CddRelayCfg_GetRelayStatus(uint8_t port);
static uint8_t CddRelayCfg_GetRelayAdhesionStatus(uint8_t port);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const CddRelayOpsConfig_Struct c_stCddRelayOpsConfigTable = 
{
    .pFuncCtrlSwitchOn = CddRelayCfg_CtrlSwitchOn,
    .pFuncCtrlSwitchOff = CddRelayCfg_CtrlSwitchOff,
    .pFuncHoldSwitchOn = CddRelayCfg_HoldSwitchOn,
    .pFuncGetSwitchStatus = CddRelayCfg_GetRelayStatus,
    .pFuncGetRelayAdhesionStatus = CddRelayCfg_GetRelayAdhesionStatus,
    .pFuncCtrlShortCutOn = CddRelayCfg_CtrlShortCutOn,
    .pFuncCtrlShortCutOff = CddRelayCfg_CtrlShortCutOff,
    .pFuncGetShortCutStatus = CddRelayCfg_GetShortCutStatus,
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddRelayCfg_CtrlSwitchOn(uint8_t port)
{
    if (port == 0)
    {
        McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_HIGH);
    }
}

static void CddRelayCfg_HoldSwitchOn(uint8_t port)
{
    if (port == 0)
    {
        McalPWM_SetSingleDuty(eMcalPWMOCChannel_Relay, 500);
        McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_PWM);
    }
}

static void CddRelayCfg_CtrlSwitchOff(uint8_t port)
{
    if (port == 0)
    {
        McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_LOW);
    }
}

static uint8_t CddRelayCfg_GetRelayStatus(uint8_t port)
{  
    CddRelayState_Enum eRet = eCddRelayState_Off;

    if (port == 0)
    {
        eRet = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC7_OutBack1)) ? \
        eCddRelayState_On : eCddRelayState_Off);
    }

    return (uint8_t)eRet;
}

static uint8_t CddRelayCfg_GetRelayAdhesionStatus(uint8_t port)
{  
    uint8_t ret = FALSE;

    if (port == 0)
    {
        ret = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC6_RelayAdhesion)) ? TRUE : FALSE);
    }

    return ret;
}

static void CddRelayCfg_CtrlShortCutOff(uint8_t port)
{
    if (port == 0)
    {
        McalPort_ResetPin(eMcalPortPinChanel_PA8_ShortCutEn);
    }
}

static void CddRelayCfg_CtrlShortCutOn(uint8_t port)
{
    if (port == 0)
    {
        McalPort_SetPin(eMcalPortPinChanel_PA8_ShortCutEn);
    }
}

static uint8_t CddRelayCfg_GetShortCutStatus(uint8_t port)
{
    uint16_t adcData[CDDRELAY_CFG_ADC_BUFF_POINT] = {0};
    uint16_t averageAdcData = 0;
    float shortCutVol = 0.0;
    uint8_t ret = TRUE;

    if (port == 0)
    {
        McalADC_GetChannelData(eMcalADCChanel_ShortCut, adcData, CDDRELAY_CFG_ADC_BUFF_POINT);
    }

    averageAdcData = Common_MedianU16Filter(adcData, CDDRELAY_CFG_ADC_BUFF_POINT, CDDRELAY_CFG_ADC_BUFF_POINT / 2);
    shortCutVol = averageAdcData * 100 / 4096.0 * 3.3;

    if (shortCutVol < CDDRELAY_CFG_SHORTCUT_UPPER_LIMIT && shortCutVol > CDDRELAY_CFG_SHORTCUT_LOWER_LIMIT)
    {
        ret = FALSE;
    }

    return ret;
}

















