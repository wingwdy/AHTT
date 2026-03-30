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
#ifndef ASW_IOT_PROTO_XDTM_H_
#define ASW_IOT_PROTO_XDTM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoXDTTypes.h"
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
	eIotXDTWorkState_Init,
	eIotXDTWorkState_Offline,
	eIotXDTWorkState_Login,
	eIotXDTWorkState_Normal,
}IotXDTWorkState_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    IotXDTRecvData_Struct stRecvData[SYSCFG_CFG_GUN_NUM];

    MSNvmOrderInfo_Struct stOrderInfo[SYSCFG_CFG_GUN_NUM];

    IotXDTGunStatus_Enum eGunStatus[SYSCFG_CFG_GUN_NUM];
    IotXDTPileStatus_Enum ePileStatus;

    uint32_t totalChargeEnergy;         /* 累计充电电量  单位：度，保留4位小数 */
    uint32_t totalChargeTimes;          /* 累计充电次数 */
    uint32_t totalChargeTimeSec;        /* 累计充电时间，单位：秒 */
    uint32_t powerOnTick;               /* 上电时刻 */
    uint32_t runTime;                   /* 运行时间，单位：秒 */

    uint8_t t1SetFlag;
	uint8_t t2SetFlag;

    uint8_t rebootFlag;
    uint32_t rebootTick;

    uint8_t otaStartFlag;

    char mainIp[MSNVM_PLAT_IP_LEN + 1];
    char mainPort[6];
}IotXDTProtoData_Struct;

typedef struct 
{
    IotXDTWorkState_Enum eWorkState;
    MSNvmPlatPrivateParam_Union param;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;

    char platDn[16 + 1];
    IotXDTProtoData_Struct stProtoData;
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;
    
    /* 离线后需清除数据 */
    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;
    
    uint8_t sendIndex;
	uint8_t sendPort;
    uint16_t reqSeq;

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_XDT_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_XDT_CMD_RECV_COUNT];
}IotXDTCtx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t IotXDT_SetProductSecret(char *pProductSecret, uint8_t len);
uint8_t IotXDT_SetDevOperator(char *pDevOperator, uint8_t len); 
uint8_t IotXDT_SetProductKey(char *pProductKey, uint8_t len);
uint8_t IotXDT_GetProductKey(char *pProductKey, uint8_t *pOutLen);
uint8_t IotXDT_GetProductSecret(char *pProductSecret, uint8_t *pOutLen);
uint8_t IotXDT_GetDevOperator(char *pDevOperator, uint8_t *pOutLen);


void IotXDT_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotXDT_InitMemory(void);
void IotXDT_MainFunction(void);

/* 模块内部调用函数 */
void IotXDT_OfflineHandle(void);
IotXDTPileStatus_Enum IotXDT_GetPileStatus(void);
IotXDTGunStatus_Enum IotXDT_GetGunStatus(uint8_t port);
uint8_t IotXDT_IsPileOnCharging(void);
#endif /* ASW_IOT_PROTO_XDTM_H_ */













