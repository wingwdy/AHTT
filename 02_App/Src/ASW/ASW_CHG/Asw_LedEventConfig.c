/******************************************************************************
* File Name          : Asw_LedEventConfig.c
* Description        : Code for Led event manage
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
#include "Asw_LedEventConfig.h"
#include "Asw_Charge.h"
#include "Asw_ErrorHandle.h"
#include "Cdd_Relay.h"

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
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t AswLedEventCfg_GetGBIdleExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetQBIdleExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetStartingExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetChargingExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetPausingExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetStopFinishExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetConnectedUnAuthExsit(uint8_t port);
static uint8_t AswLedEventCfg_GetErrorStateExsit(uint8_t port);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const AswLedEventConfig_Struct  c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Count] =
{ 
    [eAswLedDevice0_DispType_None] = 
    {
        .pFuncGetStateExsist = NULL,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_0,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_0,  /* 全灭 */
        .desc = "初始状态",
    },

    [eAswLedDevice0_DispType_GBIdle] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetGBIdleExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_1,  /* 蓝蓝蓝 慢闪 */
        .desc = "国标待机",
    },

    [eAswLedDevice0_DispType_QBIdle] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetQBIdleExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_2, /* 绿绿绿 慢闪 */ 
        .desc = "企标待机",
    },

    [eAswLedDevice0_DispType_Starting] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetStartingExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_3, /* 黄黄黄 常亮 */ 
        .desc = "启动中",
    },

    [eAswLedDevice0_DispType_Charging] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetChargingExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_4, /* 绿绿绿 闪烁 */ 
        .desc = "正在充电",
    },

    [eAswLedDevice0_DispType_Pausing] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetPausingExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_5, /* 黄黄黄 常亮 */ 
        .desc = "暂停充电",
    },

    [eAswLedDevice0_DispType_StopFinish] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetStopFinishExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_6,  /* 绿绿绿 常亮 */ 
        .desc = "停止完成",
    },

    [eAswLedDevice0_DispType_ConnectdUnAuth] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetConnectedUnAuthExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_1,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_7,  /* 黄黄黄 闪烁 */
        .desc = "插枪未授权",
    },

    [eAswLedDevice0_DispType_ReadCardSucc] = 
    {
        .pFuncGetStateExsist = NULL,
        .eLedEventType = eAswLedEventype_Event,
        .eLedPriority = eAswLedPriority_5,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_8,  /* 绿绿绿 快闪 */
        .desc = "读卡成功",
    },

    [eAswLedDevice0_DispType_InvalidCard] = 
    {
        .pFuncGetStateExsist = NULL,
        .eLedEventType = eAswLedEventype_Event,
        .eLedPriority = eAswLedPriority_5,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_9, /* 红红红 快闪 */
        .desc = "无效卡",
    },

    [eAswLedDevice0_DispType_Error] = 
    {
        .pFuncGetStateExsist = AswLedEventCfg_GetErrorStateExsit,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_3,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_10, /* 红红红 常亮 */
        .desc = "设备故障",
    },

    [eAswLedDevice0_DispType_Offline] = 
    {
        .pFuncGetStateExsist = NULL,
        .eLedEventType = eAswLedEventType_State,
        .eLedPriority = eAswLedPriority_4,
        .ledDispType = CDD_LEDM_DEVICE0_DISP_TYPE_11, /* 白色 常亮 */
        .desc = "设备离线",
    },
};

static AswLedEventCtrl_Struct g_stAswLedDevice0EventCtrl[eAswLedDevice0_DispType_Count] = 
{
    [eAswLedDevice0_DispType_None] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_None],
    },

    [eAswLedDevice0_DispType_GBIdle] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_GBIdle],
    },

    [eAswLedDevice0_DispType_QBIdle] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_QBIdle],
    },

    [eAswLedDevice0_DispType_Starting] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Starting],
    },

    [eAswLedDevice0_DispType_Charging] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Charging],
    },

    [eAswLedDevice0_DispType_Pausing] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Pausing],
    },

    [eAswLedDevice0_DispType_StopFinish] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_StopFinish],
    },

    [eAswLedDevice0_DispType_ConnectdUnAuth] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_ConnectdUnAuth],
    },

    [eAswLedDevice0_DispType_ReadCardSucc] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_ReadCardSucc],
    },

    [eAswLedDevice0_DispType_InvalidCard] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_InvalidCard],
    },

    [eAswLedDevice0_DispType_Error] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Error],
    },

    [eAswLedDevice0_DispType_Offline] = 
    {
        .currentState = {0},
        .lastState = {0},
        .pLedEventConfig = &c_stAswLed_Device0_EventConfigTable[eAswLedDevice0_DispType_Offline],
    },
};

AswLedConfig_Struct g_AswLedConfigTable[CDD_LEDM_DEVICE_COUNT] = 
{
    [CDD_LEDM_DEVICE_0_WS2812B] = 
    {
        .ledEventCount = eAswLedDevice0_DispType_Count,
        .pEventCtrl = g_stAswLedDevice0EventCtrl,
        .currentHighestLedPriority = { 0 },
        .currentHighestPriorityledDispType = { 0 },
        .pFuncCddDrv = CddLedM_UpdateState,
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t AswLedEventCfg_GetGBIdleExsit(uint8_t port)
{
    uint8_t ret = FALSE;

    if (ASWLED_CFG_IsQBStandardMode() == FALSE &&  AswCharge_GetWorkState(port) == ASWCHARGE_WORKSTATE_IDLE)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetQBIdleExsit(uint8_t port)
{
    uint8_t ret = FALSE;

    if (ASWLED_CFG_IsQBStandardMode() == TRUE &&  AswCharge_GetWorkState(port) == ASWCHARGE_WORKSTATE_IDLE)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetStartingExsit(uint8_t port)
{
    uint8_t ret = FALSE;
    uint8_t workState = AswCharge_GetWorkState(port);

    if (workState == ASWCHARGE_WORKSTATE_WAKEUP || workState == ASWCHARGE_WORKSTATE_STARTING)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetChargingExsit(uint8_t port)
{
    uint8_t ret = FALSE;
    uint8_t workState = AswCharge_GetWorkState(port);

    if (workState == ASWCHARGE_WORKSTATE_CHARGING)
    {
        ret = TRUE;
    }
    else if (workState == ASWCHARGE_WORKSTATE_STOPPING)
    {
        if (CddRelay_GetRelayState(port) == eCddRelayState_On)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetPausingExsit(uint8_t port)
{
    uint8_t ret = FALSE;
    uint8_t workState = AswCharge_GetWorkState(port);

    if (workState == ASWCHARGE_WORKSTATE_PAUSEA || workState == ASWCHARGE_WORKSTATE_PAUSEB)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetStopFinishExsit(uint8_t port)
{
    uint8_t ret = FALSE;
    uint8_t workState = AswCharge_GetWorkState(port);

    if (workState == ASWCHARGE_WORKSTATE_FINISH)
    {
        ret = TRUE;
    }
    else if (workState == ASWCHARGE_WORKSTATE_STOPPING)
    {
        if (CddRelay_GetRelayState(port) == eCddRelayState_Off)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetConnectedUnAuthExsit(uint8_t port)
{
    uint8_t ret = FALSE;
    uint8_t workState = AswCharge_GetWorkState(port);

    if (workState == ASWCHARGE_WORKSTATE_READY)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetErrorStateExsit(uint8_t port)
{
    uint8_t ret = FALSE;

    if ((TRUE == AswErrHandle_IsExsistError(port)) || (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr)))
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t AswLedEventCfg_GetOfflineExsit(uint8_t port)
{
    uint8_t ret = FALSE;

    if ((TRUE == AswErrHandle_CheckErrExit(port, eErr_NetNoSIMErr)) || (TRUE == AswErrHandle_CheckErrExit(port, eErr_PlatformOffline)))
    {
        ret = TRUE;
    }

    return ret;
}



