
#ifndef __UART_RESOURCE_MANAGE_H__
#define __UART_RESOURCE_MANAGE_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceSummaryDescribe.h"


#if defined(GD32E230)

#elif defined(GD32E50X)
typedef enum
{
    E_FIRST_UART_INDEX = 0,
    E_UART0_INDEX = 0,
    E_UART1_INDEX,
    E_UART2_INDEX,
    E_UART3_INDEX,
    E_UART4_INDEX,
    E_UART5_INDEX,
    E_MAX_UART_NUMBER,
}E_UART_CHANNEL_LIST;
#endif

#pragma pack(1)

typedef struct
{
    E_DIO_RESOURCE_MANAGE   TxPinNum;
    E_DIO_RESOURCE_MANAGE   RxPinNum;
    E_UART_CHANNEL_LIST     UartChannelIndex;
#if defined(GD32E230) || defined(GD32E50X)
    uint32_t                TX_RCU_GPIOx;
    uint32_t                TX_GPIOx;
    uint16_t	    		TX_GPIO_Pin;
    uint32_t                RX_RCU_GPIOx;
    uint32_t                RX_GPIOx;
    uint16_t	    		RX_GPIO_Pin;
#endif
    uint8_t                 AlternateFuncFlag;
}STRU_UART_RESOURCE_MANAGE;

typedef struct
{
    E_UART_CHANNEL_LIST  UartChannelIndex;
    uint8_t             EnableFlag;
    uint8_t             ResourceTableIndex;
#if defined(GD32E230) || defined(GD32E50X)
    uint32_t            UartNumber;
    uint32_t            rcc_periph_function;
#endif
}STRU_UART_ENABLE_MANAGE;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /*__UART_RESOURCE_MANAGE_H__*/
