#ifndef __BSP_W25QXX_H_
#define __BSP_W25QXX_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_spi.h"
#include "bsp_delay.h"


/*=====================================================================================
W25Q64
tPUW >= 5ms 上电10ms才能读写操作
=====================================================================================*/
#define	WM_READBACK	             0x01	   //需回读

/*time*/
#define W25Q_tW_MS               (15+5)        //典型值10ms，最大值15ms
#define W25Q_tPP_MS              (3+5)         //典型值0.8ms，最大值3ms
#define W25Q_tSE_MS              400       //典型值45ms，最大值400ms
#define W25Q_tBE1_MS             1600      //典型值120ms，最大值1600ms
#define W25Q_tBE2_MS             2000      //典型值150ms，最大值2000ms

/*cmd*/
#define W25Q_STAT0_BUSY          0x01
#define W25Q_DUMMY               0xFF      // 读数据使用 
#define W25Q_CMD_WREN            0x06      // 写使能
#define W25Q_CMD_WRDI            0x04      // 写失能
#define W25Q_CMD_READ            0x03      // Read Data
#define W25Q_CMD_READ_FAST       0x0B      // Fast Read
#define W25Q_CMD_PP              0x02      // Page Program(256B)
#define W25Q_CMD_SE              0x20      // Sector Erase(4KB)
#define W25Q_CMD_BE              0xD8      // Block Erase(64KB)
#define W25Q_CMD_CE              0xC7      // Chip Erase 
#define W25Q_CMD_RDSR            0x05      // 读状态寄存器(S7~S0)
#define W25Q_CMD_JEDEC_ID        0x9F      // JEDEC_ID


uint8_t bsp_w25q_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len);
uint8_t bsp_w25q_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len);
uint8_t bsp_w25q_sector_erase(uint32_t u32_Addr);
uint8_t bsp_w25q_block_erase(void);


#endif /* __BSP_W25QXX_H_ */
