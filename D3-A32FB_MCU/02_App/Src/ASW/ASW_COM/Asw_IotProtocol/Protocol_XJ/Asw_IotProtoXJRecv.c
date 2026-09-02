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
#include "Asw_IotProtoXJM.h"
#include "Asw_IotProtoXJSend.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "Version.h"
#include "SS_Ucm.h"
#include "Asw_ChargeIf.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_XJ_RecvGunNoTransform(inputPort, outputPort)    do{ \
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
typedef struct 
{
    uint8_t paraAddr;       /* 参数地址*/
    uint8_t paraLen;        /* 参数长度*/
    uint8_t supportFlag;    /* 是否支持该参数*/
    uint8_t (*pFuncRecvSetPara)(uint8_t *para, uint8_t len);
}IotXJParaStr_Struct;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint32_t IotXJ_GetRealTimeout(uint32_t cmd, uint32_t cfgMaxTimeout);
static uint32_t IotXJ_GetRealTimeoutCount(uint32_t cmd, uint32_t cfgMaxTimeoutCount);
static const IotXJRecvCtrl_Struct* IotXJ_GetRecvCtrlPtr(uint16_t cmd);
static IotXJFrameHead_Struct *IotXJ_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen);
static void IotXJ_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen);


static uint8_t IotXJ_ParaSet106Interval(uint8_t *para, uint8_t len);
static uint8_t IotXJ_ParaSet104Interval(uint8_t *para, uint8_t len);
static uint8_t IotXJ_ParaSet102Interval(uint8_t *para, uint8_t len);
static uint8_t IotXJ_ParaSet102MaxTimeoutTimes(uint8_t *para, uint8_t len);
static uint8_t IotXJ_ParaSetIp(uint8_t *para, uint8_t len);
static uint8_t IotXJ_ParaSetPort(uint8_t *para, uint8_t len);


static uint8_t IotXJ_RecvSignRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvHeartRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvStateInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvSetRateMode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvSetStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvSetStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvEventRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvOrderInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvErrInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvSetIntegerPara(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvSetCommonPara(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvRemoteControl(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvPowerControl(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvQueryCommonPara(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvCardAuthRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotXJ_RecvCardRequestChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotXJCtx_Struct *pIotXJCtx;

static const IotXJRecvCtrl_Struct c_stIotXJRecvctrlTable[IOT_XJ_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_XJ_CMD_SEND_SIGN_INFO_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvSignRsp,
        .maxTimeout = 5 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_XJ_CMD_SEND_SIGN_INFO,
        .printFlag = TRUE,
        .cMeaning = "签到应答",
    },

    [1] = 
    {
        .cmd = IOT_XJ_CMD_SEND_HEART_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvHeartRsp,
        .maxTimeout = 10 * 1000,   /* 可设置，此为临时值 */
        .maxTryCnt = 3,  /* 可设置，此为临时值 */
        .matchCmd = IOT_XJ_CMD_SEND_HEART,
        .printFlag = TRUE,
        .cMeaning = "心跳应答",
    },

    [2] = 
    {
        .cmd = IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvStateInfoRsp,
        .maxTimeout = 5 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_XJ_CMD_SEND_STATE_INFO,
        .printFlag = TRUE,
        .cMeaning = "状态信息应答",
    },

    [3] = 
    {
        .cmd = IOT_XJ_CMD_SET_RATEMODE,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvSetRateMode,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_RATEMODE_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "设置计费模型",
    },

    [4] = 
    {
        .cmd = IOT_XJ_CMD_SET_START_CHARGE,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvSetStartCharge,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_START_CHARGE_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "启动充电",
    },

    [5] = 
    {
        .cmd = IOT_XJ_CMD_SET_STOP_CHARGE,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvSetStopCharge,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_STOP_CHARGE_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "停止充电",
    },

    [6] = 
    {
        .cmd = IOT_XJ_CMD_SEND_EVENT_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvEventRsp,
        .maxTimeout = 5 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_XJ_CMD_SEND_EVENT,
        .printFlag = TRUE,
        .cMeaning = "事件信息应答",
    },

    [7] = 
    {
        .cmd = IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvOrderInfoRsp,
        .maxTimeout = 5 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_XJ_CMD_SEND_ORDER_INFO,
        .printFlag = TRUE,
        .cMeaning = "订单信息应答",
    },

    [8] = 
    {
        .cmd = IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvErrInfoRsp,
        .maxTimeout = 5 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_XJ_CMD_SEND_ERROR_INFO,
        .printFlag = TRUE,
        .cMeaning = "故障信息应答",
    },

    [9] = 
    {
        .cmd = IOT_XJ_CMD_SET_INTEGER_PARA,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvSetIntegerPara,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_INTEGER_PARA_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "设置工作参数",
    },

    [10] = 
    {
        .cmd = IOT_XJ_CMD_SET_COMMON_PARA,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvSetCommonPara,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_COMMON_PARA_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "设置通用参数",
    },

    [11] = 
    {
        .cmd = IOT_XJ_CMD_QUERY_COMMON_PARA,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvQueryCommonPara,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_QUERY_COMMON_PARA_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "查询通用参数",
    },

    [12] = 
    {
        .cmd = IOT_XJ_CMD_REMOTE_CONFIG,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvRemoteControl,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_REMOTE_CONFIG_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "后台服务器终端控制",
    },

    [13] = 
    {
        .cmd = IOT_XJ_CMD_SET_POWER_ALLOC,
        .cmdType = IOT_XJ_CMDTYPE_REQUSET,
        .pRecvParse = IotXJ_RecvPowerControl,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_SET_POWER_ALLOC_RESPONSE,
        .printFlag = TRUE,
        .cMeaning = "远程功率控制",
    },

    [14] = 
    {
        .cmd = IOT_XJ_CMD_REQUEST_CARD_AUTH_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvCardAuthRsp,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_REQUEST_CARD_AUTH,
        .printFlag = TRUE,
        .cMeaning = "刷卡鉴权请求应答",
    },

    [15] = 
    {
        .cmd = IOT_XJ_CMD_REQUEST_CARD_CHARGE_RESPONSE,
        .cmdType = IOT_XJ_CMDTYPE_RESPONSE,
        .pRecvParse = IotXJ_RecvCardRequestChargeRsp,
        .maxTimeout = 0,
        .maxTryCnt = 0,
        .matchCmd = IOT_XJ_CMD_REQUEST_CARD_CHARGE,
        .printFlag = TRUE,
        .cMeaning = "刷卡启动充电请求应答",
    },
};

static const IotXJParaStr_Struct c_stIotXJInterParaTable[] = 
{
    { 1,       2,       TRUE,    IotXJ_ParaSet106Interval },       /*  106 报文间隔, 按照分钟为单位（默认 30 分）必须大于 0 */
    { 2,       2,       TRUE,    IotXJ_ParaSet104Interval},        /*  104 报文间隔, 定时上报的间隔单位：秒: 必须大于 0 */
    { 3,       2,       TRUE,    IotXJ_ParaSet102Interval},        /*  102 心跳上报周期, 单位：秒, 必须大于 0 */
    { 4,       2,       TRUE,    IotXJ_ParaSet102MaxTimeoutTimes}, /*  心跳包检测超时次数, 必须大于 0 */
    { 6,     130,      FALSE,    NULL },                           /*  充电枪二维码 */
    { 7,      64,       TRUE,    IotXJ_ParaSetIp },                /*  服务器域名IP，ASCII码，例如：unicron.didichuxing.co */
    { 8,       2,       TRUE,    IotXJ_ParaSetPort},               /*  服务器端口 2字节整数，例如端口 10001，对应：0x11 0x27*/
    { 9,       1,      FALSE,    NULL },                           /*  TCU日志打印level */
    { 10,      1,      FALSE,    NULL },                           /*  TCU日志上传策略 */
    { 11,      2,      FALSE,    NULL },                           /*  TCU日志上传时间间隔 */
    { 12,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 13,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 14,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 15,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 16,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 17,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 18,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 19,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 20,      4,      FALSE,    NULL },                           /*  直流相关参数 */
    { 21,      4,      FALSE,    NULL },                           /*  直流相关参数 */
    { 22,      4,      FALSE,    NULL },                           /*  直流相关参数 */
    { 23,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 24,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 25,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 26,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 27,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 28,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 29,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 30,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 31,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 32,      4,      FALSE,    NULL },                           /*  直流相关参数 */
    { 33,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 34,      2,      FALSE,    NULL },                           /*  直流相关参数 */
    { 35,      2,      FALSE,    NULL },                           /*  直流相关参数 */
};

static const IotXJParaStr_Struct c_stIotXJCommonParaTable[] = 
{
    { 0,       1,      FALSE,    NULL },      /*  杭州平台连接开关（第三平台连接开关）  */
    { 1,       1,      FALSE,    NULL },      /*  三位虚拟电量显示开关 */
    { 2,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 3,       1,      FALSE,    NULL },      /*  刷卡充电支持开关, 默认支持，不可配置 */
    { 4,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 5,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 6,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 7,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 8,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 9,       1,      FALSE,    NULL },      /*  直流相关参数 */
    { 10,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 11,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 12,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 13,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 14,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 15,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 16,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 17,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 18,      1,      FALSE,    NULL },      /*  启动充电限制时间参数 */
    { 19,      1,      FALSE,    NULL },      /*  远程重启功能， 默认支持，不可配置*/
    { 20,      1,      FALSE,    NULL },      /*  功率控制功能， 默认支持，不可配置*/
    { 21,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 22,      1,      FALSE,    NULL },      /*  离线充电功能 */
    { 23,      1,      FALSE,    NULL },      /*  直流相关参数 */
    { 24,      1,      FALSE,    NULL },      /*  自启动功能开关 */
    { 25,      1,      FALSE,    NULL },      /*  无感即插即充开关 */
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotXJ_ParaSet106Interval(uint8_t *para, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;
    uint16_t interval = 0;
    uint8_t ret = FALSE;

    interval = Common_TwoUint8ToUint16(para);

    if (interval != 0)
    {
        if (pPlatInfo->frame106Interval  != interval)
        {
            pPlatInfo->frame106Interval = interval;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }

        ret = TRUE;
    }

    return ret;
}

static uint8_t IotXJ_ParaSet104Interval(uint8_t *para, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;
    uint16_t interval = 0;
    uint8_t ret = FALSE;

    interval = Common_TwoUint8ToUint16(para);

    if (interval != 0)
    {
        if (pPlatInfo->frame104Interval  != interval)
        {
            pPlatInfo->frame104Interval = interval;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }

        ret = TRUE;
    }

    return ret;
}

static uint8_t IotXJ_ParaSet102Interval(uint8_t *para, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;
    uint16_t interval = 0;
    uint8_t ret = FALSE;

    interval = Common_TwoUint8ToUint16(para);

    if (interval != 0)
    {
        if (pPlatInfo->frame102Interval  != interval)
        {
            pPlatInfo->frame102Interval = interval;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }

        ret = TRUE;
    }

    return ret;
}

static uint8_t IotXJ_ParaSet102MaxTimeoutTimes(uint8_t *para, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;
    uint16_t maxTimeoutTimes = 0;
    uint8_t ret = FALSE;

    maxTimeoutTimes = Common_TwoUint8ToUint16(para);

    if (maxTimeoutTimes != 0)
    {
        if (pPlatInfo->frame102MaxTimeoutTimes  != maxTimeoutTimes)
        {
            pPlatInfo->frame102MaxTimeoutTimes = maxTimeoutTimes;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }

        ret = TRUE;
    }

    return ret;
}

static uint8_t IotXJ_ParaSetIp(uint8_t *para, uint8_t len)
{
    MSNvmPlatParam_Struct *pPlatParam = AswPlatM_GetPlatParamPtr();
    uint8_t ret = FALSE;

    if (len < MSNVM_PLAT_IP_LEN)
    {
        if (strncmp(pPlatParam->platMainIp, (char *)para, len) != 0)
        {
            memcpy(pPlatParam->platMainIp, para, len);
            CddNetM_UpdateIpPort(eCddNetMPlatType_O, pPlatParam->platMainIp, pPlatParam->platMainPort);
            CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
        }

        ret = (MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pPlatParam, sizeof(MSNvmPlatParam_Struct)) == eGlobalRet_OK) ? TRUE : FALSE;
    }

    return ret;
}

static uint8_t IotXJ_ParaSetPort(uint8_t *para, uint8_t len)
{
    MSNvmPlatParam_Struct *pPlatParam = AswPlatM_GetPlatParamPtr();
    uint8_t ret = TRUE;
    uint16_t port = Common_TwoUint8ToUint16(para);

    if (pPlatParam->platMainPort != port)
    {
        pPlatParam->platMainPort = port;
        CddNetM_UpdateIpPort(eCddNetMPlatType_O, pPlatParam->platMainIp, pPlatParam->platMainPort);
        CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    }

    ret = (MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pPlatParam, sizeof(MSNvmPlatParam_Struct)) == eGlobalRet_OK) ? TRUE : FALSE;
    
    return ret;
}

static uint32_t IotXJ_GetRealTimeout(uint32_t cmd, uint32_t cfgMaxTimeout)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
	uint32_t maxTimeout = 0;


    if (cmd == IOT_XJ_CMD_SEND_HEART_RESPONSE)
    {
		maxTimeout = pPrivateParam->stXJParam.platinfo.frame102Interval * 1000;
    }
    else if (cmd == IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE)
    {
		maxTimeout = pPrivateParam->stXJParam.platinfo.frame104Interval * 1000;
    }
    else
    {
        maxTimeout = cfgMaxTimeout;
    }

	return maxTimeout;
}

static uint32_t IotXJ_GetRealTimeoutCount(uint32_t cmd, uint32_t cfgMaxTimeoutCount)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
	uint32_t maxTimeoutCount = 0;

	if (cmd == IOT_XJ_CMD_SEND_HEART_RESPONSE)
	{
		maxTimeoutCount = pPrivateParam->stXJParam.platinfo.frame102MaxTimeoutTimes;
	}
	else 
	{
		maxTimeoutCount = cfgMaxTimeoutCount;
	}

	return maxTimeoutCount;
}


static uint8_t IotXJ_RecvSignRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
	CommonDateTime_Struct dateTime = { 0 };
    uint8_t index = 4;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;
	uint32_t timeStamp = 0;

	if (pIotXJCtx->loginSucc != TRUE)
	{
		pIotXJCtx->loginSucc = TRUE;
		IOTXJ_CFG_InfoPrint("[运营平台]登陆成功!\r\n");
		Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_HEART, TRUE);
		Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_STATE_INFO, TRUE);
		AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
	}

	IotXJ_BcdTimeToDateTime(&pRecvData[index], &dateTime);
	SSTM_SynTimeByDateTime(&dateTime);
    return TRUE;
}

static uint8_t IotXJ_RecvHeartRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    return TRUE;
}

static uint8_t IotXJ_RecvStateInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    return TRUE;
}

static uint8_t IotXJ_RecvEventRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 36;
    uint8_t *pRecvData = r_data;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(pRecvData[index], port[0]);

    IotXJ_DelEventQueue(port[0]);
    return TRUE;
}

static uint8_t IotXJ_RecvOrderInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotXJCtx->time);
    IOTXJ_CFG_InfoPrint("交易记录上报成功!\r\n");
    return TRUE;    
}

static uint8_t IotXJ_RecvErrInfoRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(pRecvData[index], port[0]);
    IotXJ_DelErrInfoQueue(port[0]);
    return TRUE;
}

static uint8_t IotXJ_RecvSetIntegerPara(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 4;
    uint8_t *pRecvData = r_data;
    uint8_t paraAddr = 0;
    uint8_t paraLen = 0;
    uint8_t supportFlag = FALSE;
    uint8_t paraCount = 0;
    uint8_t findFlag = FALSE;
    uint8_t temp = 0;

    pIotXJCtx->stProtoData.cmd501SetSuccesCount = 0;

    for (; (index < len) && (paraCount < ARRAY_SIZE(c_stIotXJInterParaTable)); )
    {
        paraAddr = Common_TwoUint8ToUint16(&pRecvData[index]);
        index += 2;
        findFlag = FALSE;
        supportFlag = FALSE;

        for (temp = 0; temp < ARRAY_SIZE(c_stIotXJInterParaTable); temp++)
        {
            if (c_stIotXJInterParaTable[temp].paraAddr == paraAddr)
            {
                paraLen = c_stIotXJInterParaTable[temp].paraLen;
                supportFlag = c_stIotXJInterParaTable[temp].supportFlag;
                findFlag = TRUE;
                break;
            }
        }

        if (findFlag == TRUE)
        {
            if (paraLen > (len - index) || paraLen == 0)
            {
                /* 长度错误 */
                pIotXJCtx->stProtoData.cmd501SetResult = 1;
                break;
            }
            else if (supportFlag == FALSE)
            {
                /* 其它*/
                pIotXJCtx->stProtoData.cmd501SetResult = 5;
                break;
            }
            else
            {
                if (c_stIotXJInterParaTable[temp].pFuncRecvSetPara != NULL)
                {
                    if (c_stIotXJInterParaTable[temp].pFuncRecvSetPara(pRecvData + index, paraLen) == TRUE)
                    {
                        /* 成功 */
                        pIotXJCtx->stProtoData.cmd501SetResult = 0;
                        pIotXJCtx->stProtoData.cmd501SetSuccesCount++;
                    }
                    else
                    {
                        /* 其它*/
                        pIotXJCtx->stProtoData.cmd501SetResult = 5;
                    }
                }
                else
                {
                    /* 其它*/
                    pIotXJCtx->stProtoData.cmd501SetResult = 5;
                }

                index += paraLen;
            }
        }
        else
        {
            if (paraAddr < 1)  /* 超过最小范围 */
            {
                pIotXJCtx->stProtoData.cmd501SetResult = 4;
            }
            else /* 超过最大范围 */
            {
                pIotXJCtx->stProtoData.cmd501SetResult = 4;
            }

            break;
        }

        paraCount++;
    }

    return TRUE;
}

static uint8_t IotXJ_RecvSetCommonPara(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 36;
    uint8_t *pRecvData = r_data;
    uint8_t paraAddr = 0;
    uint8_t paraLen = 0;
    uint8_t supportFlag = FALSE;
    uint8_t paraCount = 0;
    uint8_t findFlag = FALSE;
    uint8_t temp = 0;
    uint8_t totalParaCount = 0;

    pIotXJCtx->stProtoData.cmd507SetSuccesCount = 0;
    pIotXJCtx->stProtoData.cmd507SetFailCount = 0;
    pIotXJCtx->stProtoData.cmd507SetResult = 0;
    memset(pIotXJCtx->stProtoData.cmd507SetParaAddr, 0, sizeof(pIotXJCtx->stProtoData.cmd507SetParaAddr));

    /* 设置参数个数 */
    totalParaCount = pRecvData[index++];
    /* 保留 */
    index++;

    for (; (index < len) && (paraCount < ARRAY_SIZE(c_stIotXJCommonParaTable)) && (paraCount < totalParaCount); )
    {
        /* 参数项 */
        paraAddr = pRecvData[index++];
        findFlag = FALSE;
        supportFlag = FALSE;

        for (temp = 0; temp < ARRAY_SIZE(c_stIotXJCommonParaTable); temp++)
        {
            if (c_stIotXJCommonParaTable[temp].paraAddr == paraAddr)
            {
                supportFlag = c_stIotXJCommonParaTable[temp].supportFlag;
                findFlag = TRUE;
                break;
            }
        }

        /* 参数长度 */
        paraLen = pRecvData[index++];

        if (findFlag == TRUE)
        {
            if (paraLen != c_stIotXJCommonParaTable[temp].paraLen)
            {
                /* 长度错误 */
                pIotXJCtx->stProtoData.cmd507SetResult = 1;
                break;
            }
            else if (supportFlag == FALSE)
            {
                pIotXJCtx->stProtoData.cmd507SetFailCount++;
                pIotXJCtx->stProtoData.cmd507SetParaAddr[paraAddr] = 1;
                index += paraLen;
            }
            else
            {
                if (c_stIotXJCommonParaTable[temp].pFuncRecvSetPara != NULL)
                {
                    if (c_stIotXJCommonParaTable[temp].pFuncRecvSetPara(pRecvData + index, paraLen) == TRUE)
                    {
                        pIotXJCtx->stProtoData.cmd507SetSuccesCount++;
                    }
                    else
                    {
                        pIotXJCtx->stProtoData.cmd507SetFailCount++;
                        pIotXJCtx->stProtoData.cmd507SetParaAddr[paraAddr] = 1;
                    }
                }
                else
                {
                    pIotXJCtx->stProtoData.cmd507SetFailCount++;
                    pIotXJCtx->stProtoData.cmd507SetParaAddr[paraAddr] = 1;
                }

                index += paraLen;
            }
        }
        else
        {
            pIotXJCtx->stProtoData.cmd507SetResult = 1;
            break;
        }

        paraCount++;
    }

    return TRUE;
}

static uint8_t IotXJ_RecvRemoteControl(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 36;
    uint8_t *pRecvData = r_data;
    uint8_t paraAddr = pRecvData[index++];
    uint16_t para1 = 0, para2 = 0;

    pIotXJCtx->stProtoData.cmd511SetResult = 0;

    /* 保留项 */
    index+= 3;

    /* 参数1、参数2*/
    para1 = Common_TwoUint8ToUint16(&pRecvData[index]);
    index += 2;
    para2 = Common_TwoUint8ToUint16(&pRecvData[index]);
    index += 2;

    switch (paraAddr)
    {
        /* 电子解锁指令 */
        case 0x00:
        /* 地锁控制升起/降下 */
        case 0x01:
        case 0x03:
        case 0x04:
        {
            pIotXJCtx->stProtoData.cmd511SetResult = 1;
            break;
        }
        /* 重启设备 */
        case 0x02:
        {
            AswMonitor_SetReboot(eAswMonitorRebootType_WaitIdle);
            break;
        }
    }

    return TRUE;
}

static uint8_t IotXJ_RecvPowerControl(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    int32_t recvMaxPower = 0;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    /* 最大功率 */
    memcpy(&recvMaxPower, &pRecvData[index], sizeof(recvMaxPower));
    index += 4;

    /* 最大功率错误, 不支持放电 */
    if (recvMaxPower < 0)
    {
        pIotXJCtx->stProtoData.cmd20SetResult[port[0]] = 3;
    }
    else
    {
        AswChargeIf_AdjustOutputCurrent(port[0], ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, recvMaxPower * 1000);
        pIotXJCtx->stProtoData.cmd20SetResult[port[0]] = 0;
    }

    return TRUE;
}

static uint8_t IotXJ_RecvCardAuthRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    uint16_t gunNo = 0;
    uint16_t authResult = 0;

    /* 枪号 */
    memcpy(&gunNo, &pRecvData[index], 2);
    index += 2;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(gunNo, port[0]);

    /* 授权结果 */
    memcpy(&authResult, &pRecvData[index], 2);
    index += 2;

    if (authResult == 0)
    {
        IOTXJ_CFG_InfoPrint("刷卡鉴权成功!\r\n");
        Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port[0], IOT_XJ_CMD_REQUEST_CARD_CHARGE, TRUE);
    }
    else
    {
        IOTXJ_CFG_InfoPrint("刷卡鉴权失败，失败原因：%d!\r\n", authResult);
    }

    return TRUE;
}



static uint8_t IotXJ_RecvCardRequestChargeRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    uint16_t gunNo = 0;
    uint16_t authResult = 0;

    /* 枪号 */
    memcpy(&gunNo, &pRecvData[index], 2);
    index += 2;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(gunNo, port[0]);

    index += 16;

    /* 反馈结果 */
    memcpy(&authResult, &pRecvData[index], 2);
    index += 2;

    if (authResult == 0)
    {
        IOTXJ_CFG_InfoPrint("反馈成功，等待启动命令!\r\n");
    }
    else
    {
        IOTXJ_CFG_InfoPrint("反馈失败，失败原因：%d!\r\n", authResult);
    }

    return TRUE;
}


static uint8_t IotXJ_RecvQueryCommonPara(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 36;
    uint8_t *pRecvData = r_data;
    uint8_t paraCount = pRecvData[index++];
    uint8_t temp = 0;
    uint8_t paraAddr = 0;

    memset(pIotXJCtx->stProtoData.cmd514QueryParaResult, 0, sizeof(pIotXJCtx->stProtoData.cmd514QueryParaResult));
    memset(pIotXJCtx->stProtoData.cmd514QueryParaAddr, 0, sizeof(pIotXJCtx->stProtoData.cmd514QueryParaAddr));

    /* 保留项 */
    index+= 1;

    for (temp = 0; temp < paraCount; temp++)
    {
        paraAddr = pRecvData[index++];

        if (paraAddr < 26)
        {
            pIotXJCtx->stProtoData.cmd514QueryParaCount++;
            pIotXJCtx->stProtoData.cmd514QueryParaAddr[paraAddr] = 1;
        }
    }

    return TRUE;
}

static uint8_t IotXJ_RecvSetRateMode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJParamBillMode_Struct *pBillMode = &pPrivateParam->stXJParam.stBillMode;
    uint8_t index = 0;
    uint8_t *pRecvData = r_data;
    uint8_t periodCount = 0;
    uint16_t temp16Price = 0;
    uint32_t temp32Price = 0;
    uint8_t periodIndex = 0;

    pIotXJCtx->stProtoData.cmd1309SetResult = 0;

    if (len < 2)
    {
        /* 长度错误 */
        pIotXJCtx->stProtoData.cmd1309SetResult = 1;
        IOTXJ_CFG_InfoPrint("平台设置费率长度错误!\r\n");
    }
    else
    {
        periodCount = Common_TwoUint8ToUint16(&pRecvData[index]);
        index += 2;

        if (periodCount != 0 && periodCount <= 48)
        {
            if ((len - 2) != (periodCount * 16))
            {
                /* 长度错误 */
                pIotXJCtx->stProtoData.cmd1309SetResult = 1;
                IOTXJ_CFG_InfoPrint("平台设置费率长度错误!\r\n");
            }
            else
            {
                pBillMode->periodCount = periodCount;

                for (periodIndex = 0; periodIndex < periodCount; periodIndex++)
                {
                    memcpy(&pBillMode->periodDetail[periodIndex].startPeriod, &pRecvData[index], 2);
                    index += 2;
                    memcpy(&pBillMode->periodDetail[periodIndex].continuesPeriodCount, &pRecvData[index], 2);
                    index += 2;
                    memcpy(&temp32Price, &pRecvData[index], 4);
                    index += 4;
                    temp16Price = temp32Price;
                    pBillMode->periodDetail[periodIndex].elecPrice = temp16Price;
                    memcpy(&temp32Price, &pRecvData[index], 4);
                    index += 4;
                    temp16Price = temp32Price;
                    pBillMode->periodDetail[periodIndex].servePrice = temp16Price;
                    memcpy(&temp32Price, &pRecvData[index], 4);
                    index += 4;
                    temp16Price = temp32Price;
                    pBillMode->periodDetail[periodIndex].delayPrice = temp16Price;
                }

                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                IOTXJ_CFG_InfoPrint("平台设置费率成功!\r\n");
            }
        }
        else
        {
            /* 时段错误 */
            pIotXJCtx->stProtoData.cmd1309SetResult = 2;
            IOTXJ_CFG_InfoPrint("平台设置费率分段数量错误，分段数：%d!\r\n", periodCount);
        }
    }

    return TRUE;
}






static uint8_t IotXJ_CheckChargeStart(uint8_t port, uint16_t *pFailReason)
{
    uint8_t ret = FALSE;
    uint16_t reason = 0;

    /* 订单未结束 */
    if (TRUE != AswMonitor_IsOrderIdle(port))
    {
        reason = eXJErrCode_OnCharging;
    }
    /* 枪未连接 */
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        reason = eXJErrCode_GunDisconnect;
    }
    /* 存在故障 */
    else if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        IotXJ_TransfromErrCode(AswErrHandle_GetExsistError(port), (IotXJErrCode_Enum *)&reason);
        pIotXJCtx->stProtoData.chargeStarFailFlag[port] = TRUE;
    }
    /* 计费异常 */
    else if (TRUE != AswMonitor_CheckBillModeValid(port))
    {
        reason = eXJErrCode_RateModeErr;
        pIotXJCtx->stProtoData.chargeStarFailFlag[port] = TRUE;
    }
    /* 升级中 */
    else if (TRUE == SSUcm_IsUpdating())
    {
        reason = eXJErrCode_Updating;
        pIotXJCtx->stProtoData.chargeStarFailFlag[port] = TRUE;
    }
    /* 设备禁用 */
    else if (TRUE == AswMonitor_CheckForbidState())
    {
        reason = eXJErrCode_DevForbid;
        pIotXJCtx->stProtoData.chargeStarFailFlag[port] = TRUE;
    }
    else
    {}

    pFailReason[0] = reason;
    return (reason == 0);
}

static uint8_t IotXJ_RecvSetStartCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint8_t index = 4;
    uint8_t *pRecvData = r_data;
    uint32_t type = 0;
    uint16_t reason = eXJErrCode_Succ;
    uint32_t chargeType = 0;
    uint32_t chargeVal = 0;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port[0]);
    /* 充电生效类型 */
    memcpy(&type, &pRecvData[index], 4);
    index += 4;
    /* 预留 */
    index += 4;
    /* 充电策略 */
    memcpy(&chargeType, &pRecvData[index], 4);
    index += 4;
    /* 充电变量 */
    memcpy(&chargeVal, &pRecvData[index], 4);
    index += 4;

    index += 9;

    /* 订单号 */
    memcpy(pIotXJCtx->stProtoData.newRecvOrderNo[port[0]], &pRecvData[index], 32);

    if (type == 0 || chargeType == 2 || chargeType == 3)
    {
        if (TRUE == IotXJ_CheckChargeStart(port[0], &reason))
        {
            memcpy(pIotXJCtx->stProtoData.curUsedOrderNo[port[0]], pIotXJCtx->stProtoData.newRecvOrderNo[port[0]], 32);

            /* 按时间充电 */
            if (chargeType == 1)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                pChargeCtrl->chargeCtrlVal = chargeVal;
            }
            /* 按电量充电 */
            else if (chargeType == 3)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = chargeVal;
            }
            /* 自动充满 */
            else
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
            }

            pChargeCtrl->accountMoney = 999999;
            AswMonitor_ChargeStart(port[0], ASWMONITOR_ORDER_START_SRC_APP, TRUE);
        }
    }
    else
    {
        reason = eXJErrCode_ParaErr;
    }

    pIotXJCtx->stProtoData.cmd07SetReuslt[port[0]] = reason;
    return TRUE;
}


static uint8_t IotXJ_RecvSetStopCharge(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;

    /* 充电枪口 */
    IOT_XJ_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;
    /* 订单号 */
    memcpy(pIotXJCtx->stProtoData.newRecvOrderNo[port[0]], &pRecvData[index], 32);
    index += 32;

       /* 订单号 */
    if (memcmp(pIotXJCtx->stProtoData.newRecvOrderNo[port[0]], pIotXJCtx->stProtoData.curUsedOrderNo[port[0]], 32) == 0)
    {
        AswErrhandle_SetErrExsitCallback(port[0], eSrc_AppStop);
        pIotXJCtx->stProtoData.cmd11SetReuslt[port[0]] = eXJErrCode_Succ;
    }
    else
    {
        pIotXJCtx->stProtoData.cmd11SetReuslt[port[0]] = eXJErrCode_StopOrderNoErr;
    }

    return TRUE;
}









static const IotXJRecvCtrl_Struct* IotXJ_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotXJRecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_XJ_CMD_RECV_COUNT; index++)
    {
        if (c_stIotXJRecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotXJRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

int8_t IotXJ_CalcChecksum(int8_t* datas,uint16_t len)
{
    int i=0;
    int8_t ret=0;
    int32_t sum=0;

    for(i=12; i<len; i++)
    {
        sum+=datas[i];
    }
    ret = sum % 127;
    return ret;
}

static IotXJFrameHead_Struct *IotXJ_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint16_t remainLen = dataLen;
    IotXJFrameHead_Struct *pHead = NULL;
	int8_t calcCs = 0, recvCs = 0;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotXJFrameHead_Struct) + 1))
    {
        pHead = (IotXJFrameHead_Struct *)pStart;

        if ((pHead->head[0] == IOT_XJ_HEAD1) && (pHead->head[1] == IOT_XJ_HEAD2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (frameLen > (sizeof(IotXJFrameHead_Struct) + 1))
            {
                calcCs = IotXJ_CalcChecksum((int8_t *)pHead, frameLen - 1);
                recvCs = *((int8_t *)pHead + frameLen - 1);

                if (calcCs == recvCs)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + frameLen;
                    break;
                }
                else
                {
                    IOTXJ_CFG_DebugPrint("CRC校验错误，calcCs: %02X, recvCs: %02X\r\n", calcCs, recvCs);
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


static void IotXJ_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotXJRecvCtrl_Struct *pCmdRecvCtrl = NULL;
	uint8_t recvBuf[IOT_XJ_TXRX_BUFFER_SIZE];
	uint16_t remainLen = Common_ConvertUtf8ToIso(pData, dataLen, recvBuf);
    IotXJFrameHead_Struct *pFrameHead = IotXJ_FindValidFrameLen(recvBuf, remainLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;
	uint16_t recvCmd = 0;

    if (pFrameHead != NULL)
    {
		recvCmd = Common_TwoUint8ToUint16(pFrameHead->cmd);
        pCmdRecvCtrl = IotXJ_GetRecvCtrlPtr(recvCmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = Common_TwoUint8ToUint16(pFrameHead->dataLen);
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotXJFrameHead_Struct), frameLen - sizeof(IotXJFrameHead_Struct) - 1))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTXJ_CFG_DebugPrint("[枪：%d]接收[cmd: %02d, %s][%d]：", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_XJ_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, recvCmd, FALSE);
                        Common_ClearRptCount(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_XJ_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotXJCtx->pFuncRecvCtrl, port, recvCmd, Common_FourUint8ToUint32(pFrameHead->seq));
                            Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_XJ_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTXJ_CFG_DebugPrint("[枪：%d]接收[cmd: %03d, %s][%d] 处理失败：", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}

void IotXJ_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotXJCtx->frameQueueChannelID, IotXJ_DecodeData);
}

void IotXJ_TimeoutDetect(void)
{
    const IotXJRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;
	uint32_t realTimeoutCount = 0;

    for (index = 0; index < IOT_XJ_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotXJRecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_XJ_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                 continue;
            }

			realTimeoutCount = IotXJ_GetRealTimeoutCount(pCmdRecvCtrl->cmd, pCmdRecvCtrl->maxTryCnt);

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTXJ_CFG_InfoPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (timeoutCount >= realTimeoutCount)
                {
                    if (pCmdRecvCtrl->cmd == IOT_XJ_CMD_SEND_HEART_RESPONSE)
                    {
                        IotXJ_OfflineHandle();
                    }
                    else
                    {
                        if (pCmdRecvCtrl->cmd == IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE)
                        {
                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotXJCtx->time);
                            IOTXJ_CFG_InfoPrint("交易记录上报失败, 强行置为成功!\r\n");
                        }
                        else if (pCmdRecvCtrl->cmd == IOT_XJ_CMD_SEND_EVENT_RESPONSE)
                        {
                            IotXJ_DelEventQueue(port);
                        }
                        else if (pCmdRecvCtrl->cmd == IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE)
                        {
                            IotXJ_DelErrInfoQueue(port);
                        }
                        else
                        {}
                        
                        Common_SetSendFlag(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
						Common_ClearRptCount(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                    }
                }
                else
                {
                    Common_SetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
					Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
					Common_SetSendImmdFlag(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
					Common_SetSendFlag(pIotXJCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                }
            }
        }
    }
}
