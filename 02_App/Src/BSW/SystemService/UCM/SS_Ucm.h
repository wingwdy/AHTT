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
#ifndef SS_UCM_H_
#define SS_UCM_H_


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
    eSSUcmChannelType_FTP,
    eSSUcmChannelType_Count,
}eSSUcmChannelType_Enum;

typedef enum
{
    eSSUcmExcuteMode_WaitIdle,
    eSSUcmExcuteMode_Immediate,
}eSSUcmExcuteMode_Enum;

typedef enum
{
    eSSUcmResult_None,
    eSSUcmResult_Succ,
    eSSUcmResult_GetFileErr,             /* 获取文件失败 */
    eSSUcmResult_HeadErr,                /* 文件头错误 */
    eSSUcmResult_DataRecvInterrupt,      /* 数据接收中断 */
    eSSUcmResult_Timeout,                /* 升级超时 */
    eSSUcmResult_ModuleNoEnoughSpace,    /* 模组空间不足 */
    eSSUcmResult_UnexpectedError,        /* 未知错误 */
}SSUcmResult_Enum;

typedef enum
{
    eSSUcmWorkState_Idle,
    eSSUcmWorkState_WaitIdle,
    eSSUcmWorkState_Connecting,
    eSSUcmWorkState_Downloading,
    eSSUcmWorkState_Finish,
}SSUcmWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t SSUcm_IsUpdating(void);
uint8_t SSUcm_IsOngoging(void);
uint8_t SSUcm_FileDataHandle(uint8_t *data, uint32_t dataLen);
void SSUcm_ReqStartOTA(CddNetMSocketPara_Union *pNetPara, eSSUcmChannelType_Enum eChannelType, 
    eSSUcmExcuteMode_Enum eExcuteMode, uint32_t timeout);
uint8_t SSUcm_GetPackIndex(uint8_t *pPackIndex);
uint8_t SSUcm_GetReadLenAndOffSet(uint16_t *pReadLen, uint32_t* pReadOffset);
void SSUcm_SetResult(SSUcmResult_Enum eResult);
SSUcmWorkState_Enum SSUcm_GetWorkState(void);
SSUcmResult_Enum SSUcm_GetResult(void);
uint8_t SSUcm_CheckUpdateCondition(void);
void SSUcm_InitMemory(void);
void SSUcm_MainFunction(void);
#endif /* SS_UCM_H_ */






















