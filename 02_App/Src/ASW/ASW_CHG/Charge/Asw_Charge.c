/******************************************************************************
* File Name          : Asw_Charge.c
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
*    Global variables Declaration
*******************************************************************************/
static AswChargeCtrl_Struct g_stAswChargeCtrl[SYSCFG_CFG_GUN_NUM];
static AswChargeCtrlProfile_Enum g_eAswChargeCtrlProfile = eAswChargeCtrlProfile_GN;

const struct 
{
    char *cName;
}c_ChargeStateName[] = 
{
    {"空闲状态"},
    {"已准备状态"},
    {"启动中状态"},
    {"尝试唤醒状态"},
    {"充电中状态"},
    {"车端暂停状态"},
    {"桩端暂停状态"},
    {"停止中状态"},
    {"停止完成状态"},
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void AswCharge_IdleStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_ReadyStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_StartingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_WakeupStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_ChargingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_PauseAStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_PauseBStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_StoppingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_FinishStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);
static void AswCharge_WorkStateManage(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static void AswCharge_IdleStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    uint8_t evseState = AswEVSE_GetEVSEState(port);

    if (evseState == ASWEVSE_STATE_2)
    {
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_READY);
    }
    else
    {
        /*担心因为时序问题，在接收启动充电时，判断条件都OK，但是在执行时条件又不满足，出现逻辑死角 */
        if (pChargeCtrl->authFlag == TRUE)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_READY);
        }
    }
}

static void AswCharge_ReadyStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    uint8_t evseState = AswEVSE_GetEVSEState(port);
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    CddCPVolState_Enum eCPState = CddCP_GetVolState(port);

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (eChargeCondition == eErrChargeCondition_Cancel)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        }
        else if (eChargeCondition == eErrChargeCondition_Suspend)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEB);
        }
        else
        {
            if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_GunDisConn);
            }
            else if (evseState == ASWEVSE_STATE_2)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STARTING);
                AswEVSE_StartCharge(port);

                pChargeCtrl->pwmStartTimer = 0;
                pChargeCtrl->tryWakeupFlag = FALSE;
                memset(&pChargeCtrl->stFilterChargeStable, 0, sizeof(FilterProfile1_Struct));
                pChargeCtrl->chargeStableFlag = FALSE;

            }
            else
            {}
        }
    }
    else
    {
        if (eCPState == eCddCPVolState_12V)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_IDLE);
        }
    }
}

static void AswCharge_StartingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    uint8_t evseState = AswEVSE_GetEVSEState(port);

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (eChargeCondition == eErrChargeCondition_Cancel)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        }
        else if (eChargeCondition == eErrChargeCondition_Suspend || ASWCHARGE_CFG_GetCurRateCurrent(port) == 0)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEB);
            AswEVSE_StopCharge(port);
        }
        else
        {
            if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_GunDisConn);
            }
            else
            {
                if (evseState != ASWEVSE_STATE_3_DOT)
                {
                    if (evseState == ASWEVSE_STATE_2_DOT)
                    {
                        if (pChargeCtrl->pwmStartTimer == 0)
                        {
                            pChargeCtrl->pwmStartTimer = Common_GetSystick();
                        }
                    }

                    if (c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncStartingHandle)
                    {
                        c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncStartingHandle(port, pChargeCtrl);
                    }
                }
                else
                {
                    AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_CHARGING);
                }
            }
        }
    }
    else
    {
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
    }
}

static void AswCharge_WakeupStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    uint8_t evseState = AswEVSE_GetEVSEState(port);
    uint8_t wakeupStatus = CddCP_GetWakeupStatus(port);

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (eChargeCondition == eErrChargeCondition_Cancel)
        {
            CddCP_SetReqStopWakeUp(port);
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        }
        else if (eChargeCondition == eErrChargeCondition_Suspend || ASWCHARGE_CFG_GetCurRateCurrent(port) == 0)
        {
            CddCP_SetReqStopWakeUp(port);
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEB);
            AswEVSE_StopCharge(port);
        }
        else
        {
            if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
            {
                CddCP_SetReqStopWakeUp(port);
                AswErrhandle_SetErrExsitCallback(port, eErr_GunDisConn);
            }
            else
            {
                if (evseState != ASWEVSE_STATE_3_DOT)
                {
                    if (wakeupStatus != GLOBAL_OPT_STATE_PROCESS)
                    {
                        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STARTING);
                        pChargeCtrl->pwmStartTimer = 0;
                    }
                }
                else
                {
                    AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_CHARGING);
                }
            }
        }    
    }
    else
    {
        CddCP_SetReqStopWakeUp(port);
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
    }
}

static void AswCharge_ChargingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    uint8_t evseState = AswEVSE_GetEVSEState(port);
    uint32_t outputCurrent = 0;

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (eChargeCondition == eErrChargeCondition_Cancel)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        }
        else if (eChargeCondition == eErrChargeCondition_Suspend || ASWCHARGE_CFG_GetCurRateCurrent(port) == 0)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEB);
            AswEVSE_StopCharge(port);
        }
        else 
        {
            if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_CPBreakOff);
            }
            else if (evseState == ASWEVSE_STATE_2_DOT)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEA);
                pChargeCtrl->vehiclePauseTimer = Common_GetSystick();
            }
            else if (evseState == ASWEVSE_STATE_3_DOT)
            {
                if (c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncChargingHandle)
                {
                    c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncChargingHandle(port, pChargeCtrl);
                }
            }
            else
            {}
        }
    }
    else
    {
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
    }
}

static void AswCharge_PauseAStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    uint8_t evseState = AswEVSE_GetEVSEState(port);

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (eChargeCondition == eErrChargeCondition_Cancel)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        }
        else if (eChargeCondition == eErrChargeCondition_Suspend || ASWCHARGE_CFG_GetCurRateCurrent(port) == 0)
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_PAUSEB);
            AswEVSE_StopCharge(port);
        }
        else 
        {
            if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_GunDisConn);
            }
            else if (evseState == ASWEVSE_STATE_2_DOT)
            {
                if (c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncChargingPauseAHandle)
                {
                    c_AswChargeProfileConfigTable[g_eAswChargeCtrlProfile].pFuncChargingPauseAHandle(port, pChargeCtrl);
                }
            }
            else if (evseState == ASWEVSE_STATE_3_DOT)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_CHARGING);
            }
            else
            {}
        }
    }
    else
    {
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
    }
}

static void AswCharge_PauseBStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);
    uint8_t evseState = AswEVSE_GetEVSEState(port);

    if (pChargeCtrl->authFlag == TRUE)
    {
        if (evseState == ASWEVSE_STATE_1 || evseState == ASWEVSE_STATE_1_DOT)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_GunDisConn);
        }
        else 
        {
            if (eChargeCondition == eErrChargeCondition_Cancel)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
            }
            else if (eChargeCondition == eErrChargeCondition_Allow)
            {
                if (ASWCHARGE_CFG_GetCurRateCurrent(port) > 0)
                {
                    AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STARTING);
                    AswEVSE_StartCharge(port);
                    pChargeCtrl->pwmStartTimer = 0;
                    pChargeCtrl->tryWakeupFlag = FALSE;
                }
            }
            else
            {}
        }
    }
    else
    {
        AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_STOPPING);
        
    }
}

static void AswCharge_StoppingStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    CddRelayState_Enum eRelayState = CddRelay_GetRelayState(port);

    pChargeCtrl->authFlag = FALSE;

    if (pChargeCtrl->stopTimer == 0)
    {
        pChargeCtrl->stopTimer = Common_GetSystick();
    }
    else
    {
        if (eRelayState != eCddRelayState_Off)
        {
            if (Common_JudgeTimeoutMs(pChargeCtrl->stopTimer, ASWCHARGE_CFG_STOP_TIMEOUT))
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_FINISH);
                pChargeCtrl->stopTimer = 0;
            }
        }
        else
        {
            AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_FINISH);
            pChargeCtrl->stopTimer = 0;
        }
    }
}

static void AswCharge_FinishStateHandle(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    CddCPVolState_Enum eCPState = CddCP_GetVolState(port);

    if (pChargeCtrl->quitStopFinishTimer == 0)
    {
        pChargeCtrl->quitStopFinishTimer = Common_GetSystick();
    }
    else
    {
        if (Common_JudgeTimeoutMs(pChargeCtrl->quitStopFinishTimer, ASWCHARGE_CFG_QUIT_FINISH_TIMEOUT))
        {
            if (pChargeCtrl->authFlag == TRUE)
            {
                AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_READY);
                pChargeCtrl->quitStopFinishTimer = 0;
                AswErrHandle_ClearNormalStopReason(port);
            }
            else
            {
                if (eCPState == eCddCPVolState_12V)
                {
                    AswCharge_SetWorkState(port, ASWCHARGE_WORKSTATE_IDLE);
                    pChargeCtrl->quitStopFinishTimer = 0;
                    AswErrHandle_ClearNormalStopReason(port);
                }
            }
        }
    }
}

static void AswCharge_WorkStateManage(uint8_t port, AswChargeCtrl_Struct *pChargeCtrl)
{
    switch (pChargeCtrl->workState)
    {
        case ASWCHARGE_WORKSTATE_IDLE:
        {
            AswCharge_IdleStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_READY:
        {
            AswCharge_ReadyStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_STARTING:
        {
            AswCharge_StartingStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_WAKEUP:
        {
            AswCharge_WakeupStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_CHARGING:
        {
            AswCharge_ChargingStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_PAUSEA:
        {
            AswCharge_PauseAStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_PAUSEB:
        {
            AswCharge_PauseBStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_STOPPING:
        {
            AswCharge_StoppingStateHandle(port, pChargeCtrl);
            break;
        }
        case ASWCHARGE_WORKSTATE_FINISH:
        {
            AswCharge_FinishStateHandle(port, pChargeCtrl);
            break;
        }
        default:
        {
            pChargeCtrl->workState = ASWCHARGE_WORKSTATE_IDLE;
            break;
        }
    }
}

void AswCharge_SetWorkState(uint8_t port, uint8_t workState)
{
    AswChargeCtrl_Struct *pChargeCtrl = &g_stAswChargeCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pChargeCtrl->workState != workState)
        {
            ASWCHARGE_CFG_LogPrint("[枪：%d]充电状态变化: %s ---> %s\r\n", port, 
                c_ChargeStateName[pChargeCtrl->workState], c_ChargeStateName[workState]);
                pChargeCtrl->workState = workState;

            if (workState == ASWCHARGE_WORKSTATE_WAKEUP)
            {
                CddCP_SetReqStartWakeup(port);
                pChargeCtrl->tryWakeupFlag = TRUE;
            }
            else if (workState == ASWCHARGE_WORKSTATE_CHARGING)
            {
                memset(&pChargeCtrl->stFilterlittleCur, 0x00, sizeof(FilterProfile1_Struct));
                memset(&pChargeCtrl->stFilterChargeStable, 0x00, sizeof(FilterProfile1_Struct));
            }
            else if (workState == ASWCHARGE_WORKSTATE_FINISH || workState == ASWCHARGE_WORKSTATE_STOPPING)
            {
                if (workState == ASWCHARGE_WORKSTATE_STOPPING)
                {
                    AswEVSE_StopCharge(port);
                }

                if (pChargeCtrl->eStopReason == eErr_none)
                {
                    AswCharge_SetStopReason(port, AswErrHandle_GetExsistError(port));
                }

                pChargeCtrl->chargeStableFlag = FALSE;
            }
        }
    }
}

uint8_t AswCharge_GetWorkState(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = &g_stAswChargeCtrl[port];
    uint8_t workState = ASWCHARGE_WORKSTATE_IDLE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        workState = pChargeCtrl->workState;
    }

    return workState;
}

uint8_t AswCharge_IsAuth(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = &g_stAswChargeCtrl[port];
    uint8_t ret  = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        ret = pChargeCtrl->authFlag;
    }

    return ret;
}

void AswCharge_InitMemory(void)
{
    memset(g_stAswChargeCtrl, 0x00, sizeof(g_stAswChargeCtrl));
}



void AswCharge_MainFunction(void)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pChargeCtrl = &g_stAswChargeCtrl[port];
        AswCharge_WorkStateManage(port, pChargeCtrl);
    }
}

void AswCharge_StartAuth(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pChargeCtrl = &g_stAswChargeCtrl[port];

        if (pChargeCtrl->authFlag != TRUE)
        {
            pChargeCtrl->eStopReason = eErr_none;
            pChargeCtrl->authFlag = TRUE;
            ASWCHARGE_CFG_LogPrint("[枪：%d]充电授权!\r\n", port);
        }
    }
}

void AswCharge_StopAuth(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pChargeCtrl->authFlag == TRUE)
        {
            pChargeCtrl = &g_stAswChargeCtrl[port];
            pChargeCtrl->authFlag = FALSE;
            ASWCHARGE_CFG_LogPrint("[枪：%d]取消充电授权\r\n", port);
        }
    }
}

uint8_t AswCharge_GetAuthFlag(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pChargeCtrl = &g_stAswChargeCtrl[port];
        ret = pChargeCtrl->authFlag;
    }

    return ret;
}

void AswCharge_SetStopReason(uint8_t port, AswErrorType_Enum eReason)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;
    char *pErrDesc = AswErrHandle_GetErrdesc(eReason); 

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pChargeCtrl = &g_stAswChargeCtrl[port];

        if (pChargeCtrl->authFlag == TRUE && pChargeCtrl->eStopReason == eErr_none)
        {
            pChargeCtrl->eStopReason = eReason;
            ASWCHARGE_CFG_LogPrint("[枪：%d]取消充电授权, 停止原因：%s!\r\n", port, pErrDesc);
        }
    }
}

AswErrorType_Enum AswCharge_GetStopReason(uint8_t port)
{
    AswChargeCtrl_Struct *pChargeCtrl = NULL;
    AswErrorType_Enum ret = eErr_none;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pChargeCtrl = &g_stAswChargeCtrl[port];
        ret = pChargeCtrl->eStopReason;
    }

    return ret;
}


void AswCharge_SetProfile(AswChargeCtrlProfile_Enum eProfile)
{
    if (eProfile < eAswChargeCtrlProfile_Count)
    {
        g_eAswChargeCtrlProfile = eProfile;
    }
}