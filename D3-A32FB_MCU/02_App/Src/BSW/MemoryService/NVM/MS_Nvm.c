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
    uint32_t writeVerifyPendingTick;  /* 异步回读校验的起始计时 */
    uint32_t backupPendingTick;       /* 备份块延时写入及失败重试的起始计时 */
    uint8_t writeFailedFlag;          /* 最近一次写入或异步校验是否失败，用于相同数据的强制重写 */
    uint8_t writeVerifyPendingFlag;   /* 是否存在等待异步回读校验的数据 */
    uint8_t writeVerifyRetryCount;    /* 当前异步回读校验流程已经执行的重写次数 */
    uint8_t backupPendingFlag;        /* 主块校验成功后是否存在待写入的备份块 */
}MSNvmBlockCtrl_Struct;

typedef struct
{
    SemaphoreHandle_t mutex;                                      /* NVM读写及异步状态访问的互斥锁 */
    uint8_t initFlag;                                             /* NVM存储接口和互斥锁是否初始化成功 */
    MSNvmBlockCtrl_Struct blockCtrl[eMSNvmBlockID_KVDBCount];     /* 各KVDB参数块独立的写入、校验和备份运行状态 */
}MSNvmCtrl_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static MSNvmCtrl_Struct g_stMsNvmCtrlCtx;
static uint8_t g_MSNvmWriteVerifyBuf[MSNVM_CFG_WRITE_VERIFY_BUF_SIZE];


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void MSNvm_LoadFlashData(MSNvmBlockID_Enum eBlockID);
static void MSNvm_LoadRedundantFlashData(MSNvmBlockID_Enum eBlockID);
static uint8_t MSNvm_ReadFlashData(MSNvmBlockID_Enum eBlockID);
static uint8_t MSNvm_IsBackupBlock(MSNvmBlockID_Enum eBlockID);
static GlobalRet_Enum MSNvm_SyncBackupBlock(MSNvmBlockID_Enum eBlockID, uint8_t forceWrite);
static GlobalRet_Enum MSNvm_RequestBackupBlock(MSNvmBlockID_Enum eBlockID);
static void MSNvm_RequestWriteVerifyBlock(MSNvmBlockID_Enum eBlockID);
static GlobalRet_Enum MSNvm_WriteFlashData(MSNvmBlockID_Enum eBlockID);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t MSNvm_ReadFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t calcCrc16 = 0;
    uint16_t savedCrc16 = 0;
    uint8_t result = FALSE;

    eRet = MSMemIf_Read(pDescriptor->deviceID, pDescriptor->memIfID, pDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN);
    if (eRet == eGlobalRet_OK)
    {
        savedCrc16 = Common_TwoUint8ToUint16(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize);
        calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);

        if (calcCrc16 == savedCrc16)
        {
            result = TRUE;
        }
        else
        {
            MSNVM_CFG_InfoPrint("NVM参数块[%d] CRC校验失败，存储CRC:0x%04X，计算CRC:0x%04X\r\n", eBlockID, savedCrc16, calcCrc16);
        }
    }
    else
    {
        MSNVM_CFG_InfoPrint("NVM参数块[%d]读取失败，错误码:%d\r\n", eBlockID, eRet);
    }

    return result;
}

static void MSNvm_LoadFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    uint16_t calcCrc16 = 0;

    if (MSNvm_ReadFlashData(eBlockID) != TRUE)
    {
        if (pDescriptor->pFuncDefault != NULL)
        {
            MSNVM_CFG_InfoPrint("NVM参数块[%d]无效，使用默认参数\r\n", eBlockID);
            pDescriptor->pFuncDefault(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
            calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
            Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
            MSNvm_WriteFlashData(eBlockID);
        }
    }
}

static uint8_t MSNvm_IsBackupBlock(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    uint8_t result = FALSE;
    uint8_t index = 0;

    for (index = 0; index < eMSNvmBlockID_Count; index++)
    {
        pDescriptor = &c_stMSNvmBlockDescriptorTable[index];

        if (pDescriptor->backupEnable == TRUE && pDescriptor->backupBlockID == eBlockID)
        {
            result = TRUE;
            break;
        }
    }

    return result;
}

static GlobalRet_Enum MSNvm_SyncBackupBlock(MSNvmBlockID_Enum eBlockID, uint8_t forceWrite)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    const MSNvmBlockDescriptor_Struct *pBackupDescriptor = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;

    if (pDescriptor->backupEnable == TRUE)
    {
        if (pDescriptor->backupBlockID < eMSNvmBlockID_Count && pDescriptor->backupBlockID != eBlockID)
        {
            pBackupDescriptor = &c_stMSNvmBlockDescriptorTable[pDescriptor->backupBlockID];

            if (pBackupDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB && pBackupDescriptor->blockSize == pDescriptor->blockSize)
            {
                if (forceWrite == TRUE || g_stMsNvmCtrlCtx.blockCtrl[pDescriptor->backupBlockID].writeFailedFlag == TRUE ||
                    memcmp(pDescriptor->ramBlockDataAddr, pBackupDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN) != 0)
                {
                    memcpy(pBackupDescriptor->ramBlockDataAddr, pDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN);
                    eRet = MSNvm_WriteFlashData((MSNvmBlockID_Enum)pDescriptor->backupBlockID);
                }
            }
            else
            {
                MSNVM_CFG_InfoPrint("NVM参数块[%d]备份配置无效，备份块ID:%u\r\n", eBlockID, pDescriptor->backupBlockID);
                eRet = eGlobalRet_ParaInvalid;
            }
        }
        else
        {
            MSNVM_CFG_InfoPrint("NVM参数块[%d]备份块ID无效:%u\r\n", eBlockID, pDescriptor->backupBlockID);
            eRet = eGlobalRet_ParaInvalid;
        }
    }

    return eRet;
}

static void MSNvm_LoadRedundantFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    const MSNvmBlockDescriptor_Struct *pBackupDescriptor = NULL;
    uint16_t calcCrc16 = 0;
    uint8_t mainValid = FALSE;
    uint8_t backupValid = FALSE;

    if (pDescriptor->backupBlockID < eMSNvmBlockID_Count && pDescriptor->backupBlockID != eBlockID)
    {
        pBackupDescriptor = &c_stMSNvmBlockDescriptorTable[pDescriptor->backupBlockID];

        if (pBackupDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB && pBackupDescriptor->blockSize == pDescriptor->blockSize)
        {
            mainValid = MSNvm_ReadFlashData(eBlockID);
            backupValid = MSNvm_ReadFlashData((MSNvmBlockID_Enum)pDescriptor->backupBlockID);

            if (mainValid == TRUE)
            {
                if (backupValid != TRUE)
                {
                    MSNVM_CFG_InfoPrint("NVM参数块[%d]备份无效，重新建立备份\r\n", eBlockID);
                }
                MSNvm_SyncBackupBlock(eBlockID, (backupValid == TRUE) ? FALSE : TRUE);
            }
            else if (backupValid == TRUE)
            {
                /* 主块损坏时用校验有效的备份恢复主块。 */
                MSNVM_CFG_InfoPrint("NVM参数块[%d]主块无效，使用备份块[%u]恢复\r\n", eBlockID, pDescriptor->backupBlockID);
                memcpy(pDescriptor->ramBlockDataAddr, pBackupDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN);
                MSNvm_WriteFlashData(eBlockID);
            }
            else if (pDescriptor->pFuncDefault != NULL)
            {
                /* 主、备块均无效时生成默认参数，并同步建立有效备份。 */
                MSNVM_CFG_InfoPrint("NVM参数块[%d]主备均无效，使用默认参数恢复\r\n", eBlockID);
                pDescriptor->pFuncDefault(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
                calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
                Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
                MSNvm_WriteFlashData(eBlockID);
                MSNvm_SyncBackupBlock(eBlockID, TRUE);
            }
        }
        else
        {
            MSNvm_LoadFlashData(eBlockID);
        }
    }
    else
    {
        MSNvm_LoadFlashData(eBlockID);
    }
}

static GlobalRet_Enum MSNvm_WriteFlashData(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint8_t previousFailed = g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeFailedFlag;

    eRet = MSMemIf_Write(pDescriptor->deviceID, pDescriptor->memIfID, pDescriptor->ramBlockDataAddr, pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN);
    g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeFailedFlag = (eRet == eGlobalRet_OK) ? FALSE : TRUE;

    if (eRet != eGlobalRet_OK)
    {
        MSNVM_CFG_InfoPrint("NVM参数块[%d]写入失败，错误码:%d\r\n", eBlockID, eRet);
    }
    else if (previousFailed == TRUE)
    {
        MSNVM_CFG_DebugPrint("NVM参数块[%d]重写成功\r\n", eBlockID);
    }

    return eRet;
}

static GlobalRet_Enum MSNvm_RequestBackupBlock(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    if (pDescriptor->backupEnable == TRUE)
    {
        if (pDescriptor->asyncBackupEnable == TRUE)
        {
            /* 参数连续修改时重新计时，最终只备份最后一次写入的数据。 */
            g_stMsNvmCtrlCtx.blockCtrl[eBlockID].backupPendingFlag = TRUE;
            g_stMsNvmCtrlCtx.blockCtrl[eBlockID].backupPendingTick = Common_GetSystick();
        }
        else
        {
            eRet = MSNvm_SyncBackupBlock(eBlockID, FALSE);
        }
    }

    return eRet;
}

static void MSNvm_RequestWriteVerifyBlock(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeVerifyPendingFlag = TRUE;
    g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeVerifyRetryCount = 0;
    g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeVerifyPendingTick = Common_GetSystick();

    if (pDescriptor->backupEnable == TRUE)
    {
        /* 新主块尚未确认时取消旧备份计划，确认成功后再重新安排。 */
        g_stMsNvmCtrlCtx.blockCtrl[eBlockID].backupPendingFlag = FALSE;
    }
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
        else
        {
            MSNVM_CFG_InfoPrint("NVM存储接口初始化失败\r\n");
        }
    }
    else
    {
        MSNVM_CFG_InfoPrint("NVM互斥锁创建失败\r\n");
    }
}
void MSNvm_ReadAll(void)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    uint8_t index = 0;

    for (index = 0; index < eMSNvmBlockID_Count; index++)
    {
        pDescriptor = &c_stMSNvmBlockDescriptorTable[index];

        if (pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB && MSNvm_IsBackupBlock((MSNvmBlockID_Enum)index) != TRUE)
        {
            if (pDescriptor->backupEnable == TRUE)
            {
                MSNvm_LoadRedundantFlashData((MSNvmBlockID_Enum)index);
            }
            else
            {
                MSNvm_LoadFlashData((MSNvmBlockID_Enum)index);
            }
        }
    }
}

void MSNvm_MainFunction(void)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = NULL;
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t verifyLen = 0;
    uint8_t operationHandled = FALSE;
    uint8_t index = 0;

    if (g_stMsNvmCtrlCtx.initFlag == TRUE && g_stMsNvmCtrlCtx.mutex != NULL && xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, 0) == pdTRUE)
    {
        /* 主块回读确认优先于备份，保证只有确认有效的数据才会写入备份块。 */
        for (index = 0; index < eMSNvmBlockID_KVDBCount; index++)
        {
            if (g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyPendingFlag == TRUE &&
                Common_JudgeTimeoutMs(g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyPendingTick, MSNVM_CFG_ASYNC_VERIFY_DELAY_MS) == TRUE)
            {
                pDescriptor = &c_stMSNvmBlockDescriptorTable[index];
                verifyLen = pDescriptor->blockSize + MSNVM_CFG_ADDTION_CRC16_LEN;

                if (verifyLen <= sizeof(g_MSNvmWriteVerifyBuf))
                {
                    eRet = MSMemIf_Read(pDescriptor->deviceID, pDescriptor->memIfID, g_MSNvmWriteVerifyBuf, verifyLen);
                }
                else
                {
                    eRet = eGlobalRet_ParaInvalid;
                    MSNVM_CFG_InfoPrint("NVM参数块[%d]异步回读缓存不足，参数长度:%u，缓存长度:%u\r\n", index, verifyLen, (uint16_t)sizeof(g_MSNvmWriteVerifyBuf));
                }

                if (eRet == eGlobalRet_OK && memcmp(pDescriptor->ramBlockDataAddr, g_MSNvmWriteVerifyBuf, verifyLen) == 0)
                {
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyPendingFlag = FALSE;
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeFailedFlag = FALSE;
                    eRet = MSNvm_RequestBackupBlock((MSNvmBlockID_Enum)index);
                }
                else if (g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyRetryCount < MSNVM_CFG_ASYNC_VERIFY_RETRY_COUNT)
                {
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyRetryCount++;
                    MSNVM_CFG_InfoPrint("NVM参数块[%d]异步回读校验失败，第%d次重写\r\n", index, g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyRetryCount);
                    MSNvm_WriteFlashData((MSNvmBlockID_Enum)index);
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyPendingTick = Common_GetSystick();
                }
                else
                {
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeVerifyPendingFlag = FALSE;
                    g_stMsNvmCtrlCtx.blockCtrl[index].writeFailedFlag = TRUE;
                    MSNVM_CFG_InfoPrint("NVM参数块[%d]异步回读校验最终失败，等待下次参数写入重试\r\n", index);
                }
                operationHandled = TRUE;
                break;
            }
        }

        /* 本周期未处理主块校验时，再处理一个到期的备份写入。 */
        if (operationHandled != TRUE)
        {
            for (index = 0; index < eMSNvmBlockID_KVDBCount; index++)
            {
                if (g_stMsNvmCtrlCtx.blockCtrl[index].backupPendingFlag == TRUE && Common_JudgeTimeoutMs(g_stMsNvmCtrlCtx.blockCtrl[index].backupPendingTick, MSNVM_CFG_ASYNC_BACKUP_DELAY_MS) == TRUE)
                {
                    pDescriptor = &c_stMSNvmBlockDescriptorTable[index];
                    eRet = MSNvm_SyncBackupBlock((MSNvmBlockID_Enum)index, FALSE);
                    if (eRet == eGlobalRet_OK)
                    {
                        g_stMsNvmCtrlCtx.blockCtrl[index].backupPendingFlag = FALSE;
                        if (c_stMSNvmBlockDescriptorTable[pDescriptor->backupBlockID].asyncWriteVerifyEnable == TRUE)
                        {
                            MSNvm_RequestWriteVerifyBlock((MSNvmBlockID_Enum)pDescriptor->backupBlockID);
                        }
                    }
                    else
                    {
                        g_stMsNvmCtrlCtx.blockCtrl[index].backupPendingTick = Common_GetSystick();
                    }
                    break;
                }
            }
        }
        xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
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
    uint8_t mainBlockWriteAttempted = FALSE;

    PARA_ASSERT_RET((eBlockID < eMSNvmBlockID_Count) && (pInBuf != NULL) && dataLen != 0, eGlobalRet_ParaInvalid);

    pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_KVDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);

    copyLen = (dataLen >= pDescriptor->blockSize) ? pDescriptor->blockSize : dataLen;

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);

    if (g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeFailedFlag == TRUE || memcmp(pInBuf, pDescriptor->ramBlockDataAddr, copyLen) != 0)
    {
        memcpy(pDescriptor->ramBlockDataAddr, pInBuf, copyLen);
        memset(pDescriptor->ramBlockDataAddr + copyLen, 0x00, pDescriptor->blockSize - copyLen);
        calcCrc16 = Common_CalcCRC16(pDescriptor->ramBlockDataAddr, pDescriptor->blockSize);
        Common_Uint16ToTwoUint8(pDescriptor->ramBlockDataAddr + pDescriptor->blockSize, calcCrc16);
        mainBlockWriteAttempted = TRUE;
        eRet = MSNvm_WriteFlashData(eBlockID);
    }

    if (mainBlockWriteAttempted == TRUE)
    {
        if (pDescriptor->asyncWriteVerifyEnable == TRUE)
        {
            /* 首次写失败也保留异步校验任务，后续由周期函数回读并重写。 */
            MSNvm_RequestWriteVerifyBlock(eBlockID);
        }
        else if (eRet == eGlobalRet_OK)
        {
            eRet = MSNvm_RequestBackupBlock(eBlockID);
        }
    }

    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

/******************************************************************************
* @brief 查询参数块最近一次异步写校验状态
* @param[in] eBlockID 参数块标识
* @retval eMSNvmWriteVerifyState_Success 最近一次写入已经回读确认
* @retval eMSNvmWriteVerifyState_Pending 最近一次写入仍在等待回读
* @retval eMSNvmWriteVerifyState_Failed 参数块无效或最近一次写入最终失败
* @note Step1：在互斥锁内读取NVM控制状态，避免与异步校验过程并发。
*       Step2：该接口只观察状态，不改变现有写入、重试和备份机制。
******************************************************************************/
MSNvmWriteVerifyState_Enum MSNvm_GetParaBlockWriteVerifyState(MSNvmBlockID_Enum eBlockID)
{
    MSNvmWriteVerifyState_Enum eState = eMSNvmWriteVerifyState_Failed;

    if ((eBlockID < eMSNvmBlockID_KVDBCount) &&
        (g_stMsNvmCtrlCtx.initFlag == TRUE) && (g_stMsNvmCtrlCtx.mutex != NULL))
    {
        xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
        if (g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeVerifyPendingFlag == TRUE)
        {
            eState = eMSNvmWriteVerifyState_Pending;
        }
        else if (g_stMsNvmCtrlCtx.blockCtrl[eBlockID].writeFailedFlag == TRUE)
        {
            eState = eMSNvmWriteVerifyState_Failed;
        }
        else
        {
            eState = eMSNvmWriteVerifyState_Success;
        }
        xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    }

    return eState;
}

GlobalRet_Enum MSNvm_ClearRecord(MSNvmBlockID_Enum eBlockID)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    eRet = MSMemIf_ClearRecord(pDescriptor->deviceID, pDescriptor->memIfID);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

uint32_t MSNvm_QueryUnreportedRecordCount(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    uint32_t recordCount = 0;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, 0);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, 0);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    recordCount = MSMemIf_QueryUnreportedRecordCount(pDescriptor->deviceID, pDescriptor->memIfID);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return recordCount;
}

uint32_t MSNvm_QueryTotalRecordCount(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    uint32_t recordCount = 0;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, 0);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, 0);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, 0);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    recordCount = MSMemIf_QueryTotalRecordCount(pDescriptor->deviceID, pDescriptor->memIfID);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return recordCount;
}

GlobalRet_Enum MSNvm_InsertNewRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pInRecord, uint16_t recordSize)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count && pInRecord != NULL && recordSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);
    eRet = MSMemIf_InsertRecord(pDescriptor->deviceID, pDescriptor->memIfID, pInRecord, recordSize);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

GlobalRet_Enum MSNvm_InsertNewRecordNoLock(MSNvmBlockID_Enum eBlockID, uint8_t *pInRecord, uint16_t recordSize)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count && pInRecord != NULL && recordSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    eRet = MSMemIf_InsertRecord(pDescriptor->deviceID, pDescriptor->memIfID, pInRecord, recordSize);

    return eRet;
}

GlobalRet_Enum MSNvm_SetRecordReportSuccess(MSNvmBlockID_Enum eBlockID, uint32_t time)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);   
    eRet = MSMemIf_SetReportSuccess(pDescriptor->deviceID, pDescriptor->memIfID, time);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

GlobalRet_Enum MSNvm_QueryLatestUnreportedRecord(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t *pTime)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pOutRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);   
    eRet = MSMemIf_QueryLatestUnreportedRecord(pDescriptor->deviceID, pDescriptor->memIfID, pOutRecord, recordSize, pTime);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

uint32_t MSNvm_QueryRecordLatestTime(MSNvmBlockID_Enum eBlockID)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    uint32_t time = 0;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);   
    MSMemIf_QueryLatestRecordTime(pDescriptor->deviceID, pDescriptor->memIfID, &time);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return time;
}

GlobalRet_Enum MSNvm_QueryRecordByTime(MSNvmBlockID_Enum eBlockID, uint8_t *pOutRecord, uint16_t recordSize, uint32_t time)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pOutRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);   
    eRet = MSMemIf_QueryRecordByTime(pDescriptor->deviceID, pDescriptor->memIfID, pOutRecord, recordSize, time);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

GlobalRet_Enum MSNvm_QueryRecordByExternal(MSNvmBlockID_Enum eBlockID, uint8_t *para, uint16_t paraSize, pNvmCmpFunc pCmpFunc, 
    uint8_t *pInRecord, uint16_t recordSize)
{
    const MSNvmBlockDescriptor_Struct *pDescriptor = &c_stMSNvmBlockDescriptorTable[eBlockID];
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(eBlockID < eMSNvmBlockID_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pInRecord != NULL && recordSize <= pDescriptor->blockSize, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDescriptor->deviceID == MSMEMIF_DEVICE_EA_TSDB, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCmpFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stMsNvmCtrlCtx.initFlag == TRUE, eGlobalRet_NotInit);

    xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, portMAX_DELAY);   
    eRet = MSMemIf_QueryRecordByExternal(pDescriptor->deviceID, pDescriptor->memIfID, para, paraSize, pCmpFunc, pInRecord, recordSize);
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
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
    eRet = MSNvm_WriteFlashData(eBlockID);

    if (pDescriptor->asyncWriteVerifyEnable == TRUE)
    {
        /* 默认参数首次写失败时同样进入异步校验和重写流程。 */
        MSNvm_RequestWriteVerifyBlock(eBlockID);
    }
    else if (eRet == eGlobalRet_OK)
    {
        eRet = MSNvm_RequestBackupBlock(eBlockID);
    }
    xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
    return eRet;
}

uint8_t MSNvm_IsBusy(void)
{
    uint8_t ret = TRUE;

    if (g_stMsNvmCtrlCtx.initFlag == TRUE && g_stMsNvmCtrlCtx.mutex != NULL)
    {
        if (xSemaphoreTake(g_stMsNvmCtrlCtx.mutex, 0) == pdTRUE)
        {
            xSemaphoreGive(g_stMsNvmCtrlCtx.mutex);
            ret = FALSE;
        }
    }

    return ret;
}
