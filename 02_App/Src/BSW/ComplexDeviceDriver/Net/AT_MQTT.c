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
#include "AT_MQTT.h"
#include "Cdd_NetM.h"
#include "Cdd_Drv_EG800AK.h"
#include "FrameQueue.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint32_t cycleReadTickStart;
    uint32_t reconnectInterval;

    uint8_t topicSubscribeIndex;

    uint8_t waitMqttOpenOkFlag;
    uint32_t waitMqttOpenOkTickStart;

    uint8_t waitMqttConnectOkFlag;
    uint32_t waitMqttConnectOkTickStart;

    uint32_t cycleDetectSocketStateTickStart;
}ATMQTTPrivate_Struct;




/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t ATMQTT_PackConfig(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATMQTT_PackOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATMQTT_PackConnect(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATMQTT_PackSubscribe(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATMQTT_PackPublish(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);

static uint8_t ATMQTT_RecvConnect(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATMQTT_RecvSubscribe(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATMQTT_RecvQueryState(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATMQTT_RecvClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATMQTT_RecvPublish(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);

static uint8_t ATMQTT_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID);

static void ATMQTT_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stMQTTATCmdDescribtor[eATMQTTCmd_Count] =
{
    [eATMQTTCmd_ConfigDataFormat] =
    { "AT+QMTCFG=\"dataformat\",[ID]\r\n",        "+QMTCFG:",            "OK\r\n",      3,   5000,     3000,  TRUE,    "查询数据格式",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_ConfigVersion] =
    { "AT+QMTCFG=\"version\",[ID],[VSN]\r\n",     "OK\r\n",               NULL,        3,   5000,     3000,  TRUE,    "MQTT版本配置",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_ConfigPing] =
    { "AT+QMTCFG=\"qmtping\",[ID],30\r\n",        "OK\r\n",               NULL,        3,   5000,     3000,  TRUE,    "PingReq配置",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_ConfigKeepAlive] =
    { "AT+QMTCFG=\"keepalive\",[ID],[ALIVE]\r\n", "OK\r\n",               NULL,        3,   5000,     3000,  TRUE,    "保活配置",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_ConfigCleanSession] =
    { "AT+QMTCFG=\"session\",[ID], 1\r\n",        "OK\r\n",               NULL,         3,   5000,     3000,  TRUE,    "配置会话类型",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_ConfigRecvMode] =
    { "AT+QMTCFG=\"recv/mode\",[ID],0,1\r\n",     "OK\r\n",               NULL,        3,   5000,     3000,  TRUE,    "配置接收模式",
    ATMQTT_PackConfig,                            NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_Open] =
    { "AT+QMTOPEN=[ID],\"[IP]\",[PORT]\r\n",      "OK\r\n",               NULL,        3,   30000,   10000,  TRUE,    "打开客户端网络",
    ATMQTT_PackOpen,                              NULL,                                ATMQTT_FailHandle },

    [eATMQTTCmd_Connect] =
    { "AT+QMTCONN=[ID],\"[PID]\",\"[NAME]\",\"[PASSWORD]\"\r\n","OK\r\n",  NULL,       3,   30000,  3000,      TRUE,      "连接客户端服务器",
    ATMQTT_PackConnect,                           ATMQTT_RecvConnect,                  ATMQTT_FailHandle },

    [eATMQTTCmd_Subscribe] =
    { "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n",   "+QMTSUB:",              NULL,       10,   1000,     500,  TRUE,    "订阅主题",
    ATMQTT_PackSubscribe,                         ATMQTT_RecvSubscribe,                ATMQTT_FailHandle },

    [eATMQTTCmd_Publish] =
    { "AT+QMTPUBEX=[ID],3,1,0,\"[TOPIC]\",[LEN]\r\n", "> ",                NULL,       3,   5000,      3000,  TRUE,   "发布消息",
    ATMQTT_PackPublish,                            ATMQTT_RecvPublish,                 ATMQTT_FailHandle },

    [eATMQTTCmd_QueryState] =
    { "AT+QMTCONN?\r\n",                          "+QMTCONN",            "OK\r\n",    3,   5000,      3000,  TRUE,   "查询连接状态",
    NULL,                                         ATMQTT_RecvQueryState,               ATMQTT_FailHandle },

    [eATMQTTCmd_Close] =
    { "AT+QMTCLOSE=[ID]\r\n",                     "OK\r\n",             NULL,          1,   5000,      3000,  TRUE,   "关闭客户端网络",
    ATMQTT_PackConfig,                            ATMQTT_RecvClose,                    ATMQTT_FailHandle },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void ATMQTT_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (eSocketState != pSocketCtrl->eSocketState)
        {
            pSocketCtrl->eSocketState = eSocketState;

            if (eSocketState == eCddNetMSocketState_Connecting)
            {
                pPrivate->waitMqttOpenOkFlag = FALSE;
                pPrivate->waitMqttConnectOkFlag = FALSE;
            }
            else if (eSocketState == eCddNetMSocketState_ConnectOK)
            {
                pSocketCtrl->reconectTimes = 0;

                if (pSocketCtrl->ePlatType == eCddNetMPlatType_O)
                {
                    CDDDRV_EG800AK_CFG_LogPrint("[socket: %d]运营平台建立连接成功!\r\n", socketIndex);
                }
                else if (pSocketCtrl->ePlatType == eCddNetMPlatType_OM)
                {
                    CDDDRV_EG800AK_CFG_LogPrint("[socket: %d]运维平台建立连接成功!\r\n", socketIndex);
                }
                else
                {}
            }
            else if (eSocketState == eCddNetMSocketState_WaitReconnect)
            {
                pPrivate->reconnectInterval = 0;
            }
        }
    }
}
static uint16_t ATMQTT_PackConfig(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

	nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);

    if (pSocketPara->eVersion == eCddNetMMqttVersion_V3_1_1)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[VSN]", 4, 4);
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[VSN]", 3, 3);
    }

    if (pSocketPara->keepAliveTime > 0)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[ALIVE]", pSocketPara->keepAliveTime, pSocketPara->keepAliveTime);
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[ALIVE]", 180, 180);
    }

	return nATLen;
}

static uint16_t ATMQTT_PackOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
    nATLen = Common_ReplaceStr(pData, nATLen, "[IP]", pSocketPara->ip, strlen(pSocketPara->ip), "00000000000000");
    nATLen = Common_ReplaceNum(pData, nATLen, "[PORT]", pSocketPara->port, 0);

    if (pPrivate->waitMqttOpenOkFlag == FALSE)
    {
        pPrivate->waitMqttOpenOkFlag = TRUE;
        pPrivate->waitMqttOpenOkTickStart = Common_GetSystick();
    }

    return nATLen;
}

static uint16_t ATMQTT_PackConnect(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

    nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
    nATLen = Common_ReplaceStr(pData, nATLen, "[PID]", pSocketPara->pid, strlen(pSocketPara->pid), "00000000000000");
    nATLen = Common_ReplaceStr(pData, nATLen, "[NAME]", pSocketPara->userName, strlen(pSocketPara->userName), "00000000000000");
    nATLen = Common_ReplaceStr(pData, nATLen, "[PASSWORD]", pSocketPara->password, strlen(pSocketPara->password), "00000000000000");
    return nATLen;
}

static uint16_t ATMQTT_PackSubscribe(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketPara->topicCount == 0 ||  pPrivate->topicSubscribeIndex >= pSocketPara->topicCount)
    {
        nATLen = 0;
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
        nATLen = Common_ReplaceNum(pData, nATLen, "[MID]", pPrivate->topicSubscribeIndex + 1, pPrivate->topicSubscribeIndex + 1);
        nATLen = Common_ReplaceStr(pData, nATLen, "[TOPIC]", pSocketPara->topic[pPrivate->topicSubscribeIndex], 
            strlen(pSocketPara->topic[pPrivate->topicSubscribeIndex]), "00000000000000");
    }

    return nATLen;
}

static uint16_t ATMQTT_PackPublish(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;
    char topic[CDD_NETM_CFG_MQTT_TOPIC_LEN + 1] = {0};
    uint16_t nDataLen = 0;

    if (eGlobalRet_OK == FrameQueue_GetLastTxFrameDataLen(pSocketPara->frameQueueChannelID, &nDataLen, topic, NULL))
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
        nATLen = Common_ReplaceStr(pData, nATLen, "[TOPIC]", topic, strlen(topic), "00000000000000");
        nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", nDataLen, nDataLen);
    }
    else
    {
        nATLen = 0;
    }

    return nATLen;
}

static uint8_t ATMQTT_RecvConnect(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->waitMqttConnectOkFlag = TRUE;
    pPrivate->waitMqttConnectOkTickStart = Common_GetSystick();
    return TRUE;
}

static uint8_t ATMQTT_RecvSubscribe(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    int32_t clientIndex = 0, msgId = 0, result = 0;

    pTemp = Common_SearchData(pData, dataLen, "+QMTSUB:", strlen("+QMTSUB:"));
    
    if (pTemp != NULL)
    {
        if (sscanf((char*)pTemp, "+QMTSUB: %d,%d,%d,", &clientIndex, &msgId, &result) == 3)
        {
            if (result == 0 && clientIndex == socketIndex)
            {
                pPrivate->topicSubscribeIndex++;
                
                if (pPrivate->topicSubscribeIndex >= pSocketPara->topicCount)
                {
                    pPrivate->topicSubscribeIndex = 0;
                    ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                    ret = TRUE;
                }
                else
                {
                    ret = FALSE;
                }
            }
        }
    }

    return ret;
}

static uint8_t ATMQTT_RecvQueryState(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pMqttPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    int32_t clinetIndex = 0;
    int32_t connectState = 0;
    uint8_t credentialFlag = TRUE;

    pTemp = Common_SearchData(pData, dataLen, "+QMTCONN:", strlen("+QMTCONN:"));

    if (pTemp != NULL)
    {
        if (sscanf((char*)pTemp, "+QMTCONN: %d,%d\r", &clinetIndex, &connectState) == 2)
        {
            if (connectState == 3)
            {
                if (pPrivate->waitMqttConnectOkFlag == TRUE)
                {
                    pPrivate->waitMqttConnectOkFlag = FALSE;

                    if (pMqttPara->pFuncMqttConnectCallback != NULL)
                    {
                        pMqttPara->pFuncMqttConnectCallback(TRUE, &credentialFlag);
                    }

                    /* 如果凭据有效，那么订阅主题  */
                    if (credentialFlag == TRUE)
                    {
                        if (pMqttPara->topicCount > 0)
                        {
                            pPrivate->topicSubscribeIndex = 0;
                            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_Subscribe);
                        }
                        else
                        {
                            ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                        }
                    }
                    else
                    {
                        ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                    }
                }
            }

            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t ATMQTT_RecvClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    return TRUE;
}

static uint8_t ATMQTT_RecvPublish(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Send);
    FrameQueue_TransmitTxData(pSocketPara->frameQueueChannelID,  CDDDRVEG800AK_CFG_WriteData, pSocketCtrl);
    return TRUE;
}

static uint8_t ATMQTT_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    uint8_t ret = FALSE;

    if (atTaskID == eATMQTTCmd_Close)
    {
        ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    }
    else
    {
        ATMQTT_CloseSocket(pSocketCtrl);
        ret = TRUE;
    }

    return ret;
}

static void ATMQTT_PeriodDetectWriteData(CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl)
{
    CddNetMSocketPara_Union *pSocketPara = (CddNetMSocketPara_Union *)pSocketCtrl->specificPara;
    uint16_t dataLen = 0;

     if (eGlobalRet_OK == FrameQueue_GetLastTxFrameDataLen(pSocketPara->stMqttPara.frameQueueChannelID, &dataLen, NULL, NULL))
     {
        if (dataLen > 0)
        {
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_Publish);
        }
     }
}

static void ATMQTT_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMMqttPara_Struct *pMqttPara)
{
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Connecting)
    {
        if (pPrivate->waitMqttOpenOkFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->waitMqttOpenOkTickStart, ATMQTT_WAIT_OPEN_TIMEOUT))
            {
                ATMQTT_CloseSocket(pSocketCtrl);
            }
        }
        else if (pPrivate->waitMqttConnectOkFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->waitMqttConnectOkTickStart, ATMQTT_WAIT_CONNECT_TIMEOUT))
            {
                ATMQTT_CloseSocket(pSocketCtrl);
            }
            else if (Common_JudgeTimeoutMs(pPrivate->cycleDetectSocketStateTickStart, ATMQTT_DECTECT_STATE_PERIOD))
            {
                pPrivate->cycleDetectSocketStateTickStart = Common_GetSystick();
                CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_QueryState);
            }
            else
            {}
        }
        else
        {}
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
    {
        if (TRUE != CddDrvEG800AK_CheckTransparentMode())
        {
            if (Common_JudgeTimeoutMs(pSocketCtrl->periodDetectDataSendTick, ATMQTT_CYCLE_WRITE_PERIOD))
            {
                pSocketCtrl->periodDetectDataSendTick = Common_GetSystick();
                ATMQTT_PeriodDetectWriteData(pSocketCtrl);
            }
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_Abnormal)
    {

    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_WaitReconnect)
    {
        if (pPrivate->reconnectInterval == 0)
        {
            pSocketCtrl->reconectTimes++;

            if (pSocketCtrl->reconectTimes >= CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES)
            {
                pSocketCtrl->reconectTimes = CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES;
            }

            if (pSocketCtrl->socketDisconnectCallback != NULL)
            {
                pSocketCtrl->socketDisconnectCallback();
            }

            CDDDRV_EG800AK_CFG_RECONECT_TIMEOUT(pSocketCtrl->reconectTimes, pPrivate->reconnectInterval);
            pSocketCtrl->disconectTickStart = Common_GetSystick();
            CDDDRV_EG800AK_CFG_LogPrint("[socket: %d] %d ms 后进行第 %d 次 重新连接!\r\n", socketIndex, pPrivate->reconnectInterval, pSocketCtrl->reconectTimes);
        }
        else
        {
            if (Common_JudgeTimeoutMs(pSocketCtrl->disconectTickStart, pPrivate->reconnectInterval))
            {
                ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Init);
            }
        }
    }
    else
    {}
}

void ATMQTT_UpdateIpPort(void *socketPara, char *pIp, uint16_t port)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

    strncpy(pSocketPara->ip, pIp, CDD_NETM_CFG_IP_LEN);
    pSocketPara->port = port;
}

void ATMQTT_UpdateMqttUserNamePassword(void *socketPara, char *pUserName, char *pPassword)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMMqttPara_Struct *pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

    strncpy(pSocketPara->userName, pUserName, CDD_NETM_CFG_MQTT_USER_NAME_LEN);
    strncpy(pSocketPara->password, pPassword, CDD_NETM_CFG_MQTT_PASSWORD_LEN);
}

uint32_t ATMQTT_UrcQMTOpen(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    ATMQTTPrivate_Struct *pPrivate = NULL;
    int32_t socketIndex = 0;
    int32_t connectState = 0;

    if (2 == sscanf((char*)pData, "+QMTOPEN: %d,%d\r\n", &socketIndex, &connectState))
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
            pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

            if (connectState == 0)
            {
                if (pPrivate->waitMqttOpenOkFlag == TRUE)
                {
                    pPrivate->waitMqttOpenOkFlag = FALSE;
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_Connect);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("[socket: %d]连接失败，errcode: %d !\r\n", socketIndex, connectState);

                if (connectState == 2 || connectState == 3)
                {
                    CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
                    CddDrvEG800AK_SetAbnormalType(eCddDrvEG800AKAbnormalHandle_CFun);
                }
                else
                {
                    ATMQTT_CloseSocket(pSocketCtrl);
                }
            }
        }
    }

    return 0;
}

uint32_t ATMQTT_UrcQMTConnect(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddNetMMqttPara_Struct *pMqttPara = NULL;
    ATMQTTPrivate_Struct *pPrivate = NULL;
    int32_t socketIndex = 0;
    int32_t result = 0;
    int32_t connectState = 0;
    uint8_t credentialFlag = TRUE;

    if (3 == sscanf((char*)pData, "+QMTCONN: %d,%d,%d\r\n", &socketIndex, &result, &connectState))
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
            pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;
            pMqttPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

            if (connectState == 0 && result == 0)
            {
                if (pPrivate->waitMqttConnectOkFlag == TRUE)
                {
                    pPrivate->waitMqttConnectOkFlag = FALSE;

                    if (pMqttPara->pFuncMqttConnectCallback != NULL)
                    {
                        pMqttPara->pFuncMqttConnectCallback(TRUE, &credentialFlag);
                    }

                    /* 如果凭据有效，那么订阅主题  */
                    if (credentialFlag == TRUE)
                    {
                        if (pMqttPara->topicCount > 0)
                        {
                            pPrivate->topicSubscribeIndex = 0;
                            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_Subscribe);
                        }
                        else
                        {
                            ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                        }
                    }
                    else
                    {
                        ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                    }
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("[socket: %d]连接失败，errcode: %d !\r\n", socketIndex, connectState);

                if (pMqttPara->pFuncMqttConnectCallback != NULL)
                {
                    pMqttPara->pFuncMqttConnectCallback(FALSE, NULL);
                }

                if (connectState == 2)
                {
                    CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
                    CddDrvEG800AK_SetAbnormalType(eCddDrvEG800AKAbnormalHandle_CFun);
                }
                else
                {
                    ATMQTT_CloseSocket(pSocketCtrl);
                }
            }
        }
    }

    return 0;
}

uint32_t ATMQTT_UrcQMTPubex(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AK_ExitTransparentMode();

    return 0;
}

uint32_t ATMQTT_UrcQMTStat(uint8_t *pData, void * modulePara, uint16_t dataLen) 
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    int32_t socketIndex = 0;
    int32_t connectState = 0;

    if (2 == sscanf((char*)pData, "+QMTSTAT: %d,%d\r\n", &socketIndex, &connectState))
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];

            if (connectState != 0)
            {
                if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
                {
                    ATMQTT_CloseSocket(pSocketCtrl);
                    CDDDRV_EG800AK_CFG_LogPrint("[socket: %d] 后台主动断开连接!\r\n", socketIndex);
                }
            }
        }
    }

    return 0;
} 

uint32_t ATMQTT_UrcQMTRecv(uint8_t *pData, void * modulePara, uint16_t dataLen) 
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddNetMMqttPara_Struct *pSocketPara = NULL;
	uint8_t *pTemp = NULL, *pPayload = NULL, *pCurrent = pData;
	int32_t topicLen = 0, payloadLen = 0, msgId = 0, parsed_chars = 0;
    int32_t socketIndex = 0;
    int32_t remainingLen = dataLen;
    char topic_buf[CDD_NETM_CFG_MQTT_TOPIC_LEN + 1] = {0};
    int32_t offset = 0;

    /* 循环处理所有粘在一起的报文 */
    while (remainingLen > 0)
    {
        /* 寻找接收到数据标志 */
        pTemp = Common_SearchData(pCurrent, remainingLen, "+QMTRECV:", strlen("+QMTRECV:"));

        if (pTemp != NULL)
        {
            /* 计算当前标志位置相对于起始位置的偏移 */
            offset = pTemp - pCurrent;
            
            if (4 == sscanf((char *)pTemp, "+QMTRECV: %d,%d,\"%32[^\"]\",%d,%n",&socketIndex, &msgId, topic_buf, &payloadLen, &parsed_chars))
            {
                pPayload = pTemp + parsed_chars;
                pPayload += 1;

                /* 检查解析的数据长度是否在剩余长度范围内 */
                if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT && (parsed_chars + payloadLen) <= remainingLen - offset)
                {
                    pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
                    pSocketPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;
                    FrameQueue_PushRx(pSocketPara->frameQueueChannelID, topic_buf, strlen(topic_buf), pPayload, payloadLen);
                }

                /* 更新当前指针和剩余长度，准备处理下一个报文 */
                pCurrent = pTemp + parsed_chars + payloadLen + 1;
                remainingLen = dataLen - (pCurrent - pData);
            }
            else
            {
                /* 解析失败，移动到下一个位置继续搜索 */
                pCurrent = pTemp + 1;
                remainingLen = dataLen - (pCurrent - pData);
            }
        }
        else
        {
            /* 没有找到更多的报文标志，退出循环 */
            break;
        }
    }

    return (pCurrent - pData);
}


void ATMQTT_CloseSocket(void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    ATMQTTPrivate_Struct *pPrivate = (ATMQTTPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->waitMqttOpenOkFlag = FALSE;
    pPrivate->waitMqttConnectOkFlag = FALSE;
    pPrivate->topicSubscribeIndex = 0;

    if (pSocketCtrl->eSocketState != eCddNetMSocketState_Init &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_Abnormal &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect)
    {
        CddDrvEG800AK_ClearSocketCmd(pSocketCtrl->socketIndex);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATMQTTCmd_Close);
        ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Abnormal);
    }
}


void ATMQTT_StateHandle(uint8_t socketIndex, void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    CddNetMMqttPara_Struct *pMqttPara = (CddNetMMqttPara_Struct *)pSocketCtrl->specificPara;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE && 
            CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work &&
            CddNetM_CheckFileLinkExsit() == FALSE)
        {
            memset(pSocketCtrl->user_data, 0, sizeof(pSocketCtrl->user_data));
            ATMQTT_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Connecting);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigDataFormat);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigVersion);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigPing);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigKeepAlive);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigCleanSession);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_ConfigRecvMode);
            CddDrvEG800AK_AddCmd(socketIndex, eATMQTTCmd_Open);
        }
    }
    else
    {
        ATMQTT_SocketStateMange(socketIndex, pSocketCtrl, pMqttPara);
    }
}








