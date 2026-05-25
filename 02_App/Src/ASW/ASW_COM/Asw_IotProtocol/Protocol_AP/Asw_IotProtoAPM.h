/******************************************************************************
* File Name          : Asw_IotProtoAPM.h
* Description        : 安培协议主模块头文件
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
#ifndef ASW_IOT_PROTO_AP_M_H_
#define ASW_IOT_PROTO_AP_M_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoAPTypes.h"
#include "SysCfg.h"
#include "Cdd_NetM.h"
#include "Ms_Nvm.h"
#include "Asw_Monitor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eIotAPWorkState_Init,
    eIotAPWorkState_Offline,
    eIotAPWorkState_Login,
    eIotAPWorkState_Normal,
}IotAPWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/

/* 协议交互数据结构 */
typedef struct
{
    uint8_t remoteStartResult;              /* 启动结果 */
    uint8_t remoteStartFailReason;          /* 启动失败原因 */
    uint8_t newRecvOrderTransactionNum[16]; /* 新接收的订单交易流水号 */
    uint8_t curUsedOrderTransactionNum[16]; /* 正在使用的订单交易流水号 */

    uint8_t remoteStopResult;               /* 停止结果 */
    uint8_t remoteStopFailReason;           /* 停止失败原因 */

}IotAPProtoData_Struct;

/* 协议上下文结构 */
typedef struct
{
    IotAPWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[7];
    IotAPProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;

    /* 离线后需清除数据 */
    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;

    uint8_t sendIndex;
    uint8_t sendPort;
    uint16_t reqSeq;

    uint32_t realDataReportTick[SYSCFG_CFG_GUN_NUM];
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_AP_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_AP_CMD_RECV_COUNT];
}IotAPCtx_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern IotAPCtx_Struct *pIotAPCtx;

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotAP_InitMemory(void);
void IotAP_MainFunction(void);
void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotAP_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotAP_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
uint8_t IotAP_SwipCardCharge(uint8_t port);

#endif /* ASW_IOT_PROTO_AP_M_H_ */
