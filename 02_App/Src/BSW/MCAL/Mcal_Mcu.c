/******************************************************************************
* File Name          : Mcal_Mcu.c
* Description        : Code for the driver for Mcu
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
#include "gd32e50x_misc.h"
#include "FreeRTOS.h"
#include "gd32e50x_rcu.h"
#include "Mcal_Mcu.h"
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
void McalMCU_SystickInit(void)
{
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);

    if(0U == SysTick_Config(SystemCoreClock / 1000U))
    {
        NVIC_SetPriority(SysTick_IRQn, 0x00U);
    }
}

void McalMcu_SystemReset(void)
{
    NVIC_SystemReset();
}

McalMcuResetSource_Enum McalMcu_GetResetSource(void)
{
    FlagStatus flag_status;
    McalMcuResetSource_Enum eResetSource = eMcalMcuResetSource_Unknown;

    if (SET == rcu_flag_get(RCU_FLAG_PORRST)) 
    {
        eResetSource = eMcalMcuResetSource_PowerOn;
    } 
    else if (SET == rcu_flag_get(RCU_FLAG_SWRST)) 
    {
        eResetSource =  eMcalMcuResetSource_Software;
    } 
    else if (SET == rcu_flag_get(RCU_FLAG_FWDGTRST)) 
    {
        eResetSource = eMcalMcuResetSource_FWDGT;
    } 
    else if (SET == rcu_flag_get(RCU_FLAG_WWDGTRST)) 
    {
        eResetSource = eMcalMcuResetSource_WWDGT;
    } 
    else if (SET == rcu_flag_get(RCU_FLAG_LPRST)) 
    {
        eResetSource = eMcalMcuResetSource_Lowpower;
    }
    else if (SET == rcu_flag_get(RCU_FLAG_EPRST))
    {
        eResetSource = eMcalMcuResetSource_ExternalPin;
    }
    else
    {
        eResetSource = eMcalMcuResetSource_Unknown;
    }

    return eResetSource;
}

void McalMcu_ClearResetFlags(void)
{
    rcu_all_reset_flag_clear();
}

void SysTick_Handler(void)
{
    xPortSysTickHandler();
}

void NMI_Handler(void)
{

}

void HardFault_Handler(void)
{
    while(1)
    {}
}


























