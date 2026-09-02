
#ifndef __SPI_RESOURCE_MANAGE_H__
#define __SPI_RESOURCE_MANAGE_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceSummaryDescribe.h"


typedef enum
{
    SPI_NO_ERR = 0,
    SPI_GPIO_INIT_ERR,
    SPI_NUM_ERR,
    SPI_WR_WAIT_ERR,
    SPI_RD_WAIT_ERR,
    SPI_NO_INITED_ERR,
}E_SPI_ERR;

#if defined(GD32E230)

#elif defined(GD32E50X)

typedef enum
{
    E_FIRST_SPI_INDEX = 0,
    E_SPI1_INDEX = 0,
    E_SPI2_INDEX,
    E_SPI3_INDEX,
    E_MAX_SPI_NUMBER,
}E_SPI_CHANNEL_LIST;

#endif

#pragma pack(1)

typedef struct
{
    E_DIO_RESOURCE_MANAGE   SclkPinNum;
    E_DIO_RESOURCE_MANAGE   MisoPinNum;
    E_DIO_RESOURCE_MANAGE   MosiPinNum;
    E_SPI_CHANNEL_LIST      SpiChannelIndex;
#if defined(GD32E230) || defined (GD32E50X)
    uint32_t                SCLK_RCU_GPIOx;
    uint32_t                SCLK_GPIOx;
    uint16_t	    		SCLK_GPIO_Pin;

    uint32_t                MISO_RCU_GPIOx;
    uint32_t                MISO_GPIOx;
    uint16_t	    		MISO_GPIO_Pin;

    uint32_t                MOSI_RCU_GPIOx;
    uint32_t                MOSI_GPIOx;
    uint16_t	    		MOSI_GPIO_Pin;

    uint8_t                 AlternateFuncFlag;
#endif    
}STRU_SPI_RESOURCE_MANAGE;

typedef struct
{
    E_SPI_CHANNEL_LIST  SpiChannelIndex;
    uint8_t             EnableFlag;
    uint8_t             ResourceTableIndex;
#if defined(GD32E230) || defined (GD32E50X)
    uint32_t            SPIx;
    uint32_t            RCUSPIx;
#endif
}STRU_SPI_ENABLE_MANAGE;

#pragma pack()


// #pragma pack(1)
// typedef struct
// {
// 	uint8_t             u8_Enable;
// 	uint32_t	    	u32_SPIx_PERIPH;
// 	uint32_t            u32_SPIx_RCU;
//     uint32_t            u32_SCLK_RCU_GPIOx;
//     uint32_t            u32_SCLK_GPIOx;
//     uint16_t	    	u16_SCLK_GPIO_Pin;
//     uint32_t            u32_MISO_RCU_GPIOx;
//     uint32_t            u32_MISO_GPIOx;
//     uint16_t	    	u16_MISO_GPIO_Pin;
//     uint32_t            u32_MOSI_RCU_GPIOx;
//     uint32_t            u32_MOSI_GPIOx;
//     uint16_t	    	u16_MOSI_GPIO_Pin;
// }STRU_SPI_CFG;
// #pragma pack


#define     USE_SPI_GPIO_TYPE 	        1

#ifdef USE_SPI_GPIO_TYPE
// #define     FLASH_HOLD_RCU_GPIOx        RCU_GPIOA
// #define     FLASH_HOLD_GPIOx            GPIOA
// #define     FLASH_HOLD_GPIO_Pin         GPIO_PIN_4

// #define     FLASH_WP_RCU_GPIOx          RCU_GPIOC
// #define     FLASH_WP_GPIOx            	GPIOC
// #define     FLASH_WP_GPIO_Pin         	GPIO_PIN_4

#define     SPI_CS_RCU_GPIOx            RCU_GPIOA
#define     SPI_CS_GPIOx                GPIOA
#define     SPI_CS_GPIO_Pin             GPIO_PIN_4

#define     SPI_SCLK_RCU_GPIOx          RCU_GPIOA
#define     SPI_SCLK_GPIOx              GPIOA
#define     SPI_SCLK_GPIO_Pin           GPIO_PIN_5

#define     SPI_MISO_RCU_GPIOx          RCU_GPIOA
#define     SPI_MISO_GPIOx              GPIOA
#define     SPI_MISO_GPIO_Pin           GPIO_PIN_6

#define     SPI_MOSI_RCU_GPIOx          RCU_GPIOA
#define     SPI_MOSI_GPIOx              GPIOA
#define     SPI_MOSI_GPIO_Pin           GPIO_PIN_7

void fgv_SoftSpiGpioInit();
#endif

void fgv_SpiGpioTableInit(E_SPI_CHANNEL_LIST SpiChannel, E_DIO_RESOURCE_MANAGE SclkPinNum, E_DIO_RESOURCE_MANAGE MisoPinNum, E_DIO_RESOURCE_MANAGE MosiPinNum);
void fgv_SpiParaInit(E_SPI_CHANNEL_LIST SpiChannel);
E_SPI_ERR fge_SpiWriteReadByte(E_SPI_CHANNEL_LIST SpiChannel, uint16_t u16_wbyte, uint16_t* pu16_rbyte);

#ifdef __cplusplus
}
#endif

#endif /*__SPI_RESOURCE_MANAGE_H__*/
