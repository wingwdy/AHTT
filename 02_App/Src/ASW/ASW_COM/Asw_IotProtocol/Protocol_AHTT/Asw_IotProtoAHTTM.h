/******************************************************************************
* File Name          : Asw_IotProtoAHTTM.h
* Description        : AHTT protocol management definitions
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/08/31      V1.0.0      wdy        初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_AHTTM_H_
#define ASW_IOT_PROTO_AHTTM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoAHTTTypes.h"
#include "SysCfg.h"
#include "Cdd_NetM.h"
#include "MS_Nvm.h"
#include "Asw_Monitor.h"

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eIOTAHTTWorkState_Init,       /* 初始化状态 */
    eIOTAHTTWorkState_Offline,    /* 离线状态 */
    eIOTAHTTWorkState_Login,      /* 登录状态 */
    eIOTAHTTWorkState_Normal,     /* 正常通信状态 */
}IotAHTTWorkState_Enum;

typedef enum
{
    eIotAHTTDomainSwitchState_Idle,           /* 无域名切换事务 */
    eIotAHTTDomainSwitchState_Trying,         /* 候选地址试连中 */
    eIotAHTTDomainSwitchState_Rollback,       /* 旧地址回滚签到中 */
    eIotAHTTDomainSwitchState_FailRspPending, /* 失败应答待发送 */
}IotAHTTDomainSwitchState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    IotAHTTWorkState_Enum eWorkState;       /* 当前协议工作状态 */
    typeFuncSendCtrl pFuncSendCtrl;         /* 获取发送控制块的回调函数 */
    typeFuncRecvCtrl pFuncRecvCtrl;         /* 获取接收控制块的回调函数 */
    uint8_t frameQueueChannelID;            /* AHTT帧队列通道编号 */
    uint8_t deviceNum[5];                   /* 5字节BCD逆序设备编号 */
    uint8_t loginSucc;                      /* 平台登录成功标志 */
    uint8_t queueBusyFlag;                  /* 发送队列忙标志 */
    uint32_t waitQueueIdleTick;             /* 等待发送队列空闲的起始时刻 */
    uint8_t sendIndex;                      /* 当前发送命令表索引 */
    uint8_t sendPort;                       /* 当前发送端口索引 */
    uint16_t reqSeq;                        /* 设备主动请求流水号 */
    uint8_t setHeartCycleResult;            /* 设置心跳周期应答结果 */
    uint8_t setMaxChargeTimeResult;         /* 设置最大充电时长应答结果 */
    uint8_t setDevParamResult;              /* 设置设备运维参数应答结果 */
    uint8_t privateParamPersistPending;     /* AHTT私有参数待持久化标志 */
    IotAHTTDomainSwitchState_Enum eDomainSwitchState; /* 域名切换事务状态 */
    char oldDomain[MSNVM_PLAT_IP_LEN];       /* 切换前运营平台域名 */
    uint16_t oldDomainPort;                  /* 切换前运营平台端口 */
    char candidateDomain[IOT_AHTT_DOMAIN_FIELD_LEN + 1]; /* 候选运营平台域名 */
    uint16_t candidateDomainPort;            /* 候选运营平台端口 */
    uint16_t domainSwitchReqSeq;             /* 原0x04下行流水号 */
    uint32_t domainSwitchTryTick;            /* 候选TCP试连起始时刻 */
    uint8_t domainSwitchResult;              /* 0x04失败应答结果 */
    uint8_t domainSwitchBusyRspPending;      /* 事务忙失败应答待发送标志 */
    uint16_t domainSwitchBusyReqSeq;         /* 事务忙请求流水号 */
    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_AHTT_CMD_SEND_COUNT]; /* 各端口发送运行控制块 */
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_AHTT_CMD_RECV_COUNT]; /* 各端口接收运行控制块 */
}IotAHTTCtx_Struct;

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t IotAHTT_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotAHTT_InitMemory(void);
void IotAHTT_MainFunction(void);
void IotAHTT_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode);
void IotAHTT_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
uint8_t IotAHTT_SwipCardCharge(uint8_t port, uint8_t *pCardID);
void IotAHTT_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord,
    uint8_t *pProtocolRecord, uint16_t *pRecordLen);
void IotAHTT_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam);
void IotAHTT_OfflineHandle(void);
void IotAHTT_LoginSuccess(void);
uint8_t IotAHTT_GetGunState(uint8_t port);
uint8_t IotAHTT_CommitPrivateParam(const MSNvmAHTTParam_Struct *pNewParam);
uint8_t IotAHTT_BeginDomainSwitch(const char *pDomain, uint16_t port, uint16_t reqSeq);
void IotAHTT_ScheduleDomainSwitchFailRsp(uint16_t reqSeq);
void IotAHTT_QueueDomainSwitchBusyRsp(uint16_t reqSeq);
void IotAHTT_NotifyDomainSwitchRspQueued(void);

#endif /* ASW_IOT_PROTO_AHTTM_H_ */
