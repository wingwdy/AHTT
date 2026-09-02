#include  "rgb2hvs.h"
#include <math.h>

#define TO_RGB(R, G, B)	((R << 16) | (G << 8) | B)
static float min(float a, float b, float c)
{
	float m;
	m = a < b ? a : b;
	return (m < c ? m : c);
}
static float max(float a, float b, float c)
{
	float m;
	m = a > b ? a : b;
	return (m > c ? m : c);
}

COLOR_RGB rgb_v;

/**------------------------------------------------------------------------------------------------
  * @brief  : This is RGB to HSV convert function
  * @param  : None
  * @retval : None
  *----------------------------------------------------------------------------------------------*/
static void RGB_TO_HSV(const COLOR_RGB* input,COLOR_HSV* output)  // convert RGB value to HSV value 
 {
     float r,g,b,minRGB,maxRGB,deltaRGB;
 
     r = input->R/255.0f;
     g = input->G/255.0f;
     b = input->B/255.0f;
		minRGB = min(r,g,b);
		maxRGB = max(r,g,b);
     deltaRGB = maxRGB - minRGB;
 
     output->V = maxRGB;
     if(maxRGB != 0.0f)
      output->S = deltaRGB / maxRGB;
     else
      output->S = 0.0f;
     if (output->S <= 0.0f)
     {
      output->H = 0.0f;
     }
     else
     {
      if (r == maxRGB)
      {
       output->H = (g-b)/deltaRGB;
      }
      else
      {
       if (g == maxRGB)
       {
        output->H = 2.0f + (b-r)/deltaRGB;
       }
       else
       {
        if (b == maxRGB)
        {
         output->H = 4.0f + (r-g)/deltaRGB;
        }
       }
      }
      output->H = output->H * 60.0f;
      if (output->H < 0.0f)
      {
       output->H += 360;
      }
      output->H /= 360;
     }
 
 }
/**------------------------------------------------------------------------------------------------
  * @brief  : This is HSV to RGB convert function
  * @param  : None
  * @retval : None
  *----------------------------------------------------------------------------------------------*/
static void HSV_TO_RGB(COLOR_HSV* input,COLOR_RGB* output)  //convert HSV value to RGB value
 {
     float R,G,B;
     int k;
     float aa,bb,cc,f;
     if (input->S <= 0.0f)
      R = G = B = input->V;
     else
     {
      if (input->H == 1.0f)
       input->H = 0.0f;
      input->H *= 6.0f;
      k = (int)floor(input->H);
      f = input->H - k;
      aa = input->V * (1.0f - input->S);
      bb = input->V * (1.0f - input->S * f);
      cc = input->V * (1.0f -(input->S * (1.0f - f)));
      switch(k)
      {
      case 0:
       R = input->V; 
       G = cc; 
       B =aa;
       break;
      case 1:
       R = bb; 
       G = input->V;
       B = aa;
       break;
      case 2:
       R =aa;
       G = input->V;
       B = cc;
       break;
      case 3:
       R = aa;
       G = bb;
       B = input->V;
       break;
      case 4:
       R = cc;
       G = aa;
       B = input->V;
       break;
      case 5:
       R = input->V;
       G = aa;
       B = bb;
       break;
      }
     }
     output->R = (unsigned char)(R * 255);
     output->G = (unsigned char)(G * 255);
     output->B = (unsigned char)(B * 255);
 }

void led_breath_fun(uint32_t step, uint8_t *flag, COLOR_RGB *c_rgb)
{
	COLOR_HSV hsv_v;
	COLOR_RGB *rgb_v = c_rgb;
	
	RGB_TO_HSV(rgb_v,&hsv_v);

	if (*flag == 0) {
		rgb_v->l += step;
	} else {
		rgb_v->l -= step;
	}
	if(rgb_v->l <= 0)
	{
		rgb_v->l = 1;
		*flag = 0;
	}else if(rgb_v->l >= 50)
	{
		rgb_v->l = 50;
		*flag = 1;
	}
	hsv_v.V = rgb_v->l /100.0;
	HSV_TO_RGB(&hsv_v,rgb_v); 
}

void led_reduce_fun(uint32_t step, COLOR_RGB *c_rgb)
{
	COLOR_HSV hsv_v;
	COLOR_RGB *rgb_v = c_rgb;

  rgb_v->l = 100;
	
	RGB_TO_HSV(rgb_v,&hsv_v);
  
  if (step > rgb_v->l) {
		rgb_v->l = 0;
  } else {
	  rgb_v->l -= step;
  }
  
	hsv_v.V = rgb_v->l /100.0;
	HSV_TO_RGB(&hsv_v,rgb_v); 
}

void led_increase_fun(uint32_t step, COLOR_RGB *c_rgb)
{
	COLOR_HSV hsv_v;
	COLOR_RGB *rgb_v = c_rgb;

  rgb_v->l = 0;
	
	RGB_TO_HSV(rgb_v,&hsv_v);

	rgb_v->l += step;
  if(rgb_v->l >= 100)
	{
		rgb_v->l = 100;
	}

	hsv_v.V = rgb_v->l /100.0;
	HSV_TO_RGB(&hsv_v,rgb_v); 
}
