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
#include "FreeRTOS.h"
#include "semphr.h"
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
typedef struct SS_Snapshot
{
    uint8_t dealFlag;
    char errInfoPack[SSSNAPSHOT_CFG_ERROR_INFO_SZIE];
}SSSnapshotErrorCache_Struct;

typedef struct 
{
    SSSnapshotErrorCache_Struct stErrorCache[SSSNAPSHOT_CFG_ERROR_ITEM_COUNT];
    SemaphoreHandle_t mutex;
    uint32_t errLogTotalCount;
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


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void SSSnapshot_InsertItem(void)
{



}

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

void SSSnapshot_InsertRunningLog(void)
{


}

uint8_t SSSnapshot_ReadItemByTime(SSSnapshotItemType_Enum itemType, uint8_t *buf, uint16_t buffLen, uint32_t time)
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

uint32_t SSSnapshot_ReadItemCount(SSSnapshotItemType_Enum itemType)
{
    uint32_t count = 0;

    switch (itemType)
    {
        case eSSSnapshotItemType_ErrorLog:
        {
            count = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_ErrorRecord);
            break;
        }
        case eSSSnapshotItemType_RunningLog:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    return count;
}

void SSSnapshot_InsertErrorItem(uint8_t port, char *pErrorInfo,  uint8_t flag)
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

void SSSnapshot_InitMemory(void)
{
    memset(&g_stSnapshotCtx, 0x00, sizeof(SSSnapshotCtx_Struct));
    g_stSnapshotCtx.mutex = xSemaphoreCreateMutex();
}

void SSSnapshot_MainFunction(void)
{
    SSSnapshot_HandleErrLog();
}
















