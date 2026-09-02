/******************************************************************************
* File Name          : Mcal_ADCConfig.c
* Description        : Code for ADC configuration module for hardware
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

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_ADCConfig.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
uint16_t g_adc0DMABuf[MCALADC_CFG_ADC0_SAMPLE_CNT][MCALADC_CFG_ADC0_CHANNEL_CNT] = {0};

const McalADCChannelConfig_Struct c_stADC0ChannelConfigTable[] = 
{
    [eMcalADCChanel_CP] =           { 0,         ADC_CHANNEL_5  },
    [eMcalADCChanel_ShortCut] =     { 1,         ADC_CHANNEL_12 },
    [eMcalADCChanel_GunNTC] =       { 2,         ADC_CHANNEL_7  },
    [eMcalADCChanel_EnvNtc] =       { 3,         ADC_CHANNEL_0  },
    [eMcalADCChannel_PE] =          { 4,         ADC_CHANNEL_15 },
};

const McalADCConfig_Struct c_stADCConfigTable[MCALADC_CFG_INSTANCE_COUNT] = 
{
    [MCALADC_CFG_INSTANCE0_ADC0] = 
    {
        .rcu_adc_periph = RCU_ADC0,
        .adc_periph = ADC0,
        .adc_psc = RCU_CKADC_CKAPB2_DIV6,
        .continusModeEn = DISABLE,
        .scanModeEn = ENABLE,
        .channelCount = eMcalADCChanel_Count,
        .pChannelConfigArr = (McalADCChannelConfig_Struct *)c_stADC0ChannelConfigTable,
        .external_trigger_source = ADC0_1_2_EXTTRIG_REGULAR_NONE,
        .external_trigger_enable = ENABLE,
        .adc_intEn = FALSE,
        .DMAEn = TRUE,
        .adc_dmaConfig = 
        {
            .rcu_DMA_periph = RCU_DMA0,
            .DMA_periph = DMA0,
            .DMA_ch = DMA_CH0,
            .DMA_parameter = 
            {
                .direction = DMA_PERIPHERAL_TO_MEMORY,
                .periph_addr = (uint32_t)(&ADC_RDATA(ADC0)),
                .periph_inc = DMA_PERIPH_INCREASE_DISABLE,
                .periph_width = DMA_PERIPHERAL_WIDTH_16BIT,
                .memory_addr = (uint32_t)g_adc0DMABuf,
                .memory_inc = DMA_MEMORY_INCREASE_ENABLE,   
                .memory_width = DMA_MEMORY_WIDTH_16BIT,
                .number = MCALADC_CFG_ADC0_DMABUF_LEN,        
                .priority = DMA_PRIORITY_MEDIUM,   
            },
            .circulationEn = TRUE,
            .DMA_intEn = FALSE,
            .DMA_intCfg = 
            {
                .nvic_irq = DMA0_Channel0_IRQn,
                .DMA_intSrc = DMA_INT_FTF,
                .nvic_irq_pre_priority = 2,
                .nvic_irq_sub_priority = 0
            },
        }
    },
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/





/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void DMA0_Channel0_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
    }
}




















