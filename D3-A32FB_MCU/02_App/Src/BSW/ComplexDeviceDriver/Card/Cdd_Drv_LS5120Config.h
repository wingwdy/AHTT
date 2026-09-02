/******************************************************************************
* File Name          : Cdd_Drv_LS5120Config.h
* Description        : Code for Configuration of LS5120 interface
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
******************************************************************************/
#ifndef CDD_DRV_LS5120_CONFIG_H_
#define CDD_DRV_LS5120_CONFIG_H_


/******************************************************************************
*    Include Files
******************************************************************************/
#include "Mcal_Port.h"
#include "Mcal_SPI.h"
#include "Global.h"
#include "DS_LogM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_DRV_CFG_LS5120_RESET_LOW()             McalPort_ResetPin(eMcalPortPinChanel_PA15_NFCRST3)
#define CDD_DRV_CFG_LS5120_RESET_HIGH()            McalPort_SetPin(eMcalPortPinChanel_PA15_NFCRST3)
#define CDD_DRV_CFG_LS5120_SPI_CS_LOW()            McalPort_ResetPin(eMcalPortPinChanel_PD2_NFCSpiCS)
#define CDD_DRV_CFG_LS5120_SPI_CS_HIGH()           McalPort_SetPin(eMcalPortPinChanel_PD2_NFCSpiCS)
#define CDD_DRV_CFG_LS5120_SPI_SCLK_LOW()          McalPort_ResetPin(eMcalPortPinChanel_PC10_NFCSpiCLK)
#define CDD_DRV_CFG_LS5120_SPI_SCLK_HIGH()         McalPort_SetPin(eMcalPortPinChanel_PC10_NFCSpiCLK)
#define CDD_DRV_CFG_LS5120_SPI_MOSI_LOW()          McalPort_ResetPin(eMcalPortPinChanel_PC12_NFCSpiMOSI)
#define CDD_DRV_CFG_LS5120_SPI_MOSI_HIGH()         McalPort_SetPin(eMcalPortPinChanel_PC12_NFCSpiMOSI)
#define CDD_DRV_CFG_LS5120_SPI_MISO_VALUE()        McalPort_GetPin(eMcalPortPinChanel_PC11_NFCSpiMISO)


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

/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t CddDrvLS5120_SPI_Read(void);
void CddDrvLS5120_SPI_Write(uint8_t byte);
void CddDrvLS5120_CHIP_HardwareResetStart(void);
void CddDrvLS5120_CHIP_HardwareResetEnd(void);







#endif /* CDD_DRV_LS5120Config_H_ */

