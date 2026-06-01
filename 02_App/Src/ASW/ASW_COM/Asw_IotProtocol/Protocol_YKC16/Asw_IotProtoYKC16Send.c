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
#include "Asw_IotProtoYKC16M.h"
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
static uint16_t IotYKC16_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendHeartBeat(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendBillModeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendRealData(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendChargeStartRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendChargeStopRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendOrderRecordReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendParaSetRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendSetBillMode4RateRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendSetRebootRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC16_SendUpdateRsp(uint8_t port, uint8_t *pBuf);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotYKC16Ctx_Struct *pIotYKC16Ctx;

static const IotYKC16SendCtrl_Struct c_stIotYKC16SendctrlTable[IOT_YKC16_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_YKC16_CMD_LOGIN_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_LOGIN_RSP,
        .pSendFunc = IotYKC16_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_YKC16_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_HEARTBEAT_RSP,
        .pSendFunc = IotYKC16_SendHeartBeat,
        .sendCycle = 10000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },

    [2] = 
    {
        .cmd = IOT_YKC16_CMD_BILLMODE_VERIFY_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_BILLMODE_VERIFY_RSP,
        .pSendFunc = IotYKC16_SendBillModeVerifyReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证请求"
    },

    [3] = 
    {
        .cmd = IOT_YKC16_CMD_BILLMODE_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_BILLMODE_4RATE_RSP,
        .pSendFunc = IotYKC16_SendBillModeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求"
    },

    [4] = 
    {
        .cmd = IOT_YKC16_CMD_REPORT_REALDATA,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_NULL,
        .pSendFunc = IotYKC16_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,  
        .cMeaning = "主动上报实时数据"
    },

    [5] = 
    {
        .cmd = IOT_YKC16_CMD_CALL_REALDATA_ACK,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_CALL_REALDATA,
        .pSendFunc = IotYKC16_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "实时数据召测应答"
    },

    [6] = 
    {
        .cmd = IOT_YKC16_CMD_REMOTE_START_CHARGE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_REMOTE_START_CHARGE,
        .pSendFunc = IotYKC16_SendChargeStartRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电应答"
    },

    [7] = 
    {
        .cmd = IOT_YKC16_CMD_REMOTE_STOP_CHARGE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_REMOTE_STOP_CHARGE,
        .pSendFunc = IotYKC16_SendChargeStopRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电应答"
    },

    [8] = 
    {
        .cmd = IOT_YKC16_CMD_ORDER_RECORD_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_ORDER_RECORD_RSP,
        .pSendFunc = IotYKC16_SendOrderRecordReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "四类电价交易记录"
    },

    [9] = 
    {
        .cmd = IOT_YKC16_CMD_PILE_START_CHARGE_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC16_CMD_PILE_START_CHARGE_RSP,
        .pSendFunc = IotYKC16_SendPileStartChargeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动申请启动充电"
    },

    [10] = 
    {
        .cmd = IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY,
        .pSendFunc = IotYKC16_SendUpdateAccountMoneyRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额应答"
    },

    [11] = 
    {
        .cmd = IOT_YKC16_CMD_Para_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_Para_REQ,
        .pSendFunc = IotYKC16_SendParaSetRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "工作参数设置应答"
    },

    [12] = 
    {
        .cmd = IOT_YKC16_CMD_SYNC_TIME_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_SYNC_TIME,
        .pSendFunc = IotYKC16_SendSyncTimeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程对时应答"
    },

    [13] = 
    {
        .cmd = IOT_YKC16_CMD_SET_BILLMODE_4RATE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_SET_BILLMODE_4RATE,
        .pSendFunc = IotYKC16_SendSetBillMode4RateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置四类电价计费模型应答"
    },

    [14] =
    {
        .cmd = IOT_YKC16_CMD_SET_QRCODE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_SET_QRCODE,
        .pSendFunc = IotYKC16_SendSetQrcodeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置二维码应答"
    },

    [15] =
    {
        .cmd = IOT_YKC16_CMD_REBOOT_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_REBOOT,
        .pSendFunc = IotYKC16_SendSetRebootRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置远程重启应答"
    },

    [16] =
    {
        .cmd = IOT_YKC16_CMD_UPDATE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC16_CMD_UPDATE,
        .pSendFunc = IotYKC16_SendUpdateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新应答"
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static uint8_t IotYKC16_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotYKC16Ctx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotYKC16Ctx->pFuncSendCtrl, port, cmd);
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

static uint16_t IotYKC16_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();
    uint8_t versionLen = 0;
    uint8_t cSimID[20] = {0};

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 设备类型 交流桩*/
    pBuf[dataLen++] = 0x01;
    /* 充电枪数量*/
    pBuf[dataLen++] = SYSCFG_CFG_GUN_NUM;
    /* 通信协议版本 */
    if (AswPlatM_GetPlatType() == eAswPlatType_TT24)
    {
        pBuf[dataLen++] = IOT_TT24_PROTOCOL_VERSION;
    }
    else
    {
        pBuf[dataLen++] = IOT_YKC16_PROTOCOL_VERSION;
    }
    /* 程序版本 */
    versionLen = strlen(APP_SW_VERSION_STRING) > 8 ? 8 : strlen(APP_SW_VERSION_STRING);
    memset(&pBuf[dataLen], 0x00, 8);
    memcpy(&pBuf[dataLen], APP_SW_VERSION_STRING, versionLen); 
    dataLen += 8;
    /* 网络连接类型  sim卡*/
    pBuf[dataLen++] = 0x00;
    /* Sim 卡 */
    CddNetM_GetIccid(&cSimID[0]);
    Common_AsciiToBCD((char *)cSimID, &pBuf[dataLen], 20);
    dataLen += 10;
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

static uint16_t IotYKC16_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 状态 */
    pBuf[dataLen++] = (AswErrHandle_IsExsistError(port) == TRUE) ? 0x01 : 0x00;
    return dataLen;
}

static uint16_t IotYKC16_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC16Param.stBillMode;
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 计费模型ID */
    memcpy(&pBuf[dataLen], pBillMode->billModeID, sizeof(pBillMode->billModeID));
    dataLen += sizeof(pBillMode->billModeID);

    return dataLen;
}

static uint16_t IotYKC16_SendBillModeReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    return dataLen;
}

static void IotYKC16_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
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
            Common_SetBitFlag(pBuf, 0);
        }
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCommErr))
        {
            Common_SetBitFlag(pBuf, 6);
        }
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr))
        {
            Common_SetBitFlag(pBuf, 7);
        }
        else
        {}
    }
}

static void IotTT24_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataLen = 0;

    if (TRUE != AswErrHandle_CheckErrExit(port, eErr_LeakageCurrErr))
    {
        Common_SetBitFlag(pBuf, 0);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCalcErr))
    {
        Common_SetBitFlag(pBuf, 1);
        Common_SetBitFlag(pBuf, 23);
    }
    
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault))
    {
        Common_SetBitFlag(pBuf, 5);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation))
    {
        Common_SetBitFlag(pBuf, 6);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor))
    {
        Common_SetBitFlag(pBuf, 7);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr))
    {
        Common_SetBitFlag(pBuf, 8);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputOverVol))
    {
        Common_SetBitFlag(pBuf, 9);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputLessVol))
    {
        Common_SetBitFlag(pBuf, 10);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpGroundFault))
    {
        Common_SetBitFlag(pBuf, 11);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_PEBreakFault))
    {
        Common_SetBitFlag(pBuf, 12);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_DiodeStop))
    {
        Common_SetBitFlag(pBuf, 13);
        Common_SetBitFlag(pBuf, 23);
    }

    if (TRUE != Common_GetBitFlag(pBuf, 23))
    {
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EmergencyStop))
        {
            Common_SetBitFlag(pBuf, 0);
        }
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCommErr))
        {
            Common_SetBitFlag(pBuf, 6);
        }
        if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr))
        {
            Common_SetBitFlag(pBuf, 7);
        }
        else
        {}
    }
}

static uint16_t IotYKC16_SendRealData(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);

    /* 交易流水号 */
    if (orderIdleFlag != TRUE)
    {
        memcpy(&pBuf[dataLen], pIotYKC16Ctx->stProtoData[port].curUsedOrderTransactionNum, 16);
        dataLen += 16;
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 16);
        dataLen += 16;
    }
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 状态 */
    pBuf[dataLen++] = IotYKC16_GetGunState(port);
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
    if (AswPlatM_GetPlatType() == eAswPlatType_TT24)
    {
       IotTT24_SetRealDataErrBit(port, &pBuf[dataLen]);
       dataLen += 3;
    }
    else
    {
        IotYKC16_SetRealDataErrBit(port, &pBuf[dataLen]);
        dataLen += 2;
    }

    return dataLen;
}

static uint16_t IotYKC16_SendChargeStartRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;

    /* 交易流水号 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->stProtoData[port].newRecvOrderTransactionNum, 16);
    dataLen += 16;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 远程启动结果 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].remoteStartResult;
    /* 启动失败原因 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].remoteStartFailReason;
    return dataLen;
}

static uint16_t IotYKC16_SendChargeStopRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;

    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 停止结果 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].remoteStopResult;
    /* 停止失败原因 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].remoteStopFailReason;

    return dataLen;
}

static uint16_t IotYKC16_SendOrderRecordReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmYKC16OrderInfo_Struct *pOrderData = &pIotYKC16Ctx->stOrderInfo.platOrderInfo.stYKC16OrderInfo;
    uint16_t dataLen = 0;

    IotYKC16_TransformChargeRecord(&pIotYKC16Ctx->stOrderInfo.platOrderInfo, pBuf, &dataLen);
    return dataLen;
}

static uint16_t IotYKC16_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 启动方式 01-刷卡*/
    pBuf[dataLen++] = 0x01;
    /* 是否需要密码 */
    pBuf[dataLen++] = 0x00;
    /* 物理卡号（逆序） */
    for (uint8_t i = 0; i < 8; i++)
    {
        pBuf[dataLen + i] = pstChargeCtrl->authCardID[7 - i];
        pIotYKC16Ctx->stProtoData[port].authCardID[i] = pstChargeCtrl->authCardID[7 - i];
    }
    dataLen += 8;
    /* 是否输入密码 */
    memset(&pBuf[dataLen], 0x00, 16);
    dataLen += 16;
    /* VIN码 */
    memset(&pBuf[dataLen], 0x00, 17);
    dataLen += 17;
    return dataLen;
}

static uint16_t IotYKC16_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->stProtoData[port].updateAccountMoneyCardID, 8);
    dataLen += 8;
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].updateAccountMoneyResult;
    return dataLen;
}

static uint16_t IotYKC16_SendParaSetRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[port].setPowerChangeResult;
    return dataLen;
}

static uint16_t IotYKC16_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
{
    CommonDateTime_Struct dateTime;
    uint16_t dataLen = 0;
    uint16_t temp = 0;
    uint32_t SecTimestamp;
    
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 当前时间 CP56Time2a*/
    SecTimestamp = SSTM_GetSecTimestamp();
    Common_TimestampToCp56Time2a(SecTimestamp, &pBuf[dataLen]);
    dataLen += 7;
    
    return dataLen;
}

static uint16_t IotYKC16_SendSetBillMode4RateRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotYKC16_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotYKC16_SendSetRebootRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotYKC16_SendUpdateRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 设置结果 */
    pBuf[dataLen++] = pIotYKC16Ctx->stProtoData[0].setUpdateResult;
    return dataLen;
}

static uint16_t IotYKC16_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    /* 起始标志 数据长度 序列号域 加密标志 帧类型标志 消息体  帧校验域 */
    /*   1 字节  1 字节   2 字节   1 字节   1 字节    N 字节  2 字节 */
    IotYKC16FrameHead_Struct *pFrameHead = (IotYKC16FrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + 4;
    uint16_t crc16Len = totalLen + 2;
    uint16_t crc16 = 0;
    uint8_t head = IOT_YKC16_HEAD; 

    pFrameHead->head = head;
    pFrameHead->dataLen = (uint8_t)totalLen;
    pFrameHead->seq[0] = (uint8_t)((seq >> 8) & 0xFF);   /* 高字节在前 */
    pFrameHead->seq[1] = (uint8_t)(seq & 0xFF);          /* 低字节在后 */
    pFrameHead->encryptFlag = 0;
    pFrameHead->cmd = cmd;
    crc16 = Common_CalcCRC16(&pBuf[2], totalLen);
    pBuf[crc16Len] = (crc16 >> 8) & 0xFF;
    crc16Len += 1;
    pBuf[crc16Len] = (crc16) & 0xFF;
    crc16Len += 1;

    /* 全数据长度 */
    return crc16Len;
}

void IotYKC16_UpCtrlSendDeal(void)
{
    const IotYKC16SendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_YKC16_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotYKC16Ctx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotYKC16Ctx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotYKC16Ctx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotYKC16Ctx->sendIndex < ARRAY_SIZE(c_stIotYKC16SendctrlTable))
            {
                index = pIotYKC16Ctx->sendIndex;
                port = pIotYKC16Ctx->sendPort;

                pCmdSendCtrl = &c_stIotYKC16SendctrlTable[index];

                if ((Common_GetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotYKC16_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    /* 请求使用自增序列号 */
                    if (pCmdSendCtrl->cmdType == IOT_YKC16_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotYKC16Ctx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_YKC16_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotYKC16Ctx->reqSeq++;
                    }
                    /* 响应使用收到的序列号 */
                    else
                    {
                        reqSeq = Common_GetRecvSeq(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotYKC16FrameHead_Struct);
                        /* 数据域长度 */
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        /* 整包长度 */
                        dataLen = IotYKC16_PackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);

                        /* 发送到队列 */
                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotYKC16Ctx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_YKC16_CMDTYPE_REQUSET)
                            {
                                /* 序列号回退 */
                                pIotYKC16Ctx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTYKC16_CFG_DebugPrint("[枪：%d]发送[cmd: 0x%02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());
                                            
                        /* 事件型上送 */
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_YKC16_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_YKC16_CMD_NULL)
                            {
                                /* 启动接收超时计时器 */
                                Common_SetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

            /* 轮询上送 */
			pIotYKC16Ctx->sendIndex++;

			if (pIotYKC16Ctx->sendIndex >= ARRAY_SIZE(c_stIotYKC16SendctrlTable))
			{
				pIotYKC16Ctx->sendIndex = 0;
				pIotYKC16Ctx->sendPort++;

				if (pIotYKC16Ctx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotYKC16Ctx->sendPort = 0;
					break;
				}
			}
			
			if (pIotYKC16Ctx->queueBusyFlag == TRUE)
			{
				break;
			}
        }
    }
}