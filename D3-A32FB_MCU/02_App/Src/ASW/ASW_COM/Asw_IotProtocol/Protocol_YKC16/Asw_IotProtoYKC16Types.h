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
#ifndef ASW_IOT_PROTO_YKC16_TYPES_H_
#define ASW_IOT_PROTO_YKC16_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 通信协议版本 */
#define IOT_YKC16_PROTOCOL_VERSION                 (16U)
#define IOT_TT24_PROTOCOL_VERSION                  (24U)

/* 通信协议头定义--YKC1.6协议 */
#define IOT_YKC16_HEAD                            (0x68U)                

/* 通信buff缓存定义 */
#define IOT_YKC16_TXRX_BUFFER_SIZE                 (3072U)	

/* 计费模型类型定义 */
#define IOT_YKC16_BILLMODE_RATE_TYPE_4             4

/* 实时数据上报周期定义 */
#define IOTYKC16_CFG_IDLE_REALDATA_CYCLE           (5 * 60 * 1000)
#define IOTYKC16_CFG_CHARGING_REALDATA_CYCLE       (15 * 1000)

/* 日志接口函数定义 */
#define IOTYKC16_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTYKC16_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/* 充电最小余额，1元，保留2位小数 */
#define IOTYKC16_CFG_CHARGE_MIN_ACCOUNT_MONEY      (100)   

/* 协议CMD 定义 */
#define IOT_YKC16_CMDTYPE_REQUSET			       (0x00U)             /* 请求 */
#define IOT_YKC16_CMDTYPE_RESPONSE                 (0x01U)             /* 应答 */
#define IOT_YKC16_CMD_NULL                         (0xFFU)             /* 无效 */

/* 协议CMD 发送定义 */
#define IOT_YKC16_CMD_LOGIN_REQ                    (0x01U)             /* 登陆 */
#define IOT_YKC16_CMD_HEARTBEAT_REQ                (0x03U)             /* 心跳请求 */
#define IOT_YKC16_CMD_BILLMODE_VERIFY_REQ          (0x05U)             /* 计费模型验证请求 */
#define IOT_YKC16_CMD_BILLMODE_REQ                 (0x09U)             /* 计费模型请求 */
#define IOT_YKC16_CMD_REPORT_REALDATA              (0x13U)             /* 上报实时数据 */
#define IOT_YKC16_CMD_CALL_REALDATA_ACK            (0xF13U)            /* 召测实时数据应答 */
#define IOT_YKC16_CMD_PILE_START_CHARGE_REQ        (0x31U)             /* 充电桩主动申请启动充电 */
#define IOT_YKC16_CMD_REMOTE_STOP_CHARGE_RSP       (0x35U)             /* 远程控制停止充电应答 */
#define IOT_YKC16_CMD_REMOTE_START_CHARGE_RSP      (0x33U)             /* 远程控制启动充电应答 */
#define IOT_YKC16_CMD_ORDER_RECORD_REQ             (0x3BU)             /* 四类电价交易记录 */
#define IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY_RSP     (0x41U)             /* 远程更新账户余额应答 */
#define IOT_YKC16_CMD_SYNC_TIME_RSP                (0x55U)             /* 远程对时应答 */
#define IOT_YKC16_CMD_SET_BILLMODE_4RATE_RSP       (0x57U)             /* 设置四类电价计费模型应答 */
#define IOT_YKC16_CMD_SET_QRCODE_RSP               (0x59U)             /* 设置二维码应答 */
#define IOT_YKC16_CMD_REBOOT_RSP                   (0x91U)             /* 设置远程重启应答 */
#define IOT_YKC16_CMD_UPDATE_RSP                   (0x93U)             /* 设置远程更新应答 */

#define IOT_YKC16_CMD_SEND_COUNT                   (16U)

/* 协议CMD 接收定义 */
#define IOT_YKC16_CMD_LOGIN_RSP                    (0x02U)             /* 登陆应答 */
#define IOT_YKC16_CMD_HEARTBEAT_RSP                (0x04U)             /* 心跳应答 */
#define IOT_YKC16_CMD_BILLMODE_VERIFY_RSP          (0x06U)             /* 计费模型验证请求应答 */
#define IOT_YKC16_CMD_BILLMODE_4RATE_RSP           (0x0AU)             /* 4类电价应答 */
#define IOT_YKC16_CMD_CALL_REALDATA                (0x12U)             /* 召测实时数据 */
#define IOT_YKC16_CMD_PILE_START_CHARGE_RSP        (0x32U)             /* 充电桩主动申请启动充电应答 */
#define IOT_YKC16_CMD_REMOTE_START_CHARGE          (0x34U)             /* 远程控制启动充电 */
#define IOT_YKC16_CMD_REMOTE_STOP_CHARGE           (0x36U)             /* 远程控制停止充电 */
#define IOT_YKC16_CMD_ORDER_RECORD_RSP             (0x40U)             /* 四类电价交易记录应答 */
#define IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY         (0x42U)             /* 远程更新账户余额 */
#define IOT_YKC16_CMD_SYNC_TIME                    (0x56U)             /* 远程对时 */
#define IOT_YKC16_CMD_SET_BILLMODE_4RATE           (0x58U)             /* 四类电价计费模型设置 */
#define IOT_YKC16_CMD_SET_QRCODE                   (0x5AU)             /* 设置二维码 */
#define IOT_YKC16_CMD_REBOOT                       (0x92U)             /* 设置远程重启 */
#define IOT_YKC16_CMD_UPDATE                       (0x94U)             /* 设置远程更新 */

#define IOT_YKC16_CMD_RECV_COUNT                   (15U)
/******************************************************************************
*    Enum Definition
******************************************************************************/

typedef enum
{
    eIotYKC16StopReason_Null = 0,
    eIotYKC16StopReason_CpVoltAbnor = 0x01,          /* CP电压异常 */
    eIotYKC16StopReason_CpGroundFault = 0x02,        /* CP对地短路 */
    eIotYKC16StopReason_PEBreakFault = 0x03,         /* PE接地故障 */
    eIotYKC16StopReason_LeakageCurrErr = 0x07,       /* 漏电故障 */

    eIotYKC16StopReason_MeterCalcErr = 0x0B,         /* 电能计量故障 */
    eIotYKC16StopReason_AmountFault = 0x0C,          /* 充电中金额异常 */

    eIotYKC16StopReason_JcqMaloperation = 0x13,      /* 继电器误动拒动故障 */
    eIotYKC16StopReason_JcqSynechiaFault = 0x14,     /* 继电器粘连故障 */

    eIotYKC16StopReason_GunTempErr = 0x1B,           /* 枪过温故障 */
    eIotYKC16StopReason_DataBaseErr = 0x1D,          /* 数据库错误 */

    eIotYKC16StopReason_ShortCut = 0x1E,             /* 输出短路 */

    eIotYKC16StopReason_BillModeErr = 0x21,          /* 计费模型异常 */
    eIotYKC16StopReason_StartTimeout = 0x20,         /* 启动超时 */
    eIotYKC16StopReason_DiodeStop = 0x25,            /* 车辆无二极管 */
    eIotYKC16StopReason_KeyStop = 0x27,              /* 按键停止 */


    eIotYKC16StopReason_AppStop = 0x40,              /* App停止 */
    eIotYKC16StopReason_CarStop = 0x41,              /* 充满停止(车辆停止) */
    eIotYKC16StopReason_StopByEnergy = 0x42,         /* 按电量停止 */
    eIotYKC16StopReason_StopByMoney = 0x43,          /* 按金额停止 */
    eIotYKC16StopReason_StopByTime = 0x44,           /* 按时间停止 */
    eIotYKC16StopReason_ManualStop = 0x45,           /* 手动停止 */

    eIotYKC16StopReason_OtherErr = 0x65,             /* 其它原因 */

    eIotYKC16StopReason_GunDisconnect = 0x6B,        /* 控制导引断开 */
    eIotYKC16StopReason_MeterCommErr = 0x6D,         /* 电表通信中断 */
    eIotYKC16StopReason_SumNoEnough = 0x6E,          /* 余额不足 */
    eIotYKC16StopReason_EmergencyStop = 0x72,        /* 急停开入 */
    eIotYKC16StopReason_TempErr = 0x74,              /* 温度异常 */
    eIotYKC16StopReason_OverCurr = 0x75,             /* 输出过流 */
    eIotYKC16StopReason_LittleCurr = 0x76,           /* 小电流 */
    eIotYKC16StopReason_VoltageErr = 0x79,           /* 电压异常（含过欠压） */
    eIotYKC16StopReason_CurrentErr = 0x7A,           /* 电流异常 */

    eIotYKC16StopReason_PowerOff = 0x83,             /* 掉电故障 */
    eIotYKC16StopReason_NoExpectedErr = 0x90,        /* 未知原因 */

    eIotTT24StopReason_CardStop = 0xE2,     	     /* 刷卡停充 */
}IotYKC16StopReason_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotYKC16_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotYKC16_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

/* 发送控制 */
typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    IotYKC16_pSendPackFuncType pSendFunc;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotYKC16SendCtrl_Struct;

/* 接收控制 */
typedef struct 
{
	uint16_t cmd;
	uint8_t cmdType; 
	IotYKC16_pRecvParseFuncType pRecvParse;
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotYKC16RecvCtrl_Struct;

typedef struct 
{
    uint8_t head;
    uint8_t dataLen;
    uint8_t seq[2];
    uint8_t encryptFlag;
    uint8_t cmd;
}IotYKC16FrameHead_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_IOT_PROTO_GN_TYPES_H_ */






















