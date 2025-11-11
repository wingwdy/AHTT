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
*2025/10/10      V1.0.0      Chenls    初版创建
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
static GlobalRet_Enum CycleBuf_SingleWriteData(CycleBuf_Struct *pCycBuf, uint8_t * pSrcData, uint16_t dataSize);
static GlobalRet_Enum CycleBuf_CircleWriteData(CycleBuf_Struct *pCycBuf, uint8_t * pSrcData, uint16_t dataSize);
static GlobalRet_Enum CycleBuf_CircleReadData(CycleBuf_Struct *pCycBuf, uint8_t * pOutData, uint16_t readSize);
static GlobalRet_Enum CycleBuf_SingleReadData(CycleBuf_Struct *pCycBuf, uint8_t * pOutData, uint16_t readSize);

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

static GlobalRet_Enum CycleBuf_SingleWriteData(CycleBuf_Struct *pCycBuf, uint8_t * pSrcData, uint16_t dataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t dataIdx = 0u;

    if ((pCycBuf->writeIdx + dataSize) > pCycBuf->buffSize)
    {
        eRet = eGlobalRet_NotEnoughBuf; 
    }
    else
    {
        for (dataIdx = 0; dataIdx < dataSize; dataIdx++)
        {
            pCycBuf->data[pCycBuf->writeIdx] = pSrcData[dataIdx];
            pCycBuf->writeIdx++;
        }

        eRet = eGlobalRet_OK;
    }

    return eRet;
}

static GlobalRet_Enum CycleBuf_CircleWriteData(CycleBuf_Struct *pCycBuf, uint8_t * pSrcData, uint16_t dataSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t dataIdx = 0u;
	uint16_t freeBuffSize = 0u;
        
    if ((pCycBuf->writeIdx + 1) % pCycBuf->buffSize == pCycBuf->readIdx)
    {
        eRet = eGlobalRet_NotEnoughBuf;
    }
    else
    {
        if (pCycBuf->writeIdx >= pCycBuf->readIdx)
        {
            freeBuffSize = pCycBuf->buffSize - (pCycBuf->writeIdx - pCycBuf->readIdx);
        }
        else 
        {
            freeBuffSize = pCycBuf->readIdx - pCycBuf->writeIdx;
        }

        if (dataSize <= freeBuffSize)
        {
            for (dataIdx = 0; dataIdx < dataSize; dataIdx++)
            {
                pCycBuf->data[pCycBuf->writeIdx] = pSrcData[dataIdx];
                pCycBuf->writeIdx = (pCycBuf->writeIdx + 1) % pCycBuf->buffSize;
            }
        
            eRet = eGlobalRet_OK;
        }
        else
        {
            eRet = eGlobalRet_NotEnoughBuf;
        }
    }

    return eRet;
}

static GlobalRet_Enum CycleBuf_CircleReadData(CycleBuf_Struct *pCycBuf, uint8_t * pOutData, uint16_t readSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t dataIdx = 0u;
	uint16_t remainDataSize = 0u;

    if (((pCycBuf->readIdx + 1) % pCycBuf->buffSize) != pCycBuf->writeIdx)
    {
        if (pCycBuf->readIdx >= pCycBuf->writeIdx)
        {
            remainDataSize = pCycBuf->buffSize - (pCycBuf->readIdx - pCycBuf->writeIdx);
        }
        else
        {
            remainDataSize = pCycBuf->writeIdx - pCycBuf->readIdx;
        }

        if (readSize <= remainDataSize)
        {
            for (dataIdx = 0; dataIdx < readSize; dataIdx++)
            {
                pOutData[dataIdx] = pCycBuf->data[pCycBuf->readIdx];
                pCycBuf->readIdx = (pCycBuf->readIdx + 1) % pCycBuf->buffSize;
            }
            
            eRet = eGlobalRet_OK;
        }
        else
        {
            eRet = eGlobalRet_NotEnoughData;
        }
    }

    return eRet;
}

static GlobalRet_Enum CycleBuf_SingleReadData(CycleBuf_Struct *pCycBuf, uint8_t * pOutData, uint16_t readSize)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint16_t dataIdx = 0u;
	uint16_t remainDataSize = 0u;

    remainDataSize = pCycBuf->writeIdx - pCycBuf->readIdx;

    if (readSize <= remainDataSize)
    {
        for (dataIdx = 0; dataIdx < readSize; dataIdx++)
        {
            pOutData[dataIdx] = pCycBuf->data[pCycBuf->readIdx];
            pCycBuf->readIdx = (pCycBuf->readIdx + 1) % pCycBuf->buffSize;
        }

        if (pCycBuf->readIdx == pCycBuf->writeIdx)
        {
            pCycBuf->readIdx = 0;
            pCycBuf->writeIdx = 0;
        }
        
        eRet = eGlobalRet_OK;
    }
    else
    {
        eRet = eGlobalRet_NotEnoughData;
    }

    return eRet;
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

GlobalRet_Enum CycleBuf_CreatChannel(uint8_t* pChannel, uint8_t* pDataBuf, uint32_t bufSize, uint8_t porfile)
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

        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            eRet = CycleBuf_CircleWriteData(pCycBuf, pSrcData, dataSize);
        }
        else
        {
            eRet = CycleBuf_SingleWriteData(pCycBuf, pSrcData, dataSize);
        }

        CYCLEBUF_EXIT_CRITICAL_AREA();
    }

    return eRet;
}

GlobalRet_Enum CycleBuf_WriteDataIsr(uint8_t channel, uint8_t * pSrcData, uint16_t dataSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
    CycleBuf_Struct *pCycBuf = &g_stCycleBufCtrl[channel];

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
            eRet = CycleBuf_CircleWriteData(pCycBuf, pSrcData, dataSize);
        }
        else
        {
            eRet = CycleBuf_SingleWriteData(pCycBuf, pSrcData, dataSize);
        }
    }

    return eRet;
}


GlobalRet_Enum CycleBuf_ReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
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
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            eRet = CycleBuf_CircleReadData(pCycBuf, pOutData, readSize);
        }
        else
        {
            eRet = CycleBuf_SingleReadData(pCycBuf, pOutData, readSize);
        }
        CYCLEBUF_EXIT_CRITICAL_AREA(); 
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_ReadDataIsr(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
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
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            eRet = CycleBuf_CircleReadData(pCycBuf, pOutData, readSize);
        }
        else
        {
            eRet = CycleBuf_SingleReadData(pCycBuf, pOutData, readSize);
        }
    }

	return eRet;
}

GlobalRet_Enum CycleBuf_PreviewReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize)
{
	GlobalRet_Enum eRet = eGlobalRet_OK;
	uint16_t dataIdx = 0u;
	uint16_t curReadIdx = 0u;
    uint16_t curWriteIdx = 0;

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
         
        if (pCycBuf->profile == CYCLEBUF_PROFILE_CIRCLE)
        {
            eRet = CycleBuf_CircleReadData(pCycBuf, pOutData, readSize);
        }
        else
        {
            eRet = CycleBuf_SingleReadData(pCycBuf, pOutData, readSize);
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
			if(pCycBuf->writeIdx >= pCycBuf->readIdx)
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
            pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
        }

        CYCLEBUF_EXIT_CRITICAL_AREA();
        eRet = eGlobalRet_OK;
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
			if(pCycBuf->writeIdx >= pCycBuf->readIdx)
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
            pRemainLen[0] = pCycBuf->writeIdx - pCycBuf->readIdx;
        }
        
        eRet = eGlobalRet_OK;
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