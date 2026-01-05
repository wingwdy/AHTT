/******************************************************************************
* File Name          : Asw_TempHandleConfig.h
* Description        : Code for TempHandleConfig
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
#ifndef ASW_TEMPHANDLE_CONFIG_H_
#define ASW_TEMPHANDLE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_TempHandle.h"
#include "Cdd_Sensor.h"
#include "DS_LogM.h"
#include "stdint.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

#define ASWTEMP_CFG_CALLCYCLE                           (100u)

#define ASWTEMP_CFG_GUN_OTEMP_60_THR                    (60 + 50)/*枪过温恢复充电阈值*/
#define ASWTEMP_CFG_GUN_OTEMP_75_THR                    (75 + 50)/*枪过温停止充电阈值*/
#define ASWTEMP_CFG_GUN_OTEMP_90_THR                    (90 + 50)/*恢复降流充电阈值*/
#define ASWTEMP_CFG_GUN_OTEMP_105_THR                   (105 + 50)/*开始降流充电阈值*/

#define ASWTEMP_CFG_GUN_ABOVE_90_KEEP_TIME              (5*60*1000u)
#define ASWTEMP_CFG_GUN_BELOW_75_KEEP_TIME              (5*60*1000u)
#define ASWTEMP_CFG_GUN_TEMP_FILTER_COUNT               (3000u / ASWTEMP_CFG_CALLCYCLE)


#define ASWTEMP_CFG_ENV_OTEMP_65_THR                    (65 + 50)
#define ASWTEMP_CFG_ENV_OTEMP_85_THR                    (85 + 50)
#define ASWTEMP_CFG_ENV_ABOVE_85_KEEP_TIME              (5*60*1000u)
#define ASWTEMP_CFG_ENV_BELOW_65_KEEP_TIME              (5*60*1000u)
#define ASWTEMP_CFG_ENV_TEMP_FILTER_COUNT               (3000u / ASWTEMP_CFG_CALLCYCLE)


#define ASWTEMP_CFG_RATED_CURRENT                       (32000u)
#define ASWTEMP_CFG_LIMIT_CURRENT                       (25600u)


#define ASWTEMP_CFG_GetGunTemp(port)                    CddSensor_GetGunTemperature(port)
#define ASWTEMP_CFG_GetEnvTemp()                        CddSensor_GetEnvTemperature()

#define ASWTEMP_CFG_LogPrint(fmt, ...)                  DSLOGM_Debug(DSLogMModule_Temp, fmt, ##__VA_ARGS__)
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


#endif /* ASW_TEMPHANDLE_CONFIG_H_ */





















