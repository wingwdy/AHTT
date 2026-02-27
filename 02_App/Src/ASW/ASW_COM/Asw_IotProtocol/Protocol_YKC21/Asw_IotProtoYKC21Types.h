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
#ifndef ASW_IOT_PROTO_YKC21_TYPES_H_
#define ASW_IOT_PROTO_YKC21_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "SysCfg.h"
#include "Asw_lotProtoYKC21aes.h"
 
/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 通信协议版本 */
#define IOT_YKC21_PROTOCOL_VERSION                 (20101U) //2.1.1

/* 通信协议头定义--YKC21协议 */
#define IOT_YKC21_PLUS_HEAD                       (0x68U)                


 
/* 通信buff缓存定义 */
#define IOT_YKC21_TXRX_BUFFER_SIZE                 (3072U)

/* 计费模型类型定义 */
#define IOT_YKC21_BILLMODE_RATE_TYPE_MULT          48

/* 实时数据上报周期定义 */
#define IOTYKC21_CFG_IDLE_REALDATA_CYCLE           (5 * 60 * 1000)
#define IOTYKC21_CFG_CHARGING_REALDATA_CYCLE       (15 * 1000)

/* 日志接口函数定义 */
#define IOTYKC21_CFG_LogPrint(fmt, ...)            DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/* 协议CMD 定义 */
#define IOT_YKC21_CMDTYPE_REQUSET			       (0x00U)
#define IOT_YKC21_CMDTYPE_RESPONSE                 (0x01U)
#define IOT_YKC21_CMD_NULL                         (0x00U)             /* 无效 */

/* 协议CMD 发送定义 */
#define IOT_YKC21_CMD_LOGIN_REQ                    (0x01U)             /* 登陆 */
#define IOT_YKC21_CMD_HEARTBEAT_REQ                (0x03U)             /* 心跳请求 */
#define IOT_YKC21_CMD_BILLMODE_VERIFY_REQ          (0x05U)             /* 计费模型验证请求 */
#define IOT_YKC21_CMD_BILLMODE_REQ                 (0x09U)             /* 计费模型请求 */
#define IOT_YKC21_CMD_REPORT_REALDATA              (0x13U)             /* 上报实时数据 */
#define IOT_YKC21_CMD_CALL_REALDATA_ACK            (0xF13U)            /* 召测实时数据应答 */
#define IOT_YKC21_CMD_REMOTE_STOP_CHARGE_RSP       (0x35U)             /* 远程控制停止充电应答 */
#define IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ       (0x3DU)             /* 多类电价交易记录 */
#define IOT_YKC21_CMD_MULTI_ORDER_RECORD_ACK       (0xF3DU)            /* 召唤交易记录 */
#define IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY_RSP     (0x41U)             /* 远程更新账户余额应答 */
#define IOT_YKC21_CMD_FAULTREST_REQ                (0x4BU)             /* 设备故障复位 */
#define IOT_YKC21_CMD_RECORD_RSP                   (0x4CU)             /* 召测交易记录确认应答 */
#define IOT_YKC21_CMD_FAULT_REQ                    (0x50U)             /* 设备故障上报 */
#define IOT_YKC21_CMD_POWERCHANG_RSP               (0x51U)             /* 功率修改应答 */
#define IOT_YKC21_CMD_SYNC_TIME_RSP                (0x55U)             /* 远程对时应答 */
#define IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE_RSP   (0x57U)             /* 设置多类电价计费模型应答 */
#define IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP         (0x59U)             /* 默认最大功率下发应答 */
#define IOT_YKC21_CMD_SET_QRCODE_RSP               (0x5AU)             /* 设置二维码应答 */
#define IOT_YKC21_CMD_SET_PARAM_RSP                (0x5EU)             /* 参数设置应答 */
#define IOT_YKC21_CMD_REBOOT_RSP                   (0x91U)             /* 设置远程重启应答 */
#define IOT_YKC21_CMD_SET_FTP_RSP                  (0x93U)             /* 平台设远程升级程序应答 */
#define IOT_YKC21_CMD_SET_KEY_RSP                  (0x95U)             /* 密钥更新应答 */
#define IOT_YKC21_CMD_PILE_START_CHARGE_REQ        (0xA5U)             /* 充电桩主动申请启动充电 */
#define IOT_YKC21_CMD_REMOTE_START_CHARGE_RSP      (0xA7U)             /* 远程控制启动充电应答 */

#define IOT_YKC21_CMD_SEND_COUNT                   (24U)

/* 协议CMD 接收定义 */
#define IOT_YKC21_CMD_LOGIN_RSP                    (0x02U)             /* 登陆应答 */
#define IOT_YKC21_CMD_HEARTBEAT_RSP                (0x04U)             /* 心跳应答 */
#define IOT_YKC21_CMD_BILLMODE_VERIFY_RSP          (0x06U)             /* 计费模型验证请求应答 */
#define IOT_YKC21_CMD_BILLMODE_MUTIRATE_RSP        (0x0AU)             /* 多类电价应答 */
#define IOT_YKC21_CMD_CALL_REALDATA                (0x12U)             /* 召测实时数据 */
#define IOT_YKC21_CMD_REMOTE_STOP_CHARGE           (0x36U)             /* 远程控制停止充电 */
#define IOT_YKC21_CMD_ORDER_RECORD_RSP             (0x40U)             /* 多类电价交易记录应答 */
#define IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY         (0x42U)             /* 远程更新账户余额 */
#define IOT_YKC21_CMD_FAULT_RSP                    (0x49U)             /* 设备故障上送回复确认 */
#define IOT_YKC21_CMD_FAULTREST_RSP                (0x4AU)             /* 设备故障复位上送回复确认 */
#define IOT_YKC21_CMD_Call_RECORD                  (0x4DU)             /* 交易记录召唤 */
#define IOT_YKC21_CMD_SET_POWERCHANG               (0x52U)             /* 功率修改 */
#define IOT_YKC21_CMD_SYNC_TIME                    (0x56U)             /* 远程对时 */
#define IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE       (0x58U)             /* 多类电价计费模型设置 */
#define IOT_YKC21_CMD_SET_QRCODE                   (0x5BU)             /* 设置二维码 */
#define IOT_YKC21_CMD_SET_PARAM                    (0x5FU)             /* 参数设置 */
#define IOT_YKC21_CMD_SET_POWERDEFAULT_MAX         (0x60U)             /* 最大功率下发 */
#define IOT_YKC21_CMD_REBOOT                       (0x92U)             /* 设置远程重启 */
#define IOT_YKC21_CMD_SET_FTP                      (0x94U)             /* 平台设远程升级程序 */
#define IOT_YKC21_CMD_SET_KEY                      (0x96U)             /* 密钥更新 */
#define IOT_YKC21_CMD_PILE_START_CHARGE_RSP        (0xA6U)             /* 充电桩主动申请启动充电应答 */
#define IOT_YKC21_CMD_REMOTE_START_CHARGE          (0xA8U)             /* 远程控制启动充电 */

#define IOT_YKC21_CMD_RECV_COUNT                   (22U)
/******************************************************************************
*    Enum Definition
******************************************************************************/


typedef enum
{
    /* 非ykc2.1协议对应部分 */
    eIotYKC21StopReason_Null             = 0,
     eIotYKC21StopReason_CpVoltAbnor     = 0x01,    /* CP电压异常 */
    eIotYKC21StopReason_CpGroundFault    = 0x02,    /* CP对地短路 */
    eIotYKC21StopReason_PEBreakFault     = 0x03,    /* PE接地故障 */
    eIotYKC21StopReason_LeakageCurrErr   = 0x07,    /* 漏电故障 */

    eIotYKC21StopReason_MeterCalcErr     = 0x0B,    /* 电能计量故障 */
    eIotYKC21StopReason_AmountFault      = 0x0C,    /* 充电中金额异常 */

    eIotYKC21StopReason_JcqMaloperation  = 0x13,    /* 继电器误动拒动故障 */
    eIotYKC21StopReason_JcqSynechiaFault = 0x14,    /* 继电器粘连故障 */

    eIotYKC21StopReason_GunTempErr       = 0x1B,    /* 枪过温故障 */
    eIotYKC21StopReason_DataBaseErr      = 0x1D,    /* 数据库错误 */

    eIotYKC21StopReason_ShortCut         = 0x1E,    /* 输出短路 */

    eIotYKC21StopReason_BillModeErr      = 0x21,    /* 计费模型异常 */
    eIotYKC21StopReason_StartTimeout     = 0x20,    /* 启动超时 */
    eIotYKC21StopReason_DiodeStop        = 0x25,    /* 车辆无二极管 */
    eIotYKC21StopReason_KeyStop          = 0x27,    /* 按键停止 */
 
    /* ykc2.1协议附录13.1 */
    /* 充电完成 */
    eIotYKC21StopReason_AppStop          = 0x40,    /* App停止 */
    eIotYKC21StopReason_CarStop          = 0x41,    /* SOC 达到 100% ；车辆停止结束充电*/
    eIotYKC21StopReason_StopByEnergy     = 0x42,    /* 按电量停止 */
    eIotYKC21StopReason_StopByMoney      = 0x43,    /* 按金额停止 */
    eIotYKC21StopReason_StopByTime       = 0x44,    /* 按时间停止 */
    eIotYKC21StopReason_ManualStop       = 0x45,    /* 手动停止 */
    eIotYKC21StopReason_reserve0         = 0x46,    /* 预留 */
    eIotYKC21StopReason_reserve1         = 0x47,    /* 预留 */
    eIotYKC21StopReason_reserve2         = 0x48,    /* 预留 */
    eIotYKC21StopReason_reserve3         = 0x49,    /* 预留 */


    /* 充电启动失败 */
    eIotYKC21StopReason_startfail_ControlSystemFault    = 0x4A,    /* 充电启动失败，充电桩控制系统故障(需要重启或自动恢复) */
    eIotYKC21StopReason_startfail_GunDisconnect		    = 0x4B,    /* 控制导引断开 */
    eIotYKC21StopReason_startfail_CircuitBreakerTrip    = 0x4C,    /* 断路器跳位 */
    eIotYKC21StopReason_startfail_MeterCommBreak	    = 0X4D,    /* 电表通信中断 */
    eIotYKC21StopReason_startfail_InsufficientBalance   = 0x4E,    /* 余额不足 */
    eIotYKC21StopReason_startfail_ChargingModuleFault   = 0x4F,    /* 充电模块故障 */
    eIotYKC21StopReason_startfail_EmergencyStopInput    = 0x50,    /* 急停开入 */
    eIotYKC21StopReason_startfail_TemperatureAbnormal   = 0x53,    /* 温度异常 */
    eIotYKC21StopReason_startfail_reserve0              = 0x58,    /* 预留 */
    eIotYKC21StopReason_OtherErr                        = 0x65,    /* 其它原因 */
   
    
    /* 充电异常中止 */
    eIotYKC21StopReason_ControlSystemLock = 0x6A,    /* 系统闭锁 */
    eIotYKC21StopReason_GunDisconnect     = 0x6B,    /* 控制导引断开 */
    eIotYKC21StopReason_MeterCommErr      = 0x6D,    /* 电表通信中断 */
    eIotYKC21StopReason_SumNoEnough       = 0x6E,    /* 余额不足 */
    eIotYKC21StopReason_EmergencyStop     = 0x72,    /* 急停开入 */
    eIotYKC21StopReason_TempErr           = 0x74,    /* 温度异常 */
    eIotYKC21StopReason_OverCurr          = 0x75,    /* 输出过流 */
    eIotYKC21StopReason_LittleCurr        = 0x76,    /* 小电流 */
    eIotYKC21StopReason_VoltageErr        = 0x79,    /* 电压异常（含过欠压） */
    eIotYKC21StopReason_CurrentErr        = 0x7A,    /* 电流异常 */
    eIotYKC21StopReason_PowerOff          = 0x83,    /* 掉电故障 */
    eIotYKC21StopReason_RechargEnergy     = 0x8A,    /* 可充电量余额不足 */
    
    eIotYKC21StopReason_NoExpectedErr     = 0x90,    /* 未知原因 */
}IotYKC21StopReason_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotYKC21_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotYKC21_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    IotYKC21_pSendPackFuncType pSendFunc;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotYKC21SendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	uint8_t cmdType; 
	IotYKC21_pRecvParseFuncType pRecvParse;
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotYKC21RecvCtrl_Struct;

typedef struct 
{
    uint8_t head;
    uint8_t dataLen[2];
    uint8_t seq[2];
    uint8_t sendcp56time[7];
    uint8_t encryptFlag;
    uint8_t cmd;
}IotYKC21FrameHead_Struct;


typedef struct {
    uint8_t err_type;
    uint8_t err_plat_type;
    uint16_t err_code;
} err_map_t;

typedef enum
{
    ePlatType_N                = 0x00,     // 无效值 */
    ePlatType_A                = 0x01,     // 车故障
    ePlatType_B                = 0x02,     // 车桩交互故障
    ePlatType_C                = 0x03,     // 桩/平台故障
    ePlatType_D                = 0x04,     // 桩故障
    ePlatType_E                = 0x05,     // 自定义故障
}ERR_PLATFORM;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern struct AES_ctx g_ex;
extern uint8_t random_key_A[16];            	// 随机密钥A
#define RSA_KEY_LEN			128
/******************************************************************************
*    Global Function Prototypes
******************************************************************************/



#endif /* ASW_IOT_PROTO_YKC21_TYPES_H_ */





















