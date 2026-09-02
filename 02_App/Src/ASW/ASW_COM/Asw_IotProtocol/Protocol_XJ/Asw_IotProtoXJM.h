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
#ifndef ASW_IOT_PROTO_XJM_H_
#define ASW_IOT_PROTO_XJM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoXJTypes.h"
#include "SysCfg.h"
#include "Cdd_NetM.h"
#include "Asw_Monitor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eIotXJWorkState_Init,
	eIotXJWorkState_Offline, 
	eIotXJWorkState_Login,
	eIotXJWorkState_Normal,
}IotXJWorkState_Enum;

typedef enum 
{
	eXJReportState_Null = 0,
	eXJReportState_ToReport = 1,
	eXJReportState_Reporting = 2,	
}IotXJReportState_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t eventType;
    uint16_t eventParam;
    uint8_t orderNo[32 + 1];
}IotXJEventType_Struct;

typedef struct
{
	uint16_t errIndex;
	uint8_t status;
	IotXJReportState_Enum eReportState;
}IotXJErrInfoReport_Struct;


typedef struct 
{
    /* 离线不能清除 */
    uint8_t rebootFlag;
    uint32_t rebootTick;
    uint8_t curUsedOrderNo[SYSCFG_CFG_GUN_NUM][32 + 1];
    uint8_t newRecvOrderNo[SYSCFG_CFG_GUN_NUM][32 + 1];
    uint8_t authCardNo[SYSCFG_CFG_GUN_NUM][16 + 1];
    uint8_t cmd1309SetResult;
    
    uint8_t cmd501SetResult;
    uint8_t cmd501SetSuccesCount;

    uint8_t cmd507SetResult;
    uint8_t cmd507SetSuccesCount;
    uint8_t cmd507SetFailCount;
    uint8_t cmd507SetParaAddr[26];

    uint8_t cmd511SetResult;
    
    uint8_t cmd514QueryParaCount;
    uint8_t cmd514QueryParaAddr[26];
    uint8_t cmd514QueryParaResult[26];

    uint8_t cmd20SetResult[SYSCFG_CFG_GUN_NUM];

    uint16_t cmd07SetReuslt[SYSCFG_CFG_GUN_NUM];
    uint16_t cmd11SetReuslt[SYSCFG_CFG_GUN_NUM];

    /* 离线需清除 */
    IotXJEventType_Struct stEventType[SYSCFG_CFG_GUN_NUM][IOT_XJ_EVENT_REPORT_QUEUE_SIZE];
    IotXJEventType_Struct stCurrentEventType[SYSCFG_CFG_GUN_NUM];
    IotXJErrInfoReport_Struct stErrInfoReport[SYSCFG_CFG_GUN_NUM][IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE];
    uint8_t lastConnectState[SYSCFG_CFG_GUN_NUM];
    uint8_t chargeStarFailFlag[SYSCFG_CFG_GUN_NUM]; /* 小桔要求，启动失败后，上报枪的状态为未拔枪 */

    IotXJGunStatus_Enum eGunStatus[SYSCFG_CFG_GUN_NUM];
}IotXJProtoData_Struct;

typedef struct 
{
    IotXJWorkState_Enum eWorkState;
    MSNvmPlatPrivateParam_Union param;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;

    char platDn[32 + 1];
    IotXJProtoData_Struct stProtoData;
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;

    uint32_t latestChargeTimestamp; /* 最近一次开始充电时间戳 */

    /* 结束充电的时候清0 */
    uint32_t chargeEnergy[SYSCFG_CFG_GUN_NUM]; /* 已充电量 0.001 kwh */
    uint8_t  forbidState;

    /* 离线后需清除数据 */
    uint32_t errVersion[SYSCFG_CFG_GUN_NUM];
    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;

    uint8_t sendIndex;
	uint8_t sendPort;
    uint32_t reqSeq;
    uint16_t heartBeatSeq;

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_XJ_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_XJ_CMD_RECV_COUNT];
}IotXJCtx_Struct;

typedef struct
{
	AswErrorType_Enum eErrorCode;
	uint16_t errIndex;
	uint8_t lastStatus[SYSCFG_CFG_GUN_NUM];
}IotXJErrDesc_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

void IotXJ_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
void IotXJ_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotXJ_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
uint8_t IotXJ_SwipCardCharge(uint8_t port, uint8_t *pCardID);
uint8_t IotXJ_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotXJ_InitMemory(void);
void IotXJ_MainFunction(void);
void IotXJ_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam);

/* 模块内部调用函数 */
void IotXJ_OfflineHandle(void);
void IotXJ_DateTimeToBcdTime(CommonDateTime_Struct *pdateTime, uint8_t *pBcdTime);
void IotXJ_BcdTimeToDateTime(uint8_t *pBcdTime, CommonDateTime_Struct *pdateTime);
void IotXJ_TransfromErrCode(AswErrorType_Enum eGnErrCode, IotXJErrCode_Enum *pXjErrCode);

IotXJEventType_Struct *IotXJ_GetFirstEventQueue(uint8_t port);
void IotXJ_DelEventQueue(uint8_t port);
void IotXJ_AddEventQueue(uint8_t port, uint8_t eventType, uint16_t eventParam, uint8_t *pOrderNo);
uint8_t IotXJ_CheckErrInfoReportStatusFree(void);
void IotXJ_AddErrInfoQueue(uint8_t port, uint16_t errIndex, uint8_t status);
void IotXJ_DelErrInfoQueue(uint8_t port);
void IotXJ_CheckErrStatus(void);
int8_t IotXJ_CalcChecksum(int8_t* datas,uint16_t len);
#endif /* ASW_IOT_PROTO_XJM_H_ */













