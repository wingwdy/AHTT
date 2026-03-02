/******************************************************************************
* File Name          : CycleBuf.c
* Description        : Code for Circular memory management algorithm
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V index S index O N   H index S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "CycleBuf.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CYCLEBUF_STATUS_INUSE    	(1u)		/* Inuse Status */
#define CYCLEBUF_STATUS_FREE  	    (0u)		/* Free Status */

/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
	uint8_t   status;
    uint8_t*  data;
    uint16_t  buffSize;
    uint16_t  readIdx;
    uint16_t  writeIdx;
    uint8_t   profile;
}CycleBuf_Struct;   



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CycleBuf_Struct g_stCycleBufCtrl[CYCLEBUF_MAX_CHANNEL_COUNT] = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CycleBuf_Struct *CycleBuf_FindFreeChannel(uint8_t *pChannel);
static void CycleBuf_CopyDataWithWrap(uint8_t *buf, uint16_t bufSize, uint16_t startIdx, const uint8_t *src, uint16_t size, uint16_t *newIdx);
static void CycleBuf_CopyDataWithWrapRead(uint8_t *dst, const uint8_t *buf, uint16_t bufSize, uint16_t startIdx, uint16_t size, uint16_t *newIdx);
static uint16_t CycleBuf_GetFreeSize(CycleBuf_Struct *pCycBuf);
static uint16_t CycleBuf_GetUsedSize(CycleBuf_Struct *pCycBuf);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CycleBuf_Struct *CycleBuf_FindFreeChannel(uint8_t *pChannel)
{
    CycleBuf_Struct *pCycBuf = NULL;
    uint8_t index;

    for (index = 0; index < CYCLEBUF_MAX_CHANNEL_COUNT; index++)
    {
        pCycBuf = &g_stCycleBufCtrl[index];

        if (pCycBuf->status == CYCLEBUF_STATUS_FREE)
        {
            if (pChannel != NULL)
            {
                pChannel[0] = index;
            }

            break;
        }

        pCycBuf = NULL;
    }

    return pCycBuf;
}

static void CycleBuf_CopyDataWithWrap(uint8_t *buf, uint16_t bufSize, uint16_t startIdx, const uint8_t *src, uint16_t size, uint16_t *newIdx)
{
    if (startIdx + size <= bufSize)
    {
        /* 不需要跨边界，直接拷贝 */
        memcpy(&buf[startIdx], src, size);
        *newIdx = startIdx + size;
    }
    else
    {
        /* 需要跨边界，分两部分拷贝 */
        uint16_t firstPartSize = bufSize - startIdx;
        memcpy(&buf[startIdx], src, firstPartSize);
        memcpy(&buf[0], &src[firstPartSize], size - firstPartSize);
        *newIdx = size - firstPartSize;
    }
}

static void CycleBuf_CopyDataWithWrapRead(uint8_t *dst, const uint8_t *buf, uint16_t bufSize, uint16_t startIdx, uint16_t size, uint16_t *newIdx)
{
    if (startIdx + size <= bufSize)
    {
        /* 不需要跨边界，直接拷贝 */
        memcpy(dst, &buf[startIdx], size);
        *newIdx = startIdx + size;
    }
    else
    {
        /* 需要跨边界，分两部分拷贝 */
        uint16_t firstPartSize = bufSize - startIdx;
        memcpy(dst, &buf[startIdx], firstPartSize);
        memcpy(&dst[firstPartSize], &buf[0], size - firstPartSize);
        *newIdx = size - firstPartSize;
    }
}

static uint16_t CycleBuf_GetFreeSize(CycleBuf_Struct *pCycBuf)
{
    uint16_t freeSize = 0;
    
    if (pCycBuf->writeIdx >= pCycBuf->readIdx)
    {
        freeSize = pCycBuf->buffSize - (pCycBuf->writeIdx - pCycBuf->readIdx) - 1;
    }
    else 
    {
        freeSize = pCycBuf->readIdx - pCycBuf->writeIdx - 1;
    }
    
    return freeSize;
}

static uint16_t CycleBuf_GetUsedSize(CycleBuf_Struct *pCycBuf)
{
    uint16_t usedSize = 0;
    
    if (pCycBuf->readIdx != pCycBuf->writeIdx)
    {
        if (pCycBuf->readIdx > pCycBuf->writeIdx)
        {
            usedSize = pCycBuf->buffSize - pCycBuf->readIdx + pCycBuf->writeIdx;
        }
        else
        {
            usedSize = pCycBuf->writeIdx - pCycBuf->readIdx;
        }
    }
    
    return usedSize;
}



void CycleBuf_Init(void)
{
    uint8_t index = 0;

	for ( index = 0; index < CYCLEBUF_MAX_CHANNEL_COUNT; index++)
	{
		g_stCycleBufCtrl[index].status 	= CYCLEBUF_STATUS_FREE;
		g_stCycleBufCtrl[index].buffSize = 0u;
		g_stCycleBufCtrl[index].data = NULL;
		g_stCycleBufCtrl[index].readIdx = 0u;
		g_stCycleBufCtrl[index].writeIdx = 0u;
        g_stCycleBufCtrl[index].profile = CYCLEBUF_PROFILE_CIRCLE;
	}
}

GlobalRet_Enum CycleBuf_CreateChannel(uint8_t* pChannel, uint8_t* pDataBuf, uint32_t bufSize, uint8_t porfile)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
    CycleBuf_Struct *pCycBuf = NULL;
	uint8_t index;

	if ((0 == bufSize) || (pDataBuf == NULL) || (pChannel == NULL))
	{
        eRet = eGlobalRet_ParaInvalid;
	}
    else if (porfile != CYCLEBUF_PROFILE_CIRCLE && porfile != CYCLEBUF_PROFILE_SINGLE)
    {
        eRet = eGlobalRet_ParaInvalid;
    }
	else
	{
        pCycBuf = CycleBuf_FindFreeChannel(&index);

        if (pCycBuf == NULL)
        {
            eRet = eGlobalRet_NotEnoughChannel;
        }
        else
        {
            pCycBuf->data = pDataBuf;
            pCycBuf->buffSize = bufSize;
            pCycBuf->readIdx = 0u;
            pCycBuf->writeIdx = 0u;
            pCycBuf->status = CYCLEBUF_STATUS_INUSE;
            pCycBuf->profile = porfile;
            pChannel[0] = index;
            eRet = eGlobalRet_OK;
        }
	}

	return eRet;
}

GlobalRet_Enum CycleBuf_WriteData(uint8_t channel, uint8_t * pSrcData, uint16_t dataSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
    CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t writeIdx = 0u;
    uint16_t newIdx = 0u;
    uint16_t freeSize = 0u;

	if ((0u == dataSize) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pSrcData))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_INUSE != pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();
        
        /* 检查缓冲区状态和可用空间 */
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if ((pCycBuf->writeIdx + 1) % pCycBuf->buffSize == pCycBuf->readIdx)
            {
                eRet = eGlobalRet_NotEnoughBuf;
            }
            else
            {
                freeSize = CycleBuf_GetFreeSize(pCycBuf);
                if (dataSize > freeSize)
                {
                    eRet = eGlobalRet_NotEnoughBuf;
                }
                else
                {
                    /* 计算写入位置 */
                    writeIdx = pCycBuf->writeIdx;
                    CYCLEBUF_EXIT_CRITICAL_AREA();
                    
                    /* 在临界区外进行数据传输 */
                    CycleBuf_CopyDataWithWrap(pCycBuf->data, pCycBuf->buffSize, writeIdx, pSrcData, dataSize, &newIdx);
                    
                    /* 重新进入临界区更新指针 */
                    CYCLEBUF_ENTER_CRITICAL_AREA();
                    pCycBuf->writeIdx = newIdx;
                    eRet = eGlobalRet_OK;
                }
            }
        }
        else
        {
            if ((pCycBuf->writeIdx + dataSize) > pCycBuf->buffSize)
            {
                eRet = eGlobalRet_NotEnoughBuf; 
            }
            else
            {
                writeIdx = pCycBuf->writeIdx;
                CYCLEBUF_EXIT_CRITICAL_AREA();
                
                /* 在临界区外进行数据传输 */
                memcpy(&pCycBuf->data[writeIdx], pSrcData, dataSize);
                
                /* 重新进入临界区更新指针 */
                CYCLEBUF_ENTER_CRITICAL_AREA();
                pCycBuf->writeIdx += dataSize;
                eRet = eGlobalRet_OK;
            }
        }
        
        CYCLEBUF_EXIT_CRITICAL_AREA();
    }

    return eRet;
}

GlobalRet_Enum CycleBuf_WriteDataIsr(uint8_t channel, uint8_t * pSrcData, uint16_t dataSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
    CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t freeBuffSize = 0u;
    uint16_t newIdx = 0u;

	if ((0u == dataSize) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pSrcData))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_INUSE != pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else
    {
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if ((pCycBuf->writeIdx + 1) % pCycBuf->buffSize == pCycBuf->readIdx)
            {
                eRet = eGlobalRet_NotEnoughBuf;
            }
            else
            {
                freeBuffSize = CycleBuf_GetFreeSize(pCycBuf);

                if (dataSize <= freeBuffSize)
                {
                    CycleBuf_CopyDataWithWrap(pCycBuf->data, pCycBuf->buffSize, pCycBuf->writeIdx, pSrcData, dataSize, &newIdx);
                    pCycBuf->writeIdx = newIdx;
                    eRet = eGlobalRet_OK;
                }
                else
                {
                    eRet = eGlobalRet_NotEnoughBuf;
                }
            }
        }
        else
        {
            if ((pCycBuf->writeIdx + dataSize) > pCycBuf->buffSize)
            {
                eRet = eGlobalRet_NotEnoughBuf; 
            }
            else
            {
                memcpy(&pCycBuf->data[pCycBuf->writeIdx], pSrcData, dataSize);
                pCycBuf->writeIdx += dataSize;
                eRet = eGlobalRet_OK;
            }
        }
    }

    return eRet;
}


GlobalRet_Enum CycleBuf_ReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t usedSize = 0u;
    uint16_t readIdx = 0u;
    uint16_t newIdx = 0u;

	if ((0u == readSize) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pOutData))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_FREE == pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else 
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();
        
        /* 检查缓冲区状态和可用数据 */
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (pCycBuf->readIdx == pCycBuf->writeIdx)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
            else
            {
                usedSize = CycleBuf_GetUsedSize(pCycBuf);
                if (readSize > usedSize)
                {
                    eRet = eGlobalRet_NotEnoughData;
                }
                else
                {
                    /* 计算读取位置 */
                    readIdx = pCycBuf->readIdx;
                    CYCLEBUF_EXIT_CRITICAL_AREA();
                    
                    /* 在临界区外进行数据传输 */
                    CycleBuf_CopyDataWithWrapRead(pOutData, pCycBuf->data, pCycBuf->buffSize, readIdx, readSize, &newIdx);
                    
                    /* 重新进入临界区更新指针 */
                    CYCLEBUF_ENTER_CRITICAL_AREA();
                    pCycBuf->readIdx = newIdx;
                    eRet = eGlobalRet_OK;
                }
            }
        }
        else
        {
            usedSize = pCycBuf->writeIdx - pCycBuf->readIdx;
            if (readSize > usedSize)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
            else
            {
                readIdx = pCycBuf->readIdx;
                CYCLEBUF_EXIT_CRITICAL_AREA();
                
                /* 在临界区外进行数据传输 */
                memcpy(pOutData, &pCycBuf->data[readIdx], readSize);
                
                /* 重新进入临界区更新指针 */
                CYCLEBUF_ENTER_CRITICAL_AREA();
                pCycBuf->readIdx += readSize;
                eRet = eGlobalRet_OK;
            }
        }
        
        CYCLEBUF_EXIT_CRITICAL_AREA(); 
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_ReadDataIsr(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t remainDataSize = 0u;
    uint16_t newIdx = 0u;

	if ((0u == readSize) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pOutData))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_FREE == pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else 
    {
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (pCycBuf->readIdx != pCycBuf->writeIdx)
            {
                remainDataSize = CycleBuf_GetUsedSize(pCycBuf);

                if (readSize <= remainDataSize)
                {
                    CycleBuf_CopyDataWithWrapRead(pOutData, pCycBuf->data, pCycBuf->buffSize, pCycBuf->readIdx, readSize, &newIdx);
                    pCycBuf->readIdx = newIdx;
                    eRet = eGlobalRet_OK;
                }
                else
                {
                    eRet = eGlobalRet_NotEnoughData;
                }
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else
        {
            remainDataSize = pCycBuf->writeIdx - pCycBuf->readIdx;
            if (readSize <= remainDataSize)
            {
                memcpy(pOutData, &pCycBuf->data[pCycBuf->readIdx], readSize);
                pCycBuf->readIdx += readSize;
                eRet = eGlobalRet_OK;
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_PreviewReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	uint16_t curReadIdx = 0u;
    uint16_t curWriteIdx = 0;
    uint16_t usedSize = 0u;
    uint16_t newIdx = 0u;

	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];

	if ((0u == readSize) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pOutData))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_FREE == pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else 
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();
        curReadIdx = pCycBuf->readIdx;
        curWriteIdx = pCycBuf->writeIdx;
        
        /* 检查缓冲区状态和可用数据 */
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (pCycBuf->readIdx == pCycBuf->writeIdx)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
            else
            {
                usedSize = CycleBuf_GetUsedSize(pCycBuf);
                if (readSize > usedSize)
                {
                    eRet = eGlobalRet_NotEnoughData;
                }
                else
                {
                    /* 计算读取位置 */
                    uint16_t readIdx = pCycBuf->readIdx;
                    CYCLEBUF_EXIT_CRITICAL_AREA();
                    
                    /* 在临界区外进行数据传输 */
                    CycleBuf_CopyDataWithWrapRead(pOutData, pCycBuf->data, pCycBuf->buffSize, readIdx, readSize, &newIdx);
                    
                    /* 重新进入临界区恢复指针 */
                    CYCLEBUF_ENTER_CRITICAL_AREA();
                    eRet = eGlobalRet_OK;
                }
            }
        }
        else
        {
            usedSize = pCycBuf->writeIdx - pCycBuf->readIdx;
            if (readSize > usedSize)
            {
                eRet = eGlobalRet_NotEnoughData;
            }
            else
            {
                uint16_t readIdx = pCycBuf->readIdx;
                CYCLEBUF_EXIT_CRITICAL_AREA();
                
                /* 在临界区外进行数据传输 */
                memcpy(pOutData, &pCycBuf->data[readIdx], readSize);
                
                /* 重新进入临界区恢复指针 */
                CYCLEBUF_ENTER_CRITICAL_AREA();
                eRet = eGlobalRet_OK;
            }
        }

        pCycBuf->readIdx = curReadIdx;
        pCycBuf->writeIdx = curWriteIdx;
        CYCLEBUF_EXIT_CRITICAL_AREA();
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_CheckDataLen(uint8_t channel, uint16_t* pRemainLen)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t remainLen = 0;

	if ((channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pRemainLen))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_INUSE != pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();

        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (pCycBuf->readIdx != pCycBuf->writeIdx)
            {
                if (pCycBuf->writeIdx > pCycBuf->readIdx)
                {
                    pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
                }
                else
                {
                    pRemainLen[0] = pCycBuf->buffSize + pCycBuf->writeIdx - pCycBuf->readIdx;
                }
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else
        {
            if (pCycBuf->readIdx != pCycBuf->writeIdx)
            {
                pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }

        CYCLEBUF_EXIT_CRITICAL_AREA(); 
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_CheckDataLenIsr(uint8_t channel, uint16_t* pRemainLen)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t remainLen = 0;

	if ((channel >= CYCLEBUF_MAX_CHANNEL_COUNT) || (NULL == pRemainLen))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_INUSE != pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else
    {
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (pCycBuf->readIdx != pCycBuf->writeIdx)
            {
                if (pCycBuf->writeIdx > pCycBuf->readIdx)
                {
                    pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
                }
                else
                {
                    pRemainLen[0] = pCycBuf->buffSize + pCycBuf->writeIdx - pCycBuf->readIdx;
                }
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
        else
        {
            if (pCycBuf->readIdx != pCycBuf->writeIdx)
            {
                pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
            }
            else
            {
                eRet = eGlobalRet_NotEnoughData;
            }
        }
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_RemoveData(uint8_t channel, uint16_t dataLen)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];
    uint16_t remainDataSize = 0u;

	if ((0u == dataLen) || (channel >= CYCLEBUF_MAX_CHANNEL_COUNT))
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else if (CYCLEBUF_STATUS_FREE == pCycBuf->status)
    {
        eRet = eGlobalRet_NotInit;
    }
    else 
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();

        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            if (((pCycBuf->readIdx + 1) % pCycBuf->buffSize) != pCycBuf->writeIdx)
            {
                if (pCycBuf->readIdx > pCycBuf->writeIdx)
                {
                    remainDataSize = pCycBuf->buffSize - (pCycBuf->readIdx - pCycBuf->writeIdx);
                }
                else
                {
                    remainDataSize = pCycBuf->writeIdx - pCycBuf->readIdx;
                }

                if (dataLen <= remainDataSize)
                {
                    pCycBuf->readIdx += dataLen;
                    eRet = eGlobalRet_OK;
                }
            }
        }
        else
        {
            remainDataSize = pCycBuf->writeIdx - pCycBuf->readIdx;

            if (dataLen <= remainDataSize)
            {
                pCycBuf->readIdx += dataLen;

                if (pCycBuf->readIdx == pCycBuf->writeIdx)
                {
                    pCycBuf->readIdx = 0;
                    pCycBuf->writeIdx = 0;
                }

                eRet = eGlobalRet_OK;
            }
        }

        CYCLEBUF_EXIT_CRITICAL_AREA(); 
    }

    return eRet;
}

GlobalRet_Enum CycleBuf_ResetBuf(uint8_t channel)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
	CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];

    if (channel >= CYCLEBUF_MAX_CHANNEL_COUNT)
    {
        eRet = eGlobalRet_ParaInvalid;
    }
    else
    {
        CYCLEBUF_ENTER_CRITICAL_AREA();
        pCycBuf->writeIdx = 0;
        pCycBuf->readIdx = 0;
        CYCLEBUF_EXIT_CRITICAL_AREA();
        eRet = eGlobalRet_OK;
    }

    return eRet;
}