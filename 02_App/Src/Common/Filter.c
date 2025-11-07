/******************************************************************************
* File Name          : Filter.c
* Description        : Code for Filter function
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Filter.h"



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
	uint16_t curInsertCount;
	uint16_t curInserPos;
	uint16_t totalCount;
	uint8_t  ioArray[FILTER_FIFO_IO_POINT_COUNT];
	uint8_t  filterState;
	uint8_t  isFull;
}FilterIOCtrl_Struct;

typedef struct 
{
	uint8_t isUsed;
	FilterIOCtrl_Struct strCtrlBlk;
}FilterIOCtrlBlk_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static FilterIOCtrlBlk_Struct  g_filterIOCtrlBlkTable[FILTER_FIFO_IO_COUNT] = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t FIlter_IO_Check(uint8_t *pBuf, uint16_t count, uint8_t *pResultVal);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t FIlter_IO_Check(uint8_t *pBuf, uint16_t count, uint8_t *pResultVal)
{
	uint16_t index = 0;
	uint8_t ret = TRUE;

	if (pResultVal == NULL || pBuf == NULL || count < 1)
	{
		ret = FALSE;
	}
	else
	{
		for (index = 0; index < (count - 1); index++)
		{
			if (pBuf[index] != pBuf[index + 1])
			{
				ret = FALSE;
				break; 
			}
		}
	}

	if (ret == TRUE)
	{
		pResultVal[0] = pBuf[0];
	}

	return ret;
}

GlobalRet_Enum Filter_IO_CreatFIFO(uint8_t *pIOFilterID, uint8_t pointCount, uint8_t initVal)
{
	FilterIOCtrlBlk_Struct *pFilterIOCtrlBlk = NULL;
	uint8_t index = 0;
	GlobalRet_Enum eRet = eGlobalRet_NotEnoughFIFO;

	if ((pointCount > FILTER_FIFO_IO_POINT_COUNT) || (pIOFilterID == NULL) || (pointCount == 0))
	{
		eRet = eGlobalRet_ParaInvalid;
	}
	else
	{
		for (index = 0; index < FILTER_FIFO_IO_COUNT; index++)
		{
			pFilterIOCtrlBlk = &g_filterIOCtrlBlkTable[index];

			if (pFilterIOCtrlBlk->isUsed != TRUE)
			{
				memset(&pFilterIOCtrlBlk->strCtrlBlk, 0x00, sizeof(FilterIOCtrl_Struct));
				pFilterIOCtrlBlk->strCtrlBlk.curInsertCount = 0;
				pFilterIOCtrlBlk->strCtrlBlk.totalCount = pointCount;
				pFilterIOCtrlBlk->strCtrlBlk.filterState = initVal;
				pFilterIOCtrlBlk->strCtrlBlk.isFull = FALSE;
				pIOFilterID[0] = index;
				eRet = eGlobalRet_OK;
				pFilterIOCtrlBlk->isUsed = TRUE;
				break;
			}
		}
	}

	return eRet;
}

GlobalRet_Enum Filter_IO_InsertFIFO(uint8_t ioFilterID, uint8_t ioVal)
{
	FilterIOCtrlBlk_Struct *pFilterIOCtrlBlk = &g_filterIOCtrlBlkTable[ioFilterID];
	GlobalRet_Enum eRet = eGlobalRet_OK;

	if (ioFilterID > FILTER_FIFO_IO_COUNT)
	{
		eRet = eGlobalRet_ParaInvalid;
	}
	else 
	{
		if (pFilterIOCtrlBlk->isUsed != TRUE)
		{
			eRet = eGlobalRet_NotInit;
		}
		else
		{
			pFilterIOCtrlBlk->strCtrlBlk.ioArray[pFilterIOCtrlBlk->strCtrlBlk.curInsertCount++] = ioVal;

			if (pFilterIOCtrlBlk->strCtrlBlk.curInsertCount == pFilterIOCtrlBlk->strCtrlBlk.totalCount)
			{
				pFilterIOCtrlBlk->strCtrlBlk.isFull = TRUE;
				pFilterIOCtrlBlk->strCtrlBlk.curInsertCount = 0;
			}

			eRet = eGlobalRet_OK;
		}
	}

	return eRet;
}

GlobalRet_Enum Filter_IO_GetVal(uint8_t ioFilterID, uint8_t *pIoVal)
{
	FilterIOCtrlBlk_Struct *pFilterIOCtrlBlk = &g_filterIOCtrlBlkTable[ioFilterID];
	GlobalRet_Enum eRet = eGlobalRet_OK;
	uint16_t index = 0;
	uint8_t temp = 0;

	if (ioFilterID > FILTER_FIFO_IO_COUNT || pIoVal == 	NULL)
	{
		eRet = eGlobalRet_ParaInvalid;
	}
	else if (pFilterIOCtrlBlk->isUsed != TRUE)
	{
		eRet = eGlobalRet_NotInit;
	}
	else if (pFilterIOCtrlBlk->strCtrlBlk.isFull != TRUE)
	{
		eRet = eGlobalRet_FIFONotFull;
	}
	else
	{
		if (TRUE == FIlter_IO_Check(pFilterIOCtrlBlk->strCtrlBlk.ioArray, pFilterIOCtrlBlk->strCtrlBlk.totalCount, &temp))
		{
			pFilterIOCtrlBlk->strCtrlBlk.filterState = temp;
		}

		pIoVal[0] = pFilterIOCtrlBlk->strCtrlBlk.filterState;
		pFilterIOCtrlBlk->strCtrlBlk.isFull = FALSE;
	}

	return eRet;
}



















