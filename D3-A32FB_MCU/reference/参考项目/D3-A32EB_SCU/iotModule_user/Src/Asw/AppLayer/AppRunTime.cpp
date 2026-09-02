/**********************************************************************************************************
  * FileName         : AppRunTime.cpp
  * Author           : sjc
  * Version          : V1.0.0
  * Description      : 
  * Date             : 2023.09.11 
**********************************************************************************************************/
#include "AppHeaderSummary.h"
#include "mbsMaster.h"

static uint32_t g_ulRunTimeTick = 0;
static uint32_t g_ulRunTimeS = 0;
static uint32_t g_sysTick = 0;
static uint32_t g_realTime = 0;

uint8_t GetRealTimeSucces()
{
    return g_realTime;
}
/**********************************************************************************************************
  * Function            :
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void RunTimeTickInc(void)
{
	g_ulRunTimeTick ++;
	g_sysTick ++;
    if (g_ulRunTimeTick % USER_TICK_PER_SECOND == 0) {
        g_ulRunTimeS++;
    }
}

// 辅助函数，判断是否为闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 计算出来的时间需要减去8小时的时差，估计是以1970年8点开始计算时间的
void timToStamp(uint32_t *pStamp, tm_struct *pucTime)
{
	static int MON1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};	//平年
	static int MON2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};	//闰年
	int *month = NULL;
	int leapYearCnt = 0;
	uint32_t days = 0;
	int year = 2000 + pucTime->yearL;
	uint32_t stamp = 0;
	//获得1970年到当前年的前一年共有多少闰天
	for(int i = 1970; i < year; i++)
	{
		if((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0))
		{
			leapYearCnt++;
		}
	}
	days = leapYearCnt * 366 + (year - 1970 -leapYearCnt) * 365;
	/*判断当前年是不是闰年*/
	if(is_leap_year(year)) {
		month = MON2;
	} else {
		month = MON1;
	}
	
	for(int i = 0; i < pucTime->month - 1; i++)
	{
		days += month[i];
	}
	
    days += pucTime->day - 1;
	// *pStamp = (days + clock.day-1) * 24 * 3600 * 1000 + clock.hour * 3600 * 1000 + clock.minute * 60 * 1000 + clock.second * 1000/* + clock.ms - 8 * 3600 * 1000*/;
	stamp = days * 24 * 3600 + pucTime->hour * 3600 + pucTime->minute * 60 + pucTime->second;
	*pStamp = stamp - 28800;   //需要减去8小时时区时差
}

// 一个简化的函数来将Unix时间戳转换为日期时间结构体
// 注意：这个实现非常基础，没有考虑闰年等复杂情况，且没有时区转换
void stampToTime(tm_struct* dt, uint32_t timestamp)
// void timestampToDateTime(uint32_t timestamp, DATETIME_T *dt, int timezoneOffset) 
{
    uint32_t totalSeconds = timestamp + 8 * 3600;
 
    // 计算年份
    uint32_t l_year = 1970;
    
    while (totalSeconds >= (31536000UL + (is_leap_year(l_year) ? 86400 : 0)))
    {
        totalSeconds -= 31536000UL + (is_leap_year(l_year) ? 86400 : 0);
        l_year++;
    }
    
    dt->yearH = l_year / 100;
    dt->yearL = l_year % 100;
 
    // 计算月份和日期
    uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap_year(l_year)) 
    {
        daysInMonth[1] = 29;  // 闰年2月有29天
    }
    dt->month = 0;
    while (totalSeconds >= (daysInMonth[dt->month] * 86400UL)) 
    {
        totalSeconds -= daysInMonth[dt->month] * 86400UL;
        dt->month++;
    }
    dt->month++;
    
    dt->day = totalSeconds / 86400UL + 1;
 
    // 计算时分秒
    dt->hour = (totalSeconds % 86400) / 3600;
    dt->minute = (totalSeconds % 3600) / 60;
    dt->second = totalSeconds % 60;
}


/**********************************************************************************************************
  * Function            :
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void getRunTime(uint8_t *timeBuf)
{
    uint32_t ulMS = 0;
	stampToTime((tm_struct* )timeBuf, g_ulRunTimeS);
    
    ulMS = g_ulRunTimeTick % USER_TICK_PER_SECOND;
    ulMS *= 1000;
    ulMS /= USER_TICK_PER_SECOND;
    timeBuf[7] = (uint8_t)ulMS;
    timeBuf[8] = (uint8_t)(ulMS>>8);
}

/**********************************************************************************************************
  * Function            :
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void getRunTimeYYMDHMS(uint8_t *timeBuf)
{
    uint32_t ulMS = 0;
	stampToTime((tm_struct* )&timeBuf[0], g_ulRunTimeS);
}

/**********************************************************************************************************
  * Function            : 
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
uint32_t getRunTimeS()
{
	return g_ulRunTimeS;
}

/**********************************************************************************************************
  * Function            : 
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void setRunTime(uint32_t ulSecond, uint8_t ucType, uint8_t ucRange)
{
    if(0 == ucType || (ulSecond - g_ulRunTimeS > ucRange || g_ulRunTimeS - ulSecond > ucRange))
    {
        g_ulRunTimeS = ulSecond;
        if(ucType != 0)
        {
            uint8_t aucTime[7] = {0};
            stampToTime((tm_struct*)aucTime, ulSecond);
        }
		//
		{
			uint8_t aucInfo[9] = {0};
			getRunTime(aucInfo);
		}
    }
}

/**********************************************************************************************************
  * Function            : 
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void setRunTime1(uint8_t *timeBuf, uint8_t ucType = 0, uint8_t ucRange = 120)
{
	uint32_t ulSecond = 0;
	timToStamp(&ulSecond, (tm_struct*)timeBuf);
	setRunTime(ulSecond, ucType, ucRange);
}

/**********************************************************************************************************
  * Function            : 
  * Description         : 设置当前时间
  * Input               : 时间戳
  * Output              : 
  * Note(s)             : 
  * Contributor         : 
  * Date                : 2023.09.11
**********************************************************************************************************/
void setCurrentRunTimeStamp(uint32_t timeStamp)
{
    uint8_t ucType = 0;
    uint8_t ucRange = 10;
    //年年月时分秒
	uint32_t ulSecond = 0;
	setRunTime(timeStamp, ucType, ucRange);
    
    g_realTime = 1;
    //设置桩时间
	fgv_PileSetTime();
}

/**********************************************************************************************************
  * Function            : 
  * Description         : 
  * Input               : V1.0.0
  * Output              : 
  * Note(s)             : 
  * Contributor         : sjc
  * Date                : 2023.09.11
**********************************************************************************************************/
void setCurrentRunTime(uint8_t *timeBuf)
{
    uint8_t ucType = 0;
    uint8_t ucRange = 10;
    //年年月时分秒
	uint32_t ulSecond = 0;
	timToStamp(&ulSecond, (tm_struct*)timeBuf);
	setRunTime(ulSecond, ucType, ucRange);

    g_realTime = 1;
    //设置桩时间
	fgv_PileSetTime();
}

uint32_t Get_Systick(void)
{
    return g_sysTick;
}

tm_struct SysTime_gs;
tm_struct get_current_time()
{
    stampToTime(&SysTime_gs, g_ulRunTimeS);
	return SysTime_gs;
}

#define MAX_DELAY      0xFFFFFFFFUL

uint8_t JudgeTimeOutMs(uint32_t startTick, uint32_t Threshold)
{
    uint32_t tickNow = NOWTICK;

    if (tickNow >= startTick)
    {
        if ((tickNow - startTick) >= Threshold)
        {
            return TRUE;
        }
    }
    else
    {
        if ((MAX_DELAY - startTick + tickNow + 1) >= Threshold)
        {
            return TRUE;
        }
    }

    return FALSE;
}
