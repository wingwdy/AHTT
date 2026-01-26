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
    pBuf[dataLen++] = APP_SW_PATCH_VERSION;
    pBuf[dataLen++] = APP_SW_CUSTORM_VERSION;
    pBuf[dataLen++] = APP_SW_MINOR_VERSION;
    pBuf[dataLen++] = APP_SW_MAJOR_VERSION;
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
    uint8_t dataLen = 0;

    /* 桩温过高故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EnvOverTempErr))
    {
        Common_SetBitFlag(pBuf, 0);
    }

    /* 急停故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EmergencyStop))
    {
        Common_SetBitFlag(pBuf, 1);
    }
    
    /* 控制导引故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor))
    {
        Common_SetBitFlag(pBuf, 3);
    }

    /* 电表通信故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_MeterCommErr))
    {
        Common_SetBitFlag(pBuf, 7);
    }

    /* 读卡器通信故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr))
    {
        Common_SetBitFlag(pBuf, 8);
    }

    /* 交流接触器故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault))
    {
        Common_SetBitFlag(pBuf, 11);
    }

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation))
    {
        Common_SetBitFlag(pBuf, 11);
    }

    /* 枪温过温故障 */
    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_GunOverTempErr))
    {
        Common_SetBitFlag(pBuf, 14);
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
        /* 已充金额 */
        memset(&pBuf[dataLen], 0x00, 4);
        dataLen += 4;
    }

    /* 硬件故障 */
    memset(&pBuf[dataLen], 0x00, 2);
    IotOM_SetRealDataErrBit(port, &pBuf[dataLen]);
    dataLen += 2;

    /* 硬件故障 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4;
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
                            IOTOM_CFG_LogPrint("[枪：%d]发送[cmd: %02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
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


















