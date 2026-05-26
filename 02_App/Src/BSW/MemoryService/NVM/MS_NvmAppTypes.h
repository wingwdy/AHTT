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

/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 二维码长度 */
#define MSNVM_QRCODE_LEN                      256

/* 订单长度 */
#define MSNVM_ORDER_MAX_LEN                   512

/* 故障信息长度 */
#define MSNVM_ERROR_INFO_MAX_LEN              288

/* 运行日志长度 */
#define MSNVM_RUNNING_LOG_MAX_LEN             3584

/* 桩编码字符串长度 */
#define MSNVM_PILE_DN_LEN                     40

/* 各平台私有参数长度 */
#define MSNVM_PLAT_PRIVATE_PARAM_LEN          (512 + 128)

/* 平台IP长度 */
#define MSNVM_PLAT_IP_LEN                     72

/************************* GN(公牛) ****************************************/
/* 计费模型 */
#define MSNVM_GN_BILLMODE_MULTRATE_COUNT      9
#define MSNVM_GN_BILLMODE_4RATE_COUNT         4
#define MSNVM_GN_BILLMIDE_PERIOD_COUNT        48

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

/************************* YKC16(云快充1.6) ****************************************/
/* 计费模型 */
#define MSNVM_YKC16_BILLMODE_4RATE_COUNT         4
#define MSNVM_YKC16_BILLMIDE_PERIOD_COUNT        48
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
    uint8_t forbidState;
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
    uint8_t reverse[64];
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
    uint8_t logicCardNum[8];                  /* 逻辑卡号 */
}MSNvmYKC16OrderInfo_Struct;

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

typedef union 
{
    MSNvmXDTOrderInfo_Struct stXDTOrderInfo;
    MSNvmGNOrderInfo_Struct stGNOrderInfo;
    MSNvmYKC16OrderInfo_Struct stYKC16OrderInfo;
    MSNvmYKC21OrderInfo_Struct stYKC21OrderInfo;
    uint8_t userData[MSNVM_ORDER_MAX_LEN];
}MSNvmPlatOrderInfo_Union;

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

typedef struct 
{
    MSNvmGNParamBillMode_Struct stBillMode;
}MSNvmGNParam_Struct;

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

typedef struct YKC16platinfo
{
    uint8_t  MaxPowerRate;                                      /* 充电桩最大允许输出功率百分比 1BIN 表示1%，最大 100%，最小30%*/
} MSNvmYKC16PlatInfo_Struct;

typedef struct 
{
    MSNvmYKC16ParamBillMode_Struct stBillMode;
    MSNvmYKC16PlatInfo_Struct platInfo;
}MSNvmYKC16Param_Struct;

typedef union 
{
    MSNvmXDTParam_Struct   stXDTParam;
    MSNvmGNParam_Struct    stGNParam;
    MSNvmYKC16Param_Struct stYKC16Param;
    MSNvmYKC21Param_Struct stYKC21Param;
    uint8_t paramArr[MSNVM_PLAT_PRIVATE_PARAM_LEN];
}MSNvmPlatPrivateParam_Union;
/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif



















