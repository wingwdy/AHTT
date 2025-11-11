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
*2025/11/11      V1.0.0      Chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Mcal_If.h"
#include "Mcal_Mcu.h"
#include "Mcal_Uart.h"
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
void SystemM_InitOne(void)
{
    McalIf_Init();
    SystemM_ShowInfo();
    McalMcu_ClearResetFlags();
}

void SystemM_InitTwo(void)
{


    
}

void SystemM_InitThree(void)
{

}

static void SystemM_ShowInfo(void)
{
    McalMcuResetSource_Enum eResetSource = McalMcu_GetResetSource();
    char printInfo[64] = {0};

    struct 
    {
        McalMcuResetSource_Enum eResetSource;
        char *cShowInfo;
    }st[] = 
    {
        {eMcalMcuResetSource_Unknown, "未知"},
        {eMcalMcuResetSource_Unknown, "外部引脚复位"},
        {eMcalMcuResetSource_Unknown, "上电/掉电复位"},
        {eMcalMcuResetSource_Unknown, "软件复位"},
        {eMcalMcuResetSource_Unknown, "独立看门狗复位"},
        {eMcalMcuResetSource_Unknown, "窗口看门狗复位"},
        {eMcalMcuResetSource_Unknown, "低功耗复位"},
    };

    snprintf(printInfo, sizeof(printInfo), "复位源：%s\r\n", st[eResetSource].cShowInfo);
    McalUart_WriteData(eMcalUartChanel_Debug, (uint8_t *)printInfo, strlen(printInfo));
}
























