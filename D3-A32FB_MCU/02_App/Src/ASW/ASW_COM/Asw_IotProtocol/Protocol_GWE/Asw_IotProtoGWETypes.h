/******************************************************************************
* File Name          : Asw_IotProtoGWETypes.h
* Description        : 国网e充电协议类型定义
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/05/22      V1.0.0      hzb        初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_GWE_TYPES_H_
#define ASW_IOT_PROTO_GWE_TYPES_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "cJSON.h"
#include "myMalloc.h"
#include "Ms_Nvm.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define IOTGWE_BULL_VERNDOR                     1187                /* 厂商代码 */

#define IOTGWE_DEVICE_TYPE_CODE                 "02"                /* 设备类型编码: 02=交流桩 */
#define IOTGWE_VERSION_LEN                      32                  /* 版本字符串最大长度 */
/* 构建版本: v{厂商代码}{设备类型}{软件版本(去V前缀)} -> "v1187021.1.0.3" */
#define IOTGWE_MAKE_SOFT_VERSION(buf, size)      do {  \
    const char *_ver = APP_SW_VERSION_STRING;     \
    if (_ver[0] == 'V' || _ver[0] == 'v') _ver++; \
    snprintf(buf, size, "v%d" IOTGWE_DEVICE_TYPE_CODE "%s", IOTGWE_BULL_VERNDOR, _ver); \
} while(0)

#define IOTGWE_MAKE_HARD_VERSION(buf, size)      do {  \
    const char *_ver = "V1.0.0.0";     \
    if (_ver[0] == 'V' || _ver[0] == 'v') _ver++; \
    snprintf(buf, size, "v%d" IOTGWE_DEVICE_TYPE_CODE "%s", IOTGWE_BULL_VERNDOR, _ver); \
} while(0)

#define IOTGWE_HEATBEAT_PERIOD                  120                 /* 心跳周期, unit: 秒 */

/* 通信buff缓存定义 */
#define IOT_GWE_TXRX_BUFFER_SIZE                (3072U)

/* 日志接口函数定义 */
#define IOTGWE_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTGWE_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/*
 * 5.4.1.2 配置更新事件参数表
 */
#define IOTGWE_CFG_EQUIP_PARAM_FREQ_DEFAULT      600                /* 设备实时监测属性上报频率, unit: 秒 */
#define IOTGWE_CFG_GUN_ELEC_FREQ_DEFAULT         90                 /* 充电中实时监测属性上报频率, unit: 秒 */
#define IOTGWE_CFG_NON_ELEC_FREQ_DEFAULT         180                /* 空闲中实时监测属性上报频率, unit: 秒 */
#define IOTGWE_CFG_FAULT_WARNINGS_DEFAULT        600                /* 故障警告全信息上传频率, unit: 秒 */
#define IOTGWE_CFG_OFFLIN_CHALEN_DEFAULT         5                  /* 离线后可充电时长, unit: 分 */
#define IOTGWE_CFG_S2CLOSE_TIME_DEFAULT          2                  /* 交流启动S2闭合超时, unit: 分 */
#define IOTGWE_CFG_S2OPEN_TIME_DEFAULT           10                 /* 交流充电中S2长时间断开超时, unit: 分 */

/* 充电启动后快速上报: 前1分钟内每5s上报 acGunRunItyData, 之后按 gunElecFreq */
#define IOTGWE_CFG_ACWORK_FAST_REPORT
#define IOTGWE_ACWORK_FAST_DURATION_MS           (60 * 1000)        /* 快速上报持续时长 */
#define IOTGWE_ACWORK_FAST_CYCLE_MS              (5 * 1000)         /* 快速上报周期 */

#define IOTGWE_METERRECORD_ASKTIME               (7 * 86400)        /* 电表底值最大查询时间 */

#define IOTGWE_RUNLOG_UPLOAD_CHUNK               1024               /* 日志上传每条大小(栈空间, 不要设置太大) */
#define IOTGWE_RUNLOG_UPLOAD_MAXCNT              100                /* 日志上传最多条数 */
#define IOTGWE_RUNLOG_UPLOAD_TOTAL               (IOTGWE_RUNLOG_UPLOAD_CHUNK * IOTGWE_RUNLOG_UPLOAD_MAXCNT)

#define IOTGWE_OTA_TIMEOUT                       (10 * 60 *1000UL)  /* OTA升级超时时间 */
#define IOTGWE_REGDEV_TIMEOUT_MS                 (30000U)           /* HTTP注册设备超时, unit: ms */

/* 功能配置 (SDK: dev_fun_config, 掉电不存储) */
typedef struct
{
    uint8_t  allowNoFeeModelStart;                                  /* 无计费模型允许充电: 1=允许, 0=禁止 */
    uint16_t allowPWMMax;                                           /* PWM最大占空比(单位%, 一位小数): 100-533 */
    uint16_t allowTempWarning;                                      /* 过温告警限值(两位小数) */
    uint16_t allowTempFault;                                        /* 过温故障限值(两位小数) */
    char     otaInf[256];                                           /* 升级模块固件信息 */
} IotGWEFunConfig_Struct;

/* 协议CMD类型 */
#define IOT_GWE_CMDTYPE_REQUSET                  (0x00U)
#define IOT_GWE_CMDTYPE_RESPONSE                 (0x01U)
#define IOT_GWE_CMD_NULL                         (0xFFFFU)

/* 超时重发: maxTryCnt=0xFFFF 表示无限重试 */
#define IOT_GWE_MAX_TRY_CNT_INFINITE             (0xFFFFU)

/*
 * 协议Topic定义 - 国网e充电MQTT主题
 */
/* ---- 订阅Topic(收): 平台 → 设备, 使用 + 通配符减少订阅数量 ---- */
#define IOT_GWE_SUB_OTA_UPGRADE                  "/ota/device/upgrade/%s/%s"
#define IOT_GWE_SUB_OTA_FIRMWARE_REPLY           "/sys/%s/%s/thing/ota/firmware/get_reply"
#define IOT_GWE_SUB_NTP_RESPONSE                 "/ext/ntp/%s/%s/response"

#define IOT_GWE_SUB_PROPERTY_POST_REPLY          "/sys/%s/%s/thing/event/property/post_reply"
#define IOT_GWE_SUB_PROPERTY_SET                 "/sys/%s/%s/thing/service/property/set"
#define IOT_GWE_SUB_SERVICE                      "/sys/%s/%s/thing/service/+"                   /* 订阅所有服务调用 */
#define IOT_GWE_SUB_EVENT_REALY                  "/sys/%s/%s/thing/event/+/post_reply"

/* ---- 发布Topic(发): 设备 → 平台, 不可用通配符, 第3个%s填充具体identifier ---- */
#define IOT_GWE_PUB_FWINFO                       "/ota/device/inform/%s/%s"
#define IOT_GWE_PUB_OTA_PROGRESS                 "/ota/device/progress/%s/%s"
#define IOT_GWE_PUB_OTA_FIRMWARE_GET             "/sys/%s/%s/thing/ota/firmware/get"
#define IOT_GWE_PUB_NTP_REQUEST                  "/ext/ntp/%s/%s/request"
#define IOT_GWE_PUB_PROPERTY_POST                "/sys/%s/%s/thing/event/property/post"
#define IOT_GWE_PUB_PROPERTY_SET_REPLY           "/sys/%s/%s/thing/service/property/set_reply"
#define IOT_GWE_PUB_SERVICE_REPLY                "/sys/%s/%s/thing/service/%s_reply"            /* 服务应答, %s=identifier */
#define IOT_GWE_PUB_EVENT                        "/sys/%s/%s/thing/event/%s/post"               /* 事件上报, %s=eventId */


/* Topic前缀,用于接收分发匹配 */
#define IOT_GWE_PRE_PROPERTY_POST_REPLY          "thing/event/property/post_reply"
#define IOT_GWE_PRE_EVENT                        "thing/event/"                                 /* 匹配所有事件 */
#define IOT_GWE_PRE_PROPERTY_SET                 "thing/service/property/set"                   /* 属性设置(必须在PRE_SERVICE之前) */
#define IOT_GWE_PRE_SERVICE                      "thing/service/"                               /* 匹配所有服务调用 */
#define IOT_GWE_PRE_OTA_UPGRADE                  "ota/device/upgrade"                           /* OTA升级 */
#define IOT_GWE_PRE_OTA_FIRMWARE_REPLY           "thing/ota/firmware/get_reply"                 /* OTA固件应答 */
#define IOT_GWE_PRE_NTP_RESPONSE                 "ext/ntp"                                      /* NTP应答 */

/*
 * 协议CMD定义 - 内部标识符, 不在MQTT/JSON报文中出现
 *
 * 编号规则: bit15=0 设备→平台, bit15=1 平台→设备
 *           应答CMD = IOT_GWE_RSP(发送CMD) = 0x8000 | cmd
 *
 * 范围:
 *   0x0001-0x0013  设备->平台 (事件/属性上报)
 *   0x0101-0x0111  设备->平台 (服务应答)
 *   0x8001-0x8013  平台->设备 (事件/属性应答)
 *   0x8101-0x8113  平台->设备 (服务下发/属性设置/OTA)
 */

/* 应答CMD辅助宏: 发送CMD → 对应的接收CMD */
#define IOT_GWE_RSP(cmd)                         (0x8000U | (cmd))

/* ---- 发送CMD(设备→平台) ---- */
/* 登录初始化(上线主动上报) */
#define IOT_GWE_CMD_FIRMWARE_INFO_REQ            (0x0001U)                  /* 设备固件信息 firmwareTwEvt */
#define IOT_GWE_CMD_VER_INFO_REQ                 (0x0002U)                  /* 版本信息 verInfoEvt */
#define IOT_GWE_CMD_DEVMDU_INFO_REQ              (0x0003U)                  /* 设备组件信息 devMduInfoEvt */
#define IOT_GWE_CMD_ASK_DEV_CONFIG_REQ           (0x0004U)                  /* 请求设备配置 askConfigEvt */
#define IOT_GWE_CMD_ASK_FEEMODEL_REQ             (0x0005U)                  /* 请求计费模型 askFeeModelTwEvt */
/* 时钟同步(NTP应答后触发, 非登录上报) */
#define IOT_GWE_CMD_TIME_SYNC_RESULT_REQ         (0x0006U)                  /* 时钟同步结果 timeSyncRetEvt */

/* 充电流程 */
#define IOT_GWE_CMD_START_CHA_RES_REQ            (0x0007U)                  /* 启动充电结果 startChaResEvt */
#define IOT_GWE_CMD_START_CHARGE_AUTH_REQ        (0x0008U)                  /* 启动充电鉴权 startChargeAuthEvt */
#define IOT_GWE_CMD_STOP_CHA_RES_REQ             (0x0009U)                  /* 停止充电结果 stopChaResEvt */
#define IOT_GWE_CMD_ORDER_TW_UPDATE_REQ          (0x000AU)                  /* 交易记录上报 orderTwUpdateEvt */

/* 故障/状态/维护 */
#define IOT_GWE_CMD_TOTAL_FAULT_REQ              (0x000BU)                  /* 故障告警 totalFaultEvt */
#define IOT_GWE_CMD_AC_ST_CH_REQ                 (0x000CU)                  /* 交流桩状态变化 acStChEvt */
#define IOT_GWE_CMD_AC_CAR_CON_CH_REQ            (0x000DU)                  /* CP连接状态变化 acCarConChEvt */
#define IOT_GWE_CMD_LOG_QUERY_RESULT_REQ         (0x000EU)                  /* 日志查询结果 logQueryEvt */
#define IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ         (0x000FU)                  /* 设备维护结果 devMaintainRetEvt */

/* 属性上报 */
#define IOT_GWE_CMD_PROPERTY_ACPILE_REQ          (0x0010U)                  /* 交流设备属性 acDeRealItyData */
#define IOT_GWE_CMD_PROPERTY_AC_WORK_REQ         (0x0011U)                  /* 交流充电实时属性 acGunRunItyData */
#define IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ      (0x0012U)                  /* 交流非充电实时属性 acGunIdleItyData */
#define IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ     (0x0013U)                  /* 交流输出电表属性 acOutMeterItyData */

/* 服务应答(设备→平台), 0x0101-0x0111, 低字节对应服务ID */
/* 服务应答CMD: 0x01XX, 低字节与对应服务请求 0x81XX 一致 */
#define IOT_GWE_SRV_REPLY(srvId)                 (0x0100U | ((srvId) & 0xFFU))

#define IOT_GWE_CMD_CONF_UPDATE_SRV_REPLY        IOT_GWE_SRV_REPLY(0x01U)   /*  0x0101, 配置更新应答 */
#define IOT_GWE_CMD_CONF_GET_SRV_REPLY           IOT_GWE_SRV_REPLY(0x02U)   /*  0x0102, 配置获取应答 */
#define IOT_GWE_CMD_FUN_CONF_UPDATE_SRV_REPLY    IOT_GWE_SRV_REPLY(0x03U)   /*  0x0103, 功能配置更新应答 */
#define IOT_GWE_CMD_FUN_CONF_GET_SRV_REPLY       IOT_GWE_SRV_REPLY(0x04U)   /*  0x0104, 功能配置获取应答 */
#define IOT_GWE_CMD_FEE_MODEL_UPDATE_SRV_REPLY   IOT_GWE_SRV_REPLY(0x05U)   /*  0x0105, 计费模型更新应答 */
#define IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY    IOT_GWE_SRV_REPLY(0x06U)   /*  0x0106, 计费模型查询应答 */
#define IOT_GWE_CMD_START_CHARGE_SRV_REPLY       IOT_GWE_SRV_REPLY(0x07U)   /*  0x0107, 远程启动充电应答 */
#define IOT_GWE_CMD_STOP_CHARGE_SRV_REPLY        IOT_GWE_SRV_REPLY(0x08U)   /*  0x0108, 远程停止充电应答 */
#define IOT_GWE_CMD_ORDER_CHECK_SRV_REPLY        IOT_GWE_SRV_REPLY(0x09U)   /*  0x0109, 交易记录确认应答 */
#define IOT_GWE_CMD_QUE_DATA_SRV_REPLY           IOT_GWE_SRV_REPLY(0x0AU)   /*  0x010A, 日志查询应答 */
#define IOT_GWE_CMD_DEV_MAINTAIN_CTRL_SRV_REPLY  IOT_GWE_SRV_REPLY(0x0BU)   /*  0x010B, 设备维护应答 */
#define IOT_GWE_CMD_DEV_MAINTAIN_QUERY_SRV_REPLY IOT_GWE_SRV_REPLY(0x0CU)   /*  0x010C, 维护状态查询应答 */
#define IOT_GWE_CMD_TRADE_RECORD_ASK_SRV_REPLY   IOT_GWE_SRV_REPLY(0x0DU)   /*  0x010D, 交易记录召测应答 */
#define IOT_GWE_CMD_METER_RECORD_ASK_SRV_REPLY   IOT_GWE_SRV_REPLY(0x0EU)   /*  0x010E, 电表底值召测应答 */
#define IOT_GWE_CMD_TIME_SYNC_SRV_REPLY          IOT_GWE_SRV_REPLY(0x0FU)   /*  0x010F, 时间同步应答 */
#define IOT_GWE_CMD_AC_ORDERLY_CHARGE_SRV_REPLY  IOT_GWE_SRV_REPLY(0x10U)   /*  0x0110, 有序充电应答 */
#define IOT_GWE_CMD_RSV_CHARGE_SRV_REPLY         IOT_GWE_SRV_REPLY(0x11U)   /*  0x0111, 预约充电应答 */

#define IOT_GWE_CMD_SEND_COUNT                   (38U)                      /* 19个事件/属性 + 17个服务应答 + 1个OTA进度 + 1个版本上报 */

/* ---- 应答CMD(平台→设备), 0x8001-0x801F, 通过 IOT_GWE_RSP(send_cmd) 得到 ---- */
#define IOT_GWE_CMD_FIRMWARE_INFO_RSP            IOT_GWE_RSP(IOT_GWE_CMD_FIRMWARE_INFO_REQ)
#define IOT_GWE_CMD_VER_INFO_RSP                 IOT_GWE_RSP(IOT_GWE_CMD_VER_INFO_REQ)
#define IOT_GWE_CMD_DEVMDU_INFO_RSP              IOT_GWE_RSP(IOT_GWE_CMD_DEVMDU_INFO_REQ)
#define IOT_GWE_CMD_DEV_CONFIG_RSP               IOT_GWE_RSP(IOT_GWE_CMD_ASK_DEV_CONFIG_REQ)
#define IOT_GWE_CMD_FEEMODEL_RSP                 IOT_GWE_RSP(IOT_GWE_CMD_ASK_FEEMODEL_REQ)
#define IOT_GWE_CMD_TIME_SYNC_RSP                IOT_GWE_RSP(IOT_GWE_CMD_TIME_SYNC_RESULT_REQ)

#define IOT_GWE_CMD_START_CHA_RES_RSP            IOT_GWE_RSP(IOT_GWE_CMD_START_CHA_RES_REQ)
#define IOT_GWE_CMD_START_CHARGE_AUTH_RSP        IOT_GWE_RSP(IOT_GWE_CMD_START_CHARGE_AUTH_REQ)
#define IOT_GWE_CMD_STOP_CHA_RES_RSP             IOT_GWE_RSP(IOT_GWE_CMD_STOP_CHA_RES_REQ)
#define IOT_GWE_CMD_ORDER_TW_UPDATE_RSP          IOT_GWE_RSP(IOT_GWE_CMD_ORDER_TW_UPDATE_REQ)

#define IOT_GWE_CMD_TOTAL_FAULT_RSP              IOT_GWE_RSP(IOT_GWE_CMD_TOTAL_FAULT_REQ)
#define IOT_GWE_CMD_AC_ST_CH_RSP                 IOT_GWE_RSP(IOT_GWE_CMD_AC_ST_CH_REQ)
#define IOT_GWE_CMD_AC_CAR_CON_CH_RSP            IOT_GWE_RSP(IOT_GWE_CMD_AC_CAR_CON_CH_REQ)
#define IOT_GWE_CMD_LOG_QUERY_RESULT_RSP         IOT_GWE_RSP(IOT_GWE_CMD_LOG_QUERY_RESULT_REQ)
#define IOT_GWE_CMD_DEV_MAINTAIN_RET_RSP         IOT_GWE_RSP(IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ)

#define IOT_GWE_CMD_PROPERTY_ACPILE_RSP          IOT_GWE_RSP(IOT_GWE_CMD_PROPERTY_ACPILE_REQ)
#define IOT_GWE_CMD_PROPERTY_AC_WORK_RSP         IOT_GWE_RSP(IOT_GWE_CMD_PROPERTY_AC_WORK_REQ)
#define IOT_GWE_CMD_PROPERTY_AC_NONWORK_RSP      IOT_GWE_RSP(IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ)
#define IOT_GWE_CMD_PROPERTY_AC_OUTMETER_RSP     IOT_GWE_RSP(IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ)

/* ---- 服务CMD(平台→设备下发), 0x8101-0x8113 ---- */
#define IOT_GWE_SRV_CONF_UPDATE                  (0x8101U)                      /* 配置更新 confUpdateCtrlSrv */
#define IOT_GWE_SRV_CONF_GET                     (0x8102U)                      /* 配置获取 getDevConfSrv */
#define IOT_GWE_SRV_FUN_CONF_UPDATE              (0x8103U)                      /* 功能配置更新 funConfUpdateDataSrv */
#define IOT_GWE_SRV_FUN_CONF_GET                 (0x8104U)                      /* 功能配置获取 getFunConfSrv */
#define IOT_GWE_SRV_FEE_MODEL_UPDATE             (0x8105U)                      /* 计费模型更新 feeModelTwUpdateSrv */
#define IOT_GWE_SRV_FEE_MODEL_QUERY              (0x8106U)                      /* 计费模型查询 feeModelTwQuerySrv */
#define IOT_GWE_SRV_START_CHARGE                 (0x8107U)                      /* 远程启动充电 startChargeSrv */
#define IOT_GWE_SRV_STOP_CHARGE                  (0x8108U)                      /* 远程停止充电 stopChargeSrv */
#define IOT_GWE_SRV_ORDER_CHECK                  (0x8109U)                      /* 交易记录确认 orderCheckSrv */
#define IOT_GWE_SRV_QUE_DATA                     (0x810AU)                      /* 日志查询 queDataSrv */
#define IOT_GWE_SRV_DEV_MAINTAIN_CTRL            (0x810BU)                      /* 设备维护 devMaintainCtrlSrv */
#define IOT_GWE_SRV_DEV_MAINTAIN_QUERY           (0x810CU)                      /* 维护状态查询 devMaintainQuerySrv */
#define IOT_GWE_SRV_TRADE_RECORD_ASK             (0x810DU)                      /* 交易记录召测 tradeRecordAskSrv */
#define IOT_GWE_SRV_METER_RECORD_ASK             (0x810EU)                      /* 电表底值召测 meterRecordAskSrv */
#define IOT_GWE_SRV_TIME_SYNC                    (0x810FU)                      /* 时间同步 timeSyncSrv */
#define IOT_GWE_SRV_AC_ORDERLY_CHARGE            (0x8110U)                      /* 有序充电 acOrderlyChargeSrv */
#define IOT_GWE_SRV_RSV_CHARGE                   (0x8111U)                      /* 预约充电 rsvChargeSrv */

/* OTA/NTP/PropertySet 接收CMD */
#define IOT_GWE_CMD_PROPERTY_SET_RECV            (0x8112U)                      /* 属性设置 property.set */
#define IOT_GWE_CMD_OTA_UPGRADE_RECV             (0x8113U)                      /* OTA升级通知 */
#define IOT_GWE_CMD_OTA_FIRMWARE_REPLY_RECV      (0x8014U)                      /* OTA固件信息应答 */
#define IOT_GWE_CMD_NTP_RESPONSE_RECV            (0x8015U)                      /* NTP时间应答 */

#define IOT_GWE_CMD_OTA_PROGRESS_REQ            (0x0014U)                       /* OTA进度上报 */
#define IOT_GWE_CMD_FWINFO_REQ                  (0x0015U)                       /* OTA固件版本上报 */

#define IOT_GWE_CMD_DEV_CONFG_UPDATE_SRV_RECV   (0x8118U)                       /* 设备配置更新 devConfgUpdateSrv */

#define IOT_GWE_CMD_RECV_COUNT                   (41U)                          /* 19个应答 + 18个服务 + 4个OTA/NTP */

/* 通用配置宏 */
#define IOT_GWE_SDK_VERSION                     "SDK_V1.1.10"
#define IOT_GWE_CT_RATIO                        1U                              /* 电流互感器系数 */
#define IOT_GWE_TRADE_NO_LEN                    41U                             /* 交易流水号最大长度 */
#define IOT_GWE_OTMINVOL                        2000                            /* 输出最小电压 */
#define IOT_GWE_OTMAXVOL                        2400                            /* 输出最大电压 */

#define IOT_GWE_METERRECORD_PREMAX              64                              /* 设备日志获取(或电表底值召测)单次最多上报条数 */
#define IOT_GWE_FAULTRECORD_PREMAX              64                              /* 设备日志获取故障告警记录单次最多上报条数 */
#define IOT_GWE_ORDERRECORD_PREMAX              64                              /* 设备日志获取交易记录单次最多上报条数 */

#define IOT_GWE_PROILEPOWER_DEFAULT         (SYSCFG_CFG_MAX_OUTPUT_POWER / 100) /* 默认功率, 单位0.1kW */
#define IOT_GWE_KW_TO_W(kw)                 ((uint32_t)(kw) * 100U)             /* 0.1kW → W */

#define IOTGWE_STOPREASON_POWEROFF_MARK     (0xFFU)                             /* 掉电-停止原因 */

/* JSON辅助宏 */
#define IOT_GWE_CheckKeyIsNull(key, keyName, ret)                   \
{                                                                   \
    if (key == NULL)                                                \
    {                                                               \
        cJSON_Delete(cRoot);                                        \
        IOTGWE_CFG_DebugPrint("[%s()]: key[%s] not found\r\n",      \
            __FUNCTION__, keyName);                                 \
        return ret;                                                 \
    }                                                               \
}

#define IOT_GWE_CheckObjIsNull(obj, ret)                            \
{                                                                   \
    if (obj == NULL)                                                \
    {                                                               \
        cJSON_Delete(cRoot);                                        \
        IOTGWE_CFG_DebugPrint("[%s()]: create obj failed\r\n",      \
            __FUNCTION__);                                          \
        return ret;                                                 \
    }                                                               \
}

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
/* 桩类型 */
typedef enum{

    eIotGWEDevType_SPAC = 10,       /* 单相交流 */
    eIotGWEDevType_TPAC,            /* 三相交流 */
    eIotGWEDevType_DC               /* 直流 */

}IotGWEDevType_Enum;

/* 坐标类型 */
typedef enum{

    eIotGWEGridType_None = 10,      /* 无经纬度坐标值 */
    eIotGWEGridType_GPS,            /* GPS坐标 */
    eIotGWEGridType_BD,             /* 北斗坐标 */
    eIotGWEGridType_GLONASS,        /* GLONASS坐标 */
    eIotGWEGridType_Galileo,        /* 伽利略坐标 */

}IotGWEGridType_Enum;

/* 计量方式 */
typedef enum{

    eIotGWEMeaType_DCEnergy = 10,   /* 直流电能表 */
    eIotGWEMeaType_ACEnergy,        /* 交流电能表 */
    eIotGWEMeaType_DCMeter,         /* 直流计量模块 */
    eIotGWEMeaType_ACMeter,         /* 交流计量模块 */
    eIotGWEMeaType_ACMeterIC,       /* 交流计量芯片 */

}IotGWEMeaType_Enum;

/* 是否有智能门锁 */
typedef enum{

    eIotGWEIsGateLock_NO = 10,      /* 无智能门锁 */
    eIotGWEIsGateLock_HAS,          /* 有智能门锁 */

}IotGWEIsGateLock_Enum;

/* 是否有智能门锁 */
typedef enum{

    eIotGWEIsGroundLock_NO = 10,     /* 无地锁 */
    eIotGWEIsGroudLock_HAS,          /* 有地锁 */

}IotGWEIsGroudLock_Enum;

/* 是否支持多时段类型计费模型 */
typedef enum{

    eIotGWEMutliChargeMode_OK = 10, /* 支持多时段计费模型，即无计费时段标识, 
                                        最大计费计费时段为96个区间 */
    eIotGWEMutliChargeMode_NO,      /* 不支持多时段计费模型, 计费时段标识仅为: 尖峰平谷*/

}IotGWEMutliChargeMode_Enum;

/* 设备注册方式 */
typedef enum{

    eIotGWEDevRegMethod_LCD = 10,   /* 屏幕 */
    eIotGWEDevRegMethod_BLE,        /* 蓝牙 */
    eIotGWEDevRegMethod_UART,       /* 串口 */
    eIotGWEDevRegMethod_NFC,        /* NFC */
    eIotGWEDevRegMethod_USB,        /* USB外设 */
    eIotGWEDevRegMethod_NOWR,       /* 本次使用非写入方式注册 */
    eIotGWEDevRegMethod_OTHER,      /* 其他 */
    
}IotGWEDevRegMethod_Enum;

/* 功能是否支持 */
typedef enum{

    eIotGWEFunciton_OK = 10,        /* 支持 */
    eIotGWEFunciton_NO,             /* 不支持 */

}IotGWEFunction_Enum;

/* 启动(充电)结果 */
typedef enum{

    eIotGWEStartResult_Success = 10,    /* 成功 */
    eIotGWEStartResult_Fault,           /* 充电设备故障启动充电失败 */
    eIotGWEStartResult_StartTimeOut,    /* 等待启动充电超时 */
    eIotGWEStartResult_EVDISCONN,       /* 车桩连接失败 */
    eIotGWEStartResult_Charging,        /* 正在充电中 */
    eIotGWEStartResult_Fail,            /* 失败 */

}IotGWEStartResult_Enum;

/* OTA升级状态 */
typedef enum{
    eIotGWEOTAState_Idle,
    eIotGWEOTAState_Starting,
} IotGWEOTAState_Enum;

/* 同步时钟结果 */
typedef enum{

    IotGWESyncTimeRet_Success = 10,         /* 同步成功 */
    IotGWESyncTimeRet_FailNoAllow,          /* 同步失败, 当前状态不允许 */
    IotGWESyncTimeRet_FailSet,              /* 同步失败,   系统时间修改失败 */

}IotGWESyncTimeRet_Enum;

/* 计量计费类型 */
typedef enum{

    eIotGWETimeDivType_MeterAndBill = 10,   /* 计量计费 */
    eIotGWETimeDivType_MeterOnly,           /* 仅计量不计费 */
    eIotGWETimeDivType_None,                /* 无计量计费 */

}IotGWETimeDivType_Enum;

/* 启动方式 */
typedef enum{

    eIotGWEStartType_APP = 10,              /* app一键启动 */
    eIotGWEStartType_VIN,                   /* VIN即插即充 */
    eIotGWEStartType_SMART,                 /* 智能枪启动 */
    eIotGWEStartType_QRCODE,                /* 二维码启动 */
    eIotGWEStartType_CARD,                  /* 桩侧扫码启动 */
    eIotGWEStartType_PLAT,                  /* 平台启动 */
    eIotGWEStartType_BLE,                   /* 蓝牙启动 */
    eIotGWEStartType_VINOffline,            /* 离线VIN即插即充 */

}IotGWEStartType_Enum;

/* 充电枪连接状态 */
typedef enum{

    eIotGWEConnStatus_Connect = 10,         /* 已连接 */
    eIotGWEConnStatus_DisConn,              /* 未连接 */

}IotGWEConnStatus_Enum;

/* CP状态 */
typedef enum{

    eIotGWECPStatus_12V = 10,
    eIotGWECPStatus_9V,
    eIotGWECPStatus_6V,
    eIotGWECPStatus_Other,

}IotGWECPStatus_Enum;

/* S3状态 */
typedef enum{

    eIotGWES3Status_No = 0,                 /* 无法获取(或无) */
    eIotGWES3Status_Press = 10,             /* 按钮按下 */
    eIotGWES3Status_Release,                /* 按钮松开 */

}IotGWES3Status_Enum;

/* 设备日志查询类型 */
typedef enum{

    eIotGWELogAskType_Order = 10,           /* 交易记录 */
    eIotGWELogAskType_MeterStart,           /* 电表底值 */
    eIotGWELogAskType_Log,                  /* 日志 */
    eIotGWELogAskType_FAULT,                /* 故障告警记录 */
    eIotGWELogAskType_BMS,

}IotGWELogAskType_Enum;

/* 交易记录召测服务类型 */
typedef enum{

    eIotGWEOrderAskType_Single = 10,        /* 单条交易记录 */
    eIotGWEOrderAskType_Unreport,           /* 未上送交易记录 */
    eIotGWEOrderAskType_All,                /* 全部交易记录 */

}IotGWEOrderAskType_Enum;

/* 电表底值召测类型 (meterRecordAskSrv.askType) */
typedef enum{

    eIotGWEMeterAskType_Midnight = 10,      /* 零点记录 */
    eIotGWEMeterAskType_NoSync,             /* 未上传记录(TODO: 暂时不支持, 以全部记录上传) */
    eIotGWEMeterAskType_All,                /* 全部记录 */

}IotGWEMeterAskType_Enum;

/* 设备日志查询响应结果 */
typedef enum{

    eIotGWELogResult_NODATA = 10,           /* 无响应数据 */
    eIotGWELogResult_DATA,                  /* 存在响应数据 */
    eIotGWELogResult_NOTALLOW,              /* 当前状态不允许上传日志 */

}IotGWELogResult_Enum;

/* 设备日志查询响应类型 */
typedef enum{

    eIotGWELogRetType_Frame = 10,           /* 报文格式 */
    eIotGWELogRetType_File,                 /* 文件格式 */

}IotGWELogRetType_Enum;

/* 网络类型 */
typedef enum{

    eIotGWENetType_Unknow = 10,
    eIotGWENetType_2G,
    eIotGWENetType_3G,
    eIotGWENetType_4G,
    eIotGWENetType_5G,
    eIotGWENetType_NBIOT,
    eIotGWENetType_WIFI,
    /* ... */

}IotGWENetType_Enum;

/* 网络运营商 */
typedef enum{

    eIotGWENetId_Unknow = 10,
    eIotGWENetId_CUCC,                  /* 联通 */
    eIotGWENetId_CMCC,                  /* 移动 */
    eIotGWENetId_CTCC,                  /* 电信 */
    /* ... */

}IotGWENetId_Enum;

/* 工作状态 */
typedef enum{

    eIotGWEWorkStatus_IDIE = 10,        /* 空闲中 */
    eIotGWEWorkStatus_READY,            /* 已插枪 */
    eIotGWEWorkStatus_STARTING,         /* 启动中 */
    eIotGWEWorkStatus_CHARGING,         /* 充电中 */
    eIotGWEWorkStatus_FINISH,           /* 充电完成未拔枪 */
    eIotGWEWorkStatus_RESERVE,          /* 预约状态 */
    eIotGWEWorkStatus_FAULT,            /* 系统故障 */

}IotGWEWorkStatus_Enum;
    
/* 输出继电器状态 */
typedef enum{

    eIotGWEOutRelayStatus_Open = 10,    /* 分断 */
    eIotGWEOutRelayStatus_Close,        /* 闭合 */

}IotGWEOutRelayStatus_Enum;

/* 充电接口电子锁状态 */
typedef enum{

    eIotGWEELockStatus_UnLock = 10,     /* 解锁 */
    eIotGWEELockStatus_Lock,            /* 锁住 */

}IotGWEELockStatus_Enum;

/* 结果code */
typedef enum{

    eIotGWEResCode_Success = 10,        /* 成功 */
    eIotGWEResCode_Fail,                /* 失败 */

}IotGWEResCode_Enum;

/* 常规停止代码 */
typedef enum{

    eIotGWEStopResultCode_ChagreFull = 1000,    /* 充满停止 */
    eIotGWEStopResultCode_LocalStop,            /* 本地停止充电 */
    eIotGWEStopResultCode_PlatStop,             /* 后台停止充电 */
    eIotGWEStopResultCode_TimeReach,            /* 达到设置充电时长停止 */
    eIotGWEStopResultCode_ElecReach,            /* 达到设置充电电量停止 */
    eIotGWEStopResultCode_MenoyReach,           /* 达到设置充电金额停止 */
    eIotGWEStopResultCode_Offline,              /* 达到离线停机条件 */
    eIotGWEStopResultCode_Soc,                  /* 达到SOC终止条件停止 */
    eIotGWEStopResultCode_GunDisconn,           /* 用户拔枪停止 */
    eIotGWEStopResultCode_AppStop,              /* APP停止充电 */
    eIotGWEStopResultCode_S2Disconn,            /* 车端S2主动断开 */
    eIotGWEStopResultCode_BMS,                  /* BMS停止充电 */
    eIotGWEStopResultCode_AmountBalance,        /* 账户余额不足 */
    eIotGWEStopResultCode_PrePay,               /* 预付金额不足 */
    eIotGWEStopResultCode_S2TimeOut,            /* 车辆S2闭合超时 */
    eIotGWEStopResultCode_S2DisconnTimeOut,     /* 充电中车辆S2断开超时 */
    eIotGWEStopResultCode_Trickle,              /* 达到涓流充电停机条件 */

}IotGWEStopResultCode_Enum;

/* 交流充电设备异常(故障)代码 */
typedef enum{

    eIotGWEFaultCode_None = 0,                  /* 无故障 */

    eIotGWEFaultCode_AccCtrl = 10000,           /* 门禁故障 */
    eIotGWEFaultCode_EmergenBtn,                /* 急停按键 */
    eIotGWEFaultCode_CardReader,                /* 读卡器故障 */
    eIotGWEFaultCode_AmMeterComm,               /* 电表通讯超时故障 */
    eIotGWEFaultCode_AmMeterData,               /* 电表数据异常故障 */
    eIotGWEFaultCode_ContactorStick,            /* 接触器粘连故障 */
    eIotGWEFaultCode_ContactorOn,               /* 接触器吸合故障 */
    eIotGWEFaultCode_Contactor,                 /* 接触器故障 */
    eIotGWEFaultCode_DevTempOverWarn,           /* 设备过温告警 */
    eIotGWEFaultCode_GunTempOverWarn,           /* 接口(枪)过温告警 */
    eIotGWEFaultCode_DevTempOver,               /* 设备过温故障 */
    eIotGWEFaultCode_GunTempOver,               /* 接口(枪)过温故障 */
    eIotGWEFaultCode_ContTempOverWarn,          /* 接触器过温告警 */
    eIotGWEFaultCode_PowerTempOverWarn,         /* 电源模块过温告警 */
    eIotGWEFaultCode_ContTempOver,              /* 接触器过温故障 */
    eIotGWEFaultCode_PowerTempOver,             /* 电源模块过温 */
    eIotGWEFaultCode_OutShort,                  /* 输出短路故障 */
    eIotGWEFaultCode_Arrester,                  /* 避雷器故障 */
    eIotGWEFaultCode_UnRepoerRecordWarn,        /* 未上送交易记录预警 */
    eIotGWEFaultCode_CassBCC,                   /* 枪座异常故障 */
    eIotGWEFaultCode_Leach,                     /* 水浸故障 */
    eIotGWEFaultCode_ElecLock,                  /* 充电接口电子锁故障 */
    eIotGWEFaultCode_FeeModeReqTimeOut,         /* 计费模型请求超时故障 */
    eIotGWEFaultCode_FeeModeIllegal,            /* 计费模型部合法故障 */
    eIotGWEFaultCode_SyncTime,                  /* 对时超时故障 */
    eIotGWEFaultCode_SyncTimeIllegal,           /* 对时实际部合法故障 */
    eIotGWEFaultCode_CtrlPowerOff,              /* 控制回路掉电故障 */
    eIotGWEFaultCode_Dump,                      /* 充电桩倾倒故障 */
    eIotGWEFaultCode_OrderStore,                /* 交易记录存储失败故障 */
    eIotGWEFaultCode_LCDComm,                   /* 显示屏通讯故障 */
    eIotGWEFaultCode_ParaConfigCheck,           /* 参数配置校验错误 */
    eIotGWEFaultCode_ParkLockExcep,             /* 车位锁异常告警 */
    eIotGWEFaultCode_ParkLockBattery,           /* 车位锁电池耗尽告警 */
    eIotGWEFaultCode_ParkLockDone,              /* 车位锁落锁失败告警 */
    eIotGWEFaultCode_LedBoardComm,              /* 灯板通讯告警 */
    eIotGWEFaultCode_UnReprtRecordFull,         /* 未上送交易记录已满 */

    eIotGWEFaultCode_InputPower = 11000,        /* 输入电源故障(过压, 过流, 欠压, 跳闸) */
    eIotGWEFaultCode_CPVol,                     /* CP回路电压超限故障 */
    eIotGWEFaultCode_InputLossPhase,            /* 输入缺相故障 */
    eIotGWEFaultCode_RCD,                       /* 漏电保护故障 */
    eIotGWEFaultCode_PE,                        /* 地线故障 */
    eIotGWEFaultCode_ACInputVolOver,            /* 交流输入电压过压故障  */
    eIotGWEFaultCode_ACInputVolUnder,           /* 交流输入电压故障*/
    eIotGWEFaultCode_OutCurOver,                /* 输出电量超过额定故障 */
    eIotGWEFaultCode_PWMCtrlCurOver,            /* pwm调控输出过流故障 */
    eIotGWEFaultCode_LNReverseConn,             /* 火零线反接 */

}IotGWEFaultCode_Enum;

/* 交直流充电启动失败代码 */
typedef enum{

    eIotGWEStartFailCode_None = 0,              /* 成功 */

    eIotGWEStartFailCode_Charging = 7000,       /* 设备已启动充电或正在中 */
    eIotGWEStartFailCode_Disconn,               /* 设备未连接车辆 */
    eIotGWEStartFailCode_OTAING,                /* 设备升级中 */
    eIotGWEStartFailCode_GunDisconn,            /* 充电枪未插到位 */
    eIotGWEStartFailCode_StartTimeout,          /* 启动充电超时 */
    eIotGWEStartFailCode_AuthTimeout,           /* 鉴权超时 */
    eIotGWEStartFailCode_DevMaintain,           /* 设备停运 */
    eIotGWEStartFailCode_DevStopRun,            /* 设备退运 */
    eIotGWEStartFailCode_DevFreeze,             /* 设备冻结 */
    eIotGWEStartFailCode_PlatParamErr,          /* 平台下发参数不正确 */
    eIotGWEStartFailCode_AuthFail,              /* 鉴权失败 */
    eIotGWEStartFailCode_PlugTimeErr,           /* 插枪时间不一致 */
    eIotGWEStartFailCode_NotSupportTheAuth,     /* 设备不支持或未配置此充电方式 */

}IotGWEStartFailCode_Enum;

/* 设备维护指令类型 */
typedef enum{

    eIotGWECtrlType_Reset = 11,         /* 重启 */
    eIotGWECtrlType_Maintenance,        /* 检修 */
    eIotGWECtrlType_Freeze,             /* 冻结 */
    eIotGWECtrlType_Commission,         /* 投运 */
    eIotGWECtrlType_Suspension,         /* 停运 */
    eIotGWECtrlType_ReturnShip,         /* 退运 */
    eIotGWECtrlType_Factory,            /* 恢复出厂设置 */
    eIotGWECtrlType_HardReset,          /* 硬重启 */
    
}IotGWECtrlType_Enum;

/* 充电模式 */
typedef enum{

    eIotGWEChargeMode_Normal = 10,      /* 不做限制的充电(默认) */
    eIotGWEChargeMode_LimitAmount,      /* 限制金额 */
    eIotGWEChargeMode_LimitElec,        /* 限制电量 */
    eIotGWEChargeMode_LimitSoc,         /* 限制Soc */
    eIotGWEChargeMode_LimitTime,        /* 限制充电时长 */
    eIotGWEChargeMode_LimitPower,       /* 限制功率 */

}IotGWEChargeMode_Enum;

/* 停止原因 */
typedef enum{

    eIotGWEStopReason_APP = 10,         /* APP控制停机，code=1009 */
    eIotGWEStopReason_AmountLack,       /* 账户余额不足，code=1012 */
    eIotGWEStopReason_PrePayConsumed,   /* 预付金额消费完毕, code=1013 */
    eIotGWEStopReason_PlatStopAbnormal, /* 平台监测充电异常, code=1002 */
    eIotGWEStopReason_NotCharging,      /* 充电枪未在充电中, code=1002 */

}IotGWEStopReason_Enum;

/* 停止失败原因 */
typedef enum{

    eIotGWEStopFailReason_TradeNoMisMatch = 10, /* 交易流水号不一致 */
    eIotGWEStopFailReason_NotWork,              /* 已停机 */

}IotGWEStopFailReason_Enum;

/* 设备维护失败原因 */
typedef enum{

    eIotGWEDevMaintainReason_Success = 10,      /* 成功 */
    eIotGWEDevMaintainReason_Charging,          /* 充电中 */
    eIotGWEDevMaintainReason_NoPermiss,         /* 系统权限不足 */
    eIotGWEDevMaintainReason_RepeatRest,        /* 重复重启 */
    eIotGWEDevMaintainReason_NoSupportHardRest, /* 不支持硬重启 */

}IotGWEDevMaintainReason_Enum;

/* 有序充电失败原因 */
typedef enum{

    eIotGWEPoileReason_Success = 10,            /* 无(成功) */
    eIotGWEPoileReason_PowerOutRange,           /* 功率值超出范围 */
    eIotGWEPoileReason_StopStatus,              /* 充电设备停止充电 */
    eIotGWEPoileReason_ParamInvaild,            /* 参数无效 */

}IotGWEPoileReason_Enum;

typedef uint16_t (*IotGWE_pSendPackFuncType)(uint8_t port, void *pBuf);
typedef uint8_t  (*IotGWE_pRecvParseFuncType)(uint8_t port, uint8_t *r_data, uint16_t len);

/* 发送控制表项 */
typedef struct
{
    char *topic;                            /* 发布Topic */
    char *identifier;                       /* 事件/服务identifier */
    uint16_t cmd;                           /* 发送命令 */
    uint8_t cmdType;                        /* 发送命令类型: REQUSET or RESPONSE */
    uint32_t sendCycle;                     /* 循环周期(ms), 0=单次发送后自禁 */
    IotGWE_pSendPackFuncType pSendFunc;     /* 发送组包回调 */
    uint16_t matchCmd;                      /* 对应RecvCtrl的CMD, 请求-应答配对 */
    char *cMeaning;                         /* 描述 */
} IotGWESendCtrl_Struct;

/* 接收控制表项 */
typedef struct
{
    uint16_t cmd;                           /* 接收命令 */
    char *matchStr;                         /* 接收匹配字符 */
    uint8_t cmdType;                        /* 接收命令类型: REQUSET or RESPONSE */
    IotGWE_pRecvParseFuncType pRecvParse;   /* 命令接收解析接口 */
    uint16_t maxTimeout;                    /* 超时时长(ms), 0=不检测超时 */
    uint16_t maxTryCnt;                     /* 超时重试次数, 0xFFFF=无限 */
    uint16_t matchCmd;                      /* 对应SendCtrl的CMD, 请求-应答配对 */
    char *cMeaning;                         /* 描述 */
} IotGWERecvCtrl_Struct;

/* 接收Topic分组表项 */
typedef struct
{
    char *topic;
    uint8_t memberCnt;
    uint8_t cmdType;
    IotGWERecvCtrl_Struct *pStrRecvCtrlTable;
} IotGWERecvTopic_Struct;

/* 有序充电策略缓存(RAM) */
typedef struct
{
    char preTradeNo[41];                    /* 订单流水号 */
    uint8_t num;                            /* 策略配置时间段数量 */
    char validTime[96][5];                  /* 策略生效时间, 字符串数组, HHMM格式 */
    uint16_t kw[96];                        /* 策略配置功率, 单位0.1kW */
} IotGWEOrderlyChargeStrategy_Struct;

/* 离线清除数据 */
typedef struct
{
    uint32_t platTimestamp;                 /* 平台下发的服务器时间戳 */
    uint8_t  timeSyncResult;                /* 校时处理结果, 10=成功 */
    uint8_t  startResult;                   /* 启动充电结果, 10=成功, 15=失败 */
    uint16_t startFaultCode;                /* 启动充电失败原因码, GWE协议故障码 */
    /* 启动失败订单缓存 */
    uint8_t  startFailOrderActive;          /* 1=有待上报的失败订单 */
    uint8_t  startFailChargeActive;         /* 1=有待上报的充电结果事件 */
    char     startFailPreTradeNo[41];
    char     startFailTradeNo[41];
    uint8_t  startFailGunNo;
    uint8_t  startFailStartType;
    uint32_t startFailStartTime;
    uint8_t  stopResult;                    /* 停止充电结果, 10=成功, 11=失败 */
    uint8_t  stopFailReson;                 /* 停止失败原因: 0=成功, 10=tradeNo不匹配, 11=已停机 */
    uint16_t stopResultCode;                /* 停止充电结果码, GWE协议停止码 */

    /* 日志查询缓存 */
    uint32_t logQueryStartDate;             /* 查询起始时间戳 */
    uint32_t logQueryStopDate;              /* 查询终止时间戳 */
    uint8_t  logQueryAskType;               /* 查询类型: 10=交易, 11=电表, 12=日志, 13=告警 */
    char     logQueryNo[41];                /* 查询流水号 */
    uint8_t  logQueryRetType;               /* 响应类型: 10=报文格式, 11=文件格式 */
    uint8_t  logQueryEvtSum;                /* 总帧数 */
    uint8_t  logQueryEvtNo;                 /* 当前帧序号 */

    /* 日志查询服务应答缓存 */
    uint8_t  srvQueDataResult;              /* 查询结果, 10=成功 */

    /* 设备维护缓存 */
    uint8_t  devMaintainCtrlType;           /* 控制类型 */
    uint32_t devMaintainReason;             /* 维护控制失败原因 */
    uint8_t  devMaintainQueryResult;        /* 维护查询结果, 10=成功 */

    /* 交易记录召测应答缓存 */
    uint8_t  srvTradeAskType;               /* 召测类型 */
    char     srvTradeAskPreTradeNo[41];     /* 平台交易流水号 */
    char     srvTradeAskTradeNo[41];        /* 设备交易流水号 */
    char     srvTradeAskStartDate[17];      /* 查询起始时间 */
    char     srvTradeAskStopDate[17];       /* 查询结束时间 */
    uint8_t  srvTradeAskResult;             /* 召测结果, 10=成功 */
    uint16_t srvTradeAskCnt;                /* 交易记录数量 */

    /* 电表底值召测应答缓存 */
    uint8_t  srvMeterAskType;               /* 召测类型 */
    char     srvMeterAskDate[17];           /* 查询时间 */
    uint8_t  srvMeterAskResult;             /* 召测结果, 10=成功 */

    /* 有序充电应答缓存 */
    char     srvOrderlyChargePreTradeNo[41];/* 订单流水号 */
    uint8_t  srvOrderlyChargeResult;        /* 返回结果, 10=成功 */
    uint8_t  srvOrderlyChargeReason;        /* 失败原因 */

    /* 预约充电应答缓存 */
    uint8_t  srvRsvChargeGunNo;             /* 枪号 */
    uint8_t  srvRsvChargeAppoMethod;        /* 预约方式 */
    uint8_t  srvRsvChargeResult;            /* 预约结果, 10=成功 */

    /* update型服务应答缓存: resCode/result由回调填充, optSn/funCode由请求提取 */
    uint8_t  srvUpdateResult;               /* 处理结果, update型服务共用(10=成功, 用于resCode/result) */
    uint16_t srvUpdateFunCode;              /* 功能配置码 */
    char     srvUpdateOptSn[41];            /* 操作流水号 */

    /* 计费模型服务缓存 */
    char     feeModelId[17];                /* 计费模型ID, 请求提取后原样应答 */
    uint8_t  feeModelResult;                /* 处理结果, 回调填充 */

} IotGWEDataOfflineClr_Struct;

/* 离线保留数据 */
typedef struct
{
    uint8_t _reserved;

} IotGWEDataOfflineNotClear_Struct;

typedef struct
{
    IotGWEDataOfflineClr_Struct offlineClearData;
    IotGWEDataOfflineNotClear_Struct offlineNotClearData;

}IotGWERecvData_Struct;
/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_IOT_PROTO_GWE_TYPES_H_ */
