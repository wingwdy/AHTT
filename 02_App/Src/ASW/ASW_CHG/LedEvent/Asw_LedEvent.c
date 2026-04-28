/******************************************************************************
* File Name          : Asw_LedEvent.c
* Description        : Code for Led event manage
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
#include "Asw_LedEventConfig.h"



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
static uint32_t g_startupTick = 0;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void AswLedEvent_ScanLedDeviceStatusSource(AswLedConfig_Struct *pLedConfig)
{
    const AswLedEventConfig_Struct *pEventConfig = NULL;
    AswLedEventCtrl_Struct *pEventCtrl = NULL;
    uint8_t eventCount = 0;
    uint8_t port = 0;
    uint8_t validFlag = FALSE;

    for (eventCount = 0; eventCount < pLedConfig->ledEventCount; eventCount++)
    {
        pEventCtrl = &pLedConfig->pEventCtrl[eventCount];
        pEventConfig = pEventCtrl->pLedEventConfig;

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (pEventConfig->pFuncGetStateExsist != NULL)
            {
                pEventCtrl->currentState[port] = pEventConfig->pFuncGetStateExsist(port);

                validFlag = FALSE;

                if (pEventConfig->eLedEventType == eAswLedEventType_State)
                {
                    if (pEventCtrl->currentState[port] == TRUE)
                    {
                        validFlag = TRUE;
                    }
                }
                else
                {
                    if (pEventCtrl->currentState[port] != pEventCtrl->lastState[port])
                    {
                        if (pEventCtrl->currentState[port] == TRUE)
                        {
                            validFlag = TRUE;
                        }
                    }
                }

                pEventCtrl->lastState[port] = pEventCtrl->currentState[port];

                if (validFlag == TRUE)
                {
                    if (pEventConfig->eLedPriority > pLedConfig->currentHighestLedPriority[port])
                    {
                        pLedConfig->currentHighestLedPriority[port] = pEventConfig->eLedPriority;
                        pLedConfig->currentHighestPriorityledDispType[port] = pEventConfig->ledDispType;
                        pLedConfig->currentHighestLedDesc[port] = pEventConfig->desc;
                    }
                }
            }
        }
    }
}

void AswLedEvent_HandleLedDeviceStatusChange(uint8_t dev, AswLedConfig_Struct *pLedConfig)
{
    const AswLedEventConfig_Struct *pEventConfig = NULL;
    AswLedEventCtrl_Struct *pEventCtrl = NULL;
    uint8_t eventCount = 0;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (pLedConfig->lastHighestPriorityledDispType[port] != pLedConfig->currentHighestPriorityledDispType[port])
        {  
            if (pLedConfig->lastHighestLedDesc[port] == NULL)
            {
                pLedConfig->lastHighestLedDesc[port] = "初始状态";
            }

            ASWLED_CFG_InfoPrint("[枪：%d]灯语状态发生变化：[%s] ---> [%s]\r\n", port, pLedConfig->lastHighestLedDesc[port], pLedConfig->currentHighestLedDesc[port]);

            pLedConfig->lastHighestLedDesc[port] = pLedConfig->currentHighestLedDesc[port];            
            pLedConfig->lastHighestPriorityledDispType[port] = pLedConfig->currentHighestPriorityledDispType[port];
        }

        if (pLedConfig->pFuncCddDrv != NULL)
        {
            pLedConfig->pFuncCddDrv(dev, port, pLedConfig->currentHighestPriorityledDispType[port]);
        }

        pLedConfig->currentHighestLedDesc[port] = NULL;
        pLedConfig->currentHighestLedPriority[port] = eAswLedPriority_0;
    }
}
void AswLedEvent_MainFunction(void)
{
    AswLedConfig_Struct *pLedConfig = NULL;
    uint8_t dev = 0;

    if (Common_JudgeTimeoutMs(g_startupTick, 500))
    {
        for (dev = 0; dev < ARRAY_SIZE(g_AswLedConfigTable); dev++)
        {
            pLedConfig = &g_AswLedConfigTable[dev];
            AswLedEvent_ScanLedDeviceStatusSource(pLedConfig);
            AswLedEvent_HandleLedDeviceStatusChange(dev, pLedConfig);
        }
    }
}























