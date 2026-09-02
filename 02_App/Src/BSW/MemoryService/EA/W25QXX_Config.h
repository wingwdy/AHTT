/******************************************************************************
* File Name          : W25QXX_Config.h
* Description        : Code for the drvier of norflash
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
#ifndef W25QXX_CONFIG_H_
#define W25QXX_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_SPI.h"
#include "Mcal_Port.h"
#include "Mcal_IWDG.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define W25Q_CFG_TOTAL_SIZE     0x400000u   /* 4M字节 */
#define W25Q_CFG_SECTOR_SIZE    0x1000u     /* 4KB扇区 */
#define W25Q_CFG_PAGE_SIZE      256u        /* 256字节页 */

#define W25Q_CFG_SpiTransmit(data, len)     McalSPI_TransmitData(eMcalSPIChanel_Norflash, data, len)
#define W25Q_CFG_SpiReceive(data, len)      McalSPI_ReceiveData(eMcalSPIChanel_Norflash, data, len)
#define W25Q_CFG_CS_LOW()                   McalPort_ResetPin(eMcalPortPinChanel_PB6_NorFlashSpiCS)
#define W25Q_CFG_CS_HIGH()                  McalPort_SetPin(eMcalPortPinChanel_PB6_NorFlashSpiCS)

#define W25Q_CFG_FeedWdg()                 McalIWDG_FeedWatchDog()



#define W25Q_CHIP_ID                        (0xEF4016U)
#define W25Q_TIMEOUT_VALUE                  (20000U)
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

#endif /* W25QXX_CONFIG_H_ */






















