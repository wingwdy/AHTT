/******************************************************************************
* File Name          : Cdd_RcdConfig.h
* Description        : Code for Leakage detection driver implementation
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      shenjc    初版创建
*
******************************************************************************/
#ifndef CDD_RCD_CONFIG_H_
#define CDD_RCD_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Port.h"
#include "DS_LogM.h"
#include "Common.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDRCD_CFG_GetSysTick()							    Common_GetSystick()

#define CDDRCD_CFG_SetCalPin()				                McalPort_SetPin(eMcalPortPinChanel_PB2_RCDZero)
#define CDDRCD_CFG_ResetCalPin()		                    McalPort_ResetPin(eMcalPortPinChanel_PB2_RCDZero)
#define CDDRCD_CFG_SetTestPin()			                    McalPort_SetPin(eMcalPortPinChanel_PB11_RCDTest)
#define CDDRCD_CFG_ResetTestPin()			                McalPort_ResetPin(eMcalPortPinChanel_PB11_RCDTest)
#define CDDRCD_CFG_GetTripPin()					        	McalPort_GetPin(eMcalPortPinChanel_PB1_RCDTrip)

#define CDDRCD_CFG_SET                                      MCALPORT_PIN_HIGH    
#define CDDRCD_CFG_RESET                                    MCALPORT_PIN_LOW

#define CDDRCD_CFG_TASK_PERIOD                    		    (uint16_t)(10u) /*10ms*/
#define CDDRCD_CFG_SELFCHECK_CAL_T2_TIME             	    ((uint16_t)80u / CDDRCD_CFG_TASK_PERIOD)/*50ms<=T2<=100ms*/
#define CDDRCD_CFG_SELFCHECK_CAL_T3_TIME             	    ((uint16_t)500u  / CDDRCD_CFG_TASK_PERIOD)/*T3>=500ms*/
#define CDDRCD_CFG_SELFCHECK_TEST_T5_TIME             	    ((uint16_t)200u  / CDDRCD_CFG_TASK_PERIOD)/*T5>=200ms*/
#define CDDRCD_CFG_SELFCHECK_CHK_INT_TIME                   ((uint16_t)50u  / CDDRCD_CFG_TASK_PERIOD)/*50ms检测1次*/
#define CDDRCD_CFG_SELFCHECK_CHK_MAX_TIME                   ((uint16_t)2000u  / CDDRCD_CFG_TASK_PERIOD)/*持续检测2000ms*/
#define CDDRCD_CFG_SELFCHECK_EXIT_T6_TIME                   ((uint16_t)100u  / CDDRCD_CFG_TASK_PERIOD)/*T6>=100ms*/

#define CDDRCD_CFG_DebugPrint(fmt, ...)                     DSLOGM_Debug(DSLogMModule_RCD, fmt, ##__VA_ARGS__)
#define CDDRCD_CFG_InfoPrint(fmt, ...)                      DSLOGM_Info(DSLogMModule_RCD, fmt, ##__VA_ARGS__)

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


#endif /* CDD_RCD_CONFIG_H_ */

