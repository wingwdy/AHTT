/******************************************************************************
* File Name          : MS_Nvm.h
* Description        : Code for The core service layer for managing non-volatile data 
                       storage of the ECU
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
#ifndef MS_NVM_H_
#define MS_NVM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "MS_NvmAppTypes.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/

 
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    /* 参数信息 */
    eMSNvmBlockID_Gun0Qrcode,
    eMSNvmBlockID_Gun0OrderInfo,
    eMSNvmBlockID_Gun0MeterEnergy,
    eMSNvmBlockID_ModeParam,
	eMSNvmBlockID_Gun0MeterCaliParam,
	eMSNvmBlockID_PlatParam,
	eMSNvmBlockID_PlatPrivateParam,
    eMSNvmBlockID_ForbidState,
    eMSNvmBlockID_PlatParamBackup,
    eMSNvmBlockID_KVDBCount,
    
    /* 记录信息 */
    eMSNvmBlockID_OrderRecord = eMSNvmBlockID_KVDBCount,
    eMSNvmBlockID_ErrorRecord,
    eMSNvmBlockID_RunningLogRecord,
    eMSNvmBlockID_OmOrderRecord,
    eMSNvmBlockID_MeterRecord,
    eMSNvmBlockID_Count,
}MSNvmBlockID_Enum;

typedef enum
{
    eMSNvmWriteVerifyState_Success,  /* 最近一次参数写入已完成校验 */
    eMSNvmWriteVerifyState_Pending,  /* 最近一次参数写入等待异步校验 */
    eMSNvmWriteVerifyState_Failed,   /* 最近一次参数写入最终校验失败 */
}MSNvmWriteVerifyState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void MSNvm_InitMemory(void);
void MSNvm_ReadAll(void);
void MSNvm_MainFunction(void);

/* 参数块读写接口 */
GlobalRet_Enum MSNvm_ReadParaBlock(MSNvmBlockID_Enum eBlockID, uint8_t *pOutBuf, uint16_t dataLen);
GlobalRet_Enum MSNvm_WriteParaBlock(MSNvmBlockID_Enum eBlockID, uint8_t *pInBuf, uint16_t dataLen);
MSNvmWriteVerifyState_Enum MSNvm_GetParaBlockWriteVerifyState(MSNvmBlockID_Enum eBlockID);
GlobalRet_Enum MSNvm_SetDefaultParaBlock(MSNvmBlockID_Enum eBlockID);

/* 记录块读写接口 */
GlobalRet_Enum MSNvm_ClearRecord(MSNvmBlockID_Enum eBlockID);
uint32_t MSNvm_QueryUnreportedRecordCount(MSNvmBlockID_Enum eBlockID);
uint32_t MSNvm_QueryTotalRecordCount(MSNvmBlockID_Enum eBlockID);
GlobalRet_Enum MSNvm_InsertNewRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pInRecord, uint16_t recordSize);
GlobalRet_Enum MSNvm_InsertNewRecordNoLock(MSNvmBlockID_Enum eBlockID, uint8_t *pInRecord, uint16_t recordSize);
GlobalRet_Enum MSNvm_SetRecordReportSuccess(MSNvmBlockID_Enum eBlockID, uint32_t time);
GlobalRet_Enum MSNvm_QueryLatestUnreportedRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t *pTime);
GlobalRet_Enum MSNvm_QueryRecordByTime(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t time);
GlobalRet_Enum MSNvm_QueryRecordByExternal(MSNvmBlockID_Enum eBlockID, uint8_t *para, uint16_t paraSize,
pNvmCmpFunc pCmpFunc, uint8_t *pInRecord, uint16_t recordSize);
uint32_t MSNvm_QueryRecordLatestTime(MSNvmBlockID_Enum eBlockID);

/* 记录块和参数块 公共接口 */
uint8_t MSNvm_IsBusy(void);

#endif



















