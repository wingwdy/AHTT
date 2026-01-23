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
#include "Asw_IotProtoOMSend.h"
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




/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t IotOM_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc);
static uint16_t IotOM_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotOM_SendHeartBeat(uint8_t port, uint8_t *pBuf);


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotOMCtx_Struct *pIotOMCtx;

static const IotOMSendCtrl_Struct c_stIotOMSendctrlTable[IOT_OM_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_OM_CMD_LOGIN_REQ,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_LOGIN_RSP,
        .pSendFunc = IotOM_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_OM_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .matchCmd = IOT_OM_CMD_HEARTBEAT_RSP,
        .pSendFunc = IotOM_SendHeartBeat,
        .sendCycle = 10000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },
};



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotOM_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotOMCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, port, cmd);
	uint8_t retFlag = FALSE;

	if (TRUE == sendImmdFlag) 
	{
		retFlag = TRUE;
	}
	else
	{
		if (Common_JudgeTimeoutMs(startTick, sendCyc))
		{
			retFlag = TRUE;
		}
	}
	
	return retFlag;
}

static uint16_t IotOM_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    return dataLen;
}

static uint16_t IotOM_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    return dataLen;
}


static uint16_t IotOM_PackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotOMFrameHead_Struct *pFrameHead = (IotOMFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotOMFrameHead_Struct);
    uint16_t crc16 = 0;

    pFrameHead->head[0] = IOT_OM_HEAD1;
    pFrameHead->head[1] = IOT_OM_HEAD2;
    Common_Uint16ToTwoUint8(pFrameHead->version, IOT_OM_PROTOCOL_VERSION);
    Common_Uint16ToTwoUint8(pFrameHead->seq, seq);
    pFrameHead->encryptFlag = 0;
    pFrameHead->cmd = cmd;
    Common_Uint16ToTwoUint8(pFrameHead->dataLen, totalLen + 2);
    crc16 = Common_CalcCRC16(pBuf, totalLen);
    pBuf[totalLen] = (crc16 >> 8) & 0xFF;
    totalLen += 1;
    pBuf[totalLen] = (crc16) & 0xFF;
    totalLen += 1;
    return totalLen;
}

void IotOM_UpCtrlSendDeal(void)
{
    const IotOMSendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_OM_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotOMCtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotOMCtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotOMCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotOMCtx->sendIndex < ARRAY_SIZE(c_stIotOMSendctrlTable))
            {
                index = pIotOMCtx->sendIndex;
                port = pIotOMCtx->sendPort;

                pCmdSendCtrl = &c_stIotOMSendctrlTable[index];

                if ((Common_GetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotOM_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotOMCtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_OM_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotOMCtx->reqSeq++;
                    }
                    else
                    {
                        reqSeq = Common_GetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotOMFrameHead_Struct);
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        dataLen = IotOM_PackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotOMCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                            {
                                pIotOMCtx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTOM_CFG_LogPrint("[枪：%d]发送[cmd: %02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());
                                            
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_OM_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_OM_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotOMCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

			pIotOMCtx->sendIndex++;

			if (pIotOMCtx->sendIndex >= ARRAY_SIZE(c_stIotOMSendctrlTable))
			{
				pIotOMCtx->sendIndex = 0;
				pIotOMCtx->sendPort++;

				if (pIotOMCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotOMCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotOMCtx->queueBusyFlag == TRUE)
			{
				break;
			}
        }
    }
}


















