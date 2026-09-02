#ifndef	__GPIO_FUNCTIONAL_MANAGE_H_
#define	__GPIO_FUNCTIONAL_MANAGE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceHeaderSummary.h"


void fgv_GpioFuncInit(E_DIO_RESOURCE_MANAGE PinNum,E_GPIO_TYPE_LIST PinDirection,E_BIT_VALUE_LIST InitValue,uint8_t AlternateValue);
void fgv_DoFuncValueWrite(uint8_t FuncIndex,uint8_t WriteValue);
uint8_t fgu1_DoFuncValueRead(E_DIO_RESOURCE_MANAGE FuncIndex);
uint8_t fgu1_DiFuncValueRead(E_DIO_RESOURCE_MANAGE FuncIndex);


#endif /*__GPIO_FUNCTIONAL_MANAGE_H_*/
