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
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoGNM.h"
#include "Asw_ErrorHandle.h" 
#include "Asw_IotProtoGNSend.h"
#include "Asw_IotProtoGNRecv.h"
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
*    Global variables Declaration
*******************************************************************************/




/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
IotGNCtx_Struct *pIotGNCtx = NULL;


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotGN_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_REQ:            pSendCtrl = &pIotGNCtx->stSendCtrl[port][0];   break;
        case IOT_GN_CMD_HEARTBEAT_REQ:        pSendCtrl = &pIotGNCtx->stSendCtrl[port][1];   break;
        case IOT_GN_CMD_BILLMODE_VERIFY_REQ:  pSendCtrl = &pIotGNCtx->stSendCtrl[port][2];   break;
        case IOT_GN_CMD_BILLMODE_REQ:         pSendCtrl = &pIotGNCtx->stSendCtrl[port][3];   break;
        case IOT_GN_CMD_REPORT_REALDATA:      pSendCtrl = &pIotGNCtx->stSendCtrl[port][4];   break;
        case IOT_GN_CMD_CALL_REALDATA_ACK:    pSendCtrl = &pIotGNCtx->stSendCtrl[port][5];   break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotGN_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_RSP:              pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][0];   break;
        case IOT_GN_CMD_HEARTBEAT_RSP:          pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][1];   break;
        case IOT_GN_CMD_BILLMODE_VERIFY_RSP:    pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][2];   break;
        case IOT_GN_CMD_BILLMODE_4RATE_RSP:     pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][3];   break;
        case IOT_GN_CMD_BILLMODE_MUTIRATE_RSP:  pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][4];   break;
        case IOT_GN_CMD_CALL_REALDATA:          pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][5];   break;
        default: break;
    }

    return pRecvCtrl;
}

static void IotGN_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotGN_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        if (pIotGNCtx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        if (pIotGNCtx->lastGunConnectState[port] != curGunConnectState)
        {
            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (IotGN_IsCharging(port) == TRUE) ? IOTGN_CFG_CHARGING_REALDATA_CYCLE : IOTGN_CFG_IDLE_REALDATA_CYCLE;
       
        if (Common_JudgeTimeoutMs(pIotGNCtx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == TRUE)
        {
            pIotGNCtx->lastGunState[port] = curGunState;
            pIotGNCtx->lastGunConnectState[port] = curGunConnectState;
            pIotGNCtx->realDataReportTick[port] = Common_GetSystick();

            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_REPORT_REALDATA, TRUE);
        }
    }
}

static void IotGN_CycleDetect(void)
{ 
    IotGN_CycleReportRealData();
}

static void IotGN_WSInitHandle(void)
{
    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)&pIotGNCtx->param, sizeof(MSNvmPlatPrivateParam_Union)) != eGlobalRet_OK)
    {
        memset(&pIotGNCtx->param, 0x00, sizeof(MSNvmPlatPrivateParam_Union));
    }

    pIotGNCtx->eWorkState = eIOTGNWorkState_Offline;
}

static void IotGN_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    pIotGNCtx->queueBusyFlag = FALSE;
    pIotGNCtx->waitQueueIdleTick = 0;

    pIotGNCtx->sendIndex = 0;
    pIotGNCtx->sendPort = 0;
    pIotGNCtx->reqSeq = 0;

    memset(pIotGNCtx->realDataReportTick, 0x00, sizeof(pIotGNCtx->realDataReportTick));
    memset(pIotGNCtx->lastGunState, 0x00, sizeof(pIotGNCtx->lastGunState));
    memset(pIotGNCtx->lastGunConnectState, 0x00, sizeof(pIotGNCtx->lastGunConnectState));

    memset(pIotGNCtx->stSendCtrl, 0x00, sizeof(pIotGNCtx->stSendCtrl));
    memset(pIotGNCtx->stRecvCtrl, 0x00, sizeof(pIotGNCtx->stRecvCtrl));

    FrameQueue_Reset(pIotGNCtx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotGNCtx->pileDnBCD, 14);

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotGNCtx->eWorkState = eIOTGNWorkState_Login;
}

static void IotGN_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotGNCtx->eWorkState = eIOTGNWorkState_Normal;
        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_LOGIN_REQ, TRUE);
    }
}

static void IotGN_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotGN_OfflineHandle();
    }
    else
    {
        IotGN_CycleDetect();

        IotLX_UpCtrlSendDeal();

        IotGN_UpCtrlRecvDeal();

        IotGN_TimeoutDetect();
    }
}

uint8_t IotGN_GetGunState(uint8_t port)
{
    uint8_t gunState = 00;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (AswErrHandle_IsExsistError(port) == TRUE)
        {
            gunState = 0x01; /* 故障 */
        }
        else if (IotGN_IsCharging(port))
        {
            gunState = 0x03; /* 充电中 */
        }
        else
        {
            gunState = 0x02; /* 空闲 */
        }
    }

    return gunState;
}

uint8_t IotGN_IsCharging(uint8_t port)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);
    uint8_t ret = TRUE;

    if (chargeState == ASWCHARGEIF_WORKSTATE_IDLE ||
        chargeState == ASWCHARGEIF_WORKSTATE_READY ||
        chargeState == ASWCHARGEIF_WORKSTATE_FINISH)
    {
        ret = FALSE;
    }

    return ret;
}



void IotGN_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotGNCtx->eWorkState = eIOTGNWorkState_Offline;
}

void IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotGNCtx != NULL)
    {
        strcpy(pLinkPara->stTcpPara.ip, pParam->platMainIp);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 3072, 3072, &pIotGNCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotGNCtx->frameQueueChannelID;
    }
}

void IotGN_InitMemory(void)
{
    pIotGNCtx = (IotGNCtx_Struct *)malloc(sizeof(IotGNCtx_Struct));
    if (pIotGNCtx != NULL)
    {
        memset(pIotGNCtx, 0, sizeof(IotGNCtx_Struct));
    }

    pIotGNCtx->pFuncSendCtrl = IotGN_GetSendCtrl;
    pIotGNCtx->pFuncRecvCtrl = IotGN_GetRecvCtrl;
}

void IotGN_MainFunction(void)
{
    switch (pIotGNCtx->eWorkState)
    {
        case eIOTGNWorkState_Init:
        {
            IotGN_WSInitHandle();
            break;
        }
        case eIOTGNWorkState_Offline:
        {
            IotGN_WSOfflineHandle();
            break;
        }
        case eIOTGNWorkState_Login:
        {
            IotGN_WSLoginHandle();
            break;
        }
        case eIOTGNWorkState_Normal:
        {
            IotGN_WSNormalHandle();
            break;
        }
        default:
        {
            pIotGNCtx->eWorkState = eIOTGNWorkState_Init;
        }
    }
}























