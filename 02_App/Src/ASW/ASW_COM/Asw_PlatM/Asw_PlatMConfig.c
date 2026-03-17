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
#include "Asw_IotProtoOMM.h"
#include "Asw_IotProtoYKC21M.h"
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
        .pFuncSwipCardCharge = IotGN_SwipCardCharge,
        .pFuncTransformChargeRecord = IotGN_TransformChargeRecord,
    },

    [eAswPlatType_YKC21] =
    {
        .pName = "ykc2.1",
        .cProtoMeaning = "云快充2.1",
        .eSocketType = eCddNetMSocketType_TCP,
        .pFuncFillLinkPara = IotYKC21_FillLinkPara,
        .pFuncInit = IotYKC21_InitMemory,
        .pMainFunction = IotYKC21_MainFunction,
        .pFuncTransformBillMode = IotYKC21_TransformBillMode,
        .pFuncPackChargeRecord = IotYKC21_PackChargeRecord,
        .pFuncSwipCardCharge = IotYKC21_SwipCardCharge,
        .pFuncTransformChargeRecord = IotYKC21_TransformChargeRecord,
        .pFuncSetToken = IotYKC21_SetToken,
        .pFuncSetCipherKey = IotYKC21_SetRsaPublicKey,
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

const AswPlatMProtocolDescriptor_Struct c_stAswOMProtocolDescriptor = 
{
    .pName = "om",
    .cProtoMeaning = "运维平台",
    .eSocketType = eCddNetMSocketType_TCP,
    .pFuncFillLinkPara = IotOM_FillLinkPara,
    .pFuncInit = IotOM_InitMemory,
    .pMainFunction = IotOM_MainFunction,
};

const AswPlatCardDescriptor_Struct c_stAswPlatMCardDescriptorTable[eAswPlatCardType_Count] =
{
    [eAswPlatCardType_GN] =
    {
        .pName = "gn",
        .cMeaning = "公牛卡",
        .cardType = eCddCardType_BullCard,
    },

    [eAswPlatCardType_YKC21] =
    {
        .pName = "ykc2.1",
        .cMeaning = "通用卡",
        .cardType = eCddCardType_UUID,
    },

    [eAswPlatCardType_XDT] =
    {
        .pName = "lxxdt",
        .cMeaning = "公牛卡",
        .cardType = eCddCardType_BullCard,
    }
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/




/*******************************************************************************
*    Function Source Code
*******************************************************************************/
























