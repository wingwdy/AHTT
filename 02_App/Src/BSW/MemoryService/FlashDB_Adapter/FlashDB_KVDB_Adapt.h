/******************************************************************************
* File Name          : FlashDB_KVDB_Adapt.h
* Description        : Code for The adapter layer of KVDB
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
#ifndef FLASHDB_KVDB_ADAPT_H_
#define FLASHDB_KVDB_ADAPT_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eKVDBAdaptChannel_Gun0Qrcode,
	eKVDBAdaptChannel_Gun0OrderInfo,
	eKVDBAdaptChannel_Gun0MeterEnergy,
	eKVDBAdaptChannel_ModeParam,
	eKVDBAdaptChannel_MeterCaliParam,
	eKVDBAdaptChannel_PlatParam,
	eKVDBAdaptChannel_PlatPrivateParam,
	eKVDBAdaptChannel_Count,
}KVDBAdaptChannel_Enum;






/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t KVDBAdapt_Init(void);
GlobalRet_Enum KVDBAdapt_Read(uint16_t blockID, uint8_t *pOutBuf,  uint16_t dataLen);
GlobalRet_Enum KVDBAdapt_Write(uint16_t blockID, uint8_t *pOutBuf,  uint16_t dataLen);
#endif





















