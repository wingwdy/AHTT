#ifndef _RGB_LED_SCAN_H
#define _RGB_LED_SCAN_H

#include "rgb2hvs.h"
#include "rgb_led.h"

#define Led_RGB_TEST   0

// #define Led_SINGLE_NUM   ((GUN_NUM == 2) ? 7 : (7 + 2))		//双枪一组个数,单枪9个一组
#define Led_SINGLE_NUM   9		//单枪9个一组
#define Led_GROUP_NUM   7		//双枪一组个数
#define Led_ARRY_MAX_NUM   4

/* LED呼吸 */
#define TIME_BREATH_STEP  25
/* LED流水灯移位间隔时间 单位ms */
#define TIME_RUNNING_STEP  100
/* LED闪烁时间间隔 单位ms */
#define TIME_BLINKING_STEP  500

/* LED闪烁交替颜色个数 */
#define BLINK_CNT	2

typedef enum {
  eColor_Off = 0x000000,
  eColor_Red = 0xFF0000,
  eColor_Pink = 0x00FFFF,
  eColor_Blue = 0x0000FF,
  eColor_Yellow = 0xFFFF00,
  eColor_Green = 0x00FF00,
  eColor_White = 0x555555,
} eColorList;


typedef enum {
  eRunType_Off,
  eRunType_Running,
  eRunType_Light,
  eRunType_Blink,
  eRunType_Breath
} eLedRunType;


typedef struct
{ 
	uint8_t rgb_b;
	uint8_t rgb_g;
	uint8_t rgb_r;
	uint8_t rgb_rev;		//预留
} COLOR_rgb;

typedef union
{ 
  uint32_t 	rgb;
  COLOR_rgb color;
} COLOR_UN;


typedef struct _led_breath{
	uint32_t IntervalTime;			//闪烁时间
	uint32_t StartTick;				//计时时间
  //hvs转换使用
	uint32_t step;
	uint8_t rhythmFlag;
	COLOR_RGB rgb_v;
} led_breath;

typedef struct _led_blink{
	uint32_t IntervalTime;			//闪烁时间
	uint32_t StartTick;				//计时时间
	uint8_t blinkCnt;				//闪烁
	COLOR_UN rgbBlink[BLINK_CNT];	//闪烁的交替颜色
} led_blink;

typedef struct _led_running{
  uint8_t LedMaxCnt;			//流水灯个数
  uint32_t RunBuffer[Led_MAX_NUM];
  uint32_t ColorBuffer[Led_MAX_NUM];
  uint32_t IntervalTime;
  uint32_t StartTick;
} led_running;

typedef struct _led_sta{
	COLOR_UN target_rgb;
	COLOR_UN current_rgb;
	led_breath brt;
	led_blink blk;
	// led_running run;
} ledStaInfo;

typedef struct _led_all_sta {
	eLedRunType sta[Led_MAX_NUM];				//0关闭，1常亮，2呼吸，3闪烁, 4流水
	COLOR_UN current_rgb[Led_MAX_NUM];
	ledStaInfo ledSta[Led_MAX_NUM];
	led_running run[GUN_NUM_MAX];

	// eLedRunType LightArraySta[GUN_NUM_MAX];		//每一组灯语的当前状态
	uint32_t LightArrayColor[GUN_NUM_MAX];		//每一组灯语的当前状颜色地址
} led_all_sta;



/* Led主函数，1ms调用一次 */
void fgv_LedDisplayMain(void);


void fsv_LedDataRefresh(void);

/* 设置led表现类型 */
void fgv_setLedLightMode(const uint32_t *color, eLedRunType type, uint16_t time);

void fgv_SetSingleLedType(uint8_t uPort, const uint32_t *color, uint8_t itv, eLedRunType type);


//测试使用
void fgv_SetSingleLedColor(uint8_t num, const uint32_t color);
void fgv_SetWholeLedColor(const uint32_t color);


void RGB_LED_Main(void);

#ifdef __cplusplus
extern "C" {
#endif
void RunRGB_LED_Task(void);
#ifdef __cplusplus
}
#endif


#endif

