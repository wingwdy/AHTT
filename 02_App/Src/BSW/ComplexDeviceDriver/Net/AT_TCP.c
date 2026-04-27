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
#include "AT_TCP.h"
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
    uint8_t waitTcpConnectOkFlag;
    uint32_t waitTcpConnectOkTickStart;
    uint32_t cycleDetectSocketStateTickStart;
}ATTCPPrivate_Struct;



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void ATTCP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMTcpPara_Struct *pTcpPara);
static void ATTCP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState);

static uint16_t ATTCP_PackQIPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATTCP_PackOpenSocket(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATTCP_PackReadData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATTCP_PackWriteData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATTCP_PackQIPQurey(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);

static uint8_t ATTCP_RecvOKACK(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATTCP_RecvOpenSocket(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATTCP_RecvData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATTCP_RecvWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATTCP_RecvClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATTCP_RecvQuery(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);

static uint8_t ATTCP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stTCPATCmdDescribtor[eATTCPCmd_Count] =
{
    [eATTCPCmd_Open] =
    { "AT+QIOPEN=1,[ID],\"TCP\",\"[MIP]\",[MPORT],0,0\r\n",     "OK\r\n",          NULL,   3,   20000,     3000,  TRUE, "建立连接",
    ATTCP_PackOpenSocket,                                       ATTCP_RecvOpenSocket,                    ATTCP_FailHandle},

    [eATTCPCmd_Read] =
    { "AT+QIRD=[ID],1460\r\n",                                  "+QIRD",            NULL,   3,   3000,      500,   FALSE, "数据读取",
    ATTCP_PackReadData,                                         ATTCP_RecvData,                          ATTCP_FailHandle},

    [eATTCPCmd_Write] =
    { "AT+QISEND=[ID],[LEN]\r\n",                               "> ",                NULL,   3,   3000,      2000,  FALSE, "数据发送",
    ATTCP_PackWriteData,                                        ATTCP_RecvWrite,                         ATTCP_FailHandle},

    [eATTCPCmd_Close] =
    { "AT+QICLOSE=[ID]\r\n",                                    "OK\r\n",          NULL,   3,   5000,      3000,  TRUE, "关闭连接",
    ATTCP_PackQIPClose,                                         ATTCP_RecvClose,                         ATTCP_FailHandle},

    [eATTCPCmd_QueryState] =
    { "AT+QISTATE=1,[ID]\r\n",                                  "+QISTATE:",         "OK\r\n",   3,   5000,      3000,  FALSE, "查询连接状态",
    ATTCP_PackQIPQurey,                                         ATTCP_RecvQuery,                         ATTCP_FailHandle},
};

/*************************************************************************
*    Function Source Code
*******************************************************************************/
static uint16_t ATTCP_PackQIPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

	nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
	return nATLen;
}

static uint16_t ATTCP_PackQIPQurey(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

	nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
	return nATLen;
}

static uint16_t ATTCP_PackOpenSocket(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMTcpPara_Struct *pTcpPara = (CddNetMTcpPara_Struct *)pSocketCtrl->specificPara;

    nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
    nATLen = Common_ReplaceStr(pData, nATLen, "[MIP]", pTcpPara->ip, strlen(pTcpPara->ip), "evse.gongniu.cn");
    nATLen = Common_ReplaceNum(pData, nATLen, "[MPORT]", pTcpPara->port, 5455);
    return nATLen;
}

static uint16_t ATTCP_PackReadData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
    return nATLen;
}

static uint16_t ATTCP_PackWriteData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMSocketPara_Union *pSocketPara = (CddNetMSocketPara_Union *)pSocketCtrl->specificPara;
    uint16_t dataLen = 0;

    if (eGlobalRet_OK == FrameQueue_GetLastTxFrameDataLen(pSocketPara->stTcpPara.frameQueueChannelID, &dataLen, NULL, NULL))
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
        nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", dataLen, dataLen);
    }
    else
    {
        nATLen = 0;
    }

    return nATLen;
}

static uint8_t ATTCP_RecvQIPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATTCP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    return TRUE;
}


static uint8_t ATTCP_RecvOpenSocket(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    pTemp = Common_SearchData(pData, dataLen, "OK", strlen("OK"));
    
    if (pTemp != NULL)
    {
        pPrivate->waitTcpConnectOkFlag = TRUE;
        pPrivate->waitTcpConnectOkTickStart = Common_GetSystick();
        pPrivate->cycleDetectSocketStateTickStart = Common_GetSystick();
        ret = TRUE;
    }

    return ret;
}

static uint8_t ATTCP_RecvData(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;
    CddNetMSocketPara_Union *pSocketPara = (CddNetMSocketPara_Union *)pSocketCtrl->specificPara;
    uint8_t *pTemp = NULL;
    uint8_t *pDest = NULL;
    uint8_t ret = FALSE;
    int32_t recvLen = 0;

    pTemp = Common_SearchData(pData, dataLen, "ERROR", strlen("ERROR"));

    if (pTemp == NULL)
    {
        pTemp = Common_SearchData(pData, dataLen, "+QIRD:", strlen("+QIRD:"));

        if (pTemp != NULL)
        {
            ret = TRUE;

            if (sscanf((char*)pTemp, "+QIRD: %d\r", &recvLen) == 1)
            {
                pDest = Common_SearchData(pTemp, dataLen - (pTemp - pData), "\r\n", strlen("\r\n"));

                if (pDest != NULL)
                {
                    if (recvLen > 0)
                    {
                        pDest += strlen("\r\n");
                        FrameQueue_PushRx(pSocketPara->stTcpPara.frameQueueChannelID, NULL, 0, pDest, recvLen);
                        pDest += recvLen;
                    }

                    pDealLen[0] = (pDest - pData);

                    pDest = Common_SearchData(pDest, dataLen - (pDest - pData), "OK\r\n", strlen("OK\r\n"));

                    if (pDest != NULL)
                    {
                        pDealLen[0] = (pDest - pData) + strlen("OK\r\n");
                    }
                }
                else
                {
                    pDealLen[0] = dataLen;
                }
            }
        }
    }

    return ret;
}

static uint8_t ATTCP_RecvWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{  
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMSocketPara_Union *pSocketPara = (CddNetMSocketPara_Union *)pSocketCtrl->specificPara;
    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Send);
    FrameQueue_TransmitTxData(pSocketPara->stTcpPara.frameQueueChannelID,  CDDDRVEG800AK_CFG_WriteData, pSocketCtrl);
    return TRUE;
}

static uint8_t ATTCP_RecvClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{  
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATTCP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    return TRUE;
}

static uint8_t ATTCP_RecvQuery(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;
    char service_type[16], ip_address[64];     
    int32_t connectID, remote_port, local_port, socket_state, contextID, access_mode, serverID;        
    uint8_t *pTemp = Common_SearchData(pData, dataLen, "+QISTATE:", strlen("+QISTATE:"));
    uint8_t parseCnt = 0;

    if (pTemp != NULL)
    {
        parseCnt = sscanf((char *)pTemp, "+QISTATE: %d,\"%15[^\"]\", \"%63[^\"]\",%d,%d,%d,%d,%d,", &connectID, 
            service_type, ip_address, &remote_port, &local_port, &socket_state, &contextID, &access_mode);

        if (8 == parseCnt)
        {
            /* 状态: 0=初始 1=正在打开 2=已连接 3=监听中 4=正在关闭 */
            if (socket_state == 2)
            {
                if (pPrivate->waitTcpConnectOkFlag == TRUE)
                {
                    pPrivate->waitTcpConnectOkFlag = FALSE;
                    ATTCP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                }
            }   
        }
    }

    return TRUE;
}

static uint8_t ATTCP_RecvOKACK(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    pTemp = Common_SearchData(pData, dataLen, "OK", strlen("OK"));
    
    if (pTemp != NULL)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t ATTCP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    uint8_t ret = FALSE;

    if (atTaskID == eATTCPCmd_Close)
    {
        ATTCP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    }
    else
    {
        ATTCP_CloseSocket(pSocketCtrl);
        ret = TRUE;
    }
    
    return ret;
}

static void ATTCP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (eSocketState != pSocketCtrl->eSocketState)
        {
            pSocketCtrl->eSocketState = eSocketState;

            if (eSocketState == eCddNetMSocketState_Connecting)
            {
                pPrivate->waitTcpConnectOkFlag = FALSE;
            }
            else if (eSocketState == eCddNetMSocketState_ConnectOK)
            {
                pSocketCtrl->reconectTimes = 0;

                if (pSocketCtrl->ePlatType == eCddNetMPlatType_O)
                {
                    CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d]运营平台建立连接成功!\r\n", socketIndex);
                }
                else if (pSocketCtrl->ePlatType == eCddNetMPlatType_OM)
                {
                    CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d]运维平台建立连接成功!\r\n", socketIndex);
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

static void ATTCP_PeriodDetectWriteData(CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl)
{
    CddNetMSocketPara_Union *pSocketPara = (CddNetMSocketPara_Union *)pSocketCtrl->specificPara;
    uint16_t dataLen = 0;

     if (eGlobalRet_OK == FrameQueue_GetLastTxFrameDataLen(pSocketPara->stTcpPara.frameQueueChannelID, &dataLen, NULL, NULL))
     {
        if (dataLen > 0)
        {
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATTCPCmd_Write);
        }
     }
}

static void ATTCP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMTcpPara_Struct *pTcpPara)
{
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Connecting)
    {
        if (pPrivate->waitTcpConnectOkFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->waitTcpConnectOkTickStart, ATTCP_WAIT_IPOPEN_TIMEOUT))
            {
                ATTCP_CloseSocket(pSocketCtrl);
            }
            else if (Common_JudgeTimeoutMs(pPrivate->cycleDetectSocketStateTickStart, ATTCP_DECTECT_STATE_PERIOD))
            {
                pPrivate->cycleDetectSocketStateTickStart = Common_GetSystick();
                CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATTCPCmd_QueryState);
            }
            else
            {}
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
    {
        if (Common_JudgeTimeoutMs(pPrivate->cycleReadTickStart, ATTCP_CYCLE_READ_PERIOD))
        {
            pPrivate->cycleReadTickStart = Common_GetSystick();
            CddDrvEG800AK_AddCmd(socketIndex, eATTCPCmd_Read);
        }

        if (TRUE != CddDrvEG800AK_CheckTransparentMode())
        {
            if (Common_JudgeTimeoutMs(pSocketCtrl->periodDetectDataSendTick, ATTCP_CYCLE_WRITE_PERIOD))
            {
                pSocketCtrl->periodDetectDataSendTick = Common_GetSystick();
                ATTCP_PeriodDetectWriteData(pSocketCtrl);
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
            CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d] %d ms 后进行第 %d 次 重新连接!\r\n", socketIndex, pPrivate->reconnectInterval, pSocketCtrl->reconectTimes);
        }
        else
        {
            if (Common_JudgeTimeoutMs(pSocketCtrl->disconectTickStart, pPrivate->reconnectInterval))
            {
                ATTCP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Init);
            }
        }
    }
    else
    {}
}



uint32_t ATTCP_UrcQIPOpen(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    ATTCPPrivate_Struct *pPrivate = NULL;
    int32_t socketIndex = 0;
    int32_t connectState = 0;

    if (2 == sscanf((char*)pData, "+QIOPEN: %d,%d\r\n", &socketIndex, &connectState))
    {
        if ((socketIndex >= 0) && (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT))
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
            pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;

            if (connectState == 0)
            {
                if (pPrivate->waitTcpConnectOkFlag == TRUE)
                {
                    pPrivate->waitTcpConnectOkFlag = FALSE;
                    ATTCP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d]连接失败，errcode: %d !\r\n", socketIndex, connectState);
                ATTCP_CloseSocket(pSocketCtrl);
            }
        }
    }
    
    return 0;
}

uint32_t ATTCP_UrcSendOK(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AK_ExitTransparentMode();

    return 0;
}

uint32_t ATTCP_UrcClose(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    int32_t socketIndex = 0;

    if (sscanf((char*)pData, "+QIURC: \"closed\",%d", &socketIndex) == 1)
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
            ATTCP_CloseSocket(pSocketCtrl);
            CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d] 后台主动断开连接!\r\n", socketIndex);
        }
    }

    return 0;
}

uint32_t ATTCP_UrcRecv(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    int32_t socketIndex = 0;
    ATTCPPrivate_Struct *pPrivate = NULL;

    if (sscanf((char*)pData, "+QIURC: \"recv\",%d\r", &socketIndex) == 1)
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pSocketCtrl = &pModulePara->stSocketCtrl[socketIndex];
            pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATTCPCmd_Read);
            pPrivate->cycleReadTickStart = Common_GetSystick();
        }
    }

    return 0; 
}

void ATTCP_CloseSocket(void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    ATTCPPrivate_Struct *pPrivate = (ATTCPPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->waitTcpConnectOkFlag = FALSE;

    if (pSocketCtrl->eSocketState != eCddNetMSocketState_Init &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_Abnormal &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect)
    {
        CddDrvEG800AK_ClearSocketCmd(pSocketCtrl->socketIndex);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATTCPCmd_Close);

        ATTCP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Abnormal);
    }
}

void ATTCP_StateHandle(uint8_t socketIndex, void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    CddNetMTcpPara_Struct *pTcpPara = (CddNetMTcpPara_Struct *)pSocketCtrl->specificPara;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE && 
            CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work &&
            CddNetM_CheckFileLinkExsit() == FALSE)
        {
            CddDrvEG800AK_AddCmd(socketIndex, eATTCPCmd_Open);
            memset(pSocketCtrl->user_data, 0, sizeof(pSocketCtrl->user_data));
            ATTCP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Connecting);
        }
    }
    else
    {
        ATTCP_SocketStateMange(socketIndex, pSocketCtrl, pTcpPara);
    }
}

