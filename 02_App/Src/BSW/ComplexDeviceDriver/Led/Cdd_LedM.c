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


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_LedMConfig.h"



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
void CddLedM_InitMemory(void)
{
    uint8_t dev = 0;

    for (dev = 0; dev < CDD_LEDM_DEVICE_COUNT; dev++)
    {
        if (c_stCddLedMConfigTable[dev].pFuncInit != NULL)
        {
            c_stCddLedMConfigTable[dev].pFuncInit();
        }
    }
}

void CddLedM_MainFunction(void)
{
    uint8_t dev = 0;

    for (dev = 0; dev < CDD_LEDM_DEVICE_COUNT; dev++)
    {
        if (c_stCddLedMConfigTable[dev].pFuncMainFunction != NULL)
        {
            c_stCddLedMConfigTable[dev].pFuncMainFunction();
        }
    }
}

void CddLedM_UpdateState(uint8_t device, uint8_t port, uint8_t ledDispType)
{
    if (device < CDD_LEDM_DEVICE_COUNT)
    {
        if (c_stCddLedMConfigTable[device].pFuncUpdateLedDispType != NULL)
        {
            c_stCddLedMConfigTable[device].pFuncUpdateLedDispType(port, ledDispType);
        }
    }
}




















