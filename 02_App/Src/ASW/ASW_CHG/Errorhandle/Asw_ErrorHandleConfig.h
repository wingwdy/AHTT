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
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWERR_CFG_CALLCYCLE                          (100U)

#define ASWERR_CFG_MULTI_ENABLE                       FALSE

#define ASWERR_CFG_ErrStateChangeNotice(port, eErr, flag, errLevel)  do\
                                                             {\
                                                                AswErrHandleCfg_NoticeCallBack(port, eErr, flag, errLevel);\
                                                             }while(0)
                                                             
#define ASWERR_CFG_LogPrint(fmt, ...)                 DSLOGM_Debug(DSLogMModule_ErrorHandle, fmt, ##__VA_ARGS__)
#define ASWERR_CFG_RunLogPrint(fmt, ...)              DSLOGM_Info(DSLogMModule_ErrorHandle, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eAswErrorLevel_0,        /* 无效值 */
    eAswErrorLevel_1,        /* 非故障告警类，正常终止类型 */
    eAswErrorLevel_2,        /* 告警提示，不影响充电 */
    eAswErrorLevel_3,        /* 不取消授权、暂停充电，故障次数取消授权清除 */
    eAswErrorLevel_4,        /* 取消授权、停止充电，故障次数取消授权清除 */
    eAswErrorLevel_5,        /* 需要断电才能清除的故障，故障次数掉电清除 */
    AswErrorLevel_Cnt,    
}AswErrorLevel_Enum; 

typedef enum
{
    eAswErrorClear_None,
    eAswErrorClear_Internal, /* 由该模块，经过恢复时间之后主动清除 */
    eAswErrorClear_External, /* 由外部模块经过检测之后清除 */
}AswErrorClear_Enum;

typedef enum
{
    eAswErrorOwner_None,
    eAswErrorOwner_Gun,      /* 故障属于某把枪 */   
    eAswErrorOwner_Pile,     /* 故障属于整个桩，产生和清除时对应的枪都受影响 */ 
}AswErrorOwner_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    AswErrorType_Enum  eError;        /* 故障类型 */
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
extern const AswErrorHandleConfig_Struct c_AswErrorHandleConfigTable[eErr_Num];
/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void AswErrHandleCfg_NoticeCallBack(uint8_t port, AswErrorType_Enum errType, uint8_t flag, AswErrorLevel_Enum errLevel);

#endif /* ASW_ERRORHANDLE_CONFIG_H_ */





















