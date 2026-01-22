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
    uint8_t remoteStartResult;          /* 启动结果 */
    uint8_t remoteStartFailReason;      /* 启动失败原因 */
    uint8_t newRecvOrderTransactionNum[16]; /* 新接收的订单交易流水号 */
    uint8_t curUsedOrderTransactionNum[16]; /* 正在使用的订单交易流水号 */

    uint8_t remoteStopResult;           /* 停止结果 */
    uint8_t remoteStopFailReason;       /* 停止失败原因 */

    MSNvmOrderInfo_Struct stOrderInfo;

    uint8_t authCardID[8];              /* 授权卡号 刷卡启动生效后，填充 */

    uint8_t updateAccountMoneyCardID[8];
    uint8_t updateAccountMoneyResult;

    uint8_t setQrCodeResult;
}IotGNProtoData_Struct;


typedef struct 
{
    IotGNWorkState_Enum eWorkState;
    MSNvmPlatPrivateParam_Union param;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[7];
    IotGNProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];
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
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];            /* 用于变位上送*/
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];     /* 用于变位上送*/

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_RECV_COUNT];
}IotGNCtx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotGN_InitMemory(void);
void IotGN_MainFunction(void);
void IotGN_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotGN_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
/* 内部适用 */
uint8_t IotGN_GetGunState(uint8_t port);
void IotGN_OfflineHandle(void);
#endif





















