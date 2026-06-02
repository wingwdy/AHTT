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

static void IotGN_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataLen = 0;

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_LeakageCurrErr))
    {
        Common_SetBitFlag(pBuf, 0);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCalcErr))
    {
        Common_SetBitFlag(pBuf, 1);
        Common_SetBitFlag(pBuf, 15);
    }
    
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault))
    {
        Common_SetBitFlag(pBuf, 5);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation))
    {
        Common_SetBitFlag(pBuf, 6);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor))
    {
        Common_SetBitFlag(pBuf, 7);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr))
    {
        Common_SetBitFlag(pBuf, 8);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputOverVol))
    {
        Common_SetBitFlag(pBuf, 9);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputLessVol))
    {
        Common_SetBitFlag(pBuf, 10);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpGroundFault))
    {
        Common_SetBitFlag(pBuf, 11);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_PEBreakFault))
    {
        Common_SetBitFlag(pBuf, 12);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_DiodeStop))
    {
        Common_SetBitFlag(pBuf, 13);
        Common_SetBitFlag(pBuf, 15);
    }

    if (TRUE != Common_GetBitFlag(pBuf, 15))
    {
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EmergencyStop))
        {
            Common_SetBitFlag(pBuf, 1);
        }
        else if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCommErr))
        {
            Common_SetBitFlag(pBuf, 7);
        }
        else if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr))
        {
            Common_SetBitFlag(pBuf, 8);
        }
        else
        {}
    }
}

static uint16_t IotGN_SendRealData(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);

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
    /* 枪线温度 */
    pBuf[dataLen++] = AswChargeIf_GetGunTemperature(port);
    /* 枪线编码 */
    memset(&pBuf[dataLen], 0x00, 8);
    dataLen += 8;
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
    memset(&pBuf[dataLen], 0x00, 2);
    IotGN_SetRealDataErrBit(port, &pBuf[dataLen]);
    dataLen += 2;
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

static uint16_t IotGN_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotGNFrameHead_Struct *pFrameHead = (IotGNFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotGNFrameHead_Struct);
    uint16_t crc16 = 0;
    uint8_t head1 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD1 : IOT_GN_HEAD1; 
    uint8_t head2 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD2 : IOT_GN_HEAD2; 

    pFrameHead->head[0] = head1;
    pFrameHead->head[1] = head2;
    Common_Uint16ToTwoUint8(pFrameHead->version, IOT_GN_PROTOCOL_VERSION);
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

                if ((Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotGN_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
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

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotGNCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
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





















