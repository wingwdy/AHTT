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
uint8_t AswPlatM_SetPileDn(char *pPileDn, uint8_t len)
{
    uint8_t ret = FALSE;

    if (len < MSNVM_PILE_DN_LEN)
    {
        if (strcmp(pPileDn, g_stAswPlatMCtx.stPlatParam.platPileDn) != 0)
        {
            ASWPLATM_CFG_LogPrint("桩平台编码变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platPileDn, pPileDn);
            memset(g_stAswPlatMCtx.stPlatParam.platPileDn, 0x00, MSNVM_PILE_DN_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.platPileDn, pPileDn, len);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
        }

        if (strlen(g_stAswPlatMCtx.stPlatParam.fixPileDn) == 0)
        {
            AswPlatM_SetFixPileDn(pPileDn, len);
        }

        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetFixPileDn(char *pFixPileDn, uint8_t len)
{
    uint8_t ret = FALSE;

    if (len < MSNVM_PILE_DN_LEN)
    {
        if (strcmp(pFixPileDn, g_stAswPlatMCtx.stPlatParam.fixPileDn) != 0)
        {
            ASWPLATM_CFG_LogPrint("桩固定编码变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.fixPileDn, pFixPileDn);
            memset(g_stAswPlatMCtx.stPlatParam.fixPileDn, 0x00, MSNVM_PILE_DN_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.fixPileDn, pFixPileDn, len);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
        }

        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatMainIpPort(char *pIp, uint8_t ipLen, uint16_t port)
{
    uint8_t ret = FALSE;

    if (ipLen < CDD_NETM_CFG_IP_LEN)
    {
        if (strcmp(pIp, g_stAswPlatMCtx.stPlatParam.platMainIp) != 0)
        {
            ASWPLATM_CFG_LogPrint("平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platMainIp, pIp);

            memset(g_stAswPlatMCtx.stPlatParam.platMainIp, 0x00, CDD_NETM_CFG_IP_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.platMainIp, pIp, ipLen);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));            
        }

        AswPlatM_SetPlatMainPort(port);
        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatMainPort(uint16_t port)
{
    if (g_stAswPlatMCtx.stPlatParam.platMainPort != port)
    {
        ASWPLATM_CFG_LogPrint("平台port变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.platMainPort, port);
        g_stAswPlatMCtx.stPlatParam.platMainPort = port;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
    }

    return TRUE;
}

uint8_t AswPlatM_SetPlatType(char *platName)
{
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    uint8_t index = 0;
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    for (index = 0; index < eAswPlatType_Count; index++)
    {
        pProtocolDescriptor = &c_stAswPlatMProtocolDescriptorTable[index];

        if (strcmp(platName, pProtocolDescriptor->pName) == 0)
        {
            if (index != g_stAswPlatMCtx.stPlatParam.platMainType)
            {
                ASWPLATM_CFG_LogPrint("平台协议变化：[%s]-->[%s]\r\n", 
                    c_stAswPlatMProtocolDescriptorTable[currentPlatType].cProtoMeaning, pProtocolDescriptor->cProtoMeaning);
                g_stAswPlatMCtx.stPlatParam.platMainType = index;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
            }

            ret = TRUE;
            break;
        }
    }

    return ret;
}
static const AswPlatMProtocolDescriptor_Struct *AswPlatM_GetProtocolDescriptor(void)
{
    AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    AswPlatType_Enum ePlatType = g_stAswPlatMCtx.stPlatParam.platMainType;

    if (ePlatType >= eAswPlatType_Count)
    {
        ePlatType = eAswPlatType_GN;
    }

    return &c_stAswPlatMProtocolDescriptorTable[ePlatType];
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
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;
    CddNetMSocketPara_Union stSocketPara = { 0 };
    FrameQueueType_Enum eFrame;

    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam, sizeof(MSNvmPlatParam_Struct)) != eGlobalRet_OK)
    {
        AswPlatM_DefaultPlatParam(pParam);
    }

    /* 注册运营平台链接 */
    pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    if (pProtocolDescriptor != NULL)
    {
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
}

void AswPlatM_MainFunction(void)
{  
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    if (pProtocolDescriptor->pMainFunction != NULL)
    {
        pProtocolDescriptor->pMainFunction();
    }
}























