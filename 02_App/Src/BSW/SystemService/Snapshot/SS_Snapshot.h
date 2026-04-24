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
#include "Cdd_NetM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eSnapshotItemReadSrc_Null,
    eSnapshotItemReadSrc_Remote,
    eSnapshotItemReadSrc_Local,    
}SSSnapshotItemReadSrc_Enum;

typedef enum
{
    eSSSnapshotItemType_RunningLog,
    eSSSnapshotItemType_ErrorLog,
    eSSSnapshotItemType_OmOrderRecord,
    eSSSnapshotItemType_Count,
}SSSnapshotItemType_Enum;

typedef enum                                                                                                                                 
{ 
    eSSSnapshotItemRead_Idle = 0,
    eSSSnapshotItemRead_ErrLog,
    eSSSnapshotItemRead_WaitErrLog,
    eSSSnapshotItemRead_RunLog,
    eSSSnapshotItemRead_WaitRunLog,
    eSSSnapshotItemRead_OmOrderRecord,
    eSSSnapshotItemRead_WaitOmOrderRecord,
} SSSnapshotItemRead_Enum;
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
void SSSnapshot_InsertRunningLog(const char *buf, uint16_t len);
void SSSnapshot_InsertErrorLog(uint8_t port, char *pErrorInfo,  uint8_t flag);
uint8_t SSSnapshot_ReadItem(uint8_t *pOutbuf, uint16_t bufSize, uint16_t *pOutLen);
GlobalRet_Enum SSSnapshot_StartReadItem(SSSnapshotItemType_Enum eItemType);
void SSSnapshot_StopReadItem(void);
void SSSnapshot_ExportItem(SSSnapshotItemType_Enum itemType, SSSnapshotItemReadSrc_Enum eReadSrc);
uint8_t SSSnapshot_ExportAllItems(CddNetMSocketPara_Union *pNetPara);
uint8_t SSSnapshot_PreviewReadItem(uint16_t bufSize, uint16_t *pOutLen);
void SSSnapshot_FlushRunningLog(void);

#endif /* SS_SNAPSHOT_H_ */



