#ifndef __BSP_FLASH_H_
#define __BSP_FLASH_H_


#include "bsp_gd25qxx.h"

#include "bsp_w25qxx.h"


#define FLASH_GD25QXX       0
#define FLASH_W25QXX        1
#define FLASH_TYPE          FLASH_GD25QXX//FLASH_W25QXX


uint8_t bsp_flash_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len);

uint8_t bsp_flash_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len);

uint8_t bsp_flash_sector_erase(uint32_t u32_Addr);

uint8_t bsp_flash_block_erase(void);

uint8_t bsp_flash_read_id(uint32_t *p_cid);

#endif /* __BSP_FLASH_H_ */
