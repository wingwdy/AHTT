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
#ifndef SS_SNAPSHOT_H_
#define SS_SNAPSHOT_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eSSSnapshotItemType_RunningLog,
    eSSSnapshotItemType_ErrorLog,
}SSSnapshotItemType_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void SSSnapshot_InitMemory(void);
void SSSnapshot_MainFunction(void);
void SSSnapshot_InsertErrorItem(uint8_t port, char *pErrorInfo,  uint8_t flag);
uint8_t SSSnapshot_ReadItemByTime(SSSnapshotItemType_Enum itemType, uint8_t *buf, uint16_t buffLen, uint32_t time);
uint32_t SSSnapshot_ReadItemCount(SSSnapshotItemType_Enum itemType);
#endif /* SS_SNAPSHOT_H_ */



