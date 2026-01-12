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
*2025/10/10      V1.0.0      chenls    初版创建
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
#include "Cdd_CP.h"
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDRELAY_CFG_CALL_CYCLE                            (10u)
#define CDDRELAY_CFG_STATE_FILTER_COUNT                    (200u / CDDRELAY_CFG_CALL_CYCLE)
#define CDDRELAY_CFG_ADHESION_FILTER_COUNT                 (2000u / CDDRELAY_CFG_CALL_CYCLE)
#define CDDRELAY_CFG_MALOPERATION_FILTER_COUNT             (2000u / CDDRELAY_CFG_CALL_CYCLE) 
#define CDDRELAY_CFG_ADHESION_DETECT_TIMEOUT               (2000u)

#define CDDRELAY_CFG_ACT_HOLD_TIMEOUT                      (100)
#define CDDRELAY_CFG_ACT_DELAY_TIMEOUT                     (5000)

#define CDDRELAY_CFG_ADC_BUFF_POINT                        (MCALADC_ADC0_SAMPLE_CNT)
#define CDDRELAY_CFG_SHORTCUT_UPPER_LIMIT                  250
#define CDDRELAY_CFG_SHORTCUT_LOWER_LIMIT                  200

#define CDDRELAY_CFG_SHORTCUT_TIMEOUT                      (5000)
#define CDDRELAY_CFG_SHORTCUT_FINSH_DELAY                  (500)
#define CDDRELAY_CFG_SHORTCUT_FILTER_COUNT                 (500 /  CDDRELAY_CFG_CALL_CYCLE)

#define CDDRELAY_CFG_CheckGunPlugout(port)                 (eCddCPVolState_12V == CddCP_GetVolState(port))

#define CDDRELAY_CFG_LogPrint(fmt, ...)                    DSLOGM_Debug(DSLogMModule_RELAY, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    void (*pFuncCtrlSwitchOn)(uint8_t port);
    void (*pFuncCtrlSwitchOff)(uint8_t port);
    void (*pFuncHoldSwitchOn)(uint8_t port);
    uint8_t (*pFuncGetSwitchStatus)(uint8_t port);
    uint8_t (*pFuncGetRelayAdhesionStatus)(uint8_t port);
    void (*pFuncCtrlShortCutOn)(uint8_t port);
    void (*pFuncCtrlShortCutOff)(uint8_t port);
    uint8_t (*pFuncGetShortCutStatus)(uint8_t port);
}CddRelayOpsConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddRelayOpsConfig_Struct c_stCddRelayOpsConfigTable;


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* MCAL_IF_H_ */





















