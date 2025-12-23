/******************************************************************************
* File Name          : Asw_LedEventConfig.h
* Description        : Code for Led event manage
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef ASW_LED_EVENT_CONFIG_H_
#define ASW_LED_EVENT_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Cdd_LedM.h"
#include "Common.h"
#include "SysCfg.h"
#include "DS_LogM.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWLED_CFG_DISP_MODE_NON_ROTATING               0   /* 非轮显 */
#define ASWLED_CFG_DISP_MODE_ROTATING                   1   /* 轮显 */
#define ASWLED_CFG_DISP_MODE_CHOOSE                     ASWLED_CFG_DISP_MODE_NON_ROTATING

#define ASWLED_CFG_LogPrint(fmt, ...)                   DSLOGM_Debug(DSLogMModule_EVSE, fmt, ##__VA_ARGS__)

#define ASWLED_CFG_IsQBStandardMode()                   (FALSE)                 
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eAswLedEventType_State,  /* 状态型 */
    eAswLedEventype_Event,   /* 事件型 */
    eAswLedEventType_Count,
}AswLedEventType_Enum;

typedef enum
{
    eAswLedDevice0_DispType_None,               /* 初始状态, 具体显示由底层驱动决定 */
    eAswLedDevice0_DispType_GBIdle,             /* 国标待机模式 */
    eAswLedDevice0_DispType_QBIdle,             /* 企标待机模式 */
    eAswLedDevice0_DispType_Starting,           /* 启动中 */
    eAswLedDevice0_DispType_Charging,           /* 正在充电 */
    eAswLedDevice0_DispType_Pausing,            /* 暂停充电 */
    eAswLedDevice0_DispType_StopFinish,         /* 充电完成 */
    eAswLedDevice0_DispType_ConnectdUnAuth,     /* 已连接待授权 */
    eAswLedDevice0_DispType_ReadCardSucc,       /* 读卡授权 */
    eAswLedDevice0_DispType_InvalidCard,        /* 无效卡 */
    eAswLedDevice0_DispType_Error,              /* 设备故障 */
    eAswLedDevice0_DispType_Offline,            /* 设备离线 */
    eAswLedDevice0_DispType_Count,
}AswLedDevice0DispType_Enum;


/* 灯语优先级，数字越大优先级越高 */
typedef enum
{
    eAswLedPriority_0,
    eAswLedPriority_1,
    eAswLedPriority_2,
    eAswLedPriority_3,
    eAswLedPriority_4,
    eAswLedPriority_5,
    eAswLedPriority_6,
}AswLedPriority_Enum;


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{ 
    uint8_t (*pFuncGetStateExsist)(uint8_t port);
    AswLedEventType_Enum eLedEventType;
    AswLedPriority_Enum eLedPriority;
    uint8_t ledDispType;
    char *desc;
}AswLedEventConfig_Struct;

typedef struct 
{
    uint8_t currentState[SYSCFG_CFG_GUN_NUM];
    uint8_t lastState[SYSCFG_CFG_GUN_NUM];
    const AswLedEventConfig_Struct *pLedEventConfig;
}AswLedEventCtrl_Struct;

typedef struct 
{   
    uint8_t ledEventCount;
    uint8_t currentHighestLedPriority[SYSCFG_CFG_GUN_NUM];
    uint8_t currentHighestPriorityledDispType[SYSCFG_CFG_GUN_NUM];
    char *currentHighestLedDesc[SYSCFG_CFG_GUN_NUM];
    uint8_t lastHighestPriorityledDispType[SYSCFG_CFG_GUN_NUM];
    char *lastHighestLedDesc[SYSCFG_CFG_GUN_NUM];
    AswLedEventCtrl_Struct *pEventCtrl;
    void (*pFuncCddDrv)(uint8_t dev, uint8_t port, uint8_t ledDispType);
}AswLedConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern AswLedConfig_Struct g_AswLedConfigTable[CDD_LEDM_DEVICE_COUNT];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
#endif























