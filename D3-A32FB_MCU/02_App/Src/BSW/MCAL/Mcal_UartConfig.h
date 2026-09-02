/******************************************************************************
* File Name          : Mcal_UartConfig.h
* Description        : Code for the driver for Uart Communication
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
#ifndef MCAL_UART_CONFIG_H_
#define MCAL_UART_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Uart.h"
#include "gd32e50x_rcu.h"
#include "CycleBuf.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
/* Uart for debug */
#define   MCALUART_CFG_UART5_RECVBUF_SIZE               (200U)
#define   MCALUART_CFG_UART5_SENDBUF_SIZE               (1024 + 2048)

/* Uart for 4G */
#define   MCALUART_CFG_UART0_RECVBUF_SIZE               (3096U)
#define   MCALUART_CFG_UART0_SENDBUF_SIZE               (3096U)

/* Uart for MeterChip */
#define   MCALUART_CFG_UART1_RECVBUF_SIZE               (128U)
#define   MCALUART_CFG_UART1_SENDBUF_SIZE               (64)

/* the definition of txmode */
#define   MCALUART_CFG_TXMODE_INT                       (0U)
#define   MCALUART_CFG_TXMODE_POLL                      (1U)
#define   MCALUART_CFG_TXMODE_DMA                       (2U)
/******************************************************************************
*    Enum Definition
******************************************************************************/



/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t *pTxBuf;
    uint16_t txBufSize;
    uint8_t  txCycleBufID;
    uint8_t  txProfile;
    uint8_t *pRxBuf;
    uint16_t rxBufSize;
    uint8_t rxCycleBufID;
}McalUartBuf_Struct;

typedef struct 
{
    rcu_periph_enum rcu_DMA_periph;
    uint32_t DMA_periph;
    uint16_t DMA_ch;
    dma_parameter_struct DMA_parameter;
    uint8_t circulationEn;
}McalUartDMACfg_Struct;

typedef struct 
{
    IRQn_Type nvic_irq;
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
}McalUartIntCfg_Struct;

typedef struct 
{
    uint8_t txtate;
    uint8_t txMode;
}McalUartCtrl_Struct;


typedef struct 
{
    uint8_t uartEnable;
    uint8_t initFlag;
    rcu_periph_enum rcu_periph;
    uint32_t uart_periph;
    uint32_t baudRate;
    uint32_t paritycfg;
    uint8_t  uartInt_En;
    McalUartIntCfg_Struct uartIntCfg;
    uint8_t  DMATx_En;
    McalUartDMACfg_Struct DMATx_Cfg;
    McalUartBuf_Struct uartBufCtrl;
    McalUartCtrl_Struct uartCtrl;
}McalUartConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern McalUartConfig_Struct g_UartConfigTable[eMcalUartChanel_Count];
#endif /* MCAL_UART_CONFIG_H_ */





















