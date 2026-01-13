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
#include "Asw_PlatM.h"
#include "Version.h"
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
static uint16_t IotGN_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotGN_SendHeartBeat(uint8_t port, uint8_t *pBuf);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGNCtx_Struct *pIotGNCtx;

static const IotGNSendCtrl_Struct c_stIotGNSendctrlTable[IOT_GN_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_GN_CMD_LOGIN_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_LOGIN_RSP,
        .pSendFunc = IotGN_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_GN_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .matchCmd = IOT_GN_CMD_LOGIN_RSP,
        .pSendFunc = IotGN_SendHeartBeat,
        .sendCycle = 10000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotGN_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotGNCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, cmd);
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



static uint16_t IotGN_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    uint16_t dataLen = 0;
    uint32_t randomNum = 0;
    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();

    srand(Common_GetSystick());
    randomNum = rand();

    /* 设备编码 */
    Common_AsciiToBCD(pParam->platPileDn, (char *)&pBuf[dataLen], 14);
    dataLen += 7;
     /* 设备识别码 */
#if (SYSCFG_CFG_GUN_NUM == 1)
    sprintf((char *)&pBuf[dataLen], "%s", SYSCFG_CFG_PRODUCT_CODE);
    dataLen += 16;
#else
    sprintf((char *)&pBuf[dataLen], "%s2", SYSCFG_CFG_PRODUCT_CODE);
    dataLen += 16;
#endif
    /* 随机数 */
    memcpy(&pBuf[dataLen], &randomNum, 4);
    dataLen += 4;
    /* 验证密钥 */
    memset(&pBuf[dataLen], 0x00, 16);
    dataLen += 16;
    /* 设备类型 交流桩*/
    pBuf[dataLen] = 0x01;
    dataLen += 1;
    /* 充电枪数量*/
    pBuf[dataLen] = SYSCFG_CFG_GUN_NUM;
    dataLen += 1;
    /* 程序版本 */
    pBuf[dataLen] = APP_SW_MAJOR_VERSION;
    dataLen += 1;
    pBuf[dataLen] = APP_SW_MINOR_VERSION;
    dataLen += 1;
    pBuf[dataLen] = APP_SW_CUSTORM_VERSION;
    dataLen += 1;
    pBuf[dataLen] = APP_SW_PATCH_VERSION;
    dataLen += 1;
    /* 网络连接类型  sim卡*/
    pBuf[dataLen] = 0x00;
    dataLen += 1;
    /* ICCID */
    CddNetM_GetIccid(&pBuf[dataLen]);
    dataLen += 20;
    /* 运营商 */
    if (eOperator == eCddNetMOperator_CMCC)
        pBuf[dataLen] = 0x00;
    else if (eOperator == eCddNetMOperator_CTCC)
        pBuf[dataLen] = 0x01;
    else if (eOperator == eCddNetMOperator_CUCC)
        pBuf[dataLen] = 0x02;
    else
        pBuf[dataLen] = 0xFF;

    dataLen += 1;
    return dataLen;
}

static uint16_t IotGN_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    uint16_t dataLen = 0;

    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();

    /* 设备编码 */
    Common_AsciiToBCD(pParam->platPileDn, (char *)&pBuf[dataLen], 14);
    dataLen += 7;

    /* 终端号 */
    pBuf[dataLen] = port + 1;
    dataLen += 1;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        /* 故障 */
        pBuf[dataLen] = 0x01;
    }
    else
    {
        pBuf[dataLen] = 0x00;
    }

    dataLen += 1;
    return dataLen;
}

static uint16_t IotGNPackHead(uint8_t cmd, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    IotGNFrameHead_Struct *pFrameHead = (IotGNFrameHead_Struct *)pBuf;
    uint16_t totalLen = dataLen + sizeof(IotGNFrameHead_Struct);
    uint16_t crc16 = 0;

    pFrameHead->head[0] = IOT_GN_HEAD1;
    pFrameHead->head[1] = IOT_GN_HEAD2;
    Common_Uint16ToTwoUint8(pFrameHead->version, IOT_GN_PROTOCOL_VERSION);
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

void IotLX_UpCtrlSendDeal(void)
{
    const IotGNSendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_GN_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotGNCtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotGNCtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotGNCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotGNCtx->sendIndex < ARRAY_SIZE(c_stIotGNSendctrlTable))
            {
                index = pIotGNCtx->sendIndex;
                port = pIotGNCtx->sendPort;

                pCmdSendCtrl = &c_stIotGNSendctrlTable[index];

                if ((Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotGN_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotGNCtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_GN_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotGNCtx->reqSeq++;
                    }
                    else
                    {
                        reqSeq = Common_GetRecvSeq(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotGNFrameHead_Struct);
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        dataLen = IotGNPackHead(pCmdSendCtrl->cmd, reqSeq, txBuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotGNCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                            {
                                pIotGNCtx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTGN_CFG_LogPrint("[枪：%d]发送[cmd: %02X, %s][%d]: ", port, pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());
                                            
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_GN_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_GN_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotGNCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

			pIotGNCtx->sendIndex++;

			if (pIotGNCtx->sendIndex >= ARRAY_SIZE(c_stIotGNSendctrlTable))
			{
				pIotGNCtx->sendIndex = 0;
				pIotGNCtx->sendPort++;

				if (pIotGNCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotGNCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotGNCtx->queueBusyFlag == TRUE)
			{
				break;
			}
        }
    }
}





















