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

#define ASWVOLTCUR_CFG_OV_SET_THR                           (26400u)/*产生过压阈值*/
#define ASWVOLTCUR_CFG_OV_CLR_THR                           (25900u)/*解除过压阈值*/

#define ASWVOLTCUR_CFG_UV_SET_THR                           (17600u)/*产生欠压阈值*/
#define ASWVOLTCUR_CFG_UV_CLR_THR                           (18100u)/*解除欠压阈值*/

#define ASWVOLTCUR_CFG_OC_SET_THR                           (3520u)/*产生过流阈值*/
#define ASWVOLTCUR_CFG_OC_CLR_THR                           (3519u)/*解除过流阈值*/


#define ASWVOLTCUR_CFG_OV_SET_FILTER_COUNT                  (3000u / ASWVOLTCUR_CFG_CALLCYCLE)
#define ASWVOLTCUR_CFG_OV_CLR_FILTER_COUNT                  (3000u / ASWVOLTCUR_CFG_CALLCYCLE)

#define ASWVOLTCUR_CFG_UV_SET_FILTER_COUNT                  (3000u / ASWVOLTCUR_CFG_CALLCYCLE)
#define ASWVOLTCUR_CFG_UV_CLR_FILTER_COUNT                  (3000u / ASWVOLTCUR_CFG_CALLCYCLE)

#define ASWVOLTCUR_CFG_OC_SET_FILTER_COUNT                  (5000u / ASWVOLTCUR_CFG_CALLCYCLE)
#define ASWVOLTCUR_CFG_OC_CLR_FILTER_COUNT                  (5000u / ASWVOLTCUR_CFG_CALLCYCLE)



/******************************************************************************
*    Enum Definition
******************************************************************************/
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
    AswVoltCur_MaxEqu = 0, /*大于等于*/
    AswVoltCur_Max,        /*大于*/
    AswVoltCur_MinEqu,     /*小于等于*/
    AswVoltCur_Min,        /*小于*/
    AswVoltCur_Equ,        /*等于*/
} AswVoltCur_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    AswVoltCurEvent_Enum event;
    uint8_t  compareType;
    uint16_t threshold;
    uint16_t filterCount;
    uint8_t  setErrorFlag;
    AswErrorType_Enum errorType;
} AswVoltCurHandleConfig_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const AswVoltCurHandleConfig_Struct c_AswVoltCurHandleConfigTable[AswVoltCur_EvtCnt];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* ASW_VOLTCURHANDLE_CONFIG_H_ */





















