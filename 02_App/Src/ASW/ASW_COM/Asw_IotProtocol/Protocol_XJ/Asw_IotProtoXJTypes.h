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
#ifndef ASW_IOT_PROTO_XJ_TYPES_H_
#define ASW_IOT_PROTO_XJ_TYPES_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "myMalloc.h"
#include "Ms_Nvm.h"
#include "Asw_ErrorHandle.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 对应协议版本号为： v4.0.0.1 */
#define IOT_XJ_PROTOCOL_VER0                      0x01
#define IOT_XJ_PROTOCOL_VER1                      0x00
#define IOT_XJ_PROTOCOL_VER2                      0x00
#define IOT_XJ_PROTOCOL_VER3                      0x04

/* 通信协议头定义*/
#define IOT_XJ_HEAD1                             (0x7DU)                
#define IOT_XJ_HEAD2                             (0xD0U)

#define IOT_XJ_TOPIC_LEN                         32

/* 通信buff缓存定义 */
#define IOT_XJ_TXRX_BUFFER_SIZE                 (2048U)

#define IOT_XJ_CREDENTIAL_TYPE                   "MQTT_BASIC"

/* 故障上报缓存定义 */
#define IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE        16

/* 事件上报缓存定义*/
#define IOT_XJ_EVENT_REPORT_QUEUE_SIZE          4

/* 协议CMD 定义 */
#define IOT_XJ_CMDTYPE_REQUSET			        (0x00U)
#define IOT_XJ_CMDTYPE_RESPONSE                 (0x01U)
#define IOT_XJ_CMD_NULL                         (0xFFFFU)             

/* 协议CMD 发送定义 */
#define IOT_XJ_CMD_SET_INTEGER_PARA_RESPONSE    (502U)        /* 充电桩整型参数设置应答 */
#define IOT_XJ_CMD_SET_COMMON_PARA_RESPONSE     (508U)        /* 充电桩通用参数设置应答 */
#define IOT_XJ_CMD_REMOTE_CONFIG_RESPONSE       (512U)        /* 充电桩终端控制应答 */
#define IOT_XJ_CMD_QUERY_COMMON_PARA_RESPONSE   (514U)        /* 后台服务器查询充电桩通用保存参数应答 */
#define IOT_XJ_CMD_SET_START_CHARGE_RESPONSE    (8U)          /* 后台服务器下发开始充电指令应答 */
#define IOT_XJ_CMD_SET_STOP_CHARGE_RESPONSE     (12U)         /* 后台服务器下发终止充电指令应答 */
#define IOT_XJ_CMD_SET_POWER_ALLOC_RESPONSE     (20U)         /* 后台服务器下发远程功率分配指令应答 */
#define IOT_XJ_CMD_SEND_HEART                   (102U)        /* 上传心跳包 */
#define IOT_XJ_CMD_SEND_STATE_INFO              (104U)        /* 上报状态信息 */
#define IOT_XJ_CMD_SEND_SIGN_INFO               (106U)        /* 上报签到信息 */
#define IOT_XJ_CMD_SEND_ORDER_INFO              (202U)        /* 上报订单信息 */
#define IOT_XJ_CMD_SET_RATEMODE_RESPONSE        (1310U)       /* 桩端响应计费模型设置 */
#define IOT_XJ_CMD_SEND_EVENT                   (108U)        /* 上报事件 */
#define IOT_XJ_CMD_SEND_ERROR_INFO              (118U)        /* 上报故障信息 */
#define IOT_XJ_CMD_SEND_WARN_INFO               (120U)        /* 上报告警信息 */
#define IOT_XJ_CMD_SET_OTA_RESPONSE             (1102U)       /* 充电桩应答OTA指令 */
#define IOT_XJ_CMD_REQUEST_CARD_AUTH            (34U)         /* 充电桩请求刷卡授权 */
#define IOT_XJ_CMD_REQUEST_CARD_CHARGE          (36U)         /* 充电桩请求刷卡启动充电 */

#define IOT_XJ_CMD_SEND_COUNT                   (18U)

/* 协议CMD 接收定义 */
#define IOT_XJ_CMD_SET_INTEGER_PARA             (501U)        /* 后台服务器设置整型参数 */
#define IOT_XJ_CMD_SET_COMMON_PARA              (507U)        /* 充电桩通用参数设置 */
#define IOT_XJ_CMD_REMOTE_CONFIG                (511U)        /* 后台服务器控制终端 */
#define IOT_XJ_CMD_QUERY_COMMON_PARA            (513U)        /* 后台服务器查询充电桩通用保存参数 */
#define IOT_XJ_CMD_SET_START_CHARGE             (7U)          /* 后台服务器下发开始充电指令 */
#define IOT_XJ_CMD_SET_STOP_CHARGE              (11U)         /* 后台服务器下发终止充电指令应答 */
#define IOT_XJ_CMD_SET_POWER_ALLOC              (19U)         /* 后台服务器下发远程功率分配指令 */
#define IOT_XJ_CMD_SEND_HEART_RESPONSE          (101U)        /* 服务器应答心跳包信息 */
#define IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE     (103U)        /* 服务器应答状态信息 */
#define IOT_XJ_CMD_SEND_SIGN_INFO_RESPONSE      (105U)        /* 服务器应答签到信息 */
#define IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE     (201U)        /* 上报订单信息应答 */
#define IOT_XJ_CMD_SET_RATEMODE                 (1309U)       /* 服务器下发计费模型 */
#define IOT_XJ_CMD_SEND_EVENT_RESPONSE          (107U)        /* 服务器应答充电桩上报事件 */
#define IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE     (117U)        /* 服务器应答充电桩故障信息 */
#define IOT_XJ_CMD_SEND_WARN_INFO_RESPONSE      (119U)        /* 服务器应答充电桩告警信息 */
#define IOT_XJ_CMD_SET_OTA                      (1101U)       /* 服务器下发OTA指令 */
#define IOT_XJ_CMD_REQUEST_CARD_AUTH_RESPONSE   (33U)         /* 充电桩应答刷卡授权结果 */
#define IOT_XJ_CMD_REQUEST_CARD_CHARGE_RESPONSE (35U)         /* 充电桩应答刷卡启动充电请求 */

#define IOT_XJ_CMD_RECV_COUNT                   (18U)

/* 日志接口函数定义 */
#define IOTXJ_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)
#define IOTXJ_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/* 小桔专用户名密码 */
#define IOTXJ_CFG_UAT_USERNAME			        "91110113MA01CF8F83"		
#define IOTXJ_CFG_UAT_PASSWORD                  "JvL8so96zyM6ppaTPfEe2JRt9lsnJ07EhT/oQhcCAyuE7Eyo5RoQ0MXBIXyyD13cNN2LqK3ViHLKCFbE/IkKXpeDfIMpCWt8niVn29Vpaf38gtVf0ne7RWPpHC4PlP+gIWLPRVUV1ei1RSeCWfJ4GtDJ0fuOuq7ij0gq/4BIiKU="
/* 公牛专用用户名密码 */
#define IOTXJ_CFG_PRD_USERNAME                  "91330282671205242Y"
#define IOTXJ_CFG_PRD_PASSWORD                  "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCHtJBa3rbGOTUD7ZhdJXLZzzX3A50q4Pq+Qct6kPwZoMeYWh51QaVX3svT+HyPpXoPscPrgFW9VQDVzvq+CbleW6QFevbcgxbB2mxjZNrK6z8N2wCy1b9qiIKbmmPKZTgvGQnGvmiO5NPFYRt7A604y2JasJMBJhX/o7/D2G2Q8wIDAQAB"




/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum	
{
	eIotXJGunStatus_Idle = 0,				/* 空闲状态 */
	eIotXJGunStatus_Connected = 1,			/* 已连接状态 */
	eIotXJGunStatus_Charging = 2,			/* 充电中状态 */
	eIotXJGunStatus_ChargeFinish = 3,		/* 未拔枪状态 */
	eIotXJGunStatus_AppointMent = 4,		/* 已预约状态 */
	eIotXJGunStatus_SelfCheck = 5,			/* 自检状态 */
	eIotXJGunStatus_Stopping = 7,			/* 停止中状态 */
	eIotXJGunStatus_Discharging = 8,		/* 放电中状态 */
}IotXJGunStatus_Enum;

typedef enum
{
	eXJErrCode_Succ = 0,					/* 无故障 */
	eXJErrCode_CarStop = 0x1000,		    /* 车端停止 */
	eXJErrCode_AppStop = 0x1001,		    /* 远程停止   */
	eXJErrCode_StopByTime = 0x1005,		    /* 按时间停止 */
	eXJErrCode_StopByEnergy = 0x1006,       /* 按电量停止 */
	eXJErrCode_CardStop = 0x1008,           /* 刷卡停止  */
	eXJErrCode_GunDisconnect = 0x100D,      /* 枪未连接 */
	eXJErrCode_DevForbid = 0x2002,          /* 桩被禁用 */
	eXJErrCode_StopOrderNoErr = 0x200E,     /* 停止充电订单号错误 */
	eXJErrCode_Updating = 0x201E,           /* 系统升级中 */
	eXJErrCode_ParaErr = 0x2024,            /* 参数错误 */
	eXJErrCode_OnCharging = 0x2025,         /* 启动充电，枪正在充电中 */
	eXJErrCode_CardReaderErr = 0x3007,      /* 读卡器异常 */
	eXJErrCode_MeterCommErr = 0x300D,       /* 电表通讯异常 */
	eXJErrCode_PileOverTemp = 0x3016,       /* 充电桩过温故障 */
	eXJErrCode_GunOverTemp = 0x3017,        /* 充电枪过温故障 */
	eXJErrCode_JcqMaloperation = 0x302A,    /* 继电器误动拒动 */
	eXJErrCode_JcqSynechiaFault = 0x3040,   /* 继电器粘连 */
	eXJErrCode_InputLineReversed = 0x501A,  /* 火零反接 */
	eXJErrCode_PowerOff = 0x4000,           /* 系统掉电 */
	eXJErrCode_ShortCircleErr = 0x4004,     /* 充电前输出短路故障 */
	eXJErrCode_LeakageCurrErr = 0x400A,     /* 漏电保护 */
	eXJErrCode_PEBreakFault = 0x400B,       /* 接地错误 */
    eXJErrCode_AphaseInputOverVol = 0x4011, /* 交流A相输入过压 */ 
    eXJErrCode_AphaseInputLessVol = 0x4012, /* 交流A相输入欠压 */
	eXJErrCode_MeterCalcErr = 0x600C,       /* 电能计量故障 */
	eXJErrCode_OutputOverCurr = 0x9006,     /* 交流输出过流 */
	eXJErrCode_CPVolErr = 0x900F,           /* CP电压错误 */
	eXJErrCode_S2ActTimeout = 0x9029,       /* S2动作超时 */
	eXJErrCode_S2BreakOff = 0x902A,         /* S2断开超时 */
	eXJErrCode_RateModeErr = 0x902D,        /* 无计费模型错误 */
	eXJErrCode_OtherErr = 0xFF00,           /* 其它错误 */
}IotXJErrCode_Enum;

typedef enum
{
	eIotXJEventType_Null = 0,				/* 无事件 */
	eIotXJEventType_GunPlugIn = 1,		    /* 充电枪插枪事件 */
	eIotXJEventType_GunPlugOut = 2,		    /* 充电枪拔枪事件 */
	eIotXJEventType_ChargeStart = 3,		/* 充电开始事件 */
	eIotXJEventType_ChargeStop = 4,		    /* 充电结束事件 */
}IotXJEventType_Enum;



/******************************************************************************
*    Typedef Definition
******************************************************************************/


typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    uint16_t (*pSendFunc)(uint8_t port, void *pBuf);
	uint16_t matchCmd;
	uint8_t printFlag;
    char *cMeaning;
}IotXJSendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	char *matchStr;
	uint8_t cmdType; 
    uint8_t (*pRecvParse)(uint8_t *port, uint8_t *r_data, uint16_t len);
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
	uint8_t printFlag;
    char *cMeaning;
}IotXJRecvCtrl_Struct;


typedef struct 
{
    uint8_t head[2];
    uint8_t dataLen[2];
    uint8_t version[4];
    uint8_t seq[4];
	uint8_t cmd[2];
}IotXJFrameHead_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* ASW_IOT_PROTO_XJ_TYPES_H_ */





















