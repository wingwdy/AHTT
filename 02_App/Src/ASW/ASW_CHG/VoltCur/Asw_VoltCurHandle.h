
/******************************************************************************
* File Name          : Asw_VoltCurHandle.h
* Description        : Code for VoltCurHandle
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      shenjc    初版创建
*
******************************************************************************/
#ifndef ASW_VOLTCURHANDLE_H_
#define ASW_VOLTCURHANDLE_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_Charge.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASW_VOLTCUR_CFG_IsAuthState(port)               AswCharge_GetAuthFlag(port)

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eAswVoltCurAdjustMode_PowerAbsolute,             /* 功率绝对值 */
    eAswVoltCurAdjustMode_PowerPercent,              /* 功率百分比 */
    eAswVoltCurAdjustMode_Count,                     /* 功率调节模式个数 */
}AswVoltCurAdjustMode_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern void AswVoltCurHandle_InitMemory(void);
extern void AswVoltCurHandle_MainFunction(void);
void AswVoltCur_AdjustOutputCurrent(uint8_t port, AswVoltCurAdjustMode_Enum eMode, uint32_t val);
uint32_t AswVoltCurHandle_GetMaxOutputCurrent(uint8_t port);


#endif /* ASW_VOLTCURHANDLE_H_ */























