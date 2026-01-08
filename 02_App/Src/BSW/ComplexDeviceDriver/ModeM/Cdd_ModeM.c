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




void CddModeM_InitMemory(void)
{
    memset(&g_stCddModeMCtx, 0x00, sizeof(g_stCddModeMCtx));

    if (eGlobalRet_OK != MSNvm_ReadParaBlock(eMSNvmBlockID_ModeParam, 
        (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct)))
    {
        CDDCP_CFG_LogPrint("读取模式参数失败!\r\n");
    }
}

void CddModeM_MainFunction(void)
{
    if (g_stCddModeMCtx.modeParam.isFactoryMode == TRUE)
    {
        CddModeM_AgingTestHandle(); 
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
        CDDCP_CFG_LogPrint("进入厂内模式!\r\n");
        g_stCddModeMCtx.modeParam.isFactoryMode = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDCP_CFG_LogPrint("已在厂内模式!\r\n");
    }
}

void CddModeM_ExsitFactoryMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isFactoryMode == TRUE)
    {
        CDDCP_CFG_LogPrint("退出厂内模式!\r\n");
        g_stCddModeMCtx.modeParam.isFactoryMode = FALSE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDCP_CFG_LogPrint("已退出厂内模式!\r\n");
    }
}

void CddModeM_ExitGBMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isGBMode == TRUE)
    {
        CDDCP_CFG_LogPrint("进入兼容模式!\r\n");
        g_stCddModeMCtx.modeParam.isGBMode = FALSE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDCP_CFG_LogPrint("已在兼容模式!\r\n");
    }
}

void CddModeM_EnterGBMode(void)
{ 
    if (g_stCddModeMCtx.modeParam.isGBMode == FALSE)
    {
        CDDCP_CFG_LogPrint("进入国标模式!\r\n");
        g_stCddModeMCtx.modeParam.isGBMode = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_ModeParam, (uint8_t *)&g_stCddModeMCtx.modeParam, sizeof(MSNvmModeParam_Struct));
    }
    else
    {
        CDDCP_CFG_LogPrint("已在国标模式!\r\n");
    }
}

uint8_t CddModeM_IsFactoryMode(void)
{
    return g_stCddModeMCtx.modeParam.isFactoryMode;
}

uint8_t CddModeM_IsGBMode(void)
{
    return g_stCddModeMCtx.modeParam.isGBMode;
}

uint8_t CddModeM_IsAgingTestFinish(void)
{
    return g_stCddModeMCtx.modeParam.isAgingTestFinish;
}









