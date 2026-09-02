#ifndef	__SPI_FUNCTIONAL_MANAGE_H_
#define	__SPI_FUNCTIONAL_MANAGE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceHeaderSummary.h"

void fgv_SpiFuncInit(E_SPI_CHANNEL_LIST SpiChannel, E_DIO_RESOURCE_MANAGE CsPinNum, E_DIO_RESOURCE_MANAGE SclkPinNum, E_DIO_RESOURCE_MANAGE MisoPinNum, E_DIO_RESOURCE_MANAGE MosiPinNum);
E_SPI_ERR fge_SpiFuncWriteRead(E_SPI_CHANNEL_LIST SpiChannel, uint16_t u16_wbyte, uint16_t* pu16_rbyte);
void fgv_SpiFuncCSEnable(E_DIO_RESOURCE_MANAGE CsPinNum, uint8_t EnableFlag);

#ifdef __cplusplus
}
#endif

#endif /*__SPI_FUNCTIONAL_MANAGE_H_*/
