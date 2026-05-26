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
#include "Asw_IotProtoYKC16M.h"
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
#define IOT_YKC16_RecvGunNoTransform(inputPort, outputPort)    do{ \
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

static uint8_t IotYKC16_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvParaSet(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC16_RecvSetUpdate(uint8_t *port, uint8_t *r_data, uint16_t len);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotYKC16Ctx_Struct *pIotYKC16Ctx;


static const IotYKC16RecvCtrl_Struct c_stIotYKC16RecvctrlTable[IOT_YKC16_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_YKC16_CMD_LOGIN_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_YKC16_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvHeartBeatRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,  //jxy 心跳暂时打开
        .cMeaning = "心跳应答",
    },

    [2] = 
    {
        .cmd = IOT_YKC16_CMD_BILLMODE_VERIFY_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvBillModeVerifyRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_BILLMODE_VERIFY_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证应答",
    },

    [3] = 
    {
        .cmd = IOT_YKC16_CMD_BILLMODE_4RATE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvBillMode4RateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [4] = 
    {
        .cmd = IOT_YKC16_CMD_CALL_REALDATA,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvCallRealData,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_CALL_REALDATA_ACK,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据",
    },

    [5] = 
    {
        .cmd = IOT_YKC16_CMD_REMOTE_START_CHARGE,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvRemoteStartCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_REMOTE_START_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电",
    },

    [6] = 
    {
        .cmd = IOT_YKC16_CMD_REMOTE_STOP_CHARGE,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvRemoteStopCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_REMOTE_STOP_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电",
    },

    [7] = 
    {
        .cmd = IOT_YKC16_CMD_ORDER_RECORD_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvOrderRecordRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_ORDER_RECORD_REQ,
        .printFlag = TRUE,
        .cMeaning = "交易记录应答",
    },

    [8] = 
    {
        .cmd = IOT_YKC16_CMD_PILE_START_CHARGE_RSP,
        .cmdType = IOT_YKC16_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC16_RecvPileStartChargeRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC16_CMD_PILE_START_CHARGE_REQ,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动启动充电应答",
    },

    [9] = 
    {
        .cmd = IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvUpdateAccountMoney,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额",
    },

    [10] = 
    {
        .cmd = IOT_YKC16_CMD_Para_REQ,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvParaSet,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_Para_RSP,
        .printFlag = TRUE,
        .cMeaning = "工作参数设置",
    },

    [11] = 
    {
        .cmd = IOT_YKC16_CMD_SYNC_TIME,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvSyncTime,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_SYNC_TIME_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程对时",
    },

    [12] = 
    {
        .cmd = IOT_YKC16_CMD_SET_BILLMODE_4RATE,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvSetBillMode4Rate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_SET_BILLMODE_4RATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置四类电价计费模型",
    },

    [13] = 
    {
        .cmd = IOT_YKC16_CMD_SET_QRCODE,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvSetQRCode,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_SET_QRCODE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置二维码",
    },

    [14] = 
    {
        .cmd = IOT_YKC16_CMD_REBOOT,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvSetReboot,
        .maxTimeout = 0, 
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程重启",
    },

    [15] = 
    {
        .cmd = IOT_YKC16_CMD_UPDATE,
        .cmdType = IOT_YKC16_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC16_RecvSetUpdate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC16_CMD_UPDATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新",
    },
};



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static const IotYKC16RecvCtrl_Struct* IotYKC16_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotYKC16RecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_YKC16_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotYKC16RecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotYKC16RecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotYKC16FrameHead_Struct *IotYKC16_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotYKC16FrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint8_t frameLen = 0;

    while (remainLen > (sizeof(IotYKC16FrameHead_Struct) + 2))
    {
        pHead = (IotYKC16FrameHead_Struct *)pStart;

        if (pHead->head == IOT_YKC16_HEAD)
        { 
            frameLen = pHead->dataLen;

            if (frameLen > (sizeof(IotYKC16FrameHead_Struct) - 2))
            {
                calcCrc16 = Common_CalcCRC16((uint8_t *)pHead->seq, frameLen);
                pRecvCrc = (uint8_t *)pHead->seq + frameLen;
                recvCrc16 = pRecvCrc [1] | (pRecvCrc [0] << 8);

                if (calcCrc16 == recvCrc16)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + frameLen + 1 + 1 + 2;
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

static void IotYKC16_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotYKC16RecvCtrl_Struct *pCmdRecvCtrl = NULL;
    /* 查找有效帧 */
    IotYKC16FrameHead_Struct *pFrameHead = IotYKC16_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        /* 查找对应的命令 */
        pCmdRecvCtrl = IotYKC16_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                /* 序列域到数据域总长度 */
                frameLen = pFrameHead->dataLen;
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotYKC16FrameHead_Struct), frameLen - 4))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTYKC16_CFG_DebugPrint("[枪：%d]接收[cmd: 0x%02X, %s][%d]", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen + 4);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen + 4);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_YKC16_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_YKC16_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotYKC16Ctx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_YKC16_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTYKC16_CFG_DebugPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败!\r\n", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen + 4);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen + 4);
                    }
                }
            }
        }
    }
}

static uint8_t IotYKC16_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    /* 登录成功 */
    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, gunNo, IOT_YKC16_CMD_HEARTBEAT_REQ, TRUE);
            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, gunNo, IOT_YKC16_CMD_BILLMODE_VERIFY_REQ, TRUE);
        }

        pIotYKC16Ctx->loginSucc = TRUE;
    }
    else
    {
        index++;
        IOTYKC16_CFG_InfoPrint("登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotYKC16_OfflineHandle();
    }

    return TRUE;
}

static uint8_t IotYKC16_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
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

static uint8_t IotYKC16_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC16Param.stBillMode;
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
                IOTYKC16_CFG_InfoPrint("计费模型，不需要更新！\r\n");
             */
            verifyRes = FALSE;

        }
    }

    if (verifyRes == FALSE)
    {
        IOTYKC16_CFG_InfoPrint("计费模型变化，需要更新！\r\n");
        Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, 0, IOT_YKC16_CMD_BILLMODE_REQ, TRUE);
    }

    return TRUE;
}

static uint8_t IotYKC16_RecvBillMode4RateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC16Param.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    memcpy(pBillMode->sharp_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->sharp_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->peak_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->peak_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->flat_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->flat_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->valley_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->valley_ser_fee, &pRecvData[index], 4);
    index += 4;

    pBillMode->elecLossRate = pRecvData[index];
    index += 1;
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotYKC16_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);

    return TRUE;
}

static uint8_t IotYKC16_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
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
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = 0x07; 
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
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

static uint8_t IotYKC16_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    /* 订单号 */
    memcpy(pIotYKC16Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[index], 16);
    index += 16;
    /* 桩编号 */
    index += 7;
    /* 提取枪号 */
    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    /* 当前状态可以充电 */
    if (TRUE == IotYKC16_CheckChargeStart(port[0], &failReason))
    {
        /* 逻辑卡号 */
        memcpy(pChargeCtrl->authCardID, &pRecvData[index], 8);
        index += 8;
        /* 物理卡号 */
        index += 8;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        if (accountMoney <= IOTYKC16_CFG_CHARGE_MIN_ACCOUNT_MONEY)
        {
            IOTYKC16_CFG_InfoPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            pIotYKC16Ctx->stProtoData[port[0]].remoteStartResult = 0;
            /* 充电启动失败，余额不足*/
            pIotYKC16Ctx->stProtoData[port[0]].remoteStartFailReason = 0x4E;
        }
        else
        {
            memcpy(pIotYKC16Ctx->stProtoData[port[0]].curUsedOrderTransactionNum,
                pIotYKC16Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, 16);

            pChargeCtrl->accountMoney = accountMoney;

            pIotYKC16Ctx->stProtoData[port[0]].remoteStartResult = 1;
            pIotYKC16Ctx->stProtoData[port[0]].remoteStartFailReason = 0;
            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP, TRUE);
        }
    }
    else
    {
        pIotYKC16Ctx->stProtoData[port[0]].remoteStartResult = 0;
        pIotYKC16Ctx->stProtoData[port[0]].remoteStartFailReason = failReason;
    }

    Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, 0, IOT_YKC16_CMD_REMOTE_START_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotYKC16_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);

    /* 枪未处于充电状态 */
    if (AswMonitor_IsOrderIdle(port[0]) == TRUE)
    {
        pIotYKC16Ctx->stProtoData[port[0]].remoteStopResult = 0;
        pIotYKC16Ctx->stProtoData[port[0]].remoteStopFailReason = 0x02;
    }
    else
    {
        pIotYKC16Ctx->stProtoData[port[0]].remoteStopResult = 0x01;
        pIotYKC16Ctx->stProtoData[port[0]].remoteStopFailReason = 0x00;
        AswErrhandle_SetErrExsitCallback(port[0], eSrc_AppStop);
    }

    Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, 0, IOT_YKC16_CMD_REMOTE_STOP_CHARGE_RSP, TRUE);
    return TRUE;
}

static uint8_t IotYKC16_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    /* 通知flash上报成功 */
    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC16Ctx->time);
    IOTYKC16_CFG_InfoPrint("交易记录上报成功!\r\n");
    return TRUE;
}

static uint8_t IotYKC16_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;
    uint8_t *pOrderNum = NULL;
    uint8_t *pCardNum = NULL;

    /* 订单号 */
    pOrderNum = &pRecvData[index];
    index += 16;
    /* 桩编码 */
    index += 7;
    /* 枪号 */
    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    /* 可以启充 */
    if (TRUE == IotYKC16_CheckChargeStart(port[0], &failReason))
    { 
        /* 逻辑卡号 */
        pCardNum = &pRecvData[index];
        index += 8;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        /* 鉴权成功标志 */
        if (pRecvData[index++] == 0x01)
        {
            /* 订单号 */
            memcpy(pIotYKC16Ctx->stProtoData[port[0]].curUsedOrderTransactionNum, pOrderNum, 16);
            /* 卡号 */
            memcpy(pIotYKC16Ctx->stProtoData[port[0]].authCardID, pCardNum, 8);
            /* 账户余额 */
            pChargeCtrl->accountMoney = accountMoney;

            if (accountMoney <= IOTYKC16_CFG_CHARGE_MIN_ACCOUNT_MONEY)
            {
                IOTYKC16_CFG_InfoPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            }
            else
            {
                AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_CARD, TRUE);
                IOTYKC16_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电成功!\r\n", port[0]);
            }
        }
        else
        {
            IOTYKC16_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电失败，失败原因：%02X!\r\n", port[0], pRecvData[index]);
        }
    }
    else
    {
        IOTYKC16_CFG_InfoPrint("[枪：%d]充电桩申请主动启动充电, 平台应答成功，但设备无法启动充电，失败原因：%d\r\n", port[0], failReason);
    }

    return TRUE;
}

static uint8_t IotYKC16_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t invalidCardID[8] = {0};

    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    /* 物理卡号 */
    memcpy(pIotYKC16Ctx->stProtoData[port[0]].updateAccountMoneyCardID, &pRecvData[index], 8);

    /* 卡号为0 接更新桩当前充电用户余额*/
    if (0 == memcmp(&pRecvData[index], invalidCardID, 8))
    {
        index += 8;
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
                IOTYKC16_CFG_InfoPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
                    pChargeCtrl->accountMoney % 100);
        pIotYKC16Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
    }
    /* 卡号非0 要校验本次充电是否为此卡充电*/
    else
    {
        if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
        {
            if (0 == memcmp(&pRecvData[index], pIotYKC16Ctx->stProtoData[port[0]].authCardID, 8))
            {
                index += 8;
                pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
                IOTYKC16_CFG_InfoPrint("[枪：%d]更新账户余额成功，余额：%d.%02d!\r\n", port[0], pChargeCtrl->accountMoney / 100, 
                    pChargeCtrl->accountMoney % 100);
                pIotYKC16Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
            }
            else
            {
                IOTYKC16_CFG_InfoPrint("[枪：%d]更新账户余额失败，卡号不一致!\r\n", port[0]);
                pIotYKC16Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
            }
        }
        else
        {
            IOTYKC16_CFG_InfoPrint("[枪：%d]更新账户余额失败，非刷卡启动充电!\r\n", port[0]);
            pIotYKC16Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
        }
    }

    return TRUE;
}

static uint8_t IotYKC16_RecvParaSet(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC16Param.platInfo;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    /* 是否允许工作 */
    index++;
    /* 充电桩最大允许输出功率 */
    memcpy(&pPlatInfo->MaxPowerRate, &pRecvData[index], 1);
    
    /* 功率百分比调节 */
    IotYKC16_SetPowerControl(0, pPlatInfo->MaxPowerRate);

    return TRUE;
}

static uint8_t IotYKC16_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    CommonDateTime_Struct dataTime;

    dataTime.millisecond = Common_TwoUint8ToUint16(&pRecvData[index]);
    dataTime.second = dataTime.millisecond / 1000;
    index += 2;

    dataTime.minute = pRecvData[index++];
    dataTime.hour = pRecvData[index++];
    dataTime.day = pRecvData[index++] & 0x1F;
    dataTime.month = pRecvData[index++];
    dataTime.year = pRecvData[index++] + 2000;
    SSTM_SynTimeByDateTime(&dataTime);

    return TRUE;
}

static uint8_t IotYKC16_RecvSetBillMode4Rate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC16Param.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;

    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    memcpy(pBillMode->sharp_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->sharp_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->peak_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->peak_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->flat_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->flat_ser_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->valley_ele_fee, &pRecvData[index], 4);
    index += 4;
    memcpy(pBillMode->valley_ser_fee, &pRecvData[index], 4);
    index += 4;


    pBillMode->elecLossRate = pRecvData[index];
    index += 1;
    memcpy(pBillMode->period_rate, &pRecvData[index], 48);
    index += 48;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    return TRUE;
}

static uint8_t IotYKC16_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;
    MSNvmDrcode_Struct qrParam = { 0 };
    
    IOT_YKC16_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index], 100);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTYKC16_CFG_InfoPrint("[枪：%d]设置的二维码内容：%.100s\r\n", port[0], &pRecvData[index]);
    }

    return TRUE;
}

static uint8_t IotYKC16_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len)
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

static uint8_t IotYKC16_RecvSetUpdate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    char path[33] = {0};
    uint32_t timeout = 0;

    CddNetMSocketPara_Union stSocketPara = {0};

    /* 0x02 交流 */
    if (pRecvData[index] != 0x02)
    {
        pIotYKC16Ctx->stProtoData[0].setUpdateResult = 0x02;
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

        pIotYKC16Ctx->stProtoData[0].setUpdateResult = 0x00;
    }

    return TRUE;
}

void IotYKC16_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotYKC16Ctx->frameQueueChannelID, IotYKC16_DecodeData);
}

void IotYKC16_TimeoutDetect(void)
{
    const IotYKC16RecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_YKC16_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotYKC16RecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_YKC16_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                continue;
            }

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotYKC16Ctx->pFuncRecvCtrl, port, 
                pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTYKC16_CFG_InfoPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                /* 是否达到最大重试次数 */
                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if (pCmdRecvCtrl->cmd == IOT_YKC16_CMD_HEARTBEAT_RSP || pCmdRecvCtrl->cmd == IOT_YKC16_CMD_LOGIN_RSP)
                    {
                        IotYKC16_OfflineHandle();
                    }
                    else
                    {
                        Common_ClearRptCount(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        
                        if (pCmdRecvCtrl->cmd == IOT_YKC16_CMD_ORDER_RECORD_RSP)
                        {
                            Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ, FALSE);

                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC16Ctx->time);
                            IOTYKC16_CFG_InfoPrint("交易记录上报失败, 强行置为成功!\r\n");
                        }
                        else
                        {
                            Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                    if (pCmdRecvCtrl->cmd == IOT_YKC16_CMD_ORDER_RECORD_RSP)
                    {
                        Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ, TRUE);
                        Common_SetSendImmdFlag(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ, TRUE);
                        Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ, FALSE);
                    }
                    else
                    {
                        Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendFlag(pIotYKC16Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
            }
        }
    }
}