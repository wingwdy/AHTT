/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_PlatM.h"
#include "Asw_PlatMConfig.h"
#include "Asw_IotProtoGNM.h"

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
const AswPlatMProtocolDescriptor_Struct c_stAswPlatMProtocolDescriptorTable[eAswPlatType_Count] =
{
    [eAswPlatType_GN] =
    {
        .pName = "gn",
        .cProtoMeaning = "公牛",
        .eSocketType = eCddNetMSocketType_TCP,
        .pFuncFillLinkPara = IotGN_FillLinkPara,
        .pFuncInit = IotGN_InitMemory,
        .pMainFunction = IotGN_MainFunction,
        .pFuncTransformBillMode = IotGN_TransformBillMode,
        .pFuncPackChargeRecord = IotGN_PackChargeRecord,
    },

    [eAswPlatType_YKC21] =
    {
        .pName = "ykc2.1",
        .cProtoMeaning = "云快充2.1",
        .eSocketType = eCddNetMSocketType_TCP,
        .pFuncFillLinkPara = NULL,
        .pFuncInit = NULL,
        .pMainFunction = NULL,
    },

    [eAswPlatType_XDT] =
    {
        .pName = "lxxdt",
        .cProtoMeaning = "朗新新电途",
        .eSocketType = eCddNetMSocketType_MQTT,
        .pFuncFillLinkPara = NULL,
        .pFuncInit = NULL,
        .pMainFunction = NULL,
    },
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/




/*******************************************************************************
*    Function Source Code
*******************************************************************************/
























