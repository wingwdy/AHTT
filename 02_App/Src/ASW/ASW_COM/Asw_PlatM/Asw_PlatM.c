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
static const AswPlatMProtocolDescriptor_Struct *AswPlatM_GetOMProtocolDescriptor(void);
static const AswPlatCardDescriptor_Struct *AswPlatM_GetCardDescriptor(void);
static void AswPlatM_PlatPrivateParamDefaultHandle(MSNvmPlatPrivateParam_Union *pPrivateParam);



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

static void AswPlatM_PlatPrivateParamDefaultHandle(MSNvmPlatPrivateParam_Union *pPrivateParam)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;

    if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetPrivateParam != NULL)
    {
        c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetPrivateParam(pPrivateParam);
    }
}

void AswPlatM_PrintAllConfigInfo(void)
{
    const AswPlatCardDescriptor_Struct * pCardDescriptor = AswPlatM_GetCardDescriptor();
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;
    char tempStr[128 + 1] = { 0 };
    uint8_t tempLen = 0;

    ASWPLATM_CFG_DebugPrint("---------------------------------配置信息------------------------------------\r\n");
    ASWPLATM_CFG_DebugPrint("平台编码：%s\r\n", pParam->platPileDn);
    ASWPLATM_CFG_DebugPrint("资产编码：%s\r\n", pParam->fixPileDn);
    ASWPLATM_CFG_DebugPrint("软件版本：(%s) %s_%s\r\n", APP_SW_VERSION_TYPE, APP_SW_VERSION_STRING, APP_SW_VERSION_DATE);
    ASWPLATM_CFG_DebugPrint("硬件信息：%s\r\n", HW_VERSION_INFO);
    ASWPLATM_CFG_DebugPrint("是否厂内模式：%s\r\n", (TRUE == CddModeM_IsFactoryMode()) ? "是" : "否");
    ASWPLATM_CFG_DebugPrint("是否国标模式：%s\r\n", (TRUE == CddModeM_IsGBMode()) ? "是" : "否");
    ASWPLATM_CFG_DebugPrint("平台类型：%s\r\n", pProtocolDescriptor->pName);
    ASWPLATM_CFG_DebugPrint("充电卡类型：%s\r\n", pCardDescriptor->pName);
    ASWPLATM_CFG_DebugPrint("sim卡类型：%s\r\n", (pParam->dedicatedNetSimFlag == 0) ? "公网卡" : "专网卡");
    ASWPLATM_CFG_DebugPrint("运营平台IP端口：%s, %d\r\n", pParam->platMainIp, pParam->platMainPort);
    ASWPLATM_CFG_DebugPrint("运维平台IP端口：%s, %d， 使能状态：%s\r\n", pParam->platAuxiliaryIp, pParam->platAuxiliaryPort, 
        pParam->AuxiliaryPlatDisableFlag == 0 ? "使能" : "禁用");

    if (pProtocolDescriptor->pFuncGetDevOperator != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetDevOperator(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("设备运营商：%s\r\n", tempStr);
    }

    if (pProtocolDescriptor->pFuncGetProductKey != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetProductKey(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("产品密钥：%s\r\n", tempStr);
    }

    if (pProtocolDescriptor->pFuncGetProductSecret != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetProductSecret(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("产品密码：%s\r\n", tempStr);
    }

    if (pProtocolDescriptor->pFuncGetToken != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetToken(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("Token：%s\r\n", tempStr);
    }

    if (pProtocolDescriptor->pFuncGetCipherKey != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetCipherKey(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("加密密钥：%s\r\n", tempStr);
    }

    if (pProtocolDescriptor->pFuncGetIv != NULL )
    {
        memset(tempStr, 0x00, sizeof(tempStr));
        pProtocolDescriptor->pFuncGetIv(tempStr, &tempLen);
        ASWPLATM_CFG_DebugPrint("初始向量：%s\r\n", tempStr);
    }

    ASWPLATM_CFG_DebugPrint("----------------------------------------------------------------------------\r\n");
}

uint8_t AswPlatM_SetPileDn(char *pPileDn, uint8_t len)
{
    uint8_t ret = FALSE;

    if (len < MSNVM_PILE_DN_LEN)
    {
        if (len != strlen(g_stAswPlatMCtx.stPlatParam.platPileDn) || memcmp(pPileDn, g_stAswPlatMCtx.stPlatParam.platPileDn, len) != 0)
        {
            ASWPLATM_CFG_DebugPrint("桩平台编码变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platPileDn, pPileDn);
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
        if (len != strlen(g_stAswPlatMCtx.stPlatParam.fixPileDn) || memcmp(pFixPileDn, g_stAswPlatMCtx.stPlatParam.fixPileDn, len) != 0)
        {
            ASWPLATM_CFG_DebugPrint("桩固定编码变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.fixPileDn, pFixPileDn);
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
        if (ipLen != strlen(g_stAswPlatMCtx.stPlatParam.platMainIp) || memcmp(pIp, g_stAswPlatMCtx.stPlatParam.platMainIp, ipLen) != 0)
        {
            ASWPLATM_CFG_DebugPrint("运营平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platMainIp, pIp);

            memset(g_stAswPlatMCtx.stPlatParam.platMainIp, 0x00, CDD_NETM_CFG_IP_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.platMainIp, pIp, ipLen);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));            
        }

        AswPlatM_SetPlatMainPort(port);
        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatMainIp(char *pIp, uint8_t ipLen)
{
    uint8_t ret = FALSE;

    if (ipLen < CDD_NETM_CFG_IP_LEN)
    {
        if (ipLen != strlen(g_stAswPlatMCtx.stPlatParam.platMainIp) || memcmp(pIp, g_stAswPlatMCtx.stPlatParam.platMainIp, ipLen) != 0)
        {
            ASWPLATM_CFG_DebugPrint("运营平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platMainIp, pIp);

            memset(g_stAswPlatMCtx.stPlatParam.platMainIp, 0x00, CDD_NETM_CFG_IP_LEN);
            memcpy(g_stAswPlatMCtx.stPlatParam.platMainIp, pIp, ipLen);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));            
        }

        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetPlatMainPort(uint16_t port)
{
    if (g_stAswPlatMCtx.stPlatParam.platMainPort != port)
    {
        ASWPLATM_CFG_DebugPrint("运营平台port变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.platMainPort, port);
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
        if (ipLen != strlen(g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp) || memcmp(pIp, g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp, ipLen) != 0)
        {
            ASWPLATM_CFG_DebugPrint("运维平台IP变化：[\"%s\"]-->[\"%s\"]\r\n", g_stAswPlatMCtx.stPlatParam.platAuxiliaryIp, pIp);

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
        ASWPLATM_CFG_DebugPrint("运维平台port变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.platAuxiliaryPort, port);
        g_stAswPlatMCtx.stPlatParam.platAuxiliaryPort = port;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
    }

    return TRUE;
}

uint8_t AswPlatM_SetOmPlatEnable(uint8_t enable)
{
    uint8_t ret = FALSE;

    if (enable == 0 || enable == 1)
    {
        if (g_stAswPlatMCtx.stPlatParam.AuxiliaryPlatDisableFlag != enable)
        {
            ASWPLATM_CFG_DebugPrint("运维平台禁用状态变化：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.AuxiliaryPlatDisableFlag, enable);
            g_stAswPlatMCtx.stPlatParam.AuxiliaryPlatDisableFlag = enable;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
        }

        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_SetSimNet(uint8_t simNet)
{
    uint8_t ret = FALSE;

    if (simNet == 0 || simNet == 1)
    {
        if (g_stAswPlatMCtx.stPlatParam.dedicatedNetSimFlag != simNet)
        {
            ASWPLATM_CFG_DebugPrint("sim卡：[%d]-->[%d]\r\n", g_stAswPlatMCtx.stPlatParam.dedicatedNetSimFlag, simNet);
            g_stAswPlatMCtx.stPlatParam.dedicatedNetSimFlag = simNet;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));
        }

        ret = TRUE;
    }

    return ret;
}


uint8_t AswPlatM_SetPlatType(char *platName, uint8_t platNameLen)
{
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = NULL;
    uint8_t index = 0;
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    for (index = 0; index < eAswPlatType_Count; index++)
    {
        pProtocolDescriptor = &c_stAswPlatMProtocolDescriptorTable[index];

        if (memcmp(platName, pProtocolDescriptor->pName, platNameLen) == 0)
        {
            if (index != g_stAswPlatMCtx.stPlatParam.platMainType)
            {
                ASWPLATM_CFG_DebugPrint("平台协议变化：[%s]-->[%s]\r\n", 
                    c_stAswPlatMProtocolDescriptorTable[currentPlatType].cProtoMeaning, pProtocolDescriptor->cProtoMeaning);
                g_stAswPlatMCtx.stPlatParam.platMainType = index;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)&g_stAswPlatMCtx.stPlatParam, sizeof(MSNvmPlatParam_Struct));

                MSNvm_SetDefaultParaBlock(eMSNvmBlockID_Gun0Qrcode);
                MSNvm_SetDefaultParaBlock(eMSNvmBlockID_Gun0OrderInfo);
                MSNvm_SetDefaultParaBlock(eMSNvmBlockID_PlatPrivateParam);
            }

            ret = TRUE;
            break;
        }
    }

    return ret;
}

uint8_t AswPlatM_SetPlatCardType(char *platCardName, uint8_t cardNameLen)
{
    const AswPlatCardDescriptor_Struct *pPlatCardDescriptor = NULL;
    uint8_t index = 0;
    uint8_t currentPlatCardType = g_stAswPlatMCtx.stPlatParam.platMainCardType;
    uint8_t ret = FALSE;

    for (index = 0; index < eAswPlatCardType_Count; index++)
    {
        pPlatCardDescriptor = &c_stAswPlatMCardDescriptorTable[index];

        if (memcmp(platCardName, pPlatCardDescriptor->pName, cardNameLen) == 0)
        {
            if (index != g_stAswPlatMCtx.stPlatParam.platMainCardType)
            {
                ASWPLATM_CFG_DebugPrint("卡类型变化：[%s]-->[%s]\r\n", 
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

uint8_t AswPlatM_GetPlatName(char *pPlatName, uint8_t *pLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (currentPlatType < eAswPlatType_Count && pPlatName != NULL && pLen != NULL)
    {
        *pLen = strlen(c_stAswPlatMProtocolDescriptorTable[currentPlatType].pName);
        strcpy(pPlatName, c_stAswPlatMProtocolDescriptorTable[currentPlatType].pName);
        ret = TRUE;
    }

    return ret;
}

uint8_t AswPlatM_GetCardName(char *pCardName, uint8_t *pLen)
{
    uint8_t currentPlatCardType = g_stAswPlatMCtx.stPlatParam.platMainCardType;
    uint8_t ret = FALSE;

    if (currentPlatCardType < eAswPlatCardType_Count && pCardName != NULL && pLen != NULL)
    {
        *pLen = strlen(c_stAswPlatMCardDescriptorTable[currentPlatCardType].pName);
        strcpy(pCardName, c_stAswPlatMCardDescriptorTable[currentPlatCardType].pName);
        ret = TRUE;
    }

    return ret;
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

uint8_t AswPlatM_SetDevOperator(char *devOperator, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (devOperator != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetDevOperator != NULL)
        {
            c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetDevOperator(devOperator, len);
            ret = TRUE;
        }
    }

    return ret;
}

uint8_t AswPlatM_GetDevOperator(char *pDevOperator, uint8_t *pOutLen) 
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pDevOperator != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetDevOperator != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetDevOperator(pDevOperator, pOutLen);
        }
    }

    return ret;
}

uint8_t AswPlatM_SetProductKey(char *pkey, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pkey != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetProductKey != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetProductKey(pkey, len);
        }
    }

    return ret;
}

uint8_t AswPlatM_GetProductKey(char *pKey, uint8_t *pOutLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pKey != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetProductKey != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetProductKey(pKey, pOutLen);
        }
    }

    return ret;
}

uint8_t AswPlatM_SetProductSecret(char *pSecret, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pSecret != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetProductSecret != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetProductSecret(pSecret, len);
        }
    }

    return ret;
}

uint8_t AswPlatM_GetProductSecret(char *pSecret, uint8_t *pOutLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pSecret != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetProductSecret != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetProductSecret(pSecret, pOutLen);
        }
    }

    return ret;
}

uint8_t AswPlatM_SetToken(char *pToken, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pToken != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetToken != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetToken(pToken, len);
        }
    }

    return ret;
}

uint8_t AswPlatM_GetToken(char *pToken, uint8_t *pOutLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pToken != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetToken != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetToken(pToken, pOutLen);
        }
    }

    return ret;
}

uint8_t AswPlatM_SetCipherKey(char *pkey, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pkey != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetCipherKey != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetCipherKey(pkey, len);
        }
    }

    return ret;
}

uint8_t AswPlatM_GetCipherKey(char *pKey, uint8_t *pOutLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pKey != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetCipherKey != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetCipherKey(pKey, pOutLen);
        }
    }

    return ret;
}

uint8_t AswPlatM_SetIv(char *pIv, uint8_t len)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pIv != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetIv != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncSetIv(pIv, len);
        }
    }

    return ret;
}

uint8_t AswPlatM_GetIv(char *pIv, uint8_t *pOutLen)
{
    uint8_t currentPlatType = g_stAswPlatMCtx.stPlatParam.platMainType;
    uint8_t ret = FALSE;

    if (pIv != NULL && pOutLen != NULL)
    {
        if (c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetIv != NULL)
        {
            ret = c_stAswPlatMProtocolDescriptorTable[currentPlatType].pFuncGetIv(pIv, pOutLen);
        }
    }

    return ret;
}

void AswPlatM_DefaultPlatParam(void *param)
{
    MSNvmPlatParam_Struct *pPlatParam = (MSNvmPlatParam_Struct *)param;
    MSNvmPlatParam_Struct *pAswPlatMPlatParam = &g_stAswPlatMCtx.stPlatParam;

    memset(pPlatParam, 0x00, sizeof(MSNvmPlatParam_Struct));

    pPlatParam->platMainType = eAswPlatType_XDT;
    pPlatParam->platMainCardType = eAswPlatCardType_XDT;

    strcpy(pPlatParam->platMainIp, "47.114.93.1");
    pPlatParam->platMainPort = 1883;
    strcpy(pPlatParam->platAuxiliaryIp, "pmgmt.gongniu.cn");
    pPlatParam->platAuxiliaryPort = 45113;

    memcpy(pAswPlatMPlatParam, pPlatParam, sizeof(MSNvmPlatParam_Struct));
}

void AswPlatM_DefaultPlatPrivateParam(void *param)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = (MSNvmPlatPrivateParam_Union *)param;
    MSNvmPlatPrivateParam_Union *pAswPlatMPrivateParam = &g_stAswPlatMCtx.stPrivateParam;

    memset(pPrivateParam, 0x00, sizeof(MSNvmPlatPrivateParam_Union));

    AswPlatM_PlatPrivateParamDefaultHandle(pPrivateParam);

    memcpy(pAswPlatMPrivateParam, pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
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

    if ((MSNvm_ReadParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam, sizeof(MSNvmPlatParam_Struct)) != eGlobalRet_OK) ||
        (pParam->platMainType >= eAswPlatType_Count))
    {
        MSNvm_SetDefaultParaBlock(eMSNvmBlockID_PlatParam);
    }

    if (MSNvm_ReadParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union)) != eGlobalRet_OK)
    {
        MSNvm_SetDefaultParaBlock(eMSNvmBlockID_PlatPrivateParam);
    }

    /* 设置SIM卡 */
    CddNetM_SetSimNet(pParam->dedicatedNetSimFlag);

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
    MSNvmPlatParam_Struct *pParam = &g_stAswPlatMCtx.stPlatParam;
    const AswPlatMProtocolDescriptor_Struct *pProtocolDescriptor = AswPlatM_GetProtocolDescriptor();
    const AswPlatMProtocolDescriptor_Struct *pOMProtocolDescriptor = AswPlatM_GetOMProtocolDescriptor();

    if (TRUE != CddModeM_IsFactoryMode())
    {
        if (pProtocolDescriptor->pMainFunction != NULL)
        {
            pProtocolDescriptor->pMainFunction();
        }
    }

    if (pParam->AuxiliaryPlatDisableFlag == FALSE)
    {
        if (pOMProtocolDescriptor->pMainFunction != NULL)
        {
            pOMProtocolDescriptor->pMainFunction();
        }
    }
}



