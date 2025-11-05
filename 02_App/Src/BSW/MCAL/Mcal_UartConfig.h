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
*2025/10/10      V1.0.0      Chenls    初版创建
*
******************************************************************************/
#ifndef MCAL_UART_CONFIG_H_
#define MCAL_UART_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Uart.h"
#include "gd32e50x_rcu.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define   MCALUART_CFG_UART5_RECVBUF_SIZE               (320U)
#define   MCALUART_CFG_UART5_SENDBUF_SIZE               (1U)

/******************************************************************************
*    Enum Definition
******************************************************************************/



/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t *pTxBuf;
    uint8_t *pRxBuf;
    uint16_t txBufSize;
    uint16_t rxBufSize;
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
    rcu_periph_enum rcu_periph;
    uint32_t uart_periph;
    uint32_t baudRate;
    uint32_t paritycfg;
    uint8_t  DMARx_En;
    McalUartDMACfg_Struct DMARx_Cfg;
}McalUartConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* MCAL_UART_CONFIG_H_ */





















