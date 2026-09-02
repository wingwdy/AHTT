/**********************************************************************************************************
  * FileName         : AppRunLog.h
  * Author           : sjc
  * Version          : V1.0.0
  * Description      :
  * Date             : 2023.09.11 
**********************************************************************************************************/
#ifndef __APP_RUN_LOG_H_
#define __APP_RUN_LOG_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "UartRouteManage.h"

#define RUNLOG_RX_BUFF_SIZE      	INPUTBUF_LEN

typedef struct
{
    uint8_t aucCode[1];
    uint8_t aucTime[9];
    char acLogInfo[128 - 10];
}STRU_LOG_MSG;

typedef enum
{
    EV_LOG_ERR  = 0,
    EV_LOG_WARN = 1,
    EV_LOG_INFO = 2,
    EV_LOG_ALL  = 3,
}E_LOG;

typedef enum
{
    LVL_LOG_NONE = 0,
    LVL_LOG_ERR  = 1,
    LVL_LOG_WARN = 2,
    LVL_LOG_INFO = 4,
    LVL_LOG_ALL  = 0xFF,
}E_LVL_LOG;

#define DEBUG_ENABLE	1

enum {
	DBG_ERRO,	//错误信息
	DBG_INFO,	//调试信息
	DBG_DEBUG	//调试信息
};

#if DEBUG_ENABLE == 1
extern int g_debug;
#define DEBUG1		g_debug
// printf("[%07d]" fmt,(jiffies/1000),##args);

#define debug_printf(fmt,args...)		printf (fmt ,##args)
#define debug(fmt,args...)		printf (fmt ,##args)
#define debugL(level,fmt,args...)	\
		do {							\
			if (DEBUG1 >= level) {		\
				printf(fmt ,##args); 	\
			}							\
		} while(0)
#else
#define debug_printf(fmt,args...)
#define debug(fmt,args...)
#define debugL(level,fmt,args...)
#define printf(fmt,args...)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
void runLogTask(void* parameter);

void log_printf(uint8_t flag);
void hex_dump_printf(uint8_t flag);
// #include <stdarg.h> // 引入处理可变参数列表的头文件
void NeedOnPrintf(const char *format, ...);
void hex_dump(const char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

void LogPrintf(uint8_t ucEvLvl, const char *pcFormat, ...);
void RunLogModuleInit(void);
void RunLogTaskInit(void);

#ifdef __cplusplus
}
#endif

#endif
