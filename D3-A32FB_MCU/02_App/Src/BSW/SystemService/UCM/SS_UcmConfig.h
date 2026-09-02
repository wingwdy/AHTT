/******************************************************************************
* File Name          : template_Config.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef SS_UCM_CONFIG_H_
#define SS_UCM_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_Monitor.h"
#include "Asw_Charge.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define SSUCM_CONFIG_SINGLE_FRAME_LEN            (1024U)

#define SSUCM_CONFIG_PACK_FRAME_CNT              (64U)

#define SSUCM_CONFIG_CUT_PACK_SIZE               (SSUCM_CONFIG_PACK_FRAME_CNT * SSUCM_CONFIG_SINGLE_FRAME_LEN)	

#define SSUCM_CONFIG_TIMEOUT_MS                  (600000)

#define SSUCM_CONFIG_STABLE_TIMEOUT              (15000U)

#define SSUCM_CFG_DebugPrint(fmt, ...)           DSLOGM_Debug(DSLogMModule_System, fmt, ##__VA_ARGS__)
#define SSUCM_CFG_InfoPrint(fmt, ...)            DSLOGM_Info(DSLogMModule_System, fmt, ##__VA_ARGS__)

#define SSUCM_CFG_Reboot()                       AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);

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


#endif /* SS_UCM_CONFIG_H_ */





















