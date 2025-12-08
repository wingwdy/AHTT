/******************************************************************************
* File Name          : MS_NvmConfig.c
* Description        : Code for The core service layer for managing non-volatile data 
                       storage of the ECU
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "MS_NvmConfig.h"
#include "MS_Nvm.h"
#include "MS_NvmAppTypes.h"
#include "MS_MemIf.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/ 



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void MSNvmConfig_DefaultGun0Qrcode(uint8_t *pIndata, uint16_t dataLen);



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static uint8_t g_MSNvmQrcodeRam[sizeof(MSNvmDrcode_Struct) + MSNVM_CFG_ADDTION_CRC16_LEN];
static uint8_t g_MSNvmOrderInfoGun0Ram[sizeof(MSNvmOrderInfo_Struct) + MSNVM_CFG_ADDTION_CRC16_LEN];


const MSNvmBlockDescriptor_Struct c_stMSNvmBlockDescriptorTable[eMSNvmBlockID_Count] = 
{
    [eMSNvmBlockID_Gun0Qrcode] = 
    {
        .blockSize = sizeof(MSNvmDrcode_Struct),
        .ramBlockDataAddr = g_MSNvmQrcodeRam,
        .deviceID = MSMEMIF_DEVICE_EA_KVDB,
        .memIfID = eKVDBAdaptChannel_Gun0Qrcode,
        .pFuncDefault = MSNvmConfig_DefaultGun0Qrcode,
    },

    [eMSNvmBlockID_OrderRecord] = 
    {
        .blockSize = sizeof(MSNvmOrderInfo_Struct),
        .ramBlockDataAddr = NULL,
        .deviceID = MSMEMIF_DEVICE_EA_TSDB,
        .memIfID = eTSDBAdaptChannel_ChargeRecord,
    },

    [eMSNvmBlockID_ErrorRecord] = 
    {
        .blockSize = sizeof(MSNvmErrorInfo_Struct),
        .ramBlockDataAddr = NULL,
        .deviceID = MSMEMIF_DEVICE_EA_TSDB,
        .memIfID = eTSDBAdaptChannel_ErrorRecord,
    },
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void MSNvmConfig_DefaultGun0Qrcode(uint8_t *pIndata, uint16_t dataLen)
{
    uint16_t index = 0;
    
    for (index = 0; index < 512; index++)
    {
        pIndata[index] = index;
    }
}























