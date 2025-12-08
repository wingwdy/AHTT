/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
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
#include "MS_MemIf.h"
#include "MS_MemIfConfig.h"
#include "FlashDB_KVDB_Adapt.h"

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
*    Global variables Declaration
*******************************************************************************/
const MSMemifConfig_Struct c_stMSMemifConfigTable[MSMEMIF_DEVICE_EA_COUNT] =
{
    [MSMEMIF_DEVICE_EA_KVDB] = 
    {
        .pFuncInit = KVDBAdapt_Init,
        .pFuncRead = KVDBAdapt_Read,
        .pFuncWrite = KVDBAdapt_Write,

    },

    [MSMEMIF_DEVICE_EA_TSDB] = 
    {
        .pFuncInit = TSDBAdapt_Init,
        .pFuncRead = NULL,
        .pFuncWrite = NULL,

        .pFuncClearDB = TSDBAdapt_CleanRecordDB,
        .pFuncQueryUnreportedRecordCount = TSDBAdapt_QueryUnreportedRecordCount,
        .pFuncQueryTotalRecordCount = TSDBAdapt_QueryTotalRecordCount,
        .pFuncInsertRecord = TSDBAdapt_InsertRecord,
        .pFuncSetReportSuccess = TSDBAdapt_SetRecordReportSuccess,
        .pFuncQueryLatestUnreportedRecord = TSDBAdapt_QueryLatestUnreportedRecord,
        .pFuncQueryRecordByTime = TSDBAdapt_QueryRecordByTime,
        .pFuncQueryRecordByExternal = TSDBAdapt_QueryRecordByExternal,
    },
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
























