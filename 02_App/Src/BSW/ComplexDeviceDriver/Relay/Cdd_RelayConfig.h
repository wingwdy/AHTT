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
#include "SysCfg.h"

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
#define CDDRELAY_CFG_SHORTCUT_UPPER_LIMIT                  260
#define CDDRELAY_CFG_SHORTCUT_LOWER_LIMIT                  200

#define CDDRELAY_CFG_SHORTCUT_TIMEOUT                      (5000)
#define CDDRELAY_CFG_SHORTCUT_FINSH_DELAY                  (500)
#define CDDRELAY_CFG_SHORTCUT_FILTER_COUNT                 (500 /  CDDRELAY_CFG_CALL_CYCLE)

#define CDDRELAY_CFG_CheckGunPlugout(port)                 (eCddCPVolState_12V == CddCP_GetVolState(port))

#define CDDRELAY_CFG_DebugPrint(fmt, ...)                  DSLOGM_Debug(DSLogMModule_RELAY, fmt, ##__VA_ARGS__)
#define CDDRELAY_CFG_InfoPrint(fmt, ...)                   DSLOGM_Info(DSLogMModule_RELAY, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    void (*pFuncCtrlSwitchOn)(void);
    void (*pFuncCtrlSwitchOff)(void);
    void (*pFuncHoldSwitchOn)(void);
    uint8_t (*pFuncGetSwitchStatus)(void);
    uint8_t (*pFuncGetRelayAdhesionStatus)(void);
    void (*pFuncCtrlShortCutOn)(void);
    void (*pFuncCtrlShortCutOff)(void);
    uint8_t (*pFuncGetShortCutStatus)(void);
}CddRelayOpsConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddRelayOpsConfig_Struct c_stCddRelayOpsConfigTable[SYSCFG_CFG_GUN_NUM];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* MCAL_IF_H_ */





















