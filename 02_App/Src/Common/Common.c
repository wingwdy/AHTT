/******************************************************************************
* File Name          : Common.c
* Description        : Code for Common function
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
#include "Common.h"
#include "Mcal_Mcu.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
/* Table of CRC values for high–order byte */
static const uint8_t c_CRCHighByte[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
} ;

/* Table of CRC values for low–order byte */
static const uint8_t c_CRCLowByte[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
uint16_t Common_CalcCRC16(uint8_t *pData, uint16_t dataLen)
{
    uint8_t crcHi = 0xFF;                            
    uint8_t crcLo = 0xFF;                            
    uint8_t index;                                    

    while(dataLen--)                                  
    {
        index = crcHi ^ *pData++;                 
        crcHi = crcLo ^ c_CRCHighByte[index];
        crcLo = c_CRCLowByte[index];
    }

    return  (crcHi << 8) | crcLo;
}

/* 插入排序,从小到大 */
void Common_InsertSort(uint16_t *pData, uint16_t n)
{
	uint16_t i = 0,j = 0;
	uint16_t temp = 0;

	for( i = 1;i < n; i++ )
	{
		temp = pData[i]; 
		j = i;  
		while( j > 0 && temp < pData[j-1] )
		{ 
			pData[j] = pData[j-1];
			j--;
		}
		pData[j] = temp; 
	}
}

/* 获取系统时钟tick */
uint32_t Common_GetSystick(void)
{
	return Mcal_GetSystick();
}

/* 超时检测函数 */
uint8_t Common_JudgeTimeoutMs(uint32_t startTick, uint32_t threshold)
{
    uint32_t tickNow = Common_GetSystick();
	uint8_t ret = FALSE;

    if (tickNow >= startTick)
    {
        if ((tickNow - startTick) >= threshold)
        {
            ret = TRUE;
        }
    }
    else
    {
        if ((0xFFFFFFFFU - startTick + tickNow + 1) >= threshold)
        {
            ret = TRUE;
        }
    }

    return ret;
}


/* 冒泡排序,从小到大 */
void Common_BubbleSort(uint16_t *pData, uint16_t size)
{
    uint16_t i, j;
    uint16_t temp;

    for (j = 0; j < size; j++) //冒泡排序
    {
        for (i = 0; i < (size - j - 1); i++)
        {
            if (pData[i] > pData[i + 1])
            {
                temp = pData[i];
                pData[i] = pData[i + 1];
                pData[i + 1] = temp; //大的下沉到数组高端
            }
        }
    }
}

/* 中位值滤波法 */
uint16_t Common_MedianU16Filter(uint16_t *pData, uint16_t sample_num, uint16_t discard_num)
{
    uint16_t count = 0, sum = 0, ret = 0, tempDiscard = 0;

    if (sample_num < 3 || discard_num >= sample_num || pData == NULL)
    {
        ret = 0;
    }
    else
    {
        Common_BubbleSort(pData, sample_num);
        tempDiscard = (discard_num % 2 == 0) ? discard_num : (discard_num - 1);

        for (count = discard_num / 2; count < (sample_num - discard_num / 2); count++) 
        {
            sum +=  pData[count];
        }

        ret = sum / (sample_num - discard_num);
    }

    return ret;
}

void Common_Uint32ToFourUint8(uint8_t *pData, uint32_t CurValue)
{
    *pData++ = (CurValue & 0x00ff);
    *pData++ = ((CurValue >> 8) & 0x00ff);
    *pData++ = ((CurValue >> 16) & 0x00ff);
    *pData++ = ((CurValue >> 24) & 0x00ff);
}

void Common_Uint32ToTwoUint8(uint8_t *pData, uint32_t CurValue)
{
	*pData++ = (CurValue & 0x00ff);
	*pData++ = ((CurValue >>8 ) & 0x00ff);
}

uint32_t Common_FourUint8ToUint32(uint8_t *pData)
{
    uint32_t temp1, temp2, temp3, temp4;
    temp1 = *pData++;
    temp2 = *pData++;
    temp3 = *pData++;
    temp4 = *pData++;
    return ((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1);
}

uint16_t Common_TwoUint8ToUint16(uint8_t *pData)
{
    uint8_t temp1, temp2;
    temp1 = *pData++;
    temp2 = *pData++;
    return ((temp2 << 8) + temp1);
}

void Common_Uint16ToTwoUint8(uint8_t *pData, uint16_t curVal)
{
    *pData++ = (curVal & 0x00ff);
    *pData++ = ((curVal >> 8) & 0x00ff);
}

uint8_t* Common_SearchData(uint8_t *pData, uint16_t dataLen, void *pString, uint16_t stringLen)
{
    uint8_t *pTr = NULL;

	while(dataLen >= stringLen)
	{
		if (0 == memcmp(pData, pString, stringLen))
		{
			pTr = pData;
		}
		pData++;
		dataLen--;
	}

	return pTr;
}

uint16_t Common_ReplaceStr(uint8_t* pData, uint16_t nDataLen, char* cDestStr, void* pReplace, uint16_t nReplaceLen, char* pDefault)
{
	uint8_t *pDest = NULL;
	char cTailStr[100];
	void *pCopyData = pReplace;
	uint16_t nCopyLen = nReplaceLen;
	uint16_t nDestStrLen = strlen(cDestStr);
    uint16_t offset = 0;
    uint16_t tailLen = 0;

	memset(cTailStr, 0x00, sizeof(cTailStr));

	pDest = Common_SearchData(pData, nDataLen, cDestStr, nDestStrLen);

	if (NULL != pDest)
	{
		offset = pDest - pData;

        // 计算尾部字符串长度，并确保不会溢出cTailStr缓冲区
        tailLen = nDataLen - offset - nDestStrLen;

        if (tailLen >= sizeof(cTailStr))
        {
            tailLen = sizeof(cTailStr) - 1; // 确保留出终止符空间
        }
        
        // 保存尾巴
        strncpy(cTailStr, (char*)(pDest + nDestStrLen), tailLen);
        cTailStr[tailLen] = '\0'; // 确保字符串终止

        //默认
        if (0 == nCopyLen)
        {
            pCopyData = pDefault;

            if (pDefault != NULL) 
            {
                nCopyLen = strlen(pDefault);
            } 
            else 
            {
                nCopyLen = 0;
            }
        }

        memcpy(pDest, pCopyData, nCopyLen);
        pDest += nCopyLen;

        // 加上尾巴
        strcpy((char*)pDest, cTailStr);

        // 重新计算长度
        nDataLen = offset + nCopyLen + strlen(cTailStr);
	}

	return nDataLen;
}

uint16_t Common_ReplaceNum(uint8_t* pData, uint16_t nDataLen, char* cDestStr, uint16_t replace, uint16_t defaultNum)
{
    uint16_t dataLen = 0;
    char cReplace[32] = { 0 };
    char cDefault[32] = { 0 };

    // 验证输入参数
    if (pData != NULL && cDestStr != NULL) 
    {
        snprintf(cReplace, sizeof(cReplace), "%d", replace);
        snprintf(cDefault, sizeof(cDefault), "%d", defaultNum);
        dataLen = Common_ReplaceStr(pData, nDataLen, cDestStr, cReplace, strlen(cReplace), cDefault);
    }

    return dataLen;
}