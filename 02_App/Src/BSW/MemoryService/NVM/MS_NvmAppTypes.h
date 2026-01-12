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
#define MSNVM_QRCODE_LEN                   256
#define MSNVM_ORDER_MAX_LEN                512
#define MSNVM_ERROR_INFO_MAX_LEN           32

#define MSNVM_PILE_DN_LEN                  40

#define MSNVM_PLAT_PRIVATE_PARAM_LEN       512

#define MSNVM_PLAT_IP_LEN                  72

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

/* 订单记录 */
typedef struct 
{
    uint8_t userData[MSNVM_ORDER_MAX_LEN];
}MSNvmOrderInfo_Struct;

/* 故障记录 */
typedef struct 
{
    uint8_t userData[MSNVM_ERROR_INFO_MAX_LEN];
}MSNvmErrorInfo_Struct;




/*********************************************************************************************** */
/* 各平台私有参数定义 */
typedef union 
{
    uint8_t paramArr[MSNVM_PLAT_PRIVATE_PARAM_LEN];
   
}MSNvmPlatPrivateParam_Union;


typedef struct 
{
    MSNvmPlatPrivateParam_Union privateParam;
}MSNvmPlatPrivateParam_Struct;





/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif



















