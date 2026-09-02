/******************************************************************************
* File Name          : Asw_IotProtoAHTTRecv.c
* Description        : AHTT protocol frame decode and receive scheduling
 -----------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
******************************************************************************/
#include "Asw_IotProtoAHTTM.h"
#include "Asw_IotProtoAHTTRecv.h"
#include "Asw_PlatM.h"
#include "FrameQueue.h"

extern IotAHTTCtx_Struct *pIotAHTTCtx;

static uint8_t IotAHTT_RecvLoginRsp(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvHeartBeatRsp(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvSetHeartCycle(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvQueryHeartCycle(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvSetDomainPort(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvSetMaxChargeTime(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvQueryMaxChargeTime(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvSetDevParam(uint8_t *port, uint8_t *pData, uint16_t len);
static uint8_t IotAHTT_RecvQueryDevParam(uint8_t *port, uint8_t *pData, uint16_t len);

static const IotAHTTRecvCtrl_Struct c_stIotAHTTRecvctrlTable[IOT_AHTT_CMD_RECV_COUNT] =
{
    {IOT_AHTT_CMD_LOGIN,                 IOT_AHTT_CMDTYPE_RESPONSE, IotAHTT_RecvLoginRsp, IOT_AHTT_LOGIN_TIMEOUT_MS, IOT_AHTT_LOGIN_MAX_TRY_COUNT, IOT_AHTT_CMD_LOGIN, TRUE, "签到应答"},
    {IOT_AHTT_CMD_SET_HEART_CYCLE,       IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvSetHeartCycle, 0, 0, IOT_AHTT_CMD_SET_HEART_CYCLE,       TRUE,  "设置心跳周期"},
    {IOT_AHTT_CMD_QUERY_HEART_CYCLE,     IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvQueryHeartCycle, 0, 0, IOT_AHTT_CMD_QUERY_HEART_CYCLE,     TRUE,  "查询心跳周期"},
    {IOT_AHTT_CMD_SET_DOMAIN_PORT,       IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvSetDomainPort, 0, 0, IOT_AHTT_CMD_NULL, TRUE,  "设置平台域名和端口"},
    {IOT_AHTT_CMD_SET_MAX_CHARGE_TIME,   IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvSetMaxChargeTime, 0, 0, IOT_AHTT_CMD_SET_MAX_CHARGE_TIME,   TRUE,  "设置最大充电时长"},
    {IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME, IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvQueryMaxChargeTime, 0, 0, IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME, TRUE,  "查询最大充电时长"},
    {IOT_AHTT_CMD_STOP_CHARGE,           IOT_AHTT_CMDTYPE_REQUSET,  NULL, 0, 0, IOT_AHTT_CMD_STOP_CHARGE,           TRUE,  "停止充电"},
    {IOT_AHTT_CMD_CARD_AUTH,             IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_CARD_AUTH,             TRUE,  "刷卡鉴权应答"},
    {IOT_AHTT_CMD_START_CHARGE,          IOT_AHTT_CMDTYPE_REQUSET,  NULL, 0, 0, IOT_AHTT_CMD_START_CHARGE,          TRUE,  "启动充电"},
    {IOT_AHTT_CMD_HEARTBEAT,             IOT_AHTT_CMDTYPE_RESPONSE, IotAHTT_RecvHeartBeatRsp, IOT_AHTT_HEART_TIMEOUT_MS, IOT_AHTT_HEART_MAX_TRY_COUNT, IOT_AHTT_CMD_HEARTBEAT, TRUE, "心跳应答"},
    {IOT_AHTT_CMD_SET_DEV_PARAM,         IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvSetDevParam, 0, 0, IOT_AHTT_CMD_SET_DEV_PARAM,         TRUE,  "设置设备运行参数"},
    {IOT_AHTT_CMD_QUERY_DEV_PARAM,       IOT_AHTT_CMDTYPE_REQUSET,  IotAHTT_RecvQueryDevParam, 0, 0, IOT_AHTT_CMD_QUERY_DEV_PARAM,       TRUE,  "查询设备运行参数"},
    {IOT_AHTT_CMD_REPORT_REALDATA,       IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_REPORT_REALDATA,       TRUE,  "实时数据应答"},
    {IOT_AHTT_CMD_REPORT_ORDER,          IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_REPORT_ORDER,          TRUE,  "订单应答"},
    {IOT_AHTT_CMD_QUERY_TIME,            IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_QUERY_TIME,            TRUE,  "平台时间应答"},
    {IOT_AHTT_CMD_REPORT_DEV_STATE,      IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_REPORT_DEV_STATE,      TRUE,  "设备状态应答"},
    {IOT_AHTT_CMD_DEV_ALARM,             IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_DEV_ALARM,             TRUE,  "设备告警应答"},
    {IOT_AHTT_CMD_NET_ALARM,             IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_NET_ALARM,             TRUE,  "网络告警应答"},
    {IOT_AHTT_CMD_TEMP_ALARM,            IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_TEMP_ALARM,            TRUE,  "温度告警应答"},
    {IOT_AHTT_CMD_SET_TEMP_LIMIT,        IOT_AHTT_CMDTYPE_REQUSET,  NULL, 0, 0, IOT_AHTT_CMD_SET_TEMP_LIMIT,        TRUE,  "设置温度阈值"},
    {IOT_AHTT_CMD_QUERY_TEMP_LIMIT,      IOT_AHTT_CMDTYPE_REQUSET,  NULL, 0, 0, IOT_AHTT_CMD_QUERY_TEMP_LIMIT,      TRUE,  "查询温度阈值"},
    {IOT_AHTT_CMD_ELECTRIC_ALARM,        IOT_AHTT_CMDTYPE_RESPONSE, NULL, 0, 0, IOT_AHTT_CMD_ELECTRIC_ALARM,        TRUE,  "电气告警应答"},
    {IOT_AHTT_CMD_UPDATE,                IOT_AHTT_CMDTYPE_REQUSET,  NULL, 0, 0, IOT_AHTT_CMD_UPDATE,                TRUE,  "远程升级"},
};

static const IotAHTTRecvCtrl_Struct *IotAHTT_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotAHTTRecvCtrl_Struct *pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_AHTT_CMD_RECV_COUNT; index++)
    {
        if (cmd == c_stIotAHTTRecvctrlTable[index].cmd)
        {
            pCtrl = &c_stIotAHTTRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotAHTTFrameHead_Struct *IotAHTT_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    uint8_t *pStart = pData;
    uint16_t remainLen = dataLen;
    IotAHTTFrameHead_Struct *pHead = NULL;
    uint16_t declareLen = 0;
    uint16_t frameLen = 0;
    uint16_t calcCrc16 = 0;
    uint16_t recvCrc16 = 0;

    while (remainLen >= IOT_AHTT_FRAME_HEAD_LEN)
    {
        pHead = (IotAHTTFrameHead_Struct *)pStart;
        if (IOT_AHTT_HEAD == pHead->head)
        {
            declareLen = Common_TwoUint8ToUint16(pHead->len);
            if ((declareLen >= IOT_AHTT_DECLARE_MIN_LEN) &&
                (declareLen <= (IOT_AHTT_FRAME_MAX_LEN - 3)))
            {
                frameLen = declareLen + 3;
                if (remainLen < frameLen)
                {
                    pHead = NULL;
                    break;
                }

                recvCrc16 = ((uint16_t)pStart[frameLen - 2] << 8) | (uint16_t)pStart[frameLen - 1];
                calcCrc16 = Common_CalcCRC16(pStart, frameLen - IOT_AHTT_CRC_LEN);
                if ((IOT_AHTT_PROTOCOL_VERSION == pHead->ver) &&
                    (0 == memcmp(pHead->deviceNum, pIotAHTTCtx->deviceNum, IOT_AHTT_DEVICE_NUM_LEN)) &&
                    (calcCrc16 == recvCrc16))
                {
                    pDealLen[0] = (uint16_t)((pStart - pData) + frameLen);
                    break;
                }
            }
        }

        pStart++;
        remainLen--;
        pDealLen[0]++;
        pHead = NULL;
    }

    return pHead;
}

static void IotAHTT_DecodeData(uint8_t *pData, uint16_t dataLen,
    uint16_t topicLen, uint8_t *pTopic, uint16_t *pDealLen)
{
    const IotAHTTRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotAHTTFrameHead_Struct *pFrameHead = NULL;
    uint8_t port = 0;
    uint8_t matchedPort = 0;
    uint8_t responseMatched = FALSE;
    uint16_t frameLen = 0;
    uint16_t paramLen = 0;
    uint16_t recvSeq = 0;


    pFrameHead = IotAHTT_FindValidFrameLen(pData, dataLen, pDealLen);
    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotAHTT_GetRecvCtrlPtr(pFrameHead->cmd);
        if ((pCmdRecvCtrl != NULL) && (pCmdRecvCtrl->pRecvParse != NULL))
        {
            frameLen = Common_TwoUint8ToUint16(pFrameHead->len) + 3;
            paramLen = frameLen - IOT_AHTT_FRAME_MIN_LEN;
            recvSeq = Common_TwoUint8ToUint16(pFrameHead->seq);

            if (IOT_AHTT_CMDTYPE_RESPONSE == pCmdRecvCtrl->cmdType)
            {
                for (matchedPort = 0; matchedPort < SYSCFG_CFG_GUN_NUM; matchedPort++)
                {
                    if (TRUE == Common_GetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
                        matchedPort, pCmdRecvCtrl->cmd))
                    {
                        port = matchedPort;
                        responseMatched = TRUE;
                        break;
                    }
                }
            }
            else
            {
                responseMatched = TRUE;
                Common_SetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, port, pFrameHead->cmd, recvSeq);
            }

            if ((TRUE == responseMatched) && (TRUE == pCmdRecvCtrl->pRecvParse(&port,
                (uint8_t *)pFrameHead + IOT_AHTT_FRAME_HEAD_LEN, paramLen)))
            {
                if (pCmdRecvCtrl->printFlag)
                {
                    IOTAHTT_CFG_DebugPrint("AHTT,[枪：%d]接收[cmd: 0x%02X, %s][%d]：", port, (uint8_t)pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                    DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                }

                if ((IOT_AHTT_CMDTYPE_RESPONSE == pCmdRecvCtrl->cmdType) &&
                    (port == matchedPort))
                {
                    Common_SetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                    Common_ClearRptCount(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    if (IOT_AHTT_CMD_NULL != pCmdRecvCtrl->matchCmd)
                    {
                        Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl, port,
                            pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else if ((IOT_AHTT_CMDTYPE_REQUSET == pCmdRecvCtrl->cmdType) &&
                    (IOT_AHTT_CMD_NULL != pCmdRecvCtrl->matchCmd))
                {
                    Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                }
                else
                {
                }
            }
            else
            {
                if (pCmdRecvCtrl->printFlag)
                {
                    IOTAHTT_CFG_DebugPrint("AHTT,[枪：%d]接收[cmd: 0x%02X, %s][%d] 处理失败!：", port, (uint8_t)pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                    DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                }
            }
        }
    }
}

static uint8_t IotAHTT_RecvLoginRsp(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    if (0 == len)
    {
        IotAHTT_LoginSuccess();
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAHTT_RecvHeartBeatRsp(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    if (0 == len)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAHTT_RecvSetHeartCycle(uint8_t *port, uint8_t *pData, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAHTTParam_Struct newParam;
    uint8_t ret = TRUE;

    pIotAHTTCtx->setHeartCycleResult = IOT_AHTT_PARAM_RESULT_FAIL;
    if ((IOT_AHTT_HEART_CYCLE_PARAM_LEN == len) &&
        (pData[0] >= IOT_AHTT_HEART_CYCLE_MIN) &&
        (pData[0] <= IOT_AHTT_HEART_CYCLE_MAX))
    {
        newParam = pPrivateParam->stAHTTParam;
        newParam.stWorkParam.heartCycleMin = pData[0];
        if (TRUE == IotAHTT_CommitPrivateParam(&newParam))
        {
            pIotAHTTCtx->setHeartCycleResult = IOT_AHTT_PARAM_RESULT_SUCCESS;
        }
    }

    return ret;
}

static uint8_t IotAHTT_RecvQueryHeartCycle(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    if (0 == len)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAHTT_RecvSetDomainPort(uint8_t *port, uint8_t *pData, uint16_t len)
{
    char domain[IOT_AHTT_DOMAIN_FIELD_LEN + 1] = {0};
    uint32_t portValue = 0;
    uint16_t reqSeq = 0;
    uint8_t index = 0;
    uint8_t domainLen = 0;
    uint8_t ret = TRUE;

    /* 域名在首个 0x00 截断，端口解析到首个非数字 ASCII 字符即止；非法地址由
       域名切换状态机在 TCP 试连失败后回滚并回失败应答 */
    if ((port != NULL) && (IOT_AHTT_DOMAIN_PORT_PARAM_LEN == len))
    {
        for (index = 0; index < IOT_AHTT_DOMAIN_FIELD_LEN; index++)
        {
            if (0 == pData[index])
            {
                break;
            }
            domain[domainLen] = (char)pData[index];
            domainLen++;
        }

        for (index = 0; index < IOT_AHTT_PORT_FIELD_LEN; index++)
        {
            if ((pData[IOT_AHTT_DOMAIN_FIELD_LEN + index] < '0') ||
                (pData[IOT_AHTT_DOMAIN_FIELD_LEN + index] > '9'))
            {
                break;
            }
            portValue = portValue * 10 +
                (uint32_t)(pData[IOT_AHTT_DOMAIN_FIELD_LEN + index] - '0');
        }

        reqSeq = Common_GetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, 0, IOT_AHTT_CMD_SET_DOMAIN_PORT);
        if (TRUE != IotAHTT_BeginDomainSwitch(domain, (uint16_t)portValue, reqSeq))
        {
            IotAHTT_QueueDomainSwitchBusyRsp(reqSeq);
        }
        else
        {
        }
    }

    return ret;
}

static uint8_t IotAHTT_RecvSetMaxChargeTime(uint8_t *port, uint8_t *pData, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAHTTParam_Struct newParam;
    uint8_t ret = TRUE;

    pIotAHTTCtx->setMaxChargeTimeResult = IOT_AHTT_PARAM_RESULT_FAIL;
    if ((IOT_AHTT_MAX_CHARGE_TIME_PARAM_LEN == len) &&
        (pData[0] >= IOT_AHTT_MAX_CHARGE_TIME_MIN) &&
        (pData[0] <= IOT_AHTT_MAX_CHARGE_TIME_MAX))
    {
        newParam = pPrivateParam->stAHTTParam;
        newParam.stWorkParam.maxChargeTimeHour = pData[0];
        if (TRUE == IotAHTT_CommitPrivateParam(&newParam))
        {
            pIotAHTTCtx->setMaxChargeTimeResult = IOT_AHTT_PARAM_RESULT_SUCCESS;
        }
    }

    return ret;
}

static uint8_t IotAHTT_RecvQueryMaxChargeTime(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    if (0 == len)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAHTT_RecvSetDevParam(uint8_t *port, uint8_t *pData, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAHTTParam_Struct newParam;
    uint8_t ret = TRUE;

    pIotAHTTCtx->setDevParamResult = IOT_AHTT_PARAM_RESULT_FAIL;
    if ((8 == len) &&
        (pData[IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_INDEX] >= IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_MIN) &&
        (pData[IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_INDEX] <= IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_MAX))
    {
        newParam = pPrivateParam->stAHTTParam;
        memcpy(newParam.stWorkParam.devOperationParam, pData, 8);
        if (TRUE == IotAHTT_CommitPrivateParam(&newParam))
        {
            pIotAHTTCtx->setDevParamResult = IOT_AHTT_PARAM_RESULT_SUCCESS;
        }
    }

    return ret;
}

static uint8_t IotAHTT_RecvQueryDevParam(uint8_t *port, uint8_t *pData, uint16_t len)
{
    uint8_t ret = FALSE;

    if (0 == len)
    {
        ret = TRUE;
    }

    return ret;
}

void IotAHTT_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotAHTTCtx->frameQueueChannelID, IotAHTT_DecodeData);
}

void IotAHTT_TimeoutDetect(void)
{
    const IotAHTTRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_AHTT_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotAHTTRecvctrlTable[index];
        if ((IOT_AHTT_CMDTYPE_RESPONSE == pCmdRecvCtrl->cmdType) &&
            (0 != pCmdRecvCtrl->maxTimeout) && (0 != pCmdRecvCtrl->maxTryCnt))
        {
            for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
            {
                if (TRUE == Common_GetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl,
                    port, pCmdRecvCtrl->cmd))
                {
                    if (TRUE == Common_JudgeTimeoutMs(Common_GetRecvTick(pIotAHTTCtx->pFuncRecvCtrl,
                        port, pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout))
                    {
                        Common_SetRptCount(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        timeoutCount = Common_GetRptCount(pIotAHTTCtx->pFuncRecvCtrl,
                            port, pCmdRecvCtrl->cmd);

                        if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                        {
                            if ((IOT_AHTT_CMD_LOGIN == pCmdRecvCtrl->cmd) ||
                                (IOT_AHTT_CMD_HEARTBEAT == pCmdRecvCtrl->cmd))
                            {
                                IotAHTT_OfflineHandle();
                            }
                        }
                        else
                        {
                            Common_SetRecvTimerEnable(pIotAHTTCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                            Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                            Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                            Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
            }
        }
    }
}
