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
#include "Asw_IotProtoOMM.h"
#include "Asw_IotProtoOMSend.h"
#include "FrameQueue.h"
#include "Version.h"
#include "Asw_Errorhandle.h"
#include "Asw_Monitor.h"
#include "Asw_ChargeIf.h"
#include "Asw_PlatM.h"
#include "Cdd_ModeM.h"
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
static uint8_t IotOM_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc);
static uint16_t IotOM_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendHeartBeat(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendNetModuleInfo(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendRealData(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendMeterVal(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendRebootRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendSetForbidRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendSetForbidState(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendUpdateResponse(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendOrderRecord(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendRemoteQuerySetParamRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendReadLocalFileRsp(uint8_t port, uint8_t *pBuf);
static void IotOM_SetRealDataErrBit(uint8_t port, uint8_t *pBuf);   
static uint16_t IotOM_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf, uint16_t dataLen);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotOMCtx_Struct *pIotOMCtx;

static const IotOMSendCtrl_Struct c_stIotOMSendctrlTable[IOT_OM_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_OM_CMD_LOGIN_REQ,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_LOGIN_RSP,
        .pSendFunc = IotOM_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_OM_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_HEARTBEAT_RSP,
        .pSendFunc = IotOM_SendHeartBeat,
        .sendCycle = 30000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },

    [2] = 
    {
        .cmd = IOT_OM_CMD_SEND_NETMODULE_INFO,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_NULL,
        .pSendFunc = IotOM_SendNetModuleInfo,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "主动上报网络模块信息"
    },

    [3] = 
    {
        .cmd = IOT_OM_CMD_CALL_NETMODULE_INFO_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_CALL_NETMODULE_INFO,
        .pSendFunc = IotOM_SendNetModuleInfo,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "网络模块信息应答"
    },

    [4] = 
    {
        .cmd = IOT_OM_CMD_CALL_REALDATA_ACK,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_CALL_REALDATA,
        .pSendFunc = IotOM_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据应答"
    },

    [5] = 
    {
        .cmd = IOT_OM_CMD_REPORT_REALDATA,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_NULL,
        .pSendFunc = IotOM_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "主动上报实时数据"
    },

    [6] = 
    {
        .cmd = IOT_OM_CMD_REPORT_METERVAL,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_NULL,
        .pSendFunc = IotOM_SendMeterVal,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "上报电表底数"
    },

    [7] = 
    {
        .cmd = IOT_OM_CMD_SET_QRCODE_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_SET_QRCODE,
        .pSendFunc = IotOM_SendSetQrcodeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置二维码应答"
    },

    [8] = 
    {
        .cmd = IOT_OM_CMD_REBOOT_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_REBOOT,
        .pSendFunc = IotOM_SendRebootRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程重启应答"
    },

    [9] = 
    {
        .cmd = IOT_OM_CMD_SET_FORBID_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_SET_FORBID,
        .pSendFunc = IotOM_SendSetForbidRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程锁机应答"
    },

    [10] = 
    {
        .cmd = IOT_OM_CMD_REPORT_FORBID_STATE,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_REPORT_FORBID_STATE_RSP,
        .pSendFunc = IotOM_SendSetForbidState,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程锁机状态上报"
    },

    [11] = 
    {
        .cmd = IOT_OM_CMD_UPDATE_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_UPDATE,
        .pSendFunc = IotOM_SendUpdateResponse,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新应答"
    },

    [12] = 
    {
        .cmd = IOT_OM_CMD_ORDER_RECORD,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_ORDER_RECORD_RSP,
        .pSendFunc = IotOM_SendOrderRecord,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "订单数据上报"
    },

    [13] = 
    {
        .cmd = IOT_OM_CMD_REMOTE_QUERY_SET_PARAM_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_REMOTE_QUERY_SET_PARAM,
        .pSendFunc = IotOM_SendRemoteQuerySetParamRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程设置查询参数应答"
    },

    [14] = 
    {
        .cmd = IOT_OM_CMD_CALL_READ_LOCALFILE_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .matchCmd = IOT_OM_CMD_CALL_READ_LOCALFILE,
        .pSendFunc = IotOM_SendReadLocalFileRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程读取桩本地文件应答"
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotOM_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotOMCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, port, cmd);
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

static uint16_t IotOM_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 运维平台桩号 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 运营平台桩号 */
    memcpy(&pBuf[dataLen], pIotOMCtx->platDn, 32);
    dataLen += 32;
    /* 设备类型 */
    pBuf[dataLen++] = 0x01;
    /* 终端数量 */
    pBuf[dataLen++] = SYSCFG_CFG_GUN_NUM;
    /* 软件版本 */
    pBuf[dataLen++] = APP_SW_MAJOR_VERSION;
    pBuf[dataLen++] = APP_SW_MINOR_VERSION;
    pBuf[dataLen++] = APP_SW_CUSTORM_VERSION;
    pBuf[dataLen++] = APP_SW_PATCH_VERSION;

    /* A硬件版本-充电模块 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* A软件版本-充电模块 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* B硬件版本-灯板 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* B软件版本-灯板 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* C硬件版本-网络模块 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* C软件版本-网络模块 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
    /* 签名算法 */
    memset(&pBuf[dataLen], 0x00, 16);
    dataLen += 16;
    return dataLen;
}

static uint16_t IotOM_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint8_t gunNo = 0;

    /* 运维平台桩号 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 设备状态 */
    pBuf[dataLen++] = AswErrHandle_IsExsistError(port);
    return dataLen;
}

static uint16_t IotOM_SendNetModuleInfo(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();

    /* 运维平台桩号 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 网络连接类型 */
    pBuf[dataLen++] = 0x00;
    /* 运营商 */
    if (eOperator == eCddNetMOperator_CMCC)
        pBuf[dataLen++] = 0x00;
    else if (eOperator == eCddNetMOperator_CTCC)
        pBuf[dataLen++] = 0x01;
    else if (eOperator == eCddNetMOperator_CUCC)
        pBuf[dataLen++] = 0x02;
    else
        pBuf[dataLen++] = 0xFF;
    /* ICCID */
    CddNetM_GetIccid(&pBuf[dataLen]);
    dataLen += 20;
    /* 获取网络模块类型信息 */
    memset(&pBuf[dataLen], 0x00, 20);
    CddNetM_GetModuleTypeInfo((char *)&pBuf[dataLen], 20);
    dataLen += 20;
    /* mac地址 */
    memset(&pBuf[dataLen], 0x00, 20);
    dataLen += 20;
    return dataLen;
}

static void IotOM_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
{
    /* CP电压异常 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor))
    {
        Common_SetBitFlag(pBuf, 1);
    }

    /* CP接地 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpGroundFault))
    {
        Common_SetBitFlag(pBuf, 2);
    }

    /* PE故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_PEBreakFault))
    {
        Common_SetBitFlag(pBuf, 3);
    }

    /* 缺相 */
    /* 急停 */

    /* 火零反接 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_InputLineReversed))
    {
        Common_SetBitFlag(pBuf, 6);
    }

    /* 漏电故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_LeakageCurrErr))
    {
        Common_SetBitFlag(pBuf, 7);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_RCDSelfcheckErr))
    {
        Common_SetBitFlag(pBuf, 7);
    }

    /* 二极管不存在故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_DiodeStop))
    {
        Common_SetBitFlag(pBuf, 8);
    }

    /* 短路故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ShortCircleErr))
    {
        Common_SetBitFlag(pBuf, 9);
    }

    /* 过压 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputOverVol))
    {
        Common_SetBitFlag(pBuf, 10);
    }    

    /* 欠压 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_AphaseInputLessVol))
    {
        Common_SetBitFlag(pBuf, 11);
    } 

    /* 过流 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr))
    {
        Common_SetBitFlag(pBuf, 12);
    }

    /* 继电器粘连 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault))
    {
        Common_SetBitFlag(pBuf, 13);
    }
    
    /* 继电器拒动 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation))
    {
        Common_SetBitFlag(pBuf, 14);
    }

    /* 环境过温 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EnvOverTempErr))
    {
        Common_SetBitFlag(pBuf, 15);
    }

    /* 枪过温 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_GunOverTempErr))
    {
        Common_SetBitFlag(pBuf, 16);
    }

    /* 插头过温故障 */
    /* 电表通信异常故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCommErr))
    {
        Common_SetBitFlag(pBuf, 24);
    }
    /* 读卡器通信异常 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr))
    {
        Common_SetBitFlag(pBuf, 25);
    }
    /* CCU通信异常 */
    /* 存储异常 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_DatabaseErr))
    {
        Common_SetBitFlag(pBuf, 27);
    }
}

static uint16_t IotOM_SendRealData(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;
    uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);
    uint32_t temp = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 状态 */
    pBuf[dataLen++] = IotOM_GetGunState(port);
    /* 是否插枪 */
    pBuf[dataLen++] = (AswChargeIf_CheckGunConnected(port) == TRUE) ? 0x01 : 0x00;
    /* 枪线温度 */
    pBuf[dataLen++] = AswChargeIf_GetGunTemperature(port);
    
    if (orderIdleFlag != TRUE)
    {
        /* 累计充电时间 */
        Common_Uint16ToTwoUint8(&pBuf[dataLen], pChargeData->chargeTime / 60);
        dataLen += 2;
        /* 充电度数 */
        Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalEnergy / 10);
        dataLen += 4;
        /* SOC */
        pBuf[dataLen++] = 0xFF;
        /* 已充金额 */
        memcpy(&pBuf[dataLen], &pChargeData->totalMoney, 4);
        dataLen += 4;
    }
    else
    {
        /* 累计充电时间 */
        memset(&pBuf[dataLen], 0x00, 2);
        dataLen += 2;
        /* 充电度数 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
        /* SOC */
        pBuf[dataLen++] = 0xFF;
        /* 已充金额 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
    }

    /* 桩硬件故障 */
    memset(&pBuf[dataLen], 0x00, 2);
    dataLen += 2;

    /* 硬件故障 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;

    /* 故障码 */
    memset(&pBuf[dataLen], 0x00, 32);
    IotOM_SetRealDataErrBit(port, &pBuf[dataLen]);
    dataLen += 32;
    return dataLen;
}

static uint16_t IotOM_SendMeterVal(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 有功功率 */
    Common_Uint32ToFourUint8(&pBuf[dataLen], AswChargeIf_GetOutputPower(port) * 10);
    dataLen += 4;
    /* 电表底数 */
    Common_Uint32ToFourUint8(&pBuf[dataLen], AswChargeIf_GetMeterEnergyVal(port) / 10);
    dataLen += 4;

    return dataLen;
}

static uint16_t IotOM_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 设置结果 */
    pBuf[dataLen++] = 0x01;
    return dataLen;
}

static uint16_t IotOM_SendRebootRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 设置结果 */
    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setRebootResult;
    return dataLen;
}

static uint16_t IotOM_SendSetForbidRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;

    /* 设置结果 */
    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setForbidStateResult;    
    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setUnforbidStateResult;    
    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setForbidStateFailReason;    
    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setUnforbidStateFailReason;

    pIotOMCtx->sendForbidStateFlag = TRUE;
    pIotOMCtx->sendForbidStateCount = 0;
    return dataLen;
}
static uint16_t IotOM_SendSetForbidState(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint8_t forBidState = 0;
    uint8_t forBidReason = 0;

    AswMonitor_GetForbidState(&forBidState, &forBidReason);

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;

    if (forBidState == TRUE)
    {
        pBuf[dataLen++] = 0x01;
        pBuf[dataLen++] = forBidReason;
        pBuf[dataLen++] = 0x00;
    }
    else
    {
        pBuf[dataLen++] = 0x02;
        pBuf[dataLen++] = 0x00;
        pBuf[dataLen++] = forBidReason;
    }

    return dataLen;
}

static uint16_t IotOM_SendUpdateResponse(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;

    pBuf[dataLen++] = pIotOMCtx->stProtoData[0].setUpdateResult;
    return dataLen;
}

static uint16_t IotOM_SendOrderRecord(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint32_t orderLen = 0;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 枪号 */
    pBuf[dataLen++] = port + 1;

    /* 订单数据长度和内容 */
    orderLen = AswPlatM_TransformRecord(&pIotOMCtx->stOrderInfo, &pBuf[dataLen + 4]);
    Common_Uint32ToFourUint8(&pBuf[dataLen], orderLen);
    dataLen += 4;
    dataLen += orderLen;
    return dataLen;
}

static uint16_t IotOM_SendRemoteQuerySetParamRsp(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatParam_Struct *pPlatParam = AswPlatM_GetPlatParamPtr();
    uint16_t dataLen = 0;
    uint8_t valueLen = 0;
    uint8_t *pParamCount = NULL;
    uint8_t paramBit = 0;
    uint8_t *pParamLen = NULL;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotOMCtx->pileFixDnAsc, 32);
    dataLen += 32;
    /* 参数操作：0-读取，1-设置 */
    pBuf[dataLen++] = pIotOMCtx->stProtoData[port].optParamAction; // 根据实际操作类型设置
    /* 参数操作结果：0-成功，1-失败 */
    pBuf[dataLen++] = pIotOMCtx->stProtoData[port].optParamResult; // 使用存储的操作结果

    if (pIotOMCtx->stProtoData[port].optParamAction == 0x00 &&
        pIotOMCtx->stProtoData[port].optParamResult == 0x00)
    {
        /* 指针指向参数个数赋值内存，方便赋值 */
        pParamCount = &pBuf[dataLen++];

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 0))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "platDn");
            dataLen += 16;
            pParamLen[0] = strlen(pPlatParam->platPileDn);
            memcpy(&pBuf[dataLen], pPlatParam->platPileDn, pParamLen[0]);
            dataLen += pParamLen[0];
            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 1))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "platType");
            dataLen += 16;
            AswPlatM_GetPlatName((char *)&pBuf[dataLen], &valueLen);
            dataLen += valueLen;
            pParamLen[0] = valueLen;
            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 2))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "ipAddr");
            dataLen += 16;
            pParamLen[0] = strlen(pPlatParam->platMainIp);
            memcpy(&pBuf[dataLen], pPlatParam->platMainIp, pParamLen[0]);
            dataLen += pParamLen[0];
            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 3))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "port");
            dataLen += 16;
            pParamLen[0] = 2;
            Common_Uint16ToTwoUint8(&pBuf[dataLen], pPlatParam->platMainPort);
            dataLen += 2;
            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 4))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "cardType");
            dataLen += 16;
            AswPlatM_GetCardName((char *)&pBuf[dataLen], &valueLen);
            dataLen += valueLen;
            pParamLen[0] = valueLen;
            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 5))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "devOperator");
            dataLen += 16;

            if (AswPlatM_GetDevOperator((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 6))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "iv");
            dataLen += 16;

            if (AswPlatM_GetIv((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 7))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "cipherKey");
            dataLen += 16;

            if (AswPlatM_GetCipherKey((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 8))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "productKey");
            dataLen += 16;
            
            if (AswPlatM_GetProductKey((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 9))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "token");
            dataLen += 16;
            
            if (AswPlatM_GetToken((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 10))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "productSecret");
            dataLen += 16;
            
            if (AswPlatM_GetProductSecret((char *)&pBuf[dataLen], &valueLen))
            {
                dataLen += valueLen;
                pParamLen[0] = valueLen;
            }
            else
            {
                strcpy((char *)&pBuf[dataLen], "null");
                dataLen += strlen("null");
                pParamLen[0] = strlen("null");
            }

            pParamCount[0]++;
        }

        if (Common_GetBitFlag(&pIotOMCtx->stProtoData[port].queryParamFlag, 11))
        {
            pParamLen = &pBuf[dataLen++];
            memset(&pBuf[dataLen], 0, 16);
            snprintf((char *)&pBuf[dataLen], 16, "%s", "workmode");
            dataLen += 16;
            pBuf[dataLen] = (CddModeM_IsGBMode()) ? 0x00 : 0x01;
            dataLen += 1;
            pParamLen[0] = 1;
            pParamCount[0]++;
        }
    }

    return dataLen;
}

static uint16_t IotOM_SendReadLocalFileRsp(uint8_t port, uint8_t *pBuf)
{
    IotOMFrameReadLocalFileRsp_Struct *pRsp = (IotOMFrameReadLocalFileRsp_Struct *)pBuf;

    /* 设备编码 */
    memcpy(pRsp->dn, pIotOMCtx->pileFixDnAsc, 32);
    /* 结果 */
    pRsp->result = pIotOMCtx->stProtoData[port].readLocalFileResult;

    return sizeof(IotOMFrameReadLocalFileRsp_Struct);
}

static uint16_t IotOM_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotOMFrameHead_Struct *pFrameHead = (IotOMFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotOMFrameHead_Struct);
    uint16_t crc16 = 0;

    pFrameHead->head[0] = IOT_OM_HEAD1;
    pFrameHead->head[1] = IOT_OM_HEAD2;
    Common_Uint16ToTwoUint8(pFrameHead->version, IOT_OM_PROTOCOL_VERSION);
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

void IotOM_UpCtrlSendDeal(void)
{
    const IotOMSendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_OM_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotOMCtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotOMCtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotOMCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotOMCtx->sendIndex < ARRAY_SIZE(c_stIotOMSendctrlTable))
            {
                index = pIotOMCtx->sendIndex;
                port = pIotOMCtx->sendPort;

                pCmdSendCtrl = &c_stIotOMSendctrlTable[index];

                if ((Common_GetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotOM_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotOMCtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_OM_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotOMCtx->reqSeq++;
                    }
                    else
                    {
                        reqSeq = Common_GetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotOMFrameHead_Struct);
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        pIotOMCtx->queueBusyFlag = TRUE;
						pIotOMCtx->waitQueueIdleTick = Common_GetSystick();

                        dataLen = IotOM_PackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotOMCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                            {
                                pIotOMCtx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTOM_CFG_DebugPrint("[枪：%d]发送[cmd: %02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());
                                            
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_OM_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

			pIotOMCtx->sendIndex++;

			if (pIotOMCtx->sendIndex >= ARRAY_SIZE(c_stIotOMSendctrlTable))
			{
				pIotOMCtx->sendIndex = 0;
				pIotOMCtx->sendPort++;

				if (pIotOMCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotOMCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotOMCtx->queueBusyFlag == TRUE)
			{
				break;
			}
        }
    }
}


















