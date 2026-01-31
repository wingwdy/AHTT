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

/******************************************************************************
*    Macro Definition
******************************************************************************/

#define ASWVOLTCUR_CFG_CALLCYCLE                            (100u)

#define ASWVOLTCUR_CFG_SET_OV_THR                           (26400u)/*产生过压故障阈值*/
#define ASWVOLTCUR_CFG_CLR_OV_THR                           (25400u)/*解除过压故障阈值*/

#define ASWVOLTCUR_CFG_SET_UV_THR                           (17600u)/*产生欠压故障阈值*/
#define ASWVOLTCUR_CFG_CLR_UV_THR                           (18500u)/*解除欠压故障阈值*/

#define ASWVOLTCUR_CFG_SET_OC_THR                           (35200u)/*产生过流故障阈值*/
#define ASWVOLTCUR_CFG_CLR_OC_THR                           (35190u)/*解除过流故障阈值*/


#define ASWVOLTCUR_CFG_OV_SET_FILTER_COUNT                  (10000u / ASWVOLTCUR_CFG_CALLCYCLE)
#define ASWVOLTCUR_CFG_OV_CLR_FILTER_COUNT                  (10000u / ASWVOLTCUR_CFG_CALLCYCLE)

#define ASWVOLTCUR_CFG_UV_SET_FILTER_COUNT                  (10000u / ASWVOLTCUR_CFG_CALLCYCLE) /* 产生过流故障阈值 */
#define ASWVOLTCUR_CFG_UV_CLR_FILTER_COUNT                  (10000u / ASWVOLTCUR_CFG_CALLCYCLE) /* 产生过流故障阈值 */

#define ASWVOLTCUR_CFG_OC_SET_FILTER_COUNT                  (5000u / ASWVOLTCUR_CFG_CALLCYCLE) /* 产生过流故障阈值 */
#define ASWVOLTCUR_CFG_OC_CLR_FILTER_COUNT                  (5000u / ASWVOLTCUR_CFG_CALLCYCLE) /* 解除过流故障阈值 */

#define ASWVOLTCUR_CFG_LogPrint(fmt, ...)                   DSLOGM_Debug(DSLogMModule_VoltCur, fmt, ##__VA_ARGS__)

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    AswVoltType = 0,
    AswCurrType,   
} AswVoltCurType_Enum;

typedef enum
{
    AswVoltCur_OV_Set = 0,
    AswVoltCur_OV_Clr,
    AswVoltCur_UV_Set,
    AswVoltCur_UV_Clr,
    AswVoltCur_OC_Set,
    AswVoltCur_OC_Clr,
    AswVoltCur_EvtCnt,
} AswVoltCurEvent_Enum;

typedef enum
{
    AswVoltCurCmp_MaxEqu = 0, /*大于等于*/
    AswVoltCurCmp_Max,        /*大于*/
    AswVoltCurCmp_MinEqu,     /*小于等于*/
    AswVoltCurCmp_Min,        /*小于*/
    AswVoltCurCmp_Equ,        /*等于*/
} AswVoltCurCMP_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    AswVoltCurEvent_Enum event;
    AswVoltCurType_Enum type;
    uint8_t  compareType;
    uint16_t threshold;
    uint16_t filterCount;
    uint8_t  setErrFlag;
    AswErrorType_Enum errType;
} AswVoltCurHandleConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const AswVoltCurHandleConfig_Struct c_AswVoltCurHandleConfigTable[AswVoltCur_EvtCnt];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* ASW_VOLTCURHANDLE_CONFIG_H_ */





















