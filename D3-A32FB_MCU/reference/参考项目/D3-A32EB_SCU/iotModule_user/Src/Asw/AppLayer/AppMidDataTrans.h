#ifndef __TASK_MIDDATATRANS_H
#define __TASK_MIDDATATRANS_H

#include "AppHeaderSummary.h"
#include "tcp_gn.h"



void Get_SoftVersion_A(uint8_t *version);
void Get_HardVersion_A(uint8_t *version);

/* 获取网络信号 */
U8 GetNet_Comm_CSQ(void);
/* 获取网络信号强度 */
U8 GetNet_SignalLevel(void);
/* 获取网络信号dBm值 */
int8_t GetNet_SignaldBm(void);
/* 获取运营商 */
U32 GetNet_Comm_Operator(void);
/* 获取SIM卡号 */
U8 GetNet_Comm_SimID(U8 *p, U16 u16Size);



//获取平台服务器参数
void Get_PlatIServer(char *ser,uint16_t *port);
void Get_QrCodeString(uint8_t u8Port, char *str);
void Get_DevNumberString(char *pNum);
void Get_PlatNumberString(char *pNum);

void Set_PlatIpPort(char *ip, uint16_t port);
void Set_PlatType(uint8_t type);
//运营平台连接状态接口
void Set_PlatConnectSta(uint8_t sta);
uint8_t Get_PlatConnectSta(void);
void Set_PlatType(uint8_t type);


//获取启动方式
uint8_t Get_OrderTradeFlag(uint8_t u8Port);

//获取枪数量
uint8_t GetPile_CfgGunNum(void);
//获取是否即插即充状态
uint8_t GetPile_CfgOffLinChrg(void);
//获取国企标状态
uint8_t GetPile_CfgNationalStandard(void);

//获取cp value,0.01v
uint16_t GetPile_GunRtCpValue(uint8_t u8Port);
//获取pwm
uint16_t GetPile_GunChargingPwm(uint8_t u8Port);
//获取pwm en
uint16_t GetPile_GunPwnEn(uint8_t u8Port);
//获取relay out
uint16_t GetPile_GunRelayOut(uint8_t u8Port);

//获取按键状态
uint8_t Get_PileBtnSta(void);

//获取起止时间
void GetPile_ChargeBgEndTime(uint8_t u8Port, uint32_t *startStamp, uint32_t *stopStamp);


//充电桩版本号
void GetPile_StrSoftVer(char *strVer);
void GetPile_U8SoftVer(uint8_t *u8Ver);
//充电枪状态
CHARGE_STATE_E GetPile_gun_state(uint8_t u8Port);
//充电枪连接状态
uint8_t GetPile_gun_connect(uint8_t u8Port);
//获取输入电压
uint32_t GetPile_ChgInVol(void);
//获取输出电压
uint32_t GetPile_ChgOutVol(uint8_t u8Port, uint8_t point);
//获取最大允许充电总电压
uint16_t GetPile_ChgMaxVol(uint8_t u8Port);
//获取最大允许充电总电流
uint16_t GetPile_ChgMaxCurrent(uint8_t u8Port);
//获取最大允许温度
uint8_t GetPile_ChgMaxTemp(uint8_t u8Port);
//获取电流
uint32_t GetPile_ChgOutCur(uint8_t u8Port, uint8_t point);
//交流没枪温采样用插销和壳体温度代替
int8_t GetPile_GunTem(uint8_t u8Port);
//环境温度
int8_t GetPile_EvnTem(void);
//获取充电时间s
uint32_t GetPile_ChgTimer(uint8_t u8Port);
//获取充电电量
uint32_t GetPile_ChgTotalPower(uint8_t u8Port);
//获取起始充电量
uint32_t GetPile_ChgStartEle(uint8_t u8Port);
//获取终止充电量
uint32_t GetPile_ChgStoptEle(uint8_t u8Port);
//获取充电损耗电量
uint32_t GetPile_ChgTotalLossPower(uint8_t u8Port);
//获取充电金额
uint32_t GetPile_ChgTotalMoney(uint8_t u8Port);
//获取2位小数充电金额
uint32_t GetPile_ChgTotalMoneyDisplay(uint8_t u8Port);
//获取结算后账户余额
uint32_t GetPile_SettleAccountBalance(uint8_t u8Port);
//获取结算前账户余额
uint32_t GetPile_AccountBalance(uint8_t u8Port);
//获取充电卡号
void GetPile_ChgCarNumber(uint8_t u8Port, uint8_t *card_number);
//获取充电卡物理卡号
void GetPile_CardPhyNumber(uint8_t u8Port, uint8_t *card_number);
//获取充电卡逻辑卡号
void GetPile_CardLogNumber(uint8_t u8Port, uint8_t *card_number);

//平台设置总充电金额，用于显示或者其他，保持上报和显示一致
uint32_t GetPlat_ChgTotalMoney(uint8_t u8Port);
void SetPlat_ChgTotalMoney(uint8_t u8Port, uint32_t money);

//平台刷卡失败
uint8_t GetPlat_CardChargeFaild(uint8_t u8Port);
void SetPlat_CardChargeFaild(uint8_t u8Port, uint8_t reason);

//获取桩停止原因
uint16_t GetPile_ChgStopReason(uint8_t u8Port);
//充电桩是否有故障
uint8_t GetPile_ErrState(uint8_t u8Port);

//桩是否有故障，非枪
uint8_t Get_OnlayPileErr(void);
//获取充电桩急停状态
uint8_t Get_PileEstopBtnSta(void);
//充电桩故障信息
uint32_t GetPile_ErrInfo(uint8_t u8Port);

//获取桩是否空闲，非枪
uint8_t GetPile_Idlet(void);

//充电完成清除数据，或者上一单充电完成没拔枪直接再次启动，需要在启动前清除数据
void g_ClearChargingInfo(uint8_t u8Port);
//充电完成拔枪500ms后清除充电以及订单信息
void GunLeaveCarChargeDataClear(uint8_t u8Port, uint8_t CnctSta);
#endif

