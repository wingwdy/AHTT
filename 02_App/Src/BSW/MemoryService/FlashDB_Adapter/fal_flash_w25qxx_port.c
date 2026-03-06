/******************************************************************************
* File Name          : fal_flash_norflash_port.c
* Description        : Code for The function of adapting to NOR flash
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include <fal.h>
#include "Global.h"
#include "W25QXX.h"
#include "Mcal_IWDG.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const struct fal_flash_dev nor_flash0 =
{
    .name       = NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = NOR_FLASH_DEV_SIZE,
    .blk_size   = 4 * 1024,
    .ops        = {
    		.init = init,
    		.read = read,
    		.write = write,
    		.erase = erase
    },
    .write_gran = 1
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static int init(void)
{
    int ret = -1;

    if (eGlobalRet_OK == W25Q_Init())
    {
        ret = 1;
    }
    return ret;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    uint32_t addr = nor_flash0.addr + offset;
    int ret = -1;

    if (eGlobalRet_OK == W25Q_Read(addr, buf, size))
    {
        ret = 1;
    }

    return ret;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr = nor_flash0.addr + offset;
    int ret = -1;
    
    if (eGlobalRet_OK == W25Q_Write(addr, buf, size))
    {
        ret = 1;
    }

    return ret;
}

static int erase(long offset, size_t size)
{
    uint32_t addr = nor_flash0.addr + offset;
    int ret = -1;

    if (eGlobalRet_OK == W25Q_Erase(addr, size))
    {
        ret = 1;
    }

    return ret;
}








