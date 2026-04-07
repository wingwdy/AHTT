/******************************************************************************
* File Name          : Asw_ChargeConfig.c
* Description        : Code for Charge State Manage
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
#include "Asw_Charge.h"
#include "Common.h"
#include "SysCfg.h"
#include "Asw_ChargeConfig.h"
#include "Cdd_CP.h"
#include "Asw_EVSE.h"

#include "Filter.h"
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
static void AswChargeCfg_GN_ChargingHandle(uint8_t port, void *pCtrlCtx);
static void AswChargeCfg_GN_StartingHandle(uint8_t port, void *pCtrlCtx);
static void AswChargeCfg_GN_ChargePauseAHandle(uint8_t port, void *pCtrlCtx);

static void AswChargeCfg_XDT_ChargingHandle(uint8_t port, void *pCtrlCtx);
static void AswChargeCfg_XDT_StartingHandle(uint8_t port, void *pCtrlCtx);
static void AswChargeCfg_XDT_ChargePauseAHandle(uint8_t port, void *pCtrlCtx);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const AswChargeProfileHandle_Struct c_AswChargeProfileConfigTable[eAswChargeCtrlProfile_Count] =
{
    [eAswChargeCtrlProfile_GN] = 
    {
        .pFuncStartingHandle = AswChargeCfg_GN_StartingHandle,
        .pFuncChargingHandle = AswChargeCfg_GN_ChargingHandle,
        .pFuncChargingPauseAHandle = AswChargeCfg_GN_ChargePauseAHandle,
    },

    [eAswChargeCtrlProfile_XDT] = 
    {
        .pFuncStartingHandle = AswChargeCfg_XDT_StartingHandle,
        .pFuncChargingHandle = AswChargeCfg_XDT_ChargingHandle,
        .pFuncChargingPauseAHandle = AswChargeCfg_XDT_ChargePauseAHandle,
    },
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswChargeCfg_GN_StartingHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;

    if (pChargeCtrl->pwmStartTimer != 0)
    {
        if (Common_JudgeTimeoutMs(pChargeCtrl->pwmStartTimer, 15 * 1000))
        {
            if (pChargeCtrl->tryWakeupFlag == FALSE)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_WAKEUP);
            }
            else
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_ChgStartTimeout);
            }
        }
    }
}

static void AswChargeCfg_XDT_StartingHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;

    if (pChargeCtrl->pwmStartTimer != 0)
    {
        if (Common_JudgeTimeoutMs(pChargeCtrl->pwmStartTimer, 15 * 1000))
        {
            if (pChargeCtrl->tryWakeupFlag == FALSE)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_WAKEUP);
            }
        }
    }
}

static void AswChargeCfg_GN_ChargingHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;
    uint32_t outputCurrent = ASWCHARGE_CFG_GetOutputCurrent(port);

    /* 电流小于1A，判断为小电流 */
    if (outputCurrent < 1000)
    {
        pChargeCtrl->stFilterlittleCur.status = TRUE;
    }
    else
    {
        pChargeCtrl->stFilterlittleCur.status = FALSE;
    }

    Filter_Profile1(&pChargeCtrl->stFilterlittleCur, ((30 * 60 * 1000) / ASWCHARGE_CFG_CALL_CYCLE));

    if (pChargeCtrl->stFilterlittleCur.validStatus == TRUE)
    {
        AswErrhandle_SetErrExsitCallback(port, eSrc_LittleCurr);
    }
}

static void AswChargeCfg_XDT_ChargingHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;
    uint32_t outputCurrent = ASWCHARGE_CFG_GetOutputCurrent(port);

    /* 电流大于等于5A，判断为正常充电电流 */
    if (pChargeCtrl->stFilterChargeStable.validStatus == FALSE)
    {
        if (outputCurrent >= 5000)
        {
            pChargeCtrl->stFilterChargeStable.status = TRUE;
        }
        else
        {
            pChargeCtrl->stFilterChargeStable.status = FALSE;
        }

        Filter_Profile1(&pChargeCtrl->stFilterChargeStable, ((10 * 60 * 1000) / ASWCHARGE_CFG_CALL_CYCLE));

        if (pChargeCtrl->stFilterChargeStable.validStatus == TRUE)
        {
            pChargeCtrl->chargeStableFlag = TRUE;
        }
    }
    else
    {
        /* 电流小于1A，判断为小电流 */
        if (outputCurrent < 500)
        {
            pChargeCtrl->stFilterlittleCur.status = TRUE;
        }
        else
        {
            pChargeCtrl->stFilterlittleCur.status = FALSE;
        }

        Filter_Profile1(&pChargeCtrl->stFilterlittleCur, ((30 * 60 * 1000) / ASWCHARGE_CFG_CALL_CYCLE));

        if (pChargeCtrl->stFilterlittleCur.validStatus == TRUE)
        {
            AswErrhandle_SetErrExsitCallback(port, eSrc_LittleCurr);
        }
    }
}

static void AswChargeCfg_GN_ChargePauseAHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;

    if (Common_JudgeTimeoutMs(pChargeCtrl->vehiclePauseTimer, 30 * 1000))
    {
        AswErrhandle_SetErrExsitCallback(port, eSrc_S2BreakOff);
    }
}

static void AswChargeCfg_XDT_ChargePauseAHandle(uint8_t port, void *pCtrlCtx)
{
    AswChargeCtrl_Struct *pChargeCtrl = (AswChargeCtrl_Struct *)pCtrlCtx;

    if (pChargeCtrl->chargeStableFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pChargeCtrl->vehiclePauseTimer, 30 * 60 * 1000))
        {
            AswErrhandle_SetErrExsitCallback(port, eSrc_S2BreakOff);
        }
    }
}