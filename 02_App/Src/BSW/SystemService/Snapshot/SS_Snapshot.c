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
    uint32_t totalCount;
    uint32_t currentReadTime;
    uint16_t singleItemSize;
    uint8_t sizeFlag;
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
    SSSnapshotItemRead_Enum eState;
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
                    SSSNAPSHOT_CFG_LogPrint("日志[%d 条]:\n", g_stSnapshotCtx.stReadItem.currentReadTime);
                    SSSNAPSHOT_CFG_LogPrint("\n%s", data);
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

    if (pReadItemHandle->readItemOngoing == TRUE)
    {
        SSSNAPSHOT_CFG_LogPrint("快照读取已开始!\r\n");
        return;
    }

    if (eReadSrc == eSnapshotItemReadSrc_Local)
    {/* 本地导出 */
        pReadItemHandle->exportItemStartTick = Common_GetSystick();
        pReadItemHandle->eReadSrc = eReadSrc;
        pReadItemHandle->localPrintItemFlag = TRUE;

        SSSnapshot_StartReadItem(itemType);
        SSSNAPSHOT_CFG_LogPrint("开始本地快照读取!\r\n");
        return;
    }

    /* 远程导出 */
    do
    {
        /* 注册升级链接 for test */
        CddNetMSocketPara_Union *pSocketPara = &g_stSnapshotCtx.stNetPara;
        CommonDateTime_Struct sttime;

        SSTM_GetDateTime(&sttime);

        const char *snapshotName = "ErrSnapshot";
        if (itemType == eSSSnapshotItemType_RunningLog)
        {
            snapshotName = "RunLogSnapshot";
        }
        else if (itemType == eSSSnapshotItemType_OmOrderRecord)
        {
            snapshotName = "OmOrderRecordSnapshot";
        }

        snprintf(pSocketPara->stFtpPara.fileName, CDD_NETM_CFG_FTP_FILENAME_LEN + 1, "%s_%s_%04d%02d%02d.txt", 
                    SYSCFG_CFG_PRODUCT_CODE, snapshotName, sttime.year, sttime.month, sttime.day);

        if (eGlobalRet_OK != CddNetM_CreatLink(eCddNetMSocketType_FTP, *pSocketPara, eCddNetMPlatType_File))
        {
            SSSNAPSHOT_CFG_LogPrint("创建FTP传输通道失败!\r\n");
            break;
        }

        pReadItemHandle->exportItemStartTick = Common_GetSystick();
        pReadItemHandle->eReadSrc = eReadSrc;
        pReadItemHandle->localPrintItemFlag = FALSE;

        SSSnapshot_StartReadItem(itemType);
        //CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
        //CddNetM_SetLinkDisconnect(eCddNetMPlatType_OM);
        SSSNAPSHOT_CFG_LogPrint("开始远程快照[%d]读取!\r\n", itemType);
        
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

    if (buf == NULL)
    {
        return;
    }

    if (len == 0 || len >= MSNVM_RUNNING_LOG_MAX_LEN)
    {
        return;
    }

    xSemaphoreTake(g_stSnapshotCtx.mutex, portMAX_DELAY);

    remain = MSNVM_RUNNING_LOG_MAX_LEN - g_stSnapshotCtx.stRunLogCache.writeOft - 1;//预留\0

    if (len > remain)
    {
        /* 当前缓冲已满，尝试切换到另一个缓冲 */
        if (g_stSnapshotCtx.stRunLogCache.flushFlag == FALSE)
        {
            /* 标记当前缓冲待 flush，切换写缓冲 */
            g_stSnapshotCtx.stRunLogCache.flushFlag = TRUE;
            g_stSnapshotCtx.stRunLogCache.writeIdx ^= 1;
            g_stSnapshotCtx.stRunLogCache.writeOft = 0;
        }
        else
        {
            /* 另一个缓冲还未被 flush，两个缓冲都占用中，丢弃本次内容 */
            xSemaphoreGive(g_stSnapshotCtx.mutex);
            return;
        }
    }

    memcpy(g_stSnapshotCtx.stRunLogCache.logBuf[g_stSnapshotCtx.stRunLogCache.writeIdx] + \
            g_stSnapshotCtx.stRunLogCache.writeOft, buf, len);
    g_stSnapshotCtx.stRunLogCache.writeOft += len;

    xSemaphoreGive(g_stSnapshotCtx.mutex);
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
            pReadItemHandle->totalCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_ErrorRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_ErrorRecord;
            pReadItemHandle->singleItemSize = MSNVM_ERROR_INFO_MAX_LEN;
            pReadItemHandle->sizeFlag = FALSE;

            if (0 == pReadItemHandle->totalCount)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else if (eItemType == eSSSnapshotItemType_RunningLog)
        {
            pReadItemHandle->totalCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_RunningLogRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_RunningLogRecord;
            pReadItemHandle->singleItemSize = MSNVM_RUNNING_LOG_MAX_LEN;
            pReadItemHandle->sizeFlag = FALSE;

            if (0 == pReadItemHandle->totalCount)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else if (eItemType == eSSSnapshotItemType_OmOrderRecord)
        {
            pReadItemHandle->totalCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_OmOrderRecord);
            pReadItemHandle->readBlockID = eMSNvmBlockID_OmOrderRecord;
            pReadItemHandle->singleItemSize = sizeof(MSNvmOrderInfo_Struct);
            pReadItemHandle->sizeFlag = TRUE;

            if (0 == pReadItemHandle->totalCount)
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
            pReadItemHandle->currentReadTime = pReadItemHandle->totalCount;
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
        SSSNAPSHOT_CFG_LogPrint("停止读取快照!\r\n");
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
            if (pReadItemHandle->currentReadTime > 0)
            {
    
                if (eGlobalRet_OK == MSNvm_QueryRecordByTime(pReadItemHandle->readBlockID,  pReadItemHandle->pItemBuf, 
                                                            pReadItemHandle->singleItemSize,pReadItemHandle->currentReadTime))
                {
                    pReadItemHandle->readOffset = 0;
                    pReadItemHandle->remainSize = pReadItemHandle->singleItemSize;
                    pReadItemHandle->currentReadTime--;
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
            SSSNAPSHOT_CFG_LogPrint("快照已全部取出!\r\n");
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
            SSSNAPSHOT_CFG_LogPrint("快照已全部取出!\r\n");
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
            SSSNAPSHOT_CFG_LogPrint("读取快照超时!\r\n");

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
        SSSNAPSHOT_CFG_LogPrint("FAILED, ReadItem is Going\r\n");
        return FALSE;
    }

    if (pNetPara != NULL)
    {/* 如果为NULL, 则用默认 */
        memcpy(&g_stSnapshotCtx.stNetPara, pNetPara, sizeof(CddNetMSocketPara_Union));
    }
    
    g_stSnapshotCtx.eState = eSSSnapshotItemRead_ErrLog;

    return TRUE;
}

static void SSSnapshot_ExportHandle(void)
{
    switch (g_stSnapshotCtx.eState)
    {
        case eSSSnapshotItemRead_ErrLog:
            SSSnapshot_ExportItem(eSSSnapshotItemType_ErrorLog, eSnapshotItemReadSrc_Remote);
            g_stSnapshotCtx.eState = eSSSnapshotItemRead_WaitErrLog;
            break;

        case eSSSnapshotItemRead_WaitErrLog:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                SSSnapshot_FlushRunningLog();
                g_stSnapshotCtx.WaitTick = Common_GetSystick();
                g_stSnapshotCtx.eState = eSSSnapshotItemRead_RunLog;
            }
            break;

        case eSSSnapshotItemRead_RunLog:
            if (Common_JudgeTimeoutMs(g_stSnapshotCtx.WaitTick, 3000))
            {/* 等ftp关闭稳定 */
                SSSnapshot_ExportItem(eSSSnapshotItemType_RunningLog, eSnapshotItemReadSrc_Remote);
                g_stSnapshotCtx.eState = eSSSnapshotItemRead_WaitRunLog;
            }
            break;

        case eSSSnapshotItemRead_WaitRunLog:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                g_stSnapshotCtx.WaitTick = Common_GetSystick();
                g_stSnapshotCtx.eState = eSSSnapshotItemRead_OmOrderRecord;
            }
            break;
        case eSSSnapshotItemRead_OmOrderRecord:
            if (Common_JudgeTimeoutMs(g_stSnapshotCtx.WaitTick, 3000))
            {/* 等ftp关闭稳定 */
                SSSnapshot_ExportItem(eSSSnapshotItemType_OmOrderRecord, eSnapshotItemReadSrc_Remote);
                g_stSnapshotCtx.eState = eSSSnapshotItemRead_WaitOmOrderRecord;
            }
            break;
        case eSSSnapshotItemRead_WaitOmOrderRecord:
            if (g_stSnapshotCtx.stReadItem.readItemOngoing == FALSE)
            {
                g_stSnapshotCtx.eState = eSSSnapshotItemRead_Idle;
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
    g_stSnapshotCtx.stNetPara.stFtpPara.port = 21;

    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.user, "gn_ftp_fw_cls", CDD_NETM_CFG_FTP_USERNAME_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.passwd, "24d79794d8b42ff5", CDD_NETM_CFG_FTP_PASSWD_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.ip, "fwftp.gongniu.cn", CDD_NETM_CFG_IP_LEN + 1);
    strncpy(g_stSnapshotCtx.stNetPara.stFtpPara.path, "/AC_pile/D3_A32FB/", CDD_NETM_CFG_FTP_PATH_LEN);
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
















