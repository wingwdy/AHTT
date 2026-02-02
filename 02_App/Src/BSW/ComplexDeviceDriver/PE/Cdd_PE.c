/******************************************************************************
* File Name          : Cdd_Sensor.c
* Description        : Code for PE
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      shenjc    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_PE.h"
#include "SysCfg.h"
#include "Cdd_CP.h"
#include "Asw_ErrorHandle.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eCddPeErrState_NULL = 0, 
    eCddPeErrState_ERR,
}CddPeErrState_Enum;

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    CddLNState_Enum eLNState;                   /* LN状态 */
    CddPEState_Enum ePEState;                   /* PE状态 */
    CddLNState_Enum eTempLNState;               /* 临时LN状态 */
    CddPEState_Enum eTempPEState;               /* 临时PE状态 */
    CddPeErrState_Enum ePeErrState;             /* PE故障状态 */
    uint8_t lnCheckFinish;                      /* LN检测结束 */
    uint32_t lnCheckTimeout;                    /* LN检测超时定时器 */
    uint32_t peFilterTimer;                     /* PE滤波定时器 */
    uint16_t peVolt;							/* PE电压值，分辨率0.001V */
} CddPEState_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddPEState_Struct g_stPEState;
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static float CddPE_GetPEAdcVolt(void);
static void CddPE_Detect(void);
static void CddPE_PeDetect(CddPEState_Struct *pPECtrl);
static void CddPE_LnDetect(CddPEState_Struct *pPECtrl);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static float CddPE_GetPEAdcVolt(void)
{
    float peVolt = 0;
    uint16_t avgAdc = 0;
    uint16_t adcData[CDDPE_CFG_ADC_BUFF_NUM] = {0};

    McalADC_GetChannelData(eMcalADCChannel_PE, adcData, CDDPE_CFG_ADC_BUFF_NUM);

    avgAdc = Common_MedianU16Filter(adcData, CDDPE_CFG_ADC_BUFF_NUM, CDDPE_CFG_ADC_BUFF_NUM / 2);
    peVolt = (avgAdc / 4096.0) * 3.3;

    return peVolt;
}

static void CddPE_LnDetect(CddPEState_Struct *pPECtrl)
{
    CddLNState_Enum curLNState = eCddPEState_LNUnkown;

    if (CDDPE_CFG_IsQBStandardMode() == FALSE && pPECtrl->lnCheckFinish == FALSE) /* 国标模式检测LN */
    {
		if (pPECtrl->peVolt >= CDDPE_CFG_LN_REVERSE_MIN_VOLT && pPECtrl->peVolt <= CDDPE_CFG_LN_REVERSE_MAX_VOLT)
		{
			curLNState = eCddPEState_LNReverse;
		}
		else
		{
			curLNState = eCddPEState_LNNormal;
		}
		
		if (pPECtrl->eTempLNState != curLNState)
		{
			pPECtrl->eTempLNState = curLNState;
			pPECtrl->peFilterTimer = Common_GetSystick();
		}
		else
		{
			if (Common_JudgeTimeoutMs(pPECtrl->peFilterTimer, CDDPE_CFG_LN_REVERSE_FILTER_TIME)) /*火零反接检测滤波时间 */
			{
				pPECtrl->eLNState = curLNState;
			}
		}

		if (pPECtrl->eLNState != eCddPEState_LNUnkown)
		{
			pPECtrl->lnCheckFinish = TRUE;
			if (pPECtrl->eLNState == eCddPEState_LNReverse)
			{
				CDDPE_CFG_AswErrHandle_PileSetErrCallback(eErr_InputLineReversed);
			}
		}
		else
		{
			if (pPECtrl->lnCheckTimeout == 0)
			{
				pPECtrl->lnCheckTimeout = Common_GetSystick();
			}
			if (Common_JudgeTimeoutMs(pPECtrl->lnCheckTimeout, CDDPE_CFG_LN_REVERSE_CHECK_MAXTIME))/* LN反接检测超时 */
			{
				pPECtrl->eLNState = eCddPEState_LNNormal;
				pPECtrl->lnCheckFinish = TRUE;
			}
		}
    }
	else
	{
		if (CDDPE_CFG_IsQBStandardMode() == TRUE && pPECtrl->eLNState == eCddPEState_LNReverse)
		{
			pPECtrl->eLNState = eCddPEState_LNUnkown;
			CDDPE_CFG_AswErrHandle_PileSetErrCallback(eErr_InputLineReversed);
		}
	}
}

static void CddPE_PeDetect(CddPEState_Struct *pPECtrl)
{
    CddPEState_Enum curPEState = eCddPEState_PEUnkown;

	if (CDDPE_CFG_IsQBStandardMode() == FALSE && pPECtrl->lnCheckFinish == TRUE) /* 国标模式检测PE */
	{
		if (pPECtrl->eLNState == eCddPEState_LNNormal) /* 火零正常 */
		{
			if (pPECtrl->peVolt >= CDDPE_CFG_PE_UNCONN_MIN_VOLT && pPECtrl->peVolt <= CDDPE_CFG_PE_UNCONN_MAX_VOLT)
			{
				curPEState = eCddPEState_PEUnconn;
			}
			else
			{
				curPEState = eCddPEState_PEConnect;
			}

			if (pPECtrl->eTempPEState != curPEState)
			{
				pPECtrl->eTempPEState = curPEState;
				pPECtrl->peFilterTimer = Common_GetSystick();
			}
			else
			{
				if (Common_JudgeTimeoutMs(pPECtrl->peFilterTimer, CDDPE_CFG_PE_CHECK_FILTER_TIME))
				{
					if (curPEState != pPECtrl->ePEState)
					{
						pPECtrl->ePEState = curPEState;
						if (curPEState == eCddPEState_PEUnconn && pPECtrl->ePeErrState == eCddPeErrState_NULL)
						{
							pPECtrl->ePeErrState = eCddPeErrState_ERR;
							CDDPE_CFG_AswErrHandle_PileSetErrCallback(eErr_PEBreakFault);
						}
					}
					
					if (curPEState == eCddPEState_PEConnect && pPECtrl->ePeErrState == eCddPeErrState_ERR)
					{
						CddCPVolState_Enum eCPState = CddCP_GetVolState(0);
						if (eCPState == eCddCPVolState_12V)
						{
							pPECtrl->ePeErrState = eCddPeErrState_NULL;
							CDDPE_CFG_AswErrHandle_PileResetErrCallback(eErr_PEBreakFault);
						}
					}
				}
			}
		}	
	}
	else
	{
		if (pPECtrl->ePeErrState == eCddPeErrState_ERR)
		{
			pPECtrl->ePeErrState = eCddPeErrState_NULL;
			pPECtrl->ePEState = eCddPEState_PEUnkown;
			pPECtrl->eTempPEState = eCddPEState_PEUnkown;
			CDDPE_CFG_AswErrHandle_PileResetErrCallback(eErr_PEBreakFault);
		}
	}
}

static void CddPE_Detect(void)
{
    CddPEState_Struct *pPECtrl = &g_stPEState;

    pPECtrl->peVolt = (uint16_t)(CddPE_GetPEAdcVolt() * 1000.0);

	CddPE_LnDetect(pPECtrl);
	CddPE_PeDetect(pPECtrl);
}

CddPEState_Enum CddPE_GetConnectState(void)
{
	return g_stPEState.ePEState;
}


void CddPE_InitMemory(void)
{
    memset((uint8_t *)&g_stPEState, 0, sizeof(g_stPEState));
}

void CddPE_MainFunction(void)
{
    CddPE_Detect();
}



