/******************************************************************************
* File Name          : Mcal_SPI.c
* Description        : Code for SPI configuration module for hardware
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
#include "Mcal_SPIConfig.h"


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
static void McalSPI_CfgChannel(McalSPIConfig_Struct *pSPICfg);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void McalSPI_CfgChannel(McalSPIConfig_Struct *pSPICfg)
{
    PARA_ASSERT(pSPICfg != NULL);

    if (pSPICfg->enable == TRUE)
    {
        rcu_periph_clock_enable(pSPICfg->rcu_spi_periph);
        spi_init(pSPICfg->spi_periph, &pSPICfg->spi_initPara);
        spi_enable(pSPICfg->spi_periph);
        pSPICfg->initFlag = TRUE;
    }
}

void McalSPI_Init(void)
{
    uint8_t index = 0;

    for (index = 0; index < eMcalSPIChanel_Count; index++)
    {
       McalSPI_CfgChannel((McalSPIConfig_Struct *)&g_stSPIConfigTable[index]);
    }
}

GlobalRet_Enum McalSPI_TransmitData(McalSPIChanel_Enum eCh, uint8_t *pTxData, uint16_t dataLen)
{
    McalSPIConfig_Struct *pSPICfg = &g_stSPIConfigTable[eCh];

    PARA_ASSERT_RET(pSPICfg->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(eCh < eMcalSPIChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((pTxData != NULL && dataLen != 0), eGlobalRet_ParaInvalid);

    uint16_t dealTxLen = dataLen;
    uint8_t *pDealTxData = pTxData;

    while (dealTxLen > 0)
    {
        while(RESET == spi_i2s_flag_get(pSPICfg->spi_periph, SPI_FLAG_TBE)) {}
        spi_i2s_data_transmit(pSPICfg->spi_periph, pDealTxData[0]);
        pDealTxData++;
        dealTxLen--;
        while(RESET == spi_i2s_flag_get(pSPICfg->spi_periph, SPI_FLAG_RBNE)) {}
        spi_i2s_data_receive(pSPICfg->spi_periph);
    }

    return eGlobalRet_OK;
}

GlobalRet_Enum McalSPI_ReceiveData(McalSPIChanel_Enum eCh, uint8_t *pRxData, uint16_t dataLen)
{
    McalSPIConfig_Struct *pSPICfg = &g_stSPIConfigTable[eCh];

    PARA_ASSERT_RET(pSPICfg->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(eCh < eMcalSPIChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((pRxData != NULL && dataLen != 0), eGlobalRet_ParaInvalid);

    uint16_t dealRxLen = dataLen;
    uint16_t count = 0;
    uint8_t *pDealRxData = pRxData;

    while (dealRxLen > 0)
    {
        while(RESET == spi_i2s_flag_get(pSPICfg->spi_periph, SPI_FLAG_TBE)) {}
        spi_i2s_data_transmit(pSPICfg->spi_periph, 00);

        while(RESET == spi_i2s_flag_get(pSPICfg->spi_periph, SPI_FLAG_RBNE)) {}
        pDealRxData[0] = spi_i2s_data_receive(pSPICfg->spi_periph);
        pDealRxData++;
        dealRxLen--;
    }

    return eGlobalRet_OK;
}

GlobalRet_Enum McalSPI_TransmitSyncReceiveData(McalSPIChanel_Enum eCh, uint8_t *pTxData, 
    uint16_t txLen, uint8_t *pRxData, uint16_t rxLen)
{
    McalSPIConfig_Struct *pSPICfg = &g_stSPIConfigTable[eCh];

    PARA_ASSERT_RET(pSPICfg->initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(eCh < eMcalSPIChanel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((pTxData != NULL && txLen == 0 && pRxData != NULL && rxLen == 0), eGlobalRet_ParaInvalid);

    McalSPI_TransmitData(eCh, pTxData, txLen);
    McalSPI_ReceiveData(eCh, pRxData, rxLen);

    return eGlobalRet_OK;
}



















