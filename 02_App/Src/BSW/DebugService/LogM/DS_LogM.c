/******************************************************************************
* File Name          : DS_LogM.c
* Description        : Code for log manage
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
#include "DS_LogMConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "Mcal_Uart.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eDSLogMOutputDirection_Null,
    eDSLogMOutputDirection_Uart,
    eDSLogMOutputDirection_File,
    eDSLogMOutputDirection_Count,
}DSLogMOutputDirection_Enum;

typedef enum
{
    eDSLogMOutputMode_Sync,
    eDSLogMOutputMode_Asyn,
}DSLogMOutputMode_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    DSLogMOutputDirection_Enum eOutputDirection;
    SemaphoreHandle_t mutex;
    DSLogOutputLevel_Enum eOutputLevel;
    uint8_t cacheBuf[DSLOGM_CFG_ASYN_BUFF_SIZE];
    uint16_t logDataLen;
}DSLogMCtrl_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static DSLogMCtrl_Struct g_stLogMCtrl;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void DSLogM_OutputFilter(DSLogMModule_Enum eModule, DSLogOutputLevel_Enum eLevel, const char *fmt, va_list args);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void DSLogM_Output(DSLogMModule_Enum eModule, DSLogOutputLevel_Enum eLevel, const char *fmt, ...)
{
    xSemaphoreTake(g_stLogMCtrl.mutex, portMAX_DELAY);
    va_list args;
    va_start(args, fmt);
    DSLogM_OutputFilter(eModule, eLevel, fmt, args);
    va_end(args);
    xSemaphoreGive(g_stLogMCtrl.mutex);
}

void DSLogM_InitMemory(void)
{
    memset(&g_stLogMCtrl, 0x00, sizeof(g_stLogMCtrl));
    g_stLogMCtrl.mutex = xSemaphoreCreateMutex();
}



static void DSLogM_OutputFilter(DSLogMModule_Enum eModule, DSLogOutputLevel_Enum eLevel, const char *fmt, va_list args)
{
    uint16_t totalLen = g_stLogMCtrl.logDataLen;
    uint16_t remainBufsize = (totalLen >= DSLOGM_CFG_ASYN_BUFF_SIZE) ? 0 : (DSLOGM_CFG_ASYN_BUFF_SIZE - totalLen);
    uint16_t dataLen = 0;

    dataLen = vsnprintf((char *)g_stLogMCtrl.cacheBuf + totalLen, remainBufsize, fmt, args);

    if (dataLen >= remainBufsize)
    {
        g_stLogMCtrl.logDataLen = DSLOGM_CFG_ASYN_BUFF_SIZE;
    }
    else
    {
        g_stLogMCtrl.logDataLen += dataLen;
    }

    McalUart_WriteData(eMcalUartChanel_Debug, g_stLogMCtrl.cacheBuf, g_stLogMCtrl.logDataLen);
    g_stLogMCtrl.logDataLen = 0;
}

const char* DSLogM_GetModuleName(DSLogMModule_Enum eModule)
{
    const char *p = NULL;

    if (eModule < DSLogMModule_Count)
    {
        p = g_logMModuleName[eModule];
    }

    return p;
}





















