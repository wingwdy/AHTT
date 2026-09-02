#ifndef __MBSMASTER_H__
#define __MBSMASTER_H__


#include "FunctionalHeaderSummary.h"
#include "AppHeaderSummary.h"



#ifdef __cplusplus
extern "C" {
#endif

extern void MbsMasterMod_Init(void);
extern void MbsMasterTaskInit(void);

//获取网络单元是否异常
uint8_t fgv_GetMbsCommSta(void);


void MbsMasteSendData(uint8_t *MbsFileData, uint16_t len);
//开始充电
void fgv_CtrlStartCharge(uint8_t ch);
//停止充电
void fgv_CtrlStopCharge(uint8_t ch);
//功率控制
void fgv_CtrlChargeCrt(uint8_t ch, uint16_t crt);
//设置需要进入厂内模式
void fgv_SetPileCfgFac(uint8_t mode);
//设置设备为即插即充
void fgv_SetPileCfgOffLinChrg(uint8_t mode);
//设置时间
void fgv_PileSetTime(void); 

extern uint8_t fgv_get_gun_startCmd(uint8_t ch);
extern void fgv_clr_gun_startCmd(uint8_t ch);

//控制充电桩重启、恢复出场设置等操作
void fgv_CtrlPileOpr(uint8_t cmd);

void MbsMasterModSendTaskMain(void);
void MbsMasterModRecvTaskMain(void);
#ifdef __cplusplus
}
#endif

#endif
