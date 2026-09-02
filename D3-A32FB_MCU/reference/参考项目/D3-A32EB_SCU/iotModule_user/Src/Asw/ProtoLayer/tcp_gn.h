#ifndef __TCP_GN_H__
#define __TCP_GN_H__



/*-----------------------------------------------------------------------------
Section: 工作状态枚举
----------------------------------------------------------------------------*/
typedef enum
{
    eChargeState_Idle            = 0,              // 空闲状态
    eChargeState_Waiting         = 1,              // 启动中状态
    eChargeState_Starting        = 2,              // 启动中状态
    eChargeState_Charging        = 3,              // 充电中状态
    eChargeState_PauseB          = 4,              // 暂停B状态
    eChargeState_Stoping         = 5,              // 停止中状态
    eChargeState_StopFinish      = 6,              // 停止完成状态
}CHARGE_STATE_E;


//所有状态必须从eMonitorState_Service进入,
//除eMonitorState_Service外所有状态必须被监视,并有一个超时时间
typedef enum
{
    eMonitorState_Service       =  0, 		// 服务状态
    eMonitorState_Auth       	=  1,  		// 鉴权状态,可以合并到Service状态
    eMonitorState_Apoint       	=  2,  		// 预约状态
    eMonitorState_Forbid       	=  3, 		// 禁用状态
    eMonitorState_UpState      	=  4,   	// 升级状态   
}MONITOR_STATE_E;


typedef enum {
    eDetectMode_Auto,
    eDetectMode_Time,
    eDetectMode_Count,
    eDetectMode_Ele,
} eDetectMode;


#ifdef __cplusplus
extern "C" {
#endif

#include "protocol_ctrl.h"


uint8_t monitor_getDevName(uint8_t *pName, uint8_t len);
uint8_t monitor_getDevNumber(uint8_t *pNum, uint8_t len);
int32_t monitor_getGunTem(uint8_t u8Port);
uint32_t monitor_getChgTimer(uint8_t u8Port);
uint32_t monitor_getChgTotalEnergy(uint8_t u8Port);
uint32_t monitor_getChgTotalPower(uint8_t u8Port);
uint32_t monitor_getChgStoptEle(uint8_t u8Port);
uint32_t monitor_getChgTotalLossPower(uint8_t u8Port);
uint32_t monitor_getChgTotalMoney(uint8_t u8Port);
uint16_t get_hard_err_bit(uint8_t u8Port);
CHARGE_STATE_E logic_get_gun_state(uint8_t u8Port);
uint8_t logic_get_gun_charging(uint8_t u8Port);
uint8_t logic_get_gun_pwmEnable(uint8_t u8Port);
uint8_t logic_get_gun_Uncharged(uint8_t u8Port);
uint8_t logic_get_gun_Stanby(uint8_t u8Port);
uint8_t logic_get_gun_StopFinish(uint8_t u8Port);
uint8_t dev_getErrState(uint8_t u8Port);
uint8_t stopPileCharge(uint8_t u8Port, uint16_t reason);
uint16_t gnUpdate_stopReason(uint8_t u8Port);
uint16_t ykcV21Update_stopReason(uint8_t u8Port);
uint8_t UpOnlineFlag();
uint8_t monitor_charge_stop(uint8_t u8Port);
void DB_UpOfflineDeal(void);
void UpOfflineDeal(eNetSocket SocketID);

void updatePileStopReason(uint8_t u8Port, uint16_t reason);  //更新停止原因

MONITOR_STATE_E monitor_get_MonitorState(uint8_t u8Port);
void monitor_set_MonitorState(uint8_t u8Port, MONITOR_STATE_E state);
void monitor_chgrcd_report_succ(void);


void ChargeConnectGunTimeScan(uint8_t u8Port);
void StartingTimeOutScan(uint8_t u8Port);
void PauseTimeOutScan(uint8_t u8Port);
void ChargeConnectGunWaitCardScan(uint8_t u8Port);
uint8_t monitor_charge_start(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney);


uint8_t monitor_getTradeFlag(uint8_t u8Port);
uint8_t monitor_getChgCardNum(uint8_t u8Port, uint8_t *pNum, uint8_t len);


void ChargingRecordUpdateScan(uint8_t u8Port);
uint8_t GNUpChargeRecordStorage(uint8_t u8Port);
uint8_t GNUpChargeRecordJudgestart(uint8_t u8Port);
void DetectAbnormalScan(uint8_t u8Port);
uint8_t GNUpChargeRecordUpDealOffline(void);
void ChargeFaultActiveStop(uint8_t u8Port);

uint8_t UpChargeRecordUpDealOffline(uint8_t u8Port);
uint8_t GNUpChargeStorageDeal(uint8_t u8Port, void *deal, uint16_t reason);

void SetDetectChargeMaxTime(uint8_t u8Port, uint32_t param);
void SetDetectModeParam(uint8_t u8Port, eDetectMode mode, uint32_t param);

void g_CardInfoClear(uint8_t u8Port);

#ifdef __cplusplus
}
#endif


#endif

