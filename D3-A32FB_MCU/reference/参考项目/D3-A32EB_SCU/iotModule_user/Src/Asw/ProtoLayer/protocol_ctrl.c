#include "protocol_ctrl.h"
#include "protocol_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "maths.h"
#include "mbsMaster.h"
#include "cost.h"
#include "AppMidDataTrans.h"
#include "AppCommon.h"
#include "iot_Monitor_Code.h"
#include "iot_YKC_Protocol_CodeV2_1.h"

#define MALLOC(size)			(malloc(size))
#define FREE(pointer)			do{free(pointer); pointer = NULL;}while(0);

static RECV_CTRL* PRecvCtrl(uint8_t u8Port, uint32_t cmd);

static SEND_CTRL* PSendCtrl(uint8_t u8Port, uint32_t cmd);

//=======================================================================

void OperatePlat_Init(void)
{	
	memset(&g_ProtocolDCB, 0, sizeof(ProtocolDCB));
	
	dev_setErrExsit_all(eErr_PlatformOffline, __LINE__);
	
	UPRINT("\r\nplat name: %s ! \r\n", NormProtocolName());
	
	//内存分配
	NormProtocolMalloc();
	
	return;
}
//上行协议

static SEND_CTRL* GetSendCtrl_GN(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
		case CMD_Request_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case CMD_Request_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case CMD_Request_billing_verify:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case CMD_Request_billing_model:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case CMD_Request_realTime_gun:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case CMD_Request_apply_statr_chrg:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case CMD_Request_statr_chrg:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case CMD_Request_stop_chrg:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case CMD_Request_deal_log:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case CMD_Request_sum_update:
			return &g_ProtocolDCB.strSendCtrl[u8Port][9];
		case CMD_Request_set_device_param:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case CMD_Request_set_timing:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case CMD_Request_set_billing_model:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case CMD_Request_set_reboot:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
		case CMD_Request_set_update_ftp:
			return &g_ProtocolDCB.strSendCtrl[u8Port][14];
        case CMD_Response_S_QR_ACK:
            return &g_ProtocolDCB.strSendCtrl[u8Port][15];
		case CMD_Request_multi_deal_log:
			return &g_ProtocolDCB.strSendCtrl[u8Port][16];
		default:
			UPRINT("\r\nUpCtrl gn send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}

static SEND_CTRL* GetSendCtrl_YKC(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
		case YKC_S_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case YKC_S_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case YKC_S_Rate_Proving:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case YKC_S_Rate_Ask:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case YKC_S_RealData:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case YKC_S_Auth:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case YKC_S_Start_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case YKC_S_Stop_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case YKC_S_Chg_Record:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case YKC_S_Chg_Record_DD:
			return &g_ProtocolDCB.strSendCtrl[u8Port][9];
		case YKC_S_Sum_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case YKC_S_Para_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case YKC_S_TimeSyn_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case YKC_S_Rate_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
		case YKC_S_QR_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][14];
		case YKC_S_reboot_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][15];
		case YKC_S_update_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][16];
		case YKC_S_QR_ACK_DD:
			return &g_ProtocolDCB.strSendCtrl[u8Port][17];
		default:
			UPRINT("\r\nUpCtrl ykc send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}


static SEND_CTRL* GetSendCtrl_YKC_V2(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
		case YKC_V2_S_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case YKC_V2_S_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case YKC_V2_S_Rate_Proving:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case YKC_V2_S_Rate_Ask:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case YKC_V2_S_RealData:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case YKC_V2_S_Auth:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case YKC_V2_S_Start_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case YKC_V2_S_Stop_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case YKC_V2_S_Chg_Record:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case YKC_V2_S_Sum_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case YKC_V2_S_Device_Fault:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case YKC_V2_S_Device_Reset:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case YKC_V2_S_Deal_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
		case YKC_V2_S_Chg_Finish:
			return &g_ProtocolDCB.strSendCtrl[u8Port][14];
		case YKC_V2_S_Power_Change_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][15];
		case YKC_V2_S_TimeSyn_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][16];
		case YKC_V2_S_Rate_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][17];
		case YKC_V2_S_Max_Power_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][18];
		case YKC_V2_S_QR_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][19];
		case YKC_V2_S_Para_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][20];
		case YKC_V2_S_reboot_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][21];
		case YKC_V2_S_update_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][22];
		case YKC_V2_S_Key_Update_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][23];
		case YKC_V2_S_QR_ACK_DD:
			return &g_ProtocolDCB.strSendCtrl[u8Port][24];
		default:
			UPRINT("\r\nUpCtrl ykc_v2.1 send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}


static SEND_CTRL* GetSendCtrl_ANPEI(uint8_t u8Port, uint32_t cmd)
{
switch (cmd)
	{
		case ANPEI_S_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case ANPEI_S_U:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case ANPEI_S_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case ANPEI_S_clocksyn:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case ANPEI_S_RealData:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case ANPEI_S_Rate_SETAskB3:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case ANPEI_S_StartEnd_ChgAsk:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case ANPEI_S_Cardinf:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case ANPEI_S_CardStart_Chg:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case ANPEI_S_onlineEnd_ChgInfB12:
			return &g_ProtocolDCB.strSendCtrl[u8Port][9];
		case ANPEI_S_offlineEnd_ChgInfB15:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case ANPEI_S_RemoteUpgradeAck:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case ANPEI_S_FixtimeCmdAsk:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case ANPEI_S_SIMInfAck:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
		case ANPEI_S_PowerConASK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][14];
		case ANPEI_S_FeeModelB36Ask:
			return &g_ProtocolDCB.strSendCtrl[u8Port][15];
		case ANPEI_S_ChgCarInf:
			return &g_ProtocolDCB.strSendCtrl[u8Port][16];
		case ANPEI_S_2400Inf:
			return &g_ProtocolDCB.strSendCtrl[u8Port][17];
	  case ANPEI_S_ftpInfAsk:
			return &g_ProtocolDCB.strSendCtrl[u8Port][18];
		case ANPEI_S_PowerValAsk:
			return &g_ProtocolDCB.strSendCtrl[u8Port][19];
	  case ANPEI_S_Rate_SETAskB48:
			return &g_ProtocolDCB.strSendCtrl[u8Port][20];
		case ANPEI_S_Rate_Swtich:
			return &g_ProtocolDCB.strSendCtrl[u8Port][21];
		case ANPEI_S_FeeModelB52Ask:
			return &g_ProtocolDCB.strSendCtrl[u8Port][22];
		case ANPEI_S_onlineEnd_ChgInfB53:
			return &g_ProtocolDCB.strSendCtrl[u8Port][23];
		case ANPEI_S_offlineEnd_ChgInfB55:
			return &g_ProtocolDCB.strSendCtrl[u8Port][24];							
		case ANPEI_S_ChgPowerCon_inf:
			return &g_ProtocolDCB.strSendCtrl[u8Port][25];
		default:
			UPRINT("\r\nUpCtrl anpei send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}

static SEND_CTRL* GetSendCtrl_AHTT(uint8_t u8Port, uint32_t cmd)
{
switch (cmd)
	{
		case AHTT_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case AHTT_Heart_Set:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case AHTT_Heart_Search:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case AHTT_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case AHTT_Port_Domain:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case AHTT_MaxChgTime:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case AHTT_Sea_MaxChgTime:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case AHTT_Auth:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case AHTT_RealData:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case AHTT_Stop_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][9];
		case AHTT_Chg_Record:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case AHTT_ChgCard_Record:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case AHTT_Equipara:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case AHTT_Sea_Equipara:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
        case AHTT_GetPower:
            return &g_ProtocolDCB.strSendCtrl[u8Port][14];
		case AHTT_TimeSyn_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][15];
		case AHTT_State:
			return &g_ProtocolDCB.strSendCtrl[u8Port][16];
		case AHTT_Alarm:
			return &g_ProtocolDCB.strSendCtrl[u8Port][17];
		case AHTT_Network_Alarm:
			return &g_ProtocolDCB.strSendCtrl[u8Port][18];
		case AHTT_Temper_Alarm:
			return &g_ProtocolDCB.strSendCtrl[u8Port][19];
		case AHTT_SetTemper:
			return &g_ProtocolDCB.strSendCtrl[u8Port][20];
		case AHTT_set_update_ftp:
			return &g_ProtocolDCB.strSendCtrl[u8Port][21];
		default:
			UPRINT("\r\nUpCtrl ahtt send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}

static SEND_CTRL* GetSendCtrl_WJY(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
		case WJY_S_Identification:
			return &g_ProtocolDCB.strSendCtrl[u8Port][0];
		case WJY_S_Auth:
			return &g_ProtocolDCB.strSendCtrl[u8Port][1];
		case WJY_S_Start_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][2];
		case WJY_S_Stop_Chg_Ack:
			return &g_ProtocolDCB.strSendCtrl[u8Port][3];
		case WJY_S_Chg_Record:
			return &g_ProtocolDCB.strSendCtrl[u8Port][4];
		case WJY_S_RealData:
			return &g_ProtocolDCB.strSendCtrl[u8Port][5];
		case WJY_S_Rate_Ask:
			return &g_ProtocolDCB.strSendCtrl[u8Port][6];
		case WJY_S_Heart:
			return &g_ProtocolDCB.strSendCtrl[u8Port][7];
		case WJY_S_Device_Fault:
			return &g_ProtocolDCB.strSendCtrl[u8Port][8];
		case WJY_S_TimeSyn_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][9];
		case WJY_S_QR:
			return &g_ProtocolDCB.strSendCtrl[u8Port][10];
		case WJY_S_update_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][11];
		case WJY_S_update_rst:
			return &g_ProtocolDCB.strSendCtrl[u8Port][12];
		case WJY_S_reboot_ACK:
			return &g_ProtocolDCB.strSendCtrl[u8Port][13];
		case WJY_S_Sum:
			return &g_ProtocolDCB.strSendCtrl[u8Port][14];
		default:
			UPRINT("\r\nUpCtrl Wjy send cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}

static SEND_CTRL* GetSendCtrl_HaiNCT(uint8_t u8Port, uint32_t cmd)
{
    uint8_t idnex = 0;
    uint8_t ret = HaiNCT_GetSendTopicAndIndex(cmd, &idnex);
    if (ret) {
        UPRINT("\r\nUpCtrl hnct send cmd:0x%x err!\r\n", cmd);
        return NULL;
    } else {
        return &g_ProtocolDCB.strSendCtrl[u8Port][idnex];	
    }
}
static SEND_CTRL* GetSendCtrl(uint8_t u8Port, uint32_t cmd)
{
	return PSendCtrl(u8Port, cmd);
}

void SetSendTick(uint8_t u8Port, uint32_t cmd, int32_t tick)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->CycTimer = tick;
}

void SetSendEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;

	pSendCtrl->u8SendEnable = flag;

	SetSendTick(u8Port, cmd, NOWTICK);
}

uint8_t GetSendEnable(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return SEND_ENABLE_NULL;
	
	return pSendCtrl->u8SendEnable;
}

void SetSendImmdFlag(uint8_t u8Port, uint32_t cmd, uint8_t Flag)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u8ImmdFlag = Flag;

	return;
}

void Send_Immediately(uint8_t u8Port, uint32_t cmd)
{
	SetSendImmdFlag(u8Port, cmd, TRUE);
	return;
}

uint8_t GetSendImmdFlag(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return FALSE;
	
	return pSendCtrl->u8ImmdFlag;
}

int32_t GetSendTick(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return -1;
	
	return pSendCtrl->CycTimer;
}

void SetSendFlag(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u8SendFlag = flag;
}

void SetSendSrm(uint8_t u8Port, uint32_t cmd, uint16_t Srm)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u16UpSrm = Srm;
}

uint16_t GetSendSrm(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return SEND_FLAG_NULL;
	
	return pSendCtrl->u16UpSrm;
}

//=========================
static RECV_CTRL* GetRecvCtrl_GN(uint8_t u8Port, uint32_t cmd)
{
		
	switch (cmd)
	{
	case CMD_Response_Identification:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][0];
	case CMD_Response_Heart:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
	case CMD_Response_billing_verify:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
	case CMD_Response_billing_model:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
	case CMD_Response_realTime_gun:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
	case CMD_Response_apply_statr_chrg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
	case CMD_Response_statr_chrg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
	case CMD_Response_stop_chrg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
	case CMD_Response_deal_log:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
	case CMD_Response_sum_update:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
	case CMD_Response_set_device_param:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
	case CMD_Response_set_timing:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
	case CMD_Response_set_billing_model:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][12];
	case CMD_Response_set_reboot:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
	case CMD_Response_set_update_ftp:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][14];
    case CMD_Request_R_Ret_QR:
        return &g_ProtocolDCB.strRecvCtrl[u8Port][15];
	case CMD_Response_multi_billing_model:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][16];
	case CMD_Response_set_multi_billing:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][17];
	default:
		UPRINT("\r\nUpCtrl gn recv cmd:0x%x err!\r\n", cmd);
		return NULL;
	}
}

static RECV_CTRL* GetRecvCtrl_YKC(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
	case YKC_R_Identification:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][0];
	case YKC_R_Heart:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
	case YKC_R_Rate_Proving:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
	case YKC_R_Rate_Ask:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
	case YKC_R_RealData:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
	case YKC_R_Auth:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
	case YKC_R_Start_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
	case YKC_R_Stop_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
	case YKC_R_Chg_Record:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
	case YKC_R_Sum_Update:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
	case YKC_R_Set_Para:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
	case YKC_R_TimeSyn:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
	case YKC_R_Set_Rate:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][12];
	case YKC_R_Ret_QR:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
	case YKC_R_set_reboot:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][14];
	case YKC_R_set_update_ftp:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][15];
	case YKC_R_Set_QR_DD:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][16];
	default:
		UPRINT("\r\nUpCtrl ykc recv cmd:0x%x err!\r\n", cmd);
		return NULL;
	}
}

static RECV_CTRL* GetRecvCtrl_YKC_V2(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
	case YKC_V2_R_Identification:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][0];
	case YKC_V2_R_Heart:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
	case YKC_V2_R_Rate_Proving:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
	case YKC_V2_R_Rate_Ask:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
	case YKC_V2_R_RealData:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
	case YKC_V2_R_Auth:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
	case YKC_V2_R_Start_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
	case YKC_V2_R_Stop_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
	case YKC_V2_R_Chg_Record:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
	case YKC_V2_R_Sum_Update:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
	case YKC_V2_R_Device_Fault_ACK:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
	case YKC_V2_R_Device_Reset_ACK:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
	case YKC_V2_R_Deal:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][12];
	case YKC_V2_R_Chg_Finish_ACK:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
	case YKC_V2_R_Power_Change_Para:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][14];
	case YKC_V2_R_TimeSyn:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][15];
	case YKC_V2_R_Set_Rate:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][16];
	case YKC_V2_R_Max_Power:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][17];
	case YKC_V2_R_Ret_QR:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][18];
	case YKC_V2_R_Para:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][19];
	case YKC_V2_R_set_reboot:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][20];
	case YKC_V2_R_set_update_ftp:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][21];
	case YKC_V2_R_Key_Update:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][22];
	case YKC_V2_R_Set_QR_DD:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][23];
	default:
		UPRINT("\r\nUpCtrl ykc_v2.1 recv cmd:0x%x err!\r\n", cmd);
		return NULL;
	}
}

static RECV_CTRL* GetRecvCtrl_ANPEI(uint8_t u8Port, uint32_t cmd)
{	
	switch (cmd)
	{	
		case ANPEI_R_Identification:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][0];		
		case ANPEI_R_U:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
		case ANPEI_R_Heart:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
		case ANPEI_R_clocksyn:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
		case ANPEI_R_Rate_SETB2:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
		case ANPEI_R_StartEnd_Chg:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
		case ANPEI_R_CardinfAck:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
		case ANPEI_R_CardStart_ChgAck:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
		case ANPEI_R_onlineEnd_ChgInfAckB13:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
		case ANPEI_R_ChgDeduction_Inf:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
		case ANPEI_R_offlineEnd_ChgInfAckB16:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
		case ANPEI_R_RemoteUpgrade:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
		case ANPEI_R_FixtimeCmd:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][12];
		case ANPEI_R_NeedSIMInf:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
		case ANPEI_R_PowerCon:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][14];
		case ANPEI_R_FeeModelB35:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][15];
		case ANPEI_R_ftpInf:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][16];
		case ANPEI_R_PowerVal:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][17];
		case ANPEI_R_Rate_SETB47:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][18];
		case ANPEI_R_Rate_SwtichAsk:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][19];
		case ANPEI_R_FeeModelB51:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][20];
		case ANPEI_R_onlineEnd_ChgInfAckB54:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][21];
		case ANPEI_R_offlineEnd_ChgInfAckB56:
		    return &g_ProtocolDCB.strRecvCtrl[u8Port][22];
		default:
            UPRINT("\r\nUpCtrl ANPEI recv cmd:0x%x err!\r\n", cmd);
            return NULL;
	}
}
static RECV_CTRL* GetRecvCtrl_AHTT(uint8_t u8Port, uint32_t cmd)
{
switch (cmd)
	{
		case AHTT_Identification:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][0];
		case AHTT_Heart_Set:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
		case AHTT_Heart_Search:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
		case AHTT_Heart:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
		case AHTT_Port_Domain:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
		case AHTT_MaxChgTime:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
		case AHTT_Sea_MaxChgTime:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
		case AHTT_Auth:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
		case AHTT_RealData:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
		case AHTT_Stop_Chg_Ack:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
		case AHTT_Chg_Record:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
		case AHTT_ChgCard_Record:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
		case AHTT_Equipara:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][12];
		case AHTT_Sea_Equipara:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
        case AHTT_GetPower:
            return &g_ProtocolDCB.strRecvCtrl[u8Port][14];
		case AHTT_TimeSyn_ACK:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][15];
		case AHTT_State:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][16];
		case AHTT_Alarm:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][17];
		case AHTT_Network_Alarm:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][18];
		case AHTT_Temper_Alarm:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][19];
		case AHTT_SetTemper:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][20];
		case AHTT_set_update_ftp:
			return &g_ProtocolDCB.strRecvCtrl[u8Port][21];
		default:
			UPRINT("\r\nUpCtrl ahtt recv cmd:0x%x err!\r\n", cmd);
			return NULL;
	}
}

static RECV_CTRL* GetRecvCtrl_WJY(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
	case WJY_R_Identification:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][0];
	case WJY_R_Auth:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][1];
	case WJY_R_Start_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][2];
	case WJY_R_Stop_Chg:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][3];
	case WJY_R_Chg_Record:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][4];
	case WJY_R_Rate:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][5];
	case WJY_R_Heart:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][6];
	case WJY_R_Device_Fault:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][7];
	case WJY_R_TimeSyn:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][8];
	case WJY_R_QR:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][9];
	case WJY_R_update_ftp:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][10];
	case WJY_R_update_rst:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][11];
	case WJY_R_set_reboot:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][12];	
	case WJY_R_Sum:
		return &g_ProtocolDCB.strRecvCtrl[u8Port][13];
	default:
		UPRINT("\r\nUpCtrl Wjy recv cmd:0x%02x err!\r\n", cmd);
		return NULL;
	}
}

static RECV_CTRL* GetRecvCtrl_HaiNCT(uint8_t u8Port, uint32_t cmd)
{	
    uint8_t idnex = 0;
    uint8_t ret = HaiNCT_GetRecvTopicAndIndex(cmd, &idnex);
    if (ret) {
        UPRINT("\r\nUpCtrl hnct recv cmd:0x%x err!\r\n", cmd);
        return NULL;
    } else {
        return &g_ProtocolDCB.strRecvCtrl[u8Port][idnex];	
    }
}


static RECV_CTRL* GetRecvCtrl(uint8_t u8Port, uint32_t cmd)
{
	return PRecvCtrl(u8Port, cmd);
}

void SetRecvTick(uint8_t u8Port, uint32_t cmd, uint32_t tick)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u32CycTimer = tick;
}

int32_t GetRecvTick(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return -1;
	
	return pRecvCtrl->u32CycTimer;
}

void SetRecvEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u8RecvEnable = flag;
}

uint8_t GetRecvEnable(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return RECV_ENABLE_NULL;
	
	return pRecvCtrl->u8RecvEnable;
}


uint8_t GetRecvRptCnt(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return RECV_ENABLE_NULL;
	
	return pRecvCtrl->u8RptCnt;
}

void SetRecvRptUpt(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u8RptCnt++;
}
void SetClearRecvRptUpt(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u8RptCnt = 0;
}

//=============================================================================================================
//设备重启
void PileRebootCheck(void)
{
	ProtocolDCB *pTcpDataCtrl = &g_ProtocolDCB;
	uint8_t i = 0;
	
	if(pTcpDataCtrl->PlatTask.reboot_flag == E_Reboot_Null)
		return;

	for(i = 0; i < GUN_NUM; i++)
	{
		//充电不重启
		if(eChargeState_Idle != logic_get_gun_state(i))
		{
            if (pTcpDataCtrl->PlatTask.reboot_flag == E_Reboot_Idle) {
			    pTcpDataCtrl->PlatTask.reboot_flag = E_Reboot_Null;
			    return;
            }
		}
	}

	if (JudgeTimeOutMs(pTcpDataCtrl->PlatTask.reboot_tick, eTick_2S) == TRUE) {

		printf("charge board reboot...\r\n");

		//主板重新启动
		fgv_CtrlPileOpr(E_DEV_CTRL_CMD_REBOOT);
		
		//控制主板重启完成后，网络单元重启
		osDelay(100);
		
		printf("network board reboot...\r\n");

		NVIC_SystemReset();
	}
	
	return;
}

static int extractMiddlePart(char *input, char *pathOutput, char *fileNameOutput) {
    // 检查最后一个字符是否为 /
    if (input[strlen(input) - 1] == '/') {
        // 如果最后一个字符为 /，直接返回默认值
        strcpy(pathOutput, input);
        strcpy(fileNameOutput, "D3-A32EB_B");
        return 0;
    }
    // 查找 .bin 的位置
    int binPos = MyStrstr(input, ".bin");
    if (binPos == -1) {
        // 如果没有找到 .bin，返回默认值
        return -1;
    }

    // 检查 .bin 是否在字符串末尾
    if (input[binPos + 4] != '\0') {
        // 如果 .bin 不是字符串末尾，返回默认值
        return -1;
    }

    // 从 .bin 的位置向前查找最后一个 /
    const char *lastSlashPos = strrchr(input, '/');
    if (lastSlashPos == NULL) {
        // 如果没有找到 / 或者 / 在 .bin 之后，返回默认值
        return -1;
    }

    // 提取路径部分
    size_t pathLength = lastSlashPos - input + 1;
    strncpy(pathOutput, input, pathLength);
    pathOutput[pathLength] = '\0';

    // 提取文件名部分
    size_t fileNameStart = lastSlashPos - input + 1;
    size_t fileNameLength = binPos - fileNameStart;
    strncpy(fileNameOutput, input + fileNameStart, fileNameLength);
    fileNameOutput[fileNameLength] = '\0';

    // 检查提取结果是否为空
    if (pathOutput[0] == '\0' && fileNameOutput[0] == '\0') {
        return -1;
    }
    return 0;
}

void g_UpdatePathToName(char *inputPath, char *filePathOutput, char *fileNameOutput)
{
    char t_path[33] = {0};
    char t_fileName[33] = {0};
    int ret = extractMiddlePart((char *)inputPath, t_path, t_fileName);
    if (ret == -1) {
        strcpy(t_path, FTP_FILDER_PATH);
        strcpy(t_fileName, "D3-A32EB_B");
    }
    strcpy(filePathOutput, t_path);
    strcpy(fileNameOutput, t_fileName);
}

void g_PileUpdateInterface(char *ip, uint16_t port, char *name, char *password, char *path, char *fileName)
{
    PlatTaskExcute *PlatTaskExcute = &g_ProtocolDCB.PlatTask;
    GN_Ftp_Info *pFtpInfo = &PlatTaskExcute->u_updateInfo.ftpInfo;
    
	memset(&g_ProtocolDCB.PlatTask.u_updateInfo.ftpInfo, 0, sizeof(g_ProtocolDCB.PlatTask.u_updateInfo.ftpInfo));

    // 寻找需要特殊处理的特殊字段
    //用户名为gnroot时，ftp选择默认路径
    //用户名为root时，ftp选择gn路径
    char t_path[33] = {0};
    char my_s[10] = "gn";
    int ret = MyStrstr((char *)name, "root");
    if (ret >= 0) {
        //使用默认用户名密码
        memcpy(pFtpInfo->update_ip, FTP_USER_IP, 17);
        pFtpInfo->update_port = FTP_USER_PORT;
        strncpy((char *)pFtpInfo->update_username, FTP_USER_NAME, 16);
        strncpy((char *)pFtpInfo->update_password, FTP_USER_PSW, 16);
        if (ret > 9) {
            ret = 9;
        }
        memcpy(my_s, (char *)name, ret);
        memcpy(t_path, FTP_FILDER_PATH, 32);
        strcat(t_path, my_s);
        strcat(t_path, "/");
        strcpy((char *)pFtpInfo->update_file_path, t_path);
        pFtpInfo->update_file_path[sizeof(pFtpInfo->update_file_path)-1] = 0;
    } else {
        //按照入参全部赋值
        strcpy((char *)pFtpInfo->update_ip, ip);
        pFtpInfo->update_ip[sizeof(pFtpInfo->update_ip)-1] = 0;
        pFtpInfo->update_port = port;
        strcpy((char *)pFtpInfo->update_username, name);
        pFtpInfo->update_username[sizeof(pFtpInfo->update_username)-1] = 0;
        strcpy((char *)pFtpInfo->update_password, password);
        pFtpInfo->update_password[sizeof(pFtpInfo->update_password)-1] = 0;
        strcpy((char *)pFtpInfo->update_file_path, path);
        pFtpInfo->update_file_path[sizeof(pFtpInfo->update_file_path)-1] = 0;
        strcpy((char *)pFtpInfo->update_file_name, fileName);
        pFtpInfo->update_file_name[sizeof(pFtpInfo->update_file_name)-1] = 0;
    }
    //区分AB升级
    if (strcmp((const char*)password, "aUpdate") == 0) {
        strncpy((char *)pFtpInfo->update_file_name, "D3-A32EB_A", sizeof("D3-A32EB_A"));
    } else if (strcmp((const char*)password, "bUpdate") == 0){
        strncpy((char *)pFtpInfo->update_file_name, "D3-A32EB_B", sizeof("D3-A32EB_B"));
    }
}

//设备升级
void PileUpdateCheck(void)
{
    PlatTaskExcute *PlatTaskExcute = &g_ProtocolDCB.PlatTask;

    GN_Ftp_Info *pFtpInfo = &PlatTaskExcute->u_updateInfo.ftpInfo;

	uint8_t i = 0;
	
	if(E_Update_Null == PlatTaskExcute->updata_flag)
		return;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		if(eMonitorState_Service != monitor_get_MonitorState(i)) {
			PlatTaskExcute->updata_flag = E_Update_Null;
			return;
		}
		
		if(eChargeState_Idle != logic_get_gun_state(i)){
			PlatTaskExcute->updata_flag = E_Update_Null;
			return;
		}
	}

	if (JudgeTimeOutMs(PlatTaskExcute->updata_delay_tick, eTick_2S) == FALSE) {
		return;
	}

    if (PlatTaskExcute->updata_flag == E_Update_Ftp) {
        SIM900FtpInfoSetip((char *)pFtpInfo->update_ip, pFtpInfo->update_port);
        SIM900FtpInfoSetUserName((char *)pFtpInfo->update_username, (char *)pFtpInfo->update_password);
        SIM900FtpInfoSetPath((char *)pFtpInfo->update_file_path, (char *)pFtpInfo->update_file_name);

        OtaStart_tcp();
    }

	PlatTaskExcute->updata_flag = E_Update_Null;
	
	return;
}



//===================================================================加密
/* len为input的len */
static void sha256_sevice(uint8_t *input, uint32_t len, uint8_t *output)
{
	sha256_get(output, input, len);
	return;
}

/* 仅用于16byte字节的加解密 */
void aes128_sevice(uint8_t mode, const unsigned char *key, unsigned char *iv, const unsigned char *input, unsigned char *output)
{
    struct AES_ctx ctx;
    uint8_t iv_tmp[16] = {0};
    uint8_t input_tmp[16] = {0};
    
    memcpy(iv_tmp,iv,sizeof(iv_tmp));
    memcpy(input_tmp,input,sizeof(input_tmp));
    
    AES_init_ctx_iv(&ctx, key, iv_tmp);
	
    if (mode == MBEDTLS_AES_ENCRYPT)
        AES_CBC_encrypt_buffer(&ctx, input_tmp, 16);
    else
        AES_CBC_decrypt_buffer(&ctx, input_tmp, 16);
	
    memcpy(output,input_tmp,16);
    return;
}
//运营平台协议重连
static void OperatePlat_Reconnect(eNetSocket SocketID)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM_MAX; i++)
	{
		memset(pProtocolDCB->strRecvCtrl[i], 0, sizeof(pProtocolDCB->strRecvCtrl[i]));
		memset(pProtocolDCB->strSendCtrl[i], 0, sizeof(pProtocolDCB->strSendCtrl[i]));
	}
	
    Set_PlatConnectSta(eOnline_Off);
	
	dev_setErrExsit_all(eErr_PlatformOffline, __LINE__);
	
	NormProtocolInit();
	
	return;
}

//运维平台协议重连
static void MaintenancePlat_Reconnect(eNetSocket SocketID)
{
	
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM_MAX; i++)
	{
		memset(pProtocolDCB->strOmSendCtrl[i], 0, sizeof(pProtocolDCB->strOmSendCtrl[i]));
		memset(pProtocolDCB->strOmRecvCtrl[i], 0, sizeof(pProtocolDCB->strOmRecvCtrl[i]));
	}
	
    g_ProtocolDCB.OmPlatSta.eOnlineType = eOnline_Off;
	
	//dev_setErrExsit_all(eErr_PlatformOffline, __LINE__); //JJUNIVETEST
	
	return;
}

void Plat_Reconnect(eNetSocket SocketID)
{
	if(eSocket_GPRS2 == SocketID)
	{
		MaintenancePlat_Reconnect(SocketID);
	}
	else
	{
		OperatePlat_Reconnect(SocketID);
	}
	
	return;
}

void PlatHeartTickRefresh(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	pProtocolDCB->PlatSta.no_Comm_tick = Get_Systick();
	
	return;
}

//三分钟无心跳，重连；10分钟无任何交互重连
static void UpNoCommTimeout(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	//3分钟无通信,直接重连
	if(JudgeTimeOutMs(pProtocolDCB->PlatSta.no_Comm_tick, eTick_180S))
	{
        printf("\r\nUpNoCommTimeout\r\n");
		pProtocolDCB->PlatSta.no_Comm_tick = Get_Systick();
        UpOfflineDeal(eSocket_GPRS1);
	}
	return;
}


static void GN_Malloc(void)
{
	uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	malloc_len = sizeof(RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pRecvData = (RECV_Data*)MALLOC(malloc_len);
	
	if(NULL == pProtocolDCB->pRecvData)
		UPRINT("\r\n%s malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pRecvData, 0, malloc_len);
		UPRINT("\r\n%s malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}
	
	return;
}

static void YKC_Malloc(void)
{
	uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	malloc_len = sizeof(YKC_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pYKCRecvData = (YKC_RECV_Data*)MALLOC(malloc_len);
	
	if(NULL == pProtocolDCB->pYKCRecvData)
		UPRINT("\r\n%s malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pYKCRecvData, 0, malloc_len);
		UPRINT("\r\n%s malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}
	
	return;
}

static void YKC_Malloc_V2(void)
{
	uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	malloc_len = sizeof(YKC_RECV_Data_V2)*GUN_NUM_MAX;
	pProtocolDCB->pYKCRecvData_v2 = (YKC_RECV_Data_V2*)MALLOC(malloc_len);
	
	if(NULL == pProtocolDCB->pYKCRecvData_v2)
		UPRINT("\r\n%s ykc2.1_malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pYKCRecvData, 0, malloc_len);
		UPRINT("\r\n%s ykc2.1_malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}
	
	return;
}


static void ANPEI_Malloc(void)
{
    uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    
	malloc_len = sizeof(ANPEI_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pANPEIRecvData = (ANPEI_RECV_Data*)MALLOC(malloc_len);
	if(NULL == pProtocolDCB->pANPEIRecvData)
		UPRINT("\r\n%s malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pANPEIRecvData, 0, malloc_len);
		UPRINT("\r\n%s malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}	
	return;
}

static void AHTT_Malloc(void)
{
	uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	malloc_len = sizeof(AHTT_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pAHTTRecvData = (AHTT_RECV_Data*)MALLOC(malloc_len);
	
	if(NULL == pProtocolDCB->pAHTTRecvData)
		UPRINT("\r\n%s malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pYKCRecvData, 0, malloc_len);
		UPRINT("\r\n%s malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}
	
	return;
}
static void HaiNCT_Malloc(void)
{
    uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    
	malloc_len = sizeof(HaiNCT_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pHaiNCTRecvData = (HaiNCT_RECV_Data*)MALLOC(malloc_len);
	if(NULL == pProtocolDCB->pHaiNCTRecvData)
		UPRINT("\r\n%s malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pHaiNCTRecvData, 0, malloc_len);
		UPRINT("\r\n%s malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}	
	return;
}


static void WJY_Malloc(void)
{
    uint32_t malloc_len = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    
	malloc_len = sizeof(WJY_RECV_Data)*GUN_NUM_MAX;
	pProtocolDCB->pWJYRecvData = (WJY_RECV_Data*)MALLOC(malloc_len);
	if(NULL == pProtocolDCB->pHaiNCTRecvData)
		UPRINT("\r\n%s wjy malloc len:0x%x fail! \r\n", NormProtocolName(), malloc_len);
	else
	{
		memset(pProtocolDCB->pHaiNCTRecvData, 0, malloc_len);
		UPRINT("\r\n%s wjy malloc len:0x%x succ! \r\n", NormProtocolName(), malloc_len);
	}	
	return;
}

//==============================
//协议控制
typedef void (*PProtocolFunc)(void);
typedef RECV_CTRL* (*PlatRecvCtrl)(uint8_t u8Port, uint32_t cmd);
typedef SEND_CTRL* (*PlatSendCtrl)(uint8_t u8Port, uint32_t cmd);

typedef struct
{
    uint32_t		u32PlatType;			//平台类型
    eSocketType     SocketType;				//连接类型
	U8				u8BSTimeSyn; 			//是否需要周期基站对时
	
    PProtocolFunc 	pProtocolMalloc;		//内存分配
    PProtocolFunc 	pProtocolInit;			//平台初始化
    
    PlatRecvCtrl	pPlatRecvCtrl;			//接收控制
	PlatSendCtrl	pPlatSendCtrl;			//发送控制
    PProtocolFunc 	pProtocolDeal;			//平台主函数
	char			cName[16];				//平台名
}Plat_Type_Ctrl;

const Plat_Type_Ctrl PlatCtrlMap[]={
    {ePlatType_GN		,eSocket_TCP	,TRUE	,GN_Malloc		,NULL,			GetRecvCtrl_GN,		GetSendCtrl_GN,		GNUpProtocolDeal,		"公牛"},		//
    {ePlatType_GNP		,eSocket_TCP	,TRUE	,GN_Malloc		,NULL,			GetRecvCtrl_GN, 	GetSendCtrl_GN,		GNUpProtocolDeal,		"快速直连gn+"},//
    {ePlatType_YKC		,eSocket_TCP	,TRUE	,YKC_Malloc		,NULL,			GetRecvCtrl_YKC, 	GetSendCtrl_YKC,	YKCUpProtocolDeal,		"云快充"},		//
    {ePlatType_YKC_V2	,eSocket_TCP	,TRUE	,YKC_Malloc_V2	,NULL,			GetRecvCtrl_YKC_V2, GetSendCtrl_YKC_V2,	YKCUpProtocolDeal_V2,	"云快充v2.1"},		//
	{ePlatType_gwYKC    ,eSocket_TCP	,TRUE	,YKC_Malloc		,NULL,			GetRecvCtrl_YKC, 	GetSendCtrl_YKC,	YKCUpProtocolDeal,		"云快充"},		//
    {ePlatType_TOWER	,eSocket_TCP	,TRUE	,YKC_Malloc		,NULL,			GetRecvCtrl_YKC, 	GetSendCtrl_YKC,	YKCUpProtocolDeal,		"铁塔"},		//
    {ePlatType_DD		,eSocket_TCP	,TRUE	,YKC_Malloc		,NULL,			GetRecvCtrl_YKC, 	GetSendCtrl_YKC,	YKCUpProtocolDeal,		"东电"},		//
    {ePlatType_ANPEI    ,eSocket_TCP    ,TRUE   ,ANPEI_Malloc   ,NULL,          GetRecvCtrl_ANPEI,  GetSendCtrl_ANPEI,  ANPEIUpProtocolDeal,    "浙江国网" },
    {ePlatType_HaiNCT   ,eSocket_TCP    ,TRUE   ,HaiNCT_Malloc  ,NULL,          GetRecvCtrl_HaiNCT, GetSendCtrl_HaiNCT, HaiNCTUpProtocolDeal,   "海宁城投" },
    {ePlatType_WJY   	,eSocket_TCP    ,TRUE   ,WJY_Malloc	  	,NULL,          GetRecvCtrl_WJY, 	GetSendCtrl_WJY, 	WJYUpProtocolDeal,   	"蔚景云" },
	// 开机，modbus类型未设置，为了不死机					xw 202407
	{ePlatType_AHTT		,eSocket_TCP	,TRUE	,AHTT_Malloc	,NULL,			GetRecvCtrl_AHTT,	GetSendCtrl_AHTT,	AHTTUpProtocolDeal,		"安徽铁塔"}, //
	{ePlatType_WLD		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"卫莱电"},//
	{ePlatType_XJK		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"小桔-快"},//
	{ePlatType_YS		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"ys"},//
	{ePlatType_TLD		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"特来电"},//
	{ePlatType_WJYB		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"蔚景云-标"},//
	{ePlatType_QWJ		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"芊万佳"},//
	{ePlatType_XX		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"小象"},//
	{ePlatType_ZQZC		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"朱雀智充"},//
	{ePlatType_BJYCW	,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"北京易充网"},//
	{ePlatType_GWYCD	,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"国网易充电"},//
	{ePlatType_JNJTJT	,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"jnjtjt"},    // 济南静态交通
	{ePlatType_XJB		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"小桔-标"},//
	{ePlatType_NFDW		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"南方电网"},//
	{ePlatType_XXCD		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"星星充电"},//
	{ePlatType_PTXNY	,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"普天新能源"},//
	{ePlatType_FYJG		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"阜阳监管"},//
	{ePlatType_AHLLCD	,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"ahllcd"},// 安徽洛洛充电
	{ePlatType_BSC		,eSocket_TCP	,TRUE	,NULL			,NULL,			NULL,				NULL,				NULL,					"巴士充"},//
};


//=======================================================================

//获取当前平台在map中的位置
uint8_t GetPlatMapIndex()
{
    uint8_t u8PlatIndex = get_ChgParam_plat_type();
    for (int i = 0; i < sizeof(PlatCtrlMap) / sizeof(Plat_Type_Ctrl); i++) {
        if (u8PlatIndex == PlatCtrlMap[i].u32PlatType) {
            return i;
        }
    }
    //如果没有匹配的，返回公牛
    return ePlatType_GN;
}

static RECV_CTRL* PRecvCtrl(uint8_t u8Port, uint32_t cmd)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	if(NULL != pPlatCtrlMap->pPlatRecvCtrl)
		return pPlatCtrlMap->pPlatRecvCtrl(u8Port, cmd);
	
	return NULL;
}

static SEND_CTRL* PSendCtrl(uint8_t u8Port, uint32_t cmd)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	if(NULL != pPlatCtrlMap->pPlatSendCtrl)
		return pPlatCtrlMap->pPlatSendCtrl(u8Port, cmd);
	
	return NULL;
}

//协议是否需要基站对时
U8 NormProtocolIsBSTimeSyn(void)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	return pPlatCtrlMap->u8BSTimeSyn;
}


//
eSocketType NormProtocolSocketType(void)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	return pPlatCtrlMap->SocketType;
}

char const *NormProtocolName(void)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	return pPlatCtrlMap->cName;
}
//=======================================================================

void NormProtocolMalloc(void)
{
	uint8_t u8PlatIndex = get_ChgParam_plat_type();
	PlatCfgInfo *platCfgInfo = fgv_GetPlatCfgInfo();

	// 平台类型错误，改为默认公牛平台，不然会导致程序无法运行								xw 202407
	if(u8PlatIndex > ARRAY_SIZE(PlatCtrlMap))
	{
		platCfgInfo->PltMainType = ePlatType_GN;
		u8PlatIndex = ePlatType_GN;
	    Set_platParam(platCfgInfo);
		UPRINT("\r\n palt default %d\r\n", platCfgInfo->PltMainType);
	}

	u8PlatIndex = GetPlatMapIndex();

	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	if(NULL != pPlatCtrlMap->pProtocolMalloc)
		pPlatCtrlMap->pProtocolMalloc();
	
	return;
}

void NormProtocolInit(void)
{
	uint8_t u8PlatIndex = GetPlatMapIndex();
	const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];
	
	if(NULL != pPlatCtrlMap->pProtocolInit)
		pPlatCtrlMap->pProtocolInit();
	
	return;
}

//平台连接公共部分
void PlatConnectCommDeal(void)
{
	PileRebootCheck();      //重启命令执行
	PileUpdateCheck();      //升级命令执行

    
	for (int i = 0; i < GUN_NUM; i++) {
        ChargingRecordUpdateScan(i);        //订单部分实时数据更新
    }
	//即插即充
	if (fgv_GetPileCfgOffLinChrg()) {
		return;
	}

	for (int i = 0; i < GUN_NUM; i++) {
		StartingTimeOutScan(i);	            //车辆超时不启动停止机制
        PauseTimeOutScan(i);                //暂停超时机制
        Cost_Main_Single(i);                //平台计费部分
        // ChargingRecordUpdateScan(i);        //订单部分实时数据更新
		GNUpChargeRecordStorage(i);	        //充电中1分钟存储订单信息
		DetectAbnormalScan(i);		        //充电中异常检测，例如余额不足
		GNUpChargeRecordJudgestart(i);      //收到启动命令后，状态是否改变
		ChargeConnectGunWaitCardScan(i);	//刷卡充电

        ChargeFaultActiveStop(i);            //网络的单元产生故障主动停止

        //拔枪清除数据
        GunLeaveCarChargeDataClear(i, GetPile_gun_connect(i));
    }
}

//主函数
void UpProtocolDeal(void)
{
	//升级的时候不链接平台，但需要更新超时时间
	if (GprsGetSocketType() == eSocket_FTP) {
        PlatHeartTickRefresh();
		return;
	}
	
	//场内模式也重连，用来刷新时钟
	UpNoCommTimeout();
	
	//厂内即插即充
	if(fgv_GetPileCfgOffLinChrg())
		return;
	
    uint8_t u8PlatIndex = GetPlatMapIndex();
    const Plat_Type_Ctrl *pPlatCtrlMap = &PlatCtrlMap[u8PlatIndex];

	if(NULL != pPlatCtrlMap->pProtocolDeal)
		pPlatCtrlMap->pProtocolDeal();

	return;
}

void runProtocolTask()
{
	OperatePlat_Init();

    while(1)
    {
        OM_GNUpProtocolDeal();  //运维平台

        UpProtocolDeal();       //平台协议部分
        
        PlatConnectCommDeal();  //平台连接处理公共部分

		ota_run_check();

		fgv_otaMain();	//升级任务
		
        vTaskDelay(20);
    }
}

