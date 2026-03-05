/******************************************************************************
* File Name          : W25QXX.c
* Description        : Code for the drvier of norflash
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
#include "W25QXX_Config.h"
#include "W25QXX.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
/* 华邦Flash命令集 */
#define W25Q_CMD_WRITE_ENABLE                          0x06
#define W25Q_CMD_WRITE_DISABLE                         0x04
#define W25Q_CMD_READ_STATUS1                          0x05
#define W25Q_CMD_READ_STATUS2                          0x35
#define W25Q_CMD_WRITE_STATUS                          0x01
#define W25Q_CMD_READ_DATA                             0x03
#define W25Q_CMD_FAST_READ                             0x0B
#define W25Q_CMD_PAGE_PROGRAM                          0x02
#define W25Q_CMD_SECTOR_ERASE                          0x20
#define W25Q_CMD_CHIP_ERASE                            0xC7
#define W25Q_CMD_POWER_DOWN                            0xB9
#define W25Q_CMD_RELEASE_POWER                         0xAB
#define W25Q_CMD_READ_ID                               0x9F

/* 状态寄存器位 */
#define W25Q_STATUS_BUSY                               0x01
#define W25Q_STATUS_WEL                                0x02

#define W25Q_OPT_BUSY                                  0x01
#define W25Q_OPT_IDLE                                  0x00


/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct W25QXX
{
    uint8_t initFlag;               /* 初始化标记 */
    uint8_t optState;               /* 操作状态  */
}W25QXXCtrl_Struct;



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static W25QXXCtrl_Struct g_stW25qxx = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t W25Q_CheckDeviceId(void)
{
    uint8_t cmd = W25Q_CMD_READ_ID;
    uint8_t id[3] = {0};
    uint32_t chipID = 0;
    uint8_t ret = FALSE;

    W25Q_CFG_CS_LOW();
    W25Q_CFG_SpiTransmit(&cmd, 1);
    W25Q_CFG_SpiReceive(id, 3);
    W25Q_CFG_CS_HIGH();

    chipID = (id[0] << 16) | (id[1] << 8) | (id[2]);

    if (chipID == W25Q_CHIP_ID)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t W25Q_WaitForIdle(void)
{
    uint32_t timeout = W25Q_TIMEOUT_VALUE;
    uint8_t status = 0xFF;
    uint8_t cmd = W25Q_CMD_READ_STATUS1;
    uint8_t ret = FALSE;

    do 
    {
        W25Q_CFG_CS_LOW();
        W25Q_CFG_SpiTransmit(&cmd, 1);
        W25Q_CFG_SpiReceive(&status, 1);
        W25Q_CFG_CS_HIGH();
        
        if ((status & 0x01U) == 0U)
        {
            ret = TRUE;
            break;
        }
        
        timeout--;
    } while (timeout > 0);

    return ret;
}

static void W25Q_ResetChip(void)
{
    uint8_t cmd = W25Q_CMD_POWER_DOWN;

    W25Q_CFG_CS_LOW();
    W25Q_CFG_SpiTransmit(&cmd, 1);
    W25Q_CFG_CS_HIGH();
}

static void W25Q_WriteEnable(void)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    uint8_t status = 0;

    W25Q_CFG_CS_LOW();
    W25Q_CFG_SpiTransmit(&cmd, 1);
    W25Q_CFG_CS_HIGH();
}

static void W25Q_WriteDisable(void)
{
    uint8_t cmd = W25Q_CMD_WRITE_DISABLE;
    uint8_t status = 0;

    W25Q_CFG_CS_LOW();
    W25Q_CFG_SpiTransmit(&cmd, 1);
    W25Q_CFG_CS_HIGH();
}

GlobalRet_Enum W25Q_Init(void) 
{   
    GlobalRet_Enum eRet = eGlobalRet_OK;
    memset(&g_stW25qxx, 0x00, sizeof(g_stW25qxx));

    if (TRUE != W25Q_CheckDeviceId())
    {
        eRet = eGlobalRet_Error;
    }
    else
    {
        g_stW25qxx.initFlag = TRUE;
    }

    g_stW25qxx.optState = W25Q_OPT_IDLE;
    return eRet;
}

GlobalRet_Enum W25Q_Read(uint32_t srcAddr, uint8_t* targetAddr, uint32_t len)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;

    PARA_ASSERT_RET(len != 0 && targetAddr != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((srcAddr + len) <= W25Q_CFG_TOTAL_SIZE, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stW25qxx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(g_stW25qxx.optState == W25Q_OPT_IDLE, eGlobalRet_DeviceBusy);

    g_stW25qxx.optState = W25Q_OPT_BUSY;

    uint8_t dummy = 0xFF;
    uint8_t cmd[4] = { 0 };
    cmd[0] = W25Q_CMD_FAST_READ;
    cmd[1] = (srcAddr >> 16) & 0xFF;  // 地址高位
    cmd[2] = (srcAddr >> 8) & 0xFF;   // 地址中位
    cmd[3] = srcAddr & 0xFF;          // 地址低位

    W25Q_CFG_SpiTransmit(&dummy, 1);

    W25Q_CFG_CS_LOW();
    W25Q_CFG_SpiTransmit(cmd, 4);
    W25Q_CFG_SpiTransmit(&dummy, 1);
    W25Q_CFG_SpiReceive(targetAddr, len);
    W25Q_CFG_CS_HIGH();

    if (TRUE == W25Q_WaitForIdle())
    {
        g_stW25qxx.optState = W25Q_OPT_IDLE;

    }
    else
    {
        eRet = eGlobalRet_Error;
    }

    return eRet;
}
GlobalRet_Enum W25Q_Write(uint32_t targetAddr, const uint8_t *srcAddr, uint32_t len)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint32_t remaining = len;
    uint32_t currentAddr = targetAddr;
    const uint8_t *dataPtr = srcAddr;
    uint8_t cmdFrame[4] = { 0 };
    uint32_t pageOffset = 0;
    uint32_t bytesToWrite = 0;
    uint8_t dummy = 0xFF;

    PARA_ASSERT_RET(len != 0 && srcAddr != NULL, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((targetAddr + len) <= W25Q_CFG_TOTAL_SIZE, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stW25qxx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(g_stW25qxx.optState == W25Q_OPT_IDLE, eGlobalRet_DeviceBusy);

    g_stW25qxx.optState = W25Q_OPT_BUSY;

    while (remaining > 0)
    {
        pageOffset = currentAddr % W25Q_CFG_PAGE_SIZE;

        if (remaining < (W25Q_CFG_PAGE_SIZE - pageOffset))
        {
            bytesToWrite = remaining;
        }
        else
        {
            bytesToWrite = W25Q_CFG_PAGE_SIZE - pageOffset;
        }
        W25Q_WriteEnable();

        W25Q_CFG_SpiTransmit(&dummy, 1);
 
        cmdFrame[0] = W25Q_CMD_PAGE_PROGRAM;
        cmdFrame[1] = (currentAddr >> 16) & 0xFF;
        cmdFrame[2] = (currentAddr >> 8) & 0xFF;
        cmdFrame[3] = currentAddr & 0xFF;

        W25Q_CFG_CS_LOW();
        W25Q_CFG_SpiTransmit(cmdFrame, 4);
        W25Q_CFG_SpiTransmit((uint8_t *)dataPtr, bytesToWrite);
        W25Q_CFG_CS_HIGH();
        if (TRUE != W25Q_WaitForIdle())
        {
            eRet = eGlobalRet_Error;
            break;
        }

        W25Q_WriteDisable();
        currentAddr += bytesToWrite;
        dataPtr += bytesToWrite;
        remaining -= bytesToWrite;
    }
    
    g_stW25qxx.optState = W25Q_OPT_IDLE;
    return eRet;
}

GlobalRet_Enum W25Q_Erase(uint32_t targetAddr, uint32_t len)
{
    GlobalRet_Enum eRet = eGlobalRet_OK;
    uint32_t remaining = len;
    uint32_t currentAddr = targetAddr;
    uint8_t cmdFrame[4] = { 0 };
    uint32_t pageOffset = 0;
    uint32_t bytesToWrite = 0;
    uint8_t dummy = 0xFF;

    PARA_ASSERT_RET(len != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET((targetAddr + len) <= W25Q_CFG_TOTAL_SIZE, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(targetAddr % W25Q_CFG_SECTOR_SIZE == 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(len % W25Q_CFG_SECTOR_SIZE == 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(g_stW25qxx.initFlag == TRUE, eGlobalRet_NotInit);
    PARA_ASSERT_RET(g_stW25qxx.optState == W25Q_OPT_IDLE, eGlobalRet_DeviceBusy);

    g_stW25qxx.optState = W25Q_OPT_BUSY;

    while (remaining > 0)
    {
        W25Q_WriteEnable();
        g_stW25qxx.optState = W25Q_OPT_BUSY;

        cmdFrame[0] = W25Q_CMD_SECTOR_ERASE;
        cmdFrame[1] = (currentAddr >> 16) & 0xFF;
        cmdFrame[2] = (currentAddr >> 8) & 0xFF;
        cmdFrame[3] = currentAddr & 0xFF;

        W25Q_CFG_SpiTransmit(&dummy, 1);

        W25Q_CFG_CS_LOW();
        W25Q_CFG_SpiTransmit(cmdFrame, 4);
        W25Q_CFG_CS_HIGH();

        if (TRUE != W25Q_WaitForIdle())
        {
            eRet = eGlobalRet_Error;
            break;
        }

        W25Q_CFG_FeedWdg();

        currentAddr += W25Q_CFG_SECTOR_SIZE;
        remaining -= W25Q_CFG_SECTOR_SIZE;
        W25Q_WriteDisable();
    }

    g_stW25qxx.optState = W25Q_OPT_IDLE;

    return eRet;
}








