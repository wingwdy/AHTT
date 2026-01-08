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
*2025/10/10      V1.0.0      chenls    初版创建
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

#define CDDRELAY_SHORTCUT_STEP0         0
#define CDDRELAY_SHORTCUT_STEP1         1
#define CDDRELAY_SHORTCUT_STEP2         2
#define CDDRELAY_SHORTCUT_STEP3         3
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

    uint8_t shortCutDetectStep;
    uint8_t shortCutDetectResult;
    FilterProfile1_Struct stFilterShortCutDetect;
    uint32_t shortCutDetectTimer;

    FilterProfile1_Struct stFilterAdhesionDetect;
    uint8_t adhesionDetectValidFlag;
    uint32_t adhesionDetectStartTick;
    
    FilterProfile1_Struct stFilterMaloperationDetect;
    uint32_t relayCtrlHoldTick;
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
static void CddRelay_ShortCutDetect(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddRelay_IdleHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{    
    if (pRelayCtrl->eRelayCtrlOpt == eCddRelayCtrlState_SwitchOn)
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
            if (pRelayCtrl->stFilterMaloperationDetect.validStatus == TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_JcqMaloperation);
            }
        }
    }
    else
    {
        if (c_stCddRelayOpsConfigTable.pFuncGetRelayAdhesionStatus != NULL)
        {
            pRelayCtrl->stFilterAdhesionDetect.status = c_stCddRelayOpsConfigTable.pFuncGetRelayAdhesionStatus(port);
        }
        else
        {
            pRelayCtrl->stFilterAdhesionDetect.status = FALSE;
        }

        if (Filter_Profile1(&pRelayCtrl->stFilterAdhesionDetect, CDDRELAY_CFG_ADHESION_FILTER_COUNT))
        {
            pRelayCtrl->adhesionDetectValidFlag = TRUE;
            CDDRELAY_CFG_LogPrint("[枪：%d]粘连检测完成!\r\n", port);
            if (pRelayCtrl->stFilterAdhesionDetect.validStatus == TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, eErr_JcqSynechiaFault);
            }
        }
        else if (pRelayCtrl->adhesionDetectValidFlag == FALSE)
        {
            if (Common_JudgeTimeoutMs(pRelayCtrl->adhesionDetectStartTick, CDDRELAY_CFG_ADHESION_DETECT_TIMEOUT))
            {
                pRelayCtrl->adhesionDetectStartTick = Common_GetSystick();
                pRelayCtrl->adhesionDetectValidFlag = TRUE;
                CDDRELAY_CFG_LogPrint("[枪：%d]粘连检测完成!\r\n", port);
            }
        }
        else
        {}
        
        if ((pRelayCtrl->stFilterMaloperationDetect.validStatus == TRUE) &&
            (TRUE == CDDRELAY_CFG_CheckGunPlugout(port)))
        {
            memset(&pRelayCtrl->stFilterMaloperationDetect, 0x00, sizeof(FilterProfile1_Struct));
            AswErrhandle_ResetErrExsitCallback(port, eErr_JcqMaloperation);
        }
    }
 
}

static void CddRelay_SwitchOnHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->relayCtrlStep)
    {
        case CDDRELAY_CTRL_STEP_1:
        {
            if (Common_JudgeTimeoutMs(pRelayCtrl->relayCtrlHoldTick, CDDRELAY_CFG_ACT_HOLD_TIMEOUT))
            {
                pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_2;

                if (c_stCddRelayOpsConfigTable.pFuncHoldSwitchOn != NULL)
                {
                    c_stCddRelayOpsConfigTable.pFuncHoldSwitchOn(port);
                }

                pRelayCtrl->relayCtrlHoldTick = Common_GetSystick();
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

static void CddRelay_SwitchOffHandle(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->relayCtrlStep)
    {
        case CDDRELAY_CTRL_STEP_1:
        {
            if (Common_JudgeTimeoutMs(pRelayCtrl->relayCtrlHoldTick, CDDRELAY_CFG_ACT_DELAY_TIMEOUT))
            {
                pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_2;
            }

            break;
        }
        case CDDRELAY_CTRL_STEP_2:
        {
            pRelayCtrl->adhesionDetectStartTick = Common_GetSystick();
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
    pRelayCtrl->stRelayState.status = (uint8_t)c_stCddRelayOpsConfigTable.pFuncGetSwitchStatus(port);

    Filter_Profile1(&pRelayCtrl->stRelayState, CDDRELAY_CFG_STATE_FILTER_COUNT);

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

static void CddRelay_ShortCutDetect(uint8_t port, CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->shortCutDetectStep)
    {
        case CDDRELAY_SHORTCUT_STEP0:
        {
            break;
        }

        case CDDRELAY_SHORTCUT_STEP1:
        {
            if (pRelayCtrl->adhesionDetectValidFlag == TRUE)
            {
                if (c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOn != NULL)
                {
                    c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOn(port);
                }

                CDDRELAY_CFG_LogPrint("[枪：%d]投入输出短路检测回路!\r\n", port);

                pRelayCtrl->shortCutDetectTimer = Common_GetSystick();
                pRelayCtrl->shortCutDetectStep = CDDRELAY_SHORTCUT_STEP2;
                pRelayCtrl->shortCutDetectResult = GLOBAL_OPT_STATE_PROCESS;
            }

            break;
        }

        case CDDRELAY_SHORTCUT_STEP2:
        {
            if (Common_JudgeTimeoutMs(pRelayCtrl->shortCutDetectTimer, CDDRELAY_CFG_SHORTCUT_TIMEOUT))
            {
                pRelayCtrl->shortCutDetectResult = GLOBAL_OPT_STATE_FAIL;
                pRelayCtrl->shortCutDetectStep = CDDRELAY_SHORTCUT_STEP0;
                CDDRELAY_CFG_LogPrint("[枪：%d]输出短路检测超时[%d]ms!\r\n", port, CDDRELAY_CFG_SHORTCUT_TIMEOUT);
                AswErrhandle_SetErrExsitCallback(port, eErr_ShortCircleErr);
                if (c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff != NULL)
                {
                    c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff(port);
                }

                CDDRELAY_CFG_LogPrint("[枪：%d]断开输出短路检测回路!\r\n", port);
            }
            else
            {
                if (c_stCddRelayOpsConfigTable.pFuncGetShortCutStatus != NULL)
                {
                    pRelayCtrl->stFilterShortCutDetect.status = c_stCddRelayOpsConfigTable.pFuncGetShortCutStatus(port);
                }
                else
                {
                    pRelayCtrl->stFilterShortCutDetect.status = FALSE;
                }

                if (Filter_Profile1(&pRelayCtrl->stFilterShortCutDetect, CDDRELAY_CFG_SHORTCUT_FILTER_COUNT))
                {
                    if (pRelayCtrl->stFilterShortCutDetect.validStatus == TRUE)
                    {
                        pRelayCtrl->shortCutDetectResult = GLOBAL_OPT_STATE_SUCCESS;
                        pRelayCtrl->shortCutDetectStep = CDDRELAY_SHORTCUT_STEP0;
                        CDDRELAY_CFG_LogPrint("[枪：%d]输出短路检测成功!\r\n", port);

                        if (c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff != NULL)
                        {
                            c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff(port);
                        }

                        CDDRELAY_CFG_LogPrint("[枪：%d]断开输出短路检测回路!\r\n", port);
                    }
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

uint8_t CddRelay_GetShortCutResult(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = NULL;
    uint8_t ret = GLOBAL_OPT_STATE_FAIL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pRelayCtrl = &g_stRelayCtrl[port];
        ret = pRelayCtrl->shortCutDetectResult;
    }

    return ret;
}

void CddRelay_SetReqStartShortCutDetect(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pRelayCtrl->shortCutDetectStep == CDDRELAY_SHORTCUT_STEP0)
        {
            pRelayCtrl->shortCutDetectStep = CDDRELAY_SHORTCUT_STEP1;
            pRelayCtrl->shortCutDetectResult = GLOBAL_OPT_STATE_PROCESS;
            memset(&pRelayCtrl->stFilterShortCutDetect, 0x00, sizeof(pRelayCtrl->stFilterShortCutDetect));
            CDDRELAY_CFG_LogPrint("[枪：%d]请求执行输出短路检测!\r\n", port);
        }
    }
}

void CddRelay_SetReqStopShortCutDetect(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pRelayCtrl->shortCutDetectStep != CDDRELAY_SHORTCUT_STEP0)
        {
            pRelayCtrl->shortCutDetectStep = CDDRELAY_SHORTCUT_STEP0;
            pRelayCtrl->shortCutDetectResult = GLOBAL_OPT_STATE_IDLE;

            CDDRELAY_CFG_LogPrint("[枪：%d]请求中止输出短路检测!\r\n", port);

            if (c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff != NULL)
            {
                c_stCddRelayOpsConfigTable.pFuncCtrlShortCutOff(port);
            }

            CDDRELAY_CFG_LogPrint("[枪：%d]断开输出短路检测回路!\r\n", port);
        }
    }
}

void CddRelay_InitMemory(void)
{
    CddRelayCtrl_Struct *pRelayCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pRelayCtrl = &g_stRelayCtrl[port];

        memset(pRelayCtrl, 0x00, sizeof(CddRelayCtrl_Struct));
        pRelayCtrl->eRelayCtrlOpt = eCddRelayCtrlState_SwitchOff;
        pRelayCtrl->adhesionDetectStartTick = Common_GetSystick();
    }
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
            pRelayCtrl->relayCtrlHoldTick = Common_GetSystick();

            if (c_stCddRelayOpsConfigTable.pFuncCtrlSwitchOn != NULL)
            {
                c_stCddRelayOpsConfigTable.pFuncCtrlSwitchOn(port);
            }

            CDDRELAY_CFG_LogPrint("[枪：%d]请求闭合继电器!\r\n", port);
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
            pRelayCtrl->relayCtrlHoldTick = Common_GetSystick();

            if (c_stCddRelayOpsConfigTable.pFuncCtrlSwitchOff != NULL)
            {
                c_stCddRelayOpsConfigTable.pFuncCtrlSwitchOff(port);
            }

            pRelayCtrl->adhesionDetectValidFlag = FALSE;

            memset(&pRelayCtrl->stFilterAdhesionDetect, 0x00, sizeof(FilterProfile1_Struct));
            CDDRELAY_CFG_LogPrint("[枪：%d]请求断开继电器!\r\n", port);
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
        CddRelay_ShortCutDetect(port, pRelayCtrl);
    }
}


















