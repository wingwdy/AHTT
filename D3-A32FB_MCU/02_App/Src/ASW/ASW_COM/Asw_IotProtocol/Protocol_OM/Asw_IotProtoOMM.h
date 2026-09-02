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
#ifndef ASW_IOT_PROTO_OMM_H_
#define ASW_IOT_PROTO_OMM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_IotProtoOMTypes.h"
#include "Cdd_NetM.h"
#include "Ms_Nvm.h"
#include "SS_Ucm.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eIOTOMWorkState_Init,
	eIOTOMWorkState_Offline,
	eIOTOMWorkState_Login,
	eIOTOMWorkState_Normal,
}IotOMWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t setRebootResult;
    uint8_t setForbidStateResult;
    uint8_t setForbidStateFailReason;
    uint8_t setUnforbidStateResult;
    uint8_t setUnforbidStateFailReason;
    uint8_t recvUpdateFlag;
    uint8_t setUpdateResult;
    SSUcmResult_Enum lastUcmResult;
    uint8_t optParamAction;             /* 0x00: 查询参数, 0x01: 设置参数 */
    uint8_t optParamResult;             /* 0x00: 成功, 0x01: 失败 */
    uint16_t queryParamFlag;            /* 每个bit对应一个参数, 用于对应参数查询  */
    uint8_t readLocalFileResult;
}IotOMProtoData_Struct;

typedef struct 
{
    IotOMWorkState_Enum eWorkState;
    MSNvmPlatPrivateParam_Union param;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    uint8_t frameQueueChannelID;
    uint8_t platDn[32 + 1];
    uint8_t pileFixDnAsc[32 + 1];
    IotOMProtoData_Struct stProtoData[SYSCFG_CFG_GUN_NUM];
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time;
    uint8_t sendForbidStateFlag;
    uint8_t sendForbidStateCount;

    /* 离线后需清除数据 */

    uint8_t loginSucc;
    uint8_t queueBusyFlag;
    uint32_t waitQueueIdleTick;
    
    uint8_t sendIndex;
	uint8_t sendPort;
    uint16_t reqSeq;

    uint32_t reportForBidStateTick;
    uint32_t meterValReportTick[SYSCFG_CFG_GUN_NUM];
    uint32_t realDataReportTick[SYSCFG_CFG_GUN_NUM];
    uint8_t lastGunState[SYSCFG_CFG_GUN_NUM];            /* 用于变位上送*/
    uint8_t lastGunConnectState[SYSCFG_CFG_GUN_NUM];     /* 用于变位上送*/
    uint32_t errVersion[SYSCFG_CFG_GUN_NUM];
    uint8_t lastErrInfo[SYSCFG_CFG_GUN_NUM][32];         /* 用于变位上送*/

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_OM_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_OM_CMD_RECV_COUNT];
}IotOMCtx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotOM_InitMemory(void);
void IotOM_MainFunction(void);
uint8_t IotOM_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);

/* 内部适用 */
void IotOM_OfflineHandle(void);
uint8_t IotOM_GetGunState(uint8_t port);
#endif

