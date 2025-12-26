/******************************************************************************
* File Name          : Cdd_Drv_WS2812BConfig.h
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
#ifndef CDD_DRV_WS2812B_CONFIG_H_
#define CDD_DRV_WS2812B_CONFIG_H_



/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Mcal_PWM.h"
#include "Mcal_PWMConfig.h"
#include "Cdd_Drv_WS2812B.h"
#include "Cdd_LedM.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
/* 默认每个通道的灯数量一致 */
#define CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT  (MCALPWM_CFG_LED_COUNT / eCddDrvWS2812BChannel_Count)

/* 根据灯的数量，去规划存储RGB值的buf大小，单位：32bit */      
#define CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE             (MCALPWM_CFG_LED_COUNT) 

/* 需要转换成归零码buff的大小  单位：8bit*/
#define CDDDRV_WS2812B_CFG_DUTY_BUFF_SIZE            (MCALPWM_CFG_LED_DMABUF_LEN)

/* 快闪的亮灭时间定义  ms */
#define CDDDRV_WS2812B_CFG_FASTBLINK_ON_TIMEOUT      (250)
#define CDDDRV_WS2812B_CFG_FASTBLINK_OFF_TIMEOUT     (250)

/* 闪烁的亮灭定义 ms*/
#define CDDDRV_WS2812B_CFG_BLINK_ON_TIMEOUT          (500)
#define CDDDRV_WS2812B_CFG_BLINK_OFF_TIMEOUT         (500)

/* 慢闪的亮灭定义 ms*/
#define CDDDRV_WS2812B_CFG_SLOWBLINK_ON_TIMEOUT      (1000)
#define CDDDRV_WS2812B_CFG_SLOWBLINK_OFF_TIMEOUT     (1000)

/* 高低对应的占空比值  112(502), 62(280)*/
#define CDDDRV_WS2812B_CFG_HIGH_BIT_DUTY             (112u)
#define CDDDRV_WS2812B_CFG_LOW_BIT_DUTY              (62u) 

/* 呼吸单次控制步长 */
#define CDDDRV_WS2812B_CFG_BREATH_FOOTSTEP            (2)
/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eCddDrvWS2812BDispMode_Off,
    eCddDrvWS2812BDispMode_SlowBlink,
    eCddDrvWS2812BDispMode_Blink,
    eCddDrvWS2812BDispMode_FastBlink,
    eCddDrvWS2812BDispMode_Breath,
    eCddDrvWS2812BDispMode_On,
    eCddDrvWS2812BDispMode_Count,
}CddDrvWS2812BDispMode_Enum;


typedef struct
{
    uint32_t RGBArray[CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE];
    CddDrvWS2812BDispMode_Enum eDispMode;
    uint8_t maxDispTimes;
}CddDrvWS2812BConfig_Struct;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern const CddDrvWS2812BConfig_Struct c_stCddDrvWS2812BConfigTable[CDD_LEDM_DEVICE0_DISP_TYPE_COUNT];
extern const uint8_t c_CddDrvWS2812BLedLocation[eCddDrvWS2812BChannel_Count][CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT];

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
void CddDrvWS2812BCfg_ShowLed(uint16_t *pData, uint16_t length);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

#endif






















