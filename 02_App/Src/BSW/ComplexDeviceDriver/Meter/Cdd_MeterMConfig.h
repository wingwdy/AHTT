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

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_METERM_CFG_DEVICE_BL0942      0U
#define CDD_METERM_CFG_DEVICE_COUNT       1U
#define CDD_METERM_CFG_DEVICE_TYPE        CDD_METERM_CFG_DEVICE_BL0942
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





















