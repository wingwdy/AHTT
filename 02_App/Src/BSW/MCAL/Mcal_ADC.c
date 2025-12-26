/******************************************************************************
* File Name          : Mcal_ADC.c
* Description        : Code for ADC configuration module for hardware
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
#include "Mcal_ADCConfig.h"


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
static void McalADC_CfgChannel(McalADCConfig_Struct *pADCCfg);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void McalADC_CfgChannel(McalADCConfig_Struct *pADCCfg)
{
    uint8_t index = 0;
    uint16_t delay = 2000;
    PARA_ASSERT(pADCCfg != NULL);

    if (pADCCfg->DMAEn)
    {
        rcu_periph_clock_enable(pADCCfg->adc_dmaConfig.rcu_DMA_periph);
        dma_deinit(pADCCfg->adc_dmaConfig.DMA_periph, pADCCfg->adc_dmaConfig.DMA_ch);
        dma_init(pADCCfg->adc_dmaConfig.DMA_periph, pADCCfg->adc_dmaConfig.DMA_ch,
        &pADCCfg->adc_dmaConfig.DMA_parameter);
        
        if (pADCCfg->adc_dmaConfig.circulationEn)
        {
            dma_circulation_enable(pADCCfg->adc_dmaConfig.DMA_periph, pADCCfg->adc_dmaConfig.DMA_ch);
        }

        if (pADCCfg->adc_dmaConfig.DMA_intEn)
        {
            dma_interrupt_enable(pADCCfg->adc_dmaConfig.DMA_periph, pADCCfg->adc_dmaConfig.DMA_ch, pADCCfg->adc_dmaConfig.DMA_intCfg.DMA_intSrc);
            nvic_irq_enable(pADCCfg->adc_dmaConfig.DMA_intCfg.nvic_irq, pADCCfg->adc_dmaConfig.DMA_intCfg.nvic_irq_pre_priority, 
                pADCCfg->adc_dmaConfig.DMA_intCfg.nvic_irq_sub_priority);
        }

        dma_channel_enable(pADCCfg->adc_dmaConfig.DMA_periph, pADCCfg->adc_dmaConfig.DMA_ch);
    }

    rcu_periph_clock_enable(pADCCfg->rcu_adc_periph);
    rcu_adc_clock_config(pADCCfg->adc_psc);
    adc_mode_config(ADC_MODE_FREE);
    /* ADC continus function enable config*/
    adc_special_function_config(pADCCfg->adc_periph, ADC_CONTINUOUS_MODE, pADCCfg->continusModeEn);
    /* ADC scan function enable config*/
    adc_special_function_config(pADCCfg->adc_periph, ADC_SCAN_MODE, pADCCfg->scanModeEn);
    /* ADC data alignment config */
    adc_data_alignment_config(pADCCfg->adc_periph, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(pADCCfg->adc_periph, ADC_REGULAR_CHANNEL, pADCCfg->channelCount);

    for (index = 0; index < pADCCfg->channelCount; index++)
    {
        adc_regular_channel_config(pADCCfg->adc_periph, pADCCfg->pChannelConfigArr[index].rank, \
            pADCCfg->pChannelConfigArr[index].channel, ADC_SAMPLETIME_55POINT5);
    }

    /* ADC trigger config */
    adc_external_trigger_source_config(pADCCfg->adc_periph, ADC_REGULAR_CHANNEL, pADCCfg->external_trigger_source);
    adc_external_trigger_config(pADCCfg->adc_periph, ADC_REGULAR_CHANNEL, pADCCfg->external_trigger_enable);

    if (pADCCfg->adc_intEn)
    {
        adc_interrupt_enable(pADCCfg->adc_periph, pADCCfg->adc_intConfg.adc_int_src);
        nvic_irq_enable(pADCCfg->adc_intConfg.nvic_irq, pADCCfg->adc_intConfg.nvic_irq_pre_priority, 
            pADCCfg->adc_intConfg.nvic_irq_sub_priority);
    }

    if ((pADCCfg->DMAEn))
    {
        adc_dma_mode_enable(pADCCfg->adc_periph);
    }

    adc_enable(pADCCfg->adc_periph);

    while (delay)
    {
        delay--;
    }  
    
    adc_calibration_enable(pADCCfg->adc_periph);
}

void McalADC_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < MCALADC_CFG_INSTANCE_COUNT; index++)
    {
       McalADC_CfgChannel((McalADCConfig_Struct *)&c_stADCConfigTable[index]);
    }
}

void McalADC_GetChannelData(McalADCChanel_Enum ch, uint16_t *pOutBuf, uint8_t count)
{
    PARA_ASSERT(ch < eMcalADCChanel_Count);
    PARA_ASSERT(pOutBuf != NULL);

    uint8_t copyPointCount = 0;
    uint8_t index = 0;

    copyPointCount = (count > MCALADC_CFG_ADC0_SAMPLE_CNT) ? MCALADC_CFG_ADC0_SAMPLE_CNT : count;

    for (index = 0; index < copyPointCount; index++)
    {
        pOutBuf[index] = g_adc0DMABuf[index][ch];
    }
}


void McalADC_Test(void)
{
#if 0
    uint16_t TestData[MCALADC_CFG_ADC0_CHANNEL_CNT][MCALADC_CFG_ADC0_SAMPLE_CNT] = { 0 };

    McalADC_GetChannelData(eMcalADCChanel_CP, TestData[eMcalADCChanel_CP], 8);
    McalADC_GetChannelData(eMcalADCChanel_ShortCut, TestData[eMcalADCChanel_ShortCut], 8);
    McalADC_GetChannelData(eMcalADCChanel_GunNTC, TestData[eMcalADCChanel_GunNTC], 8);
    McalADC_GetChannelData(eMcalADCChanel_EnvNtc, TestData[eMcalADCChanel_EnvNtc], 8);
    McalADC_GetChannelData(eMcalADCChannel_PE, TestData[eMcalADCChannel_PE], 5);
#endif
}





















