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
#include "Asw_IotProtoYKC21M.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "SS_Ucm.h"
#include "Asw_ChargeIf.h"
#include "SS_Tm.h"
#include "Asw_Monitor.h"
#include "Asw_lotProtoYKC21aes.h"
#include "Common.h"
// #include "Asw_IotProtoYKC21Recv.h"
// #include "Asw_IotProtoYKC21Send.h"
#include "Asw_PlatM.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_YKC21_RecvGunNoTransform(inputPort, outputPort)    do{ \
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
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t IotYKC21_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvFaultRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvFaultRestRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvCallRecord(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetPowerCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetParam(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetPowerDefaultMax(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetFTP(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetKey(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len);


 


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotYKC21Ctx_Struct *pIotYKC21Ctx;


static const IotYKC21RecvCtrl_Struct c_stIotYKC21RecvctrlTable[IOT_YKC21_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_YKC21_CMD_LOGIN_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_YKC21_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvHeartBeatRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,
        .cMeaning = "心跳应答",
    },

    [2] = 
    {
        .cmd = IOT_YKC21_CMD_BILLMODE_VERIFY_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvBillModeVerifyRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_BILLMODE_VERIFY_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证应答",
    },

    [3] = 
    {
        .cmd = IOT_YKC21_CMD_BILLMODE_MUTIRATE_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvBillModeMultRateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_BILLMODE_REQ,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求应答",
    },

    [4] = 
    {
        .cmd = IOT_YKC21_CMD_CALL_REALDATA,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvCallRealData,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_CALL_REALDATA_ACK,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据",
    },

    [5] = 
    {
         .cmd = IOT_YKC21_CMD_REMOTE_STOP_CHARGE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvRemoteStopCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_REMOTE_STOP_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电",
    },

    [6] = 
    {
        .cmd = IOT_YKC21_CMD_ORDER_RECORD_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvOrderRecordRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ,
        .printFlag = TRUE,
        .cMeaning = "交易记录应答",
    },

    [7] = 
    {
        .cmd = IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvUpdateAccountMoney,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额",
    },

     
    [8] = 
    {
        .cmd = IOT_YKC21_CMD_FAULT_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvFaultRsp,
        .maxTimeout = 30*1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_FAULT_REQ,
        .printFlag = TRUE,
        .cMeaning = "设备故障上送回复确认",
    },

    [9] = 
    {
        .cmd = IOT_YKC21_CMD_FAULTREST_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvFaultRestRsp,
        .maxTimeout = 30*1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_FAULTREST_REQ,
        .printFlag = TRUE,
        .cMeaning = "设备故障上送回复确认",
    },

    [10] = 
    {
        .cmd = IOT_YKC21_CMD_Call_RECORD,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvCallRecord,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_RECORD_RSP,
        .printFlag = TRUE,
        .cMeaning = "交易记录召唤",
    },

    [11] = 
    {
        .cmd = IOT_YKC21_CMD_SET_POWERCHANG,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetPowerCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_POWERCHANG_RSP,
        .printFlag = TRUE,
        .cMeaning = "功率修改",
    },

    [12] = 
    {
        .cmd = IOT_YKC21_CMD_SYNC_TIME,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSyncTime,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SYNC_TIME_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程对时",
    },

    [13] = 
    {
        .cmd = IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetBillModeMultiRate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置多类电价计费模型",
    },

    [14] = 
    {
        .cmd = IOT_YKC21_CMD_SET_QRCODE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetQRCode,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SET_QRCODE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置二维码",
    },



    [15] = 
    {
        .cmd = IOT_YKC21_CMD_SET_PARAM,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetParam,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SET_PARAM_RSP,
        .printFlag = TRUE,
        .cMeaning = "参数设置",
    },


    
    [16] = 
    {
        .cmd = IOT_YKC21_CMD_SET_POWERDEFAULT_MAX,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetPowerDefaultMax,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP,
        .printFlag = TRUE,
        .cMeaning = "最大功率下发",
    },

    
    [17] = 
    {
        .cmd = IOT_YKC21_CMD_REBOOT,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetReboot,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程重启",
    },

    [18] = 
    {
        .cmd = IOT_YKC21_CMD_SET_FTP,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetFTP,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "平台设远程升级程序",
    },

    
    [19] = 
    {
        .cmd = IOT_YKC21_CMD_SET_KEY,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetKey,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SET_KEY_RSP,
        .printFlag = TRUE,
        .cMeaning = "密钥更新",
    },

    [20] = 
    {
        .cmd = IOT_YKC21_CMD_PILE_START_CHARGE_RSP,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .pRecvParse = IotYKC21_RecvPileStartChargeRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_YKC21_CMD_PILE_START_CHARGE_REQ,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动启动充电应答",
    },


    [21] = 
    {
        .cmd = IOT_YKC21_CMD_REMOTE_START_CHARGE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvRemoteStartCharge,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_REMOTE_START_CHARGE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电",
    },


};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void YKC21_Recv_Data_Decrypt(uint16_t cmd, uint8_t *r_data, int len)
{
    /*
    uint8_t test_iv[16];
    memcpy(test_iv, random_key_A, 16);
    AES_init_ctx_iv(&g_ex, random_key_A, test_iv);
    */

    if (cmd == IOT_YKC21_CMD_HEARTBEAT_RSP || cmd == IOT_YKC21_CMD_SYNC_TIME)
    {
    }
    else
    {
        /* 初始向量与密钥一致 */
        AES_init_ctx_iv(&g_ex, random_key_A, random_key_A);
        AES_CBC_decrypt_buffer(&g_ex, r_data, len);
    }
}
static const IotYKC21RecvCtrl_Struct* IotYKC21_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotYKC21RecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_YKC21_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotYKC21RecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotYKC21RecvctrlTable[index];
            break;
        }
    }

    return pCtrl;

}

static IotYKC21FrameHead_Struct *IotYKC21_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotYKC21FrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotYKC21FrameHead_Struct) + 2))
    {
        pHead = (IotYKC21FrameHead_Struct *)pStart;

        if ((pHead->head == IOT_YKC21_PLUS_HEAD))
        {
            frameLen = pHead->dataLen[0] << 8 | pHead->dataLen[1]; /* 序列号域+发送时间+加密标志+帧类型标志+消息体 */

            if (frameLen > (sizeof(IotYKC21FrameHead_Struct) - 3))
            {
                calcCrc16 = Common_CalcCRC16((uint8_t *)pHead->seq, frameLen);
                pRecvCrc = (uint8_t *)pHead->seq + frameLen;
                recvCrc16 = pRecvCrc[1] | (pRecvCrc[0] << 8);

                if (calcCrc16 == recvCrc16)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + 1 + 2 + frameLen + 2;
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


static void IotYKC21_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotYKC21RecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotYKC21FrameHead_Struct *pFrameHead = IotYKC21_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotYKC21_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = pFrameHead->dataLen[0] << 8 | pFrameHead->dataLen[1];
                /* 解密数据 */
                YKC21_Recv_Data_Decrypt(pFrameHead->cmd, (uint8_t *)pFrameHead + sizeof(IotYKC21FrameHead_Struct), (frameLen - 2 - 7 - 1 - 1));

                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotYKC21FrameHead_Struct), frameLen))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTYKC21_CFG_LogPrint("[枪：%d]接收解密后数据[cmd: 0x%02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_YKC21_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_YKC21_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotYKC21Ctx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_YKC21_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTYKC21_CFG_LogPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}




static uint8_t IotYKC21_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        /* 更新Rsa参数 */
        pPlatInfo->rsa_Keylength = pRecvData[index + 1];
        memcpy(&pPlatInfo->rsa_Key, &pRecvData[index + 2], pPlatInfo->rsa_Keylength);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, gunNo, IOT_YKC21_CMD_HEARTBEAT_REQ, TRUE);
            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, gunNo, IOT_YKC21_CMD_BILLMODE_VERIFY_REQ, TRUE);
        }

        pIotYKC21Ctx->loginSucc = TRUE;
    }
    else
    {
        index++;
        IOTYKC21_CFG_LogPrint("登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotYKC21_OfflineHandle();
    }

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    return TRUE;

}

static uint8_t IotYKC21_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
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


static uint8_t IotYKC21_RecvBillModeVerifyRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC21Param.stBillMode;
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
                IOTYKC21_CFG_LogPrint("计费模型，不需要更新！\r\n");
             */
            verifyRes = FALSE;
        }
    }

    if (verifyRes == FALSE)
    {
        IOTYKC21_CFG_LogPrint("计费模型变化，需要更新！\r\n");
        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, 0, IOT_YKC21_CMD_BILLMODE_REQ, TRUE);
    }

    return TRUE;

}
static uint8_t IotYKC21_RecvBillModeMultRateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC21Param.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;


    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->billnum = pRecvData[index++];

    for (temp = 0; temp < (pBillMode->billnum); temp++)
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

static uint8_t IotYKC21_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    
    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);

    return TRUE;

}
static uint8_t IotYKC21_RecvRemoteStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);

    if (AswMonitor_IsOrderIdle(port[0]) == TRUE)
    {
        pIotYKC21Ctx->stProtoData[port[0]].remoteStopResult = 0;
        pIotYKC21Ctx->stProtoData[port[0]].remoteStopFailReason = 0x02;
    }
    else
    {
        pIotYKC21Ctx->stProtoData[port[0]].remoteStopResult = 0x01;
        pIotYKC21Ctx->stProtoData[port[0]].remoteStopFailReason = 0x00;
        AswErrhandle_SetErrExsitCallback(port[0], eSrc_AppStop);
    }

    Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, 0, IOT_YKC21_CMD_REMOTE_STOP_CHARGE_RSP, TRUE);
    return TRUE;

}
static uint8_t IotYKC21_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;

    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC21Ctx->time);
    IOTYKC21_CFG_LogPrint("交易记录上报成功!\r\n");
    return TRUE;

}
static uint8_t IotYKC21_RecvUpdateAccountMoney(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t invalidCardID[8] = {0};

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    memcpy(pIotYKC21Ctx->stProtoData[port[0]].updateAccountMoneyCardID, &pRecvData[index], 8);

    if (0 == memcmp(&pRecvData[index], invalidCardID, 8) == 0)
    {
        index += 8;
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        IOTYKC21_CFG_LogPrint("[枪：%d]更新账户余额成功，余额：%d!\r\n", port[0], pChargeCtrl->accountMoney);
        pIotYKC21Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
    }
    else
    {
        if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
        {
            if (0 == memcmp(&pRecvData[index], pIotYKC21Ctx->stProtoData[port[0]].authCardID, 8))
            {
                index += 8;
                pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
                IOTYKC21_CFG_LogPrint("[枪：%d]更新账户余额成功，余额：%d!\r\n", port[0], pChargeCtrl->accountMoney);
                pIotYKC21Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x00;
            }
            else
            {
                IOTYKC21_CFG_LogPrint("[枪：%d]更新账户余额失败，卡号不一致!\r\n", port[0]);
                pIotYKC21Ctx->stProtoData[port[0]].updateAccountMoneyResult = 0x02;
            }
        }
    }

    return TRUE;
}
static uint8_t IotYKC21_RecvFaultRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{ 
     return TRUE;
}
static uint8_t IotYKC21_RecvFaultRestRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
     return TRUE;
}
static uint8_t IotYKC21_RecvCallRecord(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t *pRecvData = r_data;
    MSNvmYKC21OrderInfo_Struct *pOrderData = &pIotYKC21Ctx->stOrderInfo.platOrderInfo.stYKC21OrderInfo;
   
    memset(pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, 0, 16);
    memcpy(pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[0], 16);

    IOT_YKC21_RecvGunNoTransform(pRecvData[16+7], port[0]);

    return TRUE;

}

static uint8_t IotYKC21_RecvSetPowerCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint16_t limittimess = 0 ;
    IOT_YKC21_RecvGunNoTransform(r_data[index], port[0]);

    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.power_running = (r_data[index+2]<<8|r_data[index+1]);
    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.priority = (r_data[index+3]); 
    limittimess = (r_data[index+5]<<8|r_data[index+4])*60 ;
    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.limitendtimess = limittimess + SSTM_GetSecTimestamp();

    return TRUE;
}

static uint8_t IotYKC21_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t tempBin1 = 0;
    uint8_t tempBin2 = 0;
    CommonDateTime_Struct dataTime;
    uint8_t millisecond_H, millisecond_L = 0;
    millisecond_L = pRecvData[index++];
    millisecond_H = pRecvData[index++];
    dataTime.millisecond = millisecond_H << 8 | millisecond_L;
    dataTime.second = dataTime.millisecond / 1000;
    dataTime.minute = pRecvData[index++];
    dataTime.hour = pRecvData[index++];
    dataTime.day = pRecvData[index++] & 0x1F;
    dataTime.month = pRecvData[index++];
    dataTime.year = pRecvData[index++] + 2000;

    SSTM_SynTimeByDateTime(&dataTime);

    return TRUE;
}
static uint8_t IotYKC21_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC21Param.stBillMode;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;
    
    memcpy(pBillMode->billModeID, &pRecvData[index], 2);
    index += 2;

    pBillMode->billnum = pRecvData[index++];

    for (temp = 0; temp < (pBillMode->billnum); temp++)
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
static uint8_t IotYKC21_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t QRCodelength = 0;
    MSNvmDrcode_Struct qrParam = {0};

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    QRCodelength = pRecvData[index + 2];

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index + 2 + 1], QRCodelength);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTYKC21_CFG_LogPrint("[枪：%d]设置的二维码内容：%.100s\r\n", port[0], &pRecvData[index + 2 + 1]);
    }

    return TRUE;
}
static uint8_t IotYKC21_RecvSetParam(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t overstarttime = 0;
    uint8_t overofflinettime = 0;

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);

    /* 鉴权超时时间 */
    overstarttime = pRecvData[index + 2]; // 目前默认15s，不支持更改 

    /* 离线充电时间 */
    overofflinettime = pRecvData[index + 3]; // 离线续充时间 

    return TRUE;
}
static uint8_t IotYKC21_RecvSetPowerDefaultMax(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;

    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    IOT_YKC21_RecvGunNoTransform(r_data[index], port[0]);

    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.defaultPower_max = (r_data[index + 2] << 8 | r_data[index + 1]);
    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerStartTimess = Common_Cp56Time2aToTimestamp(&r_data[index + 3]);
    pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerEndTimess = Common_Cp56Time2aToTimestamp(&r_data[index + 3 + 7]);

    /* 判断数值正确性 */
    if (pIotYKC21Ctx->stProtoData[port[0]].powerConfig.defaultPower_max > 7)
    {
        pIotYKC21Ctx->stProtoData[port[0]].powerConfig.defaultPower_max = 0;
        pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerStartTimess = 0;
        pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerEndTimess = 0;
    }

    /* 存储 */
    pPlatInfo->defaultMAX_power[port[0]] = pIotYKC21Ctx->stProtoData[port[0]].powerConfig.defaultPower_max;
    pPlatInfo->deaultMaxPowerStartTimess[port[0]] = pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerStartTimess;
    pPlatInfo->deaultMaxPowerStartTimess[port[0]] = pIotYKC21Ctx->stProtoData[port[0]].powerConfig.deaultMaxPowerEndTimess;

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, 0, IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP, TRUE);

    return TRUE;
}
static uint8_t IotYKC21_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len)
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

static uint8_t IotYKC21_CheckChargeStart(uint8_t port, uint8_t *pFailReason)
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

static uint8_t IotYKC21_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;

    /* 订单号 */
    memcpy(pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[index], 16);
    index += 16;

    /* 枪号 */
    index += 7;
    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    if (TRUE == IotYKC21_CheckChargeStart(port[0], &failReason))
    {
        memcpy(pIotYKC21Ctx->stProtoData[port[0]].curUsedOrderTransactionNum,
               pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum,
               16);

        /* 逻辑卡号 物理卡号 */
        index += 16;
        /* 账户余额 */
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
        pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;
        index += 4;

        /* 本次充电当前允许的最大功率*/
        uint16_t powerChange = Common_TwoUint8ToUint16(&pRecvData[index]);
        if (powerChange != 0)
            IotYkc21_powercontrol(port[0], powerChange * 1000);
        index += 2;
        /* SOC限制 */
        index += 1;
        /* 充电电量限制 */
        if (0 != Common_FourUint8ToUint32(&pRecvData[index]))
        {
            pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
            pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) / 100;
        }

        pIotYKC21Ctx->stProtoData[port[0]].remoteStartResult = 1;
        pIotYKC21Ctx->stProtoData[port[0]].remoteStartFailReason = 0;
        AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP);
    }
    else
    {
        pIotYKC21Ctx->stProtoData[port[0]].remoteStartResult = 0;
        pIotYKC21Ctx->stProtoData[port[0]].remoteStartFailReason = failReason;
    }

    Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, 0, IOT_YKC21_CMD_REMOTE_START_CHARGE_RSP, TRUE);
    return TRUE;
}


static uint8_t IotYKC21_RecvPileStartChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = (16 + 7);
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);
    index = 0;
    if (TRUE == IotYKC21_CheckChargeStart(port[0], &failReason))
    {
        /* 鉴权成功标志 */
        if (pRecvData[index + 16 + 7 + 1 + 8 + 4 + 2 + 1 + 4] == 0x01)
        {
            /* 订单号 */
            memcpy(pIotYKC21Ctx->stProtoData[port[0]].curUsedOrderTransactionNum, &pRecvData[index], 16);
            index += (16 + 7 + 1);
            /* 卡号 */
            memcpy(pIotYKC21Ctx->stProtoData[port[0]].authCardID, &pRecvData[index], 8);
            index += 8;
            /* 账户余额 */
            pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
            index += 4;

            pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
            pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;

            /*本次充电当前允许的最大功率 */
            index += 2;

            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_CARD);
            IOTYKC21_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电成功!\r\n", port[0]);
        }
        else
        {
            index += (16 + 7 + 1 + 8 + 4 + 2 + 1 + 4 + 1);
            IOTYKC21_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电失败，失败原因：%02X!\r\n", port[0], pRecvData[index]);
        }
    }

    return TRUE;
}

static uint8_t IotYKC21_RecvSetKey(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t refreshglg = FALSE;

     pIotYKC21Ctx->rsaRefreshflg = FALSE;

    for (uint8_t i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
    {
        if (0x03 == IotYKC21_GetGunState(i)) // 充电中
        {
            refreshglg = FALSE;
            break;
        }
    }

    if (TRUE == refreshglg)
    {
        pPlatInfo->rsa_Keylength = pRecvData[index];
        memset(pPlatInfo->rsa_Key, 0, 128);
        memcpy(pPlatInfo->rsa_Key, &pRecvData[index + 1], pPlatInfo->rsa_Keylength);

        IotYKC21_PrintfYKC21KeyAndToken();

        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

        pIotYKC21Ctx->rsaRefreshflg = TRUE;

    }

    return TRUE;
}

static uint8_t IotYKC21_RecvSetFTP(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint32_t timeout = 0;

    CddNetMSocketPara_Union stSocketPara = {0};

    if (pRecvData[index] != 0x02)
    {
        pIotYKC21Ctx->stProtoData[0].setUpdateResult = 0x02;
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
        memcpy(stSocketPara.stFtpPara.path, &pRecvData[index], 32);
        index += 32;
        memcpy(stSocketPara.stFtpPara.fileName, &pRecvData[index], 32);
        index += 32;

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

        pIotYKC21Ctx->stProtoData[0].setUpdateResult = 0x00;
    }

    return TRUE;
}


void IotYKC21_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotYKC21Ctx->frameQueueChannelID, IotYKC21_DecodeData);
}

void IotYKC21_TimeoutDetect(void)
{
    const IotYKC21RecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_YKC21_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotYKC21RecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_YKC21_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                continue;
            }

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotYKC21Ctx->pFuncRecvCtrl, port,
                                                         pCmdRecvCtrl->cmd),
                                      pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTYKC21_CFG_LogPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if (pCmdRecvCtrl->cmd == IOT_YKC21_CMD_HEARTBEAT_RSP || pCmdRecvCtrl->cmd == IOT_YKC21_CMD_LOGIN_RSP)
                    {
                        IotYKC21_OfflineHandle();
                    }
                    else
                    {
                        Common_ClearRptCount(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                        if (pCmdRecvCtrl->cmd == IOT_YKC21_CMD_ORDER_RECORD_RSP)
                        {

                            Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ, FALSE);

                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC21Ctx->time);
                            IOTYKC21_CFG_LogPrint("交易记录上报失败, 强行置为成功!\r\n");
                        }
                        else
                        {
                            Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                    if (pCmdRecvCtrl->cmd == IOT_YKC21_CMD_ORDER_RECORD_RSP)
                    {

                        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                        Common_SetSendImmdFlag(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                        Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ, FALSE);
                    }
                    else
                    {
                        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendImmdFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
            }
        }
    }
}


