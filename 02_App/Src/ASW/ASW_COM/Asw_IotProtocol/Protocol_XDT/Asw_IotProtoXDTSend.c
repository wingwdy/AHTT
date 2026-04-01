/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoXDTM.h"
#include "FrameQueue.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "Asw_ChargeIf.h"
#include "Version.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/



/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t IotXDT_QueryAttachCredential_ITEM811(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReqeustTimeSync_ITEM821(uint8_t port, void *pBuf);
static uint16_t IotXDT_RequestLink_ITEM823(uint8_t port, void *pBuf);
static uint16_t IotXDT_RequestResetResponse_ITEM826(uint8_t port, void *pBuf);
static uint16_t IotXDT_RequestRateMode_ITEM831(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportRateMode_ITEM835(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportRateModeSetResponseEvent_ITEM836(uint8_t port, void *pBuf);
static uint16_t IotXDT_QueryRateModeRsp_ITEM837(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportPileState_ITEM841(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportPileErrInfo_ITEM843(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportPileData_ITEM845(uint8_t port, void *pBuf);
static uint16_t IotXDT_CallRealDataResponse_ITEM847(uint8_t port, void *pBuf);
static uint16_t IotXDT_RequestAuth_ITEM851(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportCategory_ITEM853(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportChargeStartReponse_ITEM862(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportChargeStartEvent_ITEM863(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportChargeStopReponse_ITEM865(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportChargeStopEvent_ITEM866(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportContinueChargeResponse_ITEM868(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportPowerControlResponse_ITEM8612(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportChargeRecord_ITEM8613(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportQueryChargeRecordRsp_ITEM8616(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportQueryBoardInfoRsp_ITEM872(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportParaSetRsp_ITEM874(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportParaGetRsp_ITEM876(uint8_t port, void *pBuf);
static uint16_t IotXDT_RequestOTAAttribute_ITEM882(uint8_t port, void *pBuf);
static uint16_t IotXDT_ReportFwState_ITEM886(uint8_t port, void *pBuf);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotXDTCtx_Struct *pIotXDTCtx;

static IotXDTSendCtrl_Struct  c_IotXDTSendCtrlTable[] = 
{	
	[0] = {
		.topic = IOT_XDT_TOPIC_PROVISION_REQUEST,
		.cmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_QueryAttachCredential_ITEM811,
		.matchCmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP,
		.cMeaning = "请求获取凭据",
	}, 

	[1] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_TIMESYNC,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = (2 * 60 * 60 * 1000),
		.pSendFunc = IotXDT_ReqeustTimeSync_ITEM821,
		.matchCmd = IOT_XDT_CMD_REQUEST_TIMESYNC_RSP,
		.cMeaning = "请求时间同步",
	},

	[2] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_LINK,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestLink_ITEM823,
		.matchCmd = IOT_XDT_CMD_REQUEST_LINK_RSP,
		.cMeaning = "上线请求",
	},

	[3] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_PILE_STATE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 20 * 60 * 1000,
		.pSendFunc = IotXDT_ReportPileState_ITEM841,
		.matchCmd = IOT_XDT_CMD_PILE_STATE_RSP,
		.cMeaning = "电桩状态",
	},

	[4] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_RATEMODE_SET_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportRateMode_ITEM835,
		.matchCmd = IOT_XDT_CMD_RATEMODE_SET,
		.cMeaning = "费率设置响应",
	},

	[5] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_RATEMODE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestRateMode_ITEM831,
		.matchCmd = IOT_XDT_CMD_REQUEST_RATEMODE_RSP,
		.cMeaning = "请求费率",
	},

	[6] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportRateModeSetResponseEvent_ITEM836,
		.matchCmd = IOT_XDT_CMD_NULL,
		.cMeaning = "费率设置事件",
	},

	[7] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_QUERY_RATEMODE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_QueryRateModeRsp_ITEM837,
		.matchCmd = IOT_XDT_CMD_QUERY_RATEMODE,
		.cMeaning = "查询计费模式响应",
	},

	[8] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_SET_RESTART_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestResetResponse_ITEM826,
		.matchCmd = IOT_XDT_CMD_SET_RESTART,
		.cMeaning = "重启响应",
	},

	[9] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_PARA_SET_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportParaSetRsp_ITEM874,
		.matchCmd = IOT_XDT_PARA_SET,
		.cMeaning = "参数设置响应",
	},	

	[10] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_PARA_QUERY_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportParaGetRsp_ITEM876,
		.matchCmd = IOT_XDT_PARA_QUERY,
		.cMeaning = "查询参数响应",
	},

	[11] = {
		.topic = IOT_XDT_PRE_TOPIC_V2A_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestOTAAttribute_ITEM882,
		.matchCmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP,
		.cMeaning = "升级属性请求",
	},

	[12] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_START_EVNET,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStartEvent_ITEM863,
		.matchCmd = IOT_XDT_CMD_NULL,
		.cMeaning = "充电启动事件",
	},

	[13] = {
		.topic = IOT_XDT_PRE_TOPIC_V2T,
		.cmd = IOT_XDT_CMD_FIRMWARE_STATE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportFwState_ITEM886,
		.matchCmd = IOT_XDT_CMD_NULL,
		.cMeaning = "固件状态",
	},

	[14] = {
		.topic = IOT_XDT_PRE_TOPIC_TSDATA,
		.cmd = IOT_XDT_CMD_PILE_DATA,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPileData_ITEM845,
		.matchCmd = IOT_XDT_CMD_NULL,
		.cMeaning = "电桩数据",
	},

	[15] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_STOP_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStopReponse_ITEM865,
		.matchCmd = IOT_XDT_CHARGE_STOP,
		.cMeaning = "停止充电响应",
	},

	[16] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_STOP_EVNET,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStopEvent_ITEM866,
		.matchCmd = IOT_XDT_CMD_NULL,
		.cMeaning = "停止充电事件",
	},

	[17] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_RECORD,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeRecord_ITEM8613,
		.matchCmd = IOT_XDT_CHARGE_RECORD_RSP,
		.cMeaning = "充电记录",
	},

	[18] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_CHARGE_RECORD_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportQueryChargeRecordRsp_ITEM8616,
		.matchCmd = IOT_XDT_QUERY_CHARGE_RECORD,
		.cMeaning = "查询充电记录响应",
	},		

	[19] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_PWRCTRL_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPowerControlResponse_ITEM8612,
		.matchCmd = IOT_XDT_CHARGE_PWRCTRL,
		.cMeaning = "功率控制响应",
	},		

	[20] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportContinueChargeResponse_ITEM868,
		.matchCmd = IOT_XDT_CHARGE_CONTINUE_CHARGE,
		.cMeaning = "续充命令",
	},	

	[21] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_CARDAUTH,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestAuth_ITEM851,
		.matchCmd = IOT_XDT_CMD_REQUEST_CARDAUTH_RSP,
		.cMeaning = "鉴权请求",
	},	

	[22] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_SET_CATEGORY,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportCategory_ITEM853,
		.matchCmd = IOT_XDT_SET_CATEGORY_RSP,
		.cMeaning = "策略设置",
	},	

	[23] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_BOARDINFO_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportQueryBoardInfoRsp_ITEM872,
		.matchCmd = IOT_XDT_QUERY_BOARDINFO,
		.cMeaning = "控制板信息请求",
	},	

	[24] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_CALL_REALDATA_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_CallRealDataResponse_ITEM847,
		.matchCmd = IOT_XDT_CMD_CALL_REALDATA,
		.cMeaning = "召测实时数据",
		},
 
	[25] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_ERRINFO,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPileErrInfo_ITEM843,
		.matchCmd = IOT_XDT_CMD_ERRINFO_RSP,
		.cMeaning = "故障信息",
		},

	[26] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_START_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStartReponse_ITEM862,
		.matchCmd = IOT_XDT_CHARGE_START,
		.cMeaning = "充电启动响应",
	},
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotXDT_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotXDTCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, cmd);
	uint8_t retFlag = FALSE;

	if (TRUE == sendImmdFlag) 
	{
		retFlag = TRUE;
	}
	else
	{
		if (Common_JudgeTimeoutMs(startTick, sendCyc))
		{
			retFlag = TRUE;
		}
	}
	
	return retFlag;
}


static uint16_t IotXDT_QueryAttachCredential_ITEM811(uint8_t port, void *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
	cJSON *cRoot = NULL;
	char cTempString[32 + 1] = { 0 };
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	snprintf(cTempString, 32, "%s-%s", pPlatInfo->cOperator, pIotXDTCtx->platDn);
	cJSON_AddStringToObject(cRoot, "deviceName", cTempString);
	cJSON_AddStringToObject(cRoot, "productKey", pPlatInfo->cProductKey);
	cJSON_AddStringToObject(cRoot, "productSecret", pPlatInfo->cProductSecret);
	cJSON_AddStringToObject(cRoot, "credentialsType", IOT_XDT_CREDENTIAL_TYPE);
	
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReqeustTimeSync_ITEM821(uint8_t port, void *pBuf)
{
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	
	cJSON_AddStringToObject(cRoot, "method", "sync_time");
	
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddItemToObject(cRoot,"params",params);
	
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_RequestLink_ITEM823(uint8_t port, void *pBuf)
{
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	
	cJSON_AddStringToObject(cRoot, "method", "link_req");
	
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);
	
	cJSON_AddStringToObject(params, "snHard", pIotXDTCtx->platDn);
	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "type", 1);
	cJSON_AddItemToObject(cRoot,"params",params);
	
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_RequestRateMode_ITEM831(uint8_t port, void *pBuf)
{
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	
	cJSON_AddStringToObject(cRoot, "method", "rate_req");
	
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);
	
	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "type", 0);
	cJSON_AddItemToObject(cRoot,"params",params);
	
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportPileState_ITEM841(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	char cTempString[21] = { 0 };
	cJSON *gun[SYSCFG_CFG_GUN_NUM] = { 0 };
	cJSON *gunArray = NULL;
	cJSON *params = NULL;
	uint8_t index = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "method", "upload_status_pile");

	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);
	cJSON_AddItemToObject(cRoot,"params",params);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "statusPile", IotXDT_GetPileStatus());  
	cJSON_AddNumberToObject(params, "cntReset", pPlatInfo->resetCount);

	CddNetM_GetIccid((uint8_t *)cTempString);
	cJSON_AddStringToObject(params, "ICCID", cTempString);

	cJSON_AddNumberToObject(params, "csq", CddNetM_GetCsq());
	cJSON_AddNumberToObject(params, "ts", SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT);

	gunArray = cJSON_CreateArray();
	IOT_XDT_CheckObjIsNull(gunArray, 0);

	for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)    
	{ 
		gun[index] = cJSON_CreateObject();
		IOT_XDT_CheckObjIsNull(gun[index], 0);

		cJSON_AddNumberToObject(gun[index], "no", index + 1);   
		cJSON_AddNumberToObject(gun[index], "status", IotXDT_GetGunStatus(index)); 
		cJSON_AddNumberToObject(gun[index], "lockGround", 0); 
		cJSON_AddNumberToObject(gun[index], "lockGun", 0);
		cJSON_AddNumberToObject(gun[index], "mountGun", 0);
		cJSON_AddNumberToObject(gun[index], "K1K2", AswChargeIf_GetRelayState(index));
		cJSON_AddNumberToObject(gun[index], "S2", AswChargeIf_CheckS2Closed(index));
		cJSON_AddNumberToObject(gun[index], "detectVol", AswChargeIf_GetCpVoltage(index) / 1000.0);
		cJSON_AddNumberToObject(gun[index], "detectPWM", AswChargeIf_GetCpDuty(index) / 1000.0);
		cJSON_AddItemToArray(gunArray, gun[index]);
	}

	cJSON_AddItemToObject(params,"gun",gunArray);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportPileErrInfo_ITEM843(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTErrDesc_Struct *pErrDesc = NULL;
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	char cTempString[64] = { 0 };
	cJSON *alarmsArray = NULL;
	cJSON *alarms[IOT_XDT_SINGLE_FRAME_MAX_ERROR_COUNT] = { 0 };
	cJSON *params = NULL;
	uint8_t index = 0, temp = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "method", "upload_fault_pile");

	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);
	cJSON_AddItemToObject(cRoot,"params",params);
	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "ts", SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT);
	cJSON_AddNumberToObject(params, "gunNo", port + 1);

	alarmsArray = cJSON_CreateArray();
	IOT_XDT_CheckObjIsNull(alarmsArray, 0);
	memset(cTempString, 0x00, sizeof(cTempString));

	if (pRecvDataInfo->offlineClearData.errClearInfoReportFlag == TRUE)
	{
		for (index = 0; index < IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE; index++)
		{
			if (pRecvDataInfo->offlineClearData.errInfoReportQueue[index].eReportState != eXDTReportState_Null)
			{
				pErrDesc = (IotXDTErrDesc_Struct *)pRecvDataInfo->offlineClearData.errInfoReportQueue[index].p;

				alarms[temp] = cJSON_CreateObject();
				IOT_XDT_CheckObjIsNull(alarms[temp], 0);
				cJSON_AddNumberToObject(alarms[temp], "level", pErrDesc->eLevel);
				snprintf(cTempString, sizeof(cTempString), "GN_00%02d", pErrDesc->errNo);
				cJSON_AddStringToObject(alarms[temp], "code", cTempString);
				cJSON_AddStringToObject(alarms[temp], "text", pErrDesc->alarmDesc);
				cJSON_AddStringToObject(alarms[temp], "explain", pErrDesc->alarmDesc);
				cJSON_AddNumberToObject(alarms[temp], "status", pErrDesc->lastStatus[port]);   
				cJSON_AddItemToArray(alarmsArray, alarms[temp]);
				pRecvDataInfo->offlineClearData.errInfoReportQueue[index].eReportState = eXDTReportState_Reporting;
				temp++;
				if (temp >= IOT_XDT_SINGLE_FRAME_MAX_ERROR_COUNT)
				{
					break;
				}
			}
		}
	}

	cJSON_AddItemToObject(params,"alarms",alarmsArray);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static void IotXDT_ConvertRateModeToXDT(IotXDTRateMode_Struct *pXdtRateMode, MSNvmXDTParamBillMode_Struct *pGnRateMode)
{
	uint8_t index = 0;
	uint8_t startIndex = 0xFF, stopIndex = 0xFF;
	uint8_t startFlag = 0xFF, stopFlag = 0xFF;
	uint8_t tempTimeH = 0, tempTimeL = 0,temp = 0;
	IotXDTPeriodList_Struct *pPeriodList = NULL;
	double tempElec = 0, tempSer = 0;

	memcpy(pXdtRateMode->billModeID, pGnRateMode->billModeID, sizeof(pXdtRateMode->billModeID));
	pXdtRateMode->typeRule = pGnRateMode->typeRule;

	snprintf(pXdtRateMode->stdElec, sizeof(pXdtRateMode->stdElec), "%g", 
		(double)Common_FourUint8ToUint32(pGnRateMode->std_ele_fee) / 100000.0);

	snprintf(pXdtRateMode->stdSer, sizeof(pXdtRateMode->stdSer), "%g", 
		(double)Common_FourUint8ToUint32(pGnRateMode->std_ser_fee) / 100000.0);

	if (pXdtRateMode->typeRule == 1)
	{
		for (index = 0; index < pGnRateMode->period_count; index++)
		{
			if (pGnRateMode->period[index].validFlag == TRUE)
			{
				pPeriodList = &pXdtRateMode->periodListInfo[index];

				tempTimeH = pGnRateMode->period[index].startTime / 2;
				tempTimeL = (pGnRateMode->period[index].startTime % 2) * 30;
				snprintf(pPeriodList->begin, sizeof(pPeriodList->begin), "%02d:%02d", tempTimeH, tempTimeL);
				tempTimeH = pGnRateMode->period[index].stopTime / 2;
				tempTimeL = (pGnRateMode->period[index].stopTime % 2) * 30;
				snprintf(pPeriodList->end, sizeof(pPeriodList->end), "%02d:%02d", tempTimeH, tempTimeL);
				snprintf(pPeriodList->flag, sizeof(pPeriodList->flag), "%d", pGnRateMode->period[index].flag + 1);

				switch (pGnRateMode->period[index].flag + 1)
				{
					case 1:
					{
						tempElec = Common_FourUint8ToUint32(pGnRateMode->sharp_ele_fee) / 100000.0;
						tempSer =  Common_FourUint8ToUint32(pGnRateMode->sharp_ser_fee) / 100000.0;
						break;
					}
					case 2:
					{
						tempElec = Common_FourUint8ToUint32(pGnRateMode->peak_ele_fee) / 100000.0;
						tempSer =  Common_FourUint8ToUint32(pGnRateMode->peak_ser_fee) / 100000.0;
						break;
					}
					case 3:
					{
						tempElec = Common_FourUint8ToUint32(pGnRateMode->flat_ele_fee) / 100000.0;
						tempSer =  Common_FourUint8ToUint32(pGnRateMode->flat_ser_fee) / 100000.0;
						break;
					}

					case 4:
					{
						tempElec = Common_FourUint8ToUint32(pGnRateMode->valley_ele_fee) / 100000.0;
						tempSer =  Common_FourUint8ToUint32(pGnRateMode->valley_ser_fee) / 100000.0;
						break;
					}
					case 5:
					{
						tempElec = Common_FourUint8ToUint32(pGnRateMode->deep_ele_fee) / 100000.0;
						tempSer =  Common_FourUint8ToUint32(pGnRateMode->deep_ser_fee) / 100000.0;
						break;
					}
					default:
					{
						break;
					}
				}

				snprintf(pPeriodList->elec, sizeof(pPeriodList->elec), "%g", tempElec);
				snprintf(pPeriodList->ser, sizeof(pPeriodList->ser), "%g", tempSer);	
			}
		}

		pXdtRateMode->periodCnt = pGnRateMode->period_count;
	}
}

static uint16_t IotXDT_ReportRateMode_ITEM835(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pBillMode = &pPrivateParam->stXDTParam.stBillMode;
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTPeriodList_Struct *pPeriodList = NULL;
	IotXDTRateMode_Struct strRateMode = { 0 };
	cJSON *cRoot = NULL;
	cJSON *periodsList = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	cJSON *period[IOT_XDT_RATE_MODE_MAX_PERIOD] = { 0 };

	IotXDT_ConvertRateModeToXDT(&strRateMode, &pRecvDataInfo->offlineClearData.strGnRate_Item834);
	
	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "ans", pRecvDataInfo->offlineClearData.eAns_Item834);
	cJSON_AddNumberToObject(cRoot, "type", pRecvDataInfo->offlineClearData.type_ITEM834);
	cJSON_AddNumberToObject(cRoot, "id", Common_FourUint8ToUint32(strRateMode.billModeID));
	cJSON_AddNumberToObject(cRoot, "typeRule", strRateMode.typeRule);
	cJSON_AddStringToObject(cRoot, "elec", strRateMode.stdElec);
	cJSON_AddStringToObject(cRoot, "ser", strRateMode.stdSer);

	if (strRateMode.typeRule == 1)
	{
		periodsList = cJSON_CreateArray();
		IOT_XDT_CheckObjIsNull(periodsList, 0);
		
		for (index = 0; index < strRateMode.periodCnt; index++)    
		{
			pPeriodList = &strRateMode.periodListInfo[index];
			period[index] = cJSON_CreateObject();
			IOT_XDT_CheckObjIsNull(period[index], 0);
			cJSON_AddNumberToObject(period[index], "sn", index + 1);    
			cJSON_AddStringToObject(period[index], "begin", pPeriodList->begin);  
			cJSON_AddStringToObject(period[index], "end", pPeriodList->end);   
			cJSON_AddStringToObject(period[index], "flag", pPeriodList->flag);     
			cJSON_AddStringToObject(period[index], "elec", pPeriodList->elec);  
			cJSON_AddStringToObject(period[index], "ser", pPeriodList->ser);   
			cJSON_AddItemToArray(periodsList, period[index]);
		}
		
		cJSON_AddItemToObject(cRoot,"periodsList",periodsList);
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);

	if (pRecvDataInfo->offlineClearData.eAns_Item834 == eIotXDTErrCode_Success)
	{
		Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT, TRUE);
		Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT, TRUE);
	}

	return dataLen;
}

static uint16_t IotXDT_ReportRateModeSetResponseEvent_ITEM836(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTPeriodList_Struct *pPeriodList = NULL;
	IotXDTRateMode_Struct strRateMode = { 0 };
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	cJSON *periodsList = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	cJSON *period[IOT_XDT_RATE_MODE_MAX_PERIOD] = { 0 };

	IotXDT_ConvertRateModeToXDT(&strRateMode, &pRecvDataInfo->offlineClearData.strGnRate_Item834);
	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	
	cJSON_AddStringToObject(cRoot, "method", "event_rate_set");
	
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);
	cJSON_AddItemToObject(cRoot,"params",params);
	
	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "ans", 0);
	cJSON_AddNumberToObject(params, "type", 0);   
	cJSON_AddNumberToObject(params, "id", Common_FourUint8ToUint32(strRateMode.billModeID)); 
	cJSON_AddNumberToObject(params, "typeRule", strRateMode.typeRule);  
	cJSON_AddStringToObject(params, "elec", strRateMode.stdElec);
	cJSON_AddStringToObject(params, "ser", strRateMode.stdSer);

	if (strRateMode.typeRule == 1)
	{
		periodsList = cJSON_CreateArray();
		IOT_XDT_CheckObjIsNull(periodsList, 0);
		
		for (index = 0; index < strRateMode.periodCnt; index++)    
		{
			pPeriodList = &strRateMode.periodListInfo[index];
			period[index] = cJSON_CreateObject();
			IOT_XDT_CheckObjIsNull(period[index], 0);
			cJSON_AddNumberToObject(period[index], "sn", index + 1);    
			cJSON_AddStringToObject(period[index], "begin", pPeriodList->begin);  
			cJSON_AddStringToObject(period[index], "end", pPeriodList->end);   
			cJSON_AddStringToObject(period[index], "flag", pPeriodList->flag);     
			cJSON_AddStringToObject(period[index], "elec", pPeriodList->elec);  
			cJSON_AddStringToObject(period[index], "ser", pPeriodList->ser);   
			cJSON_AddItemToArray(periodsList, period[index]);
		}
		
		cJSON_AddItemToObject(params,"periodsList",periodsList);
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_QueryRateModeRsp_ITEM837(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pBillMode = &pPrivateParam->stXDTParam.stBillMode;
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTPeriodList_Struct *pPeriodList = NULL;
	IotXDTRateMode_Struct strRateMode = { 0 };
	cJSON *cRoot = NULL;
	cJSON *periodsList = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	cJSON *period[IOT_XDT_RATE_MODE_MAX_PERIOD] = { 0 };

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "ans", pRecvDataInfo->offlineClearData.eAns_Item833);
	cJSON_AddNumberToObject(cRoot, "type", pRecvDataInfo->offlineClearData.type_ITEM833);
   
	if (pRecvDataInfo->offlineClearData.type_ITEM833 != 0)
	{
		cJSON_AddNumberToObject(cRoot, "id", 0); 
		cJSON_AddNumberToObject(cRoot, "typeRule", 0);
		cJSON_AddStringToObject(cRoot, "elec", "");
		cJSON_AddStringToObject(cRoot, "ser", "");
	}
	else
	{
		IotXDT_ConvertRateModeToXDT(&strRateMode, pBillMode);
		cJSON_AddNumberToObject(cRoot, "id", Common_FourUint8ToUint32(strRateMode.billModeID));
		cJSON_AddNumberToObject(cRoot, "typeRule", strRateMode.typeRule);  
		cJSON_AddStringToObject(cRoot, "elec", strRateMode.stdElec);
		cJSON_AddStringToObject(cRoot, "ser", strRateMode.stdSer);

		if (strRateMode.typeRule == 1)
		{
			periodsList = cJSON_CreateArray();
			IOT_XDT_CheckObjIsNull(periodsList, 0);
			
			for (index = 0; index < strRateMode.periodCnt; index++)    
			{
				pPeriodList = &strRateMode.periodListInfo[index];
				period[index] = cJSON_CreateObject();
				IOT_XDT_CheckObjIsNull(period[index], 0);
				cJSON_AddNumberToObject(period[index], "sn", index + 1);    
				cJSON_AddStringToObject(period[index], "begin", pPeriodList->begin);  
				cJSON_AddStringToObject(period[index], "end", pPeriodList->end);   
				cJSON_AddStringToObject(period[index], "flag", pPeriodList->flag);     
				cJSON_AddStringToObject(period[index], "elec", pPeriodList->elec);  
				cJSON_AddStringToObject(period[index], "ser", pPeriodList->ser);   
				cJSON_AddItemToArray(periodsList, period[index]);
			}
			
			cJSON_AddItemToObject(cRoot,"periodsList",periodsList);
		}
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_RequestResetResponse_ITEM826(uint8_t port, void *pBuf)
{
	IotXDTProtoData_Struct *pProtoData = &pIotXDTCtx->stProtoData;
	IotXDTRecvData_Struct *pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	
	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn); 

	if (pRecvData->offlineClearData.force_Item825 == 1)
	{
		cJSON_AddNumberToObject(cRoot, "statusCmd", eIotXDTErrCode_Success);

		for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)
		{
			if (IotXDT_GetGunStatus(index) == eIotXDTGunStatus_Charging)
			{
				AswErrhandle_SetErrExsitCallback(index, eSrc_AppStop);
			}
		}

		if (pProtoData->rebootFlag != TRUE)
		{
			pProtoData->rebootFlag = TRUE;
			pProtoData->rebootTick = Common_GetSystick();
		}
	}
	else
	{
		if (TRUE != IotXDT_IsPileOnCharging())
		{
			cJSON_AddNumberToObject(cRoot, "statusCmd", eIotXDTErrCode_Success);
			
			if (pProtoData->rebootFlag != TRUE)
			{
				pProtoData->rebootFlag = TRUE;
				pProtoData->rebootTick = Common_GetSystick();
			}
		}
		else
		{
			cJSON_AddNumberToObject(cRoot, "statusCmd", eIotXDTErrCode_OnCharging);
		}
	}

	cJSON_AddNumberToObject(cRoot, "statusCharge", IotXDT_IsPileOnCharging());
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportParaSetRsp_ITEM874(uint8_t port, void *pBuf)
{
	IOTXDTParamOpt_Struct *pParamOpt = pIotXDTCtx->stProtoData.stRecvData[0].offlineClearData.paramOpt;
	IOTXDTParamOpt_Struct *pTempParamOpt = NULL;
	cJSON *cRoot, *cParamX;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	uint16_t tempVal = 0;
	double tempDouble = 0;
	uint8_t atLeastParaValid = FALSE;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);

	for (index = 0; index < eIotXDTParamType_Count; index++)
	{
		pTempParamOpt = &pParamOpt[index];

		if (pTempParamOpt->optFlag == TRUE)
		{
			atLeastParaValid = TRUE;
			cParamX = cJSON_CreateObject();
			IOT_XDT_CheckObjIsNull(cParamX, 0);
			
			switch (pTempParamOpt->eParaType)
			{
			case eIotXDTParamType_t1:
			{
				cJSON_AddStringToObject(cParamX, "value" , pTempParamOpt->paramString);
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
			case eIotXDTParamType_t2:
			{
				tempVal = atoi(pTempParamOpt->paramString);
				cJSON_AddNumberToObject(cParamX, "value" , tempVal);
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
			case eIotXDTParamType_t18:
			{
				cJSON_AddStringToObject(cParamX, "value" , pTempParamOpt->paramString);
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
			case eIotXDTParamType_t40:
			{
				if (strcmp(pTempParamOpt->paramString, "true") == 0)
				{
					cJSON_AddBoolToObject(cParamX, "value" , TRUE);
				}
				else
				{
					cJSON_AddBoolToObject(cParamX, "value" , FALSE);
				}
				
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
		    case eIotXDTParamType_t41:
			{
				tempVal = atoi(pTempParamOpt->paramString);
				cJSON_AddNumberToObject(cParamX, "value" , tempVal);
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
			case eIotXDTParamType_t42:
			{
				tempDouble = atof(pTempParamOpt->paramString);
				cJSON_AddNumberToObject(cParamX, "value" , tempDouble);
				cJSON_AddNumberToObject(cParamX, "status", pTempParamOpt->eAns);
				break;
			}
			default:
			break;
			}

			cJSON_AddItemToObject(cRoot, pTempParamOpt->keyName, cParamX);
		}
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);

	dataLen = (atLeastParaValid == TRUE) ? dataLen : 0;
	return dataLen;
}

static uint16_t IotXDT_ReportParaGetRsp_ITEM876(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
	IOTXDTParamOpt_Struct *pParamOpt = pIotXDTCtx->stProtoData.stRecvData[0].offlineClearData.paramOpt;
	IOTXDTParamOpt_Struct *pTempParamOpt = NULL;
	cJSON *cRoot, *cParamX;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	uint16_t tempVal = 0;
	float tempDouble = 0;
	char tempString[IOT_XDT_PARAM_MAX_LEN + 1] = { 0 };
	uint8_t atLeastParaValid = FALSE;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);

	for (index = 0; index < eIotXDTParamType_Count; index++)
	{
		pTempParamOpt = &pParamOpt[index];

		if (pTempParamOpt->optFlag == TRUE)
		{
			memset(tempString, 0x00, sizeof(tempString));
			cParamX = cJSON_CreateObject();
			IOT_XDT_CheckObjIsNull(cParamX, 0);
			atLeastParaValid = TRUE;

			switch (pTempParamOpt->eParaType)
			{
			case eIotXDTParamType_t1:
			{
				memcpy(tempString, pIotXDTCtx->stProtoData.mainIp, IOT_XDT_PARAM_MAX_LEN);
				cJSON_AddStringToObject(cParamX, "value" , tempString);
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
			case eIotXDTParamType_t2:
			{
				tempVal = atoi(pIotXDTCtx->stProtoData.mainPort);
				cJSON_AddNumberToObject(cParamX, "value" , tempVal);
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
			case eIotXDTParamType_t18:
			{
				CddNetM_GetIccid((uint8_t *)tempString);
				cJSON_AddStringToObject(cParamX, "value" , tempString);
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
			case eIotXDTParamType_t40:
			{
				tempVal = pPlatInfo->pileDataCycleReportEnable;
				
				if (tempVal)
				{
					cJSON_AddBoolToObject(cParamX, "value" , TRUE);
				}
				else
				{
					cJSON_AddBoolToObject(cParamX, "value" , FALSE);
				}
				
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
		    case eIotXDTParamType_t41:
			{
				tempVal = pPlatInfo->pileDataReportCycle;
				cJSON_AddNumberToObject(cParamX, "value" , tempVal);
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
			case eIotXDTParamType_t42:
			{
				memcpy(&tempDouble, &pPlatInfo->amountChangeThreshold, 4);
				cJSON_AddNumberToObject(cParamX, "value" , round(tempDouble * 10000) / 10000.0);
				cJSON_AddNumberToObject(cParamX, "status", eIotXDTErrCode_Success);
				break;
			}
			default:
			break;
			}

			cJSON_AddItemToObject(cRoot, pTempParamOpt->keyName, cParamX);
		}
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	dataLen = (atLeastParaValid == TRUE) ? dataLen : 0;
	return dataLen;
}

static uint16_t IotXDT_ReportPileData_ITEM845(uint8_t port, void *pBuf)
{
	MSNvmOrderInfo_Struct *pOrder = AswMonitor_GerOrderDataPtr(port);
	MSNvmXDTOrderInfo_Struct *pOrderRecord = &pOrder->platOrderInfo.stXDTOrderInfo;
	AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
	cJSON *cRoot = NULL, *pile = NULL, *gunArray = NULL, *gun = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	uint32_t tempI, tempU;
	double tempP;
	char *pJson = NULL;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "ts",  SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT);

	pile = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(pile, 0);

	cJSON_AddNumberToObject(pile, "runtime", pIotXDTCtx->stProtoData.runTime);
	cJSON_AddNumberToObject(pile, "cTime", pIotXDTCtx->stProtoData.totalChargeTimeSec);
	cJSON_AddNumberToObject(pile, "cEnergy", pIotXDTCtx->stProtoData.totalChargeEnergy / 10000.0);
	cJSON_AddNumberToObject(pile, "cCount", pIotXDTCtx->stProtoData.totalChargeTimes);
	cJSON_AddNumberToObject(pile, "dTime", 0);
	cJSON_AddNumberToObject(pile, "dEnergy", 0.00);
	cJSON_AddNumberToObject(pile, "dCount", 0);
	cJSON_AddNumberToObject(pile, "tIn", ((int16_t)AswChargeIf_GetEnvTemperature()  - 50) / 1.0);
	cJSON_AddNumberToObject(pile, "tPcu", 0);
	cJSON_AddNumberToObject(pile, "tMaxIndex", 0);
	cJSON_AddItemToObject(cRoot, "pile", pile);

	gunArray = cJSON_CreateArray();
	IOT_XDT_CheckObjIsNull(gunArray, 0);
	gun = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(gun, 0);

	cJSON_AddNumberToObject(gun, "no", port + 1);
	cJSON_AddNumberToObject(gun, "type", 0);
	cJSON_AddNumberToObject(gun, "temp", ((int16_t)AswChargeIf_GetGunTemperature(port)  - 50) / 1.0);
	cJSON_AddStringToObject(gun, "orderNo", pOrderRecord->orderNo);
	cJSON_AddNumberToObject(gun, "voltA", AswChargeIf_GetInputVoltage(port) / 100.0); 
	cJSON_AddNumberToObject(gun, "voltB", 0);
	cJSON_AddNumberToObject(gun, "voltC", 0);
	tempI = AswChargeIf_GetOutputCurrent(port);
	cJSON_AddNumberToObject(gun, "curA",  tempI / 1000.0);
	cJSON_AddNumberToObject(gun, "curB",  0);
	cJSON_AddNumberToObject(gun, "curC",  0);
	tempU = AswChargeIf_GetOutputVoltage(port);
	cJSON_AddNumberToObject(gun, "outU",  tempU / 100.0);
	cJSON_AddNumberToObject(gun, "outI",  tempI / 1000.0);
	tempP = round(((double)(tempU * tempI)) / 1000) / 100000.0 ;
	cJSON_AddNumberToObject(gun, "outP",  tempP);
	cJSON_AddNumberToObject(gun, "pq",  pChargeData->totalLossEnergy / 10000.0);

	cJSON_AddNumberToObject(gun, "elecAmt",  pChargeData->totalElecMoney / 10000.0);
	cJSON_AddNumberToObject(gun, "serAmt",  pChargeData->totalServeMoney / 10000.0);
	cJSON_AddNumberToObject(gun, "amt",  pChargeData->totalMoney / 10000.0);
	cJSON_AddNumberToObject(gun, "chDu",  pChargeData->chargeTime);
	cJSON_AddNumberToObject(gun, "surDu", 0);
	cJSON_AddNumberToObject(gun, "sdU",   0);
	cJSON_AddNumberToObject(gun, "sdI",  0);
	cJSON_AddNumberToObject(gun, "soc",  0);
	cJSON_AddItemToArray(gunArray, gun);
	
	cJSON_AddItemToObject(cRoot,"gun",gunArray);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_CallRealDataResponse_ITEM847(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "type", pRecvData->offlineClearData.type_ITEM846);
	cJSON_AddNumberToObject(cRoot, "mode", pRecvData->offlineClearData.mode_ITEM846);
	cJSON_AddNumberToObject(cRoot, "status", pRecvData->offlineClearData.eAns_ITEM846);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_RequestAuth_ITEM851(uint8_t port, void *pBuf)
{
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	char tempString[IOT_XDT_USERNUM_LEN + 1] = { 0 };
	AswMonitorChargeCtrl_Struct *pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "method", "auth_req");

	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "gunNo", port + 1);
	cJSON_AddNumberToObject(params, "initiator", 3);
	sprintf(tempString, "%02X%02X%02X%02X", pChargeCtrl->authCardID[0], pChargeCtrl->authCardID[1], 
		pChargeCtrl->authCardID[2], pChargeCtrl->authCardID[3]);
	cJSON_AddStringToObject(params, "user", tempString);
	cJSON_AddStringToObject(params, "passwd", "");
	cJSON_AddItemToObject(cRoot, "params", params);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportCategory_ITEM853(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
	AswMonitorChargeCtrl_Struct *pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
	char tempString[IOT_XDT_USERNUM_LEN + 1] = { 0 };
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	cJSON *plan = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "method", "plan_set");

	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "gunNo", port + 1);
	cJSON_AddNumberToObject(params, "type", 0);
	cJSON_AddNumberToObject(params, "initiator", 3);
	sprintf(tempString, "%02X%02X%02X%02X", pChargeCtrl->authCardID[0], pChargeCtrl->authCardID[1], 
		pChargeCtrl->authCardID[2], pChargeCtrl->authCardID[3]);
	cJSON_AddStringToObject(params, "user", tempString);
	plan = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(plan, 0);
	cJSON_AddNumberToObject(plan, "typePlan", 2);
	cJSON_AddNumberToObject(plan, "typeStart", 0);
	cJSON_AddNumberToObject(plan, "tsStart", SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT);
	cJSON_AddNumberToObject(plan, "value", pRecvData->offlineClearData.balance_ITEM852);
	cJSON_AddItemToObject(params, "plan", plan);
	cJSON_AddItemToObject(cRoot, "params", params);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportChargeStartReponse_ITEM862(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "gunNo", pRecvDataInfo->offlineClearData.gunNo_ITEM861);
	cJSON_AddNumberToObject(cRoot, "type", pRecvDataInfo->offlineClearData.type_ITEM861);
	cJSON_AddNumberToObject(cRoot, "status", pRecvDataInfo->offlineClearData.eAns_ITEM861);
	cJSON_AddStringToObject(cRoot, "fDetail", pRecvDataInfo->offlineClearData.fDetail_ITEM861);
	cJSON_AddStringToObject(cRoot, "orderNo", pRecvDataInfo->offlineClearData.orderNo_ITEM861);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);

	Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CHARGE_START_EVNET, TRUE);
	Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CHARGE_START_EVNET, TRUE);
	return dataLen;
}

static uint16_t IotXDT_ReportChargeStartEvent_ITEM863(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	cJSON *params = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	cJSON_AddStringToObject(cRoot, "method", "event_start");
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "gunNo", pRecvDataInfo->offlineClearData.gunNo_ITEM861);
	cJSON_AddNumberToObject(params, "type",  pRecvDataInfo->offlineClearData.type_ITEM861);

	if (pRecvDataInfo->offlineClearData.eAns_ITEM861 == eIotXDTErrCode_Success)
	{
		cJSON_AddNumberToObject(params, "status",  0);
	}
	else
	{
		cJSON_AddNumberToObject(params, "status",  1);
	}

	cJSON_AddStringToObject(params, "fDetail", pRecvDataInfo->offlineClearData.fDetail_ITEM861);
	cJSON_AddStringToObject(params, "orderNo", pRecvDataInfo->offlineClearData.orderNo_ITEM861);
	cJSON_AddNumberToObject(params, "tsStart",  SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT);
	cJSON_AddItemToObject(cRoot, "params", params);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportChargeStopReponse_ITEM865(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "gunNo", pRecvDataInfo->offlineClearData.gunNo_ITEM864);
	cJSON_AddNumberToObject(cRoot, "type",  0);
	cJSON_AddNumberToObject(cRoot, "status", pRecvDataInfo->offlineClearData.eAns_ITEM864);
	cJSON_AddStringToObject(cRoot, "fDetail", pRecvDataInfo->offlineClearData.fDetail_ITEM865);
	cJSON_AddStringToObject(cRoot, "orderNo", pRecvDataInfo->offlineClearData.orderNo_ITEM864);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportChargeStopEvent_ITEM866(uint8_t port, void *pBuf)
{
	MSNvmOrderInfo_Struct *pOrder = AswMonitor_GerOrderDataPtr(port);
	MSNvmXDTOrderInfo_Struct *pOrderRecord = &pOrder->platOrderInfo.stXDTOrderInfo;
	cJSON *params = NULL;
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	cJSON_AddStringToObject(cRoot, "method", "event_stop");
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(params, "gunNo",   port + 1);
	cJSON_AddNumberToObject(params, "fDetail", pOrderRecord->stopReason);
	cJSON_AddStringToObject(params, "orderNo", pOrderRecord->orderNo);
	cJSON_AddNumberToObject(params, "tsStart", Common_FourUint8ToUint32(pOrderRecord->endTs));
	cJSON_AddItemToObject(cRoot, "params", params);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportContinueChargeResponse_ITEM868(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
	MSNvmOrderInfo_Struct *pOrder = AswMonitor_GerOrderDataPtr(port);
	MSNvmXDTOrderInfo_Struct *pOrderRecord = &pOrder->platOrderInfo.stXDTOrderInfo;
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "gunNo", pRecvData->offlineClearData.gunNo_ITEM867);
	cJSON_AddNumberToObject(cRoot, "status", pRecvData->offlineClearData.eAns_ITEM867);
	cJSON_AddNumberToObject(cRoot, "initiator", pRecvData->offlineClearData.initiator_ITEM867);
	cJSON_AddNumberToObject(cRoot, "typePlan", pRecvData->offlineClearData.typePlan_ITEM867);

	if (pRecvData->offlineClearData.eAns_ITEM867 == eIotXDTErrCode_Success)
	{
		cJSON_AddNumberToObject(cRoot, "value", Common_FourUint8ToUint32(pOrderRecord->value) / 10000.0);
	}
	else
	{
		cJSON_AddNumberToObject(cRoot, "value", pRecvData->offlineClearData.value_ITEM867);
	}
		
	cJSON_AddStringToObject(cRoot, "user", pRecvData->offlineClearData.user_ITEM867);
	cJSON_AddStringToObject(cRoot, "orderNo", pRecvData->offlineClearData.orderNo_ITEM867);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportPowerControlResponse_ITEM8612(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "gunNo", pRecvDataInfo->offlineClearData.gunNo_ITEM8611);
	cJSON_AddNumberToObject(cRoot, "status", pRecvDataInfo->offlineClearData.eAns_ITEM8611);
	cJSON_AddNumberToObject(cRoot, "type", pRecvDataInfo->offlineClearData.type_ITEM8611);

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportChargeRecord_ITEM8613(uint8_t port, void *pBuf)
{
	MSNvmXDTOrderInfo_Struct *pRecord = &pIotXDTCtx->stOrderInfo.platOrderInfo.stXDTOrderInfo;
	cJSON *cRoot, *params, *plan, *periodsList;
	MSNvmXDTPeriodInfo_Struct *pPeriodInfo = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	cJSON *period[IOT_XDT_RATE_MODE_MAX_PERIOD] = { 0 };

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	cJSON_AddStringToObject(cRoot, "method", "record_upload");
	params = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(params, 0);

	cJSON_AddStringToObject(params, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddStringToObject(params, "orderNo", pRecord->orderNo);
	cJSON_AddNumberToObject(params, "gunNo", port + 1);
	cJSON_AddNumberToObject(params, "ts", Common_FourUint8ToUint32(pRecord->ts));
	cJSON_AddNumberToObject(params, "indexRec", Common_FourUint8ToUint32(pRecord->indexRec));
	cJSON_AddNumberToObject(params, "typeRec", pRecord->typeRec);
	cJSON_AddNumberToObject(params, "type", pRecord->type);	
	cJSON_AddNumberToObject(params, "initiator", pRecord->initiator);
	cJSON_AddStringToObject(params, "user", pRecord->user);

	plan = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(plan, 0);
	cJSON_AddNumberToObject(plan, "typePlan", pRecord->typePlan);
	cJSON_AddNumberToObject(plan, "typeStart", pRecord->typeStart);
	cJSON_AddNumberToObject(plan, "tsStart", Common_FourUint8ToUint32(pRecord->tsStart));
	cJSON_AddNumberToObject(plan, "value", Common_FourUint8ToUint32(pRecord->value) / 10000.0);
	cJSON_AddItemToObject(params, "plan", plan);
	
	cJSON_AddNumberToObject(params, "pricingId", Common_FourUint8ToUint32(pRecord->pricingID));
	cJSON_AddNumberToObject(params, "beginTs", Common_FourUint8ToUint32(pRecord->beginTs));
	cJSON_AddNumberToObject(params, "endTs", Common_FourUint8ToUint32(pRecord->endTs));	
	cJSON_AddNumberToObject(params, "beginMr", round(Common_FourUint8ToUint32(pRecord->beginMr) / 10.0) / 1000.0);
	cJSON_AddNumberToObject(params, "endMr", round(Common_FourUint8ToUint32(pRecord->endMr) / 10.0) / 1000.0);
	cJSON_AddNumberToObject(params, "tPq", Common_FourUint8ToUint32(pRecord->tPq) / 10000.0);
	cJSON_AddNumberToObject(params, "elecAmt", Common_FourUint8ToUint32(pRecord->elecAmt) / 10000.0);
	cJSON_AddNumberToObject(params, "serAmt", Common_FourUint8ToUint32(pRecord->serMt) / 10000.0);
	cJSON_AddNumberToObject(params, "amt", Common_FourUint8ToUint32(pRecord->amt) / 10000.0);
	cJSON_AddNumberToObject(params, "beginSoc", 0);
	cJSON_AddNumberToObject(params, "endSoc", 0);
	cJSON_AddNumberToObject(params, "stopReason", pRecord->stopReason);
	cJSON_AddNumberToObject(params, "typeRule", pRecord->typeRule);
	cJSON_AddNumberToObject(params, "pq_total", Common_FourUint8ToUint32(pRecord->pqTotal) / 10000.0);

	if (pRecord->typeRule == 1)
	{
		periodsList = cJSON_CreateArray();
		IOT_XDT_CheckObjIsNull(periodsList, 0);
		
		for (index = 0; index < IOT_XDT_RATE_MODE_MAX_PERIOD; index++)
		{
			pPeriodInfo = &pRecord->periodInfoArray[index];

			if (pPeriodInfo->valid == TRUE)
			{
				period[index] = cJSON_CreateObject();
				IOT_XDT_CheckObjIsNull(period[index], 0);
				cJSON_AddNumberToObject(period[index], "sn", pPeriodInfo->sn);
				cJSON_AddNumberToObject(period[index], "pq",Common_FourUint8ToUint32(pPeriodInfo->pq) / 10000.0);
				cJSON_AddItemToArray(periodsList, period[index]);
			}
		}
						
		cJSON_AddItemToObject(params,"periodsList",periodsList);
	}
	
	cJSON_AddItemToObject(cRoot, "params", params);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportQueryChargeRecordRsp_ITEM8616(uint8_t port, void *pBuf)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	MSNvmXDTOrderInfo_Struct *pRecord = &pRecvDataInfo->offlineClearData.queryChargeRecord.platOrderInfo.stXDTOrderInfo;
	MSNvmXDTPeriodInfo_Struct *pPeriodInfo = NULL;
	cJSON *cRoot, *plan, *periodsList;
	char *pJson = NULL;
	uint16_t dataLen = 0;
	uint8_t index = 0;
	cJSON *period[IOT_XDT_RATE_MODE_MAX_PERIOD] = { 0 };

	if (pRecvDataInfo->offlineClearData.eAns_ITEM8615 == eIotXDTErrCode_Success)
	{
		cRoot = cJSON_CreateObject();
		IOT_XDT_CheckObjIsNull(cRoot, 0);
		
		cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
		cJSON_AddStringToObject(cRoot, "orderNo", pRecord->orderNo);
		cJSON_AddNumberToObject(cRoot, "status", pRecvDataInfo->offlineClearData.eAns_ITEM8615);
		cJSON_AddNumberToObject(cRoot, "gunNo", port + 1);
		cJSON_AddNumberToObject(cRoot, "ts", Common_FourUint8ToUint32(pRecord->ts));
		cJSON_AddNumberToObject(cRoot, "indexRec", Common_FourUint8ToUint32(pRecord->indexRec));
		cJSON_AddNumberToObject(cRoot, "typeRec", pRecord->typeRec);
		cJSON_AddNumberToObject(cRoot, "type", pRecord->type);	
		cJSON_AddNumberToObject(cRoot, "initiator", pRecord->initiator);
		cJSON_AddStringToObject(cRoot, "user", pRecord->user);

		plan = cJSON_CreateObject();
		IOT_XDT_CheckObjIsNull(plan, 0);
		cJSON_AddNumberToObject(plan, "typePlan", pRecord->typePlan);
		cJSON_AddNumberToObject(plan, "typeStart", pRecord->typeStart);
		cJSON_AddNumberToObject(plan, "tsStart", Common_FourUint8ToUint32(pRecord->tsStart));
		cJSON_AddNumberToObject(plan, "value", Common_FourUint8ToUint32(pRecord->value) / 10000.0);
		cJSON_AddItemToObject(cRoot, "plan", plan);
		
		cJSON_AddNumberToObject(cRoot, "pricingId", Common_FourUint8ToUint32(pRecord->pricingID));
		cJSON_AddNumberToObject(cRoot, "beginTs", Common_FourUint8ToUint32(pRecord->beginTs));
		cJSON_AddNumberToObject(cRoot, "endTs", Common_FourUint8ToUint32(pRecord->endTs)); 
		cJSON_AddNumberToObject(cRoot, "beginMr", round(Common_FourUint8ToUint32(pRecord->beginMr) / 10.0) / 1000.0);
		cJSON_AddNumberToObject(cRoot, "endMr", round(Common_FourUint8ToUint32(pRecord->endMr) / 10.0) / 1000.0);
		cJSON_AddNumberToObject(cRoot, "tPq", Common_FourUint8ToUint32(pRecord->tPq) / 10000.0);
		cJSON_AddNumberToObject(cRoot, "elecAmt", Common_FourUint8ToUint32(pRecord->elecAmt) / 10000.0);
		cJSON_AddNumberToObject(cRoot, "serAmt", Common_FourUint8ToUint32(pRecord->serMt) / 10000.0);
		cJSON_AddNumberToObject(cRoot, "amt", Common_FourUint8ToUint32(pRecord->amt) / 10000.0);
		cJSON_AddNumberToObject(cRoot, "beginSoc", 0);
		cJSON_AddNumberToObject(cRoot, "endSoc", 0);
		cJSON_AddNumberToObject(cRoot, "stopReason", pRecord->stopReason);
		cJSON_AddNumberToObject(cRoot, "typeRule", pRecord->typeRule);
		cJSON_AddNumberToObject(cRoot, "pq_total", Common_FourUint8ToUint32(pRecord->pqTotal) / 10000.0);
	
		if (pRecord->typeRule == 1)
		{
			periodsList = cJSON_CreateArray();
			IOT_XDT_CheckObjIsNull(periodsList, 0);
			
			for (index = 0; index < IOT_XDT_RATE_MODE_MAX_PERIOD; index++)
			{
				pPeriodInfo = &pRecord->periodInfoArray[index];
			
				if (pPeriodInfo->valid == TRUE)
				{
					period[index] = cJSON_CreateObject();
					IOT_XDT_CheckObjIsNull(period[index], 0);
					cJSON_AddNumberToObject(period[index], "sn", pPeriodInfo->sn);
					cJSON_AddNumberToObject(period[index], "pq",Common_FourUint8ToUint32(pPeriodInfo->pq) / 10000.0);
					cJSON_AddItemToArray(periodsList, period[index]);
				}
			}
			
			cJSON_AddItemToObject(cRoot,"periodsList",periodsList);
		}
	}
	else
	{
		cRoot = cJSON_CreateObject();
		IOT_XDT_CheckObjIsNull(cRoot, 0);
		
		cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
		cJSON_AddStringToObject(cRoot, "orderNo", pRecvDataInfo->offlineClearData.orderNo_ITEM8615);
		cJSON_AddNumberToObject(cRoot, "status", pRecvDataInfo->offlineClearData.eAns_ITEM8615);
		cJSON_AddNumberToObject(cRoot, "gunNo", pRecvDataInfo->offlineClearData.gunNo_ITEM8615);
		cJSON_AddNumberToObject(cRoot, "ts", 0);
		cJSON_AddNumberToObject(cRoot, "indexRec", 0);
		cJSON_AddNumberToObject(cRoot, "typeRec", 0);
		cJSON_AddNumberToObject(cRoot, "type", 0);	
		cJSON_AddNumberToObject(cRoot, "initiator", 0);
		cJSON_AddStringToObject(cRoot, "user", "");
		
		plan = cJSON_CreateObject();
		IOT_XDT_CheckObjIsNull(plan, 0);
		cJSON_AddNumberToObject(plan, "typePlan", 0);
		cJSON_AddNumberToObject(plan, "typeStart", 0);
		cJSON_AddNumberToObject(plan, "tsStart", 0);
		cJSON_AddNumberToObject(plan, "value", 0);
		cJSON_AddItemToObject(cRoot, "plan", plan);
		
		cJSON_AddNumberToObject(cRoot, "pricingId", 0);
		cJSON_AddNumberToObject(cRoot, "beginTs", 0);
		cJSON_AddNumberToObject(cRoot, "endTs", 0); 
		cJSON_AddNumberToObject(cRoot, "beginMr", 0);
		cJSON_AddNumberToObject(cRoot, "endMr", 0);
		cJSON_AddNumberToObject(cRoot, "tPq", 0);
		cJSON_AddNumberToObject(cRoot, "elecAmt", 0);
		cJSON_AddNumberToObject(cRoot, "serAmt", 0);
		cJSON_AddNumberToObject(cRoot, "amt", 0);
		cJSON_AddNumberToObject(cRoot, "beginSoc", 0);
		cJSON_AddNumberToObject(cRoot, "endSoc", 0);
		cJSON_AddNumberToObject(cRoot, "stopReason", 0);
		cJSON_AddNumberToObject(cRoot, "typeRule", 0);
		cJSON_AddNumberToObject(cRoot, "pq_total", 0);
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportQueryBoardInfoRsp_ITEM872(uint8_t port, void *pBuf)
{
	cJSON *cRoot, *cBoardCtrl, *boardInfo;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "snPlat", pIotXDTCtx->platDn);
	cJSON_AddNumberToObject(cRoot, "status", 0);

	cBoardCtrl = cJSON_CreateArray();
	IOT_XDT_CheckObjIsNull(cBoardCtrl, 0);

	boardInfo = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(boardInfo, 0);

	cJSON_AddNumberToObject(boardInfo, "no", 1);
	cJSON_AddStringToObject(boardInfo, "name", SYSCFG_CFG_PRODUCT_CODE);
	cJSON_AddNumberToObject(boardInfo, "type", 0);
	cJSON_AddStringToObject(boardInfo, "verHard", HW_VERSION_INFO);
	cJSON_AddStringToObject(boardInfo, "verSoft", APP_SW_VERSION_STRING);

	cJSON_AddItemToArray(cBoardCtrl, boardInfo);
	cJSON_AddItemToObject(cRoot, "boardCtrl", cBoardCtrl);
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_RequestOTAAttribute_ITEM882(uint8_t port, void *pBuf)
{
	cJSON *cRoot = NULL;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	cJSON_AddStringToObject(cRoot, "sharedKeys", "fw_tag,fw_title,fw_version,fw_url");
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);

	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static uint16_t IotXDT_ReportFwState_ITEM886(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;	
	cJSON *cRoot;
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);
	cJSON_AddStringToObject(cRoot, "current_fw_title", "GN");

	if (pPlatInfo->otaState == eIotXDTOtaState_Idle)
	{
		cJSON_AddStringToObject(cRoot, "current_fw_version", APP_SW_VERSION_STRING);
		cJSON_AddStringToObject(cRoot, "current_protocol_version", IOT_XDT_PROTOCOL_VER);
		cJSON_AddStringToObject(cRoot, "fw_state", "UPDATED");
		cJSON_AddStringToObject(cRoot, "fw_error", "");
	}
	else if (pPlatInfo->otaState == eIotXDTOtaState_Starting)
	{
		cJSON_AddStringToObject(cRoot, "current_fw_version", pPlatInfo->otaSoftwareVersion);
		cJSON_AddStringToObject(cRoot, "current_protocol_version", IOT_XDT_PROTOCOL_VER);
		cJSON_AddStringToObject(cRoot, "fw_state", "UPDATING");
		cJSON_AddStringToObject(cRoot, "fw_error", "");
	}
	else
	{
		if (pPlatInfo->otaState == eIotXDTOtaState_Succ)
		{
			cJSON_AddStringToObject(cRoot, "current_fw_version", APP_SW_VERSION_STRING);
			cJSON_AddStringToObject(cRoot, "current_protocol_version", IOT_XDT_PROTOCOL_VER);
			cJSON_AddStringToObject(cRoot, "fw_state", "UPDATED");
			cJSON_AddStringToObject(cRoot, "fw_error", "");
		}
		else
		{
			cJSON_AddStringToObject(cRoot, "current_fw_version", APP_SW_VERSION_STRING);
			cJSON_AddStringToObject(cRoot, "current_protocol_version", IOT_XDT_PROTOCOL_VER);
			cJSON_AddStringToObject(cRoot, "fw_state", "FAILED");
			cJSON_AddStringToObject(cRoot, "fw_error", "update failed");
		}

		pPlatInfo->otaState = eIotXDTOtaState_Idle;
		MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
	}

	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static void IotXDT_PackTopic(char *pInTopic, char *pOutTopic, uint32_t rpcSeq)
{
	char *pStart = NULL;

	if (pInTopic != NULL && pOutTopic != NULL)
	{
		if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2R_REQUEST, strlen(IOT_XDT_PRE_TOPIC_V2R_REQUEST)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2R_RESPONSE, strlen(IOT_XDT_PRE_TOPIC_V2R_RESPONSE)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2A_REQUEST, strlen(IOT_XDT_PRE_TOPIC_V2A_REQUEST)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s", pInTopic);	
		}
	}
}

uint32_t IotXDT_GetSendCmdSendCycle(uint32_t cmd)
{
	IotXDTSendCtrl_Struct *pSendCtrl = NULL;
	uint32_t retCycle = 0;
	uint8_t index = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTSendCtrlTable); index++)
	{
		pSendCtrl = &c_IotXDTSendCtrlTable[index];
		
		if (pSendCtrl->cmd == cmd)
		{
			retCycle = pSendCtrl->sendCycle;
			break;
		}
	}

	return retCycle;
}

void IotXDT_UpCtrlSendDeal(void)
{
	IotXDTSendCtrl_Struct *pSendCtrl = NULL;
	uint8_t port = 0;
	uint8_t index = 0;
	char cTopic[IOT_XDT_TOPIC_LEN + 1] = { 0 };
	uint16_t dataLen = 0;
	uint32_t rpcSeq = 0;
	uint8_t txbuf[IOT_XDT_TXRX_BUFFER_SIZE] = {0};

	if (pIotXDTCtx->queueBusyFlag == TRUE)
	{
		if (Common_JudgeTimeoutMs(pIotXDTCtx->waitQueueIdleTick, 500))
		{
			pIotXDTCtx->queueBusyFlag = FALSE;
		}
	}
	else
	{
		while (1)
		{
			if (pIotXDTCtx->sendIndex < ARRAY_SIZE(c_IotXDTSendCtrlTable))
			{
				index = pIotXDTCtx->sendIndex;
				port = pIotXDTCtx->sendPort;
					
				pSendCtrl = &c_IotXDTSendCtrlTable[index];

				if (Common_GetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd) == TRUE &&
			        IotXDT_ReportCycleCheck(port, pSendCtrl->cmd, pSendCtrl->sendCycle) == TRUE)
				{
					if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
					{
						rpcSeq = pIotXDTCtx->reqSeq;
						
						if (pSendCtrl->matchCmd != IOT_XDT_CMD_NULL)
						{
                            Common_SetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, rpcSeq);
						}
						
						pIotXDTCtx->reqSeq++;
					}
					else
					{
                        rpcSeq = Common_GetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd);
					}

					IotXDT_PackTopic(pSendCtrl->topic, cTopic, rpcSeq);

					if (NULL != pSendCtrl->pSendFunc)
					{
						dataLen = pSendCtrl->pSendFunc(port, txbuf);
					}

					if (dataLen > 0)
					{
						pIotXDTCtx->queueBusyFlag = TRUE;
						pIotXDTCtx->waitQueueIdleTick = Common_GetSystick();

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotXDTCtx->frameQueueChannelID, cTopic, strlen(cTopic), txbuf, dataLen))
                        {
                            if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
                            {
                                pIotXDTCtx->reqSeq--;
                            }

                            IOTXDT_CFG_LogPrint("FrameQueue_PushTx fail...[cmd: 0x%03X][dataLen = %d]\r\n", pSendCtrl->cmd, dataLen);
                            break;
                        }
						
						extern size_t xPortGetFreeHeapSize( void );
						IOTXDT_CFG_LogPrint("Remaining heap size: %zu bytes\n", xPortGetFreeHeapSize());
                        Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, Common_GetSystick());
                        
                        if (pSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        }

                        if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
                        {
                            if (pSendCtrl->matchCmd != IOT_XDT_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
					}
				}
			}

			pIotXDTCtx->sendIndex++;

			if (pIotXDTCtx->sendIndex >= ARRAY_SIZE(c_IotXDTSendCtrlTable))
			{
				pIotXDTCtx->sendIndex = 0;
				pIotXDTCtx->sendPort++;

				if (pIotXDTCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotXDTCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotXDTCtx->queueBusyFlag == TRUE)
			{
				break;
			}
		}
	}
}




















