/***********************************************************************************
 * 文 件 名  : iot_AHTT_Protocol.c
 * 版 本 号  : V0.1
 * 负 责 人  : JXY
 * 创建日期  : 2025-03-18
 * 文件描述  : 安徽铁塔低速
 * 版权说明  : 
 * 函数列表  :    
 * 其    他  :
 * 修改日志  :
***********************************************************************************/
#include "iot_AHTT_Protocol_Code.h"
#include "protocol_data.h"
#include "maths.h"
#include "mbsMaster.h"
#include "common.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "cost.h"
#include "AppDealFlash.h"

void Reverse_order(uint8_t *output, int len);

static AHTT_UpPlatInfo s_AhttUpInfo;
static AHTT_FlashPlatInfo s_AhttFlashInfo;      //需要存储的数据

// #define AHTTDATA_REALTIME_TIMEINTER   	    2      	// 上报时间间隔，上报数据需要用，按照分钟计算升级电量剩余时间等

#define AHTT_MAX_CHARGETIME   10                                               //充电默认最大时长，10h

#define AHTT_HEART_PERIOD_MIN   1                                               //心跳默认周期,分钟
// #define AHTT_HEART_PERIOD       (AHTT_HEART_PERIOD_MIN*eTick_60S)               //心跳默认周期，s
#define AHTT_0x93_PERIOD_MIN    2                                           //通道数据上报默认周期,分钟
// #define AHTT_0x93_PERIOD        (AHTT_0x93_PERIOD_MIN * eTick_60S)       //通道数据上报默认周期，s
#define AHTT_CARD_PERMNY        10                                              //刷卡一次默认消费金额，单位0.1元
#define AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE            1         /* 刷卡未鉴权立即启动充电功能使能，第一次刷卡超时XX秒后向平台鉴权 */
#define AHTT_CFG_SWIPE_CARD_UNAUTH_CARD_VIRTUAL_BALANCE     100       /* 刷卡未鉴权立即启动充电，预设的卡内虚拟余额 */
#define AHTT_CFG_SWIPE_CARD_VALID_TIME                      eTick_10S /* 刷卡有效时间 */
#define AHTT_CFG_SWIPE_CARD_INIT_STATE                      0
#define AHTT_CFG_SWIPE_CARD_READY_STATE                     1
#define AHTT_CFG_SWIPE_CARD_AUTH_STATE                      2


AHTT_UpPlatInfo *Get_AhttAllInfo()
{
    return &s_AhttUpInfo;
}

//给充电部分那边进行参数设置，设置最大时长
void AHTT_SetChargePara_MaxTime(uint8_t para)
{
    printf("AHTT_Max time %d\r\n", para);
    for (uint8_t i = 0; i < GUN_NUM; i++) {
        SetDetectChargeMaxTime(i, para * 60);
    }
}

typedef struct
{
    uint32_t		gnReason;
	uint32_t		PlatReason;
}AHTT_StopReasonMap;

// 安徽铁塔故障对应表
const AHTT_StopReasonMap StrAHTTStopReasonMap[] = {
	{Pile_Stop_Reason_OverSum	        ,E_AHTT_StopReason_MnyOver          },
	{Pile_Stop_Reason_CarOk	            ,E_AHTT_StopReason_FullAuto          },
	{Pile_Stop_Reason_CrtUnder	        ,E_AHTT_StopReason_LoadOver          },
	{Pile_Stop_Reason_GunBreak	        ,E_AHTT_StopReason_DrawGun          },
	{Pile_Stop_Reason_EStop	            ,E_AHTT_StopReason_Estop          },
	{Pile_Stop_Reason_S2TimeOut	        ,E_AHTT_StopReason_StartFail          },
	{Pile_Stop_Reason_OverTime	        ,E_AHTT_StopReason_TimeOver          },
	{Pile_Stop_Reason_OverEle	        ,E_AHTT_StopReason_EleOver          },
	{Pile_Stop_Reason_PEGnd		        ,E_AHTT_StopReason_PeFault          },
	{Pile_Stop_Reason_VolOver	        ,E_AHTT_StopReason_OverVol          },
	{Pile_Stop_Reason_VolUnder	        ,E_AHTT_StopReason_UnderVol          },
	{Pile_Stop_Reason_APP	            ,E_AHTT_StopReason_RmtFns          },
	{Pile_Stop_Reason_Card	            ,E_AHTT_StopReason_CardStop          },
	{Pile_Stop_Reason_Leak		        ,E_AHTT_StopReason_LeakStop          },
	{Pile_Stop_Reason_GunTempOver		,E_AHTT_StopReason_RelayTp          },
	{Pile_Stop_Reason_CPGnd		        ,E_AHTT_StopReason_VolFault          },
	{Pile_Stop_Reason_CPErro		    ,E_AHTT_StopReason_VolFault          },
	{Pile_Stop_Reason_PlugTempOver		,E_AHTT_StopReason_NLTemp          },
	{Pile_Stop_Reason_PwOff		        ,E_AHTT_StopReason_Power          },
	{Pile_Stop_Reason_None		        ,E_AHTT_Unknow          },
	{Pile_Stop_Reason_Other		        ,E_AHTT_Other          },
};


//一个字节内按位倒序
uint8_t reverse_bits(uint8_t byte) 
{
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 1;           // 左移一位，腾出最低位
        result |= (byte & 1);    // 将 byte 的最低位赋给 result
        byte >>= 1;             // byte 右移一位，处理下一个 bit
    }
    return result;
}


//清除卡数据
void AhttClearCardData(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];
    memset(pCardData, 0, sizeof(AHTT_CardStartData));
}

//订单数据更新
static void ahtt_packStopReasonChgRecord(uint8_t u8Port, AHTT_charge_record *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    U8 pileReason = pChgGunData->DealRecord.PileStopReason;
	const AHTT_StopReasonMap *pAHTTStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrAHTTStopReasonMap); u32i++) {
        pAHTTStopMap = &StrAHTTStopReasonMap[u32i];
        if (pileReason == pAHTTStopMap->gnReason) {
            pRecord->stop_reason = pAHTTStopMap->PlatReason;  
            printf("ahtt_stop reason: %d %d\r\n", pAHTTStopMap->gnReason, pAHTTStopMap->PlatReason);
            return;
        }
    }
    printf("Else OgrReason: %d\r\n", pileReason);
    pRecord->stop_reason = pileReason + 0x50;  
}

uint8_t ahtt_packChgRecord(uint8_t u8Port, AHTT_charge_record *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	uint16_t u16Data = 0;/* 剩余金额 */
    uint32_t u32Data = 0;
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

    if ((pAhttUpInfo->CardStartData[u8Port].CardAuthIng != AHTT_CFG_SWIPE_CARD_INIT_STATE) \
      && (eUP_Start_Style_CardOnline == monitor_getTradeFlag(u8Port)))
    {/* 刷卡启动充电后，未向平台鉴权或鉴权成功 */
        return FALSE;
    }

    ahtt_packStopReasonChgRecord(u8Port, pRecord);
	pRecord->gun_num = u8Port;

    u32Data = pcostdata->total_power / 10;
    pRecord->total_power[0] = u32Data & 0xFF;
    pRecord->total_power[1] = (u32Data >> 8) & 0xFF;
    pRecord->total_power[2] = u32Data >> 16;

    if (pAhttUpInfo->chargeMode_4D == E_AHTT_ChargeMode_Full) 
    {
        u16Data = 0;
    } 
    else 
    {
        u16Data = pChgGunData->sum_balance - pcostdata->allEleMoney / 100 - pcostdata->allServerMoney / 100;
    }
    pRecord->Remaining_sum[0] = u16Data & 0xFF;
    pRecord->Remaining_sum[1] = u16Data >> 8;

    u16Data = pcostdata->allEleMoney / 100;
    pRecord->total_money[0] = u16Data & 0xFF;
    pRecord->total_money[1] = u16Data >> 8;

    u16Data = pcostdata->allServerMoney / 100;
    pRecord->total_sever48[0] = u16Data & 0xFF;
    pRecord->total_sever48[1] = u16Data >> 8;

	return TRUE;
}

static void AHTT_dev_getGunState(uint8_t *StateArray) {

    for (uint8_t ch = 0; ch < GUN_NUM; ch++) {
        uint8_t state;
        if (dev_getErrState(ch) == TRUE) {
            state = E_AHTT_GunState_Fault;
        } else {
            state = logic_get_gun_charging(ch) ? E_AHTT_GunState_Work : E_AHTT_GunState_Free; 
        }

        // 2. 计算存储位置
        uint8_t byte_idx = ch / 4;       // 确定字节索引（0-2）
        uint8_t bit_pos  = (ch % 4) * 2; // 每通道占2bit（0,2,4,6）

        StateArray[byte_idx] |= (state << bit_pos);
    }
}
/******************************************************************************************************************************************* */
/******************************************************************************************************************************************* */
/******************************************************************************************************************************************* */
static uint8_t AHTT_getPerCardMoney(uint8_t u8Port)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_Set_Param *pRecvSetParam = &pAhttFlashInfo->Param_0x84;
    //0.1元
    if (!pRecvSetParam->cardmoney) {
        return AHTT_CARD_PERMNY;
    }
    return pRecvSetParam->cardmoney;
}
static uint8_t AHTT_getPerCardTime(uint8_t u8Port)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_Set_Param *pRecvSetParam = &pAhttFlashInfo->Param_0x84;
    //分钟
    if (!pRecvSetParam->cardtime) {
        return AHTT_CARD_PERMNY;
    }
    return pRecvSetParam->cardtime;
}

//心跳上报周期，单位ms
static uint32_t AHTT_getHeartPeriod()
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_SetCyc *pRecvSetParam = &pAhttFlashInfo->cyc_0x03;
    uint32_t period = AHTT_HEART_PERIOD_MIN * eTick_60S;

    if (!pRecvSetParam->cyctime) {
        return period;
    }
    period = pRecvSetParam->cyctime * eTick_60S;
    return period;
}

//通道数据上报间隔，单位ms,参数为0表示ms，参数为1表示分钟；
static uint32_t AHTT_get0x93Period(int type)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_Set_Param *pRecvSetParam = &pAhttFlashInfo->Param_0x84;
    uint32_t period = AHTT_0x93_PERIOD_MIN;
    if (!pRecvSetParam->uploadcyc) {
        return period;
    } else {
        period = pRecvSetParam->uploadcyc;
    }

    period = period * (type ? 1: eTick_60S);
    return period;
}

//安徽铁塔参数存储
static void AHTT_ReadStoragePara(AHTT_FlashPlatInfo *pFlashInfo)
{
    if (load_EEOP_Param((uint8_t *)pFlashInfo, sizeof(AHTT_FlashPlatInfo)) == FALSE) {
        pFlashInfo->Param_0x84.cardmoney = AHTT_CARD_PERMNY;
        pFlashInfo->Param_0x84.uploadcyc = AHTT_0x93_PERIOD_MIN;
        pFlashInfo->cyc_0x03.cyctime = AHTT_HEART_PERIOD_MIN;
        pFlashInfo->u8ChargeMaxTime = AHTT_MAX_CHARGETIME;
        printf("AHTT_ReadStoragePara init.\r\n");
    };
    
    AHTT_SetChargePara_MaxTime(pFlashInfo->u8ChargeMaxTime);
}
//安徽铁塔参数存储
static void AHTT_WriteStoragePara(AHTT_FlashPlatInfo *pFlashInfo)
{
    Set_EEOP_Param((uint8_t *)pFlashInfo, sizeof(AHTT_FlashPlatInfo));
}



 /********************************************************************
 * @brief 	 读取最新的记录并判断是否上传成功
 * @param[in]	
 * @return 	
 *********************************************************************/	
static void AHTTUpChargeRecordUpDealOffline(void)
{
   for (uint8_t i = 0; i < GUN_NUM; i++ ) {
        uint8_t uGun = i;

        uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }
    
        uint8_t cmd = AHTT_Chg_Record;

		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("AHTTUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}

	    AHTT_charge_record *UpRecord = &g_chgData[uGun].DealRecord.ChgRecord.AhttChgRecord;
        
        ahtt_packStopReasonChgRecord(uGun, UpRecord);

        SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
        Send_Immediately(uGun, cmd);
   }
}


/*******************************************************/
//充电设备登录认证
uint16_t Ahtt_send_identification_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	char cSimID[20] = {0};
	GetNet_Comm_SimID((uint8_t *)cSimID, 20);
    memcpy(&data[data_len], cSimID, 20);
    data_len += 20;

    uint8_t ver[4] = {0};
	string_split_to_int(ver, SOFTWARE_VERSION, 4);
	data[data_len++] = ver[1];
    uint8_t u8ver = (uint8_t)(ver[2] * 10 + ver[3]);
	data[data_len++] = u8ver;
    
	data[data_len] = 0;
	data_len += 6;
	return data_len;
}

void Reverse_order(uint8_t *output, int len) {
    uint8_t temp;
    for (int i = 0; i < len / 2; i++) {
        temp = output[i];
        output[i] = output[len - 1 - i];
        output[len - 1 - i] = temp;
    }
	return;
}

void send_login_ahtt_Succ(uint8_t u8Port)
{
    return;
}

// 处理平台下发的心跳周期设置命令
uint16_t Ahtt_send_heart_set(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
	
	uint16_t data_len = 0;  // 此命令数据包长度
    U8* data = (uint8_t*)pdata;

    data[data_len++] = pAhttUpInfo->upResult;

    return data_len;
}

// 桩端应答心跳周期设置
void Ahtt_send_heart_set_Succ(uint8_t u8Port)
{   
	SetSendEnable(u8Port, AHTT_Heart_Set, SEND_ENABLE_OFF);
}

// 处理平台下发的心跳周期查询命令
uint16_t Ahtt_send_heart_search(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    //存储flash
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_SetCyc *pRecvSetParam = &pAhttFlashInfo->cyc_0x03;
	
	uint16_t data_len = 0;  // 此命令数据包长度
    U8* data = (uint8_t*)pdata;

    data[data_len++] = pRecvSetParam->cyctime;

    return data_len;
}

// 桩端应答心跳周期查询
void Ahtt_send_heart_search_Succ(uint8_t u8Port)
{
    // 更新接收状态
    if (RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Heart_Search))
    {
        SetRecvEnable(u8Port, AHTT_Heart_Search, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Heart_Search, Get_Systick());
    }
}

/***************心跳上报的数据应该如何获取*******/
uint16_t Ahtt_send_heart_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	data[data_len] = 2;
	data_len++;
	
	data[data_len] = GetNet_SignalLevel();
	data_len++;
	
	memset(&data[data_len], 0, 3);
  	AHTT_dev_getGunState(&data[data_len]);
	data_len += 3;
	
	return data_len;
}



void Ahtt_send_heart_Succ(uint8_t u8Port)
{
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Heart)) {
        SetRecvEnable(u8Port, AHTT_Heart, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Heart, Get_Systick());
    }

	return;
}

#define DOMAIN_MAX_LEN 24  // 域名最大长度
#define PORT_MAX_LEN 6     // 端口最大长度

/*******************默认域名是什么？？？？？？ */
char g_currentDomain[DOMAIN_MAX_LEN] = "????"; // 默认域名
char g_currentPort[PORT_MAX_LEN] = "??????";                  // 默认端口

// 处理平台下发的域名与端口设置命令
uint16_t Ahtt_send_port_domain(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  // 此命令数据包长度
    U8* data = (uint8_t*)pdata;

    data[data_len++] = 1; // 0表示失败

    return data_len;
}

// 桩端应答域名与端口设置
void Ahtt_send_port_domain_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Port_Domain, SEND_ENABLE_OFF);
}

//剩余金额和剩余电量 同时有数据？ 怎么处理？
uint16_t Ahtt_send_real_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    
	COST_GUN_DATA *pAhttRecord = &g_cost_ctrl.strCostGunData[u8Port];
	
	data[data_len++] = u8Port;

    uint16_t J1 = pAhttRecord->allEleMoney / 100;    //电费
    uint16_t J2 = pAhttRecord->allServerMoney / 100;    //服务费
    uint16_t J3 = J1 + J2;    //总金额
    uint16_t Y1 = pChgGunData->gun_chrg_mode_param * 10 - J3;   //剩余金额
    
    if (pChgGunData->gun_chrg_mode == eDetectMode_Count) {
        Y1 = pChgGunData->gun_chrg_mode_param * 10 - J3;
    } else {
        Y1 = 0;
    }

	//剩余时间Y2=(Y1/J3)*A
    uint32_t Y2 = 0;
    if (J3)
    {
        Y2 = (Y1/J3)*AHTT_get0x93Period(1);    //剩余时间
    }

	data[data_len++] = Y2 & 0xFF;
	data[data_len++] = (Y2 >> 8) & 0xFF;
	data[data_len++] = (Y2 >> 16) & 0xFF;
    printf("ahtt_93 remainTime: %d\r\n", Y2);

    uint16_t power = 0;
	//功率
	power = (GetPile_ChgOutVol(u8Port, 1) * GetPile_ChgOutCur(u8Port, 1)) / 100;
    data[data_len++] = power & 0xFF;
    data[data_len++] = power >> 8;
    printf("ahtt_93 power: %d\r\n", power);
	//电量
    uint32_t reTime = monitor_getChgTotalEnergy(u8Port)/10;
	data[data_len++] = reTime & 0xFF;
	data[data_len++] = (reTime >> 8) & 0xFF;
	data[data_len++] = (reTime >> 16) & 0xFF;
    printf("ahtt_93 ele: %d\r\n", reTime);

	//剩余金额
    data[data_len++] = Y1 & 0xFF;
    data[data_len++] = Y1 >> 8;
    printf("ahtt_93 remain money: %d\r\n", Y1);

	//消费金额
    data[data_len++] = J1 & 0xFF;
    data[data_len++] = J1 >> 8;
    printf("ahtt_93 money: %d\r\n", J1);

	//服务费
    data[data_len++] = J2 & 0xFF;
    data[data_len++] = J2 >> 8;
    printf("ahtt_93 serMoney: %d\r\n", J2);

	//剩余电量
    uint32_t Y3 = Y2*power/60;
    printf("ahtt_93 remainEle: %d\r\n", Y3);
	data[data_len++] = Y3 & 0xFF;
	data[data_len++] = (Y3 >> 8) & 0xFF;
	data[data_len++] = (Y3 >> 16) & 0xFF;
	
    return data_len;
}

void Ahtt_send_real_Succ(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_RealData)) {
        SetRecvEnable(u8Port, AHTT_RealData, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_RealData, Get_Systick());
    }
	return;
}

uint16_t g_maxChargingTime = 600; // 默认最大充电时长为600分钟（10小时）
// 处理平台下发的最大充电时长设置命令
uint16_t Ahtt_send_max_chg_time(uint8_t u8Port, void *pdata, uint16_t inlen)
{  
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    data[0] = pAhttUpInfo->upResult;
	data_len++;

    return data_len;
}

// 桩端应答最大充电时长设置
void Ahtt_send_max_chg_time_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_MaxChgTime, SEND_ENABLE_OFF);
	return;
}

// 处理平台下发的查询最大充电时长命令
uint16_t Ahtt_send_sea_max_chg_time(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    uint16_t data_len = 0;  // 此命令数据包长度
    U8* data = (U8*)pdata;
    
    data[0] = pAhttFlashInfo->u8ChargeMaxTime;
    data_len++;

    return data_len;
}

void Ahtt_send_sea_max_chg_time_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Sea_MaxChgTime, SEND_ENABLE_OFF);
	return;
}

// 执行结果如何跟故障关联？,单号如何获取?
uint16_t Ahtt_send_start_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
	AHTT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvStartCharge;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;

	data[data_len++] = pRecvStartCharge->odd_number[0];
	data[data_len++] = pRecvStartCharge->odd_number[1];
	data[data_len++] = pAhttUpInfo->chargeResult_4D;
	
    return data_len;
}

// 开始充电回复
void Ahtt_send_start_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Auth, SEND_ENABLE_OFF);
	
	return;
}

//电量/消费金额如何区分,平台下发的金额如何获取?
uint16_t Ahtt_send_stop_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    AHTT_Recv_Stop_Charge *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvStopCharge;
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
	U32 totalMoney = 0;

	//单号
	memcpy(&data[data_len], pRecvStartCharge->odd_number, sizeof(pRecvStartCharge->odd_number));
	data_len += 2;
	
	//结果
    if (pAhttUpInfo->chargeMode_4D == E_AHTT_ChargeMode_Full) {
        data[data_len++] = 5;
    } else {
        data[data_len++] = 1;
    }

    //可以使用订单信息
	AHTT_charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.AhttChgRecord;
	
    memcpy(&data[data_len], &pRecord->total_power, 9);
	data_len += 9;

    totalMoney = (pRecord->total_money[0] | (U32)pRecord->total_money[1] << 8);      //总电费
    totalMoney += (pRecord->total_sever48[0] | (U32)pRecord->total_sever48[1] << 8); //总服务费
    SetPlat_ChgTotalMoney(u8Port, totalMoney * 100);

    return data_len;
}

void Ahtt_send_stop_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Stop_Chg_Ack, SEND_ENABLE_OFF);
	return;
}

//交易记录上传
uint16_t Ahtt_send_charge_record_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	AHTT_charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.AhttChgRecord;
	U32 totalMoney = 0;

    memcpy(&data[data_len], &pRecord->stop_reason, sizeof(AHTT_charge_record));
	data_len += sizeof(AHTT_charge_record);
    
    totalMoney = (pRecord->total_money[0] | (U32)pRecord->total_money[1] << 8);      //总电费
    totalMoney += (pRecord->total_sever48[0] | (U32)pRecord->total_sever48[1] << 8); //总服务费
    SetPlat_ChgTotalMoney(u8Port, totalMoney * 100);

    return data_len;
}

void Ahtt_send_charge_record_Succ(uint8_t u8Port)
{	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Chg_Record)) {
        SetRecvEnable(u8Port, AHTT_Chg_Record, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Chg_Record, Get_Systick());
    }

    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;
	
	if (pChgGunData->upDealCnt >= 3)
    {
        pChgGunData->upDealCnt = 0;
        //上报3次未回复，停止上报
        if(SEND_ENABLE_ON == GetSendEnable(u8Port, AHTT_Chg_Record))
        {
            SetSendEnable(u8Port, AHTT_Chg_Record, SEND_ENABLE_OFF);
        }
	}
    else if (pChgGunData->upDealCnt == 1) 
    {
        pChgGunData->ExistChargeDeal = 0;
    }

	return;
}



//刷卡启动测试
uint16_t Ahtt_send_CardRequest_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    
    pAhttUpInfo->cardSendData.number++;
    /* 单号 */
	data[data_len++] = pAhttUpInfo->cardSendData.number & 0xFF;
	data[data_len++] = pAhttUpInfo->cardSendData.number >> 8;
    /* 通道号 */
	data[data_len++] = u8Port;
    /* 卡号 */
    memcpy(&data[data_len], pCardData->Ahtt_PhyCard_number, GNDATA_PHYCARD_LEN);
    // Reverse_order(&data[data_len], GNDATA_PHYCARD_LEN);//平台协议要求卡号倒序后上传，由于刷卡程序读取的卡号是按倒序存放在数组内的，因此此处无需倒序
    data_len += 4;
    data[data_len++] = 0x00;

    U16 money = (pAhttUpInfo->CardStartData[u8Port].CardCnt * AHTT_getPerCardMoney(u8Port)) / 10;
    printf("Ahtt_send_CardRequest_data: %d  %d\r\n", money, AHTT_getPerCardMoney(u8Port));
    /* 刷卡金额 单位元 */
	data[data_len++] = money & 0xFF;
	data[data_len++] = money >> 8;

    return data_len;
}

void Ahtt_send_CardRequest_Succ(uint8_t u8Port)
{	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_ChgCard_Record)) {
        SetRecvEnable(u8Port, AHTT_ChgCard_Record, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_ChgCard_Record, Get_Systick());
    }
	return;
}


uint16_t Ahtt_send_set_para_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    data[data_len++] = pAhttUpInfo->upResult;

    return data_len;
}

void Ahtt_send_set_para_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Equipara, SEND_ENABLE_OFF);
	
	return;
}


//平台查询设备参数响应
uint16_t Ahtt_send_devParam_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_Set_Param *pRecvSetParam = &pAhttFlashInfo->Param_0x84;
    
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    
    memcpy(data, &pRecvSetParam->cardmoney, sizeof(AHTT_Recv_Set_Param));
    data_len = sizeof(AHTT_Recv_Set_Param);
	
    return data_len;
}

void Ahtt_send_devParam_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Sea_Equipara, SEND_ENABLE_OFF);
	
	return;
}

//设备功率应答
uint16_t Ahtt_send_getPower_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_Recv_Set_Param *pRecvSetParam = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetParam;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	data[data_len++] = u8Port;

    uint16_t power = (GetPile_ChgOutCur(u8Port, 1) / 10) * (GetPile_ChgOutVol(u8Port, 1) / 10);
	data[data_len++] = power & 0xFF;
	data[data_len++] = power >> 8;

    return data_len;
}

void Ahtt_send_getPower_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_GetPower, SEND_ENABLE_OFF);
	
	return;
}




uint16_t Ahtt_send_Get_time_rqst(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;

    return data_len;
}

void Ahtt_send_get_time_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_TimeSyn_ACK)) {
        SetRecvEnable(u8Port, AHTT_TimeSyn_ACK, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_TimeSyn_ACK, Get_Systick());
    }
	return;
}

uint16_t Ahtt_send_set_rate_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;

	data[data_len] = UP_RESULT_SUCC;
	data_len++;
	
    return data_len;
}

void Ahtt_send_set_rate_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Auth, SEND_ENABLE_OFF);
	
	return;
}

//设置温度告警值应答
uint16_t Ahtt_send_setTemp_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;

	data[data_len] = pAhttUpInfo->upResult;
	data_len++;
	
    return data_len;
}

void Ahtt_send_setTemp_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_SetTemper, SEND_ENABLE_OFF);
	
	return;
}

uint16_t Ahtt_send_ftp_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;

	data[data_len++] = pAhttUpInfo->upResult;
	
    return data_len;
}

void Ahtt_send_ftp_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_set_update_ftp, SEND_ENABLE_OFF);
	
	return;
}

uint8_t AHTTUpCtrlSendCyc(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_SetCyc *pRecvSetParam = &pAhttFlashInfo->cyc_0x03;
	
	if(TRUE == u8SendImmdFlag)
	{
		return TRUE;
	}

	if(AHTT_Heart == cmd)
	{
        Cyc = AHTT_getHeartPeriod();         //分钟转换为ms
	}
    
	if(AHTT_RealData == cmd)
	{
        Cyc = AHTT_get0x93Period(0);          //分钟转换为ms
	}
	

	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

uint16_t AHTT_Send_DeviceState(uint8_t u8Port, void *pdata, uint16_t inlen) {
    uint8_t *data = (uint8_t *)pdata;
    uint16_t data_len = 0;

    data[data_len++] = monitor_getGunTem(0);
    data[data_len++] = monitor_getGunTem(1);
    data[data_len++] = monitor_getGunTem(0);
    data[data_len++] = monitor_getGunTem(1);

    // 急停按钮状态
    data[data_len++] = Get_PileEstopBtnSta();

    // CP电压及状态
    data[data_len++] = GetPile_GunRtCpValue(u8Port) / 10;
    data[data_len++] = 1;

    //计量状态暂时无
    data[data_len++] = 0;

    // 填充电压和通道信息
    uint16_t voltage = GetPile_ChgInVol() / 10;
    data[data_len++] = voltage & 0xFF;
    data[data_len++] = voltage >> 8;
    printf("ahtt_96 voltage: %d,%d\r\n", voltage, GetPile_GunRtCpValue(u8Port));


    data[data_len++] = GetPile_CfgGunNum(); 
    
    for (uint8_t i = 0; i < GUN_NUM; i++) {
        data[data_len++] = i;
        data[data_len++] = GetPile_gun_connect(i);
        uint8_t sta = GetPile_gun_state(i);
        uint8_t outsta = 0;
        if (sta < eChargeState_Starting) {
            outsta = 0;
        } else if (sta == eChargeState_PauseB) {
            outsta = 3;
        }  else if (sta == eChargeState_StopFinish) {
            outsta = 2;
        } else if (sta < eChargeState_StopFinish) {
            outsta = 1;
        }
        data[data_len++] = outsta;
    }

    return data_len;
}

void AHTT_Send_DeviceState_Succ(uint8_t u8Port) {
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

	SetRecvEnable(u8Port, AHTT_State, RECV_ENABLE_ON);
    SetRecvTick(u8Port, AHTT_State, Get_Systick());

	return;
}


//通道状态
uint16_t AHTT_Send_Alarm(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
    U8* data = (uint8_t*)pdata;

    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

    // 拆分： 01000100 11000000（将上面的状态， 拆分成 2 个字节）
    // 倒序： 00100010 00000011（字节内部倒序， 前后字节不倒序）
	data[data_len++] = reverse_bits(pAhttUpInfo->devErrAlm[0]);
	data[data_len++] = reverse_bits(pAhttUpInfo->devErrAlm[1]);

	return data_len;
}

void AHTT_Send_Alarm_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Alarm)) {
        SetRecvEnable(u8Port, AHTT_Alarm, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Alarm, Get_Systick());
    }

	return;
}

//网络告警
uint16_t AHTT_Send_Network_Alarm(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
    U8* data = (uint8_t*)pdata;
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
	data[data_len++] = pAhttUpInfo->netSgnAlm;

    return data_len;
}

void AHTT_Send_Network_Alarm_Succ(uint8_t u8Port){
    
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Network_Alarm)) {
        SetRecvEnable(u8Port, AHTT_Network_Alarm, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Network_Alarm, Get_Systick());
    }
    // SetSendEnable(u8Port, AHTT_Network_Alarm, SEND_ENABLE_OFF);
	return;
}


//温度告警
uint16_t AHTT_Send_Temper_Alarm(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    data[data_len++] = pAhttUpInfo->overTempAlm;

	data[data_len++] = GetPile_EvnTem();

	return data_len;
}

void AHTT_Send_Temper_Alarm_Succ(uint8_t u8Port)
{
    
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, AHTT_Temper_Alarm)) {
        SetRecvEnable(u8Port, AHTT_Temper_Alarm, RECV_ENABLE_ON);
        SetRecvTick(u8Port, AHTT_Temper_Alarm, Get_Systick());
    }

	return;
}



static uint16_t Ahtt_dataEncode(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	AHTT_HEAD_T *pHeart = (AHTT_HEAD_T*)p;
	uint8_t headLen = sizeof(AHTT_HEAD_T);
	uint16_t all_len = data_len[0] + headLen + 2;
	uint16_t packlen = all_len - 3;
    uint16_t crc_len = all_len  - 2;
	uint16_t crc = 0;
	
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    pHeart->head = 0xEA;
	pHeart->ver	= 0x01;
	//设备编码
	monitor_getDevNumber(pHeart->device_num, 5);
	Reverse_order(pHeart->device_num, 5);

	// 流水号处理
    static uint16_t serial = 0;  // 静态变量保存流水号
    serial++;                    // 自增
    if (serial >= 60000) {       // 达到阈值归1
        serial = 1;
    }
    pHeart->ser[0] = (serial >> 8) & 0xFF;  // 高字节
    pHeart->ser[1] = serial & 0xFF;         // 低字节
    Reverse_order(pHeart->ser, 2); 

	pHeart->cmd = cmd;
	
	Uint16ToTwoUint8(pHeart->len, packlen);
	

	uint8_t test[40] = {0};
    //对校验位之前的数据进行CRC校验
    crc = ModbusCRC(&pHeart->head, crc_len);
	
	p[crc_len] = crc & 0Xff;
	p[crc_len + 1] = crc >> 8;
	
	data_len[0] = all_len;
	
    return all_len;
}

//应答查询心跳周期


/*******************************************************
*
*
*
*
*
*
*******************************************************/
typedef uint8_t (*PSendCyc)(uint8_t u8Port, uint32_t cmd, uint32_t	Cyc);
typedef uint16_t (*PSend)(uint8_t u8Port, void *pBuf ,uint16_t u32BufSize);
typedef void (*PSendSucc)(uint8_t u8Port);

typedef struct
{
    uint32_t		cmd;
	uint8_t			FType;
    uint32_t     	cyc;
    PSendCyc 		pSendCyc;
    PSend 			pSend;
    PSendSucc 		pSendSucc;
}AHTT_Send_ctrl;

#define  JX_SEND_IMMD 0

const AHTT_Send_ctrl StrAHTTSendCtrl[]={
    {AHTT_Identification	,UP_S_FRAME_SELF	,eTick_30S,		AHTTUpCtrlSendCyc	,Ahtt_send_identification_data	,send_login_ahtt_Succ},		//
	{AHTT_Heart_Set       	,UP_S_FRAME_ACK     ,eTick_30S,    AHTTUpCtrlSendCyc	,Ahtt_send_heart_set			,Ahtt_send_heart_set_Succ},//设置心跳周期
    {AHTT_Heart_Search    ,UP_S_FRAME_ACK       ,eTick_15S,     AHTTUpCtrlSendCyc	,Ahtt_send_heart_search			,Ahtt_send_heart_search_Succ},//查询心跳周期
    {AHTT_Heart 			,UP_S_FRAME_SELF	,eTick_60S,	    AHTTUpCtrlSendCyc 	,Ahtt_send_heart_data			,Ahtt_send_heart_Succ},
	{AHTT_Port_Domain       ,UP_S_FRAME_ACK      ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_port_domain		,Ahtt_send_port_domain_Succ},//端口域名
    {AHTT_MaxChgTime      ,UP_S_FRAME_ACK     ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_max_chg_time			,Ahtt_send_max_chg_time_Succ},//最大充电时长
    {AHTT_Sea_MaxChgTime  ,UP_S_FRAME_ACK     ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_sea_max_chg_time		,Ahtt_send_sea_max_chg_time_Succ},//查询最大充电时长
    {AHTT_Auth            ,UP_S_FRAME_ACK     ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_start_ack            ,Ahtt_send_start_Succ},
    {AHTT_RealData          ,UP_S_FRAME_ACK     ,eTick_60S,	AHTTUpCtrlSendCyc   ,Ahtt_send_real_data            ,Ahtt_send_real_Succ},
    {AHTT_Stop_Chg_Ack    ,UP_S_FRAME_ACK     ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_stop_ack             ,Ahtt_send_stop_Succ},
    {AHTT_Chg_Record      ,UP_S_FRAME_SELF    ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_charge_record_data   ,Ahtt_send_charge_record_Succ},
    {AHTT_ChgCard_Record  ,UP_S_FRAME_SELF    ,eTick_60S,		AHTTUpCtrlSendCyc   ,Ahtt_send_CardRequest_data     ,Ahtt_send_CardRequest_Succ     },//刷卡订单上报
    {AHTT_Equipara        ,UP_S_FRAME_ACK       ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_set_para_ack         ,Ahtt_send_set_para_Succ},
    {AHTT_Sea_Equipara    ,UP_S_FRAME_ACK       ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_devParam_ack      ,Ahtt_send_devParam_Succ},//查询设备参数应答
    {AHTT_GetPower      ,UP_S_FRAME_ACK       ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_getPower_ack      ,Ahtt_send_getPower_Succ},//查询设别功率应答
    {AHTT_TimeSyn_ACK     ,UP_S_FRAME_SELF      ,eTick_1H,		AHTTUpCtrlSendCyc   ,Ahtt_send_Get_time_rqst         ,Ahtt_send_get_time_Succ},
    
    {AHTT_State           	,UP_S_FRAME_SELF    ,eTick_60S*5,	AHTTUpCtrlSendCyc   ,AHTT_Send_DeviceState			,AHTT_Send_DeviceState_Succ},//设备状态
    {AHTT_Alarm           	,UP_S_FRAME_SELF    ,eTick_10S,		AHTTUpCtrlSendCyc   ,AHTT_Send_Alarm				,AHTT_Send_Alarm_Succ},//通道状态告警
    {AHTT_Network_Alarm   	,UP_S_FRAME_SELF    ,eTick_10S,		AHTTUpCtrlSendCyc   ,AHTT_Send_Network_Alarm        ,AHTT_Send_Network_Alarm_Succ},//告警
    {AHTT_Temper_Alarm      ,UP_S_FRAME_SELF    ,eTick_10S,		AHTTUpCtrlSendCyc   ,AHTT_Send_Temper_Alarm         ,AHTT_Send_Temper_Alarm_Succ},//告警
    {AHTT_SetTemper         ,UP_S_FRAME_SELF    ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_setTemp_ack          ,Ahtt_send_setTemp_Succ},//告警
    {AHTT_set_update_ftp    ,UP_S_FRAME_ACK     ,eTick_30S,		AHTTUpCtrlSendCyc   ,Ahtt_send_ftp_ack              ,Ahtt_send_ftp_Succ},
};

static uint16_t AHTTUpCtrlSend(void *pBuf ,uint32_t u32BufSize)
{
	const AHTT_Send_ctrl *pAHTTSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	AHTT_HEAD_T *pHead = (AHTT_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + sizeof(AHTT_HEAD_T);
	
	//不论后台支不支持都不能连帧发送
	// if (0 != dtu_data.tx_len) return outLen;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrAHTTSendCtrl); u32i++)
		{
			pAHTTSendCtrl = &StrAHTTSendCtrl[u32i];
			
			if (SEND_ENABLE_ON != GetSendEnable(i, pAHTTSendCtrl->cmd))
				continue;
			
			if (TRUE == pAHTTSendCtrl->pSendCyc(i, pAHTTSendCtrl->cmd, pAHTTSendCtrl->cyc))
			{
                outLen = pAHTTSendCtrl->pSend(i, pData, u32BufSize);
	            Ahtt_dataEncode(i, (uint8_t*)pHead, pAHTTSendCtrl->cmd, pAHTTSendCtrl->FType, &outLen);
                pAHTTSendCtrl->pSendSucc(i);
                SetSendTick(i, pAHTTSendCtrl->cmd, Get_Systick());
                SetSendFlag(i, pAHTTSendCtrl->cmd, SEND_FLAG_YES);
                SetSendImmdFlag(i, pAHTTSendCtrl->cmd, FALSE);
                
                SetClearRecvRptUpt(i, pAHTTSendCtrl->cmd);
                
                printf("\r\nUpProtocol --> GUN: %d, SendDealcmd: 0x%x \r\n", i, pAHTTSendCtrl->cmd);
                return outLen;
			}
		}
	}
	
	return outLen;
}

static void AHTTUpCtrlSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;

	outLen = AHTTUpCtrlSend(pbuf, sizeof(pbuf));
    
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);

    return;
}

/*******************************************************/
//登录认证应答解析
uint8_t Ahtt_recv_identification_data_parse( U8 *r_data, int len, uint8_t* gun)
{
	return TRUE;
}

void Ahtt_recv_identification_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Heart, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Heart);

	SetSendEnable(u8Port, AHTT_State, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_State);

	SetSendEnable(u8Port, AHTT_Identification, SEND_ENABLE_OFF);
    
	SetSendEnable(u8Port, AHTT_TimeSyn_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_TimeSyn_ACK);
    
    AHTTUpChargeRecordUpDealOffline();

	return;
}

uint8_t Ahtt_recv_heart_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	//AHTT_Recv_SetCyc *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTime;
	//pRecvStartCharge->time = r_data[0];
	return TRUE;
}

void Ahtt_recv_heart_Succ(uint8_t u8Port)
{
    PlatHeartTickRefresh();
    
	Set_PlatConnectSta(eOnline_Heart);

	SetRecvEnable(u8Port, AHTT_Heart, RECV_ENABLE_OFF);

	return;
}

uint8_t Ahtt_recv_set_heart_cyc(U8 *r_data, int len, uint8_t* gun){
	AHTT_Recv_SetCyc *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTime;
	pRecvStartCharge->cyctime = r_data[0];
	return TRUE;
}

void Ahtt_recv_set_heart_cyc_Succ(uint8_t u8Port){
    
    //存储flash
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    AHTT_Recv_SetCyc *pRecvSetParam = &pAhttFlashInfo->cyc_0x03;

	AHTT_Recv_SetCyc *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTime;
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    pAhttUpInfo->upResult = 1;
    if (pRecvStartCharge->cyctime >= 1 && pRecvStartCharge->cyctime <= 10)
    {
        pAhttUpInfo->upResult = 1; // 设置成功
        
        pRecvSetParam->cyctime = pRecvStartCharge->cyctime;
        //存储
        AHTT_WriteStoragePara(pAhttFlashInfo);
    }
    else
    {
        pAhttUpInfo->upResult = 0; // 设置失败
    }

	SetSendEnable(u8Port, AHTT_Heart_Set, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Heart_Set);
	return;
}

uint8_t AHTT_recv_Heart_Search(U8 *r_data, int len, uint8_t* gun){
	return TRUE;
}

void AHTT_recv_Heart_Search_Succ(uint8_t u8Port){
	return;
}



uint8_t AHTT_recv_Port_Domain(U8 *r_data, int len, uint8_t* gun){
	AHTT_Recv_Port_Domain *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRevPorDOmain;
	memcpy(pRecvStartCharge, r_data, sizeof(AHTT_Recv_Port_Domain));
    Reverse_order((uint8_t *)pRecvStartCharge->domain, sizeof(pRecvStartCharge->domain));
    Reverse_order((uint8_t *)pRecvStartCharge->port, sizeof(pRecvStartCharge->port));
	return TRUE;
}

void AHTT_recv_Port_Domain_Succ(uint8_t u8Port) 
{
	AHTT_Recv_Port_Domain *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRevPorDOmain;
    
    char ipDomain[PLAT_DNS_LEN] = {0};
    uint16_t port = 0 ;
    memcpy(ipDomain, pRecvStartCharge->domain, sizeof(pRecvStartCharge->domain));
	sscanf(pRecvStartCharge->port, "%hd", &port);
    
	printf("ok! MainIp = %s, MainPort = %d\r\n", ipDomain, port);
    
    Set_PlatIpPort(ipDomain, port);

	SetSendEnable(u8Port, AHTT_Port_Domain, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Port_Domain);

	return;
}

uint8_t AHTT_recv_MaxChgTime(U8 *r_data, int len, uint8_t* gun){
    
	CHG_DATA_T *pChgGunData = &g_chgData[*gun];
    
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    pAhttUpInfo->upResult = 0;

    if (r_data[0] > 0 && r_data[0] <= 16)
    {
        //参数正确
        pAhttUpInfo->upResult = 1; // 设置成功
        
        pAhttFlashInfo->u8ChargeMaxTime = r_data[0];
        //存储
        AHTT_WriteStoragePara(pAhttFlashInfo);

        AHTT_SetChargePara_MaxTime(r_data[0]);
    }

	return TRUE;
}

void AHTT_recv_MaxChgTimeSucc(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_MaxChgTime, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_MaxChgTime);

	return;
}

uint8_t AHTT_recv_Sea_MaxChgTime(U8 *r_data, int len, uint8_t* gun){
	return TRUE;
}

void AHTT_recv_Sea_MaxChgTime_Succ(uint8_t u8Port){
    
	SetSendEnable(u8Port, AHTT_Sea_MaxChgTime, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Sea_MaxChgTime);
	return;
}



uint8_t AHTT_recv_Auth(U8 *r_data, int len, uint8_t* gun){
    
	U8 u8Port = r_data[2];

	AHTT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvStartCharge;
    *gun = u8Port;
	memcpy(pRecvStartCharge, r_data, sizeof(AHTT_Recv_Start_Charge));
    
	return TRUE;
}

uint8_t ahttGetStartFault(uint8_t u8Port, uint8_t reason)
{
    //此函数需要补充
    GN_PLATMOD  *lpm = &sg_platmod;
//    AHTT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRecvStartCharge;
//    AHTT_Recv_SetTemper *temper = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTemper;
   U_GunFault *pFault = &lpm->gun[u8Port].gunRtInfo.hardfault;
   U_PlatChargeSta *p_Fault = &lpm->gun[u8Port].gunRtInfo.u_platChrgsta;

   
    // 检查急停按钮
    if(pFault->bit.faultEStop_1b == TRUE) {
        return E_AHTT_ChargeResult_EmergStop; // 17-急停按钮按下
    }
    // 检查充电枪连接
    if(p_Fault->bit.connect_1b == FALSE) {
        return E_AHTT_ChargeResult_GunNotConn; // 16-充电枪未插入车辆
    }
    // 检查继电器状态
    // L/N继电器粘连如何区分？当前为相同  -继电器过温用枪头温度替代
    if(pFault->bit.faultRelay_1b == TRUE) {
        return E_AHTT_ChargeResult_RelayLStuck; // 10-L继电器粘连
    }
    if(pFault->bit.faultRelay_1b == TRUE) {
        return E_AHTT_ChargeResult_RelayNStuck; // 11-N继电器粘连
    }
    if(pFault->bit.faultPlugTemp_1b == TRUE) {
        return E_AHTT_ChargeResult_RelayOverTemp; // 15-继电器过温
    }
    // 检查电压状态
    if(pFault->bit.faultVoltage_1b == 2) {
        return E_AHTT_ChargeResult_VoltageLow; // 7-输入欠压
    }
    if(pFault->bit.faultVoltage_1b == TRUE) {
        return E_AHTT_ChargeResult_VoltageHigh; // 8-输入过压
    }
    // 检查温度状态
    if(pFault->bit.faultPlugTemp_1b == TRUE) {
        return E_AHTT_ChargeResult_LineOverTemp; // 6-进线接线过温
    }
    // 接地
    if(pFault->bit.faultGround_1b == TRUE) {
        return E_AHTT_ChargeResult_PhaseGroundFault; // 9接地异常
    }
    // 检查计量模块
    if(pFault->bit.faultEleMeasure_1b == TRUE) {
        return E_AHTT_ChargeResult_MeterFault; // 13-计量电路故障
    }
    // 引导电压
    if(pFault->bit.faultCP_1b == TRUE) {
        return E_AHTT_ChargeResult_PilotVolAbnormal; // 14-充电引导电压异常
    }
    // 检查输出短路
    if(pFault->bit.faultShortC_1b == TRUE) {
        return E_AHTT_ChargeResult_OutputShort; // 12-输出负载短路
    }
    if(logic_get_gun_Uncharged(u8Port) == 1) {
        return E_AHTT_ChargeResult_ChannelBusy; // 3-通道不在空闲
    }

    
    return E_AHTT_ChargeResult_ChannelFault; // 2-通道故障
}

//安徽铁塔启动充电，计费模型异常不进行判断
uint8_t AHTT_StartChargeResult(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney)
{
    uint8_t reason = 0;
    uint8_t ret = monitor_charge_start(u8Port, \
		&reason, \
		trade_flag, \
		NULL, \
		pTrdNum, \
		BlMoney);

    if (ret == FALSE) 
    {
        if (reason == eUP_Start_Fail_Rate) 
        {
            ret = TRUE;    //计费模型不存储，充电会下发
        }
        else
        {
            up_fail_reason[0] = reason;
        }
    }
    
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];
    memcpy(pChgGunData->Swip_PhyCard_number, pCardData->Ahtt_PhyCard_number, GNDATA_PHYCARD_LEN);

    return ret;
}
//启动充电
void AHTT_recv_Auth_Succ(uint8_t u8Port){
	//将计费模型转换为cost里面使用的入参在进行充电统一计费
	AHTT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvStartCharge;

    printf("ahtt start charge\r\n");

    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    pAhttUpInfo->chargeResult_4D = 0;

    uint32_t sum_balance = 0;

    // uint16_t logNum = pRecvStartCharge->odd_number[0] | pRecvStartCharge->odd_number[1] << 8;
    if (pRecvStartCharge->gun_chrg_mode == E_AHTT_ChargeMode_Start) {
        sum_balance = pRecvStartCharge->gun_chrg_mode_param[0] | pRecvStartCharge->gun_chrg_mode_param[1] << 8;
        SetDetectModeParam(u8Port, eDetectMode_Count, sum_balance / 10);
    } else if (pRecvStartCharge->gun_chrg_mode == E_AHTT_ChargeMode_Full) {
        sum_balance = 0xFFFFFFFF;
    } else {
        pAhttUpInfo->chargeResult_4D = E_AHTT_ChargeResult_AutoStopOff;
        printf("err:ahtt charge mode = %d\r\n", pRecvStartCharge->gun_chrg_mode);
        return;
    }
    pAhttUpInfo->chargeMode_4D = pRecvStartCharge->gun_chrg_mode;
    
    printf("ahtt_startCharge gunnum = %d\r\n", pRecvStartCharge->gun_no);
    printf("ahtt_startCharge mode = %d\r\n", pRecvStartCharge->gun_chrg_mode);
    printf("ahtt_startCharge money = %d\r\n", sum_balance);
    printf("ahtt_startCharge TimeStep = %d\r\n", pRecvStartCharge->gun_chrg_step);

    for (uint8_t costI = 0; costI < pRecvStartCharge->gun_chrg_step; costI++ ) {
        printf("endTime = %d\r\n", pRecvStartCharge->ahCostModel[costI].endTime);
        printf("eleMny = %d\r\n", pRecvStartCharge->ahCostModel[costI].ele_fee);
        printf("serMny = %d\r\n", pRecvStartCharge->ahCostModel[costI].ser_fee);
    }

    uint8_t u8chanel = pRecvStartCharge->gun_no;
    if (u8Port > GUN_NUM) {
        printf("err:ahtt charge channel = %d\r\n", u8Port);
        pAhttUpInfo->chargeResult_4D = E_AHTT_ChargeResult_DeviceIDErr;
        return;
    }

    pAhttUpInfo->gun_chrg_step = pRecvStartCharge->gun_chrg_step;
    memcpy(pAhttUpInfo->ahCostModel, pRecvStartCharge->ahCostModel, sizeof(pRecvStartCharge->ahCostModel));
	

    uint8_t reason = 0;
	if(TRUE == AHTT_StartChargeResult(u8chanel, \
		&reason, \
		eUP_Start_Style_App, \
		NULL, \
		pRecvStartCharge->odd_number, \
		&sum_balance))
	{
		pAhttUpInfo->chargeResult_4D = E_AHTT_ChargeResult_Sucs;
		fgv_CtrlStartCharge(u8chanel);
	}
	else
	{
		pAhttUpInfo->chargeResult_4D = ahttGetStartFault(u8Port, reason);
        printf("err:ahtt charge fail %d %d\r\n", u8Port, pAhttUpInfo->chargeResult_4D);
		// fgv_CtrlStopCharge(u8chanel);
	}
	
	SetSendEnable(u8chanel, AHTT_Auth, SEND_ENABLE_ON);
	Send_Immediately(u8chanel, AHTT_Auth);

	return;
}

uint8_t AHTT_revc_SetTemper(U8 *r_data, int len, uint8_t* gun){
	AHTT_Recv_SetTemper *pRecvStartCharge = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTemper;
	memcpy(pRecvStartCharge, r_data, sizeof(AHTT_Recv_Start_Charge));
	return TRUE;
}

void AHTT_revc_SetTemper_Succ(uint8_t u8Port){
	return;
}




uint8_t Ahtt_recv_realTime_data_parse(U8 *r_data, int len, uint8_t* gun)
{
    * gun = r_data[0];
	return TRUE;
}

void Ahtt_recv_realTime_Succ(uint8_t u8Port)
{
	SetRecvEnable(u8Port, AHTT_RealData, RECV_ENABLE_OFF);
	return;
}

uint8_t Ahtt_recv_Stop_Charge(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[2];
	AHTT_Recv_Stop_Charge *pRecvStopCharge = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	pRecvStopCharge = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvStopCharge;
	
	memcpy(pRecvStopCharge, r_data, sizeof(AHTT_Recv_Stop_Charge));
	
    *gun = pRecvStopCharge->gun_no;

	return TRUE;
}

void Ahtt_recv_Stop_Charge_Succ(uint8_t u8Port)
{
	stopPileCharge(u8Port, Pile_Stop_Reason_APP);
	return;
}

uint8_t Ahtt_recv_Record_Ack(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[0];
	AHTT_Recv_Record_Ack *pRecvRecordAck = NULL;
	pRecvRecordAck = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvRecordAck;
	
    pRecvRecordAck->gunNum = r_data[0];

    * gun = pRecvRecordAck->gunNum;

	if(u8Port >= GUN_NUM) return FALSE;

	return TRUE;
}

void Ahtt_recv_Record_Succ(uint8_t u8Port)
{
	SetRecvEnable(u8Port, AHTT_Chg_Record, RECV_ENABLE_OFF);
	SetSendEnable(u8Port, AHTT_Chg_Record, SEND_ENABLE_OFF);
    
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
    AHTT_charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.AhttChgRecord;
    pRecord->stop_reason = E_AHTT_Reason_Finish;

    GNUpChargeStorageDeal(u8Port, UpRecord->RecordData, sizeof(UpRecord->RecordData) - 1);

	return;
}

//刷卡接收
uint8_t Ahtt_recv_CardAuth_Ack(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[2];
    *gun = u8Port;
	AHTT_Recv_Auth_Ack *pRecvCardAuthAck = NULL;
	pRecvCardAuthAck = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvAuthAck;

	if(u8Port >= GUN_NUM) return FALSE;

	memcpy(pRecvCardAuthAck, r_data, offsetof(AHTT_Recv_Auth_Ack, baseEleMny));
    
    uint8_t tLen = offsetof(AHTT_Recv_Auth_Ack, baseEleMny);

    uint8_t tLen2 = tLen + 6; /* 这里的6是指功率阶梯的6个字节*/

    pRecvCardAuthAck->gun_chrg_step = r_data[tLen2];
    
	memcpy(&pRecvCardAuthAck->ahCostModel, &r_data[tLen2 + 1], pRecvCardAuthAck->gun_chrg_step * 3);

	return TRUE;
}

void Ahtt_recv_CardAuth_Succ(uint8_t u8Port)
{
	AHTT_Recv_Auth_Ack *pRecvCardAuthAck = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvAuthAck;
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];
    uint32_t sum_balance = 0;
	uint8_t reason = 0;
    uint32_t card_auth_prev_energy = 0; /* 刷卡鉴权前的电量 */
	//鉴权状态退出
	monitor_set_MonitorState(u8Port, eMonitorState_Service);

	SetSendEnable(u8Port, AHTT_ChgCard_Record, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, AHTT_ChgCard_Record, RECV_ENABLE_OFF);

    pCardData->CardAuthIng = AHTT_CFG_SWIPE_CARD_INIT_STATE;
    pCardData->CardCnt = 0;
    
    if (pRecvCardAuthAck->rateType !=  0x4D)
    {
        printf("erro: card rateType 0x%x\r\n", pRecvCardAuthAck->rateType);
    #if AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE
        /* 停止充电，且不产生订单 */
        stopPileCharge(u8Port, Pile_Stop_Reason_Other);
    #endif
        return;
    }

    if (pRecvCardAuthAck->startResult != 0x01)
    {
        printf("erro: card StartResult 0x%x\r\n", pRecvCardAuthAck->startResult);
        SetPlat_CardChargeFaild(u8Port, 1);
    #if AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE
        /* 停止充电，且不产生订单 */
        stopPileCharge(u8Port, Pile_Stop_Reason_Other);
    #endif
        return;
    }

    pAhttUpInfo->gun_chrg_step = pRecvCardAuthAck->gun_chrg_step;
    memcpy(pAhttUpInfo->ahCostModel, pRecvCardAuthAck->ahCostModel, sizeof(pRecvCardAuthAck->ahCostModel));

    //设置充电金额
    sum_balance = (pRecvCardAuthAck->ChargePara[2] << 16) | (pRecvCardAuthAck->ChargePara[1] << 8) | pRecvCardAuthAck->ChargePara[0];
    printf("card balance: %d\r\n", sum_balance);
    SetDetectModeParam(u8Port, eDetectMode_Count, sum_balance / 10);

	card_auth_prev_energy = monitor_getChgTotalEnergy(u8Port);

	if (TRUE == AHTT_StartChargeResult(u8Port, \
		                               &reason, \
		                               eUP_Start_Style_CardOnline, \
		                               NULL, \
		                               pRecvCardAuthAck->odd_number, \
		                               &sum_balance))
    {
    #if AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE
        Cost_charge_init(u8Port);/* 清除刷卡启动充电无计费模型前的充电电量等信息 */
        /* 计算刷卡鉴权前的电量和费用 */
        Cost_AppendExtEnergy(u8Port, card_auth_prev_energy);
    #else
        fgv_CtrlStartCharge(u8Port);
    #endif
    }
    else
    {
        printf("StartChargeResult Fail: %d\r\n", reason);
        SetPlat_CardChargeFaild(u8Port, 1);
    #if AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE
        /* 停止充电，且不产生订单 */
        stopPileCharge(u8Port, Pile_Stop_Reason_Other);
    #endif
    }

	return;
}

uint8_t Ahtt_recv_set_param(U8 *r_data, int len, uint8_t* gun)
{
    *gun = 0;
	AHTT_Recv_Set_Param *pRecvSetParam = &g_ProtocolDCB.pAHTTRecvData[*gun].strRecvSetParam;
	memcpy(pRecvSetParam, r_data, sizeof(AHTT_Recv_Set_Param));
	
	return TRUE;
}

void Ahtt_recv_param_Succ(uint8_t u8Port)
{
    AHTT_FlashPlatInfo *pAhttFlashInfo = &s_AhttFlashInfo;

	AHTT_Recv_Set_Param *pRecvSetParam = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvSetParam;
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    pAhttUpInfo->upResult = 1;
    if (pRecvSetParam->uploadcyc >= 1 && pRecvSetParam->uploadcyc <= 30)
    {
        pAhttUpInfo->upResult = 1; // 设置成功

        memcpy(&pAhttFlashInfo->Param_0x84.cardmoney, &pRecvSetParam->cardmoney, sizeof(AHTT_Recv_Set_Param));
        //存储
        AHTT_WriteStoragePara(pAhttFlashInfo);
    }
    else
    {
        pAhttUpInfo->upResult = 0; // 设置失败
    }

	SetSendEnable(u8Port, AHTT_Equipara, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Equipara);
	return;
}


uint8_t Ahtt_recv_search_param(U8 *r_data, int len, uint8_t* gun)
{	
	return TRUE;
}

void Ahtt_recv_search_param_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_Sea_Equipara, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_Sea_Equipara);
	return;
}

//获取设备功率
uint8_t Ahtt_recv_getPower(U8 *r_data, int len, uint8_t* gun)
{	
    * gun = r_data[0];
	return TRUE;
}

void Ahtt_recv_getPower_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_GetPower, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_GetPower);
	return;
}

uint8_t Ahtt_recv_set_time(U8 *r_data, int len, uint8_t* gun)
{
    uint32_t l_time = r_data[3] << 24 | r_data[2] << 16 | r_data[1] << 8 | r_data[0];
	setCurrentRunTimeStamp(l_time);
	return TRUE;
}

void Ahtt_recv_set_time_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, AHTT_TimeSyn_ACK, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, AHTT_TimeSyn_ACK, RECV_ENABLE_OFF);
	return;
}


uint8_t AHTT_recv_DeviceState(U8 *r_data, int len, uint8_t* gun)
{
	AHTT_Recv_AHTT_State *devicesta = &g_ProtocolDCB.pAHTTRecvData[0].strRecvDevState;
	devicesta->Report_results = r_data[0];
	return TRUE;
}

void AHTT_recv_DeviceState_Succ(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

    PlatHeartTickRefresh(); //心跳周期太长了，否则会超时重连

	SetRecvEnable(u8Port, AHTT_State, RECV_ENABLE_OFF);
    
	return;
}

uint8_t AHTT_recv_Alarm(U8 *r_data, int len, uint8_t* gun)
{
	return TRUE;
}

void AHTT_recv_Alarm_Succ(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    SetSendEnable(u8Port, AHTT_Alarm, SEND_ENABLE_OFF);
	return;
}

uint8_t AHTT_recv_Temper_Alarm(U8 *r_data, int len, uint8_t* gun)
{
	return TRUE;
}

void AHTT_recv_Temper_Alarm_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, AHTT_Temper_Alarm, SEND_ENABLE_OFF);
	return;
}


uint8_t AHTT_recv_Network_Alarm(U8 *r_data, int len, uint8_t* gun)
{
	return TRUE;
}

void AHTT_recv_Network_Alarm_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, AHTT_Network_Alarm, SEND_ENABLE_OFF);
	return;
}



//平台下发温度告警值
uint8_t Ahtt_recv_SetTemper(U8 *r_data, int len, uint8_t* gun)
{
	AHTT_Recv_SetTemper *pRecvSetTime = &g_ProtocolDCB.pAHTTRecvData[* gun].strRecvSetTemper;
	
    pRecvSetTime->Temalarm = r_data[0];
	
	return TRUE;
}

void Ahtt_recv_SetTemper_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, AHTT_SetTemper, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_SetTemper);
	
	AHTT_Recv_SetTemper *pRecvSetTime = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvSetTemper;
    
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;

	uint8_t alarmeTemp = pRecvSetTime->Temalarm;

	if (alarmeTemp >= 50 && alarmeTemp <= 120)
     {
        pRecvSetTime->Temalarm = alarmeTemp;
         pAhttUpInfo->upResult = 1; // 设置成功
     }
     else
     {
        pAhttUpInfo->upResult = 0; // 设置失败
     }

	return;
}


uint8_t Ahtt_recv_ftp(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	AHTT_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvFtp;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvFtp, r_data, sizeof(AHTT_Recv_Update_ftp));
	
	return TRUE;
}


void Ahtt_recv_ftp_Succ(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    pAhttUpInfo->upResult = 0;
	AHTT_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pAHTTRecvData[u8Port].strRecvFtp;

    g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;				//升级
    g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间

    char aUpd[8] = "aUpdate";
    char bUpd[8] = "bUpdate";

    char password[9] = {'\0'};
    char tPsw[9] = {'\0'};
    memcpy(password, pRecvFtp->UpdateFileName, sizeof(pRecvFtp->UpdateFileName));
    
    printf("%s\r\n", password);

    
    if (memcmp(password, aUpd, 7) == 0) {
        memcpy(tPsw, aUpd, sizeof(aUpd));
    } else if (memcmp(password, bUpd, 7) == 0) {
        memcpy(tPsw, bUpd, sizeof(bUpd));
    } else {
        pAhttUpInfo->upResult = 1;      //失败

        g_ProtocolDCB.PlatTask.updata_flag = E_Update_Null;				//升级
        g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间
    }

    g_PileUpdateInterface("admin", 21, "ahttroot", tPsw, NULL, NULL);

	SetSendEnable(u8Port, AHTT_set_update_ftp, SEND_ENABLE_ON);
	Send_Immediately(u8Port, AHTT_set_update_ftp);

	return;
}

uint8_t AHTTUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    static uint8_t heartSendCnt = 0;
	int32_t start_tick = GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	// Cyc += eTick_5S;

	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

typedef uint8_t (*PRecvTimer)(uint8_t u8Port, uint32_t cmd, uint32_t OutTimer);
typedef uint8_t (*PRecv)(U8 *r_data, int len, uint8_t* gun);
typedef void (*PRecvSucc)(uint8_t u8Port);

typedef struct
{
    uint32_t		cmd;
    uint32_t     	timer;
    uint8_t     	MaxCnt;
    PRecvTimer 		pRecvTimer;
    PRecv 			pRecv;
    PRecvSucc 		pRecvSucc;
}AHTT_Recv_ctrl;

const AHTT_Recv_ctrl StrAHTTRecvCtrl[] = {
	{AHTT_Identification		,eTick_30S,		   0,       AHTTUpCtrlRecvTimer	,Ahtt_recv_identification_data_parse	,Ahtt_recv_identification_Succ},		//
	{AHTT_Heart_Set           	,0xffffffff,       0,       AHTTUpCtrlRecvTimer	,Ahtt_recv_set_heart_cyc			,Ahtt_recv_set_heart_cyc_Succ},//
    {AHTT_Heart 				,eTick_10S,		   3,       AHTTUpCtrlRecvTimer 	,Ahtt_recv_heart_data_parse			,Ahtt_recv_heart_Succ},
	{AHTT_Port_Domain         	,0xffffffff,	    0,      AHTTUpCtrlRecvTimer   ,AHTT_recv_Port_Domain				,AHTT_recv_Port_Domain_Succ},//
    {AHTT_MaxChgTime          	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   ,AHTT_recv_MaxChgTime                     ,AHTT_recv_MaxChgTimeSucc},//
    {AHTT_Sea_MaxChgTime      	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   ,AHTT_recv_Sea_MaxChgTime				,AHTT_recv_Sea_MaxChgTime_Succ},//
    {AHTT_Auth                	,eTick_15S,		   0,       AHTTUpCtrlRecvTimer   ,AHTT_recv_Auth            			,AHTT_recv_Auth_Succ},
    {AHTT_RealData            	,eTick_10S,		   3,       AHTTUpCtrlRecvTimer   ,Ahtt_recv_realTime_data_parse        ,Ahtt_recv_realTime_Succ},
    {AHTT_Stop_Chg_Ack        	,0xffffffff,		0,       AHTTUpCtrlRecvTimer   ,Ahtt_recv_Stop_Charge             	,Ahtt_recv_Stop_Charge_Succ},
    {AHTT_Chg_Record          	,eTick_10S,		    3,      AHTTUpCtrlRecvTimer   ,Ahtt_recv_Record_Ack   				,Ahtt_recv_Record_Succ},
    {AHTT_ChgCard_Record      	,eTick_10S,		    3,      AHTTUpCtrlRecvTimer   ,Ahtt_recv_CardAuth_Ack               ,Ahtt_recv_CardAuth_Succ},//
    {AHTT_Equipara            	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   ,Ahtt_recv_set_param            		,Ahtt_recv_param_Succ},
    {AHTT_Sea_Equipara        	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   , Ahtt_recv_search_param,                     Ahtt_recv_search_param_Succ},//查询设备参数
    {AHTT_GetPower        	    ,0xffffffff,		0,      AHTTUpCtrlRecvTimer   , Ahtt_recv_getPower,                     Ahtt_recv_getPower_Succ},//获取设备功率

    {AHTT_TimeSyn_ACK         	,eTick_15S,		   3,       AHTTUpCtrlRecvTimer   ,Ahtt_recv_set_time         		,Ahtt_recv_set_time_Succ},
    {AHTT_State               	,eTick_10S,		   3,       AHTTUpCtrlRecvTimer   ,AHTT_recv_DeviceState				,AHTT_recv_DeviceState_Succ},//设备状态
    {AHTT_Alarm               	,eTick_10S,		    3,      AHTTUpCtrlRecvTimer   ,AHTT_recv_Alarm						,AHTT_recv_Alarm_Succ},//告警
    {AHTT_Network_Alarm       	,eTick_10S,		    3,      AHTTUpCtrlRecvTimer   ,AHTT_recv_Network_Alarm              ,AHTT_recv_Network_Alarm_Succ},//告警
    {AHTT_Temper_Alarm        	,eTick_10S,		    3,      AHTTUpCtrlRecvTimer   ,AHTT_recv_Temper_Alarm               ,AHTT_recv_Temper_Alarm_Succ},//超温告警
    {AHTT_SetTemper           	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   ,Ahtt_recv_SetTemper                     ,Ahtt_recv_SetTemper_Succ},//告警
    {AHTT_set_update_ftp      	,0xffffffff,		0,      AHTTUpCtrlRecvTimer   ,Ahtt_recv_ftp              			,Ahtt_recv_ftp_Succ},
};	

void AHTTUpCtrlRecvDeal(AHTT_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const AHTT_Recv_ctrl *pAHTTRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(StrAHTTRecvCtrl); u32i++)
    {
		pAHTTRecvCtrl = &StrAHTTRecvCtrl[u32i];
		
		if (cmd == pAHTTRecvCtrl->cmd)
		{
			if(TRUE == pAHTTRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pAHTTRecvCtrl->pRecvSucc(port);
				
				SetRecvTick(port, cmd, Get_Systick());
				
				// debug_printf("\r\nUpProtocol --> GUN: %d, RecvDealcmd: %x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}

//判断tcp接收到的所有数据是否合法
static int Tcp_Read_Data_Check(uint8_t *r_data)
{
	uint8_t head = 0xEA;
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
    if (r_data[0] != head) {
        printf("Check head erro  0x%x \r\n", r_data[0]);
        return -1;
    }
    //检查校验，读取所有数据长度
    uint16_t r_len = 0;
    memcpy(&r_len, &r_data[1], 2);
    uint16_t crc_len = r_len + 1;
    
    //对校验位之前的数据进行CRC校验
    uint16_t c_crc = ModbusCRC(r_data, crc_len);

    uint16_t r_crc = 0;
	r_crc = r_data[crc_len + 1] << 8 | r_data[crc_len];

    if (c_crc !=  r_crc) {
        printf("Check crc erro  0x%x  0x%x\r\n", r_crc, c_crc);
        return -2;
    }
	
	//桩码不匹配
	uint8_t dataNum[5] = {0};
	monitor_getDevNumber(dataNum, 5);
	Reverse_order(dataNum, 5);
	if (memcmp(&r_data[4], dataNum, 5) != 0) {
		printf("Check DevNumber erro \r\n");
		return -2;
	}

    return 0;
}

void AHTTfrom_buffer_data(U8 *recv_buf, int *len)
{
    //从buffer里查找合法数据进行校验
    U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, (U16 *)len, TCP_DATA_LEN_MAX);
	// hex_dump("tcp_recv_data:", recv_buf, *len);
	 uint8_t head = 0xEA;
    if (recv_buf[0] == head) {
        //继续寻找len
		read_len = *len;
		// read_len = twoUint8ToUint16(&recv_buf[8]);
        if (read_len > TCP_DATA_LEN_MAX) {
			printf("\r\nprotocol--> recv buf full ! ");
            return;
        }
        *len = read_len;
    }
}


void AHTT_PackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	AHTT_HEAD_T *pHead = NULL;

	while(surplusLen) {
		pHead = (AHTT_HEAD_T*)(recv_buf + currentIndex);

		int packLen = twoUint8ToUint16(&pHead->len[0]);
		//防止乱数据导致程序死掉
		if (packLen > surplusLen) {
			return;
		}
		surplusLen = surplusLen - packLen;

		// printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);
		
		if (Tcp_Read_Data_Check(recv_buf + currentIndex) < 0) {
			return;
		}
        // LogPrintf(LVL_LOG_WARN, "\r\n");

		printf("\r\nUpProtocol --> RecvDealcmd: 0x%x \r\n", pHead->cmd);

		hex_dump("tcp_recv_data:", recv_buf + currentIndex, packLen+3);

		AHTTUpCtrlRecvDeal(pHead, pHead->cmd, recv_buf+currentIndex+sizeof(AHTT_HEAD_T), packLen);
		
		currentIndex = currentIndex + packLen;
		}
}
void AHTTUpRecvDeal(void)
{
    U8 from_tcp_data[TCP_DATA_LEN_MAX];
    int r_len = 0;
	AHTT_HEAD_T *pHead = NULL;
	
    AHTTfrom_buffer_data(from_tcp_data, &r_len);
    if (r_len == 0) 
        return;
    if (r_len > TCP_DATA_LEN_MAX) 
        return;
	
	//粘包处理
	AHTT_PackConnectHandle(from_tcp_data, r_len);

	return;
}

void AHTTRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if(AHTT_Auth == cmd)
	{
		SetSendEnable(u8Port, AHTT_Auth, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, AHTT_Auth, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
        AhttClearCardData(u8Port);
	}
	
	return;
}

void AHTTRecvOutCntDeal(uint8_t u8Port, uint32_t cmd)
{
	if((AHTT_Chg_Record == cmd)         //订单
    ||(AHTT_ChgCard_Record == cmd))     //充电
	{
        SetSendEnable(u8Port, cmd, SEND_ENABLE_OFF);
	}
    SetRecvEnable(u8Port, cmd, RECV_ENABLE_OFF);

	return;
}

void AHTTUpCtrlRecvOutTime(void)
{
	const AHTT_Recv_ctrl *pJXRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrAHTTRecvCtrl); u32i++)
	    {
			pJXRecvCtrl = &StrAHTTRecvCtrl[u32i];
			
			if(RECV_ENABLE_ON != GetRecvEnable(i, pJXRecvCtrl->cmd))
				continue;
			
			if (TRUE == pJXRecvCtrl->pRecvTimer(i, pJXRecvCtrl->cmd, pJXRecvCtrl->timer))
			{
                if (pJXRecvCtrl->MaxCnt) {
                    uint8_t RptCnt = GetRecvRptCnt(i, pJXRecvCtrl->cmd);
                    if (RptCnt < pJXRecvCtrl->MaxCnt) {
                        //开启重新发送
                        Send_Immediately(i, pJXRecvCtrl->cmd);
                        SetRecvTick(i, pJXRecvCtrl->cmd, Get_Systick());
                        SetRecvRptUpt(i, pJXRecvCtrl->cmd);                 //发送次数自增，发送时次数清零
                    } else {
                        AHTTRecvOutCntDeal(i, pJXRecvCtrl->cmd);    //重复次数超次处理
                    }
                }
				AHTTRecvOutTimeDeal(i, pJXRecvCtrl->cmd);
//				SetRecvTick(i, pJXRecvCtrl->cmd, Get_Systick());
			}
		}
	}
	return;
}

//================================================================================================
//================================================================================================
void AHTT_CardAuthStart_Cmd(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    uint32_t card_balance = AHTT_CFG_SWIPE_CARD_UNAUTH_CARD_VIRTUAL_BALANCE;
    uint8_t fail_reason = 0;

    switch(pCardData->CardAuthIng)
    {
        case AHTT_CFG_SWIPE_CARD_INIT_STATE:
        {
            memcpy(pCardData->Ahtt_PhyCard_number, pChgGunData->PhyCard_number, GNDATA_PHYCARD_LEN);
    #if AHTT_CFG_SWIPE_CARD_UNAUTH_CHARGE_ENABLE
            /* 计费模型初始化 金额模式 金额设置1元 充电时段阶梯个数设置为1, 0~24 */
            pAhttUpInfo->gun_chrg_step = 1;
            memset(pAhttUpInfo->ahCostModel, 0, sizeof(pAhttUpInfo->ahCostModel));
            pAhttUpInfo->ahCostModel[0].endTime = 24;
            SetDetectModeParam(u8Port, eDetectMode_Count, card_balance/10);
            /* 直接启动充电，不计费，不可刷卡停止充电，*/
            if (TRUE == AHTT_StartChargeResult(u8Port, &fail_reason, eUP_Start_Style_CardOnline, NULL, NULL, &card_balance))
            {
                fgv_CtrlStartCharge(u8Port);
                pCardData->CardAuthIng = AHTT_CFG_SWIPE_CARD_READY_STATE;
            }
            else
            {
                SetPlat_CardChargeFaild(u8Port, 1);
            }
            pChgGunData->ExistChargeDeal = 0;
    #endif
            break;
        }
        case AHTT_CFG_SWIPE_CARD_AUTH_STATE:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    if (pCardData->CardAuthIng == AHTT_CFG_SWIPE_CARD_READY_STATE)
    {
        if (0 == memcmp(pCardData->Ahtt_PhyCard_number, pChgGunData->PhyCard_number, GNDATA_PHYCARD_LEN)) 
        {
            pCardData->CardCnt++;
            printf("card cnt:%d\r\n", pCardData->CardCnt);
        }
        else
        {
            printf("Erro: card number not same\r\n");
        }
    }
}

static void AHTT_CardTimeoutAuthStartScan(uint8_t u8Port)
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    AHTT_CardStartData *pCardData = &pAhttUpInfo->CardStartData[u8Port];
    
    //鉴权中不进行再次鉴权
    if (pCardData->CardAuthIng == AHTT_CFG_SWIPE_CARD_AUTH_STATE) {
        return;
    }
    //拔枪需要重新计数重置
    if (GetPile_gun_connect(u8Port) == 0) {
        AhttClearCardData(u8Port);
        return;
    }

    if (pCardData->CardCnt == 0) {
        pCardData->CardTimeInit = NOWTICK;
    }


    if (JudgeTimeOutMs(pCardData->CardTimeInit, AHTT_CFG_SWIPE_CARD_VALID_TIME) == FALSE) {
        return;
    }

    pCardData->CardTimeInit = NOWTICK;
    pCardData->CardAuthIng = AHTT_CFG_SWIPE_CARD_AUTH_STATE;
    
	//插枪状态下刷有效卡，进行充电鉴权
    if(SEND_ENABLE_ON == GetSendEnable(u8Port, AHTT_ChgCard_Record)) {
        return;
    }

    SetSendEnable(u8Port, AHTT_ChgCard_Record, SEND_ENABLE_ON);
    Send_Immediately(u8Port, AHTT_ChgCard_Record);

    printf("card auth %d\n", pCardData->CardCnt);
}


void AHTT_DealUpdate_Cmd(uint8_t u8Port)
{
    uint8_t t_Chg_Record = AHTT_Chg_Record;
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    //刷卡和app启动要做区分

    if (pChgGunData->DealRecord.PileStopReason == Pile_Stop_Reason_APP)
    {//如果是APP停止充电，继电器断开后，再应答平台停止充电指令（实测桩大概6秒多后回复），平台下发停止充电指令，超过10秒未收到应答会重发
        SetSendEnable(u8Port, AHTT_Stop_Chg_Ack, SEND_ENABLE_ON);
        Send_Immediately(u8Port, AHTT_Stop_Chg_Ack);
    }
    SetSendEnable(u8Port, t_Chg_Record, SEND_ENABLE_ON);
    Send_Immediately(u8Port, t_Chg_Record);

    //订单上报，关闭0x93通道数据上报
    SetSendEnable(u8Port, AHTT_RealData, SEND_ENABLE_OFF);
}

void AHTTUpLogin(void)
{
    ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;

    if (!Comm_getIpSuces(eSocket_GPRS1)) {
		return;
	}

	if(eOnline_Off == Get_PlatConnectSta())
	{
        Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, AHTT_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, AHTT_Identification);
	}
}

void AHTTUpGunStateCheck(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	uint8_t report_flag[GUN_NUM_MAX] = {0};       //是否需要上报

	static uint8_t preAhttGunState[GUN_NUM_MAX] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};

    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    uint8_t sta = GetPile_gun_state(u8Port);
    
    if (sta < eChargeState_Idle) {
        pAhttUpInfo->ahttGunState[u8Port] = E_AHTT_GunCharge_Free;
    } else if ((sta > eChargeState_Idle) && (sta < eChargeState_Charging)) {
        pAhttUpInfo->ahttGunState[u8Port] = E_AHTT_GunCharge_Charging;
    } else if (sta == eChargeState_PauseB) {
        pAhttUpInfo->ahttGunState[u8Port] = E_AHTT_GunCharge_Pause;
    }  else if (sta == eChargeState_StopFinish) {
        pAhttUpInfo->ahttGunState[u8Port] = E_AHTT_GunCharge_Finish;
    } else if (sta < eChargeState_Stoping) {
        pAhttUpInfo->ahttGunState[u8Port] = E_AHTT_GunCharge_Charging;
    }

	if(preAhttGunState[u8Port] != pAhttUpInfo->ahttGunState[u8Port])
	{
		preAhttGunState[u8Port] = pAhttUpInfo->ahttGunState[u8Port];
		report_flag[u8Port] = TRUE;
	}
	
	if(gun_conn_state[u8Port] != GetPile_gun_connect(u8Port))
	{
		gun_conn_state[u8Port] = GetPile_gun_connect(u8Port);
		report_flag[u8Port] = TRUE;
	}
    //计量状态和急停变化上报
    static GN_PLATMOD  *lpMidPile = &sg_platmod;	//用于本文件中
    static uint8_t PreErrflag = 0; 
    uint8_t errflag = 0; 
    
    uint8_t u8Port0 = 0;
    if(lpMidPile->gun[u8Port0].gunRtInfo.hardfault.bit.faultEStop_1b
    || lpMidPile->gun[u8Port0].gunRtInfo.hardfault.bit.faultEleMeasure_1b)
    {
        errflag = lpMidPile->gun[u8Port0].gunRtInfo.hardfault.bit.faultEStop_1b
        + lpMidPile->gun[u8Port0].gunRtInfo.hardfault.bit.faultEleMeasure_1b;
    }
    if (errflag != PreErrflag) {
		report_flag[u8Port0] = TRUE;
        PreErrflag = errflag;
    }
	
	if(TRUE == report_flag[u8Port])
	{
        SetSendEnable(0, AHTT_State, SEND_ENABLE_ON);
		Send_Immediately(0, AHTT_State);
	}
	
	return;
}


//过温告警轮询
static void AHTTTEnvTempAlarmScan()
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    //温度告警是否上报
    static uint8_t temp_overTmp = 0;
	AHTT_Recv_SetTemper *pRecvSetTime = &g_ProtocolDCB.pAHTTRecvData[0].strRecvSetTemper;
    if (pRecvSetTime->Temalarm == 0) {
        return;
    }
    int8_t envTemp = GetPile_EvnTem();
    if (pAhttUpInfo->overTempAlm) {
        int8_t diffTemp = envTemp - (pRecvSetTime->Temalarm - 50);
        if (diffTemp) {
            pAhttUpInfo->overTempAlm = 1;
            pAhttUpInfo->overTempValue = envTemp + 50;
        } else {
            pAhttUpInfo->overTempAlm = 0;
            pAhttUpInfo->overTempValue = envTemp + 50;
        }
    }
    if (temp_overTmp == pAhttUpInfo->overTempAlm) {
        return;
    }

    temp_overTmp = pAhttUpInfo->overTempAlm;

    // for (uint8_t i = 0; i < GUN_NUM; i++) {
        Send_Immediately(0, AHTT_Temper_Alarm);
        SetSendEnable(0, AHTT_Temper_Alarm, SEND_ENABLE_ON);
    // }
}

//网络异常告警轮询
static void AHTTNetSgnTempAlarmScan()
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    if (!Comm_getIpSuces(eSocket_GPRS1)) {
		return;
	}

    //网络异常是否上报
    static uint8_t temp_netSgn = 0;

    uint8_t signal = GetNet_SignalLevel();
    if (signal <= 1) {
        pAhttUpInfo->netSgnAlm = 1;
    } else {
        pAhttUpInfo->netSgnAlm = 0;
    }
    if (temp_netSgn == pAhttUpInfo->netSgnAlm) {
        return;
    }

    temp_netSgn = pAhttUpInfo->netSgnAlm;

    Send_Immediately(0, AHTT_Network_Alarm);
    SetSendEnable(0, AHTT_Network_Alarm, SEND_ENABLE_ON);
}
//设备故障异常告警轮询
static void AHTTDevErrAlarmScan()
{
    AHTT_UpPlatInfo *pAhttUpInfo = &s_AhttUpInfo;
    
    //设备故障是否上报
    static uint16_t temp_devErr = 0;
    
    uint16_t tempDevErrAlm = 0;

    for (int i = 0; i < GUN_NUM; i++) {
        uint8_t devErr = GetPile_ErrState(i);
        
        if (devErr == 1) {
            tempDevErrAlm |= (1 << (15 - i));
        } else {
            tempDevErrAlm &= ~(1 << (15 - i));
        }
    }

    if (temp_devErr == tempDevErrAlm) {
        return;
    }

    temp_devErr = tempDevErrAlm;
    pAhttUpInfo->devErrAlm[0] = tempDevErrAlm >> 8;
    pAhttUpInfo->devErrAlm[1] = tempDevErrAlm & 0xFF;

    SetSendEnable(0, AHTT_Alarm, SEND_ENABLE_ON);
    Send_Immediately(0, AHTT_Alarm);

}
//开始启动停止充电操作
static void AHTTChargingOpenRealData(uint8_t u8Port)
{
    //充电中开启AHTT_RealData上报
    uint8_t chrgFlag = logic_get_gun_charging(u8Port);
    if (chrgFlag) {
        if (SEND_ENABLE_ON != GetSendEnable(u8Port, AHTT_RealData)) {
            SetSendEnable(u8Port, AHTT_RealData, SEND_ENABLE_ON);
            Send_Immediately(u8Port, AHTT_RealData);
        }
    } else {
        SetSendEnable(u8Port, AHTT_RealData, SEND_ENABLE_OFF);
    }
}


static void AHTTGetPlatTimeScan()
{
    //30min获取一次
    static uint32_t getTime = 0;
    if (JudgeTimeOutMs(getTime, eTick_60S * 30) == FALSE) {
        return;
    }
    getTime = Get_Systick();
    SetSendEnable(0, AHTT_TimeSyn_ACK, SEND_ENABLE_ON);
    Send_Immediately(0, AHTT_TimeSyn_ACK);
}


static void AHTTAllAlarmScan()
{
    AHTTTEnvTempAlarmScan();
    AHTTNetSgnTempAlarmScan();
    AHTTDevErrAlarmScan();

    AHTTGetPlatTimeScan();      //去平台获取时间
}

void AHTTUpCtrlTaskDeal(void)
{
	uint8_t i = 0;
	
	AHTTUpLogin();
	
	for (i = 0; i < GUN_NUM; i++)
	{
		AHTTUpGunStateCheck(i);
        AHTTChargingOpenRealData(i);  //充电订单实时上报开启关闭

        AHTT_CardTimeoutAuthStartScan(i);
	}
	
	return;
}

void AHTT_TaskInit(void)
{
    static uint8_t firstRead = 0;
    if (firstRead) {
        return;
    }
    firstRead = 1;

    AHTT_ReadStoragePara(&s_AhttFlashInfo);     //配置信息读取
}

void AHTTUpProtocolDeal(void)
{
	ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;

	if(NULL == pTcpDataCtrl->pAHTTRecvData)
		return;

    AHTT_TaskInit();

	AHTTUpCtrlTaskDeal();		//任务状态处理
	AHTTUpRecvDeal();			//接收处理
	AHTTUpCtrlSendDeal();		//发送处理
	AHTTUpCtrlRecvOutTime();	//超时处理

    AHTTAllAlarmScan();
	
	return;
}