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

    copyLen = strlen(pParam->fixPileDn);
    copyLen = copyLen > 32 ? 32 : copyLen;
    offset = 32 - copyLen;
    memset(pIotOMCtx->pileFixDnAsc, 0x30, 32);
    memcpy(pIotOMCtx->pileFixDnAsc + offset, pParam->fixPileDn, copyLen);

    copyLen = strlen(pParam->platPileDn);
    copyLen = copyLen > 32 ? 32 : copyLen;
    offset = 32 - copyLen;
    memset(pIotOMCtx->platDn, 0x30, 32);
    memcpy(pIotOMCtx->platDn + offset, pParam->platPileDn, copyLen);


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

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotOM_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        if (pIotOMCtx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        if (pIotOMCtx->lastGunConnectState[port] != curGunConnectState)
        {
            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTOM_CFG_CHARGING_REALDATA_CYCLE : IOTOM_CFG_IDLE_REALDATA_CYCLE;
       
        if (Common_JudgeTimeoutMs(pIotOMCtx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == TRUE)
        {
            pIotOMCtx->lastGunState[port] = curGunState;
            pIotOMCtx->lastGunConnectState[port] = curGunConnectState;
            pIotOMCtx->realDataReportTick[port] = Common_GetSystick();

            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, IOT_OM_CMD_REPORT_REALDATA, TRUE);
        }
    }
}

static void IotOM_CycleDetectUnreporteRecord(void)
{






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

static void IotOM_CycleDetect(void)
{ 
    IotOM_CycleReportRealData();

    IotOM_CycleReportMeterVal();

    IotOM_CycleDetectUnreporteRecord();

    IotOM_CycleDetectReportForbidState();
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
        else if (chargeState == ASWCHARGEIF_WORKSTATE_CHARGING || 
                chargeState == ASWCHARGEIF_WORKSTATE_PAUSEA || 
                chargeState == ASWCHARGEIF_WORKSTATE_PAUSEB)
        {
            gunState = 0x02; /* 充电中 */
        }
        else if (chargeState == ASWCHARGEIF_WORKSTATE_STOPPING)
        {
            if (AswChargeIf_GetRelayState(port) == AswCHARGEIF_RELAYSTATE_ON)
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
        strcpy(pLinkPara->stTcpPara.ip, pParam->platAuxiliaryIp);
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























