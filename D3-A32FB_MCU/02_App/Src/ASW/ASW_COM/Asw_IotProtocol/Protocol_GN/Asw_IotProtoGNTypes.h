/******************************************************************************
* File Name          : template.h
* Description        : Code for xxxxxxxxxxx
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_GN_TYPES_H_
#define ASW_IOT_PROTO_GN_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 通信协议版本 */
#define IOT_GN_PROTOCOL_VERSION                 (10008U)
#define IOT_DXL_PROTOCOL_VERSION                (50003U)

/* 通信协议头定义--GN+协议 */
#define IOT_GN_PLUS_HEAD1                       (0x5AU)                
#define IOT_GN_PLUS_HEAD2                       (0xA5U)

/* 通信协议头定义--GN协议 */
#define IOT_GN_HEAD1                            (0xFAU)                
#define IOT_GN_HEAD2                            (0xAFU)

/* 通信buff缓存定义 */
#define IOT_GN_TXRX_BUFFER_SIZE                 (3072U)

/* 计费模型类型定义 */
#define IOT_GN_BILLMODE_RATE_TYPE_4             4
#define IOT_GN_BILLMODE_RATE_TYPE_MULT          9

/* 实时数据上报周期定义 */
/* GN GNP */
#define IOTGN_CFG_IDLE_REALDATA_CYCLE           (5 * 60 * 1000)
#define IOTGN_CFG_CHARGING_REALDATA_CYCLE       (15 * 1000)
/* DXL */
#define IOTDXL_CFG_IDLE_REALDATA_CYCLE           (30 * 60 * 1000)
#define IOTDXL_CFG_CHARGING_REALDATA_CYCLE       (30 * 1000)

#define IOTDXL_CFG_IDLE_RES_REALDATA_CYCLE           (30)          /* 单位 min*/
#define IOTDXL_CFG_CHARGING_RES_REALDATA_CYCLE       (30)          /* 单位 s*/

/* DXL 工作参数最大数量 */
#define IOTDXL_CFG_WORK_PARAM_MAX_COUNT         5

/* 日志接口函数定义 */
#define IOTGN_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTGN_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/* 充电最小余额，1元，保留2位小数 */
#define IOTGN_CFG_CHARGE_MIN_ACCOUNT_MONEY      (100)   

/* 协议CMD 定义 */
#define IOT_GN_CMDTYPE_REQUSET			        (0x00U)             /* 请求 */
#define IOT_GN_CMDTYPE_RESPONSE                 (0x01U)             /* 应答 */
#define IOT_GN_CMD_NULL                         (0xFFU)             /* 无效 */

/* 协议CMD 发送定义 */
#define IOT_GN_CMD_LOGIN_REQ                    (0x01U)             /* 登陆 */
#define IOT_GN_CMD_HEARTBEAT_REQ                (0x03U)             /* 心跳请求 */
#define IOT_GN_CMD_BILLMODE_VERIFY_REQ          (0x05U)             /* 计费模型验证请求 */
#define IOT_GN_CMD_BILLMODE_REQ                 (0x09U)             /* 计费模型请求 */
#define IOT_GN_CMD_REPORT_REALDATA              (0x13U)             /* 上报实时数据 */
#define IOT_GN_CMD_CALL_REALDATA_ACK            (0xF13U)            /* 召测实时数据应答 */
#define IOT_GN_CMD_PILE_START_CHARGE_REQ        (0x31U)             /* 充电桩主动申请启动充电 */
#define IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP       (0x35U)             /* 远程控制停止充电应答 */
#define IOT_GN_CMD_REMOTE_START_CHARGE_RSP      (0x33U)             /* 远程控制启动充电应答 */
#define IOT_GN_CMD_MULTI_ORDER_RECORD_REQ       (0x3EU)             /* 多类电价交易记录 */
#define IOT_GN_CMD_ORDER_RECORD_REQ             (0x3FU)             /* 四类电价交易记录 */
#define IOT_GN_CMD_UPDATE_ACCOUNT_MONEY_RSP     (0x41U)             /* 远程更新账户余额应答 */
#define IOT_GN_CMD_OFFLINE_CARD_RSP             (0x43U)             /* 离线卡下发应答 */
#define IOT_GN_CMD_OFFLINE_CARD_CLEAR_RSP       (0x45U)             /* 离线卡清除应答 */
#define IOT_GN_CMD_OFFLINE_CARD_SEARCH_RSP      (0x47U)             /* 离线卡查询应答 */
#define IOT_GN_CMD_SYNC_TIME_RSP                (0x55U)             /* 远程对时应答 */
#define IOT_GN_CMD_SET_BILLMODE_4RATE_RSP       (0xF57U)            /* 设置四类电价计费模型应答 */
#define IOT_GN_CMD_SET_BILLMODE_MULTIRATE_RSP   (0x57U)             /* 设置多类电价计费模型应答 */
#define IOT_GN_CMD_SET_QRCODE_RSP               (0x59U)             /* 设置二维码应答 */
#define IOT_GN_CMD_SET_DEV_WORK_PARAM_RSP       (0x5BU)             /* 充电设备工作参数设置应答 */
#define IOT_GN_CMD_QUERY_DEV_WORK_PARAM_RSP     (0x5DU)             /* 充电设备工作参数查询应答 */
#define IOT_GN_CMD_REBOOT_RSP                   (0x91U)             /* 设置远程重启应答 */
#define IOT_GN_CMD_UPDATE_RSP                   (0x93U)             /* 设置远程更新应答 */

#define IOT_GN_CMD_SEND_COUNT                   (23U)

/* 协议CMD 接收定义 */
#define IOT_GN_CMD_LOGIN_RSP                    (0x02U)             /* 登陆应答 */
#define IOT_GN_CMD_HEARTBEAT_RSP                (0x04U)             /* 心跳应答 */
#define IOT_GN_CMD_BILLMODE_VERIFY_RSP          (0x06U)             /* 计费模型验证请求应答 */
#define IOT_GN_CMD_BILLMODE_4RATE_RSP           (0x0AU)             /* 4类电价应答 */
#define IOT_GN_CMD_BILLMODE_MUTIRATE_RSP        (0x0BU)             /* 多类电价应答 */
#define IOT_GN_CMD_CALL_REALDATA                (0x12U)             /* 召测实时数据 */
#define IOT_GN_CMD_PILE_START_CHARGE_RSP        (0x32U)             /* 充电桩主动申请启动充电应答 */
#define IOT_GN_CMD_REMOTE_START_CHARGE          (0x34U)             /* 远程控制启动充电 */
#define IOT_GN_CMD_REMOTE_STOP_CHARGE           (0x36U)             /* 远程控制停止充电 */
#define IOT_GN_CMD_ORDER_RECORD_RSP             (0x40U)             /* 四类/多类电价交易记录应答 */
#define IOT_GN_CMD_UPDATE_ACCOUNT_MONEY         (0x42U)             /* 远程更新账户余额 */
#define IOT_GN_CMD_OFFLINE_CARD                 (0x44U)             /* 离线卡下发 */
#define IOT_GN_CMD_OFFLINE_CARD_CLEAR           (0x46U)             /* 离线卡清除 */
#define IOT_GN_CMD_OFFLINE_CARD_SEARCH          (0x48U)             /* 离线卡查询 */
#define IOT_GN_CMD_SYNC_TIME                    (0x56U)             /* 远程对时 */
#define IOT_GN_CMD_SET_BILLMODE_4RATE           (0x58U)             /* 四类电价计费模型设置 */
#define IOT_GN_CMD_SET_BILLMODE_MULTIRATE       (0x54U)             /* 多类电价计费模型设置 */
#define IOT_GN_CMD_SET_QRCODE                   (0x5AU)             /* 设置二维码 */
#define IOT_GN_CMD_SET_DEV_WORK_PARAM           (0x5CU)             /* 充电设备工作参数设置 */
#define IOT_GN_CMD_QUERY_DEV_WORK_PARAM         (0x5EU)             /* 充电设备工作参数查询 */
#define IOT_GN_CMD_REBOOT                       (0x92U)             /* 设置远程重启 */
#define IOT_GN_CMD_UPDATE                       (0x94U)             /* 设置远程更新 */

#define IOT_GN_CMD_RECV_COUNT                   (22U)
/******************************************************************************
*    Enum Definition
******************************************************************************/

typedef enum
{
    eIotGNStopReason_Null = 0,
    eIotGNStopReason_CpVoltAbnor = 0x01,          /* CP电压异常 */
    eIotGNStopReason_CpGroundFault = 0x02,        /* CP对地短路 */
    eIotGNStopReason_PEBreakFault = 0x03,         /* PE接地故障 */
    eIotGNStopReason_LeakageCurrErr = 0x07,       /* 漏电故障 */

    eIotGNStopReason_MeterCalcErr = 0x0B,         /* 电能计量故障 */
    eIotGNStopReason_AmountFault = 0x0C,          /* 充电中金额异常 */

    eIotGNStopReason_JcqMaloperation = 0x13,      /* 继电器误动拒动故障 */
    eIotGNStopReason_JcqSynechiaFault = 0x14,     /* 继电器粘连故障 */

    eIotGNStopReason_GunTempErr = 0x1B,           /* 枪过温故障 */
    eIotGNStopReason_DataBaseErr = 0x1D,          /* 数据库错误 */

    eIotGNStopReason_ShortCut = 0x1E,             /* 输出短路 */

    eIotGNStopReason_BillModeErr = 0x21,          /* 计费模型异常 */
    eIotGNStopReason_StartTimeout = 0x20,         /* 启动超时 */
    eIotGNStopReason_DiodeStop = 0x25,            /* 车辆无二极管 */
    eIotGNStopReason_KeyStop = 0x27,              /* 按键停止 */


    eIotGNStopReason_AppStop = 0x40,              /* App停止 */
    eIotGNStopReason_StopByEnergy = 0x42,         /* 按电量停止 */
    eIotGNStopReason_StopByMoney = 0x43,          /* 按金额停止 */
    eIotGNStopReason_StopByTime = 0x44,           /* 按时间停止 */
    eIotGNStopReason_ManualStop = 0x45,           /* 手动停止 */
    eIotGNStopReason_CarStop = 0x46,              /* 车辆停止 */

    eIotGNStopReason_OtherErr = 0x65,             /* 其它原因 */

    eIotGNStopReason_GunDisconnect = 0x6B,        /* 控制导引断开 */
    eIotGNStopReason_MeterCommErr = 0x6D,         /* 电表通信中断 */
    eIotGNStopReason_SumNoEnough = 0x6E,          /* 余额不足 */
    eIotGNStopReason_EmergencyStop = 0x72,        /* 急停开入 */
    eIotGNStopReason_TempErr = 0x74,              /* 温度异常 */
    eIotGNStopReason_OverCurr = 0x75,             /* 输出过流 */
    eIotGNStopReason_LittleCurr = 0x76,           /* 小电流 */
    eIotGNStopReason_VoltageErr = 0x79,           /* 电压异常（含过欠压） */
    eIotGNStopReason_CurrentErr = 0x7A,           /* 电流异常 */

    eIotGNStopReason_PowerOff = 0x83,             /* 掉电故障 */
    eIotGNStopReason_NoExpectedErr = 0x90,        /* 未知原因 */
}IotGNStopReason_Enum;

typedef enum
{
    /* 正常停止原因 */
    eIotDXLStopReason_Null = 0x00,                    /* 无 */
    eIotDXLStopReason_AppStop = 0x01,                 /* APP远程停止 */
    eIotDXLStopReason_Soc100 = 0x02,                  /* SOC达到100% */
    eIotDXLStopReason_EnergyReached = 0x03,           /* 充电电量满足设定条件 */
    eIotDXLStopReason_AmountReached = 0x04,           /* 充电金额满足设定条件 */
    eIotDXLStopReason_TimeReached = 0x05,             /* 充电时间满足设定条件 */
    eIotDXLStopReason_ManualStop = 0x06,              /* 手动停止充电 */
    eIotDXLStopReason_SocReached = 0x07,              /* SOC达到设定值 */
    eIotDXLStopReason_InsufficientBalance = 0x08,     /* 余额不足 */
    eIotDXLStopReason_VehicleActiveStop = 0x09,       /* 车辆主动停止 */
    /* 0x0A ~ 0x1F 为预留 */

    /* 故障停止原因 */
    eIotDXLStopReason_InputOverVol = 0x20,            /* 交流输入过压故障 */
    eIotDXLStopReason_OutputOverCurr = 0x21,          /* 交流输出过流故障 */
    eIotDXLStopReason_InputUnderVol = 0x22,           /* 交流输入欠压故障 */
    eIotDXLStopReason_InputMissPhase = 0x23,          /* 交流输入缺相故障 */
    eIotDXLStopReason_LineReversed = 0x24,            /* 火零反接故障 */
    eIotDXLStopReason_HmiCommErr = 0x25,              /* 人机交互通信故障 */
    eIotDXLStopReason_RelayAdhesion = 0x26,           /* 继电器粘连故障 */
    eIotDXLStopReason_RelayMaloperation = 0x27,       /* 继电器误动拒动故障 */
    eIotDXLStopReason_LeakageCurrErr = 0x28,          /* 漏电故障 */
    eIotDXLStopReason_RcdSelfCheckErr = 0x29,         /* 漏电自检故障 */
    eIotDXLStopReason_EnvOverTemp = 0x2A,             /* 环境过温故障 */
    eIotDXLStopReason_GunOverTemp = 0x2B,             /* 枪头过温故障 */
    eIotDXLStopReason_PlugOverTemp = 0x2C,            /* 插头过温故障 */
    eIotDXLStopReason_MeterCalcErr = 0x2D,            /* 电能计量异常 */
    eIotDXLStopReason_MeterCommErr = 0x2E,            /* 电表通信故障 */
    eIotDXLStopReason_StorageErr = 0x2F,              /* 数据存储错误 */
    eIotDXLStopReason_ReaderCommErr = 0x30,           /* 读卡器通信故障 */
    eIotDXLStopReason_DeviceOta = 0x31,               /* 设备OTA中 */
    eIotDXLStopReason_DeviceOccupied = 0x32,          /* 设备已占用 */
    eIotDXLStopReason_DeviceDisabled = 0x33,          /* 设备被禁用 */
    eIotDXLStopReason_GunDisconnected = 0x34,         /* 充电枪未连接 */
    eIotDXLStopReason_GroundErr = 0x35,               /* 接地错误 */
    eIotDXLStopReason_NoDiode = 0x36,                 /* 车辆不存在二极管 */
    eIotDXLStopReason_CpVoltAbnor = 0x37,             /* CP电压错误 */
    eIotDXLStopReason_CpGroundFault = 0x38,           /* CP接地错误 */
    eIotDXLStopReason_StartTimeout = 0x39,            /* 启动超时 */
    eIotDXLStopReason_LittleCurrStop = 0x3A,          /* 小电流停止 */
    eIotDXLStopReason_EmergencyStop = 0x3B,           /* 急停故障 */
    eIotDXLStopReason_OutputShortCircuit = 0x3C,      /* 充电前输出短路故障 */
    eIotDXLStopReason_PowerDownStop = 0x3D,           /* 掉电停止 */
    eIotDXLStopReason_BillingModelErr = 0x3E,         /* 计费模型异常 */

    eIotDXLStopReason_Unknown = 0xFF,                 /* 未知原因停止 */
} IotDXLStopReason_Enum;

/* 离线卡前置校验 */
typedef enum
{
    eIotGNOfflineCardValid = 0,          /* 有效卡号 */
    eIotGNOfflineCardOutOfBounds = 1,    /* 数据越界 */
    eIotGNOfflineCardInvalid = 2,        /* 无效卡号(全0/全FF) */
} IotGNOfflineCardValidateResult_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotGN_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotGN_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    IotGN_pSendPackFuncType pSendFunc;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotGNSendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	uint8_t cmdType; 
	IotGN_pRecvParseFuncType pRecvParse;
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotGNRecvCtrl_Struct;

typedef struct 
{
    uint8_t head[2];
    uint8_t version[2];
    uint8_t seq[2];
    uint8_t encryptFlag;
    uint8_t cmd;
    uint8_t dataLen[2];
}IotGNFrameHead_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_IOT_PROTO_GN_TYPES_H_ */






















