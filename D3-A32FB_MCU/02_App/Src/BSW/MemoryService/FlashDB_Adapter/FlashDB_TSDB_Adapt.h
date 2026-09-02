/******************************************************************************
* File Name          : FlashDB_TSDB_Adapt.h
* Description        : Code for The adapter layer of TSDB
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef FLASHDB_TSDB_ADAPT_H_
#define FLASHDB_TSDB_ADAPT_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "MS_NvmAppTypes.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eTSDBAdaptChannel_ChargeRecord,
	eTSDBAdaptChannel_ErrorRecord,
	eTSDBAdaptChannel_RunningLog,
	eTSDBAdaptChannel_OmChargeRecord,
	eTSDBAdaptChannel_MeterRecord,
	eTSDBAdaptChannel_Count,
}TSDBAdaptChannel_Enum;



/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t TSDBAdapt_Init(void);
GlobalRet_Enum TSDBAdapt_CleanRecordDB(uint16_t ch);
uint32_t TSDBAdapt_QueryTotalRecordCount(uint16_t ch);
uint32_t TSDBAdapt_QueryUnreportedRecordCount(uint16_t ch);
GlobalRet_Enum TSDBAdapt_InsertRecord(uint16_t ch, uint8_t *pInBuf, uint16_t dataLen);
GlobalRet_Enum TSDBAdapt_SetRecordReportSuccess(uint16_t ch, uint32_t time);
GlobalRet_Enum TSDBAdapt_QueryLatestUnreportedRecord(uint16_t ch, uint8_t *pOutBuf, uint16_t dataLen, uint32_t *pTime);
GlobalRet_Enum TSDBAdapt_QueryLatestRecordTime(uint16_t ch, uint32_t *pTime);
GlobalRet_Enum TSDBAdapt_QueryRecordByTime(uint16_t ch, uint8_t *pOutBuf, uint16_t dataLen, uint32_t time);
GlobalRet_Enum TSDBAdapt_QueryRecordByExternal(uint16_t ch, uint8_t *para, uint16_t paraSize,
    pNvmCmpFunc pCmpFunc, uint8_t *pInBuf, uint16_t dataLen);
#endif






















