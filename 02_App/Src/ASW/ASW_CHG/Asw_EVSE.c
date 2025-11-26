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

const struct 
{
    char *cName;
}c_EVSEStateName[] = 
{
    {"状态0"},
    {"状态1"},
    {"状态1'"},
    {"状态2"},
    {"状态2'"},
    {"状态3"},
    {"状态3'"},
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void AswEVSE_SetEVSEState(uint8_t port, uint8_t state);
static void AswEVSE_StateEnterState0(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl);
static void AswEVSE_State0Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State1Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State1DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State2Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State2DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State3Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_State3DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState);
static void AswEVSE_StateManage(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswEVSE_SetEVSEState(uint8_t port, uint8_t state)
{
    AswEVSECtrl_Struct *pEVSECtrl = &g_stAswEVSECtrl[port];

    if (pEVSECtrl->evseState != state)
    {
        ASWEVSE_CFG_LogPrint("[枪：%d]EVSE状态变化: %s ---> %s\r\n", port, 
            c_EVSEStateName[pEVSECtrl->evseState], c_EVSEStateName[state]);
        pEVSECtrl->evseState = state;
    }
}

static void AswEVSE_StateEnterState0(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl)
{
    if (pEVSECtrl->evseState != ASWEVSE_STATE_0)
    {
        if (TRUE == AswErrHandle_IsExsistError(port))
        {
            CddRelay_SetReqStopShortCutDetect(port);
            CddCP_SetReqStopDiodeExsitDetect(port);
            pEVSECtrl->quitState0DelayTimer = Common_GetSystick();
            CddCP_StopPWM(port);
            CddRelay_CtrlSwichOff(port);
            AswEVSE_SetEVSEState(port, ASWEVSE_STATE_0);
        }
    }
}

static void AswEVSE_State0Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (TRUE == AswErrHandle_IsExsistError(port))
    {
        pEVSECtrl->quitState0DelayTimer = Common_GetSystick();
    }

    if (Common_JudgeTimeoutMs(pEVSECtrl->quitState0DelayTimer, ASWEVSE_CFG_QUIT_STATE0_TIMEOUT))
    {
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1);
    }
}

static void AswEVSE_State1Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_9V || eCpState == eCddCPVolState_6V)
    {
        pEVSECtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;
        pEVSECtrl->enterState2DelayTimer = Common_GetSystick();
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2);

#if (ASWEVSE_CFG_DIODE_DETECT_ENABLE == TRUE)
        CddCP_SetReqStartDiodeExsitDetect(port);
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
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2_DOT);
    }
    else
    {
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1);
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

        pEVSECtrl->shortCutDetectResult = CddRelay_GetShortCutResult(port);
    }

    if (eCpState == eCddCPVolState_9V || eCpState == eCddCPVolState_6V)
    {
#if (ASWEVSE_CFG_DIODE_DETECT_ENABLE == TRUE)
        pEVSECtrl->diodeDetectResult = CddCP_GetDiodeExsitDetectResult(port);
#endif
        if (pEVSECtrl->shortCutDetectResult == GLOBAL_OPT_STATE_SUCCESS && pEVSECtrl->diodeDetectResult == GLOBAL_OPT_STATE_SUCCESS)
        {
            if (pEVSECtrl->startCharge == TRUE)
            {
                CddCP_StartPWM(port);
                AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2_DOT);
                pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
            }
        }
    }
    else if (eCpState == eCddCPVolState_12V)
    {
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1);
        CddCP_SetReqStopDiodeExsitDetect(port);
        CddRelay_SetReqStopShortCutDetect(port);
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
                    AswEVSE_SetEVSEState(port, ASWEVSE_STATE_3_DOT);
                }
            }
        }
        else if (eCpState == eCddCPVolState_9V)
        {
            CddRelay_CtrlSwichOff(port);
            pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
        }
        else if (eCpState == eCddCPVolState_12V)
        {
            CddRelay_CtrlSwichOff(port);
            AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1_DOT);
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
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2);
    }
}

static void AswEVSE_State3Hanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_12V)
    {
        CddRelay_CtrlSwichOff(port);
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1);
    }
    else if (eCpState == eCddCPVolState_9V)
    {
        CddRelay_CtrlSwichOff(port);
        pEVSECtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;
        pEVSECtrl->enterState2DelayTimer = Common_GetSystick();
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2);
    }
    else
    {
        if (Common_JudgeTimeoutMs(pEVSECtrl->S2CloseTimer, ASWEVSE_CFG_S2_CLOSE_TIMEOUT))
        {
            CddRelay_CtrlSwichOff(port);
            pEVSECtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;
            pEVSECtrl->enterState2DelayTimer = Common_GetSystick();
            AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2);
        }
    }
}

static void AswEVSE_State3DotHanlde(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl, CddCPVolState_Enum eCpState)
{
    if (eCpState == eCddCPVolState_9V)
    {
        CddRelay_CtrlSwichOff(port);
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_2_DOT);
        pEVSECtrl->relaySwitchOnDetectDelayTimer = 0;
    }
    else if (eCpState == eCddCPVolState_12V)
    {
        CddRelay_CtrlSwichOff(port);
        AswEVSE_SetEVSEState(port, ASWEVSE_STATE_1_DOT);
    }
    else
    {
        if (pEVSECtrl->startCharge == FALSE)
        {
            CddCP_StopPWM(port);
            AswEVSE_SetEVSEState(port, ASWEVSE_STATE_3);
            pEVSECtrl->S2CloseTimer = Common_GetSystick();
        }
    }
}

static void AswEVSE_StateManage(uint8_t port, AswEVSECtrl_Struct *pEVSECtrl)
{
    CddCPVolState_Enum eCpState = CddCP_GetVolState(port);

    AswEVSE_StateEnterState0(port, pEVSECtrl);

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
        if (pEVSECtrl->startCharge != TRUE)
        {
            pEVSECtrl->startCharge = TRUE;
            ASWEVSE_CFG_LogPrint("[枪：%d]请求开始充电！\r\n");
        }
    }
}

void AswEVSE_StopCharge(uint8_t port)
{
    AswEVSECtrl_Struct *pEVSECtrl = &g_stAswEVSECtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pEVSECtrl->startCharge != FALSE)
        {
            pEVSECtrl->startCharge = FALSE;
            ASWEVSE_CFG_LogPrint("[枪：%d]请求停止充电！\r\n");
        }
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
