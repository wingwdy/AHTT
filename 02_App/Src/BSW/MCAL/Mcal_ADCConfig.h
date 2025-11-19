/******************************************************************************
* File Name          : Mcal_ADCConfig.h
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
#ifndef MCAL_ADC_CONFIG_H_
#define MCAL_ADC_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "gd32e50x_adc.h"
#include "Mcal_ADC.h"
#include "gd32e50x_rcu.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALADC_CFG_INSTANCE0_ADC0            0
#define MCALADC_CFG_INSTANCE_COUNT            1

#define MCALADC_CFG_CHANNEL_MAX_COUNT         5

#define MCALADC_CFG_ADC0_SAMPLE_CNT           MCALADC_ADC0_SAMPLE_CNT

#define MCALADC_CFG_ADC0_CHANNEL_CNT          eMcalADCChanel_Count                

#define MCALADC_CFG_ADC0_DMABUF_LEN           (MCALADC_CFG_ADC0_SAMPLE_CNT * MCALADC_CFG_ADC0_CHANNEL_CNT)       

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t rank;
    uint8_t channel; 
}McalADCChannelConfig_Struct;

typedef struct 
{
    IRQn_Type nvic_irq;
    uint8_t adc_int_src;
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
}McalADCIntCfg_Struct;

typedef struct 
{
    IRQn_Type nvic_irq;
    uint32_t DMA_intSrc;
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
}McalADCDMAIntCfg_Struct;

typedef struct 
{
    rcu_periph_enum rcu_DMA_periph;
    uint32_t DMA_periph;
    uint16_t DMA_ch;
    dma_parameter_struct DMA_parameter;
    uint8_t circulationEn;
    uint8_t DMA_intEn;
    McalADCDMAIntCfg_Struct DMA_intCfg;
}McalADCDMACfg_Struct;

typedef struct 
{
    rcu_periph_enum rcu_adc_periph;
    uint32_t adc_periph;
    uint32_t adc_psc;
    uint8_t continusModeEn;
    uint8_t scanModeEn;
    uint8_t channelCount;
    McalADCChannelConfig_Struct *pChannelConfigArr;
    uint32_t external_trigger_source;
    uint32_t external_trigger_enable;
    uint8_t adc_intEn;
    McalADCIntCfg_Struct adc_intConfg;
    uint8_t DMAEn;
    McalADCDMACfg_Struct adc_dmaConfig;
}McalADCConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern uint16_t g_adc0DMABuf[MCALADC_CFG_ADC0_SAMPLE_CNT][MCALADC_CFG_ADC0_CHANNEL_CNT];
extern const McalADCConfig_Struct c_mcalADCConfigTable[MCALADC_CFG_INSTANCE_COUNT];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* MCAL_ADC_CONFIG_H_ */





















