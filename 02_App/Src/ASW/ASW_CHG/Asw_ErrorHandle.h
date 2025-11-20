
/******************************************************************************
* File Name          : Asw_ErrorHandle.h
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
#ifndef ASW_ERRORHANDLE_H_
#define ASW_ERRORHANDLE_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eErrChargeCondition_Allow,         /* 允许充电 */
    eErrChargeCondition_Suspend,       /* 暂停充电 */
    eErrChargeCondition_Cancel,        /* 取消授权 */
}AswErrChargeCondition_Enum;

typedef enum
{
    eErr_none,								 
    eErr_CpVoltAbnor,                        /* CP电压异常 */    
    eErr_CpGroundFault,                      /* CP对地短路 */ 
    eErr_PEBreakFault,                       /* PE接地故障 */  
    eErr_EmergencyStop,                      /* 急停故障 */        
    eErr_InputLineReversed,                  /* 火零反接 */
    eErr_LeakageCurrErr,                     /* 漏电流故障 */ 
    eErr_ShortCircleErr,                     /* 短路故障 */ 
    eErr_RCDSelfcheckErr,                    /* RCD自检故障 */             
    eErr_AphaseInputOverVol,                 /* 交流A相输入过压 */ 

    eErr_BphaseInputOverVol,                 /* 交流B相输入过压 */
    eErr_CphaseInputOverVol,                 /* 交流C相输入过压 */
    eErr_AphaseInputLessVol,                 /* 交流A相输入欠压 */
    eErr_BphaseInputLessVol,                 /* 交流B相输入欠压 */
    eErr_CphaseInputLessVol,                 /* 交流C相输入欠压 */
    eErr_OutputOverCurr,                     /* 交流输出过流 */
    eErr_JcqMaloperation,                    /* 交流输出接触器误动拒动 */
    eErr_JcqSynechiaFault,                   /* 交流输出接触器粘连 */
    eErr_HmiCommErr,                         /* 人机交互通信故障 */
    eErr_ReaderCommErr,                      /* 读卡器通信故障 */

    eErr_MeterCommErr,                       /* 电表通信故障 */ 
    eErr_EnvOverTempErr,                     /* 环境过温故障 */
    eErr_GunOverTempErr,                     /* 枪过温故障 */
    eErr_POverTempErr,                       /* 插头过温 */ 
    eErr_DatabaseErr,                        /* 数据库存储错误 */
    eErr_MeterCalcErr,                       /* 电能表计量故障 */
    eErr_ChgStartTimeout,                    /* 启动超时 */
    eErr_DiodeStop,                          /* 不存在二极管 */   
    eSrc_LittleCurr,                         /* 小电流停止 */
    eSrc_S2BreakOff,                         /* S2断开主动停止 */ 

    eSrc_AppStop,                            /* 远程停止 */
    eSrc_CardStop,                           /* 刷卡停止 */
    eSrc_InsuffBalance,                      /* 余额不足停止 */
    eSrc_StopbyMoney,                        /* 按金额停止 */
    eSrc_StopbyTime,                         /* 按时间停止 */
    eSrc_StopbyEnergy,                       /* 按电量停止 */
    eErr_GunDisConn,                         /* 拔枪停止 */
    eErr_NetNoSIMErr,                        /* 未检测到SIM卡 */
    eErr_PlatformOffline,                    /* 平*未正常上线，平台通信异常 */
    eErr_Num,                                /* 故障数(含告警) */
}AswErrorType_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void AswErrhandle_SetErrExsitCallback(uint8_t port, AswErrorType_Enum errType);
void AswErrhandle_ResetErrExsitCallback(uint8_t port, AswErrorType_Enum errType);
uint8_t AswErrHandle_CheckErrExit(uint8_t port, AswErrorType_Enum errType);
AswErrChargeCondition_Enum AswErrHandle_GetChargeCondition(uint8_t port);
void AswErrHandle_ClearStopReason(uint8_t port);
AswErrorType_Enum AswErrHandle_GetStopReason(uint8_t port);
void AswErrHandle_InitMemory(void);
void AswErrHandle_MainFunction(void);
uint8_t AswErrHandle_IsExsistError(uint8_t port);

#endif /* ASW_ERRORHANDLE_H_ */























