/******************************************************************************
* File Name          : Cdd_CP.h
* Description        : Code for Control Pilot
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
#ifndef CDD_CP_H_
#define CDD_CP_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "stdint.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eCddCPVolState_Ground,          /* 接地态 */
    eCddCPVolState_6V,              /* 6V态 */
    eCddCPVolState_9V,              /* 9V态 */
    eCddCPVolState_12V,             /* 12V态 */
    eCddCPVolState_Err,             /* 故障态 */
}CddCPVolState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
CddCPVolState_Enum CddCP_GetVolState(uint8_t port);
uint16_t CddCP_GetVoltage(uint8_t port);
void CddCP_AdjustCurRateCurrent(uint8_t port, uint32_t current);
void CddCP_StartPWM(uint8_t port);
void CddCP_StopPWM(uint8_t port);
void CddCP_SetErrNotice(uint8_t port);
void CddCP_SetReqStartWakeup(uint8_t port);
void CddCP_SetReqStopWakeUp(uint8_t port);
void CddCP_SetReqStartDiodeExsitDetect(uint8_t port);
void CddCP_SetReqStopDiodeExsitDetect(uint8_t port);
uint8_t CddCP_GetDiodeExsitDetectResult(uint8_t port);
void CddCP_MainFunction(void);
void CddCP_InitMemory(void);
#endif
 











