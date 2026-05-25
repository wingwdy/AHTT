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
*2026/05/21     V1.0.0       AI        初版创建 - 骨架代码
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
#define IOT_AP_PROTOCOL_VERSION                 (10141U)

/* 通信协议头定义 */
#define IOT_AP_HEAD1                            (0x68U)
#define IOT_AP_HEAD2                            (0x68U)

/* 通信buff缓存定义 */
#define IOT_AP_TXRX_BUFFER_SIZE                 (4096U)

/* 实时数据上报周期定义 */
#define IOTAP_CFG_IDLE_REALDATA_CYCLE           (5 * 60 * 1000)
#define IOTAP_CFG_CHARGING_REALDATA_CYCLE       (15 * 1000)

/* 日志接口函数定义 */
#define IOTAP_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTAP_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/*
 * 安培协议V1.4.1 记录类型码定义
 * 参照附录B：报文帧列表
 */

/* ====== F帧（基础通信链路）====== */
#define IOT_AP_CMD_LOGIN_REQ                    (0xF1U)  /* 登录验证请求 */
#define IOT_AP_CMD_LOGIN_RSP                    (0xF2U)  /* 登录验证应答 */
#define IOT_AP_CMD_UFRAME_AUTH                  (0xF3U)  /* U帧认证请求 */
#define IOT_AP_CMD_UFRAME_ACK                   (0xF4U)  /* U帧认证应答 */
#define IOT_AP_CMD_HEARTBEAT_REQ                (0xF5U)  /* 心跳上报 */
#define IOT_AP_CMD_HEARTBEAT_RSP                (0xF6U)  /* 心跳应答 */
#define IOT_AP_CMD_SYNC_TIME_REQ                (0xF7U)  /* 时钟同步请求 */
#define IOT_AP_CMD_SYNC_TIME_RSP                (0xF8U)  /* 时钟同步应答 */

/* ====== B帧（业务数据）====== */
#define IOT_AP_CMD_B01_REALTIME_DATA            (0x01U)  /* 充电过程实时监测数据（基础） */
#define IOT_AP_CMD_B02_BILLMODEL_DOWN           (0x02U)  /* 下发计费模型下行数据（基础） */
#define IOT_AP_CMD_B03_BILLMODEL_RESULT         (0x03U)  /* 下发计费模型结果数据（基础） */
#define IOT_AP_CMD_B04_CHG_CTRL_DOWN            (0x04U)  /* 充电启停控制命令下发下行数据（扫码充电） */
#define IOT_AP_CMD_B05_CHG_CTRL_RESULT          (0x05U)  /* 充电启停控制命令结果确认（扫码充电） */
#define IOT_AP_CMD_B06_CARD_AUTH_UP             (0x06U)  /* 刷卡鉴权上行（在线刷卡充电） */
#define IOT_AP_CMD_B07_CARD_AUTH_DOWN           (0x07U)  /* 刷卡鉴权下行（在线刷卡充电） */
#define IOT_AP_CMD_B08_VIN_AUTH_UP              (0x08U)  /* VIN码鉴权上行（在线vin码充电） */
#define IOT_AP_CMD_B09_VIN_AUTH_DOWN            (0x09U)  /* VIN码鉴权下行（在线vin码充电） */
#define IOT_AP_CMD_B10_START_NOTIFY_UP          (0x10U)  /* 启动通知上报（在线刷卡/vin码充电） */
#define IOT_AP_CMD_B11_START_NOTIFY_DOWN        (0x11U)  /* 启动通知下行（在线刷卡/vin码充电） */
#define IOT_AP_CMD_B12_ONLINE_ORDER_UP          (0x12U)  /* 在线情况下停止充电时上传记录数据（基础） */
#define IOT_AP_CMD_B13_ONLINE_ORDER_DOWN        (0x13U)  /* 在线交易包下行数据（基础） */
#define IOT_AP_CMD_B14_DEDUCT_CONFIRM           (0x14U)  /* 充电扣款后下行数据（基础） */
#define IOT_AP_CMD_B15_OFFLINE_ORDER_UP         (0x15U)  /* 离线交易上线后上传交易记录数据（基础） */
#define IOT_AP_CMD_B16_OFFLINE_ORDER_DOWN       (0x16U)  /* 离线交易包下行数据（基础） */
#define IOT_AP_CMD_B17_WHITELIST_CARD_DOWN      (0x17U)  /* 充电卡白名单下行数据（离线刷卡充电） */
#define IOT_AP_CMD_B18_WHITELIST_CARD_UP        (0x18U)  /* 充电卡白名单上行数据（离线刷卡充电） */
#define IOT_AP_CMD_B19_WHITELIST_VIN_DOWN       (0x19U)  /* VIN码白名单下行数据（离线vin码充电） */
#define IOT_AP_CMD_B20_WHITELIST_VIN_UP         (0x20U)  /* VIN码白名单上行数据（离线vin码充电） */
#define IOT_AP_CMD_B21_WHITELIST_CLEAR_DOWN     (0x21U)  /* 白名单清空下行（离线刷卡/vin码充电） */
#define IOT_AP_CMD_B22_WHITELIST_CLEAR_UP       (0x22U)  /* 白名单清空上行（离线刷卡/vin码充电） */

/* 扩展功能帧 */
#define IOT_AP_CMD_B23_UPGRADE_START            (0x23U)  /* 远程升级启动（扩展） */
#define IOT_AP_CMD_B24_UPGRADE_RESULT           (0x24U)  /* 远程升级启动命令接收结果（扩展） */
#define IOT_AP_CMD_B25_RESERVE_CMD_DOWN         (0x25U)  /* 预约/定时命令下行数据（扩展） */
#define IOT_AP_CMD_B26_RESERVE_RESULT_UP        (0x26U)  /* 桩回复预约/定时结果上行数据（扩展） */
#define IOT_AP_CMD_B27_RESERVE_PILE_UP          (0x27U)  /* 桩预约命令上行数据（扩展） */
#define IOT_AP_CMD_B28_RESERVE_RESULT_DOWN      (0x28U)  /* 远程回复预约结果下行数据（扩展） */
#define IOT_AP_CMD_B29_REMOTE_PARAM_SET_DOWN    (0x29U)  /* 远程设置桩参数下行（扩展） */
#define IOT_AP_CMD_B30_REMOTE_PARAM_SET_UP      (0x30U)  /* 远程设置桩参数上行（扩展） */
#define IOT_AP_CMD_B31_SIM_INFO_UP              (0x31U)  /* SIM卡信息上行数据（扩展） */
#define IOT_AP_CMD_B32_TERMINAL_REQ_DOWN        (0x32U)  /* 请求终端数据下行数据（扩展） */
#define IOT_AP_CMD_B33_POWER_CTRL_DOWN          (0x33U)  /* 充电功率控制下行（扩展） */
#define IOT_AP_CMD_B34_POWER_CTRL_UP            (0x34U)  /* 充电功率控制上行（扩展） */
#define IOT_AP_CMD_B35_BILLMODEL_POLL_DOWN      (0x35U)  /* 计费模型召测下行数据（扩展） */
#define IOT_AP_CMD_B36_BILLMODEL_POLL_UP        (0x36U)  /* 计费模型召测上行数据（扩展） */
#define IOT_AP_CMD_B37_VEHICLE_MONITOR          (0x37U)  /* 充电中车辆监测数据（扩展） */
#define IOT_AP_CMD_B38_ZERO_METER_VALUE         (0x38U)  /* 零点示值上报(扩展) */
#define IOT_AP_CMD_B39_FTP_ADDR_DOWN            (0x39U)  /* 平台ftp服务器地址下发（扩展） */
#define IOT_AP_CMD_B40_FTP_ADDR_UP              (0x40U)  /* 平台ftp服务器地址上行（扩展） */
#define IOT_AP_CMD_B41_LOG_POLL_DOWN            (0x41U)  /* 平台召测桩车下行（扩展） */
#define IOT_AP_CMD_B42_LOG_POLL_UP              (0x42U)  /* 平台召测桩数据上行（扩展） */
#define IOT_AP_CMD_B43_ELOCK_DOWN               (0x43U)  /* 平台电子锁功能下行（扩展） */
#define IOT_AP_CMD_B44_ELOCK_UP                 (0x44U)  /* 桩电子锁功能上行（扩展） */
#define IOT_AP_CMD_B45_POWER_POLL_DOWN          (0x45U)  /* 充电功率召测下行（扩展） */
#define IOT_AP_CMD_B46_POWER_POLL_UP            (0x46U)  /* 充电功率召测上行（扩展） */

/* 分时服务费帧 */
#define IOT_AP_CMD_B47_TIMEBILL_DOWN            (0x47U)  /* 下发计费模型下行数据—分时服务费 */
#define IOT_AP_CMD_B48_TIMEBILL_UP              (0x48U)  /* 下发计费模型上行数据—分时服务费 */
#define IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP       (0x49U)  /* 计费模型切换生效上行—分时服务费 */
#define IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN     (0x50U)  /* 计费模型切换生效下行—分时服务费 */
#define IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN       (0x51U)  /* 计费模型召测下行数据-分时服务费 */
#define IOT_AP_CMD_B52_TIMEBILL_POLL_UP         (0x52U)  /* 计费模型召测上行数据—分时服务费 */
#define IOT_AP_CMD_B53_ONLINE_DETAIL_UP         (0x53U)  /* 在线情况下停止充电上传分时交易明细数据 */
#define IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN       (0x54U)  /* 在线分时明细交易包下行数据 */
#define IOT_AP_CMD_B55_OFFLINE_DETAIL_UP        (0x55U)  /* 离线情况下停止充电上传分时交易明细数据 */
#define IOT_AP_CMD_B56_OFFLINE_DETAIL_DOWN      (0x56U)  /* 离线分时明细交易包下行数据 */
#define IOT_AP_CMD_B57_POWER_STATUS_UP          (0x57U)  /* 充电功率控制过程中的扩展实时状态（扩展） */
#define IOT_AP_CMD_B58_LOAD_INTERR_DOWN         (0x58U)  /* 充电负荷中断下行（扩展） */
#define IOT_AP_CMD_B59_LOAD_INTERR_UP           (0x59U)  /* 充电负荷中断上行（扩展） */
#define IOT_AP_CMD_B60_METER_ENC_UP             (0x60U)  /* 电表加密数据上报上行（扩展） */
#define IOT_AP_CMD_B61_METER_ENC_DOWN           (0x61U)  /* 电表加密数据下行（扩展） */

/* 发送CMD计数 (实际有效帧数: F1/F5/F8/B1/B5/B6/B10/B14/B53/B48/B49/B52/B34/B46/B57/B31/B38/B40/B24) */
#define IOT_AP_CMD_SEND_COUNT                  (19U)

/* 接收CMD计数 (实际有效帧数: F2/F6/F7/B4/B7/B11/B54/B14/B47/B50/B51/B33/B45/B32/B39/B23) */
#define IOT_AP_CMD_RECV_COUNT                  (16U)

/******************************************************************************
*    Enum Definition
******************************************************************************/

typedef enum
{
    eIotAPStopReason_Null = 0,

    /* 故障类原因 */
    eIotAPStopReason_CpVoltAbnor = 0x01,       /* CP电压异常 */
    eIotAPStopReason_CpGroundFault = 0x02,     /* CP对地短路 */
    eIotAPStopReason_PEBreakFault = 0x03,      /* PE接地故障 */
    eIotAPStopReason_LeakageCurrErr = 0x07,    /* 漏电流故障 */
    eIotAPStopReason_JcqMaloperation = 0x13,   /* 继电器误动拒动故障 */
    eIotAPStopReason_JcqSynechiaFault = 0x14,  /* 继电器粘连故障 */
    eIotAPStopReason_GunTempErr = 0x1B,        /* 枪过温故障 */
    eIotAPStopReason_ShortCut = 0x1E,          /* 输出短路 */
    eIotAPStopReason_MeterCalcErr = 0x0B,      /* 电能计量故障 */
    eIotAPStopReason_AmountFault = 0x0C,       /* 充电中金额异常 */
    eIotAPStopReason_BillModeErr = 0x21,       /* 计费模型异常 */
    eIotAPStopReason_StartTimeout = 0x20,      /* 启动超时 */
    eIotAPStopReason_DiodeStop = 0x25,         /* 车辆无二极管 */

    /* 用户主动停止 */
    eIotAPStopReason_KeyStop = 0x27,           /* 按键停止 */
    eIotAPStopReason_AppStop = 0x40,           /* APP停止 */
    eIotAPStopReason_ManualStop = 0x45,        /* 手动停止 */
    eIotAPStopReason_CarStop = 0x46,           /* 车辆停止 */

    /* 条件停止 */
    eIotAPStopReason_StopByEnergy = 0x42,      /* 按电量停止 */
    eIotAPStopReason_StopByMoney = 0x43,       /* 按金额停止 */
    eIotAPStopReason_StopByTime = 0x44,        /* 按时间停止 */

    /* 异常断开 */
    eIotAPStopReason_GunDisconnect = 0x6B,     /* 控制导引断开 */
    eIotAPStopReason_MeterCommErr = 0x6D,      /* 电表通信中断 */
    eIotAPStopReason_SumNoEnough = 0x6E,       /* 余额不足 */
    eIotAPStopReason_EmergencyStop = 0x72,     /* 急停按下 */
    eIotAPStopReason_TempErr = 0x74,           /* 温度异常 */
    eIotAPStopReason_OverCurr = 0x75,          /* 输出过流 */
    eIotAPStopReason_LittleCurr = 0x76,        /* 小电流 */
    eIotAPStopReason_VoltageErr = 0x79,        /* 电压异常 */
    eIotAPStopReason_CurrentErr = 0x7A,        /* 电流异常 */
    eIotAPStopReason_PowerOff = 0x83,          /* 掉电故障 */
    eIotAPStopReason_DataBaseErr = 0x1D,       /* 数据库错误 */

    /* 其他 */
    eIotAPStopReason_OtherErr = 0x65,          /* 其它原因 */
    eIotAPStopReason_NoExpectedErr = 0x90,     /* 未知原因 */
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

/* 帧头结构 */
typedef struct
{
    uint8_t head[2];        /* 帧头标识 0x68 0x68 */
    uint8_t length[2];      /* 数据长度 */
    uint8_t ctrlCode;       /* 控制码 */
    uint8_t address[4];     /* 地址域 */
    uint8_t cif[2];         /* CIF */
    uint8_t seq[2];         /* 序列号 */
    uint8_t dataLen[2];     /* 数据长度 */
    uint8_t cs;             /* 校验码 */
    uint8_t tail[2];        /* 帧尾标识 0x16 */
}IotAPFrameHead_Struct;


#endif /* ASW_IOT_PROTO_AP_TYPES_H_ */
