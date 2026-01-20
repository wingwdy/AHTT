/******************************************************************************
* File Name          : Cdd_Drv_LS5120.c
* Description        : Code for the device driver for LS5120
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "SysCfg.h"
#include "Cdd_Drv_LS5120Config.h"
#include "Cdd_Drv_LS5120.h"
#include "Asw_ErrorHandle.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define COMIRQ_REG_TIMEOUT              (30/5)//30ms
#define CRCCALC_REG_TIMEOUT             (15/5)//15ms


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

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void CddDrvLS5120_DelayUs(uint32_t us)
{
    uint32_t i = 0;

    for (i = 0; i < 10*us; i ++)
    {
        __NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();
    }
}

uint8_t CddDrvLS5120_ReadReg(uint8_t regAddr)
{
    uint8_t addr = 0;
    uint8_t ret = 0;

    CDD_DRV_CFG_LS5120_SPI_CS_LOW();
    addr = ((regAddr << 1) & 0x7E) | 0x80;
    CddDrvLS5120_SPI_Write(addr);
    ret = CddDrvLS5120_SPI_Read();
    CDD_DRV_CFG_LS5120_SPI_CS_HIGH();

    return ret;
}

void CddDrvLS5120_WriteReg(uint8_t regAddr, uint8_t value)
{
    uint8_t addr = 0;

    CDD_DRV_CFG_LS5120_SPI_CS_LOW();
    addr = ((regAddr << 1) & 0x7E);
    CddDrvLS5120_SPI_Write(addr);
    CddDrvLS5120_SPI_Write(value);
    CDD_DRV_CFG_LS5120_SPI_CS_HIGH();
}

void CddDrvLS5120_SetBitMask(uint8_t regAddr, uint8_t mask)
{
    uint8_t temp = 0;

    temp = CddDrvLS5120_ReadReg(regAddr);
    CddDrvLS5120_WriteReg(regAddr, temp | mask);
}

void CddDrvLS5120_ClrBitMask(uint8_t regAddr, uint8_t mask)
{
    uint8_t temp = 0;

    temp = CddDrvLS5120_ReadReg(regAddr);
    CddDrvLS5120_WriteReg(regAddr, temp & ~mask);
}

// void CddDrvLS5120_HardwareReset(void)
// {
//     CDD_DRV_CFG_LS5120_RESET_LOW();
//     CddDrvLS5120_DelayUs(1);/*>100ns*/
//     CDD_DRV_CFG_LS5120_RESET_HIGH();
//     CddDrvLS5120_DelayUs(100);/* tosc = td + tstartup > 50us +  */
// }

void CddDrvLS5120_SoftwareReset(void)
{
    CddDrvLS5120_WriteReg(CommandReg, PCD_RESETPHASE);
}

uint8_t CddDrvLS5120_VersionCheck(void)
{
    uint8_t ret = eGlobalRet_Error;

    if (CDD_DRV_LS5120_CHIP_VER == CddDrvLS5120_ReadReg(VersionReg))
    {
        ret = eGlobalRet_OK;
    }

    return ret;
}

void CddDrvLS5120_RegisterInit(void)
{
	CddDrvLS5120_WriteReg(ModeReg, 0x3D);       /* 模式设置 */
	CddDrvLS5120_WriteReg(TReloadRegLSB, 30);   /* 设置定时器重装值 */
	CddDrvLS5120_WriteReg(TReloadRegMSB, 0);
	CddDrvLS5120_WriteReg(TModeReg, 0x8D);
	CddDrvLS5120_WriteReg(TPrescalerReg, 0x3E);
	CddDrvLS5120_WriteReg(TxASKReg, 0x40);
	CddDrvLS5120_WriteReg(ControlReg, 0x10);
}

void CddDrvLS5120_OpenAntenna(uint8_t idx)
{
    uint8_t val = 0;
    CddDrvLS5120_ClrBitMask(TxControlReg, 0x03);
    if (idx == 0)
    {
        idx = 0x03;/* 全部开启 */
    }
    val = CddDrvLS5120_ReadReg(TxControlReg);
    if (!(val & idx))
    {
        CddDrvLS5120_SetBitMask(TxControlReg, idx);
    }
}

void CddDrvLS5120_CloseAntenna(void)
{
    CddDrvLS5120_ClrBitMask(TxControlReg, 0x03);
}

void CddDrvLS5120_PcdConfigType(uint8_t type)
{
    if (type == 'A')
    {
        CddDrvLS5120_ClrBitMask(Status2Reg, 0x08);
        CddDrvLS5120_WriteReg(ModeReg, 0x3D);
        CddDrvLS5120_WriteReg(TxSelReg, 0x10);
        CddDrvLS5120_WriteReg(RxSelReg, 0x86);
        CddDrvLS5120_WriteReg(RFCfgReg, 0x7F);
        CddDrvLS5120_WriteReg(TReloadRegLSB, 0x39);
        CddDrvLS5120_WriteReg(TReloadRegMSB, 0);
        CddDrvLS5120_WriteReg(TModeReg, 0x86);
        CddDrvLS5120_WriteReg(TPrescalerReg, 0x9F);
    }
}

uint8_t CddDrvLS5120_PcdCommAuth(uint8_t *pData, uint8_t dataLen)
{
    static uint8_t optStep = 0;
    static uint8_t timeOutCnt = COMIRQ_REG_TIMEOUT;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint16_t i = 0;
    uint8_t regData = 0;

    if (pData == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        if (optStep == 0)
        { 
            optStep = 1;
            timeOutCnt = COMIRQ_REG_TIMEOUT;
            CddDrvLS5120_WriteReg(ComIEnReg, (0x12 | 0x80));
            CddDrvLS5120_ClrBitMask(ComIrqReg, 0x80);
            CddDrvLS5120_WriteReg(CommandReg, PCD_IDLE);
            CddDrvLS5120_SetBitMask(FIFOLevelReg, 0x80);

            for (i = 0; i < dataLen; i++ )
            {
                CddDrvLS5120_WriteReg(FIFODataReg, pData[i]);
            }
            CddDrvLS5120_WriteReg(CommandReg, PCD_AUTHENT);
        }
        else if (optStep == 1)
        {
            if (timeOutCnt)
            {
                timeOutCnt--;
            }
            regData = CddDrvLS5120_ReadReg(ComIrqReg);
            if ((timeOutCnt == 0) || (regData & 0x01) || (regData & 0x10))
            {
                optStep =  2;
            }
        }
        else
        {}

        if (optStep == 2)
        {
            optStep = 0;
            CddDrvLS5120_ClrBitMask(BitFramingReg, 0x80);
            if (0 == (CddDrvLS5120_ReadReg(ErrorReg) & 0x1B))
            {
                optStatus = GLOBAL_OPT_STATE_SUCCESS;
            }
            else
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
            CddDrvLS5120_SetBitMask(ControlReg, 0x80);
            CddDrvLS5120_WriteReg(CommandReg, PCD_IDLE);
        }

    }

    return optStatus;
}

uint8_t CddDrvLS5120_PcdCommTransceive(uint8_t *pData, uint8_t dataLen, uint8_t *pDataOut, uint16_t *pBitLenOut)
{
    static uint8_t optStep = 0;
    static uint8_t timeOutCnt = COMIRQ_REG_TIMEOUT;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint16_t i = 0;
    uint8_t regData = 0;
    uint8_t fifoBytes = 0, lastBits = 0;

    if(pData == NULL || pDataOut == NULL || pBitLenOut == NULL || dataLen > MAX_FIFO_LEN)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        if (optStep == 0)
        {
            optStep = 1;
            timeOutCnt = COMIRQ_REG_TIMEOUT;
            CddDrvLS5120_WriteReg(ComIEnReg, (0x77 | 0x80));
            CddDrvLS5120_ClrBitMask(ComIrqReg, 0x80);
            CddDrvLS5120_WriteReg(CommandReg, PCD_IDLE);
            CddDrvLS5120_SetBitMask(FIFOLevelReg, 0x80);

            for (i = 0; i < dataLen; i++ )
            {
                CddDrvLS5120_WriteReg(FIFODataReg, pData[i]);
            }
            CddDrvLS5120_WriteReg(CommandReg, PCD_TRANSCEIVE);
            CddDrvLS5120_SetBitMask(BitFramingReg, 0x80);
        }

        if (optStep == 1)
        {
            if (timeOutCnt)
            {
                timeOutCnt--;
            }
            regData = CddDrvLS5120_ReadReg(ComIrqReg);
            if ((timeOutCnt == 0) || (regData & 0x01) || (regData & 0x30))
            {
                optStep = 2;
            }
        }

        if (optStep == 2)
        {
            optStep = 0;
            CddDrvLS5120_ClrBitMask(BitFramingReg, 0x80);
            if (0 == (CddDrvLS5120_ReadReg(ErrorReg) & 0x1B))
            {
                if (regData & 0x77 & 0x01)
                {
                    optStatus = GLOBAL_OPT_STATE_FAIL;
                }
                else
                {
                    fifoBytes = CddDrvLS5120_ReadReg(FIFOLevelReg);
                    lastBits = CddDrvLS5120_ReadReg(ControlReg) & 0x07;
                    if (lastBits)
                    {
                        *pBitLenOut = (fifoBytes - 1)*8 + lastBits;
                    }
                    else
                    {
                        *pBitLenOut = fifoBytes*8;
                    }
                    if (fifoBytes > MAX_RECV_LEN)
                    {
                        fifoBytes = MAX_RECV_LEN;
                    }
                    for (i = 0; i < fifoBytes; i++)
                    {
                        pDataOut[i] = CddDrvLS5120_ReadReg(FIFODataReg);
                    }

                    optStatus = GLOBAL_OPT_STATE_SUCCESS;
                }
            }
            else
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
            CddDrvLS5120_SetBitMask(ControlReg, 0x80);
            CddDrvLS5120_WriteReg(CommandReg, PCD_IDLE);
        }
    }

    return optStatus;
}

uint8_t CddDrvLS5120_CalulateCRC(uint8_t *pData, uint8_t dataLen, uint8_t *pDataOut)
{
    static uint8_t optStep = 0;
    static uint8_t timeOutCnt = CRCCALC_REG_TIMEOUT;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
	uint8_t i = 0;
    uint8_t regData = 0;
	
    if (pData == NULL || pDataOut == NULL || dataLen > MAX_FIFO_LEN)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        if (optStep == 0)
        {
            optStep = 1;
            timeOutCnt = CRCCALC_REG_TIMEOUT;
            CddDrvLS5120_ClrBitMask(DivIrqReg, 0x04);
            CddDrvLS5120_WriteReg(CommandReg, PCD_IDLE);
            CddDrvLS5120_SetBitMask(FIFOLevelReg, 0x80);

            for (i = 0; i < dataLen; i++)
            {
                CddDrvLS5120_WriteReg(FIFODataReg, *(pData + i));
            }

            CddDrvLS5120_WriteReg(CommandReg, PCD_CALCCRC);
        }
        else if (optStep == 1)
        {
            if (timeOutCnt)
            {
                timeOutCnt--;
            }
            regData = CddDrvLS5120_ReadReg(DivIrqReg);
            if ((timeOutCnt == 0) || (regData & 0x04))
            {
                optStep = 2;
            }
        }
        else
        {}

        if (optStep == 2)
        {
            optStep = 0;
            pDataOut[0] = CddDrvLS5120_ReadReg(CRCResultRegLSB);
            pDataOut[1] = CddDrvLS5120_ReadReg(CRCResultRegMSB);
            optStatus = GLOBAL_OPT_STATE_SUCCESS;
        }
    }

    return optStatus;
}


uint8_t CddDrvLS5120_PcdRequest(uint8_t reqCode, uint8_t *pTagType)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint16_t outLen = 0;
    uint8_t sndData = reqCode;
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};

    if (optStep == 0)
    {
        optStep = 1;
        CddDrvLS5120_ClrBitMask(Status2Reg, 0x08);
        CddDrvLS5120_WriteReg(BitFramingReg, 0x07);
    }
    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(&sndData, 1, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS && outLen == 0x10)
            {
                *pTagType = rcvBuffer[0];
                *(pTagType + 1) = rcvBuffer[1];
            }
            else
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }

	return optStatus;
}

uint8_t CddDrvLS5120_PcdAntiCollision(uint8_t *pOutUid, uint8_t *pUidLenOut)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t i = 0;
    uint8_t xorUid = 0;
    uint16_t outLen = 0;
    uint8_t sndBuffer[2] = {PICC_ANTICOLL1, 0x20};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};
	*pUidLenOut = 0;
	
    if (optStep == 0)
    {
        optStep = 1;
        CddDrvLS5120_ClrBitMask(Status2Reg, 0x08);
        CddDrvLS5120_WriteReg(BitFramingReg, 0x00);
        CddDrvLS5120_ClrBitMask(CollReg, 0x80);
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 2, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            CddDrvLS5120_SetBitMask(CollReg, 0x80);
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                for (i = 0; i < 4; i++)
                {
                    *(pOutUid + i) = rcvBuffer[i];
                    xorUid ^= rcvBuffer[i];
                }
                // if ((xorUid != rcvBuffer[4]) || (outLen < 40))
                if ((xorUid != rcvBuffer[4]))
                {
                    *(pOutUid) = 0;
                    optStatus = GLOBAL_OPT_STATE_FAIL;
                }
            }
        }
    }

    return optStatus;
}

uint8_t CddDrvLS5120_PcdSelect(uint8_t *pUid)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t i = 0;
    uint16_t outLen = 0;
    static uint8_t sndBuffer[18] = {0};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};

    if (optStep == 0)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_ANTICOLL1;
        sndBuffer[1] = 0x70;
        sndBuffer[6] = 0x00;
        for (i = 0; i < 4; i++)
        {
            sndBuffer[2 + i] = *(pUid + i);
            sndBuffer[6]  ^= *(pUid + i);
        }
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 7, &sndBuffer[7]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 1;
            // optStatus = GLOBAL_OPT_STATE_PROCESS;
            CddDrvLS5120_ClrBitMask(Status2Reg, 0x08);
        }
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 9, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (outLen != 0x18)
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }

    return optStatus;
}

uint8_t CddDrvLS5120_PcdAuthKeyA(uint8_t addr, const uint8_t *pKey, const uint8_t *pUid)
{
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t sndBuffer[18] = {0};

    sndBuffer[0] = PICC_AUTHENT1A;
    sndBuffer[1] = addr;
    memcpy(sndBuffer + 2, pKey, 6);
    memcpy(sndBuffer + 8, pUid, 4);
    optStatus = CddDrvLS5120_PcdCommAuth(sndBuffer, 12);
    if (optStatus != GLOBAL_OPT_STATE_PROCESS)
    {
        if (0 == (CddDrvLS5120_ReadReg(Status2Reg) & 0x08))
        {
            optStatus = GLOBAL_OPT_STATE_FAIL;
        }
    }

    return optStatus;
}

uint8_t CddDrvLS5120_PcdReadSector(uint8_t addr, uint8_t *pDataOut)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint16_t outLen = 0;
    static uint8_t sndBuffer[18] = {0};
    static uint8_t buffer[MAX_RECV_LEN] = {0};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};
    

    if (optStep == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_READ;
        sndBuffer[1] = addr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 1;
        }
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS && (outLen == 0x90))
            {
                #if 0
                    memcpy(pDataOut, rcvBuffer, 16);
                #else
                    optStep = 2;
                    memcpy(buffer, rcvBuffer, 18);
                #endif
            }
            else
            {
                
                optStatus = GLOBAL_OPT_STATE_FAIL;  
            }
        }
    }

#if 0 /* 是否需要CRC */


#else
    if (optStep == 2)
    {/* 校验数据 */
        optStatus = CddDrvLS5120_CalulateCRC(buffer, 16, rcvBuffer);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS && buffer[16] == rcvBuffer[0] && buffer[17] == rcvBuffer[1])
            {
                memcpy(pDataOut, buffer, 16);
            }
            else
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }

#endif


    return optStatus;
}

uint8_t CddDrvLS5120_PcdWriteSector(uint8_t addr, uint8_t *pData)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint16_t outLen = 0;
    static uint8_t sndBuffer[16] = {0};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};

    if (optStep == 0)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_WRITE;
        sndBuffer[1] = addr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 1;
        }
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
            else if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            { 
                optStep = 2;
            }
            else
            {}
        }
    }

    if (optStep == 2)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        memcpy(sndBuffer, pData, 16);
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 16, &sndBuffer[16]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 3;
        }
    }

    if (optStep == 3)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 18, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }
    
   return optStatus;
}

uint8_t CddDrvLS5120_PcdValue(uint8_t dd_mode, uint8_t addr, uint8_t *pValue)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t i = 0;
    uint16_t outLen = 0;
    static uint8_t sndBuffer[16] = {0};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};

    if (optStep == 0)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = dd_mode;
        sndBuffer[1] = addr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 1;
        }
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
            else if(optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 2;
            }
            else
            {}
        }
    }

    if (optStep == 2)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        memcpy(sndBuffer, pValue, 4);
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 4, &sndBuffer[4]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 3;
        }
    }

    if (optStep == 3)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 6, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 4;
            }
        }
    }

    if (optStep == 4)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_TRANSFER;
        sndBuffer[1] = addr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 5;
        }
    }

    if (optStep == 5)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }
		
    return optStatus;
}

uint8_t CddDrvLS5120_PcdBakValue(uint8_t srcAddr, uint8_t dstAddr)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t i = 0;
    uint16_t outLen = 0;
    static uint8_t sndBuffer[16] = {0};
    uint8_t rcvBuffer[MAX_RECV_LEN] = {0};

    if (optStep == 0)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_RESTORE;
        sndBuffer[1] = srcAddr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 1;
        }
    }

    if (optStep == 1)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
            else if(optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 2;
            }
            else
            {}
        }
    }

    if (optStep == 2)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 4, &sndBuffer[4]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 3;
        }
    }

    if (optStep == 3)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 6, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 4;
            }
        }
    }

    if (optStep == 4)
    {
        memset(sndBuffer, 0, sizeof(sndBuffer));
        sndBuffer[0] = PICC_TRANSFER;
        sndBuffer[1] = dstAddr;
        optStatus = CddDrvLS5120_CalulateCRC(sndBuffer, 2, &sndBuffer[2]);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 5;
        }
    }

    if (optStep == 5)
    {
        optStatus = CddDrvLS5120_PcdCommTransceive(sndBuffer, 4, rcvBuffer, &outLen);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if ((outLen != 4) || ((rcvBuffer[0] & 0x0F) != 0x0A))
            {
                optStatus = GLOBAL_OPT_STATE_FAIL;
            }
        }
    }

    return optStatus;
}

void CddDrvLS5120_Init(void)
{
	// CddDrvLS5120_HardwareReset();
	//CddDrvLS5120_SoftwareReset();
	//CddDrvLS5120_DelayUs(10);
    CddDrvLS5120_RegisterInit();
	CddDrvLS5120_PcdConfigType('A');
	CddDrvLS5120_CloseAntenna();
	CddDrvLS5120_OpenAntenna(0);
}

void CddDrvLS5120_HardwareResetStart(void)
{
    CddDrvLS5120_CHIP_HardwareResetStart();
}

void CddDrvLS5120_HardwareResetEnd(void)
{
    CddDrvLS5120_CHIP_HardwareResetEnd();
}
