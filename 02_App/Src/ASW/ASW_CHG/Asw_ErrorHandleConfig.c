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
    {eErr_none,               AswErrorOwner_None,  AswErrorLevel_0, AswErrorLevel_0, AswErrorClear_None,       1,     0,   "无效值"},
    {eErr_CpVoltAbnor,        AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "CP电压异常"},
    {eErr_CpGroundFault,      AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "CP对地短路"},
    {eErr_PEBreakFault,       AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "PE接地故障"},
    {eErr_EmergencyStop,      AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "急停故障"},
    {eErr_InputLineReversed,  AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "火零反接"},
    {eErr_LeakageCurrErr,     AswErrorOwner_Pile,  AswErrorLevel_5, AswErrorLevel_5, AswErrorClear_External,   1,     0,   "漏电流故障"},
    {eErr_ShortCircleErr,     AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "短路故障"},
    {eErr_RCDSelfcheckErr,    AswErrorOwner_Pile,  AswErrorLevel_5, AswErrorLevel_5, AswErrorClear_External,   1,     0,   "RCD自检故障"},
    {eErr_AphaseInputOverVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流A相输入过压"},

    {eErr_BphaseInputOverVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流B相输入过压"},
    {eErr_CphaseInputOverVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流C相输入过压"},
    {eErr_AphaseInputLessVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流A相输入欠压"},
    {eErr_BphaseInputLessVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流B相输入欠压"},
    {eErr_CphaseInputLessVol, AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流C相输入欠压"},
    {eErr_OutputOverCurr,     AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流输出过流"}, 
    {eErr_JcqMaloperation,    AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "交流输出接触器误动拒动"},         
    {eErr_JcqSynechiaFault,   AswErrorOwner_Gun,   AswErrorLevel_5, AswErrorLevel_5, AswErrorClear_External,   1,     0,   "交流输出接触器粘连"},       
    {eErr_HmiCommErr,         AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "人机交互通信故障"},      
    {eErr_ReaderCommErr,      AswErrorOwner_Gun,   AswErrorLevel_2, AswErrorLevel_2, AswErrorClear_External,   1,     0,   "读卡器通信故障"}, 

    {eErr_MeterCommErr,       AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "电表通信故障"}, 
    {eErr_EnvOverTempErr,     AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "环境过温故障"},
    {eErr_GunOverTempErr,     AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "枪过温故障"},
    {eErr_POverTempErr,       AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "插头过温"},
    {eErr_DatabaseErr,        AswErrorOwner_Pile,  AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "数据库存储错误"},
    {eErr_MeterCalcErr,       AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "电能表计量故障"},
    {eErr_ChgStartTimeout,    AswErrorOwner_Gun,   AswErrorLevel_4, AswErrorLevel_4, AswErrorClear_External,   1,     0,   "启动超时"},
    {eErr_DiodeStop,          AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "不存在二极管"},
    {eSrc_LittleCurr,         AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "小电流停止"},
    {eSrc_S2BreakOff,         AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "S2断开主动停止"},
    
    {eSrc_AppStop,            AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "远程停止"},
    {eSrc_CardStop,           AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "刷卡停止"},
    {eSrc_InsuffBalance,      AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "余额不足停止"},  
    {eSrc_StopbyMoney,        AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "按金额停止"},  
    {eSrc_StopbyTime,         AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "按时间停止"},  
    {eSrc_StopbyEnergy,       AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "按电量停止"},  
    {eErr_GunDisConn,         AswErrorOwner_Gun,   AswErrorLevel_1, AswErrorLevel_1, AswErrorClear_External,   1,     0,   "拔枪停止"},  
    {eErr_NetNoSIMErr,        AswErrorOwner_Pile,  AswErrorLevel_2, AswErrorLevel_2, AswErrorClear_External,   1,     0,   "未检测到SIM卡"},  
    {eErr_PlatformOffline,    AswErrorOwner_Pile,  AswErrorLevel_2, AswErrorLevel_2, AswErrorClear_External,   1,     0,   "运营平台通信异常"},  
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

  




















