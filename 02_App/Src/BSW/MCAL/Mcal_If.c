/******************************************************************************
* File Name          : Mcal_if.c
* Description        : Code for the interface for the layer of MCAL
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
#include "Mcal_Mcu.h"
#include "Mcal_Port.h"
#include "Mcal_PWM.h"
#include "Mcal_ADC.h"
#include "Mcal_Uart.h"
#include "CycleBuf.h"
#include "Mcal_IWDG.h"
#include "Mcal_SPI.h"



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



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void McalIf_Init(void)
{
    McalMCU_SystickInit();
    McalIWDG_Init();
    CycleBuf_Init();
    McalPort_Init();
    McalUart_Init();
    McalADC_Init();
    McalPWM_Init();
    McalSPI_Init();
}

void McalIf_Test(void)
{
    McalPWM_Test();
    // McalADC_Test();

    // MalPort_TogglePin(eMcalPortPinChanel_PA1_RunLed);

    static uint32_t StateCnt = 0;
    if (StateCnt < 8)
    {
        StateCnt++;
    }

    if (StateCnt == 1)
    {
        McalPort_SetPin(eMcalPortPinChanel_PC15_4GPwrEn);
    }
    else if (StateCnt == 3)
    {
        McalPort_ResetPin(eMcalPortPinChanel_PC15_4GPwrEn);
    }
    else if (StateCnt == 5)
    {
        McalPort_SetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn);
    }
    else if (StateCnt == 7)
    {
        McalPort_ResetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn); 
    } 
    else if (StateCnt == 8)
    {
        McalUart_Test();
    }
}




























