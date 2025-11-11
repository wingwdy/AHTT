/******************************************************************************
* File Name          : Mcal_Mcu.h
* Description        : Code for the driver for Mcu
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
******************************************************************************/
#ifndef MCAL_MCU_H_
#define MCAL_MCU_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "stdint.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eMcalMcuResetSource_Unknown = 0,   /* 未知 */
    eMcalMcuResetSource_ExternalPin,   /* 外部引脚复位 */
    eMcalMcuResetSource_PowerOn,       /* 上电/掉电复位 */
    eMcalMcuResetSource_Software,      /* 软件复位 */
    eMcalMcuResetSource_FWDGT,         /* 独立看门狗复位 */
    eMcalMcuResetSource_WWDGT,         /* 窗口看门狗复位 */
    eMcalMcuResetSource_Lowpower,      /* 低功耗复位 */
}McalMcuResetSource_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void McalMCU_SystickInit(void);
void McalMcu_SystemReset(void);
McalMcuResetSource_Enum McalMcu_GetResetSource(void);
void McalMcu_ClearResetFlags(void);
uint32_t Mcal_GetSystick(void);
#endif /* MCAL_MCU_H_ */





















