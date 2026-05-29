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
*2026/05/21     V1.0.0       WDY        初版创建 - 骨架代码
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

/* B47双缓冲计费模型A/B组索引 */
#define IOTAP_B47_A    (0U)  /* B47计费模型 A组(第0套) */
#define IOTAP_B47_B    (1U)  /* B47计费模型 B组(第1套) */

/* B47无效索引标记(表示无活跃计费模型) */
#define IOTAP_B47_INDEX_INVALID  (0xFFU)


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
    uint8_t remoteCtrlCmd;                  /* B5回复的控制命令 */

    uint8_t timeBillModelId[8];              /* B47 time bill model ID for B48 response */
    uint8_t timeBillResult;                  /* B48 response result: 0 success, 1 error */
    uint8_t onlineDetailResult;              /* B54 online detail result: 0 success */

    /* B33/B34 功率控制 */
    uint8_t powerCtrlTimepower[7];           /* B33下发的CP56时间戳 */
    uint8_t powerCtrlKind;                   /* 功率控制类型: 1-平台下发功率值, 2-恢复默认功率值 */
    uint32_t powerCtrlValue;                 /* 功率值 W */
    uint8_t powerCtrlDefaultFlag;            /* 是否默认值标志: 0-非默认, 1-默认值 */
    uint16_t powerCtrlReportCycle;           /* 上报周期 分钟 */
    uint8_t powerCtrlResult;                 /* B34应答结果: 0成功 1失败 */

}IotAPProtoData_Struct;

/* 协议上下文结构 */
typedef struct
{
    IotAPWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[8];
    uint8_t syncTimeCp56[7];
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

/* B47双缓冲计费模型全局变量 */
extern IotAPBillModeSave_Struct g_stIotAPBillModeSave[SYSCFG_CFG_GUN_NUM];  /* 每枪独立的A/B双缓冲存储区 */
extern uint8_t g_iotapBillActiveIndex[SYSCFG_CFG_GUN_NUM];                  /* 当前活跃的计费模型索引(0=A/1=B, 0xFF=无效) */
extern uint8_t g_iotapB49SwitchFlag[SYSCFG_CFG_GUN_NUM];                    /* B49切换上报判断标志(1=需判断是否到切换时间) */

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotAP_InitMemory(void);
void IotAP_MainFunction(void);
void IotAP_UpCtrlSendDeal(void);
void IotAP_UpCtrlRecvDeal(void);
void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotAP_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotAP_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
uint8_t IotAP_SwipCardCharge(uint8_t port);
void IotAP_OfflineHandle(void);
/* 内部适用 */
uint8_t IotAP_GetGunState(uint8_t port);
static void IotAP_CycleReportRealData(void);

/* ====== B47双缓冲计费模型管理函数 ====== */
uint16_t IotAP_SearchBillModeID(uint8_t port, const uint8_t *pSearchID);
uint8_t IotAP_CompareContentBillMode(const MSNvmAPParamBillMode_Struct *pA,
                                     const MSNvmAPParamBillMode_Struct *pB);
uint8_t IotAP_IsFeeModelValid(const MSNvmAPParamBillMode_Struct *pBillMode);
void IotAP_SaveRateB47Model(const MSNvmAPParamBillMode_Struct *pNewMode, uint8_t port);
void IotAP_RefreshNowbillModel(uint8_t port);
void IotAP_ReadRateB47ModelFromNVM(void);
#endif /* ASW_IOT_PROTO_AP_M_H_ */
