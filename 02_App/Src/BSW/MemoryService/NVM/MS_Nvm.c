/******************************************************************************
* File Name          : MS_Nvm.c
* Description        : Code for The core service layer for managing non-volatile data 
                       storage of the ECU
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
#include "MS_Nvm.h"
#include "MS_NvmConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

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
    SemaphoreHandle_t mutex;
    uint8_t initFlag;
}MSNvmCtrl_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static MSNvmCtrl_Struct g_stMsNvmCtrlCtx;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void MSNvm_LoadFlashData(MSNvmBlockID_Enum eBlockID);
static void MSNvm_WriteFlashData(MSNvmBlockID_Enum eBlockID);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void MSNvm_LoadFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    uint16_t calcCrc16 = 0;
    uint16_t savedCrc16 = 0;
    uint8_t result = FALSE;

    if (eGlobalRet_OK == MSMemIf_Read(pDescriptor->deviceID, pDescriptor->memIfID, pDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN))
    {
        savedCrc16 = Common_TwoUint8ToUint16(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize);
        calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);

        if (calcCrc16 == savedCrc16)
        {
            result = TRUE;
        }
    }

    if (result != TRUE)
    {
        if (pDescriptor->pFuncDefault != NULL)
        {
            pDescriptor->pFuncDefault(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
            calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
            Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
            MSNvm_WriteFlashData(eBlockID);
        }
    }
}

static void MSNvm_WriteFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    MSMemIf_Write(pDescriptor->deviceID, pDescriptor->memIfID, pDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN);
}
void MSNvm_InitMemory(void)
{
    memset(&g_stMsNvmCtrlCtx, 0x00, sizeof(g_stMsNvmCtrlCtx));
    g_stMsNvmCtrlCtx.mutex = xSemaphoreCreateMutex();

    if (g_stMsNvmCtrlCtx.mutex != NULL)
    {
        if (eGlobalRet_OK == MSMemIf_Init())
        {
            g_stMsNvmCtrlCtx.initFlag = TRUE;
        }
    }
}
void MSNvm_ReadAll(void)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    uint8_t index = 0;

    for (index = 0; index < eMSNvmBlockID_Count; index++)
    {
        pDescriptor = &c_stMSNvmBlockDescriptorTable[index];

        if (pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB)
        {
            MSNvm_LoadFlashData((MSNvmBlockID_Enum)index);
        }
    }
}

GlobalRet_Enum MSNvm_ReadParaBlock(MSNvmBlockID_Enum eBlockID, uint8_t *pOutBuf, uint16_t dataLen)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t copyLen = 0;

    PARA_ASSERT_RET((eBlockID < eMSNvmBlockID_Count) && (pOutBuf != NULL) && dataLen != 0, eGlobalRet_ParaInvalid);

    pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    copyLen = (dataLen >= pDescriptor->blockSize) ? pDescriptor->blockSize : dataLen;
    memcpy(pOutBuf, pDescriptor->ramBlockDataAddr, copyLen);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);

    return eGlobalRet_OK;
}

GlobalRet_Enum MSNvm_WriteParaBlock(MSNvmBlockID_Enum eBlockID, uint8_t *pInBuf, uint16_t dataLen)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t copyLen = 0;
    uint16_t calcCrc16 = 0;

    PARA_ASSERT_RET((eBlockID < eMSNvmBlockID_Count) && (pInBuf != NULL) && dataLen != 0, eGlobalRet_ParaInvalid);

    pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);

    copyLen = (dataLen >= pDescriptor->blockSize) ? pDescriptor->blockSize : dataLen;

    if (0 != memcmp(pInBuf, pDescriptor->ramBlockDataAddr, copyLen))
    {
        xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
        memcpy(pDescriptor->ramBlockDataAddr, pInBuf, copyLen);
        memset(pDescriptor->ramBlockDataAddr + copyLen, 0x00, pDescriptor->blockSize - copyLen);
        xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
        calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
        Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
        MSNvm_WriteFlashData(eBlockID);
    }

    return eGlobalRet_OK;
}

GlobalRet_Enum MSNvm_ClearRecord(MSNvmBlockID_Enum eBlockID)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    eRet = MSMemIf_ClearRecord(pDescriptor->deviceID, pDescriptor->memIfID);
    return eRet;
}

uint32_t MSNvm_QueryUnreportedRecordCount(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, 0);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, 0);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    return MSMemIf_QueryUnreportedRecordCount(pDescriptor->deviceID, pDescriptor->memIfID);
}

uint32_t MSNvm_QueryTotalRecordCount(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, 0);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, 0);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, 0);

    return MSMemIf_QueryTotalRecordCount(pDescriptor->deviceID, pDescriptor->memIfID);
}

GlobalRet_Enum MSNvm_InsertNewRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pInRecord, uint16_t recordSize)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count && pInRecord != NULL && recordSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    return MSMemIf_InsertRecord(pDescriptor->deviceID, pDescriptor->memIfID, pInRecord, recordSize);
}

GlobalRet_Enum MSNvm_SetRecordReportSuccess(MSNvmBlockID_Enum eBlockID, uint32_t time)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);
    
    return MSMemIf_SetReportSuccess(pDescriptor->deviceID, pDescriptor->memIfID, time);
}

GlobalRet_Enum MSNvm_QueryLatestUnreportedRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t *pTime)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pOutRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    return MSMemIf_QueryLatestUnreportedRecord(pDescriptor->deviceID, pDescriptor->memIfID, pOutRecord, recordSize, pTime);
}

GlobalRet_Enum MSNvm_QueryRecordByTime(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t time)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pOutRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    return MSMemIf_QueryRecordByTime(pDescriptor->deviceID, pDescriptor->memIfID, pOutRecord, recordSize, time);
}

GlobalRet_Enum MSNvm_QueryRecordByExternal(MSNvmBlockID_Enum eBlockID, uint8_t *para, uint16_t paraSize, pNvmCmpFunc pCmpFunc, 
    uint8_t *pInRecord, uint16_t recordSize)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pInRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCmpFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);

    return MSMemIf_QueryRecordByExternal(pDescriptor->deviceID, pDescriptor->memIfID, para, paraSize, pCmpFunc, pInRecord, recordSize);
}

GlobalRet_Enum MSNvm_SetDefaultParaBlock(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t calcCrc16 = 0;

    PARA_ASSERT_RET((eBlockID < eMSNvmBlockID_Count), eGlobalRet_ParaInvalid);

    pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->pFuncDefault != NULL, eGlobalRet_Unsupported);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    pDescriptor->pFuncDefault(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
    calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
    Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    
    MSNvm_WriteFlashData(eBlockID);

    return eGlobalRet_OK;
}





