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
#ifndef ASW_IOT_PROTO_OM_TYPES_H_
#define ASW_IOT_PROTO_OM_TYPES_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 通信协议版本 */
#define IOT_OM_PROTOCOL_VERSION                 (10008U)

/* 通信协议头定义--GN协议 */
#define IOT_OM_HEAD1                            (0xEBU)                
#define IOT_OM_HEAD2                            (0xBEU)

/* 通信buff缓存定义 */
#define IOT_OM_TXRX_BUFFER_SIZE                 (2048U)

/* 日志接口函数定义 */
#define IOTOM_CFG_LogPrint(fmt, ...)            DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

/* 实时数据上报周期定义 */
#define IOTOM_CFG_IDLE_REALDATA_CYCLE           (10 * 60 * 1000)
#define IOTOM_CFG_CHARGING_REALDATA_CYCLE       (60 * 1000)

#define IOTOM_CFG_IDLE_METERVAL_CYCLE           (10 * 60 * 1000)
#define IOTOM_CFG_CHARGING_METERVAL_CYCLE       (60 * 1000)

/* 操作的参数 */
#define IOTOM_CFG_OPT_PARAM_COUNT               (12U)

/* 协议CMD 定义 */
#define IOT_OM_CMDTYPE_REQUSET			        (0x00U)
#define IOT_OM_CMDTYPE_RESPONSE                 (0x01U)
#define IOT_OM_CMD_NULL                         (0x00U)             /* 无效 *OM */

/* 协议CMD 发送定义 */
#define IOT_OM_CMD_LOGIN_REQ                    (0x01U)             /* 登陆 */
#define IOT_OM_CMD_HEARTBEAT_REQ                (0x03U)             /* 心跳请求 */
#define IOT_OM_CMD_SEND_NETMODULE_INFO          (0x05U)             /* 主动上报网络模块信息 */
#define IOT_OM_CMD_CALL_NETMODULE_INFO_RSP      (0xF05U)            /* 网络模块信息应答 */
#define IOT_OM_CMD_REPORT_REALDATA              (0x13U)             /* 上报实时数据 */
#define IOT_OM_CMD_CALL_REALDATA_ACK            (0xF13U)            /* 召测实时数据应答 */
#define IOT_OM_CMD_REPORT_METERVAL              (0x15U)             /* 上报电表底数 */
#define IOT_OM_CMD_ORDER_RECORD                 (0x17U)             /* 订单记录 */
#define IOT_OM_CMD_REMOTE_QUERY_SET_PARAM_RSP   (0x51U)             /* 远程设置查询记录应答 */
#define IOT_OM_CMD_SET_QRCODE_RSP               (0x59U)             /* 设置二维码应答 */
#define IOT_OM_CMD_REBOOT_RSP                   (0x91U)             /* 设置远程重启应答 */
#define IOT_OM_CMD_UPDATE_RSP                   (0x93U)             /* 远程更新应答 */
#define IOT_OM_CMD_SET_FORBID_RSP               (0x95U)             /* 设置禁用应答状态 */
#define IOT_OM_CMD_REPORT_FORBID_STATE          (0x97U)             /* 上报禁用状态 */
#define IOT_OM_CMD_SEND_COUNT                   (14U)

/* 协议CMD 接收定义 */
#define IOT_OM_CMD_LOGIN_RSP                    (0x02U)             /* 登陆应答 */
#define IOT_OM_CMD_HEARTBEAT_RSP                (0x04U)             /* 心跳应答 */
#define IOT_OM_CMD_CALL_NETMODULE_INFO          (0x06U)             /* 请求网络模块信息 */
#define IOT_OM_CMD_CALL_REALDATA                (0x12U)             /* 召测实时数据 */
#define IOT_OM_CMD_ORDER_RECORD_RSP             (0x16U)             /* 订单上报应答 */
#define IOT_OM_CMD_REMOTE_QUERY_SET_PARAM       (0x52U)             /* 远程设置查询记录 */
#define IOT_OM_CMD_SET_QRCODE                   (0x5AU)             /* 设置二维码 */
#define IOT_OM_CMD_REBOOT                       (0x92U)             /* 设置远程重启 */
#define IOT_OM_CMD_UPDATE                       (0x94U)             /* 设置远程更新 */
#define IOT_OM_CMD_SET_FORBID                   (0x96U)             /* 设置禁用状态 */
#define IOT_OM_CMD_REPORT_FORBID_STATE_RSP      (0x98U)             /* 上报禁用状态应答 */
#define IOT_OM_CMD_RECV_COUNT                   (11U)
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotOM_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotOM_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    IotOM_pSendPackFuncType pSendFunc;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotOMSendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	uint8_t cmdType; 
	IotOM_pRecvParseFuncType pRecvParse;
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    uint8_t printFlag;
    char *cMeaning;
}IotOMRecvCtrl_Struct;


typedef struct 
{
    uint8_t head[2];
    uint8_t version[2];
    uint8_t seq[2];
    uint8_t encryptFlag;
    uint8_t cmd;
    uint8_t dataLen[2];
}IotOMFrameHead_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/




#endif

