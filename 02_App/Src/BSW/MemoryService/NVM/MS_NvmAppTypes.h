/******************************************************************************
* File Name          : MS_NvmAppTypes.h
* Description        : Code for The core service layer for managing non-volatile data 
                       storage of the ECU
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
#ifndef MS_NVM_APP_TYPES_H_
#define MS_NVM_APP_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "SysCfg.h"
#include "DS_LogM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MSNVM_CFG_DebugPrint(fmt, ...)                 DSLOGM_Debug(DSLogMModule_Flash, fmt, ##__VA_ARGS__)
#define MSNVM_CFG_InfoPrint(fmt, ...)                  DSLOGM_Info(DSLogMModule_Flash, fmt, ##__VA_ARGS__)

/* 二维码长度 */
#define MSNVM_QRCODE_LEN                      256

/* 订单长度 */
#define MSNVM_ORDER_MAX_LEN                   1280

/* 故障信息长度 */
#define MSNVM_ERROR_INFO_MAX_LEN              288

/* 运行日志长度 */
#define MSNVM_RUNNING_LOG_MAX_LEN             3584

/* 桩编码字符串长度 */
#define MSNVM_PILE_DN_LEN                     40

/* 各平台私有参数长度 */
#define MSNVM_PLAT_PRIVATE_PARAM_LEN          (512 + 768)

/* 平台IP长度 */
#define MSNVM_PLAT_IP_LEN                     72

/************************* GN(公牛) ****************************************/
/* 计费模型 */
#define MSNVM_GN_BILLMODE_MULTRATE_COUNT      9
#define MSNVM_GN_BILLMODE_4RATE_COUNT         4
#define MSNVM_GN_BILLMIDE_PERIOD_COUNT        48

/* 服务器域名长度 */
#define MSNVM_GN_SERVER_DOMAIN_LEN            128

/* 离线卡 */
#define MSNVM_GN_OFFLINE_CARD_MAX_COUNT       10            /* 离线卡最大存储数量 */
#define MSNVM_GN_OFFLINE_CARD_ID_LEN          8             /* 离线卡卡号长度 */

/************************* YKC21（云快充2.1） ******************************/
/* 费率数、时段数 */
#define MSNVM_YKC21_BILLMIDE_MULTRATE_COUNT   48
#define MSNVM_YKC21_BILLMIDE_PERIOD_COUNT     48

/* RSA 密钥长度 token长度 */
#define MSNVM_YKC21_RSA_PUBLIC_KEY_LEN        128
#define MSNVM_YKC21_TOKEN_LEN                 14

/************************* XDT（朗新新电途） ******************************/
#define MSNVM_XDT_DEV_OPERATOR_LEN            16
#define MSNVM_XDT_PRODUCT_KEY_LEN             64
#define MSNVM_XDT_PRODUCT_SECRET_LEN          64
#define MSNVM_XDT_USER_NAME_LEN               64
#define MSNVM_XDT_PASSWORD_LEN                64
#define MSNVM_XDT_VERSION_LEN                 32

#define MSNVM_XDT_BILLMODE_PERIOD_COUNT       16

/************************* XJ（小桔） ******************************/
#define MSNVM_XJ_BILLMIDE_PERIOD_COUNT        48

/************************* YKC16(云快充1.6) ****************************************/
/* 计费模型 */
#define MSNVM_YKC16_BILLMODE_4RATE_COUNT      4
#define MSNVM_YKC16_BILLMIDE_PERIOD_COUNT     48
/************************* GWE（国网e充电） ******************************/
#define MSNVM_GWE_USER_NAME_LEN               64
#define MSNVM_GWE_PASSWORD_LEN                64
#define MSNVM_GWE_PRODUCT_KEY_LEN             32
#define MSNVM_GWE_DEVICE_NAME_LEN             32
#define MSNVM_GWE_DEVICE_SECRET_LEN           64

#define MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX     96
/************************* AP（安培） *************************************/
#define MSNVM_AP_BILLMODE_PERIOD_COUNT        12
#define MSNVM_AP_BILLMODE_MAX_NUM             3   /* 从2套扩展到3套(节前/节日/节后) */

/******************************************************************************
*    Enum Definition
******************************************************************************/



/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint8_t (*pNvmCmpFunc)(uint8_t *record, uint8_t *pCompara, uint16_t paraSize);

/* 二维码 */
typedef struct
{
    char qrcode[MSNVM_QRCODE_LEN];
}MSNvmDrcode_Struct;

/* 电能示值 */
typedef struct
{
    uint64_t meterEnergy;
}MSNvmMeterEnergy_Struct;

/* 锁机状态 */
typedef struct
{
    uint8_t forbidState;  /* 锁机状态 FALSE-表示不禁用, TRUE-表示禁用 */
    uint8_t forbidReason;
}MSNvmForbidState_Struct;

/* 校表参数 */
typedef struct
{
    uint32_t VoltageCaliK;
}MSNvmMeterCaliParam_Struct;

/* 模式参数 */
typedef struct
{
    uint8_t isFactoryMode;       /* 是否为厂内模式 */
    uint8_t isQBMode;            /* 是否为国标模式 0-表示国标模式 */
    uint8_t isAgingTestFinish;   /* 是否完成老化测试 */
    uint8_t isSynTime;           /* 是否同步时间 */
    uint32_t sysTimeStamp;       /* 同步时间戳 */
    uint8_t res[28];
}MSNvmModeParam_Struct;

/* 平台参数 */
typedef struct
{
    char platPileDn[MSNVM_PILE_DN_LEN];        /* 运营平台桩号 */
    char fixPileDn[MSNVM_PILE_DN_LEN];         /* 运维平台桩号 */

    char platMainIp[MSNVM_PLAT_IP_LEN];        /* 运营平台IP */
    uint16_t platMainPort;                     /* 运营平台端口 */

    char platAuxiliaryIp[MSNVM_PLAT_IP_LEN];   /* 运维平台IP */
    uint16_t platAuxiliaryPort;                /* 运维平台IP */
    uint8_t platAuxiliaryDisable;              /* 运维平台失能标记 0-表示使能*/

    uint8_t platMainType;                      /* 运营平台类型 gn, ykc...*/
    uint8_t platMainCardType;                  /* 运营卡类型 gn, ykc...*/

    uint8_t AuxiliaryPlatDisableFlag;          /* 运维平台失能标记 0-表示不禁用, 1-表示禁用 */
    uint8_t dedicatedNetSimFlag;               /* 是否为专网SIM卡 0-表示不是, 1-表示是 */
    uint8_t envFlag;                           /* 运行环境 0-正式环境, 1-测试环境 */
    uint8_t reverse[61];
}MSNvmPlatParam_Struct;

/*********************************************************************************************** */
/* 具体各平台订单数据定义 */
typedef struct 
{
    uint8_t billmodeType;                     /* 计费模型类型 四类或者多类 */
    uint8_t pileDnBCD[7];                     /* 设备编号 */
    uint8_t port;                             /* 枪号 */
    uint8_t orderTransactionNum[16];          /* 交易流水号 */
    uint32_t startTime;                       /* 充电开始时间 */
    uint32_t stopTime;                        /* 充电结束时间 */
    uint32_t startMeterVal;                   /* 充电开始电表值 ，小数点后4位*/
    uint32_t stopMeterVal;                    /* 充电结束电表值 ，小数点后4位*/
    uint32_t totalEnergy;                     /* 总电能 ，小数点后4位*/
    uint32_t totalLossEnergy;                 /* 总计损电能 ，小数点后4位*/
    uint32_t totalMoney;                      /* 总金额 ，小数点后4位*/
    uint8_t vin[17];                          /* 电动汽车唯一标识 */
    uint8_t dealFlag;                         /* 交易标识 */
    uint32_t dealDate;                        /* 交易日期 */
    uint8_t stopReason;                       /* 停止原因 */
    uint8_t logicCardNum[8];                  /* 逻辑卡号 */
    uint32_t billInfo[9][4];                  /* 9个费率对应的 单价(5位小数)、电量(4位小数)、计损电量(4位小数)、金额(4位小数) */
}MSNvmGNOrderInfo_Struct;

/*********************************** AHTT安徽铁塔 */
typedef struct
{
    uint8_t deviceNum[5];      /* 5字节BCD逆序设备编号 */
    uint8_t port;              /* 充电端口编号 */
    uint16_t orderNo;          /* 订单单号 */
    uint8_t cardNo[5];         /* 5字节卡号 */
    uint32_t startTime;        /* 充电开始时间 */
    uint32_t stopTime;         /* 充电结束时间 */
    uint32_t totalEnergy;      /* 订单累计电量原始值 */
    uint16_t remainMoney;      /* 卡内剩余金额原始值 */
    uint16_t elecFee;          /* 订单电费原始值 */
    uint16_t serviceFee;       /* 订单服务费原始值 */
    uint8_t stopReason;        /* 充电停止原因 */
}MSNvmAHTTOrderInfo_Struct;

/*********************************** YKC1.6 云快充1.6 */
typedef struct 
{
    uint8_t orderTransactionNum[16];          /* 交易流水号 BCD*/
    uint8_t pileDnBCD[7];                     /* 设备编号 BCD*/
    uint8_t port;                             /* 枪号 BCD*/
    uint8_t startTime[7];                     /* 充电开始时间 CP56Time2a 格式*/
    uint8_t stopTime[7];                      /* 充电结束时间 CP56Time2a 格式*/
    uint32_t billInfo[4][4];                  /* 4个费率对应的 单价(5位小数)、电量(4位小数)、计损电量(4位小数)、金额(4位小数) */
    uint8_t startMeterVal[5];                 /* 充电开始电表值 ，小数点后4位*/
    uint8_t stopMeterVal[5];                  /* 充电结束电表值 ，小数点后4位*/
    uint32_t totalEnergy;                     /* 总电能 ，小数点后4位*/
    uint32_t totalLossEnergy;                 /* 总计损电能 ，小数点后4位*/
    uint32_t totalMoney;                      /* 总金额 ，小数点后4位*/
    uint8_t vin[17];                          /* 电动汽车唯一标识 */
    uint8_t dealFlag;                         /* 交易标识 */
    uint8_t dealDate[7];                      /* 交易日期 CP56Time2a 格式*/
    uint8_t stopReason;                       /* 停止原因 */
    uint8_t physCardNum[8];                   /* 物理卡号 */
}MSNvmYKC16OrderInfo_Struct;

/*********************************** YKC2.1 云快充2.1 */
typedef struct 
{
    /* 根据协议2.1.1全部存储有 330 字节 */
    uint8_t orderTransactionNum[16];          /* 交易流水号 */
    uint8_t pileDnBCD[7];                     /* 设备编号 */
    uint8_t port;                             /* 枪号 */
    uint8_t startTime[7];                     /* 充电开始时间 CP56Time2a 格式 ）98 B7 0E 11 10 03 14（开始时间：2020-03-16 17:14:47） */
    uint8_t stopTime[7];                      /* 充电结束时间 CP56Time2a 格式 */
    uint8_t meterDn[6];                       /* 电表表号 默认全0 */
    uint8_t meterCipher[34];                  /* 电表密文 默认全0 */
    uint8_t meterVer[2];                      /* 电表协议版本号 默认全0 */
    uint8_t encryptionType;                   /* 加密方式 */
    uint8_t startMeterVal[5];                 /* 充电开始电表值 ，小数点后4位*/
    uint8_t stopMeterVal[5];                  /* 充电结束电表值 ，小数点后4位*/
    uint8_t totalEnergy[4];                   /* 总电能 ，小数点后4位*/
    uint8_t totalLossEnergy[4];               /* 总计损电能 ，小数点后4位*/
    uint8_t totalMoney[4];                    /* 总金额 ，小数点后4位 */
    uint8_t vin[17];          		          /* 电动汽车唯一标识vin码,正序上传，ASCII, 默认全0 暂时注释 */
    uint8_t dealFlag;           	          /* 交易标识 0x01-app ; 0x02-卡 ;0x04-离线卡 ; 0x05-vin码 */
    uint8_t dealDate[7];                      /* 交易日期 CP56Time2a 格式 */
    uint8_t stopReason;                       /* 停止原因 */
    uint8_t logicCardNum[8];                  /* 逻辑卡号 */
    uint8_t fee_num;                          /* 费率个数 */
    uint32_t time_power[48];                  /* 48时段电量 小数点后四位 */
}MSNvmYKC21OrderInfo_Struct;

/*********************************** AP 安培 */
typedef struct
{
    uint8_t timeSerialNumber;                  /* 时段序号 */
    uint8_t timeKind;                          /* 时段类型 */
    uint8_t chargeEnergy[3];                   /* 电量，0.001kWh，小端 */
    uint8_t chargeElecFee[3];                  /* 电费，0.01元，小端 */
    uint8_t chargeServeFee[3];                 /* 服务费，0.01元，小端 */
}MSNvmAPPeriodTradeInfo_Struct;

typedef struct
{
    uint8_t port;                             /* 枪号 */
    uint8_t orderTransactionNum[16];           /* 交易流水号 */
    uint8_t periodCount;                       /* 时段个数 */
    MSNvmAPPeriodTradeInfo_Struct periodInfo[MSNVM_AP_BILLMODE_PERIOD_COUNT];
    uint8_t startTime[7];                      /* 充电开始时间 CP56Time2a */
    uint8_t stopTime[7];                       /* 充电结束时间 CP56Time2a */
    uint8_t chargeTimeMin[2];                  /* 累计充电时间，min，小端 */
    uint8_t totalElecFee[3];                   /* 充电费，0.01元，小端 */
    uint8_t totalServeFee[3];                  /* 服务费，0.01元，小端 */
    uint8_t totalEnergy[3];                    /* 总电量，0.001kWh，小端 */
    uint8_t startMeterVal[4];                  /* 总起示值，0.001kWh，小端 */
    uint8_t stopMeterVal[4];                   /* 总止示值，0.001kWh，小端 */
    uint8_t startSoc[2];                       /* 充电前SOC */
    uint8_t stopSoc[2];                        /* 结束后SOC */
    uint8_t logicCardNum[8];                   /* 物理卡号 */
    uint8_t vin[32];                           /* 电动汽车唯一标识 */
    uint8_t stopReason[2];                     /* 停止原因，BCD */
}MSNvmAPOrderInfo_Struct;

/*********************************** XDT 新电途 */
typedef struct
{
	uint8_t valid;
	uint8_t sn;
	uint8_t pq[4];
}MSNvmXDTPeriodInfo_Struct;

typedef struct 
{
	uint8_t orderState;
	char    orderNo[32 + 1];
	uint8_t gunNo;
	uint8_t ts[4];
	uint8_t indexRec[4];
	uint8_t typeRec;
	uint8_t type;
	uint8_t initiator;
	char    user[17 + 1];
	uint8_t typePlan;
	uint8_t typeStart;
	uint8_t tsStart[4];
	uint8_t value[4];
	uint8_t pricingID[4]; 
	uint8_t beginTs[4];
	uint8_t endTs[4];
	uint8_t beginMr[4];
	uint8_t endMr[4];
	uint8_t tPq[4];
	uint8_t elecAmt[4];
	uint8_t serMt[4];
	uint8_t amt[4];
    uint8_t stopReason;
	uint8_t typeRule;
	uint8_t pqTotal[4];
	MSNvmXDTPeriodInfo_Struct periodInfoArray[MSNVM_XDT_BILLMODE_PERIOD_COUNT];
}MSNvmXDTOrderInfo_Struct;

/*********************************** XJ小桔 */
typedef struct 
{
	uint16_t charge_type;
	uint8_t pileDn[32];
	uint8_t gun_type;
	uint8_t port;
	uint8_t charge_user_id[32];
	uint32_t charge_start_time;
	uint32_t charge_end_time;
	uint32_t charge_time;
	uint8_t start_soc;
	uint8_t end_soc;
	uint16_t err_no;
	uint32_t charge_kwh_amount;
	uint64_t start_charge_kwh_meter; 
	uint64_t end_charge_kwh_meter;   
	uint32_t total_charge_fee;
	uint32_t is_not_stoped_by_card;
	uint32_t start_card_money;
	uint32_t end_card_money;
	uint32_t total_service_fee;
	uint8_t is_paid_by_offline;
	uint8_t charge_policy;
	uint32_t charge_policy_param;
	uint8_t car_vin[17];
	uint8_t car_plate_no[8];
	uint32_t kwh_amount[48];
	uint8_t start_charge_type;
	uint8_t card_id[16];
	uint64_t start_discharge_kwh_meter; 
	uint64_t end_discharge_kwh_meter;
}MSNvmXJOrderInfo_Struct;

/*********************************** GWE国网e充电 */
/* size = 1192byte */
typedef struct
{
    uint8_t gunNo;                  /* 枪号(1~255) */
    char preTradeNo[41];            /* 平台交易流水号 */
    char tradeNo[39];               /* 设备交易流水号 */
    uint8_t startType;              /* 启动方式 */
    uint32_t startTime;             /* 开始时间(时间戳) */
    uint32_t endTime;               /* 结束时间(时间戳) */
    uint8_t stopReason;             /* 停止原因 */
    uint8_t billModeID[17];         /* 计费模型编号 */
    uint32_t sumStart;              /* 电表总起示值(0.0001KWH) */
    uint32_t sumEnd;                /* 电表总止示值(0.0001KWH) */
    uint32_t totalElec;             /* 总电量 (0.0001KWH)*/
    uint32_t totalPowerCost;        /* 总电费(0.0001元) */
    uint32_t totalServCost;         /* 总服务费 (0.0001元)*/
    uint8_t timeNum;                /* 时段数(1~96) */
    uint8_t partElect[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX][3];   /* 时段电量(0.0001KWH) */
    uint8_t chargeFee[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX][3];   /* 时段电费(0.0001元) */
    uint8_t serviceFee[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX][3];  /* 时段服务费(0.0001元) */
    uint8_t startPoint;             /* 起始点标识 */
    uint8_t crossPoints;            /* 跨越点数M */
    uint8_t pointsElect[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX][2]; /* 跨越电量(0.0001KWH) */

}MSNvmGWEOrderInfo_Struct;

typedef union 
{
    MSNvmXDTOrderInfo_Struct stXDTOrderInfo;
    MSNvmGNOrderInfo_Struct stGNOrderInfo;
    MSNvmYKC16OrderInfo_Struct stYKC16OrderInfo;
    MSNvmYKC21OrderInfo_Struct stYKC21OrderInfo;
    MSNvmXJOrderInfo_Struct stXJOrderInfo;
    MSNvmGWEOrderInfo_Struct stGWEOrderInfo;
    MSNvmAPOrderInfo_Struct stAPOrderInfo;
    MSNvmAHTTOrderInfo_Struct stAHTTOrderInfo; /* AHTT平台订单数据 */
    uint8_t userData[MSNVM_ORDER_MAX_LEN];
}MSNvmPlatOrderInfo_Union;

typedef char MSNvmAHTTOrderUnionSizeCheck[
    (sizeof(MSNvmPlatOrderInfo_Union) == MSNVM_ORDER_MAX_LEN) ? 1 : -1];

/* 订单记录 */
typedef struct 
{
    uint8_t orderSaveState;                   /* 订单保存状态 */
    uint16_t orderLen;                        /* 订单数据长度 */
    uint8_t port;                             /* 枪号 */
    uint8_t protocolType;                     /* 协议类型 */
    MSNvmPlatOrderInfo_Union platOrderInfo;   /* 各平台订单类型数据 */
}MSNvmOrderInfo_Struct;

/* 故障记录 */
typedef struct 
{
    uint8_t userData[MSNVM_ERROR_INFO_MAX_LEN];
}MSNvmErrorInfo_Struct;

/* 运行日志记录 */
typedef struct
{
    uint8_t userData[MSNVM_RUNNING_LOG_MAX_LEN];
}MSNvmRunningLog_Struct;

/* 电表底值记录 */
typedef struct
{
    uint8_t  sync;                                  /* 是否上报同步标志位 */
    uint8_t  gunNo;                                 /* 枪号(1~SYSCFG_CFG_GUN_NUM) */
    uint32_t acqTime;                               /* 采集时间(Unix时间戳), 上报时转 yyyyMMddHHmmss */
    uint64_t sumMeter;                              /* 电表底值(0.0001kWh) */
    char     lastTrade[42];                         /* 最后交易流水号 */
    uint32_t elec;                                  /* 充电中订单已充电量(0.0001kWh) */
}MSNvmMeterRecord_Struct;


/*********************************************************************************************** */
/* 各平台私有参数定义 */
/*********************************** GN */
typedef struct 
{
    uint8_t billType;                                           /* 4类电价或者多类电价 */
    uint8_t billModeID[2];                                      /* 计费模型编号 */
    uint8_t elecLossRate;                                       /* 计量损耗费率 */         
    uint32_t elecPriceRate[MSNVM_GN_BILLMODE_MULTRATE_COUNT];   /* 电费费率，小数点后5位 */ 
    uint32_t servePriceRate[MSNVM_GN_BILLMODE_MULTRATE_COUNT];  /* 服务费费率，小数点后5位 */ 
    uint8_t period_rate[MSNVM_GN_BILLMIDE_PERIOD_COUNT];        /* 48个30分钟，每个30分钟对应的费率号 */
}MSNvmGNParamBillMode_Struct;

/* 离线卡信息 */
typedef struct
{
    uint8_t cardID[MSNVM_GN_OFFLINE_CARD_ID_LEN];                /* 卡号 */
}MSNvmGNOfflineCard_Struct;

/* 离线卡存储区 */
typedef struct
{
    uint16_t cardCount;                                                        /* 已存储离线卡数量 */
    MSNvmGNOfflineCard_Struct offlineCards[MSNVM_GN_OFFLINE_CARD_MAX_COUNT];   /* 离线卡列表 */
}MSNvmGNOfflineCardStore_Struct;

/* 充电设备工作参数 */
typedef struct
{
    uint8_t authConfig;                                 /* 授权配置: 0x00-授权模式, 0x01-即插即充 */
    char serverDomain[MSNVM_GN_SERVER_DOMAIN_LEN + 1];      /* 服务器域名, ASCII, 不足末尾补0 */
    uint16_t serverPort;                                /* 服务器端口号 */
    uint8_t idleReportCycle;                            /* DXL待机状态实时数据上报周期, 单位: min, 0表示使用默认值 */
    uint8_t chargingReportCycle;                        /* DXL充电状态实时数据上报周期, 单位: s, 0表示使用默认值 */
}MSNvmGNWorkParam_Struct;

typedef struct 
{
    MSNvmGNParamBillMode_Struct stBillMode;
    MSNvmGNOfflineCardStore_Struct stOfflineCardStore;       /* 离线卡存储区 */
    MSNvmGNWorkParam_Struct stWorkParam;                     /* 充电设备工作参数 */
}MSNvmGNParam_Struct;

/*********************************** AHTT安徽铁塔 */
typedef struct
{
    uint8_t heartCycleMin;         /* 心跳周期，单位：分钟 */
    uint8_t maxChargeTimeHour;     /* 最大充电时长，单位：小时 */
    uint8_t devOperationParam[8];  /* 0x84设备运维参数原始值 */
    uint8_t tempAlarmLimit;        /* 温度告警阈值原始值 */
    uint8_t paramVersion;          /* AHTT私有参数结构版本 */
}MSNvmAHTTWorkParam_Struct;

typedef struct
{
    MSNvmAHTTWorkParam_Struct stWorkParam; /* AHTT设备工作参数 */
}MSNvmAHTTParam_Struct;

/*********************************** YKC21 */
typedef struct 
{
    uint8_t  billModeID[2];                                        /* 计费模型编号 */
    uint8_t  billnum;                                              /* 费率数量 */  
    uint8_t  elecLossRate;                                         /* 计量损耗费率，目前平台不支持计损功能，计损比例置 0 */        
    uint32_t elecPriceRate[MSNVM_YKC21_BILLMIDE_MULTRATE_COUNT];   /* 电费费率，小数点后5位 */ 
    uint32_t servePriceRate[MSNVM_YKC21_BILLMIDE_MULTRATE_COUNT];  /* 服务费费率，小数点后5位 */ 
    uint8_t  period_rate[MSNVM_YKC21_BILLMIDE_PERIOD_COUNT];       /* 48个30分钟，每个30分钟对应的费率号 */
}MSNvmYKC21ParamBillMode_Struct;

typedef struct YKC21platinfo
{
    uint8_t   defaultMaxPowerLimitFlag[SYSCFG_CFG_GUN_NUM];        /* 默认最大功率限制标志位 */
    uint16_t  defaultMaxPower[SYSCFG_CFG_GUN_NUM];                 /* 默认最大功率 kW，保留3位小数*/
    uint32_t  deaultMaxPowerStartTimeStamp[SYSCFG_CFG_GUN_NUM];    /* 默认最大功率开始时间戳 */
    uint32_t  deaultMaxPowerEndTimeStamp[SYSCFG_CFG_GUN_NUM];      /* 默认最大功率结束时间戳 */
	uint8_t   rsa_Keylength;	                                   /* 云快充2.1 rsa公钥长度 */
    uint8_t   rsa_Key[MSNVM_YKC21_RSA_PUBLIC_KEY_LEN + 1];	       /* 云快充2.1 rsa公钥 */
    uint8_t   tokenLen;                                            /* 云快充2.1 token长度 */
    uint8_t   token[MSNVM_YKC21_TOKEN_LEN + 1];	                   /* 云快充2.1 token */
} MSNvmYKC21PlatInfo_Struct;

typedef struct 
{
    MSNvmYKC21ParamBillMode_Struct stBillMode;
    MSNvmYKC21PlatInfo_Struct platinfo;
}MSNvmYKC21Param_Struct;

/*********************************** XDT */
typedef struct
{
	uint8_t validFlag;
	uint8_t startTime;
	uint8_t stopTime;
	uint8_t flag;
}MSNvmXDTRatePeriodInfo_Struct;

typedef struct
{
    uint8_t billModeID[4];
    uint8_t sharp_ele_fee[4];
    uint8_t sharp_ser_fee[4];
    uint8_t peak_ele_fee[4];
    uint8_t peak_ser_fee[4];
	uint8_t flat_ele_fee[4];
    uint8_t flat_ser_fee[4];
	uint8_t valley_ele_fee[4];
    uint8_t valley_ser_fee[4];
    uint8_t deep_ele_fee[4];
	uint8_t deep_ser_fee[4];
    uint8_t measure_wastage_rates;
    uint8_t segmentation_rate[48];
    uint8_t period_count;
	MSNvmXDTRatePeriodInfo_Struct period[MSNVM_XDT_BILLMODE_PERIOD_COUNT];
    uint8_t typeRule;
    uint8_t std_ele_fee[4];
	uint8_t std_ser_fee[4];
	uint8_t validFlag[4];
}MSNvmXDTParamBillMode_Struct;

typedef struct 
{
    uint32_t resetCount;                                           /* 复位次数 */
	uint8_t  pileDataCycleReportEnable;                            /* 数据周期上报使能 */
	uint16_t pileDataReportCycle;                                  /* 数据上报周期，单位：秒 */
	uint32_t amountChangeThreshold;                                /* 金额变化阈值  */
	char     cOperator[MSNVM_XDT_DEV_OPERATOR_LEN + 1];            /* 设备运营商  */
	char     cProductKey[MSNVM_XDT_PRODUCT_KEY_LEN + 1];           /* 产品密钥  */
	char     cProductSecret[MSNVM_XDT_PRODUCT_SECRET_LEN + 1];     /* 产品密码  */
    uint8_t  credentialSaveFlag;                                   /* 凭据已获取标记 */
    uint8_t  credentialValidFlag;                                  /* 凭据有效标记（连接成功生效） */
    char     cUserName[MSNVM_XDT_USER_NAME_LEN + 1];               /* 用户名  */
    char     cPassword[MSNVM_XDT_PASSWORD_LEN + 1];                /* 密码  */

    char     lastOtaSoftwareVersion[MSNVM_XDT_VERSION_LEN + 1];    /* 上一次升级的OTA软件版本 */
    char     otaSoftwareVersion[MSNVM_XDT_VERSION_LEN + 1];        /* 当前正在升级的OTA软件版本 */
    uint8_t  otaState;                                             /* OTA状态 */

    uint8_t  orderCount;                                           /* 订单数量 */
}MSNvmXDTPlatInfo_Struct;

typedef struct 
{
    MSNvmXDTParamBillMode_Struct stBillMode;
    MSNvmXDTPlatInfo_Struct platinfo;
}MSNvmXDTParam_Struct;

/*********************************** XJ */
typedef struct
{
    uint16_t startPeriod;                                         /* 开始时段编号 */
    uint16_t continuesPeriodCount;                                /* 连续时段数 */
    uint16_t elecPrice;                                           /* 电费费率，小数点后2位 */ 
    uint16_t servePrice;                                          /* 服务费费率，小数点后2位 */ 
    uint16_t delayPrice;                                          /* 延时费费率，小数点后2位 */ 
}MSNvmXJRatePeriodInfo_Struct;

typedef struct 
{
    uint16_t periodCount;                                         /* 时段数 */
    MSNvmXJRatePeriodInfo_Struct periodDetail[MSNVM_XJ_BILLMIDE_PERIOD_COUNT];
}MSNvmXJParamBillMode_Struct;

typedef struct 
{
    uint16_t frame106Interval;                                     /* 106(签到)报文时间间隔，单位：分钟 */
    uint16_t frame104Interval;                                     /* 104（状态信息）报文时间间隔，单位：秒 */
    uint16_t frame102Interval;                                     /* 102（心跳）报文时间间隔，单位：秒 */
    uint8_t  frame102MaxTimeoutTimes;                              /* 102（心跳）报文最大超时次数 */
    uint32_t rebootCount;                                          /* 重启次数 */
}MSNvmXJPlatInfo_Struct;

typedef struct
{
    MSNvmXJParamBillMode_Struct stBillMode;
    MSNvmXJPlatInfo_Struct platinfo;
}MSNvmXJParam_Struct;
 
/*********************************** AP */
typedef struct
{
    uint8_t device_number[7];                                   /* 桩编码*/
    uint8_t billModeID[2];                                      /* 计费模型编号 */
    uint8_t sharp_ele_fee[4];        	
    uint8_t sharp_ser_fee[4];
    uint8_t peak_ele_fee[4];
    uint8_t peak_ser_fee[4];
	uint8_t flat_ele_fee[4];
    uint8_t flat_ser_fee[4];
	uint8_t valley_ele_fee[4];
    uint8_t valley_ser_fee[4];
    uint8_t elecLossRate;                                       /* 计量损耗费率 */         
    uint8_t period_rate[MSNVM_YKC16_BILLMIDE_PERIOD_COUNT];     /* 48个30分钟，每个30分钟对应的费率号 */
}MSNvmYKC16ParamBillMode_Struct;

typedef struct
{
    MSNvmYKC16ParamBillMode_Struct stBillMode;
}MSNvmYKC16Param_Struct;
/*********************************** 国网e充电 */
typedef struct
{
    uint8_t  billModeID[17];                                    /* 计费模型ID */
    uint8_t  validFlag;                                         /* 有效标志 */
    uint8_t  periodCount;                                       /* 实际时段数(1-96) */
    uint8_t  startTime[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX][2];   /* 起始时间 HHMM */
    uint32_t elecPrice[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX];      /* 电费单价(0.00001元) */
    uint32_t servPrice[MSNVM_GWE_BILLMODE_NVM_PERIOD_MAX];      /* 服务费单价(0.00001元) */

} MSNvmGWEParamBillMode_Struct;

typedef struct
{
    uint32_t equipParamReportCycle;                             /* 实时监测属性上报频率 */
    uint32_t gunElecReportCycle;                                /* 充电中实时监测属性上报频率 */
    uint32_t nonElecReportCycle;                                /* 非充电中实时监测属性上报频率 */
    uint32_t faultWarningsCycle;                                /* 故障告警全信息上传频率 */
    uint32_t offlineChaLen;                                     /* 离线后可充电时长 */

    char cProductKey[MSNVM_GWE_PRODUCT_KEY_LEN + 1];
    char cDeviceName[MSNVM_GWE_DEVICE_NAME_LEN + 1];
    char cDeviceSecret[MSNVM_GWE_DEVICE_SECRET_LEN + 1];
    uint8_t credentialSaveFlag;                                 /* 凭据已获取标记 */
    char cUserName[MSNVM_GWE_USER_NAME_LEN + 1];                /* 用户名   */
    char cPassword[MSNVM_GWE_PASSWORD_LEN + 1];                 /* 密码  */
    uint16_t chargeSeq;                                         /* 充电序号, 0001-9999(用于设备订单号创建规则) */

}MSNvmGWEPlatInfo_Struct;

typedef struct
{
    MSNvmGWEParamBillMode_Struct stBillMode;
    MSNvmGWEPlatInfo_Struct platinfo;
}MSNvmGWEParam_Struct;


typedef struct
{
    uint8_t periodSerial;
    uint8_t periodRate;
    uint8_t startTime[2];
    uint8_t stopTime[2];
    uint8_t elecPrice[4];
    uint8_t servePrice[4];
}MSNvmAPParamBillPeriod_Struct;

typedef struct
{
    uint8_t billModeID[8];
    uint8_t switchTime[7];
    uint8_t invalidTime[7];
    uint8_t workState[2];
    uint8_t periodCount;
    MSNvmAPParamBillPeriod_Struct period[MSNVM_AP_BILLMODE_PERIOD_COUNT];
}MSNvmAPParamBillMode_Struct;

/* B47双缓冲计费模型存储结构(每枪独立一套A/B) */
typedef struct
{
    uint8_t recentUpdateIndex;                                              /* 最近更新的A/B/C组索引(0=A组, 1=B组, 2=C组) */
    MSNvmAPParamBillMode_Struct billModeData[MSNVM_AP_BILLMODE_MAX_NUM];    /* A/B/C三缓冲计费模型数据 [0]=A组 [1]=B组 [2]=C组 */
}IotAPBillModeSave_Struct;

typedef struct
{
    IotAPBillModeSave_Struct stBillModeSave[SYSCFG_CFG_GUN_NUM];
    uint32_t powerCtrlDefaultValue[SYSCFG_CFG_GUN_NUM];            /* B33默认功率，0.01kW */
}MSNvmAPParam_Struct;

typedef union 
{
    MSNvmXDTParam_Struct   stXDTParam;
    MSNvmGNParam_Struct    stGNParam;
    MSNvmYKC16Param_Struct stYKC16Param;
    MSNvmYKC21Param_Struct stYKC21Param;
    MSNvmXJParam_Struct    stXJParam;
    MSNvmGWEParam_Struct   stGWEParam;
    MSNvmAPParam_Struct    stAPParam;
    MSNvmAHTTParam_Struct  stAHTTParam;
    uint8_t paramArr[MSNVM_PLAT_PRIVATE_PARAM_LEN];
}MSNvmPlatPrivateParam_Union;

typedef char MSNvmAHTTPrivateUnionSizeCheck[
    (sizeof(MSNvmPlatPrivateParam_Union) == MSNVM_PLAT_PRIVATE_PARAM_LEN) ? 1 : -1];
/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif



















