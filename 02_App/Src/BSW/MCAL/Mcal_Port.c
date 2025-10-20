
/******************************************************************************
* File Name          : Mcal_Port.c
* Description        : Code for Pin-level configuration module for hardware
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
#include "Mcal_Port_Config.h"
#include "Common.h"



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
const McalPortPinConfig_Struct c_stPorPinConfigTable[] = McalPort_CFG_PinLISTArray;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void McalPort_ConfigPin(const McalPortPinConfig_Struct *pPortPinConfig);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static void McalPort_ConfigPin(const McalPortPinConfig_Struct *pPortPinConfig)
{
    if (pPortPinConfig != NULL)
    {
        rcu_periph_clock_enable(pPortPinConfig->rcu_periph);
        gpio_init(pPortPinConfig->rcu_periph, pPortPinConfig->mode, pPortPinConfig->speed, pPortPinConfig->pin);

        if (pPortPinConfig->mode == GPIO_MODE_OUT_PP || pPortPinConfig->mode == GPIO_MODE_OUT_OD)
        {
            gpio_bit_write(pPortPinConfig->rcu_periph, pPortPinConfig->pin, pPortPinConfig->sta_init);
        }
    }
}

void McalPort_WritePin(McalPortPinChanel_Enum ePinChannel, uint8_t pinVal)
{
    uint8_t val = (pinVal != MCALPORT_PIN_LOW) ? MCALPORT_PIN_HIGH : MCALPORT_PIN_LOW;
    const McalPortPinConfig_Struct *pPortPinConfig = NULL;

    if (ePinChannel < eMcalPortPinChanel_Count)
    {
        pPortPinConfig = &c_stPorPinConfigTable[ePinChannel];
        gpio_bit_write(pPortPinConfig->rcu_periph, pPortPinConfig->pin, val);
    }
}

void McalPort_SetPin(McalPortPinChanel_Enum ePinChannel)
{
    McalPort_WritePin(ePinChannel, MCALPORT_PIN_HIGH);
}

void McalPort_ResetPin(McalPortPinChanel_Enum ePinChannel)
{
    McalPort_WritePin(ePinChannel, MCALPORT_PIN_LOW);
}

void McalPort_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < ARRAY_SIZE(c_stPorPinConfigTable); index++)
    {
        McalPort_ConfigPin(&c_stPorPinConfigTable[index]);
    }
}




























