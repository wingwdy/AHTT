/******************************************************************************
* File Name          : template.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef MS_MEMIF_H_
#define MS_MEMIF_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "FlashDB_KVDB_Adapt.h"
#include "FlashDB_TSDB_Adapt.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MSMEMIF_DEVICE_EA_KVDB         0x00u
#define MSMEMIF_DEVICE_EA_TSDB         0x01u
#define MSMEMIF_DEVICE_EA_COUNT        0x02u


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
  


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
GlobalRet_Enum MSMemIf_Write(uint16_t deviceID, uint16_t memIfID, uint8_t *pIndata, uint16_t dataLen);
GlobalRet_Enum MSMemIf_Read(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutdata, uint16_t dataLen);
GlobalRet_Enum MSMemIf_ClearRecord(uint16_t deviceID, uint16_t memIfID);
uint32_t MSMemIf_QueryUnreportedRecordCount(uint16_t deviceID, uint16_t memIfID);
uint32_t MSMemIf_QueryTotalRecordCount(uint16_t deviceID, uint16_t memIfID);
GlobalRet_Enum MSMemIf_InsertRecord(uint16_t deviceID, uint16_t memIfID, uint8_t *pInRecord, uint16_t recordSize);
GlobalRet_Enum MSMemIf_SetReportSuccess(uint16_t deviceID, uint16_t memIfID, uint32_t time);
GlobalRet_Enum MSMemIf_QueryLatestUnreportedRecord(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t *pTime);
GlobalRet_Enum MSMemIf_QueryRecordByTime(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t time);
GlobalRet_Enum MSMemIf_QueryRecordByExternal(uint16_t deviceID, uint16_t memIfID, uint8_t *para, uint16_t paraSize, pNvmCmpFunc pCmpFunc, 
    uint8_t *pInRecord, uint16_t recordSize);
GlobalRet_Enum MSMemIf_Init(void);
#endif






















