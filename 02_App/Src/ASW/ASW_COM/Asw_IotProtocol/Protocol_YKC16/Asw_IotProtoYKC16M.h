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
#ifndef ASW_IOT_PROTO_YKC16M_H_
#define ASW_IOT_PROTO_YKC16M_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoYKC16Types.h"
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
	eIOTYKC16WorkState_Init,
	eIOTYKC16WorkState_Offline,
	eIOTYKC16WorkState_Login,
	eIOTYKC16WorkState_Normal,
}IotYKC16WorkState_Enum;

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
    uint8_t authCardID[8];              /* 授权卡号 刷卡启动生效后，填充 */

    uint8_t updateAccountMoneyCardID[8];    /* 更新账户余额卡号 */
    uint8_t updateAccountMoneyResult;       /* 更新账户余额结果 */

    uint8_t setQrCodeResult;                /* 设置二维码结果 */
    uint8_t setUpdateResult;                /* 升级结果 */ 
}IotYKC16ProtoData_Struct;


typedef struct 
{
    IotYKC16WorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;     /* 发送控制函数 */
    typeFuncRecvCtrl pFuncRecvCtrl;     /* 接收控制函数 */
    uint8_t frameQueueChannelID;        /* 帧队列通道ID */
    uint8_t pileDnBCD[7];
    IotYKC16ProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];   /* 协议数据 */
    MSNvmOrderInfo_Struct stOrderInfo;  /* 订单信息 */
    uint32_t time;

    /* 离线后需清除数据 */
    uint8_t loginSucc;                  /* 登录成功标志 */
    uint8_t queueBusyFlag;              /* 队列忙标志*/
    uint32_t waitQueueIdleTick;         /* 等待队列空闲空闲的Tick计数 */
    
    uint8_t sendIndex;                  /* 发送索引 */
	uint8_t sendPort;                   /* 发送端口号*/
    uint16_t reqSeq;                    /* 请求序列号 */

    uint32_t realDataReportTick[SYSCFG_CFG_GUN_NUM];     /* 实际数据上报Tick */
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];            /* 用于变位上送*/
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];     /* 用于变位上送*/
    uint32_t errVersion[SYSCFG_CFG_GUN_NUM];
    uint8_t lastErrInfo[SYSCFG_CFG_GUN_NUM][32];         /* 用于变位上送*/
    

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC16_CMD_SEND_COUNT]; /* 发送控制 */
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC16_CMD_RECV_COUNT]; /* 接收控制 */
}IotYKC16Ctx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotYKC16_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotYKC16_InitMemory(void);
void IotYKC16_MainFunction(void);
void IotYKC16_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotYKC16_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotYKC16_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
uint8_t IotYKC16_SwipCardCharge(uint8_t port);
/* 内部适用 */
uint8_t IotYKC16_GetGunState(uint8_t port);
void IotYKC16_OfflineHandle(void);
void IotYKC16_SetPowerControl(uint8_t port, uint8_t powerRate);
#endif





















