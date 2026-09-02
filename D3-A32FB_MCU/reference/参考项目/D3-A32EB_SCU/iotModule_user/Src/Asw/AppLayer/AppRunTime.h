/**********************************************************************************************************
  * FileName         : AppRunTime.h
  * Author           : sjc
  * Version          : V1.0.0
  * Description      :
  * Date             : 2023.09.11
**********************************************************************************************************/
#ifndef __APP_RUN_TIME_H_
#define __APP_RUN_TIME_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "RouteHeaderSummary.h"

#define USER_TICK_PER_SECOND	1000

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint8_t yearH;
    uint8_t yearL;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} tm_struct;

extern uint8_t GetRealTimeSucces(void);
extern void RunTimeTickInc(void);


void timToStamp(uint32_t *pStamp, tm_struct *pucTime);
void stampToTime(tm_struct* dt, uint32_t timestamp);

extern uint32_t Get_Systick(void);
tm_struct get_current_time(void);
extern uint8_t JudgeTimeOutMs(uint32_t startTick, uint32_t Threshold);
#define NOWTICK		Get_Systick()
extern void getRunTime(uint8_t *timeBuf);
extern uint32_t getRunTimeS(void);
void getRunTimeYYMDHMS(uint8_t *timeBuf);
//
extern void setRunTime(uint32_t ulSecond, uint8_t ucType, uint8_t ucRange);
extern void setRunTime1(uint8_t *timeBuf, uint8_t ucType, uint8_t ucRange);

void setCurrentRunTime(uint8_t *timeBuf);
void setCurrentRunTimeStamp(uint32_t timeStamp);
#ifdef __cplusplus
}
#endif

#endif