/******************************************************************************
* File Name          : FlashDB_KVDB_AdaptConfig.c
* Description        : Code for The adapter layer of KVDB
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
#include "FlashDB_KVDB_AdaptConfig.h"



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
static uint8_t KVDBAdaptConfig_CreatLock(void);
static void KVDBAdaptConfig_SetLock(void);
static void KVDBAdaptConfig_SetUnlock(void);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
KVDBAdaptConfig_Struct g_stKVDBAdaptConfig = 
{
    .kvdb = {0},
    .initFlag = FALSE,
    .pFuncCreatLock = KVDBAdaptConfig_CreatLock,
    .pFuncSetLock = KVDBAdaptConfig_SetLock,
    .pFuncSetUnlock = KVDBAdaptConfig_SetUnlock,
    .dbName = "D3_A32FB",
    .flashPartName = FAL_KVDB_NAME_PARA,
    .default_kvs = NULL,
    .default_kvs_num = 0,
    .userData = NULL,
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t KVDBAdaptConfig_CreatLock(void)
{
    uint8_t ret = FALSE;
    g_stKVDBAdaptConfig.mutex = xSemaphoreCreateMutex();

    if (g_stKVDBAdaptConfig.mutex != NULL)
    {
        ret = TRUE;
    }

    return ret;
}

static void KVDBAdaptConfig_SetLock(void)
{
	xSemaphoreTake(g_stKVDBAdaptConfig.mutex, portMAX_DELAY);
}

static void KVDBAdaptConfig_SetUnlock(void)
{
	xSemaphoreGive(g_stKVDBAdaptConfig.mutex);
}


















