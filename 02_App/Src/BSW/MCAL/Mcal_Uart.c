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
*2025/10/10      V1.0.0      Chenls    初版创建
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



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void McalUart_Init(void)
{
    rcu_periph_clock_enable(RCU_USART5);
    /* configure USART5 Tx Rx as alternate function */
    gpio_afio_port_config(AFIO_PA11_USART5_CFG, ENABLE);
    gpio_afio_port_config(AFIO_PA12_USART5_CFG, ENABLE);

    /* USART configure */
    usart_deinit(USART5);
    usart_baudrate_set(USART5, 115200U);
    usart_receive_config(USART5, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART5, USART_TRANSMIT_ENABLE);
    usart_enable(USART5);

    rcu_periph_clock_enable(RCU_UART4);
    usart_deinit(UART4);
    usart_baudrate_set(UART4, 115200U);
    usart_word_length_set(UART4, USART_WL_8BIT);
    usart_stop_bit_set(UART4, USART_STB_1BIT);
    usart_parity_config(UART4, USART_PM_NONE);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);

    usart_flag_clear(UART4, USART_FLAG_IDLE);
    usart_flag_clear(UART4, USART_FLAG_RBNE);
    usart_interrupt_enable(UART4, USART_INT_RBNE);
    usart_interrupt_enable(UART4, USART_INT_IDLE);
    usart_enable(UART4);

    rcu_periph_clock_enable(RCU_USART1);
    usart_deinit(USART1);
    usart_baudrate_set(USART1, 4800);
    usart_word_length_set(USART1, USART_WL_8BIT);
    usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_parity_config(USART1, USART_PM_NONE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);

    usart_flag_clear(USART1, USART_FLAG_IDLE);
    usart_flag_clear(USART1, USART_FLAG_RBNE);
    usart_interrupt_enable(USART1, USART_INT_RBNE);
    usart_interrupt_enable(USART1, USART_INT_IDLE);
    usart_enable(USART1);
}


int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART5, (uint8_t)ch);
    while(RESET == usart_flag_get(USART5, USART5_FLAG_TBE));
    return ch;
}

void McaUart_Uart4Tx(void)
{
    char txStr[16] = "ATI\r\n";
    uint8_t count = strlen(txStr);
    uint8_t read = 0;

    while (count > 0)
    {
        usart_data_transmit(UART4, txStr[read]);
        while(RESET == usart_flag_get(UART4, USART_FLAG_TBE));
        read++;
        count--;
    }
}

void McaUart_Uart1Tx(void)
{
    char txStr[2] = {0x58, 0x18};
    uint8_t count = 2;
    uint8_t read = 0;

    while (count > 0)
    {
        usart_data_transmit(USART1, txStr[read]);
        while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        read++;
        count--;
    }
}


void McalUart_Test(void)
{
    printf("Hello world!\r\n");

    McaUart_Uart4Tx();

    McaUart_Uart1Tx();
}

























