#ifndef _RGB_LED_H
#define _RGB_LED_H

#include <stdint.h>
#include <string.h>
#include "AppHeaderSummary.h"


/* LED灯总个数 */
#define Led_MAX_NUM   15

extern uint8_t pixelBuffer[Led_MAX_NUM][24];

void LED_Timer_Init(void);

void ws281x_BufferTransfer(uint8_t* buf, int len);
void ws281x_BufferUpdate(uint32_t *RGBcolor, uint8_t n);

#endif

