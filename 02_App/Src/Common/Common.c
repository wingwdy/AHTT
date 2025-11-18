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



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
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

