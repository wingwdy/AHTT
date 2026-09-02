
#ifndef STM32_FREERTOS_HOOK_H
#define STM32_FREERTOS_HOOK_H

#include "FreeRTOS.h"

extern uint32_t os_sys_tick;

extern void stm32_vApplicationTickHook(void);

#endif /* STM32_FREERTOS_HOOK_H */
