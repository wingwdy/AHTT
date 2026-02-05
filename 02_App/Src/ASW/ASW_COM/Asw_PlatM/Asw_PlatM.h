/******************************************************************************
* File Name          : template.h
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
#ifndef ASW_PLATM_H_
#define ASW_PLATM_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "MS_Nvm.h"
#include "Asw_Monitor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eAswPlatType_GN,
    eAswPlatType_YKC21,    
    eAswPlatType_XDT,
    eAswPlatType_Count,
}AswPlatType_Enum;

typedef enum
{
    eAswPlatCardType_GN,
    eAswPlatCardType_YKC21,    
    eAswPlatCardType_XDT,
    eAswPlatCardType_Count,
}AswPlatCardType_Enum;


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t AswPlatM_SetPileDn(char *pPileDn, uint8_t len);
uint8_t AswPlatM_SetFixPileDn(char *pFixPileDn, uint8_t len);
uint8_t AswPlatM_SetPlatMainIpPort(char *pIp, uint8_t ipLen, uint16_t port);
uint8_t AswPlatM_SetPlatMainPort(uint16_t port);
uint8_t AswPlatM_SetPlatType(char *platName);
uint8_t AswPlatM_SetPlatCardType(char *platCardName);
AswPlatType_Enum AswPlatM_GetPlatType(void);
uint8_t AswPlatM_SwipCardCharge(uint8_t port);
void AswPlatM_PrintAllConfigInfo(void);

void AswPlatM_DefaultPlatParam(void *param);
MSNvmPlatParam_Struct * AswPlatM_GetPlatParamPtr(void);
void AswPlatM_InitMemory(void);
void AswPlatM_MainFunction(void);

void AswPlatM_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason);
void AswPlatM_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pBillMode);
#endif




















