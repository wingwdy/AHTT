/******************************************************************************
* File Name          : Cdd_Relay.c
* Description        : Code for the driver of relay
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Common.h"
#include "Cdd_RelayConfig.h"
#include "Asw_ErrorHandle.h"
#include "SysCfg.h"
#include "Filter.h"
#include "Common.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDDRELAY_CTRL_STEP_1            0
#define CDDRELAY_CTRL_STEP_2            1
#define CDDRELAY_CTRL_STEP_3            2
#define CDDRELAY_CTRL_STEP_4            3

/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eCddRelayCtrlState_Idle,
    eCddRelayCtrlState_SwitchOn,
    eCddRelayCtrlState_SwitchOff,
}CddRelayCtrlState_Enum;






/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t relayCtrlStep;
    CddRelayCtrlState_Enum eRelayCtrlState;
    CddRelayCtrlState_Enum eRelayCtrlOpt;
    FilterProfile1_Struct stRelayState;
    FilterProfile1_Struct stFilterAdhesionDetect;
    FilterProfile1_Struct stFilterMaloperationDetect;
    uint32_t holdTick;
}CddRelayCtrl_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddRelayCtrl_Struct g_stRelayCtrl[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void CddRelay_IdleHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl);
static void CddRelay_SwitchOnHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl);
static void CddRelay_SwitchOffHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddRelay_IdleHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{    
    CDDRELAY_CFG_GetRelayState(port, pRelayCtrl->stRelayState.status);
    Filter_Profile1(&pRelayCtrl->stRelayState, CDDRELAY_CFG_STATE_FILTER_COUNT);

    if (pRelayCtrl->eRelayCtrlState == eCddRelayCtrlState_SwitchOn)
    {
        if (pRelayCtrl->stRelayState.validStatus == (uint8_t)eCddRelayState_Off)
        {
            pRelayCtrl->stFilterMaloperationDetect.status = TRUE;
        }
        else
        {
            pRelayCtrl->stFilterMaloperationDetect.status = FALSE;
        }

        if (Filter_Profile1(&pRelayCtrl->stFilterMaloperationDetect, CDDRELAY_CFG_MALOPERATION_FILTER_COUNT))
        {
            if (pRelayCtrl->stFilterAdhesionDetect.validStatus == TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_JcqMaloperation);
            }
        }
    }
    else
    {
        CDDRELAY_CFG_GetRelayAdhesionState(port, pRelayCtrl->stFilterAdhesionDetect.status);

        if (Filter_Profile1(&pRelayCtrl->stFilterAdhesionDetect, CDDRELAY_CFG_ADHESION_FILTER_COUNT))
        {
            if (pRelayCtrl->stFilterAdhesionDetect.validStatus == TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_JcqSynechiaFault);
            }
        }
    }
}

static void CddRelay_SwitchOnHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->relayCtrlStep)
    {
    case CDDRELAY_CTRL_STEP_1:
    {
        if (Common_JudgeTimeoutMs(pRelayCtrl->holdTick, CDDRELAY_ACT_HOLD_TIMEOUT))
        {
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_2;
            CDDRELAY_CFG_HoldSwitchOn(port);
            pRelayCtrl->holdTick = Common_GetSystick();
        }

        break;
    }
    case CDDRELAY_CTRL_STEP_2:
    {
        if (Common_JudgeTimeoutMs(pRelayCtrl->holdTick, CDDRELAY_ACT_DELAY_TIMEOUT))
        {
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_3;
        }

        break;
    }
    case CDDRELAY_CTRL_STEP_3:
    {
        pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_Idle;
        break;
    }
    default:
    {
        pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_Idle;
        break;
    }
    }
}

static void CddRelay_SwitchOffHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->relayCtrlStep)
    {
    case CDDRELAY_CTRL_STEP_1:
    {
        if (Common_JudgeTimeoutMs(pRelayCtrl->holdTick, CDDRELAY_ACT_DELAY_TIMEOUT))
        {
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_2;
            CDDRELAY_CFG_CtrlSwitchOff(port);
        }

        break;
    }
    case CDDRELAY_CTRL_STEP_2:
    {
        pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_Idle;
        break;
    }
    default:
    {
        pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_Idle;
        break;
    }
    }
}

static void CddRelay_RelayStateManage(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->eRelayCtrlState)
    {
    case eCddRelayCtrlState_Idle:
    {
        CddRelay_IdleHandle(port, pRelayCtrl);
        break;
    }
    case eCddRelayCtrlState_SwitchOn:
    {
        CddRelay_SwitchOnHandle(port, pRelayCtrl);
        break;
    }
    case eCddRelayCtrlState_SwitchOff:
    {
        CddRelay_SwitchOffHandle(port, pRelayCtrl);
        break;
    }    
    default:
    {
        CddRelay_SwitchOffHandle(port, pRelayCtrl);
        break;       
    }   
    }
}

void CddRelay_InitMemory(void)
{
    memset(g_stRelayCtrl, 0x00, sizeof(g_stRelayCtrl));
}

CddRelayState_Enum CddRelay_GetRelayState(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];
    CddRelayState_Enum eState = eCddRelayState_Off;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        eState = (CddRelayState_Enum)pRelayCtrl->stRelayState.validStatus;
    }

    return eState;
}

void CddRelay_CtrlSwichOn(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pRelayCtrl->eRelayCtrlOpt != eCddRelayCtrlState_SwitchOn)
        {
            pRelayCtrl->eRelayCtrlOpt = eCddRelayCtrlState_SwitchOn;
            pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_SwitchOn;
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_1;
            pRelayCtrl->holdTick = Common_GetSystick();
            CDDRELAY_CFG_CtrlSwitchOn(port);
            memset(&pRelayCtrl->stFilterMaloperationDetect, 0x00, sizeof(FilterProfile1_Struct));
        }
    }
}

void CddRelay_CtrlSwichOff(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pRelayCtrl->eRelayCtrlOpt != eCddRelayCtrlState_SwitchOff)
        {
            pRelayCtrl->eRelayCtrlOpt = eCddRelayCtrlState_SwitchOff;
            pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_SwitchOff;
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_1;
            pRelayCtrl->holdTick = Common_GetSystick();
            CDDRELAY_CFG_CtrlSwitchOff(port);
            memset(&pRelayCtrl->stFilterMaloperationDetect, 0x00, sizeof(FilterProfile1_Struct));
        }
    }
}

void CddRelay_MainFunction(void)
{
    CddRelayCtrl_Struct *pRelayCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pRelayCtrl = &g_stRelayCtrl[port];
        CddRelay_RelayStateManage(port, pRelayCtrl);
    }
}


















