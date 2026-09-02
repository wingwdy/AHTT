#include "InfoStoreRouteManage.h"

STRU_STORE_APP_FUNC_ROUTE_MANAGE StoreRouteTable[APP_MAX_STORE_MEDIUM_NUMBER] = 
{
    {
        .StoreMediumType = APP_STORE_MEDIUM_INTERNAL_FLASH,
        .WriteFunc = NULL,
        .ReadFunc  = NULL,
        .EraseFunc = NULL,
    },
    {
        .StoreMediumType = APP_STORE_MEDIUM_SPI_FLASH,   
        .WriteFunc = fgu8_ExFlashFuncWrite,    
        .ReadFunc = fgu8_ExFlashFuncRead,
        .EraseFunc = fgu8_ExFlashFuncErase,
    },    
};


static const STRU_EXTERNAL_MEMORY_RESOURCE_DISTRUB ExternalMemoryResource[] = 
{
    {
        .StoreMediumType = APP_STORE_MEDIUM_SPI_FLASH,
        .SpiChannelIndex = E_SPI1_INDEX,  
        .CsPinNum   = E_DIO_PIN20,
        .SclkPinNum = E_DIO_PIN21,
        .MisoPinNum = E_DIO_PIN22,
        .MosiPinNum = E_DIO_PIN23
    }
};

void fgv_AppInfoStoreInit(void)
{
	E_APP_STORE_MEDIUM_LIST AppStoreIndex = APP_FIRST_STORE_MEDIUM;
	
	for(AppStoreIndex = APP_FIRST_STORE_MEDIUM; AppStoreIndex < APP_MAX_STORE_MEDIUM_NUMBER; AppStoreIndex++)
	{
	    if(AppStoreIndex == APP_STORE_MEDIUM_SPI_FLASH)
		{
			fgv_ExFlashFuncHardwareInit(ExternalMemoryResource[0].SpiChannelIndex,
										ExternalMemoryResource[0].CsPinNum,
										ExternalMemoryResource[0].SclkPinNum,
										ExternalMemoryResource[0].MisoPinNum,
										ExternalMemoryResource[0].MosiPinNum);
		}
	}
}

uint8_t fgu8_AppInfoStoreWriteRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t WriteAddr, const uint8_t *pBuffer, uint32_t NumToWrite)
{
    uint8_t err = 0;
    if(AppIndex >= APP_MAX_STORE_MEDIUM_NUMBER)
    {
        return 1;
    }
    if(NULL == StoreRouteTable[AppIndex].WriteFunc)
    {
        return 2;
    }

    err = StoreRouteTable[AppIndex].WriteFunc(OperateType, WriteAddr, pBuffer, NumToWrite);

    return err;
}

uint8_t fgu8_AppInfoStoreReadRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t ReadAddr, uint8_t *pBuffer, uint32_t NumToRead)
{
    uint8_t err = 0;
    if(AppIndex >= APP_MAX_STORE_MEDIUM_NUMBER)
    {
        return 1;
    }
    if(NULL == StoreRouteTable[AppIndex].ReadFunc)
    {
        return 2;  
    }
    err = StoreRouteTable[AppIndex].ReadFunc(OperateType, ReadAddr, pBuffer, NumToRead);
    return err;

}

uint8_t fgu8_AppInfoStoreEraseRoute(E_APP_STORE_MEDIUM_LIST AppIndex, E_APP_OPERATE_TYPE_LIST OperateType, uint32_t Addr)
{
    uint8_t err = 0;
    if(AppIndex >= APP_MAX_STORE_MEDIUM_NUMBER)
    {
        return 1;
    }
    if(NULL == StoreRouteTable[AppIndex].EraseFunc)
    {
        return 2;  
    }

    err = StoreRouteTable[AppIndex].EraseFunc(OperateType, Addr);

    return err;

}






