/******************************************************************************
* File Name          : SS_WdgM.c
* Description        : Code for the Implementation of the Watchdog Monitor
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/03/02      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "SS_WdgM.h"
#include "SS_WdgMConfig.h"




/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
/* 模块信息结构体 */
typedef struct
{
    uint8_t   registered;         /* 模块是否已注册 */
    uint32_t  timeout;            /* 超时时间(ms) */
    uint32_t  lastCheckinTime;    /* 最后签到时间 */
    char*     moduleName;         /* 模块名称 */
}SSWdgModuleInfo_Struct;

/* 上下文结构体 */
typedef struct
{
    uint8_t   moduleCount;        /* 当前注册的模块数 */
    uint32_t  lastCheckTime;      /* 最后检查时间 */
    uint8_t   hasTimeout;         /* 是否有模块超时 */
}SSWdgCtx_Struct;

/* 内部上下文结构体 */
typedef struct
{
    SSWdgModuleInfo_Struct modules[SSWDGM_CFG_MAX_MODULES]; /* 模块信息数组 */
    SSWdgCtx_Struct ctx;                                    /* 上下文 */
}SSWdgInternalCtx_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SSWdgInternalCtx_Struct g_stSSWdgCtx = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void SSWdgM_CheckTimeout(void);
static uint8_t SSWdgM_FindAvailableModuleId(void);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t SSWdgM_FindAvailableModuleId(void)
{
    uint8_t moduleId;
    uint8_t availableId = 0xFF;
    
    /* 寻找可用的模块ID */
    for (moduleId = 0; moduleId < SSWDGM_CFG_MAX_MODULES; moduleId++)
    {
        if (!g_stSSWdgCtx.modules[moduleId].registered)
        {
            availableId = moduleId;
            break;
        }
    }
    
    return availableId; /* 返回可用的模块ID，没有可用时返回0xFF */
}

static void SSWdgM_CheckTimeout(void)
{
    uint8_t moduleIndex;
    uint8_t hasTimeout = FALSE;
    
    for (moduleIndex = 0; moduleIndex < g_stSSWdgCtx.ctx.moduleCount; moduleIndex++)
    {
        if (g_stSSWdgCtx.modules[moduleIndex].registered)
        {
            if (Common_JudgeTimeoutMs(g_stSSWdgCtx.modules[moduleIndex].lastCheckinTime, g_stSSWdgCtx.modules[moduleIndex].timeout))
            {
                /* 模块超时 */
                SSWDGM_CFG_DebugPrint("Watchdog timeout: module %s (ID: %d), timeout: %dms\r\n", 
                    g_stSSWdgCtx.modules[moduleIndex].moduleName, moduleIndex, g_stSSWdgCtx.modules[moduleIndex].timeout);
                
                hasTimeout = TRUE;
            }
        }
    }
    
    g_stSSWdgCtx.ctx.hasTimeout = hasTimeout;
}

void SSWdgM_InitMemory(void)
{
    SSWdgWdgIndex_Enum wdgIndex;
    uint8_t moduleIndex;
    
    /* 初始化结构体 */
    memset(&g_stSSWdgCtx, 0, sizeof(SSWdgInternalCtx_Struct));
    g_stSSWdgCtx.ctx.lastCheckTime = Common_GetSystick();
    g_stSSWdgCtx.ctx.hasTimeout = FALSE;
    
    /* 初始化看门狗 */
    for (wdgIndex = eSSWdgWdgIndex_Internal; wdgIndex < eSSWdgWdgIndex_Count; wdgIndex++)
    {
        if (c_SSWdgConfig.wdgObjs[wdgIndex].enable && c_SSWdgConfig.wdgObjs[wdgIndex].pWdgInit != NULL)
        {
            c_SSWdgConfig.wdgObjs[wdgIndex].pWdgInit();
        }
    } 
}

void SSWdgM_MainFunction(void)
{
    uint32_t currentTime = Common_GetSystick();
    SSWdgWdgIndex_Enum wdgIndex;
    
    /* 检查是否到了检查间隔，且没有超时 */
    if (Common_JudgeTimeoutMs(g_stSSWdgCtx.ctx.lastCheckTime, SSWDGM_CFG_CHECK_INTERVAL))
    {
        SSWdgM_CheckTimeout();
        g_stSSWdgCtx.ctx.lastCheckTime = currentTime;
    }
    
    /* 只有当没有线程超时时才喂狗 */
    if (TRUE != g_stSSWdgCtx.ctx.hasTimeout)
    {
        for (wdgIndex = eSSWdgWdgIndex_Internal; wdgIndex < eSSWdgWdgIndex_Count; wdgIndex++)
        {
            if (c_SSWdgConfig.wdgObjs[wdgIndex].enable && c_SSWdgConfig.wdgObjs[wdgIndex].pWdgFeed != NULL)
            {
                c_SSWdgConfig.wdgObjs[wdgIndex].pWdgFeed();
            }
        }
    }
}

SSWdgStatus_Enum SSWdgM_RegisterModule(const char *moduleName, uint32_t timeout, uint8_t *pModuleId)
{
    uint8_t moduleId;
    SSWdgStatus_Enum eStatus = eSSWdgStatus_Ok;
    
    if (pModuleId == NULL)
    {
        eStatus = eSSWdgStatus_Error;
    }
    else
    {
        /* 寻找可用的模块ID */
        moduleId = SSWdgM_FindAvailableModuleId();
        
        if (moduleId == 0xFF)
        {
            eStatus = eSSWdgStatus_Full; /* 没有可用的模块ID */
        }
        else
        {
            /* 注册模块 */
            g_stSSWdgCtx.modules[moduleId].registered      = TRUE;
            g_stSSWdgCtx.modules[moduleId].timeout         = timeout;
            g_stSSWdgCtx.modules[moduleId].lastCheckinTime = Common_GetSystick();
            g_stSSWdgCtx.modules[moduleId].moduleName      = (char *)moduleName;
            g_stSSWdgCtx.ctx.moduleCount++;
            /* 输出模块ID */
            *pModuleId = moduleId;
        }
    }
    
    return eStatus;
}

SSWdgStatus_Enum SSWdgM_Checkin(uint8_t moduleId)
{
    SSWdgStatus_Enum eStatus = eSSWdgStatus_Ok;
    
    if (moduleId >= SSWDGM_CFG_MAX_MODULES)
    {
        eStatus = eSSWdgStatus_Error;
    }
    else if (!g_stSSWdgCtx.modules[moduleId].registered)
    {
        eStatus = eSSWdgStatus_NotFound;
    }
    else
    {
        /* 更新最后签到时间 */
        g_stSSWdgCtx.modules[moduleId].lastCheckinTime = Common_GetSystick();
    }
    
    return eStatus;
}
