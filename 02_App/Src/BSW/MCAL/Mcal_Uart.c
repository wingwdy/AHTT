/******************************************************************************
* File Name          : Mcal_Uart.c
* Description        : Code for the driver for Uart Communication
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
#include "Mcal_UartConfig.h"
#include "stdio.h"



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
static void McalUart_ConfigChannel(McalUartConfig_Struct *pUartConfig);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void McalUart_ConfigChannel(McalUartConfig_Struct *pUartConfig)
{
    GlobalRet_Enum eRet1 = eGlobalRet_OK, eRet2 = eGlobalRet_OK;
    PARA_ASSERT(pUartConfig != NULL);

    if (pUartConfig->uartEnable == TRUE)
    {
        rcu_periph_clock_enable(pUartConfig->rcu_periph);
        usart_deinit(pUartConfig->uart_periph);

        usart_baudrate_set(pUartConfig->uart_periph, pUartConfig->baudRate);
        usart_word_length_set(pUartConfig->uart_periph, USART_WL_8BIT);
        usart_stop_bit_set(pUartConfig->uart_periph, USART_STB_1BIT);
        usart_parity_config(pUartConfig->uart_periph, USART_PM_NONE);
        usart_receive_config(pUartConfig->uart_periph, USART_RECEIVE_ENABLE);
        usart_transmit_config(pUartConfig->uart_periph, USART_TRANSMIT_ENABLE);

        if (pUartConfig->uartInt_En == TRUE)
          {
            nvic_irq_enable(pUartConfig->uartIntCfg.nvic_irq, 
                pUartConfig->uartIntCfg.nvic_irq_pre_priority, pUartConfig->uartIntCfg.nvic_irq_sub_priority);
        }
  
        if (pUartConfig->uart_periph == USART5)
        {
            usart5_flag_clear(pUartConfig->uart_periph, USART5_FLAG_IDLE);
            usart5_flag_clear(pUartConfig->uart_periph, USART5_FLAG_RBNE);
            usart5_interrupt_enable(pUartConfig->uart_periph, USART5_INT_RBNE);
            usart5_interrupt_enable(pUartConfig->uart_periph, USART5_INT_IDLE);
        }
        else
        {
            usart_flag_clear(pUartConfig->uart_periph, USART_FLAG_IDLE);
            usart_flag_clear(pUartConfig->uart_periph, USART_FLAG_RBNE);
            usart_interrupt_enable(pUartConfig->uart_periph, USART_INT_RBNE);
            usart_interrupt_enable(pUartConfig->uart_periph, USART_INT_IDLE);
        }

        eRet1 = CycleBuf_CreateChannel(&pUartConfig->uartBufCtrl.txCycleBufID, pUartConfig->uartBufCtrl.pTxBuf,
        pUartConfig->uartBufCtrl.txBufSize, pUartConfig->uartBufCtrl.txProfile);

        eRet2 = CycleBuf_CreateChannel(&pUartConfig->uartBufCtrl.rxCycleBufID, pUartConfig->uartBufCtrl.pRxBuf,
        pUartConfig->uartBufCtrl.rxBufSize, CYCLEBUF_PROFILE_CIRCLE);

        if (pUartConfig->DMATx_En == TRUE)
        {
            rcu_periph_clock_enable(pUartConfig->DMATx_Cfg.rcu_DMA_periph);
            dma_deinit(pUartConfig->DMATx_Cfg.DMA_periph, pUartConfig->DMATx_Cfg.DMA_ch);
            dma_init(pUartConfig->DMATx_Cfg.DMA_periph, pUartConfig->DMATx_Cfg.DMA_ch, &pUartConfig->DMATx_Cfg.DMA_parameter);
            dma_circulation_disable(pUartConfig->DMATx_Cfg.DMA_periph, pUartConfig->DMATx_Cfg.DMA_ch);
            dma_memory_to_memory_disable(pUartConfig->DMATx_Cfg.DMA_periph, pUartConfig->DMATx_Cfg.DMA_ch);
            dma_channel_disable(pUartConfig->DMATx_Cfg.DMA_periph, pUartConfig->DMATx_Cfg.DMA_ch);
        }

        if (eRet1 == eGlobalRet_OK && eRet2 == eGlobalRet_OK)
        {
            usart_enable(pUartConfig->uart_periph);
            pUartConfig->initFlag = TRUE;
        }  
    }
}

void McalUart_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < eMcalUartChanel_Count; index++)
    {
        McalUart_ConfigChannel(&g_UartConfigTable[index]);
    }
}

GlobalRet_Enum McalUart_WriteData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pBuf != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    eRet = CycleBuf_WriteData(pUartCfg->uartBufCtrl.txCycleBufID, pBuf, dataLen);

    if (eRet == eGlobalRet_OK)
    {
        if (pUartCfg->uartCtrl.txtate == MCALUART_TXSTATE_IDLE)
        {
            pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_BUSY;

            if (pUartCfg->uartCtrl.txMode == MCALUART_CFG_TXMODE_INT)
            {
                if (pUartCfg->uart_periph == USART5)
                {
                    usart5_interrupt_enable(pUartCfg->uart_periph, USART5_INT_TBE);
                }
                else
                {
                    usart_interrupt_enable(pUartCfg->uart_periph, USART_INT_TBE);
                }
            }
            else if (pUartCfg->uartCtrl.txMode == MCALUART_CFG_TXMODE_DMA)
            {
                while (eGlobalRet_OK == CycleBuf_CheckDataLen(pUartCfg->uartBufCtrl.txCycleBufID, &remainLen))
                {
                    dma_channel_disable(pUartCfg->DMATx_Cfg.DMA_periph, pUartCfg->DMATx_Cfg.DMA_ch);
                    dma_memory_address_config(pUartCfg->DMATx_Cfg.DMA_periph, pUartCfg->DMATx_Cfg.DMA_ch,pUartCfg->DMATx_Cfg.DMA_parameter.memory_addr);
                    dma_transfer_number_config(pUartCfg->DMATx_Cfg.DMA_periph, pUartCfg->DMATx_Cfg.DMA_ch, remainLen);
                    dma_channel_enable(pUartCfg->DMATx_Cfg.DMA_periph, pUartCfg->DMATx_Cfg.DMA_ch);
                    usart_dma_transmit_config(pUartCfg->uart_periph, USART_TRANSMIT_DMA_ENABLE);
                    while (RESET == dma_flag_get(pUartCfg->DMATx_Cfg.DMA_periph, pUartCfg->DMATx_Cfg.DMA_ch, DMA_FLAG_FTF)) {}
                    CycleBuf_RemoveData(pUartCfg->uartBufCtrl.txCycleBufID, remainLen);
                }
            
                pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_IDLE;
            }
            else
            {
                while (eGlobalRet_OK == CycleBuf_ReadData(pUartCfg->uartBufCtrl.txCycleBufID, &byte, 1))
                {
                    if (pUartCfg->uart_periph == USART5)
                    {
                        usart_data_transmit(pUartCfg->uart_periph, byte);
                        while (RESET == usart5_flag_get(pUartCfg->uart_periph, USART5_FLAG_TBE)) {}
                    }
                    else
                    {
                        usart_data_transmit(pUartCfg->uart_periph, byte);
                        while (RESET == usart_flag_get(pUartCfg->uart_periph, USART_FLAG_TBE)) {}
                    }
                }

                pUartCfg->uartCtrl.txtate = MCALUART_TXSTATE_IDLE;
            }
        }
    }

    return eRet;
}

GlobalRet_Enum McalUart_ReadData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pBuf != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    return CycleBuf_ReadData(pUartCfg->uartBufCtrl.rxCycleBufID, pBuf, dataLen);
}

GlobalRet_Enum McalUart_PreviewReadData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pBuf != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    return CycleBuf_PreviewReadData(pUartCfg->uartBufCtrl.rxCycleBufID, pBuf, dataLen);
}

GlobalRet_Enum McalUart_CheckDataLen(McalUartChanel_Enum eCh, uint16_t* pRemainLen)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pRemainLen != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    return CycleBuf_CheckDataLen(pUartCfg->uartBufCtrl.rxCycleBufID, pRemainLen);
}

GlobalRet_Enum McalUart_RemoveData(McalUartChanel_Enum eCh, uint16_t removeLen)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(removeLen != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    return CycleBuf_RemoveData(pUartCfg->uartBufCtrl.rxCycleBufID, removeLen);
}

GlobalRet_Enum McalUart_ResetRecvBuf(McalUartChanel_Enum eCh)
{
    McalUartConfig_Struct *pUartCfg = &g_UartConfigTable[eCh];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t remainLen = 0;
    uint8_t byte = 0;

    PARA_ASSERT_RET(eCh < eMcalUartChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pUartCfg->initFlag == TRUE, eGlobalRet_NotInit);

    return CycleBuf_ResetBuf(pUartCfg->uartBufCtrl.rxCycleBufID);
}

void McalUart_Test(void)
{
    // char buf[] = "Hello world!!!\r\n";
    // McalUart_WriteData(eMcalUartChanel_Debug, (uint8_t *)buf, strlen(buf));

    char txStr[] = "ATI\r\n";
    McalUart_WriteData(eMcalUartChanel_4G, (uint8_t *)txStr, strlen(txStr));

    // uint8_t txbuf[] = {0x58, 0x18};
    // McalUart_WriteData(eMcalUartChanel_MeterChip, (uint8_t *)txbuf, 2);
}























