/******************************************************************************
* File Name          : Mcal_PWMConfig.c
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

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_PWMConfig.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static uint8_t g_TimerLedDMAMemoryBuf[MCALPWM_CFG_LED_COUNT][MCALPWM_CFG_LED_POINT];

const McalPWMOC_Struct c_TimerOCParaTable[eMcalPWMOCChannel_Count] =
{
    [eMcalPWMOCChannel_Led] =
    {
        .rcu_timer_periph = RCU_TIMER7,
        .timer_periph = TIMER7,
        .timer_ch = TIMER_CH_2,
        .timer_initpara = {
            .prescaler = 1 - 1,
            .alignedmode = TIMER_COUNTER_EDGE,
            .counterdirection = TIMER_COUNTER_UP,
            .period = 225 - 1,
            .clockdivision = TIMER_CKDIV_DIV1,
            .repetitioncounter = 0,
        },

        .timer_ocintpara = {
            .outputstate = TIMER_CCX_ENABLE,
            .outputnstate = TIMER_CCXN_DISABLE,
            .ocpolarity = TIMER_OC_POLARITY_HIGH,
            .ocnpolarity = TIMER_OCN_POLARITY_HIGH,
            .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
            .ocnidlestate = TIMER_OCN_IDLE_STATE_LOW,
        },

        .initOutputMode = TIMER_OC_MODE_PWM0,
        .initOutputPulse = 0,
        .initCounterVal = 0,

        .timer_intEn = FALSE,
        .DMAEn = TRUE,
        .DMA_Cfg = {
            .rcu_DMA_periph = RCU_DMA1,
            .DMA_periph = DMA1,
            .DMA_ch = DMA_CH0,    
            .DMA_dataCV = TIMER_DMACFG_DMATA_CH2CV,       
            .DMA_parameter = {
                .direction = DMA_MEMORY_TO_PERIPHERAL,
                .periph_addr = (uint32_t)&TIMER_CH2CV(TIMER7),
                .periph_inc = DMA_PERIPH_INCREASE_DISABLE,
                .periph_width = DMA_PERIPHERAL_WIDTH_16BIT,
                .memory_addr = (uint32_t)g_TimerLedDMAMemoryBuf,
                .memory_inc = DMA_MEMORY_INCREASE_ENABLE,   
                .memory_width = DMA_MEMORY_WIDTH_8BIT,
                .number = MCALPWM_CFG_LED_DMABUF_LEN,        
                .priority = DMA_PRIORITY_ULTRA_HIGH,   
            },
            .circulationEn = FALSE,
            .DMA_intEn = TRUE,
            .DMA_int = {
                .nvic_irq = DMA1_Channel0_IRQn,
                .DMA_intSrc = DMA_INT_FTF,
                .nvic_irq_pre_priority = 2,
                .nvic_irq_sub_priority = 1
            },
        },
    },

    [eMcalPWMOCChannel_CP] =
    {
        .rcu_timer_periph = RCU_TIMER12,
        .timer_periph = TIMER12,
        .timer_ch = TIMER_CH_0,
        .timer_initpara = {
            .prescaler = 180 - 1,
            .alignedmode = TIMER_COUNTER_EDGE,
            .counterdirection = TIMER_COUNTER_UP,
            .period = 1000 - 1,
            .clockdivision = TIMER_CKDIV_DIV1,
            .repetitioncounter = 0,
        },
        .timer_ocintpara = {
            .outputstate = TIMER_CCX_ENABLE,
            .outputnstate = TIMER_CCXN_DISABLE,
            .ocpolarity = TIMER_OC_POLARITY_HIGH,
            .ocnpolarity = TIMER_OCN_POLARITY_HIGH,
            .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
            .ocnidlestate = TIMER_OCN_IDLE_STATE_LOW,
        },

        .initOutputMode = TIMER_OC_MODE_HIGH,
        .initOutputPulse = 0,
        .initCounterVal = 0,

        .timer_intEn = FALSE,
        .DMAEn = FALSE,
    },

    [eMcalPWMOCChannel_CPDetect] =
    {
        .rcu_timer_periph = RCU_TIMER1,
        .timer_periph = TIMER1,
        .timer_ch = TIMER_CH_1,
        .timer_initpara = {
            .prescaler = 180 - 1,
            .alignedmode = TIMER_COUNTER_EDGE,
            .counterdirection = TIMER_COUNTER_UP,
            .period = 1000 - 1,
            .clockdivision = TIMER_CKDIV_DIV1,
            .repetitioncounter = 0,
        },
        .timer_ocintpara = {
            .outputstate = TIMER_CCX_ENABLE,
            .outputnstate = TIMER_CCXN_DISABLE,
            .ocpolarity = TIMER_OC_POLARITY_HIGH,
            .ocnpolarity = TIMER_OCN_POLARITY_HIGH,
            .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
            .ocnidlestate = TIMER_OCN_IDLE_STATE_LOW,
        },

        .initOutputMode = TIMER_OC_MODE_PWM0,
        .initOutputPulse = 250,
        .initCounterVal = 0,

        .timer_intEn = TRUE,
        .timer_int = 
        {
            .nvic_irq = TIMER1_IRQn,
            .timer_int_ch = TIMER_INT_CH1,
            .nvic_irq_pre_priority = 2,
            .nvic_irq_sub_priority = 0,
        },
        .DMAEn = FALSE,
    },

#if 1
    [eMcalPWMOCChannel_Relay] =
    {
        .rcu_timer_periph = RCU_TIMER2,
        .timer_periph = TIMER2,
        .timer_ch = TIMER_CH_2,
        .timer_initpara = {
            .prescaler = 18 - 1,
            .alignedmode = TIMER_COUNTER_EDGE,
            .counterdirection = TIMER_COUNTER_UP,
            .period = 500 - 1,
            .clockdivision = TIMER_CKDIV_DIV1,
            .repetitioncounter = 0,
        },
        .timer_ocintpara = {
            .outputstate = TIMER_CCX_ENABLE,
            .outputnstate = TIMER_CCXN_DISABLE,
            .ocpolarity = TIMER_OC_POLARITY_HIGH,
            .ocnpolarity = TIMER_OCN_POLARITY_HIGH,
            .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
            .ocnidlestate = TIMER_OCN_IDLE_STATE_LOW,
        },

        .initOutputMode = TIMER_OC_MODE_LOW,
        .initOutputPulse = 0,
        .initCounterVal = 0,

        .timer_intEn = FALSE,
        .DMAEn = FALSE,
    },
#endif
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void TIMER1_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_CH1) == SET) 
    {
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_CH1);
        adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    }
}

void DMA1_Channel0_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_INT_FLAG_FTF))
    {
        timer_disable(TIMER7);
        dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);
        timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, 0);
        dma_channel_disable(DMA1, DMA_CH0);
        dma_transfer_number_config(DMA1, DMA_CH0, MCALPWM_CFG_LED_DMABUF_LEN);
    }
}



















