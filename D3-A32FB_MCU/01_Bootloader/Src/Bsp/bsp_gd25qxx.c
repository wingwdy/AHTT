/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_gd25qxx.h"


#define FLASH_GD25Q80E_SIZE       (1*1024*1024UL)
#define FLASH_GD25QXXE_SIZE       FLASH_GD25Q80E_SIZE

#ifndef 	NULL
#define 	NULL	 0
#endif

//返回值：3超时
static uint8_t bsp_gd25q_wait_for_complited(uint32_t u32_TimeoutMs)
{
    E_SPI_ERR spi_err = SPI_NO_ERR;
	uint32_t i = 0;
    uint16_t rbyte = GD25Q_STAT0_WIP;

	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_RDSR, NULL);

	while((rbyte & GD25Q_STAT0_WIP))
	{
		bsp_delay_ms(1);
        spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, &rbyte);
		if (i++ > u32_TimeoutMs)
        {
            bsp_spi_cs_disable();
            return 3;
        }
	}
	bsp_spi_cs_disable();

    return 0;
}

uint8_t bsp_gd25q_read(uint32_t u32_Addr, uint8_t *pu8_Buf, uint32_t u32_Len)
{
    E_SPI_ERR spi_err = SPI_NO_ERR;

    //参数检查
	if (u32_Addr > FLASH_GD25QXXE_SIZE)
	{
#ifdef LOGRUNNING
		Log_printf(0, 0, LVL_LOG_ERR, "Addr Err");
#endif
		return 1;
	}
	//
#ifdef LOGRUNNING
    Log_printf(0, 0, LVL_LOG_INFO, "Block Read: 0x%08X 0x%04X",ulAddr,uLen);
#endif

	//发送READ指令
    bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_READ, NULL);
	//发送地址
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>16), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>8), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr), NULL);
	//读取数据
    uint16_t rbyte = 0;
	while (u32_Len != 0)
	{
        spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr), &rbyte);
		*(pu8_Buf++) = rbyte;
		u32_Len--;
	}
    bsp_spi_cs_disable();
    /*休息一下*/
    spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);

	return 0;
}

uint8_t bsp_gd25q_write_page(uint32_t u32_Addr, const uint8_t *pu8_Buf, uint32_t u32_Len)
{
    E_SPI_ERR spi_err = SPI_NO_ERR;
    //参数检查
	if (u32_Addr > FLASH_GD25QXXE_SIZE)
	{
#ifdef LOGRUNNING
		Log_printf(0, 0, LVL_LOG_ERR, "Addr Err");
#endif
		return 1;
	}

	if ((u32_Len + (uint8_t)u32_Addr) > 256)
	{
#ifdef LOGRUNNING
		Log_printf(0, 0, LVL_LOG_ERR, "Lenth Err");
#endif
		return 2;
	}
	//
#ifdef LOGRUNNING
    Log_print(0, 0, LVL_LOG_INFO, "Page Write: 0x%08X 0x%04X", u32_Addr, u16_Len);
#endif
    //发送WREN指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WREN, NULL);
    if(SPI_NO_ERR != spi_err)
    {
#ifdef LOGRUNNING
        Log_printf(0, 0, LVL_LOG_ERR, "Send WREN Err");
#endif
    }
    bsp_spi_cs_disable();
    /*休息一下*/
    spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
    //发送PP指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_PP, NULL);
	//发送地址
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>16), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>8), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr), NULL);
	//发送数据
	while (u32_Len != 0)
	{
        spi_err = bsp_spi_rw_byte(*(pu8_Buf++), NULL);
		u32_Len--;
	}
	bsp_spi_cs_disable();
    /*休息一下*/
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
	uint8_t err = bsp_gd25q_wait_for_complited(GD25Q_tPP_MS);
	if (err == 3)
	{
#ifdef LOGRUNNING
		Log_printf(0, 0, LVL_LOG_ERR, "Process Wait Timeout");
#endif
		return 3;
	}

	//发送WRDI指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WRDI, NULL);
	bsp_spi_cs_disable();
    /*休息一下*/
    spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);

	return 0;
}

uint8_t bsp_gd25q_sector_erase(uint32_t u32_Addr)
{
    E_SPI_ERR spi_err = SPI_NO_ERR;

    //参数检查
	if (u32_Addr > FLASH_GD25QXXE_SIZE)
	{
		return 1;
	}
	//
#ifdef LOGRUNNING
    Log_printf(0, 0, LVL_LOG_INFO, "Sector erase: 0x%08X", u32_Addr);
#endif

	//发送WREN指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WREN, NULL);
    if(spi_err != SPI_NO_ERR)
    {
#ifdef LOGRUNNING
        Log_printf(0, 0, LVL_LOG_ERR, "Send WREN err");
#endif
    }
	bsp_spi_cs_disable();
    /*休息一下*/
    spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
    bsp_spi_cs_enable();
	//发送SE指令
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_SE, NULL);
	//发送地址
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>16), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr>>8), NULL);
    spi_err = bsp_spi_rw_byte((uint8_t)(u32_Addr), NULL);
	bsp_spi_cs_disable();
    /*休息一下*/
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
	uint8_t err = bsp_gd25q_wait_for_complited(GD25Q_tSE_MS);
	if (err == 3)
	{
		return 3;
	}
	
	//发送WRDI指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WRDI, NULL);
	bsp_spi_cs_disable();
    /*休息一下*/
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);

	return 0;
}

uint8_t bsp_gd25q_block_erase(void)
{
    E_SPI_ERR spi_err = SPI_NO_ERR;
	//
#ifdef LOGRUNNING
	Log_print(0, 0, LVL_LOG_INFO, "Block erase");
#endif
	//发送WREN指令
    bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WREN, NULL);
    bsp_spi_cs_disable();
    /*休息一下*/
    spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
    //发送BE指令
	bsp_spi_cs_enable();
	spi_err = bsp_spi_rw_byte(GD25Q_CMD_BE, NULL);
    bsp_spi_cs_disable();
    /*休息一下*/
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);
	
	uint8_t err = bsp_gd25q_wait_for_complited(GD25Q_tBE2_MS);
	if (err == 3)
	{
		return 3;
	}
	
	//发送WRDI指令
	bsp_spi_cs_enable();
    spi_err = bsp_spi_rw_byte(GD25Q_CMD_WRDI, NULL);
	bsp_spi_cs_disable();
    /*休息一下*/
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, NULL);

	return 0;
}

uint8_t bsp_gd25q_read_id(uint32_t *p_uid)
{
	E_SPI_ERR spi_err = SPI_NO_ERR;
	uint16_t temp[3] = {0};
	
	bsp_spi_cs_enable();
	spi_err = bsp_spi_rw_byte(GD25Q_CMD_JEDEC_ID, NULL);
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, &temp[0]);
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, &temp[1]);
	spi_err = bsp_spi_rw_byte(GD25Q_DUMMY, &temp[2]);
	
	bsp_spi_cs_disable();
	*p_uid = ((uint32_t)temp[0] << 16) | (temp[1] << 8) | temp[2];
	
	return spi_err;
}
