#include <stdio.h>


#if defined(GD32E230)
#include "gd32e23x.h"
#elif defined(GD32E50X)
#include "gd32e50x.h"
#endif

#include "bsp_delay.h"
#include "bsp_flash.h"
#include "AppOta.h"
#include "bsp_LogUart.h"


#define BOOT_LOADER_VER 				0x1000102 //v1.0.0.2 (GD芯片)




uint32_t g_sec_test = 0;

int main(void)
{
//	SystemInit();
	bsp_delay_init();
	printf_USART_Config();
	bsp_spi_init();
	bsp_delay_ms(50);
	printf("Boot start...\r\n");
	
	while(flash_check(BOOT_LOADER_VER))//flash chip check ok
	{
		bsp_delay_ms(500);
	}

	ota_Init();
	
	while(1)
	{
		ota_task();
		
		sflv_BootUpdateJumpToAppManage();
		
	}

	return 0;
}
