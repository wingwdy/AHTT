#ifndef _KEY_H
#define _KEY_H
#include "common.h"

enum {
	KEY_SETKEY,
	KEY_NUM
};
// typedef uint32_t  u32;
// typedef uint16_t u16;
// typedef uint8_t  u8;

typedef struct _key_var {
	uint8_t key_none;			//表示按键有事件发生
	uint8_t key_pro;				//按键事件，可以传出公用；无操作、按下、释放、长按、长按释放；
	uint8_t key_press;			//按键是否按下
	uint32_t press_time;			//按下的事件
} key_var;
extern key_var key_info[KEY_NUM];
enum {
	KEY_POWER,
	KEY_ZERO,
	KEY_MEASURE
};

enum {
	KEY_NONE,
	KEY_DOWN,
	KEY_UP,
	KEY_LONG,
	KEY_LONG_UP
};

enum {
	KEY_NONE_ENABLE,	//按键没操作时
	KEY_ENABLE,				//按键已经被操作
	KEY_WAIT_BREAK		//等待长按释放
};

void Key_Main(void);


uint8_t get_key_event_3s(void);

#endif

