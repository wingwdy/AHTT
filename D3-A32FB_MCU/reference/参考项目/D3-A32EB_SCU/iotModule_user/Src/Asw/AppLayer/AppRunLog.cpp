/**********************************************************************************************************
  * FileName         : AppRunLog.cpp
  * Author           : sjc
  * Version          : V1.0.0
  * Description      :
  * Date             : 2023.09.11 
**********************************************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "AppHeaderSummary.h"
#include "AppInputCfg.h"

#define Usart_RunLog	E_UART0_INDEX

static QueueHandle_t pQueueLogMsg = NULL;
static uint8_t sg_ucLogLvl = LVL_LOG_WARN;

STRU_LOG_MSG stuLogMsg;

uint16_t u16_RecvLen_ = 0;
uint8_t u8_RecvBuff_[RUNLOG_RX_BUFF_SIZE] = {0};
int g_debug = 5;


static uint8_t hex_dump_printf_flag = 0;

//设置sg_ucLogLvl等级
void log_printf(uint8_t flag)
{
	sg_ucLogLvl = flag;
}

void hex_dump_printf(uint8_t flag)
{
	hex_dump_printf_flag = flag;
}
void printf_Current_time()
{
	uint8_t now_time[8] = { 0 };
	getRunTimeYYMDHMS(now_time);

    printf("\r\n%d/%02d/%02d %02d:%02d:%02d\r\n", now_time[0] * 100 + now_time[1], now_time[2], now_time[3], now_time[4], now_time[5], now_time[6]);
}
// 打印日志的封装函数
void NeedOnPrintf(const char *format, ...) {
    va_list args;
    
    if (!hex_dump_printf_flag) return;
    
    // 使用va_start, va_arg, va_end处理可变参数
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    // fputc('\r\n\r\n', stderr);
}

void hex_dump(const char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	const unsigned char *pt;
	int x;
	char buf[256];
	char *p = buf;

	if (!hex_dump_printf_flag) {
		return;
	}

	if (SrcBufLen == 0) {
		return;
	}
    
    printf_Current_time();

	pt = pSrcBufVA;
	printf("%s: %p, len = %d\r\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		sprintf(p, "%02x ", ((unsigned char)pt[x]));
        p = buf + strlen(buf);
        if (strlen(buf) >= (sizeof(buf)-4)) {
            printf("%s", buf);
            memset(buf, 0, sizeof(buf));
            p = buf;
        }
	}
    if (p!=buf) {
        sprintf(p, "\r\n");
        printf("%s", buf);
    }
}

static void _inputPrintf(uint8_t *buf, uint16_t len)
{
	uint16_t n = 0;
	while(len--) {
		switch (buf[n]) {
			case '\r' :
			case '\n' :		/* Enter */
				printf("\r\n");
				continue;
			case '\b' :		/* backspace */
			// case 0x1B:		/* delete */
				printf("\b \b");	/* backspace and delete */
				continue;
		}
		printf("%c", buf[n]);
		n++;
	}
}

static void RunLogProcess()
{
	E_UART_ERR e_Err = E_UART_NO_ERR;
	
	u16_RecvLen_ = fgu16_UartRecvRoute(Usart_RunLog, u8_RecvBuff_, RUNLOG_RX_BUFF_SIZE, &e_Err);
	
	if(u16_RecvLen_)
	{
		uint16_t len = 0;
		do
		{
			osDelay(10);
			len = fgu16_UartRecvRoute(Usart_RunLog, u8_RecvBuff_ + u16_RecvLen_, RUNLOG_RX_BUFF_SIZE - u16_RecvLen_, &e_Err);
			u16_RecvLen_ += len;
		}
		while(len);

		// _inputPrintf(u8_RecvBuff_, u16_RecvLen_);
		pushInputCmd(u8_RecvBuff_, u16_RecvLen_);

		u16_RecvLen_ = 0;
	}
	else
	{
		if (pQueueLogMsg) 
		{
			uint8_t MOD_LOG = 6;
			STRU_LOG_MSG RecvMsg;
			STRU_LOG_MSG *pRecvMsg = &RecvMsg;
			static uint8_t s_ucLogLvlCur = 3;

			if (pdTRUE == xQueueReceive(pQueueLogMsg, pRecvMsg, 0))
			{
				if(pRecvMsg)
				{
					uint8_t ulLogID = 0;
					ulLogID = (uint8_t)pRecvMsg->aucCode[0];

					bsp_printf( "%02d", pRecvMsg->aucTime[0] );
					bsp_printf( "%02d/", pRecvMsg->aucTime[1] );
					bsp_printf( "%02d/", pRecvMsg->aucTime[2] );
					bsp_printf( "%02d ", pRecvMsg->aucTime[3] );
					bsp_printf( "%02d:", pRecvMsg->aucTime[4] );
					bsp_printf( "%02d:", pRecvMsg->aucTime[5] );
					bsp_printf( "%02d.", pRecvMsg->aucTime[6] );
					bsp_printf( "%03d ", pRecvMsg->aucTime[7] + pRecvMsg->aucTime[8] * 256);

					bsp_printf( "LogID: %02X ", ulLogID );
					bsp_printf( "Info: %s \r\n", pRecvMsg->acLogInfo );

//					vPortFree(pRcvMsg);
				}
			}
			else
			{
			 	if( s_ucLogLvlCur != sg_ucLogLvl )
			 	{
					s_ucLogLvlCur = sg_ucLogLvl;
			 		switch (sg_ucLogLvl)
			 		{
			 			case LVL_LOG_NONE:
			 			{
			 				LogPrintf( LVL_LOG_NONE, "No Log print!" );
			 			}
			 				break;
			 			case LVL_LOG_ERR:
			 			{
			 				LogPrintf( LVL_LOG_ERR, "Log print errcode only!" );
			 			}
			 				break;
			 			case LVL_LOG_WARN:
			 			{
			 				LogPrintf( LVL_LOG_WARN, "Log print warncode only!" );
			 			}
			 				break;
			 			case LVL_LOG_INFO:
			 			{
			 				LogPrintf( LVL_LOG_INFO, "Log print infocode only!" );
			 			}
			 				break;
			 			case LVL_LOG_ALL:
			 			{
							LogPrintf( LVL_LOG_ALL, "Log print all!" );
			 			}
			 				break;
						default:
							break;
			 		}
			    }
			}
		}

	}
}

void LogPrintf(uint8_t ucEvLvl, const char *pcFormat, ...)
{
	if(pQueueLogMsg == 0)
		return;

	if(sg_ucLogLvl)
	{
		if(sg_ucLogLvl & ucEvLvl)
		{
			STRU_LOG_MSG *pSndMsgItem = 0;
			pSndMsgItem = &stuLogMsg;
			if(!pSndMsgItem)
			{
				return;
			}
			uint8_t aucTime[9] = {0};
			pSndMsgItem->aucCode[0] = ucEvLvl;
			getRunTime(aucTime);
			memcpy( pSndMsgItem->aucTime, aucTime, 9);

			if(pcFormat == 0)
			{
				pSndMsgItem->acLogInfo[0] = 'N';
				pSndMsgItem->acLogInfo[1] = '/';
				pSndMsgItem->acLogInfo[2] = 'A';
				pSndMsgItem->acLogInfo[3] = 0;
			}
			else
			{
				va_list arg_ptr;
				//
				va_start(arg_ptr, pcFormat);
				vsnprintf(pSndMsgItem->acLogInfo, sizeof(pSndMsgItem->acLogInfo) - 1, pcFormat, arg_ptr);
				va_end(arg_ptr);
			}
			xQueueSend(pQueueLogMsg, pSndMsgItem, 100);
		}
	}
}

uint8_t flash_test_buf[256] = {0};

void runLogTask(void* parameter)
{
	bsp_printf("\r\n========================\r\n");
	// bsp_printf("[version] %x.%x.%x.%x \r\n",g_SoftVer.X,g_SoftVer.Y,g_SoftVer.Z,g_SoftVer.W);
	
	uint32_t time_cnt = 0;
	uint32_t time_sec = 0;
//	for(int i = 0; i < sizeof(flash_test_buf); i++)
//	{
//		flash_test_buf[i] = i;
//	}
//	fgu8_AppInfoStoreWriteRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_PAGE_TYPE, 0x100, flash_test_buf, sizeof(flash_test_buf));
//	memset(flash_test_buf, 0, sizeof(flash_test_buf) );
//	fgu8_AppInfoStoreReadRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_PAGE_TYPE, 0x100, flash_test_buf, sizeof(flash_test_buf));
	
	    /* enable the key A GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* configure button pin as input */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	while(1)
	{
		osDelay(10);
		RunLogProcess();
		if(time_cnt++ >= 100)
		{
			time_cnt = 0;
			time_sec = !time_sec;
			if(time_sec)
			{
				fgv_DoWriteRoute(APP_LED_RUN_DO, SET);
			}
			else
			{
				fgv_DoWriteRoute(APP_LED_RUN_DO, RESET);
			}
			//LogPrintf(LVL_LOG_INFO, "Run Log Module Test >>>>>>>>>>>> %d", time_sec++);
		}
	}
}

void RunLogModuleInit()
{
	pQueueLogMsg = xQueueCreate(16, sizeof(STRU_LOG_MSG));

	if (!pQueueLogMsg)
		return;

	return;
}
