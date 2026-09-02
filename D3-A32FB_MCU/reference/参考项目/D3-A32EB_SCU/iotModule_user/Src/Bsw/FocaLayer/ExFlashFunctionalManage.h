#ifndef	__EXFLASH_FUNCTIONAL_MANAGE_H_
#define	__EXFLASH_FUNCTIONAL_MANAGE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes-----------------------------------------------------------------------------------*/
#include "FunctionalHeaderSummary.h"
#include "FlashW25qDriver.h"
#include "FlashGD25qDriver.h"

void fgv_ExFlashFuncHardwareInit(E_SPI_CHANNEL_LIST SpiChannel, 
                                E_DIO_RESOURCE_MANAGE CsPinNum,
                                E_DIO_RESOURCE_MANAGE SclkPinNum,
                                E_DIO_RESOURCE_MANAGE MisoPinNum,
                                E_DIO_RESOURCE_MANAGE MosiPinNum);

uint8_t fgu8_ExFlashFuncWrite(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t WriteAddr, const uint8_t *pBuffer, uint32_t NumToWrite);
uint8_t fgu8_ExFlashFuncRead(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t ReadAddr, uint8_t *pBuffer, uint32_t NumToRead);
uint8_t fgu8_ExFlashFuncErase(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t u32_Addr);

#ifdef __cplusplus
}
#endif

#endif /*__EXFLASH_FUNCTIONAL_MANAGE_H_*/
