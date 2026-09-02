/******************************************************************************
* File Name          : Asw_IotProtoGWERecv.c
* Description        : 国网e充电协议接收模块
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/05/22      V1.0.0      hzb        初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoGWEM.h"
#include "Asw_IotProtoGWERecv.h"
#include "Asw_IotProtoGWESend.h"
#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "Asw_ErrorHandle.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "SS_Ucm.h"
#include "FrameQueue.h"
#include "Version.h"
#include "Cdd_CP.h"
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
static uint8_t IotGWE_RecvPropReply(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvConfUpdate(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvConfGet(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvFunConfUpdate(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvFunConfGet(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvFeeModelUpdate(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvFeeModelQuery(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvStartCharge(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvStopCharge(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvOrderCheck(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvQueData(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvDevMaintainCtrl(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvDevMaintainQuery(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvTradeRecordAsk(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvMeterRecordAsk(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvTimeSync(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvAcOrderlyCharge(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvSrvRsvCharge(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvPropertySet(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvOtaUpgrade(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvOtaFirmwareReply(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotGWE_RecvNtpResponse(uint8_t port, uint8_t *r_data, uint16_t len);

extern uint32_t IotGWE_ExtractFaultTimestamp(MSNvmErrorInfo_Struct *pRec);
extern uint32_t IotGWE_ExtractOrderTimestamp(MSNvmOrderInfo_Struct *pRec);
extern uint8_t  IotGWE_CmpOrderByPreTradeNo(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize);
extern uint32_t IotGWE_CountOrdersByTimeRange(uint32_t startTs, uint32_t stopTs);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGWECtx_Struct *pIotGWECtx;

/* 属性应答表 thing.event.property.post */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlPropertyTable[] =
{
    [0] =  { .cmd = IOT_GWE_CMD_PROPERTY_ACPILE_RSP,   .matchStr = "thing.event.property.post", .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "属性上报应答" },
};

/* 事件表(设备->平台上报) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlEventTable[] =
{
    [0]  = { .cmd = IOT_GWE_CMD_FIRMWARE_INFO_RSP,     .matchStr = "firmwareTwEvt",     .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 5000, .maxTryCnt = IOT_GWE_MAX_TRY_CNT_INFINITE, .matchCmd = IOT_GWE_CMD_FIRMWARE_INFO_REQ, .cMeaning = "固件信息" },
    [1]  = { .cmd = IOT_GWE_CMD_VER_INFO_RSP,          .matchStr = "verInfoEvt",        .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "版本信息" },
    [2]  = { .cmd = IOT_GWE_CMD_DEVMDU_INFO_RSP,       .matchStr = "devMduInfoEvt",     .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "组件信息" },
    [3]  = { .cmd = IOT_GWE_CMD_DEV_CONFIG_RSP,        .matchStr = "askConfigEvt",      .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "配置信息" },
    [4]  = { .cmd = IOT_GWE_CMD_FEEMODEL_RSP,          .matchStr = "askFeeModelTwEvt",  .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "计费模型" },
    [5]  = { .cmd = IOT_GWE_CMD_TIME_SYNC_RSP,         .matchStr = "timeSyncRetEvt",    .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "时钟同步" },
    [6]  = { .cmd = IOT_GWE_CMD_START_CHA_RES_RSP,     .matchStr = "startChaResEvt",    .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 500, .maxTryCnt = 3, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "启动充电结果" },
    [7]  = { .cmd = IOT_GWE_CMD_START_CHARGE_AUTH_RSP, .matchStr = "startChargeAuthEvt",.pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "启动充电鉴权" },
    [8]  = { .cmd = IOT_GWE_CMD_STOP_CHA_RES_RSP,      .matchStr = "stopChaResEvt",     .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 500, .maxTryCnt = 3, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "停止充电结果" },
    [9]  = { .cmd = IOT_GWE_CMD_ORDER_TW_UPDATE_RSP,   .matchStr = "orderTwUpdateEvt",  .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 10000, .maxTryCnt = 3, .matchCmd = IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, .cMeaning = "交易记录" },
    [10] = { .cmd = IOT_GWE_CMD_TOTAL_FAULT_RSP,       .matchStr = "totalFaultEvt",     .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "故障告警" },
    [11] = { .cmd = IOT_GWE_CMD_AC_ST_CH_RSP,          .matchStr = "acStChEvt",         .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "枪状态" },
    [12] = { .cmd = IOT_GWE_CMD_AC_CAR_CON_CH_RSP,     .matchStr = "acCarConChEvt",     .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "CP状态" },
    [13] = { .cmd = IOT_GWE_CMD_LOG_QUERY_RESULT_RSP,  .matchStr = "logQueryEvt",       .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 5000, .maxTryCnt = IOT_GWE_MAX_TRY_CNT_INFINITE, .matchCmd = IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, .cMeaning = "日志查询结果" },
    [14] = { .cmd = IOT_GWE_CMD_DEV_MAINTAIN_RET_RSP,  .matchStr = "devMaintainRetEvt", .pRecvParse = IotGWE_RecvPropReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "维护结果" },
};

/* 服务表(平台主动下发) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlServiceTable[] =
{
    [0]  = { .cmd = IOT_GWE_SRV_CONF_UPDATE,        .matchStr = "confUpdateCtrlSrv",    .pRecvParse = IotGWE_RecvSrvConfUpdate,      .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_CONF_UPDATE_SRV_REPLY,        .cMeaning = "配置更新" },
    [1]  = { .cmd = IOT_GWE_SRV_CONF_GET,           .matchStr = "getDevConfSrv",        .pRecvParse = IotGWE_RecvSrvConfGet,         .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_CONF_GET_SRV_REPLY,           .cMeaning = "配置获取" },
    [2]  = { .cmd = IOT_GWE_SRV_FUN_CONF_UPDATE,    .matchStr = "funConfUpdateDataSrv", .pRecvParse = IotGWE_RecvSrvFunConfUpdate,   .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_FUN_CONF_UPDATE_SRV_REPLY,    .cMeaning = "功能配置更新" },
    [3]  = { .cmd = IOT_GWE_SRV_FUN_CONF_GET,       .matchStr = "getFunConfSrv",        .pRecvParse = IotGWE_RecvSrvFunConfGet,      .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_FUN_CONF_GET_SRV_REPLY,       .cMeaning = "功能配置查询" },
    [4]  = { .cmd = IOT_GWE_SRV_FEE_MODEL_UPDATE,   .matchStr = "feeModelTwUpdateSrv",  .pRecvParse = IotGWE_RecvSrvFeeModelUpdate,  .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_FEE_MODEL_UPDATE_SRV_REPLY,   .cMeaning = "计费模型更新" },
    [5]  = { .cmd = IOT_GWE_SRV_FEE_MODEL_QUERY,    .matchStr = "feeModelTwQuerySrv",   .pRecvParse = IotGWE_RecvSrvFeeModelQuery,   .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY,    .cMeaning = "计费模型查询" },
    [6]  = { .cmd = IOT_GWE_SRV_START_CHARGE,       .matchStr = "startChargeSrv",       .pRecvParse = IotGWE_RecvSrvStartCharge,     .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_START_CHARGE_SRV_REPLY,       .cMeaning = "远程启动充电" },
    [7]  = { .cmd = IOT_GWE_SRV_STOP_CHARGE,        .matchStr = "stopChargeSrv",        .pRecvParse = IotGWE_RecvSrvStopCharge,      .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_STOP_CHARGE_SRV_REPLY,        .cMeaning = "远程停止充电" },
    [8]  = { .cmd = IOT_GWE_SRV_ORDER_CHECK,        .matchStr = "orderCheckSrv",        .pRecvParse = IotGWE_RecvSrvOrderCheck,      .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL,                         .cMeaning = "交易记录确认" },
    [9]  = { .cmd = IOT_GWE_SRV_QUE_DATA,           .matchStr = "queDataSrv",           .pRecvParse = IotGWE_RecvSrvQueData,         .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_QUE_DATA_SRV_REPLY,           .cMeaning = "日志查询" },
    [10] = { .cmd = IOT_GWE_SRV_DEV_MAINTAIN_CTRL,  .matchStr = "devMaintainCtrlSrv",   .pRecvParse = IotGWE_RecvSrvDevMaintainCtrl, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_DEV_MAINTAIN_CTRL_SRV_REPLY,  .cMeaning = "设备维护" },
    [11] = { .cmd = IOT_GWE_SRV_DEV_MAINTAIN_QUERY, .matchStr = "devMaintainQuerySrv",  .pRecvParse = IotGWE_RecvSrvDevMaintainQuery,.maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_DEV_MAINTAIN_QUERY_SRV_REPLY, .cMeaning = "维护状态查询" },
    [12] = { .cmd = IOT_GWE_SRV_TRADE_RECORD_ASK,   .matchStr = "tradeRecordAskSrv",    .pRecvParse = IotGWE_RecvSrvTradeRecordAsk,  .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_TRADE_RECORD_ASK_SRV_REPLY,   .cMeaning = "交易记录召测" },
    [13] = { .cmd = IOT_GWE_SRV_METER_RECORD_ASK,   .matchStr = "meterRecordAskSrv",    .pRecvParse = IotGWE_RecvSrvMeterRecordAsk,  .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_METER_RECORD_ASK_SRV_REPLY,   .cMeaning = "电表底值召测" },
    [14] = { .cmd = IOT_GWE_SRV_TIME_SYNC,          .matchStr = "timeSyncSrv",          .pRecvParse = IotGWE_RecvSrvTimeSync,        .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_SRV_TIME_SYNC,                    .cMeaning = "时间同步" },
    [15] = { .cmd = IOT_GWE_SRV_AC_ORDERLY_CHARGE,  .matchStr = "acOrderlyChargeSrv",   .pRecvParse = IotGWE_RecvSrvAcOrderlyCharge, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_AC_ORDERLY_CHARGE_SRV_REPLY,  .cMeaning = "有序充电" },
    [16] = { .cmd = IOT_GWE_SRV_RSV_CHARGE,         .matchStr = "rsvChargeSrv",         .pRecvParse = IotGWE_RecvSrvRsvCharge,       .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_RSV_CHARGE_SRV_REPLY,         .cMeaning = "预约充电" },
    [17] = { .cmd = IOT_GWE_CMD_DEV_CONFG_UPDATE_SRV_RECV, .matchStr = "devConfgUpdateSrv", .pRecvParse = IotGWE_RecvSrvConfUpdate,  .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_CONF_UPDATE_SRV_REPLY,        .cMeaning = "设备配置更新" },
};

/* 属性设置表(平台->设备 thing/service/property/set) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlPropertySetTable[] =
{
    [0] = { .cmd = IOT_GWE_CMD_PROPERTY_SET_RECV, .matchStr = "property.set", .pRecvParse = IotGWE_RecvPropertySet, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "属性设置" },
};

/* OTA升级表(平台->设备, /ota/device/upgrade/) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlOtaUpgradeTable[] =
{
    [0] = { .cmd = IOT_GWE_CMD_OTA_UPGRADE_RECV, .matchStr = "ota", .pRecvParse = IotGWE_RecvOtaUpgrade, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "OTA升级通知" },
};

/* OTA固件信息应答表(平台->设备, thing/ota/firmware/get_reply) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlOtaFirmwareReplyTable[] =
{
    [0] = { .cmd = IOT_GWE_CMD_OTA_FIRMWARE_REPLY_RECV, .matchStr = "firmware", .pRecvParse = IotGWE_RecvOtaFirmwareReply, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "OTA固件应答" },
};

/* NTP时间应答表(平台->设备, /ext/ntp/.../response) */
static IotGWERecvCtrl_Struct c_stIotGWERecvctrlNtpResponseTable[] =
{
    [0] = { .cmd = IOT_GWE_CMD_NTP_RESPONSE_RECV, .matchStr = "serverSendTime", .pRecvParse = IotGWE_RecvNtpResponse, .maxTimeout = 0, .maxTryCnt = 0, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "NTP时间应答" },
};

static IotGWERecvTopic_Struct c_stIotGWERecvTopicTable[] =
{
    [0] =
    {
        .topic = IOT_GWE_PRE_PROPERTY_POST_REPLY,
        .cmdType = IOT_GWE_CMDTYPE_RESPONSE,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlPropertyTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlPropertyTable,
    },

    [1] =
    {
        .topic = IOT_GWE_PRE_EVENT,
        .cmdType = IOT_GWE_CMDTYPE_RESPONSE,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlEventTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlEventTable,
    },

    /* PropertySet必须在Service之前, 避免strstr将"thing/service/property/set"误匹配到"thing/service/" */
    [2] =
    {
        .topic = IOT_GWE_PRE_PROPERTY_SET,
        .cmdType = IOT_GWE_CMDTYPE_REQUSET,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlPropertySetTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlPropertySetTable,
    },

    [3] =
    {
        .topic = IOT_GWE_PRE_SERVICE,
        .cmdType = IOT_GWE_CMDTYPE_REQUSET,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlServiceTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlServiceTable,
    },

    [4] =
    {
        .topic = IOT_GWE_PRE_OTA_UPGRADE,
        .cmdType = IOT_GWE_CMDTYPE_REQUSET,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlOtaUpgradeTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlOtaUpgradeTable,
    },

    [5] =
    {
        .topic = IOT_GWE_PRE_OTA_FIRMWARE_REPLY,
        .cmdType = IOT_GWE_CMDTYPE_RESPONSE,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlOtaFirmwareReplyTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlOtaFirmwareReplyTable,
    },

    [6] =
    {
        .topic = IOT_GWE_PRE_NTP_RESPONSE,
        .cmdType = IOT_GWE_CMDTYPE_RESPONSE,
        .memberCnt = ARRAY_SIZE(c_stIotGWERecvctrlNtpResponseTable),
        .pStrRecvCtrlTable = c_stIotGWERecvctrlNtpResponseTable,
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
/**
 ******************************************************************************
 * @brief  充电前检查(是否满足充电条件)
 ******************************************************************************
 */
static uint8_t IotGWE_ChargePreCheck(uint8_t port, uint16_t *fail_code)
{
    uint8_t fail_flag = 0;

    *fail_code = eIotGWEStartFailCode_None;

    do {

        if (AswMonitor_IsOrderIdle(port) != TRUE)
        {
            fail_flag = 1;
            *fail_code = eIotGWEStartFailCode_Charging;
            break;
        }

        if (AswMonitor_CheckForbidState())
        {
            fail_flag = 2;
            *fail_code = eIotGWEStartFailCode_DevFreeze;
            break;
        }

        if (TRUE == SSUcm_IsUpdating())
        {
            fail_flag = 3;
            *fail_code = eIotGWEStartFailCode_OTAING;
            break;
        }

        if (TRUE == AswErrHandle_IsExsistError(port))
        {
            fail_flag = 4;
            *fail_code = eIotGWEStartFailCode_DevMaintain;
            break;
        }

        if (TRUE != AswChargeIf_CheckGunConnected(port))
        {
            fail_flag = 5;
            *fail_code = eIotGWEStartFailCode_GunDisconn;
            break;
        }

        if (pIotGWECtx->funConfig.allowNoFeeModelStart == 0 && TRUE != AswMonitor_CheckBillModeValid(port))
        {
            fail_flag = 6;
            *fail_code = eIotGWEStartFailCode_AuthFail;
            break;
        }

    }while (0);

    if (fail_flag)
    {
        IOTGWE_CFG_DebugPrint("[GWE] charge pre check fail[%d]\r\n", fail_flag);
    }

    return (fail_flag == 0) ? TRUE : FALSE;
}

/**
 ******************************************************************************
 * @brief  获取充电序号
 ******************************************************************************
 */
static uint16_t IotGWE_ChargeSeq_Get(void)
{
#define IOTGWE_CHARGESEQ_START  1
#define IOTGWE_CHARGESEQ_MAX    9999

    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    uint16_t curSeq = pPlatInfo->chargeSeq;
    uint16_t retSeq = curSeq;

    if (curSeq < IOTGWE_CHARGESEQ_START || curSeq > IOTGWE_CHARGESEQ_MAX)
    {
        curSeq = IOTGWE_CHARGESEQ_START;
        retSeq = curSeq;
    }

    if (++curSeq > IOTGWE_CHARGESEQ_MAX)
    {
        curSeq = IOTGWE_CHARGESEQ_START;
    }

    pPlatInfo->chargeSeq = curSeq;
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    return retSeq;
}

/**
 ******************************************************************************
 * @brief  获取操作序号
 ******************************************************************************
 */
static uint8_t IotGWE_OptSeq_Get(void)
{
#define IOTGWE_OPTSEQ_START  1
#define IOTGWE_OPTSEQ_MAX    99

    if (++pIotGWECtx->optSeq > IOTGWE_OPTSEQ_MAX)
    {
        pIotGWECtx->optSeq = IOTGWE_OPTSEQ_START;
    }

    return pIotGWECtx->optSeq;
}

/**
 ******************************************************************************
 * @brief  充电启动类型转换
 ******************************************************************************
 */
static uint8_t IotGWE_StartType_Conver(uint8_t startType)
{
    uint8_t startSrc = ASWMONITOR_ORDER_START_SRC_NULL;

    switch (startType)
    {
        case eIotGWEStartType_VIN:
        case eIotGWEStartType_VINOffline:
            startSrc = ASWMONITOR_ORDER_START_SRC_PNC;
            break;
        case eIotGWEStartType_CARD:
            startSrc = ASWMONITOR_ORDER_START_SRC_CARD;
            break;
        case eIotGWEStartType_APP:
        case eIotGWEStartType_QRCODE:
        case eIotGWEStartType_PLAT:
        case eIotGWEStartType_BLE:
        default:
            startSrc = ASWMONITOR_ORDER_START_SRC_APP;
            break;
    }

    return startSrc;
}

/**
 ******************************************************************************
 * @brief  通用属性回复处理
 ******************************************************************************
 */
static uint8_t IotGWE_RecvPropReply(uint8_t port, uint8_t *r_data, uint16_t len)
{
    cJSON *cRoot = NULL;
    cJSON *cCode = NULL;
    uint8_t ret = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
        {
            break;
        }

        cCode = cJSON_GetObjectItem(cRoot, "code");
        if (cCode != NULL && cJSON_GetNumberValue(cCode) == 200)
        {
            ret = TRUE;
        }
        else
        {
            IOTGWE_CFG_DebugPrint("[GWE] prop reply fail: %s\r\n", r_data);
        }

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [0] 配置更新 confUpdateCtrlSrv (Part4 §5.4.1.3)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvConfUpdate(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
        {
            break;
        }

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
        {
            break;
        }

        cVal = cJSON_GetObjectItem(cParams, "equipParamFreq");
        if (cVal != NULL) 
        {
            pPlatInfo->equipParamReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "gunElecFreq");
        if (cVal != NULL) 
        {
            pPlatInfo->gunElecReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "nonElecFreq");
        if (cVal != NULL) 
        {
            pPlatInfo->nonElecReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "faultWarnings");
        if (cVal != NULL) 
        {
            pPlatInfo->faultWarningsCycle = (uint32_t)cJSON_GetNumberValue(cVal);
        }
        
        cVal = cJSON_GetObjectItem(cParams, "offlinChaLen");
        if (cVal != NULL)
        {
            pPlatInfo->offlineChaLen = (uint32_t)cJSON_GetNumberValue(cVal);
        }

        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

        /* 获取二维码    */
        cJSON *qrArray = cJSON_GetObjectItem(cParams, "qrCode");
        if (qrArray != NULL && cJSON_IsArray(qrArray))
        {
            uint8_t qrCount = (uint8_t)cJSON_GetArraySize(qrArray);
            qrCount = (qrCount > SYSCFG_CFG_GUN_NUM) ? SYSCFG_CFG_GUN_NUM : qrCount;
            for (uint8_t i = 0; i < qrCount; i++)
            {
                cJSON *qrItem = cJSON_GetArrayItem(qrArray, i);
                if (qrItem != NULL && cJSON_IsString(qrItem) && qrItem->valuestring != NULL)
                {
                    MSNvmDrcode_Struct qrData;
                    memset(&qrData, 0, sizeof(qrData));
                    strncpy(qrData.qrcode, qrItem->valuestring, MSNVM_QRCODE_LEN - 1);
                    /* TODO: 多枪需扩展 NVM block, 当前仅支持枪0 */
                    MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrData, sizeof(qrData));
                }
            }
        }

        pOfflineClr->srvUpdateResult = eIotGWEResCode_Success;
        pIotGWECtx->devConfigReceived = TRUE;
        pIotGWECtx->devConfigReqCnt = 3;
        ret = TRUE;

        IOTGWE_CFG_DebugPrint("[GWE] config update: equipFreq=%u, gunFreq=%u, nonFreq=%u\r\n",
            pPlatInfo->equipParamReportCycle, pPlatInfo->gunElecReportCycle, pPlatInfo->nonElecReportCycle);

    }while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [1] 配置获取 getDevConfSrv (Part4 §5.4.1.4)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvConfGet(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IOTGWE_CFG_DebugPrint("[GWE] recv getDevConf\r\n");

    Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_CONF_GET_SRV_REPLY, TRUE);
    Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_CONF_GET_SRV_REPLY, TRUE);

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  [2] 功能配置更新 funConfUpdateDataSrv (Part4 §5.5.1.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvFunConfUpdate(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t done = FALSE;
    uint16_t funCode = 0;
    uint16_t confInt = 0;
    char confString[128] = {0};
    char optSn[41] = {0};
    uint8_t i = 0;

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
        {
            break;
        }

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
        {
            break;
        }

        cVal = cJSON_GetObjectItem(cParams, "funCode");
        if (cVal == NULL) 
        {
            break;
        }

        funCode = (uint16_t)cJSON_GetNumberValue(cVal);

        cVal = cJSON_GetObjectItem(cParams, "confInt");
        if (cVal != NULL) 
        {
            confInt = (uint16_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "confString");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(confString, cVal->valuestring, sizeof(confString) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "optSn");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(optSn, cVal->valuestring, sizeof(optSn) - 1);
        }

        switch (funCode)
        {
            case 10:    /* 允许无计费模型启动 */
                if ((confInt >> 6) & 0x1)
                {
                    pIotGWECtx->funConfig.allowNoFeeModelStart = 1;
                }
                done = TRUE;
                break;

            case 11:    /* 禁止无计费模型启动 */
                if ((confInt >> 6) & 0x1)
                {
                    pIotGWECtx->funConfig.allowNoFeeModelStart = 0;
                }
                done = TRUE;
                break;

            case 17:    /* PWM最大占空比 */
            {
                uint32_t current_mA;
                pIotGWECtx->funConfig.allowPWMMax = confInt;
                if (confInt >= 533 || confInt < 100)
                {/* 超过最大或最小值 */
                    current_mA = SYSCFG_CFG_MAX_OUTPUT_CURRENT;
                }
                else
                {
                    current_mA = (uint32_t)confInt * 60;
                }

                CddCP_AdjustCurRateCurrent(port, current_mA);
                done = TRUE;
                break;
            }

            case 18:    /* 过温告警限值 (TODO: 目前温度阈值硬编码) */
                pIotGWECtx->funConfig.allowTempWarning = confInt;
                done = TRUE;
                break;

            case 19:    /* 过温故障限值 (TODO: 目前温度阈值硬编码)  */
                pIotGWECtx->funConfig.allowTempFault = confInt;
                done = TRUE;
                break;

            case 20:
                /* TODO: OTA信息(目前没用到) */
                strncpy(pIotGWECtx->funConfig.otaInf, confString, sizeof(pIotGWECtx->funConfig.otaInf) - 1);
                done = TRUE;
                break;

            default:
                break;
        }

        pOfflineClr->srvUpdateResult = (done == TRUE) ? eIotGWEResCode_Success : eIotGWEResCode_Fail;
        pOfflineClr->srvUpdateFunCode = funCode;
        strncpy(pOfflineClr->srvUpdateOptSn, optSn, sizeof(pOfflineClr->srvUpdateOptSn) - 1);
        ret = TRUE;

        IOTGWE_CFG_DebugPrint("[GWE] funConf update: funCode=%u, confInt=%u, confString=%s, result=%u\r\n",
            funCode, confInt, confString, pOfflineClr->srvUpdateResult);

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [3] 功能配置获取 getFunConfSrv (Part4 §5.5.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvFunConfGet(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
        {
            break;
        }

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
        { 
            break;
        }

        cVal = cJSON_GetObjectItem(cParams, "funCode");
        if (cVal != NULL) 
        {
            pOfflineClr->srvUpdateFunCode = (uint16_t)cJSON_GetNumberValue(cVal);
        }

        ret = TRUE;
        IOTGWE_CFG_DebugPrint("[GWE] getFunConf: funCode=0x%04X\r\n", pOfflineClr->srvUpdateFunCode);

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  从JSON字符串中提取key对应的数字值
 ******************************************************************************
 */
static uint8_t IotGWE_JsonGetNumber(const char *data, const char *key, uint32_t *out)
{
    char buf[16];
    char *p = strstr(data, key);

    if (p == NULL)
    {
        return FALSE;
    }

    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '\"')
    {
        p++;
    }
    if (*p < '0' || *p > '9')
    {
        return FALSE;
    }

    *out = (uint32_t)atoi(p);

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  从JSON字符串中提取key对应的字符串值
 ******************************************************************************
 */
static uint8_t IotGWE_JsonGetString(const char *data, const char *key, char *outBuf, uint8_t bufSize)
{
    char *p = strstr(data, key);
    char *pEnd;
    uint8_t len;
    if (p == NULL)
    {
        return FALSE;
    }

    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '\"')
    {
        p++;
    }
    pEnd = strchr(p, '\"');
    if (pEnd == NULL)
    {
        return FALSE;
    }

    len = (uint8_t)(pEnd - p);
    if (len >= bufSize)
    {
        len = bufSize - 1;
    }
    memcpy(outBuf, p, len);
    outBuf[len] = '\0';

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  遍历JSON数字数组: 找到 "key":[n1,n2,...] 并对每个元素调用回调
 * @return 成功解析的元素个数
 ******************************************************************************
 */
static uint8_t IotGWE_ParseNumberArray(const char *data, const char *key,
    MSNvmGWEParamBillMode_Struct *pNvm, uint8_t isChargeFee)
{
    char *p = strstr(data, key);
    char *pEnd;
    uint8_t count = 0;

    if (p == NULL)
    {
        return 0;
    }

    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '\"')
    {
        p++;
    }
    if (*p != '[')
    {
        return 0;
    }
    p++;

    while (count < MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX)
    {
        while (*p == ' ' || *p == ',' || *p == '\r' || *p == '\n')
        {
            p++;
        }
        if (*p == ']' || *p == '\0')
        {
            break;
        }

        if (isChargeFee)
        {
            pNvm->elecPrice[count] = (uint32_t)atoi(p);
        }
        else
        {
            pNvm->servPrice[count] = (uint32_t)atoi(p);
        }

        while (*p >= '0' && *p <= '9')
        {
            p++;
        }
        count++;
    }

    return count;
}

/**
 ******************************************************************************
 * @brief  遍历JSON字符串数组: 找到 "key":["s1","s2",...] 并解析时间字符串为HH:MM
 * @return 成功解析的元素个数
 ******************************************************************************
 */
static uint8_t IotGWE_ParseTimeSegArray(const char *data, const char *key,
    MSNvmGWEParamBillMode_Struct *pNvm)
{
    char *p = strstr(data, key);
    uint8_t count = 0;

    if (p == NULL)
    {
        return 0;
    }
    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '\"')
    {
        p++;
    }
    if (*p != '[')
    {
        return 0;
    }
    p++;

    while (count < MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX)
    {
        while (*p == ' ' || *p == ',' || *p == '\r' || *p == '\n')
        {
            p++;
        }
        if (*p == ']' || *p == '\0')
        {
            break;
        }
        if (*p != '\"')
        {
            break;
        }
        p++;

        if (p[0] >= '0' && p[0] <= '9' && p[1] >= '0' && p[1] <= '9' &&
            p[2] >= '0' && p[2] <= '9' && p[3] >= '0' && p[3] <= '9')
        {
            pNvm->startTime[count][0] = (uint8_t)((p[0] - '0') * 10 + (p[1] - '0'));
            pNvm->startTime[count][1] = (uint8_t)((p[2] - '0') * 10 + (p[3] - '0'));
            count++;
        }

        while (*p != '\"' && *p != '\0')
        {
            p++;
        }
        if (*p == '\"')
        {
            p++;
        }
    }

    return count;
}

/**
 ******************************************************************************
 * @brief  [4] 计费模型更新 feeModelTwUpdateSrv (Part4 §6.1.3)
 * @note   使用轻量级字符串解析替代cJSON, 避免96段大JSON解析时堆内存不足
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvFeeModelUpdate(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    uint8_t ret = FALSE;
    uint8_t parsed = 0;
    uint8_t tNum = 0;
    uint8_t segCnt = 0, chgCnt = 0, svcCnt = 0;
    uint8_t gun;
    uint32_t numVal;
    char *pData = (char *)r_data;

    (void)len;
    pOfflineClr->feeModelResult = eIotGWEResCode_Fail;

    do {
        /* 充电中不允许修改计费模型 */
        for (gun = 0; gun < SYSCFG_CFG_GUN_NUM; gun++)
        {
            if (AswMonitor_IsOrderIdle(gun) != TRUE)
            {
                break;
            }
        }
        if (gun < SYSCFG_CFG_GUN_NUM)
        {
            IOTGWE_CFG_DebugPrint("[GWE] feeModel update fail: is charging\r\n");
            break;
        }

        /* 获取计费模型ID */
        IotGWE_JsonGetString(pData, "\"feeModelId\"", pOfflineClr->feeModelId, sizeof(pOfflineClr->feeModelId));

        /* 解析时段数 */
        if (!IotGWE_JsonGetNumber(pData, "\"timeNum\"", &numVal))
        {
            break;
        }
        tNum = (uint8_t)numVal;
        if (tNum == 0 || tNum > MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX)
        {
            break;
        }

        /* 写入NVM */
        MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
        MSNvmGWEParamBillMode_Struct *pNvm = &pPrivateParam->stGWEParam.stBillMode;
        memset(pNvm, 0, sizeof(MSNvmGWEParamBillMode_Struct));
        strncpy((char *)pNvm->billModeID, pOfflineClr->feeModelId, sizeof(pNvm->billModeID) - 1);
        pNvm->validFlag = 1;

        /* 轻量级解析三个数组(不使用cJSON, 避免96段大JSON解析时堆内存不足！！！) */
        segCnt = IotGWE_ParseTimeSegArray(pData, "\"timeSeg\"", pNvm);
        chgCnt = IotGWE_ParseNumberArray(pData, "\"chargeFee\"",  pNvm, TRUE);
        svcCnt = IotGWE_ParseNumberArray(pData, "\"serviceFee\"", pNvm, FALSE);

        parsed = (segCnt < tNum) ? segCnt : tNum;
        pNvm->periodCount = parsed;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

        if (parsed == 0)
        {
            break;
        }

        /* 更新当前计费模型到Monitor */
        AswPlatM_TransformBillMode(port, AswMonitor_GetCurUsedBillModePtr(port));

        pOfflineClr->feeModelResult = eIotGWEResCode_Success;
        pIotGWECtx->feeModelReceived = TRUE;
        pIotGWECtx->lastFeeModelReqTick = 0;
        ret = TRUE;

        IOTGWE_CFG_InfoPrint("[GWE] feeModel update: id=%s, periods=%u ok\r\n", pOfflineClr->feeModelId, parsed);

    } while (0);

    return ret;
}

/**
 ******************************************************************************
 * @brief  [5] 计费模型查询 feeModelTwQuerySrv (Part4 §6.1.4)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvFeeModelQuery(uint8_t port, uint8_t *r_data, uint16_t len)
{
    Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY, TRUE);
    Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY, TRUE);

    IOTGWE_CFG_DebugPrint("[GWE] feeModel[%d] query\r\n", port);

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  [6] 远程启动充电 startChargeSrv (Part4 §6.2.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvStartCharge(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmOrderInfo_Struct *pOrder = NULL;
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    uint8_t startType = eIotGWEStartType_APP;
    uint8_t chargeMode = eIotGWEChargeMode_Normal, chargeModeFlag = TRUE;
    uint32_t limitData = 0;
    char preTradeNo[41] = {0};
    char tradeNo[41] = {0};
    uint8_t startSrc = ASWMONITOR_ORDER_START_SRC_APP;
    CommonDateTime_Struct dt;
    uint32_t ts = 0;

    IOTGWE_CFG_DebugPrint("[%s] .\r\n", __FUNCTION__);

    pOfflineClr->startResult = eIotGWEStartResult_Fail;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
        {
            break;
        }

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
        {
            break;
        }

        /* 获取枪号 */
        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL) 
        {
            break;
        }

        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo == 0 || gunNo > SYSCFG_CFG_GUN_NUM) 
        {
            break;
        }
        port = gunNo - 1;
        pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

        /* 获取平台交易流水号   */
        cVal = cJSON_GetObjectItem(cParams, "preTradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(preTradeNo, cVal->valuestring, sizeof(preTradeNo) - 1);
        }

        /* 获取启动方式 */
        cVal = cJSON_GetObjectItem(cParams, "startType");
        if (cVal != NULL) 
        {
            startType = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        /* 获取充电模式 */
        cVal = cJSON_GetObjectItem(cParams, "chargeMode");
        if (cVal != NULL) 
        {
            chargeMode = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        /* 获取限制值 */
        cVal = cJSON_GetObjectItem(cParams, "limitData");
        if (cVal != NULL) 
        {
            limitData = (uint32_t)cJSON_GetNumberValue(cVal);
        }

        /* 生成设备交易流水号 */
        ts = SSTM_GetSecTimestamp();
        Common_TimestampToDateTime(ts, &dt);
        snprintf(tradeNo, sizeof(tradeNo), "%.24s%02u%02u%02u%02u%04u%02u",
            pIotGWECtx->deviceName, gunNo, dt.year % 100, dt.month, dt.day,
            IotGWE_ChargeSeq_Get(), IotGWE_OptSeq_Get());

        /* 启动前置检查 */
        if (IotGWE_ChargePreCheck(port, &pOfflineClr->startFaultCode) != TRUE)
        {
            pOfflineClr->startResult = eIotGWEStartResult_Fault;
            /* 写失败订单缓存(满足启动失败也要上报交易记录, 但不能破坏当前订单) */
            pOfflineClr->startFailOrderActive = TRUE;
            pOfflineClr->startFailChargeActive = TRUE;
            strncpy(pOfflineClr->startFailPreTradeNo, preTradeNo, sizeof(pOfflineClr->startFailPreTradeNo) - 1);
            strncpy(pOfflineClr->startFailTradeNo, tradeNo, sizeof(pOfflineClr->startFailTradeNo) - 1);
            pOfflineClr->startFailGunNo = gunNo;
            pOfflineClr->startFailStartType = startType;
            pOfflineClr->startFailStartTime = ts;
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_START_CHA_RES_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_START_CHA_RES_REQ, TRUE);

            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
            ret = TRUE;
            break;
        }

        /* 存储订单数据 */
        pOrder = AswMonitor_GerOrderDataPtr(port);
        if (pOrder == NULL)
        {
            break;
        }
        memset(pOrder, 0x00, sizeof(MSNvmOrderInfo_Struct));
        pOrder->protocolType = eAswPlatType_GWE;
        pGWEOrder = &pOrder->platOrderInfo.stGWEOrderInfo;
        strncpy(pGWEOrder->preTradeNo, preTradeNo, sizeof(pGWEOrder->preTradeNo) - 1);
        strncpy(pGWEOrder->tradeNo, tradeNo, sizeof(pGWEOrder->tradeNo) - 1);
        pGWEOrder->gunNo = gunNo;
        pGWEOrder->startType = startType;
        pGWEOrder->startTime = ts;
        {
            MSNvmPlatPrivateParam_Union *pPP = AswPlatM_GetPlatPrivateParamPtr();
            MSNvmGWEParamBillMode_Struct *pBM = &pPP->stGWEParam.stBillMode;
            memcpy(pGWEOrder->billModeID, pBM->billModeID, sizeof(pBM->billModeID));
        }

        /* 启动类型转换 */
        startSrc = IotGWE_StartType_Conver(startType);

        /* 充电模式 */
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
        switch (chargeMode)
        {
            case eIotGWEChargeMode_LimitAmount: /* 按金额充电 */
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                pChargeCtrl->chargeCtrlVal = limitData;   /*  0.01元 */
                pChargeCtrl->accountMoney = limitData;
                break;
            case eIotGWEChargeMode_LimitElec:   /* 按电量充电 */
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = limitData * 10;   /* 0.1kWh */
                pChargeCtrl->accountMoney = 999999;
                break;
            case eIotGWEChargeMode_LimitTime:   /* 按时间充电 */
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                pChargeCtrl->chargeCtrlVal = limitData * 60;    /* 分 */
                pChargeCtrl->accountMoney = 999999;
                break;
            case eIotGWEChargeMode_Normal:      /* 自动充满 */
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                pChargeCtrl->chargeCtrlVal = 0;
                pChargeCtrl->accountMoney = 999999;
                break;
            case eIotGWEChargeMode_LimitPower:  /* 限制功率 */
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                pChargeCtrl->chargeCtrlVal = 0;
                pChargeCtrl->accountMoney = 999999;
                AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, limitData * 100);
                break;
            default: /* 其他充电模式不支持 */
                chargeModeFlag = FALSE;
                break;
        }

        if (chargeModeFlag == FALSE)
        {/* 充电模式不支持 */
            pOfflineClr->startResult = eIotGWEStartResult_Fail;
            pOfflineClr->startFaultCode = eIotGWEStartFailCode_NotSupportTheAuth;
            ret = TRUE;
            IOTGWE_CFG_DebugPrint("[GWE] fail, unsupported chargeMode=%u\r\n", chargeMode);
            break;
        }

        /* 启动充电   */
        AswMonitor_ChargeStart(port, startSrc, FALSE);

        pOfflineClr->startResult = eIotGWEStartResult_Success;
        pOfflineClr->startFaultCode = eIotGWEStartFailCode_None;

        /* 触发启动充电结果上报 (交易记录等充电结束再报) */
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_START_CHA_RES_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_START_CHA_RES_REQ, TRUE);

        ret = TRUE;

        IOTGWE_CFG_DebugPrint("[GWE] startCharge: gunNo[%u], preTradeNo[%s], tradeNo[%s], startType[%u], chargeMode[%u]\r\n",
            gunNo, preTradeNo, tradeNo, startType, chargeMode);

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [7] 远程停止充电 stopChargeSrv (Part4 §6.4.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvStopCharge(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmOrderInfo_Struct *pOrder = NULL;
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    char preTradeNo[41] = {0};
    char tradeNo[41] = {0};
    int stopResultCode = 0;

    IOTGWE_CFG_DebugPrint("[%s] .\r\n", __FUNCTION__);

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
        {
            break;
        }

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL)
        {
            break;
        }

        /* 获取枪序号 */
        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL)
        {
            break;
        }

        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo < 1 || gunNo > SYSCFG_CFG_GUN_NUM)
        {
            break;
        }
        port = gunNo - 1;

        /* 获取平台交易流水号 */
        cVal = cJSON_GetObjectItem(cParams, "preTradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(preTradeNo, cVal->valuestring, sizeof(preTradeNo) - 1);
        }

        /* 获取设备交易流水号 */
        cVal = cJSON_GetObjectItem(cParams, "tradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(tradeNo, cVal->valuestring, sizeof(tradeNo) - 1);
        }

        /* 获取停止原因 */
        cVal = cJSON_GetObjectItem(cParams, "stopReason");
        if (cVal != NULL)
        {
            stopResultCode = cVal->valueint;
        }
        
        /* 获取当前订单, 校验交易流水号 */
        pOrder = AswMonitor_GerOrderDataPtr(port);
        if (pOrder == NULL)
        {
            break;
        }
        pGWEOrder = &pOrder->platOrderInfo.stGWEOrderInfo;
        pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
        switch(stopResultCode)
        {
            case 10: pOfflineClr->stopResultCode = eIotGWEStopResultCode_AppStop;       break;
            case 11: pOfflineClr->stopResultCode = eIotGWEStopResultCode_AmountBalance; break;
            case 12: pOfflineClr->stopResultCode = eIotGWEStopResultCode_PrePay;        break;
            case 13: pOfflineClr->stopResultCode = eIotGWEStopResultCode_PlatStop;      break;
            case 14: pOfflineClr->stopResultCode = eIotGWEStopResultCode_PlatStop;      break;
            default: pOfflineClr->stopResultCode = eIotGWEStopResultCode_AppStop;       break;
        }

        if (AswMonitor_IsOrderIdle(port) == TRUE)
        {/* 当前无进行中的订单 */
            pOfflineClr->stopResult = eIotGWEStartResult_Fail;
            pOfflineClr->stopFailReson = eIotGWEStopFailReason_NotWork;
            IOTGWE_CFG_DebugPrint("[GWE] stopCharge fail: port=%u not charging\r\n", port);
            break;
        }

        if (strlen(preTradeNo) > 0 && strcmp(preTradeNo, pGWEOrder->preTradeNo) != 0)
        {/* preTradeNo 不匹配 */
            pOfflineClr->stopResult = eIotGWEStartResult_Fail;
            pOfflineClr->stopFailReson = eIotGWEStopFailReason_TradeNoMisMatch;
            IOTGWE_CFG_DebugPrint("[GWE] stopCharge fail: preTradeNo mismatch req=[%s] cur=[%s]\r\n",
                preTradeNo, pGWEOrder->preTradeNo);
            break;
        }

        /* 更新设备交易流水号 tradeNo */
        if (strlen(tradeNo) > 0)
        {
            strncpy(pGWEOrder->tradeNo, tradeNo, sizeof(pGWEOrder->tradeNo) - 1);
        }

        AswErrhandle_SetErrExsitCallback(port, eSrc_AppStop);

        pOfflineClr->stopResult = eIotGWEStartResult_Success;
        pOfflineClr->stopFailReson = 0;

        ret = TRUE;

        IOTGWE_CFG_DebugPrint("[GWE] stopCharge: gunNo=%u, preTradeNo=%s\r\n", gunNo, preTradeNo);

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [8] 交易记录确认 orderCheckSrv (Part4 §6.5.3)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvOrderCheck(uint8_t port, uint8_t *r_data, uint16_t len)
{
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    uint8_t errcode = 0;
    char preTradeNo[41] = {0};

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL) 
            break;

        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo < 1 || gunNo > SYSCFG_CFG_GUN_NUM) 
            break;

        port = gunNo - 1;

        cVal = cJSON_GetObjectItem(cParams, "preTradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(preTradeNo, cVal->valuestring, sizeof(preTradeNo) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "errCode");
        if (cVal != NULL)
        {
            errcode = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        ret = TRUE;
        IOTGWE_CFG_DebugPrint("[GWE] orderCheck %s: gun=%u, preTradeNo=%s\r\n", errcode == 10 ? "OK" : "Fail", gunNo, preTradeNo);

        if (pIotGWECtx->reportingPreTradeNo[0] != '\0' &&
            strncmp(preTradeNo, pIotGWECtx->reportingPreTradeNo, sizeof(pIotGWECtx->reportingPreTradeNo) - 1) == 0)
        {
            if (pIotGWECtx->reportingRecordTime > 0)
            {
                MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGWECtx->reportingRecordTime);
            }
            pIotGWECtx->orderReportAwaitTick = 0;
            pIotGWECtx->reportingPreTradeNo[0] = '\0';
            pIotGWECtx->reportUseRuntime[port] = FALSE;
        }

    } while (0);

    if (cRoot) 
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [9] 设备日志查询 queDataSrv (Part4 §5.8.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvQueData(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    uint8_t isCharging = FALSE;
    uint8_t i = 0;
    uint32_t latestTime; 
    uint32_t recordCount = 0;
    uint8_t  validType = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL)
            break;

        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL)
            break;

        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo < 1 || gunNo > SYSCFG_CFG_GUN_NUM)
            break;

        port = gunNo - 1;
        pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

        cVal = cJSON_GetObjectItem(cParams, "startDate");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            pOfflineClr->logQueryStartDate = (uint32_t)atol(cVal->valuestring) + SSTM_BASE_TIMESTAMP_1970_BJT;
        }

        cVal = cJSON_GetObjectItem(cParams, "stopDate");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            pOfflineClr->logQueryStopDate = (uint32_t)atol(cVal->valuestring) + SSTM_BASE_TIMESTAMP_1970_BJT;
        }

        cVal = cJSON_GetObjectItem(cParams, "askType");
        if (cVal != NULL)
        {
            pOfflineClr->logQueryAskType = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "logQueryNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->logQueryNo, cVal->valuestring, sizeof(pOfflineClr->logQueryNo) - 1);
        }

        /* 检查是否有枪在充电 (充电中不允许上传日志) */
        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            if (AswMonitor_IsOrderIdle(i) != TRUE)
            {
                isCharging = TRUE;
                break;
            }
        }

        if (isCharging)
        {
            pOfflineClr->srvQueDataResult = eIotGWELogResult_NOTALLOW;
            IOTGWE_CFG_DebugPrint("[GWE] queData: charging, not allowed\r\n");
        }
        else
        {
            switch (pOfflineClr->logQueryAskType)
            {
                case eIotGWELogAskType_Order:
                    recordCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_OrderRecord);
                    validType = TRUE;
                    break;
                case eIotGWELogAskType_Log:
                    recordCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_RunningLogRecord);
                    validType = TRUE;
                    break;
                case eIotGWELogAskType_MeterStart:
                    recordCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_MeterRecord);
                    validType = TRUE;
                    break;
                case eIotGWELogAskType_FAULT:
                    recordCount = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_ErrorRecord);
                    validType = TRUE;
                    break;
                default:
                    break;
            }

            if (validType == FALSE)
            {
                pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
            }
            else if (recordCount > 0)
            {
                /* 检查是否有其他类型的日志查询正在上传 */
                if (pIotGWECtx->meterRecordUploadActive == TRUE ||
                    pIotGWECtx->faultRecordUploadActive == TRUE ||
                    pIotGWECtx->tradeRecordUploadActive == TRUE ||
                    pIotGWECtx->runLogUploadActive == TRUE)
                {
                    pOfflineClr->srvQueDataResult = eIotGWELogResult_NOTALLOW;
                    break;
                }

                pOfflineClr->srvQueDataResult = eIotGWELogResult_DATA;
                pOfflineClr->logQueryRetType = eIotGWELogRetType_Frame;

                if (pOfflineClr->logQueryAskType == eIotGWELogAskType_MeterStart)
                {/* 电表底值*/
                    MSNvmMeterRecord_Struct latestRec;
                    uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_MeterRecord);
                    if (latestIdx > 0 && 
                        eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_MeterRecord, (uint8_t *)&latestRec, sizeof(latestRec), latestIdx))
                    {
                        latestTime = latestRec.acqTime;
                    }
                    else
                    {
                        latestTime = 0;
                    }

                    if (pOfflineClr->logQueryStopDate > 0 && pOfflineClr->logQueryStopDate < latestTime)
                    {/* 平台指定了结束时间且早于最新记录, 则封顶上界 */
                        latestTime = pOfflineClr->logQueryStopDate;
                    }
                    if (latestTime < pOfflineClr->logQueryStartDate)
                    {
                        latestTime = pOfflineClr->logQueryStopDate;
                    }
                    pOfflineClr->logQueryEvtSum = (recordCount > IOT_GWE_METERRECORD_PREMAX) ? IOT_GWE_METERRECORD_PREMAX : (uint8_t)recordCount;
                    pOfflineClr->logQueryEvtNo = 1;
                    pIotGWECtx->meterRecordAskMode = eIotGWEMeterAskType_All;
                    pIotGWECtx->meterRecordCursorTime = latestTime;
                    pIotGWECtx->meterRecordUploadActive = TRUE;
                    pIotGWECtx->meterRecordUploadPort = port;
                }
                else if (pOfflineClr->logQueryAskType == eIotGWELogAskType_FAULT)
                {/* 故障告警 */
                    uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_ErrorRecord);
                    MSNvmErrorInfo_Struct latestRec;
                    pOfflineClr->logQueryEvtSum = (recordCount > IOT_GWE_FAULTRECORD_PREMAX) ? IOT_GWE_FAULTRECORD_PREMAX : (uint8_t)recordCount;
                    pOfflineClr->logQueryEvtNo = 1;
                    if (latestIdx > 0 &&
                        eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_ErrorRecord, (uint8_t *)&latestRec, sizeof(latestRec), latestIdx))
                    {
                        pIotGWECtx->faultRecordCursorTime = IotGWE_ExtractFaultTimestamp(&latestRec);
                    }
                    else
                    {
                        pIotGWECtx->faultRecordCursorTime = 0;
                    }
                    if (pOfflineClr->logQueryStopDate > 0 && pOfflineClr->logQueryStopDate < pIotGWECtx->faultRecordCursorTime)
                    {/* 平台指定了结束时间且早于最新记录, 则封顶上界 */
                        pIotGWECtx->faultRecordCursorTime = pOfflineClr->logQueryStopDate;
                    }
                    if (pIotGWECtx->faultRecordCursorTime < pOfflineClr->logQueryStartDate)
                    {
                        pIotGWECtx->faultRecordCursorTime = pOfflineClr->logQueryStopDate;
                    }

                    pIotGWECtx->faultRecordUploadActive = TRUE;
                    pIotGWECtx->faultRecordUploadPort = port;
                }
                else if (pOfflineClr->logQueryAskType == eIotGWELogAskType_Order)
                {/* 交易记录 */
                    uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_OrderRecord);
                    MSNvmOrderInfo_Struct latestRec;
                    pOfflineClr->logQueryEvtSum = (recordCount > IOT_GWE_ORDERRECORD_PREMAX) ? IOT_GWE_ORDERRECORD_PREMAX : (uint8_t)recordCount;
                    pOfflineClr->logQueryEvtNo = 1;
                    if (latestIdx > 0 &&
                        eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_OrderRecord, (uint8_t *)&latestRec, sizeof(latestRec), latestIdx))
                    {
                        pIotGWECtx->tradeRecordCursorTime = IotGWE_ExtractOrderTimestamp(&latestRec);
                    }
                    else
                    {
                        pIotGWECtx->tradeRecordCursorTime = 0;
                    }

                    pIotGWECtx->tradeRecordUploadActive = TRUE;
                    pIotGWECtx->tradeRecordUploadPort = port;
                }
                else if (pOfflineClr->logQueryAskType == eIotGWELogAskType_Log)
                {/* 设备日志: 从最新记录开始分片上传, 最多100KB */
                    pOfflineClr->logQueryEvtSum = (recordCount > IOTGWE_RUNLOG_UPLOAD_MAXCNT) ? IOTGWE_RUNLOG_UPLOAD_MAXCNT : (uint8_t)recordCount;
                    pOfflineClr->logQueryEvtNo = 1;
                    pIotGWECtx->runLogCursorIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_RunningLogRecord);
                    pIotGWECtx->runLogByteOffset = 0;
                    pIotGWECtx->runLogSentBytes = 0;
                    pIotGWECtx->runLogUploadActive = TRUE;
                }

                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
            }
            else
            {
                pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
            }

            IOTGWE_CFG_DebugPrint("[GWE] queData: gun=%u, askType=%u, [%u-%u], result=%u, cnt=%u\r\n",
                gunNo, pOfflineClr->logQueryAskType,
                (uint32_t)pOfflineClr->logQueryStartDate, (uint32_t)pOfflineClr->logQueryStopDate,
                pOfflineClr->srvQueDataResult, (uint32_t)recordCount);
        }

        ret = TRUE;

    } while (0);

    if (cRoot) 
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [10] 设备维护控制 devMaintainCtrlSrv (Part4 §5.9.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvDevMaintainCtrl(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t ctrlType = 0;
    uint8_t isCharging = FALSE;
    uint8_t i = 0;

    do { 
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL)
            break;

        cVal = cJSON_GetObjectItem(cParams, "ctrlType");
        if (cVal == NULL)
            break;

        ctrlType = (uint8_t)cJSON_GetNumberValue(cVal);

        /* 检查是否有枪在充电 (充电中不允许执行设备维护) */
        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            if (AswMonitor_IsOrderIdle(i) != TRUE)
            {
                isCharging = TRUE;
                break;
            }
        }

        pOfflineClr->devMaintainCtrlType = ctrlType;
        if (isCharging)
        {
            pOfflineClr->devMaintainReason = eIotGWEDevMaintainReason_Charging;
            IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: ctrlType=%u fail, charging\r\n", ctrlType);
        }
        else
        {
            pOfflineClr->devMaintainReason = eIotGWEDevMaintainReason_Success;

            switch (ctrlType)
            {
                case eIotGWECtrlType_Reset:
                case eIotGWECtrlType_HardReset:
                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: reboot ctrlType=%u\r\n", ctrlType);
                    AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);
                    break;

                case eIotGWECtrlType_Maintenance:
                    /* 检修: 设备应为故障状态但可充电 */
                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: maintenance mode\r\n");
                    AswMonitor_SetForbidState(FALSE, 0);
                    break;

                case eIotGWECtrlType_Freeze:
                case eIotGWECtrlType_Suspension:
                case eIotGWECtrlType_ReturnShip:
                    /* 冻结/停运/退运: 设备故障状态, 不能充电 */
                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: lock mode ctrlType=%u\r\n", ctrlType);
                    AswMonitor_SetForbidState(TRUE, ctrlType);
                    break;

                case eIotGWECtrlType_Commission:
                    /* 投运: 恢复正常充电服务 */
                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: commission\r\n");
                    AswMonitor_SetForbidState(FALSE, 0);
                    break;

                case eIotGWECtrlType_Factory:
                {
                    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
                    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;

                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: factory reset\r\n");
                    AswMonitor_SetForbidState(FALSE, 0);
                    memset(pPlatInfo->cProductKey, 0, sizeof(pPlatInfo->cProductKey));
                    memset(pPlatInfo->cDeviceName, 0, sizeof(pPlatInfo->cDeviceName));
                    memset(pPlatInfo->cDeviceSecret, 0, sizeof(pPlatInfo->cDeviceSecret));
                    pPlatInfo->credentialSaveFlag = FALSE;
                    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                    AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);
                    break;
                }

                default:
                    IOTGWE_CFG_DebugPrint("[GWE] devMaintainCtrl: unknown ctrlType=%u\r\n", ctrlType);
                    pOfflineClr->devMaintainReason = 0;
                    break;
            }
        }

        ret = TRUE;

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    if (ret == TRUE)
    {
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ, TRUE);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [11] 设备维护状态查询 devMaintainQuerySrv (Part4 §5.9.4)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvDevMaintainQuery(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    uint8_t lockState = 0, lockReason = 0;

    AswMonitor_GetForbidState(&lockState, &lockReason);
    pOfflineClr->devMaintainCtrlType = (lockState == TRUE) ? lockReason : eIotGWECtrlType_Commission;

    pOfflineClr->devMaintainQueryResult = eIotGWEResCode_Success;

    IOTGWE_CFG_DebugPrint("[GWE] devMaintainQuery: current ctrlType=%u\r\n", pOfflineClr->devMaintainCtrlType);

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  [12] 交易记录召测 tradeRecordAskSrv (Part4 §5.12.1.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvTradeRecordAsk(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    uint8_t isCharging = FALSE;
    uint8_t i = 0;
    uint32_t startTs = 0, stopTs = 0xFFFFFFFF;

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL) 
            break;

        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo < 1 || gunNo > SYSCFG_CFG_GUN_NUM) 
            break;
        
        port = gunNo - 1;
        pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

        cVal = cJSON_GetObjectItem(cParams, "askType");
        if (cVal != NULL) 
        {
            pOfflineClr->srvTradeAskType = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "preTradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->srvTradeAskPreTradeNo, cVal->valuestring, sizeof(pOfflineClr->srvTradeAskPreTradeNo) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "tradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->srvTradeAskTradeNo, cVal->valuestring, sizeof(pOfflineClr->srvTradeAskTradeNo) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "startDate");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->srvTradeAskStartDate, cVal->valuestring, sizeof(pOfflineClr->srvTradeAskStartDate) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "stopDate");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->srvTradeAskStopDate, cVal->valuestring, sizeof(pOfflineClr->srvTradeAskStopDate) - 1);
        }

        /* 检查是否有枪在充电 */
        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            if (AswMonitor_IsOrderIdle(i) != TRUE)
            {
                isCharging = TRUE;
                break;
            }
        }

        /* 获取开始和结束时间 */
        {
            unsigned int y = 0, m = 0, d = 0, h = 0, min = 0, sec = 0;
            CommonDateTime_Struct dt;
            if (pOfflineClr->srvTradeAskStartDate[0] != '\0' &&
                sscanf(pOfflineClr->srvTradeAskStartDate, "%4u%2u%2u%2u%2u%2u", &y, &m, &d, &h, &min, &sec) == 6)
            {
                memset(&dt, 0, sizeof(dt));
                dt.year = (uint16_t)y; dt.month = (uint8_t)m; dt.day = (uint8_t)d;
                dt.hour = (uint8_t)h; dt.minute = (uint8_t)min; dt.second = (uint8_t)sec;
                startTs = Common_DateTimeToTimestamp(&dt);
            }
            if (pOfflineClr->srvTradeAskStopDate[0] != '\0' &&
                sscanf(pOfflineClr->srvTradeAskStopDate, "%4u%2u%2u%2u%2u%2u", &y, &m, &d, &h, &min, &sec) == 6)
            {
                memset(&dt, 0, sizeof(dt));
                dt.year = (uint16_t)y; dt.month = (uint8_t)m; dt.day = (uint8_t)d;
                dt.hour = (uint8_t)h; dt.minute = (uint8_t)min; dt.second = (uint8_t)sec;
                stopTs = Common_DateTimeToTimestamp(&dt);
            }
        }

        if (pOfflineClr->srvTradeAskType == eIotGWEOrderAskType_Single)
        {/* 单条记录: 按 preTradeNo 查找 */
            MSNvmOrderInfo_Struct rec;
            if (eGlobalRet_OK == MSNvm_QueryRecordByExternal(eMSNvmBlockID_OrderRecord,
                  (uint8_t *)pOfflineClr->srvTradeAskPreTradeNo, strlen(pOfflineClr->srvTradeAskPreTradeNo),
                  IotGWE_CmpOrderByPreTradeNo, (uint8_t *)&rec, sizeof(rec)))
            {
                pOfflineClr->srvTradeAskCnt = 1;
                pOfflineClr->srvTradeAskResult = eIotGWEResCode_Success;
            }
            else
            {
                pOfflineClr->srvTradeAskCnt = 0;
                pOfflineClr->srvTradeAskResult = eIotGWEResCode_Fail;
            }
        }
        else if (pOfflineClr->srvTradeAskType == eIotGWEOrderAskType_Unreport)
        {/* 未上送记录 */
            pOfflineClr->srvTradeAskCnt = (uint8_t)MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord);
            pOfflineClr->srvTradeAskResult = (pOfflineClr->srvTradeAskCnt > 0) ? eIotGWEResCode_Success : eIotGWEResCode_Fail;
        }
        else
        {/* 全部记录: 按时间范围统计 */
            pOfflineClr->srvTradeAskCnt = (uint8_t)IotGWE_CountOrdersByTimeRange(startTs, stopTs);
            pOfflineClr->srvTradeAskResult = (pOfflineClr->srvTradeAskCnt > 0) ? eIotGWEResCode_Success : eIotGWEResCode_Fail;
        }

        if (pOfflineClr->srvTradeAskCnt == 0 || isCharging == TRUE)
        {
            if (isCharging == TRUE)
            {
                pOfflineClr->srvTradeAskResult = eIotGWEResCode_Success;
                pOfflineClr->srvTradeAskCnt = 0;
                IOTGWE_CFG_DebugPrint("[GWE] tradeRecordAsk: charging, skip upload\r\n");
            }
            ret = TRUE;
            break;
        }

        if (pOfflineClr->srvTradeAskType == eIotGWEOrderAskType_Unreport)
        {
            pIotGWECtx->tradeRecordUploadFlag = TRUE;
        }
        else if (pOfflineClr->srvTradeAskType == eIotGWEOrderAskType_Single)
        {/* 单条 */
            uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_OrderRecord);
            uint32_t idx;
            for (idx = 1; idx <= latestIdx; idx++)
            {/* 遍历, 查找匹配的订单号 */
                MSNvmOrderInfo_Struct tmp;
                if (eGlobalRet_OK != MSNvm_QueryRecordByTime(eMSNvmBlockID_OrderRecord, (uint8_t *)&tmp, sizeof(tmp), idx))
                    continue;

                if (strncmp(tmp.platOrderInfo.stGWEOrderInfo.preTradeNo, pOfflineClr->srvTradeAskPreTradeNo, 40) == 0)
                {
                    pIotGWECtx->reportingRecordTime = idx;
                    pIotGWECtx->reportUseRuntime[port] = FALSE;
                    Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
                    Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
                    break;
                }
            }
        }
        else
        {
            pIotGWECtx->tradeRecordAskAllStartTs = startTs;
            pIotGWECtx->tradeRecordAskAllStopTs = stopTs;
            pIotGWECtx->tradeRecordAskAllCursor = startTs;
            pIotGWECtx->tradeRecordAskAllPort = port;
            pIotGWECtx->tradeRecordAskAllActive = TRUE;
        }

        ret = TRUE;

        IOTGWE_CFG_DebugPrint("[GWE] tradeRecordAsk: gun=%u, askType=%u, [%s,%s], result=%u, cnt=%u\r\n",
            gunNo, pOfflineClr->srvTradeAskType,
            pOfflineClr->srvTradeAskStartDate, pOfflineClr->srvTradeAskStopDate,
            pOfflineClr->srvTradeAskResult, pOfflineClr->srvTradeAskCnt);

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [13] 电表底值召测 meterRecordAskSrv (Part4 §5.12.2.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvMeterRecordAsk(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;
    uint8_t isCharging = FALSE;
    uint32_t recordCnt = 0;
    uint8_t i = 0;

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cParams, "gunNo");
        if (cVal == NULL) 
            break;
        gunNo = (uint8_t)cJSON_GetNumberValue(cVal);
        if (gunNo < 1 || gunNo > SYSCFG_CFG_GUN_NUM) 
            break;

        port = gunNo - 1;
        pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

        cVal = cJSON_GetObjectItem(cParams, "askType");
        if (cVal != NULL) 
        {
            pOfflineClr->srvMeterAskType = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        cVal = cJSON_GetObjectItem(cParams, "askDate");
        if (cVal != NULL && cVal->valuestring != NULL)
        {    
            strncpy(pOfflineClr->srvMeterAskDate, cVal->valuestring, sizeof(pOfflineClr->srvMeterAskDate) - 1);
        }

        /* 检查是否有枪在充电   */
        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            if (AswMonitor_IsOrderIdle(i) != TRUE)
            {
                isCharging = TRUE;
                break;
            }
        }

        /* 正在上传中 */
        if (pIotGWECtx->meterRecordUploadActive == TRUE)
        {
            pOfflineClr->srvMeterAskResult = eIotGWEResCode_Fail;
            ret = TRUE;
            IOTGWE_CFG_DebugPrint("[GWE] meterRecordAsk: upload busy, reject\r\n");
            break;
        }

        pIotGWECtx->meterRecordAskMode = pOfflineClr->srvMeterAskType;
        recordCnt = MSNvm_QueryTotalRecordCount(eMSNvmBlockID_MeterRecord);

        /* 充电中/无记录均不触发上传 */
        if (isCharging || recordCnt == 0)
        {
            pOfflineClr->srvMeterAskResult = eIotGWEResCode_Fail;
            ret = TRUE;
            break;
        }

        pOfflineClr->srvMeterAskResult = eIotGWEResCode_Success;
        ret = TRUE;

        /* 触发 logQueryEvt 逐帧上传 */
        pOfflineClr->logQueryAskType = eIotGWELogAskType_MeterStart;
        unsigned int y = 0, m = 0, d = 0, h = 0, min = 0, sec = 0;
        if (sscanf(pOfflineClr->srvMeterAskDate, "%4u%2u%2u%2u%2u%2u", &y, &m, &d, &h, &min, &sec) == 6)
        {
            CommonDateTime_Struct dt;
            memset(&dt, 0, sizeof(dt));
            dt.year = (uint16_t)y;
            dt.month = (uint8_t)m;
            dt.day = (uint8_t)d;
            dt.hour = (uint8_t)h;
            dt.minute = (uint8_t)min;
            dt.second = (uint8_t)sec;
            pOfflineClr->logQueryStartDate = Common_DateTimeToTimestamp(&dt);
        }
        else
        {
            pOfflineClr->logQueryStartDate = SSTM_GetSecTimestamp() - IOTGWE_METERRECORD_ASKTIME;
        }

        pOfflineClr->logQueryStopDate = SSTM_GetSecTimestamp();
        pOfflineClr->logQueryRetType = eIotGWELogRetType_Frame;
        pOfflineClr->logQueryEvtSum = (recordCnt > IOT_GWE_METERRECORD_PREMAX) ? IOT_GWE_METERRECORD_PREMAX : recordCnt;
        pOfflineClr->logQueryEvtNo = 1;
        pOfflineClr->srvQueDataResult = eIotGWELogResult_DATA;

        /* 获取最新记录时间戳 */
        uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_MeterRecord);
        if (latestIdx > 0)
        {
            MSNvmMeterRecord_Struct latestRec;
            if (eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_MeterRecord,
                (uint8_t *)&latestRec, sizeof(latestRec), latestIdx))
            {
                pIotGWECtx->meterRecordCursorTime = latestRec.acqTime;
            }
        }

        pIotGWECtx->meterRecordUploadActive = TRUE;
        pIotGWECtx->meterRecordUploadPort = port;

        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);

        IOTGWE_CFG_DebugPrint("[GWE] meterRecordAsk: gun=%u, askType=%u, date=%s, result=%u, cnt=%u\r\n",
            gunNo, pOfflineClr->srvMeterAskType, pOfflineClr->srvMeterAskDate,
            pOfflineClr->srvMeterAskResult, recordCnt);

    } while (0);

    if (cRoot) 
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [14] 时间同步 timeSyncSrv (Part4 §5.13.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvTimeSync(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t isCharging = FALSE;
    uint8_t i;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cParams, "srvTime");
        if (cVal == NULL) 
            break;

        pOfflineClr->platTimestamp = (cVal->valuestring != NULL) ?
            (uint32_t)atol(cVal->valuestring) : 0;

        /* 充电中不允许校时 */
        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            if (AswMonitor_IsOrderIdle(i) != TRUE)
            {
                isCharging = TRUE;
                break;
            }
        }

        if (isCharging)
        {
            pOfflineClr->timeSyncResult = IotGWESyncTimeRet_FailNoAllow;
            IOTGWE_CFG_DebugPrint("[GWE] timeSync: charging, deferred ts=%u\r\n", pOfflineClr->platTimestamp);
        }
        else
        {
            SSTM_SynTimeBySecTimeStamp(pOfflineClr->platTimestamp + SSTM_BASE_TIMESTAMP_1970_BJT);
            pOfflineClr->timeSyncResult = IotGWESyncTimeRet_Success;

            /* 标记对时成功 */
            pIotGWECtx->timeSyncFlag = 1;
            pIotGWECtx->timeSyncReqCnt = 0;

            /* 触发时钟同步结果上报 (timeSyncRetEvt) */
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_TIME_SYNC_RESULT_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_TIME_SYNC_RESULT_REQ, TRUE);
            IOTGWE_CFG_DebugPrint("[GWE] timeSync: ts=%u OK\r\n", pOfflineClr->platTimestamp);
        }

        ret = TRUE;

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [15] 有序充电 acOrderlyChargeSrv (Part4 §7.2.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvAcOrderlyCharge(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    cJSON *cValidTime = NULL, *cKw = NULL;
    uint8_t ret = FALSE;
    uint8_t num = 0;
    uint8_t i = 0;
    uint8_t reason = eIotGWEPoileReason_Success; 

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
        { 
            reason = eIotGWEPoileReason_ParamInvaild; 
            break; 
        }

        cVal = cJSON_GetObjectItem(cParams, "preTradeNo");
        if (cVal != NULL && cVal->valuestring != NULL)
        {
            strncpy(pOfflineClr->srvOrderlyChargePreTradeNo, cVal->valuestring, sizeof(pOfflineClr->srvOrderlyChargePreTradeNo) - 1);
        }

        cVal = cJSON_GetObjectItem(cParams, "num");
        if (cVal != NULL)
        {
            num = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        if (num == 0 || num > MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX) 
        {
            reason = eIotGWEPoileReason_ParamInvaild; 
            break; 
        }

        /* 获取策略时间:HHMM */
        cValidTime = cJSON_GetObjectItem(cParams, "validTime");
        if (cValidTime != NULL)
        {
            for (i = 0; i < num; i++)
            {
                cJSON *timeItem = cJSON_GetArrayItem(cValidTime, i);
                if (timeItem == NULL || timeItem->valuestring == NULL || strlen(timeItem->valuestring) < 4)
                {
                    reason = eIotGWEPoileReason_ParamInvaild;
                    break;
                }
            }
        }
        if (reason == eIotGWEPoileReason_ParamInvaild) 
            break;

        /* 校验设置功率有效性 */
        cKw = cJSON_GetObjectItem(cParams, "kw");
        if (cKw == NULL) 
        { 
            reason = eIotGWEPoileReason_ParamInvaild; 
            break; 
        }

        for (i = 0; i < num; i++)
        {
            cJSON *kwItem = cJSON_GetArrayItem(cKw, i);
            if (kwItem == NULL) 
            { 
                reason = eIotGWEPoileReason_ParamInvaild; 
                break; 
            }

            uint16_t kw = (uint16_t)cJSON_GetNumberValue(kwItem);
            if (kw < 13 || kw > 70)
            {/* 功率值超出范围 */
                reason = eIotGWEPoileReason_PowerOutRange;
                IOTGWE_CFG_DebugPrint("[GWE] acOrderlyCharge: kw[%u]=%u out of range\r\n", i, kw);
                break;
            }
        }

        if (reason != eIotGWEPoileReason_Success)
            break;

        IotGWEOrderlyChargeStrategy_Struct *pStrategy = &pIotGWECtx->sOrderlyCharge[port];
        memset(pStrategy, 0, sizeof(IotGWEOrderlyChargeStrategy_Struct));
        strncpy(pStrategy->preTradeNo, pOfflineClr->srvOrderlyChargePreTradeNo, sizeof(pStrategy->preTradeNo) - 1);
        pStrategy->num = num;

        for (i = 0; i < num; i++)
        {
            cJSON *timeItem = cJSON_GetArrayItem(cValidTime, i);
            if (timeItem != NULL && timeItem->valuestring != NULL)
            {
                strncpy(pStrategy->validTime[i], timeItem->valuestring, sizeof(pStrategy->validTime[i]) - 1);
            }

            cJSON *kwItem = cJSON_GetArrayItem(cKw, i);
            if (kwItem != NULL)
            {
                pStrategy->kw[i] = (uint16_t)cJSON_GetNumberValue(kwItem);
            }
        }

        pIotGWECtx->OrderlyChargeFlg[port] = TRUE;
        pIotGWECtx->lastOrderlyChargeKw[port] = IOT_GWE_PROILEPOWER_DEFAULT;

        IOTGWE_CFG_DebugPrint("[GWE] acOrderlyCharge: preTradeNo=%s, num=%u, result=ok\r\n",
                                                    pOfflineClr->srvOrderlyChargePreTradeNo, num);

    } while (0);

    pOfflineClr->srvOrderlyChargeReason = reason;
    if (reason == eIotGWEPoileReason_Success)
    {
        pOfflineClr->srvOrderlyChargeResult = eIotGWEResCode_Success;
    }
    else
    {
        pOfflineClr->srvOrderlyChargeResult = eIotGWEResCode_Fail;
        pIotGWECtx->OrderlyChargeFlg[port] = FALSE;
        memset(&pIotGWECtx->sOrderlyCharge[port], 0, sizeof(IotGWEOrderlyChargeStrategy_Struct));
    }

    ret = TRUE;

    if (cRoot) 
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  [16] 预约充电 rsvChargeSrv (Part4 §6.7.2)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvSrvRsvCharge(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t gunNo = 0;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL)
            break;

        cVal = cJSON_GetObjectItem(cParams, "appoMethod");
        if (cVal != NULL) 
        {
            pOfflineClr->srvRsvChargeAppoMethod = (uint8_t)cJSON_GetNumberValue(cVal);
        }

        /* TODO: 暂不支持预约充电 */
        pOfflineClr->srvRsvChargeGunNo = gunNo;
        pOfflineClr->srvRsvChargeResult = eIotGWEResCode_Fail;

        IOTGWE_CFG_DebugPrint("[GWE] rsvCharge: gun=%u, appoMethod=%u\r\n", gunNo, pOfflineClr->srvRsvChargeAppoMethod);

        ret = TRUE;

    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  属性设置 property.set (Part2 §6.3.1)
 ******************************************************************************
 */
static uint8_t IotGWE_RecvPropertySet(uint8_t port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    cJSON *cRoot = NULL, *cParams = NULL, *cVal = NULL;
    uint8_t ret = FALSE;
    uint8_t updated = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cParams = cJSON_GetObjectItem(cRoot, "params");
        if (cParams == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cParams, "equipParamFreq");
        if (cVal != NULL)
        {
            pPlatInfo->equipParamReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
            updated = TRUE;
        }

        cVal = cJSON_GetObjectItem(cParams, "gunElecFreq");
        if (cVal != NULL)
        {
            pPlatInfo->gunElecReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
            updated = TRUE;
        }

        cVal = cJSON_GetObjectItem(cParams, "nonElecFreq");
        if (cVal != NULL)
        {
            pPlatInfo->nonElecReportCycle = (uint32_t)cJSON_GetNumberValue(cVal);
            updated = TRUE;
        }

        cVal = cJSON_GetObjectItem(cParams, "faultWarnings");
        if (cVal != NULL)
        {
            pPlatInfo->faultWarningsCycle = (uint32_t)cJSON_GetNumberValue(cVal);
            updated = TRUE;
        }

        cVal = cJSON_GetObjectItem(cParams, "offlinChaLen");
        if (cVal != NULL)
        {
            pPlatInfo->offlineChaLen = (uint32_t)cJSON_GetNumberValue(cVal);
            updated = TRUE;
        }

        if (updated == TRUE)
        {
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }

        ret = TRUE;
        IOTGWE_CFG_DebugPrint("[GWE] property.set: equipFreq=%u, gunFreq=%u, nonFreq=%u, fault=%u, offline=%u\r\n",
                                pPlatInfo->equipParamReportCycle, pPlatInfo->gunElecReportCycle,
                                pPlatInfo->nonElecReportCycle, pPlatInfo->faultWarningsCycle,
                                pPlatInfo->offlineChaLen);

    } while (0);

    if (cRoot) cJSON_Delete(cRoot);

    return ret;
}

/**
 ******************************************************************************
 * @brief  解析OTA下载URL, 提取HTTP参数
 ******************************************************************************
 */
static uint8_t IotGWE_ParseOtaUrl(char *otaUrl, CddNetMSocketPara_Union *pSocketPara)
{
    uint16_t urlLen;
    uint8_t ret = FALSE;

    do {
        if (otaUrl == NULL || pSocketPara == NULL)
            break;

        if (strncmp(otaUrl, "https://", 8) != 0 && strncmp(otaUrl, "http://", 7) != 0)
            break;

        urlLen = (uint16_t)strlen(otaUrl);
        if (urlLen >= CDD_NETM_CFG_HTTP_URL_LEN)
        {
            urlLen = CDD_NETM_CFG_HTTP_URL_LEN - 1;
        }

        memcpy(pSocketPara->stHttpPara.url, otaUrl, urlLen);
        pSocketPara->stHttpPara.url[urlLen] = '\0';
        pSocketPara->stHttpPara.urlLen = urlLen;

        ret = TRUE;

    } while (0);

    return ret;
}

/**
 ******************************************************************************
 * @brief  OTA升级校验
 * @return TRUE=已启动OTA, FALSE=校验不通过
 ******************************************************************************
 */
static uint8_t IotGWE_TryStartOta(const char *otaVersion, const char *otaUrl,
                                  const char *tag)
{
    CddNetMSocketPara_Union socketPara;
    uint8_t i;

    if (pIotGWECtx->otaState == eIotGWEOTAState_Starting)
    {
        IOTGWE_CFG_DebugPrint("[GWE] %s: already upgrading, skip\r\n", tag);
        return FALSE;
    }

    for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
    {
        if (AswMonitor_IsOrderIdle(i) != TRUE)
        {
            IOTGWE_CFG_DebugPrint("[GWE] %s: charging, reject\r\n", tag);
            return FALSE;
        }
    }

    {
        char curVersion[IOTGWE_VERSION_LEN] = {0};
        IOTGWE_MAKE_SOFT_VERSION(curVersion, sizeof(curVersion));
        if (strcmp(otaVersion, curVersion) == 0)
        {
            IOTGWE_CFG_DebugPrint("[GWE] %s: same version %s, skip\r\n", tag, curVersion);
            return FALSE;
        }
    }

    if (SSUcm_CheckUpdateCondition() != TRUE)
    {
        IOTGWE_CFG_DebugPrint("[GWE] %s: condition not met\r\n", tag);
        return FALSE;
    }

    memset(&socketPara, 0, sizeof(socketPara));
    if (IotGWE_ParseOtaUrl((char *)otaUrl, &socketPara) == FALSE)
    {
        IOTGWE_CFG_DebugPrint("[GWE] %s: url parse failed: %s\r\n", tag, otaUrl);
        return FALSE;
    }

    socketPara.stHttpPara.type = eCddNetMHttpType_SegGET;
    SSUcm_ReqStartOTA(&socketPara, eSSUcmChannelType_HTTP, eSSUcmExcuteMode_Immediate, IOTGWE_OTA_TIMEOUT);

    pIotGWECtx->otaState = eIotGWEOTAState_Starting;
    pIotGWECtx->lastOtaProgress = 0xFF;
    IOTGWE_CFG_DebugPrint("[GWE] %s: started, version=%s, url=%s\r\n", tag, otaVersion, otaUrl);

    return TRUE;
}

/**
 ******************************************************************************
 * @brief  OTA升级通知
 ******************************************************************************
 */
static uint8_t IotGWE_RecvOtaUpgrade(uint8_t port, uint8_t *r_data, uint16_t len)
{
    cJSON *cRoot = NULL, *cData = NULL, *cVal = NULL;
    char otaVersion[32] = {0};
    char otaUrl[CDD_NETM_CFG_HTTP_URL_LEN] = {0};
    uint8_t ret = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
            break;

        cVal = cJSON_GetObjectItem(cRoot, "code");
        if (cVal == NULL || cVal->valuestring == NULL ||
            strcmp(cVal->valuestring, "1000") != 0)
        {
            IOTGWE_CFG_DebugPrint("[GWE] OTA: no upgrade task, code=%s\r\n",
                (cVal && cVal->valuestring) ? cVal->valuestring : "null");
            break;
        }

        cData = cJSON_GetObjectItem(cRoot, "data");
        if (cData == NULL)
            break;

        cVal = cJSON_GetObjectItem(cData, "version");
        if (cVal == NULL || cVal->valuestring == NULL)
            break;
        strncpy(otaVersion, cVal->valuestring, sizeof(otaVersion) - 1);

        cVal = cJSON_GetObjectItem(cData, "url");
        if (cVal == NULL || cVal->valuestring == NULL)
            break;
        strncpy(otaUrl, cVal->valuestring, sizeof(otaUrl) - 1);

        IOTGWE_CFG_DebugPrint("[GWE] OTA: version=%s, size=%d\r\n", otaVersion,
            (int)cJSON_GetNumberValue(cJSON_GetObjectItem(cData, "size")));

        ret = IotGWE_TryStartOta(otaVersion, otaUrl, "OTA");
    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  OTA固件信息应答 /sys/.../thing/ota/firmware/get_reply
 ******************************************************************************
 */
static uint8_t IotGWE_RecvOtaFirmwareReply(uint8_t port, uint8_t *r_data, uint16_t len)
{
    cJSON *cRoot = NULL, *cData = NULL, *cVal = NULL;
    char otaVersion[32] = {0};
    char otaUrl[CDD_NETM_CFG_HTTP_URL_LEN] = {0};
    uint8_t ret = FALSE;

    do {

        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL)
            break;

        cVal = cJSON_GetObjectItem(cRoot, "code");
        if (cVal == NULL || (int)cJSON_GetNumberValue(cVal) != 200)
            break;

        cData = cJSON_GetObjectItem(cRoot, "data");
        if (cData == NULL)
            break;

        cVal = cJSON_GetObjectItem(cData, "version");
        if (cVal == NULL || cVal->valuestring == NULL)
            break;
        strncpy(otaVersion, cVal->valuestring, sizeof(otaVersion) - 1);

        cVal = cJSON_GetObjectItem(cData, "url");
        if (cVal == NULL || cVal->valuestring == NULL)
            break;
        strncpy(otaUrl, cVal->valuestring, sizeof(otaUrl) - 1);

        IOTGWE_CFG_DebugPrint("[GWE] OTA firmware reply: version=%s, size=%d\r\n",
            otaVersion, (int)cJSON_GetNumberValue(cJSON_GetObjectItem(cData, "size")));

        ret = IotGWE_TryStartOta(otaVersion, otaUrl, "OTA firmware");
    } while (0);

    if (cRoot)
    {
        cJSON_Delete(cRoot);
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  NTP时间应答 /ext/ntp/{pk}/{dn}/response
 ******************************************************************************
 */
static uint8_t IotGWE_RecvNtpResponse(uint8_t port, uint8_t *r_data, uint16_t len)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cVal = NULL;
    uint8_t ret = FALSE;

    do {
        cRoot = cJSON_Parse((const char *)r_data);
        if (cRoot == NULL) 
            break;

        cVal = cJSON_GetObjectItem(cRoot, "serverSendTime");
        if (cVal != NULL)
        {
            /* NTP应答时间戳为ms */
            pOfflineClr->platTimestamp = (uint32_t)(cJSON_GetNumberValue(cVal) / 1000);
            SSTM_SynTimeBySecTimeStamp(pOfflineClr->platTimestamp + SSTM_BASE_TIMESTAMP_1970_BJT);
            pOfflineClr->timeSyncResult = IotGWESyncTimeRet_Success;

            pIotGWECtx->timeSyncFlag = 1;
            pIotGWECtx->timeSyncReqCnt = 0;

            /* 触发时钟同步结果上报 */
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_TIME_SYNC_RESULT_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_TIME_SYNC_RESULT_REQ, TRUE);

            IOTGWE_CFG_DebugPrint("[GWE] NTP time sync: serverSendTime=%u\r\n", pOfflineClr->platTimestamp);
            ret = TRUE;
        }

        cJSON_Delete(cRoot);

    } while (0);

    return ret;
}

/**
 ******************************************************************************
 * @brief  从接收Topic中解析RPC序列号(topic末尾数字)
 ******************************************************************************
 */
static uint32_t IotGWE_ParseRecvRpc(uint8_t *recvBuf, uint16_t dataLen)
{
    cJSON *cRoot = NULL;
    cJSON *cId = NULL;
    uint32_t retRpc = 0;

    cRoot = cJSON_Parse((const char *)recvBuf);
    if (cRoot != NULL)
    {
        cId = cJSON_GetObjectItem(cRoot, "id");
        if (cId != NULL && cId->valuestring != NULL)
        {
            retRpc = (uint32_t)atoi(cId->valuestring);
        }
        cJSON_Delete(cRoot);
    }

    return retRpc;
}

/**
 ******************************************************************************
 * @brief  从JSON中解析枪号(从0索引开始)
 ******************************************************************************
 */
static uint8_t IotGWE_ParseRecvPort(uint8_t *recvBuf, uint16_t dataLen, uint8_t *pEnsure)
{
    cJSON *params = NULL;
    cJSON *gunNo = NULL;
    uint8_t port = 0;
    uint8_t gunPort = 0;

    cJSON *cRoot = cJSON_Parse((const char *)recvBuf);
    if (cRoot != NULL)
    {
        params = cJSON_GetObjectItem(cRoot, "params");

        if (params == NULL)
        {
            gunNo = cJSON_GetObjectItem(cRoot, "gunNo");
        }
        else
        {
            gunNo = cJSON_GetObjectItem(params, "gunNo");
        }

        if (gunNo != NULL)
        {
            if (pEnsure != NULL)
            {
                pEnsure[0] = TRUE;
            }

            gunPort = (uint8_t)cJSON_GetNumberValue(gunNo);
            port = (gunPort > 0 && gunPort <= SYSCFG_CFG_GUN_NUM) ? (gunPort - 1): 0;
        }
    }

    cJSON_Delete(cRoot);

    return port;
}

/**
 ******************************************************************************
 * @brief  匹配接收命令控制表
 ******************************************************************************
 */
static uint8_t IotGWE_GetMatchRecvCtrlTable(uint8_t *pPort, IotGWERecvTopic_Struct *pRecvTopicTable,
    uint8_t *recvBuf, uint16_t dataLen, uint32_t rpc, uint8_t *pTabelIndex, uint8_t ensureGunNoFlag)
{
    IotGWERecvCtrl_Struct *pRecvCtrlTable = NULL;
    uint8_t ret = FALSE;
    uint8_t index = 0;
    uint8_t gunNo = 0;

    /* 1. 先按mathStr */
    for (index = 0; index < pRecvTopicTable->memberCnt; index++)
    {
        pRecvCtrlTable = &pRecvTopicTable->pStrRecvCtrlTable[index];
        if (pRecvCtrlTable->matchStr == NULL)
        {
            continue;
        }

        if (pRecvTopicTable->cmdType != IOT_GWE_CMDTYPE_REQUSET &&
            pRecvCtrlTable->matchCmd != IOT_GWE_CMD_NULL)
        {
            continue;
        }

        if (Common_SearchData(recvBuf, dataLen, pRecvCtrlTable->matchStr, strlen(pRecvCtrlTable->matchStr)))
        {
            pTabelIndex[0] = index;
            ret = TRUE;
            break;
        }
    }

    /* 2. 未匹配则按 sendFlag + rpc(id) 兜底 */
    if (ret == FALSE)
    {
        for (index = 0; index < pRecvTopicTable->memberCnt; index++)
        {
            pRecvCtrlTable = &pRecvTopicTable->pStrRecvCtrlTable[index];

            if (ensureGunNoFlag != TRUE)
            {
                for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
                {
                    if (TRUE == Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, gunNo, pRecvCtrlTable->matchCmd) &&
                        rpc == Common_GetRecvSeq(pIotGWECtx->pFuncRecvCtrl, gunNo, pRecvCtrlTable->cmd))
                    {
                        pPort[0] = gunNo;
                        pTabelIndex[0] = index;
                        ret = TRUE;
                        break;
                    }
                }
                if (ret == TRUE) 
                    break;
            }
            else
            {
                if (TRUE == Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, pPort[0],
                    pRecvCtrlTable->matchCmd) &&
                    rpc == Common_GetRecvSeq(pIotGWECtx->pFuncRecvCtrl, pPort[0],
                    pRecvCtrlTable->cmd))
                {
                    pTabelIndex[0] = index;
                    ret = TRUE;
                    break;
                }
            }
        }
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  根据接收Topic查找对应的Topic分组控制表
 ******************************************************************************
 */
static IotGWERecvTopic_Struct* IotGWE_FindRecvTopicTablePointer(char *topic, uint8_t topicLen)
{
    IotGWERecvTopic_Struct *pRecvTopicTable = NULL;
    uint8_t index = 0;

    for (index = 0; index < ARRAY_SIZE(c_stIotGWERecvTopicTable); index++)
    {
        if (strstr(topic, c_stIotGWERecvTopicTable[index].topic) != NULL)
        {
            pRecvTopicTable = &c_stIotGWERecvTopicTable[index];
            break;
        }
    }

    return pRecvTopicTable;
}

/**
 ******************************************************************************
 * @brief  解码接收数据
 ******************************************************************************
 */
static void IotGWE_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    IotGWERecvTopic_Struct *pRecvTopicTable = NULL;
    IotGWERecvCtrl_Struct *pRecvCtrl = NULL;
    uint8_t index = 0;
    uint32_t rpc = 0;
    uint8_t port = 0;
    uint8_t ensureGunNoFlag = FALSE;
    char topic[128] = { 0 };

    if (dataLen > 0 && topicLen > 0)
    {
        if (topicLen < (sizeof(topic) - 1))
        {
            memcpy(topic, pTopic, topicLen);
        }
        /* 匹配topic主题 */
        pRecvTopicTable = IotGWE_FindRecvTopicTablePointer(topic, topicLen);

        if (pRecvTopicTable != NULL)
        {
            rpc = IotGWE_ParseRecvRpc(pData, dataLen);
            port = IotGWE_ParseRecvPort(pData, dataLen, &ensureGunNoFlag);

            if (IotGWE_GetMatchRecvCtrlTable(&port, pRecvTopicTable, pData, dataLen, rpc, &index, ensureGunNoFlag))
            {
                pRecvCtrl = &pRecvTopicTable->pStrRecvCtrlTable[index];

                if (pRecvCtrl->pRecvParse != NULL)
                {/* 调用对应帧的解析接口 */
                    if (TRUE == pRecvCtrl->pRecvParse(port, pData, dataLen))
                    {
                        if (pRecvTopicTable->cmdType == IOT_GWE_CMDTYPE_RESPONSE)
                        {/* 应答包 */
                            Common_SetRecvEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, TRUE);
                            Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                            Common_ClearRptCount(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
                            
                            if (pRecvCtrl->cmd == IOT_GWE_CMD_STOP_CHA_RES_RSP)
                            {/* 停止充电结果上报已确认, 清除停止结果码 */
                                pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.stopResultCode = 0;
                            }
                        }
                        else
                        {/* 请求包 */
                            if (pRecvCtrl->matchCmd != IOT_GWE_CMD_NULL)
                            {
                                Common_SetRecvSeq(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, rpc);
                                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            }
                        }

                        if (pRecvCtrl->matchCmd != IOT_GWE_CMD_NULL)
                        {
                            Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                        }
                    }
                    else
                    {
                        IOTGWE_CFG_DebugPrint("[国网e充电] %s parse fail: %s\r\n", pRecvCtrl->cMeaning, (char *)pData);
                    }
                }
            }
            else
            {
                IOTGWE_CFG_DebugPrint("[国网e充电] JSON解析失败: %s\r\n", (char *)pData);
            }
        }
    }
}

/**
 ******************************************************************************
 * @brief  超时重发: 重置收发状态, 放弃当前等待立刻重来
 ******************************************************************************
 */
static void IotGWE_RetrySendNow(uint8_t port, const IotGWERecvCtrl_Struct *pRecvCtrl)
{
    Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
    Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
    Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
    Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
}

/**
 ******************************************************************************
 * @brief  超时处理
 ******************************************************************************
 */
static void IotGWE_CmdTimeoutHandle(uint8_t port, uint16_t cmd)
{
    switch (cmd)
    {
        case IOT_GWE_CMD_OTA_FIRMWARE_REPLY_RECV:
            /* OTA固件应答超时 */
            IOTGWE_CFG_DebugPrint("[GWE] OTA firmware reply timeout, port=%u\r\n", port);
            break;

        case IOT_GWE_CMD_NTP_RESPONSE_RECV:
            /* NTP时间应答超时 */
            IOTGWE_CFG_DebugPrint("[GWE] NTP response timeout, port=%u\r\n", port);
            break;

        case IOT_GWE_CMD_LOG_QUERY_RESULT_RSP:
            /* 逐帧上传不重发 */
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, FALSE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, FALSE);
            IOTGWE_CFG_DebugPrint("[GWE] logQueryEvt timeout, skip frame port=%u\r\n", port);
            break;

        case IOT_GWE_CMD_ORDER_TW_UPDATE_RSP:
            if (pIotGWECtx->reportingRecordTime == 0 && pIotGWECtx->reportUseRuntime[port] != TRUE)
            {/* 启动失败的交易记录不重发 */
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, FALSE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, FALSE);
            }
            IOTGWE_CFG_DebugPrint("[GWE] orderTwUpdate RSP timeout, port=%u\r\n", port);
            break;

        default:
            break;
    }
}

/**
 ******************************************************************************
 * @brief  接收超时检测 - 遍历所有RESPONSE类型的接收控制表项
 ******************************************************************************
 */
void IotGWE_TimeoutDetect(void)
{
    IotGWERecvTopic_Struct *pRecvTopicTable = NULL;
    IotGWERecvCtrl_Struct *pRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t temp = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < ARRAY_SIZE(c_stIotGWERecvTopicTable); index++)
    {
        pRecvTopicTable = &c_stIotGWERecvTopicTable[index];

        if (pRecvTopicTable->memberCnt == 0)
        {
            continue;
        }

        /* 跳过REQUEST类型(只检测RESPONSE类型) */
        if (pRecvTopicTable->cmdType != IOT_GWE_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (temp = 0; temp < pRecvTopicTable->memberCnt; temp++)
        {
            pRecvCtrl = &pRecvTopicTable->pStrRecvCtrlTable[temp];

            for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
            {
                if (Common_GetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd) != TRUE)
                {
                    continue;
                }

                if (pRecvCtrl->maxTimeout == 0)
                {
                    continue;
                }

                if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd), pRecvCtrl->maxTimeout) == TRUE)
                {
                    Common_SetRptCount(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
                    timeoutCount = Common_GetRptCount(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);

                    IOTGWE_CFG_DebugPrint("[cmd:0x%04X %s] 接收超时第%d次, 超时时间:%dms\r\n", pRecvCtrl->cmd, pRecvCtrl->cMeaning, timeoutCount, pRecvCtrl->maxTimeout);

                    if (pRecvCtrl->maxTryCnt == IOT_GWE_MAX_TRY_CNT_INFINITE)
                    {
                        if (timeoutCount >= 3)
                        {
                            Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                            Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                        }
                        else
                        {
                            IotGWE_RetrySendNow(port, pRecvCtrl);
                            IotGWE_CmdTimeoutHandle(port, pRecvCtrl->cmd);
                        }
                    }
                    else
                    {
                        if (timeoutCount >= pRecvCtrl->maxTryCnt)
                        {
                            if (pRecvCtrl->cmd == IOT_GWE_CMD_ORDER_TW_UPDATE_RSP)
                            {/* 交易记录上报: 重试3次(10s)仍无应答, 强制置为上报成功并清在报上下文, 不重连 */
                                Common_ClearRptCount(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
                                Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                                Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                                if (pIotGWECtx->reportingRecordTime > 0)
                                {
                                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGWECtx->reportingRecordTime);
                                }
                                pIotGWECtx->orderReportAwaitTick = 0;
                                pIotGWECtx->reportingPreTradeNo[0] = '\0';
                                pIotGWECtx->reportUseRuntime[port] = FALSE;
                                IOTGWE_CFG_DebugPrint("[GWE] 交易记录上报失败, 强行置为成功!\r\n");
                            }
                            else if (pRecvCtrl->cmd == IOT_GWE_CMD_STOP_CHA_RES_RSP)
                            {/* 停止充电结果重发超时放弃: 清结果码, 停止重发, 不断连(由订单上报兜底) */
                                Common_ClearRptCount(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
                                Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                                Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                                pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.stopResultCode = 0;
                            }
                            else
                            {
                                IotGWE_OfflineHandle();
                            }
                        }
                        else
                        {
                            IotGWE_RetrySendNow(port, pRecvCtrl);
                            IotGWE_CmdTimeoutHandle(port, pRecvCtrl->cmd);
                        }
                    }
                }
            }
        }
    }
}

/**
 ******************************************************************************
 * @brief  接收数据处理 - 从FrameQueue取数据并分发到DecodeData
 ******************************************************************************
 */
void IotGWE_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotGWECtx->frameQueueChannelID, IotGWE_DecodeData);
}
