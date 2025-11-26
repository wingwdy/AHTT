/******************************************************************************
* File Name          : Asw_EVSEConfig.h
* Description        : Code for Charge State Manage
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
#ifndef ASW_CHARGE_CONFIG_H_
#define ASW_CHARGE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWCHARGE_CFG_CALL_CYCLE                       (10)

#define ASWCHARGE_CFG_START_TIMEOUT                    15000

#define ASWCHARGE_CFG_STOP_TIMEOUT                     10000

#define ASWCHARGE_CFG_PAUSE_TIMEOUT                    30000

#define ASWCHARGE_CFG_LITTLE_CURRENT_FILTER_COUNT      ((30 * 60 * 1000) / ASWCHARGE_CFG_CALL_CYCLE)

#define ASWCHARGE_CFG_LITTLE_CURRENT_THRESHOLD         1000

#define ASWCHARGE_CFG_GetCurRateCurrent(port)          32000

#define ASWCHARGE_CFG_LogPrint(fmt, ...)               DSLOGM_Debug(DSLogMModule_Charge, fmt, ##__VA_ARGS__)

#define ASWCHARGE_CFG_GetOutputCurrent(port)           1000

#define ASWCHARGE_CFG_QUIT_FINISH_TIMEOUT              200
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_EVSE_CONFIG_H_ */























