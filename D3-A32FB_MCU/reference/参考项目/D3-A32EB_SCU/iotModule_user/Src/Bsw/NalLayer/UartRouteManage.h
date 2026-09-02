#ifndef __UART_ROUTE_MANAGE_H_
#define __UART_ROUTE_MANAGE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "FunctionalHeaderSummary.h"
#include "FuncExternHeaderSummary.h"

#define bsp_printf					printf
 
#define UART_BLOCK_MIN_SIZE 		32
/*DEBUG*/
#define UART1_TX_BUF_SIZE 			(16 * UART_BLOCK_MIN_SIZE)
#define UART1_RX_BUF_SIZE 			(16 * UART_BLOCK_MIN_SIZE)
/*CCU */
#define UART2_TX_BUF_SIZE 			(8 * UART_BLOCK_MIN_SIZE)
#define UART2_RX_BUF_SIZE 			(8 * UART_BLOCK_MIN_SIZE)
/*4G*/
#define UART3_TX_BUF_SIZE 			(48 * UART_BLOCK_MIN_SIZE)
#define UART3_RX_BUF_SIZE 			(48 * UART_BLOCK_MIN_SIZE)
/*RESVER*/
#define UART4_TX_BUF_SIZE 			(1 * UART_BLOCK_MIN_SIZE)
#define UART4_RX_BUF_SIZE 			(1 * UART_BLOCK_MIN_SIZE)
/*HMI*/
#define UART5_TX_BUF_SIZE 			(32 * UART_BLOCK_MIN_SIZE)
#define UART5_RX_BUF_SIZE 			(32 * UART_BLOCK_MIN_SIZE)
/*RESVER*/
#define UART6_TX_BUF_SIZE 			(1 * UART_BLOCK_MIN_SIZE)
#define UART6_RX_BUF_SIZE 			(1 * UART_BLOCK_MIN_SIZE)


typedef enum
{
	E_UART_NO_ERR = 0,
	E_UART_GPIO_INIT_ERR,
	E_UART_BAUD_SET_ERR,
	E_UART_PARA_ERR,
	E_UART_ID_ERR,
	E_UART_NO_INIT_ERR,
	E_UART_BUF_LEN_ERR,
}E_UART_ERR;

/* 发送模式 */
typedef enum
{
	E_TX_MODE_DIR = 0,
	E_TX_MODE_INT,
	E_TX_MODE_DMA,
}E_UART_TX_MODE;

#pragma pack(1)
typedef struct
{
	uint8_t             u8_ManageChannel;
	uint8_t             u8_Enable;
	uint32_t	    	u32_UARTx_PERIPH;
	uint32_t            u32_UARTx_RCU;
	uint32_t            u32_UARTx_IRQ;
    uint32_t            u32_TX_RCU_GPIOx;
    uint32_t            u32_TX_GPIOx;
    uint16_t	    	u16_TX_GPIO_Pin;
    uint32_t            u32_RX_RCU_GPIOx;
    uint32_t            u32_RX_GPIOx;
    uint16_t	    	u16_RX_GPIO_Pin;
    uint32_t 			u32_BaudRate;
}STRU_UART_CFG;

typedef struct
{
    uint8_t* 			pu8_TxBuf;		// 发送缓存
	uint8_t* 			pu8_RxBuf;		// 接收缓存
	uint16_t 			u16_TxBufSize;	// 发送缓存大小
	uint16_t 			u16_RxBufSize;	// 接收缓存大小
	__IO uint16_t 		u16_TxRead;		// 发送缓存新数据头位置
	__IO uint16_t 		u16_TxLen;	    // 还未处理的发送缓存中的数据长度
	__IO uint16_t 		u16_RxHead;	    // 接收缓存新数据头位置
	__IO uint16_t 		u16_RxTail;	    // 接收缓存尾部位置
	__IO uint16_t 		u16_RxLen;	    // 还未处理的接收缓存中的数据长度
    uint8_t  			u8_UartInited;  // 串口x是否已经初始化
	uint8_t             u8_UartTxMode;  // 发送模式
    __IO uint8_t 		u8_BufTC;       // 串口x缓存发送完
    uint32_t 			u32_DMA_FLAG_TCIFx;  // 串口DMA发送完成中断标志的通道号

    
	__IO uint16_t 		u16_TxHead;	    // 发送缓存尾部位置
	__IO uint16_t 		u16_TxTail;	    // 发送缓存尾部位置
}STRU_UART_MANAGE;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif


E_UART_ERR Bsp_GetTCSta(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_TCFlag);

uint8_t fgu8_getRecvCompleteFlagRoute(E_UART_CHANNEL_LIST e_UartId);
void fgv_setRecvCompleteFlagRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t flag);
uint16_t fgu16_UartRecvRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_Buf, uint16_t u16_RcvLen, E_UART_ERR *pe_UartErr);
E_UART_ERR fge_UartClearRxBufRoute(E_UART_CHANNEL_LIST e_UartId);
E_UART_ERR fge_UartSendRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_Buf, uint16_t u16_SndLen);

E_UART_ERR fge_CCU_UartDMASendRoute(uint8_t *pu8_Buf, uint16_t u16_SndLen);
E_UART_ERR fge_GPRS_UartDMASendRoute(uint8_t *pu8_Buf, uint16_t u16_SndLen);
uint32_t fgu32_AppUartInit();

//4G模块异常处理,电路电平转换问题导致上电需要手动下拉下串口引脚，以确保模块重启无异常
void BspUartInitGprsIpd(void);
void BspUartInitGprs(void);

#ifdef __cplusplus
}
#endif

#endif /*__UART_ROUTE_MANAGE_H_*/
