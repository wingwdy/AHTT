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
#define MSNVM_APP_QRCODE_LEN                  256
#define MSNVM_APP_ORDER_PRIVATE_LEN           512
#define MSNVM_APP_ERROR_PRIVATE_LEN           32
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
    char qrcode[MSNVM_APP_QRCODE_LEN];
}MSNvmDrcode_Struct;

/* 电能示值 */
typedef struct
{
    uint64_t meterEnergy;
}MSNvmMeterEnergy_Struct;

/* 订单记录 */
typedef struct 
{
    uint8_t userData[MSNVM_APP_ORDER_PRIVATE_LEN];
}MSNvmOrderInfo_Struct;

/* 故障记录 */
typedef struct 
{
    uint8_t userData[MSNVM_APP_ERROR_PRIVATE_LEN];
}MSNvmErrorInfo_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif



















