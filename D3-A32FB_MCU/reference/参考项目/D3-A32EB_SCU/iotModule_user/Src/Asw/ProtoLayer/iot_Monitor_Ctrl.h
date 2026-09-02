#ifndef __IOT_MONITOR_CTRL_H__
#define __IOT_MONITOR_CTRL_H__
#include "globals.h"
#include "AppHeaderSummary.h"

void OM_SetSendTick(uint8_t u8Port, uint32_t cmd, int32_t tick);
void OM_SetSendEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);
uint8_t OM_GetSendEnable(uint8_t u8Port, uint32_t cmd);
void OM_Send_Immediately(uint8_t u8Port, uint32_t cmd);
void OM_SetSendImmdFlag(uint8_t u8Port, uint32_t cmd, uint8_t Flag);
uint8_t OM_GetSendImmdFlag(uint8_t u8Port, uint32_t cmd);
int32_t OM_GetSendTick(uint8_t u8Port, uint32_t cmd);
void OM_SetSendFlag(uint8_t u8Port, uint32_t cmd, uint8_t flag);
//upsrm
void OM_SetSendSrm(uint8_t u8Port, uint32_t cmd, uint16_t Srm);
uint16_t OM_GetSendSrm(uint8_t u8Port, uint32_t cmd);


void OM_SetRecvTick(uint8_t u8Port, uint32_t cmd, uint32_t tick);
int32_t OM_GetRecvTick(uint8_t u8Port, uint32_t cmd);
void OM_SetRecvEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);
uint8_t OM_GetRecvEnable(uint8_t u8Port, uint32_t cmd);



#endif
