/******************************************************************************
* File Name          : Cdd_4GM.c
* Description        : Code for 4GM Driver
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
#include "Cdd_4GM.h"
#include "Cdd_4GMConfig.h"
/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/



/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDD_4GM_CTRL_STEP0         0
#define CDD_4GM_CTRL_STEP1         1
#define CDD_4GM_CTRL_STEP2         2
#define CDD_4GM_CTRL_STEP3         3   
#define CDD_4GM_CTRL_STEP4         4
#define CDD_4GM_CTRL_STEPEND       9

/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint8_t powerOnStep;                    /*: 重启控制步骤 */
    uint32_t powerCtrlStartTick;            /*: 重启控制时间 */
    uint8_t transparentMode;                /*TRUE: 透传模式 */  
    NetMModuleState_Enum eModuleState;


}CddDrv4GMCtrl_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddDrv4GMCtrl_Struct g_stCddDrv4GMCtrl = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t Cdd4GM_PowerOn(void);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t Cdd4GM_PowerOn(void)
{
    uint8_t result = GLOBAL_OPT_STATE_PROCESS;

    switch (g_stCddDrv4GMCtrl.powerOnStep)
    {
        case CDD_4GM_CTRL_STEP0:
        {
            CDD_4GM_CFG_PwrOff();
            g_stCddDrv4GMCtrl.powerCtrlStartTick = Common_GetSystick();
            g_stCddDrv4GMCtrl.powerOnStep = CDD_4GM_CTRL_STEP1;
            break;
        }
        case CDD_4GM_CTRL_STEP1:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrv4GMCtrl.powerCtrlStartTick, CDD_4GM_CFG_POWEROFF_HOLD_TIME))
            {
                CDD_4GM_CFG_PwrOn();
                g_stCddDrv4GMCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrv4GMCtrl.powerOnStep = CDD_4GM_CTRL_STEP2;
            }

            break;
        }
        case CDD_4GM_CTRL_STEP2:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrv4GMCtrl.powerCtrlStartTick, CDD_4GM_CFG_POWERON_HOLD_TIME))
            {
                CDD_4GM_CFG_PwrKeyOff();
                g_stCddDrv4GMCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrv4GMCtrl.powerOnStep = CDD_4GM_CTRL_STEP3;
            }

            break;
        }
        case CDD_4GM_CTRL_STEP3:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrv4GMCtrl.powerCtrlStartTick, CDD_4GM_CFG_POWERKEY_OFF_HOLD_TIME))
            {
                CDD_4GM_CFG_PwrKeyOn();
                g_stCddDrv4GMCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrv4GMCtrl.powerOnStep = CDD_4GM_CTRL_STEP4;
            }

            break;
        }
        case CDD_4GM_CTRL_STEP4:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrv4GMCtrl.powerCtrlStartTick, CDD_4GM_CFG_POWERKEY_ON_HOLD_TIME))
            {
                g_stCddDrv4GMCtrl.powerOnStep = CDD_4GM_CTRL_STEPEND;
            }

            break;
        }
        case CDD_4GM_CTRL_STEPEND:
        {
            result = GLOBAL_OPT_STATE_SUCCESS;
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

void Cdd4GM_InitMemory(void)
{



}

void Cdd4GM_SetModuleState(NetMModuleState_Enum eModuleState)
{
    if (eModuleState != g_stCddDrv4GMCtrl.eModuleState)
    {
        g_stCddDrv4GMCtrl.eModuleState = eModuleState;
    }
}

void Cdd4GM_MainFunction(void)
{
    static uint8_t initFlag = FALSE;

    switch (g_stCddDrv4GMCtrl.eModuleState)
    {
        case eNetMModuleState_Init:
        {
            if (GLOBAL_OPT_STATE_SUCCESS == Cdd4GM_PowerOn())
            {
                g_stCddDrv4GMCtrl.eModuleState = eNetMModuleState_Cfg;
            }

            break;
        }
        case eNetMModuleState_Cfg:
        {
            // if (initFlag == FALSE)
            {
                initFlag = TRUE;
                char txStr[] = "AT+CGREG?\r\n";
                CDD_4GM_CFG_WriteData((uint8_t *)txStr, strlen(txStr));
            }

          //  g_stCddDrv4GMCtrl.eModuleState = eNetMModuleState_Work;
            break;
        }
        case eNetMModuleState_Work:
        {
            break;
        }
        case eNetMModuleState_AbNormal:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}






























