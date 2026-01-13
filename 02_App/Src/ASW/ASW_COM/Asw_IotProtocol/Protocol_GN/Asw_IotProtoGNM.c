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
static CommonSendCtrl_Struct* IotGN_GetSendCtrl(uint8_t port, uint8_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_REQ:  pSendCtrl = &pIotGNCtx->stSendCtrl[port][0];   break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotGN_GetRecvCtrl(uint8_t port, uint8_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_RSP:  pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][0];   break;
        default: break;
    }

    return pRecvCtrl;
}

static void IotGN_WSInitHandle(void)
{
    pIotGNCtx->eWorkState = eIOTGNWorkState_Offline;
}

static void IotGN_WSOfflineHandle(void)
{
    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotGNCtx->eWorkState = eIOTGNWorkState_Login;
}

static void IotGN_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
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
        IotLX_UpCtrlSendDeal();

        IotGN_UpCtrlRecvDeal();
    }
}

void IotGN_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    memset(pIotGNCtx->stSendCtrl, 0x00, sizeof(pIotGNCtx->stSendCtrl));
    memset(pIotGNCtx->stRecvCtrl, 0x00, sizeof(pIotGNCtx->stRecvCtrl));
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























