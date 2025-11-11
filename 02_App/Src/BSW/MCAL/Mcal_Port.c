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
#include "Mcal_PortConfig.h"



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
static void McalPort_ConfigPin(const McalPortPinConfig_Struct *pPortPinConfig);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static void McalPort_ConfigPin(const McalPortPinConfig_Struct *pPortPinConfig)
{
    PARA_ASSERT(pPortPinConfig != NULL);

    rcu_periph_clock_enable(pPortPinConfig->rcu_periph);

    if (pPortPinConfig->remap_cfg != MCALPORT_CFG_INVALID_REMAP_CFG)
    {
        gpio_pin_remap_config(pPortPinConfig->remap_cfg, ENABLE);
    }

    gpio_init(pPortPinConfig->gpio_periph, pPortPinConfig->mode, pPortPinConfig->speed, pPortPinConfig->pin);

    if (pPortPinConfig->mode == GPIO_MODE_OUT_PP || pPortPinConfig->mode == GPIO_MODE_OUT_OD)
    {
        gpio_bit_write(pPortPinConfig->gpio_periph, pPortPinConfig->pin, pPortPinConfig->sta_init);
    }

    if (pPortPinConfig->afio_cfg != MCALPORT_CFG_INVALID_AFIO_CFG)
    {
        gpio_afio_port_config(pPortPinConfig->afio_cfg, ENABLE);
    }
}

void McalPort_WritePin(McalPortPinChanel_Enum ePinChannel, uint8_t pinVal)
{
    uint8_t val = (pinVal != MCALPORT_PIN_LOW) ? MCALPORT_PIN_HIGH : MCALPORT_PIN_LOW;
    const McalPortPinConfig_Struct *pPortPinConfig = NULL;

    PARA_ASSERT(ePinChannel < eMcalPortPinChanel_Count);

    pPortPinConfig = &c_stPorPinConfigTable[ePinChannel];
    gpio_bit_write(pPortPinConfig->gpio_periph, pPortPinConfig->pin, val);
}

void McalPort_SetPin(McalPortPinChanel_Enum ePinChannel)
{
    McalPort_WritePin(ePinChannel, MCALPORT_PIN_HIGH);
}

void McalPort_ResetPin(McalPortPinChanel_Enum ePinChannel)
{
    McalPort_WritePin(ePinChannel, MCALPORT_PIN_LOW);

}

void MalPort_TogglePin(McalPortPinChanel_Enum ePinChannel)
{
    const McalPortPinConfig_Struct *pPortPinConfig = NULL;
    FlagStatus flag;

    PARA_ASSERT(ePinChannel < eMcalPortPinChanel_Count);

    pPortPinConfig = &c_stPorPinConfigTable[ePinChannel];
    flag = (gpio_output_bit_get(pPortPinConfig->gpio_periph, pPortPinConfig->pin) == SET) ? RESET : SET;
    gpio_bit_write(pPortPinConfig->gpio_periph, pPortPinConfig->pin, flag);
}


uint8_t McalPort_GetPin(McalPortPinChanel_Enum ePinChannel)
{
    PARA_ASSERT(ePinChannel < eMcalPortPinChanel_Count);
    const McalPortPinConfig_Struct *pPortPinConfig = NULL;

    pPortPinConfig = &c_stPorPinConfigTable[ePinChannel];

    return gpio_input_bit_get(pPortPinConfig->gpio_periph, pPortPinConfig->pin);
}

void McalPort_Init(void)
{
    uint8_t index = 0;
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    rcu_periph_clock_enable(RCU_AF);

    for (index = 0; index < eMcalPortPinChanel_Count; index++)
    {
        McalPort_ConfigPin(&c_stPorPinConfigTable[index]);
    }

    
}




























