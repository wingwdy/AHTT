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
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "SS_Ucm.h"
#include "Asw_ChargeIf.h"
#include "SS_Tm.h"
#include "Asw_Monitor.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_GN_RecvGunNoTransform(inputPort, outputPort)    do{ \
                                                                if (inputPort > SYSCFG_CFG_GUN_NUM)\
                                                                {\
                                                                    outputPort = 0;\
                                                                }\
                                                                else\
                                                                {\
                                                                    outputPort = (inputPort - 1);\
                                                                }\
                                                            }while(0)

/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

static uint8_t IotGN_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGNCtx_Struct *pIotGNCtx;


static const IotGNRecvCtrl_Struct c_stIotGNRecvctrlTable[IOT_GN_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_GN_CMD_LOGIN_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_GN_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvHeartBeatRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,
        .cMeaning = "心跳应答",
    },

    [2] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_VERIFY_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillModeVerifyRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_VERIFY_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证应答",
    },

    [3] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_4RATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillMode4RateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [4] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_MUTIRATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillModeMultRateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [5] = 
    {
        .cmd = IOT_GN_CMD_CALL_REALDATA,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvCallRealData,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_CALL_REALDATA_ACK,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据",
    },

    [6] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_START_CHARGE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvRemoteStartCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_REMOTE_START_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电",
    },

    [7] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_STOP_CHARGE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvRemoteStopCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电",
    },

    [8] = 
    {
        .cmd = IOT_GN_CMD_ORDER_RECORD_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvOrderRecordRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_MULTI_ORDER_RECORD_REQ,
        .printFlag = TRUE,
        .cMeaning = "交易记录应答",
    },

    [9] = 
    {
        .cmd = IOT_GN_CMD_PILE_START_CHARGE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvPileStartChargeRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_PILE_START_CHARGE_REQ,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动启动充电应答",
    },

    [10] = 
    {
        .cmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvUpdateAccountMoney,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额",
    },

    [11] = 
    {
        .cmd = IOT_GN_CMD_SYNC_TIME,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSyncTime,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_SYNC_TIME_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程对时",
    },

    [12] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_4RATE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetBillMode4Rate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_4RATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置四类电价计费模型",
    },

    [13] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetBillModeMultiRate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置多类电价计费模型",
    },

    [14] = 
    {
        .cmd = IOT_GN_CMD_SET_QRCODE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetQRCode,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_SET_QRCODE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置二维码",
    },

    [15] = 
    {
        .cmd = IOT_GN_CMD_REBOOT,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetReboot,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_GN_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程重启",
    },
};



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static const IotGNRecvCtrl_Struct* IotGN_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotGNRecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_GN_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotGNRecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotGNRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotGNFrameHead_Struct *IotGN_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotGNFrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotGNFrameHead_Struct) + 2))
    {
        pHead = (IotGNFrameHead_Struct *)pStart;

        if ((pHead->head[0] == IOT_GN_HEAD1) && (pHead->head[1] == IOT_GN_HEAD2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (Common_TwoUint8ToUint16(pHead->version) == IOT_GN_PROTOCOL_VERSION &&
                frameLen > (sizeof(IotGNFrameHead_Struct) + 2))
            {
                calcCrc16 = Common_CalcCRC16((uint8_t *)pHead, frameLen - 2);
                pRecvCrc = (uint8_t *)pHead + frameLen - 2;
                recvCrc16 = pRecvCrc [1] | (pRecvCrc [0] << 8);

                if (calcCrc16 == recvCrc16)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + frameLen;
                    break;
                }
            }
        }

        pStart++;
        remainLen--;
        dealLen[0]++;
    }

    return pHead;
}

static void IotGN_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotGNRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotGNFrameHead_Struct *pFrameHead = IotGN_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotGN_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = Common_TwoUint8ToUint16(pFrameHead->dataLen);
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotGNFrameHead_Struct), frameLen - sizeof(IotGNFrameHead_Struct) - 2))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTGN_CFG_LogPrint("[枪：%d]接收[cmd: 0x%02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_GN_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_GN_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_GN_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTGN_CFG_LogPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}

static uint8_t IotGN_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, gunNo, IOT_GN_CMD_HEARTBEAT_REQ, TRUE);
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, gunNo, IOT_GN_CMD_BILLMODE_VERIFY_REQ, TRUE);
        }

        pIotGNCtx->loginSucc = TRUE;
    }
    else
    {
        index++;
        IOTGN_CFG_LogPrint("登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotGN_OfflineHandle();
    }

    return TRUE;
}

static uint8_t IotGN_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    if (pRecvData[index] > 0)
    {
        pRecvData[index]--;
        port[0] = pRecvData[index];
    }

    return TRUE;
}

static uint8_t IotGN_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t verifyRes = FALSE;

    if (memcmp(pBillMode->billModeID, &pRecvData[index], 2) == 0)
    {
        index += 2;

        if (pRecvData[index] == 0x00)
        {
            /*  和平台沟通，建议每次都请求，这个标识暂时不使用
                verifyRes = TRUE;
                IOTGN_CFG_LogPrint("计费模型，不需要更新！\r\n");
             */
            verifyRes = FALSE;

        }
    }

    if (verifyRes == FALSE)
    {
        IOTGN_CFG_LogPrint("计费模型变化，需要更新！\r\n");
        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_BILLMODE_REQ, TRUE);
    }

    return TRUE;
}

static uint8_t IotGN_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_4;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    for (temp = 0; temp < 4; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    pBillMode->elecLossRate = pRecvData[index++];
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)&pIotGNCtx->param, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_MULT;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->elecLossRate = pRecvData[index++];

    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    for (temp = 0; temp < 9; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, 0, IOT_GN_CMD_BILLMODE_4RATE_RSP, FALSE);
    Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, 0, IOT_GN_CMD_BILLMODE_4RATE_RSP);
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)&pIotGNCtx->param, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    return TRUE;
}

static uint8_t IotGN_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t ret = FALSE;
    uint8_t reason = 0;

    /* 订单未结束 */
    if (TRUE != AswMonitor_IsOrderIdle(port))
    {
        reason = 0x02;
    }
    /* 存在故障 */
    else if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        reason = 0x03;
    }
    /* 枪未连接 */
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        reason = 0x05;
    }
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = 0x07; 
    }
    /* 升级中 */
    else if (TRUE == SSUcm_IsUpdating())
    {
        reason = 0x08;
    }
    /* 设备禁用 */
    else if (TRUE == AswMonitor_CheckForbidState())
    {
        reason = 0x09;
    }
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

static uint8_t IotGN_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    /* 订单号 */
    memcpy(pIotGNCtx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[index], 16);
    index += 16;

    if (TRUE == IotGN_CheckChargeStart(port[0], &failReason))
    {
        /* 卡号 */
        memcpy(pChargeCtrl->authCardID, &pRecvData[index], 8);
        index += 8;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        if (accountMoney <= IOTGN_CFG_CHARGE_MIN_ACCOUNT_MONEY)
        {
            IOTGN_CFG_LogPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            pIotGNCtx->stProtoData[port[0]].remoteStartResult = 0;
            /* 无余额不足的失败响应，暂时填离线*/
            pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = 0x04;
        }
        else
        {
            memcpy(pIotGNCtx->stProtoData[port[0]].curUsedOrderTransactionNum,
                pIotGNCtx->stProtoData[port[0]].newRecvOrderTransactionNum, 16);

            pChargeCtrl->accountMoney = accountMoney;

            /* 充电控制方式转换 */
            if (pRecvData[index++] == 1)  /* 按时间 */
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 60;
            }
            else if (pRecvData[index++] == 2)  /* 按金额 */
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
            }
            else if (pRecvData[index++] == 3)  /* 按电量 */
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
            }
            else /* 自动充满 */
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
            }

            index += 4;

            pIotGNCtx->stProtoData[port[0]].remoteStartResult = 1;
            pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = 0;
            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP);
        }
    }
    else
    {
        pIotGNCtx->stProtoData[port[0]].remoteStartResult = 0;
        pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = failReason;
    }

    Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_REMOTE_START_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotGN_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    if (AswMonitor_IsOrderIdle(port[0]) == TRUE)
    {
        pIotGNCtx->stProtoData[port[0]].remoteStopResult = 0;
        pIotGNCtx->stProtoData[port[0]].remoteStopFailReason = 0x02;
    }
    else
    {
        pIotGNCtx->stProtoData[port[0]].remoteStopResult = 0x01;
        pIotGNCtx->stProtoData[port[0]].remoteStopFailReason = 0x00;
        AswErrhandle_SetErrExsitCallback(port[0], eSrc_AppStop);
    }

    Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotGN_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNOrderInfo_Struct *pOrderData = &pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;

    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGNCtx->time);
    IOTGN_CFG_LogPrint("交易记录上报成功!\r\n");
    return TRUE;
}

static uint8_t IotGN_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    if (TRUE == IotGN_CheckChargeStart(port[0], &failReason))
    { 
        /* 鉴权成功标志 */
        if (pRecvData[index + 28] == 0x01)
        {
            accountMoney = Common_FourUint8ToUint32(&pRecvData[index + 24]);

            if (accountMoney <= IOTGN_CFG_CHARGE_MIN_ACCOUNT_MONEY)
            {
                IOTGN_CFG_LogPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            }
            else
            {
                /* 订单号 */
                memcpy(pIotGNCtx->stProtoData[port[0]].curUsedOrderTransactionNum, &pRecvData[index], 16);
                index += 16;
                /* 卡号 */
                memcpy(pIotGNCtx->stProtoData[port[0]].authCardID, &pRecvData[index], 8);
                index += 8;
                /* 账户余额 */
                pChargeCtrl->accountMoney = accountMoney;
                index += 4;
                /* 失败原因 */
                index += 1;

                /* 充电控制方式转换 */
                if (pRecvData[index++] == 1)  /* 按时间 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 60;
                }
                else if (pRecvData[index++] == 2)  /* 按金额 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                }
                else if (pRecvData[index++] == 3)  /* 按电量 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                }
                else /* 自动充满 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                }

                AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_CARD);
                IOTGN_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电成功!\r\n", port[0]);
            }
        }
        else
        {
            index += 29;
            IOTGN_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电失败，失败原因：%02X!\r\n", port[0], pRecvData[index]);
        }
    }
    else
    {
        IOTGN_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电, 平台应答成功，但设备无法启动充电，失败原因：%d\r\n", port[0], failReason);
    }

    return TRUE;
}

static uint8_t IotGN_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t invalidCardID[8] = {0};

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    memcpy(pIotGNCtx->stProtoData[port[0]].updateAccountMoneyCardID, &pRecvData[index], 8);

    if (0 == memcmp(&pRecvData[index], invalidCardID, 8))
    {
        index += 8;
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
                IOTGN_CFG_LogPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
                    pChargeCtrl->accountMoney % 100);
        pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
    }
    else
    {
        if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
        {
            if (0 == memcmp(&pRecvData[index], pIotGNCtx->stProtoData[port[0]].authCardID, 8))
            {
                index += 8;
                pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
                IOTGN_CFG_LogPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
                    pChargeCtrl->accountMoney % 100);
                pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
            }
            else
            {
                IOTGN_CFG_LogPrint("[枪：%d]更新账户余额失败，卡号不一致!\r\n", port[0]);
                pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
            }
        }
        else
        {
            IOTGN_CFG_LogPrint("[枪：%d]更新账户余额失败，非刷卡启动充电!\r\n", port[0]);
            pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
        }
    }

    return TRUE;
}

static uint8_t IotGN_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t tempBin1 = 0;
    uint8_t tempBin2 = 0;
    CommonDateTime_Struct dataTime;

    Common_BCDToBIN(&pRecvData[index++], &tempBin1, 1);
    Common_BCDToBIN(&pRecvData[index++], &tempBin2, 1);
    dataTime.year = tempBin1 * 100 + tempBin2;
    Common_BCDToBIN(&pRecvData[index++], &dataTime.month, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.day, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.hour, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.minute, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.second, 1);
    SSTM_SynTimeByDateTime(&dataTime);
    return TRUE;
}

static uint8_t IotGN_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_4;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    for (temp = 0; temp < 4; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    pBillMode->elecLossRate = pRecvData[index++];
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)&pIotGNCtx->param, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmGNParamBillMode_Struct *pBillMode = &pIotGNCtx->param.stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_MULT;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->elecLossRate = pRecvData[index++];

    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    for (temp = 0; temp < 9; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;
    }  

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)&pIotGNCtx->param, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;
    MSNvmDrcode_Struct qrParam = { 0 };
    
    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index], 100);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTGN_CFG_LogPrint("[枪：%d]设置的二维码内容：%.100s\r\n", port[0], &pRecvData[index]);
    }

    return TRUE;
}

static uint8_t IotGN_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    if (pRecvData[index] == 0x01)
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_Immediate); 
    }
    else
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_WaitIdle);
    }

    return TRUE;
}

void IotGN_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotGNCtx->frameQueueChannelID, IotGN_DecodeData);
}

void IotGN_TimeoutDetect(void)
{
    const IotGNRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_GN_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotGNRecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_GN_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                 continue;
            }

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotGNCtx->pFuncRecvCtrl, port, 
                pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTGN_CFG_LogPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if (pCmdRecvCtrl->cmd == IOT_GN_CMD_HEARTBEAT_RSP || pCmdRecvCtrl->cmd == IOT_GN_CMD_LOGIN_RSP)
                    {
                        IotGN_OfflineHandle();
                    }
                    else
                    {
                        Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        
                        if (pCmdRecvCtrl->cmd == IOT_GN_CMD_ORDER_RECORD_RSP)
                        {
                            if (pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
                            {
                                Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, FALSE);
                            }
                            else
                            {
                                Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, FALSE);
                            }

                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGNCtx->time);
                            IOTGN_CFG_LogPrint("交易记录上报失败, 强行置为成功!\r\n");
                        }
                        else
                        {
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                    if (pCmdRecvCtrl->cmd == IOT_GN_CMD_ORDER_RECORD_RSP)
                    {
                        if (pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, FALSE);
                        }
                        else
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, FALSE);
                        }
                    }
                    else
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
            }
        }
    }
}






















