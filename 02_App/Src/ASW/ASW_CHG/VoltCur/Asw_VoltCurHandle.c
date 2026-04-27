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

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t  lastAuthStatus;

    uint8_t  setCurrentValid;
    uint32_t setOutputCurrent;

    uint32_t limitOutputCurrent;

    uint32_t maxOutputCurrent;
	uint32_t prevMaxOutputCurrent;
    uint8_t arFaultStatus[AswVoltCur_EvtCnt];
    FilterProfile1_Struct arfilter[AswVoltCur_EvtCnt];
}AswVoltCurHandle_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswVoltCurHandle_Struct g_arAswVoltCurHandle[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswVoltCur_ErrorManage(uint8_t port)
{
    uint8_t i = 0;
    uint8_t result = FALSE;
    uint8_t tempStatus = 0;
    uint32_t tmpVoltCurr = 0;
    AswVoltCurHandle_Struct *pHandle = &g_arAswVoltCurHandle[port];

    for (i = 0; i < AswVoltCur_EvtCnt; i++)
    {
        if (c_AswVoltCurHandleConfigTable[i].type == AswVoltType)
        {
            tmpVoltCurr = CddMeterM_GetRmsVoltage(port);
        }
        else
        {
            tmpVoltCurr = CddMeterM_GetRmsCurrent(port);
        }

        switch (c_AswVoltCurHandleConfigTable[i].compareType)
        {
            case AswVoltCurCmp_MaxEqu:
            {
                tempStatus = CHECK_MAX_EQU(tmpVoltCurr, c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCurCmp_Max:
            {
                tempStatus = CHECK_MAX(tmpVoltCurr, c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCurCmp_MinEqu:
            {
                tempStatus = CHECK_MIN_EQU(tmpVoltCurr, c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCurCmp_Min:
            {
                tempStatus = CHECK_MIN(tmpVoltCurr, c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCurCmp_Equ:
            {
                tempStatus = CHECK_EQU(tmpVoltCurr, c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            default:
            {
                break;
            }
        }

        pHandle->arfilter[i].status = tempStatus;
        result = Filter_Profile1(&pHandle->arfilter[i], c_AswVoltCurHandleConfigTable[i].filterCount);
     	
		if (pHandle->arfilter[i].validStatus == TRUE)
		{
			if (c_AswVoltCurHandleConfigTable[i].setErrFlag == TRUE && pHandle->arFaultStatus[i] == FALSE)
			{
				pHandle->arFaultStatus[i] = TRUE;
				AswErrhandle_SetErrExsitCallback(port, c_AswVoltCurHandleConfigTable[i].errType);
			}
			if (c_AswVoltCurHandleConfigTable[i].setErrFlag == FALSE)
			{
				CddCPVolState_Enum eCPState = CddCP_GetVolState(port);
				if (pHandle->arFaultStatus[i] == FALSE && eCPState == eCddCPVolState_12V)
				{/*拔枪状态且满足阈值条件*/
					pHandle->arFaultStatus[i] = TRUE;
					AswErrhandle_ResetErrExsitCallback(port, c_AswVoltCurHandleConfigTable[i].errType);
				}
			}
		}
		else
		{
			pHandle->arFaultStatus[i] = FALSE;
		}	
    }
}

static void AswVoltCur_LimitManage(uint8_t port)
{
    LimitCurrentLevel_Enum level = AswLimitCurrLevelZero;
    uint8_t authStatus = 0;
    AswVoltCurHandle_Struct *pHandle = NULL;

	if (port < SYSCFG_CFG_GUN_NUM)
	{
        pHandle = &g_arAswVoltCurHandle[port];
        authStatus = ASW_VOLTCUR_CFG_IsAuthState(port);
        level = AswTempHandle_GetLimitCurrentLevel(port);

        if (pHandle->maxOutputCurrent != pHandle->prevMaxOutputCurrent)
        {
            ASWVOLTCUR_CFG_DebugPrint("当前额定电流发生变化：%d.%03d --->%d.%03d A\r\n", pHandle->prevMaxOutputCurrent / 1000, 
            pHandle->prevMaxOutputCurrent % 1000, pHandle->maxOutputCurrent / 1000, pHandle->maxOutputCurrent % 1000);
            pHandle->prevMaxOutputCurrent = pHandle->maxOutputCurrent;
            CddCP_AdjustCurRateCurrent(port, pHandle->maxOutputCurrent);
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
                pHandle->limitOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
            }
            else
            {
                pHandle->limitOutputCurrent = (SYSCFG_CFG_MAX_OUTPUT_CURRENT * 80) / 100;
            }

            if (pHandle->setCurrentValid == TRUE)
            {
                if (pHandle->setOutputCurrent <= pHandle->limitOutputCurrent)
                {
                    pHandle->maxOutputCurrent = pHandle->setOutputCurrent;
                }
                else
                {
                    pHandle->maxOutputCurrent = pHandle->limitOutputCurrent;
                }
            }
            else
            {
                pHandle->maxOutputCurrent = pHandle->limitOutputCurrent;
            }
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
            ASWVOLTCUR_CFG_DebugPrint("远程调节功率，调节模式：绝对值调节，调节功率为：%dW\r\n", val);

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
            ASWVOLTCUR_CFG_DebugPrint("远程调节功率，调节模式：百分比调节，调节百分比为：%d.%d%%\r\n", val / 10, val % 10);
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
    }
}

void AswVoltCurHandle_MainFunction(void)
{
    uint8_t port = 0;
    uint8_t readFlag = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        readFlag = CddMeterM_GetReadyFlag(port);
        if (readFlag == TRUE)
        {
            AswVoltCur_ErrorManage(port);
            AswVoltCur_LimitManage(port);
        }
    }
}

