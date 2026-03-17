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
#include "Asw_ErrorHandle.h"
#include "Cdd_CP.h"

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
    uint64_t lastCalcEnergy;    /*  上次计算的电能 小数点后4位，单位：kwh */
    uint64_t lastSaveEnergy;    /*  上次保存的电能 小数点后4位，单位：kwh */
    uint8_t  errCount;          /*  计量出错次数 */
    uint8_t  errFlag;           /*  计量错误标记 */
    uint64_t totalEnergy;       /*  累计总电能 小数点后4位，单位：kwh */
    uint32_t periodCalcTick;    /*  计算增量电能间隔计时 */
    uint32_t periodSaveTick;    /*  计算保存总电能间隔计时 */
    uint32_t cyclePrintTick;    /*  打印间隔 */
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
    CddCPVolState_Enum eCpVolState = CddCP_GetVolState(port);
    uint64_t tempEnergy = 0;
    uint64_t incEnergy = 0;
    uint8_t saveFlag = FALSE;
    uint32_t deltaEnergy = 0;

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
                deltaEnergy = pstMeterM->totalEnergy - pstMeterM->lastSaveEnergy;

                if (deltaEnergy > CDD_METERM_CFG_ENERGY_ABNORMAL_VALUE)
                {
                    pstMeterM->errCount++;
                }
                else
                {
                    if (deltaEnergy > CDD_METERM_CFG_ENERGY_IMMEDIATE_SAVE_VALUE)
                    {
                        saveFlag = TRUE;
                    }
                    
                    if (Common_JudgeTimeoutMs(pstMeterM->periodSaveTick, CDD_METERM_CFG_ENERGY_SAVE_PERIOD) && 
                        (deltaEnergy >= CDD_METERM_CFG_ENERGY_MIN_SAVE_VALUE))
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

    if (eCddCPVolState_12V != eCpVolState)
    {
        if (pstMeterM->errFlag == FALSE)
        {
            if (pstMeterM->errCount > CDD_METERM_CFG_ERROR_TRY_CNT)
            {
                pstMeterM->errFlag = TRUE;
                AswErrhandle_SetErrExsitCallback(port, eErr_MeterCalcErr);
            }
        }
    }
    else
    {
        if (pstMeterM->errFlag == TRUE)
        {
            pstMeterM->errFlag = FALSE;
            pstMeterM->errCount = 0;
            AswErrhandle_ResetErrExsitCallback(port, eErr_MeterCalcErr);
        }
    }
}

static void CddMeterM_LoadPara(void)
{ 
    CddMeterM_Struct *pstMeterM = NULL;
    uint8_t port = 0;
    GlobalRet_Enum eRet;
    uint32_t temp1, temp2;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pstMeterM = &g_stCddMeterM[port];

        CDD_METERM_CFG_ReadBlockEnergy(port, (uint8_t *)&pstMeterM->totalEnergy, sizeof(pstMeterM->totalEnergy), eRet);

        if (eRet == eGlobalRet_OK)
        {
            pstMeterM->lastSaveEnergy = pstMeterM->totalEnergy;
            temp1 = pstMeterM->totalEnergy / 10000;
            temp2 = pstMeterM->totalEnergy % 10000;
            CDD_METERM_CFG_LogPrint("[枪：%d]上电加载电表示值成功，累计已充电能：%ld.%04ld kwh \r\n", port, temp1, temp2);
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
uint64_t CddMeterM_GetEnergyVal(uint8_t port)
{
    uint64_t totalEnergy = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        totalEnergy = g_stCddMeterM[port].totalEnergy;
    }

    return totalEnergy;
}

















