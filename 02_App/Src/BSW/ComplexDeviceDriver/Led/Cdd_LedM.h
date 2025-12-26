/******************************************************************************
* File Name          : Cdd_LedM.h
* Description        : Code for Led Driver router
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef CDD_LEDM_H_
#define CDD_LEDM_H_

/******************************************************************************
*    Include Files
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_LEDM_DEVICE_0_WS2812B              0
#define CDD_LEDM_DEVICE_COUNT                  1

/* 不同的灯语显示样式 */
#define CDD_LEDM_DEVICE0_DISP_TYPE_0           0
#define CDD_LEDM_DEVICE0_DISP_TYPE_1           1
#define CDD_LEDM_DEVICE0_DISP_TYPE_2           2
#define CDD_LEDM_DEVICE0_DISP_TYPE_3           3
#define CDD_LEDM_DEVICE0_DISP_TYPE_4           4
#define CDD_LEDM_DEVICE0_DISP_TYPE_5           5
#define CDD_LEDM_DEVICE0_DISP_TYPE_6           6
#define CDD_LEDM_DEVICE0_DISP_TYPE_7           7
#define CDD_LEDM_DEVICE0_DISP_TYPE_8           8
#define CDD_LEDM_DEVICE0_DISP_TYPE_9           9
#define CDD_LEDM_DEVICE0_DISP_TYPE_10          10
#define CDD_LEDM_DEVICE0_DISP_TYPE_11          11
#define CDD_LEDM_DEVICE0_DISP_TYPE_12          12
#define CDD_LEDM_DEVICE0_DISP_TYPE_13          13
#define CDD_LEDM_DEVICE0_DISP_TYPE_COUNT       14

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddLedM_InitMemory(void);
void CddLedM_MainFunction(void);
void CddLedM_UpdateState(uint8_t device, uint8_t port, uint8_t ledDispType);
#endif























