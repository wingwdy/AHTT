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
#include "FrameQueue.h"
#include "myMalloc.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define FRAME_QUEUE_MAGIC_NUMBER  0x55AA

#define FRAME_QUEUE_DIRECTION_TX  0
#define FRAME_QUEUE_DIRECTION_RX  1


/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t ctrlWord[2];
    uint8_t topicLen[2];
    uint8_t dataLen[2];
}FrameQueueHead_Struct;

typedef struct 
{
    FrameQueueType_Enum frameType;
    uint8_t initFlag;
    uint16_t txLen;
	uint16_t txBufSize;
	uint8_t* pTXBuf;
	
	uint16_t rxLen;
	uint16_t rxBufSize;
	uint8_t* pRXBuf;
}FrameQueueCtrlDCB_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static GlobalRet_Enum FrameQueue_PopTCP(FrameQueueCtrlDCB_Struct *pDCB, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction);
static GlobalRet_Enum FrameQueue_PopMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction);
static GlobalRet_Enum FrameQueue_PushMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction);
static GlobalRet_Enum FrameQueue_PushTCP(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction);
static GlobalRet_Enum FrameQueue_GetLastFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen, uint8_t direction);

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static FrameQueueCtrlDCB_Struct g_stFrameQueueCtrlDCB[FRAME_QUEUE_CHANNEL_COUNT] = {0};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static GlobalRet_Enum FrameQueue_FindFreeChannel(uint8_t *pChannelID)
{
    uint8_t index = 0;
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughChannel;

    for(index = 0; index < FRAME_QUEUE_CHANNEL_COUNT; index++)
    {
        if(g_stFrameQueueCtrlDCB[index].initFlag == FALSE)
        {
            pChannelID[0] = index;
            eRet = eGlobalRet_OK;
            break;
        }
    }

    return eRet;
}

static GlobalRet_Enum FrameQueue_PopMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction)
{
    FrameQueueHead_Struct stHead = {0};
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughData;
    uint8_t *pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;
    uint16_t magicNumber = 0;
    uint16_t topicLength = 0;
    uint16_t dataLength = 0;
    uint8_t *pDataStart = NULL;
    uint16_t remainingBytes = 0;
    
    if (pBufLen[0] >= sizeof(FrameQueueHead_Struct))
    {
        memcpy(&stHead, pOptBuf, sizeof(FrameQueueHead_Struct));
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        topicLength = Common_TwoUint8ToUint16(stHead.topicLen);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        
        if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER &&
            (sizeof(FrameQueueHead_Struct) + topicLength + dataLength) <= pBufLen[0])
        {
            pDataStart = pOptBuf + sizeof(FrameQueueHead_Struct);
            
            if (topicLength > 0)
            {
                memcpy(pTopic, pDataStart, topicLength);
                *pTopicLen = topicLength;
            }
            
            if (dataLength > 0)
            {
                memcpy(pDstData, pDataStart + topicLength, dataLength);
                *pDataSize = dataLength;
            }
            
            remainingBytes = pBufLen[0] - (sizeof(FrameQueueHead_Struct) + topicLength + dataLength);

            if (remainingBytes > 0)
            {
                memmove(pOptBuf, 
                        pOptBuf + sizeof(FrameQueueHead_Struct) + topicLength + dataLength, 
                        remainingBytes);
            }
            
            // 更新缓冲区长度
            *pBufLen = remainingBytes;
            
            eRet = eGlobalRet_OK;
        }
    }
    
    return eRet;
}

static GlobalRet_Enum FrameQueue_PopTCP(FrameQueueCtrlDCB_Struct *pDCB, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction)
{
    FrameQueueHead_Struct stHead = {0};
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughData;
    uint8_t *pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;
    uint16_t magicNumber = 0;
    uint16_t dataLength = 0;
    uint8_t *pDataStart = NULL;
    uint16_t remainingBytes = 0;

    if (pBufLen[0] >= sizeof(FrameQueueHead_Struct))
    {
        memcpy(&stHead, pOptBuf, sizeof(FrameQueueHead_Struct));
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        
        if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && 
            (sizeof(FrameQueueHead_Struct) + dataLength) <= pBufLen[0])
        {

            pDataStart = pOptBuf + sizeof(FrameQueueHead_Struct);
            
            if (pDstData != NULL && pDataSize != NULL && dataLength > 0)
            {
                memcpy(pDstData, pDataStart, dataLength);
                *pDataSize = dataLength;
            }
    
            remainingBytes = pBufLen[0] - (sizeof(FrameQueueHead_Struct) + dataLength);
            if (remainingBytes > 0)
            {
                memmove(pOptBuf, 
                        pOptBuf + sizeof(FrameQueueHead_Struct) + dataLength, 
                        remainingBytes);
            }
            
            pBufLen[0] = remainingBytes;
            eRet = eGlobalRet_OK;
        }
    }
    
    return eRet;
}

static GlobalRet_Enum FrameQueue_PushMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction)
{
    FrameQueueHead_Struct stHead = {0}; 
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughBuf;
    uint16_t ctrlWord = FRAME_QUEUE_MAGIC_NUMBER;
    uint8_t *pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    uint16_t bufSize = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txBufSize : pDCB->rxBufSize;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;

    if (pBufLen[0] + topicLen + dataLen + sizeof(FrameQueueHead_Struct) < bufSize)
    {
        Common_Uint16ToTwoUint8(stHead.ctrlWord, ctrlWord);
        Common_Uint16ToTwoUint8(stHead.topicLen, topicLen);
        Common_Uint16ToTwoUint8(stHead.dataLen, dataLen);

        memcpy(pOptBuf + pBufLen[0], &stHead, sizeof(FrameQueueHead_Struct));
        pBufLen[0] += sizeof(FrameQueueHead_Struct);
        memcpy(pOptBuf + pBufLen[0], pTopic, topicLen);
        pBufLen[0] += topicLen;
        memcpy(pOptBuf + pBufLen[0], pSrcData, dataLen);
        pBufLen[0] += dataLen;
        eRet = eGlobalRet_OK;
    }

    return eRet;
}

static GlobalRet_Enum FrameQueue_PushTCP(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction)
{
    FrameQueueHead_Struct stHead = {0}; 
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughBuf;
    uint16_t ctrlWord = FRAME_QUEUE_MAGIC_NUMBER;
    uint8_t *pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    uint16_t bufSize = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txBufSize : pDCB->rxBufSize;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;

    if (pBufLen[0] + dataLen + sizeof(FrameQueueHead_Struct) < bufSize)
    {
        Common_Uint16ToTwoUint8(stHead.ctrlWord, ctrlWord);
        Common_Uint16ToTwoUint8(stHead.dataLen, dataLen);

        memcpy(pOptBuf + pBufLen[0], &stHead, sizeof(FrameQueueHead_Struct));
        pBufLen[0] += sizeof(FrameQueueHead_Struct);
        memcpy(pOptBuf + pBufLen[0], pSrcData, dataLen);
        pBufLen[0] += dataLen;
        eRet = eGlobalRet_OK;
    }

    return eRet;
}

static GlobalRet_Enum FrameQueue_GetLastFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen, uint8_t direction)
{
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    FrameQueueHead_Struct stHead = {0};
    GlobalRet_Enum eRet = eGlobalRet_NotEnoughData;
    uint8_t *pOptBuf = NULL;
    uint16_t bufLen = 0;
    uint16_t magicNumber = 0;
    uint16_t dataLength = 0;
    uint16_t topicLength = 0;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDataLen != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    bufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txLen : pDCB->rxLen;
    
    if (bufLen >= sizeof(FrameQueueHead_Struct))
    {
        memcpy(&stHead, pOptBuf, sizeof(FrameQueueHead_Struct));
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        
        if (pDCB->frameType == eFrameQueueType_MQTT)
        {
            topicLength = Common_TwoUint8ToUint16(stHead.topicLen);
            if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && 
                (sizeof(FrameQueueHead_Struct) + topicLength + dataLength) <= bufLen)
            {
                *pDataLen = dataLength;
                
                // 如果需要获取Topic信息
                if (pTopicLen != NULL)
                {
                    *pTopicLen = topicLength;
                }

                if (pTopic != NULL)
                {
                    memcpy(pTopic, pOptBuf + sizeof(FrameQueueHead_Struct), topicLength);
                }

                eRet = eGlobalRet_OK;
            }
        }
        else if (pDCB->frameType == eFrameQueueType_TCP)
        {
            if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && 
                (sizeof(FrameQueueHead_Struct) + dataLength) <= bufLen)
            {
                *pDataLen = dataLength;
                
                // TCP模式下没有Topic，如果需要Topic信息则设为0
                if (pTopic != NULL && pTopicLen != NULL)
                {
                    *pTopicLen = 0;
                }
                
                eRet = eGlobalRet_OK;
            }
        }
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_Creat(FrameQueueType_Enum eFrame, uint16_t txBufSize, uint16_t rxBufSize, uint8_t *pChannelID)
{
    GlobalRet_Enum eRet = eGlobalRet_InitFail;
    FrameQueueCtrlDCB_Struct *pDCB = NULL;
    uint8_t newChannel = 0;
    PARA_ASSERT_RET(eFrame < FRAME_QUEUE_CHANNEL_COUNT && txBufSize != 0 && rxBufSize != 0 && pChannelID != NULL, eGlobalRet_ParaInvalid);

    if (eGlobalRet_OK == FrameQueue_FindFreeChannel(&newChannel))
    {
        pDCB = &g_stFrameQueueCtrlDCB[newChannel];

        pDCB->frameType = eFrame;

        pDCB->txLen = 0;
        pDCB->txBufSize = txBufSize;
        pDCB->pTXBuf = (uint8_t *)myCalloc(txBufSize, 1);

        pDCB->rxLen = 0;
        pDCB->rxBufSize = rxBufSize;
        pDCB->pRXBuf = (uint8_t *)myCalloc(rxBufSize, 1);

        if (pDCB->pRXBuf != NULL && pDCB->pTXBuf != NULL)
        {
            pDCB->initFlag = TRUE;
            pChannelID[0] = newChannel;
            eRet = eGlobalRet_OK;
        }
        else
        {
            if (pDCB->pTXBuf != NULL)
            {
                free(pDCB->pTXBuf);
                pDCB->pTXBuf = NULL;
            }

            if (pDCB->pRXBuf != NULL)
            {
                free(pDCB->pRXBuf);
                pDCB->pRXBuf = NULL;
            }

            memset(pDCB, 0, sizeof(FrameQueueCtrlDCB_Struct));
        }
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_Reset(uint8_t channelID)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    pDCB->txLen = 0;
    pDCB->rxLen = 0;
    return eRet;
}

GlobalRet_Enum FrameQueue_PushTx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize)
{ 
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pSrcData != NULL && dataSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || topicLen == 0)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PushMQTT(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_TX);
        }
    }
    else if (pDCB->frameType == eFrameQueueType_TCP)
    {
        if (pTopic != NULL || topicLen != 0)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PushTCP(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_TX);
        }
    }
    else
    {
        eRet = eGlobalRet_NotSupported;
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_PushRx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize)
{ 
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pSrcData != NULL && dataSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || topicLen == 0)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PushMQTT(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_RX);
        }
    }
    else if (pDCB->frameType == eFrameQueueType_TCP)
    {
        if (pTopic != NULL || topicLen != 0)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PushTCP(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_RX);
        }
    }
    else
    {
        eRet = eGlobalRet_NotSupported;
    }

    return eRet;
}


GlobalRet_Enum FrameQueue_PopTx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDstData != NULL && pDataSize != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || pTopicLen == NULL)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PopMQTT(pDCB, pTopic, pTopicLen, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_TX);
        }
    }
    else if (pDCB->frameType == eFrameQueueType_TCP)
    {
        if (pTopic != NULL || pTopicLen != NULL)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PopTCP(pDCB, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_TX);
        }
    }
    else
    {
        eRet = eGlobalRet_NotSupported;
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_PopRx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDstData != NULL && pDataSize != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || pTopicLen == NULL)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PopMQTT(pDCB, pTopic, pTopicLen, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_RX);
        }
    }
    else if (pDCB->frameType == eFrameQueueType_TCP)
    {
        if (pTopic != NULL || pTopicLen != NULL)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PopTCP(pDCB, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_RX);
        }
    }
    else
    {
        eRet = eGlobalRet_NotSupported;
    }

    return eRet;
}
GlobalRet_Enum FrameQueue_GetLastTxFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen)
{
    return FrameQueue_GetLastFrameDataLen(channelID, pDataLen, pTopic, pTopicLen, FRAME_QUEUE_DIRECTION_TX);
}

GlobalRet_Enum FrameQueue_GetLastRxFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen)
{
    return FrameQueue_GetLastFrameDataLen(channelID, pDataLen, pTopic, pTopicLen, FRAME_QUEUE_DIRECTION_RX);
}

GlobalRet_Enum FrameQueue_ProcessRxData(uint8_t channelID, typeFuncDecode pDecodeFunc)
{
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t processedLen = 0;
    FrameQueueHead_Struct *pHead = NULL;
    uint16_t dataLen = 0;
    uint16_t topicLen = 0;
    uint16_t dealLen = 0;  
    uint8_t *pTopicData = NULL;
    uint8_t *pFrameData = NULL;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDecodeFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDCB->rxLen > 0, eGlobalRet_NotEnoughData);

    pHead = (FrameQueueHead_Struct *)pDCB->pRXBuf;

    if (Common_TwoUint8ToUint16(pHead->ctrlWord) != FRAME_QUEUE_MAGIC_NUMBER)
    {
        pDCB->rxLen = 0;
        eRet = eGlobalRet_UnexpectedError;
    }
    else
    { 
        dataLen = Common_TwoUint8ToUint16(pHead->dataLen);
        topicLen = Common_TwoUint8ToUint16(pHead->topicLen);

        if ((sizeof(FrameQueueHead_Struct) + dataLen + topicLen) > pDCB->rxLen)
        {
            pDCB->rxLen = 0;
            eRet = eGlobalRet_UnexpectedError;
        }
        else
        {
            if (pDCB->frameType == eFrameQueueType_MQTT)
            {
                pTopicData = pDCB->pRXBuf + sizeof(FrameQueueHead_Struct);
                pFrameData = pDCB->pRXBuf + sizeof(FrameQueueHead_Struct) + topicLen;
                pDecodeFunc(pFrameData, dataLen, topicLen, pTopicData, &dealLen);
                memmove(pDCB->pRXBuf, 
                        pDCB->pRXBuf + sizeof(FrameQueueHead_Struct) + dataLen + topicLen, 
                        pDCB->rxLen - sizeof(FrameQueueHead_Struct) - dataLen - topicLen);

                pDCB->rxLen -= (sizeof(FrameQueueHead_Struct) + dataLen + topicLen);
            }
            else if (pDCB->frameType == eFrameQueueType_TCP)
            {
                while (dataLen > processedLen)
                {
                    pFrameData = pDCB->pRXBuf + sizeof(FrameQueueHead_Struct) + processedLen;
                    pDecodeFunc(pFrameData, dataLen - processedLen, 0, NULL, &dealLen);
                    processedLen += dealLen;
                }

                memmove(pDCB->pRXBuf, 
                        pDCB->pRXBuf + sizeof(FrameQueueHead_Struct) + dataLen, 
                        pDCB->rxLen - sizeof(FrameQueueHead_Struct) - dataLen);

                pDCB->rxLen -= (sizeof(FrameQueueHead_Struct) + dataLen);
            }
            else
            {
                eRet = eGlobalRet_NotSupported;
            }
        }
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_TransmitTxData(uint8_t channelID, typeFuncTransmit pTransmitFunc, void *userData)
{
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueHead_Struct *pHead = NULL;
    uint16_t dataLen = 0;
    uint16_t topicLen = 0;
    uint8_t *pFrameData = NULL;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pTransmitFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDCB->txLen > 0, eGlobalRet_NotEnoughData);

    pHead = (FrameQueueHead_Struct *)pDCB->pTXBuf;

    if (Common_TwoUint8ToUint16(pHead->ctrlWord) != FRAME_QUEUE_MAGIC_NUMBER)
    {
        pDCB->txLen = 0;
        eRet = eGlobalRet_UnexpectedError;
    }
    else
    { 
        dataLen = Common_TwoUint8ToUint16(pHead->dataLen);
        topicLen = Common_TwoUint8ToUint16(pHead->topicLen);

        if ((sizeof(FrameQueueHead_Struct) + dataLen + topicLen) > pDCB->txLen)
        {
            pDCB->txLen = 0;
            eRet = eGlobalRet_UnexpectedError;
        }
        else
        {
            if (pDCB->frameType == eFrameQueueType_MQTT)
            {
                pFrameData = pDCB->pTXBuf + sizeof(FrameQueueHead_Struct) + topicLen;
                pTransmitFunc(pFrameData, dataLen, userData);

                memmove(pDCB->pTXBuf, 
                        pDCB->pTXBuf + sizeof(FrameQueueHead_Struct) + dataLen + topicLen, 
                        pDCB->txLen - sizeof(FrameQueueHead_Struct) - dataLen - topicLen);

                pDCB->txLen -= (sizeof(FrameQueueHead_Struct) + dataLen + topicLen);
            }
            else if (pDCB->frameType == eFrameQueueType_TCP)
            {
                pFrameData = pDCB->pTXBuf + sizeof(FrameQueueHead_Struct);
                pTransmitFunc(pFrameData, dataLen, userData);

                memmove(pDCB->pTXBuf, 
                        pDCB->pTXBuf + sizeof(FrameQueueHead_Struct) + dataLen, 
                        pDCB->txLen - sizeof(FrameQueueHead_Struct) - dataLen);

                pDCB->txLen -= (sizeof(FrameQueueHead_Struct) + dataLen);
            }
            else
            {
                eRet = eGlobalRet_NotSupported;
            }
        }
    }

    return eRet;
}




