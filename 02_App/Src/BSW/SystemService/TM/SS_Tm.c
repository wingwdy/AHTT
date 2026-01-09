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
#include "SS_Tm.h"
#include "SS_TmConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define SSTM_BASE_TIMESTAMP_1970_BJT    28800U    /* 1970年1月1日0时0分0秒对应的北京时间时间戳偏移(28800秒 = 8小时) */




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint64_t msecSysTimeStamp;
    uint32_t secSysTimestamp;
    uint32_t lastTickCount;
    uint8_t syncSysTimeFlag;
    char timeStr[32];
    SemaphoreHandle_t mutex;
}SSTimeCtrl_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SSTimeCtrl_Struct g_stTmCtrl;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void SSTM_UpdateSysTimeStamp(void);
static uint8_t SSTM_CheckTimeStampDff(uint32_t newTimeStamp);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static void SSTM_UpdateSysTimeStamp(void)
{
    uint32_t currentTickCount = Common_GetSystick();
    uint32_t tickDiff = 0;
    CommonDateTime_Struct dateTime;

    xSemaphoreTake(g_stTmCtrl.mutex, portMAX_DELAY);

    if (currentTickCount >= g_stTmCtrl.lastTickCount)
    {
        tickDiff = currentTickCount - g_stTmCtrl.lastTickCount;
    }
    else
    {
        tickDiff = (0xFFFFFFFFU - g_stTmCtrl.lastTickCount) + currentTickCount + 1U;
    }

    g_stTmCtrl.msecSysTimeStamp += tickDiff;

    if (tickDiff >= 1000U)
    {
        g_stTmCtrl.secSysTimestamp += (tickDiff / 1000U);
        g_stTmCtrl.lastTickCount = currentTickCount;
    }

    Conmon_TimestampToDateTime(g_stTmCtrl.secSysTimestamp, &dateTime);

    sprintf(g_stTmCtrl.timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
        dateTime.year, dateTime.month, dateTime.day, 
        dateTime.hour, dateTime.minute, dateTime.second);

    xSemaphoreGive(g_stTmCtrl.mutex);
}

static uint8_t SSTM_CheckTimeStampDff(uint32_t newTimeStamp)
{
    uint32_t tickDiff = 0;
    uint8_t ret = FALSE;

    if (g_stTmCtrl.secSysTimestamp >= newTimeStamp)
    {
        tickDiff = g_stTmCtrl.secSysTimestamp - newTimeStamp;
    }
    else
    {
        tickDiff = newTimeStamp - g_stTmCtrl.secSysTimestamp;
    }

    if (tickDiff > TM_SYNC_THRESHOLD_SECONDS)
    {
        ret = TRUE;
    }

    return ret;
}

void SSTM_SynTimeBySecTimeStamp(uint32_t neWSecTimeStamp)
{
    CommonDateTime_Struct dateTime;
    uint32_t tickDiff = 0;

    xSemaphoreTake(g_stTmCtrl.mutex, portMAX_DELAY);

    if (TRUE == SSTM_CheckTimeStampDff(neWSecTimeStamp))
    {
        g_stTmCtrl.secSysTimestamp = neWSecTimeStamp;
        g_stTmCtrl.msecSysTimeStamp = neWSecTimeStamp * 1000;
        g_stTmCtrl.lastTickCount = Common_GetSystick();

        Conmon_TimestampToDateTime(g_stTmCtrl.secSysTimestamp, &dateTime);

        sprintf(g_stTmCtrl.timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
        dateTime.year, dateTime.month, dateTime.day, 
        dateTime.hour, dateTime.minute, dateTime.second);
    }

    g_stTmCtrl.syncSysTimeFlag = TRUE;
    xSemaphoreGive(g_stTmCtrl.mutex);
}

void SSTM_SynTimeByDateTime(CommonDateTime_Struct *pTime)
{
    uint32_t timeStamp = 0;

    if (pTime != NULL)
    {
        timeStamp = Common_DateTimeToTimestamp(pTime);
        SSTM_SynTimeBySecTimeStamp(timeStamp);
    }
}

uint8_t SSTM_GetSyncTimeFlag(void)
{
    return g_stTmCtrl.syncSysTimeFlag;
}

void SSTM_InitMemory(void)
{
    g_stTmCtrl.msecSysTimeStamp = SSTM_BASE_TIMESTAMP_1970_BJT * 1000;
    g_stTmCtrl.secSysTimestamp = SSTM_BASE_TIMESTAMP_1970_BJT;
    g_stTmCtrl.syncSysTimeFlag = FALSE;
    g_stTmCtrl.mutex = xSemaphoreCreateMutex();
}

uint32_t SSTM_GetSecTimestamp(void)
{
    return g_stTmCtrl.secSysTimestamp;
}

uint32_t SSTM_GetMsecTimestamp(void)
{
    return g_stTmCtrl.msecSysTimeStamp;
}

void SSTM_MainFunction(void)
{
    SSTM_UpdateSysTimeStamp();
}

void SSTM_GetTimeStr(char *pTimeStr)
{
    if (pTimeStr != NULL)
    {
        memcpy(pTimeStr, g_stTmCtrl.timeStr, strlen(g_stTmCtrl.timeStr));
    }
}

