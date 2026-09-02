/******************************************************************************
* File Name          : Asw_IotProtoGWEM.h
* Description        : 国网e充电协议主模块
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/05/22     V1.0.0      hzb        初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_GWEM_H_
#define ASW_IOT_PROTO_GWEM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoGWETypes.h"
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
    eIOTGWEWorkState_Init,
    eIOTGWEWorkState_Register,      /* HTTP注册中(获取三元组) */
    eIOTGWEWorkState_Offline,
    eIOTGWEWorkState_Login,
    eIOTGWEWorkState_Normal,
} IotGWEWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    IotGWERecvData_Struct stRecvData[SYSCFG_CFG_GUN_NUM];

} IotGWEProtoData_Struct;

typedef struct
{
    IotGWEWorkState_Enum eWorkState;                    /* 工作状态 */
    MSNvmPlatPrivateParam_Union param;                  /* 从Nvm读取的平台参数 */
    typeFuncSendCtrl pFuncSendCtrl;                     /* 获取发送cmd控制块回调 */
    typeFuncRecvCtrl pFuncRecvCtrl;                     /* 获取接收cmd控制块回调 */
    uint8_t frameQueueChannelID;                        /* 收发环形队列ID */

    char deviceName[32 + 1];                            /* 设备名称 */
    char productKey[32 + 1];                            /* 产品Key */
    char deviceSecret[64 + 1];                          /* 设备密钥 */
    const char *regBaseUrl;                             /* 注册设备基础url */
    IotGWEProtoData_Struct stProtoData;                 /* 协议业务数据 */

    /* 离线后需清除数据 */
    uint8_t loginSucc;                                  /* 登陆成功标志 */
    uint8_t queueBusyFlag;                              /* TX队列满暂停标志 */
    uint32_t waitQueueIdleTick;                         /* 队列忙时间戳 */
    uint32_t lastFeeModelReqTick;                       /* 上次计费模型请求tick */
    uint32_t lastDevConfigReqTick;                      /* 上次设备配置请求tick */
    uint8_t  devConfigReqCnt;                           /* 设备配置请求次数 (最多3次) */
    uint8_t  devConfigReceived;                         /* 设备配置已接收标志 */
    uint8_t  feeModelReceived;                          /* 计费模型已接收标志 */
    uint8_t  feeModelReqCnt;                            /* 计费模型请求计数 (0<=4:10s重试, >4:2h) */
    uint32_t reportingRecordTime;                       /* 当前正在上报的订单记录时间戳, 用于平台确认后标记 */
    uint8_t  reportUseRuntime[SYSCFG_CFG_GUN_NUM];      /* TRUE-当前上报当前订单,FALSE-当前上报历史nvm历史订单 */
    char     reportingPreTradeNo[41];                   /* 当前在报订单的preTradeNo */
    uint32_t orderReportAwaitTick;                      /* 等待orderCheckSrv业务确认的时刻, 0=空闲 */
    uint8_t  tradeRecordUploadFlag;                     /* 有未上报交易记录待上报 */

    /* 对时用 */
    uint8_t  timeSyncFlag;                              /* 0=未对时, 1=已对时 */
    uint8_t  timeSyncReqCnt;                            /* 本次对时请求次数 */
    uint32_t timeSyncReqTick;                           /* 上次对时请求tick */

    /* 状态变化检测 */
    uint8_t  prevGunConnected[SYSCFG_CFG_GUN_NUM];      /* 上次枪连接状态 */
    uint8_t  prevCPState[SYSCFG_CFG_GUN_NUM];           /* 上次CP电压状态 */
    uint8_t  loginFirstReport;                          /* 首次上线标志, 用于上线主动上报 */

    /* 电表底值整点上报 (30s检查一次, 整点触发) */
    uint32_t acOutMeterTick;                            /* 上次检查tick */
    uint8_t  acOutMeterLastHour;                        /* 上次上报的小时, 防重复 */

    /* 电表底值多帧上传 (召测/日志查询逐条上报) */
    uint32_t meterRecordCursorTime;                     /* 当前帧记录的timestamp, 0=未开始 */
    uint8_t  meterRecordUploadActive;                   /* 上传进行中标志 */
    uint8_t  meterRecordUploadPort;                     /* 上传对应的port */
    uint8_t  meterRecordAskMode;                        /* 召测模式: 10=全部, 11=零点 */

    /* 故障告警多帧上传 (日志查询逐条上报) */
    uint32_t faultRecordCursorTime;                     /* 当前帧记录的时间戳, 0=未开始 */
    uint8_t  faultRecordUploadActive;                   /* 上传进行中标志 */
    uint8_t  faultRecordUploadPort;                     /* 上传对应的port */

    /* 交易记录多帧上传 (日志查询逐条上报) */
    uint32_t tradeRecordCursorTime;                     /* 当前帧记录时间戳(startTime), 0=未开始 */
    uint8_t  tradeRecordUploadActive;                   /* 上传进行中标志 */
    uint8_t  tradeRecordUploadPort;                     /* 上传对应的port */

    /* 运行日志分片上传(日志查询逐条上报, 每条1K, 最多100KB) */
    uint32_t runLogCursorIdx;                           /* 当前记录的TSDB序号 */
    uint16_t runLogByteOffset;                          /* 当前记录内字节偏移 */
    uint32_t runLogSentBytes;                           /* 已发送总字节数(<=100KB) */
    uint8_t  runLogUploadActive;                        /* 上传进行中标志 */

    /* 交易记录召测全部记录 (askType=12, orderTwUpdateEvt逐条) */
    uint8_t  tradeRecordAskAllActive;                   /* 全部记录召测上传进行中 */
    uint8_t  tradeRecordAskAllPort;                     /* 对应的port */
    uint32_t tradeRecordAskAllStartTs;                  /* 查询起始时间戳 */
    uint32_t tradeRecordAskAllStopTs;                   /* 查询终止时间戳 */
    uint32_t tradeRecordAskAllCursor;                   /* 当前迭代时间戳 */

    /* 故障变位检测 */
    uint32_t lastErrVersion[SYSCFG_CFG_GUN_NUM];

    /* 离线充电管理 */
    uint32_t offlineStartTick;                          /* 离线开始时刻, 0=在线 */

    /* OTA状态   */
    uint8_t  otaState;
    int8_t   lastOtaProgress;                            /* 上次上报的OTA进度, -128=待首次上报 */
    uint8_t  otaEndResult;                               /* OTA结束结果码(GetResult接口消费完就释放了) */

    /* HTTP注册状态机 */
    uint8_t  regStep;                                   /* 注册步骤: 0=检查, 1=获取注册码, 2=获取三元组, 3=创建MQTT link, 4=登陆 */
    uint8_t  regRetryCount;                             /* 当前步骤重试计数 */
    uint32_t regWaitTick;                               /* 等待tick */
    char     registerCode[128];                         /* 注册码(Step1返回) */

    /* 功能配置 */
    IotGWEFunConfig_Struct funConfig;

    /* 有序充电 */
    uint8_t  OrderlyChargeFlg[SYSCFG_CFG_GUN_NUM];      /* 有序充电启用标志 */
    uint16_t lastOrderlyChargeKw[SYSCFG_CFG_GUN_NUM];   /* 上次调节功率值(0.1kW), 用于变化检测 */
    IotGWEOrderlyChargeStrategy_Struct sOrderlyCharge[SYSCFG_CFG_GUN_NUM]; /* 有序充电策略缓存 */

#ifdef IOTGWE_CFG_ACWORK_FAST_REPORT
    uint8_t  acWorkFastFlag[SYSCFG_CFG_GUN_NUM];        /* 充电快速上报进行中标记 */
    uint32_t acWorkFastStartTick[SYSCFG_CFG_GUN_NUM];   /* 快速上报起始tick */
#endif

    uint8_t optSeq;                                     /* 操作序号 */

    /* 发送和接收 */
    uint8_t sendIndex;                                  /* 当前遍历发送控制表索引 */
    uint8_t sendPort;                                   /* 当前遍历枪口号 */
    uint32_t reqSeq;                                    /* 请求帧序列号(自增) */
    uint32_t curMsgId;                                  /* 当前报文JSON的id字段值 (REQUEST=reqSeq递增前值, RESPONSE=平台请求id) */

    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_GWE_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_GWE_CMD_RECV_COUNT];

} IotGWECtx_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotGWE_InitMemory(void);
void IotGWE_MainFunction(void);
uint8_t IotGWE_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotGWE_OfflineHandle(void);

void IotGWE_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pBillMode);
void IotGWE_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void IotGWE_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);
void IotGWE_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam);
void IotGWE_ComputePointsElect(AswMonitorChargeData_Struct *pChargeData, AswMonitorBillMode_Struct *pBillMode, uint8_t (*pointsElect)[2], uint8_t *pStartPoint, uint8_t *pCrossPoints);


#endif /* ASW_IOT_PROTO_GWEM_H_ */
