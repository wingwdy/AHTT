/******************************************************************************
* File Name          : MS_NvmConfig.h
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
#ifndef MS_NVM_CONFIG_H_
#define MS_NVM_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "MS_MemIf.h"
#include "Common.h"
#include "MS_Nvm.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MSNVM_CFG_ADDTION_CRC16_LEN              2U


/******************************************************************************
*    Enum Definition
******************************************************************************/




/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint16_t  blockSize;
    uint8_t   *ramBlockDataAddr;
    uint8_t   deviceID;
    uint16_t  memIfID;
    void (*pFuncDefault)(uint8_t *pIndata, uint16_t dataLen);
}MSNvmBlockDescriptor_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern const MSNvmBlockDescriptor_Struct c_stMSNvmBlockDescriptorTable[eMSNvmBlockID_Count];

#endif





















