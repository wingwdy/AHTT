/******************************************************************************
* File Name          : Mcal_SPI.h
* Description        : Code for SPI configuration module for hardware
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
#ifndef MCAL_SPI_H_
#define MCAL_SPI_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h" 

/******************************************************************************
*    Macro Definition
******************************************************************************/

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eMcalSPIChanel_Norflash,
    eMcalSPIChanel_Count,
}McalSPIChanel_Enum;




/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void McalSPI_Init(void);
GlobalRet_Enum McalSPI_TransmitData(McalSPIChanel_Enum eCh, uint8_t *pTxData, uint16_t dataLen);
GlobalRet_Enum McalSPI_ReceiveData(McalSPIChanel_Enum eCh, uint8_t *pRxData, uint16_t dataLen);
GlobalRet_Enum McalSPI_TransmitSyncReceiveData(McalSPIChanel_Enum eCh, uint8_t *pTxData, 
    uint16_t txLen, uint8_t *pRxData, uint16_t rxLen);
#endif /* MCAL_SPI_H_ */


















