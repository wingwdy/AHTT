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
#include "Asw_IotProtoXDTM.h"
#include "Asw_IotProtoXDTSend.h"
#include "Asw_IotProtoXDTRecv.h"
#include "Asw_PlatM.h"
#include "FrameQueue.h"
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
IotXDTCtx_Struct *pIotXDTCtx = NULL;

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotXDT_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL:      pSendCtrl = &pIotXDTCtx->stSendCtrl[port][0];   break;
        case IOT_XDT_CMD_REQUEST_TIMESYNC:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][1];   break;
        case IOT_XDT_CMD_REQUEST_LINK:                 pSendCtrl = &pIotXDTCtx->stSendCtrl[port][2];   break;
        case IOT_XDT_CMD_SET_RESTART_RSP:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][3];   break;
        case IOT_XDT_CMD_REQUEST_RATEMODE:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][4];   break;
        case IOT_XDT_CMD_QUERY_RATEMODE_RSP:           pSendCtrl = &pIotXDTCtx->stSendCtrl[port][5];   break;
        case IOT_XDT_CMD_RATEMODE_SET_RSP:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][6];   break;
        case IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT:       pSendCtrl = &pIotXDTCtx->stSendCtrl[port][7];   break;
        case IOT_XDT_CMD_PILE_STATE:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][8];   break;
        case IOT_XDT_CMD_ERRINFO:                      pSendCtrl = &pIotXDTCtx->stSendCtrl[port][9];   break;
        case IOT_XDT_CMD_PILE_DATA:                    pSendCtrl = &pIotXDTCtx->stSendCtrl[port][10];  break;
        case IOT_XDT_CMD_CALL_REALDATA:                pSendCtrl = &pIotXDTCtx->stSendCtrl[port][11];  break;
        case IOT_XDT_CMD_REQUEST_CARDAUTH:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][12];  break;
        case IOT_XDT_SET_CATEGORY:                     pSendCtrl = &pIotXDTCtx->stSendCtrl[port][13];  break;
        case IOT_XDT_REQUEST_ERCODE:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][14];  break;
        case IOT_XDT_SET_ERCODE_RSP:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][15];  break;
        case IOT_XDT_SET_ERCODE_RSP_EVENT:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][16];  break;
        case IOT_XDT_QUERY_ERCODE_RSP:                 pSendCtrl = &pIotXDTCtx->stSendCtrl[port][17];  break;
        case IOT_XDT_CHARGE_START_RSP:                 pSendCtrl = &pIotXDTCtx->stSendCtrl[port][18];  break;
        case IOT_XDT_CHARGE_START_EVNET:               pSendCtrl = &pIotXDTCtx->stSendCtrl[port][19];  break;
        case IOT_XDT_CHARGE_STOP_RSP:                  pSendCtrl = &pIotXDTCtx->stSendCtrl[port][20];  break;
        case IOT_XDT_CHARGE_STOP_EVNET:                pSendCtrl = &pIotXDTCtx->stSendCtrl[port][21];  break;
        case IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP:       pSendCtrl = &pIotXDTCtx->stSendCtrl[port][22];  break;
        case IOT_XDT_CHARGE_START_EVNETA:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][23];  break;
        case IOT_XDT_QUERY_BOARDINFO_RSP:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][24];  break;
        case IOT_XDT_PARA_SET_RSP:                     pSendCtrl = &pIotXDTCtx->stSendCtrl[port][25];  break;
        case IOT_XDT_PARA_QUERY_RSP:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][26];  break;
        case IOT_XDT_CHARGE_PWRCTRL_RSP:               pSendCtrl = &pIotXDTCtx->stSendCtrl[port][27];  break;
        case IOT_XDT_CHARGE_RECORD:                    pSendCtrl = &pIotXDTCtx->stSendCtrl[port][28];  break;
        case IOT_XDT_QUERY_CHARGE_RECORD_RSP:          pSendCtrl = &pIotXDTCtx->stSendCtrl[port][29];  break;
        case IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE:        pSendCtrl = &pIotXDTCtx->stSendCtrl[port][30];  break;
        case IOT_XDT_CMD_FIRMWARE_STATE:               pSendCtrl = &pIotXDTCtx->stSendCtrl[port][31];  break;
        default:
        {
            break;
        } 
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotXDT_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP:  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][0];   break;
        case IOT_XDT_CMD_REQUEST_TIMESYNC_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][1];   break;
        case IOT_XDT_CMD_REQUEST_LINK_RSP:             pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][2];   break;
        case IOT_XDT_CMD_SET_RESTART:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][3];   break;
        case IOT_XDT_CMD_REQUEST_RATEMODE_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][4];   break;
        case IOT_XDT_CMD_QUERY_RATEMODE:               pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][5];   break;
        case IOT_XDT_CMD_RATEMODE_SET:                 pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][6];   break;
        case IOT_XDT_CMD_PILE_STATE_RSP:               pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][7];   break;
        case IOT_XDT_CMD_ERRINFO_RSP:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][8];   break;
        case IOT_XDT_CMD_CALL_REALDATA_RSP:            pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][9];   break;
        case IOT_XDT_CMD_REQUEST_CARDAUTH_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][10];  break;
        case IOT_XDT_SET_CATEGORY_RSP:                 pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][11];  break;
        case IOT_XDT_REQUEST_ERCODE_RSP:               pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][12];  break;
        case IOT_XDT_SET_ERCODE:                       pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][13];  break;
        case IOT_XDT_QUERY_ERCODE:                     pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][14];  break;
        case IOT_XDT_CHARGE_START:                     pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][15];  break;
        case IOT_XDT_CHARGE_STOP:                      pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][16];  break;
        case IOT_XDT_QUERY_BOARDINFO:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][17];  break;
        case IOT_XDT_PARA_SET:                         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][18];  break;
        case IOT_XDT_PARA_QUERY:                       pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][19];  break;
        case IOT_XDT_CHARGE_PWRCTRL:                   pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][20];  break;
        case IOT_XDT_CHARGE_RECORD_RSP:                pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][21];  break;
        case IOT_XDT_QUERY_CHARGE_RECORD:              pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][22];  break;
        case IOT_XDT_CMD_OTA_ATTRIBUTE_SET:            pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][23];  break;
        case IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP:    pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][24];  break;
        case IOT_XDT_CHARGE_CONTINUE_CHARGE:           pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][25];  break;
        default:
        {
            break;
        }
    }

    return pRecvCtrl;
}

static void IotXDT_WSInitHandle(void)
{
    pIotXDTCtx->eWorkState = eIotXDTWorkState_Offline;
}

static void IotXDT_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    pIotXDTCtx->loginSucc = FALSE;
    pIotXDTCtx->queueBusyFlag = FALSE;
    pIotXDTCtx->waitQueueIdleTick = 0;

    pIotXDTCtx->sendIndex = 0;
    pIotXDTCtx->sendPort = 0;    
    pIotXDTCtx->reqSeq = 0;

    pIotXDTCtx->t1SetFlag = FALSE;
    pIotXDTCtx->t2SetFlag = FALSE;

    memset(pIotXDTCtx->stSendCtrl, 0x00, sizeof(pIotXDTCtx->stSendCtrl));
    memset(pIotXDTCtx->stRecvCtrl, 0x00, sizeof(pIotXDTCtx->stRecvCtrl));

    FrameQueue_Reset(pIotXDTCtx->frameQueueChannelID);
    memcpy(pIotXDTCtx->platDn, pParam->platPileDn, 16);


    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotXDTCtx->eWorkState = eIotXDTWorkState_Login;
}

static void IotXDT_WSLoginHandle(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;

    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        if (pPlatInfo->credentialSaveFlag != TRUE)
        {
            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL, TRUE);
        }

        pIotXDTCtx->eWorkState = eIotXDTWorkState_Normal;
    }
}

static void IotXDT_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotXDT_WSOfflineHandle();
    }
    else
    {
        if (pIotXDTCtx->loginSucc == TRUE)
        {
 //           IotXDT_CycleDetect();
        }

        IotXDT_UpCtrlSendDeal();

        IotXDT_UpCtrlRecvDeal();

  //      IotXDT_TimeoutDetect();
    }
}

static void IotXDT_MqttConnectCallback(uint8_t connectResult, uint8_t *pCredential)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t flag = FALSE;

    if (connectResult == TRUE)
    {
        /* 如果已获取到凭据,且拿着新凭据连接成功 */
        if (pPlatInfo->credentialSaveFlag == TRUE)
        {
            if (pPlatInfo->credentialValidFlag != TRUE)
            {
                pPlatInfo->credentialValidFlag = TRUE;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            }
        }

        flag = pPlatInfo->credentialSaveFlag;
    }
    else
    {
        if (pPlatInfo->credentialSaveFlag == TRUE || pPlatInfo->credentialValidFlag == TRUE)
        {
            pPlatInfo->credentialSaveFlag = FALSE;
            pPlatInfo->credentialValidFlag = FALSE;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }
    }

    if (pCredential != NULL)
    {
        pCredential[0] = flag;
    }
}

void IotXDT_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotXDTCtx->loginSucc = FALSE;
    pIotXDTCtx->eWorkState = eIotXDTWorkState_Offline;
}

uint8_t IotXDT_SetProductKey(char *pProductKey, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_PRODUCT_KEY_LEN)
    {
        memset(pPlatInfo->cProductKey, 0, MSNVM_XDT_PRODUCT_KEY_LEN);
        memcpy(pPlatInfo->cProductKey, pProductKey, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetProductKey(char *pProductKey, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t productKeyLen = 0;

    if (pProductKey != NULL && pOutLen != NULL)
    {
        productKeyLen = strlen(pPlatInfo->cProductKey);
        productKeyLen = (productKeyLen >= MSNVM_XDT_PRODUCT_KEY_LEN) ? MSNVM_XDT_PRODUCT_KEY_LEN : productKeyLen;
        memcpy(pProductKey, pPlatInfo->cProductKey, productKeyLen);
        *pOutLen = productKeyLen;
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_SetProductSecret(char *pProductSecret, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_PRODUCT_SECRET_LEN)
    {
        memset(pPlatInfo->cProductSecret, 0, MSNVM_XDT_PRODUCT_SECRET_LEN);
        memcpy(pPlatInfo->cProductSecret, pProductSecret, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetProductSecret(char *pProductSecret, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t productSecretLen = 0;

    if (pProductSecret != NULL && pOutLen != NULL)
    {
        productSecretLen = strlen(pPlatInfo->cProductSecret);
        productSecretLen = (productSecretLen >= MSNVM_XDT_PRODUCT_SECRET_LEN) ? MSNVM_XDT_PRODUCT_SECRET_LEN : productSecretLen;
        memcpy(pProductSecret, pPlatInfo->cProductSecret, productSecretLen);
        *pOutLen = productSecretLen;
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_SetDevOperator(char *pDevOperator, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_DEV_OPERATOR_LEN)
    {
        memset(pPlatInfo->cOperator, 0, MSNVM_XDT_DEV_OPERATOR_LEN);
        memcpy(pPlatInfo->cOperator, pDevOperator, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetDevOperator(char *pDevOperator, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t devOperatorLen = 0;

    if (pDevOperator != NULL && pOutLen != NULL)
    {
        devOperatorLen = strlen(pPlatInfo->cOperator);
        devOperatorLen = (devOperatorLen >= MSNVM_XDT_DEV_OPERATOR_LEN) ? MSNVM_XDT_DEV_OPERATOR_LEN : devOperatorLen;
        memcpy(pDevOperator, pPlatInfo->cOperator, devOperatorLen);
        *pOutLen = devOperatorLen;
        ret = TRUE;
    }

    return ret;
}

void IotXDT_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();

    if (pLinkPara != NULL && pIotXDTCtx != NULL)
    {
        strcpy(pLinkPara->stMqttPara.ip, pParam->platMainIp);
        pLinkPara->stMqttPara.port = pParam->platMainPort;

        pLinkPara->stMqttPara.eVersion = eCddNetMMqttVersion_V3_1_1;
        pLinkPara->stMqttPara.keepAliveTime = 180;

        if (pPrivateParam->stXDTParam.platinfo.credentialSaveFlag == TRUE)
        {
            strcpy(pLinkPara->stMqttPara.userName, pPrivateParam->stXDTParam.platinfo.cUserName);
            strcpy(pLinkPara->stMqttPara.password, pPrivateParam->stXDTParam.platinfo.cPassword);
        }
        else
        {
            strcpy(pLinkPara->stMqttPara.userName, "provision");
            strcpy(pLinkPara->stMqttPara.password, "provision");
        }

        memcpy(pLinkPara->stMqttPara.pid, pParam->platPileDn, 16);

        pLinkPara->stMqttPara.topicCount = 0;
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/r/res/+");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/r/req/+");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/fw/error");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/a");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/a/res/+");

        pLinkPara->stMqttPara.pFuncMqttConnectCallback = IotXDT_MqttConnectCallback;

        FrameQueue_Creat(eFrameQueueType_MQTT, 3072, 3072, &pIotXDTCtx->frameQueueChannelID);
        pLinkPara->stMqttPara.frameQueueChannelID = pIotXDTCtx->frameQueueChannelID;
    }
}

void IotXDT_InitMemory(void)
{
    pIotXDTCtx = (IotXDTCtx_Struct *)myMalloc(sizeof(IotXDTCtx_Struct));

    if (pIotXDTCtx != NULL)
    {
        memset(pIotXDTCtx, 0, sizeof(IotXDTCtx_Struct));
    }

    pIotXDTCtx->pFuncSendCtrl = IotXDT_GetSendCtrl;
    pIotXDTCtx->pFuncRecvCtrl = IotXDT_GetRecvCtrl;
}

void IotXDT_MainFunction(void)
{
    switch (pIotXDTCtx->eWorkState)
    {
        case eIotXDTWorkState_Init:
        {
            IotXDT_WSInitHandle();
            break;
        }
        case eIotXDTWorkState_Offline:
        {
            IotXDT_WSOfflineHandle();
            break;
        }
        case eIotXDTWorkState_Login:
        {
            IotXDT_WSLoginHandle();
            break;
        }
        case eIotXDTWorkState_Normal:
        {
            IotXDT_WSNormalHandle();
            break;
        }
        default:
        {
            pIotXDTCtx->eWorkState = eIotXDTWorkState_Init;
        }
    }
}





















