#ifndef _RGB2HVS_H
#define _RGB2HVS_H

#include <stdint.h>
#include <string.h>

//color data struct
typedef struct{
 unsigned char R;
 unsigned char G;
 unsigned char B;
 unsigned char l;
}COLOR_RGB;
 
 
//hsv data struct
typedef struct{
 float H;
 float S;
 float V;
}COLOR_HSV;

void adjustBrightness(int step);
void led_breath_fun(uint32_t step, uint8_t *flag, COLOR_RGB *c_rgb);


void led_reduce_fun(uint32_t step, COLOR_RGB *c_rgb);
void led_increase_fun(uint32_t step, COLOR_RGB *c_rgb);
//void Brightness_Set(int lv, COLOR_UN *rgb);


#endif

