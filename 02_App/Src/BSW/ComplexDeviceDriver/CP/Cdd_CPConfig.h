/******************************************************************************
* File Name          : Cdd_CPConfig.h
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
#ifndef CDD_CP_CONFIG_H_
#define CDD_CP_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Cdd_CP.h"
#include "SysCfg.h"
#include "DS_LogM.h"
#include "Cdd_ModeM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDCP_CFG_CALL_CYCLE                  (10U)

#define CDDCP_CFG_RATE_CURRENT                SYSCFG_CFG_MAX_OUTPUT_CURRENT
#define CDDCP_CFG_RATE_MIN_CURRENT            (6000)
#define CDDCP_CFG_RATE_THRESOLD_CURRENT       (51000)

#define CDDCP_CFG_ADC_BUFF_POINT              (MCALADC_ADC0_SAMPLE_CNT)

#define CDDCP_CFG_IsQBStandardMode()          (CddModeM_IsGBMode() == FALSE)    

#define CDDCP_CFG_GB_FILERCNT                 (50  / CDDCP_CFG_CALL_CYCLE)
#define CDDCP_CFG_QB_FILERCNT                 (1000 / CDDCP_CFG_CALL_CYCLE)

#define CDDCP_CFG_DIODE_THREOLD               (10000)

#define CDDCP_CFG_DIODE_FILTER_POINT          (200 / CDDCP_CFG_CALL_CYCLE)

#define CDDCP_CFG_DIODE_DETECT_TIMEOUT        (500)

#define CDDCP_CFG_WAKEUP_LOW_HOLDTIME         30000
#define CDDCP_CFG_WAKEUP_HIGH_HOLDTIME        15000

#define CDDCP_CFG_DebugPrint(fmt, ...)        DSLOGM_Debug(DSLogMModule_CP, fmt, ##__VA_ARGS__)
#define CDDCP_CFG_InfoPrint(fmt, ...)         DSLOGM_Info(DSLogMModule_CP, fmt, ##__VA_ARGS__)

#define CDDCP_CFG_VOLSTATE_COUNT              (4)
/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    float (*pFuncGetCpVol)(uint8_t port);
    void (*pFunSetPwmDuty)(uint8_t port, uint16_t duty);
}CddCPOpsConfig_Struct;

typedef struct 
{
    uint16_t abnormalLowerVolLimit;
    uint16_t normalLowerVolLimit;
    uint16_t normalUpperVolLimit;
    uint16_t abnormalUpperVolLimit;
    uint16_t statefiterCnt;
    CddCPVolState_Enum eVoltageState;
}CddCPVolStateFilter_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddCPOpsConfig_Struct c_stCddCPOpsConfigTable;
extern const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterGB[4];
extern const CddCPVolStateFilter_Struct c_stCddCPVolStateFilterQB[4];
/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif























