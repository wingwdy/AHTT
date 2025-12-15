/******************************************************************************
* File Name          : Mcal_UartConfig.c
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

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_UartConfig.h"

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


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t g_Uart5RecvBuf[MCALUART_CFG_UART5_RECVBUF_SIZE] = { 0 };
static uint8_t g_Uart5SendBuf[MCALUART_CFG_UART5_SENDBUF_SIZE] = { 0 };

static uint8_t g_Uart0RecvBuf[MCALUART_CFG_UART0_RECVBUF_SIZE] = { 0 };
static uint8_t g_Uart0SendBuf[MCALUART_CFG_UART0_SENDBUF_SIZE] = { 0 };

static uint8_t g_Uart1RecvBuf[MCALUART_CFG_UART1_RECVBUF_SIZE] = { 0 };
static uint8_t g_Uart1SendBuf[MCALUART_CFG_UART1_SENDBUF_SIZE] = { 0 };

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
McalUartConfig_Struct g_UartConfigTable[eMcalUartChanel_Count] = 
{
    [eMcalUartChanel_Debug] = 
    {
        .uartEnable = TRUE,
        .initFlag = FALSE,
        .rcu_periph = RCU_USART5,
        .uart_periph = USART5,
        .baudRate = 115200,
        .paritycfg = USART_PM_NONE,
        .uartInt_En = TRUE,
        .uartIntCfg = 
        {
            .nvic_irq = USART5_IRQn,
            .nvic_irq_pre_priority = 0,
            .nvic_irq_sub_priority = 1,
        },
        .DMATx_En = FALSE,
        .uartBufCtrl = 
        {
            .pTxBuf = g_Uart5SendBuf,
            .txBufSize = MCALUART_CFG_UART5_SENDBUF_SIZE,
            .txCycleBufID = CYCLEBUF_INVALID_ID,
            .txProfile = CYCLEBUF_PROFILE_CIRCLE,
            .pRxBuf = g_Uart5RecvBuf,
            .rxBufSize = MCALUART_CFG_UART5_RECVBUF_SIZE,
            .rxCycleBufID = CYCLEBUF_INVALID_ID,
        },
        .uartCtrl = 
        {
            .txtate = MCALUART_TXSTATE_IDLE,
            .txMode = MCALUART_CFG_TXMODE_INT,
        }
    },

    [eMcalUartChanel_4G] = 
    {
        .uartEnable = TRUE,
        .initFlag = FALSE,
        .rcu_periph = RCU_USART0,
        .uart_periph = USART0,
        .baudRate = 115200,
        .paritycfg = USART_PM_NONE,
        .uartInt_En = TRUE,
        .uartIntCfg = 
        {
            .nvic_irq = USART0_IRQn,
            .nvic_irq_pre_priority = 0,
            .nvic_irq_sub_priority = 1,
        },
        .DMATx_En = FALSE,
        .uartBufCtrl = 
        {
            .pTxBuf = g_Uart0SendBuf,
            .txBufSize = MCALUART_CFG_UART0_SENDBUF_SIZE,
            .txCycleBufID = CYCLEBUF_INVALID_ID,
            .txProfile = CYCLEBUF_PROFILE_CIRCLE,
            .pRxBuf = g_Uart0RecvBuf,
            .rxBufSize = MCALUART_CFG_UART0_RECVBUF_SIZE,
            .rxCycleBufID = CYCLEBUF_INVALID_ID,
        },
        .uartCtrl = 
        {
            .txtate = MCALUART_TXSTATE_IDLE,
            .txMode = MCALUART_CFG_TXMODE_INT,
        }
    },

    [eMcalUartChanel_MeterChip] = 
    {
        .uartEnable = TRUE,
        .initFlag = FALSE,
        .rcu_periph = RCU_USART1,
        .uart_periph = USART1,
        .baudRate = 4800,
        .paritycfg = USART_PM_NONE,
        .uartInt_En = TRUE,
        .uartIntCfg = 
        {
            .nvic_irq = USART1_IRQn,
            .nvic_irq_pre_priority = 0,
            .nvic_irq_sub_priority = 1,
        },
        .DMATx_En = FALSE,
        .uartBufCtrl = 
        {
            .pTxBuf = g_Uart1SendBuf,
            .txBufSize = MCALUART_CFG_UART1_SENDBUF_SIZE,
            .txCycleBufID = CYCLEBUF_INVALID_ID,
            .txProfile = CYCLEBUF_PROFILE_CIRCLE,
            .pRxBuf = g_Uart1RecvBuf,
            .rxBufSize = MCALUART_CFG_UART1_RECVBUF_SIZE,
            .rxCycleBufID = CYCLEBUF_INVALID_ID,
        },
        .uartCtrl = 
        {
            .txtate = MCALUART_TXSTATE_IDLE,
            .txMode = MCALUART_CFG_TXMODE_INT,
        }
    },
};

static void USART_IRQHandler(McalUartChanel_Enum eCh)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    uint8_t byte = 0;
    uint32_t clear;
    uint16_t remainLen;
    PARA_ASSERT(eCh < eMcalUartChanel_Count);

    if (usart_interrupt_flag_get(pUartCfg->uart_periph, USART_INT_FLAG_RBNE))
    {
        byte = usart_data_receive(pUartCfg->uart_periph);
        CycleBuf_WriteDataIsr(pUartCfg->uartBufCtrl.rxCycleBufID, &byte, 1);
    }

	if (usart_flag_get(pUartCfg->uart_periph, USART_FLAG_ORERR))
	{
		clear = USART_STAT0(pUartCfg->uart_periph);
		usart_data_receive(pUartCfg->uart_periph);
	}

    if (usart_flag_get(pUartCfg->uart_periph, USART_FLAG_NERR))
	{
		clear = USART_STAT0(pUartCfg->uart_periph);
		usart_data_receive(pUartCfg->uart_periph);
	}

	if (usart_flag_get(pUartCfg->uart_periph, USART_FLAG_FERR))
	{
		clear = USART_STAT0(pUartCfg->uart_periph);
		usart_data_receive(pUartCfg->uart_periph);
	}

	if (usart_flag_get(pUartCfg->uart_periph, USART_FLAG_PERR))
	{
		clear = USART_STAT0(pUartCfg->uart_periph);
		usart_data_receive(pUartCfg->uart_periph);
	}

    if (RESET != usart_interrupt_flag_get(pUartCfg->uart_periph, USART_INT_FLAG_IDLE))
	{
		clear = USART_STAT0(pUartCfg->uart_periph);
		clear = (uint16_t)(GET_BITS(USART_DATA(pUartCfg->uart_periph), 0U, 8U));
    }

    if (usart_interrupt_flag_get(pUartCfg->uart_periph, USART_INT_FLAG_TBE))
    {
        usart_interrupt_flag_clear(pUartCfg->uart_periph, USART_INT_FLAG_TBE);

        if (eGlobalRet_OK != CycleBuf_ReadDataIsr(pUartCfg->uartBufCtrl.txCycleBufID, &byte, 1))
        {
			usart_interrupt_disable(pUartCfg->uart_periph, USART_INT_TBE);
            usart_interrupt_enable(pUartCfg->uart_periph, USART_INT_TC);            
        }
        else
        {
            usart_data_transmit(pUartCfg->uart_periph, byte);
        }
    }
    else if (usart_interrupt_flag_get(pUartCfg->uart_periph, USART_INT_FLAG_TC))
    {
        usart_interrupt_flag_clear(pUartCfg->uart_periph, USART_INT_FLAG_TC);

        if (eGlobalRet_OK != CycleBuf_ReadDataIsr(pUartCfg->uartBufCtrl.txCycleBufID, &byte, 1))
        {
            usart_interrupt_enable(pUartCfg->uart_periph, USART_INT_TC);
            pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_IDLE;
        }
        else
        {
            usart_data_transmit(pUartCfg->uart_periph, byte);
        }
    }
}

void USART5_IRQHandler(void)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eMcalUartChanel_Debug];
    uint8_t byte = 0;
    uint32_t clear;
    uint16_t remainLen;

    if (usart5_interrupt_flag_get(pUartCfg->uart_periph, USART5_INT_FLAG_RBNE))
    {
        byte = usart_data_receive(pUartCfg->uart_periph);
        CycleBuf_WriteDataIsr(pUartCfg->uartBufCtrl.rxCycleBufID, &byte, 1);
    }

	if (usart5_flag_get(pUartCfg->uart_periph, USART5_FLAG_ORERR))
	{
        usart5_flag_clear(pUartCfg->uart_periph, USART5_FLAG_ORERR);
	}

    if (usart5_flag_get(pUartCfg->uart_periph, USART5_FLAG_NERR))
	{
         usart5_flag_clear(pUartCfg->uart_periph, USART5_FLAG_NERR);
	}

	if (usart5_flag_get(pUartCfg->uart_periph, USART5_FLAG_FERR))
	{
        usart5_flag_clear(pUartCfg->uart_periph, USART5_FLAG_FERR);
	}

	if (usart5_flag_get(pUartCfg->uart_periph, USART5_FLAG_PERR))
	{
        usart5_flag_clear(pUartCfg->uart_periph, USART5_FLAG_PERR);
	}

    if (RESET != usart5_interrupt_flag_get(pUartCfg->uart_periph, USART5_INT_FLAG_IDLE))
	{
        usart5_interrupt_flag_clear(pUartCfg->uart_periph, USART5_INT_FLAG_IDLE);
    }

    if (usart5_interrupt_flag_get(pUartCfg->uart_periph, USART5_INT_FLAG_TBE))
    {
        usart5_interrupt_flag_clear(pUartCfg->uart_periph, USART5_INT_FLAG_TBE);
        
        if (eGlobalRet_OK != CycleBuf_ReadDataIsr(pUartCfg->uartBufCtrl.txCycleBufID, &byte, 1))
        {
			usart5_interrupt_disable(pUartCfg->uart_periph, USART5_INT_TBE);
            usart5_interrupt_enable(pUartCfg->uart_periph, USART5_INT_TC);    
            pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_IDLE;
        }
        else
        {
            usart_data_transmit(pUartCfg->uart_periph, byte);
        }
    }
    else if (usart5_interrupt_flag_get(pUartCfg->uart_periph, USART5_INT_FLAG_TC))
    {
        usart5_interrupt_flag_clear(pUartCfg->uart_periph, USART5_INT_FLAG_TC);

        if (eGlobalRet_OK != CycleBuf_ReadDataIsr(pUartCfg->uartBufCtrl.txCycleBufID, &byte, 1))
        {
            usart5_interrupt_enable(pUartCfg->uart_periph, USART5_INT_TC);
            pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_IDLE;
        }
        else
        {
            usart_data_transmit(pUartCfg->uart_periph, byte);
        }
    }
}

void USART0_IRQHandler(void)
{
    USART_IRQHandler(eMcalUartChanel_4G);
}

void USART1_IRQHandler(void)
{
   USART_IRQHandler(eMcalUartChanel_MeterChip);
}

















