//////////////////////////////////////////////////////////////////////////
//Copyright (C), 2007-2050, 
//文件名：		Uart.c
//作者:			
//版本号:		V1.0
//创建日期:		2007-04-27
//说明:			(GPRS/RS232、2路485、红外)通讯发送/接受程序
//函数列表:		Uart0Server(...)	//串口0通讯服务
//				Uart1Server(...)	//串口1通讯服务
//				Uart2Server(...)	//串口2通讯服务
//				GetUartBuffer(...)	//获得当前端口状态
//				UartSendData(...)	//根据不同类型数据发送
//				DefaultUartPort(...)//设置端口Uart1,Uart2
//				SetUartPortByDT(...)//设定相应的端口通讯参数
//				SetUartPort(...)	//设定指定端口的工作参数
//				
//修改记录:
//////////////////////////////////////////////////////////////////////////

#define __UART_GLONALS__

#include "Uart.h"
#include "Gprslib.h"
#include "screenUart.h"


#define Usart_GPRS	E_UART2_INDEX
#define Usart_HMI	E_UART4_INDEX

//====================================================
//Uart缓存信息
#define U1_TXBUF_MAXLEN      		(1024*2)		//云快充2.1订单上报超1024，改为2048
#define U1_RXBUF_MAXLEN      		(1024*2)
#define U2_TXBUF_MAXLEN      		(256)
#define U2_RXBUF_MAXLEN      		(64)
#define U3_TXBUF_MAXLEN             (1460)
#define U3_RXBUF_MAXLEN             (1460)
static U8 U8Uart1TXBuf[U1_TXBUF_MAXLEN] = {0};		//U1发送缓存
static U8 U8Uart1RXBuf[U1_RXBUF_MAXLEN] = {0};		//U1接收缓存
static U8 U8Uart2TXBuf[U2_TXBUF_MAXLEN] = {0};		//U2发送缓存
static U8 U8Uart2RXBuf[U2_RXBUF_MAXLEN] = {0};		//U2接收缓存
static U8 U8Uart3TXBuf[U3_TXBUF_MAXLEN] = {0};		//U3发送缓存
static U8 U8Uart3RXBuf[U3_RXBUF_MAXLEN] = {0};		//U3接收缓存
static UartDCB g_strUart[UART_CNT];

UartDCB *GetUartDCB(const U8 uartID)
{
	if (uartID < UART_CNT) {
		return &g_strUart[uartID];
	}
	return NULL;
}

static void UartRTSEnable(const U8 uartID)
{
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	
	switch (uartID)
	{
	case UART_GPRS:
		fge_UartSendRoute(Usart_GPRS, pUartDCB->pTxBuffer, pUartDCB->TxLength);
		return;
	case UART_HMI:
		fge_UartSendRoute(Usart_HMI, pUartDCB->pTxBuffer, pUartDCB->TxLength);
		return;
	default:
		return;
	}
}

static U8 UartTCSta(const U8 uartID)
{
//	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	U8 u8TCFlag = FALSE;
	
	switch (uartID)
	{
	case UART_GPRS:
		Bsp_GetTCSta(Usart_GPRS, &u8TCFlag);
		break;
	case UART_HMI:
		Bsp_GetTCSta(Usart_HMI, &u8TCFlag);
		break;
	default:
		break;
	}
	
	return u8TCFlag;
}

//////////////////////////////////////////////////////////////////////////
//

static void UartRxCntDec(const U8 uartID)
{
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	U16 u16RxLength = 0;
	E_UART_ERR e_Err = E_UART_NO_ERR;

	if(UART_GPRS == uartID)
		u16RxLength = fgu16_UartRecvRoute(Usart_GPRS, &pUartDCB->pRxBuffer[pUartDCB->RxLength], pUartDCB->u16RxBufSize - pUartDCB->RxLength, &e_Err);
	else if(UART_HMI == uartID)
		u16RxLength = fgu16_UartRecvRoute(Usart_HMI, &pUartDCB->pRxBuffer[pUartDCB->RxLength], pUartDCB->u16RxBufSize - pUartDCB->RxLength, &e_Err);
	
	if(0 != u16RxLength)
	{
		pUartDCB->RxLength += u16RxLength;
		pUartDCB->RxState = eUart_RxTrans;
		pUartDCB->RxTick = NOWTICK;
		pUartDCB->RxInterval = pUartDCB->RxFrameInterval;  
	}
	
	if(eUart_RxTrans != pUartDCB->RxState)
		return;
	
	if (JudgeTimeOutMs(pUartDCB->RxTick, pUartDCB->RxInterval))
	{
		pUartDCB->RxState = eUart_RxFinish;
	}
	return;
}

static void UartTxCntDec(const U8 uartID)
{
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	
	if(eUart_TxTrans == pUartDCB->TxState)
	{
//		if(TRUE == UartTCSta(uartID))
		{
			pUartDCB->TxState = eUart_TxFinish;
		}
		return;
	}
	
	if(eUart_TxFinish != pUartDCB->TxState)
		return;
	
	if (JudgeTimeOutMs(pUartDCB->TxTick, pUartDCB->TxInterval))
	{
		// LogPrintf(LVL_LOG_INFO, "\r\n%s\r\n", pUartDCB->TxBuffer);
		pUartDCB->TxState = eUart_Idle;
	}
	return;
}


void UartCntProc(void)
{
	U8 uartID = 0;
	
	for(uartID = 0; uartID < UART_CNT; uartID++)
	{
		UartRxCntDec(uartID);
		UartTxCntDec(uartID);
	}
	
	return;
}

static void UartGPRSServer(void)
{
	U8 uartID = UART_GPRS;
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	
	if (NULL == pUartDCB)
		return;
	
	if (eUart_RxFinish == pUartDCB->RxState)
	{
		LogPrintf(LVL_LOG_INFO, "\r\n%s\r\n", pUartDCB->pRxBuffer);
		
		//模块数据接收处理
		MTRecvDecode(pUartDCB->pRxBuffer, pUartDCB->RxLength);
		
		pUartDCB->RxState = eUart_Idle;
		memset(pUartDCB->pRxBuffer, 0, pUartDCB->u16RxBufSize);
		pUartDCB->RxLength = 0;
	}
	return;
}

static void UartHMIServer(void)
{
	U8 uartID = UART_HMI;
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	
	if (NULL == pUartDCB)
		return;
	
	if (eUart_RxFinish == pUartDCB->RxState)
	{
		//屏幕数据接收处理
		RecvScreenCmdHandle(pUartDCB->pRxBuffer, pUartDCB->RxLength);

		pUartDCB->RxState = eUart_Idle;
		memset(pUartDCB->pRxBuffer, 0, pUartDCB->u16RxBufSize);
		pUartDCB->RxLength = 0;
	}
	return;
}

//处理过的数据移除
void UartRecvBufMove(U8 uartID, U8 *pHead, U32 u32MoveLen)
{
	UartDCB *pUartDCB = (UartDCB*)GetUartDCB(uartID);
	
	if((U32)pHead < (U32)pUartDCB->pRxBuffer
		|| (U32)pHead > (U32)&pUartDCB->pRxBuffer[pUartDCB->u16RxBufSize-1]
		|| u32MoveLen > pUartDCB->u16RxBufSize
		|| 0 == u32MoveLen
		|| u32MoveLen > pUartDCB->RxLength)
		return;
	
	memmove(pHead, (pHead + u32MoveLen), u32MoveLen);
	pUartDCB->RxLength -= u32MoveLen;
//	memset(&pUartDCB->RxBuffer[UART_RXBUFFER_LEN-u32MoveLen], 0, u32MoveLen);
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名:		UartSendData
//功能描述:		使用串口发送数据
//输入参数:		U8 uartID:	串口ID
//				U8 PID	:进行本次发送操作的PID号
//				U16 recttime	:等待应答的时间 单位：毫秒
//				U16 frameinterval	:数据帧间隔时间。单位ms
//				U16 datalen	: 要发送的数据长度
//输出参数:		无
//访问:			public 
//返回类型:		U8
//作者:			
//日期:			2007-04-27
//////////////////////////////////////////////////////////////////////////
U8 UartSendData(U8 uartID, U16 datalen)
{
	UartDCB *pUartDCB = (UartDCB *)GetUartDCB(uartID);
	
	if (0 == datalen || NULL == pUartDCB)
		return FALSE;
	
	pUartDCB->TxLength = datalen;
	
	pUartDCB->TxState = eUart_TxTrans;
	UartRTSEnable(uartID);
	
	return TRUE;
}

void UartServer(void)
{
	UartCntProc();
	UartGPRSServer();
	UartHMIServer();
}

void InitGPRSUart(void)
{
	U8 uartID = UART_GPRS;
	UartDCB *pUart = &g_strUart[uartID];
	
	memset(pUart, 0, sizeof(UartDCB));
	
	pUart->RxFrameInterval = eTick_300ms;
	pUart->TxFrameInterval = eTick_50ms;
	memset(U8Uart1RXBuf, 0, U1_RXBUF_MAXLEN);
	memset(U8Uart1TXBuf, 0, U1_TXBUF_MAXLEN);
	pUart->u16RxBufSize = U1_RXBUF_MAXLEN;
	pUart->u16TxBufSize = U1_TXBUF_MAXLEN;
	pUart->pRxBuffer = U8Uart1RXBuf;
	pUart->pTxBuffer = U8Uart1TXBuf;
	
	return;
}

void InitHmiUart(void)
{
	U8 uartID = UART_HMI;
	UartDCB *pUart = &g_strUart[uartID];
	
	memset(pUart, 0, sizeof(UartDCB));
	
	pUart->RxFrameInterval = eTick_5ms;
	pUart->TxFrameInterval = eTick_50ms;
	
	memset(U8Uart2RXBuf, 0, U2_RXBUF_MAXLEN);
	memset(U8Uart2TXBuf, 0, U2_TXBUF_MAXLEN);
	pUart->u16RxBufSize = U2_RXBUF_MAXLEN;
	pUart->u16TxBufSize = U2_TXBUF_MAXLEN;
	pUart->pRxBuffer = U8Uart2RXBuf;
	pUart->pTxBuffer = U8Uart2TXBuf;
	
	return;
}
void InitEthUart(void)
{
	U8 uartID = UART_ETH;
	UartDCB *pUart = &g_strUart[uartID];
	
	memset(pUart, 0, sizeof(UartDCB));
	
	pUart->RxFrameInterval = eTick_100ms;
	pUart->TxFrameInterval = eTick_100ms;
	
	memset(U8Uart3RXBuf, 0, U3_RXBUF_MAXLEN);
	memset(U8Uart3TXBuf, 0, U3_TXBUF_MAXLEN);
	pUart->u16RxBufSize = U3_RXBUF_MAXLEN;
	pUart->u16TxBufSize = U3_TXBUF_MAXLEN;
	pUart->pRxBuffer = U8Uart3RXBuf;
	pUart->pTxBuffer = U8Uart3TXBuf;
	return;
}
void InitAllUart(void)
{
	InitGPRSUart();
	InitHmiUart();
    InitEthUart();
	return;
}




