/******************************************************************************
* File Name          : Cdd_Drv_WS2812BConfig.c
* Description        : Code for WS2812B Driver
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_Drv_WS2812BConfig.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/



/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
/* 限制与约束： 
 * 当前的这种配置架构，只能适配于通道的配置逻辑都是一样的，
 * 针对WS2812B灯，比如两把枪相当于两个通道，每个通道的灯的数量需要是一样的，
 * 且对应的事件分类也是一样的，它们的核心区别是位置不一样。
 * 假如后续有需求不一样，再调整
 */
const uint8_t c_CddDrvWS2812BLedLocation[eCddDrvWS2812BChannel_Count][CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT] = 
{
    {0,  1,  2},
};

const CddDrvWS2812BConfig_Struct c_stCddDrvWS2812BConfigTable[CDD_LEDM_DEVICE0_DISP_TYPE_COUNT] = 
{  
    [CDD_LEDM_DEVICE0_DISP_TYPE_0] =
    {
        .RGBArray = { CDDDRV_WS2812B_NONE,    CDDDRV_WS2812B_NONE,     CDDDRV_WS2812B_NONE   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_Off,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_1] =
    {
        .RGBArray = { CDDDRV_WS2812B_BLUE,    CDDDRV_WS2812B_BLUE,     CDDDRV_WS2812B_BLUE   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_SlowBlink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_2] =
    {
        .RGBArray = { CDDDRV_WS2812B_GREEN,   CDDDRV_WS2812B_GREEN,    CDDDRV_WS2812B_GREEN   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_SlowBlink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_3] =
    {
        .RGBArray = { CDDDRV_WS2812B_YELLOW,  CDDDRV_WS2812B_YELLOW,   CDDDRV_WS2812B_YELLOW   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_On,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_4] =
    {
        .RGBArray = { CDDDRV_WS2812B_GREEN,   CDDDRV_WS2812B_GREEN,    CDDDRV_WS2812B_GREEN   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_Blink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_5] =
    {
        .RGBArray = { CDDDRV_WS2812B_YELLOW,  CDDDRV_WS2812B_YELLOW,   CDDDRV_WS2812B_YELLOW   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_On,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_6] =
    {
        .RGBArray = { CDDDRV_WS2812B_GREEN,   CDDDRV_WS2812B_GREEN,    CDDDRV_WS2812B_GREEN   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_On,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_7] =
    {
        .RGBArray = { CDDDRV_WS2812B_YELLOW,   CDDDRV_WS2812B_YELLOW,    CDDDRV_WS2812B_YELLOW   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_Blink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_8] =
    {
        .RGBArray = { CDDDRV_WS2812B_GREEN,   CDDDRV_WS2812B_GREEN,    CDDDRV_WS2812B_GREEN   },
        .maxDispTimes = 0x03,
        .eDispMode = eCddDrvWS2812BDispMode_FastBlink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_9] =
    {
        .RGBArray = { CDDDRV_WS2812B_RED,   CDDDRV_WS2812B_RED,    CDDDRV_WS2812B_RED   },
        .maxDispTimes = 0x03,
        .eDispMode = eCddDrvWS2812BDispMode_FastBlink,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_10] =
    {
        .RGBArray = { CDDDRV_WS2812B_RED,   CDDDRV_WS2812B_RED,    CDDDRV_WS2812B_RED   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_On,
    },

    [CDD_LEDM_DEVICE0_DISP_TYPE_11] =
    {
        .RGBArray = { CDDDRV_WS2812B_WHITE,   CDDDRV_WS2812B_WHITE,    CDDDRV_WS2812B_WHITE   },
        .maxDispTimes = 0xFF,
        .eDispMode = eCddDrvWS2812BDispMode_On,
    },
};

void CddDrvWS2812BCfg_ShowLed(uint16_t *pData, uint16_t length)
{
     McalPWM_SetMultiDuty(eMcalPWMOCChannel_Led, MCALPWM_CFG_SET_REG, pData, length);
}


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/




























