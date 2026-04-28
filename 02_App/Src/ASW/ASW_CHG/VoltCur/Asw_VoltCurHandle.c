/******************************************************************************
* File Name          : Asw_VoltCurHandle.c
* Description        : Code for VoltCurHandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      shenjc    初版创建
*
*******************************************************************************/
#include "Asw_VoltCurHandleConfig.h"
#include "Asw_TempHandle.h"
#include "SysCfg.h"
#include "Filter.h"
#include "Asw_ErrorHandle.h"
#include "Cdd_MeterM.h"
#include "Cdd_CP.h"

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/

/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eAswVoltCurWorkState_Init,          /* 初始状态 */
    eAswVoltCurWorkState_Normal,        /* 正常状态 */
}AswVoltCurWorkState_Enum;

typedef enum
{
    eAswVoltCurErrorType_L1OverVoltage,
    eAswVoltCurErrorType_L1LessVoltage,
    eAswVoltCurErrorType_L1OverCurrent,
    eAswVoltCurErrorType_Count,
}AswVoltCurHandleState_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    AswVoltCurWorkState_Enum eWorkState;
    uint8_t  lastAuthStatus;

    uint8_t  setCurrentValid;       /* 更新过额定电流值标记 */
    uint32_t setOutputCurrent;      /* 当前设置电流值 */

    uint32_t maxOutputCurrent;
	uint32_t prevMaxOutputCurrent;

    uint32_t overCurrSetThreshold;      /* 过流阈值 */
    uint32_t overCurrClearThreshold;    /* 过流恢复阈值 */

    uint8_t arFaultStatus[eAswVoltCurErrorType_Count];
    FilterProfile1_Struct stErrorfilter[eAswVoltCurErrorType_Count];
}AswVoltCurHandle_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswVoltCurHandle_Struct g_arAswVoltCurHandle[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void AswVolCur_L1OverVoltageDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle);
static void AswVolCur_L1LessVoltageDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle);
static void AswVolCur_L1OverCurrentDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle);
static void AswVoltCur_WorkStateManage(uint8_t port, AswVoltCurHandle_Struct *pHandle);
static void AswVoltCur_UpdateOverCurrThreshold(uint8_t port, AswVoltCurHandle_Struct *pHandle);
static void AswVoltCur_CurrentLimitManage(uint8_t port, AswVoltCurHandle_Struct *pHandle);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswVolCur_L1OverVoltageDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    CddCPVolState_Enum eCPState;
    uint32_t tmpVolt = CddMeterM_GetRmsVoltage(port);
    FilterProfile1_Struct *pFilter = &pHandle->stErrorfilter[eAswVoltCurErrorType_L1OverVoltage];

    if (tmpVolt >= ASWVOLTCUR_CFG_SET_OV_THR)
    {
        pFilter->status = TRUE;
    }
    else if (tmpVolt <= ASWVOLTCUR_CFG_CLR_OV_THR)
    {
        pFilter->status = FALSE;
    } 
    else
    {
        pFilter->status = pFilter->validStatus;
    }

    if (Filter_Profile1(pFilter, ASWVOLTCUR_CFG_OV_FILTER_COUNT) == TRUE)
    {
        if (pFilter->validStatus == TRUE)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_AphaseInputOverVol);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverVoltage] = TRUE;
        }
    }

    if (pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverVoltage] == TRUE && pFilter->validStatus == FALSE)
    {
        eCPState = CddCP_GetVolState(port);

        if (eCPState == eCddCPVolState_12V)
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_AphaseInputOverVol);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverVoltage] = FALSE;
            memset(pFilter, 0, sizeof(FilterProfile1_Struct));
        }
    }
}

static void AswVolCur_L1LessVoltageDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    CddCPVolState_Enum eCPState;
    uint32_t tmpVolt = CddMeterM_GetRmsVoltage(port);
    FilterProfile1_Struct *pFilter = &pHandle->stErrorfilter[eAswVoltCurErrorType_L1LessVoltage];

    if (tmpVolt <= ASWVOLTCUR_CFG_SET_UV_THR)
    {
        pFilter->status = TRUE;
    }
    else if (tmpVolt >= ASWVOLTCUR_CFG_CLR_UV_THR)
    {
        pFilter->status = FALSE;
    } 
    else
    {
        pFilter->status = pFilter->validStatus;
    }

    if (Filter_Profile1(pFilter, ASWVOLTCUR_CFG_UV_FILTER_COUNT) == TRUE)
    {
        if (pFilter->validStatus == TRUE)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_AphaseInputLessVol);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1LessVoltage] = TRUE;
        }
    }

    if (pHandle->arFaultStatus[eAswVoltCurErrorType_L1LessVoltage] == TRUE && pFilter->validStatus == FALSE)
    {
        eCPState = CddCP_GetVolState(port);

        if (eCPState == eCddCPVolState_12V)
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_AphaseInputLessVol);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1LessVoltage] = FALSE;
            memset(pFilter, 0, sizeof(FilterProfile1_Struct));
        }
    }
}

static void AswVolCur_L1OverCurrentDetect(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    CddCPVolState_Enum eCPState;
    uint32_t tmpCurr = CddMeterM_GetRmsCurrent(port);
    FilterProfile1_Struct *pFilter = &pHandle->stErrorfilter[eAswVoltCurErrorType_L1OverCurrent];

    if (tmpCurr >= pHandle->overCurrSetThreshold)
    {
        pFilter->status = TRUE;
    }
    else if (tmpCurr <= pHandle->overCurrClearThreshold)
    {
        pFilter->status = FALSE;
    } 
    else
    {
        pFilter->status = pFilter->validStatus;
    }

    if (Filter_Profile1(pFilter, ASWVOLTCUR_CFG_OC_FILTER_COUNT) == TRUE)
    {
        if (pFilter->validStatus == TRUE)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_OutputOverCurr);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverCurrent] = TRUE;
        }
    }

    if (pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverCurrent] == TRUE)
    {
        eCPState = CddCP_GetVolState(port);

        if (eCPState == eCddCPVolState_12V)
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_OutputOverCurr);
            pHandle->arFaultStatus[eAswVoltCurErrorType_L1OverCurrent] = FALSE;
            memset(pFilter, 0, sizeof(FilterProfile1_Struct));
        }
    }
}

static void AswVoltCur_WorkStateManage(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    switch (pHandle->eWorkState)
    {
    case eAswVoltCurWorkState_Init:
    {
        if (CddMeterM_GetReadyFlag(port) == TRUE)
        {
            pHandle->eWorkState = eAswVoltCurWorkState_Normal;
        }

        break;
    }

    case eAswVoltCurWorkState_Normal:
    {
        AswVolCur_L1OverVoltageDetect(port, pHandle);
        AswVolCur_L1LessVoltageDetect(port, pHandle);
        AswVolCur_L1OverCurrentDetect(port, pHandle);
        AswVoltCur_CurrentLimitManage(port, pHandle);
        break;
    }

    default:
    {
        pHandle->eWorkState = eAswVoltCurWorkState_Init;
        memset(pHandle, 0, sizeof(AswVoltCurHandle_Struct));
        break;
    }
    }
}

static void AswVoltCur_UpdateOverCurrThreshold(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    uint32_t tempOverCurrSetThreshold = 0;
    uint32_t tempOverCurrClearThreshold = 0;
    
    if (pHandle->maxOutputCurrent != 0)
    {
        if (pHandle->maxOutputCurrent > 20000)
        {
            tempOverCurrSetThreshold = (pHandle->maxOutputCurrent * 110) / 100;
            tempOverCurrClearThreshold = tempOverCurrSetThreshold - 10;
        }
        else
        {
            tempOverCurrSetThreshold = pHandle->maxOutputCurrent + 2000;
            tempOverCurrClearThreshold = tempOverCurrSetThreshold - 10;
        }

        if (tempOverCurrSetThreshold != pHandle->overCurrSetThreshold || 
            tempOverCurrClearThreshold != pHandle->overCurrClearThreshold)
        {
            ASWVOLTCUR_CFG_InfoPrint("更新过流阈值：%d.%03d A --->%d.%03d A\r\n", pHandle->overCurrSetThreshold / 1000, 
            pHandle->overCurrSetThreshold % 1000, tempOverCurrSetThreshold / 1000, tempOverCurrSetThreshold % 1000);
            pHandle->overCurrSetThreshold = tempOverCurrSetThreshold;
            pHandle->overCurrClearThreshold = tempOverCurrClearThreshold;
        }
    }
}

static void AswVoltCur_CurrentLimitManage(uint8_t port, AswVoltCurHandle_Struct *pHandle)
{
    LimitCurrentLevel_Enum level = AswTempHandle_GetLimitCurrentLevel(port);;
    uint8_t authStatus = ASW_VOLTCUR_CFG_IsAuthState(port);
    uint32_t limitOutputCurrent = 0;

    if (pHandle->maxOutputCurrent != pHandle->prevMaxOutputCurrent)
    {
        ASWVOLTCUR_CFG_InfoPrint("当前额定电流发生变化：%d.%03d --->%d.%03d A\r\n", pHandle->prevMaxOutputCurrent / 1000, 
        pHandle->prevMaxOutputCurrent % 1000, pHandle->maxOutputCurrent / 1000, pHandle->maxOutputCurrent % 1000);
        pHandle->prevMaxOutputCurrent = pHandle->maxOutputCurrent;
        CddCP_AdjustCurRateCurrent(port, pHandle->maxOutputCurrent);
        AswVoltCur_UpdateOverCurrThreshold(port, pHandle);
    }

    if (authStatus != pHandle->lastAuthStatus)
    {
        if (authStatus == FALSE)
        {
            pHandle->maxOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
            pHandle->setCurrentValid = FALSE;
        }

        pHandle->lastAuthStatus = authStatus;
    }
    else
    {
        if (level != AswLimitCurrLevelOne)
        {
            limitOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
        }
        else
        {
            limitOutputCurrent = (SYSCFG_CFG_MAX_OUTPUT_CURRENT * 80) / 100;
        }

        if (pHandle->setCurrentValid == TRUE)
        {
            if (pHandle->setOutputCurrent <= limitOutputCurrent)
            {
                pHandle->maxOutputCurrent = pHandle->setOutputCurrent;
            }
            else
            {
                pHandle->maxOutputCurrent = limitOutputCurrent;
            }
        }
        else
        {
            pHandle->maxOutputCurrent = limitOutputCurrent;    
        }
    }
}

/* 
    当eMode = eAswVoltCurAdjustMode_PowerAbsolute时， val为绝对值，单位为W
    当eMode = eAswVoltCurAdjustMode_PowerPercent时，val为百分比 保留1位小数
*/
void AswVoltCur_AdjustOutputCurrent(uint8_t port, AswVoltCurAdjustMode_Enum eMode, uint32_t val)
{
    AswVoltCurHandle_Struct *pHandle = &g_arAswVoltCurHandle[port];

    switch (eMode)
    {
        case eAswVoltCurAdjustMode_PowerAbsolute:
        {
            ASWVOLTCUR_CFG_InfoPrint("远程调节功率，调节模式：绝对值调节，调节功率为：%dW\r\n", val);

            if (val == SYSCFG_CFG_MAX_OUTPUT_POWER)
            {
                pHandle->setOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
                pHandle->setCurrentValid = TRUE;
            }
            else if (val < SYSCFG_CFG_MAX_OUTPUT_POWER)
            {
                pHandle->setOutputCurrent = val * 1000 / 220;
                pHandle->setCurrentValid = TRUE;

                if (pHandle->setOutputCurrent < SYSCFG_CFG_MIN_OUTPUT_CURRENT)
                {
                    pHandle->setOutputCurrent = 0;
                }
            }
            else
            {}

            break;
        }
        case eAswVoltCurAdjustMode_PowerPercent:
        {
            ASWVOLTCUR_CFG_InfoPrint("远程调节功率，调节模式：百分比调节，调节百分比为：%d.%d%%\r\n", val / 10, val % 10);
            if (val == 1000)
            {
                pHandle->setOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
                pHandle->setCurrentValid = TRUE;
            }
            else if (val < 1000)
            {
                pHandle->setOutputCurrent = val * SYSCFG_CFG_MAX_OUTPUT_POWER / 220;
                pHandle->setCurrentValid = TRUE;

                if (pHandle->setOutputCurrent < SYSCFG_CFG_MIN_OUTPUT_CURRENT)
                {
                    pHandle->setOutputCurrent = 0;
                }
            }
            else
            {}

            break;
        }
        default:
        {
            break;
        }
    }
}

uint32_t AswVoltCurHandle_GetMaxOutputCurrent(uint8_t port)
{
    AswVoltCurHandle_Struct *pHandle = &g_arAswVoltCurHandle[port];
    uint32_t maxOutputCurrent = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        maxOutputCurrent = pHandle->maxOutputCurrent;
    }

    return maxOutputCurrent;
}

void AswVoltCurHandle_InitMemory(void)
{
    memset(g_arAswVoltCurHandle, 0, sizeof(g_arAswVoltCurHandle));

    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        g_arAswVoltCurHandle[port].prevMaxOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
        g_arAswVoltCurHandle[port].maxOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
        g_arAswVoltCurHandle[port].overCurrSetThreshold = ASWVOLTCUR_CFG_SET_OC_THR;
        g_arAswVoltCurHandle[port].overCurrClearThreshold = ASWVOLTCUR_CFG_CLR_OC_THR;
    }
}

void AswVoltCurHandle_MainFunction(void)
{
    AswVoltCurHandle_Struct *pHandle = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pHandle = &g_arAswVoltCurHandle[port];
        AswVoltCur_WorkStateManage(port, pHandle);
    }
}

