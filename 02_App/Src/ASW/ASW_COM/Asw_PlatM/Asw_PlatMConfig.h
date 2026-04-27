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
#include "Asw_Monitor.h"
#include "Cdd_CardM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/

#define ASWPLATM_CFG_DebugPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_PlatM, fmt, ##__VA_ARGS__)
#define ASWPLATM_CFG_InfoPrint(fmt, ...)           DSLOGM_Info(DSLogMModule_PlatM, fmt, ##__VA_ARGS__)
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
    void (*pFuncPackChargeRecord)(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
    void (*pFuncTransformBillMode)(uint8_t port, AswMonitorBillMode_Struct *pBillMode);
    uint8_t (*pFuncSwipCardCharge)(uint8_t port);
    void (*pFuncTransformChargeRecord)(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen);

    uint8_t (*pFuncSetDevOperator)(char *pDevOperator, uint8_t len);
    uint8_t (*pFuncGetDevOperator)(char *pDevOperator, uint8_t *pOutLen);
    uint8_t (*pFuncSetProductKey)(char *pkey, uint8_t len);
    uint8_t (*pFuncGetProductKey)(char *pKey, uint8_t *pOutLen);
    uint8_t (*pFuncSetProductSecret)(char *pSecret, uint8_t len);
    uint8_t (*pFuncGetProductSecret)(char *pSecret, uint8_t *pOutLen);
    uint8_t (*pFuncSetToken)(char *pToken, uint8_t len); 
    uint8_t (*pFuncGetToken)(char *pToken, uint8_t *pOutLen);
    uint8_t (*pFuncSetCipherKey)(char *pkey, uint8_t len);
    uint8_t (*pFuncGetCipherKey)(char *pKey, uint8_t *pOutLen);
    uint8_t (*pFuncSetIv)(char *pIv, uint8_t len);
    uint8_t (*pFuncGetIv)(char *pIv, uint8_t *pOutLen);
}AswPlatMProtocolDescriptor_Struct;


typedef struct Asw_PlatMConfig
{
    char *pName;
    char *cMeaning;
    CddCardType_Enum cardType;
}AswPlatCardDescriptor_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const AswPlatMProtocolDescriptor_Struct c_stAswPlatMProtocolDescriptorTable[eAswPlatType_Count];
extern const AswPlatMProtocolDescriptor_Struct c_stAswOMProtocolDescriptor;
extern const AswPlatCardDescriptor_Struct c_stAswPlatMCardDescriptorTable[eAswPlatCardType_Count];

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif






















