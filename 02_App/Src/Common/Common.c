/******************************************************************************
* File Name          : Common.c
* Description        : Code for Common function
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
#include "Common.h"
#include "Mcal_Mcu.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define COMMON_IsLeapYear(year)   (((year) % 4 == 0 && (year) % 100 != 0) || (year) % 400 == 0)

/* CP56TIME2A格式定义 */
#define COMMON_CP56TIME2A_MS_LSB_OFFSET      0         /* 毫秒低字节偏移 */
#define COMMON_CP56TIME2A_MS_MSB_OFFSET      1         /* 毫秒高字节偏移 */
#define COMMON_CP56TIME2A_MINUTE_OFFSET      2         /* 分钟偏移 */
#define COMMON_CP56TIME2A_HOUR_OFFSET        3         /* 小时偏移 */
#define COMMON_CP56TIME2A_DAY_OFFSET         4         /* 日偏移 */
#define COMMON_CP56TIME2A_MONTH_OFFSET       5         /* 月偏移 */
#define COMMON_CP56TIME2A_YEAR_OFFSET        6         /* 年偏移 */

/* CP56TIME2A字段掩码 */
#define COMMON_CP56TIME2A_MINUTE_MASK        0x3F      /* 分钟掩码 (0-59) */
#define COMMON_CP56TIME2A_MINUTE_IV_MASK     0x40      /* 无效标志位 */
#define COMMON_CP56TIME2A_MINUTE_SU_MASK     0x80      /* 夏令时标志位 */
#define COMMON_CP56TIME2A_HOUR_MASK          0x1F      /* 小时掩码 (0-23) */
#define COMMON_CP56TIME2A_HOUR_IV_MASK       0x20      /* 无效标志位 */
#define COMMON_CP56TIME2A_HOUR_SU_MASK       0x80      /* 夏令时标志位 */
#define COMMON_CP56TIME2A_DAY_MASK           0x1F      /* 日掩码 (1-31) */
#define COMMON_CP56TIME2A_DAY_WDAY_MASK      0xE0      /* 星期几掩码 */
#define COMMON_CP56TIME2A_MONTH_MASK         0x0F      /* 月掩码 (1-12) */
#define COMMON_CP56TIME2A_MONTH_IV_MASK      0x10      /* 无效标志位 */
#define COMMON_CP56TIME2A_YEAR_MASK          0x7F      /* 年掩码 (0-99, 代表2000+年份) */
#define COMMON_CP56TIME2A_YEAR_IV_MASK       0x80      /* 无效标志位 */

/* CP56TIME2A质量标志位定义 */
#define COMMON_CP56TIME2A_MINUTE_IV          0x40      /* 分钟无效标志 */
#define COMMON_CP56TIME2A_MINUTE_SU          0x80      /* 分钟夏令时标志 */
#define COMMON_CP56TIME2A_HOUR_IV            0x20      /* 小时无效标志 */    
#define COMMON_CP56TIME2A_HOUR_SU            0x80      /* 小时夏令时标志 */
#define COMMON_CP56TIME2A_MONTH_IV           0x10      /* 月无效标志 */
#define COMMON_CP56TIME2A_YEAR_IV            0x80      /* 年无效标志 */

/*******************************************************************************
*    Enum Definition
*******************************************************************************/


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
/* Table of CRC values for high–order byte */
static const uint8_t c_CRCHighByte[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
} ;

/* Table of CRC values for low–order byte */
static const uint8_t c_CRCLowByte[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

static const uint8_t c_daysInMonth[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},  /* 非闰年 */
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}   /* 闰年 */
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t Common_IsLeapYear(uint16_t year)
{
    return COMMON_IsLeapYear(year) ? 1 : 0;
}

static uint8_t Common_DaysInMonth(uint16_t year, uint8_t month)
{
    if (month < 1 || month > 12)
    {
        return 0;
    }
    
    return c_daysInMonth[Common_IsLeapYear(year)][month - 1];
}

static uint32_t Common_DateToDays(uint16_t year, uint8_t month, uint8_t day)
{
    uint32_t days = 0;
    uint16_t y;
    
    // 从1970年开始计算
    for (y = 1970; y < year; y++)
    {
        days += Common_IsLeapYear(y) ? 366 : 365;
    }
    
    // 加上当年的月份天数
    for (y = 1; y < month; y++)
    {
        days += Common_DaysInMonth(year, y);
    }
    
    // 加上当月天数
    days += day - 1; // 减1是因为当天也要计算
    return days;
}

static void Common_DaysToDate(uint32_t days, CommonDateTime_Struct *dt)
{
    uint16_t year = 1970;
    uint8_t month = 1;
    uint8_t day = 1;
    
    // 计算年份
    while (days >= (Common_IsLeapYear(year) ? 366 : 365))
    {
        days -= (Common_IsLeapYear(year) ? 366 : 365);
        year++;
    }
    
    // 计算月份
    while (days >= Common_DaysInMonth(year, month))
    {
        days -= Common_DaysInMonth(year, month);
        month++;
    }
    
    // 计算日期
    day = (uint8_t)(days + 1);
    
    dt->year = year;
    dt->month = month;
    dt->day = day;
}


static uint8_t Common_GetCp56Time2aWeekday(uint32_t days)
{
    // 1970年1月1日是星期四, 在CP56TIME2A中索引为3 (在CP56TIME2A中是3，因为0表示周一)
    return (days + 3) % 7; 
}

uint32_t Common_DateTimeToTimestamp(CommonDateTime_Struct *dt)
{
    uint32_t timestamp = 0;
    uint32_t days = 0;
    
    // 计算日期部分的天数
    days = Common_DateToDays(dt->year, dt->month, dt->day);
    
    // 转换为时间戳 (天数 * 24小时 * 3600秒 + 小时 * 3600 + 分钟 * 60 + 秒)
    timestamp = days * 86400UL;
    timestamp += dt->hour * 3600UL;
    timestamp += dt->minute * 60UL;
    timestamp += dt->second;
    
    return timestamp;
}

void Conmon_TimestampToDateTime(uint32_t timestamp, CommonDateTime_Struct *dt)
{
    uint32_t days = timestamp / 86400UL;
    uint32_t remainder = timestamp % 86400UL;
    
    // 计算日期部分
    Common_DaysToDate(days, dt);
    
    // 计算时间部分
    dt->hour = (uint8_t)(remainder / 3600);
    remainder %= 3600;
    dt->minute = (uint8_t)(remainder / 60);
    dt->second = (uint8_t)(remainder % 60);
    dt->millisecond = 0; // 时间戳本身不包含毫秒
}

void Common_TimestampToCp56Time2a(uint32_t timestamp, uint8_t *cp56time2a)
{
    uint8_t qualityFlags = 0;
    CommonDateTime_Struct dt;
    uint16_t milliSec = 0; // 假设毫秒为0
    uint32_t days;

    // 将时间戳转换为日期时间结构
    Conmon_TimestampToDateTime(timestamp, &dt);

     // 计算从1970年1月1日到指定日期的天数
    days = Common_DateToDays(dt.year, dt.month, dt.day);
    
    // 填充CP56TIME2A格式
    // 毫秒 (低字节)COMMON_
    cp56time2a[COMMON_CP56TIME2A_MS_LSB_OFFSET] = (uint8_t)(milliSec & 0xFF);
    // 毫秒 (高字节)
    cp56time2a[COMMON_CP56TIME2A_MS_MSB_OFFSET] = (uint8_t)((milliSec >> 8) & 0xFF);
    // 分钟 (加上质量标志位)
    cp56time2a[COMMON_CP56TIME2A_MINUTE_OFFSET] = (dt.minute & COMMON_CP56TIME2A_MINUTE_MASK) | qualityFlags;
    // 小时 (加上质量标志位)
    cp56time2a[COMMON_CP56TIME2A_HOUR_OFFSET] = (dt.hour & COMMON_CP56TIME2A_HOUR_MASK) | qualityFlags;
    // 日 (加上星期几信息)
    cp56time2a[COMMON_CP56TIME2A_DAY_OFFSET] = (dt.day & COMMON_CP56TIME2A_DAY_MASK) | ((Common_GetCp56Time2aWeekday(days) & 0x07) << 5);
    // 月 (加上无效标志)
    cp56time2a[COMMON_CP56TIME2A_MONTH_OFFSET] = (dt.month & COMMON_CP56TIME2A_MONTH_MASK) | qualityFlags;
    // 年 (加上无效标志)
    cp56time2a[COMMON_CP56TIME2A_YEAR_OFFSET] = ((dt.year - 2000) & COMMON_CP56TIME2A_YEAR_MASK) | qualityFlags;
}

uint32_t Common_Cp56Time2aToTimestamp(const uint8_t *cp56time2a)
{
    CommonDateTime_Struct dt = {0};
    uint32_t timestamp;

    // 解析CP56TIME2A格式
    dt.millisecond = ((uint16_t)cp56time2a[COMMON_CP56TIME2A_MS_MSB_OFFSET] << 8) | 
                     cp56time2a[COMMON_CP56TIME2A_MS_LSB_OFFSET];
    dt.minute = cp56time2a[COMMON_CP56TIME2A_MINUTE_OFFSET] & COMMON_CP56TIME2A_MINUTE_MASK;
    dt.hour = cp56time2a[COMMON_CP56TIME2A_HOUR_OFFSET] & COMMON_CP56TIME2A_HOUR_MASK;
    dt.day = cp56time2a[COMMON_CP56TIME2A_DAY_OFFSET] & COMMON_CP56TIME2A_DAY_MASK;
    dt.month = cp56time2a[COMMON_CP56TIME2A_MONTH_OFFSET] & COMMON_CP56TIME2A_MONTH_MASK;
    dt.year = (cp56time2a[COMMON_CP56TIME2A_YEAR_OFFSET] & COMMON_CP56TIME2A_YEAR_MASK) + 2000;
    
    // 将日期时间转换为时间戳
    timestamp = Common_DateTimeToTimestamp(&dt);
    return timestamp;
}

void Common_SetSendEnable(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable)
{ 
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);

    if (pSendCtrl != NULL)
    {
        pSendCtrl->sendEnable = enable;
    }
}

uint8_t Common_GetSendEnable(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);
    uint8_t sendEnable = FALSE;

    if (pSendCtrl != NULL)
    {
        sendEnable = pSendCtrl->sendEnable;
    }

    return sendEnable;
}

void Common_SetSendFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t flag)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);

    if (pSendCtrl != NULL)
    {
        pSendCtrl->sendFlag = flag;
    }
}

uint8_t Common_GetSendFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);
    uint8_t sendFlag = FALSE;

    if (pSendCtrl != NULL)
    {
        sendFlag = pSendCtrl->sendFlag;
    }

    return sendFlag;
}

void Common_SetSendTick(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t tick)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);

    if (pSendCtrl != NULL)
    {
        pSendCtrl->cycTimer = tick;
    }
}

uint32_t Common_GetSendTick(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);
    uint32_t cycTimer = pSendCtrl->cycTimer;

    if (pSendCtrl != NULL)
    {
        cycTimer = pSendCtrl->cycTimer;
    }

    return cycTimer;
}

void Common_SetSendImmdFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t flag)
{
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);

    if (pSendCtrl != NULL)
    {
        pSendCtrl->immdFlag = flag;
    }
}

uint8_t Common_GetSendImmdFlag(typeFuncSendCtrl pFunc, uint8_t port, uint16_t cmd)
{ 
    CommonSendCtrl_Struct* pSendCtrl = pFunc(port, cmd);
    uint8_t immdFlag = FALSE;

    if (pSendCtrl != NULL)
    {
        immdFlag = pSendCtrl->immdFlag;
    }

    return immdFlag;
}

void Common_SetRecvEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable)
{ 
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->recvEnable = enable;
    }
}

void Common_SetRecvSeq(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t recvSeq)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->recvSeq = recvSeq;
    }
}

uint32_t Common_GetRecvSeq(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);
    uint32_t recvSeq = 0;

    if (pRecvCtrl != NULL)
    {
        recvSeq = pRecvCtrl->recvSeq;
    }

    return recvSeq;
}

void Common_SetRecvTimerEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint8_t enable)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->timerEnable = enable;
    }
}
uint8_t Common_GetRecvTimerEnable(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);
    uint8_t timerEnable = FALSE;

    if (pRecvCtrl != NULL)
    {
        timerEnable = pRecvCtrl->timerEnable;
    }

    return timerEnable;
}

void Common_SetRecvTick(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd, uint32_t tick)
{ 
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->cycTimer = tick;
    }
}

uint32_t Common_GetRecvTick(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);
    uint32_t cycTimer = pRecvCtrl->cycTimer;

    if (pRecvCtrl != NULL)
    {
        cycTimer = pRecvCtrl->cycTimer;
    }

    return cycTimer;
}

void Common_SetRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->rptCount++;
    }
}

void Common_ClearRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);

    if (pRecvCtrl != NULL)
    {
        pRecvCtrl->rptCount = 0;
    }
}

uint8_t Common_GetRptCount(typeFuncRecvCtrl pFunc, uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = pFunc(port, cmd);
    uint8_t rptCount = 0;

    if (pRecvCtrl != NULL)
    {
        rptCount = pRecvCtrl->rptCount;
    }

    return rptCount;
}
uint16_t Common_CalcCRC16(uint8_t *pData, uint16_t dataLen)
{
    uint8_t crcHi = 0xFF;                            
    uint8_t crcLo = 0xFF;                            
    uint8_t index;                                    

    while(dataLen--)                                  
    {
        index = crcHi ^ *pData++;                 
        crcHi = crcLo ^ c_CRCHighByte[index];
        crcLo = c_CRCLowByte[index];
    }

    return  (crcHi << 8) | crcLo;
}

/* 插入排序,从小到大 */
void Common_InsertSort(uint16_t *pData, uint16_t n)
{
	uint16_t i = 0,j = 0;
	uint16_t temp = 0;

	for( i = 1;i < n; i++ )
	{
		temp = pData[i]; 
		j = i;  
		while( j > 0 && temp < pData[j-1] )
		{ 
			pData[j] = pData[j-1];
			j--;
		}
		pData[j] = temp; 
	}
}

/* 获取系统时钟tick */
uint32_t Common_GetSystick(void)
{
	return Mcal_GetSystick();
}

/* 超时检测函数 */
uint8_t Common_JudgeTimeoutMs(uint32_t startTick, uint32_t threshold)
{
    uint32_t tickNow = Common_GetSystick();
	uint8_t ret = FALSE;

    if (tickNow >= startTick)
    {
        if ((tickNow - startTick) >= threshold)
        {
            ret = TRUE;
        }
    }
    else
    {
        if ((0xFFFFFFFFU - startTick + tickNow + 1) >= threshold)
        {
            ret = TRUE;
        }
    }

    return ret;
}


/* 冒泡排序,从小到大 */
void Common_BubbleSort(uint16_t *pData, uint16_t size)
{
    uint16_t i, j;
    uint16_t temp;

    for (j = 0; j < size; j++) //冒泡排序
    {
        for (i = 0; i < (size - j - 1); i++)
        {
            if (pData[i] > pData[i + 1])
            {
                temp = pData[i];
                pData[i] = pData[i + 1];
                pData[i + 1] = temp; //大的下沉到数组高端
            }
        }
    }
}

/* 中位值滤波法 */
uint16_t Common_MedianU16Filter(uint16_t *pData, uint16_t sample_num, uint16_t discard_num)
{
    uint16_t count = 0, sum = 0, ret = 0, tempDiscard = 0;

    if (sample_num < 3 || discard_num >= sample_num || pData == NULL)
    {
        ret = 0;
    }
    else
    {
        Common_BubbleSort(pData, sample_num);
        tempDiscard = (discard_num % 2 == 0) ? discard_num : (discard_num - 1);

        for (count = discard_num / 2; count < (sample_num - discard_num / 2); count++) 
        {
            sum +=  pData[count];
        }

        ret = sum / (sample_num - discard_num);
    }

    return ret;
}

void Common_Uint32ToFourUint8(uint8_t *pData, uint32_t CurValue)
{
    *pData++ = (CurValue & 0x00ff);
    *pData++ = ((CurValue >> 8) & 0x00ff);
    *pData++ = ((CurValue >> 16) & 0x00ff);
    *pData++ = ((CurValue >> 24) & 0x00ff);
}

void Common_Uint32ToTwoUint8(uint8_t *pData, uint32_t CurValue)
{
	*pData++ = (CurValue & 0x00ff);
	*pData++ = ((CurValue >>8 ) & 0x00ff);
}

uint32_t Common_FourUint8ToUint32(uint8_t *pData)
{
    uint32_t temp1, temp2, temp3, temp4;
    temp1 = *pData++;
    temp2 = *pData++;
    temp3 = *pData++;
    temp4 = *pData++;
    return ((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1);
}

uint16_t Common_TwoUint8ToUint16(uint8_t *pData)
{
    uint8_t temp1, temp2;
    temp1 = *pData++;
    temp2 = *pData++;
    return ((temp2 << 8) + temp1);
}

void Common_Uint16ToTwoUint8(uint8_t *pData, uint16_t curVal)
{
    *pData++ = (curVal & 0x00ff);
    *pData++ = ((curVal >> 8) & 0x00ff);
}

uint8_t* Common_SearchData(uint8_t *pData, uint16_t dataLen, void *pString, uint16_t stringLen)
{
    uint8_t *pTr = NULL;

	while(dataLen >= stringLen)
	{
		if (0 == memcmp(pData, pString, stringLen))
		{
			pTr = pData;
            break;
		}
		pData++;
		dataLen--;
	}

	return pTr;
}

uint16_t Common_ReplaceStr(uint8_t* pData, uint16_t nDataLen, char* cDestStr, void* pReplace, uint16_t nReplaceLen, char* pDefault)
{
	uint8_t *pDest = NULL;
	char cTailStr[100] = { 0 };
	void *pCopyData = pReplace;
	uint16_t nCopyLen = nReplaceLen;
	uint16_t nDestStrLen = strlen(cDestStr);
    uint16_t offset = 0;
    uint16_t tailLen = 0;

	pDest = Common_SearchData(pData, nDataLen, cDestStr, nDestStrLen);

	if (NULL != pDest)
	{
		offset = pDest - pData;

        // 计算尾部字符串长度，并确保不会溢出cTailStr缓冲区
        tailLen = nDataLen - offset - nDestStrLen;

        if (tailLen >= sizeof(cTailStr))
        {
            tailLen = sizeof(cTailStr) - 1; // 确保留出终止符空间
        }
        
        // 保存尾巴
        strncpy(cTailStr, (char*)(pDest + nDestStrLen), tailLen);
        cTailStr[tailLen] = '\0'; // 确保字符串终止

        //默认
        if (0 == nCopyLen)
        {
            pCopyData = pDefault;

            if (pDefault != NULL) 
            {
                nCopyLen = strlen(pDefault);
            } 
            else 
            {
                nCopyLen = 0;
            }
        }

        memcpy(pDest, pCopyData, nCopyLen);
        pDest += nCopyLen;

        // 加上尾巴
        strcpy((char*)pDest, cTailStr);

        // 重新计算长度
        nDataLen = offset + nCopyLen + strlen(cTailStr);
	}

	return nDataLen;
}

uint16_t Common_ReplaceNum(uint8_t* pData, uint16_t nDataLen, char* cDestStr, uint16_t replace, uint16_t defaultNum)
{
    uint16_t dataLen = 0;
    char cReplace[32] = { 0 };
    char cDefault[32] = { 0 };

    // 验证输入参数
    if (pData != NULL && cDestStr != NULL) 
    {
        snprintf(cReplace, sizeof(cReplace), "%d", replace);
        snprintf(cDefault, sizeof(cDefault), "%d", defaultNum);
        dataLen = Common_ReplaceStr(pData, nDataLen, cDestStr, cReplace, strlen(cReplace), cDefault);
    }

    return dataLen;
}

void Common_AsciiToBCD(char *pASC, uint8_t *pBCD, uint16_t length)
{
    uint16_t index;
    
    // 通过条件保护块替代提前return
    if (pASC != NULL && pBCD != NULL && length > 0)
    {
        for (index = 0; index < length; index += 2)
        {
            // ASCII合法性检查，非法则中断处理
            if (pASC[index] < '0' || pASC[index] > '9') 
            {
                break;
            }
            
            if (index + 1 < length)
            {
                if (pASC[index + 1] < '0' || pASC[index + 1] > '9') 
                {
                    break;
                }
                pBCD[index / 2] = ((pASC[index] - '0') << 4) | (pASC[index + 1] - '0');
            }
            else
            {
                // 奇数长度时高4位清零
                pBCD[index / 2] = (pASC[index] - '0') & 0x0F;  
            }
        }
    }
}

void Common_BCDToBIN(uint8_t *pBCD, uint8_t *pBIN, uint16_t length)
{ 
    uint16_t index = 0;

    if (pBCD != NULL && pBIN != NULL && length > 0)
    {
        for (index = 0; index < length; index++) 
        {
            pBIN[index] = (pBCD[index] / 16) * 10 + pBCD[index] % 16;
        }
    }
}

void Common_BINToBCD(uint8_t *pBIN, uint8_t *pBCD, uint16_t length)
{
    uint16_t index = 0;

    if (pBCD != NULL && pBIN != NULL && length > 0)
    {
        for (index = 0; index < length; index++) 
        {
            pBCD[index] = (pBIN[index] / 10) * 16 + pBIN[index] % 10;
        }
    }
}

uint8_t Common_GetBitFlag(void *pflag, uint16_t bitPos)
{
	uint8_t *p = (uint8_t *)pflag;
    uint8_t ret = FALSE;

	if((p[bitPos >> 3] & (1 << (bitPos & 0x07))) != 0)
	{
		ret =  TRUE;
	}

    return ret;
}


void Common_SetBitFlag(void *pflag, uint16_t bitPos)
{
    uint8_t *p = (uint8_t *)pflag;

    p[bitPos >> 3] |= (1 << (bitPos & 0x07));
}

void Common_ClrBitFlag(void *pflag, uint16_t bitPos)
{
    uint8_t *p = (uint8_t *)pflag;

    p[bitPos >> 3] &= (~(1 << (bitPos & 0x07)));
}

uint64_t Common_uintBINToBCD(uint32_t bin) 
{
    uint64_t bcd = 0;
    uint64_t recvBin = bin;
    uint8_t shift = 0;
    
    while (recvBin > 0) 
    {
        // 取出最低位的十进制数字，放到对应的4位BCD位置
        bcd |= (recvBin % 10) << (shift * 4);
        recvBin /= 10;      // 去掉已处理的最低位
        shift++;        // 移动到下4个bit位置
    }
    
    return bcd;
}

/**
 * @brief 一个数据值(半个字节：如 0xE->'E')
 * @param[in] hexHalfByte:转换对象数据(半字节数据)
 * @return 半字节数据的Ascii码表示
 * @note
 * @attention
 */
uint8_t Common_CvtHex2AsciiHalfByte(uint8_t hexHalfByte)
{
    uint8_t retAscii = 0;

    if (hexHalfByte >= 0x0A && hexHalfByte <= 0x0F)
    {
        retAscii = hexHalfByte + 0x37;
    }
    else if (hexHalfByte <= 0x09)
    {
        retAscii = hexHalfByte + 0x30;
    }
    else
    {}

    return retAscii;
}

/**
 * @brief 十六进制数据 =》 Ascii码表示数据（2字节） exp) 0x2D => "2D"
 * @param[in] hexData:一个字节
 * @param[out] pAsciiData:转换后的Ascii码数据
 * @return 无
 * @note
 * @attention
 *   1字节数据，低4位，在右，高4位在左，所以要对调。 如:0(高4位） 2（低4位）
 */
void Common_CvtHex2Ascii(uint8_t hexData, uint8_t* pAsciiData)
{
    if (pAsciiData)
    {
        pAsciiData[1] = Common_CvtHex2AsciiHalfByte(0x0F & hexData);
        pAsciiData[0] = Common_CvtHex2AsciiHalfByte(hexData >> 4);
    }
}