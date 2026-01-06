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


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define ATTCP_CYCLE_READ_PERIOD          3000



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
}ATTcpPrivate_Struct;



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void ATTCP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMTcpPara_Struct *pTcpPara);
static void ATTCP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState);
static void ATTCP_CloseSocket(void *socketPara);
static uint16_t ATTCP_PackQIPClose(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stTCPATCmdDescribtor[] =
{
    [eATTcpCmd_Open] =
    { "AT+QIOPEN=1,[ID],\"TCP\",\"[MIP]\",[MPORT],0,0\r\n",     "+QIOPEN",              3,          5000,      3000,  "建立连接",
        NULL,                                   NULL,                         NULL},

    [eATTcpCmd_Read] =
    { "AT+QIRD=[ID],1460\r\n",                                  "+QIRD:",               3,          3000,      500,  "数据读取",
        NULL,                                   NULL,                         NULL},

    [eATTcpCmd_Write] =
    { "AT+QISEND=[ID],[LEN]\r\n",                               "> ",                   3,          3000,      500,  "数据发送",
        NULL,                                   NULL,                         NULL},

    [eATTcpCmd_Close] =
    { "AT+QICLOSE=[ID]\r\n",                                    "+QICLOSE",             3,          5000,      3000,  "关闭连接",
        ATTCP_PackQIPClose,                     NULL,                         NULL},
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static uint16_t ATTCP_PackQIPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

	nATLen = Common_ReplaceNum(pData, nATLen, "[ID]", socketIndex, socketIndex);
	return nATLen;
}

static uint8_t ATTCP_RecvQIPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATTCP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    return TRUE;
}

static void ATTCP_FailHandleQIPClose(uint8_t socketIndex, void * socketPara, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATTCP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
}

static void ATTCP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (eSocketState != pSocketCtrl->eSocketState)
        {
            pSocketCtrl->eSocketState = eSocketState;
        }
    }
}

static void ATTCP_CloseSocket(void *socketPara)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    if (pSocketCtrl->eSocketState != eCddNetMSocketState_Init &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_Abnormal &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect)
    {
        CddDrvEG800AK_ClearSocketCmd(pSocketCtrl->socketIndex);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATTcpCmd_Close);
    }
}

static void ATTCP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMTcpPara_Struct *pTcpPara)
{
    ATTcpPrivate_Struct *pPrivate = (ATTcpPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
    {
        if (Common_JudgeTimeoutMs(pPrivate->cycleReadTickStart, ATTCP_CYCLE_READ_PERIOD))
        {
            pPrivate->cycleReadTickStart = Common_GetSystick();
            CddDrvEG800AK_AddCmd(socketIndex, eATTcpCmd_Read);
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
                pSocketCtrl->socketDisconnectCallback(pSocketCtrl);
            }

            pPrivate->reconnectInterval = CDDDRV_EG800AK_CFG_RECONECT_TIMEOUT(pSocketCtrl->reconectTimes);
            pSocketCtrl->disconectTickStart = Common_GetSystick();
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

void ATTCP_StateHandle(uint8_t socketIndex, void *socketPara)
{ 
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMTcpPara_Struct *pTcpPara = (CddNetMTcpPara_Struct *)pSocketCtrl->specificPara;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE && CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work)
        {
            CddDrvEG800AK_AddCmd(socketIndex, eATTcpCmd_Open);
            memset(pSocketCtrl->user_data, 0, sizeof(pSocketCtrl->user_data));
            pSocketCtrl->eSocketState = eCddNetMSocketState_Connecting;
        }
    }
    else
    {
        ATTCP_SocketStateMange(socketIndex, pSocketCtrl, pTcpPara);
    }
}




