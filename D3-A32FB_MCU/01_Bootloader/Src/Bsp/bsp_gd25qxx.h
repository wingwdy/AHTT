#ifndef __BSP_GD25QXX_H_
#define __BSP_GD25QXX_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_spi.h"
#include "bsp_delay.h"


/*=====================================================================================
GD25Q80E
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
#define GD25Q_DUMMY               0xA5      // 读数据使用 
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


uint8_t bsp_gd25q_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len);
uint8_t bsp_gd25q_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len);
uint8_t bsp_gd25q_sector_erase(uint32_t u32_Addr);
uint8_t bsp_gd25q_block_erase(void);
uint8_t bsp_gd25q_read_id(uint32_t *p_cid);

#endif /* __BSP_GD25QXX_H_ */
