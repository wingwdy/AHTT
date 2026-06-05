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
	uint16_t txReadIdx;
	uint16_t txWriteIdx;
	
	uint16_t rxLen;
	uint16_t rxBufSize;
	uint8_t* pRXBuf;
	uint16_t rxReadIdx;
	uint16_t rxWriteIdx;
}FrameQueueCtrlDCB_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static GlobalRet_Enum FrameQueue_PopTCP(FrameQueueCtrlDCB_Struct *pDCB, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction);
static GlobalRet_Enum FrameQueue_PopMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction);
static GlobalRet_Enum FrameQueue_PushMQTT(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction);
static GlobalRet_Enum FrameQueue_PushTCP(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataLen, uint8_t direction);
static GlobalRet_Enum FrameQueue_GetLastFrameDataLen(uint8_t channelID, uint16_t *pDataLen, char *pTopic, uint16_t *pTopicLen, uint8_t direction);
static GlobalRet_Enum FrameQueue_Push(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize, uint8_t direction);
static GlobalRet_Enum FrameQueue_Pop(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction);

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
    uint16_t bufSize = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txBufSize : pDCB->rxBufSize;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;
    uint16_t *pReadIdx = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txReadIdx : &pDCB->rxReadIdx;
    uint16_t magicNumber = 0;
    uint16_t topicLength = 0;
    uint16_t dataLength = 0;
    uint16_t totalSize = 0;
    uint16_t readIdx = *pReadIdx;
    uint16_t firstPart = 0;
    
    if (*pBufLen >= sizeof(FrameQueueHead_Struct))
    {
        /* 读取头部 */
        if (readIdx + sizeof(FrameQueueHead_Struct) <= bufSize)
        {
            memcpy(&stHead, &pOptBuf[readIdx], sizeof(FrameQueueHead_Struct));
        }
        else
        {
            firstPart = bufSize - readIdx;
            memcpy(&stHead, &pOptBuf[readIdx], firstPart);
            memcpy((uint8_t*)&stHead + firstPart, &pOptBuf[0], sizeof(FrameQueueHead_Struct) - firstPart);
        }
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        topicLength = Common_TwoUint8ToUint16(stHead.topicLen);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        totalSize = sizeof(FrameQueueHead_Struct) + topicLength + dataLength;
        
        if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && totalSize <= *pBufLen)
        {
            /* 跳过头部 */
            readIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % bufSize;
            
            /* 读取Topic */
            if (topicLength > 0)
            {
                if (readIdx + topicLength <= bufSize)
                {
                    memcpy(pTopic, &pOptBuf[readIdx], topicLength);
                }
                else
                {
                    firstPart = bufSize - readIdx;
                    memcpy(pTopic, &pOptBuf[readIdx], firstPart);
                    memcpy(&pTopic[firstPart], &pOptBuf[0], topicLength - firstPart);
                }
                *pTopicLen = topicLength;
                readIdx = (readIdx + topicLength) % bufSize;
            }
            
            /* 读取数据 */
            if (dataLength > 0)
            {
                if (readIdx + dataLength <= bufSize)
                {
                    memcpy(pDstData, &pOptBuf[readIdx], dataLength);
                }
                else
                {
                    firstPart = bufSize - readIdx;
                    memcpy(pDstData, &pOptBuf[readIdx], firstPart);
                    memcpy(&pDstData[firstPart], &pOptBuf[0], dataLength - firstPart);
                }
                *pDataSize = dataLength;
                readIdx = (readIdx + dataLength) % bufSize;
            }
            
            // 更新读取指针和缓冲区长度
            *pReadIdx = readIdx;
            *pBufLen -= totalSize;
            
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
    uint16_t bufSize = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txBufSize : pDCB->rxBufSize;
    uint16_t *pBufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txLen : &pDCB->rxLen;
    uint16_t *pReadIdx = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txReadIdx : &pDCB->rxReadIdx;
    uint16_t magicNumber = 0;
    uint16_t dataLength = 0;
    uint16_t totalSize = 0;
    uint16_t readIdx = *pReadIdx;
    uint16_t firstPart = 0;

    if (*pBufLen >= sizeof(FrameQueueHead_Struct))
    {
        /* 读取头部 */
        if (readIdx + sizeof(FrameQueueHead_Struct) <= bufSize)
        {
            memcpy(&stHead, &pOptBuf[readIdx], sizeof(FrameQueueHead_Struct));
        }
        else
        {
            firstPart = bufSize - readIdx;
            memcpy(&stHead, &pOptBuf[readIdx], firstPart);
            memcpy((uint8_t*)&stHead + firstPart, &pOptBuf[0], sizeof(FrameQueueHead_Struct) - firstPart);
        }
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        totalSize = sizeof(FrameQueueHead_Struct) + dataLength;
        
        if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && totalSize <= *pBufLen)
        {
            /* 跳过头部 */
            readIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % bufSize;
            
            /* 读取数据 */
            if (pDstData != NULL && pDataSize != NULL && dataLength > 0)
            {
                if (readIdx + dataLength <= bufSize)
                {
                    memcpy(pDstData, &pOptBuf[readIdx], dataLength);
                }
                else
                {
                    firstPart = bufSize - readIdx;
                    memcpy(pDstData, &pOptBuf[readIdx], firstPart);
                    memcpy(&pDstData[firstPart], &pOptBuf[0], dataLength - firstPart);
                }
                *pDataSize = dataLength;
                readIdx = (readIdx + dataLength) % bufSize;
            }
    
            // 更新读取指针和缓冲区长度
            *pReadIdx = readIdx;
            *pBufLen -= totalSize;
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
    uint16_t *pWriteIdx = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txWriteIdx : &pDCB->rxWriteIdx;
    uint16_t totalSize = sizeof(FrameQueueHead_Struct) + topicLen + dataLen;
    uint16_t writeIdx = *pWriteIdx;
    uint16_t firstPart = 0;

    if (*pBufLen + totalSize < bufSize)
    {
        Common_Uint16ToTwoUint8(stHead.ctrlWord, ctrlWord);
        Common_Uint16ToTwoUint8(stHead.topicLen, topicLen);
        Common_Uint16ToTwoUint8(stHead.dataLen, dataLen);

        /* 写入头部 */
        if (writeIdx + sizeof(FrameQueueHead_Struct) <= bufSize)
        {
            memcpy(&pOptBuf[writeIdx], &stHead, sizeof(FrameQueueHead_Struct));
            writeIdx += sizeof(FrameQueueHead_Struct);
        }
        else
        {
            firstPart = bufSize - writeIdx;
            memcpy(&pOptBuf[writeIdx], &stHead, firstPart);
            memcpy(&pOptBuf[0], (uint8_t*)&stHead + firstPart, sizeof(FrameQueueHead_Struct) - firstPart);
            writeIdx = sizeof(FrameQueueHead_Struct) - firstPart;
        }

        /* 写入Topic */
        if (writeIdx + topicLen <= bufSize)
        {
            memcpy(&pOptBuf[writeIdx], pTopic, topicLen);
            writeIdx += topicLen;
        }
        else
        {
            firstPart = bufSize - writeIdx;
            memcpy(&pOptBuf[writeIdx], pTopic, firstPart);
            memcpy(&pOptBuf[0], &pTopic[firstPart], topicLen - firstPart);
            writeIdx = topicLen - firstPart;
        }

        /* 写入数据 */
        if (writeIdx + dataLen <= bufSize)
        {
            memcpy(&pOptBuf[writeIdx], pSrcData, dataLen);
            writeIdx += dataLen;
        }
        else
        {
            firstPart = bufSize - writeIdx;
            memcpy(&pOptBuf[writeIdx], pSrcData, firstPart);
            memcpy(&pOptBuf[0], &pSrcData[firstPart], dataLen - firstPart);
            writeIdx = dataLen - firstPart;
        }

        *pWriteIdx = writeIdx;
        *pBufLen += totalSize;
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
    uint16_t *pWriteIdx = (direction == FRAME_QUEUE_DIRECTION_TX) ? &pDCB->txWriteIdx : &pDCB->rxWriteIdx;
    uint16_t totalSize = sizeof(FrameQueueHead_Struct) + dataLen;
    uint16_t writeIdx = *pWriteIdx;
    uint16_t firstPart = 0;

    if (*pBufLen + totalSize < bufSize)
    {
        Common_Uint16ToTwoUint8(stHead.ctrlWord, ctrlWord);
        Common_Uint16ToTwoUint8(stHead.dataLen, dataLen);

        /* 写入头部 */
        if (writeIdx + sizeof(FrameQueueHead_Struct) <= bufSize)
        {
            memcpy(&pOptBuf[writeIdx], &stHead, sizeof(FrameQueueHead_Struct));
            writeIdx += sizeof(FrameQueueHead_Struct);
        }
        else
        {
            firstPart = bufSize - writeIdx;
            memcpy(&pOptBuf[writeIdx], &stHead, firstPart);
            memcpy(&pOptBuf[0], (uint8_t*)&stHead + firstPart, sizeof(FrameQueueHead_Struct) - firstPart);
            writeIdx = sizeof(FrameQueueHead_Struct) - firstPart;
        }

        /* 写入数据 */
        if (writeIdx + dataLen <= bufSize)
        {
            memcpy(&pOptBuf[writeIdx], pSrcData, dataLen);
            writeIdx += dataLen;
        }
        else
        {
            firstPart = bufSize - writeIdx;
            memcpy(&pOptBuf[writeIdx], pSrcData, firstPart);
            memcpy(&pOptBuf[0], &pSrcData[firstPart], dataLen - firstPart);
            writeIdx = dataLen - firstPart;
        }

        *pWriteIdx = writeIdx;
        *pBufLen += totalSize;
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
    uint16_t bufSize = 0;
    uint16_t bufLen = 0;
    uint16_t readIdx = 0;
    uint16_t magicNumber = 0;
    uint16_t dataLength = 0;
    uint16_t topicLength = 0;
    uint16_t totalSize = 0;
    uint16_t firstPart = 0;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDataLen != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    pOptBuf = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->pTXBuf : pDCB->pRXBuf;
    bufSize = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txBufSize : pDCB->rxBufSize;
    bufLen = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txLen : pDCB->rxLen;
    readIdx = (direction == FRAME_QUEUE_DIRECTION_TX) ? pDCB->txReadIdx : pDCB->rxReadIdx;
    
    if (bufLen >= sizeof(FrameQueueHead_Struct))
    {
        /* 读取头部 */
        if (readIdx + sizeof(FrameQueueHead_Struct) <= bufSize)
        {
            memcpy(&stHead, &pOptBuf[readIdx], sizeof(FrameQueueHead_Struct));
        }
        else
        {
            firstPart = bufSize - readIdx;
            memcpy(&stHead, &pOptBuf[readIdx], firstPart);
            memcpy((uint8_t*)&stHead + firstPart, &pOptBuf[0], sizeof(FrameQueueHead_Struct) - firstPart);
        }
        
        magicNumber = Common_TwoUint8ToUint16(stHead.ctrlWord);
        dataLength = Common_TwoUint8ToUint16(stHead.dataLen);
        
        if (pDCB->frameType == eFrameQueueType_MQTT)
        {
            topicLength = Common_TwoUint8ToUint16(stHead.topicLen);
            totalSize = sizeof(FrameQueueHead_Struct) + topicLength + dataLength;
            if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && totalSize <= bufLen)
            {
                *pDataLen = dataLength;
                
                // 如果需要获取Topic信息
                if (pTopicLen != NULL)
                {
                    *pTopicLen = topicLength;
                }

                if (pTopic != NULL)
                {
                    uint16_t topicReadIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % bufSize;
                    if (topicReadIdx + topicLength <= bufSize)
                    {
                        memcpy(pTopic, &pOptBuf[topicReadIdx], topicLength);
                    }
                    else
                {
                    firstPart = bufSize - topicReadIdx;
                    memcpy(pTopic, &pOptBuf[topicReadIdx], firstPart);
                    memcpy(&pTopic[firstPart], &pOptBuf[0], topicLength - firstPart);
                }
                }

                eRet = eGlobalRet_OK;
            }
        }
        else if (pDCB->frameType == eFrameQueueType_TCP)
        {
            totalSize = sizeof(FrameQueueHead_Struct) + dataLength;
            if (magicNumber == FRAME_QUEUE_MAGIC_NUMBER && totalSize <= bufLen)
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

static GlobalRet_Enum FrameQueue_Push(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize, uint8_t direction)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || topicLen == 0)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PushMQTT(pDCB, pTopic, topicLen, pSrcData, dataSize, direction);
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
            eRet = FrameQueue_PushTCP(pDCB, pTopic, topicLen, pSrcData, dataSize, direction);
        }
    }
    else
    {
        eRet = eGlobalRet_Unsupported;
    }

    return eRet;
}

static GlobalRet_Enum FrameQueue_Pop(FrameQueueCtrlDCB_Struct *pDCB, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize, uint8_t direction)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;

    if (pDCB->frameType == eFrameQueueType_MQTT)
    {
        if (pTopic == NULL || pTopicLen == NULL)
        {
            eRet = eGlobalRet_ParaInvalid;
        }
        else
        {
            eRet = FrameQueue_PopMQTT(pDCB, pTopic, pTopicLen, pDstData, pDataSize, direction);
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
            eRet = FrameQueue_PopTCP(pDCB, pDstData, pDataSize, direction);
        }
    }
    else
    {
        eRet = eGlobalRet_Unsupported;
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
        pDCB->txReadIdx = 0;
        pDCB->txWriteIdx = 0;

        pDCB->rxLen = 0;
        pDCB->rxBufSize = rxBufSize;
        pDCB->pRXBuf = (uint8_t *)myCalloc(rxBufSize, 1);
        pDCB->rxReadIdx = 0;
        pDCB->rxWriteIdx = 0;

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
                myFree(pDCB->pTXBuf);
                pDCB->pTXBuf = NULL;
            }

            if (pDCB->pRXBuf != NULL)
            {
                myFree(pDCB->pRXBuf);
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
    pDCB->txReadIdx = 0;
    pDCB->txWriteIdx = 0;
    pDCB->rxLen = 0;
    pDCB->rxReadIdx = 0;
    pDCB->rxWriteIdx = 0;
    return eRet;
}

GlobalRet_Enum FrameQueue_PushTx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize)
{ 
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pSrcData != NULL && dataSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    return FrameQueue_Push(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_TX);
}

GlobalRet_Enum FrameQueue_PushRx(uint8_t channelID, char *pTopic, uint16_t topicLen, uint8_t *pSrcData, uint16_t dataSize)
{ 
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pSrcData != NULL && dataSize != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    return FrameQueue_Push(pDCB, pTopic, topicLen, pSrcData, dataSize, FRAME_QUEUE_DIRECTION_RX);
}


GlobalRet_Enum FrameQueue_PopTx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDstData != NULL && pDataSize != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    return FrameQueue_Pop(pDCB, pTopic, pTopicLen, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_TX);
}

GlobalRet_Enum FrameQueue_PopRx(uint8_t channelID, char *pTopic, uint16_t *pTopicLen, uint8_t *pDstData, uint16_t *pDataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDstData != NULL && pDataSize != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);

    return FrameQueue_Pop(pDCB, pTopic, pTopicLen, pDstData, pDataSize, FRAME_QUEUE_DIRECTION_RX);
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
    FrameQueueHead_Struct stHead = {0};
    uint16_t dataLen = 0;
    uint16_t topicLen = 0;
    uint16_t dealLen = 0;  
    uint8_t *pTopicData = NULL;
    uint8_t *pFrameData = NULL;
    uint16_t totalSize = 0;
    uint16_t readIdx = pDCB->rxReadIdx;
    uint16_t firstPart = 0;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDecodeFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDCB->rxLen > 0, eGlobalRet_NotEnoughData);

    /* 读取头部 */
    if (readIdx + sizeof(FrameQueueHead_Struct) <= pDCB->rxBufSize)
    {
        memcpy(&stHead, &pDCB->pRXBuf[readIdx], sizeof(FrameQueueHead_Struct));
    }
    else
    {
        firstPart = pDCB->rxBufSize - readIdx;
        memcpy(&stHead, &pDCB->pRXBuf[readIdx], firstPart);
        memcpy((uint8_t*)&stHead + firstPart, &pDCB->pRXBuf[0], sizeof(FrameQueueHead_Struct) - firstPart);
    }

    if (Common_TwoUint8ToUint16(stHead.ctrlWord) != FRAME_QUEUE_MAGIC_NUMBER)
    {
        pDCB->rxLen = 0;
        pDCB->rxReadIdx = 0;
        pDCB->rxWriteIdx = 0;
        eRet = eGlobalRet_UnexpectedError;
    }
    else
    { 
        dataLen = Common_TwoUint8ToUint16(stHead.dataLen);
        topicLen = Common_TwoUint8ToUint16(stHead.topicLen);
        totalSize = sizeof(FrameQueueHead_Struct) + dataLen + topicLen;

        if (totalSize > pDCB->rxLen)
        {
            pDCB->rxLen = 0;
            pDCB->rxReadIdx = 0;
            pDCB->rxWriteIdx = 0;
            eRet = eGlobalRet_UnexpectedError;
        }
        else
        {
            if (pDCB->frameType == eFrameQueueType_MQTT)
            {
                /* 跳过头部 */
                readIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % pDCB->rxBufSize;
                
                /* 直接使用环形缓冲区中的数据 */
                if (readIdx + topicLen <= pDCB->rxBufSize)
                {
                    pTopicData = &pDCB->pRXBuf[readIdx];
                }
                else
                {
                    /* 如果Topic跨边界，需要特殊处理，这里暂时使用临时缓冲区 */
                    pTopicData = (uint8_t*)myCalloc(topicLen, 1);
                    if (pTopicData != NULL)
                    {
                        firstPart = pDCB->rxBufSize - readIdx;
                        memcpy(pTopicData, &pDCB->pRXBuf[readIdx], firstPart);
                        memcpy(&pTopicData[firstPart], &pDCB->pRXBuf[0], topicLen - firstPart);
                    }
                }
                
                readIdx = (readIdx + topicLen) % pDCB->rxBufSize;
                
                /* 直接使用环形缓冲区中的数据 */
                if (readIdx + dataLen <= pDCB->rxBufSize)
                {
                    pFrameData = &pDCB->pRXBuf[readIdx];
                    pDecodeFunc(pFrameData, dataLen, topicLen, pTopicData, &dealLen);
                }
                else
                {
                    /* 如果数据跨边界，需要特殊处理，这里暂时使用临时缓冲区 */
                    pFrameData = (uint8_t*)myCalloc(dataLen, 1);
                    if (pFrameData != NULL)
                    {
                        firstPart = pDCB->rxBufSize - readIdx;
                        memcpy(pFrameData, &pDCB->pRXBuf[readIdx], firstPart);
                        memcpy(&pFrameData[firstPart], &pDCB->pRXBuf[0], dataLen - firstPart);
                        pDecodeFunc(pFrameData, dataLen, topicLen, pTopicData, &dealLen);
                        myFree(pFrameData);
                    }
                }
                
                /* 释放临时缓冲区 */
                if (pTopicData != &pDCB->pRXBuf[(readIdx - topicLen + pDCB->rxBufSize) % pDCB->rxBufSize])
                {
                    myFree(pTopicData);
                }
                
                readIdx = (readIdx + dataLen) % pDCB->rxBufSize;
                pDCB->rxReadIdx = readIdx;
                pDCB->rxLen -= totalSize;
            }
            else if (pDCB->frameType == eFrameQueueType_TCP)
            {
                /* 跳过头部 */
                readIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % pDCB->rxBufSize;
                
                /* 直接使用环形缓冲区中的数据 */
                if (readIdx + dataLen <= pDCB->rxBufSize)
                {
                    pFrameData = &pDCB->pRXBuf[readIdx];
                    while (dataLen > processedLen)
                    {
                        dealLen = 0;
                        pDecodeFunc(&pFrameData[processedLen], dataLen - processedLen, 0, NULL, &dealLen);
                        if (dealLen == 0)  { break; }
                        processedLen += dealLen;

                    }
                }
                else
                {
                    /* 如果数据跨边界，需要特殊处理，这里暂时使用临时缓冲区 */
                    pFrameData = (uint8_t*)myCalloc(dataLen, 1);
                    if (pFrameData != NULL)
                    {
                        firstPart = pDCB->rxBufSize - readIdx;
                        memcpy(pFrameData, &pDCB->pRXBuf[readIdx], firstPart);
                        memcpy(&pFrameData[firstPart], &pDCB->pRXBuf[0], dataLen - firstPart);
                        while (dataLen > processedLen)
                        {
                            dealLen = 0;
                            pDecodeFunc(&pFrameData[processedLen], dataLen - processedLen, 0, NULL, &dealLen);
                            if (dealLen == 0)  { break; }
                            processedLen += dealLen;
                        }

                        myFree(pFrameData);
                    }
                }
                
                readIdx = (readIdx + dataLen) % pDCB->rxBufSize;
                pDCB->rxReadIdx = readIdx;
                pDCB->rxLen -= (sizeof(FrameQueueHead_Struct) + dataLen);
            }
            else
            {
                eRet = eGlobalRet_Unsupported;
            }
        }
    }

    return eRet;
}

GlobalRet_Enum FrameQueue_TransmitTxData(uint8_t channelID, typeFuncTransmit pTransmitFunc, void *userData)
{
    FrameQueueCtrlDCB_Struct *pDCB = &g_stFrameQueueCtrlDCB[channelID];
    GlobalRet_Enum eRet = eGlobalRet_OK;
    FrameQueueHead_Struct stHead = {0};
    uint16_t dataLen = 0;
    uint16_t topicLen = 0;
    uint8_t *pFrameData = NULL;
    uint16_t totalSize = 0;
    uint16_t readIdx = pDCB->txReadIdx;
    uint16_t firstPart = 0;

    PARA_ASSERT_RET(channelID < FRAME_QUEUE_CHANNEL_COUNT, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pTransmitFunc != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pDCB->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(pDCB->txLen > 0, eGlobalRet_NotEnoughData);

    /* 读取头部 */
    if (readIdx + sizeof(FrameQueueHead_Struct) <= pDCB->txBufSize)
    {
        memcpy(&stHead, &pDCB->pTXBuf[readIdx], sizeof(FrameQueueHead_Struct));
    }
    else
    {
        firstPart = pDCB->txBufSize - readIdx;
        memcpy(&stHead, &pDCB->pTXBuf[readIdx], firstPart);
        memcpy((uint8_t*)&stHead + firstPart, &pDCB->pTXBuf[0], sizeof(FrameQueueHead_Struct) - firstPart);
    }

    if (Common_TwoUint8ToUint16(stHead.ctrlWord) != FRAME_QUEUE_MAGIC_NUMBER)
    {
        pDCB->txLen = 0;
        pDCB->txReadIdx = 0;
        pDCB->txWriteIdx = 0;
        eRet = eGlobalRet_UnexpectedError;
    }
    else
    { 
        dataLen = Common_TwoUint8ToUint16(stHead.dataLen);
        topicLen = Common_TwoUint8ToUint16(stHead.topicLen);
        totalSize = sizeof(FrameQueueHead_Struct) + dataLen + topicLen;

        if (totalSize > pDCB->txLen)
        {
            pDCB->txLen = 0;
            pDCB->txReadIdx = 0;
            pDCB->txWriteIdx = 0;
            eRet = eGlobalRet_UnexpectedError;
        }
        else
        {
            if (pDCB->frameType == eFrameQueueType_MQTT)
            {
                /* 跳过头部和Topic */
                readIdx = (readIdx + sizeof(FrameQueueHead_Struct) + topicLen) % pDCB->txBufSize;
                
                /* 直接使用环形缓冲区中的数据 */
                if (readIdx + dataLen <= pDCB->txBufSize)
                {
                    pFrameData = &pDCB->pTXBuf[readIdx];
                    pTransmitFunc(pFrameData, dataLen, userData);
                }
                else
                {
                    /* 如果数据跨边界，需要特殊处理，这里暂时使用临时缓冲区 */
                    pFrameData = (uint8_t*)myCalloc(dataLen, 1);
                    if (pFrameData != NULL)
                    {
                        firstPart = pDCB->txBufSize - readIdx;
                        memcpy(pFrameData, &pDCB->pTXBuf[readIdx], firstPart);
                        memcpy(&pFrameData[firstPart], &pDCB->pTXBuf[0], dataLen - firstPart);
                        pTransmitFunc(pFrameData, dataLen, userData);
                        myFree(pFrameData);
                    }
                }
                
                readIdx = (readIdx + dataLen) % pDCB->txBufSize;
                pDCB->txReadIdx = readIdx;
                pDCB->txLen -= totalSize;
            }
            else if (pDCB->frameType == eFrameQueueType_TCP)
            {
                /* 跳过头部 */
                readIdx = (readIdx + sizeof(FrameQueueHead_Struct)) % pDCB->txBufSize;
                
                /* 直接使用环形缓冲区中的数据 */
                if (readIdx + dataLen <= pDCB->txBufSize)
                {
                    pFrameData = &pDCB->pTXBuf[readIdx];
                    pTransmitFunc(pFrameData, dataLen, userData);
                }
                else
                {
                    /* 如果数据跨边界，需要特殊处理，这里暂时使用临时缓冲区 */
                    pFrameData = (uint8_t*)myCalloc(dataLen, 1);
                    if (pFrameData != NULL)
                    {
                        firstPart = pDCB->txBufSize - readIdx;
                        memcpy(pFrameData, &pDCB->pTXBuf[readIdx], firstPart);
                        memcpy(&pFrameData[firstPart], &pDCB->pTXBuf[0], dataLen - firstPart);
                        pTransmitFunc(pFrameData, dataLen, userData);
                        myFree(pFrameData);
                    }
                }
                
                readIdx = (readIdx + dataLen) % pDCB->txBufSize;
                pDCB->txReadIdx = readIdx;
                pDCB->txLen -= (sizeof(FrameQueueHead_Struct) + dataLen);
            }
            else
            {
                eRet = eGlobalRet_Unsupported;
            }
        }
    }

    return eRet;
}




