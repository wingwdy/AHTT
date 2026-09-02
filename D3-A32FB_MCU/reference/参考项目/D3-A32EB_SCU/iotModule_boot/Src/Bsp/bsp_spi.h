#ifndef __BSP_SPI_H_
#define __BSP_SPI_H_


#if defined(GD32E230)
#include "gd32e23x.h"
#elif defined(GD32E50X)
#include "gd32e50x.h"
#endif

#if defined(GD32E230)

#define     SPI_TIME_OUT_SET_VAL        0xffff

#define     SPI_RCUSPIx                 RCU_SPI0
#define     SPI_SPIx                    SPI0

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

#elif defined(GD32E50X)

#define     SPI_TIME_OUT_SET_VAL        0xffff

#define     SPI_RCUSPIx                 RCU_SPI0
#define     SPI_SPIx                    SPI0

#define     FLASH_WP_RCU_GPIOx          RCU_GPIOC
#define     FLASH_WP_GPIOx            	GPIOC
#define     FLASH_WP_GPIO_Pin         	GPIO_PIN_4

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

//#define     SPI_MISO_RCU_GPIOx          RCU_GPIOA
//#define     SPI_MISO_GPIOx              GPIOA
//#define     SPI_MISO_GPIO_Pin           GPIO_PIN_6

//#define     SPI_MOSI_RCU_GPIOx          RCU_GPIOA
//#define     SPI_MOSI_GPIOx              GPIOA
//#define     SPI_MOSI_GPIO_Pin           GPIO_PIN_7

#endif


typedef enum
{
    SPI_NO_ERR = 0,
    SPI_GPIO_INIT_ERR,
    SPI_NUM_ERR,
    SPI_WR_WAIT_ERR,
    SPI_RD_WAIT_ERR,
    SPI_NO_INITED_ERR,
}E_SPI_ERR;


void bsp_spi_init();

void bsp_spi_cs_enable();

void bsp_spi_cs_disable();

E_SPI_ERR bsp_spi_rw_byte(uint16_t u16_wbyte, uint16_t* pu16_rbyte);


#endif /* __BSP_SPI_H_ */
