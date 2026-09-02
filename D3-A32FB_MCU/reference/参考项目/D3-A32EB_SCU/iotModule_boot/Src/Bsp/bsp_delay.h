#ifndef __BSP_DELAY_H_
#define __BSP_DELAY_H_

#define GD32E50X    1

#if defined(GD32E230)
#include "gd32e23x.h"
#elif defined(GD32E50X)
#include "gd32e50x.h"
#endif


// #define DWT_CR      *(__IO uint32_t*)0xE0001000
// #define DWT_CYCCNT  *(__IO uint32_t*)0xE0001004
// #define DEM_CR      *(__IO uint32_t*)0xE000EDFC

// #define DEM_CR_TRCENA       (1 << 24)
// #define DWT_CR_CYCCNTENA    (1 << 0)


void bsp_delay_init(void);

void bsp_delay_us(uint32_t u32_us);

void bsp_delay_ms(uint32_t u32_ms);


#endif /* __BSP_DELAY_H_ */
