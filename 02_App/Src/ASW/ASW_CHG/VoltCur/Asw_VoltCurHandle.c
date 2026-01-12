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
    uint32_t maxOutputCurrent;
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
void AswVoltCurHandle_InitMemory(void)
{
    memset(g_arAswVoltCurHandle, 0, sizeof(g_arAswVoltCurHandle));
}

void AswVoltCur_ErrorManage(uint8_t port)
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

void AswVoltCur_LimitManage(uint8_t port)
{
    LimitCurrentLevel_Enum Level = AswLimitCurrLevelZero;
    uint32_t current = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
    AswVoltCurHandle_Struct *pHandle = &g_arAswVoltCurHandle[port];

    pHandle->maxOutputCurrent = SYSCFG_CFG_MAX_OUTPUT_CURRENT;

    Level = AswTempHandle_GetLimitCurrentLevel(port);
    if ((Level == AswLimitCurrLevelOne) && ASW_VOLTCUR_CFG_ISAuthState())
    {
        current = (SYSCFG_CFG_MAX_OUTPUT_CURRENT * 80) / 100;
    }

    if (pHandle->maxOutputCurrent != current)
    {
        pHandle->maxOutputCurrent = current;
        CddCP_AdjustCurRateCurrent(port, pHandle->maxOutputCurrent);
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

