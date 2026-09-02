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
#include "Asw_IotProtoXJM.h"
#include "FrameQueue.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "Asw_ChargeIf.h"
#include "Version.h"
#include "Asw_Monitor.h"
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
static uint16_t IotXJ_SendSignInfo(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendHeartbeat(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendStateInfo(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetRateModeResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetStartChargeResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetStopChargeResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendEvent(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendOrderInfo(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendErrorInfo(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetIntegerParaResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetCommonParaResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetRemoteConfigResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendQueryCommonParaResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendSetPowerAllocResponse(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendCardAuth(uint8_t port, void *pBuf);
static uint16_t IotXJ_SendCardRequestCharge(uint8_t port, void *pBuf);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotXJCtx_Struct *pIotXJCtx;

static IotXJSendCtrl_Struct  c_IotXJSendCtrlTable[] = 
{	
	[0] = {
		.cmd = IOT_XJ_CMD_SEND_SIGN_INFO,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSignInfo,
		.matchCmd = IOT_XJ_CMD_SEND_SIGN_INFO_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "签到信息",
	},

	[1] = {
		.cmd = IOT_XJ_CMD_SEND_HEART,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendHeartbeat,
		.matchCmd = IOT_XJ_CMD_SEND_HEART_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "心跳信息",
	},

	[2] = {
		.cmd = IOT_XJ_CMD_SEND_STATE_INFO,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendStateInfo,
		.matchCmd = IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "状态信息",
	},

	[3] = {
		.cmd = IOT_XJ_CMD_SET_RATEMODE_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetRateModeResponse,
		.matchCmd = IOT_XJ_CMD_SET_RATEMODE,
		.printFlag = TRUE,
		.cMeaning = "设置费率响应",
	},

	[4] = {
		.cmd = IOT_XJ_CMD_SET_START_CHARGE_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetStartChargeResponse,
		.matchCmd = IOT_XJ_CMD_SET_START_CHARGE,
		.printFlag = TRUE,
		.cMeaning = "启动充电响应",
	},

	[5] = {
		.cmd = IOT_XJ_CMD_SET_STOP_CHARGE_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetStopChargeResponse,
		.matchCmd = IOT_XJ_CMD_SET_STOP_CHARGE,
		.printFlag = TRUE,
		.cMeaning = "停止充电响应",
	},

	[6] = {
		.cmd = IOT_XJ_CMD_SEND_EVENT,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendEvent,
		.matchCmd = IOT_XJ_CMD_SEND_EVENT_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "事件信息",
	},

	[7] = {
		.cmd = IOT_XJ_CMD_SEND_ORDER_INFO,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendOrderInfo,
		.matchCmd = IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "订单信息",
	},

	[8] = {
		.cmd = IOT_XJ_CMD_SEND_ERROR_INFO,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendErrorInfo,
		.matchCmd = IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "故障信息",
	},

	[9] = {
		.cmd = IOT_XJ_CMD_SET_INTEGER_PARA_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetIntegerParaResponse,
		.matchCmd = IOT_XJ_CMD_SET_INTEGER_PARA,
		.printFlag = TRUE,
		.cMeaning = "工作参数设置应答",
	},

	[10] = {
		.cmd = IOT_XJ_CMD_SET_COMMON_PARA_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetCommonParaResponse,
		.matchCmd = IOT_XJ_CMD_SET_COMMON_PARA,
		.printFlag = TRUE,
		.cMeaning = "通用参数设置应答",
	},

	[11] = {
		.cmd = IOT_XJ_CMD_REMOTE_CONFIG_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetRemoteConfigResponse,
		.matchCmd = IOT_XJ_CMD_REMOTE_CONFIG,
		.printFlag = TRUE,
		.cMeaning = "后台服务器终端控制应答",
	},

	[12] = {
		.cmd = IOT_XJ_CMD_QUERY_COMMON_PARA_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendQueryCommonParaResponse,
		.matchCmd = IOT_XJ_CMD_QUERY_COMMON_PARA,
		.printFlag = TRUE,
		.cMeaning = "查询通用参数应答",
	},

	[13] = {
		.cmd = IOT_XJ_CMD_SET_POWER_ALLOC_RESPONSE,
		.cmdType = IOT_XJ_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendSetPowerAllocResponse,
		.matchCmd = IOT_XJ_CMD_SET_POWER_ALLOC,
		.printFlag = TRUE,
		.cMeaning = "功率设置应答",
	},

	[14] = {
		.cmd = IOT_XJ_CMD_REQUEST_CARD_AUTH,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendCardAuth,
		.matchCmd = IOT_XJ_CMD_REQUEST_CARD_AUTH_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "刷卡鉴权请求",
	},

	[15] = {
		.cmd = IOT_XJ_CMD_REQUEST_CARD_CHARGE,
		.cmdType = IOT_XJ_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXJ_SendCardRequestCharge,
		.matchCmd = IOT_XJ_CMD_REQUEST_CARD_CHARGE_RESPONSE,
		.printFlag = TRUE,
		.cMeaning = "刷卡启动充电请求",
	},
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotXJ_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotXJCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotXJCtx->pFuncSendCtrl, port, cmd);
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

static uint16_t IotXJ_PackHead(uint16_t cmd, uint32_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotXJFrameHead_Struct *pFrameHead = (IotXJFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotXJFrameHead_Struct);
    uint8_t cs = 0;

    pFrameHead->head[0] = IOT_XJ_HEAD1;
    pFrameHead->head[1] = IOT_XJ_HEAD2;

	Common_Uint16ToTwoUint8(pFrameHead->dataLen, totalLen + 1);

	pFrameHead->version[0] = IOT_XJ_PROTOCOL_VER3;
	pFrameHead->version[1] = IOT_XJ_PROTOCOL_VER2;
	pFrameHead->version[2] = IOT_XJ_PROTOCOL_VER1;
	pFrameHead->version[3] = IOT_XJ_PROTOCOL_VER0;

	memcpy(&pFrameHead->seq, &seq, 4);
	memcpy(&pFrameHead->cmd, &cmd, 2);

	/* 命令代码和数据域 */
	cs = IotXJ_CalcChecksum((int8_t *)pFrameHead->cmd, totalLen - 12);
    pBuf[totalLen++] = cs;
    return totalLen;
}


static uint16_t IotXJ_SendSignInfo(uint8_t port, void *pBuf)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
	uint16_t dataLen = 0;
	uint32_t totalRecordCount = 0;
	CommonDateTime_Struct dateTime = {0};
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint8_t imei[15 + 1] = {0};

	/* 电源模块总数 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 电源模块总功率大小*/
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 离线在线允许充电设置 */
	if (AswMonitor_CheckForbidState() == TRUE)
	{
		pTxBuf[dataLen++] = 0x11;
	}
	else
	{
		pTxBuf[dataLen++] = 0x00;
	}

	/* TCU充电桩软件版本*/
	pTxBuf[dataLen++] = APP_SW_MAJOR_VERSION;
	pTxBuf[dataLen++] = APP_SW_MINOR_VERSION;
	pTxBuf[dataLen++] = APP_SW_CUSTORM_VERSION;
	pTxBuf[dataLen++] = APP_SW_PATCH_VERSION;

	/* 充电桩类型 */
	Common_Uint16ToTwoUint8(pTxBuf + dataLen, 0x01);
	dataLen += 2;
	/* 启动次数 */
	memcpy(&pTxBuf[dataLen], &pPrivateParam->stXJParam.platinfo.rebootCount, 4);
	dataLen += 4;
	/* 数据上传模式（保留） */
	pTxBuf[dataLen++] = 0x00;
	/* 签到间隔时间 */
	memcpy(&pTxBuf[dataLen], &pPrivateParam->stXJParam.platinfo.frame106Interval, 2);
	dataLen += 2;
	/* 小桔TCU标识 */
	pTxBuf[dataLen++] = 0x00;
	/* 充电枪个数 */
	pTxBuf[dataLen++] = SYSCFG_CFG_GUN_NUM;
	/* 心跳上报周期 */
	pTxBuf[dataLen++] = pPrivateParam->stXJParam.platinfo.frame102Interval;
	/* 心跳检测超时次数 */
	pTxBuf[dataLen++] = pPrivateParam->stXJParam.platinfo.frame102MaxTimeoutTimes;
	/* 充电记录数量 */
	totalRecordCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_OrderRecord);
	memcpy(&pTxBuf[dataLen], &totalRecordCount, 4);
	dataLen += 4;
	/* 当前充电桩系统时间 */
	SSTM_GetDateTime(&dateTime);
	IotXJ_DateTimeToBcdTime(&dateTime, pTxBuf + dataLen);
	dataLen += 8;
	/* 最近一次充电时间 */
	Common_TimestampToDateTime(pIotXJCtx->latestChargeTimestamp, &dateTime);  
	IotXJ_DateTimeToBcdTime(&dateTime, pTxBuf + dataLen);
	dataLen += 8;
	/* 最近一次启动时间 */
	memset(pTxBuf + dataLen, 0, 8);
	dataLen += 2;
	/* 签到密码（保留）*/
	memset(pTxBuf + dataLen, 0, 8);
	dataLen += 8;
	/* IMEI */
	CddNetM_GetIMEI(imei);
	memcpy(&pTxBuf[dataLen], imei, 15);
	dataLen += 32;
	/* 充电桩CCU软件版本 */
	pTxBuf[dataLen++] = 0x00;
	pTxBuf[dataLen++] = 0x00;
	pTxBuf[dataLen++] = 0x00;
	pTxBuf[dataLen++] = 0x00;
	/* 充电桩CCU2软件版本 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 充电桩PCU软件版本 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	return dataLen;
}

static uint16_t IotXJ_SendHeartbeat(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 心跳序列号 */
	memcpy(&pTxBuf[dataLen], &pIotXJCtx->heartBeatSeq, 2);
	dataLen += 2;

	pIotXJCtx->heartBeatSeq++;
	return dataLen;
}

static uint16_t IotXJ_SendStateInfo(uint8_t port, void *pBuf)
{
	AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
	AswMonitorChargeCtrl_Struct *pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
	uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);
	CommonDateTime_Struct dateTime = {0};
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint64_t temp64 = 0;

	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 充电枪个数 */
	pTxBuf[dataLen++] = SYSCFG_CFG_GUN_NUM;
	/* 充电枪口号 */
	pTxBuf[dataLen++] = port + 1;
	/* 充电枪类型 */
	pTxBuf[dataLen++] = 0x02;
	/* 工作状态 */
	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.eGunStatus[port];
	/* SOC */
	pTxBuf[dataLen++] = 0;
	/* 告警状态 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 车连接状态*/
	if (AswChargeIf_CheckS2Closed(port) == TRUE)
	{
		pTxBuf[dataLen++] = 0x02; /* 已连接 */
	}
	else
	{
		if (AswChargeIf_CheckGunConnected(port) == TRUE)
		{
			pTxBuf[dataLen++] = 0x01;  /* 半连接 */
		}
		else
		{
			pTxBuf[dataLen++] = 0x00;  /* 未连接 */
		}
	}
	/* 已充金额 */
	if (orderIdleFlag != TRUE)
	{
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], pChargeData->totalMoney / 100);
		dataLen += 4;
	}
	else
	{
		memset(pTxBuf + dataLen, 0, 4);
		dataLen += 4;
	}
	/* 使用模块数 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 预留 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 直流充电电压 */
	Common_Uint16ToTwoUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputVoltage(port) / 10);
	dataLen += 2;
	/* 直流充电电流 */
	Common_Uint16ToTwoUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputCurrent(port) / 100);
	dataLen += 2;
	/* BMS需求电压 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* BMS需求电流 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* BMS充电模式 */
	pTxBuf[dataLen++] = 0x00;
	/* 交流A相充电电压 */
	Common_Uint16ToTwoUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputVoltage(port) / 10);
	dataLen += 2;
	/* 交流B相充电电压 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 交流C相充电电压 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 交流A相充电电流 */
	Common_Uint16ToTwoUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputCurrent(port) / 100);
	dataLen += 2;
	/* 交流B相充电电流 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 交流C相充电电流 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 剩余充电时间 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;

	if (orderIdleFlag != TRUE)
	{
		/* 充电时长 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], pChargeData->chargeTime);
		dataLen += 4;
		/* 本次充电累计充电电量 */
		pIotXJCtx->chargeEnergy[port] = (pChargeData->stopMeterVal / 10) - (pChargeData->startMeterVal / 10);
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], pIotXJCtx->chargeEnergy[port]);
		dataLen += 4;
		/* 充电前电表度数 */
		temp64 = pChargeData->startMeterVal / 10;
		memcpy(&pTxBuf[dataLen], &temp64, 8);
		dataLen += 8;
		/* 当前电表度数 */
		temp64 = AswChargeIf_GetMeterEnergyVal(port) / 10;
		memcpy(&pTxBuf[dataLen], &temp64, 8);
		dataLen += 8;
		/* 充电启动方式 */
		if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_APP)
		{
			pTxBuf[dataLen++] = 0x01;
		}
		else if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
		{
			pTxBuf[dataLen++] = 0x00;
		}
		else
		{
			pTxBuf[dataLen++] = 0x02;
		}
		/* 充电策略 */
		if (pChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeTime)
		{
			pTxBuf[dataLen++] = 0x01;
		}
		else if (pChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeMoney)
		{
			pTxBuf[dataLen++] = 0x02;
		}
		else if (pChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeEnergy)
		{
			pTxBuf[dataLen++] = 0x03;
		}
		else
		{
			pTxBuf[dataLen++] = 0x00;
		}
		/* 充电策略值 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], pChargeCtrl->chargeCtrlVal);
		dataLen += 4;
	}
	else
	{
		/* 充电时长 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], 0);
		dataLen += 4;
		/* 本次充电累计充电电量 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], 0);
		dataLen += 4;
		/* 充电前电表度数 */
		temp64 = 0;
		memcpy(&pTxBuf[dataLen], &temp64, 8);
		dataLen += 8;
		/* 当前电表度数 */
		temp64 = AswChargeIf_GetMeterEnergyVal(port) / 10;
		memcpy(&pTxBuf[dataLen], &temp64, 8);
		dataLen += 8;
		/* 充电启动方式 */
		pTxBuf[dataLen++] = 0x00;
		/* 充电策略 */
		pTxBuf[dataLen++] = 0x00;
		/* 充电策略值 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], 0);
		dataLen += 4;
	}

	/* 预约标志 */
	pTxBuf[dataLen++] = 0x00;

	if (orderIdleFlag != TRUE)
	{
		/* 订单号 */
		memcpy(pTxBuf + dataLen, pIotXJCtx->stProtoData.curUsedOrderNo[port], 32);
		dataLen += 32;
		/* 预约超时时间 */
		pTxBuf[dataLen++] = 0x00;
		/* 充电开始时间 */
		Common_TimestampToDateTime(pChargeData->chargeStartTime, &dateTime);
		IotXJ_DateTimeToBcdTime(&dateTime, &pTxBuf[dataLen]);
		dataLen += 8;
		/* 充电前卡余额*/
		memset(&pTxBuf[dataLen], 0x00, 4);
		dataLen += 4;
		/* 充电模式 */
		memset(&pTxBuf[dataLen], 0x00, 4);
		dataLen += 4;
		/* 充电功率 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputPower(port) / 100);
		dataLen += 4;
	}
	else
	{
		/* 订单号 */
		memset(&pTxBuf[dataLen], 0x00, 32);
		dataLen += 32;
		/* 预约超时时间 */
		pTxBuf[dataLen++] = 0x00;
		/* 充电开始时间 */
		memset(&pTxBuf[dataLen], 0x00, 8);
		dataLen += 8;
		/* 充电前卡余额*/
		memset(&pTxBuf[dataLen], 0x00, 4);
		dataLen += 4;
		/* 充电模式 */
		memset(&pTxBuf[dataLen], 0x00, 4);
		dataLen += 4;
		/* 充电功率 */
		Common_Uint32ToFourUint8(&pTxBuf[dataLen], AswChargeIf_GetOutputPower(port) / 100);
		dataLen += 4;
	}

	/* 充电桩进风口温度 */
	memset(&pTxBuf[dataLen], 0x00, 4);
	dataLen += 4;
	/* 充电接口+温度 */
	Common_Uint32ToFourUint8(&pTxBuf[dataLen], AswChargeIf_GetGunTemperature(port));
	dataLen += 4;
	/* 充电接口-温度 */
	memset(&pTxBuf[dataLen], 0x00, 4);
	dataLen += 4;
	/* 充电模块当前最高温度 */
	memset(&pTxBuf[dataLen], 0x00, 4);
	dataLen += 4;
	/* 充电模块当前最低温度 */
	memset(&pTxBuf[dataLen], 0x00, 4);
	dataLen += 4;
	/* 充电桩出风温度 */
	memset(&pTxBuf[dataLen], 0x00, 4);
	dataLen += 4;
	/* 充电桩温度 */
	Common_Uint32ToFourUint8(&pTxBuf[dataLen], AswChargeIf_GetEnvTemperature());
	dataLen += 4;
	/* 检测点1电压 */
	pTxBuf[dataLen++] = AswChargeIf_GetCpVoltage(port) / 100;
	/* 放电前电表度数 */
	memset(&pTxBuf[dataLen], 0x00, 8);
	dataLen += 8;
	/* 当前电表读数 */
	memset(&pTxBuf[dataLen], 0x00, 8);
	dataLen += 8;

	return dataLen;
}

static uint16_t IotXJ_SendSetRateModeResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd1309SetResult;
	return dataLen;
}

static uint16_t IotXJ_SendSetStartChargeResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 充电枪口号 */
	pTxBuf[dataLen++] = port + 1;
	/* 命令执行结果 */
	sprintf((char *)&pTxBuf[dataLen], "%04X", pIotXJCtx->stProtoData.cmd07SetReuslt[port]);
	dataLen += 4;
	/* 订单号 */
	memcpy(pTxBuf + dataLen, pIotXJCtx->stProtoData.newRecvOrderNo[port], 32);
	dataLen += 32;	

	/* 按需求，启动命令后，立即发送一次状态信息 */
	Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO, TRUE);
	return dataLen;
}

static uint16_t IotXJ_SendSetStopChargeResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 充电枪口号 */
	pTxBuf[dataLen++] = port + 1;
	/* 订单号 */
	memcpy(pTxBuf + dataLen, pIotXJCtx->stProtoData.newRecvOrderNo[port], 32);
	dataLen += 32;
	/* 命令执行结果 */
	sprintf((char *)&pTxBuf[dataLen], "%04X", pIotXJCtx->stProtoData.cmd11SetReuslt[port]);
	dataLen += 4;

	/* 按需求，停止命令后，立即发送一次状态信息 */
	Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO, TRUE);
	return dataLen;
}

static uint16_t IotXJ_SendEvent(uint8_t port, void *pBuf)
{
	IotXJEventType_Struct *pEventQueue = IotXJ_GetFirstEventQueue(port);
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint32_t temp = 0;

	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 2);
	dataLen += 2;
	/* 充电枪口号 */
	pTxBuf[dataLen++] = port + 1;
	/* 事件对应地址编码 */
	temp = pEventQueue->eventType;
	Common_Uint32ToFourUint8(&pTxBuf[dataLen], temp);
	dataLen += 4;
	/* 事件参数 */
	sprintf((char *)&pTxBuf[dataLen], "%04X", pEventQueue->eventParam);
	dataLen += 4;
	/* 订单号 */
	memcpy(pTxBuf + dataLen, pEventQueue->orderNo, 32);
	dataLen += 32;

	return dataLen;
}

static uint16_t IotXJ_SendOrderInfo(uint8_t port, void *pBuf)
{
    MSNvmXJOrderInfo_Struct *pOrderData = &pIotXJCtx->stOrderInfo.platOrderInfo.stXJOrderInfo;
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

    IotXJ_TransformChargeRecord(&pIotXJCtx->stOrderInfo.platOrderInfo, pTxBuf, &dataLen);
    return dataLen;
}

static uint16_t IotXJ_SendErrorInfo(uint8_t port, void *pBuf)
{
	IotXJErrInfoReport_Struct *pErrInfo = pIotXJCtx->stProtoData.stErrInfoReport[port];
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 充电枪口号 */
	pTxBuf[dataLen++] = port + 1;
	/* 故障错误码 */
	sprintf((char *)&pTxBuf[dataLen], "%04X", pErrInfo[0].errIndex);
	dataLen += 4;
	/* 故障状态 */
	pTxBuf[dataLen++] = pErrInfo[0].status ? 0x00 : 0x01;
	pErrInfo[0].eReportState = eXJReportState_Reporting;
    return dataLen;
}

static uint16_t IotXJ_SendSetIntegerParaResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 设置成功个数 */
	Common_Uint32ToFourUint8(&pTxBuf[dataLen], pIotXJCtx->stProtoData.cmd501SetSuccesCount);
	dataLen += 4;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	
	/* 设置结果 */
	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd501SetResult;
	return dataLen;
}

static uint16_t IotXJ_SendSetCommonParaResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint8_t index = 0;
	uint8_t paraFailIndex = 0;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 保留个数 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 结果 */
	Common_Uint16ToTwoUint8(&pTxBuf[dataLen], pIotXJCtx->stProtoData.cmd507SetResult);
	dataLen += 2;

	if (pIotXJCtx->stProtoData.cmd507SetResult == 0x00)
	{
		/* 设置成功个数 */
		pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd507SetSuccesCount;
		/* 设置失败个数 */
		pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd507SetFailCount;

		if (pIotXJCtx->stProtoData.cmd507SetFailCount > 0)
		{
			for (index = 0; index < sizeof(pIotXJCtx->stProtoData.cmd507SetParaAddr) && 
			     paraFailIndex < pIotXJCtx->stProtoData.cmd507SetFailCount; index++)
			{
				if (pIotXJCtx->stProtoData.cmd507SetParaAddr[index] == 1)
				{
					pTxBuf[dataLen++] = index;
					pTxBuf[dataLen++] = 0x00;
					paraFailIndex++;
				}
			}
		}
	}

	return dataLen;
}

static uint16_t IotXJ_SendSetRemoteConfigResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 保留个数 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 结果 */
	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd511SetResult;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 3);
	dataLen += 3;

	return dataLen;
}

static uint16_t IotXJ_SendQueryCommonParaResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint8_t index = 0;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 保留 */
	memset(pTxBuf + dataLen, 0, 4);
	dataLen += 4;
	/* 参数个数 */
	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd514QueryParaCount;
	/* 保留 */
	pTxBuf[dataLen++] = 0x00;

	for (index = 0; index < sizeof(pIotXJCtx->stProtoData.cmd514QueryParaAddr); index++)
	{
		if (pIotXJCtx->stProtoData.cmd514QueryParaAddr[index] == 1)
		{
			/* 参数地址 */
			pTxBuf[dataLen++] = index;
			/* 参数长度 */
			pTxBuf[dataLen++] = 1;
			/* 参数内容 */
			if (index == 3)   /* 刷卡充电 */
			{
				pTxBuf[dataLen++] = 0x01;
			}
			else if (index == 18)   /* 启动充电限制时间参数 */
			{
				pTxBuf[dataLen++] = 75;
			}
			else if (index == 19)   /* 远程重启 */
			{
				pTxBuf[dataLen++] = 0x01;
			}
			else if (index == 20)  /* 功率控制 */
			{
				pTxBuf[dataLen++] = 0x01;
			}
			else if (index == 22)  /* 离线充电功能 */
			{
				pTxBuf[dataLen++] = 0x01;
			}
			else
			{
				pTxBuf[dataLen++] = 0x00;
			}
		}
	}

	return dataLen;
}

static uint16_t IotXJ_SendSetPowerAllocResponse(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;

	pTxBuf[dataLen++] = port + 1;

	pTxBuf[dataLen++] = pIotXJCtx->stProtoData.cmd20SetResult[port];

	return dataLen;
}

static uint16_t IotXJ_SendCardAuth(uint8_t port, void *pBuf)
{
	AswMonitorChargeCtrl_Struct *pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint16_t gunNo = port + 1;

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;

	/* 枪号 */
	memcpy(&pTxBuf[dataLen], &gunNo, 2);
	dataLen += 2;

	/* 卡号 */
	sprintf((char *)&pTxBuf[dataLen], "%02X%02X%02X%02X%02X%02X%02X%02X", pChargeCtrl->authCardID[0], pChargeCtrl->authCardID[1], 
		pChargeCtrl->authCardID[2], pChargeCtrl->authCardID[3], pChargeCtrl->authCardID[4], 
		pChargeCtrl->authCardID[5], pChargeCtrl->authCardID[6], pChargeCtrl->authCardID[7]);
	dataLen += 16;
	/* 随机数 */
	memcpy(&pTxBuf[dataLen], pChargeCtrl->randomNum, ASWMONITOR_RANDOM_LEN);
	dataLen += ASWMONITOR_RANDOM_LEN;
	/* 物理卡号 */
	memcpy(&pTxBuf[dataLen], pChargeCtrl->phyCardID, 4);
	dataLen += 4;

	return dataLen;
}

static uint16_t IotXJ_SendCardRequestCharge(uint8_t port, void *pBuf)
{
	uint16_t dataLen = 0;
	uint8_t *pTxBuf = (uint8_t *)pBuf;
	uint16_t gunNo = port + 1;
	AswMonitorChargeCtrl_Struct *pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);

	/* 充电桩编码 */
	memcpy(&pTxBuf[dataLen], pIotXJCtx->platDn, 32);
	dataLen += 32;
	/* 枪号 */
	memcpy(&pTxBuf[dataLen], &gunNo, 2);
	dataLen += 2;
	/* 卡号 */
	sprintf((char *)&pTxBuf[dataLen], "%02X%02X%02X%02X%02X%02X%02X%02X", pChargeCtrl->authCardID[0], pChargeCtrl->authCardID[1], 
		pChargeCtrl->authCardID[2], pChargeCtrl->authCardID[3], pChargeCtrl->authCardID[4], 
		pChargeCtrl->authCardID[5], pChargeCtrl->authCardID[6], pChargeCtrl->authCardID[7]);
	dataLen += 16;

	return dataLen;
}




void IotXJ_UpCtrlSendDeal(void)
{
	IotXJSendCtrl_Struct *pSendCtrl = NULL;
	uint8_t port = 0;
	uint8_t index = 0;
	uint16_t dataLen = 0;
	uint32_t rpcSeq = 0;
	uint8_t txbuf[IOT_XJ_TXRX_BUFFER_SIZE] = {0};
	uint16_t cmd = 0;

	if (pIotXJCtx->queueBusyFlag == TRUE)
	{
		if (Common_JudgeTimeoutMs(pIotXJCtx->waitQueueIdleTick, 500))
		{
			pIotXJCtx->queueBusyFlag = FALSE;
		}
	}
	else
	{
		while (1)
		{
			if (pIotXJCtx->sendIndex < ARRAY_SIZE(c_IotXJSendCtrlTable))
			{
				index = pIotXJCtx->sendIndex;
				port = pIotXJCtx->sendPort;

				pSendCtrl = &c_IotXJSendCtrlTable[index];

				if (Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, pSendCtrl->cmd) == TRUE &&
			        IotXJ_ReportCycleCheck(port, pSendCtrl->cmd, pSendCtrl->sendCycle) == TRUE)
				{
					if (pSendCtrl->cmdType == IOT_XJ_CMDTYPE_REQUSET)
					{
						rpcSeq = pIotXJCtx->reqSeq;
						
						if (pSendCtrl->matchCmd != IOT_XJ_CMD_NULL)
						{
                            Common_SetRecvSeq(pIotXJCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, rpcSeq);
						}
						
						pIotXJCtx->reqSeq++;
					}
					else
					{
                        rpcSeq = Common_GetRecvSeq(pIotXJCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd);
					}

					if (NULL != pSendCtrl->pSendFunc)
					{
						dataLen = sizeof(IotXJFrameHead_Struct);
						dataLen = pSendCtrl->pSendFunc(port, &txbuf[dataLen]);
					}

					if (dataLen > 0)
					{
						pIotXJCtx->queueBusyFlag = TRUE;
						pIotXJCtx->waitQueueIdleTick = Common_GetSystick();

						dataLen = IotXJ_PackHead(pSendCtrl->cmd, rpcSeq, txbuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotXJCtx->frameQueueChannelID, pIotXJCtx->platDn, strlen(pIotXJCtx->platDn), txbuf, dataLen, 0))
                        {
                            if (pSendCtrl->cmdType == IOT_XJ_CMDTYPE_REQUSET)
                            {
                                pIotXJCtx->reqSeq--;
                            }

                            IOTXJ_CFG_DebugPrint("FrameQueue_PushTx fail...[cmd: %03d][dataLen = %d]\r\n", pSendCtrl->cmd, dataLen);
                            break;
                        }

                        if (pSendCtrl->printFlag)
                        {
                            IOTXJ_CFG_DebugPrint("[枪：%d]发送[cmd: %02d, %s][%d]: ", port, pSendCtrl->cmd, pSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txbuf, dataLen);
                        }

                        Common_SetSendFlag(pIotXJCtx->pFuncSendCtrl, port, pSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotXJCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotXJCtx->pFuncSendCtrl, port, pSendCtrl->cmd, Common_GetSystick());
                        
                        if (pSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        }

                        if (pSendCtrl->cmdType == IOT_XJ_CMDTYPE_REQUSET)
                        {
                            if (pSendCtrl->matchCmd != IOT_XJ_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotXJCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
					}
				}
			}

			pIotXJCtx->sendIndex++;

			if (pIotXJCtx->sendIndex >= ARRAY_SIZE(c_IotXJSendCtrlTable))
			{
				pIotXJCtx->sendIndex = 0;
				pIotXJCtx->sendPort++;

				if (pIotXJCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotXJCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotXJCtx->queueBusyFlag == TRUE)
			{
				break;
			}
		}
	}
}



















