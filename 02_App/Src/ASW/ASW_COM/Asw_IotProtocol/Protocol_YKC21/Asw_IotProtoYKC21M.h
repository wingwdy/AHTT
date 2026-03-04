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
#ifndef ASW_IOT_PROTO_YKC21M_H_
#define ASW_IOT_PROTO_YKC21M_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoYKC21Types.h"
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
	eIOTYKC21WorkState_Init,
	eIOTYKC21WorkState_Offline,
	eIOTYKC21WorkState_Login,
	eIOTYKC21WorkState_Normal,
}IotYKC21WorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{  
 
    uint16_t defaultPower_max;          /* 当前默认最大功率 kw */
    uint32_t deaultMaxPowerStartTimess; /* 到达此时间后按配置最大功率执行时间戳 */
    uint32_t deaultMaxPowerEndTimess;   /* 到达此时间后解除最大功率执行时间戳 */

 
    uint8_t  priority;                  /* 指令响应优先级 */
    uint16_t power_running;             /* 运行中功率 kw */
    uint32_t limitendtimess;            /* 运行中功率结束时间戳 */
}IotYKC21PowerConfig_Struct;

typedef struct
{			 
	uint8_t  errorAppearType;				  
	uint16_t errorAppearId;
    uint32_t errorAppearTime;	

    uint8_t  errorDisppearType;				  
	uint16_t errorDisppearId;						 							 			 
	uint32_t  errorDisppearTime;					 
}IotYKC21Err_Struct;


typedef struct 
{
    uint8_t remoteStartResult;              /* 启动结果 */
    uint8_t remoteStartFailReason;          /* 启动失败原因 */
    uint8_t newRecvOrderTransactionNum[16]; /* 新接收的订单交易流水号 */
    uint8_t curUsedOrderTransactionNum[16]; /* 正在使用的订单交易流水号 */

    uint8_t remoteStopResult;               /* 停止结果 */
    uint8_t remoteStopFailReason;           /* 停止失败原因 */

    MSNvmOrderInfo_Struct stOrderInfo;

    uint8_t authCardID[8];                  /* 授权卡号 刷卡启动生效后，填充 */

    uint8_t updateAccountMoneyCardID[8];
    uint8_t updateAccountMoneyResult;

    uint8_t setQrCodeResult;
    uint8_t setUpdateResult;
    
    IotYKC21PowerConfig_Struct powerConfig;  /* 功率调节参数 */
    IotYKC21Err_Struct erroInfo;             /* 故障发生和消除 */

}IotYKC21ProtoData_Struct;


typedef struct 
{
    IotYKC21WorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t pileDnBCD[7];
    IotYKC21ProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;
   
    /* 离线后需清除数据 */
    
    /* RSA密钥刷新 */
    uint8_t rsaRefreshflg; 
    uint32_t rsaReponseDelaytick;

    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;
    
    uint8_t sendIndex;
	uint8_t sendPort;
    uint16_t reqSeq;

    uint32_t realDataReportTick[SYSCFG_CFG_GUN_NUM];
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];            /* 用于变位上送*/
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];     /* 用于变位上送*/

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC21_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC21_CMD_RECV_COUNT];
}IotYKC21Ctx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

void IotYKC21_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotYKC21_InitMemory(void);
void IotYKC21_MainFunction(void);
void IotYKC21_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotYKC21_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
uint8_t IotYKC21_SwipCardCharge(uint8_t port);
uint8_t IotYKC21_RfreshYKC21key(char *YKC21key, uint16_t YKC21key_len);
uint8_t IotYKC21_RfreshYKC21token(char *YKC21token,uint16_t YKC21token_len);
void IotYKC21_PrintfYKC21KeyAndToken(void);
void IotYkc21_powercontrol(uint8_t port,uint16_t power);
void IotYKC21_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);

/* 内部适用 */
uint8_t IotYKC21_GetGunState(uint8_t port);
void IotYKC21_OfflineHandle(void);
uint8_t IotYKC21_CompareRecordOrderNum(uint8_t *record, uint8_t *pCompara, uint16_t paraSize);








#endif













