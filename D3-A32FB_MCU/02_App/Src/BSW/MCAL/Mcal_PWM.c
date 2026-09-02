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
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Mcal_PWMConfig.h"
#include "string.h"


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
static void McalPWM_CfgChannel(McalPWMOC_Struct *pPwmOCCfg);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void McalPWM_CfgChannel(McalPWMOC_Struct *pPwmOCCfg)
{
    PARA_ASSERT(pPwmOCCfg != NULL);

    if (pPwmOCCfg->DMAEn)
    {
        rcu_periph_clock_enable(pPwmOCCfg->DMA_Cfg.rcu_DMA_periph);
        dma_deinit(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch);
        dma_init(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch ,&pPwmOCCfg->DMA_Cfg.DMA_parameter);

        if (pPwmOCCfg->DMA_Cfg.circulationEn)
        {
            dma_circulation_enable(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch);
        }

        if (pPwmOCCfg->DMA_Cfg.DMA_intEn)
        {
            dma_interrupt_enable(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch, pPwmOCCfg->DMA_Cfg.DMA_int.DMA_intSrc);
            nvic_irq_enable(pPwmOCCfg->DMA_Cfg.DMA_int.nvic_irq, pPwmOCCfg->DMA_Cfg.DMA_int.nvic_irq_pre_priority, 
                pPwmOCCfg->DMA_Cfg.DMA_int.nvic_irq_sub_priority);            
        }

        dma_transfer_number_config(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch, pPwmOCCfg->DMA_Cfg.DMA_parameter.number);
        dma_channel_enable(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch);
    }

    rcu_periph_clock_enable(pPwmOCCfg->rcu_timer_periph);
    timer_deinit(pPwmOCCfg->timer_periph);
    timer_init(pPwmOCCfg->timer_periph, &pPwmOCCfg->timer_initpara);
    timer_channel_output_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, &pPwmOCCfg->timer_ocintpara);

    timer_counter_value_config(pPwmOCCfg->timer_periph, pPwmOCCfg->initCounterVal);
    timer_channel_output_pulse_value_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pPwmOCCfg->initOutputPulse);
    timer_channel_output_mode_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pPwmOCCfg->initOutputMode);
    timer_channel_output_shadow_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, TIMER_OC_SHADOW_DISABLE);
    timer_auto_reload_shadow_disable(pPwmOCCfg->timer_periph);
    timer_primary_output_config(pPwmOCCfg->timer_periph, ENABLE);

    if (pPwmOCCfg->timer_intEn)
    {
        timer_interrupt_enable(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_int.timer_int_ch);
        nvic_irq_enable(pPwmOCCfg->timer_int.nvic_irq, pPwmOCCfg->timer_int.nvic_irq_pre_priority,
        pPwmOCCfg->timer_int.nvic_irq_sub_priority);
    }

    if (pPwmOCCfg->DMAEn)
    {
        timer_channel_dma_request_source_select(pPwmOCCfg->timer_periph, TIMER_DMAREQUEST_UPDATEEVENT);
        timer_dma_transfer_config(pPwmOCCfg->timer_periph, pPwmOCCfg->DMA_Cfg.DMA_dataCV, TIMER_DMACFG_DMATC_1TRANSFER);
        timer_dma_enable(pPwmOCCfg->timer_periph, TIMER_DMA_CH2D);
    }

    timer_enable(pPwmOCCfg->timer_periph);
}

void McalPWM_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < eMcalPWMOCChannel_Count; index++)
    {
        McalPWM_CfgChannel((McalPWMOC_Struct *)&c_stTimerOCParaTable[index]);
    }
}

void McalPWM_SetOutputMode(McalPWMOCChannel_Enum ch,  uint8_t mode)
{
    PARA_ASSERT(ch < eMcalPWMOCChannel_Count);
    PARA_ASSERT((mode == MCALPWM_MODE_FORCE_HIGH || mode == MCALPWM_MODE_FORCE_LOW || mode == MCALPWM_MODE_FORCE_PWM));

    const McalPWMOC_Struct *pPwmOCCfg = &c_stTimerOCParaTable[ch];

    timer_channel_output_mode_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, mode);
}

void McalPWM_SetSingleDuty(McalPWMOCChannel_Enum ch,  uint16_t duty)
{
    PARA_ASSERT(ch < eMcalPWMOCChannel_Count);
    const McalPWMOC_Struct *pPwmOCCfg = &c_stTimerOCParaTable[ch];
    uint16_t pulse = 0;

    pulse = duty * pPwmOCCfg->timer_initpara.period / 1000;
    timer_channel_output_pulse_value_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pulse);
}

void McalPWM_SetMultiDuty(McalPWMOCChannel_Enum ch,   uint8_t setType, uint16_t* setVal,  uint16_t dutyCount)
{
    PARA_ASSERT(ch < eMcalPWMOCChannel_Count);
    PARA_ASSERT(dutyCount > 0);
    PARA_ASSERT(setVal != NULL);

    const McalPWMOC_Struct *pPwmOCCfg = &c_stTimerOCParaTable[ch];
    uint8_t index = 0;
    uint16_t pulse = 0;
    uint32_t memoryWidth = 0;
    uint8_t copysize = 0;

    if (pPwmOCCfg->DMAEn == TRUE && dutyCount == pPwmOCCfg->DMA_Cfg.DMA_parameter.number)
    {
        memoryWidth = pPwmOCCfg->DMA_Cfg.DMA_parameter.memory_width;
        copysize = (memoryWidth == DMA_MEMORY_WIDTH_8BIT) ? 1 : (memoryWidth == DMA_MEMORY_WIDTH_16BIT) ? 2 : 4;

        if (setType == MCALPWM_CFG_SET_DUTY)
        {
            for (index = 0; index < dutyCount; index++)
            {
                pulse = setVal[index] * pPwmOCCfg->timer_initpara.period / 1000;
                memcpy((uint8_t *)pPwmOCCfg->DMA_Cfg.DMA_parameter.memory_addr + (index * copysize), &pulse, copysize);
            }
        }
        else
        {
            for (index = 0; index < dutyCount; index++)
            {
                pulse = setVal[index];
                memcpy((uint8_t *)pPwmOCCfg->DMA_Cfg.DMA_parameter.memory_addr + (index * copysize), &pulse, copysize);
            }
        }

        dma_channel_enable(pPwmOCCfg->DMA_Cfg.DMA_periph, pPwmOCCfg->DMA_Cfg.DMA_ch);
        timer_enable(pPwmOCCfg->timer_periph);
    }
}










void McalPWM_Test(void)
{
#if 0
    static uint8_t flag = 0;

    uint16_t timerLedDMAMemoryBuf[MCALPWM_CFG_LED_COUNT][MCALPWM_CFG_LED_POINT] = 
    {
        {
        280, 280, 280, 280, 280, 280, 280, 280,    
        502, 502, 502, 502, 502, 502, 502, 502,
        502, 502, 502, 502, 502, 502, 502, 502,},

        {
        280, 280, 280, 280, 280, 280, 280, 280,    
        502, 502, 502, 502, 502, 502, 502, 502,
        502, 502, 502, 502, 502, 502, 502, 502,},

        {
        280, 280, 280, 280, 280, 280, 280, 280,    
        502, 502, 502, 502, 502, 502, 502, 502,
        502, 502, 502, 502, 502, 502, 502, 502,},
    };

    uint16_t timerLedDMAMemoryBuf1[MCALPWM_CFG_LED_COUNT][MCALPWM_CFG_LED_POINT] = 
    {
        {280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280},

        {280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280},

        {280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280},
    };

    uint16_t *pArray = NULL;

    if (flag)
    {
        pArray = (uint16_t *)timerLedDMAMemoryBuf;
    }
    else
    {
        pArray = (uint16_t *)timerLedDMAMemoryBuf1;
    }
 
    flag = !flag;
    McalPWM_SetMultiDuty(eMcalPWMOCChannel_Led, MCALPWM_CFG_SET_DUTY, pArray, MCALPWM_CFG_LED_DMABUF_LEN);
#endif
}
