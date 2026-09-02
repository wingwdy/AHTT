//////////////////////////////////////////////////////////////////////////
//Copyright (C), 2023, 
//文件名：		CommInterface.c
//作者:			
//版本号:       V1.0
//创建日期:		2023-11-30
//说明:			GPRS/CDMA 通讯
//函数列表:		
//修改记录:
//				2023-11-30
//////////////////////////////////////////////////////////////////////////
#include "CommInterface.h"
#include "protocol_ctrl.h"
#include "ProtoLayerHeaderSummary.h"


//联网步骤
U8 Comm_getNetStep(eNetSocket SocketID)
{
	const ATCmdItem *pATCmd = GetCurrentATCmdItem(SocketID);

	if (pATCmd != NULL) {
		return pATCmd->ATFunc;
	}
	return eAT_Null;
}

//连接是否成功
U8 Comm_getIpSuces(eNetSocket SocketID)
{
	SocketDCB* pATMDData = &g_strGprsModemDCB.strSocketDCB[SocketID];

	if (pATMDData->ConnectState == eSocket_Online) {
		return TRUE;
	}
	return FALSE;
}

//获取网络异常状态
U8 Comm_getNetAbnormalSta(eNetSocket SocketID)
{
    uint8_t t_netState = GPRS_Getonlineflag(SocketID);

    if (t_netState == eSocket_Neting) {
        return eNet_SimErro;//网络连接异常
    } else if (t_netState == eSocket_Attach) {
        return eNet_IPErro;//ip连接异常
    } else if (t_netState == eSocket_Online) {
        return eNet_CommErro;//通信异常
    }
	return eNet_Connecting;
}

//4G模块平台重连
//此函数会加AT任务，注意：最好不要放在AT接收处理函数里
void Comm_PlatReconnect(eNetSocket SocketID, uint16_t line)
{  
    if (eSocket_Attach > GPRS_Getonlineflag(SocketID)) {
        return;
    }
    Plat_Reconnect(SocketID);
	GPRS_Setonlineflag(SocketID, eSocket_Reconnect);
    printf("Comm_PlatReconnect SocketID:%d\r\n", SocketID);

	return;
}


void SocketConnectSucsClear(eNetSocket SocketID)
{  
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];

    pSocketDCB->ReconnectCnt = 0;
    
	return;
}

//立即重连
void AT_ImtlyReconnct(eNetSocket SocketID, uint16_t line)
{  
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
    
    printf("AT_ImtlyReconnct\r\n");

    pGprsModemDCB->ATAbnormal = eATStatus_Abnormal;
    ReStartATModem();

	return;
}

void Comm_HttpsStart(void)
{
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;

	AddModemOptTask(eSocket_GPRS1, eAT_HPSetPDP); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_HPRequestheader); 	//
	AddModemOptTask(eSocket_GPRS1, eAT_HPSetURL);			//
	AddModemOptTask(eSocket_GPRS1, eAT_HPDownload);			//
	
	pHttpDCB->u8JSOtaOrParam = eJSHttp_Ota;	
	return;
}

void Comm_JSHttpsParamStart(void)
{
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	AddModemOptTask(eSocket_GPRS1, eAT_HPSetPDP); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_HPSslId); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_SSLVer); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_SSLSut); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_SSLLvl); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_UFSCatch);		//
	AddModemOptTask(eSocket_GPRS1, eAT_UFSDelete); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_QFUpload); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_QFUploadAck); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_SSLCAPath); 			//
	
	AddModemOptTask(eSocket_GPRS1, eAT_HPRequestheader); 	//
	AddModemOptTask(eSocket_GPRS1, eAT_HPSetURL);			//
	AddModemOptTask(eSocket_GPRS1, eAT_HPDownload);			//	
//	AddModemOptTask(eSocket_GPRS1, eAT_HPGet);			//	
	
	pHttpDCB->u8JSOtaOrParam = eJSHttp_Param;
	pHttpDCB->u8JSParamSucc = FALSE;
	
	return;
}

void Comm_JSHttpsParamSucc(void)
{
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	if(TRUE == pHttpDCB->u8JSParamSucc)
	{
		pHttpDCB->u8JSParamSucc = FALSE;
		Comm_PlatReconnect(eSocket_GPRS1, __LINE__);
	}
	
	return;
}

void Comm_Init(void)
{
	Comm_PlatReconnect(eSocket_GPRS1, __LINE__);
	
	// platmod_init();
	return;
}