/******************************************************************************
* File Name          : Cdd_Rcd.c
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
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "SysCfg.h"
#include "Cdd_Rcd.h"
#include "Filter.h"
#include "Asw_ErrorHandle.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define MCAL_BIT_SET                        SET
#define MCAL_BIT_RESET                      RESET


/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
	eRcdState_Idle = 0u,
    eRcdState_SelfCheck,
	eRcdState_Work,
} RcdStatus_Enum;

typedef enum
{
    eRCDTest_Idle = 0u,
	eRCDCalib_Open,
    eRCDCalib_Wait,
	eRCDCalib_Close,
    eRCDCalib_Exit,
	eRCDTest_Open,
    eRCDTest_Wait,
    eRCDTest_Ready,
    eRCDTest_Exit,
} RcdSelfTestStep_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
	RcdStatus_Enum eRcdStatus;
	RcdSelfTestStep_Enum eSelfCheckStep;
	uint8_t stepWaitCnt;
    uint16_t rcdSelfCheckWaitCnt;
    uint8_t rcdSelfCheckStatus;
	uint8_t rcdFaultStatus;
} CddRcd_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddRcd_Struct g_stCddRcd[SYSCFG_CFG_GUN_NUM] = {0};
static TimerHandle_t g_RcdTimerHandle;
static uint32_t g_TimerId;
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void CddRcd_SelfCheckHandle(uint8_t port, CddRcd_Struct *pRcdCtrl);
static void CddRcd_WorkHandle(uint8_t port, CddRcd_Struct *pRcdCtrl);
static void CddRcd_Timer1msCallback(void);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddRcd_WorkHandle(uint8_t port, CddRcd_Struct *pRcdCtrl)
{
	uint8_t TrigState = 0;

    if (pRcdCtrl->rcdSelfCheckStatus == GLOBAL_OPT_STATE_SUCCESS)
    {
		if(eGlobalRet_OK == Filter_IO_GetVal(eFilterIOChannel_RCD, &TrigState))
		{
			if (TrigState)
			{
				if(GLOBAL_OPT_STATE_FAIL != pRcdCtrl->rcdFaultStatus)
				{
					pRcdCtrl->rcdFaultStatus = GLOBAL_OPT_STATE_FAIL;
					AswErrhandle_SetErrExsitCallback(port, eErr_LeakageCurrErr);
				}
			}
		}
    }
}

static void CddRcd_SelfCheckHandle(uint8_t port, CddRcd_Struct *pRcdCtrl)
{
	uint8_t TrigSignalState = 0;

	switch(pRcdCtrl->eSelfCheckStep)
	{
		case eRCDCalib_Open:
		{/* RCD CAL置低 */
            CDDRCD_CFG_ResetTestPin();
			CDDRCD_CFG_SetCalPin();
			pRcdCtrl->stepWaitCnt = 0;
			pRcdCtrl->eSelfCheckStep = eRCDCalib_Wait;
			break;
		}
		case eRCDCalib_Wait:
		{/* RCD CAL置低 50ms <= T2 <= 100ms */
			pRcdCtrl->stepWaitCnt++;
			if (pRcdCtrl->stepWaitCnt >= CDDRCD_CFG_SELFCHECK_CAL_T2_TIME)
			{
				pRcdCtrl->stepWaitCnt = 0u;
				pRcdCtrl->eSelfCheckStep = eRCDCalib_Close;
			}

			break;
		}
        case eRCDCalib_Close:
        {/* RCD CAL置高 */
            CDDRCD_CFG_ResetCalPin();
            pRcdCtrl->stepWaitCnt = 0;
            pRcdCtrl->eSelfCheckStep = eRCDCalib_Exit;
			break;
        }
        case eRCDCalib_Exit:
        {/* RCD CAL置高 T3 >= 500ms */
			pRcdCtrl->stepWaitCnt++;
			if (pRcdCtrl->stepWaitCnt >= CDDRCD_CFG_SELFCHECK_CAL_T3_TIME)
			{
				pRcdCtrl->stepWaitCnt = 0u;
				pRcdCtrl->eSelfCheckStep = eRCDTest_Open;
			}

			break;
        }
        case eRCDTest_Open:
        {/* RCD TEST置高 */
            CDDRCD_CFG_SetTestPin();
            pRcdCtrl->stepWaitCnt = 0;
            pRcdCtrl->eSelfCheckStep = eRCDTest_Wait;
			break;
        }
        case eRCDTest_Wait:
        {/* RCD TEST置高 T5 >= 120ms */
			pRcdCtrl->stepWaitCnt++;
			if (pRcdCtrl->stepWaitCnt >= CDDRCD_CFG_SELFCHECK_TEST_T5_TIME)
			{
				pRcdCtrl->stepWaitCnt = 0u;
				pRcdCtrl->eSelfCheckStep = eRCDTest_Ready;
                pRcdCtrl->rcdSelfCheckWaitCnt = 0;
			}

			break;
        }
		case eRCDTest_Ready:
		{
			pRcdCtrl->stepWaitCnt++;
			pRcdCtrl->rcdSelfCheckWaitCnt++;

			/* 自检漏电检测间隔 50ms */
			if (pRcdCtrl->stepWaitCnt >= CDDRCD_CFG_SELFCHECK_CHK_INT_TIME)
			{
				pRcdCtrl->stepWaitCnt = 0u;
				
				if(eGlobalRet_OK == Filter_IO_GetVal(eFilterIOChannel_RCD, &TrigSignalState))
				{
					if (TrigSignalState)
					{
						pRcdCtrl->rcdSelfCheckStatus = GLOBAL_OPT_STATE_SUCCESS;
                    	pRcdCtrl->eSelfCheckStep = eRCDTest_Exit;
					}
				}
			}

			/* 自检超时 2s */
			if(pRcdCtrl->rcdSelfCheckWaitCnt >= CDDRCD_CFG_SELFCHECK_CHK_MAX_TIME)
			{
				AswErrhandle_SetErrExsitCallback(port, eErr_RCDSelfcheckErr);
				pRcdCtrl->rcdSelfCheckStatus = GLOBAL_OPT_STATE_FAIL;
				pRcdCtrl->eSelfCheckStep = eRCDTest_Exit;
				pRcdCtrl->stepWaitCnt = 0;
			}

			break;
		}
        case eRCDTest_Exit:
        {/* RCD TEST置低 T6 >= 100ms */
			pRcdCtrl->stepWaitCnt++;
			if(pRcdCtrl->stepWaitCnt >= CDDRCD_CFG_SELFCHECK_EXIT_T6_TIME)
            {
                CDDRCD_CFG_ResetTestPin();
                pRcdCtrl->stepWaitCnt = 0;
				pRcdCtrl->eRcdStatus = eRcdState_Work;
                pRcdCtrl->eSelfCheckStep = eRCDTest_Idle;
            }

			break;
        }
		default:
		{
			pRcdCtrl->eSelfCheckStep = eRCDTest_Idle;
			break;
		}
	}
}

static void CddRcd_Timer1msCallback(void)
{
	uint8_t TrigState = 0;

	if (MCAL_BIT_SET == CDDRCD_CFG_GetTripPin())
	{
		TrigState = 1;
	}

	Filter_IO_InsertFIFO(eFilterIOChannel_RCD, TrigState);
}

void CddRcd_InitMemory(void)
{
	memset(g_stCddRcd, 0, sizeof(g_stCddRcd));
	g_RcdTimerHandle = 0;
    g_TimerId = 0;

	if (eGlobalRet_OK != Filter_IO_CreatFIFO(eFilterIOChannel_RCD, 5, 0))
	{
		RCD_DEBUG("RCD FilterIO Create Fail\r\n");
	}

	g_RcdTimerHandle = xTimerCreate( "Timer1ms", 1, pdTRUE, &g_TimerId, (void *)CddRcd_Timer1msCallback);

	if (g_RcdTimerHandle == NULL)
	{
		RCD_DEBUG("RCD Timer Create Fail\r\n");
	}
	else
	{
		xTimerStart(g_RcdTimerHandle, 0);
	}
}

uint8_t CddRcd_ReqSelfCheck(uint8_t port)
{
 	CddRcd_Struct *pRcdCtrl = &g_stCddRcd[port];

	if (eRcdState_Idle == pRcdCtrl->eRcdStatus)
	{
		pRcdCtrl->eRcdStatus = eRcdState_SelfCheck;
		pRcdCtrl->rcdSelfCheckStatus = GLOBAL_OPT_STATE_PROCESS;
		pRcdCtrl->rcdFaultStatus = GLOBAL_OPT_STATE_IDLE;
		pRcdCtrl->eSelfCheckStep = eRCDCalib_Open;
	}
	else
	{
		pRcdCtrl->rcdSelfCheckStatus = GLOBAL_OPT_STATE_FAIL;
	}

	return pRcdCtrl->rcdSelfCheckStatus;
}

void CddRcd_StateManage(uint8_t port, CddRcd_Struct *pRcdCtrl)
{
	if(pRcdCtrl->eRcdStatus != eRcdState_Idle)
	{
		switch(pRcdCtrl->eRcdStatus)
		{
		case eRcdState_SelfCheck:
		{
			CddRcd_SelfCheckHandle(port, pRcdCtrl);
			break;
		}

		case eRcdState_Work:
		{
			CddRcd_WorkHandle(port, pRcdCtrl);
			break;
		}
		
		default:
		{
			pRcdCtrl->eRcdStatus = eRcdState_Idle;
			break;
		}	
		}
	}
}

void CddRcd_MainFunction(void)
{
	CddRcd_Struct *pRcdCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pRcdCtrl = &g_stCddRcd[port];
        CddRcd_StateManage(port, pRcdCtrl);
    }
}




