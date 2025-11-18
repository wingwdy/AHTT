/******************************************************************************
* File Name          : Mcal_PortConfig.h
* Description        : Code for Pin-level configuration module for hardware
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef MCAL_PORT_CONFIG_H_
#define MCAL_PORT_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Port.h"
#include "gd32e50x_rcu.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALPORT_CFG_INVALID_REMAP_CFG       (0xFFFFFFFF)

#define MCALPORT_CFG_INVALID_AFIO_CFG        (0xFFFFFFFF)

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
	McalPortPinChanel_Enum  ePinChannel;
    rcu_periph_enum     rcu_periph;
    uint32_t            gpio_periph;
	uint32_t            mode;
	uint32_t            speed;
    uint32_t            pin;
    bit_status          sta_init;
    uint32_t            remap_cfg;
    uint32_t            afio_cfg;
}McalPortPinConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const McalPortPinConfig_Struct c_stPorPinConfigTable[eMcalPortPinChanel_Count];

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* MCAL_PORT_CONFIG_H_ */





















