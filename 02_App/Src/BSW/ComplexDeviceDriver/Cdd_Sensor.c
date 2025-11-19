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
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_Sensor.h"

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
	uint16_t arrayAdcValue[CDDSENSOR_CFG_ENV_ADC_BUFF_NUM];	    /* ADC value array*/
} CddSensorEnv_Struct;

typedef struct
{
    uint8_t  tempValue;									/*temperature value*/
	uint16_t arrayAdcValue[CDDSENSOR_CFG_GUN_ADC_BUFF_NUM];	    /* ADC value array*/
} CddSensorGun_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddSensorEnv_Struct 		g_stSensorEnv;
static CddSensorGun_Struct 		g_stSensorGun;
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

static uint8_t CddSensor_EnvAdcValueToTemperature(uint16_t adcValue);
static uint8_t CddSensor_GunAdcValueToTemperature(uint16_t adcValue);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t CddSensor_EnvAdcValueToTemperature(uint16_t adcValue)
{
    uint8_t temperature = 0;
    uint8_t index = CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM;

    temperature = c_stSensorAdcMapEnvNTC[index - 1].tempVal;/* 默认未查询到为最大温度 */

    for (index = 1; index < CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM; index++)
    {
        if ((c_stSensorAdcMapEnvNTC[index].adcVal) >= adcValue)
        {
            if ( 2* adcValue > (c_stSensorAdcMapEnvNTC[index].adcVal + c_stSensorAdcMapEnvNTC[index-1].adcVal))
            {
                temperature = c_stSensorAdcMapEnvNTC[index].tempVal;
            }
            else
            {
                temperature = c_stSensorAdcMapEnvNTC[index-1].tempVal;
            }
            break;
        }
    }
    
    return temperature;
}

static uint8_t CddSensor_GunAdcValueToTemperature(uint16_t adcValue)
{
    uint8_t temperature = 0;
    uint8_t index = 0;

    temperature = c_stSensorAdcMapGunNTC[index].tempVal;

    for (index = CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM - 1; index > 0; index--)
    {
        if ((c_stSensorAdcMapGunNTC[index].adcVal) >= adcValue)
        {
            if ( 2* adcValue > (c_stSensorAdcMapGunNTC[index].adcVal + c_stSensorAdcMapGunNTC[index-1].adcVal))
            {
                temperature = c_stSensorAdcMapGunNTC[index].tempVal;
            }
            else
            {
                temperature = c_stSensorAdcMapGunNTC[index-1].tempVal;
            }
            break;
        }
    }

    return temperature;
}

void CddSensor_InitMemory(void)
{
    memset((uint8_t *)&g_stSensorEnv, 0, sizeof(g_stSensorEnv));
    memset((uint8_t *)&g_stSensorGun, 0, sizeof(g_stSensorGun));
}

uint8_t CddSensor_GetEnvTempValue(uint8_t ucPort)
{
	return g_stSensorEnv.tempValue;
}

uint8_t CddSensor_GetGunTempValue(uint8_t ucPort)
{
	return g_stSensorGun.tempValue;
}

void CddSensor_MainFunction(void)
{
    uint16_t temp = 0;

    McalADC_GetChannelData(eMcalADCChanel_EnvNtc, g_stSensorEnv.arrayAdcValue, CDDSENSOR_CFG_ENV_ADC_BUFF_NUM);
    temp = Common_MedianU16Filter(g_stSensorEnv.arrayAdcValue, CDDSENSOR_CFG_ENV_ADC_BUFF_NUM, CDDSENSOR_CFG_ENV_ADC_BUFF_NUM / 2);
    g_stSensorEnv.tempValue = CddSensor_EnvAdcValueToTemperature(temp);

    McalADC_GetChannelData(eMcalADCChanel_GunNTC, g_stSensorGun.arrayAdcValue, CDDSENSOR_CFG_GUN_ADC_BUFF_NUM);
    temp = Common_MedianU16Filter(g_stSensorGun.arrayAdcValue, CDDSENSOR_CFG_GUN_ADC_BUFF_NUM, CDDSENSOR_CFG_GUN_ADC_BUFF_NUM / 2);
    g_stSensorGun.tempValue = CddSensor_GunAdcValueToTemperature(temp);
}



