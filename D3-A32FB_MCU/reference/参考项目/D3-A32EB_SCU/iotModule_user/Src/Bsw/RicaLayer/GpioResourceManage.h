#ifndef __GPIO_RESOURCE_MANAGE_H_
#define __GPIO_RESOURCE_MANAGE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceSummaryDescribe.h"


#pragma pack(1)

typedef struct
{
#if defined(GD32E50X) || defined(GD32E230)
	E_DIO_RESOURCE_MANAGE   DioResourceName;
    E_GPIO_TYPE_LIST        PinDirect;
	uint32_t	    		GPIOx;
	uint32_t				GPIO_Pin;
	uint32_t			    RCU_GPIOx;
#endif
}STRU_DIO_RESOURCE_MANAGE;

#pragma pack()


void fgv_GpioResourceInit(E_DIO_RESOURCE_MANAGE PinNum, E_GPIO_TYPE_LIST PinDirection, E_BIT_VALUE_LIST InitValue, uint8_t AlternateValue);
void fgv_GpioValueWrite(uint8_t FuncIndex,uint8_t WriteValue);
uint8_t fgu1_DoFuncValueRead(E_DIO_RESOURCE_MANAGE FuncIndex);
uint8_t fgu1_DiFuncValueRead(E_DIO_RESOURCE_MANAGE FuncIndex);
uint8_t fgu1_GpioDoValueRead(uint8_t FuncIndex);
uint8_t fgu1_GpioDiValueRead(uint8_t FuncIndex);

#endif /*__GPIO_RESOURCE_MANAGE_H_*/
