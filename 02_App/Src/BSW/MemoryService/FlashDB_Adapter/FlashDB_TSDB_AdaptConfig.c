/******************************************************************************
* File Name          : FlashDB_TSDB_AdaptConfig.c
* Description        : Code for The adapter layer of TSDB
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
#include "FlashDB_TSDB_AdaptConfig.h"
#include "MS_NvmAppTypes.h"

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/




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
static void TSDBAdaptConfig_SetLock(void);
static void TSDBAdaptConfig_SetUnlock(void);
static uint8_t TSDBAdaptConfig_CreatLock(void);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SemaphoreHandle_t g_TSDBMutex = NULL;

TSDBAdaptConfig_Struct g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_Count] = 
{
    [eTSDBAdaptChannel_ChargeRecord] = 
    {
        .eCh = eTSDBAdaptChannel_ChargeRecord,
        .tsdb = {
            .user_data = &g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_ChargeRecord],
        },
        .pFuncCreatLock = TSDBAdaptConfig_CreatLock,
        .pFuncSetLock = TSDBAdaptConfig_SetLock,
        .pFuncSetUnlock = TSDBAdaptConfig_SetUnlock,
        .dbName = "charge_record",
        .flashPartName = FAL_TSDB_NAME_CHARGE_RECORD,
	    .maxLen = sizeof(MSNvmOrderInfo_Struct),
        .staticInfo = { 0 },
    },

    [eTSDBAdaptChannel_ErrorRecord] = 
    {
        .eCh = eTSDBAdaptChannel_ErrorRecord,
        .tsdb = {
            .user_data = &g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_ErrorRecord],
        },
        .pFuncCreatLock = TSDBAdaptConfig_CreatLock,
        .pFuncSetLock = TSDBAdaptConfig_SetLock,
        .pFuncSetUnlock = TSDBAdaptConfig_SetUnlock,
        .dbName = "error_record",
        .flashPartName = FAL_TSDB_NAME_ERROR_RECORD,
	    .maxLen = sizeof(MSNvmErrorInfo_Struct),
        .staticInfo = { 0 },
    },  
    
    [eTSDBAdaptChannel_RunningLog] = 
    {
        .eCh = eTSDBAdaptChannel_RunningLog,
        .tsdb = {
            .user_data = &g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_RunningLog],
        },
        .pFuncCreatLock = TSDBAdaptConfig_CreatLock,
        .pFuncSetLock = TSDBAdaptConfig_SetLock,
        .pFuncSetUnlock = TSDBAdaptConfig_SetUnlock,
        .dbName = "running_log",
        .flashPartName = FAL_TSDB_NAME_RUNNING_LOG,
	    .maxLen = sizeof(MSNvmRunningLog_Struct),
        .staticInfo = { 0 },
    },

    [eTSDBAdaptChannel_OmChargeRecord] = 
    {
        .eCh = eTSDBAdaptChannel_OmChargeRecord,
        .tsdb = {
            .user_data = &g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_OmChargeRecord],
        },
        .pFuncCreatLock = TSDBAdaptConfig_CreatLock,
        .pFuncSetLock = TSDBAdaptConfig_SetLock,
        .pFuncSetUnlock = TSDBAdaptConfig_SetUnlock,
        .dbName = "om_charge_record",
        .flashPartName = FAL_TSDB_NAME_OM_CHARGE_RECORD,
	    .maxLen = sizeof(MSNvmOrderInfo_Struct),
        .staticInfo = { 0 },
    },

    [eTSDBAdaptChannel_MeterRecord] =
    {
        .eCh = eTSDBAdaptChannel_MeterRecord,
        .tsdb = {
            .user_data = &g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_MeterRecord],
        },
        .pFuncCreatLock = TSDBAdaptConfig_CreatLock,
        .pFuncSetLock = TSDBAdaptConfig_SetLock,
        .pFuncSetUnlock = TSDBAdaptConfig_SetUnlock,
        .dbName = "meter_record",
        .flashPartName = FAL_TSDB_NAME_METER_RECORD,
        .maxLen = sizeof(MSNvmMeterRecord_Struct),
        .staticInfo = { 0 },
    },
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void TSDBAdaptConfig_SetLock(void)
{
	xSemaphoreTake(g_TSDBMutex, portMAX_DELAY);
}

static void TSDBAdaptConfig_SetUnlock(void)
{
	xSemaphoreGive(g_TSDBMutex);
}

static uint8_t TSDBAdaptConfig_CreatLock(void)
{
    uint8_t ret = FALSE;
     
    if (g_TSDBMutex == NULL)
    {
        g_TSDBMutex = xSemaphoreCreateMutex();

        if (g_TSDBMutex != NULL)
        {
            ret = TRUE;
        }
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}




















