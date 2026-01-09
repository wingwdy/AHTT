/******************************************************************************
* File Name          : template.h
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
#ifndef CDD_MODEM_CONFIG_H_
#define CDD_MODEM_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "Mcal_Port.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_MODEM_CFG_CALL_CYCLE                           100

#define CDD_MODEM_CFG_GetFCTPin()                          (MCALPORT_PIN_HIGH == McalPort_GetPin(eMcalPortPinChanel_PC0_FCTPin) ? FALSE : TRUE)

#define CDD_MODEM_CFG_AGING_TEST_CURRENT_THRESHOLD         (28000U)
#define CDD_MODEM_CFG_AGING_TEST_TIMEOUT                   (27 * 60000U)

#define CDD_MODEM_CFG_FACTORY_TIMEOUT                      (72 * 3600)

#define CDD_MODEM_CFG_FCT_FILTER_POINT                     (300 / CDD_MODEM_CFG_CALL_CYCLE)

#define CDDMODE_CFG_LogPrint(fmt, ...)                     DSLOGM_Debug(DSLogMModule_ModeM, fmt, ##__VA_ARGS__)


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
#endif /* CDD_MODEM_CONFIG_H_ */























