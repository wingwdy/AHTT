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
#include "Asw_IotProtoGNM.h"
#include "Asw_PlatM.h"
#include "Version.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "SS_Tm.h"
#include "SS_Ucm.h"

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
static uint16_t IotGN_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendHeartBeat(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendBillModeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendRealData(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendChargeStartRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendChargeStopRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendMultyOrderRecordReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendOrderRecordReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSetBillMode4RateRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSetBillModeMultiRateRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSetRebootRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendUpdateRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendOfflineCardRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendOfflineCardClearRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendOfflineCardSearchRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendSetDevWorkParamRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendQueryDevWorkParamRsp(uint8_t port, uint8_t *pBuf);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGNCtx_Struct *pIotGNCtx;

static const IotGNSendCtrl_Struct c_stIotGNSendctrlTable[IOT_GN_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_GN_CMD_LOGIN_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_LOGIN_RSP,
        .pSendFunc = IotGN_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_GN_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_HEARTBEAT_RSP,
        .pSendFunc = IotGN_SendHeartBeat,
        .sendCycle = 10000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },

    [2] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_VERIFY_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_BILLMODE_VERIFY_RSP,
        .pSendFunc = IotGN_SendBillModeVerifyReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证请求"
    },

    [3] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_BILLMODE_4RATE_RSP,
        .pSendFunc = IotGN_SendBillModeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求"
    },

    [4] = 
    {
        .cmd = IOT_GN_CMD_REPORT_REALDATA,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_NULL,
        .pSendFunc = IotGN_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,  
        .cMeaning = "主动上报实时数据"
    },

    [5] = 
    {
        .cmd = IOT_GN_CMD_CALL_REALDATA_ACK,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_CALL_REALDATA,
        .pSendFunc = IotGN_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "实时数据召测应答"
    },

    [6] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_START_CHARGE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_REMOTE_START_CHARGE,
        .pSendFunc = IotGN_SendChargeStartRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电应答"
    },

    [7] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_REMOTE_STOP_CHARGE,
        .pSendFunc = IotGN_SendChargeStopRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电应答"
    },

    [8] = 
    {
        .cmd = IOT_GN_CMD_MULTI_ORDER_RECORD_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_ORDER_RECORD_RSP,
        .pSendFunc = IotGN_SendMultyOrderRecordReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "多类电价交易记录"
    },

    [9] = 
    {
        .cmd = IOT_GN_CMD_ORDER_RECORD_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_ORDER_RECORD_RSP,
        .pSendFunc = IotGN_SendOrderRecordReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "四类电价交易记录"
    },

    [10] = 
    {
        .cmd = IOT_GN_CMD_PILE_START_CHARGE_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_PILE_START_CHARGE_RSP,
        .pSendFunc = IotGN_SendPileStartChargeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动申请启动充电"
    },

    [11] = 
    {
        .cmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY,
        .pSendFunc = IotGN_SendUpdateAccountMoneyRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额应答"
    },

    [12] = 
    {
        .cmd = IOT_GN_CMD_SYNC_TIME_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_SYNC_TIME,
        .pSendFunc = IotGN_SendSyncTimeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程对时应答"
    },

    [13] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_4RATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_4RATE,
        .pSendFunc = IotGN_SendSetBillMode4RateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置四类电价计费模型应答"
    },

    [14] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE,
        .pSendFunc = IotGN_SendSetBillModeMultiRateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置多类电价计费模型应答"
    },

    [15] =
    {
        .cmd = IOT_GN_CMD_SET_QRCODE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_SET_QRCODE,
        .pSendFunc = IotGN_SendSetQrcodeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置二维码应答"
    },

    [16] =
    {
        .cmd = IOT_GN_CMD_REBOOT_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_REBOOT,
        .pSendFunc = IotGN_SendSetRebootRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置远程重启应答"
    },

    [17] =
    {
        .cmd = IOT_GN_CMD_UPDATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_UPDATE,
        .pSendFunc = IotGN_SendUpdateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新应答"
    },

    [18] =
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD,
        .pSendFunc = IotGN_SendOfflineCardRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "离线卡下发应答"
    },

    [19] =
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD_CLEAR_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD_CLEAR,
        .pSendFunc = IotGN_SendOfflineCardClearRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "离线卡清除应答"
    },

    [20] =
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD_SEARCH_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD_SEARCH,
        .pSendFunc = IotGN_SendOfflineCardSearchRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "离线卡查询应答"
    },

    [21] =
    {
        .cmd = IOT_GN_CMD_SET_DEV_WORK_PARAM_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_SET_DEV_WORK_PARAM,
        .pSendFunc = IotGN_SendSetDevWorkParamRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "充电设备工作参数设置应答"
    },

    [22] =
    {
        .cmd = IOT_GN_CMD_QUERY_DEV_WORK_PARAM_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .matchCmd = IOT_GN_CMD_QUERY_DEV_WORK_PARAM,
        .pSendFunc = IotGN_SendQueryDevWorkParamRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "充电设备工作参数查询应答"
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static uint8_t IotGN_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotGNCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, cmd);
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

static uint16_t IotGN_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint32_t randomNum = 0;
    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();

    srand(Common_GetSystick());
    randomNum = rand();
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 设备识别码 */
#if (SYSCFG_CFG_GUN_NUM == 1)
    sprintf((char *)&pBuf[dataLen], "%s", SYSCFG_CFG_PRODUCT_CODE);
    dataLen += 16;
#else
    sprintf((char *)&pBuf[dataLen], "%s2", SYSCFG_CFG_PRODUCT_CODE);
    dataLen += 16;
#endif
    /* 随机数 */
    memcpy(&pBuf[dataLen], &randomNum, 4);
    dataLen += 4;
    /* 验证密钥 */
    memset(&pBuf[dataLen], 0x00, 16);
    dataLen += 16;
    /* 设备类型 交流桩*/
    pBuf[dataLen++] = 0x01;
    /* 充电枪数量*/
    pBuf[dataLen++] = SYSCFG_CFG_GUN_NUM;
    /* 程序版本 */
    pBuf[dataLen++] = APP_SW_MAJOR_VERSION;
    pBuf[dataLen++] = APP_SW_MINOR_VERSION;
    pBuf[dataLen++] = APP_SW_CUSTORM_VERSION;
    pBuf[dataLen++] = APP_SW_PATCH_VERSION;
    /* 网络连接类型  sim卡*/
    pBuf[dataLen++] = 0x00;
    /* ICCID */
    CddNetM_GetIccid(&pBuf[dataLen]);
    dataLen += 20;
    /* 运营商 */
    if (eOperator == eCddNetMOperator_CMCC)
        pBuf[dataLen++] = 0x00;
    else if (eOperator == eCddNetMOperator_CTCC)
        pBuf[dataLen++] = 0x01;
    else if (eOperator == eCddNetMOperator_CUCC)
        pBuf[dataLen++] = 0x02;
    else
        pBuf[dataLen++] = 0xFF;

    return dataLen;
}

static uint16_t IotGN_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 状态 */
    pBuf[dataLen++] = (AswErrHandle_IsExsistError(port) == TRUE) ? 0x01 : 0x00;
    return dataLen;
}

static uint16_t IotGN_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    memcpy(&pBuf[dataLen], pBillMode->billModeID, sizeof(pBillMode->billModeID));
    dataLen += sizeof(pBillMode->billModeID);

    return dataLen;
}

static uint16_t IotGN_SendBillModeReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    return dataLen;
}

static uint16_t IotGN_SendRealData(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);
    uint32_t errBitMap = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 交易流水号 */
    if (orderIdleFlag != TRUE)
    {
        memcpy(&pBuf[dataLen], pIotGNCtx->stProtoData[port].curUsedOrderTransactionNum, 16);
        dataLen += 16;
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 16);
        dataLen += 16;
    }

    /* 状态 */
    pBuf[dataLen++] = IotGN_GetGunState(port);
    /* 枪是否归位 */
    pBuf[dataLen++] = 02;
    /* 是否插枪 */
    pBuf[dataLen++] = (AswChargeIf_CheckGunConnected(port) == TRUE) ? 0x01 : 0x00;
    /* 输出电压 */
    Common_Uint16ToTwoUint8(&pBuf[dataLen], AswChargeIf_GetInputVoltage(port) / 10);
    dataLen += 2;
    /* 输出电流 */
    Common_Uint16ToTwoUint8(&pBuf[dataLen], AswChargeIf_GetOutputCurrent(port) / 100);
    dataLen += 2;
    if (eAswPlatType_DXL == AswPlatM_GetPlatType())
    {
        /* 输出功率 */
        Common_Uint16ToTwoUint8(&pBuf[dataLen], AswChargeIf_GetOutputPower(port) / 100);
        dataLen += 2;
        /* 枪头温度 */
        pBuf[dataLen++] = AswChargeIf_GetGunTemperature(port);
        /* 壳体温度 */
        pBuf[dataLen++] = AswChargeIf_GetEnvTemperature();

    }
    else
    {
        /* 枪线温度 */
        pBuf[dataLen++] = AswChargeIf_GetGunTemperature(port);
        /* 枪线编码 */
        memset(&pBuf[dataLen], 0x00, 8);
        dataLen += 8;
    }
    /* SOC */
    pBuf[dataLen++] = 0x00;
    /* 电池组最高温度 */
    pBuf[dataLen++] = 0x00;

    /* 累计充电时间 */
    if (orderIdleFlag != TRUE)
    {
        Common_Uint16ToTwoUint8(&pBuf[dataLen], pChargeData->chargeTime / 60);
        dataLen += 2;
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 2);
        dataLen += 2;
    }

    /* 剩余时间 */
    memset(&pBuf[dataLen], 0x00, 2);
    dataLen += 2;

    if (orderIdleFlag != TRUE)
    {
        /* 充电度数 */
        memcpy(&pBuf[dataLen], &pChargeData->totalEnergy, 4);
        dataLen += 4;
        /* 计损充电度数 */
        memcpy(&pBuf[dataLen], &pChargeData->totalLossEnergy, 4);
        dataLen += 4;
        /* 已充金额 */
        memcpy(&pBuf[dataLen], &pChargeData->totalMoney, 4);
        dataLen += 4;
    }
    else
    {
        /* 充电度数 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
        /* 计损充电度数 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
        /* 已充金额 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
    }

    /* 硬件故障 */
    if (eAswPlatType_DXL == AswPlatM_GetPlatType())
    {
        memset(&pBuf[dataLen], 0x00, 32);
        memcpy(&pBuf[dataLen], pIotGNCtx->lastErrInfo[port], 32);
        dataLen += 32;

        /* 4G信号强度 */
        pBuf[dataLen++] = (uint8_t)CddNetM_GetCsq();
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 2);
        memcpy(&pBuf[dataLen], pIotGNCtx->lastErrInfo[port], 2);
        dataLen += 2;
    }
    return dataLen;
}

static uint16_t IotGN_SendChargeStartRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 交易流水号 */
    memcpy(&pBuf[dataLen], pIotGNCtx->stProtoData[port].newRecvOrderTransactionNum, 16);
    dataLen += 16;
    /* 远程启动结果 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].remoteStartResult;
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].remoteStartFailReason;
    return dataLen;
}

static uint16_t IotGN_SendChargeStopRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 远程启动结果 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].remoteStopResult;
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].remoteStopFailReason;

    return dataLen;
}

static uint16_t IotGN_SendMultyOrderRecordReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmGNOrderInfo_Struct *pOrderData = &pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo;
    uint16_t dataLen = 0;

    IotGN_TransformChargeRecord(&pIotGNCtx->stOrderInfo.platOrderInfo, pBuf, &dataLen);
    return dataLen;
}

static uint16_t IotGN_SendOrderRecordReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmGNOrderInfo_Struct *pOrderData = &pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo;
    uint16_t dataLen = 0;

    IotGN_TransformChargeRecord(&pIotGNCtx->stOrderInfo.platOrderInfo, pBuf, &dataLen);
    return dataLen;
}

static uint16_t IotGN_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 启动方式 01-刷卡*/
    pBuf[dataLen++] = 0x01;
    /* 是否需要密码 */
    pBuf[dataLen++] = 0x00;
    /* 卡号 */
    memcpy(&pBuf[dataLen], pstChargeCtrl->authCardID, 8);
    dataLen += 8;
    /* 是否输入密码 */
    memset(&pBuf[dataLen], 0x00, 16);
    dataLen += 16;
    /* VIN码 */
    memset(&pBuf[dataLen], 0x00, 17);
    dataLen += 17;
    return dataLen;
}

static uint16_t IotGN_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    memcpy(&pBuf[dataLen], pIotGNCtx->stProtoData[port].updateAccountMoneyCardID, 8);
    dataLen += 8;
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].updateAccountMoneyResult;
    return dataLen;
}

static uint16_t IotGN_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
{
    CommonDateTime_Struct dateTime;
    uint16_t dataLen = 0;
    uint16_t temp = 0;
    
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    SSTM_GetDateTime(&dateTime);
    temp = Common_uintBINToBCD(dateTime.year);
    pBuf[dataLen++] = (temp >> 8) & 0xFF;
    pBuf[dataLen++] = (uint8_t)(temp);
    Common_BINToBCD(&dateTime.month, (uint8_t *)&temp, 1);
    pBuf[dataLen++] = (uint8_t)(temp);
    Common_BINToBCD(&dateTime.day, (uint8_t *)&temp, 1);
    pBuf[dataLen++] = (uint8_t)(temp);
    Common_BINToBCD(&dateTime.hour, (uint8_t *)&temp, 1);
    pBuf[dataLen++] = (uint8_t)(temp);
    Common_BINToBCD(&dateTime.minute, (uint8_t *)&temp, 1);
    pBuf[dataLen++] = (uint8_t)(temp);
    Common_BINToBCD(&dateTime.second, (uint8_t *)&temp, 1);
    pBuf[dataLen++] = (uint8_t)(temp);
    return dataLen;
}

static uint16_t IotGN_SendSetBillMode4RateRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotGN_SendSetBillModeMultiRateRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotGN_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotGN_SendSetRebootRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotGN_SendUpdateRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[0].setUpdateResult;
    return dataLen;
}

static uint16_t IotGN_SendOfflineCardRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 保存结果 0-失败 1-成功 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].offlineCardSaveResult;
    /* 失败原因 0x01-卡号格式错误 0x02-存储空间不足 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].offlineCardFailReason;
    return dataLen;
}

static uint16_t IotGN_SendOfflineCardClearRsp(uint8_t port, uint8_t *pBuf)
{
    IotGNOfflineCardClearResult_Struct *pClearResult = NULL;
    uint16_t dataLen = 0;
    uint8_t cardIndex = 0;
    uint8_t clearCount = pIotGNCtx->stProtoData[port].offlineCardClearCount;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    /* 清除的离线卡个数*/
    pBuf[dataLen++] = clearCount;

    /* 逐张卡填充应答 */
    for (cardIndex = 0; cardIndex < clearCount; cardIndex++)
    {
        pClearResult = &pIotGNCtx->stProtoData[port].offlineCardClearResults[cardIndex];
        /* 卡号 */
        memcpy(&pBuf[dataLen], pClearResult->cardID, 8);
        dataLen += 8;
        /* 清除标记 0-失败 1-成功 */
        pBuf[dataLen++] = pClearResult->clearResult;
        /* 失败原因 0x01-卡号格式错误 0x02-清除成功 */
        pBuf[dataLen++] = pClearResult->failReason;
    }

    return dataLen;
}

static uint16_t IotGN_SendOfflineCardSearchRsp(uint8_t port, uint8_t *pBuf)
{
    IotGNOfflineCardSearchResult_Struct *pSearchResult = NULL;
    uint16_t dataLen = 0;
    uint8_t cardIndex = 0;
    uint8_t searchCount = pIotGNCtx->stProtoData[port].offlineCardSearchCount;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;

    /* 查询的离线卡数量 */
    pBuf[dataLen++] = searchCount;

    /* 逐张卡填充应答 */
    for (cardIndex = 0; cardIndex < searchCount; cardIndex++)
    {
        pSearchResult = &pIotGNCtx->stProtoData[port].offlineCardSearchResults[cardIndex];
        /* 卡号 */
        memcpy(&pBuf[dataLen], pSearchResult->cardID, 8);
        dataLen += 8;
        /* 查询结果 0-不存在 1-存在 */
        pBuf[dataLen++] = pSearchResult->searchResult;
    }

    return dataLen;
}

static uint16_t IotGN_SendSetDevWorkParamRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 设置结果 0-失败 1-成功 */
    pBuf[dataLen++] = pIotGNCtx->stProtoData[port].setDevWorkParamResult;

    return dataLen;
}

static uint16_t IotGN_SendQueryDevWorkParamRsp(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNWorkParam_Struct *pWorkParam = &pPrivateParam->stGNParam.stWorkParam;
    MSNvmPlatParam_Struct *pPlatParam = AswPlatM_GetPlatParamPtr();
    uint16_t dataLen = 0;
    uint8_t paramIndex = 0;
    uint8_t paramId = 0;
    uint8_t paramCount = pIotGNCtx->stProtoData[port].queryDevWorkParamCount;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 查询结果 0-失败 1-成功 */
    pBuf[dataLen++] = 0x01;
    /* 参数个数 */
    pBuf[dataLen++] = paramCount;

    for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
    {
        /* 参数编号 */
        paramId = pIotGNCtx->stProtoData[port].queryDevWorkParamIds[paramIndex];
        
        pBuf[dataLen++] = paramId;

        switch (paramId)
        {
            case 0x01:  /* 授权配置: 1字节 */
                pBuf[dataLen++] = 1;
                pBuf[dataLen++] = pWorkParam->authConfig;
                break;

            case 0x02:  /* 服务器域名: 从实际连接的PlatParam取，保证查询应答与实际连接一致 */
                pBuf[dataLen++] = MSNVM_GN_SERVER_DOMAIN_LEN;
                memcpy((char *)&pBuf[dataLen], pPlatParam->platMainIp, sizeof(pPlatParam->platMainIp));
                dataLen += MSNVM_GN_SERVER_DOMAIN_LEN;
                break;

            case 0x03:  /* 服务器端口号: 从实际连接的PlatParam取，保证查询应答与实际连接一致 */
                pBuf[dataLen++] = 2;
                Common_Uint16ToTwoUint8(&pBuf[dataLen], pPlatParam->platMainPort);
                dataLen += 2;
                break;

            case 0x04:  /* DXL待机状态实时数据上报周期: 1字节BIN, 单位: min */
                pBuf[dataLen++] = 1;
                pBuf[dataLen++] = pWorkParam->idleReportCycle;
                break;

            case 0x05:  /* DXL充电状态实时数据上报周期: 1字节BIN, 单位: s */
                pBuf[dataLen++] = 1;
                pBuf[dataLen++] = pWorkParam->chargingReportCycle;
                break;

            default:
                break;
        }
    }

    return dataLen;
}

static uint16_t IotGN_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotGNFrameHead_Struct *pFrameHead = (IotGNFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotGNFrameHead_Struct);
    uint16_t crc16 = 0;
    uint8_t head1 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD1 : IOT_GN_HEAD1; 
    uint8_t head2 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD2 : IOT_GN_HEAD2; 

    pFrameHead->head[0] = head1;
    pFrameHead->head[1] = head2;
    if (AswPlatM_GetPlatType() == eAswPlatType_DXL)
    {
        Common_Uint16ToTwoUint8(pFrameHead->version, IOT_DXL_PROTOCOL_VERSION);
    }    
    else
    {
        Common_Uint16ToTwoUint8(pFrameHead->version, IOT_GN_PROTOCOL_VERSION);
    }
    Common_Uint16ToTwoUint8(pFrameHead->seq, seq);
    pFrameHead->encryptFlag = 0;
    pFrameHead->cmd = cmd;
    Common_Uint16ToTwoUint8(pFrameHead->dataLen, totalLen + 2);
    crc16 = Common_CalcCRC16(pBuf, totalLen);
    pBuf[totalLen] = (crc16 >> 8) & 0xFF;
    totalLen += 1;
    pBuf[totalLen] = (crc16) & 0xFF;
    totalLen += 1;
    return totalLen;
}

void IotGN_UpCtrlSendDeal(void)
{
    const IotGNSendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint32_t sendCycle = 0;
    uint8_t txBuf[IOT_GN_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotGNCtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotGNCtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotGNCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotGNCtx->sendIndex < ARRAY_SIZE(c_stIotGNSendctrlTable))
            {
                index = pIotGNCtx->sendIndex;
                port = pIotGNCtx->sendPort;

                pCmdSendCtrl = &c_stIotGNSendctrlTable[index];
                
                sendCycle = pCmdSendCtrl->sendCycle;
                if (eAswPlatType_DXL == AswPlatM_GetPlatType() && pCmdSendCtrl->cmd == IOT_GN_CMD_HEARTBEAT_REQ)
                {
                    sendCycle = 60000;  /* DXL心跳周期 60s */
                }

                if ((Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotGN_ReportCycleCheck(port, pCmdSendCtrl->cmd, sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotGNCtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_GN_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotGNCtx->reqSeq++;
                    }
                    else
                    {
                        reqSeq = Common_GetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotGNFrameHead_Struct);
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        pIotGNCtx->queueBusyFlag = TRUE;
						pIotGNCtx->waitQueueIdleTick = Common_GetSystick();
                        
                        dataLen = IotGN_PackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotGNCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen, 0))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                            {
                                pIotGNCtx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTGN_CFG_DebugPrint("[枪：%d]发送[cmd: 0x%02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());
                                            
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_GN_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

			pIotGNCtx->sendIndex++;

			if (pIotGNCtx->sendIndex >= ARRAY_SIZE(c_stIotGNSendctrlTable))
			{
				pIotGNCtx->sendIndex = 0;
				pIotGNCtx->sendPort++;

				if (pIotGNCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotGNCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotGNCtx->queueBusyFlag == TRUE)
			{
				break;
			}
        }
    }
}





















