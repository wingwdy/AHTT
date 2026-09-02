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
*2026/05/21     V1.0.0       WDY        初版创建
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

/* B47三缓冲计费模型索引(支持节前/节日/节后3套费率) */
#define IOTAP_B47_A    (0)  /* B47计费模型 A组(第0套, 节前费率) */
#define IOTAP_B47_B    (1)  /* B47计费模型 B组(第1套, 节日费率) */
#define IOTAP_B47_C    (2)  /* B47计费模型 C组(第2套, 节后费率) */

/* B47无效索引标记(表示无活跃计费模型) */
#define IOTAP_B47_INDEX_INVALID  (0xFF)

/* 判断索引是否为有效的B47活跃计费模型索引(A/B/C) */
#define IOTAP_IS_VALID_BILL_INDEX(idx)  (((idx) == IOTAP_B47_A) || ((idx) == IOTAP_B47_B) || ((idx) == IOTAP_B47_C))


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
    uint8_t remoteStartResult;               /* 启动结果 */
    uint8_t remoteStartFailReason;           /* 启动失败原因 */
    uint8_t newRecvOrderTransactionNum[16];  /* 新接收的订单交易流水号 */
    uint8_t curUsedOrderTransactionNum[16];  /* 正在使用的订单交易流水号 */

    uint8_t remoteStopResult;                /* 停止结果 */
    uint8_t remoteStopFailReason;            /* 停止失败原因 */
    uint8_t remoteCtrlCmd;                   /* B5回复的控制命令 */

    uint8_t timeBillModelId[8];              /* B47计费模型ID，用于B48应答 */
    uint8_t timeBillResult;                  /* B48应答结果: 0成功, 1失败 */
    uint8_t timeBillSwitchModelId[8];        /* B49切换计费模型ID */
    uint8_t timeBillSwitchTime[7];           /* B49切换时间 */
    uint8_t timeBillSwitchIndex;             /* B49切换计费模型索引 */
    uint8_t timeBillSwitchResult;            /* B49切换结果: 0成功, 1失败 */
    uint8_t timeBillPollTime[7];             /* B51轮询 CP56Time2a 时间戳 */
    uint8_t onlineDetailResult;              /* B54在线详情结果: 0成功 */

    uint8_t cardAuthResult;                  /* B7刷卡鉴权结果: 1成功, 0失败 */
    uint8_t cardAuthFailReason[2];           /* B7刷卡鉴权失败原因 */
    uint32_t cardAccountBalance;             /* B7账户余额, 精确到0.01元 */
    uint8_t cardVin[32];                     /* B7下发VIN */
    uint8_t startNotifyResult;               /* B11启动通知结果: 1成功, 0失败 */
    uint8_t startNotifyFailReason[2];        /* B11启动通知失败原因 */

    /* B33/B34 功率控制 */
    uint8_t powerCtrlTimepower[7];           /* B33下发的CP56时间戳 */
    uint8_t powerCtrlKind;                   /* 功率控制类型: 1-默认功率, 2-动态功率，3-控制功率 */
    uint32_t powerCtrlValue;                 /* 功率值 0.01kW */
    uint8_t powerCtrlPauseCmd;               /* 暂停控制: 0-普通功率指令, 1-暂停, 2-恢复 */
    uint16_t powerCtrlReportCycle;           /* 上报周期 分钟 */
    uint8_t powerCtrlResult;                 /* B34应答结果: 0成功 1失败 */
    uint32_t powerCtrlDefaultValue;          /* 默认功率值 0.01kW */
    uint32_t powerCtrlDynamicValue;          /* 动态功率值 0.01kW */
    uint32_t powerCtrlControlValue;          /* 控制功率值 0.01kW */
    uint32_t powerCtrlActiveValue;           /* 当前生效功率值 0.01kW */
    uint8_t powerCtrlActiveKind;             /* 当前生效功率类型 */
    uint32_t powerCtrlStatusTick;            /* B57上次上报的分钟秒数(0-59), 用于防同秒重复触发 */
    uint32_t powerCtrlDynamicTick;           /* 动态功率接收时间 */
    uint8_t powerCtrlDynamicValid;           /* 动态功率15s有效标志 */
    uint8_t powerCtrlControlActive;          /* 控制功率有效标志 */
    uint8_t powerCtrlPauseActive;            /* 当前订单平台暂停标志 */
    uint8_t powerCtrlB57Enable;              /* 控制功率生效时B57上报开关 */
    uint8_t powerCtrlLastOrderBusy;          /* 上一次订单忙状态 */

    /* B23/B24/B31/B32/B39/B40 终端信息、FTP与升级 */
    uint8_t terminalReqKind[2];              /* B32请求类型 */
    CddNetMSocketPara_Union ftpPara;         /* B39下发的FTP参数 */
    uint8_t ftpAddrResult;                   /* B40应答结果: 0成功 1失败 */
    CddNetMSocketPara_Union upgradePara;     /* B23远程升级FTP参数 */

}IotAPProtoData_Struct;

/* 协议上下文结构 */
typedef struct
{
    IotAPWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[IOT_AP_PILE_DN_LEN];
    uint8_t syncTimeCp56[7];
    IotAPProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];

    /* 远程升级(B23/B24)全局状态: 升级为整桩固件级操作, 不区分枪号 */
    uint8_t upgradeResult;                   /* B24结果: 0成功 1失败 */
    uint8_t upgradeOngoing;                  /* 已触发UCM升级流程 */

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
    IotAPBillModeSave_Struct stBillModeSave[SYSCFG_CFG_GUN_NUM];
    uint8_t billActiveIndex[SYSCFG_CFG_GUN_NUM];
    uint32_t nextSwitchSec[SYSCFG_CFG_GUN_NUM];

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
uint8_t IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotAP_InitMemory(void);
void IotAP_MainFunction(void);
void IotAP_UpCtrlSendDeal(void);
void IotAP_UpCtrlRecvDeal(void);
void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotAP_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotAP_StopReasonToBcd(uint8_t *pData, IotAPStopReason_Enum stopReason);
void IotAP_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
uint8_t IotAP_SwipCardCharge(uint8_t port, uint8_t *pCardID);
void IotAP_OfflineHandle(void);
void IotAP_PowerCtrlSetDefaultValue(uint8_t port, uint32_t powerValue);
void IotAP_PowerCtrlApply(uint8_t port);
void IotAP_PowerCtrlClearChargeValue(uint8_t port);
/* 内部适用 */
uint8_t IotAP_GetGunState(uint8_t port);
static void IotAP_CycleReportRealData(void);
const MSNvmAPParamBillMode_Struct *IotAP_GetActiveBillMode(uint8_t port);

/* ====== B47三缓冲计费模型管理函数(节前/节日/节后) ====== */
uint16_t IotAP_SearchBillModeID(uint8_t port, const uint8_t *pSearchID);
uint8_t IotAP_CompareContentBillMode(const MSNvmAPParamBillMode_Struct *pA,
                                     const MSNvmAPParamBillMode_Struct *pB);
uint8_t IotAP_IsFeeModelValid(const MSNvmAPParamBillMode_Struct *pBillMode);
void IotAP_SaveRateB47Model(const MSNvmAPParamBillMode_Struct *pNewMode, uint8_t port);
void IotAP_RefreshNowbillModel(uint8_t port);
void IotAP_ReadRateB47ModelFromNVM(void);
#endif /* ASW_IOT_PROTO_AP_M_H_ */
