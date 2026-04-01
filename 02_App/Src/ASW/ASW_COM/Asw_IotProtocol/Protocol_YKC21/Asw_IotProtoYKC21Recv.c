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
#include "Asw_PlatM.h"
#include "Asw_ChargeIf.h"

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
uint8_t IotYKC21Decryptbuf[IOT_YKC21_RX_ECRPTBUFFER_MAXSIZE]; /* 加密的数据 */


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
static uint8_t IotYKC21_RecvSetPowerChange(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetBillModeMultiRate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetQRCode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetParam(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotYKC21_RecvSetDefaultMaxPower(uint8_t *port, uint8_t *r_data, uint16_t len);
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = FALSE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetPowerChange,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_POWERCHANG_RSP,
        .printFlag = TRUE,
        .cMeaning = "功率修改",
    },

    [12] = 
    {
        .cmd = IOT_YKC21_CMD_SYNC_TIME,
        .encryptionFlag = FALSE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetDefaultMaxPower,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP,
        .printFlag = TRUE,
        .cMeaning = "最大功率下发",
    },

    [17] = 
    {
        .cmd = IOT_YKC21_CMD_REBOOT,
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .pRecvParse = IotYKC21_RecvSetFTP,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_YKC21_CMD_SET_FTP_RSP,
        .printFlag = TRUE,
        .cMeaning = "平台设远程升级程序",
    },

    
    [19] = 
    {
        .cmd = IOT_YKC21_CMD_SET_KEY,
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
        .encryptionFlag = TRUE,
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
static uint32_t YKC21_Recv_Data_Decrypt(uint8_t *r_data, int len)
{
    /*
    uint8_t test_iv[16];
    memcpy(test_iv, random_key_A, 16);
    AES_init_ctx_iv(&g_ex, random_key_A, test_iv);
    */
    /* 初始向量与密钥一致 */
    AES_init_ctx_iv(&g_ex, random_key_A, random_key_A);
    AES_CBC_decrypt_buffer(&g_ex, r_data, len);

    return len;
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
    uint16_t decrypted_len = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotYKC21_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = pFrameHead->dataLen[0] << 8 | pFrameHead->dataLen[1];
                /* 解密数据 */
                memset(IotYKC21Decryptbuf, 0, IOT_YKC21_RX_ECRPTBUFFER_MAXSIZE);  
                if (pCmdRecvCtrl->encryptionFlag)
                {
					memcpy(IotYKC21Decryptbuf, (uint8_t *)pFrameHead + sizeof(IotYKC21FrameHead_Struct),IOTYKC21_RX_EcrptMessageBodylength(frameLen));

                    IOTYKC21_CFG_LogPrint("[枪：%d]接收密文消息体[cmd: 0x%02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, 
                        pCmdRecvCtrl->cMeaning, IOTYKC21_RX_EcrptMessageBodylength(frameLen));
                    DSLogM_HexOutput((uint8_t *)IotYKC21Decryptbuf, IOTYKC21_RX_EcrptMessageBodylength(frameLen));

                    decrypted_len =  YKC21_Recv_Data_Decrypt(IotYKC21Decryptbuf,IOTYKC21_RX_EcrptMessageBodylength(frameLen));
                }
                else
                {
                    memcpy(IotYKC21Decryptbuf, (uint8_t *)pFrameHead + sizeof(IotYKC21FrameHead_Struct),IOTYKC21_RX_EcrptMessageBodylength(frameLen));
                    decrypted_len = IOTYKC21_RX_EcrptMessageBodylength(frameLen);
                }

                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, IotYKC21Decryptbuf, decrypted_len))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTYKC21_CFG_LogPrint("[枪：%d]接收明文消息体[cmd: 0x%02X, %s][%d]: ", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, decrypted_len);
                        DSLogM_HexOutput((uint8_t *)IotYKC21Decryptbuf, decrypted_len);
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
    MSNvmYKC21PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;
    uint8_t revRsakeylen = 0;

    if (pRecvData[index] == 0x00)
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        /* 更新Rsa参数 */
        revRsakeylen = pRecvData[index + 1];
        pPlatInfo->rsa_Keylength = revRsakeylen > MSNVM_YKC21_RSA_PUBLIC_KEY_LEN ? MSNVM_YKC21_RSA_PUBLIC_KEY_LEN : revRsakeylen;
        memcpy(&pPlatInfo->rsa_Key, &pRecvData[index + 2], pPlatInfo->rsa_Keylength);

        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, gunNo, IOT_YKC21_CMD_HEARTBEAT_REQ, TRUE);
        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, gunNo, IOT_YKC21_CMD_BILLMODE_VERIFY_REQ, TRUE);

        pIotYKC21Ctx->loginSucc = TRUE;

        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    }
    else
    {
        index++;
        IOTYKC21_CFG_LogPrint("登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotYKC21_OfflineHandle();
    }

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

    if (0 == memcmp(&pRecvData[index], invalidCardID, 8))
    {
        index += 8;
        pChargeCtrl->accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        if (pChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeMoney)
        {
            pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;
        }
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
                if (pChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeMoney)
                {
                    pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;
                }
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
    IOT_YKC21_RecvGunNoTransform(pRecvData[16 + 7], port[0]);

    return TRUE;
}

static uint8_t IotYKC21_RecvSetPowerChange(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint32_t limitTimeSec = 0 ;
    uint32_t curTimeStamp = SSTM_GetSecTimestamp();
    uint32_t tempPowerLimit = 0;

    /* 转换枪号 */
    IOT_YKC21_RecvGunNoTransform(r_data[index], port[0]);
    index++;
    /* 允许最大功率，单位：kW */
    tempPowerLimit = Common_TwoUint8ToUint16(&r_data[index]) * 1000;
    index += 2;
    /* 指令响应优先级，数字越大优先级越高, 暂时用不到 */
    index++;
    /* 限制时间 */
    limitTimeSec = Common_TwoUint8ToUint16(&r_data[index]) * 60;
    /* 判断设置条件 */
    if (0x03 == IotYKC21_GetGunState(port[0]) && tempPowerLimit <= SYSCFG_CFG_MAX_OUTPUT_POWER)
    {
        pIotYKC21Ctx->stProtoData[port[0]].platLimitPower = tempPowerLimit ;
        pIotYKC21Ctx->stProtoData[port[0]].powerlimitEndTimeStamp = limitTimeSec + curTimeStamp;
        pIotYKC21Ctx->stProtoData[port[0]].powerLimitFlag = TRUE;
        pIotYKC21Ctx->stProtoData[port[0]].setPowerChangeResult = 0x01;
    }
    else
    {
        pIotYKC21Ctx->stProtoData[port[0]].setPowerChangeResult = 0x00;
    }

    return TRUE;
}

static uint8_t IotYKC21_RecvSyncTime(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t tempBin1 = 0;
    uint8_t tempBin2 = 0;
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

    /* 提取枪号 */
    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;
    /* 二维码限制，暂时用不到 */
    index++;
     /* 二维码长度 */
    QRCodelength = pRecvData[index++];

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index], QRCodelength);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTYKC21_CFG_LogPrint("[枪：%d]设置的二维码内容：%.100s\r\n", port[0], &pRecvData[index]);
    }

    return TRUE;
}
static uint8_t IotYKC21_RecvSetParam(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;

    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    return TRUE;
}
static uint8_t IotYKC21_RecvSetDefaultMaxPower(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    uint8_t index = 7;
    uint16_t tempMaxDefaultPower = 0;
    uint32_t tempDefaultPowerStartTimeStamp = 0;
    uint32_t tempDefaultPowerStopTimeStamp = 0;
    /* 转换枪号 */
    IOT_YKC21_RecvGunNoTransform(r_data[index], port[0]);
    index++;
    /* 默认最大功率 kW*/
    tempMaxDefaultPower = Common_TwoUint8ToUint16(&r_data[index]) * 1000;
    index += 2;
    /* 默认最大功率开始时间 */
    tempDefaultPowerStartTimeStamp = Common_Cp56Time2aToTimestamp(&r_data[index]);
    index += 7;
    /* 默认最大功率结束时间 */
    tempDefaultPowerStopTimeStamp = Common_Cp56Time2aToTimestamp(&r_data[index]);
    index += 7;
    /* 默认最大功率超出范围 或 结束时间早于当前时间 */
    if (tempMaxDefaultPower > SYSCFG_CFG_MAX_OUTPUT_POWER ||
        tempDefaultPowerStopTimeStamp < SSTM_GetSecTimestamp())
    {
        /* 设置失败 */
        pIotYKC21Ctx->stProtoData[port[0]].setDefaultMaxPowerResult = 0x00;
    }
    else
    {
        /* 设置成功 */
        pIotYKC21Ctx->stProtoData[port[0]].setDefaultMaxPowerResult = 0x01;
        /* 保存参数 */
        pPlatInfo->defaultMaxPower[port[0]] = tempMaxDefaultPower;
        pPlatInfo->deaultMaxPowerStartTimeStamp[port[0]] = tempDefaultPowerStartTimeStamp;
        pPlatInfo->deaultMaxPowerEndTimeStamp[port[0]] = tempDefaultPowerStopTimeStamp;
        pPlatInfo->defaultMaxPowerLimitFlag[port[0]] = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    }

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
        reason = 0x02; /* 枪已在充电 */
    }
    /* 存在故障 */
    else if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        reason = 0x03; /* 设备故障 */
    }
    /* 枪未连接 */
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        reason = 0x05;/* 未插枪 */
    }
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = 0x07; /* 自定义 */
    }
    /* 升级中 */
    else if (TRUE == SSUcm_IsUpdating())
    {
        reason = 0x08; /* 自定义 */
    }
    /* 设备禁用 */
    else if (TRUE == AswMonitor_CheckForbidState())
    {
        reason = 0x09; /* 设备离线 */
    }
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

static uint8_t IotYKC21_RecvRemoteStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;
    uint16_t powerLimit = 0;

    /* 订单号 */
    memcpy(pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, &pRecvData[index], 16);
    index += 16;
    /* 桩编号 */
    index += 7;
    /* 提取枪号 */
    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    if (TRUE == IotYKC21_CheckChargeStart(port[0], &failReason))
    {
        /* 逻辑卡号 物理卡号 */
        index += 16;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        if (accountMoney <= IOTYKC21_CFG_CHARGE_MIN_ACCOUNT_MONEY)
        {
            IOTYKC21_CFG_LogPrint("余额不足，拒绝充电！余额：%d.%02d 元!\r\n", accountMoney / 100, accountMoney % 100);
            pIotYKC21Ctx->stProtoData[port[0]].remoteStartResult = 0;
            /* 充电启动失败，余额不足 */
            pIotYKC21Ctx->stProtoData[port[0]].remoteStartFailReason = 0x4E;
        }
        else
        {
            pChargeCtrl->accountMoney = accountMoney;

            memcpy(pIotYKC21Ctx->stProtoData[port[0]].curUsedOrderTransactionNum,
                   pIotYKC21Ctx->stProtoData[port[0]].newRecvOrderTransactionNum, 16);

            /* 本次充电当前允许的最大功率 */
            powerLimit = Common_TwoUint8ToUint16(&pRecvData[index]) * 1000;
            index += 2;

            if (powerLimit == 0)
            {
                if (pPlatInfo->defaultMaxPowerLimitFlag[port[0]] == TRUE)
                {
                    powerLimit = pPlatInfo->defaultMaxPower[port[0]];
                }
                else
                {
                    powerLimit = SYSCFG_CFG_MAX_OUTPUT_POWER;
                }
            }

            IotYKC21_SetPowerControl(port[0], powerLimit);
            /* SOC限制 */
            index += 1;
            /* 充电电量限制 */
            if (0 != Common_FourUint8ToUint32(&pRecvData[index]))
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = Common_FourUint8ToUint32(&pRecvData[index]) / 100;
            }
            else
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;
            }

            pIotYKC21Ctx->stProtoData[port[0]].remoteStartResult = 1;
            pIotYKC21Ctx->stProtoData[port[0]].remoteStartFailReason = 0;
            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP, TRUE);
        }
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
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t failReason = 0;
    uint32_t accountMoney = 0;
    uint32_t powerLimit = 0;
    uint32_t energyLimit = 0;
    uint8_t *pOrderNum = NULL;
    uint8_t *pCardNum = NULL;

    /* 交易流水号 */
    pOrderNum = &pRecvData[index];
    index += 16;
    /* 桩编号, 不需要 */
    index += 7;
    /* 提取枪号 */
    IOT_YKC21_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);

    if (TRUE == IotYKC21_CheckChargeStart(port[0], &failReason))
    {
        /* 逻辑卡号 */
        pCardNum = &pRecvData[index];
        index += 8;
        /* 账户余额 */
        accountMoney = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;
        /* 本次充电当前允许的最大功率 */
        powerLimit = Common_TwoUint8ToUint16(&pRecvData[index]) * 1000;
        index += 2;
        /* SOC限制用不着 */
        index += 1;
        /* 充电电量限制 */
        energyLimit = Common_FourUint8ToUint32(&pRecvData[index]);
        index += 4;

        /* 鉴权成功标志 */
        if (pRecvData[index++] == 0x01)
        {
            /* 订单号 */
            memcpy(pIotYKC21Ctx->stProtoData[port[0]].curUsedOrderTransactionNum, pOrderNum, 16);
            /* 卡号 */
            memcpy(pIotYKC21Ctx->stProtoData[port[0]].authCardID, pCardNum, 8);
            /* 账户余额 */
            pChargeCtrl->accountMoney = accountMoney;

            /* 本次充电当前允许的最大功率 */
            if (powerLimit == 0)
            {
                if (pPlatInfo->defaultMaxPowerLimitFlag[port[0]] == TRUE)
                {
                    powerLimit = pPlatInfo->defaultMaxPower[port[0]];
                }
                else
                {
                    powerLimit = SYSCFG_CFG_MAX_OUTPUT_POWER;
                }
            }

            IotYKC21_SetPowerControl(port[0], powerLimit);

            /* 充电电量限制 */
            if (0 != energyLimit)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = energyLimit / 100;
            }
            else
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                pChargeCtrl->chargeCtrlVal = pChargeCtrl->accountMoney;
            }

            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_CARD, TRUE);
            IOTYKC21_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电成功!\r\n", port[0]);
        }
        else
        {
            IOTYKC21_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电失败，失败原因：0x%02X!\r\n", port[0], pRecvData[index]);
        }
    }
    else
    {
        IOTYKC21_CFG_LogPrint("[枪：%d]充电桩申请主动启动充电, 平台应答成功，但设备无法启动充电，失败原因：%d\r\n", port[0], failReason);
    }

    return TRUE;
}

static uint8_t IotYKC21_RecvSetKey(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21PlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    uint8_t index = 7;
    uint8_t *pRecvData = r_data;
    uint8_t refreshFlag = TRUE;
    uint8_t gunNo = 0;
    uint8_t keyLen = 0;
    uint8_t excuteCtrl = 0;
    uint8_t *pKey = NULL;

    /* 密钥长度 */
    keyLen = pRecvData[index++];
    /* 密钥 */
    pKey = &pRecvData[index];
    index += keyLen;
    /* 执行控制 */
    excuteCtrl = pRecvData[index];

    if (keyLen <= MSNVM_YKC21_RSA_PUBLIC_KEY_LEN)
    {
        /* 空闲执行 */
        if (excuteCtrl == 0x02)
        {
            for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
            {
                if (0x02 != IotYKC21_GetGunState(gunNo)) 
                {
                    pIotYKC21Ctx->rsaPubicKeyWaitIdleRefreshFlag = TRUE;
                    break;
                }
            }
        }

        pPlatInfo->rsa_Keylength = keyLen;
        memset(pPlatInfo->rsa_Key, 0, MSNVM_YKC21_RSA_PUBLIC_KEY_LEN);
        memcpy(pPlatInfo->rsa_Key, pKey, pPlatInfo->rsa_Keylength);
        IotYKC21_PrintfYKC21KeyAndToken();
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

        pIotYKC21Ctx->rsaPublicKeyRefreshFlag = TRUE;
        pIotYKC21Ctx->rsaPubicKeyDelayRefreshTick = Common_GetSystick();
        pIotYKC21Ctx->stProtoData[0].setKeyResult = 0x01;
    }
    else
    {
        pIotYKC21Ctx->stProtoData[0].setKeyResult = 0x00;
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


