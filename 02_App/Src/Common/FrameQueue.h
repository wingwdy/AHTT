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
#ifndef FRAME_QUEUE_H_
#define FRAME_QUEUE_H_



/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define FRAME_QUEUE_CHANNEL_COUNT   2


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eFrameQueueType_TCP,
    eFrameQueueType_MQTT,
    eFrameQueueType_Count,
}FrameQueueType_Enum;



/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef void (*typeFuncDecode)(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen);
typedef void (*typeFuncTransmit)(uint8_t *pData, uint16_t dataLen);

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
GlobalRet_Enum FrameQueue_Reset(uint8_t channelID);
GlobalRet_Enum FrameQueue_Creat(FrameQueueType_Enum eFrame, uint16_t txBufSize, uint16_t rxBufSize, uint8_t *pChannelID);
GlobalRet_Enum FrameQueue_PushTx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize);
GlobalRet_Enum FrameQueue_PushRx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize);
GlobalRet_Enum FrameQueue_PopTx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize);
GlobalRet_Enum FrameQueue_PopRx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize);
GlobalRet_Enum FrameQueue_GetLastTxFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen);
GlobalRet_Enum FrameQueue_GetLastRxFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen);
GlobalRet_Enum FrameQueue_ProcessRxData(uint8_t channelID, typeFuncDecode pDecodeFunc);
GlobalRet_Enum FrameQueue_TransmitTxData(uint8_t channelID, typeFuncTransmit pTransmitFunc);
#endif /* FRAME_QUEUE_H_ */





















