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
#include "SS_Ucm.h"
#include "Asw_ChargeIf.h"
#include "SS_Tm.h"
#include "Asw_Monitor.h"
#include "Asw_PlatM.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_GN_RecvGunNoTransform(inputPort, outputPort)    do{ \
                                                                if (inputPort > SYSCFG_CFG_GUN_NUM)\
                                                                {\
                                                                    outputPort = 0;\
                                                                }\
                                                                else\
                                                                {\
                                                                    outputPort = (inputPort - 1);\
                                                                }\
                                                            }while(0)

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
static uint8_t IotGN_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetUpdate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvOfflineCard(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvOfflineCardClear(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvOfflineCardSearch(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvSetDevWorkParam(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotGN_RecvQueryDevWorkParam(uint8_t *port, uint8_t *r_data, uint16_t len);
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

    [2] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_VERIFY_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillModeVerifyRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_VERIFY_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证应答",
    },

    [3] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_4RATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillMode4RateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [4] = 
    {
        .cmd = IOT_GN_CMD_BILLMODE_MUTIRATE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvBillModeMultRateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [5] = 
    {
        .cmd = IOT_GN_CMD_CALL_REALDATA,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvCallRealData,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_CALL_REALDATA_ACK,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据",
    },

    [6] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_START_CHARGE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvRemoteStartCharge,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_REMOTE_START_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电",
    },

    [7] = 
    {
        .cmd = IOT_GN_CMD_REMOTE_STOP_CHARGE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvRemoteStopCharge,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电",
    },

    [8] = 
    {
        .cmd = IOT_GN_CMD_ORDER_RECORD_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvOrderRecordRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_MULTI_ORDER_RECORD_REQ,
        .printFlag = TRUE,
        .cMeaning = "交易记录应答",
    },

    [9] = 
    {
        .cmd = IOT_GN_CMD_PILE_START_CHARGE_RSP,
        .cmdType = IOT_GN_CMDTYPE_RESPONSE,
        .pRecvParse = IotGN_RecvPileStartChargeRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_GN_CMD_PILE_START_CHARGE_REQ,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动启动充电应答",
    },

    [10] = 
    {
        .cmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvUpdateAccountMoney,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额",
    },

    [11] = 
    {
        .cmd = IOT_GN_CMD_SYNC_TIME,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSyncTime,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_SYNC_TIME_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程对时",
    },

    [12] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_4RATE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetBillMode4Rate,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_4RATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置四类电价计费模型",
    },

    [13] = 
    {
        .cmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetBillModeMultiRate,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_SET_BILLMODE_MULTIRATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置多类电价计费模型",
    },

    [14] = 
    {
        .cmd = IOT_GN_CMD_SET_QRCODE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetQRCode,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_SET_QRCODE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置二维码",
    },

    [15] = 
    {
        .cmd = IOT_GN_CMD_REBOOT,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetReboot,
        .maxTimeout = 0, 
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程重启",
    },

    [16] = 
    {
        .cmd = IOT_GN_CMD_UPDATE,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetUpdate,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_UPDATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新",
    },

    [17] = 
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvOfflineCard,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD_RSP,
        .printFlag = TRUE,
        .cMeaning = "离线卡下发",
    },

    [18] = 
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD_CLEAR,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvOfflineCardClear,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD_CLEAR_RSP,
        .printFlag = TRUE,
        .cMeaning = "离线卡清除",
    },

    [19] = 
    {
        .cmd = IOT_GN_CMD_OFFLINE_CARD_SEARCH,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvOfflineCardSearch,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_OFFLINE_CARD_SEARCH_RSP,
        .printFlag = TRUE,
        .cMeaning = "离线卡查询",
    },

    [20] = 
    {
        .cmd = IOT_GN_CMD_SET_DEV_WORK_PARAM,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvSetDevWorkParam,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_SET_DEV_WORK_PARAM_RSP,
        .printFlag = TRUE,
        .cMeaning = "充电设备工作参数设置",
    },

    [21] = 
    {
        .cmd = IOT_GN_CMD_QUERY_DEV_WORK_PARAM,
        .cmdType = IOT_GN_CMDTYPE_REQUSET,
        .pRecvParse = IotGN_RecvQueryDevWorkParam,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_GN_CMD_QUERY_DEV_WORK_PARAM_RSP,
        .printFlag = TRUE,
        .cMeaning = "充电设备工作参数查询",
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
    uint8_t head1 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD1 : IOT_GN_HEAD1; 
    uint8_t head2 = (AswPlatM_GetPlatType() == eAswPlatType_GNP) ? IOT_GN_PLUS_HEAD2 : IOT_GN_HEAD2; 
    
    while (remainLen > (sizeof(IotGNFrameHead_Struct) + 2))
    {
        pHead = (IotGNFrameHead_Struct *)pStart;

        if ((pHead->head[0] == head1) && (pHead->head[1] == head2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (frameLen > (sizeof(IotGNFrameHead_Struct) + 2))
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
		pHead = NULL;
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
                        IOTGN_CFG_DebugPrint("[枪：%d]接收[cmd: 0x%02X, %s][%d]：", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
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
                        IOTGN_CFG_DebugPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败!：", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}

static uint8_t IotGN_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, gunNo, IOT_GN_CMD_HEARTBEAT_REQ, TRUE);
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, gunNo, IOT_GN_CMD_BILLMODE_VERIFY_REQ, TRUE);
        }

        pIotGNCtx->loginSucc = TRUE;
    }
    else
    {
        index++;
        IOTGN_CFG_InfoPrint("登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotGN_OfflineHandle();
    }

    return TRUE;
}

static uint8_t IotGN_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    if (pRecvData[index] > 0)
    {
        pRecvData[index]--;
        port[0] = pRecvData[index];
    }

    return TRUE;
}

static uint8_t IotGN_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t verifyRes = FALSE;

    if (memcmp(pBillMode->billModeID, &pRecvData[index], 2) == 0)
    {
        index += 2;

        if (pRecvData[index] == 0x00)
        {
            /*  和平台沟通，建议每次都请求，这个标识暂时不使用
                verifyRes = TRUE;
                IOTGN_CFG_InfoPrint("计费模型，不需要更新！\r\n");
             */
            verifyRes = FALSE;

        }
    }

    if (verifyRes == FALSE)
    {
        IOTGN_CFG_InfoPrint("计费模型变化，需要更新！\r\n");
        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_BILLMODE_REQ, TRUE);
    }

    return TRUE;
}

static uint8_t IotGN_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_4;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    for (temp = 0; temp < 4; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    pBillMode->elecLossRate = pRecvData[index++];
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_MULT;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->elecLossRate = pRecvData[index++];

    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    for (temp = 0; temp < 9; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, 0, IOT_GN_CMD_BILLMODE_4RATE_RSP, FALSE);
    Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, 0, IOT_GN_CMD_BILLMODE_4RATE_RSP);
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pBillMode, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    return TRUE;
}

static uint8_t IotGN_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
{
    uint8_t ret = FALSE;
    uint8_t reason = 0;

    /* 订单未结束 */
    if (TRUE != AswMonitor_IsOrderIdle(port))
    {
        reason = 0x02;
    }
    /* 存在故障 */
    else if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        reason = 0x03;
    }
    /* 枪未连接 */
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        reason = 0x05;
    }
    /* 升级中 */
    else if (TRUE == SSUcm_IsUpdating())
    {
        reason = 0x08;
    }
    /* 设备禁用 */
    else if (TRUE == AswMonitor_CheckForbidState())
    {
        reason = 0x09;
    }
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = 0x07; 
    }
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

static uint8_t IotDXL_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
{
    uint8_t ret = FALSE;
    uint8_t reason = 0;
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);

    /* 订单未结束 */
    if (TRUE != AswMonitor_IsOrderIdle(port))
    {
        reason = eIotDXLStopReason_DeviceOccupied;
    }
    /* 故障 */
    else if(AswErrHandle_IsExsistError(port) == TRUE)
    {
        reason = IotDXL_ConverStopReason(AswErrHandle_GetExsistError(port));
    }
    /* 枪未连接 */
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        reason = eIotDXLStopReason_GunDisconnected;
    }
    /* 升级中 */
    else if (TRUE == SSUcm_IsUpdating())
    {
        reason = eIotDXLStopReason_DeviceOta;
    }
    /* 设备禁用 */
    else if (TRUE == AswMonitor_CheckForbidState())
    {
        reason = eIotDXLStopReason_DeviceDisabled;
    }
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = eIotDXLStopReason_BillingModelErr; 
    }
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

uint8_t Iot_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
{
    uint8_t platType = AswPlatM_GetPlatType();
    
    if (platType == eAswPlatType_DXL)
    {
        return IotDXL_CheckChargeStart(port, pFailReason);
    }
    else
    {
        return IotGN_CheckChargeStart(port, pFailReason);
    }
}

static uint8_t IotGN_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;
    uint8_t ctrlType = 0;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    /* 订单号 */
    memcpy(pIotGNCtx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[index], 16);
    index += 16;

    if (TRUE == Iot_CheckChargeStart(port[0], &failReason))
    {
        /* 卡号 */
        memcpy(pChargeCtrl->authCardID, &pRecvData[index], 8);
        index += 8;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        if (accountMoney <= IOTGN_CFG_CHARGE_MIN_ACCOUNT_MONEY)
        {
            IOTGN_CFG_InfoPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            pIotGNCtx->stProtoData[port[0]].remoteStartResult = 0;
            /* 充电启动失败，余额不足*/
            pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = 0x4E;
        }
        else
        {
            memcpy(pIotGNCtx->stProtoData[port[0]].curUsedOrderTransactionNum,
                pIotGNCtx->stProtoData[port[0]].newRecvOrderTransactionNum, 16);

            pChargeCtrl->accountMoney = accountMoney;

            /* 充电控制方式转换 */
            {
                ctrlType = pRecvData[index++];
                if (AswPlatM_GetPlatType() == eAswPlatType_DXL)
                {
                    if (ctrlType == 1)  /* 按时间 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 60;
                    }
                    else if (ctrlType == 2)  /* 按金额 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]);
                    }
                    else if (ctrlType == 3)  /* 按电量 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]);
                    }
                    else /* 自动充满 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                    }
                }

                else
                {
                    if (ctrlType == 1)  /* 按时间 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 60;
                    }
                    else if (ctrlType == 2)  /* 按金额 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                    }
                    else if (ctrlType == 3)  /* 按电量 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                        pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                    }
                    else /* 自动充满 */
                    {
                        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                    }
                }
            }

            index += 4;

            pIotGNCtx->stProtoData[port[0]].remoteStartResult = 1;
            pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = 0;
            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP, TRUE);
        }
    }
    else
    {
        pIotGNCtx->stProtoData[port[0]].remoteStartResult = 0;
        pIotGNCtx->stProtoData[port[0]].remoteStartFailReason = failReason;
    }

    Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_REMOTE_START_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotGN_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    if (AswMonitor_IsOrderIdle(port[0]) == TRUE)
    {
        pIotGNCtx->stProtoData[port[0]].remoteStopResult = 0;
        pIotGNCtx->stProtoData[port[0]].remoteStopFailReason = 0x02;
    }
    else
    {
        pIotGNCtx->stProtoData[port[0]].remoteStopResult = 0x01;
        pIotGNCtx->stProtoData[port[0]].remoteStopFailReason = 0x00;
        AswErrhandle_SetErrExsitCallback(port[0], eSrc_AppStop);
    }

    Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotGN_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGNCtx->time);
    IOTGN_CFG_InfoPrint("交易记录上报成功!\r\n");
    return TRUE;
}

static uint8_t IotGN_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;
    uint8_t ctrlType = 0;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    if (TRUE == IotGN_CheckChargeStart(port[0], &failReason))
    { 
        /* 鉴权成功标志 */
        if (pRecvData[index + 28] == 0x01)
        {
            accountMoney = Common_FourUint8ToUint32(&pRecvData[index + 24]);

            if (accountMoney <= IOTGN_CFG_CHARGE_MIN_ACCOUNT_MONEY)
            {
                IOTGN_CFG_InfoPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            }
            else
            {
                /* 订单号 */
                memcpy(pIotGNCtx->stProtoData[port[0]].curUsedOrderTransactionNum, &pRecvData[index], 16);
                index += 16;
                /* 卡号 */
                memcpy(pIotGNCtx->stProtoData[port[0]].authCardID, &pRecvData[index], 8);
                index += 8;
                /* 账户余额 */
                pChargeCtrl->accountMoney = accountMoney;
                index += 4;
                /* 失败原因 */
                index += 1;

                /* 充电控制方式转换 */
                ctrlType = pRecvData[index++];
                if (ctrlType == 1)  /* 按时间 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 60;
                }
                else if (ctrlType == 2)  /* 按金额 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                }
                else if (ctrlType == 3)  /* 按电量 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                    pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) * 100;
                }
                else /* 自动充满 */
                {
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                }

                AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_CARD, TRUE);
                IOTGN_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电成功!\r\n", port[0]);
            }
        }
        else
        {
            index += 29;
            IOTGN_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电失败，失败原因：%02X!\r\n", port[0], pRecvData[index]);
        }
    }
    else
    {
        IOTGN_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电, 平台应答成功，但设备无法启动充电，失败原因：%d\r\n", port[0], failReason);
    }

    return TRUE;
}

static uint8_t IotGN_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    memcpy(pIotGNCtx->stProtoData[port[0]].updateAccountMoneyCardID, &pRecvData[index], 8);

    /* 刷卡启充需要判断卡号 */  
    if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
    {
        if (0 == memcmp(&pRecvData[index], pIotGNCtx->stProtoData[port[0]].authCardID, 8))
        {
            index += 8;
            pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
            IOTGN_CFG_InfoPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
                pChargeCtrl->accountMoney % 100);
            pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
        }
        else
        {
            IOTGN_CFG_InfoPrint("[枪：%d]更新账户余额失败，卡号不一致!\r\n", port[0]);
            pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
        }
    }
    /* 非刷卡启充直接更新余额 */
    else
    {
        index += 8;
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        IOTGN_CFG_InfoPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
            pChargeCtrl->accountMoney % 100);
        pIotGNCtx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
    }
    

    return TRUE;
}

static uint8_t IotGN_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t tempBin1 = 0;
    uint8_t tempBin2 = 0;
    CommonDateTime_Struct dataTime;

    Common_BCDToBIN(&pRecvData[index++], &tempBin1, 1);
    Common_BCDToBIN(&pRecvData[index++], &tempBin2, 1);
    dataTime.year = tempBin1 * 100 + tempBin2;
    Common_BCDToBIN(&pRecvData[index++], &dataTime.month, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.day, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.hour, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.minute, 1);
    Common_BCDToBIN(&pRecvData[index++], &dataTime.second, 1);
    SSTM_SynTimeByDateTime(&dataTime);
    return TRUE;
}

static uint8_t IotGN_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_4;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    for (temp = 0; temp < 4; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;   
    }

    pBillMode->elecLossRate = pRecvData[index++];
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    pBillMode->billType = IOT_GN_BILLMODE_RATE_TYPE_MULT;
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->elecLossRate = pRecvData[index++];

    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    for (temp = 0; temp < 9; temp++)
    {
        memcpy(&pBillMode->elecPriceRate[temp], &pRecvData[index], 4);
        index += 4;
        memcpy(&pBillMode->servePriceRate[temp], &pRecvData[index], 4);
        index += 4;
    }  

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotGN_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;
    MSNvmDrcode_Struct qrParam = { 0 };
    
    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index], 100);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTGN_CFG_InfoPrint("[枪：%d]设置的二维码内容：%.100s\r\n", port[0], &pRecvData[index]);
    }

    return TRUE;
}

static uint8_t IotGN_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    if (pRecvData[index] == 0x01)
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_Immediate); 
    }
    else
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_WaitIdle);
    }

    return TRUE;
}

static uint8_t IotGN_RecvSetUpdate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    char path[33] = {0};
    char dxlPath[129] = {0};
    uint32_t timeout = 0;

    CddNetMSocketPara_Union stSocketPara = {0};

    if (AswPlatM_GetPlatType() == eAswPlatType_DXL)
    {
        stSocketPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;
        stSocketPara.stFtpPara.eMode = eCddNetMFtpMode_Download;
        
        memcpy(stSocketPara.stFtpPara.ip, &pRecvData[index], 64);
        index += 64;
        memcpy(&stSocketPara.stFtpPara.port, &pRecvData[index], 2);
        index += 2;
        memcpy(stSocketPara.stFtpPara.user, &pRecvData[index], CDD_NETM_CFG_FTP_USERNAME_LEN);
        index += 32;
        memcpy(stSocketPara.stFtpPara.passwd, &pRecvData[index], CDD_NETM_CFG_FTP_PASSWD_LEN);
        index += 32;
        memcpy(dxlPath, &pRecvData[index], 128);
        index += 128;
        Common_ExtractPathAndFileName(dxlPath, stSocketPara.stFtpPara.path, sizeof(stSocketPara.stFtpPara.path), 
        stSocketPara.stFtpPara.fileName, sizeof(stSocketPara.stFtpPara.fileName));

        if (pRecvData[index] == 0x01)
            {
                index += 1;
                timeout = pRecvData[index] * 60 * 1000;
                SSUcm_ReqStartOTA(&stSocketPara, eSSUcmChannelType_FTP, eSSUcmExcuteMode_Immediate, timeout);
            }
            else
            {
                index += 1;
                timeout = pRecvData[index] * 60 * 1000;
                SSUcm_ReqStartOTA(&stSocketPara, eSSUcmChannelType_FTP, eSSUcmExcuteMode_WaitIdle, timeout);
            }

            pIotGNCtx->stProtoData[0].setUpdateResult = 0x00;
    }

    else
    {
        if (pRecvData[index] != 0x02)
        {
            pIotGNCtx->stProtoData[0].setUpdateResult = 0x02;
        }
        else
        {
            index += 3;

            stSocketPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;
            stSocketPara.stFtpPara.eMode = eCddNetMFtpMode_Download;

            memcpy(stSocketPara.stFtpPara.ip, &pRecvData[index], 16);
            index += 16;
            memcpy(&stSocketPara.stFtpPara.port, &pRecvData[index], 2);
            index += 2;
            memcpy(stSocketPara.stFtpPara.user, &pRecvData[index], 16);
            index += 16;
            memcpy(stSocketPara.stFtpPara.passwd, &pRecvData[index], 16);
            index += 16;
            memcpy(path, &pRecvData[index], 32);
            index += 32;
            Common_ExtractPathAndFileName(path, stSocketPara.stFtpPara.path, sizeof(stSocketPara.stFtpPara.path), 
            stSocketPara.stFtpPara.fileName, sizeof(stSocketPara.stFtpPara.fileName));

            if (pRecvData[index] == 0x01)
            {
                index += 1;
                timeout = pRecvData[index] * 60 * 1000;
                SSUcm_ReqStartOTA(&stSocketPara, eSSUcmChannelType_FTP, eSSUcmExcuteMode_Immediate, timeout);
            }
            else
            {
                index += 1;
                timeout = pRecvData[index] * 60 * 1000;
                SSUcm_ReqStartOTA(&stSocketPara, eSSUcmChannelType_FTP, eSSUcmExcuteMode_WaitIdle, timeout);
            }

            pIotGNCtx->stProtoData[0].setUpdateResult = 0x00;
        }
    }

    return TRUE;
}

/* 离线卡ID校验，返回 IotGNOfflineCardValidateResult_Enum */
static IotGNOfflineCardValidateResult_Enum IotGN_OfflineCardValidate(uint8_t *pRecvData, uint16_t dataLen, uint8_t *pIndex)
{
    IotGNOfflineCardValidateResult_Enum result = eIotGNOfflineCardValid;
    uint8_t i;
    uint8_t allZero = 1;
    uint8_t allFF = 1;

    /* 边界检查 */
    if ((*pIndex + MSNVM_GN_OFFLINE_CARD_ID_LEN) > dataLen)
    {
        result = eIotGNOfflineCardOutOfBounds;
    }
    else
    {
        /* 全0/全FF检查 */
        for (i = 0; i < MSNVM_GN_OFFLINE_CARD_ID_LEN; i++)
        {
            if (pRecvData[*pIndex + i] != 0x00) { allZero = 0; }
            if (pRecvData[*pIndex + i] != 0xFF) { allFF = 0; }
            if ((allZero == 0) && (allFF == 0)) { break; }
        }

        if (allZero || allFF)
        {
            result = eIotGNOfflineCardInvalid;
        }
    }

    return result;
}

static uint8_t IotGN_RecvOfflineCard(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNOfflineCardStore_Struct *pStore = &pPrivateParam->stGNParam.stOfflineCardStore;
    uint8_t *pRecvData = r_data;
    uint8_t index = 7;
    uint8_t recvCardCount = 0;
    uint8_t cardIndex = 0;
    uint8_t existFlag = FALSE;
    uint16_t existIndex = 0;
    uint8_t saveResult = 1;   /* 0-失败 1-成功 */
    uint8_t failReason = 0;   /* 0x01-卡号格式错误 0x02-存储空间不足 */
    IotGNOfflineCardValidateResult_Enum validateResult = eIotGNOfflineCardValid;

    /* 卡数量 */
    recvCardCount = pRecvData[index];
    index += 1;

    if (recvCardCount > MSNVM_GN_OFFLINE_CARD_MAX_COUNT)
    {
        recvCardCount = MSNVM_GN_OFFLINE_CARD_MAX_COUNT;
    }

    /* 遍历收到的每张卡，存入本地 */
    for (cardIndex = 0; cardIndex < recvCardCount; cardIndex++)
    {
        existFlag = FALSE;

        /* 离线卡ID校验(越界 + 全0/全FF) */
        {
            validateResult = IotGN_OfflineCardValidate(pRecvData, len, &index);
            if (validateResult != eIotGNOfflineCardValid)
            {
                IOTGN_CFG_InfoPrint("离线卡下发数据错误!\r\n");
                saveResult = 0;
                failReason = 0x01;
                break;
            }
        }

        /* 检查是否已存在 */
        for (existIndex = 0; existIndex < pStore->cardCount && existIndex < MSNVM_GN_OFFLINE_CARD_MAX_COUNT; existIndex++)
        {
            if (0 == memcmp(pStore->offlineCards[existIndex].cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN))
            {
                existFlag = TRUE;
                /* 已存在则用最新数据覆盖 */
                memcpy(pStore->offlineCards[existIndex].cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN);
                break;
            }
        }

        /* 不存在则插入新卡 */
        if (existFlag == FALSE)
        {
            if (pStore->cardCount < MSNVM_GN_OFFLINE_CARD_MAX_COUNT)
            {
                memset(&pStore->offlineCards[pStore->cardCount], 0, sizeof(MSNvmGNOfflineCard_Struct));
                /* 卡号存储在尾部 */
                memcpy(pStore->offlineCards[pStore->cardCount].cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN);
                pStore->cardCount++;
            }
            else
            {
                IOTGN_CFG_InfoPrint("离线卡存储已满(%d)，无法插入新卡!\r\n", MSNVM_GN_OFFLINE_CARD_MAX_COUNT);
                saveResult = 0;
                failReason = 0x02;  /* 存储空间不足 */
                break;
            }
        }

        index += MSNVM_GN_OFFLINE_CARD_ID_LEN;
    }

    /* 保存成功 */
    if (saveResult == 1)
    {
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    }
    /* 设置应答结果 */
    pIotGNCtx->stProtoData[0].offlineCardSaveResult = saveResult;
    pIotGNCtx->stProtoData[0].offlineCardFailReason = failReason;

    IOTGN_CFG_InfoPrint("离线卡下发处理完成，结果：%d, 失败原因: 0x%02X\r\n", saveResult, failReason);
    
    return TRUE;
}

static uint8_t IotGN_RecvOfflineCardClear(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNOfflineCardStore_Struct *pStore = &pPrivateParam->stGNParam.stOfflineCardStore;
    IotGNOfflineCardClearResult_Struct *pClearResult = NULL;
    uint8_t *pRecvData = r_data;
    uint8_t index = 7;  /* 跳过设备编码(7字节) */
    uint8_t clearCount = 0;
    uint8_t clearCardIndex = 0;
    uint8_t foundFlag = FALSE;
    uint16_t storeIndex = 0;
    IotGNOfflineCardValidateResult_Enum validateResult = eIotGNOfflineCardValid;

    port[0] = 0;

    /* 清除个数 (1字节, 0全部清除) */
    clearCount = pRecvData[index];
    index += 1;

    memset(pIotGNCtx->stProtoData[port[0]].offlineCardClearResults, 0, sizeof(pIotGNCtx->stProtoData[port[0]].offlineCardClearResults));

    /* 全量清除 */
    if (clearCount == 0)
    {
        pIotGNCtx->stProtoData[port[0]].offlineCardClearCount = pStore->cardCount;
        
        /* 记录每张卡的清除结果 */
        for (storeIndex = 0; storeIndex < pStore->cardCount; storeIndex++)
        {
            pClearResult = &pIotGNCtx->stProtoData[port[0]].offlineCardClearResults[storeIndex];
            memcpy(pClearResult->cardID, pStore->offlineCards[storeIndex].cardID, MSNVM_GN_OFFLINE_CARD_ID_LEN);
            pClearResult->clearResult = 1;
            pClearResult->failReason = 0x00;
        }
        IOTGN_CFG_InfoPrint("离线卡全量清除完成, 清除卡数：%d\r\n", pStore->cardCount);
        
        memset(pStore, 0, sizeof(MSNvmGNOfflineCardStore_Struct));
    }
    /* 部分清除 */
    else
    {
        /* 遍历要清除的每张卡 */
        for (clearCardIndex = 0; clearCardIndex < clearCount; clearCardIndex++)
        {
            foundFlag = FALSE;

            /* 离线卡ID校验(越界 + 全0/全FF) */
            validateResult = IotGN_OfflineCardValidate(pRecvData, len, &index);
            if (validateResult == eIotGNOfflineCardOutOfBounds)
            {
                IOTGN_CFG_InfoPrint("离线卡清除数据解析越界!\r\n");
                break;
            }

            pClearResult = &pIotGNCtx->stProtoData[port[0]].offlineCardClearResults[clearCardIndex];
            /* 记录待清除卡号 */
            memcpy(pClearResult->cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN);

            if (validateResult == eIotGNOfflineCardInvalid)
            {
                pClearResult->clearResult = 0;
                pClearResult->failReason = 0x01;
                index += MSNVM_GN_OFFLINE_CARD_ID_LEN;
                continue;
            }

            /* 在当前离线卡列表中查找匹配的卡 */
            for (storeIndex = 0; storeIndex < pStore->cardCount && storeIndex < MSNVM_GN_OFFLINE_CARD_MAX_COUNT; storeIndex++)
            {
                if (0 == memcmp(pStore->offlineCards[storeIndex].cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN))
                {
                    foundFlag = TRUE;
                    /* 从列表中移除 */
                    pStore->cardCount--;

                    /* 将后面的卡前移，保持列表紧凑 */
                    if (storeIndex < pStore->cardCount)
                    {
                        memmove(&pStore->offlineCards[storeIndex], &pStore->offlineCards[storeIndex + 1],
                                (pStore->cardCount - storeIndex) * sizeof(MSNvmGNOfflineCard_Struct));
                    }
                    /* 清除最后一个位置 */
                    memset(&pStore->offlineCards[pStore->cardCount], 0, sizeof(MSNvmGNOfflineCard_Struct));
                    break;
                }
            }

            /* 记录清除结果 */
            if (foundFlag == TRUE)
            {
                pClearResult->clearResult = 1;
                pClearResult->failReason = 0x00;  /* 清除成功 */
            }
            else
            {
                pClearResult->clearResult = 0;
                pClearResult->failReason = 0x01;  /* 卡号不在本地列表中，视为格式错误 */
            }
            index += MSNVM_GN_OFFLINE_CARD_ID_LEN;
        }

        /* 实际处理的卡数量 */
        pIotGNCtx->stProtoData[port[0]].offlineCardClearCount = clearCardIndex;
    }

    /* 更新离线卡存储区 */
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    IOTGN_CFG_InfoPrint("离线卡清除完成，剩余卡数量：%d\r\n", pStore->cardCount);

    return TRUE;
}

static uint8_t IotGN_RecvOfflineCardSearch(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNOfflineCardStore_Struct *pStore = &pPrivateParam->stGNParam.stOfflineCardStore;
    IotGNOfflineCardSearchResult_Struct *pSearchResult = NULL;
    uint8_t *pRecvData = r_data;
    uint8_t index = 7;  /* 跳过设备编码(7字节) */
    port[0] = 0;
    uint8_t searchIndex = 0;
    uint8_t searchCount = 0;
    uint8_t foundFlag = FALSE;
    uint16_t storeIndex = 0;
    IotGNOfflineCardValidateResult_Enum validateResult = eIotGNOfflineCardValid;

    /* 查询的离线卡个数 */
    searchCount = pRecvData[index];
    index += 1;

    for (searchIndex = 0; searchIndex < searchCount; searchIndex++)
    {
        foundFlag = FALSE;

        /* 离线卡ID校验(越界 + 全0/全FF) */
        validateResult = IotGN_OfflineCardValidate(pRecvData, len, &index);

        if (validateResult == eIotGNOfflineCardOutOfBounds)
        {
            IOTGN_CFG_InfoPrint("离线卡查询数据解析越界!\r\n");
            break;
        }

        pSearchResult = &pIotGNCtx->stProtoData[port[0]].offlineCardSearchResults[searchIndex];
        /* 记录待查询卡号 */
        memcpy(pSearchResult->cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN);

        if (validateResult == eIotGNOfflineCardInvalid)
        {
            pSearchResult->searchResult = 0;
            index += MSNVM_GN_OFFLINE_CARD_ID_LEN;

            continue;
        }

        for (storeIndex = 0; storeIndex < pStore->cardCount && storeIndex < MSNVM_GN_OFFLINE_CARD_MAX_COUNT; storeIndex++)
        {
            if (0 == memcmp(pStore->offlineCards[storeIndex].cardID, &pRecvData[index], MSNVM_GN_OFFLINE_CARD_ID_LEN))
            {
                foundFlag = TRUE;

            }
        }

        if (foundFlag == TRUE)
        {
            pSearchResult->searchResult = 1;    /* 卡号存在 */
        }
        else
        {
            pSearchResult->searchResult = 0;    /* 卡号不存在 */
        }
        index += MSNVM_GN_OFFLINE_CARD_ID_LEN;
    }

    /* 实际处理的卡数量 */
    pIotGNCtx->stProtoData[port[0]].offlineCardSearchCount = searchIndex;
    IOTGN_CFG_InfoPrint("离线卡查询完成, 当前卡数量: %d\r\n", pStore->cardCount);

    return TRUE;
}


static uint8_t IotGN_RecvSetDevWorkParam(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNWorkParam_Struct *pWorkParam = &pPrivateParam->stGNParam.stWorkParam;
    MSNvmPlatParam_Struct *pPlatParam = AswPlatM_GetPlatParamPtr();
    uint8_t *pRecvData = r_data;
    uint8_t index = 7;          /* 跳过设备编码(7字节) */
    uint8_t paramCount = 0;
    uint8_t paramIndex = 0;
    uint8_t paramId = 0;
    uint8_t paramLen = 0;
    uint8_t paramChanged = FALSE;   /* 是否下发过域名/端口参数 */

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    /* 终端号 */
    index += 1;

    /* 参数个数 */
    paramCount = pRecvData[index];
    index += 1;

    /* 遍历所有参数 */
    for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
    {
        /* 参数编号 */
        paramId = pRecvData[index];
        index += 1;
        /* 参数长度 */
        paramLen = pRecvData[index];
        index += 1;

        switch (paramId)
        {
            case 0x01:  /* 授权配置: 1字节 0x00-授权模式, 0x01-即插即充 */
                pWorkParam->authConfig = pRecvData[index];
                break;

            case 0x02:  /* 服务器域名: 128字节ASCII, 不足末尾补0 */
                memcpy(pWorkParam->serverDomain, &pRecvData[index], MSNVM_GN_SERVER_DOMAIN_LEN);
                /* 域名变更 */
                if (strcmp((char *)pRecvData + index, pPlatParam->platMainIp) != 0)
                {
                    paramChanged = TRUE;
                }
                break;

            case 0x03:  /* 服务器端口号: 2字节BIN */
                pWorkParam->serverPort = Common_TwoUint8ToUint16(&pRecvData[index]);
                if (pWorkParam->serverPort != pPlatParam->platMainPort)
                {
                    paramChanged = TRUE;
                }
                break;

            case 0x04:  /* DXL待机状态实时数据上报周期: 1字节BIN, 单位: min */
                pWorkParam->idleReportCycle = pRecvData[index];
                break;

            case 0x05:  /* DXL充电状态实时数据上报周期: 1字节BIN, 单位: s */
                pWorkParam->chargingReportCycle = pRecvData[index];
                break;

            default:
                break;
        }

        index += paramLen;
    }

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    if (paramChanged)
    {
        pIotGNCtx->devWorkParamChangedFlag = TRUE;
        pIotGNCtx->devWorkParamChangedTick = Common_GetSystick();
    }

    pIotGNCtx->stProtoData[0].setDevWorkParamResult = 0x00;

    return TRUE;
}

static uint8_t IotGN_RecvQueryDevWorkParam(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t *pRecvData = r_data;
    uint8_t index = 7;          /* 跳过设备编码(7字节) */
    uint8_t paramCount = 0;
    uint8_t paramIndex = 0;
    uint8_t paramId = 0;

    IOT_GN_RecvGunNoTransform(pRecvData[index], port[0]);

    /* 终端号 */
    index += 1;

    /* 参数个数 */
    paramCount = pRecvData[index];
    index += 1;

    /* 查询参数信息 最大5个 */
    if (paramCount > IOTDXL_CFG_WORK_PARAM_MAX_COUNT)
    {
        paramCount = IOTDXL_CFG_WORK_PARAM_MAX_COUNT;
    }
    pIotGNCtx->stProtoData[0].queryDevWorkParamCount = paramCount;

    for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
    {
        /* 参数编号 */
        paramId = pRecvData[index];
        index += 1;

        pIotGNCtx->stProtoData[0].queryDevWorkParamIds[paramIndex] = paramId;
    }

    return TRUE;
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

                IOTGN_CFG_InfoPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if (pCmdRecvCtrl->cmd == IOT_GN_CMD_HEARTBEAT_RSP || pCmdRecvCtrl->cmd == IOT_GN_CMD_LOGIN_RSP)
                    {
                        IotGN_OfflineHandle();
                    }
                    else
                    {
                        Common_ClearRptCount(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        
                        if (pCmdRecvCtrl->cmd == IOT_GN_CMD_ORDER_RECORD_RSP)
                        {
                            if (pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
                            {
                                Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, FALSE);
                            }
                            else
                            {
                                Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, FALSE);
                            }

                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGNCtx->time);
                            IOTGN_CFG_InfoPrint("交易记录上报失败, 强行置为成功!\r\n");
                        }
                        else
                        {
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                    if (pCmdRecvCtrl->cmd == IOT_GN_CMD_ORDER_RECORD_RSP)
                    {
                        if (pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, FALSE);
                        }
                        else
                        {
                            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, TRUE);
                            Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, FALSE);
                        }
                    }
                    else
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendFlag(pIotGNCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
            }
        }
    }
}






















