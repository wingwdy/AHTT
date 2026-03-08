/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
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
#include "Cdd_CP.h"
#include "Cdd_MeterM.h"
#include "Cdd_Relay.h"
#include "Asw_ChargeIf.h"
#include "Cdd_Sensor.h"
/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/




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
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
uint8_t AswChargeIf_CheckGunConnected(uint8_t port)
{
    CddCPVolState_Enum eCpVolState = CddCP_GetVolState(port);
    uint8_t ret = FALSE;

    if (eCpVolState == eCddCPVolState_6V || eCpVolState == eCddCPVolState_9V)
    {
        ret = TRUE;
    }

    return ret;
}

uint32_t AswChargeIf_GetOutputVoltage(uint8_t port)
{
    uint32_t outputVol = 0;

    if (CddRelay_GetRelayState(port) == eCddRelayState_On)
    {
        outputVol = CddMeterM_GetRmsVoltage(port);
    }

    return outputVol;
}

uint32_t AswChargeIf_GetInputVoltage(uint8_t port)
{
    return CddMeterM_GetRmsVoltage(port);;
}

uint32_t AswChargeIf_GetOutputPower(uint8_t port)
{
    uint32_t power = 0;

    if (CddRelay_GetRelayState(port) == eCddRelayState_On)
    {
        power = CddMeterM_GetPower(port);
    }

    return power;
}

uint16_t AswChargeIf_GetCpVoltage(uint8_t port)
{
    return CddCP_GetVoltage(port);
}

uint16_t AswChargeIf_GetCpDuty(uint8_t port)
{
    return CddCP_GetCpDuty(port);
}

uint32_t AswChargeIf_GetOutputCurrent(uint8_t port)
{
    uint32_t outputCurrent = 0;

    if (CddRelay_GetRelayState(port) == eCddRelayState_On)
    {
        outputCurrent = CddMeterM_GetRmsCurrent(port);
    }

    return outputCurrent;
}

uint8_t AswChargeIf_GetChargeState(uint8_t port)
{
    return AswCharge_GetWorkState(port);
}

uint8_t AswChargeIf_GetRelayState(uint8_t port)
{
    return (uint8_t)CddRelay_GetRelayState(port);
}

uint8_t AswChargeIf_GetGunTemperature(uint8_t port)
{
    return CddSensor_GetGunTemperature(port);
}

uint8_t AswChargeIf_GetEnvTemperature(void)
{
    return CddSensor_GetEnvTemperature();
}

uint64_t AswChargeIf_GetMeterEnergyVal(uint8_t port)
{
    return CddMeterM_GetEnergyVal(port);
}

void AswChargeIf_ChargeStart(uint8_t port)
{
    AswCharge_StartAuth(port);
}

AswErrorType_Enum AswChargeIf_GetStopReason(uint8_t port)
{
    return AswCharge_GetStopReason(port);
}

uint8_t AswChargeIf_GetAuthFlag(uint8_t port)
{
    return AswCharge_GetAuthFlag(port);
}

void AswChargeIf_AdjustOutputCurrent(uint8_t port, uint8_t adjustMode, uint32_t val)
{
    switch (adjustMode)
    {
        case ASWCHARGEIF_ADJUST_POWER_ABSOLUTE:
            AswVoltCur_AdjustOutputCurrent(port, eAswVoltCurAdjustMode_PowerAbsolute, val);
            break;
        case ASWCHARGEIF_ADJUST_POWER_RELATIVE:
            AswVoltCur_AdjustOutputCurrent(port, eAswVoltCurAdjustMode_PowerPercent, val);
            break;
        default:
            break;
    }
}















