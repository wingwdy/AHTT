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
#include "Mcal_PWMConfig.h"


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

    if (pPwmOCCfg->initOutputSrcTrigo != MCALPWM_CFG_INVALID_SRC_OC_TRIGO)
    {
        timer_master_output_trigger_source_select(pPwmOCCfg->timer_periph, pPwmOCCfg->initOutputSrcTrigo);
        timer_primary_output_config(pPwmOCCfg->timer_periph, ENABLE);
    }

    timer_counter_value_config(pPwmOCCfg->timer_periph, 0);
    timer_channel_output_pulse_value_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pPwmOCCfg->initOutputPulse);
    timer_channel_output_mode_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pPwmOCCfg->initOutputMode);
    timer_channel_output_shadow_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, TIMER_OC_SHADOW_DISABLE);
    timer_auto_reload_shadow_enable(pPwmOCCfg->timer_periph);

    if (pPwmOCCfg->timer_intEn)
    {
        timer_interrupt_enable(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_int.timer_int_ch);
        nvic_irq_enable(pPwmOCCfg->timer_int.nvic_irq, pPwmOCCfg->timer_int.nvic_irq_pre_priority,
        pPwmOCCfg->timer_int.nvic_irq_sub_priority);
    }

    if (pPwmOCCfg->DMAEn)
    {
        timer_channel_dma_request_source_select(pPwmOCCfg->timer_periph, TIMER_DMAREQUEST_UPDATEEVENT);
        timer_dma_transfer_config(pPwmOCCfg->timer_periph, pPwmOCCfg->DMA_Cfg.DMA_dataCV,TIMER_DMACFG_DMATC_1TRANSFER);
        timer_dma_enable(pPwmOCCfg->timer_periph, TIMER_DMA_UPD);
    }

    timer_enable(pPwmOCCfg->timer_periph);
}

void McalPWM_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < eMcalPWMOCChannel_Count; index++)
    {
        McalPWM_CfgChannel((McalPWMOC_Struct *)&c_TimerOCParaTable[index]);
    }
}

void McalPWM_CtrlSetMode(McalPWMOCChannel_Enum ch,  uint8_t mode)
{
    PARA_ASSERT(ch < eMcalPWMOCChannel_Count);
    PARA_ASSERT(mode == MCALPWM_MODE_FORCE_HIGH || 
        mode == MCALPWM_MODE_FORCE_LOW || 
        mode == MCALPWM_MODE_FORCE_PWM);

    McalPWMOC_Struct *pPwmOCCfg = &c_TimerOCParaTable[ch];
    timer_channel_output_mode_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, mode);
}


void McalPWM_CtrlSetsSingleChannelDuty(McalPWMOCChannel_Enum ch,  uint16_t duty)
{
    PARA_ASSERT(ch < eMcalPWMOCChannel_Count);

    McalPWMOC_Struct *pPwmOCCfg = &c_TimerOCParaTable[ch];
    uint16_t pulse = 0;
    pulse = duty * pPwmOCCfg->timer_initpara.period / 1000;
    timer_channel_output_pulse_value_config(pPwmOCCfg->timer_periph, pPwmOCCfg->timer_ch, pulse);
}

