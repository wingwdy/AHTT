/******************************************************************************
* File Name          : Cdd_Drv_BL0942Config.c
* Description        : Code for Configuration of BL0942 interface
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Mcal_Uart.h"
#include "Global.h"
#include "Cdd_Drv_BL0942Config.h"
#include "Cdd_Drv_BL0942.h" 


/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t g_caliRegVal[eCddDrvBL0942CaliReg_Cnt][3] = 
{
    {0xC7, 0x00, 0x00},
//  {0x55, 0x0D, 0x00},
//  {0xD8, 0x0D, 0x00},
//  {0x06, 0x00, 0x00},
//  {0x00, 0x00, 0x00},
    {0x16, 0x00, 0x00},    // 0B 对应的是0.6W
}; 

const CddDrvBL0942WriteRegister_Struct c_stCddDrvBL0942WriteRegisterTable[eCddDrvBL0942CaliReg_Cnt] =
{
    {CDDDRV_BL0942_REG_MODE,     0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_V_MODE]    },
//  {CDDDRV_BL0942_REG_I_CHGN,   0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_I_CHGN]    },
//  {CDDDRV_BL0942_REG_V_CHGN,   0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_V_CHGN]    },
//  {CDDDRV_BL0942_REG_PHCAL,    0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_PHCAL]     },
//  {CDDDRV_BL0942_REG_WATTOS,   0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_WATTOS]    },
    {CDDDRV_BL0942_REG_WA_CREEP, 0x03,    g_caliRegVal[eCddDrvBL0942CaliReg_WA_CREEP]  },
};   

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void CddDrvBL0942Cfg_WriteData(uint8_t port, uint8_t *pData, uint16_t length)
{
    if (port == 0)
    {
        McalUart_WriteData(eMcalUartChanel_MeterChip, pData, length);
    }
}

void CddDrvBL0942Cfg_ResetRecvBuf(uint8_t port)
{
    if (port == 0)
    {
        McalUart_ResetRecvBuf(eMcalUartChanel_MeterChip);
    }
}

GlobalRet_Enum CddDrvBL0942Cfg_ReadData(uint8_t port, uint8_t *pData, uint16_t length)
{
    uint16_t mcalRecvLen = 0;
    GlobalRet_Enum ret = eGlobalRet_Error;

    if (port == 0)
    {
        ret = McalUart_CheckDataLen(eMcalUartChanel_MeterChip, &mcalRecvLen);
        
        if (ret == eGlobalRet_OK)
        {
            ret = McalUart_ReadData(eMcalUartChanel_MeterChip, pData, length);
        }
    }

    return ret;
}













