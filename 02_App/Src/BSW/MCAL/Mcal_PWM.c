/******************************************************************************
* File Name          : Mcal_PWM.c
* Description        : Code for the driver for General timer PWM output
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
#include "gd32e50x_rcu.h"
#include "gd32e50x_timer.h"
#include "Common.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define MCALPWM_MODE_FORCE_HIGH         TIMER_OC_MODE_HIGH
#define MCALPWM_MODE_FORCE_LOW          TIMER_OC_MODE_HIGH
#define MCALPWM_MODE_FORCE_PWM          TIMER_OC_MODE_PWM0

/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eMcalPWMOCChannel_Led,
    eMcalPWMOCChannel_CP,
    eMcalPWMOCChannel_Relay,
    eMcalPWMOCChannel_Count,
}McalPWMOCChannel_Enum;




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
#if 0
typedef struct 
{







}McalPWMOCDMA_Struct;

typedef struct Mcal_PWM
{

    uint16_t phyChannel;




}McalPWMOCChannelCfg_Struct;


typedef struct 
{
    McalPWMOCChannel_Enum ePWMOCChannel;
    uint32_t timer_periph;
    rcu_periph_enum rcu_periph;
    timer_parameter_struct *pTimer_initpara;
    timer_oc_parameter_struct *pTimer_ocintpara;
    uint8_t DMAEn;
}McalPWMOC_Struct;
#endif
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
#if 0
static void McalPWM_CfgChannel(McalPWMOC_Struct *pPwmOCCfg)
{
    PARA_ASSERT(pPwmOCCfg != NULL)
    PARA_ASSERT(pPwmOCCfg->pTimer_initpara != NULL)

    rcu_periph_clock_enable(pPwmOCCfg->rcu_periph);
    timer_deinit(pPwmOCCfg->timer_periph);
    timer_init(pPwmOCCfg->pTimer_initpara);
}

void McalPWM_SetMode(McalPWMOCChannel_Enum ePWMOCChannel, uint8_t pwmMode)
{




}

#endif

void McalPWM_Init(void)
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER12);

    timer_deinit(TIMER12);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 179;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1000 - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER12,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER12,TIMER_CH_0,&timer_ocintpara);

    /* CH0 configuration in PWM mode0,duty cycle 25% */ 
    timer_channel_output_pulse_value_config(TIMER12,TIMER_CH_0,500);
    timer_channel_output_mode_config(TIMER12,TIMER_CH_0,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER12,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER12);
    /* auto-reload preload enable */
    timer_enable(TIMER12);
}





























