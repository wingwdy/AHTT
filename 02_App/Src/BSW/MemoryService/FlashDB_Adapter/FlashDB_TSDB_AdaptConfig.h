/******************************************************************************
* File Name          : FlashDB_TSDB_Adapt.h
* Description        : Code for The adapter layer of TSDB
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
#ifndef FLASHDB_TSDB_ADAPT_CONFIG_H_
#define FLASHDB_TSDB_ADAPT_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include <flashdb.h>
#include <common.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "fal_cfg.h"
#include "FlashDB_TSDB_Adapt.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eTSDBAdaptReportState_unReported,
	eTSDBAdaptReportState_Reported,
	eTSDBAdaptReportState_Count,
}TSDBAdaptReportState_Enum;
  
/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
	uint16_t totalCount;
}TSDBAdaptStaticInfo_Struct;


typedef struct 
{
	TSDBAdaptChannel_Enum eCh;
    struct fdb_tsdb tsdb;
    uint8_t initFlag;   
    uint8_t (*pFuncCreatLock)(void);
    void (*pFuncSetLock)(void); 
    void (*pFuncSetUnlock)(void); 
    const char *dbName;
    const char *flashPartName;
	uint16_t maxLen;
	uint32_t time;
	TSDBAdaptStaticInfo_Struct staticInfo[eTSDBAdaptReportState_Count];
}TSDBAdaptConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern TSDBAdaptConfig_Struct g_stTSDBAdaptConfigTable[eTSDBAdaptChannel_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif






















