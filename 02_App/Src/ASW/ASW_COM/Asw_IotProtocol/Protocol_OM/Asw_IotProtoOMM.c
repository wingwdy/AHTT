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
#include "Asw_IotProtoOMRecv.h"
#include "Asw_IotProtoOMSend.h"
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_ChargeIf.h"
#include "MS_Nvm.h"
#include "myMalloc.h"
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
*    Global variables Declaration
*******************************************************************************/
IotOMCtx_Struct *pIotOMCtx = NULL;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void IotOM_WSInitHandle(void);
static void IotOM_WSOfflineHandle(void);
static void IotOM_WSLoginHandle(void);
static void IotOM_WSNormalHandle(void);
static CommonSendCtrl_Struct* IotOM_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotOM_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotOM_CycleReportRealData(void);
static void IotOM_CycleDetectUnreporteRecord(void);
static void IotOM_CycleDetectUnreportedUcmResult(void);
static void IotOM_CycleDetectReportForbidState(void);
static void IotOM_CycleReportMeterVal(void);
static void IotOM_CycleDetect(void);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotOM_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_OM_CMD_LOGIN_REQ:                  pSendCtrl = &pIotOMCtx->stSendCtrl[port][0];   break;
        case IOT_OM_CMD_HEARTBEAT_REQ:              pSendCtrl = &pIotOMCtx->stSendCtrl[port][1];   break;
        case IOT_OM_CMD_SEND_NETMODULE_INFO:        pSendCtrl = &pIotOMCtx->stSendCtrl[port][2];   break;
        case IOT_OM_CMD_CALL_NETMODULE_INFO_RSP:    pSendCtrl = &pIotOMCtx->stSendCtrl[port][3];   break;
        case IOT_OM_CMD_REPORT_REALDATA:            pSendCtrl = &pIotOMCtx->stSendCtrl[port][4];   break;
        case IOT_OM_CMD_CALL_REALDATA_ACK:          pSendCtrl = &pIotOMCtx->stSendCtrl[port][5];   break;
        case IOT_OM_CMD_REPORT_METERVAL:            pSendCtrl = &pIotOMCtx->stSendCtrl[port][6];   break;
        case IOT_OM_CMD_SET_QRCODE_RSP:             pSendCtrl = &pIotOMCtx->stSendCtrl[port][7];   break;
        case IOT_OM_CMD_REBOOT_RSP:                 pSendCtrl = &pIotOMCtx->stSendCtrl[port][8];   break;
        case IOT_OM_CMD_SET_FORBID_RSP:             pSendCtrl = &pIotOMCtx->stSendCtrl[port][9];   break;
        case IOT_OM_CMD_REPORT_FORBID_STATE:        pSendCtrl = &pIotOMCtx->stSendCtrl[port][10];   break;
        case IOT_OM_CMD_UPDATE_RSP:                 pSendCtrl = &pIotOMCtx->stSendCtrl[port][11];   break;
        case IOT_OM_CMD_ORDER_RECORD:               pSendCtrl = &pIotOMCtx->stSendCtrl[port][12];   break;
        case IOT_OM_CMD_REMOTE_QUERY_SET_PARAM_RSP: pSendCtrl = &pIotOMCtx->stSendCtrl[port][13];   break;
        case IOT_OM_CMD_CALL_READ_LOCALFILE_RSP:    pSendCtrl = &pIotOMCtx->stSendCtrl[port][14];   break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotOM_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_OM_CMD_LOGIN_RSP:                  pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][0];   break;
        case IOT_OM_CMD_HEARTBEAT_RSP:              pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][1];   break;
        case IOT_OM_CMD_CALL_NETMODULE_INFO:        pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][2];   break;
        case IOT_OM_CMD_CALL_REALDATA:              pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][3];   break;
        case IOT_OM_CMD_SET_QRCODE:                 pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][4];   break;
        case IOT_OM_CMD_REBOOT:                     pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][5];   break;
        case IOT_OM_CMD_SET_FORBID:                 pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][6];   break;
        case IOT_OM_CMD_REPORT_FORBID_STATE_RSP:    pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][7];   break;
        case IOT_OM_CMD_UPDATE:                     pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][8];   break;
        case IOT_OM_CMD_ORDER_RECORD_RSP:           pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][9];   break;
        case IOT_OM_CMD_REMOTE_QUERY_SET_PARAM:     pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][10];  break;
        case IOT_OM_CMD_CALL_READ_LOCALFILE:        pRecvCtrl = &pIotOMCtx->stRecvCtrl[port][11];  break;
        default: break;
    }
    
    return pRecvCtrl;
}

static void IotOM_WSInitHandle(void)
{
    pIotOMCtx->eWorkState = eIOTOMWorkState_Offline;
}

static void IotOM_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    uint8_t copyLen = 0;
    uint8_t offset = 0;

    memset(pIotOMCtx->pileFixDnAsc, 0, 32);
    memcpy(pIotOMCtx->pileFixDnAsc, pParam->fixPileDn, 32);

    memset(pIotOMCtx->platDn, 0, 32);
    memcpy(pIotOMCtx->platDn, pParam->platPileDn, 32);

    pIotOMCtx->loginSucc = FALSE;
    pIotOMCtx->queueBusyFlag = FALSE;
    pIotOMCtx->waitQueueIdleTick = 0;
    pIotOMCtx->reportForBidStateTick = 0;

    pIotOMCtx->sendIndex = 0;
    pIotOMCtx->sendPort = 0;
    pIotOMCtx->reqSeq = 0;

    memset(pIotOMCtx->meterValReportTick, 0x00, sizeof(pIotOMCtx->meterValReportTick));
    memset(pIotOMCtx->realDataReportTick, 0x00, sizeof(pIotOMCtx->realDataReportTick));
    memset(pIotOMCtx->lastGunState, 0x00, sizeof(pIotOMCtx->lastGunState));
    memset(pIotOMCtx->lastGunConnectState, 0x00, sizeof(pIotOMCtx->lastGunConnectState));
    memset(pIotOMCtx->lastErrInfo, 0x00, sizeof(pIotOMCtx->lastErrInfo));
    memset(pIotOMCtx->errVersion, 0x00, sizeof(pIotOMCtx->errVersion));

    memset(pIotOMCtx->stSendCtrl, 0x00, sizeof(pIotOMCtx->stSendCtrl));
    memset(pIotOMCtx->stRecvCtrl, 0x00, sizeof(pIotOMCtx->stRecvCtrl));
    FrameQueue_Reset(pIotOMCtx->frameQueueChannelID);
    pIotOMCtx->eWorkState = eIOTOMWorkState_Login;
}

static void IotOM_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_OM))
    {
        pIotOMCtx->eWorkState = eIOTOMWorkState_Normal;
        Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_LOGIN_REQ, TRUE);
    }
}

static void IotOM_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;
    uint8_t curErrInfo[32] = {0};

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotOM_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        /* 故障变位上报 */
        if (pIotOMCtx->errVersion[port] != AswErrHandle_GetErrStatusVersion(port))
        {
            pIotOMCtx->errVersion[port] = AswErrHandle_GetErrStatusVersion(port);
            IotOM_SetRealDataErrBit(port, curErrInfo);

            if (0 != memcmp(curErrInfo, pIotOMCtx->lastErrInfo[port], 32))
            {
                realDataReportFlag = TRUE;
            }
        }

        /* 枪状态变位上报 */
        if (pIotOMCtx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        /* 枪连接状态变位上报 */
        if (pIotOMCtx->lastGunConnectState[port] != curGunConnectState)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == FALSE)
        {
            realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTOM_CFG_CHARGING_REALDATA_CYCLE : IOTOM_CFG_IDLE_REALDATA_CYCLE;
        
            if (Common_JudgeTimeoutMs(pIotOMCtx->realDataReportTick[port], realDataReportCycle) == TRUE)
            {
                realDataReportFlag = TRUE;
            }
        }

        if (realDataReportFlag == TRUE)
        {
            realDataReportFlag = FALSE;
            pIotOMCtx->lastGunState[port] = curGunState;
            pIotOMCtx->lastGunConnectState[port] = curGunConnectState;
            pIotOMCtx->realDataReportTick[port] = Common_GetSystick();
            memcpy(pIotOMCtx->lastErrInfo[port], curErrInfo, 32);
            memset(curErrInfo, 0x00, 32);
            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, IOT_OM_CMD_REPORT_REALDATA, TRUE);
        }
    }
}

static void IotOM_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OmOrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotOMCtx->pFuncSendCtrl, port, IOT_OM_CMD_ORDER_RECORD) ||
                Common_GetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, IOT_OM_CMD_ORDER_RECORD_RSP))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OmOrderRecord, (uint8_t *)&pIotOMCtx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotOMCtx->time))
            {
                port = pIotOMCtx->stOrderInfo.port;

               /* 避免当数据库存在脏数据时，脏数据有问题，持续进入到这边 */
               if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotOMCtx->stOrderInfo.protocolType >= eAswPlatCardType_Count ||
                    pIotOMCtx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotOMCtx->time);
                }
                else
                {
                    Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, pIotOMCtx->stOrderInfo.port, IOT_OM_CMD_ORDER_RECORD, TRUE);
                }
            }
        }
    }
}

static void IotOM_CycleDetectUnreportedUcmResult(void)
{
    SSUcmResult_Enum UcmResult;

    if (pIotOMCtx->stProtoData[0].recvUpdateFlag == TRUE)
    {
        UcmResult = SSUcm_GetResult();

        if (UcmResult != eSSUcmResult_None && UcmResult != eSSUcmResult_Succ)
        {
            if (TRUE != Common_GetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_UPDATE_RSP))
            {
                Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_UPDATE_RSP, TRUE);
            }

            if (UcmResult == eSSUcmResult_HeadErr)
            {
                pIotOMCtx->stProtoData[0].setUpdateResult = 0x02;
            }
            else
            {
                pIotOMCtx->stProtoData[0].setUpdateResult = 0x03;
            }
        }

        pIotOMCtx->stProtoData[0].recvUpdateFlag = FALSE;
    }
}

static void IotOM_CycleDetectReportForbidState(void)
{
    if (pIotOMCtx->sendForbidStateFlag == TRUE)
    {
        if (pIotOMCtx->sendForbidStateCount >= 10)
        {
            pIotOMCtx->sendForbidStateFlag = FALSE;
        }
        else
        {
            if (Common_JudgeTimeoutMs(pIotOMCtx->reportForBidStateTick, 10000))
            {
                pIotOMCtx->reportForBidStateTick = Common_GetSystick();
                Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_REPORT_FORBID_STATE, TRUE); 
                Common_SetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_REPORT_FORBID_STATE, TRUE);
            }
        }
    }
}

static void IotOM_CycleReportMeterVal(void)
{
    uint32_t meterValReportCycle;
    uint8_t port;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        meterValReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTOM_CFG_CHARGING_REALDATA_CYCLE : IOTOM_CFG_IDLE_REALDATA_CYCLE;

        if (Common_JudgeTimeoutMs(pIotOMCtx->meterValReportTick[port], meterValReportCycle) == TRUE)
        {
            pIotOMCtx->meterValReportTick[port] = Common_GetSystick();
            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, IOT_OM_CMD_REPORT_METERVAL, TRUE);
        }
    }
}

void IotOM_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
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


static void IotOM_CycleDetect(void)
{ 
    IotOM_CycleReportRealData();

    IotOM_CycleReportMeterVal();

    IotOM_CycleDetectUnreporteRecord();

    IotOM_CycleDetectReportForbidState();

    IotOM_CycleDetectUnreportedUcmResult();
}

static void IotOM_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_OM))
    {
        IotOM_OfflineHandle();
    }
    else
    {
        if (pIotOMCtx->loginSucc == TRUE)
        {
            IotOM_CycleDetect();
        }

        IotOM_UpCtrlSendDeal();

        IotOM_UpCtrlRecvDeal();

        IotOM_TimeoutDetect();
    }
}

uint8_t IotOM_GetGunState(uint8_t port)
{
    uint8_t gunState = 0;
    uint8_t chargeState = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        chargeState = AswChargeIf_GetChargeState(port);

        if (AswErrHandle_IsExsistError(port) == TRUE)
        {
            gunState = 0x04; /* 故障中 */
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_STARTING || chargeState == ASWCHARGEIF_WORKSTATE_WAKEUP)
        {
            gunState = 0x01; /* 启动中 */
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_CHARGING)
        {
            gunState = 0x02; /* 充电中 */
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_PAUSEA || chargeState == ASWCHARGEIF_WORKSTATE_PAUSEB)
        {
            gunState = 0x06; /* 充电暂停 */
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_STOPPING)
        {
            if (AswChargeIf_GetRelayState(port) == ASWCHARGEIF_RELAYSTATE_ON)
            {
                gunState = 0x02; /* 充电中 */
            }
            else
            {
                gunState = 0x03; /* 充电结束但未拔枪 */
            }
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_FINISH)
        {
            gunState = 0x03; /* 充电结束但未拔枪 */
        }
        else
        {
            gunState = 0x00;  /* 未插枪或者插枪未充电 */
        }
    }

    return gunState;
}

void IotOM_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotOMCtx != NULL)
    {
        strncpy(pLinkPara->stTcpPara.ip, pParam->platAuxiliaryIp, sizeof(pParam->platAuxiliaryIp) - 1);
        pLinkPara->stTcpPara.port = pParam->platAuxiliaryPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 1024, 1024, &pIotOMCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotOMCtx->frameQueueChannelID;
    }
}

void IotOM_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_OM);
    pIotOMCtx->loginSucc = FALSE;
    pIotOMCtx->eWorkState = eIOTOMWorkState_Offline;
}

void IotOM_InitMemory(void)
{
    pIotOMCtx = (IotOMCtx_Struct *)myMalloc(sizeof(IotOMCtx_Struct));
    if (pIotOMCtx != NULL)
    {
        memset(pIotOMCtx, 0, sizeof(IotOMCtx_Struct));
    }

    pIotOMCtx->pFuncSendCtrl = IotOM_GetSendCtrl;
    pIotOMCtx->pFuncRecvCtrl = IotOM_GetRecvCtrl;
}

void IotOM_MainFunction(void)
{
    switch (pIotOMCtx->eWorkState)
    {
        case eIOTOMWorkState_Init:
        {
            IotOM_WSInitHandle();
            break;
        }
        case eIOTOMWorkState_Offline:
        {
            IotOM_WSOfflineHandle();
            break;
        }
        case eIOTOMWorkState_Login:
        {
            IotOM_WSLoginHandle();
            break;
        }
        case eIOTOMWorkState_Normal:
        {
            IotOM_WSNormalHandle();
            break;
        }
        default:
        {
            pIotOMCtx->eWorkState = eIOTOMWorkState_Init;
        }
    }
}























