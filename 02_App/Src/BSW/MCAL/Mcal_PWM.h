/******************************************************************************
* File Name          : Mcal_PWM.h
* Description        : Code for the driver for General timer PWM output
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
#ifndef MCAL_PWM_H_
#define MCAL_PWM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "gd32e50x_rcu.h"
#include "gd32e50x_timer.h"
#include "gd32e50x_dma.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALPWM_MODE_FORCE_HIGH         TIMER_OC_MODE_HIGH
#define MCALPWM_MODE_FORCE_LOW          TIMER_OC_MODE_LOW
#define MCALPWM_MODE_FORCE_PWM          TIMER_OC_MODE_PWM0


#define MCALPWM_CFG_SET_DUTY            0
#define MCALPWM_CFG_SET_REG             1

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eMcalPWMOCChannel_Led,          /* LED 输出控制 */
    eMcalPWMOCChannel_CP,           /* CP PWM输出 */
    eMcalPWMOCChannel_CPDetect,     /* CP 触发检测 */
    eMcalPWMOCChannel_Relay,        /* 继电器输出控制 */
    eMcalPWMOCChannel_Count,        
}McalPWMOCChannel_Enum;






/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void McalPWM_Init(void);
void McalPWM_SetOutputMode(McalPWMOCChannel_Enum ch,  uint8_t mode);
void McalPWM_SetSingleDuty(McalPWMOCChannel_Enum ch,  uint16_t duty);
void McalPWM_SetMultiDuty(McalPWMOCChannel_Enum ch,   uint8_t setType, uint16_t* duty,  uint16_t dutyCount);
void McalPWM_Test(void);
#endif /* MCAL_PWM_H_ */




















