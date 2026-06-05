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
#include "Asw_IotProtoXDTM.h"
#include "Asw_IotProtoAPM.h"
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
        .pFuncSetPrivateParam = NULL,
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
        .pFuncSetPrivateParam = IotYKC21_SetPrivateParam,
        
        .pFuncSetToken = IotYKC21_SetToken,
        .pFuncSetCipherKey = IotYKC21_SetRsaPublicKey,
        .pFuncGetToken = IotYKC21_GetToken,
        .pFuncGetCipherKey = IotYKC21_GetRsaPublicKey,
    },

    [eAswPlatType_XDT] =
    {
        .pName = "lxxdt",
        .cProtoMeaning = "朗新新电途",
        .eSocketType = eCddNetMSocketType_MQTT,
        .pFuncFillLinkPara = IotXDT_FillLinkPara,
        .pFuncInit = IotXDT_InitMemory,
        .pMainFunction = IotXDT_MainFunction,
        .pFuncTransformBillMode = IotXDT_TransformBillMode,
        .pFuncPackChargeRecord = IotXDT_PackChargeRecord,
        .pFuncSwipCardCharge = IotXDT_SwipCardCharge,
        .pFuncTransformChargeRecord = IotXDT_TransformChargeRecord,
        .pFuncSetPrivateParam = IotXDT_SetPrivateParam,

        .pFuncSetProductKey = IotXDT_SetProductKey,
        .pFuncSetProductSecret = IotXDT_SetProductSecret,
        .pFuncSetDevOperator = IotXDT_SetDevOperator,
        .pFuncGetProductKey = IotXDT_GetProductKey,
        .pFuncGetProductSecret = IotXDT_GetProductSecret,
        .pFuncGetDevOperator = IotXDT_GetDevOperator,
    },

    [eAswPlatType_GNP] =
    {
        .pName = "gn+",
        .cProtoMeaning = "公牛+",
        .eSocketType = eCddNetMSocketType_TCP,
        .pFuncFillLinkPara = IotGN_FillLinkPara,
        .pFuncInit = IotGN_InitMemory,
        .pMainFunction = IotGN_MainFunction,
        .pFuncTransformBillMode = IotGN_TransformBillMode,
        .pFuncPackChargeRecord = IotGN_PackChargeRecord,
        .pFuncSwipCardCharge = IotGN_SwipCardCharge,
        .pFuncTransformChargeRecord = IotGN_TransformChargeRecord,
        .pFuncSetPrivateParam = NULL,
    },

    [eAswPlatType_AP] =
    {
        .pName = "ap",
        .cProtoMeaning = "安培",
        .eSocketType = eCddNetMSocketType_TCP,
        .pFuncFillLinkPara = IotAP_FillLinkPara,
        .pFuncInit = IotAP_InitMemory,
        .pMainFunction = IotAP_MainFunction,
        .pFuncTransformBillMode = IotAP_TransformBillMode,
        .pFuncPackChargeRecord = IotAP_PackChargeRecord,
        .pFuncSwipCardCharge = IotAP_SwipCardCharge,
        .pFuncTransformChargeRecord = IotAP_TransformChargeRecord,
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
        .cMeaning = "通用卡",
        .cardType = eCddCardType_UUID,
    },

    [eAswPlatCardType_GNP] =
    {
        .pName = "gn+",
        .cMeaning = "通用卡",
        .cardType = eCddCardType_UUID,
    },

    [eAswPlatCardType_AP] =
    {
        .pName = "ap",
        .cMeaning = "安培卡",
        .cardType = eCddCardType_UUID,
    },
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/




/*******************************************************************************
*    Function Source Code
*******************************************************************************/
























