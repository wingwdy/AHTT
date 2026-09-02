/******************************************************************************
* File Name          : Cdd_Drv_LS5120Config.c
* Description        : Code for Configuration of LS5120 interface
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Mcal_Port.h"
#include "Mcal_SPI.h"
#include "Global.h"
#include "Cdd_Drv_LS5120Config.h"
#include "Cdd_Drv_LS5120.h" 


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
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void CddDrvLS5120_SPI_Delay(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
	    __NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
	}
}

/* SPI读数据，时钟小于10Mbit/S */
uint8_t CddDrvLS5120_SPI_Read(void)
{
    uint8_t byte = 0;
    uint8_t i = 0;

    CDD_DRV_CFG_LS5120_SPI_SCLK_LOW();
    for (i = 0; i < 8; i++)
    {
		byte <<= 1;
        CddDrvLS5120_SPI_Delay();
        CDD_DRV_CFG_LS5120_SPI_SCLK_HIGH();
        CddDrvLS5120_SPI_Delay();
        if (CDD_DRV_CFG_LS5120_SPI_MISO_VALUE())
        {
            byte |= 0x01;
        }
        CDD_DRV_CFG_LS5120_SPI_SCLK_LOW();
    }
	CddDrvLS5120_SPI_Delay();
    return byte;   
}

void CddDrvLS5120_SPI_Write(uint8_t byte)
{
    uint8_t data = byte;
    uint8_t i = 0;

    CDD_DRV_CFG_LS5120_SPI_SCLK_LOW();
    for (i = 0; i < 8; i++)
    {
        CddDrvLS5120_SPI_Delay();
        if (data & 0x80)
        {
            CDD_DRV_CFG_LS5120_SPI_MOSI_HIGH();
        }
        else
        {
            CDD_DRV_CFG_LS5120_SPI_MOSI_LOW(); 
        }
        CDD_DRV_CFG_LS5120_SPI_SCLK_HIGH();
        CddDrvLS5120_SPI_Delay();
        CDD_DRV_CFG_LS5120_SPI_SCLK_LOW();
        data <<= 1;
    }
    CDD_DRV_CFG_LS5120_SPI_MOSI_HIGH();
	CddDrvLS5120_SPI_Delay();
}

void CddDrvLS5120_CHIP_HardwareResetStart(void)
{
    CDD_DRV_CFG_LS5120_RESET_LOW();
}

void CddDrvLS5120_CHIP_HardwareResetEnd(void)
{
    CDD_DRV_CFG_LS5120_RESET_HIGH();
}

