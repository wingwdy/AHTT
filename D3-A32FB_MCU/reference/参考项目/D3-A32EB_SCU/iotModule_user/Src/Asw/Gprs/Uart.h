#ifndef __UART__
#define __UART__

#ifdef __cplusplus
extern "C" {
#endif

#include "Libinclude.h"
#include "UartRouteManage.h"

#ifdef __UART_GLONALS__
#define EXTERN_UART_DATA 
#else
#define EXTERN_UART_DATA extern
#endif

enum {
	UART_GPRS,	//GPRSID
	UART_HMI,	//HMI_ID
	UART_ETH,
	UART_CNT_MAX,
};

#define  UART_CNT		UART_CNT_MAX				//

//////////////////////////////////////////////////////////////////////////
//通讯状态定义
enum 
{
    eUart_Idle             = 0x00,     //  
    eUart_Busy,     	//  
    eUart_TxTrans,     //  
    eUart_RxTrans,     //  
    eUart_TxFinish,     //  
    eUart_RxFinish,     //  
    eUart_Timeout,     //  
};

//////////////////////////////////////////////////////////////////////////
//串口控制结构

typedef struct __UARTDCB__
{
	U16 RxLength;			//接收数据长度
	U16 TxLength;			//发送数据长度
	
	U16 TxCounter;			//发送数据计数器
	
	U32 TxFrameInterval;	//帧间隔时间，单位，ms
	U32 RxFrameInterval;	//帧间隔时间，单位，ms
	
	U32 TxTick;				//发送心跳
	U32 TxInterval;			//发送间隔计数器
	
	U32 RxTick;				//接收心跳
	U32 RxInterval;			//接收间隔计数器
	
	volatile U8 RxState;	//接收通讯状态
	volatile U8 TxState;	//发送通讯状态
   	
	U16 u16RxBufSize;		//接收缓存长度
	U16 u16TxBufSize;		//发送缓存长度
	U8 *pRxBuffer;
	U8 *pTxBuffer;
}UartDCB;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////


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
//作者:			南京能瑞自动化设备有限公司
//日期:			2007-04-27
//////////////////////////////////////////////////////////////////////////
U8 UartSendData(U8 uartID, U16 datalen);

void UartServer(void);

void InitGPRSUart(void);
void InitBleUart(void);
void InitAllUart(void);

UartDCB  *GetUartDCB(const U8 uartID);

#ifdef __cplusplus
}
#endif

#endif
