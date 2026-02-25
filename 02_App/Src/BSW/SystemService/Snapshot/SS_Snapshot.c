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
    uint32_t cycPrintTick;
    SSSnapshotReadItem_Struct stReadItem;
    SSSnapshotErrorCache_Struct stErrorCache[SSSNAPSHOT_CFG_ERROR_ITEM_COUNT];
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

    if (FALSE == pReadItemHandle->readItemOngoing)
    {
        pReadItemHandle->exportItemStartTick = Common_GetSystick();
        pReadItemHandle->eReadSrc = eReadSrc;

        if (eReadSrc == eSnapshotItemReadSrc_Local)
        {
            pReadItemHandle->localPrintItemFlag = TRUE;
            SSSnapshot_StartReadItem(itemType);
            SSSNAPSHOT_CFG_LogPrint("开始本地快照读取!\r\n");
        }
        else
        {
            /* 注册升级链接 for test */
            CddNetMSocketPara_Union stSocketPara = { 0 };

            stSocketPara.stFtpPara.eMode = eCddNetMFtpMode_Upload;
            strcpy(stSocketPara.stFtpPara.fileName, "D3_A32FB_20260128.log");
            strcpy(stSocketPara.stFtpPara.user, "gn_ftp_fw_cls");
            strcpy(stSocketPara.stFtpPara.passwd, "24d79794d8b42ff5");
            strcpy(stSocketPara.stFtpPara.ip, "fwftp.gongniu.cn");
            strcpy(stSocketPara.stFtpPara.path, "/AC_pile/D3_A32FB/");
            stSocketPara.stFtpPara.port = 21;
            stSocketPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;

            if (eGlobalRet_OK == CddNetM_CreatLink(eCddNetMSocketType_FTP, stSocketPara, eCddNetMPlatType_File))
            {
                SSSnapshot_StartReadItem(itemType);
                CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
                CddNetM_SetLinkDisconnect(eCddNetMPlatType_OM);
                SSSNAPSHOT_CFG_LogPrint("开始远程快照读取!\r\n");
            }
            else
            {
                SSSNAPSHOT_CFG_LogPrint("创建FTP传输通道失败!\r\n");
            }
        }
    }
    else
    {
        SSSNAPSHOT_CFG_LogPrint("快照读取已开始!\r\n");
    }
}

void SSSnapshot_InsertRunningLog(void)
{
    // todo

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
                    pReadItemHandle->remainSize = strlen((char *)pReadItemHandle->pItemBuf);
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
                CddNetM_DeleteFileLink();
            }

            SSSnapshot_StopReadItem();
        }
    }
}


void SSSnapshot_InitMemory(void)
{
    memset(&g_stSnapshotCtx, 0x00, sizeof(SSSnapshotCtx_Struct));
    g_stSnapshotCtx.mutex = xSemaphoreCreateMutex();
}

void SSSnapshot_MainFunction(void)
{
    SSSnapshot_HandleErrLog();

    SSSnapshot_PrintItemInfo();

    SSSnapshot_ExportTimeoutHandle();
}
















