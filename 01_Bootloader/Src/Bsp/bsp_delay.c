/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_delay.h"

static uint32_t sg_timebase_us = 0;
static uint32_t sg_timebase_ms = 0;

void bsp_delay_init(void)
{
	uint32_t sys_tick_clock = 0;
	
#if defined(GD32E230) || defined(GD32E50X)
	
	sys_tick_clock = rcu_clock_freq_get(CK_SYS);
	sys_tick_clock = sys_tick_clock / 8;
	systick_clksource_set(SYSTICK_CLKSOURCE_HCLK_DIV8);
	
#endif    
    
    sg_timebase_us = sys_tick_clock / 1000000;
    sg_timebase_ms = sys_tick_clock / 1000;
}

void bsp_delay_us(uint32_t u32_us)
{
	uint32_t u32_tick_load = 0xFFFFFF;
	uint32_t u32_tick_ctrl = 0;
	
#if defined(GD32E230) || defined(GD32E50X)
	if(!u32_us) {
		return;
	}

	if(u32_us < (0xFFFFFF / sg_timebase_us))
	{
		u32_tick_load = sg_timebase_us * u32_us;
	}
	SysTick->LOAD  = u32_tick_load  - 1UL;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0UL;
	do
	{
		u32_tick_ctrl = SysTick->CTRL;
	}
	while((u32_tick_ctrl & SysTick_CTRL_ENABLE_Msk) && (!(u32_tick_ctrl & SysTick_CTRL_COUNTFLAG_Msk)));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0UL;
	
#endif	

}

void bsp_delay_ms(uint32_t u32_ms)
{
	uint32_t u32_tick_load = 0xFFFFFF;
	uint32_t u32_tick_ctrl = 0;
#if defined(GD32E230) || defined(GD32E50X)
	if(!u32_ms) {
		return;
	}
	
	if(u32_ms < (0xFFFFFF / sg_timebase_ms))
	{
		u32_tick_load = sg_timebase_ms * u32_ms;
	}
	SysTick->LOAD  = u32_tick_load - 1UL;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0UL;
	do
	{
		u32_tick_ctrl = SysTick->CTRL;
	}
	while((u32_tick_ctrl & SysTick_CTRL_ENABLE_Msk) && (!(u32_tick_ctrl & SysTick_CTRL_COUNTFLAG_Msk)));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0UL;
	
#endif
}
