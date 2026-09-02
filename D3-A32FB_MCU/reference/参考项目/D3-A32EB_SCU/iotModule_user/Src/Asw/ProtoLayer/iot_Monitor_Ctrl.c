#include <stdlib.h>
#include "iot_Monitor_Ctrl.h"
#include "protocol_data.h"


static SEND_CTRL* OM_GetSendCtrl(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
		case OM_CMD_Request_Identification:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][0];
		case OM_CMD_Request_Heart:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][1];
		case OM_CMD_Request_Network_module_info:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][2];
		case OM_CMD_Request_realTime_gun:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][3];	
		case OM_CMD_Request_Meter_base_number:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][4];
		case OM_CMD_Request_order_info:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][5];
		case OM_CMD_Request_set_device_param:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][6];
		case OM_CMD_Request_Qrcode_update:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][7];
		case OM_CMD_Request_set_reboot:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][8];
		case OM_CMD_Request_set_update_ftp_:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][9];
		case OM_CMD_Request_Lock_machine:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][10];
		case OM_CMD_Request_Lock_state:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][11];
		case OM_CMD_Request_Log_read:
			return &g_ProtocolDCB.strOmSendCtrl[u8Port][12];									
		default:
			return NULL;
	}
}

void OM_SetSendTick(uint8_t u8Port, uint32_t cmd, int32_t tick)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->CycTimer = tick;
}

void OM_SetSendEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u8SendEnable = flag;

	OM_SetSendTick(u8Port, cmd, NOWTICK);
}

uint8_t OM_GetSendEnable(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return SEND_ENABLE_NULL;
	
	return pSendCtrl->u8SendEnable;
}

void OM_SetSendImmdFlag(uint8_t u8Port, uint32_t cmd, uint8_t Flag)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u8ImmdFlag = Flag;

	return;
}
void OM_Send_Immediately(uint8_t u8Port, uint32_t cmd)
{
	OM_SetSendImmdFlag(u8Port, cmd, TRUE);
}

uint8_t OM_GetSendImmdFlag(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return FALSE;
	
	return pSendCtrl->u8ImmdFlag;
}

int32_t OM_GetSendTick(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return -1;
	
	return pSendCtrl->CycTimer;
}
void OM_SetSendFlag(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u8SendFlag = flag;
}


void OM_SetSendSrm(uint8_t u8Port, uint32_t cmd, uint16_t Srm)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return;
	
	pSendCtrl->u16UpSrm = Srm;
}

uint16_t OM_GetSendSrm(uint8_t u8Port, uint32_t cmd)
{
	SEND_CTRL *pSendCtrl = OM_GetSendCtrl(u8Port, cmd);
	
	if(NULL == pSendCtrl)
		return SEND_FLAG_NULL;
	
	return pSendCtrl->u16UpSrm;
}



static RECV_CTRL* OM_GetRecvCtrl(uint8_t u8Port, uint32_t cmd)
{
	switch (cmd)
	{
	case OM_CMD_Response_Identification:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][0];
	case OM_CMD_Response_Heart:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][1];
	case OM_CMD_Response_Network_module_info:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][2];	
	case OM_CMD_Response_realTime_gun:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][3];	
	case OM_CMD_Response_order_info:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][4];	
	case OM_CMD_Response_set_device_param:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][5];	
	case OM_CMD_Response_Qrcode_update:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][6];	
	case OM_CMD_Response_set_reboot:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][7];	
	case OM_CMD_Response_set_update_ftp:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][8];	
	case OM_CMD_Response_Lock_machine:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][9];	
	case OM_CMD_Response_Lock_state:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][10];	
	case OM_CMD_Response_Log_read:
		return &g_ProtocolDCB.strOmRecvCtrl[u8Port][11];
	default:
		UPRINT("\r\nUpCtrl gn-monitor recv cmd:0x%x err!\r\n", cmd);
		return NULL;
	}
}

void OM_SetRecvTick(uint8_t u8Port, uint32_t cmd, uint32_t tick)
{
	RECV_CTRL *pRecvCtrl = OM_GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u32CycTimer = tick;
}  

int32_t OM_GetRecvTick(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = OM_GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return -1;
	
	return pRecvCtrl->u32CycTimer;
}


void OM_SetRecvEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag)
{
	RECV_CTRL *pRecvCtrl = OM_GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return;
	
	pRecvCtrl->u8RecvEnable = flag;
}


uint8_t OM_GetRecvEnable(uint8_t u8Port, uint32_t cmd)
{
	RECV_CTRL *pRecvCtrl = OM_GetRecvCtrl(u8Port, cmd);
	
	if(NULL == pRecvCtrl)
		return RECV_ENABLE_NULL;
	
	return pRecvCtrl->u8RecvEnable;
}

