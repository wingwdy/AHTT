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
#include "Asw_PlatM.h"
#include "Version.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "SS_Tm.h"
#include "Asw_lotProtoYKC21rsaOwn.h"
#include "Asw_lotProtoYKC21aes.h"
 
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
*    Global variables Declaration
*******************************************************************************/
extern IotYKC21Ctx_Struct *pIotYKC21Ctx;

struct AES_ctx g_ex;
uint8_t random_key_A[16]="1234567890123456"; // 随机密钥A
uint8_t IotYKC21encryptbuf[IOT_YKC21_RX_ECRPTBUFFER_MAXSIZE]; /* 加密的数据 */


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t IotYKC21_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendHeartBeat(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendBillModeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendRealData(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendChargeStopRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendMultyOrderRecordReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendMultyOrderRecordack(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendFaultRestReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendRecordRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendFaultReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendPowerChangeRsp (uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetBillModeMultiRateRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendPowerDefaultMaxRsp (uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetParamRsp (uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetRebootRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetFTPRsp (uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendSetKeyRsp (uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotYKC21_SendChargeStartRsp(uint8_t port, uint8_t *pBuf);
 

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/


static const IotYKC21SendCtrl_Struct c_stIotYKC21SendctrlTable[IOT_YKC21_CMD_SEND_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_YKC21_CMD_LOGIN_REQ,
        .encryptionFlag = FALSE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_LOGIN_RSP,
        .pSendFunc = IotYKC21_SendLoginReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "登陆认证"
    },

    [1] = 
    {
        .cmd = IOT_YKC21_CMD_HEARTBEAT_REQ,
        .encryptionFlag = FALSE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_HEARTBEAT_RSP,
        .pSendFunc = IotYKC21_SendHeartBeat,
        .sendCycle = 10000,
        .printFlag = FALSE,
        .cMeaning = "设备心跳"
    },

    [2] = 
    {
        .cmd = IOT_YKC21_CMD_BILLMODE_VERIFY_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_BILLMODE_VERIFY_RSP,
        .pSendFunc = IotYKC21_SendBillModeVerifyReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型验证请求"
    },

    [3] = 
    {
        .cmd = IOT_YKC21_CMD_BILLMODE_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_BILLMODE_MUTIRATE_RSP,
        .pSendFunc = IotYKC21_SendBillModeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "计费模型请求"
    },

    [4] = 
    {
        .cmd = IOT_YKC21_CMD_REPORT_REALDATA,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_NULL,
        .pSendFunc = IotYKC21_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,  
        .cMeaning = "主动上报实时数据"
    },

    [5] = 
    {
        .cmd = IOT_YKC21_CMD_CALL_REALDATA_ACK,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_CALL_REALDATA,
        .pSendFunc = IotYKC21_SendRealData,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "实时数据召测应答"
    },


    [6] = 
    {
        .cmd = IOT_YKC21_CMD_REMOTE_STOP_CHARGE_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_REMOTE_STOP_CHARGE,
        .pSendFunc = IotYKC21_SendChargeStopRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程停止充电应答"
    },

    [7] = 
    {
        .cmd = IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_ORDER_RECORD_RSP,
        .pSendFunc = IotYKC21_SendMultyOrderRecordReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "多类电价交易记录"
    },

    [8] = 
    {
        .cmd = IOT_YKC21_CMD_MULTI_ORDER_RECORD_ACK,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = NULL,
        .pSendFunc = IotYKC21_SendMultyOrderRecordack,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "应答召唤交易记录数据"
    },

    [9] = 
    {
        .cmd = IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY_RSP,
        .encryptionFlag = FALSE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY,
        .pSendFunc = IotYKC21_SendUpdateAccountMoneyRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程更新账户余额应答"
    },

    [10] = 
    {
        .cmd = IOT_YKC21_CMD_FAULTREST_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_FAULTREST_RSP,
        .pSendFunc = IotYKC21_SendFaultRestReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设备故障复位"
    },
    
    [11] = 
    {
        .cmd = IOT_YKC21_CMD_RECORD_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_Call_RECORD,
        .pSendFunc = IotYKC21_SendRecordRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "召测交易记录确认应答"
    },

    [12] = 
    {
        .cmd = IOT_YKC21_CMD_FAULT_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_FAULT_RSP,
        .pSendFunc = IotYKC21_SendFaultReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设备故障上报"
    },

    [13] = 
    {
        .cmd = IOT_YKC21_CMD_POWERCHANG_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_POWERCHANG,
        .pSendFunc = IotYKC21_SendPowerChangeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "功率修改应答"
    },

    [14] = 
    {
        .cmd = IOT_YKC21_CMD_SYNC_TIME_RSP,
        .encryptionFlag = FALSE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SYNC_TIME,
        .pSendFunc = IotYKC21_SendSyncTimeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程对时应答"
    },

    [15] = 
    {
        .cmd = IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE,
        .pSendFunc = IotYKC21_SendSetBillModeMultiRateRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置多类电价计费模型应答"
    },

    [16] = 
    {
        .cmd = IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_POWERDEFAULT_MAX,
        .pSendFunc = IotYKC21_SendPowerDefaultMaxRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "默认最大功率下发应答"
    },

    
    [17] =
    {
        .cmd = IOT_YKC21_CMD_SET_QRCODE_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_QRCODE,
        .pSendFunc = IotYKC21_SendSetQrcodeRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置二维码应答"
    },

    [18] =
    {
        .cmd = IOT_YKC21_CMD_SET_PARAM_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_PARAM,
        .pSendFunc = IotYKC21_SendSetParamRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "参数设置应答"
    },

    
    [19] =
    {
        .cmd = IOT_YKC21_CMD_REBOOT_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_REBOOT,
        .pSendFunc = IotYKC21_SendSetRebootRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "设置远程重启应答"
    },

    [20] =
    {
        .cmd = IOT_YKC21_CMD_SET_FTP_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_FTP,
        .pSendFunc = IotYKC21_SendSetFTPRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "平台设远程升级程序应答"
    },

    [21] =
    {
        .cmd = IOT_YKC21_CMD_SET_KEY_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_SET_KEY,
        .pSendFunc = IotYKC21_SendSetKeyRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "密钥更新应答"
    },

    [22] = 
    {
        .cmd = IOT_YKC21_CMD_PILE_START_CHARGE_REQ,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_REQUSET,
        .matchCmd = IOT_YKC21_CMD_PILE_START_CHARGE_RSP,
        .pSendFunc = IotYKC21_SendPileStartChargeReq,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "充电桩主动申请启动充电"
    },

    [23] = 
    {
        .cmd = IOT_YKC21_CMD_REMOTE_START_CHARGE_RSP,
        .encryptionFlag = TRUE,
        .cmdType = IOT_YKC21_CMDTYPE_RESPONSE,
        .matchCmd = IOT_YKC21_CMD_REMOTE_START_CHARGE,
        .pSendFunc = IotYKC21_SendChargeStartRsp,
        .sendCycle = 0,
        .printFlag = TRUE,
        .cMeaning = "远程启动充电应答"
    },


};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static uint16_t YKC21_Send_Data_enecrypt(uint8_t *r_data, int len)
{
    uint16_t data_length = len;
    uint16_t padded_len = ((data_length + AES_BLOCKLEN - 1) / AES_BLOCKLEN) * AES_BLOCKLEN;
    uint8_t pad_value = padded_len - data_length;
    memset(&r_data[data_length], pad_value, pad_value);
    data_length = padded_len;

    /*
     uint8_t test_iv[16];
     memcpy(test_iv, random_key_A, 16); // 初始向量与密钥一致
     AES_init_ctx_iv(&g_ex, random_key_A, test_iv);
     */

    AES_init_ctx_iv(&g_ex, random_key_A, random_key_A);
    AES_CBC_encrypt_buffer(&g_ex, r_data, data_length);

    return data_length;
}


static uint8_t IotYKC21_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
    uint32_t startTick = Common_GetSendTick(pIotYKC21Ctx->pFuncSendCtrl, port, cmd);
    uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotYKC21Ctx->pFuncSendCtrl, port, cmd);
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

static uint16_t IotYKC21_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    uint16_t dataLen = 0;

    CddNetMOperator_Enum eOperator = CddNetM_GetOperatorType();

    /* 随机密钥 */
    uint8_t b64_buf[88];
    encrypt_and_decrypt_data(pPlatInfo->rsa_Key, b64_buf, random_key_A);
    memcpy(&pBuf[dataLen], b64_buf, 88);
    IOTYKC21_CFG_LogPrint("随机钥匙A: ");
    DSLogM_HexOutput((uint8_t *)random_key_A, 16);

    dataLen += 88;
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 桩类型 */
    pBuf[dataLen] = 1;
    dataLen += 1;
    /* 充电枪数量 */
    pBuf[dataLen] = SYSCFG_CFG_GUN_NUM;
    dataLen += 1;

    /* 通信协议版本 */

    pBuf[dataLen] = IOT_YKC21_PROTOCOL_VERSION_H;
    pBuf[dataLen + 1] = IOT_YKC21_PROTOCOL_VERSION_M;
    pBuf[dataLen + 2] = IOT_YKC21_PROTOCOL_VERSION_L;
    dataLen += 3;

    /* 程序版本 */
    uint8_t u8VerLne = strlen(APP_SW_VERSION_STRING) > 8 ? 8 : strlen(APP_SW_VERSION_STRING);
    memcpy(&pBuf[dataLen], APP_SW_VERSION_STRING, u8VerLne);
    dataLen += u8VerLne;
    if (u8VerLne == 7)
    {
        /* 不足8位补0  1.3.2.1->1.3.2.01 */
        uint8_t temp = 0;
        temp = pBuf[dataLen - 1];
        pBuf[dataLen - 1] = '0';
        pBuf[dataLen] = temp;
        dataLen++;
    }

    /* 网络链接类型 */
    pBuf[dataLen++] = 0x00;
    /* Sim 卡 */
    uint8_t cSimID[20] = {0};
    CddNetM_GetIccid(&cSimID[0]);
    Common_AsciiToBCD((char *)cSimID, &pBuf[dataLen], 10);
    dataLen += 10;

    /* 运营商 */
    if (eOperator == eCddNetMOperator_CMCC)
        pBuf[dataLen++] = 0x00;
    else if (eOperator == eCddNetMOperator_CTCC)
        pBuf[dataLen++] = 0x02;
    else if (eOperator == eCddNetMOperator_CUCC)
        pBuf[dataLen++] = 0x02;
    else
        pBuf[dataLen++] = 0x04;

    /* token */
    Common_AsciiToBCD((char *)pPlatInfo->token, &pBuf[dataLen], 14);
    dataLen += 7;
    /* 手机号码 */
    memset(&pBuf[dataLen], 0, 11);
    dataLen += 11;

    /* 支持网络制式 */
    pBuf[dataLen++] = 4;

    /* 当前网络制式 */
    pBuf[dataLen++] = 4;

    /* 经度 */
    memset(&pBuf[dataLen], 0, 4);
    dataLen += 4;

    /* 纬度 */
    memset(&pBuf[dataLen], 0, 4);
    dataLen += 4;

    return dataLen;
}


 
static uint16_t IotYKC21_SendHeartBeat(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    
    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
    dataLen += 7;
    /* 终端号 */
    pBuf[dataLen++] = port + 1;
    /* 状态 */
    pBuf[dataLen++] = (AswErrHandle_IsExsistError(port) == TRUE) ? 0x01 : 0x00;

    return dataLen;

}
 static uint16_t IotYKC21_SendBillModeVerifyReq(uint8_t port, uint8_t *pBuf)
 {
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21ParamBillMode_Struct *pBillMode = &pPrivateParam->stYKC21Param.stBillMode;
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
    dataLen += 7;

    memcpy(&pBuf[dataLen], pBillMode->billModeID, sizeof(pBillMode->billModeID));
    dataLen += sizeof(pBillMode->billModeID);

    return dataLen;

 }

 static uint16_t IotYKC21_SendBillModeReq(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     return dataLen;
 }

 
static void IotYKC21_SetRealDataErrBit(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataLen = 0;

    Common_SetBitFlag(pBuf, 15);

    if (TRUE == AswErrHandle_CheckErrExit(port, eErr_EmergencyStop))
    {
        Common_SetBitFlag(pBuf, 1);
    }
}
 static uint16_t IotYKC21_SendRealData(uint8_t port, uint8_t *pBuf)
 {
     AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
     AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
     uint16_t dataLen = 0;
     uint8_t orderIdleFlag = AswMonitor_IsOrderIdle(port);

     /* 交易流水号 */
     if (orderIdleFlag != TRUE)
     {
         memcpy(&pBuf[dataLen], pIotYKC21Ctx->stProtoData[port].curUsedOrderTransactionNum, 16);
         dataLen += 16;
     }
     else
     {
         memset(&pBuf[dataLen], 0x00, 16);
         dataLen += 16;
     }

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     /* 状态 */
     pBuf[dataLen++] = IotYKC21_GetGunState(port);
     /* 枪是否归位 */
     pBuf[dataLen++] = 02;
     /* 是否插枪 */
     pBuf[dataLen++] = (AswChargeIf_CheckGunConnected(port) == TRUE) ? 0x01 : 0x00;
     /* 输出电压 */
     Common_Uint16ToTwoUint8(&pBuf[dataLen], AswChargeIf_GetInputVoltage(port) / 10);
     dataLen += 2;
     /* 输出电流 */
     Common_Uint16ToTwoUint8(&pBuf[dataLen], AswChargeIf_GetOutputCurrent(port) / 100);
     dataLen += 2;
     /* 枪线温度 */
     pBuf[dataLen++] = AswChargeIf_GetGunTemperature(port);
     /* 枪线编码 */
     memset(&pBuf[dataLen], 0x00, 8);
     dataLen += 8;
     /* SOC */
     pBuf[dataLen++] = 0x00;
     /* 电池组最高温度 */
     pBuf[dataLen++] = 0x00;

     /* 累计充电时间 */
     if (orderIdleFlag != TRUE)
     {
         Common_Uint16ToTwoUint8(&pBuf[dataLen], pChargeData->chargeTime / 60);
         dataLen += 2;
     }
     else
     {
         memset(&pBuf[dataLen], 0x00, 2);
         dataLen += 2;
     }

     /* 剩余时间 */
     memset(&pBuf[dataLen], 0x00, 2);
     dataLen += 2;

     if (orderIdleFlag != TRUE)
     {
         /* 充电度数 */
         memcpy(&pBuf[dataLen], &pChargeData->totalEnergy, 4);
         dataLen += 4;
         /* 计损充电度数 */
         memcpy(&pBuf[dataLen], &pChargeData->totalLossEnergy, 4);
         dataLen += 4;
         /* 已充金额 */
         memcpy(&pBuf[dataLen], &pChargeData->totalMoney, 4);
         dataLen += 4;
     }
     else
     {
         /* 充电度数 */
         memset(&pBuf[dataLen], 0x00, 4);
         dataLen += 4;
         /* 计损充电度数 */
         memset(&pBuf[dataLen], 0x00, 4);
         dataLen += 4;
         /* 已充金额 */
         memset(&pBuf[dataLen], 0x00, 4);
         dataLen += 4;
     }

     /* 硬件故障 */
     memset(&pBuf[dataLen], 0x00, 2);
     IotYKC21_SetRealDataErrBit(port, &pBuf[dataLen]);
     dataLen += 2;

     /*桩体温度*/
     pBuf[dataLen++] = AswChargeIf_GetEnvTemperature();

     /*烟感状态*/
     pBuf[dataLen++] = 0;

     /*电表示值*/
     memset(&pBuf[dataLen], 0x00, 4);
     dataLen += 4;

     return dataLen;
 }


static uint16_t IotYKC21_SendChargeStopRsp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    uint16_t dataLen = 0;

    /* 设备编码 */
    memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
    dataLen += 7;

    /* 枪号 */
    pBuf[dataLen++] = port + 1;
    /* 远程启动结果 */
    pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[port].remoteStopResult;
    pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[port].remoteStopFailReason;

    return dataLen;

}

 static uint16_t IotYKC21_SendMultyOrderRecordReq(uint8_t port, uint8_t *pBuf)
 {
    uint16_t dataLen = 0;

    IotYKC21_TransformChargeRecord(&pIotYKC21Ctx->stOrderInfo.platOrderInfo, pBuf, &dataLen);

    return dataLen;

 }

/* 空闲状态下的召唤记录上传 */
 static uint16_t IotYKC21_SendMultyOrderRecordack(uint8_t port, uint8_t *pBuf)
 {
    uint16_t dataLen = 0;
    CommonDateTime_Struct dateTime;

    IotYKC21_TransformChargeRecord(&pIotYKC21Ctx->stOrderInfo.platOrderInfo, pBuf, &dataLen);

    return dataLen;

 }
 static uint16_t IotYKC21_SendUpdateAccountMoneyRsp(uint8_t port, uint8_t *pBuf)
 {
     AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
     uint16_t dataLen = 0;
     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->stProtoData[port].updateAccountMoneyCardID, 8);
     dataLen += 8;
     pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[port].updateAccountMoneyResult;

     return dataLen;
 }
 static uint16_t IotYKC21_SendRecordRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;
     CommonDateTime_Struct dateTime;
     uint16_t temp = 0;
     uint8_t upReportFlag = FALSE;

     /* 交易流水号 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->stProtoData[port].newRecvOrderTransactionNum, 16);
     dataLen += 16;
     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     if (0x03 == IotYKC21_GetGunState(port))
     {
         pBuf[dataLen++] = 1;
         pBuf[dataLen++] = 1;
     }
     else
     {
         if (eGlobalRet_OK == MSNvm_QueryRecordByExternal(eMSNvmBlockID_OrderRecord, pIotYKC21Ctx->stProtoData[port].newRecvOrderTransactionNum,
                                                          sizeof(pIotYKC21Ctx->stProtoData[port].newRecvOrderTransactionNum), IotYKC21_CompareRecordOrderNum,
                                                          (uint8_t *)&pIotYKC21Ctx->stOrderInfo, sizeof(MSNvmOrderInfo_Struct)))
         {
             pBuf[dataLen++] = 0;
             pBuf[dataLen++] = 0;
             upReportFlag = TRUE;
         }
         else
         {
             pBuf[dataLen++] = 1;
             pBuf[dataLen++] = 1;
         }
     }

     if (upReportFlag == TRUE)
     {
         Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_ACK, TRUE);
     }

     return dataLen;
 }

 static uint16_t IotYKC21_SendFaultReq(uint8_t port, uint8_t *pBuf)
 {
     IotYKC21Err_Struct *pIotykc21err = &pIotYKC21Ctx->stProtoData[port].erroInfo;

     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;
     /* 故障类型 */
     pBuf[dataLen++] = pIotykc21err->errorAppearType;

     /* 故障编码 */
     Common_Uint16ToTwoUint8(&pBuf[dataLen], pIotykc21err->errorAppearId); // 故障编码
     dataLen += 2;
     /* 故障发生时间 */
     Common_TimestampToCp56Time2a(pIotykc21err->errorAppearTime, &pBuf[dataLen]);

     dataLen += 7;
     return dataLen;
 }
 static uint16_t IotYKC21_SendFaultRestReq(uint8_t port, uint8_t *pBuf)
 {

     IotYKC21Err_Struct *pIotykc21err = &pIotYKC21Ctx->stProtoData[port].erroInfo;
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;
     /* 故障类型 */
     pBuf[dataLen++] = pIotykc21err->errorDisppearType;

     /* 故障编码 */
     Common_Uint16ToTwoUint8(&pBuf[dataLen], pIotykc21err->errorDisppearId); // 故障编码
     dataLen += 2;
     /* 故障消除时间 */
     Common_TimestampToCp56Time2a(pIotykc21err->errorDisppearTime, &pBuf[dataLen]);

     dataLen += 7;
     return dataLen;
 }

 static uint16_t IotYKC21_SendPowerChangeRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     if (0x03 == IotYKC21_GetGunState(port) && pIotYKC21Ctx->stProtoData[port].powerConfig.power_running <= 7)
     {
         pBuf[dataLen++] = 1;
     }
     else
     {
         pBuf[dataLen++] = 0; // 失败

         pIotYKC21Ctx->stProtoData[port].powerConfig.priority = 0;
         pIotYKC21Ctx->stProtoData[port].powerConfig.power_running = 7;
         pIotYKC21Ctx->stProtoData[port].powerConfig.limitendtimess = 0;
     }

     return dataLen;
 }
 static uint16_t IotYKC21_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
 {
     CommonDateTime_Struct dateTime;
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;

     SSTM_GetDateTime(&dateTime);
     pBuf[dataLen++] = dateTime.millisecond & 0xFF;
     pBuf[dataLen++] = (dateTime.millisecond >> 8) & 0xFF;
     pBuf[dataLen++] = dateTime.minute;
     pBuf[dataLen++] = dateTime.hour;
     pBuf[dataLen++] = dateTime.day;
     pBuf[dataLen++] = dateTime.month;
     pBuf[dataLen++] = (dateTime.year - 2000) & 0xFF;

     return dataLen;
 }
 static uint16_t IotYKC21_SendSetBillModeMultiRateRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;

     pBuf[dataLen++] = 1;

     return dataLen;
 }
 static uint16_t IotYKC21_SendPowerDefaultMaxRsp (uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     if (pIotYKC21Ctx->stProtoData[port].powerConfig.deaultMaxPowerStartTimess != 0 && pIotYKC21Ctx->stProtoData[port].powerConfig.deaultMaxPowerEndTimess != 0)
     {
         pBuf[dataLen++] = 1;
     }
     else
     {
         pBuf[dataLen++] = 0;
     }

     return dataLen;
 }
 static uint16_t IotYKC21_SendSetQrcodeRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     pBuf[dataLen++] = 0;

     return dataLen;
 }
 static uint16_t IotYKC21_SendSetParamRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     /* 默认失败 */
     pBuf[dataLen++] = 1;

     return dataLen;
 }
 static uint16_t IotYKC21_SendSetRebootRsp(uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;

     /* 默认 */
     pBuf[dataLen++] = 1;

     return dataLen;
 }
 static uint16_t IotYKC21_SendSetFTPRsp (uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;
     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 设置结果 */
     pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[0].setUpdateResult;

     return dataLen;
 }

  static uint16_t IotYKC21_SendChargeStartRsp(uint8_t port, uint8_t *pBuf)
 {
     AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
     AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
     uint16_t dataLen = 0;
     /* 交易流水号 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->stProtoData[port].newRecvOrderTransactionNum, 16);
     dataLen += 16;

     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;

     /* 远程启动结果 */
     pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[port].remoteStartResult;
     pBuf[dataLen++] = pIotYKC21Ctx->stProtoData[port].remoteStartFailReason;

     return dataLen;
 }

 static uint16_t IotYKC21_SendPileStartChargeReq(uint8_t port, uint8_t *pBuf)
 {
     AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
     AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
     uint16_t dataLen = 0;
     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 枪号 */
     pBuf[dataLen++] = port + 1;
     /* 启动方式 01-刷卡*/
     pBuf[dataLen++] = 0x01;
     /* 是否需要密码 */
     pBuf[dataLen++] = 0x00;
     /* 卡号 */
     memcpy(&pBuf[dataLen], pstChargeCtrl->authCardID, 8);
     dataLen += 8;
     /* 是否输入密码 */
     memset(&pBuf[dataLen], 0x00, 16);
     dataLen += 16;
     /* VIN码 */
     memset(&pBuf[dataLen], 0x00, 17);
     dataLen += 17;

     return dataLen;

 }

 static uint16_t IotYKC21_SendSetKeyRsp (uint8_t port, uint8_t *pBuf)
 {
     uint16_t dataLen = 0;
     /* 设备编码 */
     memcpy(&pBuf[dataLen], pIotYKC21Ctx->pileDnBCD, 7);
     dataLen += 7;
     /* 设置结果 */
     if (TRUE == pIotYKC21Ctx->rsaRefreshflg)
     {
         pBuf[dataLen++] = 1;
     }
     else
     {
        pBuf[dataLen++] = 0;
     }
         
     return dataLen;
 }


static uint16_t IotYKC21_PackHead(uint8_t port, uint16_t cmd, uint8_t encryptflg,uint8_t printflg, uint16_t seq, uint8_t *pBuf,  uint16_t dataLen)
{
    /* 起始标志 数据长度 序列号域 发送时间 加密标志 帧类型标志 消息体  帧校验域 */
    /*   1 字节  2 字节   2 字节   7 字节  1 字节   1 字节    N 字节  2 字节 */
    IotYKC21FrameHead_Struct *pFrameHead = (IotYKC21FrameHead_Struct *)pBuf;
    uint16_t encryptionMessageLen = 0; /*  消息体加密后长度 */
    uint32_t SecTimestamp = 0;
    uint16_t crc16totalLen = 0; /*  帧校验域：从序列号域到数据域的 CRC 校验 */
    uint16_t crc16 = 0;

    if (TRUE == printflg)
    {
        IOTYKC21_CFG_LogPrint("[枪：%d]发送未加密消息体数据[cmd: 0x%02X][%d]: ",port ,cmd, dataLen);
        DSLogM_HexOutput(&pBuf[1 + 2 + IOT_YKC21_ECRPTHEAD_LENGTH], dataLen);
    }

    pFrameHead->head = 0x68;

    if (FALSE == encryptflg)
    {
        encryptionMessageLen = dataLen;
        pFrameHead->dataLen[0] = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen) >> 8;
        pFrameHead->dataLen[1] = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen) & 0xFF; /* “序列号域+发送时间+加密标志+帧类型标志+消息体”字节数之和 */
        pFrameHead->seq[0] = seq >> 8;
        pFrameHead->seq[1] = seq & 0xFF;

        SecTimestamp = SSTM_GetSecTimestamp();
        Common_TimestampToCp56Time2a(SecTimestamp, &pFrameHead->sendcp56time[0]);

        pFrameHead->encryptFlag = 0;
        pFrameHead->cmd = cmd;

        crc16totalLen = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen);
    }
    else
    {
       memset(IotYKC21encryptbuf,0,IOT_YKC21_RX_ECRPTBUFFER_MAXSIZE);
       memcpy(IotYKC21encryptbuf,&pBuf[1 + 2 + IOT_YKC21_ECRPTHEAD_LENGTH],dataLen);
       encryptionMessageLen = YKC21_Send_Data_enecrypt(&pBuf[1 + 2 + IOT_YKC21_ECRPTHEAD_LENGTH], dataLen); /* 加密 */
      

        pFrameHead->dataLen[0] = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen) >> 8;
        pFrameHead->dataLen[1] = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen) & 0xFF; /* 序列号域+发送时间+加密标志+帧类型标志+消息体”字节数之和 */
        pFrameHead->seq[0] = seq >> 8;
        pFrameHead->seq[1] = seq & 0xFF;

        SecTimestamp = SSTM_GetSecTimestamp();
        Common_TimestampToCp56Time2a(SecTimestamp, &pFrameHead->sendcp56time[0]);

        pFrameHead->encryptFlag = 1;
        pFrameHead->cmd = cmd;

        crc16totalLen = (IOT_YKC21_ECRPTHEAD_LENGTH + encryptionMessageLen);
    }

    crc16 = Common_CalcCRC16(&pBuf[3], crc16totalLen);
    pBuf[1 + 2 + crc16totalLen] = (crc16 >> 8) & 0xFF;
    pBuf[1 + 2 + crc16totalLen + 1] = (crc16) & 0xFF;

    return (1 + 2 + crc16totalLen + 2);
}

void IotYKC21_UpCtrlSendDeal(void)
{
    const IotYKC21SendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeqtemporary = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_YKC21_TXRX_BUFFER_SIZE] = {0};

    if (pIotYKC21Ctx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotYKC21Ctx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotYKC21Ctx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotYKC21Ctx->sendIndex < ARRAY_SIZE(c_stIotYKC21SendctrlTable))
            {
                index = pIotYKC21Ctx->sendIndex;
                port = pIotYKC21Ctx->sendPort;

                pCmdSendCtrl = &c_stIotYKC21SendctrlTable[index];

                if ((Common_GetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotYKC21_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_YKC21_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotYKC21Ctx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_YKC21_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotYKC21Ctx->reqSeq++;
                    }
                    else
                    {
                        reqSeqtemporary = Common_GetRecvSeq(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                        uint8_t reqSeqH = reqSeqtemporary >> 8;
                        uint8_t reqSeqL = reqSeqtemporary & 0xFF;
                        reqSeq = reqSeqL << 8 | reqSeqH;
                    }

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = sizeof(IotYKC21FrameHead_Struct);
                        dataLen = pCmdSendCtrl->pSendFunc(port, &txBuf[dataLen]);
                    }

                    if (dataLen > 0)
                    {
                        dataLen = IotYKC21_PackHead(port,pCmdSendCtrl->cmd, pCmdSendCtrl->encryptionFlag, pCmdSendCtrl->printFlag, reqSeq, txBuf, dataLen);

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotYKC21Ctx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_YKC21_CMDTYPE_REQUSET)
                            {
                                pIotYKC21Ctx->reqSeq--;
                            }

                            break;
                        }

                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTYKC21_CFG_LogPrint("[枪：%d]加密发送[cmd: 0x%02X, %s][%d]: ", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        Common_SetSendFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());

                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_YKC21_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_YKC21_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotYKC21Ctx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                }
            }

            pIotYKC21Ctx->sendIndex++;

            if (pIotYKC21Ctx->sendIndex >= ARRAY_SIZE(c_stIotYKC21SendctrlTable))
            {
                pIotYKC21Ctx->sendIndex = 0;
                pIotYKC21Ctx->sendPort++;

                if (pIotYKC21Ctx->sendPort >= SYSCFG_CFG_GUN_NUM)
                {
                    pIotYKC21Ctx->sendPort = 0;
                    break;
                }
            }

            if (pIotYKC21Ctx->queueBusyFlag == TRUE)
            {
                break;
            }
        }
    }
}






