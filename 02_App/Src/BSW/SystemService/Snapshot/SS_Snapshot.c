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
#include "SS_Snapshot.h"
#include "SS_SnapshotConfig.h"
#include "SS_Tm.h"
#include "Cdd_NetM.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "myMalloc.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define SSSNAPSHOT_SAVE_STATE_NULL           0
#define SSSNAPSHOT_SAVE_STATE_START          1
#define SSSNAPSHOT_SAVE_STATE_FINISH         2


/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    SSSnapshotItemReadSrc_Enum eReadSrc;
    SSSnapshotItemType_Enum eItemType;
    uint8_t localPrintItemFlag;
    uint8_t readItemOngoing;
    MSNvmBlockID_Enum readBlockID;
    uint32_t exportItemStartTick;
    uint32_t latestTime;
    uint32_t currentReadTime;
    uint32_t startReadTime;
    uint16_t singleItemSize;
    uint8_t sizeFlag;
    uint8_t readDirection;
    uint8_t *pItemBuf;
    uint16_t readOffset;
    uint16_t remainSize;
}SSSnapshotReadItem_Struct;

typedef struct SS_Snapshot
{
    uint8_t dealFlag;
    char errInfoPack[SSSNAPSHOT_CFG_ERROR_INFO_SZIE];
}SSSnapshotErrorCache_Struct;

typedef struct
{
    char logBuf[2][MSNVM_RUNNING_LOG_MAX_LEN];      // 运行日志(双)缓存
    uint16_t writeOft;                              // 写入缓存偏移
    uint8_t writeIdx;                               // 当前写缓存索引(0 or 1)
    uint8_t flushFlag;                              // 运行日志flush(写入flashdb)标记

}SSSnapshotRunLogCache_Struct;

typedef struct 
{
    uint32_t cycPrintTick;
    SSSnapshotReadItem_Struct stReadItem;
    CddNetMSocketPara_Union stNetPara;
    SSSnapshotItemState_Enum eState;
    uint32_t WaitTick;
    SSSnapshotErrorCache_Struct stErrorCache[SSSNAPSHOT_CFG_ERROR_ITEM_COUNT];
    SSSnapshotRunLogCache_Struct stRunLogCache;
    SemaphoreHandle_t mutex;
}SSSnapshotCtx_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SSSnapshotCtx_Struct g_stSnapshotCtx = {0};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static SSSnapshotErrorCache_Struct *SSSnapshot_FindFreeErrCache(void);
static void SSSnapshot_DeleteErrCache(void);
static uint8_t SSSnapshot_ReadItemByTime(SSSnapshotItemType_Enum itemType, uint8_t *buf, uint16_t buffLen, uint32_t time);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static SSSnapshotErrorCache_Struct *SSSnapshot_FindFreeErrCache(void)
{
    SSSnapshotErrorCache_Struct *pErrCache = NULL;
    uint8_t index = 0;

    for (index = 0; index < SSSNAPSHOT_CFG_ERROR_ITEM_COUNT; index++)
    {
        if (SSSNAPSHOT_SAVE_STATE_NULL == g_stSnapshotCtx.stErrorCache[index].dealFlag)
        {
            pErrCache = &g_stSnapshotCtx.stErrorCache[index];
            pErrCache->dealFlag = SSSNAPSHOT_SAVE_STATE_START;
            break;
        }
    }

    return pErrCache;
}

static void SSSnapshot_DeleteErrCache(void)
{
    memmove(g_stSnapshotCtx.stErrorCache, 
            g_stSnapshotCtx.stErrorCache + 1, 
            sizeof(SSSnapshotErrorCache_Struct) * (SSSNAPSHOT_CFG_ERROR_ITEM_COUNT - 1));

    memset(&g_stSnapshotCtx.stErrorCache[SSSNAPSHOT_CFG_ERROR_ITEM_COUNT - 1], 
           0x00, sizeof(SSSnapshotErrorCache_Struct));
}

static void SSSnapshot_HandleErrLog(void)
{
    SSSnapshotErrorCache_Struct *pErrCache = &g_stSnapshotCtx.stErrorCache[0];

    if (pErrCache->dealFlag == SSSNAPSHOT_SAVE_STATE_FINISH)
    {
        MSNvm_InsertNewRecord(eMSNvmBlockID_ErrorRecord, (uint8_t *)pErrCache->errInfoPack, SSSNAPSHOT_CFG_ERROR_INFO_SZIE);
        SSSnapshot_DeleteErrCache();
    }
}

static uint8_t SSSnapshot_ReadItemByTime(SSSnapshotItemType_Enum itemType, uint8_t *buf, uint16_t buffLen, uint32_t time)
{
    uint8_t ret = FALSE;

    switch (itemType)
    {
        case eSSSnapshotItemType_ErrorLog:
        {
            if (buffLen >= SSSNAPSHOT_CFG_ERROR_INFO_SZIE)
            {
                if (eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_ErrorRecord, buf, SSSNAPSHOT_CFG_ERROR_INFO_SZIE, time))
                {
                    ret = TRUE;
                }
            }

            break;
        }
        case eSSSnapshotItemType_RunningLog:
        {

        }
        default:
        {
            break;
        }
    }

    return ret;
}

static void SSSnapshot_PrintItemInfo(void)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;
    uint8_t data[MSNVM_RUNNING_LOG_MAX_LEN] = { 0 };
    uint16_t outLen = 0;

    if (pReadItemHandle->localPrintItemFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(g_stSnapshotCtx.cycPrintTick, SSSNAPSHOT_CFG_CYCLE_PRINT_PERIOD))
        {
            g_stSnapshotCtx.cycPrintTick = Common_GetSystick();
        
            if (TRUE == SSSnapshot_ReadItem(data, MSNVM_RUNNING_LOG_MAX_LEN, &outLen))
            {
                if (outLen > 0)
                {
                    SSSNAPSHOT_CFG_DebugPrint("日志[%d 条]:\n", g_stSnapshotCtx.stReadItem.currentReadTime);
                    SSSNAPSHOT_CFG_DebugPrint("\n%s", data);
                    memset(data, 0x00, outLen);
                }
            }
            else
            {
                pReadItemHandle->localPrintItemFlag = FALSE;
                SSSnapshot_StopReadItem();
            }
        }
    }
}



void SSSnapshot_ExportItem(SSSnapshotItemType_Enum itemType, SSSnapshotItemReadSrc_Enum eReadSrc)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;
    CddNetMSocketPara_Union *pSocketPara = &g_stSnapshotCtx.stNetPara;
    CommonDateTime_Struct sttime = {0};
    MSNvmPlatParam_Struct stPlatParam = {0};
    char pileDn[MSNVM_PILE_DN_LEN + 1] = "Unknow";          
    const char *snapshotName = "ErrLog";

    do
    {
        if (pReadItemHandle->readItemOngoing == TRUE)
        {
            SSSNAPSHOT_CFG_InfoPrint("快照读取已开始!\r\n");
            break;
        }

        if (eReadSrc == eSnapshotItemReadSrc_Local)
        {/* 本地导出 */
            pReadItemHandle->exportItemStartTick = Common_GetSystick();
            pReadItemHandle->eReadSrc = eReadSrc;
            pReadItemHandle->localPrintItemFlag = TRUE;

            SSSnapshot_StartReadItem(itemType);
            SSSNAPSHOT_CFG_InfoPrint("开始本地快照读取!\r\n");
        }
        else
        {/* 远程导出 */
            SSTM_GetDateTime(&sttime);

            if (eGlobalRet_OK == MSNvm_ReadParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&stPlatParam, sizeof(stPlatParam)))
            {
                snprintf(pileDn, sizeof(pileDn), "%s", stPlatParam.platPileDn);
            }

            if (itemType == eSSSnapshotItemType_RunningLog)
            {
                snapshotName = "RunLog";
            }
            else if (itemType == eSSSnapshotItemType_OmOrderRecord)
            {
                snapshotName = "OrderRecord";
            }

            snprintf(pSocketPara->stFtpPara.fileName, CDD_NETM_CFG_FTP_FILENAME_LEN + 1, "%s_%s_%04d%02d%02d.txt", 
                        pileDn, snapshotName, sttime.year, sttime.month, sttime.day);

            if (eGlobalRet_OK != CddNetM_CreatLink(eCddNetMSocketType_FTP, *pSocketPara, eCddNetMPlatType_File))
            {
                SSSNAPSHOT_CFG_InfoPrint("创建FTP传输通道失败!\r\n");
                break;
            }

            pReadItemHandle->exportItemStartTick = Common_GetSystick();
            pReadItemHandle->eReadSrc = eReadSrc;
            pReadItemHandle->localPrintItemFlag = FALSE;

            SSSnapshot_StartReadItem(itemType);
            SSSNAPSHOT_CFG_InfoPrint("开始远程快照[%d]读取!\r\n", itemType);
        }
    } while(0);
}

void SSSnapshot_FlushRunningLog(void)
{
    xSemaphoreTake(g_stSnapshotCtx.mutex, portMAX_DELAY);

    if (g_stSnapshotCtx.stRunLogCache.writeOft > 0 && g_stSnapshotCtx.stRunLogCache.flushFlag == FALSE)
    {
        g_stSnapshotCtx.stRunLogCache.flushFlag = TRUE;
        g_stSnapshotCtx.stRunLogCache.writeIdx ^= 1;
        g_stSnapshotCtx.stRunLogCache.writeOft = 0;
    }

    xSemaphoreGive(g_stSnapshotCtx.mutex);
}

void SSSnapshot_InsertRunningLog(const char *buf, uint16_t len)
{
    uint16_t remain = 0;
    uint16_t writeLen = 0;
    uint16_t totalSpace = 0;
    uint8_t writeFlag = TRUE;

    if (buf != NULL && len > 0)
    {
        xSemaphoreTake(g_stSnapshotCtx.mutex, portMAX_DELAY);

        remain = MSNVM_RUNNING_LOG_MAX_LEN - g_stSnapshotCtx.stRunLogCache.writeOft - 1;

        if (len > remain)
        {
            if (g_stSnapshotCtx.stRunLogCache.flushFlag == FALSE)
            {
                totalSpace = remain + (MSNVM_RUNNING_LOG_MAX_LEN - 1);
                
                if (len > totalSpace)
                {
                    len = totalSpace;
                }

                if (remain > 0)
                {
                    memcpy(g_stSnapshotCtx.stRunLogCache.logBuf[g_stSnapshotCtx.stRunLogCache.writeIdx] + \
                            g_stSnapshotCtx.stRunLogCache.writeOft, buf, remain);
                }

                g_stSnapshotCtx.stRunLogCache.flushFlag = TRUE;
                g_stSnapshotCtx.stRunLogCache.writeIdx ^= 1;
                g_stSnapshotCtx.stRunLogCache.writeOft = 0;

                writeLen = len - remain;
                memcpy(g_stSnapshotCtx.stRunLogCache.logBuf[g_stSnapshotCtx.stRunLogCache.writeIdx] + \
                        g_stSnapshotCtx.stRunLogCache.writeOft, buf + remain, writeLen);
                g_stSnapshotCtx.stRunLogCache.writeOft += writeLen;
            }
            else
            {
                writeFlag = FALSE;
            }
        }
        else
        {
            memcpy(g_stSnapshotCtx.stRunLogCache.logBuf[g_stSnapshotCtx.stRunLogCache.writeIdx] + \
                    g_stSnapshotCtx.stRunLogCache.writeOft, buf, len);
            g_stSnapshotCtx.stRunLogCache.writeOft += len;
        }

        xSemaphoreGive(g_stSnapshotCtx.mutex);
    }
}

static void SSSnapshot_HandleRunningLog(void)
{
    uint8_t flushIdx = 0;

    if (g_stSnapshotCtx.stRunLogCache.flushFlag == TRUE)
    {
        flushIdx = g_stSnapshotCtx.stRunLogCache.writeIdx ^ 1;
        MSNvm_InsertNewRecord(eMSNvmBlockID_RunningLogRecord,
                              (uint8_t *)g_stSnapshotCtx.stRunLogCache.logBuf[flushIdx],
                              MSNVM_RUNNING_LOG_MAX_LEN);
        memset(g_stSnapshotCtx.stRunLogCache.logBuf[flushIdx], 0x00, MSNVM_RUNNING_LOG_MAX_LEN);
        g_stSnapshotCtx.stRunLogCache.flushFlag = FALSE;
    }
}

void SSSnapshot_InsertErrorLog(uint8_t port, char *pErrorInfo,  uint8_t flag)
{
    SSSnapshotErrorCache_Struct *pErrCache = NULL;
    char timeStr[32] = {0};
    uint16_t remainLen = 0;
    uint16_t currentPos = 0;

    if (pErrorInfo != NULL)
    {
        xSemaphoreTake(g_stSnapshotCtx.mutex, portMAX_DELAY);
        pErrCache = SSSnapshot_FindFreeErrCache();
        xSemaphoreGive(g_stSnapshotCtx.mutex);

        if (pErrCache != NULL)
        {
            SSTM_GetTimeStr(timeStr);
            snprintf(pErrCache->errInfoPack, SSSNAPSHOT_CFG_ERROR_INFO_SZIE, "[%s][枪：%d]故障%s: %s, ",
                        timeStr, port, (flag == TRUE) ? "产生" : "撤销", pErrorInfo);

            currentPos = strlen(pErrCache->errInfoPack);

            if (SSSNAPSHOT_CFG_ERROR_INFO_SZIE > currentPos)
            {
                remainLen = SSSNAPSHOT_CFG_ERROR_INFO_SZIE - currentPos;
                SSSnapshot_Cfg_PackErrStr(port, pErrCache->errInfoPack + currentPos, remainLen);
            }
        }

        pErrCache->dealFlag = SSSNAPSHOT_SAVE_STATE_FINISH;
   }
}

GlobalRet_Enum SSSnapshot_StartReadItem(SSSnapshotItemType_Enum eItemType)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;
    GlobalRet_Enum eRet = eGlobalRet_OK;

    if (pReadItemHandle->readItemOngoing == TRUE)
    {
        eRet = eGlobalRet_DeviceBusy;
    }
    else
    {
        if (eItemType == eSSSnapshotItemType_ErrorLog)
        {
            pReadItemHandle->latestTime = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_ErrorRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_ErrorRecord;
            pReadItemHandle->singleItemSize = MSNVM_ERROR_INFO_MAX_LEN;
            pReadItemHandle->sizeFlag = FALSE;

            if (0 == pReadItemHandle->latestTime)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else if (eItemType == eSSSnapshotItemType_RunningLog)
        {
            uint32_t totalCount = 0;
            uint32_t maxRecordCount = 0;
            uint32_t startRecord = 0;

            pReadItemHandle->latestTime = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_RunningLogRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_RunningLogRecord;
            pReadItemHandle->singleItemSize = MSNVM_RUNNING_LOG_MAX_LEN;
            pReadItemHandle->sizeFlag = FALSE;

            if (0 == pReadItemHandle->latestTime)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
            else
            {
                totalCount = pReadItemHandle->latestTime;
                
                maxRecordCount = (100 * 1024) / pReadItemHandle->singleItemSize;
                
                if (totalCount > maxRecordCount)
                {
                    startRecord = totalCount - maxRecordCount;
                }
                
                pReadItemHandle->startReadTime = startRecord + 1;
                pReadItemHandle->readDirection = TRUE;
            }
        }
        else if (eItemType == eSSSnapshotItemType_OmOrderRecord)
        {
            pReadItemHandle->latestTime = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_OmOrderRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_OmOrderRecord;
            pReadItemHandle->singleItemSize = sizeof(MSNvmOrderInfo_Struct);
            pReadItemHandle->sizeFlag = TRUE;

            if (0 == pReadItemHandle->latestTime)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else
        {
            eRet = eGlobalRet_ParaInvalid;
        }
    }

    if (eRet == eGlobalRet_OK)
    {
        if (pReadItemHandle->pItemBuf != NULL)
        {
            myFree(pReadItemHandle->pItemBuf);
            pReadItemHandle->pItemBuf = NULL;
        }

        pReadItemHandle->pItemBuf = myCalloc(MSNVM_RUNNING_LOG_MAX_LEN, 1);
        
        if (pReadItemHandle->pItemBuf == NULL)
        {
            eRet = eGlobalRet_NotEnoughBuf;
        }
        else
        {
            pReadItemHandle->eItemType = eItemType;
            
            if (pReadItemHandle->readDirection == TRUE)
            {
                pReadItemHandle->currentReadTime = pReadItemHandle->startReadTime;
            }
            else
            {
                pReadItemHandle->currentReadTime = pReadItemHandle->latestTime;
            }
            
            pReadItemHandle->readItemOngoing = TRUE;
            pReadItemHandle->readOffset = 0;
            pReadItemHandle->remainSize = 0;
        }
    }

    return eRet;
}

void SSSnapshot_StopReadItem(void)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;

    if (pReadItemHandle->readItemOngoing != FALSE)
    {
        if (pReadItemHandle->pItemBuf != NULL)
        {
            myFree(pReadItemHandle->pItemBuf);
            pReadItemHandle->pItemBuf = NULL;
        }

        memset(pReadItemHandle, 0x00, sizeof(SSSnapshotReadItem_Struct));
        SSSNAPSHOT_CFG_InfoPrint("停止读取快照!\r\n");
    }
}

uint8_t SSSnapshot_ReadItem(uint8_t *pOutbuf, uint16_t bufSize, uint16_t *pOutLen)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;
    uint8_t ret = FALSE;

    PARA_ASSERT_RET(pOutbuf != NULL && pOutLen != NULL && bufSize > 0, ret);

    if (pReadItemHandle->readItemOngoing == FALSE)
    {
        ret = FALSE;
    }
    else
    {
        if (pReadItemHandle->remainSize == 0)
        {
            if (pReadItemHandle->readDirection == TRUE)
            {
                if (pReadItemHandle->currentReadTime <= pReadItemHandle->latestTime)
                {
                    if (eGlobalRet_OK == MSNvm_QueryRecordByTime(pReadItemHandle->readBlockID,  pReadItemHandle->pItemBuf, 
                                                                pReadItemHandle->singleItemSize, pReadItemHandle->currentReadTime))
                    { 
                        pReadItemHandle->readOffset = 0;
                        pReadItemHandle->remainSize = pReadItemHandle->singleItemSize;
                        pReadItemHandle->currentReadTime++;
                    }
                }
            }
            else
            {
                if (pReadItemHandle->currentReadTime > 0)
                {
                    if (eGlobalRet_OK == MSNvm_QueryRecordByTime(pReadItemHandle->readBlockID,  pReadItemHandle->pItemBuf, 
                                                                pReadItemHandle->singleItemSize, pReadItemHandle->currentReadTime))
                    { 
                        pReadItemHandle->readOffset = 0;
                        pReadItemHandle->remainSize = pReadItemHandle->singleItemSize;
                        pReadItemHandle->currentReadTime--;
                    }
                }
            }
        }

        if (pReadItemHandle->remainSize > 0)
        {
            if (bufSize > pReadItemHandle->remainSize)
            {
                memcpy(pOutbuf, pReadItemHandle->pItemBuf + pReadItemHandle->readOffset, pReadItemHandle->remainSize);
                pOutLen[0] = pReadItemHandle->remainSize;
                pReadItemHandle->remainSize = 0;
            }
            else
            {
                memcpy(pOutbuf, pReadItemHandle->pItemBuf + pReadItemHandle->readOffset, bufSize);
                pReadItemHandle->readOffset += bufSize;
                pReadItemHandle->remainSize -= bufSize;
                pOutLen[0] = bufSize;
            }

            ret = TRUE;
        }
        else
        {
            SSSNAPSHOT_CFG_InfoPrint("快照已全部取出!\r\n");
            SSSnapshot_StopReadItem();
        }
    }

    return ret;
}

uint8_t SSSnapshot_PreviewReadItem(uint16_t bufSize, uint16_t *pOutLen)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;
    uint8_t ret = FALSE;
    uint8_t startResult = TRUE;

    PARA_ASSERT_RET(pOutLen != NULL && bufSize > 0, ret);

     if (pReadItemHandle->readItemOngoing == FALSE)
    {
        ret = FALSE;
    }
    else
    {
        if (pReadItemHandle->remainSize == 0)
        {
            if (pReadItemHandle->currentReadTime > 0)
            {
                memset(pReadItemHandle->pItemBuf, 0x00, pReadItemHandle->singleItemSize);
    
                if (eGlobalRet_OK == MSNvm_QueryRecordByTime(pReadItemHandle->readBlockID,  pReadItemHandle->pItemBuf, 
                                                            pReadItemHandle->singleItemSize,pReadItemHandle->currentReadTime))
                {
                    pReadItemHandle->readOffset = 0;
                    pReadItemHandle->remainSize = pReadItemHandle->sizeFlag ? pReadItemHandle->singleItemSize : strlen((char *)pReadItemHandle->pItemBuf);
                    pReadItemHandle->currentReadTime--;
                }
            }
        }

        if (pReadItemHandle->remainSize > 0)
        {
            if (bufSize > pReadItemHandle->remainSize)
            {
                pOutLen[0] = pReadItemHandle->remainSize;
            }
            else
            {
                pOutLen[0] = bufSize;
            }

            ret = TRUE;
        }
        else
        {
            pReadItemHandle->readItemOngoing = FALSE;
            SSSNAPSHOT_CFG_InfoPrint("快照已全部取出!\r\n");
            if (pReadItemHandle->pItemBuf != NULL)
            {
                myFree(pReadItemHandle->pItemBuf);
                pReadItemHandle->pItemBuf = NULL;
            }
        }
    }

    return ret;
}

void SSSnapshot_ExportTimeoutHandle(void)
{
    SSSnapshotReadItem_Struct *pReadItemHandle = &g_stSnapshotCtx.stReadItem;

    if (pReadItemHandle->readItemOngoing == TRUE)
    {
        if (Common_JudgeTimeoutMs(pReadItemHandle->exportItemStartTick, SSSNAPSHOT_CFG_EXPORT_TIMEOUT))
        {
            SSSNAPSHOT_CFG_InfoPrint("读取快照超时!\r\n");

            if (pReadItemHandle->eReadSrc == eSnapshotItemReadSrc_Remote)
            {
                CddNetM_DeleteLink(eCddNetMPlatType_File);
            }

            SSSnapshot_StopReadItem();
        }
    }
}

uint8_t SSSnapshot_ExportAllItems(CddNetMSocketPara_Union *pNetPara)
{
    if (g_stSnapshotCtx.stReadItem.readItemOngoing == TRUE)
    {
        SSSNAPSHOT_CFG_InfoPrint("快照读取已开始!\r\n");
        return FALSE;
    }

    if (pNetPara != NULL)
    {/* 如果为NULL, 则用默认 */
        memcpy(&g_stSnapshotCtx.stNetPara, pNetPara, sizeof(CddNetMSocketPara_Union));
    }
    
    g_stSnapshotCtx.eState = eSSSnapshotItemState_ErrLog;

    return TRUE;
}

static void SSSnapshot_ExportHandle(void)
{
    switch (g_stSnapshotCtx.eState)
    {
        case eSSSnapshotItemState_ErrLog:
            SSSnapshot_ExportItem(eSSSnapshotItemType_ErrorLog, eSnapshotItemReadSrc_Remote);
            g_stSnapshotCtx.eState = eSSSnapshotItemState_WaitErrLog;
            break;

        case eSSSnapshotItemState_WaitErrLog:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                SSSnapshot_FlushRunningLog();
                g_stSnapshotCtx.WaitTick = Common_GetSystick();
                g_stSnapshotCtx.eState = eSSSnapshotItemState_RunLog;
            }
            break;

        case eSSSnapshotItemState_RunLog:
            if (Common_JudgeTimeoutMs(g_stSnapshotCtx.WaitTick, 3000))
            {/* 等ftp关闭稳定 */
                SSSnapshot_ExportItem(eSSSnapshotItemType_RunningLog, eSnapshotItemReadSrc_Remote);
                g_stSnapshotCtx.eState = eSSSnapshotItemState_WaitRunLog;
            }
            break;

        case eSSSnapshotItemState_WaitRunLog:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                g_stSnapshotCtx.WaitTick = Common_GetSystick();
                g_stSnapshotCtx.eState = eSSSnapshotItemState_OmOrderRecord;
            }
            break;
        case eSSSnapshotItemState_OmOrderRecord:
            if (Common_JudgeTimeoutMs(g_stSnapshotCtx.WaitTick, 3000))
            {/* 等ftp关闭稳定 */
                SSSnapshot_ExportItem(eSSSnapshotItemType_OmOrderRecord, eSnapshotItemReadSrc_Remote);
                g_stSnapshotCtx.eState = eSSSnapshotItemState_WaitOmOrderRecord;
            }
            break;
        case eSSSnapshotItemState_WaitOmOrderRecord:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                g_stSnapshotCtx.eState = eSSSnapshotItemState_Idle;
            }
            break;
        default:
            break;
    }
}

static void SSSnapshot_NetParaDefault(void)
{
    g_stSnapshotCtx.stNetPara.stFtpPara.eMode = eCddNetMFtpMode_Upload;
    g_stSnapshotCtx.stNetPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;
    g_stSnapshotCtx.stNetPara.stFtpPara.port = SSSNAPSHOT_CFG_RMTPORT_DEFAULT;

    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.user, SSSNAPSHOT_CFG_USER_DEFAULT, CDD_NETM_CFG_FTP_USERNAME_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.passwd, SSSNAPSHOT_CFG_PWD_DEFAULT, CDD_NETM_CFG_FTP_PASSWD_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.ip, SSSNAPSHOT_CFG_RMTIP_DEFAULT, CDD_NETM_CFG_IP_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.path, SSSNAPSHOT_CFG_RMTPATH_DEFAULT, CDD_NETM_CFG_FTP_PATH_LEN);
}

void SSSnapshot_InitMemory(void)
{
    memset(&g_stSnapshotCtx, 0x00, sizeof(SSSnapshotCtx_Struct));
    g_stSnapshotCtx.mutex = xSemaphoreCreateMutex();
    DSLogM_RegisterRunLogCb(SSSnapshot_InsertRunningLog);

    SSSnapshot_NetParaDefault();
}

void SSSnapshot_MainFunction(void)
{
    SSSnapshot_HandleErrLog();
    SSSnapshot_HandleRunningLog();

    SSSnapshot_PrintItemInfo();

    SSSnapshot_ExportHandle();
    SSSnapshot_ExportTimeoutHandle();
}
















