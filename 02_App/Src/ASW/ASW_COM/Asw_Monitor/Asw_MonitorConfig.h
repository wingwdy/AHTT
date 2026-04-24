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
#ifndef ASW_MONITOR_CONFIG_H_
#define ASW_MONITOR_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_Monitor.h"
#include "DS_LogM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWMONITOR_CFG_LogPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_Monitor, fmt, ##__VA_ARGS__)
#define ASWMONITOR_CFG_RunLogPrint(fmt, ...)       DSLOGM_Info(DSLogMModule_Monitor, fmt, ##__VA_ARGS__)

#define ASWMONITOR_CFG_SAVE_CHARGE_RECORD_PERIOD   (60 * 1000U)

#define ASWMONITOR_CFG_REBOOT_DELAY_TIME           (2 * 1000U)

/* 充电最小余额，1元，保留2位小数 */
#define ASWMONITOR_CFG_CHARGE_MIN_ACCOUNT_MONEY    (100)

/* 按金额充电，最小充电金额：0.5元，保留2位小数 */
#define ASWMONITOR_CFG_CHARGE_MIN_CHARGE_MONEY     (50)
/* 按金额充电，最小充电时间：1秒， */
#define ASWMONITOR_CFG_CHARGE_MIN_CHARGE_TIME      (1)
/* 按电量充电，最小充电电量：0.05度，保留2位小数 */
#define ASWMONITOR_CFG_CHARGE_MIN_CHARGE_ENERGY    (5)

#define ASWMONITOR_CFG_ReadBlockOrderInfo(port, orderInfo, size, ret)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    ret = MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0OrderInfo, orderInfo, size);\
                                                                }\
                                                                else\
                                                                {\
                                                                    ret = eGlobalRet_Error;\
                                                                }\
                                                            }while(0)

#define ASWMONITOR_CFG_WriteBlockOrderInfo(port, orderInfo, size)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0OrderInfo, orderInfo, size);\
                                                                }\
                                                            }while(0)



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

#endif /* ASW_MONITOR_CONFIG_H_ */






















