/******************************************************************************
* File Name          : Asw_EVSEConfig.h
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
#ifndef ASW_CHARGE_CONFIG_H_
#define ASW_CHARGE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "Cdd_MeterM.h"
#include "Asw_VoltCurHandle.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWCHARGE_CFG_CALL_CYCLE                       (10)

/* 当继电器无法断开时，延时10秒退出停止充电状态 */
#define ASWCHARGE_CFG_STOP_TIMEOUT                     10000

/* 延时退出充电完成状态 */
#define ASWCHARGE_CFG_QUIT_FINISH_TIMEOUT              500

#define ASWCHARGE_CFG_GetCurRateCurrent(port)          AswVoltCurHandle_GetMaxOutputCurrent(port)

#define ASWCHARGE_CFG_LogPrint(fmt, ...)               DSLOGM_Debug(DSLogMModule_Charge, fmt, ##__VA_ARGS__)

#define ASWCHARGE_CFG_GetOutputCurrent(port)           CddMeterM_GetRmsCurrent(port)


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    void (*pFuncStartingHandle)(uint8_t port, void *pCtrlCtx);
    void (*pFuncChargingHandle)(uint8_t port, void *pCtrlCtx);
    void (*pFuncChargingPauseAHandle)(uint8_t port, void *pCtrlCtx);
}AswChargeProfileHandle_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const AswChargeProfileHandle_Struct c_AswChargeProfileConfigTable[eAswChargeCtrlProfile_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_EVSE_CONFIG_H_ */























