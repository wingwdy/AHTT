/******************************************************************************
* File Name          : SS_SystemM.c
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
#include "Cdd_CP.h"
#include "Cdd_Rcd.h"
#include "Cdd_PE.h"
#include "Cdd_Sensor.h"
#include "Cdd_MeterM.h"
#include "Cdd_LedM.h"

#include "MS_Nvm.h"

#include "Mcal_If.h" 
#include "Mcal_Mcu.h"

#include "DS_LogM.h"
#include "DS_Console.h"

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
static void SSSystemM_ShowInfo(void);
static void SSSystemM_InitOne(void);
static void SSSystemM_InitTwo(void);
static void SSSystemM_InitThree(void);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void SSSystemM_InitOne(void)
{
    McalIf_Init();
    DSLogM_InitMemory();
    DSConsole_Init();
    SSSystemM_ShowInfo();
    McalMcu_ClearResetFlags();
}

static void SSSystemM_InitTwo(void)
{
    MSNvm_InitMemory();
    MSNvm_ReadAll();
}

static void SSSystemM_InitThree(void)
{
    CddLedM_InitMemory();
    CddMeterM_InitMemory();
    CddRelay_InitMemory();
    CddCP_InitMemory();
    CddRcd_InitMemory();
    CddPE_InitMemory();

    AswEVSE_InitMemory();
    AswCharge_InitMemory();
    AswErrHandle_InitMemory();
}

static void SSSystemM_ShowInfo(void)
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
    DSLOGM_Debug(DSLogMModule_System, "%s", printInfo);
}

void SSSystemM_Init(void)
{
    SSSystemM_InitOne();
    SSSystemM_InitTwo();
    SSSystemM_InitThree();
}






















