/******************************************************************************
* File Name          : Asw_IotProtoGWESend.c
* Description        : 国网e充电协议发送模块
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
#include "Asw_IotProtoGWESend.h"
#include "FrameQueue.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "Version.h"
#include "Asw_Monitor.h"
#include "Asw_ChargeIf.h"
#include "Asw_ErrorHandle.h"
#include "Asw_ErrorHandleConfig.h"
#include "Cdd_CP.h"
#include "SS_Ucm.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define GWE_SHARED_STRBUF_SIZE  (96 * 12)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)               sizeof(x) / sizeof(x[0])
#endif

#define JSON_APPEND(...)                                                \
    do                                                                  \
    {                                                                   \
        if (pos < remain)                                               \
        {                                                               \
            int ret = snprintf(pOut + pos, remain - pos, __VA_ARGS__);  \
            if (ret < 0)                                                \
            {                                                           \
                return 0;                                               \
            }                                                           \
                                                                        \
            if ((uint16_t)ret >= (remain - pos))                        \
            {                                                           \
                pos = remain;                                           \
            }                                                           \
            else                                                        \
            {                                                           \
                pos += (uint16_t)ret;                                   \
            }                                                           \
        }                                                               \
    } while (0)

/*******************************************************************************
*    Enum Definition
*******************************************************************************/


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct { 

    uint32_t cursor; 
    uint32_t min; 
    uint8_t mode; 

}meter_query_t;

/* OTA结果->协议step/desc映射 */
typedef struct {
    int8_t      step;
    const char *desc;
} IotGWEOtaStepDesc_Struct;

typedef struct
{
    AswErrorType_Enum errorType;
    uint16_t          gweCode;
    uint8_t           isFault;    /* TRUE=故障(Level4/5/6), FALSE=告警(Level1/2/3) */
    uint8_t           lastStatus[SYSCFG_CFG_GUN_NUM];
} IotGWEFaultCodeMap_Struct;
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t IotGWE_SendFirmwareInfo(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendVerInfo(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendDevMduInfo(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendAskDevConfig(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendAskFeeModel(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendTimeSyncResult(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendStartChaRes(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendStartChargeAuth(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendStopChaRes(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendOrderTwUpdate(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendTotalFault(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendAcStCh(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendAcCarConCh(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendLogQueryResult(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendLogQueryMeterRecord(uint8_t port, void *pBuf, cJSON *cRoot, cJSON *cParams);
static uint16_t IotGWE_SendDevMaintainRet(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendPropertyAcPile(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendPropertyAcWork(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendPropertyAcNonWork(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendPropertyAcOutMeter(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvConfUpdateReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvGetDevConfReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvFunConfUpdateReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvGetFunConfReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvFeeModelUpdateReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvFeeModelQueryReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvStartChargeReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvStopChargeReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvOrderCheckReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvQueDataReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvDevMaintainCtrlReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvDevMaintainQueryReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvTradeRecordAskReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvMeterRecordAskReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvTimeSyncReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvAcOrderlyChargeReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendSrvRsvChargeReply(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendOtaProgress(uint8_t port, void *pBuf);
static uint16_t IotGWE_SendFwInfoReport(uint8_t port, void *pBuf);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotGWECtx_Struct *pIotGWECtx;

static IotGWESendCtrl_Struct c_stIotGWESendctrlTable[IOT_GWE_CMD_SEND_COUNT] =
{
    /* ---- 固件版本上报 (IOT_GWE_PUB_FWINFO), 放最前面保证上线先于其他事件上报 ---- */
    [0]  = { .topic = IOT_GWE_PUB_FWINFO, .identifier = NULL, .cmd = IOT_GWE_CMD_FWINFO_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendFwInfoReport, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "固件版本上报" },

    /* ---- 事件 (IOT_GWE_PUB_EVENT) ---- */
    [1]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "firmwareTwEvt",    .cmd = IOT_GWE_CMD_FIRMWARE_INFO_REQ,    .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendFirmwareInfo,    .matchCmd = IOT_GWE_CMD_FIRMWARE_INFO_RSP,     .cMeaning = "固件信息上报" },
    [2]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "verInfoEvt",       .cmd = IOT_GWE_CMD_VER_INFO_REQ,         .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendVerInfo,         .matchCmd = IOT_GWE_CMD_VER_INFO_RSP,          .cMeaning = "版本信息上报" },
    [3]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "devMduInfoEvt",    .cmd = IOT_GWE_CMD_DEVMDU_INFO_REQ,      .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendDevMduInfo,      .matchCmd = IOT_GWE_CMD_DEVMDU_INFO_RSP,       .cMeaning = "模组信息上报" },
    [4]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "askConfigEvt",     .cmd = IOT_GWE_CMD_ASK_DEV_CONFIG_REQ,   .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendAskDevConfig,    .matchCmd = IOT_GWE_CMD_DEV_CONFIG_RSP,        .cMeaning = "请求设备配置" },
    [5]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "askFeeModelTwEvt", .cmd = IOT_GWE_CMD_ASK_FEEMODEL_REQ,     .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendAskFeeModel,     .matchCmd = IOT_GWE_CMD_FEEMODEL_RSP,          .cMeaning = "请求计费模型" },
    [6]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "timeSyncRetEvt",   .cmd = IOT_GWE_CMD_TIME_SYNC_RESULT_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendTimeSyncResult,  .matchCmd = IOT_GWE_CMD_TIME_SYNC_RSP,         .cMeaning = "时钟同步结果" },

    [7]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "startChaResEvt",   .cmd = IOT_GWE_CMD_START_CHA_RES_REQ,    .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendStartChaRes,     .matchCmd = IOT_GWE_CMD_START_CHA_RES_RSP,     .cMeaning = "启动充电结果" },
    [8]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "startChargeAuthEvt",.cmd= IOT_GWE_CMD_START_CHARGE_AUTH_REQ,.cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendStartChargeAuth, .matchCmd = IOT_GWE_CMD_START_CHARGE_AUTH_RSP, .cMeaning = "启动充电鉴权" },
    [9]  = { .topic = IOT_GWE_PUB_EVENT, .identifier = "stopChaResEvt",    .cmd = IOT_GWE_CMD_STOP_CHA_RES_REQ,     .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendStopChaRes,      .matchCmd = IOT_GWE_CMD_STOP_CHA_RES_RSP,      .cMeaning = "停止充电结果" },
    [10] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "orderTwUpdateEvt", .cmd = IOT_GWE_CMD_ORDER_TW_UPDATE_REQ,  .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendOrderTwUpdate,   .matchCmd = IOT_GWE_CMD_ORDER_TW_UPDATE_RSP,   .cMeaning = "交易记录上报" },

    [11] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "totalFaultEvt",    .cmd = IOT_GWE_CMD_TOTAL_FAULT_REQ,      .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendTotalFault,      .matchCmd = IOT_GWE_CMD_TOTAL_FAULT_RSP,       .cMeaning = "故障告警" },
    [12] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "acStChEvt",        .cmd = IOT_GWE_CMD_AC_ST_CH_REQ,         .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendAcStCh,          .matchCmd = IOT_GWE_CMD_AC_ST_CH_RSP,          .cMeaning = "交流桩状态变化" },
    [13] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "acCarConChEvt",    .cmd = IOT_GWE_CMD_AC_CAR_CON_CH_REQ,    .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendAcCarConCh,      .matchCmd = IOT_GWE_CMD_AC_CAR_CON_CH_RSP,     .cMeaning = "CP连接状态变化" },
    [14] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "logQueryEvt",      .cmd = IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendLogQueryResult,  .matchCmd = IOT_GWE_CMD_LOG_QUERY_RESULT_RSP,  .cMeaning = "日志查询结果" },
    [15] = { .topic = IOT_GWE_PUB_EVENT, .identifier = "devMaintainRetEvt",.cmd = IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendDevMaintainRet,  .matchCmd = IOT_GWE_CMD_DEV_MAINTAIN_RET_RSP,  .cMeaning = "设备维护结果" },

    /* ---- 属性 (IOT_GWE_PUB_PROPERTY_POST: /sys/{pk}/{dn}/thing/event/property/post, 无identifier) ---- */
    [16] = { .topic = IOT_GWE_PUB_PROPERTY_POST, .identifier = NULL, .cmd = IOT_GWE_CMD_PROPERTY_ACPILE_REQ,     .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendPropertyAcPile,     .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "交流设备属性" },
    [17] = { .topic = IOT_GWE_PUB_PROPERTY_POST, .identifier = NULL, .cmd = IOT_GWE_CMD_PROPERTY_AC_WORK_REQ,    .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendPropertyAcWork,     .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "交流充电实时属性" },
    [18] = { .topic = IOT_GWE_PUB_PROPERTY_POST, .identifier = NULL, .cmd = IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendPropertyAcNonWork,  .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "交流非充电实时属性" },
    [19] = { .topic = IOT_GWE_PUB_PROPERTY_POST, .identifier = NULL, .cmd = IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ,.cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendPropertyAcOutMeter, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "交流输出电表属性" },

    /* ---- 服务应答 (IOT_GWE_PUB_SERVICE_REPLY: /sys/{pk}/{dn}/thing/service/{identifier}_reply) ---- */
    [20] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "confUpdateCtrlSrv",    .cmd = IOT_GWE_CMD_CONF_UPDATE_SRV_REPLY,        .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvConfUpdateReply,      .matchCmd = IOT_GWE_SRV_CONF_UPDATE,        .cMeaning = "配置更新应答" },
    [21] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "getDevConfSrv",        .cmd = IOT_GWE_CMD_CONF_GET_SRV_REPLY,           .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvGetDevConfReply,      .matchCmd = IOT_GWE_SRV_CONF_GET,           .cMeaning = "配置获取应答" },
    [22] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "funConfUpdateDataSrv", .cmd = IOT_GWE_CMD_FUN_CONF_UPDATE_SRV_REPLY,    .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvFunConfUpdateReply,   .matchCmd = IOT_GWE_SRV_FUN_CONF_UPDATE,    .cMeaning = "功能配置更新应答" },
    [23] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "getFunConfSrv",        .cmd = IOT_GWE_CMD_FUN_CONF_GET_SRV_REPLY,       .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvGetFunConfReply,      .matchCmd = IOT_GWE_SRV_FUN_CONF_GET,       .cMeaning = "功能配置获取应答" },
    [24] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "feeModelTwUpdateSrv",  .cmd = IOT_GWE_CMD_FEE_MODEL_UPDATE_SRV_REPLY,   .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvFeeModelUpdateReply,  .matchCmd = IOT_GWE_SRV_FEE_MODEL_UPDATE,   .cMeaning = "计费模型更新应答" },
    [25] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "feeModelTwQuerySrv",   .cmd = IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY,    .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvFeeModelQueryReply,   .matchCmd = IOT_GWE_SRV_FEE_MODEL_QUERY,    .cMeaning = "计费模型查询应答" },
    [26] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "startChargeSrv",       .cmd = IOT_GWE_CMD_START_CHARGE_SRV_REPLY,       .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvStartChargeReply,     .matchCmd = IOT_GWE_SRV_START_CHARGE,       .cMeaning = "远程启动充电应答" },
    [27] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "stopChargeSrv",        .cmd = IOT_GWE_CMD_STOP_CHARGE_SRV_REPLY,        .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvStopChargeReply,      .matchCmd = IOT_GWE_SRV_STOP_CHARGE,        .cMeaning = "远程停止充电应答" },
    [28] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "orderCheckSrv",        .cmd = IOT_GWE_CMD_ORDER_CHECK_SRV_REPLY,        .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvOrderCheckReply,      .matchCmd = IOT_GWE_SRV_ORDER_CHECK,        .cMeaning = "交易记录确认应答" },
    [29] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "queDataSrv",           .cmd = IOT_GWE_CMD_QUE_DATA_SRV_REPLY,           .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvQueDataReply,         .matchCmd = IOT_GWE_SRV_QUE_DATA,           .cMeaning = "日志查询应答" },
    [30] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "devMaintainCtrlSrv",   .cmd = IOT_GWE_CMD_DEV_MAINTAIN_CTRL_SRV_REPLY,  .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvDevMaintainCtrlReply, .matchCmd = IOT_GWE_SRV_DEV_MAINTAIN_CTRL,  .cMeaning = "设备维护应答" },
    [31] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "devMaintainQuerySrv",  .cmd = IOT_GWE_CMD_DEV_MAINTAIN_QUERY_SRV_REPLY, .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvDevMaintainQueryReply,.matchCmd = IOT_GWE_SRV_DEV_MAINTAIN_QUERY, .cMeaning = "维护状态查询应答" },
    [32] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "tradeRecordAskSrv",    .cmd = IOT_GWE_CMD_TRADE_RECORD_ASK_SRV_REPLY,   .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvTradeRecordAskReply,  .matchCmd = IOT_GWE_SRV_TRADE_RECORD_ASK,   .cMeaning = "交易记录召测应答" },
    [33] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "meterRecordAskSrv",    .cmd = IOT_GWE_CMD_METER_RECORD_ASK_SRV_REPLY,   .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvMeterRecordAskReply,  .matchCmd = IOT_GWE_SRV_METER_RECORD_ASK,   .cMeaning = "电表底值召测应答" },
    [34] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "timeSyncSrv",          .cmd = IOT_GWE_CMD_TIME_SYNC_SRV_REPLY,          .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvTimeSyncReply,        .matchCmd = IOT_GWE_SRV_TIME_SYNC,          .cMeaning = "时间同步应答" },
    [35] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "acOrderlyChargeSrv",   .cmd = IOT_GWE_CMD_AC_ORDERLY_CHARGE_SRV_REPLY,  .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvAcOrderlyChargeReply, .matchCmd = IOT_GWE_SRV_AC_ORDERLY_CHARGE,  .cMeaning = "有序充电应答" },
    [36] = { .topic = IOT_GWE_PUB_SERVICE_REPLY, .identifier = "rsvChargeSrv",         .cmd = IOT_GWE_CMD_RSV_CHARGE_SRV_REPLY,         .cmdType = IOT_GWE_CMDTYPE_RESPONSE, .sendCycle = 0, .pSendFunc = IotGWE_SendSrvRsvChargeReply,       .matchCmd = IOT_GWE_SRV_RSV_CHARGE,         .cMeaning = "预约充电应答" },

    /* ---- OTA进度 (IOT_GWE_PUB_OTA_PROGRESS) ---- */
    [37] = { .topic = IOT_GWE_PUB_OTA_PROGRESS, .identifier = NULL, .cmd = IOT_GWE_CMD_OTA_PROGRESS_REQ, .cmdType = IOT_GWE_CMDTYPE_REQUSET, .sendCycle = 0, .pSendFunc = IotGWE_SendOtaProgress, .matchCmd = IOT_GWE_CMD_NULL, .cMeaning = "OTA进度上报" },
};


/* OTA结果协议step/desc映射表: step[1,100]=进度%, 负值=失败原因 */
static const IotGWEOtaStepDesc_Struct c_stIotGWEOtaStepDescTable[] =
{
    [eSSUcmResult_Succ]               = {  100, "success"              },
    [eSSUcmResult_GetFileErr]         = {   -2, "download failed"      },
    [eSSUcmResult_HeadErr]            = {   -3, "verification failed"  },
    [eSSUcmResult_DataRecvInterrupt]  = {   -2, "download failed"      },
    [eSSUcmResult_Timeout]            = {   -1, "upgrade failed"       },
    [eSSUcmResult_ModuleNoEnoughSpace]= {   -1, "upgrade failed"       },
    [eSSUcmResult_UnexpectedError]    = {   -1, "upgrade failed"       },
};

/* GWE协议故障码映射 */
static IotGWEFaultCodeMap_Struct c_stIotGWEFaultCodeMap[] =
{
    {eErr_none,               eIotGWEFaultCode_None,             FALSE},  /* 无故障 */
    {eErr_CpVoltAbnor,        eIotGWEFaultCode_CPVol,            TRUE },  /* CP回路电压超限 */
    {eErr_CpGroundFault,      eIotGWEFaultCode_CPVol,            TRUE },  /* CP回路电压超限 */
    {eErr_PEBreakFault,       eIotGWEFaultCode_PE,               TRUE },  /* 地线故障 */
    {eErr_InputLineReversed,  eIotGWEFaultCode_LNReverseConn,    TRUE },  /* 火零反接 */
    {eErr_LeakageCurrErr,     eIotGWEFaultCode_RCD,              TRUE },  /* 漏电流故障 */
    {eErr_ShortCircleErr,     eIotGWEFaultCode_OutShort,         TRUE },  /* 输出短路 */
    {eErr_RCDSelfcheckErr,    eIotGWEFaultCode_RCD,              TRUE },  /* RCD自检故障 */
    {eErr_AphaseInputOverVol, eIotGWEFaultCode_ACInputVolOver,   TRUE },  /* 输入过压 */
    {eErr_AphaseInputLessVol, eIotGWEFaultCode_ACInputVolUnder,  TRUE },  /* 输入欠压 */
    {eErr_OutputOverCurr,     eIotGWEFaultCode_OutCurOver,       TRUE },  /* 输出过流 */
    {eErr_JcqMaloperation,    eIotGWEFaultCode_Contactor,        TRUE },  /* 继电器误动拒动 */
    {eErr_JcqSynechiaFault,   eIotGWEFaultCode_ContactorStick,   TRUE },  /* 继电器粘连 */
    {eErr_DiodeStop,          eIotGWEFaultCode_CtrlPowerOff,     FALSE},  /* 二极管(告警), 协议无对应码暂占位 */
    {eErr_HmiCommErr,         eIotGWEFaultCode_LCDComm,          TRUE },  /* 屏幕通信故障 */
    {eErr_ReaderCommErr,      eIotGWEFaultCode_CardReader,       FALSE},  /* 读卡器故障(告警) */
    {eErr_MeterCommErr,       eIotGWEFaultCode_AmMeterComm,      TRUE },  /* 电表通讯超时 */
    {eErr_MeterCalcErr,       eIotGWEFaultCode_AmMeterData,      TRUE },  /* 电表数据异常 */
    {eErr_EnvOverTempErr,     eIotGWEFaultCode_DevTempOver,      TRUE },  /* 环境过温→设备过温 */
    {eErr_GunOverTempErr,     eIotGWEFaultCode_GunTempOver,      TRUE },  /* 枪过温 */
    {eErr_POverTempErr,       eIotGWEFaultCode_PowerTempOver,    TRUE },  /* 插头过温→电源过温 */
    {eErr_DatabaseErr,        eIotGWEFaultCode_OrderStore,       TRUE },  /* 数据库存储错误 */
    {eErr_EmergencyStop,      eIotGWEFaultCode_EmergenBtn,       TRUE },  /* 急停 */
    {eErr_ChgStartTimeout,    eIotGWEStartFailCode_StartTimeout, FALSE},  /* 启动超时 */
    {eErr_GunDisConn,         eIotGWEStopResultCode_GunDisconn,  FALSE},  /* 拔枪停止(告警) */
    {eErr_CPBreakOff,         eIotGWEStopResultCode_GunDisconn,  FALSE},  /* 拔枪停止(告警) */
};

/* GWE协议停止码映射 (Part4 附录E) */
static const IotGWEFaultCodeMap_Struct c_stIotGWEStopReasonMap[] =
{
    {eSrc_AppStop,          eIotGWEStopResultCode_AppStop         },  /* 远程停止 */
    {eSrc_InsuffBalance,    eIotGWEStopResultCode_AmountBalance   },  /* 余额不足 */
    {eSrc_StopbyMoney,      eIotGWEStopResultCode_MenoyReach      },  /* 按金额停止 */
    {eSrc_StopbyTime,       eIotGWEStopResultCode_TimeReach       },  /* 按时间停止 */
    {eSrc_StopbyEnergy,     eIotGWEStopResultCode_ElecReach       },  /* 按电量停止 */
    {eSrc_MannulStop,       eIotGWEStopResultCode_Offline         },  /* 离线停机 */
    {eSrc_CardStop,         eIotGWEStopResultCode_LocalStop       },  /* 本地停止 */
    {eSrc_S2BreakOff,       eIotGWEStopResultCode_ChagreFull      },  /* 充满停止 */
    {eSrc_LittleCurr,       eIotGWEStopResultCode_Trickle         },  /* 小电流停止 */
};

/* GWE协议启动类型映射 */
static const uint8_t c_stIotGWEStartTypeMap[] =
{
    [ASWMONITOR_ORDER_START_SRC_NULL]  = eIotGWEStartType_APP,
    [ASWMONITOR_ORDER_START_SRC_PNC]   = eIotGWEStartType_VIN,
    [ASWMONITOR_ORDER_START_SRC_CARD]  = eIotGWEStartType_CARD,
    [ASWMONITOR_ORDER_START_SRC_APP]   = eIotGWEStartType_APP,
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
/**
 ******************************************************************************
 * @brief  JSON辅助: 创建根对象 {id,version,method} + params空对象
 ******************************************************************************
 */
static cJSON* IotGWE_CreatePayloadRoot(const char *pMethod, cJSON **ppParams)
{
    char idStr[12] = { 0 };
    cJSON *cRoot = NULL;

    if (ppParams == NULL)
    {
        return NULL;
    }

    cRoot = cJSON_CreateObject();
    if (cRoot == NULL) 
    {   
        return NULL;
    }

    snprintf(idStr, sizeof(idStr), "%u", pIotGWECtx->curMsgId);
    cJSON_AddStringToObject(cRoot, "id", idStr);
    cJSON_AddStringToObject(cRoot, "version", "1.0");
    cJSON_AddStringToObject(cRoot, "method", pMethod);

    *ppParams = cJSON_AddObjectToObject(cRoot, "params");
    if (*ppParams == NULL)
    {
        cJSON_Delete(cRoot);
        return NULL;
    }

    return cRoot;
}
/**
 ******************************************************************************
 * @brief  JSON辅助: 事件上报, method = thing.event.{eventId}.post
 ******************************************************************************
 */
static cJSON* IotGWE_CreateEventRoot(const char *eventId, cJSON **ppParams)
{
    char method[64];

    snprintf(method, sizeof(method), "thing.event.%s.post", eventId);

    return IotGWE_CreatePayloadRoot(method, ppParams);
}
/**
 ******************************************************************************
 * @brief  JSON辅助: 属性上报, 构建 {id,version,method="thing.event.property.post",params} + params
 ******************************************************************************
 */
static cJSON* IotGWE_CreatePropertyRoot(const char *propName, cJSON **ppBody)
{
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreatePayloadRoot("thing.event.property.post", &cParams);
    if (cRoot == NULL) return NULL;

    *ppBody = cJSON_AddObjectToObject(cParams, propName);
    if (*ppBody == NULL)
    {
        cJSON_Delete(cRoot);
        return NULL;
    }

    return cRoot;
}
/**
 ******************************************************************************
 * @brief  JSON辅助: 序列化并拷贝到pBuf, 返回长度
 ******************************************************************************
 */
static uint16_t IotGWE_SerializeJson(cJSON *cRoot, void *pBuf)
{
    char *pJsonStr = NULL;
    uint16_t len = 0;

    pJsonStr = cJSON_PrintUnformatted(cRoot);
    if (pJsonStr != NULL)
    {
        len = (uint16_t)strlen(pJsonStr);
        if (len < IOT_GWE_TXRX_BUFFER_SIZE)
        {
            memcpy(pBuf, pJsonStr, len);
        }
        else
        {
            len = 0;
        }
        cJSON_free(pJsonStr);
    }

    cJSON_Delete(cRoot);

    return len;
}
/**
 ******************************************************************************
 * @brief  JSON辅助: 服务应答根对象 {id,code,data}
 ******************************************************************************
 */
static cJSON* IotGWE_CreateServiceReplyRoot(cJSON **ppData)
{
    char idStr[12] = {0};
    cJSON *cRoot = NULL;

    do {

        if (ppData == NULL)
        {
            break;
        }

        cRoot = cJSON_CreateObject();
        if (cRoot == NULL)
        {
            break;
        }

        snprintf(idStr, sizeof(idStr), "%u", pIotGWECtx->curMsgId);
        cJSON_AddStringToObject(cRoot, "id", idStr);
        cJSON_AddNumberToObject(cRoot, "code", 200);

        *ppData = cJSON_AddObjectToObject(cRoot, "data");
        if (*ppData == NULL)
        {
            cJSON_Delete(cRoot);
            break;
        }

    } while (0);

    return cRoot;
}
/**
 ******************************************************************************
 * @brief  故障变位检测: 检查映射表中各故障状态是否变化
 * @return TRUE=有故障变位, FALSE=无变化
 ******************************************************************************
 */
uint8_t IotGWE_CheckErrStatus(void)
{
    IotGWEFaultCodeMap_Struct *pMap = NULL;
    uint8_t index = 0, gunNo = 0, status = 0;
    uint8_t changed = FALSE;

    for (index = 0; index < ARRAY_SIZE(c_stIotGWEFaultCodeMap); index++)
    {
        pMap = &c_stIotGWEFaultCodeMap[index];
        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            status = AswErrHandle_CheckErrExit(gunNo, pMap->errorType);
            if (status != pMap->lastStatus[gunNo])
            {
                pMap->lastStatus[gunNo] = status;
                changed = TRUE;
            }
        }
    }

    return changed;
}

/**
 ******************************************************************************
 * @brief  重置故障状态 (下线/重连时调用)
 ******************************************************************************
 */
void IotGWE_ResetErrStatus(void)
{
    uint8_t index = 0, gunNo = 0;

    for (index = 0; index < ARRAY_SIZE(c_stIotGWEFaultCodeMap); index++)
    {
        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            c_stIotGWEFaultCodeMap[index].lastStatus[gunNo] = 0;
        }
    }
}
/**
 ******************************************************************************
 * @brief  故障码映射
 ******************************************************************************
 */
static uint16_t IotGWE_MapFaultCode(AswErrorType_Enum eErrType)
{
    uint8_t i;
    uint16_t code = eIotGWEFaultCode_CassBCC;

    for (i = 0; i < ARRAY_SIZE(c_stIotGWEFaultCodeMap); i++)
    {
        if (c_stIotGWEFaultCodeMap[i].errorType == eErrType)
        {
            code = c_stIotGWEFaultCodeMap[i].gweCode;
            break;
        }
    }

    if (code == eIotGWEFaultCode_CassBCC)
    {
        IOTGWE_CFG_DebugPrint("[%s] eErrType = %d\r\n", __FUNCTION__, eErrType);
    }
    return code;
}
/**
 ******************************************************************************
 * @brief  停止码映射
 ******************************************************************************
 */
uint16_t IotGWE_MapStopReason(AswErrorType_Enum eStopReason)
{
    uint8_t i;
    uint16_t reason = eIotGWEStopResultCode_ChagreFull;/* 默认: 充满停止 */
    uint8_t matched = FALSE;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[0].offlineClearData;

    do {

        if (eStopReason == IOTGWE_STOPREASON_POWEROFF_MARK)
        {/* 充电中断电 */
            reason = eIotGWEFaultCode_InputPower;
            break;
        }

        if (eStopReason == eErr_none)
        {
            reason = 0;
            break;
        }

        /* 优先用故障码表(覆盖所有 eErr_* 类型) */
        reason = IotGWE_MapFaultCode(eStopReason);
        if (reason != eIotGWEFaultCode_CassBCC)
        {
            break;
        }

        /* 非故障停止原因(eSrc_*), 查停止码表 */
        for (i = 0; i < ARRAY_SIZE(c_stIotGWEStopReasonMap); i++)
        {
            if (c_stIotGWEStopReasonMap[i].errorType == eStopReason)
            {
                reason = c_stIotGWEStopReasonMap[i].gweCode;
                matched = TRUE;
                break;
            }
        }

        if (matched == FALSE)
        {/* 未匹配到, 以启动失败码为停止理由 */
            reason = pOfflineClr->startFaultCode;
        }

    } while (0);

    return reason;
}

/**
 ******************************************************************************
 * @brief  启动方式转换
 ******************************************************************************
 */
static uint8_t IotGWE_MapStartType(uint8_t startSrc)
{
    uint8_t type = eIotGWEStartType_APP;

    if (startSrc < ARRAY_SIZE(c_stIotGWEStartTypeMap))
    {
        type =  c_stIotGWEStartTypeMap[startSrc];
    }

    return type;
}
/**
 ******************************************************************************
 * @brief  工作状态映射
 ******************************************************************************
 */
static uint8_t IotGWE_MapWorkStatus(uint8_t port, uint8_t chargeState, uint8_t gunConnected)
{
    IotGWEWorkStatus_Enum eWorkStstus = eIotGWEWorkStatus_IDIE;
    uint8_t i;

    /* 故障优先 */
    for (i = 0; i < ARRAY_SIZE(c_stIotGWEFaultCodeMap); i++)
    {
        if (c_stIotGWEFaultCodeMap[i].isFault == TRUE &&
            AswErrHandle_CheckErrExit(port, c_stIotGWEFaultCodeMap[i].errorType) == TRUE)
        {
            eWorkStstus = eIotGWEWorkStatus_FAULT;
            break;
        }
    }

    if (eWorkStstus != eIotGWEWorkStatus_FAULT)
    {/* 设备维护模式: 冻结/停运/退运(forbidState) 或 检修(可充电但故障态) */
        uint8_t devCtrlType = pIotGWECtx->stProtoData.stRecvData[0].offlineClearData.devMaintainCtrlType;
        if (AswMonitor_CheckForbidState() || devCtrlType == eIotGWECtrlType_Maintenance)
        {
            eWorkStstus = eIotGWEWorkStatus_FAULT;
        }
    }

    if (eWorkStstus != eIotGWEWorkStatus_FAULT)
    {
        switch(chargeState)
        {
            case ASWCHARGEIF_WORKSTATE_STARTING:
            case ASWCHARGEIF_WORKSTATE_WAKEUP:
                eWorkStstus = eIotGWEWorkStatus_STARTING;
                break;
            case ASWCHARGEIF_WORKSTATE_FINISH:
            case ASWCHARGEIF_WORKSTATE_STOPPING:
                eWorkStstus = eIotGWEWorkStatus_FINISH;
                break;
            case ASWCHARGEIF_WORKSTATE_CHARGING:
            case ASWCHARGEIF_WORKSTATE_PAUSEA:
            case ASWCHARGEIF_WORKSTATE_PAUSEB:
                eWorkStstus = eIotGWEWorkStatus_CHARGING;
                break;
            default:
                if (gunConnected == TRUE)
                {
                    eWorkStstus = eIotGWEWorkStatus_READY;
                }
                break;
        }
    }

    return eWorkStstus;
}
/**
 ******************************************************************************
 * @brief  从故障记录userData文本中提取首条记录的时间戳([yyyy-MM-dd HH:mm:ss])
 ******************************************************************************
 */
uint32_t IotGWE_ExtractFaultTimestamp(MSNvmErrorInfo_Struct *pRec)
{
    uint32_t ret = 0;
    unsigned int y = 0, m = 0, d = 0, h = 0, min = 0, sec = 0;
    int matched;
    matched = sscanf((const char *)pRec->userData, "[%4u-%2u-%2u %2u:%2u:%2u]", &y, &m, &d, &h, &min, &sec);
    if (matched == 6)
    {
        CommonDateTime_Struct dt;
        memset(&dt, 0, sizeof(dt));
        dt.year = (uint16_t)y;
        dt.month = (uint8_t)m;
        dt.day = (uint8_t)d;
        dt.hour = (uint8_t)h;
        dt.minute = (uint8_t)min;
        dt.second = (uint8_t)sec;

        ret = Common_DateTimeToTimestamp(&dt);
    }
    else
    {
        IOTGWE_CFG_DebugPrint("[GWE] ExtractFaultTs fail: matched=%d, head=%.40s\r\n",
                              matched, (const char *)pRec->userData);
    }

    return ret;
}
/**
 ******************************************************************************
 * @brief  截掉故障记录中的快照数据(PackErrStr追加的部分, 以", 电压:"起始)
 ******************************************************************************
 */
static void IotGWE_TrimFaultSnapshot(char *pStr)
{
    char *pSnapshot = strstr(pStr, ", 电压:");
    if (pSnapshot != NULL)
    {
        pSnapshot[0] = '\0';
    }
}
/**
 ******************************************************************************
 * @brief  QueryRecordByExternal 比较函数: 寻找小于cursorTime的第一条故障记录
 ******************************************************************************
 */
static uint8_t IotGWE_CmpPrevFaultRecord(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    MSNvmErrorInfo_Struct *pRec = (MSNvmErrorInfo_Struct *)pRecord;
    meter_query_t *pCmp = (void *)pPara;
    uint32_t ts = 0;
    uint8_t ret = TRUE;

    (void)paraSize;

    ts = IotGWE_ExtractFaultTimestamp(pRec);
    if (ts == 0 || ts >= pCmp->cursor || ts < pCmp->min)
    {
        ret = FALSE;
    }

    return ret;
}
/**
 ******************************************************************************
 * @brief  QueryRecordByExternal 比较函数: 精确匹配故障记录的时间戳
 ******************************************************************************
 */
static uint8_t IotGWE_CmpExactFaultTimestamp(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    MSNvmErrorInfo_Struct *pRec = (MSNvmErrorInfo_Struct *)pRecord;
    uint32_t targetTime = *(uint32_t *)pPara;
    (void)paraSize;

    return (IotGWE_ExtractFaultTimestamp(pRec) == targetTime) ? TRUE : FALSE;
}
/**
 ******************************************************************************
 * @brief  从订单记录中提取时间戳(startTime)
 ******************************************************************************
 */
uint32_t IotGWE_ExtractOrderTimestamp(MSNvmOrderInfo_Struct *pRec)
{
    uint32_t time = 0;
    if (pRec->protocolType == eAswPlatType_GWE)
    {
        time = pRec->platOrderInfo.stGWEOrderInfo.startTime;
    }

    return time;
}
/**
 ******************************************************************************
 * @brief  订单精确时间戳匹配
 ******************************************************************************
 */
static uint8_t IotGWE_CmpExactOrderTimestamp(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    uint32_t targetTime = *(uint32_t *)pPara;
    (void)paraSize;

    return (IotGWE_ExtractOrderTimestamp((MSNvmOrderInfo_Struct *)pRecord) == targetTime) ? TRUE : FALSE;
}
/**
 ******************************************************************************
 * @brief  寻找下一条交易记录: startTime < cursor && startTime >= min
 ******************************************************************************
 */
static uint8_t IotGWE_CmpPrevOrderRecord(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    MSNvmOrderInfo_Struct *pRec = (MSNvmOrderInfo_Struct *)pRecord;
    meter_query_t *pCmp = (void *)pPara;
    uint32_t ts;
    uint8_t ret = TRUE;

    (void)paraSize;

    do {
        if (pRec->protocolType != eAswPlatType_GWE || pRec->orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
        {
            ret = FALSE;
            break;
        }

        ts = pRec->platOrderInfo.stGWEOrderInfo.startTime;
        if (ts == 0 || ts >= pCmp->cursor || ts < pCmp->min)
        {
            ret = FALSE;
        }

    } while (0);

    return ret;
}
/**
 ******************************************************************************
 * @brief  按 preTradeNo 匹配: para 为目标流水号字符串
 ******************************************************************************
 */
uint8_t IotGWE_CmpOrderByPreTradeNo(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    MSNvmOrderInfo_Struct *pRec = (MSNvmOrderInfo_Struct *)pRecord;
    const char *targetNo = (const char *)pPara;

    (void)paraSize;

    return (strncmp(pRec->platOrderInfo.stGWEOrderInfo.preTradeNo, targetNo, 40) == 0) ? TRUE : FALSE;
}
/**
 ******************************************************************************
 * @brief  按时间范围统计已完成GWE订单数
 ******************************************************************************
 */
uint32_t IotGWE_CountOrdersByTimeRange(uint32_t startTs, uint32_t stopTs)
{
    MSNvmOrderInfo_Struct rec;
    uint32_t count = 0;
    uint32_t idx;
    uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_OrderRecord);

    if (latestIdx)
    {
        for (idx = 1; idx <= latestIdx; idx++)
        {
            if (eGlobalRet_OK != MSNvm_QueryRecordByTime(eMSNvmBlockID_OrderRecord, (uint8_t *)&rec, sizeof(rec), idx))
                continue;

            if (rec.protocolType != eAswPlatType_GWE || rec.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                continue;

            uint32_t ts = rec.platOrderInfo.stGWEOrderInfo.startTime;
            if (ts >= startTs && ts <= stopTs)
                count++;
        }
    }

    return count;
}
/**
 ******************************************************************************
 * @brief  查询电表底值tsdb遍历回调: 查找时间范围内节点的记录
 ******************************************************************************
 */
static uint8_t IotGWE_CmpPrevMeterRecord(uint8_t *pRecord, uint8_t *pPara, uint16_t paraSize)
{
    MSNvmMeterRecord_Struct *pRec = (MSNvmMeterRecord_Struct *)pRecord;
    meter_query_t *pCmp = (void *)pPara;
    uint8_t ret = FALSE;

    (void)paraSize;

    do {
        if (pRec->acqTime >= pCmp->cursor)
        {/* 本条记录采集时间 > 本次要查询的最大时间，继续迭代 */
            break;
        }

        if (pRec->acqTime < pCmp->min)
        {/* 本条记录采集时间 < 本次要查询的最小时间, 继续迭代 */
            break;
        }

        if (pCmp->mode == eIotGWEMeterAskType_Midnight)
        {
            CommonDateTime_Struct dt;
            Common_TimestampToDateTime(pRec->acqTime, &dt);
            if (dt.hour != 0 || dt.minute != 0)
            {/* 本条不是零点记录, 继续迭代 */
                break;
            }
        }

        ret = TRUE;

    } while (0);

    return ret;
}
/**
 ******************************************************************************
 * @brief  [0] 固件版本上报: /ota/device/inform/{pk}/{dn}
 ******************************************************************************
 */
static uint16_t IotGWE_SendFwInfoReport(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char otaVersion[IOTGWE_VERSION_LEN] = {0};
    char idStr[12] = {0};
    uint16_t ret = 0;

    do {
        cRoot = cJSON_CreateObject();
        if (cRoot == NULL)
        {
            break;
        }

        snprintf(idStr, sizeof(idStr), "%u", pIotGWECtx->curMsgId);
        cJSON_AddStringToObject(cRoot, "id", idStr);

        cParams = cJSON_AddObjectToObject(cRoot, "params");
        if (cParams == NULL)
        {
            cJSON_Delete(cRoot);
            cRoot = NULL;
            break;
        }

        IOTGWE_MAKE_SOFT_VERSION(otaVersion, sizeof(otaVersion));
        cJSON_AddStringToObject(cParams, "version", otaVersion);

    } while (0);

    if (cRoot != NULL)
    {
        ret = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret;
}
/**
 ******************************************************************************
 * @brief  [1] 固件信息上报 firmwareTwEvt (Part4 §5.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendFirmwareInfo(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char tmp[48] = {0};

    cRoot = IotGWE_CreateEventRoot("firmwareTwEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    CddNetM_GetIccid((uint8_t *)tmp);
    cJSON_AddStringToObject(cParams, "simNo", tmp);

    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    if (pPrivateParam->stGWEParam.stBillMode.validFlag)
    {
        MSNvmGWEParamBillMode_Struct *pBM = &pPrivateParam->stGWEParam.stBillMode;
        memcpy(tmp, pBM->billModeID, sizeof(pBM->billModeID));
    }
    else
    {
        tmp[0] = '\0';
    }
    cJSON_AddStringToObject(cParams, "feeModelId", tmp);

    cJSON_AddStringToObject(cParams, "stakeModel", SYSCFG_CFG_PRODUCT_CODE);
    cJSON_AddNumberToObject(cParams, "vendorCode", IOTGWE_BULL_VERNDOR);

    snprintf(tmp, sizeof(tmp), "%.40s", AswPlatM_GetPlatParamPtr()->platPileDn);
    cJSON_AddStringToObject(cParams, "devSn", tmp);
    cJSON_AddNumberToObject(cParams, "devType", eIotGWEDevType_SPAC);
    cJSON_AddNumberToObject(cParams, "portNum", SYSCFG_CFG_GUN_NUM);
    cJSON_AddStringToObject(cParams, "simMac", "");
    cJSON_AddNumberToObject(cParams, "longitude", 0);
    cJSON_AddNumberToObject(cParams, "latitude", 0);
    cJSON_AddNumberToObject(cParams, "height", 0);
    cJSON_AddNumberToObject(cParams, "gridType", eIotGWEGridType_None);
    cJSON_AddStringToObject(cParams, "btMac", "");
    cJSON_AddNumberToObject(cParams, "meaType", eIotGWEMeaType_ACMeterIC);
    cJSON_AddNumberToObject(cParams, "otRate", (SYSCFG_CFG_MAX_OUTPUT_POWER / 100));
    cJSON_AddNumberToObject(cParams, "otMinVol", IOT_GWE_OTMINVOL);
    cJSON_AddNumberToObject(cParams, "otMaxVol", IOT_GWE_OTMAXVOL);
    cJSON_AddNumberToObject(cParams, "otCur", (SYSCFG_CFG_MAX_OUTPUT_CURRENT / 100));
    cJSON_AddItemToObject(cParams,   "inMeter", cJSON_CreateArray());
    cJSON_AddItemToObject(cParams,   "outMeter", cJSON_CreateArray());
    cJSON_AddNumberToObject(cParams, "CT", IOT_GWE_CT_RATIO);
    cJSON_AddNumberToObject(cParams, "isGateLock", eIotGWEIsGateLock_NO);
    cJSON_AddNumberToObject(cParams, "isGroundLock", eIotGWEIsGroundLock_NO);
    cJSON_AddNumberToObject(cParams, "mutliChargingMode", eIotGWEMutliChargeMode_OK);  

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [2] 版本信息上报 verInfoEvt (Part4 §5.2.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendVerInfo(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char otaVersion[IOTGWE_VERSION_LEN] = {0};
    char hdVersion[IOTGWE_VERSION_LEN] = {0};

    cRoot = IotGWE_CreateEventRoot("verInfoEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    IOTGWE_MAKE_SOFT_VERSION(otaVersion, sizeof(otaVersion));
    IOTGWE_MAKE_HARD_VERSION(hdVersion, sizeof(hdVersion));

    cJSON_AddNumberToObject(cParams, "devRegMethod", eIotGWEDevRegMethod_NOWR);
    cJSON_AddStringToObject(cParams, "pileSoftwareVer", otaVersion);
    cJSON_AddStringToObject(cParams, "pileHardwareVer", hdVersion);
    cJSON_AddStringToObject(cParams, "sdkVer", IOT_GWE_SDK_VERSION);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [3] 设备组件信息上报 devMduInfoEvt (Part4 §5.3.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendDevMduInfo(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL, *mduInfoInt = NULL, *mduInfoString = NULL;
    char ModuleTypeInfo[32] = {0};

    cRoot = IotGWE_CreateEventRoot("devMduInfoEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    CddNetM_GetModuleTypeInfo(ModuleTypeInfo, sizeof(ModuleTypeInfo));
    cJSON_AddStringToObject(cParams, "netMduInfo", ModuleTypeInfo);
    cJSON_AddStringToObject(cParams, "netMduSoftVer", "");
    cJSON_AddStringToObject(cParams, "netMduImei", "");

    cJSON_AddNumberToObject(cParams, "smartGun", eIotGWEFunciton_NO);

    /* mduInfoInt: 10=具备, 11=不具备 */
    cJSON_AddItemToObject(cParams, "mduInfoInt", mduInfoInt = cJSON_CreateArray());
    cJSON_AddItemToArray(mduInfoInt, cJSON_CreateNumber(eIotGWEFunciton_NO));  /* [0] 蓝牙即插即充 */
    cJSON_AddItemToArray(mduInfoInt, cJSON_CreateNumber(eIotGWEFunciton_NO));  /* [1] VIN即插即充 */
    cJSON_AddItemToArray(mduInfoInt, cJSON_CreateNumber(eIotGWEFunciton_NO));  /* [2] 白名单VIN离线即插即充 */
    cJSON_AddItemToArray(mduInfoInt, cJSON_CreateNumber(eIotGWEFunciton_OK));  /* [3] 扫码功能 */

    cJSON_AddItemToObject(cParams, "mduInfoString", mduInfoString = cJSON_CreateArray());
    for (int i = 0; i < 5; i++)
    {/* CPU序列号, 蓝牙模块固件版本, 蓝牙模块硬件版本, 电表软件版本号, 电表硬件版本号 */
        cJSON_AddItemToArray(mduInfoString, cJSON_CreateString(""));
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [4] 请求设备配置 askConfigEvt (Part4 §5.4.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendAskDevConfig(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreateEventRoot("askConfigEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [5] 请求计费模型 askFeeModelTwEvt (Part4 §6.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendAskFeeModel(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char strBuf[20] = {0};
    uint32_t billModeID = 0;

    cRoot = IotGWE_CreateEventRoot("askFeeModelTwEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    if (pPrivateParam->stGWEParam.stBillMode.validFlag)
    {
        MSNvmGWEParamBillMode_Struct *pBM = &pPrivateParam->stGWEParam.stBillMode;
        memcpy(strBuf, pBM->billModeID, sizeof(pBM->billModeID));
    }
    cJSON_AddStringToObject(cParams, "feeModelId", strBuf);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [6] 时钟同步结果 timeSyncRetEvt (Part4 §5.13.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendTimeSyncResult(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char srvTimeStr[16] = {0};
    char devTimeStr[16] = {0};
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateEventRoot("timeSyncRetEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    snprintf(srvTimeStr, sizeof(srvTimeStr), "%u", pOfflineClr->platTimestamp);
    snprintf(devTimeStr, sizeof(devTimeStr), "%u", (uint32_t)(SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT));
    cJSON_AddStringToObject(cParams, "srvTime", srvTimeStr);
    cJSON_AddStringToObject(cParams, "devTime", devTimeStr);
    cJSON_AddNumberToObject(cParams, "resCode", pOfflineClr->timeSyncResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [7] 启动充电结果 startChaResEvt (Part4 §6.2.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendStartChaRes(uint8_t port, void *pBuf)
{
    MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreateEventRoot("startChaResEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);

    if (pOfflineClr->startFailChargeActive == TRUE)
    {
        pOfflineClr->startFailChargeActive = FALSE;
        cJSON_AddStringToObject(cParams, "preTradeNo", pOfflineClr->startFailPreTradeNo);
        cJSON_AddStringToObject(cParams, "tradeNo", pOfflineClr->startFailTradeNo);
    }
    else if (pOrderData != NULL)
    {
        pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
        cJSON_AddStringToObject(cParams, "preTradeNo", pGWEOrder->preTradeNo);
        cJSON_AddStringToObject(cParams, "tradeNo", pGWEOrder->tradeNo);
    }
    else
    {
        cJSON_AddStringToObject(cParams, "preTradeNo", "");
        cJSON_AddStringToObject(cParams, "tradeNo", "");
    }

    cJSON_AddNumberToObject(cParams, "startResult", pOfflineClr->startResult);
    cJSON_AddNumberToObject(cParams, "faultCode", pOfflineClr->startFaultCode);
    cJSON_AddStringToObject(cParams, "vinCode", "");

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [8] 启动充电鉴权 startChargeAuthEvt (Part4 §6.3.2.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendStartChargeAuth(uint8_t port, void *pBuf)
{
    MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
    AswMonitorChargeCtrl_Struct *pCtrl = AswMonitor_GetChargeCtrlPtr(port);
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreateEventRoot("startChargeAuthEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);

    if (pOrderData != NULL)
    {
        pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
        cJSON_AddStringToObject(cParams, "preTradeNo", pGWEOrder->preTradeNo);
        cJSON_AddStringToObject(cParams, "tradeNo",    pGWEOrder->tradeNo);
    }
    else
    {
        cJSON_AddStringToObject(cParams, "preTradeNo", "");
        cJSON_AddStringToObject(cParams, "tradeNo",    "");
    }

    cJSON_AddNumberToObject(cParams, "startType",
        IotGWE_MapStartType((pCtrl != NULL) ? pCtrl->startSrc : ASWMONITOR_ORDER_START_SRC_APP));

    if (pCtrl != NULL && pCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
        cJSON_AddStringToObject(cParams, "authCode", (char *)pCtrl->authCardID);
    else
        cJSON_AddStringToObject(cParams, "authCode", "");

    cJSON_AddNumberToObject(cParams, "batterySOC",  0);
    cJSON_AddNumberToObject(cParams, "batteryCap",  0);
    cJSON_AddNumberToObject(cParams, "chargeTimes", 0);
    cJSON_AddNumberToObject(cParams, "batteryVol",  0);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [9] 停止充电结果 stopChaResEvt (Part4 §6.4.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendStopChaRes(uint8_t port, void *pBuf)
{
    MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreateEventRoot("stopChaResEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);

    if (pOrderData != NULL)
    {
        pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
        cJSON_AddStringToObject(cParams, "preTradeNo", pGWEOrder->preTradeNo);
        cJSON_AddStringToObject(cParams, "tradeNo", pGWEOrder->tradeNo);
    }
    else
    {
        cJSON_AddStringToObject(cParams, "preTradeNo", "");
        cJSON_AddStringToObject(cParams, "tradeNo", "");
    }

    cJSON_AddNumberToObject(cParams, "stopResult",   pOfflineClr->stopResult);
    cJSON_AddNumberToObject(cParams, "resultCode",   pOfflineClr->stopResultCode);
    cJSON_AddNumberToObject(cParams, "stopFailReson", pOfflineClr->stopFailReson);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [10] 交易记录上报 orderTwUpdateEvt (Part4 §6.5.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendOrderTwUpdate(uint8_t port, void *pBuf)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    static MSNvmGWEOrderInfo_Struct order;
    MSNvmOrderInfo_Struct stNvmOrder;
    uint8_t fromNvm = FALSE;
    uint8_t isStartFail = FALSE;
    uint16_t startFailReason = 0;
    cJSON *cRoot = NULL, *cParams = NULL;
    cJSON *partElectArray = NULL, *chargeFeeArray = NULL, *serviceFeeArray = NULL, *pointsElectArray = NULL;
    char timeStr[16] = { 0 };
    uint8_t i;

    memset(&order, 0, sizeof(order));
    order.gunNo = port + 1;

    /* 1: 数据填充 */
    if (pOfflineClr->startFailOrderActive == TRUE)
    {/* 启动失败 */
        isStartFail = TRUE;
        startFailReason = pOfflineClr->startFaultCode;
        pOfflineClr->startFailOrderActive = FALSE;
        order.gunNo = pOfflineClr->startFailGunNo;
        strncpy(order.preTradeNo, pOfflineClr->startFailPreTradeNo, sizeof(order.preTradeNo) - 1);
        strncpy(order.tradeNo, pOfflineClr->startFailTradeNo, sizeof(order.tradeNo) - 1);
        order.startTime = pOfflineClr->startFailStartTime;
        order.endTime = order.startTime;
        order.startType = pOfflineClr->startFailStartType;
    }
    else if (pIotGWECtx->reportUseRuntime[port] != TRUE &&
             pIotGWECtx->reportingRecordTime > 0 &&
             eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_OrderRecord,
                 (uint8_t *)&stNvmOrder, sizeof(stNvmOrder), pIotGWECtx->reportingRecordTime))
    {/* NVM历史订单 */
        order = stNvmOrder.platOrderInfo.stGWEOrderInfo;
        fromNvm = TRUE;
    }
    else if (pIotGWECtx->reportUseRuntime[port] == TRUE || pIotGWECtx->reportingRecordTime == 0)
    {/* 运行时当前订单 */
        MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
        AswMonitorChargeData_Struct *pData = AswMonitor_GetChargeDataPtr(port);
        AswMonitorChargeCtrl_Struct *pCtrl = AswMonitor_GetChargeCtrlPtr(port);

        if (pOrderData != NULL)
        {
            MSNvmGWEOrderInfo_Struct *pSrc = &pOrderData->platOrderInfo.stGWEOrderInfo;
            strncpy(order.preTradeNo, pSrc->preTradeNo, sizeof(order.preTradeNo) - 1);
            strncpy(order.tradeNo, pSrc->tradeNo, sizeof(order.tradeNo) - 1);
        }

        MSNvmPlatPrivateParam_Union *pPP = AswPlatM_GetPlatPrivateParamPtr();
        MSNvmGWEParamBillMode_Struct *pBM = &pPP->stGWEParam.stBillMode;
        memcpy(order.billModeID, pBM->billModeID, sizeof(pBM->billModeID));
        order.timeNum = pBM->periodCount;

        order.startType = pCtrl ? pCtrl->startSrc : eIotGWEStartType_APP;
        if (pData != NULL)
        {
            order.startTime      = pData->chargeStartTime;
            order.endTime        = pData->chargeStopTime;
            order.stopReason     = (uint8_t)pData->eChargeStopReason;
            order.sumStart       = pData->startMeterVal;
            order.sumEnd         = pData->stopMeterVal;
            order.totalElec      = pData->totalEnergy;
            order.totalPowerCost = pData->totalElecMoney;
            order.totalServCost  = pData->totalServeMoney;
        }

        /* 更新当前订单索引 */
        if (pIotGWECtx->reportUseRuntime[port] == TRUE)
        {
            uint32_t recTime = 0;
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord,
                    (uint8_t *)&stNvmOrder, sizeof(stNvmOrder), &recTime))
            {
                pIotGWECtx->reportingRecordTime = recTime;
            }
        }
    }

    if (fromNvm == TRUE || pIotGWECtx->reportUseRuntime[port] == TRUE)
    {
        strncpy(pIotGWECtx->reportingPreTradeNo, order.preTradeNo, sizeof(pIotGWECtx->reportingPreTradeNo) - 1);
        pIotGWECtx->reportingPreTradeNo[sizeof(pIotGWECtx->reportingPreTradeNo) - 1] = '\0';
        pIotGWECtx->orderReportAwaitTick = Common_GetSystick();
    }

    /* 2: JSON组包 */
    cRoot = IotGWE_CreateEventRoot("orderTwUpdateEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "gunNo", order.gunNo);
    cJSON_AddStringToObject(cParams, "preTradeNo", order.preTradeNo);
    cJSON_AddStringToObject(cParams, "tradeNo", order.tradeNo);
    cJSON_AddStringToObject(cParams, "vinCode", "");
    cJSON_AddNumberToObject(cParams, "timeDivType", eIotGWETimeDivType_MeterAndBill);
    cJSON_AddStringToObject(cParams, "feeModelId", (const char *)order.billModeID);
    cJSON_AddNumberToObject(cParams, "startType", IotGWE_MapStartType(order.startType));

    snprintf(timeStr, sizeof(timeStr), "%u", order.startTime - SSTM_BASE_TIMESTAMP_1970_BJT);
    cJSON_AddStringToObject(cParams, "chargeStartTime", timeStr);
    snprintf(timeStr, sizeof(timeStr), "%u", order.endTime - SSTM_BASE_TIMESTAMP_1970_BJT);
    cJSON_AddStringToObject(cParams, "chargeEndTime", timeStr);
    cJSON_AddNumberToObject(cParams, "startSoc", 0);
    cJSON_AddNumberToObject(cParams, "endSoc", 0);
    if (isStartFail)
    {
        cJSON_AddNumberToObject(cParams, "reason", startFailReason);
    }
    else
    {
        cJSON_AddNumberToObject(cParams, "reason", IotGWE_MapStopReason(order.stopReason));
    }

    snprintf(timeStr, sizeof(timeStr), "%lu", (unsigned long)order.sumStart);
    cJSON_AddStringToObject(cParams, "sumStart", timeStr);
    snprintf(timeStr, sizeof(timeStr), "%lu", (unsigned long)order.sumEnd);
    cJSON_AddStringToObject(cParams, "sumEnd", timeStr);

    cJSON_AddNumberToObject(cParams, "totalElect", order.totalElec);
    cJSON_AddNumberToObject(cParams, "totalPowerCost", order.totalPowerCost);
    cJSON_AddNumberToObject(cParams, "totalServCost", order.totalServCost);
    cJSON_AddNumberToObject(cParams, "totalCost", order.totalPowerCost + order.totalServCost);
    cJSON_AddNumberToObject(cParams, "timeNum", order.timeNum);

    /* 3: 时段/跨越点数组 */
    cJSON_AddItemToObject(cParams, "partElect",  partElectArray  = cJSON_CreateArray());
    cJSON_AddItemToObject(cParams, "chargeFee",  chargeFeeArray  = cJSON_CreateArray());
    cJSON_AddItemToObject(cParams, "serviceFee", serviceFeeArray = cJSON_CreateArray());

    if (fromNvm)
    {
        for (i = 0; i < order.timeNum && i < MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX; i++)
        {
            cJSON_AddItemToArray(partElectArray,  cJSON_CreateNumber(Common_ThreeUint8ToUint32(order.partElect[i])));
            cJSON_AddItemToArray(chargeFeeArray,  cJSON_CreateNumber(Common_ThreeUint8ToUint32(order.chargeFee[i])));
            cJSON_AddItemToArray(serviceFeeArray, cJSON_CreateNumber(Common_ThreeUint8ToUint32(order.serviceFee[i])));
        }
        cJSON_AddNumberToObject(cParams, "startPoint", order.startPoint);
        cJSON_AddNumberToObject(cParams, "crossPoints", order.crossPoints);
        cJSON_AddItemToObject(cParams, "pointsElect", pointsElectArray = cJSON_CreateArray());
        for (i = 0; i < order.crossPoints && i < MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX; i++)
        {
            uint8_t idx = (order.startPoint - 1 + i) % 96;
            cJSON_AddItemToArray(pointsElectArray, cJSON_CreateNumber(Common_TwoUint8ToUint16(order.pointsElect[idx])));
        }
    }
    else
    {/* 运行时/启动失败: 从AswMonitor读取 */
        AswMonitorChargeData_Struct *pData = AswMonitor_GetChargeDataPtr(port);
        MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
        MSNvmGWEOrderInfo_Struct *pGWEOrder = (pOrderData != NULL) ? &pOrderData->platOrderInfo.stGWEOrderInfo : NULL;

        if (pData != NULL)
        {
            for (i = 0; i < order.timeNum && i < ASWMONITOR_BILLMODE_PERIOD_COUNT; i++)
            {
                cJSON_AddItemToArray(partElectArray,  cJSON_CreateNumber(pData->periodElePower[i]));
                cJSON_AddItemToArray(chargeFeeArray,  cJSON_CreateNumber(pData->periodEleMoney[i]));
                cJSON_AddItemToArray(serviceFeeArray, cJSON_CreateNumber(pData->periodSerMoney[i]));
            }
        }

        if (pGWEOrder != NULL)
        {
            cJSON_AddNumberToObject(cParams, "startPoint", pGWEOrder->startPoint);
            cJSON_AddNumberToObject(cParams, "crossPoints", pGWEOrder->crossPoints);
            cJSON_AddItemToObject(cParams, "pointsElect", pointsElectArray = cJSON_CreateArray());
            for (i = 0; i < pGWEOrder->crossPoints && i < ASWMONITOR_BILLMODE_PERIOD_COUNT; i++)
            {
                uint8_t idx = (pGWEOrder->startPoint - 1 + i) % 96;
                uint32_t val = Common_TwoUint8ToUint16(pGWEOrder->pointsElect[idx]);
                cJSON_AddItemToArray(pointsElectArray, cJSON_CreateNumber(val));
            }
        }
        else
        {
            cJSON_AddNumberToObject(cParams, "startPoint", 0);
            cJSON_AddNumberToObject(cParams, "crossPoints", 0);
            cJSON_AddItemToObject(cParams, "pointsElect", pointsElectArray = cJSON_CreateArray());
        }
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [11] 故障告警 totalFaultEvt (Part4 §6.6.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendTotalFault(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    cJSON *faultArray = NULL, *warnArray = NULL;
    uint16_t faultValues[eErr_Num] = { 0 };
    uint16_t warnValues[eErr_Num] = { 0 };
    uint8_t faultSum = 0;
    uint8_t warnSum = 0;
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(c_stIotGWEFaultCodeMap); i++)
    {
        IotGWEFaultCodeMap_Struct *pMap = &c_stIotGWEFaultCodeMap[i];
        if (AswErrHandle_CheckErrExit(port, pMap->errorType) == TRUE)
        {
            if (pMap->isFault == TRUE)
            {
                faultValues[faultSum++] = pMap->gweCode;
            }
            else
            {
                warnValues[warnSum++] = pMap->gweCode;
            }
        }
    }

    cRoot = IotGWE_CreateEventRoot("totalFaultEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);
    cJSON_AddNumberToObject(cParams, "faultSum", faultSum);
    cJSON_AddNumberToObject(cParams, "warnSum", warnSum);

    cJSON_AddItemToObject(cParams, "faultValue", faultArray = cJSON_CreateArray());
    for (i = 0; i < faultSum; i++)
    {
        cJSON_AddItemToArray(faultArray, cJSON_CreateNumber(faultValues[i]));
    }
    cJSON_AddItemToObject(cParams, "warnValue", warnArray = cJSON_CreateArray());
    for (i = 0; i < warnSum; i++)
    {
        cJSON_AddItemToArray(warnArray, cJSON_CreateNumber(warnValues[i]));
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [12] 交流桩状态变化 acStChEvt (Part4 §7.3.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendAcStCh(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    char yxOccurTimeStr[11] = {0};

    cRoot = IotGWE_CreateEventRoot("acStChEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    snprintf(yxOccurTimeStr, sizeof(yxOccurTimeStr), "%u", (uint32_t)(SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT));

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);
    cJSON_AddStringToObject(cParams, "yxOccurTime", yxOccurTimeStr);
    cJSON_AddNumberToObject(cParams, "connCheckStatus", AswChargeIf_CheckGunConnected(port) ? \
                                eIotGWEConnStatus_Connect : eIotGWEConnStatus_DisConn);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [13] CP连接状态变化 acCarConChEvt (Part4 §7.5)
 ******************************************************************************
 */
static uint16_t IotGWE_SendAcCarConCh(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cParams = NULL;
    CddCPVolState_Enum cpState;
    uint8_t cpStatus;
    char yxOccurTimeStr[11] = {0};

    cRoot = IotGWE_CreateEventRoot("acCarConChEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cpState = CddCP_GetVolState(port);
    switch (cpState)
    {
        case eCddCPVolState_12V: cpStatus = eIotGWECPStatus_12V; break;
        case eCddCPVolState_9V:  cpStatus = eIotGWECPStatus_9V; break;
        case eCddCPVolState_6V:  cpStatus = eIotGWECPStatus_6V; break;
        default:                 cpStatus = eIotGWECPStatus_Other; break;
    }

    snprintf(yxOccurTimeStr, sizeof(yxOccurTimeStr), "%u", (uint32_t)(SSTM_GetSecTimestamp() - SSTM_BASE_TIMESTAMP_1970_BJT));

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);
    cJSON_AddNumberToObject(cParams, "cpStatus", cpStatus);
    cJSON_AddNumberToObject(cParams, "cpVolt", CddCP_GetVoltage(port) / 100);
    cJSON_AddNumberToObject(cParams, "s3Status", eIotGWES3Status_No);
    cJSON_AddStringToObject(cParams, "yxOccurTime", yxOccurTimeStr);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  电表底值逐帧上报: 全部/零点两种模式
 ******************************************************************************
 */
static uint16_t IotGWE_SendLogQueryMeterRecord(uint8_t port, void *pBuf, cJSON *cRoot, cJSON *cParams)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmMeterRecord_Struct rec;
    cJSON *meterRoot = NULL, *meterBody = NULL;
    char *meterPayload = NULL;
    char timeStr[16] = {0};
    char valStr[32] = {0};
    CommonDateTime_Struct dt;
    uint32_t curTime, nextTime;
    uint8_t  askMode;
    uint8_t  recMatches;
    uint8_t serial_flag = FALSE;
    uint16_t ret_len = 0;
    MSNvmMeterRecord_Struct nextRec;
    meter_query_t cmpPara;

    curTime = pIotGWECtx->meterRecordCursorTime;
    askMode = pIotGWECtx->meterRecordAskMode;

    do {
        /* 1.查找第一条记录 */
        memset(&rec, 0, sizeof(rec));
        cmpPara.cursor = curTime + 1;
        cmpPara.min = pOfflineClr->logQueryStartDate;
        cmpPara.mode = askMode;
        if (curTime == 0 || eGlobalRet_OK != MSNvm_QueryRecordByExternal(eMSNvmBlockID_MeterRecord,
              (uint8_t *)&cmpPara, sizeof(cmpPara), IotGWE_CmpPrevMeterRecord, (uint8_t *)&rec, sizeof(rec)))
        {
            pIotGWECtx->meterRecordUploadActive = FALSE;
            pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
            cJSON_DeleteItemFromObject(cParams, "result");
            cJSON_AddNumberToObject(cParams, "result", eIotGWELogResult_NODATA);
            cJSON_AddStringToObject(cParams, "dataArea", "");
            serial_flag = TRUE;
            break;
        }

        /* 2. 检查当前记录是否匹配 */
        recMatches = TRUE;
        if (askMode == eIotGWEMeterAskType_Midnight)
        {/* 零点模式 */
            Common_TimestampToDateTime(rec.acqTime, &dt);
            recMatches = (dt.hour == 0 && dt.minute == 0);
        }

        /* 3. 寻找下一条记录 */
        cmpPara.cursor = curTime;
        cmpPara.min = pOfflineClr->logQueryStartDate;
        cmpPara.mode = askMode;

        memset(&nextRec, 0, sizeof(nextRec));
        /* 遍历TSDB */
        if (eGlobalRet_OK == MSNvm_QueryRecordByExternal(eMSNvmBlockID_MeterRecord, (uint8_t *)&cmpPara, sizeof(cmpPara),
                                                IotGWE_CmpPrevMeterRecord, (uint8_t *)&nextRec, sizeof(nextRec)))
        {/* 找到, 则更新下一次遍历时间戳 */
            nextTime = nextRec.acqTime;
        }
        else
        {
            nextTime = 0;
        }

        pIotGWECtx->meterRecordCursorTime = nextTime;
        IOTGWE_CFG_DebugPrint("[GWE] meterRecord next: cur=%u min=%u mode=%u -> next=%u\r\n",
                              curTime, (uint32_t)pOfflineClr->logQueryStartDate, askMode, nextTime);

        /* 4.当前记录不匹配时: 跳过本帧(CycleDetect触发下一帧) */
        if (recMatches == FALSE)
        {
            if (nextTime == 0)
            {/* 遍历完成都没找到, 则发送带空dataArea的终帧通知平台结束 */
                pIotGWECtx->meterRecordUploadActive = FALSE;
                pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
                cJSON_AddStringToObject(cParams, "dataArea", "");
                serial_flag = TRUE;
                break;
            }
            cJSON_Delete(cRoot);
            break;
        }

        /* 5. 组包(本条记录有效) */
        meterRoot = cJSON_CreateObject();
        if (meterRoot == NULL)
        {
            cJSON_Delete(cRoot);
            break;
        }

        cJSON_AddItemToObject(meterRoot, "outMeterItyData", meterBody = cJSON_CreateObject());
        cJSON_AddNumberToObject(meterBody, "gunNo", rec.gunNo);

        Common_TimestampToDateTime(rec.acqTime, &dt);
        snprintf(timeStr, sizeof(timeStr), "%04u%02u%02u%02u%02u%02u", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        cJSON_AddStringToObject(meterBody, "acqTime", timeStr);

        cJSON_AddStringToObject(meterBody, "mailAddr", "");
        cJSON_AddStringToObject(meterBody, "meterNo", "");
        cJSON_AddStringToObject(meterBody, "assetId", "");

        snprintf(valStr, sizeof(valStr), "%llu", (unsigned long long)rec.sumMeter);
        cJSON_AddStringToObject(meterBody, "sumMeter", valStr);

        cJSON_AddStringToObject(meterBody, "lastTrade", rec.lastTrade);
        cJSON_AddNumberToObject(meterBody, "power", rec.elec);

        meterPayload = cJSON_PrintUnformatted(meterRoot);
        if (meterPayload != NULL)
        {
            cJSON_AddStringToObject(cParams, "dataArea", meterPayload);
            cJSON_free(meterPayload);
        }
        cJSON_Delete(meterRoot);
        
        serial_flag = TRUE;

    } while (0);

    if (serial_flag == TRUE)
    {
        ret_len = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret_len;
}
/**
 ******************************************************************************
 * @brief  故障告警逐帧上报
 ******************************************************************************
 */
static uint16_t IotGWE_SendLogQueryFaultRecord(uint8_t port, void *pBuf, cJSON *cRoot, cJSON *cParams)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmErrorInfo_Struct rec;
    MSNvmErrorInfo_Struct nextRec;
    uint32_t curTs, nextTs;
    meter_query_t cmpPara;
    uint8_t  serial_flag = FALSE;
    uint16_t ret_len = 0;

    curTs = pIotGWECtx->faultRecordCursorTime;

    do {
        /* 1.查找第一条记录 */
        memset(&rec, 0, sizeof(rec));
        cmpPara.cursor = curTs + 1;
        cmpPara.min = pOfflineClr->logQueryStartDate;
        cmpPara.mode = 0;
        if (curTs == 0 || eGlobalRet_OK != MSNvm_QueryRecordByExternal(eMSNvmBlockID_ErrorRecord,
              (uint8_t *)&cmpPara, sizeof(cmpPara), IotGWE_CmpPrevFaultRecord, (uint8_t *)&rec, sizeof(rec)))
        {
            pIotGWECtx->faultRecordUploadActive = FALSE;
            pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
            cJSON_DeleteItemFromObject(cParams, "result");
            cJSON_AddNumberToObject(cParams, "result", eIotGWELogResult_NODATA);
            cJSON_AddStringToObject(cParams, "dataArea", "");
            serial_flag = TRUE;
            break;
        }

        /* 2. 寻找下一条记录 */
        cmpPara.cursor = curTs;
        cmpPara.min = pOfflineClr->logQueryStartDate;
        cmpPara.mode = 0;

        memset(&nextRec, 0, sizeof(nextRec));
        if (eGlobalRet_OK == MSNvm_QueryRecordByExternal(eMSNvmBlockID_ErrorRecord,
              (uint8_t *)&cmpPara, sizeof(cmpPara), IotGWE_CmpPrevFaultRecord, (uint8_t *)&nextRec, sizeof(nextRec)))
        {
            nextTs = IotGWE_ExtractFaultTimestamp(&nextRec);
        }
        else
        {
            nextTs = 0;
        }

        pIotGWECtx->faultRecordCursorTime = nextTs;

        /* 3. 组包(截掉快照数据, 只保留故障描述部分) */
        IotGWE_TrimFaultSnapshot((char *)rec.userData);
        cJSON_AddStringToObject(cParams, "dataArea", (const char *)rec.userData);
        serial_flag = TRUE;

    } while (0);

    if (serial_flag == TRUE)
    {
        ret_len = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret_len;
}
/**
 ******************************************************************************
 * @brief  交易记录逐帧上报
 ******************************************************************************
 */
static uint16_t IotGWE_SendLogQueryOrderRecord(uint8_t port, void *pBuf, cJSON *cRoot, cJSON *cParams)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmOrderInfo_Struct rec;
    MSNvmOrderInfo_Struct nextRec;
    MSNvmGWEOrderInfo_Struct *pOrder;
    uint32_t curTs, nextTs;
    meter_query_t cmpPara;
    cJSON *tradeRoot = NULL;
    cJSON *partElecArray = NULL, *chargeFeeArray = NULL, *serviceFeeArray = NULL, *pointElecArray = NULL;
    char *tradePayload = NULL;
    char timeStr[16] = {0};
    char sumStr[24] = {0};
    uint8_t serial_flag = FALSE;
    uint16_t ret_len = 0;
    uint8_t i;

    curTs = pIotGWECtx->tradeRecordCursorTime;

    do {
        /* 1. 读取当前记录(按时间戳精确匹配) */
        memset(&rec, 0, sizeof(rec));
        if (curTs == 0 || \
            eGlobalRet_OK != MSNvm_QueryRecordByExternal(eMSNvmBlockID_OrderRecord, (uint8_t *)&curTs, sizeof(curTs), IotGWE_CmpExactOrderTimestamp, (uint8_t *)&rec, sizeof(rec)))
        {
            pIotGWECtx->tradeRecordUploadActive = FALSE;
            pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
            cJSON_AddStringToObject(cParams, "dataArea", "");
            serial_flag = TRUE;
            break;
        }

        pOrder = &rec.platOrderInfo.stGWEOrderInfo;

        /* 2. 寻找下一条记录 */
        cmpPara.cursor = curTs;
        cmpPara.min = pOfflineClr->logQueryStartDate;
        cmpPara.mode = 0;

        memset(&nextRec, 0, sizeof(nextRec));
        if (eGlobalRet_OK == MSNvm_QueryRecordByExternal(eMSNvmBlockID_OrderRecord,
              (uint8_t *)&cmpPara, sizeof(cmpPara), IotGWE_CmpPrevOrderRecord, (uint8_t *)&nextRec, sizeof(nextRec)))
        {
            nextTs = nextRec.platOrderInfo.stGWEOrderInfo.startTime;
        }
        else
        {
            nextTs = 0;
        }

        pIotGWECtx->tradeRecordCursorTime = nextTs;

        /* 3. 组包 */
        tradeRoot = cJSON_CreateObject();
        if (tradeRoot == NULL)
        {
            cJSON_Delete(cRoot);
            break;
        }

        cJSON_AddNumberToObject(tradeRoot, "gunNo", pOrder->gunNo);
        cJSON_AddStringToObject(tradeRoot, "preTradeNo", pOrder->preTradeNo);
        cJSON_AddStringToObject(tradeRoot, "tradeNo", pOrder->tradeNo);
        cJSON_AddStringToObject(tradeRoot, "vinCode", "");
        cJSON_AddNumberToObject(tradeRoot, "timeDivType", eIotGWETimeDivType_MeterAndBill);
        cJSON_AddNumberToObject(tradeRoot, "startType", IotGWE_MapStartType(pOrder->startType));

        snprintf(timeStr, sizeof(timeStr), "%u", (uint32_t)(pOrder->startTime - SSTM_BASE_TIMESTAMP_1970_BJT));
        cJSON_AddStringToObject(tradeRoot, "chargeStartTime", timeStr);
        snprintf(timeStr, sizeof(timeStr), "%u", (uint32_t)(pOrder->endTime - SSTM_BASE_TIMESTAMP_1970_BJT));
        cJSON_AddStringToObject(tradeRoot, "chargeEndTime", timeStr);

        cJSON_AddNumberToObject(tradeRoot, "startSoc", 0);
        cJSON_AddNumberToObject(tradeRoot, "endSoc", 0);
        cJSON_AddNumberToObject(tradeRoot, "reason", IotGWE_MapStopReason(pOrder->stopReason));

        cJSON_AddStringToObject(tradeRoot, "feeModelId", (const char *)pOrder->billModeID);

        snprintf(sumStr, sizeof(sumStr), "%lu", (unsigned long)pOrder->sumStart);
        cJSON_AddStringToObject(tradeRoot, "sumStart", sumStr);
        snprintf(sumStr, sizeof(sumStr), "%lu", (unsigned long)pOrder->sumEnd);
        cJSON_AddStringToObject(tradeRoot, "sumEnd", sumStr);

        cJSON_AddNumberToObject(tradeRoot, "totalElect", pOrder->totalElec);
        
        cJSON_AddNumberToObject(tradeRoot, "totalPowerCost", pOrder->totalPowerCost);
        cJSON_AddNumberToObject(tradeRoot, "totalServCost", pOrder->totalServCost);
        cJSON_AddNumberToObject(tradeRoot, "totalCost", pOrder->totalPowerCost + pOrder->totalServCost);
        cJSON_AddNumberToObject(tradeRoot, "timeNum", pOrder->timeNum);

        /* 时段分段数据 */
        if (pOrder->timeNum > 0 && pOrder->timeNum <= MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX)
        {
            cJSON_AddItemToObject(tradeRoot, "partElect", partElecArray = cJSON_CreateArray());
            cJSON_AddItemToObject(tradeRoot, "chargeFee", chargeFeeArray = cJSON_CreateArray());
            cJSON_AddItemToObject(tradeRoot, "serviceFee", serviceFeeArray = cJSON_CreateArray());
            for (i = 0; i < pOrder->timeNum; i++)
            {
                cJSON_AddItemToArray(partElecArray, cJSON_CreateNumber(Common_ThreeUint8ToUint32(pOrder->partElect[i])));
                cJSON_AddItemToArray(chargeFeeArray, cJSON_CreateNumber(Common_ThreeUint8ToUint32(pOrder->chargeFee[i])));
                cJSON_AddItemToArray(serviceFeeArray, cJSON_CreateNumber(Common_ThreeUint8ToUint32(pOrder->serviceFee[i])));
            }
        }

        cJSON_AddNumberToObject(tradeRoot, "startPoint", pOrder->startPoint);
        cJSON_AddNumberToObject(tradeRoot, "crossPoints", pOrder->crossPoints);

        /* 跨越点电量 */
        if (pOrder->crossPoints > 0 && pOrder->crossPoints <= MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX)
        {
            cJSON_AddItemToObject(tradeRoot, "pointsElect", pointElecArray = cJSON_CreateArray());
            for (i = 0; i < pOrder->crossPoints; i++)
            {
                uint8_t idx = (pOrder->startPoint - 1 + i) % 96;
                cJSON_AddItemToArray(pointElecArray, cJSON_CreateNumber(Common_TwoUint8ToUint16(pOrder->pointsElect[idx])));
            }
        }

        tradePayload = cJSON_PrintUnformatted(tradeRoot);
        if (tradePayload != NULL)
        {
            cJSON_AddStringToObject(cParams, "dataArea", tradePayload);
            cJSON_free(tradePayload);
        }
        cJSON_Delete(tradeRoot);
        serial_flag = TRUE;

    } while (0);

    if (serial_flag == TRUE)
    {
        ret_len = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret_len;
}
/**
 ******************************************************************************
 * @brief  运行日志分片逐帧上报: 单条3584字节→1KB分片, 最多100KB
 ******************************************************************************
 */
static uint16_t IotGWE_SendLogQueryRunLogRecord(uint8_t port, void *pBuf, cJSON *cRoot, cJSON *cParams)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    MSNvmRunningLog_Struct *pRec = NULL;
    char chunkBuf[IOTGWE_RUNLOG_UPLOAD_CHUNK + 1];
    uint8_t  serial_flag = FALSE;
    uint16_t ret_len = 0;
    uint16_t chunkLen;

    pRec = (MSNvmRunningLog_Struct *)myMalloc(sizeof(MSNvmRunningLog_Struct));
    if (pRec == NULL)
    {
        cJSON_AddStringToObject(cParams, "dataArea", "");
    }
    else 
    {
        do {
            /* 1. 读取当前记录 */
            memset(pRec, 0, sizeof(MSNvmRunningLog_Struct));
            if (pIotGWECtx->runLogCursorIdx == 0 ||
                eGlobalRet_OK != MSNvm_QueryRecordByTime(eMSNvmBlockID_RunningLogRecord, (uint8_t *)pRec, sizeof(MSNvmRunningLog_Struct),
                    pIotGWECtx->runLogCursorIdx))
            {
                pOfflineClr->srvQueDataResult = eIotGWELogResult_NODATA;
                cJSON_AddStringToObject(cParams, "dataArea", "");
                serial_flag = TRUE;
                break;
            }

            /* 2. 取本片数据(从当前偏移量开始, 最多1KB) */
            chunkLen = IOTGWE_RUNLOG_UPLOAD_CHUNK;
            if ((pIotGWECtx->runLogByteOffset + chunkLen) > MSNVM_RUNNING_LOG_MAX_LEN)
            {
                chunkLen = MSNVM_RUNNING_LOG_MAX_LEN - pIotGWECtx->runLogByteOffset;
            }
            memcpy(chunkBuf, &pRec->userData[pIotGWECtx->runLogByteOffset], chunkLen);
            chunkBuf[chunkLen] = '\0';
            cJSON_AddStringToObject(cParams, "dataArea", chunkBuf);

            pIotGWECtx->runLogByteOffset += chunkLen;
            pIotGWECtx->runLogSentBytes += chunkLen;

            /* 3. 本条记录读完 -> 移到上一条 */
            if (pIotGWECtx->runLogByteOffset >= MSNVM_RUNNING_LOG_MAX_LEN)
            {
                pIotGWECtx->runLogByteOffset = 0;
                pIotGWECtx->runLogCursorIdx--;
            }

            /* 4. 超过100KB上限 -> 强制结束 */
            if (pIotGWECtx->runLogSentBytes >= IOTGWE_RUNLOG_UPLOAD_TOTAL)
            {
                pIotGWECtx->runLogCursorIdx = 0;
            }

            serial_flag = TRUE;

        } while (0);
    }

    if (pRec != NULL)
    {
        myFree(pRec);
    }

    if (serial_flag == TRUE)
    {
        ret_len = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret_len;
}
/**
 ******************************************************************************
 * @brief  [14] 日志查询结果 logQueryEvt (Part4 §5.8.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendLogQueryResult(uint8_t port, void *pBuf)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL;
    char startDateStr[12] = {0};
    char stopDateStr[12] = {0};
    uint16_t ret = 0;

    cRoot = IotGWE_CreateEventRoot("logQueryEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    snprintf(startDateStr, sizeof(startDateStr), "%u", (uint32_t)(pOfflineClr->logQueryStartDate >= SSTM_BASE_TIMESTAMP_1970_BJT ? 
                (pOfflineClr->logQueryStartDate - SSTM_BASE_TIMESTAMP_1970_BJT) : 0));
    snprintf(stopDateStr,  sizeof(stopDateStr),  "%u", (uint32_t)(pOfflineClr->logQueryStopDate >= SSTM_BASE_TIMESTAMP_1970_BJT ? 
                (pOfflineClr->logQueryStopDate - SSTM_BASE_TIMESTAMP_1970_BJT) : 0));

    cJSON_AddNumberToObject(cParams, "gunNo", port + 1);
    cJSON_AddStringToObject(cParams, "startDate", startDateStr);
    cJSON_AddStringToObject(cParams, "stopDate", stopDateStr);
    cJSON_AddNumberToObject(cParams, "askType", pOfflineClr->logQueryAskType);
    cJSON_AddNumberToObject(cParams, "result", pOfflineClr->srvQueDataResult);
    cJSON_AddStringToObject(cParams, "logQueryNo", pOfflineClr->logQueryNo);
    cJSON_AddNumberToObject(cParams, "retType", pOfflineClr->logQueryRetType);
    cJSON_AddNumberToObject(cParams, "logQueryEvtSum", pOfflineClr->logQueryEvtSum);
    cJSON_AddNumberToObject(cParams, "logQueryEvtNo", pOfflineClr->logQueryEvtNo);

    switch(pOfflineClr->logQueryAskType)
    {
        case eIotGWELogAskType_Order:       /* 交易记录上报 */
            ret = IotGWE_SendLogQueryOrderRecord(port, pBuf, cRoot, cParams);
            break;

        case eIotGWELogAskType_MeterStart:  /* 电表底值上报 */
            ret = IotGWE_SendLogQueryMeterRecord(port, pBuf, cRoot, cParams);
            break;

        case eIotGWELogAskType_FAULT:       /* 故障警告上报 */
            ret = IotGWE_SendLogQueryFaultRecord(port, pBuf, cRoot, cParams);
            break;

        case eIotGWELogAskType_Log:         /* 设备日志上报 */
            ret = IotGWE_SendLogQueryRunLogRecord(port, pBuf, cRoot, cParams);
            break;

        default:
            cJSON_AddStringToObject(cParams, "dataArea", "");
            ret = IotGWE_SerializeJson(cRoot, pBuf);
            break;
    }

    return ret;
}
/**
 ******************************************************************************
 * @brief  [15] 设备维护结果 devMaintainRetEvt (Part4 §5.9.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendDevMaintainRet(uint8_t port, void *pBuf)
{
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    cJSON *cRoot = NULL, *cParams = NULL;

    cRoot = IotGWE_CreateEventRoot("devMaintainRetEvt", &cParams);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cParams, "ctrlType", pOfflineClr->devMaintainCtrlType);
    cJSON_AddNumberToObject(cParams, "reason", pOfflineClr->devMaintainReason);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [16] 交流设备属性 acDeRealItyData (Part4 §7.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendPropertyAcPile(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cBody = NULL;
    uint16_t csq;
    CddNetMOperator_Enum eOperator;
    uint8_t netId;
    uint8_t i;
    char strBuf[17] = {0};
     
    cRoot = IotGWE_CreatePropertyRoot("acDeRealItyData", &cBody);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    /* 信号强度: 0~31(越大越好), >31截断 */
    csq = CddNetM_GetCsq();
    cJSON_AddNumberToObject(cBody, "netType", eIotGWENetType_4G);
    cJSON_AddNumberToObject(cBody, "sigVal", (csq > 31) ? 31 : csq);

    eOperator = CddNetM_GetOperatorType();
    switch (eOperator)
    {
        case eCddNetMOperator_Null:  netId = eIotGWENetId_Unknow; break;
        case eCddNetMOperator_CUCC:  netId = eIotGWENetId_CUCC;   break;
        case eCddNetMOperator_CMCC:  netId = eIotGWENetId_CMCC;   break;
        case eCddNetMOperator_CTCC:  netId = eIotGWENetId_CTCC;   break;
        default:                     netId = eIotGWENetId_Unknow; break;
    }
    cJSON_AddNumberToObject(cBody, "netId", netId);
    cJSON_AddNumberToObject(cBody, "acVolA", AswChargeIf_GetOutputVoltage(port)/10);
    cJSON_AddNumberToObject(cBody, "acCurA", AswChargeIf_GetOutputCurrent(port)/10);
    cJSON_AddNumberToObject(cBody, "acVolB", 0);
    cJSON_AddNumberToObject(cBody, "acCurB", 0);
    cJSON_AddNumberToObject(cBody, "acVolC", 0);
    cJSON_AddNumberToObject(cBody, "acCurC", 0);

    cJSON_AddNumberToObject(cBody, "caseTemp", AswChargeIf_GetGunTemperature(port)*10);
    {
        MSNvmPlatPrivateParam_Union *pPP = AswPlatM_GetPlatPrivateParamPtr();
        MSNvmGWEParamBillMode_Struct *pBM = &pPP->stGWEParam.stBillMode;
        memcpy(strBuf, pBM->billModeID, sizeof(pBM->billModeID));
    }
    cJSON_AddStringToObject(cBody, "feeModelId", strBuf);

    /* 系统资源 */
    {
        extern size_t xPortGetFreeHeapSize(void);
        size_t freeHeap = xPortGetFreeHeapSize();
        cJSON_AddNumberToObject(cBody, "totalRam", SYSCFG_CFG_OS_HEAP_SIZE / 1024);
        cJSON_AddNumberToObject(cBody, "ramUseRate", (uint8_t)((SYSCFG_CFG_OS_HEAP_SIZE - freeHeap) * 100 / SYSCFG_CFG_OS_HEAP_SIZE));
    }
    cJSON_AddNumberToObject(cBody, "totalRom", 512);
    cJSON_AddNumberToObject(cBody, "romUseRate", 40);
    cJSON_AddNumberToObject(cBody, "cpuUseRate", 30);

    snprintf(strBuf, sizeof(strBuf), "CSQ:%d", csq);
    cJSON_AddStringToObject(cBody, "netSigQua", strBuf);

    snprintf(strBuf, sizeof(strBuf), "CP:%d", CddCP_GetCpDuty(port));
    cJSON_AddStringToObject(cBody, "devRunSampVal", strBuf);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [17] 交流充电实时属性 acGunRunItyData (Part4 §7.1.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendPropertyAcWork(uint8_t port, void *pBuf)
{
    MSNvmOrderInfo_Struct *pOrderData = AswMonitor_GerOrderDataPtr(port);
    AswMonitorChargeData_Struct *pData = AswMonitor_GetChargeDataPtr(port);
    MSNvmGWEOrderInfo_Struct *pGWEOrder = NULL;
    cJSON *cRoot = NULL, *cBody = NULL;
    char strBuf[24] = {0};
    uint8_t i;
    int16_t gunTemp;
    uint16_t pos;
    char g_sStrBuf[GWE_SHARED_STRBUF_SIZE];

    if (AswChargeIf_GetChargeState(port) != ASWCHARGE_WORKSTATE_CHARGING)
    {/* 不处于充电中,则取消上报 */
        return 0;
    }

    cRoot = IotGWE_CreatePropertyRoot("acGunRunItyData", &cBody);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    if (pOrderData != NULL)
    {
        pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
    }

    cJSON_AddNumberToObject(cBody, "gunNo", port + 1);
    cJSON_AddNumberToObject(cBody, "workStatus", eIotGWEWorkStatus_CHARGING);
    cJSON_AddNumberToObject(cBody, "conStatus", AswChargeIf_CheckGunConnected(port) ? eIotGWEConnStatus_Connect : eIotGWEConnStatus_DisConn);
    cJSON_AddNumberToObject(cBody, "eLockStatus", eIotGWEELockStatus_Lock);
    cJSON_AddNumberToObject(cBody, "outRelayStatus", (AswChargeIf_GetRelayState(port) == ASWCHARGEIF_RELAYSTATE_ON) ?
                                eIotGWEOutRelayStatus_Close : eIotGWEOutRelayStatus_Open);

    /* gunTemp: (temp+50)*10 0.1°C, 钳位≥0 */
    gunTemp = (int16_t)AswChargeIf_GetGunTemperature(port) * 10;
    cJSON_AddNumberToObject(cBody, "gunTemp", (gunTemp > 0) ? gunTemp : 0);

    /* 输出电压电流 (单相, B/C为0) */
    uint32_t vol = AswChargeIf_GetOutputVoltage(port)/10;
    uint32_t cur = AswChargeIf_GetOutputCurrent(port)/10;
    cJSON_AddNumberToObject(cBody, "acVolA", vol);
    cJSON_AddNumberToObject(cBody, "acCurA", cur);
    cJSON_AddNumberToObject(cBody, "acVolB", 0);
    cJSON_AddNumberToObject(cBody, "acCurB", 0);
    cJSON_AddNumberToObject(cBody, "acVolC", 0);
    cJSON_AddNumberToObject(cBody, "acCurC", 0);

    /* 交易流水号 */
    cJSON_AddStringToObject(cBody, "preTradeNo", (pGWEOrder != NULL) ? pGWEOrder->preTradeNo : "");
    cJSON_AddStringToObject(cBody, "tradeNo", (pGWEOrder != NULL) ? pGWEOrder->tradeNo : "");

    /* 实际功率 */
    cJSON_AddNumberToObject(cBody, "realPower", AswChargeIf_GetOutputPower(port) / 10);

    cJSON_AddNumberToObject(cBody, "chgTime", pData->chargeTime / 60);
    cJSON_AddNumberToObject(cBody, "PwmDutyRadio", CddCP_GetCpDuty(port) / 10);
    cJSON_AddNumberToObject(cBody, "s2SwhActNum", 0);

    /* 电表底值 */
    snprintf(strBuf, sizeof(strBuf), "%u", (uint32_t)pData->startMeterVal);
    cJSON_AddStringToObject(cBody, "meterStartVal", strBuf);
    snprintf(strBuf, sizeof(strBuf), "%llu", AswChargeIf_GetMeterEnergyVal(port));
    cJSON_AddStringToObject(cBody, "meterRealVal", strBuf);

    /* 时段数统计:电量, 金额 */
    {
        AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
        uint8_t periodCnt = (pBillMode != NULL && pBillMode->validFlag == TRUE) ? pBillMode->periodCount : 0;
        uint32_t sumElect = 0U, sumElecFee = 0U, sumServFee = 0U;

        /* 时段电量 */
        pos = 0;
        for (i = 0; i < periodCnt && pos < GWE_SHARED_STRBUF_SIZE - 12; i++)
        {
            uint32_t v = (uint32_t)pData->periodElePower[i];
            pos += snprintf(g_sStrBuf + pos, sizeof(g_sStrBuf) - pos, "%u,", v);
            sumElect += v;
        }
        if (pos > 0 && g_sStrBuf[pos - 1] == ',')
            g_sStrBuf[pos - 1] = '\0';
        else if (pos == 0)
            g_sStrBuf[0] = '\0';
        cJSON_AddStringToObject(cBody, "partElect", g_sStrBuf);

        /* 时段电费 */
        pos = 0;
        for (i = 0; i < periodCnt && pos < GWE_SHARED_STRBUF_SIZE - 12; i++)
        {
            uint32_t v = (uint32_t)pData->periodEleMoney[i];
            pos += snprintf(g_sStrBuf + pos, sizeof(g_sStrBuf) - pos, "%u,", v);
            sumElecFee += v;
        }
        if (pos > 0 && g_sStrBuf[pos - 1] == ',')
            g_sStrBuf[pos - 1] = '\0';
        else if (pos == 0)
            g_sStrBuf[0] = '\0';
        cJSON_AddStringToObject(cBody, "chargeFee", g_sStrBuf);

        /* 时段服务费 */
        pos = 0;
        for (i = 0; i < periodCnt && pos < GWE_SHARED_STRBUF_SIZE - 12; i++)
        {
            uint32_t v = (uint32_t)pData->periodSerMoney[i];
            pos += snprintf(g_sStrBuf + pos, sizeof(g_sStrBuf) - pos, "%u,", v);
            sumServFee += v;
        }
        if (pos > 0 && g_sStrBuf[pos - 1] == ',')
            g_sStrBuf[pos - 1] = '\0';
        else if (pos == 0)
            g_sStrBuf[0] = '\0';
        cJSON_AddStringToObject(cBody, "serviceFee", g_sStrBuf);

        cJSON_AddNumberToObject(cBody, "timeNum", periodCnt);
        cJSON_AddNumberToObject(cBody, "totalElect", (int32_t)sumElect);
        cJSON_AddNumberToObject(cBody, "totalPowerCost", (int32_t)sumElecFee);
        cJSON_AddNumberToObject(cBody, "totalServCost", (int32_t)sumServFee);
        cJSON_AddNumberToObject(cBody, "totalCost", (int32_t)(sumElecFee + sumServFee));
    }

    /* 跨越点电量 */
    {
        uint8_t pointsElect[96][2];
        uint8_t startPoint = 0, crossPoints = 0;
        AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);

        if (pData != NULL && pBillMode != NULL && pBillMode->validFlag == TRUE
            && pData->chargeStartTime > 0)
        {
            IotGWE_ComputePointsElect(pData, pBillMode, pointsElect, &startPoint, &crossPoints);
        }

        if (crossPoints > 0)
        {
            pos = 0;
            for (i = 0; i < crossPoints && i < ASWMONITOR_BILLMODE_PERIOD_COUNT
                 && pos < (int)(sizeof(g_sStrBuf) - 10); i++)
            {
                uint8_t idx = (startPoint - 1 + i) % 96;
                uint32_t val = Common_TwoUint8ToUint16(pointsElect[idx]);
                pos += snprintf(g_sStrBuf + pos, sizeof(g_sStrBuf) - pos, "%u,", val);
            }
            if (pos > 0 && g_sStrBuf[pos - 1] == ',')
                g_sStrBuf[pos - 1] = '\0';
            cJSON_AddStringToObject(cBody, "pointsElect", g_sStrBuf);
        }
        else
        {
            cJSON_AddStringToObject(cBody, "pointsElect", "");
        }

        cJSON_AddNumberToObject(cBody, "startPoint", startPoint);
        cJSON_AddNumberToObject(cBody, "crossPoints", crossPoints);
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [18] 交流非充电实时属性 acGunIdleItyData (Part4 §7.1.4)
 ******************************************************************************
 */
static uint16_t IotGWE_SendPropertyAcNonWork(uint8_t port, void *pBuf)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);
    uint8_t gunConnected = AswChargeIf_CheckGunConnected(port);
    cJSON *cRoot = NULL, *cBody = NULL;
    char strBuf[24] = {0};
    int16_t gunTemp;

    cRoot = IotGWE_CreatePropertyRoot("acGunIdleItyData", &cBody);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cBody, "gunNo", port + 1);
    cJSON_AddNumberToObject(cBody, "workStatus", IotGWE_MapWorkStatus(port, chargeState, gunConnected));
    cJSON_AddNumberToObject(cBody, "conStatus", gunConnected ? eIotGWEConnStatus_Connect : eIotGWEConnStatus_DisConn);
    cJSON_AddNumberToObject(cBody, "outRelayStatus", (AswChargeIf_GetRelayState(port) == ASWCHARGEIF_RELAYSTATE_ON) ? \
                                                    eIotGWEOutRelayStatus_Close : eIotGWEOutRelayStatus_Open);
    cJSON_AddNumberToObject(cBody, "eLockStatus", eIotGWEELockStatus_UnLock);

    gunTemp = (int16_t)AswChargeIf_GetGunTemperature(port) * 10;
    cJSON_AddNumberToObject(cBody, "gunTemp", (gunTemp > 0) ? gunTemp : 0);

    /* 电压电流 (单相) */
    cJSON_AddNumberToObject(cBody, "acVolA", AswChargeIf_GetOutputVoltage(port)/10);
    cJSON_AddNumberToObject(cBody, "acCurA", AswChargeIf_GetOutputCurrent(port)/10);
    cJSON_AddNumberToObject(cBody, "acVolB", 0);
    cJSON_AddNumberToObject(cBody, "acCurB", 0);
    cJSON_AddNumberToObject(cBody, "acVolC", 0);
    cJSON_AddNumberToObject(cBody, "acCurC", 0);

    /* sumMeter */
    snprintf(strBuf, sizeof(strBuf), "%llu", AswChargeIf_GetMeterEnergyVal(port));
    cJSON_AddStringToObject(cBody, "sumMeter", strBuf);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [19] 交流输出电表属性 acOutMeterItyData (Part4 §7.1.5)
 ******************************************************************************
 */
static uint16_t IotGWE_SendPropertyAcOutMeter(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cBody = NULL;
    char timeStr[16] = {0};
    char strBuf[24] = {0};
    CommonDateTime_Struct dt;
    MSNvmMeterRecord_Struct rec;
    uint32_t latestTime;

    cRoot = IotGWE_CreatePropertyRoot("acOutMeterItyData", &cBody);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cBody, "gunNo", port + 1);

    /* 从TSDB取最后一笔存储记录 */
    latestTime = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_MeterRecord);
    if (latestTime > 0 &&
        eGlobalRet_OK == MSNvm_QueryRecordByTime(eMSNvmBlockID_MeterRecord, (uint8_t *)&rec, sizeof(rec), latestTime))
    {
        Common_TimestampToDateTime(rec.acqTime, &dt);
        snprintf(strBuf, sizeof(strBuf), "%llu", (unsigned long long)rec.sumMeter);
        cJSON_AddStringToObject(cBody, "lastTrade", rec.lastTrade);
        cJSON_AddNumberToObject(cBody, "power", rec.elec);
    }
    else
    {/* TSDB无记录(首次上电尚未到整点), 用实时值填充 */
        Common_TimestampToDateTime(SSTM_GetSecTimestamp(), &dt);
        snprintf(strBuf, sizeof(strBuf), "%llu", AswChargeIf_GetMeterEnergyVal(port));
        cJSON_AddStringToObject(cBody, "lastTrade", "");
        cJSON_AddNumberToObject(cBody, "power", 0);
    }

    snprintf(timeStr, sizeof(timeStr), "%04u%02u%02u%02u%02u%02u",
                 dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    cJSON_AddStringToObject(cBody, "acqTime", timeStr);
    cJSON_AddStringToObject(cBody, "sumMeter", strBuf);
    
    cJSON_AddStringToObject(cBody, "mailAddr", "");
    cJSON_AddStringToObject(cBody, "meterNo", "");
    cJSON_AddStringToObject(cBody, "assetId", "");

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [20] 配置更新应答 confUpdateCtrlSrv (Part4 §5.4.1.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvConfUpdateReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "resCode", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.srvUpdateResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [21] 配置获取应答 getDevConfSrv (Part4 §5.4.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvGetDevConfReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL, *cArray = NULL;
    MSNvmPlatPrivateParam_Union *pPP = AswPlatM_GetPlatPrivateParamPtr();

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "equipParamFreq", pPP->stGWEParam.platinfo.equipParamReportCycle);
    cJSON_AddNumberToObject(cData, "gunElecFreq",    pPP->stGWEParam.platinfo.gunElecReportCycle);
    cJSON_AddNumberToObject(cData, "nonElecFreq",    pPP->stGWEParam.platinfo.nonElecReportCycle);
    cJSON_AddNumberToObject(cData, "faultWarnings",  pPP->stGWEParam.platinfo.faultWarningsCycle);
    cJSON_AddNumberToObject(cData, "offlinChaLen",   pPP->stGWEParam.platinfo.offlineChaLen);
    cJSON_AddNumberToObject(cData, "grndLock",       0);
    cJSON_AddNumberToObject(cData, "doorLock",       0);
    cJSON_AddNumberToObject(cData, "s2CloseTime",    2);
    cJSON_AddNumberToObject(cData, "s2OpenTime",     10);

    /* 二维码 */
    cJSON_AddItemToObject(cData, "qrCode", cArray = cJSON_CreateArray());
    if (cArray != NULL)
    {
        for (uint8_t i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            MSNvmDrcode_Struct qrData;
            /* TODO: 多枪需扩展 eMSNvmBlockID_Gun1Qrcode 等, 当前仅读枪0 */
            if (MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrData, sizeof(qrData)) != eGlobalRet_OK)
            {
                memset(&qrData, 0, sizeof(qrData));
            }
            cJSON_AddItemToArray(cArray, cJSON_CreateString(qrData.qrcode));
        }
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [22] 功能配置更新应答 funConfUpdateDataSrv (Part4 §5.5.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvFunConfUpdateReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "resCode", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.srvUpdateResult);
    cJSON_AddNumberToObject(cData, "funCode", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.srvUpdateFunCode);
    cJSON_AddStringToObject(cData, "optSn", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.srvUpdateOptSn);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [23] 功能配置获取应答 getFunConfSrv (Part4 §5.5.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvGetFunConfReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    uint16_t funCode = pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.srvUpdateFunCode;
    uint8_t confString = FALSE;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "funCode", funCode);

    switch (funCode)
    {
        case 10:    /* 查询"允许无计费模型启动": bit6=1 表示已开启 */
            cJSON_AddNumberToObject(cData, "confInt", pIotGWECtx->funConfig.allowNoFeeModelStart ? (1u << 6) : 0);
            break;
        case 17:
            cJSON_AddNumberToObject(cData, "confInt", pIotGWECtx->funConfig.allowPWMMax);
            break;
        case 18:
            cJSON_AddNumberToObject(cData, "confInt", pIotGWECtx->funConfig.allowTempWarning);
            break;
        case 19:
            cJSON_AddNumberToObject(cData, "confInt", pIotGWECtx->funConfig.allowTempFault);
            break;
        case 20:
            cJSON_AddNumberToObject(cData, "confInt", 0);
            cJSON_AddStringToObject(cData, "confString", pIotGWECtx->funConfig.otaInf);
            confString = TRUE;
            break;
        default:
            cJSON_AddNumberToObject(cData, "confInt", 0);
            break;
    }

    if (confString == FALSE)
    {
        cJSON_AddStringToObject(cData, "confString", "");
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [24] 计费模型更新应答 feeModelTwUpdateSrv (Part4 §6.1.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvFeeModelUpdateReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddStringToObject(cData, "feeModelId", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.feeModelId);
    cJSON_AddNumberToObject(cData, "result", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.feeModelResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [25] 计费模型查询应答 feeModelTwQuerySrv (Part4 §6.1.4)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvFeeModelQueryReply(uint8_t port, void *pBuf)
{
    AswMonitorBillMode_Struct *pBillMode;
    MSNvmPlatPrivateParam_Union *pPP;
    char *pOut = (char *)pBuf;
    uint16_t pos = 0;
    const uint16_t remain = IOT_GWE_TXRX_BUFFER_SIZE;
    uint8_t count = 0;
    uint8_t i;
    char feeId[17] = {0};

    pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    if ((pBillMode != NULL) && (pBillMode->validFlag == TRUE))
    {
        pPP = AswPlatM_GetPlatPrivateParamPtr();
        memcpy(feeId, pPP->stGWEParam.stBillMode.billModeID, sizeof(feeId));
        count = pBillMode->periodCount;
    }
    else
    {
        memcpy(feeId, pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.feeModelId, sizeof(feeId));
        count = 0;
    }

    JSON_APPEND(
        "{\"id\":\"%u\",\"code\":200,\"data\":{"
        "\"feeModelId\":\"%.16s\",\"timeNum\":%u",
        pIotGWECtx->curMsgId, feeId, (unsigned int)count);

    /* timeSeg */
    JSON_APPEND(",\"timeSeg\":[");

    if (count > 0)
    {
        for (i = 0; i < count; i++)
        {
            JSON_APPEND("%s\"%02u%02u\"", (i == 0) ? "" : ",",
                        pBillMode->startTime[i][0], pBillMode->startTime[i][1]);
        }
    }

    JSON_APPEND("]");

    /* chargeFee */
    JSON_APPEND(",\"chargeFee\":[");

    if (count > 0)
    {
        for (i = 0; i < count; i++)
        {
            JSON_APPEND("%s%u", (i == 0) ? "" : ",",
                        (unsigned int)pBillMode->rateElecPrice[i]);
        }
    }

    JSON_APPEND("]");

    /* serviceFee */
    JSON_APPEND(",\"serviceFee\":[");

    if (count > 0)
    {
        for (i = 0; i < count; i++)
        {
            JSON_APPEND("%s%u", (i == 0) ? "" : ",",
                        (unsigned int)pBillMode->rateSeverPrice[i]);
        }
    }

    JSON_APPEND("]}}");

    if (pos >= remain)
    {
        pos = 0;
    }

    IOTGWE_CFG_DebugPrint("[GWE] feeModel query reply: port=%u, len=%u\r\n", port, pos);

    return pos;
}
/**
 ******************************************************************************
 * @brief  [26] 远程启动充电应答 startChargeSrv (Part4 §6.2.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvStartChargeReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    MSNvmOrderInfo_Struct *pOrderData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "gunNo", port + 1);

    pOrderData = AswMonitor_GerOrderDataPtr(port);
    if (pOrderData != NULL)
    {
        MSNvmGWEOrderInfo_Struct *pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
        cJSON_AddStringToObject(cData, "preTradeNo", pGWEOrder->preTradeNo);
        cJSON_AddStringToObject(cData, "tradeNo",    pGWEOrder->tradeNo);
    }
    else
    {
        cJSON_AddStringToObject(cData, "preTradeNo", "");
        cJSON_AddStringToObject(cData, "tradeNo",    "");
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [27] 远程停止充电应答 stopChargeSrv (Part4 §6.4.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvStopChargeReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    MSNvmOrderInfo_Struct *pOrderData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "gunNo", port + 1);

    pOrderData = AswMonitor_GerOrderDataPtr(port);
    if (pOrderData != NULL)
    {
        MSNvmGWEOrderInfo_Struct *pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
        cJSON_AddStringToObject(cData, "preTradeNo", pGWEOrder->preTradeNo);
        cJSON_AddStringToObject(cData, "tradeNo",    pGWEOrder->tradeNo);
    }
    else
    {
        cJSON_AddStringToObject(cData, "preTradeNo", "");
        cJSON_AddStringToObject(cData, "tradeNo",    "");
    }

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [28] 交易记录确认(设备无需应答,暂时预留) orderCheckSrv (Part4 §6.5.3)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvOrderCheckReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "Result", 0);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [29] 日志查询应答 queDataSrv (Part4 §5.8.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvQueDataReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
    char startDateStr[12] = {0};
    char stopDateStr[12] = {0};

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddNumberToObject(cData, "gunNo", port + 1);
    snprintf(startDateStr, sizeof(startDateStr), "%u", (unsigned int)(pOfflineClr->logQueryStartDate >= SSTM_BASE_TIMESTAMP_1970_BJT ? 
                                (pOfflineClr->logQueryStartDate - SSTM_BASE_TIMESTAMP_1970_BJT) : 0));
    cJSON_AddStringToObject(cData, "startDate", startDateStr);
    snprintf(stopDateStr, sizeof(stopDateStr), "%u", (unsigned int)(pOfflineClr->logQueryStopDate >= SSTM_BASE_TIMESTAMP_1970_BJT ? 
                                (pOfflineClr->logQueryStopDate - SSTM_BASE_TIMESTAMP_1970_BJT) : 0));
    cJSON_AddStringToObject(cData, "stopDate", stopDateStr);
    cJSON_AddNumberToObject(cData, "askType", pOfflineClr->logQueryAskType);
    cJSON_AddNumberToObject(cData, "result", pOfflineClr->srvQueDataResult);
    cJSON_AddStringToObject(cData, "logQueryNo", pOfflineClr->logQueryNo);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [30] 设备维护应答 devMaintainCtrlSrv (Part4 §5.9.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvDevMaintainCtrlReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "ctrlType", pOfflineClr->devMaintainCtrlType);
    cJSON_AddNumberToObject(cData, "reason", pOfflineClr->devMaintainReason);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [31] 维护状态查询应答 devMaintainQuerySrv (Part4 §5.9.4)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvDevMaintainQueryReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "ctrlType", pOfflineClr->devMaintainCtrlType);
    cJSON_AddNumberToObject(cData, "result",   pOfflineClr->devMaintainQueryResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [32] 交易记录召测应答 tradeRecordAskSrv (Part4 §5.12.1.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvTradeRecordAskReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "gunNo",      port + 1);
    cJSON_AddNumberToObject(cData, "askType",    pOfflineClr->srvTradeAskType);
    cJSON_AddStringToObject(cData, "preTradeNo", pOfflineClr->srvTradeAskPreTradeNo);
    cJSON_AddStringToObject(cData, "tradeNo",    pOfflineClr->srvTradeAskTradeNo);
    cJSON_AddStringToObject(cData, "startDate",  pOfflineClr->srvTradeAskStartDate);
    cJSON_AddStringToObject(cData, "stopDate",   pOfflineClr->srvTradeAskStopDate);
    cJSON_AddNumberToObject(cData, "askResult",  pOfflineClr->srvTradeAskResult);
    cJSON_AddNumberToObject(cData, "tradeCnt",   pOfflineClr->srvTradeAskCnt);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [33] 电表底值召测应答 meterRecordAskSrv (Part4 §5.12.2.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvMeterRecordAskReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "gunNo",     port + 1);
    cJSON_AddStringToObject(cData, "askDate",   pOfflineClr->srvMeterAskDate);
    cJSON_AddNumberToObject(cData, "askType",   pOfflineClr->srvMeterAskType);
    cJSON_AddNumberToObject(cData, "askResult", pOfflineClr->srvMeterAskResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [34] 时间同步应答 timeSyncSrv (Part4 §5.13.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvTimeSyncReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "Result", pIotGWECtx->stProtoData.stRecvData[port].offlineClearData.timeSyncResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [35] 有序充电应答 acOrderlyChargeSrv (Part4 §7.2.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvAcOrderlyChargeReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);

    cJSON_AddStringToObject(cData, "preTradeNo", pOfflineClr->srvOrderlyChargePreTradeNo);
    cJSON_AddNumberToObject(cData, "reason",     pOfflineClr->srvOrderlyChargeReason);
    cJSON_AddNumberToObject(cData, "result",     pOfflineClr->srvOrderlyChargeResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [36] 预约充电应答 rsvChargeSrv (Part4 §6.7.2)
 ******************************************************************************
 */
static uint16_t IotGWE_SendSrvRsvChargeReply(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL, *cData = NULL;
    IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

    cRoot = IotGWE_CreateServiceReplyRoot(&cData);
    IOT_GWE_CheckObjIsNull(cRoot, 0);
    (void)port;

    cJSON_AddNumberToObject(cData, "gunNo",      pOfflineClr->srvRsvChargeGunNo);
    cJSON_AddNumberToObject(cData, "appoMethod", pOfflineClr->srvRsvChargeAppoMethod);
    cJSON_AddNumberToObject(cData, "result",     pOfflineClr->srvRsvChargeResult);

    return IotGWE_SerializeJson(cRoot, pBuf);
}
/**
 ******************************************************************************
 * @brief  [37] OTA进度上报
 ******************************************************************************
 */
static uint16_t IotGWE_SendOtaProgress(uint8_t port, void *pBuf)
{
    cJSON *cRoot = NULL;
    const IotGWEOtaStepDesc_Struct *pEntry;
    int8_t step;
    char idStr[12] = {0};
    uint16_t ret = 0;

    do {

        cRoot = cJSON_CreateObject();
        if (cRoot == NULL)
        {
            break;
        }

        snprintf(idStr, sizeof(idStr), "%u", pIotGWECtx->curMsgId);
        cJSON_AddStringToObject(cRoot, "id", idStr);

        if (SSUcm_IsUpdating() == TRUE)
        {
            pEntry = NULL;
            step = (int8_t)SSUcm_GetProgress();
        }
        else
        {
            pEntry = &c_stIotGWEOtaStepDescTable[pIotGWECtx->otaEndResult];
            step = pEntry->step;
        }

        IOTGWE_CFG_DebugPrint("[%s] step = %d\r\n", __FUNCTION__, step);

        cJSON_AddNumberToObject(cRoot, "step", step);
        cJSON_AddStringToObject(cRoot, "desc",
            (pEntry != NULL) ? pEntry->desc : ((step >= 100) ? "success" : "downloading"));
        cJSON_AddStringToObject(cRoot, "module", "MCU");

    } while (0);

    if (cRoot != NULL)
    {
        ret = IotGWE_SerializeJson(cRoot, pBuf);
    }

    return ret;
}
/**
 ******************************************************************************
 * @brief  发布Topic组装: 将格式串中的%s替换为productKey/deviceName
 ******************************************************************************
 */
static void IotGWE_PackTopic(char *pFmtTopic, char *pIdentifier, char *pOutTopic)
{
    if (pFmtTopic != NULL && pOutTopic != NULL)
    {
        snprintf(pOutTopic, CDD_NETM_CFG_MQTT_TOPIC_LEN + 1, pFmtTopic, pIotGWECtx->productKey, 
            pIotGWECtx->deviceName, (pIdentifier != NULL) ? pIdentifier : "");
    }
}

/**
 ******************************************************************************
 * @brief  检查循环发送是否达到时间
 ******************************************************************************
 */
uint8_t IotGWE_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t sendCyc)
{
    uint32_t startTick = Common_GetSendTick(pIotGWECtx->pFuncSendCtrl, port, cmd);
    uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, cmd);

    return (sendImmdFlag == TRUE || startTick == 0 || Common_JudgeTimeoutMs(startTick, sendCyc));
}

/**
 ******************************************************************************
 * @brief  上行发送处理 - 遍历发送控制表,组包并推入MQTT发送队列
 ******************************************************************************
 */
void IotGWE_UpCtrlSendDeal(void)
{
    const IotGWESendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint32_t reqSeq = 0;
    uint16_t dataLen = 0;
    char txBuf[IOT_GWE_TXRX_BUFFER_SIZE] = { 0 };
    char cTopic[CDD_NETM_CFG_MQTT_TOPIC_LEN + 1] = { 0 };

    if (pIotGWECtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotGWECtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotGWECtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotGWECtx->sendIndex < ARRAY_SIZE(c_stIotGWESendctrlTable))
            {
                index = pIotGWECtx->sendIndex;
                port = pIotGWECtx->sendPort;

                pCmdSendCtrl = &c_stIotGWESendctrlTable[index];

                if ((Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (IotGWE_ReportCycleCheck(port, pCmdSendCtrl->cmd, pCmdSendCtrl->sendCycle) == TRUE))
                {
                    if (pCmdSendCtrl->cmdType == IOT_GWE_CMDTYPE_REQUSET)
                    {/* 请求报文 */
                        reqSeq = pIotGWECtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_GWE_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotGWECtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotGWECtx->reqSeq++;
                    }
                    else
                    {/* 应答报文 */
                        reqSeq = Common_GetRecvSeq(pIotGWECtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                    }

                    IotGWE_PackTopic(pCmdSendCtrl->topic, pCmdSendCtrl->identifier, cTopic);

                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {/* 调用对应报文打包接口 */
                        pIotGWECtx->curMsgId = reqSeq;
                        dataLen = pCmdSendCtrl->pSendFunc(port, txBuf);
                    }

                    if (dataLen > 0)
                    {
                        pIotGWECtx->queueBusyFlag = TRUE;
                        pIotGWECtx->waitQueueIdleTick = Common_GetSystick();

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotGWECtx->frameQueueChannelID, cTopic, strlen(cTopic), (uint8_t *)txBuf, dataLen, 1))
                        {
                            if (pCmdSendCtrl->cmdType == IOT_GWE_CMDTYPE_REQUSET)
                            {
                                pIotGWECtx->reqSeq--;
                            }
                            IOTGWE_CFG_DebugPrint("FrameQueue_PushTx fail...[cmd: 0x%03X][dataLen = %d]\r\n", pCmdSendCtrl->cmd, dataLen);
                            break;
                        }

                        IOTGWE_CFG_DebugPrint("%s, %s\r\n", cTopic, txBuf);
                        extern size_t xPortGetFreeHeapSize( void );
                        IOTGWE_CFG_DebugPrint("Remaining heap size: %zu bytes\r\n", xPortGetFreeHeapSize());

                        Common_SetSendFlag(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());

                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        if (pCmdSendCtrl->cmdType == IOT_GWE_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_GWE_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotGWECtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
                    }
                    else
                    {/* pSendFunc返回0: 无数据需发送, 禁用该条目避免无限重试 */
                        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                    }
                }
            }

            pIotGWECtx->sendIndex++;

            if (pIotGWECtx->sendIndex >= ARRAY_SIZE(c_stIotGWESendctrlTable))
            {
                pIotGWECtx->sendIndex = 0;
                pIotGWECtx->sendPort++;

                if (pIotGWECtx->sendPort >= SYSCFG_CFG_GUN_NUM)
                {
                    pIotGWECtx->sendPort = 0;
                    break;
                }
            }

            if (pIotGWECtx->queueBusyFlag == TRUE)
            {
                break;
            }
        }
    }
}

