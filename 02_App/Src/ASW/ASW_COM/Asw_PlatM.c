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
#include "MS_Nvm.h"
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "Asw_PlatMConfig.h"
#include "FrameQueue.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    MSNvmPlatParam_Struct stPlatParam;

}AswPlatMCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswPlatMCtx_Struct g_stAswPlatMCtx = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static AswPlatMProtocolDescriptor_Struct *AswPlatM_GetProtocolDescriptor(void)
{
    AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    return pProtocolDescriptor;
}

MSNvmPlatParam_Struct * AswPlatM_GetPlatParamPtr(void)
{
    return &g_stAswPlatMCtx.stPlatParam;
}

void AswPlatM_DefaultPlatParam(void *param)
{
    MSNvmPlatParam_Struct *pPlatParam = (MSNvmPlatParam_Struct *)param;

    memset(pPlatParam, 0x00, sizeof(MSNvmPlatParam_Struct));

    pPlatParam->platMainType = eAswPlatType_GN;
    strcpy(pPlatParam->platMainIp, "pile.gongniu.cn");
    pPlatParam->platMainPort = 5455;

    strcpy(pPlatParam->platAuxiliaryIp, "pmgmt.gongniu.cn");
    pPlatParam->platAuxiliaryPort = 45113;
}

void AswPlatM_InitMemory(void)
{
    AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;
    CddNetMSocketPara_Union stSocketPara = { 0 };
    FrameQueueType_Enum eFrame;

    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam, sizeof(MSNvmPlatParam_Struct)) != eGlobalRet_OK)
    {
        AswPlatM_DefaultPlatParam(pParam);
    }

    /* 注册运营平台链接 */
    pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    if (pProtocolDescriptor->pFuncInit != NULL)
    {
        pProtocolDescriptor->pFuncInit();
    }

    if (pProtocolDescriptor->pFuncFillLinkPara != NULL)
    {
        pProtocolDescriptor->pFuncFillLinkPara(&stSocketPara);
    }

    CddNetM_CreatLink(pProtocolDescriptor->eSocketType, stSocketPara, eCddNetMPlatType_O);
}

void AswPlatM_MainFunction(void)
{  







}























