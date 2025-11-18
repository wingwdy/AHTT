/******************************************************************************
* File Name          : Mcal_Uart.h
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
#ifndef MCAL_UART_H_
#define MCAL_UART_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h" 
#include "gd32e50x_usart.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALUART_TXSTATE_IDLE                    0
#define MCALUART_TXSTATE_BUSY                    1
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eMcalUartChanel_Debug,
    eMcalUartChanel_4G,
    eMcalUartChanel_MeterChip,
    eMcalUartChanel_Count,
}McalUartChanel_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void McalUart_Init(void);
GlobalRet_Enum McalUart_WriteData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen);
GlobalRet_Enum McalUart_PreviewReadData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen);
GlobalRet_Enum McalUart_ReadData(McalUartChanel_Enum eCh, uint8_t *pBuf, uint16_t dataLen);
GlobalRet_Enum McalUart_CheckDataLen(McalUartChanel_Enum eCh, uint16_t* pRemainLen);
void McalUart_Test(void);
#endif /* Mcal_Uart.h */


















