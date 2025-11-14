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
*2025/10/10      V1.0.0      Chenls    初版创建
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
/* 排序,从小到大 */
void Common_InsertSort(uint16_t *a, uint16_t n)
{
	uint16_t i = 0,j = 0;
	uint16_t temp = 0;

	for( i = 1;i < n; i++ )
	{
		temp = a[i]; /*temp为要插入的元素*/
		j = i;  
		while( j > 0 && temp < a[j-1] )
		{ /*从a[i-1]开始找比a[i]小的数，同时把数组元素向后移*/
			a[j] = a[j-1];
			j--;
		}
		a[j] = temp; /*插入*/
	}
}

uint32_t Common_GetSystick(void)
{
	return Mcal_GetSystick();
}

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

uint16_t Common_BinarySearch(uint16_t *arr, uint16_t count, uint16_t target) 
{
   uint16_t result = 0xFFFFu;  

    /* 输入参数验证 */
    if ((NULL != arr) && (0u != count))
    {
        uint16_t low = 0u;
        uint16_t high = count - 1u;
        uint16_t mid = 0u;
        uint8_t found = 0u;  /* 查找标志 */

        /* 主查找循环 */
        while ((low <= high) && (0u == found))
        {
            /* 防溢出中间值计算 */
            mid = (uint16_t)(low + ((uint16_t)(high - low) / 2u));
            
            if (target == arr[mid])
            {
                result = mid;   /* 记录结果 */
                found = 1u;     /* 设置找到标志 */
            }
            else if (target > arr[mid])
            {
                /* 在右半部分继续查找 */
                low = (uint16_t)(mid + 1u);
            }
            else
            {
                /* 在左半部分继续查找，防下溢 */
                if (0u == mid)
                {
                    break;  /* 防止high减1下溢 */
                }
                high = (uint16_t)(mid - 1u);
            }
        }
    }

    return result;
}








