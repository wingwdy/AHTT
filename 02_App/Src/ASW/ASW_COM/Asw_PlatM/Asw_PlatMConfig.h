/******************************************************************************
* File Name          : template_Config.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef ASW_PLATM_CONFIG_H_
#define ASW_PLATM_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Cdd_NetM.h"
#include "DS_LogM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/

#define ASWPLATM_CFG_LogPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_PlatM, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/

typedef struct
{
    char *pName;
    char *cProtoMeaning;
    CddNetMSocketType_Enum eSocketType;
    void (*pFuncFillLinkPara)(CddNetMSocketPara_Union *pLinkPara);
    void (*pFuncInit)(void);
    void (*pMainFunction)(void);

}AswPlatMProtocolDescriptor_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const AswPlatMProtocolDescriptor_Struct c_stAswPlatMProtocolDescriptorTable[eAswPlatType_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif






















