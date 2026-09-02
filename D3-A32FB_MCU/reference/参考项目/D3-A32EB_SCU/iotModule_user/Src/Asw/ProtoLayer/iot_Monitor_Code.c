#include <stdlib.h>
#include "iot_Monitor_Code.h"
#include "AppMidDataTrans.h"
#include "protocol_data.h"
#include "common.h"
#include "mbsDataUpdate.h"
#include "AppStorage.h"
#include "maths.h"
#include "modbus.h"
#include "mbsMaster.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "tcp_gn.h"
#include "AppDealFlash.h"
#include "cost.h"
#include "iot_Monitor_Ctrl.h"


/*==============================================================*/

#define OM_PROTOCOL_ENCRYPT    0           //是否加密

#define OM_TCP_RECV_MAXSIZE		2048
#define OM_TCP_OM_MAXSIZE		1024

om_up_data_ctrl om_tcp_data_ctrl;
static GN_PLATMOD  *network_data = &sg_platmod;	//用于本文件中


//==================================================

static U16 om_up_srm[GUN_NUM_MAX] = {0}; // 序列号


//字符串右对齐输出，左补0
static void StringAlignRight(char *OutNumber, uint8_t OutLen, char *InNumber)
{
    memset(OutNumber, '0', OutLen);
    size_t len = strlen(InNumber);
    if (len > 0) {
        memmove(OutNumber + (OutLen - len), InNumber, len);
    }
}
static void OM_UP_DevNumber(char *number)
{
    char tNum[FIX_NUMBER_LEN] = {0};
    Get_DevNumberString(tNum);
    
    // memcpy(number, tNum, 32);
    StringAlignRight(number, 32, tNum);
}
static void OM_UP_PlatNumber(char *number)
{
    char tNum[PLAT_NUMBER_LEN] = {0};
    Get_PlatNumberString(tNum);
    
    // memcpy(number, tNum, 32);
    StringAlignRight(number, 32, tNum);
}

/*******************************************************/
//充电设备登录认证
uint16_t om_send_identification_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
	
	//设备编码(资产码）
    OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
  
    //平台桩编号
    OM_UP_PlatNumber((char *)&data[data_len]);
	data_len += 32;
	
    //设备类型
    data[data_len] = OM_DEVICE_TYPE;							
	data_len++;

    //设备型号--1.0.0.10增加
    // monitor_getDevName(&data[data_len], 16);
	// data_len += 16;

	//终端数量
    data[data_len] = GUN_NUM;								
	data_len++;
    
    //主程序版本
    string_split_to_int(&data[data_len], SOFTWARE_VERSION, 4);
	data_len += 4;

    Get_HardVersion_A(&data[data_len]);
	data_len += 4;

    Get_SoftVersion_A(&data[data_len]);
	data_len += 4;

	string_split_to_int(&data[data_len], HARDWARE_VERSION, 4);
	data_len += 4;

	string_split_to_int(&data[data_len], SOFTWARE_VERSION, 4);
	data_len += 4;

	memset(&data[data_len], 0, 4);
	data_len += 4;

	memset(&data[data_len], 0, 4);
	data_len += 4;

	memset(&data[data_len], 0, 16);
	data_len += 16;
	
	
	return data_len;
}

void om_send_identification_Succ(uint8_t u8Port)
{

	//OM_SetRecvEnable(u8Port, OM_CMD_Response_Network_module_info, RECV_ENABLE_ON);

	return;
}



//心跳包
uint16_t om_send_heart_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  						//此命令数据包长度
    U8* data = (uint8_t*)pdata;
//	up_data_ctrl *ptcp_data = &tcp_data_ctrl;
	 
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
	uint8_t num = u8Port + 1;
	BINToBCD(&data[data_len], &num, 1);
	data_len ++;
	
    data[data_len] = dev_getErrState(u8Port);
	data_len++;
	
	

	return data_len;
}

void om_send_heart_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != OM_GetRecvEnable(u8Port, OM_CMD_Response_Heart))
	{
		OM_SetRecvEnable(u8Port, OM_CMD_Response_Heart, RECV_ENABLE_ON);
		OM_SetRecvTick(u8Port, OM_CMD_Response_Heart, Get_Systick());		
	}

	return;
}



//网络模块信息
uint16_t om_send_network_info_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	

	//设备编码(资产码）
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
	//网络链接类型
    // data[data_len] = network_data->pileNetInfo.netType;
	data_len++;

	//运营商
    // data[data_len] = network_data->pileNetInfo.oprtType;
	data_len++;

	//SIM卡卡号
    GetNet_Comm_SimID(&data[data_len], 20);
	data_len += 20;

	//网络模块型号
	// BINToAscii(&data[data_len], ptcp_data->network_info.network_module_model, 20);//未填充数据
	data_len += 20;

	//mac地址
	// BINToAscii(&data[data_len], ptcp_data->network_info.mac_address, 20);//未填充数据
	data_len += 20;

	return data_len;
}

void om_send_network_info_Succ(uint8_t u8Port)
{
	printf("\r\n---om_send_network_info_Succ----\r\n");
	return;
}



//实时充电枪状态
uint16_t om_send_real_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	om_up_data_ctrl *ptcp_data = &om_tcp_data_ctrl;
	// ptcp_data->om_strUpGunData[u8Port].up_gun_state = 0;
	

    //设备编码(资产码）
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
	//枪号或终端号
	uint8_t num = u8Port + 1;
	BINToBCD(&data[data_len], &num, 1);			
	data_len ++;

	//状态
	data[data_len] = ptcp_data->om_strUpGunData[u8Port].up_gun_state;
	data_len++;

	//是否插枪
	data[data_len] = GetPile_gun_connect(u8Port);
	data_len++;
	
	//枪线温度
	data[data_len] = monitor_getGunTem(u8Port);
	data_len++;

	uint32_t TotalTime = monitor_getChgTimer(u8Port)/60;
	if (monitor_getChgTimer(u8Port) % 60 > 30) {
		TotalTime = TotalTime + 1;
	}

	uint32_t TotalPower = monitor_getChgTotalEnergy(u8Port)/10;
	uint32_t TotalMoney = monitor_getChgTotalMoney(u8Port);
	if (logic_get_gun_pwmEnable(u8Port) == 0) {
		TotalTime = 0; TotalPower = 0; TotalMoney = 0;
	}
	
	//累计充电时间
	Uint16ToTwoUint8(&data[data_len], TotalTime);
	data_len += 2;

	//累计充电电量
	uint32ToFourUint8(&data[data_len], TotalPower);
	data_len += 4;
	
	//充电金额
	uint32ToFourUint8(&data[data_len], TotalMoney);
	data_len += 4;

	//硬件故障
	// Uint16ToTwoUint8(&data[data_len], om_get_hard_err_bit(u8Port));
	data_len += 2;

	//模块故障
	data[data_len] = 0;
	data_len += 4;									

	//故障码
	// data[data_len] = ptcp_data->RealTime_Gun.Fault_code;
	data_len ++;									//拓展用
	
    return data_len;
}

void om_send_real_Succ(uint8_t u8Port)
{
	return;
}



//电表底数上报
uint16_t om_send_meter_base_number_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0; 
    U8* data = (uint8_t*)pdata;
	
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
    //枪号或终端号
	data[data_len] = u8Port ;
	data_len++;
	
	uint32_t Active_Power = 0;
	uint32_t Total_Active_Energy = 0;
	//有功功率
	uint32ToFourUint8(&data[data_len], Active_Power);
	data_len += 4;

	//有功总电能
    uint32ToFourUint8(&data[data_len], Total_Active_Energy);
	data_len += 4;
	
	return data_len;
}

void om_send_meter_base_number_Succ(uint8_t u8Port)
{
	printf("\r\n---om_send_meter_base_number_Succ----\r\n\r\n");

	return;
}



//订单信息
uint16_t om_send_order_info_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    //设备编码(资产码）
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;

	//订单数据长度
    // data[data_len] = ptcp_data->Order_Info.order_data_len;
	data_len += 4;

	//订单数据
	// data[data_len] = ptcp_data->Order_Info.order_data;
	data_len += 32;									//n个字节
	
	//停机原因
    // data[data_len] = ptcp_data->Order_Info.shutdown_reason;
	data_len ++;									//拓展用

	return data_len;
}

void om_send_order_info_Succ(uint8_t u8Port)
{
	printf("\r\n---om_send_order_info_Succ----\r\n\r\n");

	return;
}



//设备参数设置
uint16_t om_send_set_para_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	//设备编码(资产码）
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;

	data[data_len] = OM_UP_RESULT_SUCC;
	data_len++;
	
    return data_len;
}

void om_send_set_para_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_set_device_param, SEND_ENABLE_OFF);
	
	printf("\r\n---om_send_set_para_Succ----\r\n\r\n");

	return;
}



//二维码更新
uint16_t om_send_qrcode_update_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
	uint8_t num = u8Port + 1;
	BINToBCD(&data[data_len], &num, 1);		
	data_len ++;
	
	// //二维码字符串
	// BINToAscii(&data[data_len], ptcp_data->Qrcode_Update.qrcode_str, 200);
	// data_len += 200;
	
	data[data_len] = OM_UP_RESULT_SUCC;
	data_len++;

	return data_len;
}

void om_send_qrcode_update_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_Qrcode_update, SEND_ENABLE_OFF);
	
	return;
}



//远程重启
uint16_t om_send_reboot_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
		
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;

	data[data_len] = OM_UP_RESULT_SUCC;
	data_len++;
	
    return data_len;
}

void om_send_reboot_Succ(uint8_t u8Port)
{
	
	OM_SetSendEnable(u8Port, OM_CMD_Request_set_reboot, SEND_ENABLE_OFF);
	
	return;
}



//远程更新
uint16_t om_send_ftp_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	om_up_data_ctrl *ptcp_data = &om_tcp_data_ctrl;
	
	
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;

	data[data_len] = ptcp_data->up_update_ret;
	data_len++;
	
    return data_len;
}

void om_send_ftp_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_set_update_ftp_, SEND_ENABLE_OFF);
	
	return;
}


//远程锁机应答
uint16_t om_send_lock_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
	
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;

	//锁机执行结果
    data[data_len] = OM_LOCKDOWN_SUCC;
	data_len ++;

	//解锁执行结果
    // data[data_len] = ptcp_data->Lock_machine.unlock_result;
	data_len ++;

	//锁机失败原因
    // data[data_len] = ptcp_data->Lock_machine.lockdown_fail_reason;
	data_len ++;

	//解锁失败原因
    // data[data_len] = ptcp_data->Lock_machine.unlock_fail_reason;
	data_len ++;
	
    return data_len;
}

void om_send_lock_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_Lock_machine, SEND_ENABLE_OFF);
	
	OM_SetSendEnable(u8Port, OM_CMD_Request_Lock_state, SEND_ENABLE_ON);

	return;
}


//设备锁机状态
uint16_t om_send_lock_state_data(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	
    //设备编码(资产码）
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
	//充电设备锁机状态
    data[data_len] = OM_LOCK_STATE_SUCC;
	data_len ++;

	//锁机原因
    // data[data_len] = ptcp_data->machine_state.lockdown_reason;
	data_len ++;

	//解锁原因 
    // data[data_len] = ptcp_data->machine_state.unlock_reason;
	data_len ++;

	return data_len;
}

void om_send_lock_state_Succ(uint8_t u8Port)
{

	return;
}


//日志读取
uint16_t om_send_log_ack(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    U8* data = (uint8_t*)pdata;
	om_up_data_ctrl *ptcp_data = &om_tcp_data_ctrl;
	
	OM_UP_DevNumber((char *)&data[data_len]);
	data_len += 32;
	
    // data[data_len] = ptcp_data->log_len;
	data_len += 2;
	
    // data[data_len] = ptcp_data->log_data;
	data_len += 32;									//n个字节	
	
	return data_len;
}

void om_send_log_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_Log_read, SEND_ENABLE_OFF);
	
	return;
}

// tcp上行协议头
static uint8_t om_get_plat_head(uint8_t head_flag)
{
	uint8_t u8head = 0;

	if (1 == head_flag)
	{
		u8head = OM_TCP_HEAD_1;
	}
	else if (2 == head_flag)
	{
		u8head = OM_TCP_HEAD_2;
	}

	return u8head;
}



static uint16_t om_gn_dataEncode(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	OM_GN_HEAD_T *pHeart = (OM_GN_HEAD_T *)p;
	uint16_t all_len = data_len[0] + 10 + 2;
	uint16_t crc_len = data_len[0] + 10;
	uint16_t version = PROTOCOL_VERSION_UP;
	uint16_t crc = 0;

	// number自增
	// 前导域、版本域、序号域、加密标志、命令字、长度域、数据域、校验域
	pHeart->head[0] = om_get_plat_head(1);
	pHeart->head[1] = om_get_plat_head(2);
	

	// 版本号10000，0x2710,  小端0x10,0x27
	memcpy(pHeart->ver, &version, 2);

	if (UP_S_FRAME_ACK == type)
	{
		Uint16ToTwoUint8(pHeart->ser, OM_GetSendSrm(u8Port, cmd));
	}
	else
	{
		Uint16ToTwoUint8(pHeart->ser, om_up_srm[u8Port]);
		om_up_srm[u8Port]++;
	}

	pHeart->EncType = OM_PROTOCOL_ENCRYPT;

	pHeart->cmd = cmd;

	Uint16ToTwoUint8(pHeart->len, all_len);
	
	// 对校验位之前的数据进行CRC校验
	crc = CRC16(pHeart->head, crc_len);

	p[crc_len] = crc >> 8;
	p[crc_len + 1] = crc & 0Xff;

	data_len[0] = all_len;

	return all_len;
}

//--
uint8_t OM_GNUpCtrlSendCyc(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = OM_GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = OM_GetSendImmdFlag(u8Port, cmd);
	om_up_gun_data_ctrl *pUpGunData = &om_tcp_data_ctrl.om_strUpGunData[u8Port];
	
	if(TRUE == u8SendImmdFlag)
	{
		return TRUE;
	}
	
	if(OM_CMD_Request_realTime_gun == cmd)
	{
		if(Gun_State_Charging == pUpGunData->up_gun_state)
			Cyc = eTick_60S;
	}

	if(OM_CMD_Request_Meter_base_number == cmd)
	{
		if(Gun_State_Charging == pUpGunData->up_gun_state)
			Cyc = eTick_60S;
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
}OM_GN_Send_ctrl;

#define  JX_SEND_IMMD 0

const OM_GN_Send_ctrl OM_StrGNSendCtrl[]={
    {OM_CMD_Request_Identification		,UP_S_FRAME_SELF	,eTick_60S,			OM_GNUpCtrlSendCyc	,om_send_identification_data	,om_send_identification_Succ},		
	{OM_CMD_Request_Heart 				,UP_S_FRAME_SELF	,eTick_30S,			OM_GNUpCtrlSendCyc 	,om_send_heart_data				,om_send_heart_Succ},

	{OM_CMD_Request_Network_module_info	,UP_S_FRAME_ACK		,(eTick_60S*10),	OM_GNUpCtrlSendCyc	,om_send_network_info_ack		,om_send_network_info_Succ},		
	{OM_CMD_Request_realTime_gun		,UP_S_FRAME_SELF 	,(eTick_60S*10),	OM_GNUpCtrlSendCyc 	,om_send_real_data				,om_send_real_Succ},
	{OM_CMD_Request_Meter_base_number	,UP_S_FRAME_SELF	,(eTick_60S*10),	OM_GNUpCtrlSendCyc 	,om_send_meter_base_number_data	,om_send_meter_base_number_Succ},

	{OM_CMD_Request_order_info			,UP_S_FRAME_SELF	,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_order_info_data		,om_send_order_info_Succ},
	{OM_CMD_Request_set_device_param	,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_set_para_ack			,om_send_set_para_Succ},
	
	{OM_CMD_Request_Qrcode_update		,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_qrcode_update_ack		,om_send_qrcode_update_Succ},

	{OM_CMD_Request_set_reboot			,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_reboot_ack				,om_send_reboot_Succ},
	{OM_CMD_Request_set_update_ftp_		,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_ftp_ack				,om_send_ftp_Succ},

	{OM_CMD_Request_Lock_machine		,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_lock_ack				,om_send_lock_Succ},
	{OM_CMD_Request_Lock_state			,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_lock_state_data		,om_send_lock_state_Succ},

	{OM_CMD_Request_Log_read			,UP_S_FRAME_ACK		,eTick_15S,			OM_GNUpCtrlSendCyc 	,om_send_log_ack				,om_send_log_Succ},
	
};



static uint16_t OM_GNUpCtrlSend(void *pBuf ,uint32_t u32BufSize)
{
	const OM_GN_Send_ctrl *pGNSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	OM_GN_HEAD_T *pHead = (OM_GN_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + sizeof(OM_GN_HEAD_T);
	uint32_t time;
	//不论后台支不支持都不能连帧发送
	// if (0 != dtu_data.tx_len) return outLen;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(OM_StrGNSendCtrl); u32i++)
		{
			pGNSendCtrl = &OM_StrGNSendCtrl[u32i];
			
			if (SEND_ENABLE_ON != OM_GetSendEnable(i, pGNSendCtrl->cmd))
				continue;
			
			if (TRUE == pGNSendCtrl->pSendCyc(i, pGNSendCtrl->cmd, pGNSendCtrl->cyc))
			{
				if ((outLen = pGNSendCtrl->pSend(i, pData, u32BufSize)) > 0)
	            {	
	            	om_gn_dataEncode(i, (uint8_t*)pHead, pGNSendCtrl->cmd, pGNSendCtrl->FType, &outLen);

					pGNSendCtrl->pSendSucc(i);
					OM_SetSendTick(i, pGNSendCtrl->cmd, Get_Systick());
					OM_SetSendFlag(i, pGNSendCtrl->cmd, SEND_FLAG_YES);
					OM_SetSendImmdFlag(i, pGNSendCtrl->cmd, FALSE);
					
					printf("\r\nOM-->GUN: %d, OM_SendCmd: %x \r\n", i, pGNSendCtrl->cmd);

					return outLen;
				}
			}
		}
	}
	
	return outLen;
}




static void OM_GNUpCtrlSendDeal(void)
{
	uint8_t pbuf[OM_TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS2))
		return;

	outLen = OM_GNUpCtrlSend(pbuf, sizeof(pbuf));
    
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_2, eDataType_TCP, NULL, 0, pbuf, outLen);
}




/*******************************************************/
//登录认证应答解析
uint8_t om_recv_identification_data_parse( U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvIdenf;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvIdenf, r_data, sizeof(OM_GN_Recv_Identification));
	
	return TRUE;
}

void om_recv_identification_Succ(uint8_t u8Port)
{
    ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;
	OM_GN_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvIdenf;
	
	//桩码不匹配
	// uint8_t dataNum[33] = {0};
    // OM_UP_DevNumber((char *)dataNum);
	// if (memcmp(pRecvIdenf->device_number, dataNum, 32) != 0) {
	// 	printf("\r\n---------om_recv_identification_Fail...dev erro---------\r\n");
	// 	return;
	// }
    
    if (1 == pRecvIdenf->charge_logon_result) {
		printf("\r\n---------om_recv_identification_Fail...rslt erro---------\r\n");
        return;
    }

    printf("\r\n---------om_recv_identification_Succ---------\r\n");

    OM_SetSendEnable(u8Port, OM_CMD_Request_Identification, SEND_ENABLE_OFF);
    pTcpDataCtrl->OmPlatSta.eOnlineType = eOnline_Auth;
    
	//双枪各自发心跳
	for(int i = 0; i < GUN_NUM_MAX; i++)
	{
		if(SEND_ENABLE_ON != OM_GetSendEnable(i, OM_CMD_Request_Heart))
		{
            OM_SetSendEnable(i, OM_CMD_Request_Heart, SEND_ENABLE_ON);
            OM_Send_Immediately(i, OM_CMD_Request_Heart);
		}
	}
	
	return;
}


//心跳包
uint8_t om_recv_heart_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	OM_GN_Recv_Heart *pRecvHeart = NULL;
	

	
	// pRecvHeart = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvHeart;
	
	// memcpy(pRecvHeart, r_data, sizeof(OM_GN_Recv_Heart));

	return TRUE;
}

void om_recv_heart_Succ(uint8_t u8Port)
{
//	GN_Recv_Heart *pRecvHeart = &tcp_data_ctrl.strRecvData[u8Port].strRecvHeart;
	uint8_t i = 0;

	// 心跳报文接收到后更新tick,用做超时重连时间
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    g_ProtocolDCB.OmPlatSta.eOnlineType = eOnline_Heart;
	pProtocolDCB->OmPlatSta.no_Comm_tick = Get_Systick();

	// //实时数据所有枪都上报
	for(i = 0; i < GUN_NUM; i++)
	{
		OM_SetRecvTick(i, OM_CMD_Response_Heart, Get_Systick());    // 更新接收时间,因为协议中心跳不单独按照枪号回复,所以要更新接收时间

		if(SEND_ENABLE_ON != OM_GetSendEnable(i, OM_CMD_Request_realTime_gun))
		{
			OM_SetSendEnable(i, OM_CMD_Request_realTime_gun, SEND_ENABLE_ON);
			OM_Send_Immediately(i, OM_CMD_Request_realTime_gun);
            
		    OM_SetSendEnable(u8Port, OM_CMD_Request_Meter_base_number, SEND_ENABLE_ON);
		}
	}

	return;
}


//网络模块信息
uint8_t om_recv_network_info_parse(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Network_Module_Info *pRecvNetwork = NULL;

	if(u8Port >= GUN_NUM) 
		return FALSE;
	
	
	gun[0] = u8Port;

	pRecvNetwork = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvNetModuleInfo;
	
	memcpy(pRecvNetwork, r_data, sizeof(OM_GN_Recv_Network_Module_Info));
	
	return TRUE;
}
void om_recv_network_Succ(uint8_t u8Port)
{
	OM_GN_Recv_Network_Module_Info *pRecvHeart = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvNetModuleInfo;
	U8 i = 0;	

	OM_SetSendEnable(u8Port, OM_CMD_Request_Network_module_info, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_Network_module_info);

	return;
}
/***********************************************************/




//实时监测数据
uint8_t om_recv_realTime_data_parse(U8 *r_data, int len, uint8_t* gun)
{
	OM_GN_Recv_RealTime_Gun *pRecvRealTimeRsq = NULL;
	U8 u8Port = pRecvRealTimeRsq->gun_num - 1;

	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	pRecvRealTimeRsq = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvRealTimeGun;
	
	memcpy(pRecvRealTimeRsq, r_data, sizeof(OM_GN_Recv_RealTime_Gun));
	
	return TRUE;
}

void om_recv_realTime_Succ(uint8_t u8Port)
{
	//实时数据所有枪都上报
	int i = 0 ;
    if(SEND_ENABLE_ON != GetSendEnable(u8Port, OM_CMD_Request_realTime_gun))
    {
        OM_SetSendEnable(u8Port, OM_CMD_Request_realTime_gun, SEND_ENABLE_ON);
        OM_Send_Immediately(u8Port, OM_CMD_Request_realTime_gun);
    }
	return;
}


//订单信息
uint8_t om_recv_order(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Order_Info *pRecvOrderInfo = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvOrderInfo;

	if(u8Port >= GUN_NUM) 
		return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvOrderInfo, r_data, sizeof(OM_GN_Recv_Order_Info));
	
	return TRUE;
}
void om_recv_order_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_order_info, SEND_ENABLE_OFF);
	
	return;
}



//参数设置
uint8_t om_recv_set_param(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Set_Device_Param *pRecvSetParam = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvSetDeviceParam;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvSetParam, r_data, sizeof(OM_GN_Recv_Set_Device_Param));

    Set_PlatIpPort(pRecvSetParam->business_platform_address, pRecvSetParam->business_platform_port);
    Set_PlatType(pRecvSetParam->business_platform_name);
	
	return TRUE;
}

void om_recv_param_Succ(uint8_t u8Port)
{
	OM_GN_Recv_Set_Device_Param *pRecvSetParam = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvSetDeviceParam;

	OM_SetSendEnable(u8Port, OM_CMD_Request_set_device_param, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_set_device_param);
	
	// if(1 == pRecvSetParam->chrg_device_forbid_use)
	// {
	// 	monitor_set_MonitorState(u8Port, om_eMonitorState_Forbid);
	// }
	// else
	// {
	// 	monitor_set_MonitorState(u8Port, om_eMonitorState_Service);
	// }

	return;
}



//二维码信息
uint8_t om_recv_qrcode(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Qrcode_Update *pRecvQrcode = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvQrcodeUpdate;

	if(u8Port >= GUN_NUM) 
		return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvQrcode, r_data, sizeof(OM_GN_Recv_Qrcode_Update));
	
	return TRUE;
}
void om_recv_qrcode_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_Qrcode_update, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_Qrcode_update);
	
	return;
}




//远程重启
uint8_t om_recv_reboot(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Set_Reboot *pRecvReboot = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvSetReboot;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvReboot, r_data, sizeof(OM_GN_Recv_Set_Reboot));
	
	return TRUE;
}

void om_recv_reboot_Succ(uint8_t u8Port)
{
	ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;
	
	pTcpDataCtrl->PlatTask.reboot_flag = E_Reboot_Idle;
	pTcpDataCtrl->PlatTask.reboot_tick = Get_Systick();
	
	OM_SetSendEnable(u8Port, OM_CMD_Request_set_reboot, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_set_reboot);

	return;
}


// 去掉前面的 '0'（即 "30"），并返回剩余部分的 ASCII 字符串
uint8_t trimLeadingZerosAndConvert(char *input, uint8_t len) {
    if (input == NULL) {
        return 0;
    }

    int i = 0;
    while (input[i] == '0') {  // 或者 input[i] == 0x30
        i++;
    }
    if (i > 0) {
        int tLen = len - i;
        printf("len = %d %d\r\n", i, tLen);
        memmove(input, input + i, tLen);
        input[tLen] = '\0';
    }
    return strlen(input);
}

//远程更新
uint8_t om_recv_ftp(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Set_Update_Ftp *pRecvFtp = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvSetUpdateFtp;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvFtp, r_data, sizeof(OM_GN_Recv_Set_Update_Ftp));
    
    //000000000/AC_pile/D3-A32EB/b.bin
    //平台传输是按照前面补0方式，所以需要去掉前面的0
    trimLeadingZerosAndConvert((char *)pRecvFtp->updata_server_address, sizeof(pRecvFtp->updata_server_address));
    trimLeadingZerosAndConvert((char *)pRecvFtp->user_name, sizeof(pRecvFtp->user_name));
    trimLeadingZerosAndConvert((char *)pRecvFtp->user_password, sizeof(pRecvFtp->user_password));
    trimLeadingZerosAndConvert((char *)pRecvFtp->file_path, sizeof(pRecvFtp->file_path));
	
	return TRUE;
}

void om_recv_ftp_Succ(uint8_t u8Port)
{
	om_up_data_ctrl *ptcp_data = &om_tcp_data_ctrl;
	OM_GN_Recv_Set_Update_Ftp *pRecvFtp = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvSetUpdateFtp;
	
	uint16_t rate = 0;
	rate = pRecvFtp->device_power[1] << 8 | pRecvFtp->device_power[0];
	//设备型号以及设备功率异常不升级,应答失败
	if ((pRecvFtp->device_type != 2) || (rate != (7 * GUN_NUM))) {
		// 应答失败且不升级
		ptcp_data->up_update_ret = 2;
		pRecvFtp->update_ctrl = 0;
	} else {
		ptcp_data->up_update_ret = 0;
		g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;				//升级
		g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间

        uint16_t u16Port = pRecvFtp->updata_server_port[1] << 8 | pRecvFtp->updata_server_port[0];
        
        char filePath[33] = {0};
        char fileName[33] = {0};
        g_UpdatePathToName((char *)pRecvFtp->file_path, filePath, fileName);
        char username[17] = {0};
        char password[17] = {0};
        strncpy(username, (char *)pRecvFtp->user_name, 16);
        strncpy(password, (char *)pRecvFtp->user_password, 16);
        g_PileUpdateInterface((char *)pRecvFtp->updata_server_address, u16Port, username, password, filePath, fileName);
	}

	OM_SetSendEnable(u8Port, OM_CMD_Request_set_update_ftp_, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_set_update_ftp_);

//	UPDATA_INITIATE_PLAT

	return;
}


//远程设备锁机
uint8_t om_recv_lock_machine(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Lock_machine *pRecvFtp = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvLockMachine;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvFtp, r_data, sizeof(OM_GN_Recv_Lock_machine));
	
	return TRUE;
}

void om_recv_lock_machine_Succ(uint8_t u8Port)
{
	OM_GN_Recv_Lock_machine *pRecvFtp = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvLockMachine;
	

	OM_SetSendEnable(u8Port, OM_CMD_Request_Lock_machine, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_Lock_machine);

//	UPDATA_INITIATE_PLAT

	return;
}


//本地锁机
uint8_t om_recv_lock_machine_state(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Lock_machine_state *pRecvOrderInfo = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvLockMachineState;

	if(u8Port >= GUN_NUM) 
		return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvOrderInfo, r_data, sizeof(OM_GN_Recv_Lock_machine_state));
	
	return TRUE;
}
void om_recv_lock_machine_state_Succ(uint8_t u8Port)
{
	OM_SetSendEnable(u8Port, OM_CMD_Request_Lock_state, SEND_ENABLE_OFF);
	
	return;
}



//远程日志
uint8_t om_recv_log(U8 *r_data, int len, uint8_t* gun)
{
	U8 u8Port = GUN_A;
	OM_GN_Recv_Log_read *pRecvLog = &g_ProtocolDCB.pOMRecvData[u8Port].om_strRecvLogRead;

	if(u8Port >= GUN_NUM) 
		return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvLog, r_data, sizeof(OM_GN_Recv_Log_read));
	
	return TRUE;
}
void om_recv_log_Succ(uint8_t u8Port)
{
	
	OM_SetSendEnable(u8Port, OM_CMD_Request_Log_read, SEND_ENABLE_ON);
	OM_Send_Immediately(u8Port, OM_CMD_Request_Log_read);
	
	return;
}








uint8_t OM_GNUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = OM_GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	if(OM_CMD_Response_Heart == cmd)
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
}OM_GN_Recv_ctrl;

const OM_GN_Recv_ctrl OM_StrGNRecvCtrl[]={
    {OM_CMD_Response_Identification			,eTick_30S		,OM_GNUpCtrlRecvTimer		,om_recv_identification_data_parse		,om_recv_identification_Succ		},
    {OM_CMD_Response_Heart					,eTick_30S*3	,OM_GNUpCtrlRecvTimer		,om_recv_heart_data_parse				,om_recv_heart_Succ		        	},
    {OM_CMD_Response_Network_module_info	,eTick_20S		,OM_GNUpCtrlRecvTimer		,om_recv_network_info_parse				,om_recv_network_Succ	    		},//需修改
    {OM_CMD_Response_realTime_gun			,eTick_30S		,OM_GNUpCtrlRecvTimer		,om_recv_realTime_data_parse			,om_recv_realTime_Succ				},
	
    {OM_CMD_Response_order_info				,0xffffffff		,OM_GNUpCtrlRecvTimer		,om_recv_order							,om_recv_order_Succ		        	},//需修改
    {OM_CMD_Response_set_device_param		,0xffffffff		,OM_GNUpCtrlRecvTimer		,om_recv_set_param						,om_recv_param_Succ		        	},
	
	{OM_CMD_Response_Qrcode_update 			,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_qrcode							,om_recv_qrcode_Succ 	    		},//需修改

	{OM_CMD_Response_set_reboot				,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_reboot							,om_recv_reboot_Succ		        },
	{OM_CMD_Response_set_update_ftp			,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_ftp							,om_recv_ftp_Succ		            },
	{OM_CMD_Response_Lock_machine			,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_lock_machine					,om_recv_lock_machine_Succ		    },
	{OM_CMD_Response_Lock_state				,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_lock_machine_state				,om_recv_lock_machine_state_Succ	},

    {OM_CMD_Response_Log_read		        ,0xffffffff 	,OM_GNUpCtrlRecvTimer		,om_recv_log							,om_recv_log_Succ		       		},//需修改
};


void OM_GNUpCtrlRecvDeal(OM_GN_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const OM_GN_Recv_ctrl *pGNRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(OM_StrGNRecvCtrl); u32i++)
    {
		pGNRecvCtrl = &OM_StrGNRecvCtrl[u32i];
		if (cmd == pGNRecvCtrl->cmd)
		{	
			if(TRUE == pGNRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pGNRecvCtrl->pRecvSucc(port);
				
				OM_SetSendSrm(port, cmd - 1, twoUint8ToUint16(pHead->ser));    //此处-1是因为命令字不一样原因
				
				OM_SetRecvTick(port, cmd, Get_Systick());
				
				// debug_printf("\r\nUpProtocol --> GUN: %d, RecvDealcmd: %x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}



static int OM_Tcp_Read_Data_Check(uint8_t *r_data)
{
	if ((r_data[0] != om_get_plat_head(1)) || (r_data[1] != om_get_plat_head(2)))
	{
		printf("OM Check head erro  0x%x  0x%x\r\n", r_data[0], r_data[1]);
		return -1;
	}
	// 检查校验，读取所有数据长度
	uint16_t r_len = 0;
	memcpy(&r_len, &r_data[8], 2);
	uint16_t crc_len = r_len - 2;

	// 对校验位之前的数据进行CRC校验
	uint16_t c_crc = CRC16(r_data, crc_len);

	uint16_t r_crc = 0;
	r_crc = r_data[crc_len] << 8 | r_data[crc_len + 1];

	// memcpy(&r_crc, &r_data[crc_len], 2);

	if (c_crc != r_crc)
	{
		printf("Check crc erro  0x%x  0x%x\r\n", r_crc, c_crc);
		return -2;
	}
	return 0;
}



void OM_PackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	OM_GN_HEAD_T *pHead = NULL;

	while (surplusLen)
	{
		pHead = (OM_GN_HEAD_T *)(recv_buf + currentIndex);

		int packLen = twoUint8ToUint16(&pHead->len[0]);
		// 防止乱数据导致程序死掉
		if (packLen > surplusLen)
		{
			return;
		}
		surplusLen = surplusLen - packLen;

		// printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);

		if (OM_Tcp_Read_Data_Check(recv_buf + currentIndex) < 0)
		{
			return;
		}
        printf("\r\nOM-->OM_RecvCmd: 0x%x \r\n", pHead->cmd);

		hex_dump("OM_tcp_recv_data", recv_buf + currentIndex, packLen);

		OM_GNUpCtrlRecvDeal(pHead, pHead->cmd, recv_buf + currentIndex + sizeof(OM_GN_HEAD_T), packLen);

		currentIndex = currentIndex + packLen;
	}
}

void om_from_buffer_data(U8 *recv_buf, int *len)
{
	// 从buffer里查找合法数据进行校验
	// 先找包头，0xfa,0xaf
	U16 read_len = 0;
	// *len = 0;

	// *len = PopPalRxLen(eDataID_1, TCP_RECV_MAXSIZE);
	PalRecvPop(eDataID_2, eDataType_TCP, NULL, NULL, recv_buf, (U16 *)len, TCP_DATA_LEN_MAX);
	// hex_dump("tcp_recv_data:", recv_buf, *len);

	if ((recv_buf[0] == om_get_plat_head(1)) && (recv_buf[1] == om_get_plat_head(2)))
	{
		// 继续寻找len
		read_len = *len;
		// read_len = twoUint8ToUint16(&recv_buf[8]);
		if (read_len > TCP_DATA_LEN_MAX)
		{
			printf("\r\nprotocol--> recv buf full ! ");
			return;
		}
		*len = read_len;
	}
}

void OM_GNUpRecvDeal(void)
{
    U8 from_tcp_data[OM_TCP_DATA_LEN_MAX];
    int r_len = 0;
	OM_GN_HEAD_T *pHead = NULL;
	
    om_from_buffer_data(from_tcp_data, &r_len);
    if (r_len == 0) 
        return;
    if (r_len > OM_TCP_DATA_LEN_MAX) 
        return;
	
	//粘包处理
	OM_PackConnectHandle(from_tcp_data, r_len);

	return;
}


void OM_GNRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
	if(OM_CMD_Response_Heart == cmd)
	{
		printf("in OM_GNRecvOutTimeDeal\r\n");
        
        if (g_ProtocolDCB.OmPlatSta.eOnlineType == eOnline_Off) {
            return;
        }
        g_ProtocolDCB.OmPlatSta.eOnlineType = eOnline_Off;
        
        //socket重连
        UpOfflineDeal(eSocket_GPRS2);

	}

	return;
}

void OM_GNUpCtrlRecvOutTime(void)
{
	const OM_GN_Recv_ctrl *pJXRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(OM_StrGNRecvCtrl); u32i++)
	    {
			pJXRecvCtrl = &OM_StrGNRecvCtrl[u32i];
			
			if(RECV_ENABLE_ON != OM_GetRecvEnable(i, pJXRecvCtrl->cmd))
				continue;
			
			if (TRUE == pJXRecvCtrl->pRecvTimer(i, pJXRecvCtrl->cmd, pJXRecvCtrl->timer))
			{
				OM_GNRecvOutTimeDeal(i, pJXRecvCtrl->cmd);
//				SetRecvTick(i, pJXRecvCtrl->cmd, Get_Systick());
			}
		}
	}
	return;
}

void OM_GNUpLogin(void)
{
    ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;
	
    if (!Comm_getIpSuces(eSocket_GPRS2)) {
		return;
	}

	
	if(eOnline_Off == pTcpDataCtrl->OmPlatSta.eOnlineType)
	{
		pTcpDataCtrl->OmPlatSta.eOnlineType = eOnline_Start;

		OM_SetSendEnable(GUN_A, OM_CMD_Request_Identification, SEND_ENABLE_ON);
		OM_Send_Immediately(GUN_A, OM_CMD_Request_Identification);
	}
}

void OM_GNUpGunStateCheck(uint8_t u8Port)
{
	om_up_gun_data_ctrl *pUpGunData = &om_tcp_data_ctrl.om_strUpGunData[u8Port];
	static uint8_t gun_state[GUN_NUM_MAX] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};
	uint8_t report_flag[GUN_NUM_MAX] = {0};
	
    if (eChargeState_Starting == logic_get_gun_state(u8Port)) {
		pUpGunData->up_gun_state = eUP_OM_Gun_State_Start;
    }
	if (logic_get_gun_charging(u8Port)) {
		pUpGunData->up_gun_state = eUP_OM_Gun_State_Work;
	} else {
		if(TRUE == dev_getErrState(u8Port)) {
            pUpGunData->up_gun_state = eUP_OM_Gun_State_Err;
        } else if (eChargeState_Starting == logic_get_gun_state(u8Port)) {
		    pUpGunData->up_gun_state = eUP_OM_Gun_State_Start;
        } else if (eChargeState_StopFinish == logic_get_gun_state(u8Port)) {
		    pUpGunData->up_gun_state = eUP_OM_Gun_State_Finish;
        } else {
			pUpGunData->up_gun_state = eUP_OM_Gun_State_Idle;
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
		OM_Send_Immediately(u8Port, OM_CMD_Request_realTime_gun);
	}
	
	return;
}


static void OM_UpNoCommTimeout(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	//升级的时候不能检测
	if (GprsGetSocketType() == eSocket_FTP) {
		pProtocolDCB->OmPlatSta.no_Comm_tick = Get_Systick();
		return;
	}

	//3分钟无通信,直接重连
	if(JudgeTimeOutMs(pProtocolDCB->OmPlatSta.no_Comm_tick, eTick_180S))
	{
		printf("GN-OM Heart Timeout\r\n");
		pProtocolDCB->OmPlatSta.no_Comm_tick = Get_Systick();
        UpOfflineDeal(eSocket_GPRS2);
	}
	return;
}


void OM_GNUpCtrlTaskDeal(void)
{
	uint8_t i = 0;

	OM_GNUpLogin();

	for (i = 0; i < GUN_NUM; i++)
	{
		OM_GNUpGunStateCheck(i);
	}
    

	return;
}


static void OM_Malloc(void)
{
    static uint8_t mallocFlag = 0;
    if (mallocFlag) {
        return;
    }
    mallocFlag = 1;

	uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	malloc_len = sizeof(OM_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pOMRecvData = (OM_RECV_Data*)MALLOC(malloc_len);
	
	if(NULL == pProtocolDCB->pOMRecvData)
		UPRINT("\r\n%s om_malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pOMRecvData, 0, malloc_len);
		UPRINT("\r\n%s om_malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}
	
	return;
}


void OM_GNUpProtocolDeal(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;

	if(ePlatType_gwYKC== get_ChgParam_plat_type()) //国网字节码平台不接入运维
	 return;

    OM_Malloc();
	
	if(NULL == pProtocolDCB->pOMRecvData)
		return;
	
	OM_UpNoCommTimeout();		//接收不到平台任何数据时超时处理

	OM_GNUpCtrlTaskDeal();		//任务状态处理
	OM_GNUpRecvDeal();			//接收处理
	OM_GNUpCtrlSendDeal();		//发送处理
	OM_GNUpCtrlRecvOutTime();	//超时处理
	
	return;
}