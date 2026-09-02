/******************************************************************************
* File Name          : Asw_IotProtoAHTTSend.c
* Description        : AHTT protocol send scheduling
 -----------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
******************************************************************************/
#include "Asw_IotProtoAHTTM.h"
#include "Asw_IotProtoAHTTSend.h"
#include "Asw_PlatM.h"
#include "FrameQueue.h"
#include "Version.h"

extern IotAHTTCtx_Struct *pIotAHTTCtx;

static uint16_t IotAHTT_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendHeartBeat(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendSetHeartCycleRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendQueryHeartCycleRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendSetDomainPortRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendSetMaxChargeTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendQueryMaxChargeTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendSetDevParamRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAHTT_SendQueryDevParamRsp(uint8_t port, uint8_t *pBuf);

static const IotAHTTSendCtrl_Struct c_stIotAHTTSendctrlTable[IOT_AHTT_CMD_SEND_COUNT] =
{
    {IOT_AHTT_CMD_LOGIN,                 IOT_AHTT_CMDTYPE_REQUSET,  0, IotAHTT_SendLoginReq, IOT_AHTT_CMD_LOGIN, TRUE,  "签到"},
    {IOT_AHTT_CMD_SET_HEART_CYCLE,       IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendSetHeartCycleRsp, IOT_AHTT_CMD_SET_HEART_CYCLE,       TRUE,  "设置心跳周期应答"},
    {IOT_AHTT_CMD_QUERY_HEART_CYCLE,     IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendQueryHeartCycleRsp, IOT_AHTT_CMD_QUERY_HEART_CYCLE,     TRUE,  "查询心跳周期应答"},
    {IOT_AHTT_CMD_SET_DOMAIN_PORT,       IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendSetDomainPortRsp, IOT_AHTT_CMD_SET_DOMAIN_PORT, TRUE,  "设置平台域名和端口应答"},
    {IOT_AHTT_CMD_SET_MAX_CHARGE_TIME,   IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendSetMaxChargeTimeRsp, IOT_AHTT_CMD_SET_MAX_CHARGE_TIME,   TRUE,  "设置最大充电时长应答"},
    {IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME, IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendQueryMaxChargeTimeRsp, IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME, TRUE,  "查询最大充电时长应答"},
    {IOT_AHTT_CMD_STOP_CHARGE,           IOT_AHTT_CMDTYPE_RESPONSE, 0, NULL, IOT_AHTT_CMD_STOP_CHARGE,           TRUE,  "停止充电应答"},
    {IOT_AHTT_CMD_CARD_AUTH,             IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_CARD_AUTH,             TRUE,  "刷卡鉴权"},
    {IOT_AHTT_CMD_START_CHARGE,          IOT_AHTT_CMDTYPE_RESPONSE, 0, NULL, IOT_AHTT_CMD_START_CHARGE,          TRUE,  "启动充电应答"},
    {IOT_AHTT_CMD_HEARTBEAT,             IOT_AHTT_CMDTYPE_REQUSET,  0, IotAHTT_SendHeartBeat, IOT_AHTT_CMD_HEARTBEAT, TRUE, "心跳上报"},
    {IOT_AHTT_CMD_SET_DEV_PARAM,         IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendSetDevParamRsp, IOT_AHTT_CMD_SET_DEV_PARAM,         TRUE,  "设置设备运行参数应答"},
    {IOT_AHTT_CMD_QUERY_DEV_PARAM,       IOT_AHTT_CMDTYPE_RESPONSE, 0, IotAHTT_SendQueryDevParamRsp, IOT_AHTT_CMD_QUERY_DEV_PARAM,       TRUE,  "查询设备运行参数应答"},
    {IOT_AHTT_CMD_REPORT_REALDATA,       IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_REPORT_REALDATA,       TRUE,  "上报实时数据"},
    {IOT_AHTT_CMD_REPORT_ORDER,          IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_REPORT_ORDER,          TRUE,  "上报订单"},
    {IOT_AHTT_CMD_QUERY_TIME,            IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_QUERY_TIME,            TRUE,  "查询平台时间"},
    {IOT_AHTT_CMD_REPORT_DEV_STATE,      IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_REPORT_DEV_STATE,      TRUE,  "上报设备状态"},
    {IOT_AHTT_CMD_DEV_ALARM,             IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_DEV_ALARM,             TRUE,  "上报设备告警"},
    {IOT_AHTT_CMD_NET_ALARM,             IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_NET_ALARM,             TRUE,  "上报网络告警"},
    {IOT_AHTT_CMD_TEMP_ALARM,            IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_TEMP_ALARM,            TRUE,  "上报温度告警"},
    {IOT_AHTT_CMD_SET_TEMP_LIMIT,        IOT_AHTT_CMDTYPE_RESPONSE, 0, NULL, IOT_AHTT_CMD_SET_TEMP_LIMIT,        TRUE,  "设置温度阈值应答"},
    {IOT_AHTT_CMD_QUERY_TEMP_LIMIT,      IOT_AHTT_CMDTYPE_RESPONSE, 0, NULL, IOT_AHTT_CMD_QUERY_TEMP_LIMIT,      TRUE,  "查询温度阈值应答"},
    {IOT_AHTT_CMD_ELECTRIC_ALARM,        IOT_AHTT_CMDTYPE_REQUSET,  0, NULL, IOT_AHTT_CMD_ELECTRIC_ALARM,        TRUE,  "上报电气告警"},
    {IOT_AHTT_CMD_UPDATE,                IOT_AHTT_CMDTYPE_RESPONSE, 0, NULL, IOT_AHTT_CMD_UPDATE,                TRUE,  "远程升级应答"},
};

static uint8_t IotAHTT_ReportCycleCheck(uint8_t port, uint16_t cmd, uint32_t sendCycle)
{
    uint32_t startTick = Common_GetSendTick(pIotAHTTCtx->pFuncSendCtrl, port, cmd);
    uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, port, cmd);
    uint8_t retFlag = FALSE;

    if (TRUE == sendImmdFlag)
    {
        retFlag = TRUE;
    }
    else if (TRUE == Common_JudgeTimeoutMs(startTick, sendCycle))
    {
        retFlag = TRUE;
    }
    else
    {
    }

    return retFlag;
}

static uint16_t IotAHTT_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf, uint16_t dataLen)
{
    IotAHTTFrameHead_Struct *pFrameHead = NULL;
    uint16_t totalLen = 0;
    uint16_t declareLen = 0;
    uint16_t crc16 = 0;
    uint16_t retLen = IOT_AHTT_PACK_INVALID_LEN;

    if ((pBuf != NULL) &&
        (dataLen + IOT_AHTT_FRAME_MIN_LEN <= IOT_AHTT_FRAME_MAX_LEN))
    {
        pFrameHead = (IotAHTTFrameHead_Struct *)pBuf;
        totalLen = IOT_AHTT_FRAME_HEAD_LEN + dataLen;
        declareLen = IOT_AHTT_DECLARE_MIN_LEN + dataLen;

        pFrameHead->head = IOT_AHTT_HEAD;
        Common_Uint16ToTwoUint8(pFrameHead->len, declareLen);
        pFrameHead->ver = IOT_AHTT_PROTOCOL_VERSION;
        memcpy(pFrameHead->deviceNum, pIotAHTTCtx->deviceNum, IOT_AHTT_DEVICE_NUM_LEN);
        Common_Uint16ToTwoUint8(pFrameHead->seq, seq);
        pFrameHead->cmd = cmd;

        crc16 = Common_CalcCRC16(pBuf, totalLen);
        pBuf[totalLen] = (uint8_t)((crc16 >> 8) & 0xFF);
        pBuf[totalLen + 1] = (uint8_t)(crc16 & 0xFF);
        retLen = totalLen + IOT_AHTT_CRC_LEN;
    }

    return retLen;
}

static uint16_t IotAHTT_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    CddNetM_GetIccid(&pBuf[dataLen]);
    dataLen += 20;
    pBuf[dataLen++] = APP_SW_MINOR_VERSION;
    pBuf[dataLen++] = APP_SW_CUSTORM_VERSION * 10 + APP_SW_PATCH_VERSION;
    memset(&pBuf[dataLen], 0x00, 6);
    dataLen += 6;

    return dataLen;
}

static uint16_t IotAHTT_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint16_t csq = 0;
    uint8_t channel = 0;
    uint8_t byteIndex = 0;
    uint8_t bitPos = 0;
    uint8_t channelState = 0;
    uint8_t stateMask = 0;

    pBuf[dataLen++] = IOT_AHTT_HEART_NET_4G;
    csq = CddNetM_GetCsq();
    pBuf[dataLen++] = (uint8_t)(csq & 0xFF);
    memset(&pBuf[dataLen], 0xFF, 3);

    for (channel = 0;
        (channel < SYSCFG_CFG_GUN_NUM) && (channel < IOT_AHTT_HEART_CHANNEL_COUNT);
        channel++)
    {
        channelState = IotAHTT_GetGunState(channel);
        byteIndex = channel / 4;
        bitPos = (channel % 4) * 2;
        stateMask = (uint8_t)(0x03 << bitPos);
        pBuf[dataLen + byteIndex] &= (uint8_t)(~stateMask);
        pBuf[dataLen + byteIndex] |= (uint8_t)(channelState << bitPos);
    }

    dataLen += 3;
    return dataLen;
}

static uint16_t IotAHTT_SendSetHeartCycleRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pIotAHTTCtx->setHeartCycleResult;
    return dataLen;
}

static uint16_t IotAHTT_SendQueryHeartCycleRsp(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pPrivateParam->stAHTTParam.stWorkParam.heartCycleMin;
    return dataLen;
}

static uint16_t IotAHTT_SendSetDomainPortRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pIotAHTTCtx->domainSwitchResult;
    return dataLen;
}

static uint16_t IotAHTT_SendSetMaxChargeTimeRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pIotAHTTCtx->setMaxChargeTimeResult;
    return dataLen;
}

static uint16_t IotAHTT_SendQueryMaxChargeTimeRsp(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pPrivateParam->stAHTTParam.stWorkParam.maxChargeTimeHour;
    return dataLen;
}

static uint16_t IotAHTT_SendSetDevParamRsp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    pBuf[dataLen++] = pIotAHTTCtx->setDevParamResult;
    return dataLen;
}

static uint16_t IotAHTT_SendQueryDevParamRsp(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint16_t dataLen = 0;

    memcpy(pBuf, pPrivateParam->stAHTTParam.stWorkParam.devOperationParam,
        8);
    dataLen = 8;
    return dataLen;
}

void IotAHTT_UpCtrlSendDeal(void)
{
    const IotAHTTSendCtrl_Struct *pCmdSendCtrl = NULL;
    MSNvmPlatPrivateParam_Union *pPrivateParam = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint32_t sendCycle = 0;
    uint8_t txBuf[IOT_AHTT_FRAME_MAX_LEN] = {0};

    if (TRUE == pIotAHTTCtx->queueBusyFlag)
    {
        if (TRUE == Common_JudgeTimeoutMs(pIotAHTTCtx->waitQueueIdleTick, 500))
        {
            pIotAHTTCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotAHTTCtx->sendIndex < ARRAY_SIZE(c_stIotAHTTSendctrlTable))
            {
                index = pIotAHTTCtx->sendIndex;
                port = pIotAHTTCtx->sendPort;
                pCmdSendCtrl = &c_stIotAHTTSendctrlTable[index];
                sendCycle = pCmdSendCtrl->sendCycle;
                if (IOT_AHTT_CMD_HEARTBEAT == pCmdSendCtrl->cmd)
                {
                    pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
                    sendCycle = pPrivateParam->stAHTTParam.stWorkParam.heartCycleMin *
                        IOT_AHTT_MINUTE_MS;
                }

                if ((TRUE == Common_GetSendEnable(pIotAHTTCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd)) &&
                    (TRUE == IotAHTT_ReportCycleCheck(port, pCmdSendCtrl->cmd, sendCycle)) &&
                    (pCmdSendCtrl->pSendFunc != NULL))
                {
                    if (IOT_AHTT_CMDTYPE_REQUSET == pCmdSendCtrl->cmdType)
                    {
                        reqSeq = pIotAHTTCtx->reqSeq;
                        if ((reqSeq < IOT_AHTT_SEQ_MIN) || (reqSeq > IOT_AHTT_SEQ_MAX))
                        {
                            reqSeq = IOT_AHTT_SEQ_MIN;
                        }

                    }
                    else
                    {
                        reqSeq = (uint16_t)Common_GetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }
                    
                    dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[IOT_AHTT_FRAME_HEAD_LEN]);

                    if (IOT_AHTT_PACK_INVALID_LEN != dataLen)
                    {
                        pIotAHTTCtx->queueBusyFlag = TRUE;
                        pIotAHTTCtx->waitQueueIdleTick = Common_GetSystick();

                        dataLen = IotAHTT_PackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);
                        
                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotAHTTCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen, 0))
                        {
                            break;
                        }

                        if (IOT_AHTT_CMDTYPE_REQUSET == pCmdSendCtrl->cmdType)
                        {
                            pIotAHTTCtx->reqSeq = (reqSeq >= IOT_AHTT_SEQ_MAX) ? IOT_AHTT_SEQ_MIN : (reqSeq + 1);
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTAHTT_CFG_DebugPrint("AHTT,[枪：%d]发送[cmd: 0x%02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotAHTTCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());

                        if ((0 == pCmdSendCtrl->sendCycle) &&
                            (IOT_AHTT_CMD_HEARTBEAT != pCmdSendCtrl->cmd))
                        {
                            Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (IOT_AHTT_CMD_SET_DOMAIN_PORT == pCmdSendCtrl->cmd)
                        {
                            IotAHTT_NotifyDomainSwitchRspQueued();
                        }

                        if ((IOT_AHTT_CMDTYPE_REQUSET == pCmdSendCtrl->cmdType) &&
                            (IOT_AHTT_CMD_NULL != pCmdSendCtrl->matchCmd))
                        {
                            Common_SetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                            Common_SetRecvTick(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                        }
                    }
                }
            }

            pIotAHTTCtx->sendIndex++;
            if (pIotAHTTCtx->sendIndex >= ARRAY_SIZE(c_stIotAHTTSendctrlTable))
            {
                pIotAHTTCtx->sendIndex = 0;
                pIotAHTTCtx->sendPort++;
                if (pIotAHTTCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
                {
                    pIotAHTTCtx->sendPort = 0;
                    break;
                }
            }

            if (TRUE == pIotAHTTCtx->queueBusyFlag)
            {
                break;
            }
        }
    }
}
