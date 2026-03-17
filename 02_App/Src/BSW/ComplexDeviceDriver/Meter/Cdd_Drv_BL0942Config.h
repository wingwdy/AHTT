/******************************************************************************
* File Name          : Cdd_Drv_BL0942Config.h
* Description        : Code for Configuration of BL0942 interface
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
#ifndef CDD_DRV_BL0942_CONFIG_H_
#define CDD_DRV_BL0942_CONFIG_H_


/******************************************************************************
*    Include Files
******************************************************************************/
#include "Mcal_Uart.h"
#include "Mcal_Port.h"
#include "MS_Nvm.h"
#include "Global.h"
#include "DS_LogM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDDRV_BL0942_CFG_CALL_CYCLE                (100u)

#define CDDDRV_BL0942_CFG_WRITE_TIMEOUT             (20U)
#define CDDDRV_BL0942_CFG_READ_TIMEOUT              (100U)

#define CDDDRV_BL0942_CFG_CALC_ENERGY_PERIOD        (1000U)

#define CDDDRV_BL0942_CFG_HEAD_WRITE                (0xA8u)  
#define CDDDRV_BL0942_CFG_HEAD_READ                 (0x58u)

#define CDDDRV_BL0942_CFG_VOLTAGE_K                 (7638U)     
#define CDDDRV_BL0942_CFG_CURRENT_K                 (129378U)     
#define CDDDRV_BL0942_CFG_POWER_K                   (153.51)    

#define CDDDRV_BL0942_CFG_PULSE_CONSTANT            (1312U)

#define CDDDRV_BL0942_CFG_MAX_TIMES                 (20U)

#define CDDDRV_BL0942_CFG_FCT_FILTER_POINT          (300 / CDDDRV_BL0942_CFG_CALL_CYCLE)


#define CDD_BL0942_CFG_WriteBlockCaliParam(port, caliParam, size)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0MeterCaliParam, caliParam, size);\
                                                                }\
                                                            }while(0)

#define CDD_BL0942_CFG_ReadBlockCaliParam(port, caliParam, size, ret)  do \
                                                            {\
                                                                if (port == 0) \
                                                                {\
                                                                    ret = MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0MeterCaliParam, caliParam, size);\
                                                                }\
                                                                else\
                                                                {\
                                                                    ret = eGlobalRet_Error;\
                                                                }\
                                                            }while(0)

#define CDDDRV_BL0942_CFG_LogPrint(fmt, ...)        DSLOGM_Debug(DSLogMModule_Meter, fmt, ##__VA_ARGS__)

#define CDDDRV_BL0942_CFG_CheckFCTPin()             (MCALPORT_PIN_HIGH == McalPort_GetPin(eMcalPortPinChanel_PC0_FCTPin) ? FALSE : TRUE)
/******************************************************************************
*    Typedef Definition
******************************************************************************/




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
    eCddDrvBL0942CaliReg_V_MODE,
//  eCddDrvBL0942CaliReg_I_CHGN,
//  eCddDrvBL0942CaliReg_V_CHGN,
//  eCddDrvBL0942CaliReg_PHCAL,
//  eCddDrvBL0942CaliReg_WATTOS,
    eCddDrvBL0942CaliReg_WA_CREEP,
    eCddDrvBL0942CaliReg_Cnt,
}eCddDrvBL0942CaliReg_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    eCddDrvBL0942CaliReg_Enum regAddr;
    uint8_t dataLen;
    uint8_t *pData;
}CddDrvBL0942WriteRegister_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddDrvBL0942WriteRegister_Struct c_stCddDrvBL0942WriteRegisterTable[eCddDrvBL0942CaliReg_Cnt];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddDrvBL0942Cfg_WriteData(uint8_t port, uint8_t *pData, uint16_t length);
GlobalRet_Enum CddDrvBL0942Cfg_ReadData(uint8_t port, uint8_t *pData, uint16_t length);
void CddDrvBL0942Cfg_ResetRecvBuf(uint8_t port);
#endif






















