/******************************************************************************
* File Name          : Cdd_MeterMConfig.h
* Description        : Code for Configuration of metering interface
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
#ifndef CDD_METERM_CONFIG_H_
#define CDD_METERM_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "MS_Nvm.h"
#include "Global.h"
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_METERM_CFG_DEVICE_BL0942                0U
#define CDD_METERM_CFG_DEVICE_COUNT                 1U
#define CDD_METERM_CFG_DEVICE_TYPE                  CDD_METERM_CFG_DEVICE_BL0942

#define CDD_METERM_CFG_ERROR_TRY_CNT                (10)

#define CDD_METERM_CFG_PRINT_CYCLE                  (5 * 1000)

#define CDD_METERM_CFG_ENERGY_SAVE_PERIOD           (30 * 1000)

#define CDD_METERM_CFG_ENERGY_IMMEDIATE_SAVE_VALUE  (400)      /* 0.04度，立即存储 */

#define CDD_METERM_CFG_ENERGY_MIN_SAVE_VALUE        (10)       /* 0.001度，周期存储的最小电量 */

#define CDD_METERM_CFG_ENERGY_ABNORMAL_VALUE        (10000)      /* 1度 */

#define CDD_METERM_CFG_ENERGY_PERIOD                (1000U)

#define CDD_METERM_CFG_WriteBlockEnergy(port, energy, size)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0MeterEnergy, energy, size);\
                                                                }\
                                                            }while(0)

#define CDD_METERM_CFG_ReadBlockEnergy(port, energy, size, ret)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    ret = MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0MeterEnergy, energy, size);\
                                                                }\
                                                                else\
                                                                {\
                                                                    ret = eGlobalRet_Error;\
                                                                }\
                                                            }while(0)

#define CDD_METERM_CFG_LogPrint(fmt, ...)                   DSLOGM_Debug(DSLogMModule_Meter, fmt, ##__VA_ARGS__)

        

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{ 
    void (*pFuncInitMemory)(void);
    void (*pFuncMainFunction)(void);
    uint8_t (*pFuncGetReadyFlag)(uint8_t port);
    uint32_t (*pFuncGetPeriodEnergy)(uint8_t port);
    uint32_t (*pFuncGetPower)(uint8_t port);
    uint32_t (*pFuncGetRmsVoltage)(uint8_t port);
    uint32_t (*pFuncGetRmsCurrent)(uint8_t port);
} CddMeterMConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddMeterMConfig_Struct c_CddMeterMConfigTable[CDD_METERM_CFG_DEVICE_COUNT];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* CDD_METERM_CONFIG_H_ */





















