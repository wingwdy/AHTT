/******************************************************************************
* File Name          : Common.h
* Description        : Code for Common function
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
#ifndef COMMON_H_
#define COMMON_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "gd32e50x.h"
#include "Global.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
	uint32_t cycTimer;		/* 周期计时器 */ 
	uint8_t immdFlag;		/* 立即发送标志 */
	uint8_t sendEnable;		/* 发送使能 */
	uint8_t sendFlag;		/* 发送标记 */
	uint32_t sendSeq;		/* 序列号 */
}CommonSendCtrl_Struct;

typedef struct 
{
	uint32_t cycTimer;      /* 周期计时器 */
	uint8_t recvEnable;     /* 接收使能 */
	uint8_t timerEnable;    /* 接收超时计时使能 */
	uint8_t recvFlag;       /* 已接收标记 */
    uint8_t rptCount;       /* 重试次数 */
    uint32_t recvSeq;       /* 序列号 */
}CommonRecvCtrl_Struct;

/* 日期时间结构体 */
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
}CommonDateTime_Struct;

typedef union
{
    uint8_t data[7];      /* 7字节数组格式 */
    struct
    {
        uint16_t millisecond    : 16;   /* 毫秒 (0-59999) */
        uint8_t  minute         : 6;    /* 分钟 (0-59) */
        uint8_t  minute_iv      : 1;    /* 分钟无效标志 */
        uint8_t  minute_su      : 1;    /* 分钟夏令时标志 */
        uint8_t  hour           : 5;    /* 小时 (0-23) */
        uint8_t  hour_iv        : 1;    /* 小时无效标志 */
        uint8_t  hour_reserved  : 1;    /* 保留位 */
        uint8_t  hour_su        : 1;    /* 小时夏令时标志 */
        uint8_t  day            : 5;    /* 日期 (1-31) */
        uint8_t  weekday        : 3;    /* 星期几 (0=周一, 1=周二, ..., 6=周日) */
        uint8_t  month          : 4;    /* 月份 (1-12) */
        uint8_t  month_iv       : 1;    /* 月无效标志 */
        uint8_t  month_reserved : 3;    /* 保留位 */
        uint8_t  year           : 7;    /* 年 (0-99, 代表2000+年份) */
        uint8_t  year_iv        : 1;    /* 年无效标志 */
    } field;                            /* 按字段访问 */
} Cp56Time2a_Struct;

typedef CommonSendCtrl_Struct* (*typeFuncSendCtrl)(uint8_t port, uint16_t cmd);
typedef CommonRecvCtrl_Struct* (*typeFuncRecvCtrl)(uint8_t port, uint16_t cmd);
/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void Common_SetSendEnable(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable);
uint8_t Common_GetSendEnable(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetSendFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t flag);
uint8_t Common_GetSendFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetSendTick(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t tick);
uint32_t Common_GetSendTick(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetSendImmdFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t flag);
uint8_t Common_GetSendImmdFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetRecvEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable);
void Common_SetRecvSeq(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t recvSeq);
uint32_t Common_GetRecvSeq(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);
uint8_t Common_GetRecvTimerEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetRecvTimerEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable);
uint32_t Common_GetRecvTick(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_SetRecvTick(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t tick);
void Common_SetRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);
void Common_ClearRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);
uint8_t Common_GetRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd);

uint16_t Common_CalcCRC16(uint8_t *pData, uint16_t dataLen);
void Common_InsertSort(uint16_t *pData, uint16_t n);
void Common_BubbleSort(uint16_t *pData, uint16_t size);
uint8_t Common_JudgeTimeoutMs(uint32_t startTick, uint32_t threshold);
uint16_t Common_MedianU16Filter(uint16_t *pData, uint16_t sample_num, uint16_t discard_num);
uint32_t Common_GetSystick(void);
void Common_Uint32ToFourUint8(uint8_t *pData, uint32_t curVal);
void Common_Uint32ToTwoUint8(uint8_t *pData, uint32_t curVal);
uint32_t Common_FourUint8ToUint32(uint8_t *pData);
uint16_t Common_TwoUint8ToUint16(uint8_t *pData);
void Common_Uint16ToTwoUint8(uint8_t *pData, uint16_t curVal);

uint8_t* Common_SearchData(uint8_t *pData, uint16_t dataLen, void *pString, uint16_t stringLen);
uint16_t Common_ReplaceStr(uint8_t* pData, uint16_t nDataLen, char* cDestStr, void* pReplace, uint16_t nReplaceLen, char* pDefault);
uint16_t Common_ReplaceNum(uint8_t* pData, uint16_t nDataLen, char* cDestStr, uint16_t replace, uint16_t defaultNum);

uint32_t Common_DateTimeToTimestamp(CommonDateTime_Struct *dt);
void Common_TimestampToDateTime(uint32_t timestamp, CommonDateTime_Struct *dt);
void Common_TimestampToCp56Time2a(uint32_t timestamp, uint8_t *cp56time2a);
uint32_t Common_Cp56Time2aToTimestamp(const uint8_t *cp56time2a);

void Common_AsciiToBCD(char *pASC, uint8_t *pBCD, uint16_t length);
void Common_BCDToBIN(uint8_t *pBCD, uint8_t *pBIN, uint16_t length);
void Common_BINToBCD(uint8_t *pBIN, uint8_t *pBCD, uint16_t length);
uint64_t Common_uintBINToBCD(uint32_t bin);

void Common_SetBitFlag(void *pflag, uint16_t bitPos);
void Common_ClrBitFlag(void *pflag, uint16_t bitPos);
uint8_t Common_GetBitFlag(void *pflag, uint16_t bitPos);
void Common_CvtHex2Ascii(uint8_t hexData, uint8_t* pAsciiData);

int32_t Common_ExtractPathAndFileName(const char *input, 
                                       char *path, uint32_t pathSize,
                                       char *filename, uint32_t nameSize);

#endif /* COMMON_H_ */


























