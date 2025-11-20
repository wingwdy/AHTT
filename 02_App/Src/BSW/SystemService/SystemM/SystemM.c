/******************************************************************************
* File Name          : SystemM.c
* Description        : Code for the Implementation of the System Management
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/11      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_ErrorHandle.h"
#include "Asw_EVSE.h"
#include "Asw_Charge.h"

#include "Cdd_Relay.h"

#include "Mcal_If.h"
#include "Mcal_Mcu.h"
#include "Mcal_Uart.h"
#include "Cdd_CP.h"
#include "Common.h"
#include "stdio.h"


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



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void SystemM_ShowInfo(void);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void SystemM_InitOne(void)
{
    McalIf_Init();
    SystemM_ShowInfo();
    McalMcu_ClearResetFlags();
}

static void SystemM_InitTwo(void)
{


}

static void SystemM_InitThree(void)
{
    CddRelay_InitMemory();
    CddCP_InitMemory();
    AswErrHandle_InitMemory();
}

static void SystemM_ShowInfo(void)
{
    McalMcuResetSource_Enum eResetSource = McalMcu_GetResetSource();
    char printInfo[64] = {0};

    struct 
    {
        char *cShowInfo;
    }st[] = 
    {
        {"未知"},
        {"外部引脚复位"},
        {"上电/掉电复位"},
        {"软件复位"},
        {"独立看门狗复位"},
        {"窗口看门狗复位"},
        {"低功耗复位"},
    };

    snprintf(printInfo, sizeof(printInfo), "复位源：%s\r\n", st[eResetSource].cShowInfo);
    McalUart_WriteData(eMcalUartChanel_Debug, (uint8_t *)printInfo, strlen(printInfo));
}

void SystemM_Init(void)
{
    SystemM_InitOne();
    SystemM_InitTwo();
    SystemM_InitThree();
}






















