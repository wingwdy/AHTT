/******************************************************************************
* File Name          : Asw_IotProtoAPTypes.h
* Description        : 安培协议类型定义（帧ID、枚举、结构体）
* -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
*------------    --------     -------   ----------------------------------------
*2026/05/21     V1.0.0       WDY        初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_AP_TYPES_H_
#define ASW_IOT_PROTO_AP_TYPES_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

/* 协议版本 */
#define IOT_AP_PROTOCOL_VERSION                 (10141)

/* 通信协议头定义 */
#define IOT_AP_HEAD                            (0x68)

/* 通信buff缓存定义 */
#define IOT_AP_TXRX_BUFFER_SIZE                 (4096)

/* 安培协议充电桩编号固定长度 */
#define IOT_AP_PILE_DN_LEN                      (8)

/* 协议CMD类型定义 */
#define IOT_AP_CMDTYPE_REQUSET                  (0x00)
#define IOT_AP_CMDTYPE_RESPONSE                 (0x01)
#define IOT_AP_CMD_NULL                         (0xFFFF)

/* 信息帧固定字段定义 */
#define IOT_AP_CTRL_UP_REQ                      (130)
#define IOT_AP_CTRL_DOWN_REQ                    (133)
#define IOT_AP_TYPE_CLOCK_SYNC                  (103)
#define IOT_AP_TYPE_UP_DATA                     (130)
#define IOT_AP_TYPE_DOWN_DATA                   (133)
#define IOT_AP_VSQ_DEFAULT                      (0x00)
#define IOT_AP_COT_ACT                          (0x06)
#define IOT_AP_COT_ACTCON                       (0x07)

/* 实时数据上报周期定义 */
#define IOTAP_CFG_IDLE_REALDATA_CYCLE           (2 * 60 * 1000)
#define IOTAP_CFG_CHARGING_REALDATA_CYCLE       (30 * 1000)

/* 日志接口函数定义 */
#define IOTAP_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTAP_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/*
 * 安培协议V1.4.1 记录类型码定义
 * 参照附录B：报文帧列表
 */

/* ====== F帧（基础通信链路）====== */
#define IOT_AP_CMD_LOGIN_REQ                    (0xF1)  /* 登录验证请求 */
#define IOT_AP_CMD_LOGIN_RSP                    (0xF2)  /* 登录验证应答 */
#define IOT_AP_CMD_UFRAME_AUTH                  (0xF3)  /* U帧认证请求 */
#define IOT_AP_CMD_UFRAME_ACK                   (0xF4)  /* U帧认证应答 */
#define IOT_AP_CMD_HEARTBEAT_REQ                (0xF5)  /* 心跳上报 */
#define IOT_AP_CMD_HEARTBEAT_RSP                (0xF6)  /* 心跳应答 */
#define IOT_AP_CMD_SYNC_TIME_REQ                (0xF7)  /* 时钟同步请求 */
#define IOT_AP_CMD_SYNC_TIME_RSP                (0xF8)  /* 时钟同步应答 */

/* ====== B帧（业务数据）====== */
#define IOT_AP_CMD_B01_REALTIME_DATA            (0x01)  /* 充电过程实时监测数据（基础） */
#define IOT_AP_CMD_B02_BILLMODEL_DOWN           (0x02)  /* 下发计费模型下行数据（基础） */
#define IOT_AP_CMD_B03_BILLMODEL_RESULT         (0x03)  /* 下发计费模型结果数据（基础） */
#define IOT_AP_CMD_B04_CHG_CTRL_DOWN            (0x04)  /* 充电启停控制命令下发下行数据（扫码充电） */
#define IOT_AP_CMD_B05_CHG_CTRL_RESULT          (0x05)  /* 充电启停控制命令结果确认（扫码充电） */
#define IOT_AP_CMD_B06_CARD_AUTH_UP             (0x06)  /* 刷卡鉴权上行（在线刷卡充电） */
#define IOT_AP_CMD_B07_CARD_AUTH_DOWN           (0x07)  /* 刷卡鉴权下行（在线刷卡充电） */
#define IOT_AP_CMD_B08_VIN_AUTH_UP              (0x08)  /* VIN码鉴权上行（在线vin码充电） */
#define IOT_AP_CMD_B09_VIN_AUTH_DOWN            (0x09)  /* VIN码鉴权下行（在线vin码充电） */
#define IOT_AP_CMD_B10_START_NOTIFY_UP          (0x10)  /* 启动通知上报（在线刷卡/vin码充电） */
#define IOT_AP_CMD_B11_START_NOTIFY_DOWN        (0x11)  /* 启动通知下行（在线刷卡/vin码充电） */
#define IOT_AP_CMD_B12_ONLINE_ORDER_UP          (0x12)  /* 在线情况下停止充电时上传记录数据（基础） */
#define IOT_AP_CMD_B13_ONLINE_ORDER_DOWN        (0x13)  /* 在线交易包下行数据（基础） */
#define IOT_AP_CMD_B14_DEDUCT_CONFIRM           (0x14)  /* 充电扣款后下行数据（基础） */
#define IOT_AP_CMD_B15_OFFLINE_ORDER_UP         (0x15)  /* 离线交易上线后上传交易记录数据（基础） */
#define IOT_AP_CMD_B16_OFFLINE_ORDER_DOWN       (0x16)  /* 离线交易包下行数据（基础） */
#define IOT_AP_CMD_B17_WHITELIST_CARD_DOWN      (0x17)  /* 充电卡白名单下行数据（离线刷卡充电） */
#define IOT_AP_CMD_B18_WHITELIST_CARD_UP        (0x18)  /* 充电卡白名单上行数据（离线刷卡充电） */
#define IOT_AP_CMD_B19_WHITELIST_VIN_DOWN       (0x19)  /* VIN码白名单下行数据（离线vin码充电） */
#define IOT_AP_CMD_B20_WHITELIST_VIN_UP         (0x20)  /* VIN码白名单上行数据（离线vin码充电） */
#define IOT_AP_CMD_B21_WHITELIST_CLEAR_DOWN     (0x21)  /* 白名单清空下行（离线刷卡/vin码充电） */
#define IOT_AP_CMD_B22_WHITELIST_CLEAR_UP       (0x22)  /* 白名单清空上行（离线刷卡/vin码充电） */

/* 扩展功能帧 */
#define IOT_AP_CMD_B23_UPGRADE_START            (0x23)  /* 远程升级启动（扩展） */
#define IOT_AP_CMD_B24_UPGRADE_RESULT           (0x24)  /* 远程升级启动命令接收结果（扩展） */
#define IOT_AP_CMD_B25_RESERVE_CMD_DOWN         (0x25)  /* 预约/定时命令下行数据（扩展） */
#define IOT_AP_CMD_B26_RESERVE_RESULT_UP        (0x26)  /* 桩回复预约/定时结果上行数据（扩展） */
#define IOT_AP_CMD_B27_RESERVE_PILE_UP          (0x27)  /* 桩预约命令上行数据（扩展） */
#define IOT_AP_CMD_B28_RESERVE_RESULT_DOWN      (0x28)  /* 远程回复预约结果下行数据（扩展） */
#define IOT_AP_CMD_B29_REMOTE_PARAM_SET_DOWN    (0x29)  /* 远程设置桩参数下行（扩展） */
#define IOT_AP_CMD_B30_REMOTE_PARAM_SET_UP      (0x30)  /* 远程设置桩参数上行（扩展） */
#define IOT_AP_CMD_B31_SIM_INFO_UP              (0x31)  /* SIM卡信息上行数据（扩展） */
#define IOT_AP_CMD_B32_TERMINAL_REQ_DOWN        (0x32)  /* 请求终端数据下行数据（扩展） */
#define IOT_AP_CMD_B33_POWER_CTRL_DOWN          (0x33)  /* 充电功率控制下行（扩展） */
#define IOT_AP_CMD_B34_POWER_CTRL_UP            (0x34)  /* 充电功率控制上行（扩展） */
#define IOT_AP_CMD_B35_BILLMODEL_POLL_DOWN      (0x35)  /* 计费模型召测下行数据（扩展） */
#define IOT_AP_CMD_B36_BILLMODEL_POLL_UP        (0x36)  /* 计费模型召测上行数据（扩展） */
#define IOT_AP_CMD_B37_VEHICLE_MONITOR          (0x37)  /* 充电中车辆监测数据（扩展） */
#define IOT_AP_CMD_B38_ZERO_METER_VALUE         (0x38)  /* 零点示值上报(扩展) */
#define IOT_AP_CMD_B39_FTP_ADDR_DOWN            (0x39)  /* 平台ftp服务器地址下发（扩展） */
#define IOT_AP_CMD_B40_FTP_ADDR_UP              (0x40)  /* 平台ftp服务器地址上行（扩展） */
#define IOT_AP_CMD_B41_LOG_POLL_DOWN            (0x41)  /* 平台召测桩车下行（扩展） */
#define IOT_AP_CMD_B42_LOG_POLL_UP              (0x42)  /* 平台召测桩数据上行（扩展） */
#define IOT_AP_CMD_B43_ELOCK_DOWN               (0x43)  /* 平台电子锁功能下行（扩展） */
#define IOT_AP_CMD_B44_ELOCK_UP                 (0x44)  /* 桩电子锁功能上行（扩展） */
#define IOT_AP_CMD_B45_POWER_POLL_DOWN          (0x45)  /* 充电功率召测下行（扩展） */
#define IOT_AP_CMD_B46_POWER_POLL_UP            (0x46)  /* 充电功率召测上行（扩展） */

/* 分时服务费帧 */
#define IOT_AP_CMD_B47_TIMEBILL_DOWN            (0x47)  /* 下发计费模型下行数据—分时服务费 */
#define IOT_AP_CMD_B48_TIMEBILL_UP              (0x48)  /* 下发计费模型上行数据—分时服务费 */
#define IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP       (0x49)  /* 计费模型切换生效上行—分时服务费 */
#define IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN     (0x50)  /* 计费模型切换生效下行—分时服务费 */
#define IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN       (0x51)  /* 计费模型召测下行数据-分时服务费 */
#define IOT_AP_CMD_B52_TIMEBILL_POLL_UP         (0x52)  /* 计费模型召测上行数据—分时服务费 */
#define IOT_AP_CMD_B53_ONLINE_DETAIL_UP         (0x53)  /* 在线情况下停止充电上传分时交易明细数据 */
#define IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN       (0x54)  /* 在线分时明细交易包下行数据 */
#define IOT_AP_CMD_B55_OFFLINE_DETAIL_UP        (0x55)  /* 离线情况下停止充电上传分时交易明细数据 */
#define IOT_AP_CMD_B56_OFFLINE_DETAIL_DOWN      (0x56)  /* 离线分时明细交易包下行数据 */
#define IOT_AP_CMD_B57_POWER_STATUS_UP          (0x57)  /* 充电功率控制过程中的扩展实时状态（扩展） */
#define IOT_AP_CMD_B58_LOAD_INTERR_DOWN         (0x58)  /* 充电负荷中断下行（扩展） */
#define IOT_AP_CMD_B59_LOAD_INTERR_UP           (0x59)  /* 充电负荷中断上行（扩展） */
#define IOT_AP_CMD_B60_METER_ENC_UP             (0x60)  /* 电表加密数据上报上行（扩展） */
#define IOT_AP_CMD_B61_METER_ENC_DOWN           (0x61)  /* 电表加密数据下行（扩展） */

/* 发送CMD计数 (实际有效帧数: F1/F3/F5/F8/B1/B5/B6/B10/B24/B31/B34/B38/B40/B46/B48/B49/B52/B53/B57) */
#define IOT_AP_CMD_SEND_COUNT                  (19)

/* 接收CMD计数 (实际有效帧数: F2/F4/F6/F7/B4/B7/B11/B14/B23/B32/B33/B39/B45/B47/B50/B51/B54) */
#define IOT_AP_CMD_RECV_COUNT                  (17)

/******************************************************************************
*    Enum Definition
******************************************************************************/

typedef enum
{
    eIotAPStopReason_Null = 0,                 /* 无停止原因 */
    eIotAPStopReason_Full = 1,                 /* 充满/车辆停止/异常拔枪 */
    eIotAPStopReason_KeyStop = 2,              /* 主动停止(按键) */
    eIotAPStopReason_AppStop = 3,              /* 主动停止(远程) */
    eIotAPStopReason_CpFault = 14,             /* 充电中控制导引故障 */
    eIotAPStopReason_EmergencyStop = 17,       /* 急停按钮动作故障 */
    eIotAPStopReason_LeakageCurrErr = 21,      /* 交流输入断路器故障/漏电 */
    eIotAPStopReason_JcqMaloperation = 22,     /* 接触器拒动/误动 */
    eIotAPStopReason_JcqSynechiaFault = 23,    /* 接触器粘连 */
    eIotAPStopReason_InputFault = 24,          /* 过压/欠压/过流 */
    eIotAPStopReason_TempErr = 25,             /* 充电桩/接口过温 */
    eIotAPStopReason_DiodeStop = 50,           /* BSM连接器连接状态异常/二极管 */
    eIotAPStopReason_OtherErr = 54,            /* 其他故障 */
    eIotAPStopReason_PEBreakFault = 113,       /* PE接地/绝缘监测 */
    eIotAPStopReason_SumNoEnough = 301,        /* 达到设定金额/金额不足 */
    eIotAPStopReason_PowerOff = 312,           /* 系统掉电 */
    eIotAPStopReason_StartTimeout = 320,       /* 启动超时停止 */
    eIotAPStopReason_MeterCalcErr = 321,       /* 电表读数异常 */
    eIotAPStopReason_MeterCommErr = 326,       /* 电表通信故障 */
    eIotAPStopReason_NoExpectedErr = 54,       /* 未知原因按其他故障上报 */
}IotAPStopReason_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotAP_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotAP_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

typedef struct
{
    uint16_t cmd;
    uint8_t cmdType;
    uint32_t sendCycle;
    IotAP_pSendPackFuncType pSendFunc;
    uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotAPSendCtrl_Struct;

typedef struct
{
    uint16_t cmd;
    uint8_t cmdType;
    IotAP_pRecvParseFuncType pRecvParse;
    uint16_t maxTimeout;
    uint16_t maxTryCnt;
    uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotAPRecvCtrl_Struct;

/* 安培信息帧头结构，线格式参考示例报文和旧项目 ANPEI_HEAD_T */
typedef struct
{
    uint8_t head;           /* 帧头标识 0x68 */
    uint8_t len;            /* 从控制域到数据域末尾的长度 */
    uint8_t control[4];     /* 控制域 */
    uint8_t typeId;         /* 类型标识 */
    uint8_t vsq;            /* 可变结构限定词 */
    uint8_t cot;            /* 传送原因 */
    uint8_t appSerAddr[2];  /* 应用服务数据单元公共地址 */
    uint8_t infAddr[3];     /* 信息对象地址 */
    uint8_t recordKind;     /* 记录类型 */
}IotAPInfoFrameHead_Struct;


#endif /* ASW_IOT_PROTO_AP_TYPES_H_ */
