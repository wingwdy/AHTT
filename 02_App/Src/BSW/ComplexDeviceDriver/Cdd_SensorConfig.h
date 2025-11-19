
/******************************************************************************
* File Name          : Cdd_SensorConfig.h
* Description        : Code for xxxxxxxxxxx
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
******************************************************************************/
#ifndef CDD_SENSOR_CONFIG_H_
#define CDD_SENSOR_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Adc.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDSENSOR_CFG_ENV_ADC_BUFF_NUM 	    	    (MCALADC_ADC0_SAMPLE_CNT)    /*Env ADC sample buffer number */
#define CDDSENSOR_CFG_GUN_ADC_BUFF_NUM 	    	    (MCALADC_ADC0_SAMPLE_CNT)    /*Gun ADC sample buffer number */
#define CDDSENSOR_CFG_ADC_MIN_SAMPLE_NUM 	    	(3u)    /*ADC min sample number */

#define CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM	        (166u)
#define CDDSENSOR_CFG_GUN_NTC_ADC_MAP_NUM	        (166u)


/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t  tempVal;      
    uint16_t adcVal;
} CddSensorAdcMap_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern const CddSensorAdcMap_Struct c_stSensorAdcMapEnvNTC[CDDSENSOR_CFG_ENV_NTC_ADC_MAP_NUM];
extern const CddSensorAdcMap_Struct c_stSensorAdcMapGunNTC[CDDSENSOR_CFG_GUN_NTC_ADC_MAP_NUM];


#endif /* CDD_SENSOR_CONFIG_H_ */

