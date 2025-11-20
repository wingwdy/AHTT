/******************************************************************************
* File Name          : DS_LogM.h
* Description        : Code for log manage
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
#ifndef DS_LOGM_H_
#define DS_LOGM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/

#include "Common.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define DSLOGM_Trace(module, fmt, ...)      DSLogM_Output(module, eDSLogOutputLevel_Trace,    "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
#define DSLOGM_Debug(module, fmt, ...)      DSLogM_Output(module, eDSLogOutputLevel_Debug,    "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
#define DSLOGM_Info(module, fmt, ...)       DSLogM_Output(module, eDSLogOutputLevel_Info,     "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
#define DSLOGM_Warn(module, fmt, ...)       DSLogM_Output(module, eDSLogOutputLevel_Warn,     "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
#define DSLOGM_Error(module, fmt, ...)      DSLogM_Output(module, eDSLogOutputLevel_Error,    "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
#define DSLOGM_Critical(module, fmt, ...)   DSLogM_Output(module, eDSLogOutputLevel_Critical, "[%s] "fmt, DSLogM_GetModuleName(module), ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eDSLogOutputLevel_Trace,
    eDSLogOutputLevel_Debug,
    eDSLogOutputLevel_Info,
    eDSLogOutputLevel_Warn,
    eDSLogOutputLevel_Error,
    eDSLogOutputLevel_Critical,   
}DSLogOutputLevel_Enum;

typedef enum
{
    /* ASW */
    DSLogMModule_EVSE,
    DSLogMModule_Charge,

    /*CDD */
    DSLogMModule_CDD,

    /*System Service */
    DSLogMModule_System,


    DSLogMModule_Count,
}DSLogMModule_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void DSLogM_Output(DSLogMModule_Enum eModule, DSLogOutputLevel_Enum eLevel, const char *fmt, ...);
const char* DSLogM_GetModuleName(DSLogMModule_Enum eModule);
void DSLogM_InitMemory(void);
#endif /* DS_LOGM_H_ */























