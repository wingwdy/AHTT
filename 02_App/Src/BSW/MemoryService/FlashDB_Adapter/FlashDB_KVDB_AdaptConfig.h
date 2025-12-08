/******************************************************************************
* File Name          : FlashDB_KVDB_AdaptConfig.h
* Description        : Code for The adapter layer of KVDB
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
#ifndef FLASHDB_KVDB_ADAPT_CONFIG_H_
#define FLASHDB_KVDB_ADAPT_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include <flashdb.h>
#include "Common.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "fal_cfg.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    struct fdb_kvdb kvdb;
    SemaphoreHandle_t mutex;
    uint8_t initFlag;   
    uint8_t (*pFuncCreatLock)(void);
    void (*pFuncSetLock)(void); 
    void (*pFuncSetUnlock)(void); 
    const char *dbName;
    const char *flashPartName;
    struct fdb_default_kv_node *default_kvs;
    uint8_t default_kvs_num;
    void *userData;
}KVDBAdaptConfig_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern KVDBAdaptConfig_Struct g_stKVDBAdaptConfig;


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif





















