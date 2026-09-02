/******************************************************************************
* File Name          : template.c
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
#include "MS_Memif.h"
#include "MS_MemIfConfig.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
GlobalRet_Enum MSMemIf_Write(uint16_t deviceID, uint16_t memIfID, uint8_t *pIndata, uint16_t dataLen)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT && pIndata != NULL && dataLen != 0, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncWrite != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncWrite(memIfID, pIndata, dataLen);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_Read(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutdata, uint16_t dataLen)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT && pOutdata != NULL && dataLen != 0, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncRead != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncRead(memIfID, pOutdata, dataLen);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_ClearRecord(uint16_t deviceID, uint16_t memIfID)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncClearDB != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncClearDB(memIfID);
    }

    return eRet;
}

uint32_t MSMemIf_QueryUnreportedRecordCount(uint16_t deviceID, uint16_t memIfID)
{
    uint32_t count = 0;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, 0);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryUnreportedRecordCount != NULL)
    {
        count = c_stMSMemifConfigTable[deviceID].pFuncQueryUnreportedRecordCount(memIfID);
    }

    return count;
}

uint32_t MSMemIf_QueryTotalRecordCount(uint16_t deviceID, uint16_t memIfID)
{
    uint32_t count = 0;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, 0);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryTotalRecordCount != NULL)
    {
        count = c_stMSMemifConfigTable[deviceID].pFuncQueryTotalRecordCount(memIfID);
    }

    return count;
}

GlobalRet_Enum MSMemIf_InsertRecord(uint16_t deviceID, uint16_t memIfID, uint8_t *pInRecord, uint16_t recordSize)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT && pInRecord != NULL && recordSize != 0, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncInsertRecord != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncInsertRecord(memIfID, pInRecord, recordSize);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_SetReportSuccess(uint16_t deviceID, uint16_t memIfID, uint32_t time)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncSetReportSuccess != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncSetReportSuccess(memIfID, time);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_QueryLatestUnreportedRecord(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t *pTime)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryLatestUnreportedRecord != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncQueryLatestUnreportedRecord(memIfID, pOutRecord, recordSize, pTime);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_QueryLatestRecordTime(uint16_t deviceID, uint16_t memIfID, uint32_t *pTime)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryLatestRecordTime != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncQueryLatestRecordTime(memIfID, pTime);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_QueryRecordByTime(uint16_t deviceID, uint16_t memIfID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t time)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryRecordByTime != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncQueryRecordByTime(memIfID, pOutRecord, recordSize, time);
    }

    return eRet;
}

GlobalRet_Enum MSMemIf_QueryRecordByExternal(uint16_t deviceID, uint16_t memIfID, uint8_t *para, 
    uint16_t paraSize, pNvmCmpFunc pCmpFunc, uint8_t *pInRecord, uint16_t recordSize)
{
    GlobalRet_Enum eRet = eGlobalRet_NotInit;
    PARA_ASSERT_RET(deviceID < MSMEMIF_DEVICE_EA_COUNT, eGlobalRet_ParaInvalid);

    if (c_stMSMemifConfigTable[deviceID].pFuncQueryRecordByExternal != NULL)
    {
        eRet = c_stMSMemifConfigTable[deviceID].pFuncQueryRecordByExternal(memIfID, para,
            paraSize, pCmpFunc, pInRecord, recordSize);
    }

    return eRet;
}


GlobalRet_Enum MSMemIf_Init(void)
{
    const MSMemifConfig_Struct *pCfg = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint8_t index = 0;

    for (index = 0; index < ARRAY_SIZE(c_stMSMemifConfigTable); index++)
    {
        pCfg = &c_stMSMemifConfigTable[index];

        if (pCfg->pFuncInit != NULL)
        {
            if (TRUE != pCfg->pFuncInit())
            {
                eRet = eGlobalRet_InitFail;
                break;
            }
        }
    }

    return eRet;
}



















