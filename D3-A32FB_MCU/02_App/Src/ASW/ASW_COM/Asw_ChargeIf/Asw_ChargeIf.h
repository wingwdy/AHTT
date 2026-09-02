/******************************************************************************
* File Name          : template.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef ASW_CHARGE_IF_H_
#define ASW_CHARGE_IF_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_Charge.h"
#include "Cdd_Relay.h"
#include "Asw_VoltCurHandle.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWCHARGEIF_WORKSTATE_IDLE           ASWCHARGE_WORKSTATE_IDLE      /* 空闲状态 */
#define ASWCHARGEIF_WORKSTATE_READY          ASWCHARGE_WORKSTATE_READY     /* 已准备状态 */
#define ASWCHARGEIF_WORKSTATE_STARTING       ASWCHARGE_WORKSTATE_STARTING  /* 启动中状态 */
#define ASWCHARGEIF_WORKSTATE_WAKEUP         ASWCHARGE_WORKSTATE_WAKEUP    /* 尝试唤醒状态 */
#define ASWCHARGEIF_WORKSTATE_CHARGING       ASWCHARGE_WORKSTATE_CHARGING  /* 充电中状态 */
#define ASWCHARGEIF_WORKSTATE_PAUSEA         ASWCHARGE_WORKSTATE_PAUSEA    /* 车端暂停状态 */
#define ASWCHARGEIF_WORKSTATE_PAUSEB         ASWCHARGE_WORKSTATE_PAUSEB    /* 桩端暂停状态 */
#define ASWCHARGEIF_WORKSTATE_STOPPING       ASWCHARGE_WORKSTATE_STOPPING  /* 停止中状态 */
#define ASWCHARGEIF_WORKSTATE_FINISH         ASWCHARGE_WORKSTATE_FINISH    /* 停止完成状态 */

#define ASWCHARGEIF_RELAYSTATE_OFF           eCddRelayState_Off
#define ASWCHARGEIF_RELAYSTATE_ON            eCddRelayState_On

#define ASWCHARGEIF_ADJUST_POWER_ABSOLUTE    eAswVoltCurAdjustMode_PowerAbsolute
#define ASWCHARGEIF_ADJUST_POWER_RELATIVE    eAswVoltCurAdjustMode_PowerPercent

#define ASWCHARGEIF_PROFILE_GN               eAswChargeCtrlProfile_GN
#define ASWCHARGEIF_PROFILE_XDT              eAswChargeCtrlProfile_XDT

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t AswChargeIf_CheckGunConnected(uint8_t port);
uint32_t AswChargeIf_GetOutputVoltage(uint8_t port);
uint32_t AswChargeIf_GetOutputCurrent(uint8_t port);
uint32_t AswChargeIf_GetOutputPower(uint8_t port);
uint8_t AswChargeIf_GetChargeState(uint8_t port);
uint8_t AswChargeIf_GetRelayState(uint8_t port);
uint8_t AswChargeIf_GetGunTemperature(uint8_t port);
uint8_t AswChargeIf_GetEnvTemperature(void);
uint32_t AswChargeIf_GetInputVoltage(uint8_t port);
uint16_t AswChargeIf_GetCpVoltage(uint8_t port);
uint16_t AswChargeIf_GetCpDuty(uint8_t port);
uint64_t AswChargeIf_GetMeterEnergyVal(uint8_t port);
void AswChargeIf_ChargeStart(uint8_t port);
AswErrorType_Enum AswChargeIf_GetStopReason(uint8_t port);
uint8_t AswChargeIf_GetAuthFlag(uint8_t port);
void AswChargeIf_AdjustOutputCurrent(uint8_t port, uint8_t adjustMode, uint32_t val);
uint8_t AswChargeIf_CheckS2Closed(uint8_t port);
void AswChargeIf_SetProfile(uint8_t profile);
#endif /* ASW_CHARGEIF_H_ */    





















