/******************************************************************************
* File Name          : Asw_ErrorHandleConfig.c
* Description        : Code for Errorhandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_ErrorHandleConfig.h"
#include "Asw_Charge.h"
#include "Cdd_CP.h"
#include "SS_Snapshot.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
  
const AswErrorHandleConfig_Struct c_AswErrorHandleConfigTable[eErr_Num] = 
{
    {eErr_none,               eAswErrorOwner_None,  eAswErrorLevel_0, eAswErrorLevel_0, eAswErrorClear_None,       1,     0,   "无效值"},
    {eErr_CpVoltAbnor,        eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "CP电压异常"},
    {eErr_CpGroundFault,      eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "CP对地短路"},
    {eErr_PEBreakFault,       eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "PE接地故障"},
    {eErr_EmergencyStop,      eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "急停故障"},
    {eErr_InputLineReversed,  eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "火零反接"},
    {eErr_LeakageCurrErr,     eAswErrorOwner_Pile,  eAswErrorLevel_5, eAswErrorLevel_5, eAswErrorClear_External,   1,     0,   "漏电流故障"},
    {eErr_ShortCircleErr,     eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "充电前输出短路故障"},
    {eErr_RCDSelfcheckErr,    eAswErrorOwner_Pile,  eAswErrorLevel_5, eAswErrorLevel_5, eAswErrorClear_External,   1,     0,   "RCD自检故障"},
    {eErr_AphaseInputOverVol, eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "交流A相输入过压"},
    {eErr_AphaseInputLessVol, eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "交流A相输入欠压"},
    {eErr_OutputOverCurr,     eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "交流输出过流"}, 
    {eErr_JcqMaloperation,    eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "交流输出继电器误动拒动"},         
    {eErr_JcqSynechiaFault,   eAswErrorOwner_Gun,   eAswErrorLevel_5, eAswErrorLevel_5, eAswErrorClear_External,   1,     0,   "交流输出继电器粘连"},       
    {eErr_HmiCommErr,         eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "人机交互通信故障"},

    {eErr_ReaderCommErr,      eAswErrorOwner_Gun,   eAswErrorLevel_2, eAswErrorLevel_2, eAswErrorClear_External,   1,     0,   "读卡器通信故障"}, 

    {eErr_MeterCommErr,       eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "电表通信故障"}, 
    {eErr_EnvOverTempErr,     eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "环境过温故障"},
    {eErr_GunOverTempErr,     eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "枪过温故障"},
    {eErr_POverTempErr,       eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "插头过温"},
    {eErr_DatabaseErr,        eAswErrorOwner_Pile,  eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "数据库存储错误"},
    {eErr_MeterCalcErr,       eAswErrorOwner_Gun,   eAswErrorLevel_4, eAswErrorLevel_4, eAswErrorClear_External,   1,     0,   "电能表计量故障"},
    {eErr_ChgStartTimeout,    eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "启动超时"},
    {eErr_DiodeStop,          eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "不存在二极管"},
    {eSrc_LittleCurr,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "小电流停止"},
    {eSrc_S2BreakOff,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "S2断开主动停止"},
    
    {eSrc_AppStop,            eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "远程停止"},
    {eSrc_MannulStop,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "人工停止"},
    {eSrc_CardStop,           eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "刷卡停止"},
    {eSrc_InsuffBalance,      eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "余额不足停止"},  
    {eSrc_StopbyMoney,        eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "按金额停止"},  
    {eSrc_StopbyTime,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "按时间停止"},  
    {eSrc_StopbyEnergy,       eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "按电量停止"},  
    {eErr_GunDisConn,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "拔枪停止"},
    {eErr_CPBreakOff,         eAswErrorOwner_Gun,   eAswErrorLevel_1, eAswErrorLevel_1, eAswErrorClear_External,   1,     0,   "CP断线"},
    {eErr_NetNoSIMErr,        eAswErrorOwner_Pile,  eAswErrorLevel_2, eAswErrorLevel_2, eAswErrorClear_External,   1,     0,   "未检测到SIM卡"},  
    {eErr_PlatformOffline,    eAswErrorOwner_Pile,  eAswErrorLevel_2, eAswErrorLevel_2, eAswErrorClear_External,   1,     0,   "运营平台通信异常"}, 
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void AswErrHandleCfg_NoticeCallBack(uint8_t port, AswErrorType_Enum errType, uint8_t flag, AswErrorLevel_Enum errLevel)
{
    if (flag == TRUE)
    {
        if (errLevel == eAswErrorLevel_5)
        {
            CddCP_SetCriticalErrNotice(port);
        }

        if (errLevel == eAswErrorLevel_4 || errLevel == eAswErrorLevel_5 || errLevel == eAswErrorLevel_1)
        {
            AswCharge_SetStopReason(port, errType);
        }

        if (errLevel >= 2)
        {
            SSSnapshot_InsertErrorLog(port, AswErrHandle_GetErrdesc(errType), flag);
        }
    }
}






  


