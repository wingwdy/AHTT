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
#include "FrameQueue.h"
#include "Cdd_CardM.h"
#include "Cdd_ModeM.h"
#include "Version.h"
#include "Cdd_NetM.h"
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

typedef struct
{
    MSNvmPlatPrivateParam_Union stPrivateParam;
    MSNvmPlatParam_Struct stPlatParam;
}AswPlatMCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswPlatMCtx_Struct g_stAswPlatMCtx = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static const AswPlatMProtocolDescriptor_Struct *AswPlatM_GetProtocolDescriptor(void);



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
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

static const AswPlatMProtocolDescriptor_Struct *AswPlatM_GetOMProtocolDescriptor(void)
{
    return &c_stAswOMProtocolDescriptor;
}

static const AswPlatCardDescriptor_Struct *AswPlatM_GetCardDescriptor(void)
{
    AswPlatCardDescriptor_Struct *pCardDescriptor = NULL;
    AswPlatCardType_Enum ePlatCardType = g_stAswPlatMCtx.stPlatParam.platMainCardType;

    if (ePlatCardType >= eAswPlatCardType_Count)
    {
        ePlatCardType = eAswPlatCardType_GN;
    }

    return &c_stAswPlatMCardDescriptorTable[ePlatCardType];
}

void AswPlatM_PrintAllConfigInfo(void)
{
    const AswPlatCardDescriptor_Struct * pCardDescriptor = AswPlatM_GetCardDescriptor();
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;

    ASWPLATM_CFG_LogPrint("---------------------------------配置信息------------------------------------\r\n");
    ASWPLATM_CFG_LogPrint("平台编码：%s\r\n", pParam->platPileDn);
    ASWPLATM_CFG_LogPrint("资产编码：%s\r\n", pParam->fixPileDn);
    ASWPLATM_CFG_LogPrint("软件版本：%s(%s)\r\n", APP_SW_VERSION_STRING, APP_SW_VERSION_TYPE);
    ASWPLATM_CFG_LogPrint("硬件信息：%s\r\n", HW_VERSION_INFO);
    ASWPLATM_CFG_LogPrint("是否厂内模式：%s\r\n", (TRUE == CddModeM_IsFactoryMode()) ? "是" : "否");
    ASWPLATM_CFG_LogPrint("是否国标模式：%s\r\n", (TRUE == CddModeM_IsGBMode()) ? "是" : "否");
    ASWPLATM_CFG_LogPrint("平台类型：%s\r\n", pProtocolDescriptor->pName);
    ASWPLATM_CFG_LogPrint("卡类型：%s\r\n", pCardDescriptor->pName);
    ASWPLATM_CFG_LogPrint("运营平台IP端口：%s, %d\r\n", pParam->platMainIp, pParam->platMainPort);
    ASWPLATM_CFG_LogPrint("运维平台IP端口：%s, %d\r\n", pParam->platAuxiliaryIp, pParam->platAuxiliaryPort);

    if(0 == strcmp(pProtocolDescriptor->pName, "ykc2.1"))
    {
        IotYKC21_PrintfYKC21KeyAndToken();
    }

    ASWPLATM_CFG_LogPrint("----------------------------------------------------------------------------\r\n");
}

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
            ASWPLATM_CFG_LogPrint("运营平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platMainIp, pIp);

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
        ASWPLATM_CFG_LogPrint("运营平台port变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.platMainPort, port);
        g_stAswPlatMCtx.stPlatParam.platMainPort = port;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
    }

    return TRUE;
}


uint8_t AswPlatM_SetPlatAuxiliaryIpPort(char *pIp, uint8_t ipLen, uint16_t port)
{
    uint8_t ret = FALSE;

    if (ipLen < CDD_NETM_CFG_IP_LEN)
    {
        if (strcmp(pIp, g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp) != 0)
        {
            ASWPLATM_CFG_LogPrint("运维平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp, pIp);

            memset(g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp, 0x00, CDD_NETM_CFG_IP_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp, pIp, ipLen);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));            
        }

        AswPlatM_SetPlatAuxiliaryPort(port);
        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatAuxiliaryPort(uint16_t port)
{
    if (g_stAswPlatMCtx.stPlatParam.platAuxiliaryPort != port)
    {
        ASWPLATM_CFG_LogPrint("运维平台port变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.platAuxiliaryPort, port);
        g_stAswPlatMCtx.stPlatParam.platAuxiliaryPort = port;
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
uint8_t AswPlatM_Setykc21key(char *pykc21key, uint8_t len) 
{
    //128位密钥
   uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
   uint8_t ret = FALSE;
 
   if (currentPlatType != eAswPlatType_YKC21)
    {
         return ret;
    }

    if (len <= MSNVM_PLAT_YKC21_RSAKEYLEN)
    {
        IotYKC21_RfreshYKC21key(pykc21key,len);
        ret = TRUE;
    }

    return ret;
}


uint8_t AswPlatM_Setykc21token(char *pykc21token, uint8_t len)
{
uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
   uint8_t ret = FALSE;
 
   if (currentPlatType != eAswPlatType_YKC21)
    {
         return ret;
    }

    if (len <= MSNVM_PLAT_YKC21_TOKENLEN)
    {
        IotYKC21_RfreshYKC21token(pykc21token,len);
        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatCardType(char *platCardName)
{
    const AswPlatCardDescriptor_Struct *pPlatCardDescriptor = NULL;
    uint8_t index = 0;
    uint8_t currentPlatCardType = g_stAswPlatMCtx.stPlatParam.platMainCardType;
    uint8_t ret = FALSE;

    for (index = 0; index < eAswPlatCardType_Count; index++)
    {
        pPlatCardDescriptor = &c_stAswPlatMCardDescriptorTable[index];

        if (strcmp(platCardName, pPlatCardDescriptor->pName) == 0)
        {
            if (index != g_stAswPlatMCtx.stPlatParam.platMainCardType)
            {
                ASWPLATM_CFG_LogPrint("卡类型变化：[%s]-->[%s]\r\n", 
                    c_stAswPlatMCardDescriptorTable[currentPlatCardType].cMeaning, pPlatCardDescriptor->cMeaning);
                g_stAswPlatMCtx.stPlatParam.platMainCardType = index;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
            }

            ret = TRUE;
            break;
        }
    }

    return ret;
}

uint16_t AswPlatM_TransformRecord(MSNvmOrderInfo_Struct *pNvmOrderInfo, uint8_t *pOutRecord)
{
    uint16_t dataLen = 0;

    if (pNvmOrderInfo != NULL && pOutRecord != NULL)
    {
        if (pNvmOrderInfo->protocolType < eAswPlatType_Count)
        {
            if (NULL != c_stAswPlatMProtocolDescriptorTable[pNvmOrderInfo->protocolType].pFuncTransformChargeRecord)
            {
                c_stAswPlatMProtocolDescriptorTable[pNvmOrderInfo->protocolType].pFuncTransformChargeRecord(
                    &pNvmOrderInfo->platOrderInfo, pOutRecord, &dataLen);
            }
        }
    }

    return dataLen;
}

AswPlatType_Enum AswPlatM_GetPlatType(void)
{
    return (AswPlatType_Enum)g_stAswPlatMCtx.stPlatParam.platMainType;
}

void AswPlatM_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    if (pProtocolDescriptor->pFuncPackChargeRecord != NULL && pOrderData != NULL)
    {
        pProtocolDescriptor->pFuncPackChargeRecord(port, pOrderData, orderSaveReason);
    }
}

uint8_t AswPlatM_SwipCardCharge(uint8_t port)
{
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();
    uint8_t ret = FALSE;

    if (pProtocolDescriptor->pFuncSwipCardCharge != NULL)
    {
        ret = pProtocolDescriptor->pFuncSwipCardCharge(port);
    }

    return ret;
}


void AswPlatM_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pBillMode)
{
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    if (pProtocolDescriptor->pFuncTransformBillMode != NULL && pBillMode != NULL)
    {
        pProtocolDescriptor->pFuncTransformBillMode(port, pBillMode);
    }
}

MSNvmPlatParam_Struct * AswPlatM_GetPlatParamPtr(void)
{
    return &g_stAswPlatMCtx.stPlatParam;
}

MSNvmPlatPrivateParam_Union *AswPlatM_GetPlatPrivateParamPtr(void)
{
    return &g_stAswPlatMCtx.stPrivateParam;
}

void AswPlatM_DefaultPlatParam(void *param)
{
    MSNvmPlatParam_Struct *pPlatParam = (MSNvmPlatParam_Struct *)param;

    memset(pPlatParam, 0x00, sizeof(MSNvmPlatParam_Struct));

    pPlatParam->platMainType = eAswPlatType_GN;
    pPlatParam->platMainCardType = eAswPlatCardType_GN;

    strcpy(pPlatParam->platMainIp, "pile.gongniu.cn");
    pPlatParam->platMainPort = 5455;

    strcpy(pPlatParam->platAuxiliaryIp, "pmgmt.gongniu.cn");
    pPlatParam->platAuxiliaryPort = 45113;
}

void AswPlatM_DefaultPlatPrivateParam(void *param)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = (MSNvmPlatPrivateParam_Union *)param;

    memset(pPrivateParam, 0x00, sizeof(MSNvmPlatPrivateParam_Union));
}

void AswPlatM_InitMemory(void)
{
    const AswPlatCardDescriptor_Struct *pCardDescriptor = NULL;
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    const AswPlatMProtocolDescriptor_Struct *pOMProtocolDescriptor = AswPlatM_GetOMProtocolDescriptor();
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;
    MSNvmPlatPrivateParam_Union *pPrivateParam = &g_stAswPlatMCtx.stPrivateParam;
    CddNetMSocketPara_Union stSocketPara = { 0 };
    FrameQueueType_Enum eFrame;

    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam, sizeof(MSNvmPlatParam_Struct)) != eGlobalRet_OK)
    {
        AswPlatM_DefaultPlatParam(pParam);
    }

    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union)) != eGlobalRet_OK)
    {
        AswPlatM_DefaultPlatPrivateParam(pPrivateParam);
    }

    pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();

    /* 注册运营平台链接 */
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

    /* 注册运维平台链接 */
    if (pOMProtocolDescriptor != NULL)
    {
        if (pOMProtocolDescriptor->pFuncInit != NULL)
        {
            pOMProtocolDescriptor->pFuncInit();
        }

        if (pOMProtocolDescriptor->pFuncFillLinkPara != NULL)
        {
            pOMProtocolDescriptor->pFuncFillLinkPara(&stSocketPara);
        }

        CddNetM_CreatLink(pOMProtocolDescriptor->eSocketType, stSocketPara, eCddNetMPlatType_OM);
    }

    /* 设置卡类型 */
    pCardDescriptor = AswPlatM_GetCardDescriptor();
    CddCardM_SetCardType(pCardDescriptor->cardType);

    AswPlatM_PrintAllConfigInfo();
}

void AswPlatM_MainFunction(void)
{  
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();
    const AswPlatMProtocolDescriptor_Struct *pOMProtocolDescriptor = AswPlatM_GetOMProtocolDescriptor();

    if (TRUE != CddModeM_IsFactoryMode())
    { 
        if (pProtocolDescriptor->pMainFunction != NULL)
        {
            pProtocolDescriptor->pMainFunction();
        }
    }

    if (pOMProtocolDescriptor->pMainFunction != NULL)
    {
        pOMProtocolDescriptor->pMainFunction();
    }
}

