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
#include "Asw_IotProtoGNM.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"

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
*    Static Local Functions Declaration
*******************************************************************************/

static uint8_t IotGN_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGNCtx_Struct *pIotGNCtx;


static const IotGNRecvCtrl_Struct c_stIotGNRecvctrlTable[IOT_GN_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_GN_CMD_LOGIN_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_GN_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvHeartBeatRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,
        .cMeaning = "心跳应答",
    },
};



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static const IotGNRecvCtrl_Struct* IotGN_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotGNRecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_GN_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotGNRecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotGNRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotGNFrameHead_Struct *IotGN_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotGNFrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotGNFrameHead_Struct) + 2))
    {
        pHead = (IotGNFrameHead_Struct *)pStart;

        if ((pHead->head[0] == IOT_GN_HEAD1) && (pHead->head[1] == IOT_GN_HEAD2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (Common_TwoUint8ToUint16(pHead->version) == IOT_GN_PROTOCOL_VERSION &&
                frameLen > (sizeof(IotGNFrameHead_Struct) + 2))
            {
                calcCrc16 = Common_CalcCRC16((uint8_t *)pHead, frameLen - 2);
                pRecvCrc = (uint8_t *)pHead + frameLen - 2;
                recvCrc16 = pRecvCrc [1] | (pRecvCrc [0] << 8);

                if (calcCrc16 == recvCrc16)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + frameLen;
                    break;
                }
            }
        }

        pStart++;
        remainLen--;
        dealLen[0]++;
    }

    return pHead;
}

static void IotGN_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotGNRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotGNFrameHead_Struct *pFrameHead = IotGN_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotGN_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = Common_TwoUint8ToUint16(pFrameHead->dataLen);
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotGNFrameHead_Struct), frameLen - sizeof(IotGNFrameHead_Struct) - 2))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTGN_CFG_LogPrint("[枪：%d]接收[cmd: %02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_GN_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_GN_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_GN_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTGN_CFG_LogPrint("[枪：x]接收[cmd: %02X, %s][%d] 处理失败: ", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}

static uint8_t IotGN_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    index += 7;

    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, gunNo, IOT_GN_CMD_HEARTBEAT_REQ, TRUE);
        }
    }
    else
    {
        IOTGN_CFG_LogPrint("登陆失败，失败原因：%d\r\n", pRecvData[index]);
        IotGN_OfflineHandle();
    }

    return TRUE;
}

static uint8_t IotGN_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    index += 7;

    if (pRecvData[index] > 0)
    {
        pRecvData[index]--;
        port[0] = pRecvData[index];
    }

    return TRUE;
}




static void IotGN_CmdTimeoutHandle_3Times(uint8_t port, const IotGNRecvCtrl_Struct *pCmdRecvCtrl)
{

}

static void IotGN_CmdTimeoutHandle(uint8_t port, uint16_t cmd)
{

}


void IotGN_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotGNCtx->frameQueueChannelID, IotGN_DecodeData);
}

void IotGN_TimeoutDetect(void)
{
    const IotGNRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_GN_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotGNRecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_GN_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                 continue;
            }

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotGNCtx->pFuncRecvCtrl, port, 
                pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTGN_CFG_LogPrint("[cmd:%d %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (pCmdRecvCtrl->maxTryCnt == 0xFFFF)
                {
                    if (timeoutCount >= 3)
                    {
                        IotGN_CmdTimeoutHandle_3Times(port, pCmdRecvCtrl);
                    }
                    else
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        IotGN_CmdTimeoutHandle(port, pCmdRecvCtrl->cmd);
                    }
                }
                else
                {
                    if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                    {
                        IotGN_OfflineHandle();
                    }
                    else
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        IotGN_CmdTimeoutHandle(port, pCmdRecvCtrl->cmd);
                    }
                }
            }
        }
    }
}






















