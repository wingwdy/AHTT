/******************************************************************************
* File Name          : Mcal_PWMConfig.h
* Description        : Code for the driver for General timer PWM output
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
#ifndef MCAL_PWM_CONFIG_H_
#define MCAL_PWM_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_PWM.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALPWM_CFG_INVALID_SRC_OC_TRIGO              (0xFFU)     

/* led控制, DMABuf长度 当前3个灯 */
#define MCALPWM_CFG_LED_COUNT             3
#define MCALPWM_CFG_LED_POINT            24
#define MCALPWM_CFG_LED_DMABUF_LEN       (MCALPWM_CFG_LED_COUNT * MCALPWM_CFG_LED_POINT)     

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    IRQn_Type nvic_irq;
    uint8_t timer_int_ch;
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
}McalPWMTimerIntCfg_Struct;

typedef struct 
{
    IRQn_Type nvic_irq;
    uint32_t DMA_intSrc;
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
}McalPWMDMAIntCfg_Struct;

typedef struct 
{
    rcu_periph_enum rcu_DMA_periph;
    uint32_t DMA_periph;
    uint16_t DMA_ch;
    uint32_t DMA_dataCV;
    dma_parameter_struct DMA_parameter;
    uint8_t circulationEn;
    uint8_t DMA_intEn;
    McalPWMDMAIntCfg_Struct DMA_int;
}McalPWMTimerDMACfg_Struct;

typedef struct 
{
    rcu_periph_enum rcu_timer_periph;
    uint32_t timer_periph;
    uint16_t timer_ch;
    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocintpara;
    uint16_t initOutputMode;
    uint16_t initOutputPulse;
    uint16_t initOutputSrcTrigo;
    uint16_t initCounterVal;
    uint8_t timer_intEn;
    McalPWMTimerIntCfg_Struct timer_int;
    uint8_t DMAEn;
    McalPWMTimerDMACfg_Struct DMA_Cfg;
}McalPWMOC_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern const McalPWMOC_Struct c_TimerOCParaTable[eMcalPWMOCChannel_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/



#endif /* MCAL_PWM_CONFIG_H_ */




















