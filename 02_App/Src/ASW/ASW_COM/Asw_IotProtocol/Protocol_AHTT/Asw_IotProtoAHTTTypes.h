/******************************************************************************
* File Name          : Asw_IotProtoAHTTTypes.h
* Description        : AHTT protocol type definitions
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/08/31      V1.0.0      wdy        初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_AHTT_TYPES_H_
#define ASW_IOT_PROTO_AHTT_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
 ******************************************************************************/
#define IOTAHTT_CFG_DebugPrint(fmt, ...)    DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

#define IOT_AHTT_HEAD                         (0xEA)      /* 协议帧头 */
#define IOT_AHTT_PROTOCOL_VERSION             (0x01)      /* 协议版本号 */
#define IOT_AHTT_FRAME_HEAD_LEN               (12)        /* 固定帧头长度，单位：字节 */
#define IOT_AHTT_CRC_LEN                      (2)         /* CRC校验长度，单位：字节 */
#define IOT_AHTT_DECLARE_MIN_LEN              (11)        /* 协议声明长度最小值，单位：字节 */
#define IOT_AHTT_FRAME_MIN_LEN                (14)        /* 完整帧最小长度，单位：字节 */
#define IOT_AHTT_FRAME_MAX_LEN                (512)       /* 完整帧最大长度，单位：字节 */
#define IOT_AHTT_FRAME_QUEUE_BUF_SIZE         (3072)      /* 帧队列收发缓存大小，单位：字节 */
#define IOT_AHTT_DEVICE_NUM_LEN               (5)         /* 设备编号长度，单位：字节 */
#define IOT_AHTT_LOGIN_PARAM_LEN               (28)        /* 签到参数长度，单位：字节 */
#define IOT_AHTT_HEART_PARAM_LEN               (5)         /* 心跳参数长度，单位：字节 */
#define IOT_AHTT_HEART_CHANNEL_COUNT           (12)        /* 心跳状态通道数量 */
#define IOT_AHTT_HEART_NET_4G                  (0x02)      /* 心跳网络类型：4G */
#define IOT_AHTT_HEART_STATE_IDLE              (0x00)      /* 心跳通道状态：空闲 */
#define IOT_AHTT_HEART_STATE_WORK              (0x01)      /* 心跳通道状态：工作 */
#define IOT_AHTT_HEART_STATE_FAULT             (0x02)      /* 心跳通道状态：故障 */
#define IOT_AHTT_HEART_STATE_OFFLINE           (0x03)      /* 心跳通道状态：离线 */
#define IOT_AHTT_LOGIN_TIMEOUT_MS              (10000)     /* 签到应答超时，单位：毫秒 */
#define IOT_AHTT_LOGIN_MAX_TRY_COUNT           (3)         /* 签到最大尝试次数 */
#define IOT_AHTT_HEART_TIMEOUT_MS              (10000)     /* 心跳应答超时，单位：毫秒 */
#define IOT_AHTT_HEART_MAX_TRY_COUNT           (3)         /* 心跳最大尝试次数 */
#define IOT_AHTT_MINUTE_MS                     (60000)     /* 分钟换算为毫秒 */
#define IOT_AHTT_HEART_CYCLE_PARAM_LEN         (1)         /* 心跳周期参数长度，单位：字节 */
#define IOT_AHTT_HEART_CYCLE_MIN               (1)         /* 心跳周期最小值，单位：分钟 */
#define IOT_AHTT_HEART_CYCLE_MAX               (10)        /* 心跳周期最大值，单位：分钟 */
#define IOT_AHTT_MAX_CHARGE_TIME_PARAM_LEN     (1)         /* 最大充电时长参数长度，单位：字节 */
#define IOT_AHTT_MAX_CHARGE_TIME_MIN           (1)         /* 最大充电时长最小值，单位：小时 */
#define IOT_AHTT_MAX_CHARGE_TIME_MAX           (16)        /* 最大充电时长最大值，单位：小时 */
#define IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_INDEX  (7)         /* 实时数据上报周期参数偏移 */
#define IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_MIN    (1)         /* 实时数据上报周期最小值，单位：分钟 */
#define IOT_AHTT_DEV_PARAM_UPLOAD_CYCLE_MAX    (30)        /* 实时数据上报周期最大值，单位：分钟 */
#define IOT_AHTT_DOMAIN_FIELD_LEN              (24)        /* 域名固定字段长度，单位：字节 */
#define IOT_AHTT_PORT_FIELD_LEN                (6)         /* 端口固定字段长度，单位：字节 */
#define IOT_AHTT_DOMAIN_PORT_PARAM_LEN         (30)        /* 域名和端口参数长度，单位：字节 */
#define IOT_AHTT_DOMAIN_SWITCH_TCP_TIMEOUT_MS  (60000)     /* 候选地址TCP连接超时，单位：毫秒 */
#define IOT_AHTT_PARAM_RESULT_FAIL             (0x00)      /* 参数设置失败结果 */
#define IOT_AHTT_PARAM_RESULT_SUCCESS          (0x01)      /* 参数设置成功结果 */
#define IOT_AHTT_PRIVATE_PARAM_VERSION         (1)         /* AHTT私有参数结构版本 */
#define IOT_AHTT_SEQ_MIN                      (1)         /* 流水号最小值 */
#define IOT_AHTT_SEQ_MAX                      (60000)     /* 流水号最大值 */
#define IOT_AHTT_CMD_SEND_COUNT               (23)        /* 发送命令控制项数量 */
#define IOT_AHTT_CMD_RECV_COUNT               (23)        /* 接收命令控制项数量 */
#define IOT_AHTT_CMDTYPE_REQUSET              (0x00)      /* 请求命令类型 */
#define IOT_AHTT_CMDTYPE_RESPONSE             (0x01)      /* 应答命令类型 */
#define IOT_AHTT_CMD_NULL                     (0xFF)      /* 无效命令 */
#define IOT_AHTT_PACK_INVALID_LEN             (0xFFFF)    /* 组包失败返回长度 */

#define IOT_AHTT_CMD_LOGIN                    (0x01)      /* 签到 */
#define IOT_AHTT_CMD_SET_HEART_CYCLE          (0x02)      /* 设置心跳周期 */
#define IOT_AHTT_CMD_QUERY_HEART_CYCLE        (0x03)      /* 查询心跳周期 */
#define IOT_AHTT_CMD_SET_DOMAIN_PORT          (0x04)      /* 设置平台域名和端口 */
#define IOT_AHTT_CMD_SET_MAX_CHARGE_TIME      (0x0A)      /* 设置最大充电时长 */
#define IOT_AHTT_CMD_QUERY_MAX_CHARGE_TIME    (0x0B)      /* 查询最大充电时长 */
#define IOT_AHTT_CMD_STOP_CHARGE              (0x4B)      /* 停止充电 */
#define IOT_AHTT_CMD_CARD_AUTH                (0x4C)      /* 刷卡鉴权 */
#define IOT_AHTT_CMD_START_CHARGE             (0x4D)      /* 启动充电 */
#define IOT_AHTT_CMD_HEARTBEAT                (0x81)      /* 心跳上报 */
#define IOT_AHTT_CMD_SET_DEV_PARAM            (0x84)      /* 设置设备运行参数 */
#define IOT_AHTT_CMD_QUERY_DEV_PARAM          (0x85)      /* 查询设备运行参数 */
#define IOT_AHTT_CMD_REPORT_REALDATA          (0x93)      /* 上报实时数据 */
#define IOT_AHTT_CMD_REPORT_ORDER             (0x94)      /* 上报订单 */
#define IOT_AHTT_CMD_QUERY_TIME               (0x95)      /* 查询平台时间 */
#define IOT_AHTT_CMD_REPORT_DEV_STATE         (0x96)      /* 上报设备状态 */
#define IOT_AHTT_CMD_DEV_ALARM                (0xC1)      /* 上报设备告警 */
#define IOT_AHTT_CMD_NET_ALARM                (0xC2)      /* 上报网络告警 */
#define IOT_AHTT_CMD_TEMP_ALARM               (0xC3)      /* 上报温度告警 */
#define IOT_AHTT_CMD_SET_TEMP_LIMIT           (0xC4)      /* 设置温度告警阈值 */
#define IOT_AHTT_CMD_QUERY_TEMP_LIMIT         (0xC5)      /* 查询温度告警阈值 */
#define IOT_AHTT_CMD_ELECTRIC_ALARM           (0xC7)      /* 上报过流、过压或欠压告警 */
#define IOT_AHTT_CMD_UPDATE                   (0xD1)      /* 远程升级 */

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t head;                  /* 协议帧头 */
    uint8_t len[2];                /* 声明长度，低字节在前 */
    uint8_t ver;                   /* 协议版本号 */
    uint8_t deviceNum[5];          /* 设备编号 */
    uint8_t seq[2];                /* 通信流水号，低字节在前 */
    uint8_t cmd;                   /* 命令字 */
}IotAHTTFrameHead_Struct;

typedef uint16_t (*IotAHTT_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotAHTT_pRecvParseFuncType)(uint8_t *port, uint8_t *pData, uint16_t len);

typedef struct
{
    uint16_t cmd;                          /* 命令字 */
    uint8_t cmdType;                       /* 命令类型 */
    uint32_t sendCycle;                    /* 周期发送间隔，单位：毫秒 */
    IotAHTT_pSendPackFuncType pSendFunc;   /* 参数组包函数 */
    uint16_t matchCmd;                     /* 关联的接收命令 */
    uint8_t printFlag;                     /* 日志打印标志 */
    char *cMeaning;                        /* 命令中文含义 */
}IotAHTTSendCtrl_Struct;

typedef struct
{
    uint16_t cmd;                          /* 命令字 */
    uint8_t cmdType;                       /* 命令类型 */
    IotAHTT_pRecvParseFuncType pRecvParse; /* 参数解析函数 */
    uint16_t maxTimeout;                   /* 最大应答超时时间，单位：毫秒 */
    uint16_t maxTryCnt;                    /* 最大重试次数 */
    uint16_t matchCmd;                     /* 关联的发送命令 */
    uint8_t printFlag;                     /* 日志打印标志 */
    char *cMeaning;                        /* 命令中文含义 */
}IotAHTTRecvCtrl_Struct;

typedef char IotAHTTFrameHeadSizeCheck[(sizeof(IotAHTTFrameHead_Struct) == 12) ? 1 : -1];

#endif /* ASW_IOT_PROTO_AHTT_TYPES_H_ */
