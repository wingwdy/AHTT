/******************************************************************************
* File Name          : Cdd_Sensor.c
* Description        : Code for xxxxxxxxxxx
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      shenjc    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_Sensor.h"
#include "SysCfg.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint8_t  tempValue;									/*temperature value*/
	uint16_t adcValue[CDDSENSOR_CFG_ADC_BUFF_NUM];	    /* ADC value array*/
} CddSensor_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddSensor_Struct 		g_stSensorEnv;
static CddSensor_Struct 		g_stSensorGun[SYSCFG_CFG_GUN_NUM];
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

static uint8_t CddSensor_SearchEnvTemperature(uint16_t adcValue);
static uint8_t CddSensor_SearchGunTemperature(uint16_t adcValue);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t CddSensor_SearchEnvTemperature(uint16_t adcValue)
{
    uint8_t index = 0;
    uint8_t left = 0;
    uint8_t right = CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM - 1;
    uint8_t mid = 0;
    uint8_t find = 0;
    uint16_t midAdc = 0;

    /* 处理边界情况 */
    if (adcValue <= c_stSensorAdcMapEnvNTC[0].adcVal)
    {
        index = 0;
    }
    else if (adcValue >= c_stSensorAdcMapEnvNTC[CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM - 1].adcVal)
    {
        index = CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM - 1;
    }
    else
    {
        /* 二分查找 从小到大排序 */
        while (left <= right)
        {
            mid = left + (right - left) / 2;
            midAdc = c_stSensorAdcMapEnvNTC[mid].adcVal;
            if (midAdc == adcValue)
            {
                find = 1;
                break;
            }
            else if(midAdc < adcValue)
            {/* 中间值小于目标值，目标值在右半部分 */
                left = mid + 1;
            }
            else
            {/* 中间值大于目标值，目标值在左半部分 */
                right = mid - 1;
            }
        }

        if (find)
        {
            index = mid;
        }
        else
        {
            int lower_adc = c_stSensorAdcMapEnvNTC[right].adcVal;
            int upper_adc = c_stSensorAdcMapEnvNTC[left].adcVal;
            if (abs(adcValue - lower_adc) <= abs(upper_adc - adcValue))
            {
                index = right;
            }
            else
            {
                index = left;
            }
        }
    }

    return c_stSensorAdcMapEnvNTC[index].tempVal;
}

static uint8_t CddSensor_SearchGunTemperature(uint16_t adcValue)
{
    uint8_t index = 0;
    uint8_t left = 0;
    uint8_t right = CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM - 1;
    uint8_t mid = 0;
    uint8_t find = 0;
    uint16_t midAdc = 0;

    /* 处理边界情况 */
    if (adcValue <= c_stSensorAdcMapGunNTC[0].adcVal)
    {
        index = CDDSENSOR_CFG_GUN_NTC_ADC_MAP_NUM - 1;
    }
    else if (adcValue >= c_stSensorAdcMapGunNTC[CDDSENSOR_CFG_GUN_NTC_ADC_MAP_NUM - 1].adcVal)
    {
        index = 0;
    }
    else
    {
        /* 二分查找 从大到小排序 */
        while (left <= right)
        {
            mid = left + (right - left) / 2;
            midAdc = c_stSensorAdcMapGunNTC[mid].adcVal;
            if (midAdc == adcValue)
            {
                find = 1;
                break;
            }
            else if(midAdc > adcValue)
            {/* 中间值大于目标值，目标值在右半部分 */
                left = mid + 1;
            }
            else
            {/* 中间值小于目标值，目标值在左半部分 */
                right = mid - 1;
            }
        }

        if (find)
        {
            index = mid;
        }
        else
        {
            int lower_adc = c_stSensorAdcMapGunNTC[left].adcVal;
            int upper_adc = c_stSensorAdcMapGunNTC[right].adcVal;
            if (abs(adcValue - lower_adc) <= abs(upper_adc - adcValue))
            {
                index = right;
            }
            else
            {
                index = left;
            }
        }
    }

    return c_stSensorAdcMapGunNTC[index].tempVal;
}

void CddSensor_InitMemory(void)
{
    memset((uint8_t *)&g_stSensorEnv, 0, sizeof(g_stSensorEnv));
    memset((uint8_t *)&g_stSensorGun, 0, sizeof(g_stSensorGun));
}

uint8_t CddSensor_GetEnvTemperature(void)
{
	return g_stSensorEnv.tempValue;
}

uint8_t CddSensor_GetGunTempTemperature(uint8_t port)
{   
    uint8_t i = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        i = port;
    }

	return g_stSensorGun[i].tempValue;
}

void CddSensor_MainFunction(void)
{
    uint8_t i = 0;
    uint16_t temp = 0;

    McalADC_GetChannelData(eMcalADCChanel_EnvNtc, g_stSensorEnv.adcValue, CDDSENSOR_CFG_ADC_BUFF_NUM);
    temp = Common_MedianU16Filter(g_stSensorEnv.adcValue, CDDSENSOR_CFG_ADC_BUFF_NUM, CDDSENSOR_CFG_ADC_BUFF_NUM / 2);
    g_stSensorEnv.tempValue = CddSensor_SearchEnvTemperature(temp);

    for(i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
    {
        McalADC_GetChannelData(eMcalADCChanel_GunNTC, g_stSensorGun[i].adcValue, CDDSENSOR_CFG_ADC_BUFF_NUM);
        temp = Common_MedianU16Filter(g_stSensorGun[i].adcValue, CDDSENSOR_CFG_ADC_BUFF_NUM, CDDSENSOR_CFG_ADC_BUFF_NUM / 2);
        g_stSensorGun[i].tempValue = CddSensor_SearchGunTemperature(temp);
    }
}



