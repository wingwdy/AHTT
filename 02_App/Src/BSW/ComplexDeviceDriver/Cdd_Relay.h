/******************************************************************************
* File Name          : Cdd_Relay.h
* Description        : Code for the driver of relay
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
#ifndef CDD_RELAY_H_
#define CDD_RELAY_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eCddRelayState_Off,
    eCddRelayState_On,
}CddRelayState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddRelay_MainFunction(void);
void CddRelay_InitMemory(void);
void CddRelay_CtrlSwichOn(uint8_t port);
void CddRelay_CtrlSwichOff(uint8_t port);
CddRelayState_Enum CddRelay_GetRelayState(uint8_t port);
void CddRelay_SetReqStartShortCutDetect(uint8_t port);
void CddRelay_SetReqStopShortCutDetect(uint8_t port);
uint8_t CddRelay_GetShortCutStatus(uint8_t port);
#endif /* CDD_RELAY_H_ */





















