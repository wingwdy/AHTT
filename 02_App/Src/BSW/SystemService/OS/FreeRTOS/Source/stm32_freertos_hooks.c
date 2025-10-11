

#include "stm32_freertos_hooks.h"

uint32_t os_sys_tick = 0;

void stm32_vApplicationTickHook()
{
	os_sys_tick++;
}
