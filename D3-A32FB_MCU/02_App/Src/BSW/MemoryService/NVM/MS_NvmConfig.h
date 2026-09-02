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
#define MSNVM_CFG_ADDTION_CRC16_LEN               2U      /* 参数块尾部附加的CRC16长度 */
#define MSNVM_CFG_WRITE_VERIFY_BUF_SIZE           512U    /* 所有参数块共用的异步回读缓存大小 */
#define MSNVM_CFG_ASYNC_VERIFY_DELAY_MS           100U    /* 参数写入后启动异步回读校验的延时时间 */
#define MSNVM_CFG_ASYNC_VERIFY_RETRY_COUNT        2U      /* 异步回读不一致时允许的最大重写次数 */
#define MSNVM_CFG_ASYNC_BACKUP_DELAY_MS           500U    /* 主块校验成功后写入备份块的延时时间 */


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
    uint8_t   backupEnable;
    uint8_t   asyncWriteVerifyEnable;
    uint8_t   asyncBackupEnable;
    uint16_t  backupBlockID;
}MSNvmBlockDescriptor_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern const MSNvmBlockDescriptor_Struct c_stMSNvmBlockDescriptorTable[eMSNvmBlockID_Count];

#endif





















