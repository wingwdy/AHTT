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
#ifndef ASW_IOT_PROTO_GNM_H_
#define ASW_IOT_PROTO_GNM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoGNTypes.h"
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
	eIOTGNWorkState_Init,
	eIOTGNWorkState_Offline,
	eIOTGNWorkState_Login,
	eIOTGNWorkState_Normal,
}IotGNWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/

typedef struct
{
    uint8_t cardID[8];      /* 逻辑卡号 */
    uint8_t clearResult;    /* 清除标记 0-失败 1-成功 */
    uint8_t failReason;     /* 失败原因 0x01-卡号格式错误 0x00-清除成功 */
}IotGNOfflineCardClearResult_Struct;
typedef struct
{
    uint8_t cardID[8];      /* 逻辑卡号 */
    uint8_t searchResult;   /* 查询结果 0-不存在 1-存在 */
}IotGNOfflineCardSearchResult_Struct;

typedef struct 
{
    uint8_t remoteStartResult;          /* 启动结果 */
    uint8_t remoteStartFailReason;      /* 启动失败原因 */
    uint8_t newRecvOrderTransactionNum[16]; /* 新接收的订单交易流水号 */
    uint8_t curUsedOrderTransactionNum[16]; /* 正在使用的订单交易流水号 */

    uint8_t remoteStopResult;           /* 停止结果 */
    uint8_t remoteStopFailReason;       /* 停止失败原因 */
    uint8_t authCardID[8];              /* 授权卡号 刷卡启动生效后，填充 */

    uint8_t updateAccountMoneyCardID[8];
    uint8_t updateAccountMoneyResult;

    uint8_t setQrCodeResult;
    uint8_t setUpdateResult;

    uint8_t offlineCardSaveResult;      /* 离线卡保存结果 0-失败 1-成功 */
    uint8_t offlineCardFailReason;      /* 离线卡失败原因 0x01-卡号格式错误 0x02-存储空间不足 */

    uint8_t offlineCardClearCount;                                    /* 待清除卡数量 */
    uint8_t offlineCardSearchCount;                                   /* 待查询卡数量 */
    IotGNOfflineCardClearResult_Struct offlineCardClearResults[MSNVM_GN_OFFLINE_CARD_MAX_COUNT];  /* 每张卡的清除结果 */
    IotGNOfflineCardSearchResult_Struct offlineCardSearchResults[MSNVM_GN_OFFLINE_CARD_MAX_COUNT];/* 每张卡的查询结果 */


    uint8_t setDevWorkParamResult;      /* 工作参数设置结果 0-成功 1-失败 */

    uint8_t queryDevWorkParamCount;     /* 查询参数个数 */
    uint8_t queryDevWorkParamIds[IOTDXL_CFG_WORK_PARAM_MAX_COUNT];    /* 查询参数编号列表, 最大5个 */
}IotGNProtoData_Struct;


typedef struct 
{
    IotGNWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[7];
    IotGNProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;

    uint16_t offlineCardTradeSeq;           /* 离线卡交易流水自增序号 */

    /* 离线后需清除数据 */
    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;
    
    uint8_t sendIndex;
	uint8_t sendPort;
    uint16_t reqSeq;

    uint32_t realDataReportTick[SYSCFG_CFG_GUN_NUM];
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];            /* 用于变位上送*/
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];     /* 用于变位上送*/
    uint32_t errVersion[SYSCFG_CFG_GUN_NUM];
    uint8_t lastErrInfo[SYSCFG_CFG_GUN_NUM][32];         /* 用于变位上送*/

    /* 工作参数变更延迟重连 */
    uint8_t  devWorkParamChangedFlag;       /* 域名端口变更标志 0-未变更 1-需重连 */
    uint32_t devWorkParamChangedTick;       /* 变更时刻的时间戳 用于延迟判断 */

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_RECV_COUNT];
}IotGNCtx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotGN_InitMemory(void);
void IotGN_MainFunction(void);
void IotGN_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotGN_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotGN_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
uint8_t IotGN_SwipCardCharge(uint8_t port, uint8_t *pCardID);
void IotGN_GenerateOfflineTransNum(uint8_t port);
void IotGN_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam);
/* 内部适用 */
uint8_t IotGN_GetGunState(uint8_t port);
void IotGN_OfflineHandle(void);
IotDXLStopReason_Enum IotDXL_ConverStopReason(AswErrorType_Enum errType);
#endif





















