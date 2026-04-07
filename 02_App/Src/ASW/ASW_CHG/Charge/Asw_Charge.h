/******************************************************************************
* File Name          : Asw_Charge.h
* Description        : Code for Charge State Manage
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
#ifndef ASW_CHARGE_H_
#define ASW_CHARGE_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_ErrorHandle.h"
#include "Filter.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWCHARGE_WORKSTATE_IDLE                     (0u)     /* 空闲状态 */
#define ASWCHARGE_WORKSTATE_READY                    (1u)     /* 已准备状态 */
#define ASWCHARGE_WORKSTATE_STARTING                 (2u)     /* 启动中状态 */
#define ASWCHARGE_WORKSTATE_WAKEUP                   (3u)     /* 尝试唤醒状态 */
#define ASWCHARGE_WORKSTATE_CHARGING                 (4u)     /* 充电中状态 */
#define ASWCHARGE_WORKSTATE_PAUSEA                   (5u)     /* 车端暂停状态 */
#define ASWCHARGE_WORKSTATE_PAUSEB                   (6u)     /* 桩端暂停状态 */
#define ASWCHARGE_WORKSTATE_STOPPING                 (7u)     /* 停止中状态 */
#define ASWCHARGE_WORKSTATE_FINISH                   (8u)     /* 停止完成状态 */

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    /* 启动中超时15秒，车端暂停超时30秒，小电流小于1A，持续30分钟 */
    eAswChargeCtrlProfile_GN,
    /* 不作启动超时检测，不作车端暂停超时检测，
       case1:当充电电流大于等于5A，且持续10min，S2闭合的情况下，那么当充电电流小于0.5A，持续30min，则停止充电，结算。
       case2:当充电电流大于等于5A，且持续10min，S2断开的情况下，那么当充电电流小于0.5A，持续30min，则停止充电，结算。*/
    eAswChargeCtrlProfile_XDT,
    eAswChargeCtrlProfile_Count,
}AswChargeCtrlProfile_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t workState;            /* 工作状态 */
    uint8_t tryWakeupFlag;        /* 尝试唤醒标记，TRUE-表示已唤醒过 */
    uint8_t authFlag;             /* 授权标记，TRUE-表示已授权 */
    AswErrorType_Enum eStopReason; /* 取消授权原因 */
    uint32_t pwmStartTimer;       /* PWM发波超时计时器 */
    FilterProfile1_Struct stFilterlittleCur; /* 小电流状态滤波 */
    uint32_t vehiclePauseTimer;   /* 车端暂停超时计时器 */
    uint32_t stopTimer;           /* 停止充电超时计时器 */
    uint32_t quitStopFinishTimer; /* 退出停止完成状态延时计时器 */

    FilterProfile1_Struct stFilterChargeStable; /* 充电稳定状态滤波 */
    uint8_t chargeStableFlag;     /* 充电稳定标记，TRUE-表示已稳定,  充电电流大于等于5A，且持续10min*/
}AswChargeCtrl_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void AswCharge_SetProfile(AswChargeCtrlProfile_Enum eProfile);
uint8_t AswCharge_GetWorkState(uint8_t port);
void AswCharge_InitMemory(void);
void AswCharge_MainFunction(void);
uint8_t AswCharge_IsAuth(uint8_t port);
void AswCharge_StartAuth(uint8_t port);
void AswCharge_StopAuth(uint8_t port);
uint8_t AswCharge_GetAuthFlag(uint8_t port);
AswErrorType_Enum AswCharge_GetStopReason(uint8_t port);
void AswCharge_SetStopReason(uint8_t port, AswErrorType_Enum eReason);

/* 模块内部使用 */
void AswCharge_SetWorkState(uint8_t port, uint8_t workState);
#endif /* ASW_CHARGE_H_ */




















