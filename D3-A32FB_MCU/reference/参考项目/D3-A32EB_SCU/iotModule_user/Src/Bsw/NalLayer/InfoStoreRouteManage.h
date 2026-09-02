#ifndef __INFO_STORE_ROUTE_MANAGE_H_
#define __INFO_STORE_ROUTE_MANAGE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "FunctionalHeaderSummary.h"
#include "FuncExternHeaderSummary.h"


#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
    APP_FIRST_STORE_MEDIUM = 0,
    APP_STORE_MEDIUM_INTERNAL_FLASH = 0,
    APP_STORE_MEDIUM_SPI_FLASH,
    APP_MAX_STORE_MEDIUM_NUMBER,
}E_APP_STORE_MEDIUM_LIST;

typedef struct
{
    E_APP_STORE_MEDIUM_LIST StoreMediumType;
    E_SPI_CHANNEL_LIST      SpiChannelIndex;
    E_DIO_RESOURCE_MANAGE   CsPinNum;
    E_DIO_RESOURCE_MANAGE   SclkPinNum;
    E_DIO_RESOURCE_MANAGE   MisoPinNum;
    E_DIO_RESOURCE_MANAGE   MosiPinNum;
}STRU_EXTERNAL_MEMORY_RESOURCE_DISTRUB;

typedef struct
{
    E_APP_STORE_MEDIUM_LIST StoreMediumType;
    uint8_t                 (*WriteFunc)(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t WriteAddr,const uint8_t *pBuffer, uint32_t NumToWrite);
    uint8_t                 (*ReadFunc)(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t ReadAddr,uint8_t *pBuffer, uint32_t NumToRead);
    uint8_t                 (*EraseFunc)(E_APP_OPERATE_TYPE_LIST OperateType, uint32_t Addr);
}STRU_STORE_APP_FUNC_ROUTE_MANAGE;


void fgv_AppInfoStoreInit(void);
uint8_t fgu8_AppInfoStoreWriteRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t WriteAddr, const uint8_t *pBuffer, uint32_t NumToWrite);
uint8_t fgu8_AppInfoStoreReadRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t ReadAddr, uint8_t *pBuffer, uint32_t NumToRead);
uint8_t fgu8_AppInfoStoreEraseRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t Addr);

#ifdef __cplusplus
}
#endif

void fgv_AppSpiInit(void);


#endif /* __INFO_STORE_ROUTE_MANAGE_H_ */
