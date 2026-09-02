/***********************************************************************************
 * 文 件 名  : touch.c
 * 版 本 号  : V1.0
 * 负 责 人  : WEEN
 * 创建日期  : 2021-5-1
 * 文件描述  : 触摸处理函数
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  :
 * 其    他  :
 * 修改日志  :
***********************************************************************************/
#include "key.h"
#include "AppMidDataTrans.h"
#include "screenUart.h"

#define KEY_INDEX 0
key_var key_info[KEY_NUM];
#define KEY_PRESS_LONG_TIME 3000	//1s属于长按，触摸按键长按时间
//key事件扫描
void key_scan()
{
	//有按键事件，开始处理
	key_var *k_info = key_info;
	int i;
    uint8_t l_keySta = Get_PileBtnSta();

	//按键消抖, 15ms消抖
	static uint32_t k_tick = 0;
	if (key_info[0].key_press != l_keySta) {
		if ((Get_Systick() - k_tick) > 10) {
			key_info[0].key_press = l_keySta;
		}
	} else {
		key_info[0].key_press = l_keySta;
		k_tick = Get_Systick();
	}
	for (i = 0; i < KEY_NUM; i++) {
		//刚开始操作，记时间
		switch (k_info[i].key_none) {
			case KEY_NONE_ENABLE:
				if (k_info[i].key_press) {
					//计时
					k_info[i].press_time = NOWTICK;
					k_info[i].key_none = KEY_ENABLE;
					k_info[i].key_pro = KEY_DOWN;
				} else {
					k_info[i].key_pro = KEY_NONE;
				}
				
				break;
			case KEY_ENABLE:
				//判断是否长按
				if (k_info[i].key_press) {
					if (NOWTICK - k_info[i].press_time >= KEY_PRESS_LONG_TIME) {
						k_info[i].key_pro = KEY_LONG;
						k_info[i].key_none = KEY_WAIT_BREAK;
					}
				} else {
					//抬起事件
					k_info[i].key_pro = KEY_UP;
					k_info[i].key_none = KEY_NONE_ENABLE;
				}
				break;
			case KEY_WAIT_BREAK:
				//长按中止的，等待抬起
				if (!k_info[i].key_press) {
					k_info[i].key_pro = KEY_LONG_UP;
					k_info[i].key_none = KEY_NONE_ENABLE;
				}
				break;
			default:
				break;
		}
	}
}

uint8_t get_key_event_3s()
{
	key_var *k_info = key_info;
    if (k_info[0].key_pro == KEY_LONG_UP) {
        return 1;
    }
    return 0;
}

void handleScrDebug()
{
    Screen_data_ctrl *pScrData = GetScrMainStruct();

    uint8_t devIdle = GetPile_Idlet();
    
    if (!get_key_event_3s()) {
        return;
    }
    
    if (!devIdle) {
        return;
    }

    for (uint8_t i = 0; i < GUN_NUM; i++) {
        Screen_GunData *pScrGunData = &pScrData->gunData[i]; //枪信息，变为上送

        if (pScrGunData->UISta == UISTA_STANDBY) {
            pScrGunData->UISta = UISTA_DEBUG;
        } else if (pScrGunData->UISta == UISTA_DEBUG) {
            pScrGunData->UISta = UISTA_STANDBY;
        }
    }
}



void Key_Main()
{
	key_scan();
    handleScrDebug();
}