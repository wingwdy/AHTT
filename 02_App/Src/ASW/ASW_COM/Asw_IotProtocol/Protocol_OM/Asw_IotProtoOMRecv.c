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
#include "FrameQueue.h"
#include "Asw_Monitor.h"

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
static uint8_t IotOM_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotOMCtx_Struct *pIotOMCtx;

static const IotOMRecvCtrl_Struct c_stIotOMRecvctrlTable[IOT_OM_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_OM_CMD_LOGIN_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 10,
        .matchCmd = IOT_OM_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_OM_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvHeartBeatRsp,
        .maxTimeout = 30 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_OM_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,
        .cMeaning = "心跳应答",
    }
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotOM_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{




    return TRUE;
}

static uint8_t IotOM_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{




    return TRUE;
}








static const IotOMRecvCtrl_Struct* IotOM_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotOMRecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_OM_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotOMRecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotOMRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotOMFrameHead_Struct *IotOM_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotOMFrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotOMFrameHead_Struct) + 2))
    {
        pHead = (IotOMFrameHead_Struct *)pStart;

        if ((pHead->head[0] == IOT_OM_HEAD1) && (pHead->head[1] == IOT_OM_HEAD2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (Common_TwoUint8ToUint16(pHead->version) == IOT_OM_PROTOCOL_VERSION &&
                frameLen > (sizeof(IotOMFrameHead_Struct) + 2))
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

static void IotOM_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotOMRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotOMFrameHead_Struct *pFrameHead = IotOM_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotOM_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = Common_TwoUint8ToUint16(pFrameHead->dataLen);
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotOMFrameHead_Struct), frameLen - sizeof(IotOMFrameHead_Struct) - 2))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTOM_CFG_LogPrint("[枪：%d]接收[cmd: %02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_OM_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_OM_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_OM_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTOM_CFG_LogPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}





















