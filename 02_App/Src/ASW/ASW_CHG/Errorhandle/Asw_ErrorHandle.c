/******************************************************************************
* File Name          : Asw_ErrorHandle.c
* Description        : Code for Errorhandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      chenls    初版创建
*
*******************************************************************************/
#include "Asw_ErrorHandleConfig.h"
#include "SysCfg.h"


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
typedef struct 
{
    uint8_t arErrLevelCnt[AswErrorLevel_Cnt];
    uint8_t arErrOccurCnt[eErr_Num];
    uint8_t arErrFlag[eErr_Num];
    uint8_t arErrLevel[eErr_Num];
    uint16_t arRecoverCnt[eErr_Num];
    AswErrChargeCondition_Enum eChargeCondition;
    uint32_t errStatusVersion;  /* 故障状态版本号 */
}AswErrorHandle_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswErrorHandle_Struct g_stAswErrorHandle[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void AswErrHandle_SelfRecoverDetect(uint8_t port);
static void AswErrHandle_RefreshChargeCondition(AswErrorHandle_Struct *pErrorHandle);
static void AswErrHandle_SetErrHandle(uint8_t port, AswErrorHandle_Struct *pErrorHandle, const AswErrorHandleConfig_Struct *pConfig);
static void AswErrHandle_ClearErrHandle(uint8_t port, AswErrorHandle_Struct *pErrorHandle, const AswErrorHandleConfig_Struct *pConfig);
static void AswErrHandle_UpdateErrStatusVersion(AswErrorHandle_Struct *pErrorHandle);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static void AswErrHandle_SetErrHandle(uint8_t port, AswErrorHandle_Struct *pErrorHandle, const AswErrorHandleConfig_Struct *pConfig)
{
    AswErrorType_Enum errType = pConfig->eError;

    if (pErrorHandle->arErrFlag[errType] != TRUE)
    {
        pErrorHandle->arErrFlag[errType] = TRUE;
        pErrorHandle->arErrLevelCnt[pConfig->tempErrLevel]++;
        pErrorHandle->arRecoverCnt[errType] = 0;

        if (pErrorHandle->arErrOccurCnt[errType] < pConfig->errCount)
        {
            pErrorHandle->arErrOccurCnt[errType]++;
        }

        if (pErrorHandle->arErrOccurCnt[errType] >= pConfig->errCount)
        { 
            pErrorHandle->arErrLevel[errType] = pConfig->finalLevel;
            pErrorHandle->arErrLevelCnt[pConfig->tempErrLevel]--;
            pErrorHandle->arErrLevelCnt[pConfig->finalLevel]++;
        }
        else
        {
            pErrorHandle->arErrLevel[errType] = pConfig->tempErrLevel;
        }
        
        ASWERR_CFG_InfoPrint("[枪：%d]故障：[%s] 产生\r\n", port, pConfig->errDesc);
        AswErrHandle_RefreshChargeCondition(pErrorHandle);
        AswErrHandle_UpdateErrStatusVersion(pErrorHandle);  /* 更新故障状态版本号 */
        ASWERR_CFG_ErrStateChangeNotice(port, errType, TRUE, pErrorHandle->arErrLevel[errType]);
    }
}

static void AswErrHandle_ClearErrHandle(uint8_t port, AswErrorHandle_Struct *pErrorHandle, const AswErrorHandleConfig_Struct *pConfig)
{
    AswErrorType_Enum errType = pConfig->eError;

    if (pErrorHandle->arErrFlag[errType] == TRUE)
    {
        pErrorHandle->arErrFlag[errType] = FALSE;

        if (pErrorHandle->arErrOccurCnt[errType] >= pConfig->errCount)
        {
            if (pErrorHandle->arErrLevelCnt[pConfig->finalLevel] > 0)
            {
                pErrorHandle->arErrLevelCnt[pConfig->finalLevel]--;
            }
        }
        else
        {
            if (pErrorHandle->arErrLevelCnt[pConfig->tempErrLevel] > 0)
            {
                pErrorHandle->arErrLevelCnt[pConfig->tempErrLevel]--;
            }
        }

        ASWERR_CFG_InfoPrint("[枪：%d]故障：[%s] 撤销\r\n", port, pConfig->errDesc);

        ASWERR_CFG_ErrStateChangeNotice(port, errType, FALSE, pErrorHandle->arErrLevel[errType]);
        pErrorHandle->arErrLevel[errType] = eAswErrorLevel_0;
        AswErrHandle_RefreshChargeCondition(pErrorHandle);
        AswErrHandle_UpdateErrStatusVersion(pErrorHandle);  /* 更新故障状态版本号 */
    }
}

static void AswErrHandle_SelfRecoverDetect(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    const AswErrorHandleConfig_Struct *pErrorConfig = NULL;
    AswErrorType_Enum errType;
    uint8_t index = 0;

    for (index = 0; index < ARRAY_SIZE(c_AswErrorHandleConfigTable); index++)
    {
        pErrorConfig = &c_AswErrorHandleConfigTable[index];
        errType = pErrorConfig->eError;

        if (pErrorHandle->arErrFlag[errType] == TRUE)
        {
            if (pErrorConfig->errClearType == eAswErrorClear_Internal)
            {
                pErrorHandle->arRecoverCnt[errType]++;

                if (pErrorHandle->arRecoverCnt[errType] >= pErrorConfig->recoveryTime)
                {
                    pErrorHandle->arRecoverCnt[errType] = 0;
                    AswErrHandle_ClearErrHandle(port, pErrorHandle, pErrorConfig);
                }
            }
        }
    }
}

static void AswErrHandle_RefreshChargeCondition(AswErrorHandle_Struct *pErrorHandle)
{
    if (pErrorHandle->arErrLevelCnt[eAswErrorLevel_1] > 0 ||
        pErrorHandle->arErrLevelCnt[eAswErrorLevel_5] > 0 ||
        pErrorHandle->arErrLevelCnt[eAswErrorLevel_6] > 0)
    {
        pErrorHandle->eChargeCondition = eErrChargeCondition_Cancel;
    }
    else if (pErrorHandle->arErrLevelCnt[eAswErrorLevel_3] > 0 ||
             pErrorHandle->arErrLevelCnt[eAswErrorLevel_4] > 0)
    {
        pErrorHandle->eChargeCondition = eErrChargeCondition_Suspend;
    }
    else
    {
        pErrorHandle->eChargeCondition = eErrChargeCondition_Allow;
    }
}

static void AswErrHandle_UpdateErrStatusVersion(AswErrorHandle_Struct *pErrorHandle)
{
    /* 简单地递增版本号 */
    pErrorHandle->errStatusVersion++;
    
    /* 防止溢出，当版本号达到最大值时重置为1 */
    if (pErrorHandle->errStatusVersion == 0)
    {
        pErrorHandle->errStatusVersion = 1;
    }
}

void AswErrHandle_InitMemory(void)
{
    memset(g_stAswErrorHandle, 0x00, sizeof(g_stAswErrorHandle));
}

void AswErrhandle_SetErrExsitCallback(uint8_t port, AswErrorType_Enum errType)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    const AswErrorHandleConfig_Struct *pConfig = &c_AswErrorHandleConfigTable[errType];
    uint8_t gunIndex = port;
  
    if ((port < SYSCFG_CFG_GUN_NUM) && (errType < eErr_Num))
    {
        if (pConfig->eErrOwner == eAswErrorOwner_Pile)
        {
            for (gunIndex = 0; gunIndex < SYSCFG_CFG_GUN_NUM; gunIndex++)
            {
                pErrorHandle = &g_stAswErrorHandle[gunIndex];
                AswErrHandle_SetErrHandle(gunIndex, pErrorHandle, pConfig);
            }
        }
        else
        {
            pErrorHandle = &g_stAswErrorHandle[gunIndex];
            AswErrHandle_SetErrHandle(gunIndex, pErrorHandle, pConfig);
        }
    }
}

void AswErrhandle_ResetErrExsitCallback(uint8_t port, AswErrorType_Enum errType)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    const AswErrorHandleConfig_Struct *pConfig = &c_AswErrorHandleConfigTable[errType];
    uint8_t gunIndex = port;

    if ((port < SYSCFG_CFG_GUN_NUM) && (errType < eErr_Num))
    {
        if (pConfig->eErrOwner == eAswErrorOwner_Pile)
        {
            for (gunIndex = 0; gunIndex < SYSCFG_CFG_GUN_NUM; gunIndex++)
            {
                pErrorHandle = &g_stAswErrorHandle[gunIndex];
                AswErrHandle_ClearErrHandle(gunIndex, pErrorHandle, pConfig);
            }
        }
        else
        {
            pErrorHandle = &g_stAswErrorHandle[gunIndex];
            AswErrHandle_ClearErrHandle(gunIndex, pErrorHandle, pConfig);
        }
    }
}

uint8_t AswErrHandle_CheckErrExit(uint8_t port, AswErrorType_Enum errType)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    uint8_t ret = FALSE;

    if ((port < SYSCFG_CFG_GUN_NUM) && (errType < eErr_Num))
    {
        ret = pErrorHandle->arErrFlag[errType];
    }

    return ret;
}

AswErrChargeCondition_Enum AswErrHandle_GetChargeCondition(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    AswErrChargeCondition_Enum eChargeCondition = eErrChargeCondition_Cancel;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        eChargeCondition = pErrorHandle->eChargeCondition;
    }

    return eChargeCondition;
}

void AswErrHandle_ClearNormalStopReason(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    uint8_t index = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        AswErrhandle_ResetErrExsitCallback(port, eErr_ChgStartTimeout);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_S2BreakOff);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_LittleCurr);

        AswErrhandle_ResetErrExsitCallback(port, eSrc_AppStop);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_CardStop);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_InsuffBalance);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_StopbyMoney);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_StopbyTime);
        AswErrhandle_ResetErrExsitCallback(port, eSrc_StopbyEnergy);
        AswErrhandle_ResetErrExsitCallback(port, eErr_GunDisConn);
        AswErrhandle_ResetErrExsitCallback(port, eErr_CPBreakOff);
    }
}

void AswErrHandle_MainFunction(void)
{
#if (ASWERR_CFG_MULTI_ENABLE == TRUE)
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswErrHandle_SelfRecoverDetect(port);
    }
#endif
}

uint8_t AswErrHandle_IsExsistError(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pErrorHandle->arErrLevelCnt[eAswErrorLevel_4] > 0 || 
            pErrorHandle->arErrLevelCnt[eAswErrorLevel_5] > 0 ||   
            pErrorHandle->arErrLevelCnt[eAswErrorLevel_6] > 0)
        {
            ret = TRUE;
        }   
    }

    return ret;
}

char *AswErrHandle_GetErrdesc(AswErrorType_Enum errType)
{
    const AswErrorHandleConfig_Struct *pConfig = &c_AswErrorHandleConfigTable[errType];
    char *pDesc = NULL;

    if (errType < eErr_Num)
    {
        pConfig = &c_AswErrorHandleConfigTable[errType];
        pDesc = pConfig->errDesc;
    }

    return pDesc;
}

AswErrorType_Enum AswErrHandle_GetExsistError(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = NULL;
    uint8_t index = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pErrorHandle = &g_stAswErrorHandle[port];

        for (index = 0; index < eErr_Num; index++)
        {
            if (pErrorHandle->arErrFlag[index] == TRUE)
            {
                if (pErrorHandle->arErrLevel[index] == eAswErrorLevel_1 ||
                    pErrorHandle->arErrLevel[index] == eAswErrorLevel_5 ||
                    pErrorHandle->arErrLevel[index] == eAswErrorLevel_6)
                {
                    break;
                }
            }
        }
    }

    return (AswErrorType_Enum)index;
}

uint32_t AswErrHandle_GetErrStatusVersion(uint8_t port)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    uint32_t version = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        version = pErrorHandle->errStatusVersion;
    }

    return version;
}

void AswErrHandle_SetErrStateForTest(uint8_t port, uint32_t errState)
{
    AswErrorHandle_Struct *pErrorHandle = &g_stAswErrorHandle[port];
    uint8_t index = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        for (index = 1; index < 32; index++)
        {
            if (errState & (1 << index))
            {
                AswErrhandle_SetErrExsitCallback(port, (AswErrorType_Enum)index);
            }
            else
            {
                AswErrhandle_ResetErrExsitCallback(port, (AswErrorType_Enum)index);
            }
        }
    }
}