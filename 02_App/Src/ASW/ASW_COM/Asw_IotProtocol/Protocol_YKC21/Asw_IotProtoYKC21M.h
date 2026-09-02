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

    uint8_t authCardID[8];                  /* 授权卡号 刷卡启动生效后，填充 */

    uint8_t updateAccountMoneyCardID[8];
    uint8_t updateAccountMoneyResult;

    uint8_t setQrCodeResult;
    uint8_t setUpdateResult;
    uint8_t  setKeyResult;                   /* 设置密钥结果 */     
    
    IotYKC21Err_Struct erroInfo;             /* 故障发生和消除 */

    uint8_t setDefaultMaxPowerResult;        /* 设置默认最大功率结果 */
    uint8_t setPowerChangeResult;            /* 设置功率修改结果 */  
    uint8_t  powerLimitFlag;                 /* 充电中功率限制标志位 */
    uint32_t platLimitPower;                 /* 充电中限制功率 kw 保留3位小数 */
    uint32_t powerlimitEndTimeStamp;         /* 充电中限制功率结束时间戳 */
    uint32_t lastSetPower;                   /* 上一次设置的充电功率 */


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
    uint8_t rsaPubicKeyWaitIdleRefreshFlag;
    uint8_t rsaPublicKeyRefreshFlag;
    uint32_t rsaPubicKeyDelayRefreshTick;

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

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC21_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_YKC21_CMD_RECV_COUNT];
}IotYKC21Ctx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

uint8_t IotYKC21_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotYKC21_InitMemory(void);
void IotYKC21_MainFunction(void);
void IotYKC21_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotYKC21_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
uint8_t IotYKC21_SwipCardCharge(uint8_t port, uint8_t *pCardID);
void IotYKC21_PrintfYKC21KeyAndToken(void);
void IotYKC21_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
void IotYKC21_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam);

uint8_t IotYKC21_SetToken(char *pToken, uint8_t len);
uint8_t IotYKC21_GetToken(char *pToken, uint8_t *pOutLen);
uint8_t IotYKC21_SetRsaPublicKey(char *pCipherKey, uint8_t len);
uint8_t IotYKC21_GetRsaPublicKey(char *pKey, uint8_t *pOutLen);
/* 内部适用 */
uint8_t IotYKC21_GetGunState(uint8_t port);
void IotYKC21_OfflineHandle(void);
uint8_t IotYKC21_CompareRecordOrderNum(uint8_t *record, uint8_t *pCompara, uint16_t paraSize);
void IotYKC21_SetPowerControl(uint8_t port, uint32_t powerLimit);







#endif













