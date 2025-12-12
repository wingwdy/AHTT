/******************************************************************************
* File Name          : Cdd_MeterM.c
* Description        : Code for Meter Manage
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
#include "Cdd_MeterM.h"
#include "Cdd_MeterMConfig.h"


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
void CddMeterM_InitMemory(void)
{
    c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncInitMemory();
}

void CddMeterM_MainFunction(void)
{
    c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncMainFunction();
}

uint8_t CddMeterM_GetReadyFlag(uint8_t port)
{
    return c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncGetReadyFlag(port);
} 

uint32_t CddMeterM_GetPower(uint8_t port)
{
    return c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncGetPower(port);
}
uint32_t CddMeterM_GetRmsVoltage(uint8_t port)
{
    return c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncGetRmsVoltage(port);
}
uint32_t CddMeterM_GetRmsCurrent(uint8_t port)
{
    return c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncGetRmsCurrent(port);
}
uint32_t CddMeterM_GetEnergyVal(uint8_t port)
{
    return 0;
}

















