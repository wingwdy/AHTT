/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_spi.h"
#include "bsp_delay.h"

#define USE_SPI_GPIO_TYPE 	1

void bsp_spi_init()
{
    spi_parameter_struct spi_init_struct;

#if defined(GD32E230)
    rcu_periph_clock_enable(SPI_CS_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_SCLK_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MISO_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MOSI_RCU_GPIOx);
	
	gpio_mode_set(SPI_CS_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS_GPIO_Pin);
	gpio_output_options_set(SPI_CS_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS_GPIO_Pin);
	
	gpio_af_set(SPI_SCLK_GPIOx, GPIO_AF_0, SPI_SCLK_GPIO_Pin);
	gpio_af_set(SPI_MISO_GPIOx, GPIO_AF_0, SPI_MISO_GPIO_Pin);
	gpio_af_set(SPI_MOSI_GPIOx, GPIO_AF_0, SPI_MOSI_GPIO_Pin);
	gpio_mode_set(SPI_SCLK_GPIOx, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_SCLK_GPIO_Pin);
	gpio_mode_set(SPI_MISO_GPIOx, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MISO_GPIO_Pin);
	gpio_mode_set(SPI_MOSI_GPIOx, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MOSI_GPIO_Pin);
	
	gpio_bit_set(SPI_CS_GPIOx, SPI_CS_GPIO_Pin);
	
	rcu_periph_clock_enable(SPI_RCUSPIx);
	
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_8;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI_SPIx, &spi_init_struct);
	
	spi_crc_polynomial_set(SPI_SPIx, 7);
	spi_enable(SPI_SPIx);
	
#elif defined(GD32E50X)
	

#ifdef USE_SPI_GPIO_TYPE
	//rcu_periph_clock_enable(FLASH_HOLD_RCU_GPIOx);
	//rcu_periph_clock_enable(FLASH_WP_RCU_GPIOx);
	
    rcu_periph_clock_enable(SPI_CS_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_SCLK_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MISO_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MOSI_RCU_GPIOx);
	
    gpio_init(SPI_CS_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_CS_GPIO_Pin);
    gpio_init(SPI_SCLK_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_SCLK_GPIO_Pin);
    gpio_init(SPI_MISO_GPIOx, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SPI_MISO_GPIO_Pin);
    gpio_init(SPI_MOSI_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_MOSI_GPIO_Pin);
	
	gpio_init(FLASH_WP_GPIOx, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, FLASH_WP_GPIO_Pin);

	gpio_bit_set(SPI_CS_GPIOx, SPI_CS_GPIO_Pin);
	
	gpio_bit_set(FLASH_WP_GPIOx, FLASH_WP_GPIO_Pin);
#else

	rcu_periph_clock_enable(FLASH_HOLD_RCU_GPIOx);
	rcu_periph_clock_enable(FLASH_WP_RCU_GPIOx);
	
    rcu_periph_clock_enable(SPI_CS_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_SCLK_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MISO_RCU_GPIOx);
    rcu_periph_clock_enable(SPI_MOSI_RCU_GPIOx);
	rcu_periph_clock_enable(RCU_AF);
	
    gpio_init(SPI_CS_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_CS_GPIO_Pin);
    gpio_init(SPI_SCLK_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_SCLK_GPIO_Pin);
    gpio_init(SPI_MISO_GPIOx, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SPI_MISO_GPIO_Pin);
    gpio_init(SPI_MOSI_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_MOSI_GPIO_Pin);
	
	gpio_init(FLASH_WP_GPIOx, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, FLASH_WP_GPIO_Pin);

	gpio_bit_set(SPI_CS_GPIOx, SPI_CS_GPIO_Pin);
	
	gpio_bit_set(FLASH_WP_GPIOx, FLASH_WP_GPIO_Pin);
	
	rcu_periph_clock_enable(SPI_RCUSPIx);
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_8;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI_SPIx, &spi_init_struct);
	spi_crc_polynomial_set(SPI_SPIx, 7);
	spi_enable(SPI_SPIx);

#endif

#endif

}

void bsp_spi_delay(void)
{
	uint32_t n = 20;
	while(n)
	{
		n--;
	}
}

void bsp_spi_cs_enable()
{
	GPIO_BC(SPI_CS_GPIOx) = SPI_CS_GPIO_Pin;
}

void bsp_spi_cs_disable()
{
	GPIO_BOP(SPI_CS_GPIOx) = SPI_CS_GPIO_Pin;
}

E_SPI_ERR bsp_spi_rw_byte(uint16_t u16_wbyte, uint16_t* pu16_rbyte)
{	
	uint32_t spi_time_out = 0;
	uint16_t spi_data = 0;
	
#ifdef USE_SPI_GPIO_TYPE
	uint8_t wbyte = (uint8_t)u16_wbyte;
	
	gpio_bit_reset(SPI_SCLK_GPIOx, SPI_SCLK_GPIO_Pin);
	for(uint8_t i = 0; i < 8; i++)
	{
		bsp_spi_delay();//bsp_delay_us(2);
		if(wbyte & (0x80 >> i))
		{
			gpio_bit_set(SPI_MOSI_GPIOx, SPI_MOSI_GPIO_Pin);
		}
		else
		{
			gpio_bit_reset(SPI_MOSI_GPIOx, SPI_MOSI_GPIO_Pin);
		}
		bsp_spi_delay();//bsp_delay_us(2);
		gpio_bit_set(SPI_SCLK_GPIOx, SPI_SCLK_GPIO_Pin);
		if(gpio_input_bit_get(SPI_MISO_GPIOx, SPI_MISO_GPIO_Pin))
		{
			spi_data |= (0x80 >> i);
		}
		bsp_spi_delay();//bsp_delay_us(2);
		gpio_bit_reset(SPI_SCLK_GPIOx, SPI_SCLK_GPIO_Pin);
	}
	
#else
	
#if defined(GD32E230) || defined(GD32E50X)
		while(spi_i2s_flag_get(SPI_SPIx, SPI_FLAG_TBE) == RESET)
		{
			if(spi_time_out++ >= SPI_TIME_OUT_SET_VAL)
			{
				return SPI_WR_WAIT_ERR;
			}
		}
		spi_i2s_data_transmit(SPI_SPIx, u16_wbyte);
		spi_time_out = 0;
		while(spi_i2s_flag_get(SPI_SPIx, SPI_FLAG_RBNE) == RESET)
		{
			if(spi_time_out++ >= SPI_TIME_OUT_SET_VAL)
			{
				return SPI_RD_WAIT_ERR;
			}
		}
		spi_data = spi_i2s_data_receive(SPI_SPIx);
#endif
		
#endif
	if(pu16_rbyte)
	{
		*pu16_rbyte = spi_data;
	}

	return SPI_NO_ERR;
}

