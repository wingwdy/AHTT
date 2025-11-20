/******************************************************************************
* File Name          : Asw_EVSE.c
* Description        : Code for EVSE State Manage
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
#include "Asw_EVSE.h"
#include "Cdd_CP.h"
#include "Cdd_Relay.h"
#include "Asw_ErrorHandle.h"
#include "Asw_EVSEConfig.h"
#include "Common.h"
#include "SysCfg.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t evseState;                      /* EVSE状态 */
    uint32_t quitState0DelayTimer;          /* 退出状态0延时计时器 */
    uint32_t enterState2DelayTimer;
    uint8_t  startCharge;                   /* 启动充电，TRUE-启动充电, FALSE-停止充电 */

    uint8_t shortCutDetectResult;
    uint8_t diodeDetectResult;
    uint32_t relaySwitchOnDetectDelayTimer; /* 继电器闭合后，延时检测 */

    uint32_t S2CloseTimer;                  /* S2断开超时计时器 */
}AswEVSECtrl_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswEVSECtrl_Struct g_stAswEVSECtrl[SYSCFG_CFG_GUN_NUM];


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswEVSE_StateEnterState0(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl)
{
    AswErrChargeCondition_Enum eChargeCondition = eErrChargeCondition_Allow;
    
    if (pEVSECtrl->evseState != ASWEVSE_STATE_0)
    {
        eChargeCondition = AswErrHandle_GetChargeCondition(port);

        if (eChargeCondition != eErrChargeCondition_Allow)
        {
            CddRelay_SetReqStopShortCutDetect(port);
            CddCP_StopPWM(port);
            CddRelay_CtrlSwichOff(port);
            pEVSECtrl->evseState = ASWEVSE_STATE_0;
        }
    }
}

static void AswEVSE_State0Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    AswErrChargeCondition_Enum eChargeCondition = AswErrHandle_GetChargeCondition(port);

    if (eChargeCondition != eErrChargeCondition_Allow)
    {
        pEVSECtrl->quitState0DelayTimer = Common_GetSystick();
    }

    if (Common_JudgeTimeoutMs(pEVSECtrl->quitState0DelayTimer, ASWEVSE_CFG_QUIT_STATE0_TIMEOUT))
    {
        pEVSECtrl->evseState = ASWEVSE_STATE_1;
    }
}

static void AswEVSE_State1Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_9V || eCpState == eCddCPVolState_6V)
    {
        pEVSECtrl->evseState = ASWEVSE_STATE_2;
        pEVSECtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;
        pEVSECtrl->enterState2DelayTimer = Common_GetSystick();

#if (ASWEVSE_CFG_DIODE_DETECT_ENABLE == TRUE)
        CddCP_SetReqDiodeExsitDetect(port);
#else
        pEVSECtrl->diodeDetectResult = GLOBAL_OPT_STATE_SUCCESS;
#endif
    }
}

static void AswEVSE_State1DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_9V || eCpState == eCddCPVolState_6V)
    {
        pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
        pEVSECtrl->evseState = ASWEVSE_STATE_2_DOT;
    }
    else
    {
        pEVSECtrl->evseState = ASWEVSE_STATE_1;
        CddCP_StopPWM(port);
    }
}

static void AswEVSE_State2Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (Common_JudgeTimeoutMs(pEVSECtrl->enterState2DelayTimer, ASWEVSE_CFG_ENTER_STATE2_DELAY))
    {
        if (pEVSECtrl->shortCutDetectResult == GLOBAL_OPT_STATE_IDLE)
        {
            CddRelay_SetReqStartShortCutDetect(port);
        }
    }

    if (eCpState == eCddCPVolState_9V || eCpState == eCddCPVolState_6V)
    {
        if (pEVSECtrl->shortCutDetectResult == GLOBAL_OPT_STATE_SUCCESS && pEVSECtrl->diodeDetectResult == GLOBAL_OPT_STATE_SUCCESS)
        {
            if (pEVSECtrl->startCharge == TRUE)
            {
                CddCP_StartPWM(port);
                pEVSECtrl->evseState = ASWEVSE_STATE_2_DOT;
                pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
            }
        }
    }
    else if (eCpState == eCddCPVolState_12V)
    {
        pEVSECtrl->evseState = ASWEVSE_STATE_1;
    }
    else
    {}
}

static void AswEVSE_State2DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (pEVSECtrl->startCharge == TRUE)
    {
        if (eCpState == eCddCPVolState_6V)
        {
            if (pEVSECtrl->relaySwitchOnDetectDelayTimer == 0)
            {
                CddRelay_CtrlSwichOn(port);
                pEVSECtrl->relaySwitchOnDetectDelayTimer = Common_GetSystick();
            }

            if (Common_JudgeTimeoutMs(pEVSECtrl->relaySwitchOnDetectDelayTimer, ASWEVSE_CFG_RELAY_DETECT_DELAY))
            {
                if (eCddRelayState_On == CddRelay_GetRelayState(port))
                {
                    pEVSECtrl->evseState = ASWEVSE_STATE_3_DOT;
                }
            }
        }
        else if (eCpState == eCddCPVolState_9V)
        {
            CddRelay_CtrlSwichOff(port);
        }
        else if (eCpState == eCddCPVolState_12V)
        {
            CddRelay_CtrlSwichOff(port);
            pEVSECtrl->evseState = ASWEVSE_STATE_1_DOT;
        }
        else
        {}
    }
    else  
    {
        CddCP_StopPWM(port);
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;
        pEVSECtrl->enterState2DelayTimer = Common_GetSystick();
        pEVSECtrl->evseState = ASWEVSE_STATE_2;
    }
}

static void AswEVSE_State3Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_12V)
    {
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->evseState = ASWEVSE_STATE_1;
    }
    else if (eCpState == eCddCPVolState_9V)
    {
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->evseState = ASWEVSE_STATE_2;
    }
    else
    {
        if (Common_JudgeTimeoutMs(pEVSECtrl->S2CloseTimer, ASWEVSE_CFG_S2_CLOSE_TIMEOUT))
        {
            CddRelay_CtrlSwichOff(port);
            pEVSECtrl->evseState = ASWEVSE_STATE_2;
        }
    }
}

static void AswEVSE_State3DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_9V)
    {
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->evseState = ASWEVSE_STATE_2_DOT;
        pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
    }
    else if (eCpState == eCddCPVolState_12V)
    {
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->evseState = ASWEVSE_STATE_1_DOT;
    }
    else
    {
        if (pEVSECtrl->startCharge == FALSE)
        {
            CddCP_StopPWM(port);
            pEVSECtrl->evseState = ASWEVSE_STATE_3;
            pEVSECtrl->S2CloseTimer = Common_GetSystick();
        }
    }
}

static void AswEVSE_StateManage(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl)
{
    CddCPVolState_Enum eCpState = CddCP_GetVolState(port);

    switch (pEVSECtrl->evseState)
    {
    case ASWEVSE_STATE_0:
    {
        AswEVSE_State0Hanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_1:
    {
        AswEVSE_State1Hanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_1_DOT:
    {
        AswEVSE_State1DotHanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_2:
    {
        AswEVSE_State2Hanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_2_DOT:
    {
        AswEVSE_State2DotHanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_3:
    {
        AswEVSE_State3Hanlde(port, pEVSECtrl, eCpState);
        break;
    }
    case ASWEVSE_STATE_3_DOT:
    {
        AswEVSE_State3DotHanlde(port, pEVSECtrl, eCpState);
        break;
    }
    default:
    {
        break;
    }
    }
}

void AswEVSE_InitMemory(void)
{
    memset(g_stAswEVSECtrl, 0x00, sizeof(g_stAswEVSECtrl));
}

void AswEVSE_MainFunction(void)
{
    AswEVSECtrl_Struct *pEVSECtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pEVSECtrl = &g_stAswEVSECtrl[port];
        AswEVSE_StateManage(port, pEVSECtrl);
    }
}

void AswEVSE_StartCharge(uint8_t port)
{
    AswEVSECtrl_Struct *pEVSECtrl = &g_stAswEVSECtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pEVSECtrl->startCharge = TRUE;
    }
}

void AswEVSE_StopCharge(uint8_t port)
{
    AswEVSECtrl_Struct *pEVSECtrl = &g_stAswEVSECtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pEVSECtrl->startCharge = FALSE;
    }
}

uint8_t AswEVSE_GetEVSEState(uint8_t port)
{
    AswEVSECtrl_Struct *pEVSECtrl = &g_stAswEVSECtrl[port];
    uint8_t state = ASWEVSE_STATE_0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        state = pEVSECtrl->evseState;
    }

    return state;
}
