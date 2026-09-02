/******************************************************************************
* File Name          : Asw_VoltCurHandleConfig.h
* Description        : Code for Errorhandle
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
#ifndef ASW_VOLTCURHANDLE_CONFIG_H_
#define ASW_VOLTCURHANDLE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_VoltCurHandle.h"
#include "Asw_ErrorHandle.h"
#include "stdint.h"
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWVOLTCUR_CFG_CALLCYCLE                            (100u)

#define ASWVOLTCUR_CFG_SET_OV_THR                           (26400u)/*产生过压故障阈值*/
#define ASWVOLTCUR_CFG_CLR_OV_THR                           (25400u)/*解除过压故障阈值*/
#define ASWVOLTCUR_CFG_OV_FILTER_COUNT                      (10000u / ASWVOLTCUR_CFG_CALLCYCLE)

#define ASWVOLTCUR_CFG_SET_UV_THR                           (17600u)/*产生欠压故障阈值*/
#define ASWVOLTCUR_CFG_CLR_UV_THR                           (18500u)/*解除欠压故障阈值*/
#define ASWVOLTCUR_CFG_UV_FILTER_COUNT                      (10000u / ASWVOLTCUR_CFG_CALLCYCLE)

#define ASWVOLTCUR_CFG_SET_OC_THR                           (35200u)/*产生过流故障阈值*/
#define ASWVOLTCUR_CFG_CLR_OC_THR                           (35190u)/*解除过流故障阈值*/
#define ASWVOLTCUR_CFG_OC_FILTER_COUNT                      (5000u / ASWVOLTCUR_CFG_CALLCYCLE) 

#define ASWVOLTCUR_CFG_DebugPrint(fmt, ...)                 DSLOGM_Debug(DSLogMModule_VoltCur, fmt, ##__VA_ARGS__)
#define ASWVOLTCUR_CFG_InfoPrint(fmt, ...)                  DSLOGM_Info(DSLogMModule_VoltCur, fmt, ##__VA_ARGS__)
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


#endif /* ASW_VOLTCURHANDLE_CONFIG_H_ */





















