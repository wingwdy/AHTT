/******************************************************************************
* File Name          : template_Config.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef CDD_4GM_CONFIG_H_
#define CDD_4GM_CONFIG_H_


/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Port.h"
#include "Mcal_Uart.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/

#define CDD_4GM_CFG_PwrOff()                     McalPort_SetPin(eMcalPortPinChanel_PC15_4GPwrEn)
#define CDD_4GM_CFG_PwrOn()                      McalPort_ResetPin(eMcalPortPinChanel_PC15_4GPwrEn)
#define CDD_4GM_CFG_PwrKeyOff()                  McalPort_SetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn)
#define CDD_4GM_CFG_PwrKeyOn()                   McalPort_ResetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn)

#define CDD_4GM_CFG_WriteData(data, len)         McalUart_WriteData(eMcalUartChanel_4G, data, len)

#define CDD_4GM_CFG_POWEROFF_HOLD_TIME           500           
#define CDD_4GM_CFG_POWERON_HOLD_TIME            1000
#define CDD_4GM_CFG_POWERKEY_OFF_HOLD_TIME       1500
#define CDD_4GM_CFG_POWERKEY_ON_HOLD_TIME        3000


/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
#endif /* CDD_DRV_4GM_CONFIG_H_ */























