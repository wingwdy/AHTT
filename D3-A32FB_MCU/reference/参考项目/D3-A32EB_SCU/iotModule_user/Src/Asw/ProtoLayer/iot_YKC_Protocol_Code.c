#include "iot_YKC_Protocol_Code.h"
#include "iot_GN_Protocol_Code.h"
#include "protocol_data.h"
#include "mbsMaster.h"
#include "maths.h"
#include "modbus.h"
#include "AppMidDataTrans.h"

/*==============================================================*/
#define PROTOCOL_ENCRYPT    0           //是否加密

//==================================================
static uint8_t s_GetDealUpCmd()
{
    uint8_t t_Chg_Record = YKC_S_Chg_Record;

    if (ePlatType_DD == get_ChgParam_plat_type()) {
        t_Chg_Record = YKC_S_Chg_Record_DD;
    }
    
	return t_Chg_Record;
}

/*******************************************************
*
*
*
*
*
*
*******************************************************/
uint16_t send_login_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	uint8_t u8VerLne = 0;
	char cSimID[20] = {0};
	
    //数据区域需要整合
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    //设备类型
    data[data_len] = DEVICE_TYPE;
	data_len++;
	
    data[data_len] = GUN_NUM;
	data_len++;
	
	if(ePlatType_DD == get_ChgParam_plat_type())
	{
		data[data_len] = DD_PROTOCOL_VER;
	}
	else
	{
		data[data_len] = YKC_PROTOCOL_VER;
	}
	data_len++;
	
	u8VerLne = strlen(SOFTWARE_VERSION) > 8 ? 8 : strlen(SOFTWARE_VERSION);
	memcpy(&data[data_len], SOFTWARE_VERSION, u8VerLne);
	data_len += 8;
	
	data[data_len] = 0;
	data_len++;
	
	GetNet_Comm_SimID((uint8_t*)cSimID, 20);
	AsciiPToBCD(cSimID, (char*)&data[data_len], 20);
	data_len += 10;
	
	//1,移动 2,联通 3,电信
	if(eOperator_CMCC == GetNet_Comm_Operator())
	{
		data[data_len] = 0;
	}
	else if(eOperator_CUCC == GetNet_Comm_Operator())
	{
		data[data_len] = 3;
	}
	else if(eOperator_CTCC == GetNet_Comm_Operator())
	{
		data[data_len] = 2;
	}
	else
	{
		data[data_len] = 4;
	}
	data_len++;
	
	return data_len;
}

void send_login_ykc_Succ(uint8_t u8Port)
{
	
	return;
}

uint16_t send_heart_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    data[data_len] = u8Port + 1;
	data_len++;
	
    data[data_len] = dev_getErrState(u8Port);
	data_len++;
	
	return data_len;
}

void send_heart_ykc_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_R_Heart))
	{
		SetRecvEnable(u8Port, YKC_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_R_Heart, Get_Systick());
	}
	
	return;
}

//计费模型验证请求
uint16_t send_Rate_Proving_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
     
    YKC_Recv_Rate_Model pRecvRateModel;
	
    Read_rate_model((void *)&pRecvRateModel,sizeof(YKC_Recv_Rate_Model));
	
	
    //设备编码
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    //计费模型编码
    memcpy(&data[data_len], pRecvRateModel.billing_model, 2);
	data_len += 2;
	
    return data_len;
}

void send_Rate_Proving_ykc_Succ(uint8_t u8Port)
{
	
	return;
}

//充电设备计费模型请求
uint16_t send_Rate_Ask_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    return data_len;
}

void send_Rate_Ask_ykc_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_real_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
    
    if (pChgGunData->ExistChargeDeal == 0) {
	    memset(&data[data_len], 0, GNDATA_TRDNUM_LEN);
    } else {
	    memcpy(&data[data_len], pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);
    }
	data_len += GNDATA_TRDNUM_LEN;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pProtocolDCB->strUpGunData[u8Port].up_gun_state;
	data_len++;
	
	data[data_len] = 2;
	data_len++;

	data[data_len] = GetPile_gun_connect(u8Port);
	data_len++;
	
	Uint16ToTwoUint8(&data[data_len], GetPile_ChgOutVol(u8Port, 1));
	data_len += 2;

	Uint16ToTwoUint8(&data[data_len], GetPile_ChgOutCur(u8Port, 1));
	data_len += 2;

	data[data_len] = monitor_getGunTem(u8Port);
	data_len++;

	memset(&data[data_len], 0, 8);
	data_len += 8;
	
	//soc
	data[data_len] = 0;
	data_len++;
	
	data[data_len] = 0;
	data_len++;
	
    
	uint32_t TotalTime = monitor_getChgTimer(u8Port)/60;
	if (monitor_getChgTimer(u8Port) % 60 > 30) {
		TotalTime = TotalTime + 1;
	}
	uint32_t TotalPower = monitor_getChgTotalEnergy(u8Port);
	uint32_t LossPower = monitor_getChgTotalLossPower(u8Port);
	uint32_t TotalMoney = monitor_getChgTotalMoney(u8Port);
    
    if (pChgGunData->ExistChargeDeal == 0) {
		TotalTime = 0; TotalPower = 0; LossPower = 0; TotalMoney = 0;
    }

	Uint16ToTwoUint8(&data[data_len], TotalTime);
	data_len += 2;

	Uint16ToTwoUint8(&data[data_len], 0);
	data_len += 2;
	
	uint32ToFourUint8(&data[data_len], TotalPower);
	data_len += 4;
	
	uint32ToFourUint8(&data[data_len], LossPower);
	data_len += 4;

	uint32ToFourUint8(&data[data_len], TotalMoney);
	data_len += 4;
	
	Uint16ToTwoUint8(&data[data_len], get_hard_err_bit(u8Port));
	data_len += 2;
	
    return data_len;
}

void send_real_ykc_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_auth_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = 1;
	data_len++;
	
	data[data_len] = 0;
	data_len++;
	
	data_len += 4;
	reverse(&pChgGunData->PhyCard_number, &data[data_len], GNDATA_PHYCARD_LEN);
	// memcpy(&data[data_len], pChgGunData->PhyCard_number, GNDATA_PHYCARD_LEN);
	data_len += 4;
	
	memset(&data[data_len], 0, 16);
	data_len += 16;
	
	memset(&data[data_len], 0, 17);
	data_len += 17;
	
    return data_len;
}

void send_auth_ykc_Succ(uint8_t u8Port)
{
	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_R_Auth))
	{
		SetRecvEnable(u8Port, YKC_R_Auth, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_R_Auth, Get_Systick());
	}
	
	return;
}

uint16_t send_start_ack_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	YKC_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvStartCharge;
	
	memcpy(&data[data_len], pRecvStartCharge->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pUpGunData->up_start_ret;
	data_len++;
	
	data[data_len] = pUpGunData->up_start_fail_reason;
	data_len++;
	
    return data_len;
}

void send_start_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_Start_Chg_Ack, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_stop_ack_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pUpGunData->up_stop_ret;
	data_len++;
	
	data[data_len] = pUpGunData->up_stop_fail_reason;
	data_len++;
	
    return data_len;
}

void send_stop_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_Stop_Chg_Ack, SEND_ENABLE_OFF);
	return;
}

//交易记录上传 东电
uint16_t send_charge_record_data_dd(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	memcpy(&data[data_len], pRecord->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
	memcpy(&data[data_len], pRecord->device_number, 7);
	data_len += 7;
	
	data[data_len] = pRecord->gun_num;
	data_len++;
	
	Bcd_to_Cp56time2a(&pRecord->chrg_start_time[1], (cp56time2a*)&data[data_len]);
	data_len += sizeof(cp56time2a);
	
	Bcd_to_Cp56time2a(&pRecord->chrg_stop_time[1], (cp56time2a*)&data[data_len]);
	data_len += sizeof(cp56time2a);
	
	memcpy(&data[data_len], pRecord->sharp_rate, 4*16);
	data_len += 4*16;
	
	//东电起止示值4字节,
	if(ePlatType_DD == get_ChgParam_plat_type())
	{
		memcpy(&data[data_len], pRecord->total_start_elec, 4);
		data_len += 4;
		
		memcpy(&data[data_len], pRecord->total_stop_elec, 4);
		data_len += 4;
	}
	else
	{
		memcpy(&data[data_len], pRecord->total_start_elec, 4);
		data_len += 5;
		
		memcpy(&data[data_len], pRecord->total_stop_elec, 4);
		data_len += 5;
	}
	
	memcpy(&data[data_len], pRecord->total_power, 4*3);
	data_len += 4*3;
	
	memcpy(&data[data_len], pRecord->vin, 17);
	data_len += 17;
	
	data[data_len] = pRecord->trade_flag;
	data_len++;
	
	Bcd_to_Cp56time2a(&pRecord->trade_time[1], (cp56time2a*)&data[data_len]);
	data_len += sizeof(cp56time2a);
	
	data[data_len] = pRecord->stop_reason;
	data_len++;
	
	data_len += 4;
	reverse(&pChgGunData->PhyCard_number, &data[data_len], GNDATA_PHYCARD_LEN);		//上传平台订单用物理卡号
	data_len += 4;
    
    uint32_t tTotalMoney = 0;
    memcpy(&tTotalMoney, &pRecord->total_money, 4);
    SetPlat_ChgTotalMoney(u8Port, tTotalMoney);
	
    return data_len;
}

uint16_t send_charge_record_data_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	data_len += send_charge_record_data_dd(u8Port, data, inlen);
	
    return data_len;
}

void send_charge_record_ykc_Succ(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;
	
	if (pChgGunData->upDealCnt >= 5) {
		//上报五次未回复，停止上报
        uint8_t t_Chg_Record = s_GetDealUpCmd();
        
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, t_Chg_Record))
		{
			SetSendEnable(u8Port, t_Chg_Record, SEND_ENABLE_OFF);
		}
	} else if (pChgGunData->upDealCnt == 1) {
        pChgGunData->ExistChargeDeal = 0;
    }
    
	return;
}

uint16_t send_Sum_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	YKC_Recv_SumUpdata *pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvSumUpdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	memcpy(&data[data_len], pRecvSumUpdata->Physics_card_number, GNDATA_CARD_LEN);
	data_len += GNDATA_CARD_LEN;
	
	data[data_len] = pChgGunData->sum_updata_ret;
	data_len++;
	
    return data_len;
}

void send_Sum_ACK_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_Sum_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_Para_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = 1;
	data_len++;
	
    return data_len;
}

void send_Para_ACK_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port,YKC_S_Para_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_TimeSyn_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	tm_struct strCurTime = get_current_time();
    Bin_to_Cp56time2a((uint8_t *)(&strCurTime.yearL), (cp56time2a*)&data[data_len]);
	data_len += 7;
	
    return data_len;
}

void send_TimeSyn_ACK_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_TimeSyn_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_Rate_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = 1;
	data_len++;
	
    return data_len;
}

void send_Rate_ACK_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_Rate_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_reboot_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = 1;
	data_len++;
	
    return data_len;
}

void send_reboot_ACK_ykc_Succ(uint8_t u8Port)
{
	
	SetSendEnable(u8Port, YKC_S_reboot_ACK, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_update_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = ptcp_data->up_update_ret;
	data_len++;
    
    return data_len;
}

void send_update_ACK_ykc_Succ(uint8_t u8Port)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	SetSendEnable(u8Port, YKC_S_update_ACK, SEND_ENABLE_OFF);
	
	//应答之后再升级
	pProtocolDCB->PlatTask.updata_delay_tick = Get_Systick();
	
	return;
}

uint16_t send_QR_ACK_ykc(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port+1;
	data_len++;

	data[data_len] = 1;
	data_len++;
	
    return data_len;
}

void send_QR_ACK_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_QR_ACK, SEND_ENABLE_OFF);
	return;
}

void send_QR_ACK_Succ_DD(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_QR_ACK_DD, SEND_ENABLE_OFF);
	return;
}


uint8_t UpCtrlSendCyc(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	uint32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	if(TRUE == u8SendImmdFlag)
		return TRUE;
	
	if(YKC_S_RealData == cmd)
	{
		if(eUP_Gun_State_Work == pUpGunData->up_gun_state)
			Cyc = eTick_15S;
	}
	
	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}


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
}YKC_Send_ctrl;

#define  JX_SEND_IMMD 0

const YKC_Send_ctrl StrYKCSendCtrl[]={
    {YKC_S_Identification	,UP_S_FRAME_SELF	,eTick_60S,		UpCtrlSendCyc	,send_login_data_ykc			,send_login_ykc_Succ},		//
	{YKC_S_Heart 			,UP_S_FRAME_SELF	,eTick_10S,		UpCtrlSendCyc 	,send_heart_data_ykc			,send_heart_ykc_Succ},
	
	{YKC_S_Rate_Proving		,UP_S_FRAME_SELF	,eTick_15S,		UpCtrlSendCyc	,send_Rate_Proving_ykc			,send_Rate_Proving_ykc_Succ},		//
	{YKC_S_Rate_Ask			,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc 	,send_Rate_Ask_data_ykc			,send_Rate_Ask_ykc_Succ},
	{YKC_S_RealData			,UP_S_FRAME_SELF 	,(eTick_60S*5),	UpCtrlSendCyc 	,send_real_data_ykc				,send_real_ykc_Succ},
	
	{YKC_S_Auth				,UP_S_FRAME_SELF	,eTick_15S,		UpCtrlSendCyc 	,send_auth_data_ykc				,send_auth_ykc_Succ},
	
	{YKC_S_Start_Chg_Ack	,UP_S_FRAME_ACK		,eTick_15S,		UpCtrlSendCyc 	,send_start_ack_ykc				,send_start_ykc_Succ},
	{YKC_S_Stop_Chg_Ack		,UP_S_FRAME_ACK		,eTick_15S,		UpCtrlSendCyc 	,send_stop_ack_ykc				,send_stop_ykc_Succ},
	{YKC_S_Chg_Record		,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc	,send_charge_record_data_ykc	,send_charge_record_ykc_Succ},
	
	{YKC_S_Sum_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_Sum_ACK_ykc				,send_Sum_ACK_ykc_Succ},
	{YKC_S_Para_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_Para_ACK_ykc				,send_Para_ACK_ykc_Succ},
	{YKC_S_TimeSyn_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_TimeSyn_ACK_ykc			,send_TimeSyn_ACK_ykc_Succ},
	{YKC_S_Rate_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_Rate_ACK_ykc				,send_Rate_ACK_ykc_Succ},
	{YKC_S_reboot_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_reboot_ACK_ykc			,send_reboot_ACK_ykc_Succ},
	{YKC_S_update_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_update_ACK_ykc			,send_update_ACK_ykc_Succ},
	{YKC_S_QR_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_QR_ACK_ykc				,send_QR_ACK_ykc_Succ},
	{YKC_S_QR_ACK_DD		,UP_S_FRAME_ACK 	,eTick_30S,		UpCtrlSendCyc	,send_QR_ACK_ykc				,send_QR_ACK_Succ_DD},

};

static uint16_t YKC_dataEncode(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	YKC_HEAD_T *pHeart = (YKC_HEAD_T*)p;
	uint16_t all_len = data_len[0] + 6 + 2;
    uint16_t crc_len = data_len[0] + 4;
	uint16_t crc = 0;
    
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
    //number自增
    //前导域、版本域、序号域、加密标志、命令字、长度域、数据域、校验域
    pHeart->head = YKC_FRAME_HEAD;
	
	if((YKC_S_Chg_Record == cmd) && (ePlatType_DD == get_ChgParam_plat_type()))
	{
		cmd = YKC_S_Chg_Record_DD;
	}
	
    if ((cmd == YKC_S_RealData) && (GetSendSrm(u8Port, cmd))) {
        Uint16ToTwoUint8(pHeart->ser, GetSendSrm(u8Port, cmd));
        SetSendSrm(u8Port, cmd, 0);
    }
	else if(UP_S_FRAME_ACK == type)
	{
		Uint16ToTwoUint8(pHeart->ser, GetSendSrm(u8Port, cmd));
	}
	else
	{
		Uint16ToTwoUint8(pHeart->ser, pUpGunData->up_srm);
        pUpGunData->up_srm++;
	}
	
	pHeart->EncType = PROTOCOL_ENCRYPT;
	
	pHeart->cmd = cmd;
	
	pHeart->len = crc_len;
	
    //对校验位之前的数据进行CRC校验
    crc = CRC16(pHeart->ser, crc_len);
	
    //校验需要低字节在前，高字节在后
    p[crc_len+2] = crc >> 8;
    p[crc_len+3] = crc & 0Xff;

	data_len[0] = all_len;
	
    return all_len;
}

static uint16_t YKCUpCtrlSendDeal(void *pBuf ,uint32_t u32BufSize)
{
	const YKC_Send_ctrl *pYKCSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	YKC_HEAD_T *pHead = (YKC_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + sizeof(YKC_HEAD_T);
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrYKCSendCtrl); u32i++)
		{
			pYKCSendCtrl = &StrYKCSendCtrl[u32i];
			
			if (SEND_ENABLE_ON != GetSendEnable(i, pYKCSendCtrl->cmd))
				continue;
			
			if (TRUE == pYKCSendCtrl->pSendCyc(i, pYKCSendCtrl->cmd, pYKCSendCtrl->cyc))
			{
				if ((outLen = pYKCSendCtrl->pSend(i, pData, u32BufSize)) > 0)
	            {	
	            	YKC_dataEncode(i, (uint8_t*)pHead, pYKCSendCtrl->cmd, pYKCSendCtrl->FType, &outLen);
					pYKCSendCtrl->pSendSucc(i);
					SetSendTick(i, pYKCSendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pYKCSendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pYKCSendCtrl->cmd, FALSE);
					
					UPRINT("\r\nUpProtocol --> GUN: %d, SendDealcmd: %x \r\n", i, pYKCSendCtrl->cmd);
					return outLen;
				}
			}
		}
	}
	
	return outLen;
}

static void YKCUpSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;
	
	outLen = YKCUpCtrlSendDeal(pbuf, sizeof(pbuf));
	
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);
	
	return;
}

/*******************************************************/
//登录认证应答解析
uint8_t recv_login_data_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvIdenf;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvIdenf, r_data, sizeof(YKC_Recv_Identification));
	
	return TRUE;
}

void recv_login_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvIdenf;
	
	if(0 == pRecvIdenf->charge_login_result)
	{
		SetSendEnable(u8Port, YKC_S_Identification, SEND_ENABLE_OFF);

		SetSendEnable(u8Port, YKC_S_Rate_Ask, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_S_Rate_Ask);
	}
	
	return;
}

uint8_t recv_Heart_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;
	YKC_Recv_Heart *pRecvHeart = NULL;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvHeart = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvHeart;
	
	memcpy(pRecvHeart, r_data, sizeof(YKC_Recv_Heart));

	return TRUE;
}

void recv_heart_ykc_Succ(uint8_t u8Port)
{
	uint8_t i = 0;
    
    PlatHeartTickRefresh();
	
    Set_PlatConnectSta(eOnline_Heart);

	dev_clrErrExsit_all(eErr_PlatformOffline, __LINE__);
	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_R_Heart))
	{
		SetRecvEnable(u8Port, YKC_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_R_Heart, Get_Systick());
	}
	
	if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_S_Identification))
	{
		SetSendEnable(u8Port, YKC_S_Identification, SEND_ENABLE_OFF);
	}
	
	//实时数据所有枪都上报
	for(i = 0; i < GUN_NUM_MAX; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, YKC_S_RealData))
		{
			SetSendEnable(i, YKC_S_RealData, SEND_ENABLE_ON);
			Send_Immediately(i, YKC_S_RealData);
		}
	}
	
	return;
}

uint8_t recv_Rate_Proving_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Rate_Proving *pRecvRateProving = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRateProving;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvRateProving, r_data, sizeof(YKC_Recv_Rate_Proving));

	return TRUE;
}

void recv_Rate_Proving_ykc_Succ(uint8_t u8Port)
{
	
	return;
}

uint8_t recv_Rate_Ask_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRateModel;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvRateModel, r_data, sizeof(YKC_Recv_Rate_Model));
	
	return TRUE;
}

void recv_Rate_Ask_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRateModel;
	uint8_t i = 0;
	
	SetSendEnable(u8Port, YKC_S_Rate_Ask, SEND_ENABLE_OFF);
	
	//双枪各自发心跳
	for(i = 0; i < GUN_NUM_MAX; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, YKC_S_Heart))
		{
			SetSendEnable(i, YKC_S_Heart, SEND_ENABLE_ON);
			Send_Immediately(i, YKC_S_Heart);
		}
	}
	GNUpChargeRecordUpDealOffline();	//离线记录上报
	
	Save_rate_model(pRecvRateModel, sizeof(YKC_Recv_Rate_Model));
	
	return;
}

uint8_t recv_RealData_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;

	if(u8Port >= GUN_NUM_MAX) return FALSE;

	gun[0] = u8Port;
	
	return TRUE;
}

void recv_RealData_ykc_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, YKC_S_RealData, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_RealData);
	
	return;
}

uint8_t recv_Auth_Ack_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[23] - 1;
	YKC_Recv_Auth_Ack *pRecvAuthAck = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;

	gun[0] = u8Port;
	
	pRecvAuthAck = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvAuthAck;
	
	memcpy(pRecvAuthAck, r_data, sizeof(YKC_Recv_Auth_Ack));
	
	return TRUE;
}

void recv_Auth_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_Auth_Ack *pRecvAuthAck = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvAuthAck;
	uint8_t up_fail_reason = 0;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t sum_balance = 0;
	
	//鉴权状态退出
	monitor_set_MonitorState(u8Port, eMonitorState_Service);
	
	//云快充物理卡号鉴权,逻辑卡号应答
	if(1 == pRecvAuthAck->Auth_success)
	{
		sum_balance = fourUint8ToUint32(pRecvAuthAck->account_balance);
		monitor_charge_start(u8Port, &up_fail_reason, eUP_Start_Style_CardOnline, \
			pRecvAuthAck->Logic_card_number, \
			pRecvAuthAck->transaction_log_num, \
			&sum_balance);
		
		fgv_CtrlStartCharge(u8Port);
	}
	
	SetSendEnable(u8Port, YKC_S_Auth, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, YKC_R_Auth, RECV_ENABLE_OFF);
	
	return;
}

uint8_t recv_Start_Charge_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[23] - 1;
	YKC_Recv_Start_Charge *pRecvStartCharge = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvStartCharge;
	
	memcpy(pRecvStartCharge, r_data, sizeof(YKC_Recv_Start_Charge));
	
	return TRUE;
}

void recv_Start_Charge_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvStartCharge;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	uint32_t sum_balance = 0;
	
	sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);

	//云快充下发的逻辑卡号,无法用于停止充电,没有屏也不需要显示
	if(TRUE == monitor_charge_start(u8Port, \
		&pUpGunData->up_start_fail_reason, \
		eUP_Start_Style_App, \
		NULL, \
		pRecvStartCharge->transaction_log_num, \
		&sum_balance))
	{
		pUpGunData->up_start_ret = UP_RESULT_SUCC;
		pUpGunData->up_start_fail_reason = eStart_Fail_NULL;
		fgv_CtrlStartCharge(u8Port);
	}
	else
	{
		pUpGunData->up_start_ret = UP_RESULT_FAIL;
		fgv_CtrlStopCharge(u8Port);
	}
	
	SetSendEnable(u8Port, YKC_S_Start_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_Start_Chg_Ack);
	
	return;
}

uint8_t recv_Stop_Charge_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;
	YKC_Recv_Stop_Charge *pRecvStopCharge = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStopCharge = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvStopCharge;
	
	memcpy(pRecvStopCharge, r_data, sizeof(YKC_Recv_Stop_Charge));
  
	return TRUE;
}

void recv_Stop_Charge_ykc_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	stopPileCharge(u8Port, Pile_Stop_Reason_APP);
	
	pUpGunData->up_stop_ret = UP_RESULT_SUCC;
	pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL;
	
	SetSendEnable(u8Port, YKC_S_Stop_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_Stop_Chg_Ack);
	
	return;
}

uint8_t recv_Record_Ack_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
    U8 u8Port = GUN_A;
	YKC_Recv_Record_Ack *pRecvRecordAck = NULL;

    gun[0] = u8Port;
	pRecvRecordAck = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRecordAck;

	memcpy(pRecvRecordAck, r_data, sizeof(YKC_Recv_Record_Ack));

    uint8_t result = 1;
    //查看账单应答属于哪个枪
    for (int i = 0; i < GUN_NUM_MAX; i++) {
        charge_record *pRecord = &g_chgData[i].DealRecord.ChgRecord.GnChgRecord;
        result = memcmp(pRecord->transaction_log_num, pRecvRecordAck->transaction_log_num, GNDATA_TRDNUM_LEN);
        if (result == 0) {
            *gun = i;
            updatePileStopReason(i, Pile_Stop_Reason_Finish);
            GNUpChargeStorageDeal(i, (void *)&g_chgData[i].DealRecord, sizeof(PlatDealRecord));

            break;
        }
    }

    if(u8Port >= GUN_NUM) return FALSE;

    return TRUE;
}

void recv_Record_ykc_Succ(uint8_t u8Port)
{
	
//	if(0 == pRecvRecordAck->result)
	{
		monitor_chgrcd_report_succ();
        uint8_t t_Chg_Record = s_GetDealUpCmd();
		SetSendEnable(u8Port, t_Chg_Record, SEND_ENABLE_OFF);
	}
	
	return;
}

uint8_t recv_Sum_Update_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;
	YKC_Recv_SumUpdata *pRecvSumUpdata = NULL;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvSumUpdata;
	
	memcpy(pRecvSumUpdata, r_data, sizeof(YKC_Recv_SumUpdata));
	
	return TRUE;
}

void recv_Sum_Update_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_SumUpdata *pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvSumUpdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	SetSendEnable(u8Port, YKC_S_Sum_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_Sum_ACK);
	
    pChgGunData->sum_updata_ret = 0;
    pChgGunData->sum_balance = fourUint8ToUint32(pRecvSumUpdata->account_balance);

	return;
}

uint8_t recv_Set_Para_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Para *pRecvPara = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvPara;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvPara, r_data, sizeof(YKC_Recv_Para));
	
	return TRUE;
}

void recv_Set_Para_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_Para_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_Para_ACK);
	
	return;
}

uint8_t recv_TimeSyn_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvTimeSyn;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvTimeSyn, r_data, sizeof(YKC_Recv_TimeSyn));
	
	return TRUE;
}

void recv_TimeSyn_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvTimeSyn;
	
	tm_struct SysTime;
    SysTime.yearH = 20;
    Cp56time2a_to_Bin((uint8_t *)&SysTime.yearL, &pRecvTimeSyn->cur_time);
    setCurrentRunTime((uint8_t *)&SysTime);
	
	SetSendEnable(u8Port, YKC_S_TimeSyn_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_TimeSyn_ACK);
	return;
}

uint8_t recv_Set_Rate_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRateModel;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvRateModel, r_data, sizeof(YKC_Recv_Rate_Model));
	
	return TRUE;
}

void recv_Set_Rate_ykc_Succ(uint8_t u8Port)
{
	YKC_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvRateModel;
	
	SetSendEnable(u8Port, YKC_S_Rate_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_Rate_ACK);
	
	Save_rate_model(pRecvRateModel, sizeof(YKC_Recv_Rate_Model));
	return;
}

uint8_t recv_set_reboot_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvReboot;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvReboot, r_data, sizeof(YKC_Recv_Reboot));
	
	return TRUE;
}

void recv_set_reboot_ykc_Succ(uint8_t u8Port)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	pProtocolDCB->PlatTask.reboot_flag = E_Reboot_Idle;
	pProtocolDCB->PlatTask.reboot_tick = Get_Systick();
	
	SetSendEnable(u8Port, YKC_S_reboot_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_reboot_ACK);
	
	return;
}

uint8_t recv_update_ftp_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_Recv_Update_ftp *pRecvUpdata = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvUpdata;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvUpdata, r_data, sizeof(YKC_Recv_Update_ftp));
	
	return TRUE;
}

void recv_update_ftp_ykc_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    YKC_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvUpdata;

	SetSendEnable(u8Port, YKC_S_update_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_update_ACK);

    uint16_t rate = 0;
    rate = pRecvFtp->device_rate[1] << 8 | pRecvFtp->device_rate[0];
    //设备型号以及设备功率异常不升级,应答失败
    if ((pRecvFtp->device_type != 2) || (rate != (7 * GUN_NUM))) {
        // 应答失败且不升级
        // ptcp_data->up_update_ret = 2;
        // pRecvFtp->update_ctrl = 0;
        // return;
    }

    ptcp_data->up_update_ret = 0;
    g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;				//升级
    g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间
    
    uint16_t u16Port = pRecvFtp->update_port[1] << 8 | pRecvFtp->update_port[0];

    char filePath[33] = {0};
    char fileName[33] = {0};
    g_UpdatePathToName((char *)pRecvFtp->update_file_path, filePath, fileName);
    char username[17] = {0};
    char password[17] = {0};
    strncpy(username, (char *)pRecvFtp->update_username, 16);
    strncpy(password, (char *)pRecvFtp->update_password, 16);
    g_PileUpdateInterface((char *)pRecvFtp->update_ip, u16Port, username, password, filePath, fileName);
	
	return;
}

uint8_t recv_QR_ykc(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;
	YKC_Recv_QR *pRecvQR = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvQR = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvQR;
	
	memset(pRecvQR, 0, sizeof(YKC_Recv_QR));
	memcpy(pRecvQR, r_data, sizeof(YKC_Recv_QR)-50);
	
    storage_PlatQRCodeInfoStr(u8Port + 1, (char*)pRecvQR->QR_data);		//更新哪把枪传哪吧枪的二维码信息

	return TRUE;
}

void recv_QR_ykc_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_QR_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_QR_ACK);
	
	return;
}

uint8_t recv_QR_DD(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[0] - 1;
	YKC_Recv_QR_DD *pRecvQRDD = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvQRDD = &g_ProtocolDCB.pYKCRecvData[u8Port].strRecvQRDD;
	
	memset(pRecvQRDD, 0, sizeof(YKC_Recv_QR_DD));
//	memcpy(pRecvQRDD, r_data, sizeof(YKC_Recv_QR_DD));

	pRecvQRDD->gun = r_data[0];
	memcpy(pRecvQRDD->QR_len, &r_data[1], 2);
	memcpy(pRecvQRDD->QR_data, &r_data[3], twoUint8ToUint16(pRecvQRDD->QR_len));
	
	return TRUE;
}

void recv_QR_DD_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_QR_ACK_DD, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_S_QR_ACK_DD);
	
	return;
}


//===================================================================
uint8_t UpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	if(YKC_R_Heart == cmd)
		Cyc *= 1;
	
	Cyc += eTick_5S;
	
	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

typedef uint8_t (*PRecvTimer)(uint8_t u8Port, uint32_t cmd, uint32_t OutTimer);
typedef uint8_t (*PRecv)(uint8_t *r_data, int len, uint8_t* gun);
typedef void (*PRecvSucc)(uint8_t u8Port);

typedef struct
{
    uint32_t		cmd;
    uint32_t     	timer;
    PRecvTimer 		pRecvTimer;
    PRecv 			pRecv;
    PRecvSucc 		pRecvSucc;
}YKC_Recv_ctrl;

const YKC_Recv_ctrl StrYKCRecvCtrl[]={
    {YKC_R_Identification	,eTick_30S		,UpCtrlRecvTimer		,recv_login_data_ykc			,recv_login_ykc_Succ		},//
    {YKC_R_Heart			,eTick_60S		,UpCtrlRecvTimer		,recv_Heart_ykc					,recv_heart_ykc_Succ		},//
    {YKC_R_Rate_Proving		,eTick_20S		,UpCtrlRecvTimer		,recv_Rate_Proving_ykc			,recv_Rate_Proving_ykc_Succ	},//
    {YKC_R_Rate_Ask			,eTick_30S		,UpCtrlRecvTimer		,recv_Rate_Ask_ykc				,recv_Rate_Ask_ykc_Succ		},//
	
    {YKC_R_RealData			,0xffffffff			,UpCtrlRecvTimer		,recv_RealData_ykc				,recv_RealData_ykc_Succ		},//
    {YKC_R_Auth				,eTick_60S		,UpCtrlRecvTimer		,recv_Auth_Ack_ykc				,recv_Auth_ykc_Succ		},//
	
	{YKC_R_Start_Chg 		,0xffffffff 		,UpCtrlRecvTimer		,recv_Start_Charge_ykc			,recv_Start_Charge_ykc_Succ 	},
	
    {YKC_R_Stop_Chg			,0xffffffff			,UpCtrlRecvTimer		,recv_Stop_Charge_ykc			,recv_Stop_Charge_ykc_Succ		},//
	{YKC_R_Chg_Record		,eTick_30S 		,UpCtrlRecvTimer		,recv_Record_Ack_ykc			,recv_Record_ykc_Succ		},
	
    {YKC_R_Sum_Update		,0xffffffff			,UpCtrlRecvTimer		,recv_Sum_Update_ykc			,recv_Sum_Update_ykc_Succ		},//
	{YKC_R_Set_Para			,0xffffffff 		,UpCtrlRecvTimer		,recv_Set_Para_ykc				,recv_Set_Para_ykc_Succ		},
    {YKC_R_TimeSyn			,0xffffffff			,UpCtrlRecvTimer		,recv_TimeSyn_ykc				,recv_TimeSyn_ykc_Succ		},//
	{YKC_R_Set_Rate			,0xffffffff 		,UpCtrlRecvTimer		,recv_Set_Rate_ykc				,recv_Set_Rate_ykc_Succ		},
    {YKC_R_set_reboot		,0xffffffff			,UpCtrlRecvTimer		,recv_set_reboot_ykc			,recv_set_reboot_ykc_Succ		},//
	{YKC_R_set_update_ftp	,0xffffffff 		,UpCtrlRecvTimer		,recv_update_ftp_ykc			,recv_update_ftp_ykc_Succ		},
	{YKC_R_Ret_QR			,0xffffffff 		,UpCtrlRecvTimer		,recv_QR_ykc					,recv_QR_ykc_Succ		},
	{YKC_R_Set_QR_DD		,0xffffffff 		,UpCtrlRecvTimer		,recv_QR_DD						,recv_QR_DD_Succ		}

};

void YKCUpCtrlRecvDeal(YKC_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const YKC_Recv_ctrl *pYKCRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(StrYKCRecvCtrl); u32i++)
    {
		pYKCRecvCtrl = &StrYKCRecvCtrl[u32i];
		
		if (cmd == pYKCRecvCtrl->cmd)
		{
			if(TRUE == pYKCRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pYKCRecvCtrl->pRecvSucc(port);
				
                if (cmd == CMD_Response_realTime_gun) {
                    SetSendSrm(port, cmd + 1, twoUint8ToUint16(pHead->ser));
                } else {
                    SetSendSrm(port, cmd - 1, twoUint8ToUint16(pHead->ser));    //此处-1是因为命令字不一样原因
                }
				
				SetRecvTick(port, cmd, Get_Systick());
				
				UPRINT("\r\nUpProtocol --> GUN: %d, RecvDealcmd: 0x%x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}


//判断tcp接收到的所有数据是否合法
static int ykc_Tcp_Read_Data_Check(uint8_t *r_data)
{
    if (r_data[0] != 0x68) {
        printf("Check head erro  0x%x\r\n", r_data[0]);
        return -1;
    }
    //检查校验，读取所有数据长度
    uint16_t r_len = r_data[1];
    uint16_t crc_len = r_len + 2;
    
    //对校验位之前的数据进行CRC校验
    uint16_t c_crc = CRC16(&r_data[2], r_len);

    uint16_t r_crc = 0;
	r_crc = r_data[crc_len] << 8 | r_data[crc_len + 1];
	
    // memcpy(&r_crc, &r_data[crc_len], 2);

    if (c_crc !=  r_crc) {
        printf("Check crc erro  0x%x  0x%x\r\n", r_crc, c_crc);
        return -2;
    }
    return 0;
}

void ykcfrom_buffer_data(U8 *recv_buf, U16 *len)
{
    //从buffer里查找合法数据进行校验
    U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, len, TCP_DATA_LEN_MAX);
    
    if (recv_buf[0] == 0x68) {
        //继续寻找len
		read_len = *len;
        if (read_len > TCP_DATA_LEN_MAX) {
			printf("\r\nprotocol--> recv buf full ! ");
            return;
        }
        *len = read_len;
    }
}

void ykcPackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	YKC_HEAD_T *pHead = NULL;

	while(surplusLen) {
		pHead = (YKC_HEAD_T*)(recv_buf + currentIndex);

		int packLen = pHead->len + 4;
		//防止乱数据导致程序死掉
		if (packLen > surplusLen) {
			return;
		}
		surplusLen = surplusLen - packLen;

		// printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);
		
		if (ykc_Tcp_Read_Data_Check(recv_buf + currentIndex) < 0) {
			return;
		}

		hex_dump("tcp_recv_data", recv_buf + currentIndex, packLen);

	    YKCUpCtrlRecvDeal(pHead, pHead->cmd, recv_buf+currentIndex+sizeof(YKC_HEAD_T), packLen);
		
		currentIndex = currentIndex + packLen;
		}
}

void YKCUpRecvDeal(void)
{
    uint8_t from_tcp_data[TCP_DATA_LEN_MAX];
    U16 r_len = 0;
	YKC_HEAD_T *pHead = NULL;

    // PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, from_tcp_data, (U16 *)&r_len, TCP_DATA_LEN_MAX);

    ykcfrom_buffer_data(from_tcp_data, &r_len);
    if (r_len == 0)
        return;
    if (r_len > TCP_DATA_LEN_MAX) {
        printf("\r\nprotocol--> recv buf full ! ");
        return;
    }
	//粘包处理
	ykcPackConnectHandle(from_tcp_data, r_len);

	return;
}

void YKCRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if(YKC_R_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}
	
	if(YKC_R_Auth == cmd)
	{
		SetSendEnable(u8Port, YKC_S_Auth, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, YKC_R_Auth, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
//		pChgGunData->Card_err = eCardErr_NULL;
	}
	
	return;
}

void YKCUpCtrlRecvOutTime(void)
{
	const YKC_Recv_ctrl *pYKCRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM_MAX; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrYKCRecvCtrl); u32i++)
	    {
			pYKCRecvCtrl = &StrYKCRecvCtrl[u32i];
			
			if(RECV_ENABLE_ON != GetRecvEnable(i, pYKCRecvCtrl->cmd))
				continue;
			
			if (TRUE == pYKCRecvCtrl->pRecvTimer(i, pYKCRecvCtrl->cmd, pYKCRecvCtrl->timer))
			{
				YKCRecvOutTimeDeal(i, pYKCRecvCtrl->cmd);
			}
		}
	}
	return;
}
//===========================================================================
//===========================================================================

void YKC_CardAuthStart_Cmd(uint8_t u8Port)
{
	//插枪状态下刷有效卡，进行充电鉴权
    if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_S_Auth)) {
        return;
    }
    SetSendEnable(u8Port, YKC_S_Auth, SEND_ENABLE_ON);

    Send_Immediately(u8Port, YKC_S_Auth);
}

void YKC_DealUpdate_Cmd(uint8_t u8Port)
{
    uint8_t t_Chg_Record = s_GetDealUpCmd();

    SetSendEnable(u8Port, t_Chg_Record, SEND_ENABLE_ON);
    Send_Immediately(u8Port, t_Chg_Record);

}

void YKCUpLogin(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
    if (!Comm_getIpSuces(eSocket_GPRS1)) {
		return;
	}
    
	if(eOnline_Off == Get_PlatConnectSta())
	{
        Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, YKC_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, YKC_S_Identification);
	}
}

uint8_t YKCUpChargeRecordUpDeal(void)
{
    return FALSE;
}

void YKCUpGunStateCheck(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	static uint8_t gun_state[GUN_NUM_MAX] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};
	uint8_t report_flag[GUN_NUM_MAX] = {0};
	
	if (logic_get_gun_charging(u8Port)) {
		pUpGunData->up_gun_state = eUP_Gun_State_Work;
	} else {
		if(TRUE == dev_getErrState(u8Port)) {
			pUpGunData->up_gun_state = eUP_Gun_State_Err;
        } else {
            if (ePlatType_DD == get_ChgParam_plat_type() && (TRUE == GetPile_gun_connect(u8Port))) {
			    pUpGunData->up_gun_state = eUP_Gun_State_Conn;
            } else if (ePlatType_DD == get_ChgParam_plat_type() && (eChargeState_StopFinish == logic_get_gun_state(u8Port))) {
			    pUpGunData->up_gun_state = eUP_Gun_State_Finish;
            }
			pUpGunData->up_gun_state = eUP_Gun_State_Idle;
        }
	}
	
	if(gun_state[u8Port] != pUpGunData->up_gun_state)
	{
		gun_state[u8Port] = pUpGunData->up_gun_state;
		report_flag[u8Port] = TRUE;
	}
	
	if(gun_conn_state[u8Port] != GetPile_gun_connect(u8Port))
	{
		gun_conn_state[u8Port] = GetPile_gun_connect(u8Port);
		report_flag[u8Port] = TRUE;
	}
	
	if(TRUE == report_flag[u8Port])
	{
		Send_Immediately(u8Port, YKC_S_RealData);
	}
	
	return;
}

void YKCUpCtrlTaskDeal(void)
{
	uint8_t i = 0;
	
	YKCUpLogin();
	
	for (i = 0; i < GUN_NUM_MAX; i++)
	{
		YKCUpGunStateCheck(i);
	}
	
	YKCUpChargeRecordUpDeal();
	
	return;
}

void YKCUpProtocolDeal(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	if(NULL == pProtocolDCB->pYKCRecvData)
		return;
	
	YKCUpCtrlTaskDeal();	//任务状态处理
	YKCUpRecvDeal();		//接收处理
	YKCUpSendDeal();		//发送处理
	YKCUpCtrlRecvOutTime();	//超时处理
	
	return;
}


