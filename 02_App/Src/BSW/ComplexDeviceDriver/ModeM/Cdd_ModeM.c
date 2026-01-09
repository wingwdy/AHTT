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
#include "Cdd_ModeM.h"
#include "Filter.h"
#include "Cdd_MeterM.h"
#include "Cdd_ModeMConfig.h"
#include "MS_Nvm.h"
#include "SS_Tm.h"
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
    FilterProfile1_Struct fctPinStatusFilter;
    uint8_t isAgingTestStart;
    uint32_t agingTestTickStart;
    MSNvmModeParam_Struct modeParam;
}CddModeMCtx_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddModeMCtx_Struct  g_stCddModeMCtx;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddModeM_AgingTestHandle(void)
{
    if (g_stCddModeMCtx.modeParam.isAgingTestFinish != TRUE)
    {
        if (g_stCddModeMCtx.isAgingTestStart != TRUE)
        {
            if (CddMeterM_GetRmsCurrent(0) > CDD_MODEM_CFG_AGING_TEST_CURRENT_THRESHOLD)
            {
                g_stCddModeMCtx.isAgingTestStart = TRUE;
                g_stCddModeMCtx.agingTestTickStart = Common_GetSystick();
            }
        }
        else
        {
            if (CddMeterM_GetRmsCurrent(0) > CDD_MODEM_CFG_AGING_TEST_CURRENT_THRESHOLD)
            {
                if (Common_JudgeTimeoutMs(g_stCddModeMCtx.agingTestTickStart, CDD_MODEM_CFG_AGING_TEST_TIMEOUT))
                {
                    g_stCddModeMCtx.modeParam.isAgingTestFinish = TRUE;
                    MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
                }
            }
            else
            {
                g_stCddModeMCtx.isAgingTestStart = FALSE;
            }
        }
    }
}

static uint8_t CddModeM_CheckEnterFactoryModeCondition(void)
{
    if (TRUE == CDD_MODEM_CFG_GetFCTPin())
    {
        g_stCddModeMCtx.fctPinStatusFilter.status = TRUE;
    }
    else
    {
        g_stCddModeMCtx.fctPinStatusFilter.status = FALSE;
    }

    Filter_Profile1(&g_stCddModeMCtx.fctPinStatusFilter, CDD_MODEM_CFG_FCT_FILTER_POINT);

    return g_stCddModeMCtx.fctPinStatusFilter.validStatus;
}

static uint8_t CddModeM_CheckExsitFactoryModeCondition(void)
{
    uint32_t currentTimeStamp = 0;
    uint8_t exsitFlag = FALSE;
    uint32_t timeDiff = 0;

    if (g_stCddModeMCtx.modeParam.isSynTime == FALSE)
    {
        if (SSTM_GetSyncTimeFlag() == TRUE)
        {
            g_stCddModeMCtx.modeParam.isSynTime = TRUE;
            g_stCddModeMCtx.modeParam.sysTimeStamp = SSTM_GetSecTimestamp();
            MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
        }
    }
    else
    {
        if (SSTM_GetSyncTimeFlag() == TRUE)
        {
            currentTimeStamp = SSTM_GetSecTimestamp();

            if (currentTimeStamp >= g_stCddModeMCtx.modeParam.sysTimeStamp)
            {
                timeDiff = currentTimeStamp - g_stCddModeMCtx.modeParam.sysTimeStamp;
            }
            else
            {
                timeDiff = (0xFFFFFFFF - g_stCddModeMCtx.modeParam.sysTimeStamp) + currentTimeStamp;
            }

            if (timeDiff >= CDD_MODEM_CFG_FACTORY_TIMEOUT)
            {
                CDDMODE_CFG_LogPrint("进入产线超过72小时!!\r\n");
                exsitFlag = TRUE;
            }
        }
    }

    return exsitFlag;
}

void CddModeM_InitMemory(void)
{
    memset(&g_stCddModeMCtx, 0x00, sizeof(g_stCddModeMCtx));

    if (eGlobalRet_OK != MSNvm_ReadParaBlock(eMSNvmBlockID_ModeParam, 
        (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct)))
    {
        CDDMODE_CFG_LogPrint("读取模式参数失败!\r\n");
    }
}

void CddModeM_MainFunction(void)
{
    if (g_stCddModeMCtx.modeParam.isFactoryMode == TRUE)
    {
        CddModeM_AgingTestHandle();

        if (TRUE == CddModeM_CheckExsitFactoryModeCondition())
        {
            CddModeM_ExsitFactoryMode();
        }
    }
    else
    {
        if (TRUE == CddModeM_CheckEnterFactoryModeCondition())
        {
            CddModeM_EnterFactoryMode();
        }
    }
}

void CddModeM_EnterFactoryMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isFactoryMode == FALSE)
    {
        CDDMODE_CFG_LogPrint("进入厂内模式!\r\n");
        g_stCddModeMCtx.modeParam.isFactoryMode = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDMODE_CFG_LogPrint("已在厂内模式!\r\n");
    }
}

void CddModeM_ExsitFactoryMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isFactoryMode == TRUE)
    {
        CDDMODE_CFG_LogPrint("退出厂内模式!\r\n");
        g_stCddModeMCtx.modeParam.isFactoryMode = FALSE;
        g_stCddModeMCtx.modeParam.isSynTime = FALSE;
        g_stCddModeMCtx.modeParam.sysTimeStamp = 0;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDMODE_CFG_LogPrint("已退出厂内模式!\r\n");
    }
}

void CddModeM_ExitGBMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isQBMode == FALSE)
    {
        CDDMODE_CFG_LogPrint("进入兼容模式!\r\n");
        g_stCddModeMCtx.modeParam.isQBMode = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDMODE_CFG_LogPrint("已在兼容模式!\r\n");
    }
}

void CddModeM_EnterGBMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isQBMode == TRUE)
    {
        CDDMODE_CFG_LogPrint("进入国标模式!\r\n");
        g_stCddModeMCtx.modeParam.isQBMode = FALSE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDMODE_CFG_LogPrint("已在国标模式!\r\n");
    }
}

uint8_t CddModeM_IsFactoryMode(void)
{  
    return g_stCddModeMCtx.modeParam.isFactoryMode;
}

uint8_t CddModeM_IsGBMode(void)
{
    return (g_stCddModeMCtx.modeParam.isQBMode == FALSE);
}

uint8_t CddModeM_IsAgingTestFinish(void)
{
    return g_stCddModeMCtx.modeParam.isAgingTestFinish;
}









