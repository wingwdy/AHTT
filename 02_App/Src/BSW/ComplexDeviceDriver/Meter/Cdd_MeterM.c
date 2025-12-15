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
#include "SysCfg.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint64_t lastCalcEnergy;
    uint64_t lastSaveEnergy;
    uint64_t totalEnergy;
    uint32_t periodCalcTick;
    uint32_t periodSaveTick;
}CddMeterM_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddMeterM_Struct g_stCddMeterM[SYSCFG_CFG_GUN_NUM];


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void CddMeterM_PeriodCalcEnergy(uint8_t port, CddMeterM_Struct *pstMeterM);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddMeterM_PeriodCalcEnergy(uint8_t port, CddMeterM_Struct *pstMeterM)
{ 
    uint64_t tempEnergy = 0;
    uint64_t incEnergy = 0;
    uint8_t saveFlag = FALSE;

    if (CddMeterM_GetReadyFlag(port) == TRUE)
    {
         if (Common_JudgeTimeoutMs(pstMeterM->periodCalcTick, CDD_METERM_CFG_ENERGY_PERIOD))
        {
            pstMeterM->periodCalcTick = Common_GetSystick();
            tempEnergy = c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncGetPeriodEnergy(port);
           
            if (tempEnergy > pstMeterM->lastCalcEnergy)
            {
                incEnergy = tempEnergy - pstMeterM->lastCalcEnergy;
                pstMeterM->totalEnergy += incEnergy;
            }

            pstMeterM->lastCalcEnergy = tempEnergy;

            if (pstMeterM->totalEnergy > pstMeterM->lastSaveEnergy)
            {
                if ((pstMeterM->totalEnergy - pstMeterM->lastSaveEnergy) > CDD_METERM_CFG_ENERGY_IMMEDIATE_SAVE_VALUE)
                {
                    saveFlag = TRUE;
                }

                if (Common_JudgeTimeoutMs(pstMeterM->periodSaveTick, CDD_METERM_CFG_ENERGY_SAVE_PERIOD))
                {
                    saveFlag = TRUE;
                }

                if (saveFlag == TRUE)
                {
                    pstMeterM->periodSaveTick = Common_GetSystick();
                    CDD_METERM_CFG_WriteBlockEnergy(port, (uint8_t *)&pstMeterM->totalEnergy, sizeof(pstMeterM->totalEnergy));
                    pstMeterM->lastSaveEnergy = pstMeterM->totalEnergy;
                }
            }
        }
    }
}

static void CddMeterM_LoadPara(void)
{ 
    CddMeterM_Struct *pstMeterM = NULL;
    uint8_t port = 0;
    GlobalRet_Enum eRet;
    
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pstMeterM = &g_stCddMeterM[port];

        CDD_METERM_CFG_ReadBlockEnergy(port, (uint8_t *)&pstMeterM->totalEnergy, sizeof(pstMeterM->totalEnergy), eRet);

        if (eRet == eGlobalRet_OK)
        {
            pstMeterM->lastSaveEnergy = pstMeterM->totalEnergy;
        }
        else
        {
            pstMeterM->totalEnergy = 0;
            pstMeterM->lastSaveEnergy = 0;
        }
    }
}

void CddMeterM_InitMemory(void)
{
    CddMeterM_LoadPara();
    c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncInitMemory();
}

void CddMeterM_MainFunction(void)
{
    uint8_t port = 0;

    c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_TYPE].pFuncMainFunction();

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        CddMeterM_PeriodCalcEnergy(port, &g_stCddMeterM[port]);
    }
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

















