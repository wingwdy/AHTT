#include "bsp_flash.h"


#if(FLASH_TYPE == FLASH_GD25QXX)

uint8_t bsp_flash_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len)
{
    return bsp_gd25q_read(u32_Addr, pu8_Buf, u32_Len);
}

uint8_t bsp_flash_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len)
{
    return bsp_gd25q_write_page(u32_Addr, pu8_Buf, u32_Len);
}

uint8_t bsp_flash_sector_erase(uint32_t u32_Addr)
{
    return bsp_gd25q_sector_erase(u32_Addr);
}

uint8_t bsp_flash_block_erase(void)
{
    return bsp_gd25q_block_erase();
}

uint8_t bsp_flash_read_id(uint32_t *p_uid)
{
    return bsp_gd25q_read_id(p_uid);
}


#elif(FLASH_TYPE == FLASH_W25QXX)

uint8_t bsp_flash_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len)
{
    return bsp_w25q_read(u32_Addr, pu8_Buf, u32_Len);
}

uint8_t bsp_flash_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len)
{
    return bsp_w25q_write_page(u32_Addr, pu8_Buf, u32_Len);
}

uint8_t bsp_flash_sector_erase(uint32_t u32_Addr)
{
    return bsp_w25q_sector_erase(u32_Addr);
}

uint8_t bsp_flash_block_erase(void)
{
    return bsp_w25q_block_erase();
}

#endif
