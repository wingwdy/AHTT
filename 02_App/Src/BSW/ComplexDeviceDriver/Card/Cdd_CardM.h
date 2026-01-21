/******************************************************************************
* File Name          : Cdd_CardM.h
* Description        : Code for Card Manage
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
******************************************************************************/
#ifndef CDD_CARDM_H_
#define CDD_CARDM_H_


/******************************************************************************
*    Include Files
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eCddCardType_UUID = 0,        /* 通用卡(只读UID卡)*/
    eCddCardType_BullCard,        /* 公牛卡*/
	eCddCardType_Count,
}CddCardType_Enum;

typedef enum
{
    CddCardEvent_Null = 0,
    CddCardEvent_CardIdOK,
    CddCardEvent_CardIdError,
	CddCardEvent_HardFault,
}CddCardEvent_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/




/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern void CddCardM_InitMemory(void);
extern void CddCardM_MainFunction(void);
extern GlobalRet_Enum CddCardM_SetCardType(CddCardType_Enum eType);
extern CddCardEvent_Enum CddCardM_GetCardEvent(void);
extern GlobalRet_Enum CddCardM_GetCardUid(uint8_t *pUidOut);
extern GlobalRet_Enum CddCardM_GetCardUserId(uint8_t *pUidOut);


#endif /* CDD_CARDM_H_ */

