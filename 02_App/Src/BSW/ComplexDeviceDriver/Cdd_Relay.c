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
#include "Cdd_Relay.h"
#include "SysCfg.h"
#include "Filter.h"
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
    FilterProfile1_Struct stFilterAdhesionDetect;
}CddRelayCtrl_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddRelayCtrl_Struct g_stRelayCtrl[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void CddRelay_InitMemory(void)
{
    memset(g_stRelayCtrl, 0x00, sizeof(g_stRelayCtrl));
}

void CddRelay_CtrlSwichOn(uint8_t port)
{
    CddRelayCtrl_Struct *pRelayCtrl = &g_stRelayCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pRelayCtrl->eRelayCtrlState != eCddRelayCtrlState_SwitchOn)
        {
            pRelayCtrl->eRelayCtrlState = eCddRelayCtrlState_SwitchOn;
            pRelayCtrl->relayCtrlStep = CDDRELAY_CTRL_STEP_1;
        }
    }
}

static void CddRelay_IdleHandle(CddRelayCtrl_Struct *pRelayCtrl)
{
    if (pRelayCtrl->eRelayCtrlState == eCddRelayCtrlState_SwitchOn)
    {




    }
    else
    {
        pRelayCtrl->stFilterAdhesionDetect.status = CDDRELAY_CFG_GetRelayAdhesionState();

        if (Filter_Profile1(&pRelayCtrl->stFilterAdhesionDetect, CDDRELAY_CFG_ADHESION_CHECK_TIMEOUT))
        {
            if (pRelayCtrl->stFilterAdhesionDetect.validStatus == TRUE)
            {

            }
            else
            {
                
            }


        }




    }



}

static void CddRelay_SwitchOnHandle(CddRelayCtrl_Struct *pRelayCtrl)
{
    switch (pRelayCtrl->relayCtrlStep)
    {
    case CDDRELAY_CTRL_STEP_1:
    case CDDRELAY_CTRL_STEP_2:
    case CDDRELAY_CTRL_STEP_3:
    case CDDRELAY_CTRL_STEP_4:
















    }




}

static void CddRelay_SwitchOffHandle(CddRelayCtrl_Struct *pRelayCtrl)
{





}





static void CddRelay_RelayStateManage(CddRelayCtrl_Struct *pRelayCtrl, uint8_t port)
{
    switch (pRelayCtrl->eRelayCtrlState)
    {
        case eCddRelayCtrlState_Idle:
        {


        }












    }











}









void CddRelay_MainFunction(void)
{
    CddRelayCtrl_Struct *pRelayCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; gunNo++)
    {
        pRelayCtrl = &g_stRelayCtrl[port];
        switch (pRelayCtrl->eRelayCtrlState)
        {
            case 
















        }












    }














}


















