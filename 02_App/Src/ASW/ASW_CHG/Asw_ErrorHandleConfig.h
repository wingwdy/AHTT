/******************************************************************************
* File Name          : Asw_ErrorHandleConfig.h
* Description        : Code for Errorhandle
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
#ifndef ASW_ERRORHANDLE_CONFIG_H_
#define ASW_ERRORHANDLE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Asw_ErrorHandle.h"
#include "stdint.h"
#include "Cdd_CP.h"
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWERR_CFG_CALLCYCLE                          (100U)

#define ASWERR_CFG_MULTI_ENABLE                       FALSE

#define ASWERR_CFG_ErrStateChangeNotice(port, eErr, flag, pErrHandle)  do\
                                                             {\
                                                             }while(0)
                                                             
#define ASWERR_CFG_LogPrint(fmt, ...)                 DSLOGM_Debug(DSLogMModule_ErrorHandle, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    AswErrorLevel_0,       /* 无效值 */
    AswErrorLevel_1,       /* 非故障告警类，正常终止类型 */
    AswErrorLevel_2,       /* 告警提示，不影响充电 */
    AswErrorLevel_3,       /* 不取消授权、暂停充电，故障次数取消授权清除 */
    AswErrorLevel_4,       /* 取消授权、停止充电，故障次数取消授权清除 */
    AswErrorLevel_5,       /* 需要断电才能清除的故障，故障次数掉电清除 */
    AswErrorLevel_Cnt,    
}AswErrorLevel_Enum; 

typedef enum
{
    AswErrorClear_None,
    AswErrorClear_Internal,  /* 由该模块，经过恢复时间之后主动清除 */
    AswErrorClear_External,  /* 由外部模块经过检测之后清除 */
}AswErrorClear_Enum;

typedef enum
{
    AswErrorOwner_None,
    AswErrorOwner_Gun,  /* 故障属于某把枪 */   
    AswErrorOwner_Pile, /* 故障属于整个桩，产生和清除时对应的枪都受影响 */ 
}AswErrorOwner_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    AswErrorType_Enum eError;         /* 故障类型 */
    AswErrorOwner_Enum eErrOwner;     /* 故障Owner */
    AswErrorLevel_Enum tempErrLevel;  /* 临时故障等级*/
    AswErrorLevel_Enum finalLevel;    /* 最终故障等级*/
    AswErrorClear_Enum errClearType;  /* 故障清除方式 */
    uint8_t errCount;                 /* 故障次数 */
    uint16_t recoveryTime;            /* 恢复时间 */
    char *errDesc;                    /* 故障描述 */
}AswErrorHandleConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern const AswErrorHandleConfig_Struct c_AswErrorHandleConfigTable[eErr_Num];

#endif /* ASW_ERRORHANDLE_CONFIG_H_ */





















