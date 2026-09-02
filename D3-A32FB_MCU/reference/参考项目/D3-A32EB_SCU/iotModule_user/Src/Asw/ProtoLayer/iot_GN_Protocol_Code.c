#include "iot_GN_Protocol_Code.h"
#include "protocol_data.h"
#include "maths.h"
#include "mbsMaster.h"
#include "common.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "cost.h"

//tcp上行协议头
static uint8_t get_plat_head(uint8_t head_flag)
{
	uint8_t u8head = 0;
	
	if(1 == head_flag)
	{
		u8head = (ePlatType_GN == get_ChgParam_plat_type()) ? TCP_HEAD_1 : TCP_HEAD_GNP_1;
	}
	else if(2 == head_flag)
	{
		u8head = (ePlatType_GN == get_ChgParam_plat_type()) ? TCP_HEAD_2 : TCP_HEAD_GNP_2;
	}
	
	return u8head;
}

/*******************************************************/
//充电设备登录认证
uint16_t send_identification_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;

    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    U32 pp = random();
	
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
    printf("\r\n---充电桩开始认证，桩编号：%s----\r\n\r\n", pst_cfgInfo->pltDeviceNumber);

    //数据区域需要整合
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    //设备识别码
    monitor_getDevName(&data[data_len], 16);
	data_len += 16;
	
    memcpy(&data[data_len], &pp, 4);
	data_len += 4;
	
    // memcpy(&data[data_len], ptcp_data->verification_key, 16);
	data_len += 16;
	
    //设备类型
    data[data_len] = DEVICE_TYPE;
	data_len++;
	
    data[data_len] = GUN_NUM;
	data_len++;
	
	string_split_to_int(&data[data_len], SOFTWARE_VERSION, 4);
	data_len += 4;
	
    data[data_len] = 0;
	data_len++;
	
    memcpy(&data[data_len], pATMDData->SIMID, 20);
	data_len += 20;
	
    //1,移动 2,联通 3,电信
	if(eOperator_CMCC == pATMDData->OperatorType)
	{
		data[data_len] = 0;
	}
	else if(eOperator_CUCC == pATMDData->OperatorType)
	{
		data[data_len] = 2;
	}
	else if(eOperator_CTCC == pATMDData->OperatorType)
	{
		data[data_len] = 1;
	}
	else
	{
		data[data_len] = 0xff;
	}
	data_len++;
	
	return data_len;
}

void send_identification_Succ(uint8_t u8Port)
{
	
	return;
}

uint16_t send_heart_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    data[data_len] = u8Port + 1;
	data_len++;
	
    data[data_len] = dev_getErrState(u8Port);
	data_len++;
	
	return data_len;
}

void send_heart_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, CMD_Response_Heart))
	{
		SetRecvEnable(u8Port, CMD_Response_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, CMD_Response_Heart, Get_Systick());
	}
	
	return;
}


//计费模型验证请求
uint16_t send_billing_verify_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
     
    GN_Recv_Billing_Model pRecvBillingModel;
	
    Read_rate_model((void *)&pRecvBillingModel,sizeof(GN_Recv_Billing_Model));
	
    //设备编码
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    // //计费模型编码
    memcpy(&data[data_len], pRecvBillingModel.billing_model, 2);
	data_len += 2;
	
    return data_len;
}

void send_billing_verify_Succ(uint8_t u8Port)
{
	
	return;
}

//充电设备计费模型请求
uint16_t send_billing_model_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;	
	
	
    return data_len;
}

void send_billing_model_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_real_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;

    if (pChgGunData->ExistChargeDeal == 0) {
	    memset(&data[data_len], 0, GNDATA_TRDNUM_LEN);
    } else {
	    memcpy(&data[data_len], pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);
    }

	data_len += GNDATA_TRDNUM_LEN;
	
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

void send_real_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_auth_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
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
	
	memcpy(&data[data_len], pChgGunData->LogicCard_number, GNDATA_CARD_LEN);
	data_len += GNDATA_CARD_LEN;
	
	memset(&data[data_len], 0, 16);
	data_len += 16;
	
	memset(&data[data_len], 0, 17);
	data_len += 17;

    return data_len;
}

void send_auth_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, CMD_Request_apply_statr_chrg, SEND_ENABLE_OFF);

	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, CMD_Response_apply_statr_chrg))
	{
		SetRecvEnable(u8Port, CMD_Response_apply_statr_chrg, RECV_ENABLE_ON);
		SetRecvTick(u8Port, CMD_Response_apply_statr_chrg, Get_Systick());
	}
	
	return;
}

uint16_t send_start_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	GN_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pRecvData[u8Port].strRecvStartCharge;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	memcpy(&data[data_len], pRecvStartCharge->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
	data[data_len] = pUpGunData->up_start_ret;
	data_len++;
	
	data[data_len] = pUpGunData->up_start_fail_reason;
	data_len++;
	
    return data_len;
}

void send_start_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_statr_chrg, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_stop_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
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

void send_stop_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_stop_chrg, SEND_ENABLE_OFF);
	return;
}


//交易记录上传
uint16_t send_charge_record_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;
	
	memcpy(&data[data_len], pRecord->device_number, 7);
	data_len += 7;

	data[data_len] = u8Port + 1;
	data_len++;

	memcpy(&data[data_len], pRecord->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;

	memcpy(&data[data_len], pRecord->chrg_start_time, 7);
	data_len += 7;

	memcpy(&data[data_len], pRecord->chrg_stop_time, 7);
	data_len += 7;

	memcpy(&data[data_len], pRecord->sharp_rate, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->sharp_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->sharp_loss_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->sharp_money, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->peak_rate, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->peak_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->peak_loss_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->peak_money, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->flat_rate, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->flat_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->flat_loss_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->flat_money, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->valley_rate, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->valley_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->valley_loss_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->valley_money, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->total_start_elec, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->total_stop_elec, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->total_power, 4);
	data_len += 4;
	memcpy(&data[data_len], pRecord->total_loss_power, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->total_money, 4);
	data_len += 4;

	memset(pRecord->vin, 0, 17);
	data_len += 17;

	memcpy(&data[data_len], &pRecord->trade_flag, 1);
	data_len += 1;

	memcpy(&data[data_len], pRecord->trade_time, 7);
	data_len += 7;
	memcpy(&data[data_len], &pRecord->stop_reason, 1);
	data_len += 1;
	memcpy(&data[data_len], pRecord->card_number, GNDATA_CARD_LEN);
	data_len += GNDATA_CARD_LEN;

    uint32_t tTotalMoney = 0;
    memcpy(&tTotalMoney, &pRecord->total_money, 4);
    SetPlat_ChgTotalMoney(u8Port, tTotalMoney);
	
    return data_len;
}

void send_charge_record_Succ(uint8_t u8Port)
{	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;
	
	if (pChgGunData->upDealCnt >= 5) {
		//上报五次未回复，停止上报
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, CMD_Request_deal_log))
		{
			SetSendEnable(u8Port, CMD_Request_deal_log, SEND_ENABLE_OFF);
		}
	} else if (pChgGunData->upDealCnt == 1) {
        pChgGunData->ExistChargeDeal = 0;
    }

	return;
}

uint16_t send_charge_multi_record_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;
	
	size_t data_size = sizeof(charge_record) - 1;
	memcpy(&data[data_len], pRecord->device_number, data_size);
	data_len += data_size;

    uint32_t tTotalMoney = 0;
    memcpy(&tTotalMoney, &pRecord->total_money, 4);
    SetPlat_ChgTotalMoney(u8Port, tTotalMoney);
	
    return data_len;
}

void send_charge_multi_record_Succ(uint8_t u8Port)
{	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;
	
	if (pChgGunData->upDealCnt >= 5) {
		//上报五次未回复，停止上报
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, CMD_Request_multi_deal_log))
		{
			SetSendEnable(u8Port, CMD_Request_multi_deal_log, SEND_ENABLE_OFF);
		}
	} else if (pChgGunData->upDealCnt == 1) {
        pChgGunData->ExistChargeDeal = 0;
    }

	return;
}
uint16_t send_sum_update_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	memcpy(&data[data_len], pProtocolDCB->pRecvData[u8Port].strRecvSumUpdate.Logic_card_number, 8);
	data_len += 8;
	
	data[data_len] = pChgGunData->sum_updata_ret;
	data_len++;
	
    return data_len;
}

void send_sum_update_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_sum_update, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_set_para_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
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

void send_set_para_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_set_device_param, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_set_time_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	U8 timeOrgin[8] = {0};	//年 月 日 时 分 秒 毫秒
	getRunTimeYYMDHMS(timeOrgin);
	U8 time[7] = {0};
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	BINToBCD(time, timeOrgin, 7);
	//当前时间
	memcpy(&data[data_len], time, 7);
	data_len += 7;
	
    return data_len;
}

void send_set_time_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_set_timing, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_set_rate_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
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

void send_set_rate_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_set_billing_model, SEND_ENABLE_OFF);
	
	return;
}

//二维码应答
uint16_t send_set_qrcode_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
    
	data[data_len++] = u8Port + 1;  //wdy 更改二维码回复枪号

	data[data_len++] = UP_RESULT_SUCC;
	
    return data_len;
}

void send_set_qrcode_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Response_S_QR_ACK, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_reboot_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
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

void send_reboot_Succ(uint8_t u8Port)
{
	
	SetSendEnable(u8Port, CMD_Request_set_reboot, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_ftp_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;

	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;

	data[data_len] = ptcp_data->up_update_ret;
	data_len++;
	
    return data_len;
}

void send_ftp_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_set_update_ftp, SEND_ENABLE_OFF);
	
	return;
}


//--
uint8_t GNUpCtrlSendCyc(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);

	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	if(TRUE == u8SendImmdFlag)
	{
		return TRUE;
	}
	
	if(CMD_Request_realTime_gun == cmd)
	{
		if(eUP_Gun_State_Work == pUpGunData->up_gun_state)
			Cyc = eTick_15S;
	}
	
	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

static uint16_t gn_dataEncode(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	GN_HEAD_T *pHeart = (GN_HEAD_T*)p;
	uint16_t all_len = data_len[0] + 10 + 2;
    uint16_t crc_len = data_len[0] + 10;
    uint16_t version = PROTOCOL_VERSION_UP;
	uint16_t crc = 0;
	
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    
    //number自增
    //前导域、版本域、序号域、加密标志、命令字、长度域、数据域、校验域
    pHeart->head[0] = get_plat_head(1);
    pHeart->head[1] = get_plat_head(2);
	
    //版本号10000，0x2710,  小端0x10,0x27
    memcpy(pHeart->ver, &version, 2);

    if ((cmd == CMD_Response_realTime_gun) && (GetSendSrm(u8Port, cmd))) {
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
	
	Uint16ToTwoUint8(pHeart->len, all_len);
	
    //对校验位之前的数据进行CRC校验
    crc = CRC16(pHeart->head, crc_len);

	p[crc_len] = crc >> 8;
	p[crc_len + 1] = crc & 0Xff;
	
	data_len[0] = all_len;
	
    return all_len;
}

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
}GN_Send_ctrl;

#define  JX_SEND_IMMD 0

const GN_Send_ctrl StrGNSendCtrl[]={
    {CMD_Request_Identification		,UP_S_FRAME_SELF	,eTick_60S,	GNUpCtrlSendCyc		,send_identification_data	,send_identification_Succ},		//
	{CMD_Request_Heart 				,UP_S_FRAME_SELF	,eTick_20S,	GNUpCtrlSendCyc 	,send_heart_data			,send_heart_Succ},

	{CMD_Request_billing_verify		,UP_S_FRAME_SELF	,eTick_15S,	GNUpCtrlSendCyc		,send_billing_verify_data	,send_billing_verify_Succ},		//
	{CMD_Request_billing_model		,UP_S_FRAME_SELF	,eTick_30S,	GNUpCtrlSendCyc 	,send_billing_model_data	,send_billing_model_Succ},
	{CMD_Request_realTime_gun		,UP_S_FRAME_SELF ,(eTick_60S*5),	GNUpCtrlSendCyc 	,send_real_data				,send_real_Succ},
	{CMD_Request_apply_statr_chrg	,UP_S_FRAME_SELF	,eTick_15S,	GNUpCtrlSendCyc 	,send_auth_data				,send_auth_Succ},

	{CMD_Request_statr_chrg			,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_start_ack				,send_start_Succ},
	{CMD_Request_stop_chrg			,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_stop_ack				,send_stop_Succ},
	{CMD_Request_deal_log			,UP_S_FRAME_SELF	,eTick_10S,	GNUpCtrlSendCyc		,send_charge_record_data	,send_charge_record_Succ},
	{CMD_Request_multi_deal_log		,UP_S_FRAME_SELF	,eTick_10S,	GNUpCtrlSendCyc		,send_charge_multi_record_data	,send_charge_multi_record_Succ},
	{CMD_Request_sum_update			,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_sum_update_ack		,send_sum_update_Succ},
	{CMD_Request_set_device_param	,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_set_para_ack			,send_set_para_Succ},
	{CMD_Request_set_timing			,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_set_time_ack			,send_set_time_Succ},
	{CMD_Request_set_billing_model	,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_set_rate_ack			,send_set_rate_Succ},
	{CMD_Response_S_QR_ACK	        ,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_set_qrcode_ack		,send_set_qrcode_Succ},
	{CMD_Request_set_reboot			,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_reboot_ack			,send_reboot_Succ},
	{CMD_Request_set_update_ftp		,UP_S_FRAME_ACK		,eTick_15S,	GNUpCtrlSendCyc 	,send_ftp_ack				,send_ftp_Succ},
	
};

static uint16_t GNUpCtrlSend(void *pBuf ,uint32_t u32BufSize)
{
	const GN_Send_ctrl *pGNSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	GN_HEAD_T *pHead = (GN_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + sizeof(GN_HEAD_T);
	
	//不论后台支不支持都不能连帧发送
	// if (0 != dtu_data.tx_len) return outLen;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrGNSendCtrl); u32i++)
		{
			pGNSendCtrl = &StrGNSendCtrl[u32i];
			
			if (SEND_ENABLE_ON != GetSendEnable(i, pGNSendCtrl->cmd))
				continue;
			
			if (TRUE == pGNSendCtrl->pSendCyc(i, pGNSendCtrl->cmd, pGNSendCtrl->cyc))
			{
				if ((outLen = pGNSendCtrl->pSend(i, pData, u32BufSize)) > 0)
	            {	
	            	gn_dataEncode(i, (uint8_t*)pHead, pGNSendCtrl->cmd, pGNSendCtrl->FType, &outLen);
					pGNSendCtrl->pSendSucc(i);
					SetSendTick(i, pGNSendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pGNSendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pGNSendCtrl->cmd, FALSE);
					
					printf("\r\nUpProtocol --> GUN: %d, SendDealcmd: %x \r\n", i, pGNSendCtrl->cmd);
					return outLen;
				}
			}
		}
	}
	
	return outLen;
}

static void GNUpCtrlSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;

	outLen = GNUpCtrlSend(pbuf, sizeof(pbuf));
    
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);

    return;
}

/*******************************************************/
//登录认证应答解析
uint8_t recv_identification_data_parse( U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pRecvData[u8Port].strRecvIdenf;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvIdenf, r_data, sizeof(GN_Recv_Identification));
	
	return TRUE;
}

void recv_identification_Succ(uint8_t u8Port)
{
	GN_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pRecvData[u8Port].strRecvIdenf;

	//桩码不匹配
	uint8_t dataNum[7] = {0};
	monitor_getDevNumber(dataNum, 7);
	if (memcmp(pRecvIdenf->device_number, dataNum, 7) != 0) {
		return;
	}

	if(0 == pRecvIdenf->charge_logon_result)
	{
		SetSendEnable(u8Port, CMD_Request_Identification, SEND_ENABLE_OFF);	

		SetSendEnable(u8Port, CMD_Request_billing_model, SEND_ENABLE_ON);
		Send_Immediately(u8Port, CMD_Request_billing_model);

        Set_PlatConnectSta(eOnline_Auth);
	}
	
	return;
}

uint8_t recv_heart_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_Heart *pRecvHeart = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvHeart = &g_ProtocolDCB.pRecvData[u8Port].strRecvHeart;
	
	memcpy(pRecvHeart, r_data, sizeof(GN_Recv_Heart));

	return TRUE;
}

void recv_heart_Succ(uint8_t u8Port)
{
	uint8_t i = 0;
	
    Set_PlatConnectSta(eOnline_Heart);

	// 心跳报文接收到后更新tick,用做超时重连时间
    PlatHeartTickRefresh();

	//实时数据所有枪都上报
	for(i = 0; i < GUN_NUM; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, CMD_Request_realTime_gun))
		{
			SetSendEnable(i, CMD_Request_realTime_gun, SEND_ENABLE_ON);
			Send_Immediately(i, CMD_Request_realTime_gun);
		}
	}
	
	return;
}

uint8_t recv_billing_verify_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Billing_Verify *pRecvBillingVer = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingVer;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvBillingVer, r_data, sizeof(GN_Recv_Billing_Verify));

	return TRUE;
}

void recv_billing_verify_Succ(uint8_t u8Port)
{
	
	return;
}

uint8_t recv_billing_model_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	U8 offset = 0;
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	pRecvBillingModel->rateModelType = RATE_MODEL_4_TYPE;
	
    memcpy(pRecvBillingModel->device_number, r_data + offset, 7);
    offset += 7;
    
    memcpy(pRecvBillingModel->billing_model, r_data + offset, 2);
    offset += 2;
    
    memcpy(pRecvBillingModel->sharp_ele_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->sharp_ser_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->peak_ele_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->peak_ser_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->flat_ele_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->flat_ser_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->valley_ele_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->valley_ser_fee, r_data + offset, 4);
    offset += 4;
    
    pRecvBillingModel->measure_wastage_rates = r_data[offset];
    offset += 1;
    
    memcpy(pRecvBillingModel->segmentation_rate, r_data + offset, 48);
    offset += 48;

    return TRUE;
}

void recv_billing_model_Succ(uint8_t u8Port)
{
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;
	U8 i = 0;

	SetSendEnable(u8Port, CMD_Request_billing_model, SEND_ENABLE_OFF);

	//双枪各自发心跳
	for(i = 0; i < GUN_NUM; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, CMD_Request_Heart))
		{
			SetSendEnable(i, CMD_Request_Heart, SEND_ENABLE_ON);
			Send_Immediately(i, CMD_Request_Heart);
		}
	}
	GNUpChargeRecordUpDealOffline();	//离线记录上报
	
	Save_rate_model(pRecvBillingModel, sizeof(GN_Recv_Billing_Model));
	return;
}

uint8_t recv_multi_billing_model_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	size_t data_part_size = sizeof(GN_Recv_Billing_Model) - offsetof(GN_Recv_Billing_Model, device_number);

	pRecvBillingModel->rateModelType = RATE_MODEL_9_TYPE;
	
	memcpy(&pRecvBillingModel->device_number, r_data, data_part_size);

	return TRUE;
}

void recv_multi_billing_model_Succ(uint8_t u8Port)
{
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;
	U8 i = 0;

	SetSendEnable(u8Port, CMD_Request_billing_model, SEND_ENABLE_OFF);

	//双枪各自发心跳
	for(i = 0; i < GUN_NUM; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, CMD_Request_Heart))
		{
			SetSendEnable(i, CMD_Request_Heart, SEND_ENABLE_ON);
			Send_Immediately(i, CMD_Request_Heart);
		}
	}
	GNUpChargeRecordUpDealOffline();	//离线记录上报
	
	Save_rate_model(pRecvBillingModel, sizeof(GN_Recv_Billing_Model));
	return;
}
uint8_t recv_realTime_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_RealTime_Rsq *pRecvRealTimeRsq = NULL;

	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	pRecvRealTimeRsq = &g_ProtocolDCB.pRecvData[u8Port].strRecvRealTimeRsq;
	
	memcpy(pRecvRealTimeRsq, r_data, sizeof(GN_Recv_RealTime_Rsq));
	
	return TRUE;
}

void recv_realTime_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, CMD_Request_realTime_gun, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_realTime_gun);
	
	return;
}

uint8_t recv_Auth_Ack(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_Auth_Ack *pRecvAuthAck = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	pRecvAuthAck = &g_ProtocolDCB.pRecvData[u8Port].strRecvAuthAck;
	
	memcpy(pRecvAuthAck, r_data, sizeof(GN_Recv_Auth_Ack));
	
	return TRUE;
}

void recv_Auth_Succ(uint8_t u8Port)
{
	GN_Recv_Auth_Ack *pRecvAuthAck = &g_ProtocolDCB.pRecvData[u8Port].strRecvAuthAck;
	uint8_t up_fail_reason = 0;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	U32 sum_balance = 0;
	
	//鉴权状态退出
	monitor_set_MonitorState(u8Port, eMonitorState_Service);
	
	if((1 == pRecvAuthAck->Auth_success)
		&& (0 == memcmp(pRecvAuthAck->Logic_card_number, pChgGunData->LogicCard_number, 8)))
	{
		sum_balance = fourUint8ToUint32(pRecvAuthAck->account_balance);
		monitor_charge_start(u8Port, &up_fail_reason, eUP_Start_Style_CardOnline, \
			pRecvAuthAck->Logic_card_number, \
			pRecvAuthAck->transaction_log_num, \
			&sum_balance);
		
		fgv_CtrlStartCharge(u8Port);
	} else {
        SetPlat_CardChargeFaild(u8Port, 1);
    }
	
	SetSendEnable(u8Port, CMD_Request_apply_statr_chrg, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, CMD_Response_apply_statr_chrg, RECV_ENABLE_OFF);
	
	return;
}

uint8_t recv_Start_Charge(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_Start_Charge *pRecvStartCharge = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	pRecvStartCharge = &g_ProtocolDCB.pRecvData[u8Port].strRecvStartCharge;
	
	memcpy(pRecvStartCharge, r_data, sizeof(GN_Recv_Start_Charge));
	
	return TRUE;
}

void recv_Start_Charge_Succ(uint8_t u8Port)
{
	GN_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pRecvData[u8Port].strRecvStartCharge;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
//	CHG_DATA_T *pchgData = &g_chgData[u8Port];
	U32 sum_balance = 0;
	
	sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);
	
	if(TRUE == monitor_charge_start(u8Port, \
		&pUpGunData->up_start_fail_reason, \
		eUP_Start_Style_App, \
		pRecvStartCharge->Logic_card_number, \
		pRecvStartCharge->transaction_log_num, \
		&sum_balance))
	{
		pUpGunData->up_start_ret = UP_RESULT_SUCC;
		pUpGunData->up_start_fail_reason = eUP_Start_Fail_NULL;
		fgv_CtrlStartCharge(u8Port);
	}
	else
	{
		pUpGunData->up_start_ret = UP_RESULT_FAIL;
		fgv_CtrlStopCharge(u8Port);
	}
	
	SetSendEnable(u8Port, CMD_Request_statr_chrg, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_statr_chrg);
	
	return;
}

uint8_t recv_Stop_Charge(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_Stop_Charge *pRecvStopCharge = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStopCharge = &g_ProtocolDCB.pRecvData[u8Port].strRecvStopCharge;
	
	memcpy(pRecvStopCharge, r_data, sizeof(GN_Recv_Stop_Charge));
	
	return TRUE;
}

void recv_Stop_Charge_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	stopPileCharge(u8Port, Pile_Stop_Reason_APP);
	
	pUpGunData->up_stop_ret = UP_RESULT_SUCC;
	pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL;
	
	SetSendEnable(u8Port, CMD_Request_stop_chrg, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_stop_chrg);

	return;
}

uint8_t recv_Record_Ack(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Record_Ack *pRecvRecordAck = NULL;
	
	gun[0] = u8Port;
	pRecvRecordAck = &g_ProtocolDCB.pRecvData[u8Port].strRecvRecordAck;
	
	memcpy(pRecvRecordAck, r_data, sizeof(GN_Recv_Record_Ack));
	
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

void recv_Record_Succ(uint8_t u8Port)
{
//	if(0 == pRecvRecordAck->result)
	{
		// monitor_chgrcd_report_succ();
		SetSendEnable(u8Port, CMD_Request_deal_log, SEND_ENABLE_OFF);
		SetSendEnable(u8Port, CMD_Request_multi_deal_log, SEND_ENABLE_OFF);
	}
	
	return;
}


uint8_t recv_Sum_Update(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;
	GN_Recv_Sum_Update *PRecvSumUpdate = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	PRecvSumUpdate = &g_ProtocolDCB.pRecvData[u8Port].strRecvSumUpdate;
	
	memcpy(PRecvSumUpdate, r_data, sizeof(GN_Recv_Sum_Update));
	
	return TRUE;
}

void recv_Sum_Update_Succ(uint8_t u8Port)
{
	GN_Recv_Sum_Update *PRecvSumUpdate = &g_ProtocolDCB.pRecvData[u8Port].strRecvSumUpdate;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint8_t card_number[GNDATA_CARD_LEN] = {0};
	
	SetSendEnable(u8Port, CMD_Request_sum_update, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_sum_update);
	
	monitor_getChgCardNum(u8Port, card_number, GNDATA_CARD_LEN);
	
	if(0 == memcmp(card_number, PRecvSumUpdate->Logic_card_number, GNDATA_CARD_LEN))
	{
		pChgGunData->sum_updata_ret = 0;
		pChgGunData->sum_balance = fourUint8ToUint32(PRecvSumUpdate->account_balance);
	}
	else
	{
		pChgGunData->sum_updata_ret = 2;
	}
	
	return;
}

uint8_t recv_set_param(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Set_Param *pRecvSetParam = &g_ProtocolDCB.pRecvData[u8Port].strRecvSetParam;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvSetParam, r_data, sizeof(GN_Recv_Set_Param));
	
	return TRUE;
}

void recv_param_Succ(uint8_t u8Port)
{
	GN_Recv_Set_Param *pRecvSetParam = &g_ProtocolDCB.pRecvData[u8Port].strRecvSetParam;
	
	SetSendEnable(u8Port, CMD_Request_set_device_param, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_device_param);

	if(1 == pRecvSetParam->chrg_device_forbid_use)
	{
		monitor_set_MonitorState(u8Port, eMonitorState_Forbid);
	}
	else
	{
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
	}
	
	
	return;
}

uint8_t recv_set_time(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Set_Time *pRecvSetTime = &g_ProtocolDCB.pRecvData[u8Port].strRecvSetTime;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvSetTime->device_number, r_data, DEV_NUM_LEN);
    BCDToBin(pRecvSetTime->device_current_time, &r_data[7], 7);
	
	return TRUE;
}

void recv_set_time_Succ(uint8_t u8Port)
{
	GN_Recv_Set_Time *pRecvSetTime = &g_ProtocolDCB.pRecvData[u8Port].strRecvSetTime;
 //   U8 device_current_time[7];         //设置当前时间
    
	SetSendEnable(u8Port, CMD_Request_set_timing, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_timing);
	
    setCurrentRunTime(&pRecvSetTime->device_current_time[0]);
	
	return;
}
uint8_t recv_rate_model(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	U8 offset = 0;
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvBillingModel->rateModelType = RATE_MODEL_4_TYPE;
	
    memcpy(pRecvBillingModel->device_number, r_data + offset, 7);
    offset += 7;
    
    memcpy(pRecvBillingModel->billing_model, r_data + offset, 2);
    offset += 2;
    
    memcpy(pRecvBillingModel->sharp_ele_fee, r_data + offset, 4);
    offset += 4;
    memcpy(pRecvBillingModel->sharp_ser_fee, r_data + offset, 4);
    offset += 4;

    memcpy(pRecvBillingModel->peak_ele_fee, r_data + offset, 4);
    offset += 4;
    memcpy(pRecvBillingModel->peak_ser_fee, r_data + offset, 4);
    offset += 4;

    memcpy(pRecvBillingModel->flat_ele_fee, r_data + offset, 4);
    offset += 4;
    memcpy(pRecvBillingModel->flat_ser_fee, r_data + offset, 4);
    offset += 4;
    
    memcpy(pRecvBillingModel->valley_ele_fee, r_data + offset, 4);
    offset += 4;
    memcpy(pRecvBillingModel->valley_ser_fee, r_data + offset, 4);
    offset += 4;
    
    pRecvBillingModel->measure_wastage_rates = r_data[offset];
    offset += 1;
    memcpy(pRecvBillingModel->segmentation_rate, r_data + offset, 48);
    offset += 48;

	return TRUE;
}

void recv_set_rate_model_Succ(uint8_t u8Port)
{
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;
	
	SetSendEnable(u8Port, CMD_Request_set_billing_model, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_billing_model);
	
	Save_rate_model(pRecvBillingModel, sizeof(GN_Recv_Billing_Model));

	return;
}

uint8_t recv_multi_rate_model(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	size_t data_part_size = sizeof(GN_Recv_Billing_Model) - 1;

	pRecvBillingModel->rateModelType = RATE_MODEL_9_TYPE;
	
    memcpy(&pRecvBillingModel->device_number[0], r_data, data_part_size);
	
	return TRUE;
}

void recv_set_multi_rate_model_Succ(uint8_t u8Port)
{
	GN_Recv_Billing_Model *pRecvBillingModel = &g_ProtocolDCB.pRecvData[u8Port].strRecvBillingModel;

	SetSendEnable(u8Port, CMD_Request_set_billing_model, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_billing_model);
	
	Save_rate_model(pRecvBillingModel, sizeof(GN_Recv_Billing_Model));

	return;
}

//二维码设置
uint8_t recv_qrcodeData(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = r_data[7] - 1;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
    
    U8 qrPort = u8Port + 1;

    storage_PlatQRCodeInfoStr(qrPort, (char *)&r_data[8]);
	
	return TRUE;
}

void recv_qrcodeData_Succ(uint8_t u8Port)
{
	
	SetSendEnable(u8Port, CMD_Response_S_QR_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Response_S_QR_ACK);
	
	return;
}

uint8_t recv_reboot(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pRecvData[u8Port].strRecvReboot;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvReboot, r_data, sizeof(GN_Recv_Reboot));
	
	return TRUE;
}

void recv_reboot_Succ(uint8_t u8Port)
{
	ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;
	
	pTcpDataCtrl->PlatTask.reboot_flag = E_Reboot_Idle;
	pTcpDataCtrl->PlatTask.reboot_tick = Get_Systick();
	
	SetSendEnable(u8Port, CMD_Request_set_reboot, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_reboot);

	return;
}

uint8_t recv_ftp(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	GN_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pRecvData[u8Port].strRecvFtp;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvFtp, r_data, sizeof(GN_Recv_Update_ftp));
	
	return TRUE;
}

void recv_ftp_Succ(uint8_t u8Port)
{
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
	GN_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pRecvData[u8Port].strRecvFtp;
	
	uint16_t rate = 0;
	rate = pRecvFtp->device_rate[1] << 8 | pRecvFtp->device_rate[0];
	//设备型号以及设备功率异常不升级,应答失败
	if ((pRecvFtp->device_type != 2) || (rate != (7 * GUN_NUM))) {
		// 应答失败且不升级
		ptcp_data->up_update_ret = 2;
		pRecvFtp->update_ctrl = 0;
	} else {
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
	}

	SetSendEnable(u8Port, CMD_Request_set_update_ftp, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_set_update_ftp);

//	UPDATA_INITIATE_PLAT

	return;
}


//=====
uint8_t GNUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	if(CMD_Response_Heart == cmd)
	{
		Cyc *= 1;
	}
	
	Cyc += eTick_5S;
	
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
    PRecvTimer 		pRecvTimer;
    PRecv 			pRecv;
    PRecvSucc 		pRecvSucc;
}GN_Recv_ctrl;

const GN_Recv_ctrl StrGNRecvCtrl[]={
    {CMD_Response_Identification		,eTick_30S		,GNUpCtrlRecvTimer		,recv_identification_data_parse		,recv_identification_Succ		},//
    {CMD_Response_Heart					,eTick_30S		,GNUpCtrlRecvTimer		,recv_heart_data_parse				,recv_heart_Succ		},//
    {CMD_Response_billing_verify		,eTick_20S		,GNUpCtrlRecvTimer		,recv_billing_verify_data_parse		,recv_billing_verify_Succ	},//
    {CMD_Response_billing_model			,eTick_30S		,GNUpCtrlRecvTimer		,recv_billing_model_data_parse		,recv_billing_model_Succ		},//
	{CMD_Response_multi_billing_model	,eTick_30S		,GNUpCtrlRecvTimer		,recv_multi_billing_model_data_parse,recv_multi_billing_model_Succ		},//
    {CMD_Response_realTime_gun			,0xffffffff			,GNUpCtrlRecvTimer		,recv_realTime_data_parse			,recv_realTime_Succ		},//
    {CMD_Response_apply_statr_chrg		,eTick_30S		,GNUpCtrlRecvTimer		,recv_Auth_Ack						,recv_Auth_Succ		},//
	
	{CMD_Response_statr_chrg 			,0xffffffff 		,GNUpCtrlRecvTimer		,recv_Start_Charge					,recv_Start_Charge_Succ 	},

    {CMD_Response_stop_chrg				,0xffffffff			,GNUpCtrlRecvTimer		,recv_Stop_Charge					,recv_Stop_Charge_Succ		},//
	{CMD_Response_deal_log				,eTick_30S 		,GNUpCtrlRecvTimer		,recv_Record_Ack					,recv_Record_Succ		},
	
	{CMD_Response_sum_update			,0xffffffff 		,GNUpCtrlRecvTimer		,recv_Sum_Update					,recv_Sum_Update_Succ		},

	{CMD_Response_set_device_param		,0xffffffff 		,GNUpCtrlRecvTimer		,recv_set_param						,recv_param_Succ		},
	{CMD_Response_set_timing			,0xffffffff 		,GNUpCtrlRecvTimer		,recv_set_time						,recv_set_time_Succ		},
	{CMD_Response_set_billing_model		,0xffffffff 		,GNUpCtrlRecvTimer		,recv_rate_model					,recv_set_rate_model_Succ		},
	{CMD_Response_set_multi_billing		,0xffffffff 		,GNUpCtrlRecvTimer		,recv_multi_rate_model				,recv_set_multi_rate_model_Succ	},
	{CMD_Request_R_Ret_QR		        ,0xffffffff 		,GNUpCtrlRecvTimer		,recv_qrcodeData					,recv_qrcodeData_Succ		},
	{CMD_Response_set_reboot			,0xffffffff 		,GNUpCtrlRecvTimer		,recv_reboot						,recv_reboot_Succ		},
	{CMD_Response_set_update_ftp		,0xffffffff 		,GNUpCtrlRecvTimer		,recv_ftp							,recv_ftp_Succ		},

};

void GNUpCtrlRecvDeal(GN_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const GN_Recv_ctrl *pGNRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(StrGNRecvCtrl); u32i++)
    {
		pGNRecvCtrl = &StrGNRecvCtrl[u32i];
		
		if (cmd == pGNRecvCtrl->cmd)
		{
			if(TRUE == pGNRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pGNRecvCtrl->pRecvSucc(port);
				
                if (cmd == CMD_Response_realTime_gun) {
                    SetSendSrm(port, cmd + 1, twoUint8ToUint16(pHead->ser));
                } 
				else if(cmd == CMD_Response_set_multi_billing)
				{
					SetSendSrm(port, cmd + 3, twoUint8ToUint16(pHead->ser));
				}
				else if(cmd == CMD_Response_multi_billing_model || cmd == CMD_Response_deal_log)
				{
					SetSendSrm(port, cmd - 2, twoUint8ToUint16(pHead->ser));
				}
				else {
                    SetSendSrm(port, cmd - 1, twoUint8ToUint16(pHead->ser));    //此处-1是因为命令字不一样原因
                }
				
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
    if ((r_data[0] != get_plat_head(1)) || (r_data[1] != get_plat_head(2))) {
        printf("Check head erro  0x%x  0x%x\r\n", r_data[0], r_data[1]);
        return -1;
    }
    //检查校验，读取所有数据长度
    uint16_t r_len = 0;
    memcpy(&r_len, &r_data[8], 2);
    uint16_t crc_len = r_len - 2;
    
    //对校验位之前的数据进行CRC校验
    uint16_t c_crc = CRC16(r_data, crc_len);

    uint16_t r_crc = 0;
	r_crc = r_data[crc_len] << 8 | r_data[crc_len + 1];
	
    // memcpy(&r_crc, &r_data[crc_len], 2);

    if (c_crc !=  r_crc) {
        printf("Check crc erro  0x%x  0x%x\r\n", r_crc, c_crc);
        return -2;
    }
    return 0;
}

void from_buffer_data(U8 *recv_buf, int *len)
{
    //从buffer里查找合法数据进行校验
    U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, (U16 *)len, TCP_DATA_LEN_MAX);
	// hex_dump("tcp_recv_data:", recv_buf, *len);

    if ((recv_buf[0] == get_plat_head(1)) && (recv_buf[1] == get_plat_head(2))) {
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


void PackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	GN_HEAD_T *pHead = NULL;

	while(surplusLen) {
		pHead = (GN_HEAD_T*)(recv_buf + currentIndex);

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
		printf("\r\nUpProtocol --> RecvDealcmd: %x \r\n", pHead->cmd);

		hex_dump("tcp_recv_data:", recv_buf + currentIndex, packLen);

		GNUpCtrlRecvDeal(pHead, pHead->cmd, recv_buf+currentIndex+sizeof(GN_HEAD_T), packLen);
		
		currentIndex = currentIndex + packLen;
		}
}
void GNUpRecvDeal(void)
{
    U8 from_tcp_data[TCP_DATA_LEN_MAX];
    int r_len = 0;
	GN_HEAD_T *pHead = NULL;
	
    from_buffer_data(from_tcp_data, &r_len);
    if (r_len == 0) 
        return;
    if (r_len > TCP_DATA_LEN_MAX) 
        return;
	
	//粘包处理
	PackConnectHandle(from_tcp_data, r_len);

	return;
}


void GNRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if(CMD_Response_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}
	
	if(CMD_Response_apply_statr_chrg == cmd)
	{
		SetSendEnable(u8Port, CMD_Request_apply_statr_chrg, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, CMD_Response_apply_statr_chrg, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
	}
	
	return;
}

void GNUpCtrlRecvOutTime(void)
{
	const GN_Recv_ctrl *pJXRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrGNRecvCtrl); u32i++)
	    {
			pJXRecvCtrl = &StrGNRecvCtrl[u32i];
			
			if(RECV_ENABLE_ON != GetRecvEnable(i, pJXRecvCtrl->cmd))
				continue;
			
			if (TRUE == pJXRecvCtrl->pRecvTimer(i, pJXRecvCtrl->cmd, pJXRecvCtrl->timer))
			{
				GNRecvOutTimeDeal(i, pJXRecvCtrl->cmd);
//				SetRecvTick(i, pJXRecvCtrl->cmd, Get_Systick());
			}
		}
	}
	return;
}

//================================================================================================
//================================================================================================

void GN_CardAuthStart_Cmd(uint8_t u8Port)
{
	//插枪状态下刷有效卡，进行充电鉴权
	if(SEND_ENABLE_ON == GetSendEnable(u8Port, CMD_Request_apply_statr_chrg)) {
		return;
	}
	SetSendEnable(u8Port, CMD_Request_apply_statr_chrg, SEND_ENABLE_ON);

	Send_Immediately(u8Port, CMD_Request_apply_statr_chrg);
}

void GN_DealUpdate_Cmd(uint8_t u8Port)
{
    charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;
	if(pRecord->order_model_type == RATE_MODEL_9_TYPE)
	{
		SetSendEnable(u8Port, CMD_Request_multi_deal_log, SEND_ENABLE_ON);
    	Send_Immediately(u8Port, CMD_Request_multi_deal_log);
	}
	else
	{
		SetSendEnable(u8Port, CMD_Request_deal_log, SEND_ENABLE_ON);
    	Send_Immediately(u8Port, CMD_Request_deal_log);
	}
}

void GNUpLogin(void)
{
    ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;

    if (!Comm_getIpSuces(eSocket_GPRS1)) {
		return;
	}

	if(eOnline_Off == Get_PlatConnectSta())
	{
        Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, CMD_Request_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, CMD_Request_Identification);
	}
}


void GNUpGunStateCheck(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	static uint8_t gun_state[GUN_NUM_MAX] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};
	uint8_t report_flag[GUN_NUM_MAX] = {0};
	
	if (logic_get_gun_charging(u8Port)) {
		pUpGunData->up_gun_state = eUP_Gun_State_Work;
	} else {
		if(TRUE == dev_getErrState(u8Port))
			pUpGunData->up_gun_state = eUP_Gun_State_Err;
		else
			pUpGunData->up_gun_state = eUP_Gun_State_Idle;
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
		Send_Immediately(u8Port, CMD_Request_realTime_gun);
	}
	
	return;
}



void GNUpCtrlTaskDeal(void)
{
	uint8_t i = 0;
	
	GNUpLogin();
	
	for (i = 0; i < GUN_NUM; i++)
	{
		GNUpGunStateCheck(i);
	}
	
	return;
}
void GNUpProtocolDeal(void)
{
	ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;

	if(NULL == pTcpDataCtrl->pRecvData)
		return;
    
	GNUpCtrlTaskDeal();		//任务状态处理
	GNUpRecvDeal();			//接收处理
	GNUpCtrlSendDeal();		//发送处理
	GNUpCtrlRecvOutTime();	//超时处理
	
	return;
}