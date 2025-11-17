/******************************************************************************
* File Name          : Cdd_RelayConfig.h
* Description        : Code for the driver of relay
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
******************************************************************************/
#ifndef CDD_RELAY_CONFIG_H_
#define CDD_RELAY_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/

#include "Mcal_PWM.h"
#include "Mcal_Port.h"
#include "Cdd_Relay.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDRELAY_CFG_CALL_CYCLE                            (5u)
#define CDDRELAY_CFG_STATE_FILTER_COUNT                    (50u / CDDRELAY_CFG_CALL_CYCLE)
#define CDDRELAY_CFG_ADHESION_FILTER_COUNT                 (500u / CDDRELAY_CFG_CALL_CYCLE)
#define CDDRELAY_CFG_MALOPERATION_FILTER_COUNT             (500u / CDDRELAY_CFG_CALL_CYCLE)                                             
#define CDDRELAY_ACT_HOLD_TIMEOUT                          (100)
#define CDDRELAY_ACT_DELAY_TIMEOUT                         (300)

#define CDDRELAY_CFG_GetRelayAdhesionState(port, ret)       do\
                                                            {\
                                                                if (port == 0)\
                                                                {\
                                                                    ret = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC6_RelayAdhesion))\
                                                                     ? TRUE : FALSE);\
                                                                }\
                                                                else\
                                                                {\
                                                                    ret = FALSE;\
                                                                }\
                                                            }while(0)
                                                            
#define CDDRELAY_CFG_GetRelayState(port, ret)               do\
                                                            {\
                                                                if (port == 0)\
                                                                {\
                                                                    ret = ((MCALPORT_PIN_LOW == McalPort_GetPin(eMcalPortPinChanel_PC7_OutBack1)) ? \
                                                                    eCddRelayState_On : eCddRelayState_Off);\
                                                                }\
                                                                else\
                                                                {\
                                                                    ret = eCddRelayState_Off;\
                                                                }\
                                                            }while(0)

#define CDDRELAY_CFG_CtrlSwitchOn(port)                    do \
                                                           {\
                                                                if (port == 0)\
                                                                {\
                                                                    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_HIGH);\
                                                                }\
                                                                else\
                                                                {}\
                                                           }while(0)


#define CDDRELAY_CFG_HoldSwitchOn(port)                    do \
                                                           {\
                                                                if (port == 0)\
                                                                {\
                                                                    McalPWM_SetSingleDuty(eMcalPWMOCChannel_Relay, 800);\
                                                                    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_PWM);\
                                                                }\
                                                                else\
                                                                {}\
                                                           }while(0)

#define CDDRELAY_CFG_CtrlSwitchOff(port)                    do \
                                                           {\
                                                                if (port == 0)\
                                                                {\
                                                                    McalPWM_SetOutputMode(eMcalPWMOCChannel_Relay, MCALPWM_MODE_FORCE_LOW);\
                                                                }\
                                                                else\
                                                                {}\
                                                           }while(0)    
                                                             
#define CDDRELAY_CFG_CheckGunPlugout(port)                 TRUE
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

#endif /* MCAL_IF_H_ */





















