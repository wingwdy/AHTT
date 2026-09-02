/******************************************************************************
* File Name          : Asw_IotProtoAHTTM.c
* Description        : AHTT protocol state machine and platform adaptation
 -----------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
******************************************************************************/
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoAHTTM.h"
#include "Asw_IotProtoAHTTSend.h"
#include "Asw_IotProtoAHTTRecv.h"
#include "Asw_ErrorHandle.h"
#include "myMalloc.h"

IotAHTTCtx_Struct *pIotAHTTCtx = NULL;

static CommonSendCtrl_Struct *IotAHTT_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct *IotAHTT_GetRecvCtrl(uint8_t port, uint16_t cmd);
static uint8_t IotAHTT_InitDeviceNum(void);
static void IotAHTT_WSInitHandle(void);
static void IotAHTT_WSOfflineHandle(void);
static void IotAHTT_WSLoginHandle(void);
static void IotAHTT_WSNormalHandle(void);
static void IotAHTT_MigratePrivateParam(void);
static uint8_t IotAHTT_IsSameDomainPort(const char *pDomain, uint16_t port);
static void IotAHTT_UpdateDomainSwitchNetAddr(char *pDomain, uint16_t port);
static void IotAHTT_BeginDomainSwitchRollback(void);
static uint8_t IotAHTT_CommitDomainSwitch(void);
static void IotAHTT_SetLoginSuccess(void);
static void IotAHTT_SchedulePendingDomainSwitchBusyRsp(void);
static void IotAHTT_DomainSwitchVerifyHandle(void);

static CommonSendCtrl_Struct *IotAHTT_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct *pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_AHTT_CMD_LOGIN:                 pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][0];  break;
        case IOT_AHTT_CMD_SET_HEART_CYCLE:       pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][1];  break;
        case IOT_AHTT_CMD_QUERY_HEART_CYCLE:     pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][2];  break;
        case IOT_AHTT_CMD_SET_DOMAIN_PORT:       pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][3];  break;
        case IOT_AHTT_CMD_SET_MAX_CHARGE_TIME:   pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][4];  break;
        case IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME: pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][5];  break;
        case IOT_AHTT_CMD_STOP_CHARGE:           pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][6];  break;
        case IOT_AHTT_CMD_CARD_AUTH:             pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][7];  break;
        case IOT_AHTT_CMD_START_CHARGE:          pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][8];  break;
        case IOT_AHTT_CMD_HEARTBEAT:             pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][9];  break;
        case IOT_AHTT_CMD_SET_DEV_PARAM:         pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][10]; break;
        case IOT_AHTT_CMD_QUERY_DEV_PARAM:       pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][11]; break;
        case IOT_AHTT_CMD_REPORT_REALDATA:       pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][12]; break;
        case IOT_AHTT_CMD_REPORT_ORDER:          pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][13]; break;
        case IOT_AHTT_CMD_QUERY_TIME:            pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][14]; break;
        case IOT_AHTT_CMD_REPORT_DEV_STATE:      pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][15]; break;
        case IOT_AHTT_CMD_DEV_ALARM:             pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][16]; break;
        case IOT_AHTT_CMD_NET_ALARM:             pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][17]; break;
        case IOT_AHTT_CMD_TEMP_ALARM:            pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][18]; break;
        case IOT_AHTT_CMD_SET_TEMP_LIMIT:        pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][19]; break;
        case IOT_AHTT_CMD_QUERY_TEMP_LIMIT:      pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][20]; break;
        case IOT_AHTT_CMD_ELECTRIC_ALARM:        pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][21]; break;
        case IOT_AHTT_CMD_UPDATE:                pSendCtrl = &pIotAHTTCtx->stSendCtrl[port][22]; break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct *IotAHTT_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct *pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_AHTT_CMD_LOGIN:                 pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][0];  break;
        case IOT_AHTT_CMD_SET_HEART_CYCLE:       pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][1];  break;
        case IOT_AHTT_CMD_QUERY_HEART_CYCLE:     pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][2];  break;
        case IOT_AHTT_CMD_SET_DOMAIN_PORT:       pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][3];  break;
        case IOT_AHTT_CMD_SET_MAX_CHARGE_TIME:   pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][4];  break;
        case IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME: pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][5];  break;
        case IOT_AHTT_CMD_STOP_CHARGE:           pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][6];  break;
        case IOT_AHTT_CMD_CARD_AUTH:             pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][7];  break;
        case IOT_AHTT_CMD_START_CHARGE:          pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][8];  break;
        case IOT_AHTT_CMD_HEARTBEAT:             pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][9];  break;
        case IOT_AHTT_CMD_SET_DEV_PARAM:         pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][10]; break;
        case IOT_AHTT_CMD_QUERY_DEV_PARAM:       pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][11]; break;
        case IOT_AHTT_CMD_REPORT_REALDATA:       pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][12]; break;
        case IOT_AHTT_CMD_REPORT_ORDER:          pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][13]; break;
        case IOT_AHTT_CMD_QUERY_TIME:            pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][14]; break;
        case IOT_AHTT_CMD_REPORT_DEV_STATE:      pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][15]; break;
        case IOT_AHTT_CMD_DEV_ALARM:             pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][16]; break;
        case IOT_AHTT_CMD_NET_ALARM:             pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][17]; break;
        case IOT_AHTT_CMD_TEMP_ALARM:            pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][18]; break;
        case IOT_AHTT_CMD_SET_TEMP_LIMIT:        pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][19]; break;
        case IOT_AHTT_CMD_QUERY_TEMP_LIMIT:      pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][20]; break;
        case IOT_AHTT_CMD_ELECTRIC_ALARM:        pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][21]; break;
        case IOT_AHTT_CMD_UPDATE:                pRecvCtrl = &pIotAHTTCtx->stRecvCtrl[port][22]; break;
        default: break;
    }

    return pRecvCtrl;
}

static uint8_t IotAHTT_InitDeviceNum(void)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    uint8_t deviceNum[IOT_AHTT_DEVICE_NUM_LEN] = {0};
    uint8_t index = 0;
    uint8_t ret = FALSE;

    memset(pIotAHTTCtx->deviceNum, 0x00, sizeof(pIotAHTTCtx->deviceNum));

    if (10 == strlen(pParam->platPileDn))
    {
        ret = TRUE;

        for (index = 0; index < 10; index++)
        {
            if ((pParam->platPileDn[index] < '0') || (pParam->platPileDn[index] > '9'))
            {
                ret = FALSE;
            }
        }
    }

    if (TRUE == ret)
    {
        Common_AsciiToBCD(pParam->platPileDn, deviceNum, 10);
        for (index = 0; index < IOT_AHTT_DEVICE_NUM_LEN; index++)
        {
            pIotAHTTCtx->deviceNum[index] = deviceNum[IOT_AHTT_DEVICE_NUM_LEN - index - 1];
        }
    }

    return ret;
}

static void IotAHTT_WSInitHandle(void)
{
    pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Offline;
}

static void IotAHTT_WSOfflineHandle(void)
{
    pIotAHTTCtx->loginSucc = FALSE;
    pIotAHTTCtx->queueBusyFlag = FALSE;
    pIotAHTTCtx->waitQueueIdleTick = 0;
    pIotAHTTCtx->sendIndex = 0;
    pIotAHTTCtx->sendPort = 0;
    pIotAHTTCtx->reqSeq = IOT_AHTT_SEQ_MIN;

    memset(pIotAHTTCtx->stSendCtrl, 0x00, sizeof(pIotAHTTCtx->stSendCtrl));
    memset(pIotAHTTCtx->stRecvCtrl, 0x00, sizeof(pIotAHTTCtx->stRecvCtrl));
    FrameQueue_Reset(pIotAHTTCtx->frameQueueChannelID);
    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    if (TRUE == IotAHTT_InitDeviceNum())
    {
        pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Login;
    }
}

static void IotAHTT_WSLoginHandle(void)
{
    if ((eIotAHTTDomainSwitchState_Trying == pIotAHTTCtx->eDomainSwitchState) &&
        (TRUE == Common_JudgeTimeoutMs(pIotAHTTCtx->domainSwitchTryTick,
        IOT_AHTT_DOMAIN_SWITCH_TCP_TIMEOUT_MS)))
    {
        IotAHTT_BeginDomainSwitchRollback();
    }
    else if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Normal;
        Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_LOGIN, TRUE);
    }
}

static void IotAHTT_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotAHTT_OfflineHandle();
    }
    else
    {
        IotAHTT_UpCtrlSendDeal();
        IotAHTT_UpCtrlRecvDeal();
        IotAHTT_TimeoutDetect();
    }
}

void IotAHTT_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
}

void IotAHTT_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{
}

uint8_t IotAHTT_SwipCardCharge(uint8_t port, uint8_t *pCardID)
{
    return FALSE;
}

void IotAHTT_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord,
    uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    pRecordLen[0] = 0;
}

void IotAHTT_OfflineHandle(void)
{
    if ((eIotAHTTDomainSwitchState_Trying == pIotAHTTCtx->eDomainSwitchState) ||
        (eIotAHTTDomainSwitchState_CommitVerify == pIotAHTTCtx->eDomainSwitchState))
    {
        IotAHTT_BeginDomainSwitchRollback();
    }
    else
    {
        CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
        pIotAHTTCtx->loginSucc = FALSE;
        pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Offline;
    }
}

void IotAHTT_LoginSuccess(void)
{
    if (eIotAHTTDomainSwitchState_Trying == pIotAHTTCtx->eDomainSwitchState)
    {
        if (TRUE == IotAHTT_CommitDomainSwitch())
        {
            pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_CommitVerify;
            IotAHTT_SetLoginSuccess();
            IotAHTT_SchedulePendingDomainSwitchBusyRsp();
        }
        else
        {
            /* 候选地址入库失败：回滚旧地址并断线重连，本次登录成功作废 */
            IotAHTT_BeginDomainSwitchRollback();
        }
    }
    else if (eIotAHTTDomainSwitchState_Rollback == pIotAHTTCtx->eDomainSwitchState)
    {
        /* 旧地址回滚签到成功：转失败应答待发送态，调度 0x04 失败应答 */
        pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_FailRspPending;
        IotAHTT_SetLoginSuccess();
        IotAHTT_ScheduleDomainSwitchFailRsp(pIotAHTTCtx->domainSwitchReqSeq);
    }
    else if (eIotAHTTDomainSwitchState_Idle == pIotAHTTCtx->eDomainSwitchState)
    {
        /* 无切换事务的常规登录成功 */
        IotAHTT_SetLoginSuccess();
        IotAHTT_SchedulePendingDomainSwitchBusyRsp();
    }
    else
    {
        /* FailRspPending 边缘情形（理论上不可达）：仅置登录成功 */
        IotAHTT_SetLoginSuccess();
    }
}

static void IotAHTT_SetLoginSuccess(void)
{
    pIotAHTTCtx->loginSucc = TRUE;
    AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
    Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_HEARTBEAT, TRUE);
    Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_HEARTBEAT, TRUE);
}

static uint8_t IotAHTT_IsSameDomainPort(const char *pDomain, uint16_t port)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    uint8_t ret = FALSE;

    if ((pDomain != NULL) && (pParam->platMainPort == port) &&
        (0 == strcmp(pParam->platMainIp, pDomain)))
    {
        ret = TRUE;
    }

    return ret;
}

static void IotAHTT_UpdateDomainSwitchNetAddr(char *pDomain, uint16_t port)
{
    CddNetM_UpdateIpPort(eCddNetMPlatType_O, pDomain, port);
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotAHTTCtx->loginSucc = FALSE;
    pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Offline;
}

uint8_t IotAHTT_BeginDomainSwitch(const char *pDomain, uint16_t port, uint16_t reqSeq)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    uint8_t ret = FALSE;

    if ((pDomain != NULL) &&
        (eIotAHTTDomainSwitchState_Idle == pIotAHTTCtx->eDomainSwitchState))
    {
        if (TRUE == IotAHTT_IsSameDomainPort(pDomain, port))
        {
            ret = TRUE;
        }
        else
        {
            memset(pIotAHTTCtx->oldDomain, 0x00, sizeof(pIotAHTTCtx->oldDomain));
            strncpy(pIotAHTTCtx->oldDomain, pParam->platMainIp,
                sizeof(pIotAHTTCtx->oldDomain) - 1);
            pIotAHTTCtx->oldDomainPort = pParam->platMainPort;
            memset(pIotAHTTCtx->candidateDomain, 0x00, sizeof(pIotAHTTCtx->candidateDomain));
            strncpy(pIotAHTTCtx->candidateDomain, pDomain,
                sizeof(pIotAHTTCtx->candidateDomain) - 1);
            pIotAHTTCtx->candidateDomainPort = port;
            pIotAHTTCtx->domainSwitchReqSeq = reqSeq;
            pIotAHTTCtx->domainSwitchTryTick = Common_GetSystick();
            pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_Trying;
            IotAHTT_UpdateDomainSwitchNetAddr(pIotAHTTCtx->candidateDomain,
                pIotAHTTCtx->candidateDomainPort);
            ret = TRUE;
        }
    }

    return ret;
}

static void IotAHTT_BeginDomainSwitchRollback(void)
{
    pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_Rollback;
    IotAHTT_UpdateDomainSwitchNetAddr(pIotAHTTCtx->oldDomain, pIotAHTTCtx->oldDomainPort);
}

static uint8_t IotAHTT_CommitDomainSwitch(void)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    GlobalRet_Enum writeRet = eGlobalRet_OK;
    GlobalRet_Enum restoreRet = eGlobalRet_OK;
    uint8_t ret = FALSE;

    memset(pParam->platMainIp, 0x00, sizeof(pParam->platMainIp));
    strncpy(pParam->platMainIp, pIotAHTTCtx->candidateDomain, sizeof(pParam->platMainIp) - 1);
    pParam->platMainPort = pIotAHTTCtx->candidateDomainPort;
    writeRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam,
        sizeof(MSNvmPlatParam_Struct));
    if (eGlobalRet_OK == writeRet)
    {
        ret = TRUE;
    }
    else
    {
        memset(pParam->platMainIp, 0x00, sizeof(pParam->platMainIp));
        strncpy(pParam->platMainIp, pIotAHTTCtx->oldDomain, sizeof(pParam->platMainIp) - 1);
        pParam->platMainPort = pIotAHTTCtx->oldDomainPort;
        restoreRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam,
            sizeof(MSNvmPlatParam_Struct));
        if (eGlobalRet_OK != restoreRet)
        {
            IOTAHTT_CFG_DebugPrint("AHTT域名切换写入失败后恢复失败：%d\r\n", restoreRet);
        }
    }

    return ret;
}

void IotAHTT_ScheduleDomainSwitchFailRsp(uint16_t reqSeq)
{
    pIotAHTTCtx->domainSwitchResult = IOT_AHTT_PARAM_RESULT_FAIL;
    Common_SetRecvSeq(pIotAHTTCtx->pFuncRecvCtrl, 0, IOT_AHTT_CMD_SET_DOMAIN_PORT, reqSeq);
    Common_SetSendEnable(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_SET_DOMAIN_PORT, TRUE);
    Common_SetSendFlag(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_SET_DOMAIN_PORT, FALSE);
    Common_SetSendImmdFlag(pIotAHTTCtx->pFuncSendCtrl, 0, IOT_AHTT_CMD_SET_DOMAIN_PORT, TRUE);
}

void IotAHTT_QueueDomainSwitchBusyRsp(uint16_t reqSeq)
{
    if (FALSE == pIotAHTTCtx->domainSwitchBusyRspPending)
    {
        pIotAHTTCtx->domainSwitchBusyRspPending = TRUE;
        pIotAHTTCtx->domainSwitchBusyReqSeq = reqSeq;
    }
}

static void IotAHTT_SchedulePendingDomainSwitchBusyRsp(void)
{
    uint16_t reqSeq = 0;

    if (TRUE == pIotAHTTCtx->domainSwitchBusyRspPending)
    {
        reqSeq = pIotAHTTCtx->domainSwitchBusyReqSeq;
        pIotAHTTCtx->domainSwitchBusyRspPending = FALSE;
        IotAHTT_ScheduleDomainSwitchFailRsp(reqSeq);
    }
}

/******************************************************************************
* @brief 推进域名切换的NVM异步校验结果
* @note Step1：候选地址签到后等待平台参数块回读确认，不提前结束事务。
*       Step2：校验成功才提交新地址；最终失败立即恢复旧地址并走既有回滚签到。
*       Step3：该函数不改变公共NVM重试和备份策略。
******************************************************************************/
static void IotAHTT_DomainSwitchVerifyHandle(void)
{
    MSNvmWriteVerifyState_Enum eVerifyState = eMSNvmWriteVerifyState_Pending;

    if (eIotAHTTDomainSwitchState_CommitVerify == pIotAHTTCtx->eDomainSwitchState)
    {
        eVerifyState = MSNvm_GetParaBlockWriteVerifyState(eMSNvmBlockID_PlatParam);
        if (eMSNvmWriteVerifyState_Success == eVerifyState)
        {
            pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_Idle;
            IotAHTT_SchedulePendingDomainSwitchBusyRsp();
        }
        else if (eMSNvmWriteVerifyState_Failed == eVerifyState)
        {
            IotAHTT_BeginDomainSwitchRollback();
        }
        else
        {
        }
    }
}

void IotAHTT_NotifyDomainSwitchRspQueued(void)
{
    if (eIotAHTTDomainSwitchState_FailRspPending == pIotAHTTCtx->eDomainSwitchState)
    {
        pIotAHTTCtx->eDomainSwitchState = eIotAHTTDomainSwitchState_Idle;
        IotAHTT_SchedulePendingDomainSwitchBusyRsp();
    }
}

uint8_t IotAHTT_GetGunState(uint8_t port)
{
    uint8_t gunState = IOT_AHTT_HEART_STATE_IDLE;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        gunState = IOT_AHTT_HEART_STATE_FAULT;
    }
    else if (AswMonitor_IsOrderIdle(port) != TRUE)
    {
        gunState = IOT_AHTT_HEART_STATE_WORK;
    }
    else
    {
    }

    return gunState;
}

uint8_t IotAHTT_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    uint8_t ret = FALSE;

    if ((pLinkPara != NULL) && (pIotAHTTCtx != NULL))
    {
        if (eGlobalRet_OK == FrameQueue_Creat(eFrameQueueType_TCP,
            IOT_AHTT_FRAME_QUEUE_BUF_SIZE, IOT_AHTT_FRAME_QUEUE_BUF_SIZE,
            &pIotAHTTCtx->frameQueueChannelID))
        {
            strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pLinkPara->stTcpPara.ip) - 1);
            pLinkPara->stTcpPara.ip[sizeof(pLinkPara->stTcpPara.ip) - 1] = '\0';
            pLinkPara->stTcpPara.port = pParam->platMainPort;
            pLinkPara->stTcpPara.frameQueueChannelID = pIotAHTTCtx->frameQueueChannelID;
            ret = TRUE;
        }
    }

    return ret;
}

void IotAHTT_InitMemory(void)
{
    pIotAHTTCtx = (IotAHTTCtx_Struct *)myMalloc(sizeof(IotAHTTCtx_Struct));
    if (pIotAHTTCtx != NULL)
    {
        memset(pIotAHTTCtx, 0, sizeof(IotAHTTCtx_Struct));
        pIotAHTTCtx->pFuncSendCtrl = IotAHTT_GetSendCtrl;
        pIotAHTTCtx->pFuncRecvCtrl = IotAHTT_GetRecvCtrl;
        pIotAHTTCtx->reqSeq = IOT_AHTT_SEQ_MIN;
        IotAHTT_MigratePrivateParam();
    }
}

uint8_t IotAHTT_CommitPrivateParam(const MSNvmAHTTParam_Struct *pNewParam)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAHTTParam_Struct oldParam;
    GlobalRet_Enum writeRet = eGlobalRet_OK;
    GlobalRet_Enum restoreRet = eGlobalRet_OK;
    uint8_t ret = FALSE;

    if (pNewParam != NULL)
    {
        oldParam = pPrivateParam->stAHTTParam;
        if (0 == memcmp(&oldParam, pNewParam, sizeof(MSNvmAHTTParam_Struct)))
        {
            if (TRUE == pIotAHTTCtx->privateParamPersistPending)
            {
                writeRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                    (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                if (eGlobalRet_OK == writeRet)
                {
                    pIotAHTTCtx->privateParamPersistPending = FALSE;
                    ret = TRUE;
                }
            }
            else
            {
                ret = TRUE;
            }
        }
        else
        {
            pPrivateParam->stAHTTParam = *pNewParam;
            writeRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            if (eGlobalRet_OK == writeRet)
            {
                pIotAHTTCtx->privateParamPersistPending = FALSE;
                ret = TRUE;
            }
            else
            {
                pPrivateParam->stAHTTParam = oldParam;
                restoreRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                    (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                if (eGlobalRet_OK != restoreRet)
                {
                    pIotAHTTCtx->privateParamPersistPending = TRUE;
                    IOTAHTT_CFG_DebugPrint("AHTT私有参数写入失败后恢复失败：%d\r\n", restoreRet);
                }
                else
                {
                    pIotAHTTCtx->privateParamPersistPending = FALSE;
                }
            }
        }
    }

    return ret;
}

static void IotAHTT_MigratePrivateParam(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAHTTParam_Struct newParam;
    GlobalRet_Enum writeRet = eGlobalRet_OK;

    newParam = pPrivateParam->stAHTTParam;
    if (newParam.stWorkParam.paramVersion != IOT_AHTT_PRIVATE_PARAM_VERSION)
    {
        if ((newParam.stWorkParam.heartCycleMin < IOT_AHTT_HEART_CYCLE_MIN) ||
            (newParam.stWorkParam.heartCycleMin > IOT_AHTT_HEART_CYCLE_MAX))
        {
            newParam.stWorkParam.heartCycleMin = 5;
        }

        if ((newParam.stWorkParam.maxChargeTimeHour < IOT_AHTT_MAX_CHARGE_TIME_MIN) ||
            (newParam.stWorkParam.maxChargeTimeHour > IOT_AHTT_MAX_CHARGE_TIME_MAX))
        {
            newParam.stWorkParam.maxChargeTimeHour = 10;
        }

        newParam.stWorkParam.paramVersion = IOT_AHTT_PRIVATE_PARAM_VERSION;
        pPrivateParam->stAHTTParam = newParam;
        writeRet = MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
            (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        if (eGlobalRet_OK != writeRet)
        {
            pIotAHTTCtx->privateParamPersistPending = TRUE;
            IOTAHTT_CFG_DebugPrint("AHTT私有参数迁移写入失败，保留安全参数并等待重试\r\n");
        }
        else
        {
            pIotAHTTCtx->privateParamPersistPending = FALSE;
        }
    }
}

void IotAHTT_MainFunction(void)
{
    if (pIotAHTTCtx != NULL)
    {
        IotAHTT_DomainSwitchVerifyHandle();
        switch (pIotAHTTCtx->eWorkState)
        {
            case eIOTAHTTWorkState_Init:
            {
                IotAHTT_WSInitHandle();
                break;
            }
            case eIOTAHTTWorkState_Offline:
            {
                IotAHTT_WSOfflineHandle();
                break;
            }
            case eIOTAHTTWorkState_Login:
            {
                IotAHTT_WSLoginHandle();
                break;
            }
            case eIOTAHTTWorkState_Normal:
            {
                IotAHTT_WSNormalHandle();
                break;
            }
            default:
            {
                pIotAHTTCtx->eWorkState = eIOTAHTTWorkState_Init;
                break;
            }
        }
    }
}

void IotAHTT_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam)
{
    MSNvmAHTTParam_Struct *pAHTTParam = &pPrivateParam->stAHTTParam;

    memset(pAHTTParam, 0x00, sizeof(MSNvmAHTTParam_Struct));
    pAHTTParam->stWorkParam.heartCycleMin = 5;
    pAHTTParam->stWorkParam.maxChargeTimeHour = 10;
    pAHTTParam->stWorkParam.paramVersion = IOT_AHTT_PRIVATE_PARAM_VERSION;
}
