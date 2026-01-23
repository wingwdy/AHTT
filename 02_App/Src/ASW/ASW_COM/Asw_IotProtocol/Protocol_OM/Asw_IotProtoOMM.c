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
        default: break;
    }
    return pRecvCtrl;
}

static void IotOM_WSInitHandle(void)
{



}

static void IotOM_WSOfflineHandle(void)
{

    
}

static void IotOM_WSLoginHandle(void)
{


}

static void IotOM_WSNormalHandle(void)
{


}

void IotOM_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotOMCtx != NULL)
    {
        strcpy(pLinkPara->stTcpPara.ip, pParam->platAuxiliaryIp);
        pLinkPara->stTcpPara.port = pParam->platAuxiliaryPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 2048, 2048, &pIotOMCtx->frameQueueChannelID);
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
    pIotOMCtx = (IotOMCtx_Struct *)malloc(sizeof(IotOMCtx_Struct));
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























