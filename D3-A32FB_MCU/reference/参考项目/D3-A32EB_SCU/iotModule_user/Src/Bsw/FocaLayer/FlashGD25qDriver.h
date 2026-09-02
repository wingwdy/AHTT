#ifndef	__FLASH_GD25Q_DRIVER_H_
#define	__FLASH_GD25Q_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes-----------------------------------------------------------------------------------*/
#include "FunctionalHeaderSummary.h"

/*=====================================================================================
W25Q64
tPUW >= 5ms 上电10ms才能读写操作
=====================================================================================*/
#define	WM_READBACK	             0x01	   //需回读

/*time*/
#define GD25Q_tW_MS               30        //典型值10ms，最大值15ms
#define GD25Q_tPP_MS              2         //典型值0.4ms，最大值2ms
#define GD25Q_tSE_MS              300       //典型值45ms，最大值300ms
#define GD25Q_tBE1_MS             1200      //典型值150ms，最大值1200ms
#define GD25Q_tBE2_MS             1600      //典型值250ms，最大值1600ms
#define GD25Q_tCE_MS              10000     //典型值3s，最大值10s

/*status*/
#define GD25Q_STAT0_WIP           0x01      //Erase/Write In Progress

/*cmd*/
#define GD25Q_DUMMY               0xFF      // 读数据使用 
#define GD25Q_CMD_WREN            0x06      // Write Enable
#define GD25Q_CMD_WRDI            0x04      // Write Disable
#define GD25Q_CMD_READ            0x03      // Read Data
#define GD25Q_CMD_READ_FAST       0x0B      // Fast Read
#define GD25Q_CMD_PP              0x02      // Page Program (256B)
#define GD25Q_CMD_SE              0x20      // Sector Erase
#define GD25Q_CMD_BE              0xD8      // Block Erase (64K)
#define GD25Q_CMD_CE              0xC7      // Chip Erase 
#define GD25Q_CMD_RDSR            0x05      // Read Status Register(S7~S0)
#define GD25Q_CMD_JEDEC_ID        0x9F      // JEDEC_ID

#define FLASH_GD25Q80E_SIZE       (1*1024*1024UL)
#define FLASH_GD25QXXE_SIZE       FLASH_GD25Q80E_SIZE

#pragma pack(1)
typedef struct
{
    E_SPI_CHANNEL_LIST      SpiChannel;
    E_DIO_RESOURCE_MANAGE   CsPinNum;
    uint8_t                 FlashBusy;
}STRU_FLASH_GD25Q_OP_CTRL;
#pragma pack()

void fgv_FlashGD25q_FuncInit(E_SPI_CHANNEL_LIST SpiChannel, E_DIO_RESOURCE_MANAGE CsPinNum);
uint8_t fgu8_FlashGD25q_Read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len);
uint8_t fgu8_FlashGD25q_WritePage(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len, uint8_t u8_Mode);
uint8_t fgu8_FlashGD25q_SectorErase(uint32_t u32_Addr);
uint8_t fgu8_FlashGD25q_BlockErase(uint32_t u32_Addr);

#ifdef __cplusplus
}
#endif

#endif /*__FLASH_GD25Q_DRIVER_H_*/