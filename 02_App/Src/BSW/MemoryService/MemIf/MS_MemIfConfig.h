/******************************************************************************
* File Name          : MS_MemIfConfig.h
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
#ifndef MS_MEMIF_CONFIG_H_
#define MS_MEMIF_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct   
{
    uint8_t (*pFuncInit)(void);
    GlobalRet_Enum (*pFuncRead)(uint16_t memIfID, uint8_t *pOutBuf, uint16_t dataLen);
    GlobalRet_Enum (*pFuncWrite)(uint16_t memIfID, uint8_t *pInBuf, uint16_t dataLen);

    GlobalRet_Enum (*pFuncClearDB)(uint16_t memIfID);
    uint32_t (*pFuncQueryUnreportedRecordCount)(uint16_t memIfID);
    uint32_t (*pFuncQueryTotalRecordCount)(uint16_t memIfID);
    GlobalRet_Enum (*pFuncInsertRecord)(uint16_t memIfID, uint8_t *pInBuf, uint16_t dataLen);
    GlobalRet_Enum (*pFuncSetReportSuccess)(uint16_t memIfID, uint32_t time);
    GlobalRet_Enum (*pFuncQueryLatestUnreportedRecord)(uint16_t memIfID, uint8_t *pOutBuf, uint16_t dataLen, uint32_t *pTime);
    GlobalRet_Enum (*pFuncQueryLatestRecordTime)(uint16_t memIfID, uint32_t *pTime);
    GlobalRet_Enum (*pFuncQueryRecordByTime)(uint16_t memIfID, uint8_t *pOutBuf, uint16_t dataLen, uint32_t time);
    GlobalRet_Enum (*pFuncQueryRecordByExternal)(uint16_t memIfID, uint8_t *para, uint16_t paraSize,
    pNvmCmpFunc pCmpFunc, uint8_t *pInBuf, uint16_t dataLen);


}MSMemifConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const MSMemifConfig_Struct c_stMSMemifConfigTable[MSMEMIF_DEVICE_EA_COUNT];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif





















