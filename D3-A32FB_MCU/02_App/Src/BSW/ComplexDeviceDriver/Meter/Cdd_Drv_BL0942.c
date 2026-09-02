/******************************************************************************
* File Name          : Cdd_Drv_BL0942.c
* Description        : Code for the device driver for BL0942
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
#include "Cdd_Drv_BL0942Config.h" 
#include "SysCfg.h"
#include "Cdd_Drv_BL0942.h"
#include "Asw_ErrorHandle.h"
#include "Filter.h"
#include "MS_Nvm.h"
#include "Cdd_CP.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDDDRV_BL0942_DEFAULT_MODE                 (0x87)


#define CDDDRV_BL0942_CALI_STEP0                   0
#define CDDDRV_BL0942_CALI_STEP1                   1
#define CDDDRV_BL0942_CALI_STEP2                   2
#define CDDDRV_BL0942_CALI_STEP3                   3
#define CDDDRV_BL0942_CALI_STEP4                   4
/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eCddDrvBL0942WorkState_Init,
    eCddDrvBL0942WorkState_Cali,
    eCddDrvBL0942WorkState_Normal,
    eCddDrvBL0942WorkState_FCT,
    eCddDrvBL0942WorkState_Error,
}CddDrvBL0942WorkState_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    CddDrvBL0942WorkState_Enum eWorkState;
    uint32_t rms_voltage;                   /* 平均电压 保留2位小数  单位：V*/
    uint32_t rms_current;                   /* 平均电流 保留3位小数  单位：A*/
    uint32_t watt;                          /* 有功功率 保留3位小数  单位：KW*/
    uint64_t totalCount;                    /* 总脉冲数 */
    uint32_t tempCount;                     /* 临时脉冲数 */
    uint64_t energy;                        /* 临时累积电能 保留5位小数 */
    uint32_t period_tick;
    uint32_t tickStart;                     /* 数据发送开发时间 */
    eCddDrvBL0942CaliReg_Enum cfgMeterOject;/* 配置计量芯片寄存器对象 */
    uint8_t cfgMeterStep;                   /* 配置计量芯片步骤 */
    uint8_t optResult;                      /* 操作结果 */
    uint8_t cacheBuf[32];                   /* 缓存数据 */
    uint8_t failTryCount;                   /* 失败尝试次数 */
    FilterProfile1_Struct fctPinStatusFilter;
    uint8_t fctCaliFlag;
    MSNvmMeterCaliParam_Struct caliParam;
    uint8_t comErrorFlag;                   /* 通信错误标记 */
}CddDrvBL0942_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddDrvBL0942_Struct g_stCddDrvBL0942[SYSCFG_CFG_GUN_NUM];


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void CddDrvBL0942_SendData(uint8_t port, CddDrvBL0942_Struct *pBL0942, uint8_t cmd, uint8_t regAddr, uint8_t *pData, uint8_t dataLen);
static uint8_t CddDrvBL0942_ReadData(uint8_t port, CddDrvBL0942_Struct *pBL0942, uint8_t dataLen);
static void CddDrvBL0942_HwDetect(uint8_t port, CddDrvBL0942_Struct *pBL0942);
static void CddDrvBL0942_SoftReset(uint8_t port, CddDrvBL0942_Struct *pBL0942);
static void CddDrvBL0942_CfgRegiser(uint8_t port, CddDrvBL0942_Struct *pBL0942, const CddDrvBL0942WriteRegister_Struct *pReg);
static uint8_t CddDrvBL0942_CaliBration(uint8_t port, CddDrvBL0942_Struct *pBL0942);
static void CddDrvVL0942_RefreshData(uint8_t port, CddDrvBL0942_Struct *pBL0942);
static void CddDrvBL0942_ReadAll(uint8_t port, CddDrvBL0942_Struct *pBL0942);
static void CddDrvBL0942_WorkStateManage(uint8_t port, CddDrvBL0942_Struct *pBL0942);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddDrvBL0942_SendData(uint8_t port, CddDrvBL0942_Struct *pBL0942, uint8_t cmd, uint8_t regAddr, uint8_t *pData, uint8_t dataLen)
{
    uint8_t checkSum = 0;
    uint8_t index = 0;
    uint8_t *pSendPtr = pBL0942->cacheBuf;
    uint8_t temp = 0;

    pSendPtr[index++] = cmd;
    pSendPtr[index++] = regAddr;

    if (dataLen > 0)
    {
        memcpy(&pSendPtr[index], pData, dataLen);
        index += dataLen;

        for (temp = 0; temp < index; temp++)
        {
            checkSum += pSendPtr[temp];
        }

        pSendPtr[index++] = (~checkSum);
    }

    CddDrvBL0942Cfg_WriteData(port, pSendPtr, index);
}

static uint8_t CddDrvBL0942_ReadData(uint8_t port, CddDrvBL0942_Struct *pBL0942, uint8_t dataLen) 
{
    uint8_t checkSum = 0;
    uint8_t calcCheckSum = 0;
    uint8_t index = 0;
    uint8_t *ptr = pBL0942->cacheBuf;
    uint8_t temp = 0;
    GlobalRet_Enum eResult = eGlobalRet_OK;
    uint8_t ret = FALSE;

    if (pBL0942->cacheBuf[1] == CDDDRV_BL0942_REG_ALL)
    {
        index += 1;
    }
    else
    {
        index += 2;
    }

    eResult = CddDrvBL0942Cfg_ReadData(port, ptr + index, dataLen);

    if (eResult == eGlobalRet_OK)
    {
        index += dataLen;

        for (temp = 0; temp < index; temp++)
        {
            calcCheckSum += ptr[temp];
        }

        calcCheckSum = (~calcCheckSum);

         eResult = CddDrvBL0942Cfg_ReadData(port, &checkSum, 1);

        if (eResult == eGlobalRet_OK && calcCheckSum == checkSum)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static void CddDrvBL0942_HwDetect(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    if (pBL0942->optResult != GLOBAL_OPT_STATE_PROCESS)
    {
        CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_READ, CDDDRV_BL0942_REG_MODE, NULL, 0);
        pBL0942->optResult = GLOBAL_OPT_STATE_PROCESS;
        pBL0942->tickStart = Common_GetSystick();
    }
    else 
    {
        if (Common_JudgeTimeoutMs(pBL0942->tickStart, CDDDRV_BL0942_CFG_READ_TIMEOUT))
        {
            pBL0942->optResult = GLOBAL_OPT_STATE_FAIL;

            if (TRUE == CddDrvBL0942_ReadData(port, pBL0942, 3))
            {
                if (Common_FourUint8ToUint32(pBL0942->cacheBuf + 2) != 0)
                {
                    pBL0942->optResult = GLOBAL_OPT_STATE_SUCCESS;
                }
            }
        }
    }
}

static void CddDrvBL0942_SoftReset(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    uint8_t data[3] = {0x5A, 0x5A, 0x5A};

    if (pBL0942->optResult != GLOBAL_OPT_STATE_PROCESS)
    {
        CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_WRITE, CDDDRV_BL0942_REG_SOFT_RESET, data, sizeof(data));
    }
}

static void CddDrvBL0942_CfgRegiser(uint8_t port, CddDrvBL0942_Struct *pBL0942, const CddDrvBL0942WriteRegister_Struct *pReg)
{
    uint8_t wrProt1EnData[3] = {0x55, 0x00, 0x00};
    uint8_t wrProt2EnData[3] = {0x42, 0x00, 0x00};

    switch (pBL0942->cfgMeterStep)
    {
        case CDDDRV_BL0942_CALI_STEP0:   /* 打开写保护1 */
        {
            CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_WRITE, CDDDRV_BL0942_REG_USR_WRPROT, wrProt1EnData, sizeof(wrProt1EnData));
            pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP1;
            pBL0942->optResult = GLOBAL_OPT_STATE_PROCESS;
            break;
        }
        case CDDDRV_BL0942_CALI_STEP1: /* 打开写保护2 */
        {
            CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_WRITE, CDDDRV_BL0942_REG_WRPROT2, wrProt2EnData, sizeof(wrProt2EnData));
            pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP2;
            break;
        }
        case CDDDRV_BL0942_CALI_STEP2:  /* 设置寄存器 */
        {
            CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_WRITE, pReg->regAddr, pReg->pData, pReg->dataLen);
            pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP3;
            pBL0942->optResult = GLOBAL_OPT_STATE_PROCESS;
            pBL0942->tickStart = Common_GetSystick();
            break;
        }
        case CDDDRV_BL0942_CALI_STEP3: /* 回读寄存器 */
        {
            if (Common_JudgeTimeoutMs(pBL0942->tickStart, CDDDRV_BL0942_CFG_WRITE_TIMEOUT))
            {
                CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_READ, pReg->regAddr, NULL, 0);
                pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP4;
                pBL0942->tickStart = Common_GetSystick();
            }

            break;
        }
        case CDDDRV_BL0942_CALI_STEP4: /* 比较确认 */
        {
            if (Common_JudgeTimeoutMs(pBL0942->tickStart, CDDDRV_BL0942_CFG_READ_TIMEOUT))
            {
                if (TRUE == CddDrvBL0942_ReadData(port, pBL0942, pReg->dataLen))
                {
                    if (0 == memcmp(pReg->pData, pBL0942->cacheBuf + 2, pReg->dataLen))
                    {
                        pBL0942->optResult = GLOBAL_OPT_STATE_SUCCESS;
                    }
                    else
                    {
                        pBL0942->optResult = GLOBAL_OPT_STATE_FAIL;
                    }
                }
                else
                {
                    pBL0942->optResult = GLOBAL_OPT_STATE_FAIL;
                }
            }

            break;
        }
        default:
        {
            pBL0942->optResult = GLOBAL_OPT_STATE_FAIL;
            break;
        }
    }
}
static uint8_t CddDrvBL0942_CaliBration(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    const CddDrvBL0942WriteRegister_Struct *pReg = &c_stCddDrvBL0942WriteRegisterTable[pBL0942->cfgMeterOject];
    uint8_t ret = GLOBAL_OPT_STATE_PROCESS;

   CddDrvBL0942_CfgRegiser(port, pBL0942, pReg);

    if (pBL0942->optResult == GLOBAL_OPT_STATE_FAIL)
    {
        ret = GLOBAL_OPT_STATE_FAIL;
    }
    else if (pBL0942->optResult == GLOBAL_OPT_STATE_SUCCESS)
    {
        pBL0942->cfgMeterOject++;
        pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP2;

        if (pBL0942->cfgMeterOject == eCddDrvBL0942CaliReg_Cnt)
        {
            ret = GLOBAL_OPT_STATE_SUCCESS;
        }
    }
    else
    {}

    return ret;
}

static uint8_t CddDrvVL0942_CaliVoltageByFCT(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    uint8_t index = 5;
    uint8_t data[4] = { 0 };
    uint64_t tempReg = 0;
    uint8_t ret = FALSE;
    uint64_t tempVoltage = 0;

    /* 拷贝 V_RMS --- 电压有效值 */
    memcpy(data, pBL0942->cacheBuf + index, 3);

    tempReg = (uint64_t)Common_FourUint8ToUint32(data) * 100;
    tempVoltage = (uint32_t)(tempReg / CDDDRV_BL0942_CFG_VOLTAGE_K);

    if (tempVoltage >= 21500 && tempVoltage <= 22500)
    {
        pBL0942->caliParam.VoltageCaliK = tempReg / 22000;
        ret = TRUE;
        CDD_BL0942_CFG_WriteBlockCaliParam(port, (uint8_t *)&pBL0942->caliParam, sizeof(MSNvmMeterCaliParam_Struct));
    }

    return ret;
}

static void CddDrvVL0942_RefreshData(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    uint8_t index = 2;
    uint8_t data[4] = { 0 };
    uint64_t temp= 0;

    /* 拷贝 I_RMS --- 电流有效值 */
    memcpy(data, pBL0942->cacheBuf + index, 3);
    index += 3;

    temp = (uint64_t)Common_FourUint8ToUint32(data) * 1000 / CDDDRV_BL0942_CFG_CURRENT_K;
    pBL0942->rms_current = (uint32_t)temp;

    /* 拷贝 V_RMS --- 电压有效值 */
    memcpy(data, pBL0942->cacheBuf + index, 3);

    if (pBL0942->caliParam.VoltageCaliK != 0)
    {
        temp = (uint64_t)Common_FourUint8ToUint32(data) * 100 / pBL0942->caliParam.VoltageCaliK;
    }
    else
    {
        temp = (uint64_t)Common_FourUint8ToUint32(data) * 100 / CDDDRV_BL0942_CFG_VOLTAGE_K;
    }

    pBL0942->rms_voltage = (uint32_t)temp;
    index += 3; 

    /* I_FAST_RMS */
    index += 3;

    /* 拷贝 WATT --- 有功功率  bit23为符号位，当该位为1表示该*/
    memcpy(data, pBL0942->cacheBuf + index, 3);

    if (data[2] > 0x7F)
    {  
        data[0] = ~data[0] + 1;
        data[1] = ~data[1];
        data[2] = ~data[2]; 
    }

    pBL0942->watt = Common_FourUint8ToUint32(data) / CDDDRV_BL0942_CFG_POWER_K;
    index += 3;

    /* 拷贝 CF_CNT */
    memcpy(data, pBL0942->cacheBuf + index, 3); 
    pBL0942->tempCount = Common_FourUint8ToUint32(data);

    /* for test
        static uint32_t temptick = 0;
        if (TRUE == Common_JudgeTimeoutMs(temptick, 2000))
        {
            temptick = Common_GetSystick();
            pBL0942->tempCount = 7; 
        }
    */
    pBL0942->totalCount += pBL0942->tempCount;
}

static void CddDrvBL0942_ReadAll(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    if (pBL0942->optResult != GLOBAL_OPT_STATE_PROCESS)
    {
        CddDrvBL0942_SendData(port, pBL0942, CDDDRV_BL0942_CFG_HEAD_READ, CDDDRV_BL0942_REG_ALL, NULL, 0);
        pBL0942->optResult = GLOBAL_OPT_STATE_PROCESS;
    }
    else
    {
        if (TRUE == CddDrvBL0942_ReadData(port, pBL0942, 22))
        {
            pBL0942->optResult = GLOBAL_OPT_STATE_SUCCESS;
        }
        else
        {
            pBL0942->optResult = GLOBAL_OPT_STATE_FAIL;
        }
    }
}

static uint8_t CddDrvBL0942_CheckEnterFCTCondition(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    if (TRUE == CDDDRV_BL0942_CFG_CheckFCTPin())
    {
        pBL0942->fctPinStatusFilter.status = TRUE;
    }
    else
    {
        pBL0942->fctPinStatusFilter.status = FALSE;
    }

    Filter_Profile1(&pBL0942->fctPinStatusFilter, CDDDRV_BL0942_CFG_FCT_FILTER_POINT);
    return pBL0942->fctPinStatusFilter.validStatus;
}

static void CddDrvBL0942_WorkStateManage(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    uint8_t ret = GLOBAL_OPT_STATE_IDLE;

    switch (pBL0942->eWorkState)
    {
    case eCddDrvBL0942WorkState_Init:
    {
        CddDrvBL0942_HwDetect(port, pBL0942);

        if (GLOBAL_OPT_STATE_SUCCESS == pBL0942->optResult)
        {
            pBL0942->cfgMeterStep = CDDDRV_BL0942_CALI_STEP0;
            pBL0942->cfgMeterOject = eCddDrvBL0942CaliReg_V_MODE;
            pBL0942->optResult = GLOBAL_OPT_STATE_IDLE;
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Cali;
        }
        else if (GLOBAL_OPT_STATE_FAIL == pBL0942->optResult)
        { 
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Error;
        }
        else
        {}

        break;
    }

    case eCddDrvBL0942WorkState_Cali:
    {
        ret = CddDrvBL0942_CaliBration(port, pBL0942);

        if (GLOBAL_OPT_STATE_SUCCESS == ret)
        {
            pBL0942->comErrorFlag = FALSE;
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Normal;
            CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942配置寄存器成功!\r\n", port);
        }
        else if (GLOBAL_OPT_STATE_FAIL == ret)
        {
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Error;
            CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942配置寄存器失败!\r\n", port);
        }
        else
        {}

        break;
    }

    case eCddDrvBL0942WorkState_Normal:
    {
        CddDrvBL0942_ReadAll(port, pBL0942);

        if (GLOBAL_OPT_STATE_SUCCESS == pBL0942->optResult)
        {
            CddDrvVL0942_RefreshData(port, pBL0942);

            if (TRUE == CddDrvBL0942_CheckEnterFCTCondition(port, pBL0942))
            {
                if (pBL0942->fctCaliFlag == FALSE)
                {
                    pBL0942->eWorkState = eCddDrvBL0942WorkState_FCT;
                    CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942进入到产线标定模式!\r\n", port);
                }
            }
            else
            {
                pBL0942->fctCaliFlag = FALSE;
            }
        }
        else if (GLOBAL_OPT_STATE_FAIL == pBL0942->optResult)
        { 
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Error;
        }
        else
        {}

        break;
    }

    case eCddDrvBL0942WorkState_FCT:
    {
        CddDrvBL0942_ReadAll(port, pBL0942);

        if (GLOBAL_OPT_STATE_SUCCESS == pBL0942->optResult)
        {
            if (FALSE == CddDrvBL0942_CheckEnterFCTCondition(port, pBL0942))
            {
                pBL0942->fctCaliFlag = FALSE;
                pBL0942->eWorkState = eCddDrvBL0942WorkState_Normal;
                CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942退出产线标定模式!\r\n", port);
            }
            else
            {
                if (TRUE == CddDrvVL0942_CaliVoltageByFCT(port, pBL0942))
                {
                    pBL0942->fctCaliFlag = TRUE;
                    pBL0942->eWorkState = eCddDrvBL0942WorkState_Normal;
                    CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942电压校正完成，退出产线标定模式!\r\n", port);
                }
            }
        }
        else if (GLOBAL_OPT_STATE_FAIL == pBL0942->optResult)
        { 
            pBL0942->fctCaliFlag = FALSE;
            pBL0942->eWorkState = eCddDrvBL0942WorkState_Error;
        }
        else
        {}

        break;
    }

    case eCddDrvBL0942WorkState_Error:
    {
        pBL0942->failTryCount++;
        CDDDRV_BL0942_CFG_InfoPrint("[枪：%d]计量芯片0942失败 %d 次!\r\n", port, pBL0942->failTryCount);
        if (pBL0942->failTryCount == CDDDRV_BL0942_CFG_MAX_TIMES)
        {
            pBL0942->comErrorFlag = TRUE;
            pBL0942->failTryCount = 0;
            AswErrhandle_SetErrExsitCallback(port, eErr_MeterCommErr);
        }

        CddDrvBL0942Cfg_ResetRecvBuf(port);
        CddDrvBL0942_SoftReset(port, pBL0942);
        pBL0942->eWorkState = eCddDrvBL0942WorkState_Init;
        break;
    }

    default:
    {
        pBL0942->eWorkState = eCddDrvBL0942WorkState_Init;
        break;
    }
    }

    if (eCddCPVolState_12V == CddCP_GetVolState(port))
    {
        if (pBL0942->comErrorFlag == FALSE)
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_MeterCommErr);
        }
    }
}

static void CddDrvBL0942_CalcEnergy(uint8_t port, CddDrvBL0942_Struct *pBL0942)
{
    if (pBL0942->eWorkState == eCddDrvBL0942WorkState_Normal)
    {
        if (Common_JudgeTimeoutMs(pBL0942->period_tick, CDDDRV_BL0942_CFG_CALC_ENERGY_PERIOD))
        {
            pBL0942->energy = (pBL0942->totalCount * 10000 / CDDDRV_BL0942_CFG_PULSE_CONSTANT);
            pBL0942->period_tick = Common_GetSystick();
        }
    }
}

void CddDrvBL0942_InitMemory(void)
{
    CddDrvBL0942_Struct *pBL0942 = NULL;
    uint8_t port = 0;
    GlobalRet_Enum readResult = eGlobalRet_UnexpectedError;

    memset(&g_stCddDrvBL0942, 0x00, sizeof(g_stCddDrvBL0942));

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pBL0942 = &g_stCddDrvBL0942[port];

        CDD_BL0942_CFG_ReadBlockCaliParam(port, (uint8_t *)&pBL0942->caliParam, sizeof(MSNvmMeterCaliParam_Struct), readResult);

        if (readResult != eGlobalRet_OK || pBL0942->caliParam.VoltageCaliK == 0)
        {
            pBL0942->caliParam.VoltageCaliK = CDDDRV_BL0942_CFG_VOLTAGE_K;
            CDD_BL0942_CFG_WriteBlockCaliParam(port, (uint8_t *)&pBL0942->caliParam, sizeof(MSNvmMeterCaliParam_Struct));
        }
    }
}

void CddDrvBL0942_MainFunction(void)
{
    uint8_t port = 0;

    for (port = 0; port < ARRAY_SIZE(g_stCddDrvBL0942); port++)
    {
        CddDrvBL0942_WorkStateManage(port, &g_stCddDrvBL0942[port]);

        CddDrvBL0942_CalcEnergy(port, &g_stCddDrvBL0942[port]);
    }
}

uint32_t CddDrvBL0942_GetRmsCurrent(uint8_t port)
{
    uint32_t rms_current = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        rms_current = g_stCddDrvBL0942[port].rms_current;
    }

    return rms_current;
}

uint32_t CddDrvBL0942_GetRmsVoltage(uint8_t port)
{
    uint32_t rms_voltage = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        rms_voltage = g_stCddDrvBL0942[port].rms_voltage;
    }

    return rms_voltage;
}

uint32_t CddDrvBL0942_GetPower(uint8_t port)
{
    uint32_t power = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        power = g_stCddDrvBL0942[port].watt;
    }

    return power;
}

uint32_t CddDrvBL0942_GetPeriodEnergy(uint8_t port)
{
    uint32_t energy = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        energy = g_stCddDrvBL0942[port].energy;
    }

    return energy;
}

uint8_t CddDrvBL0942_GetReadyFlag(uint8_t port)
{
    uint8_t readyFlag = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (g_stCddDrvBL0942[port].eWorkState == eCddDrvBL0942WorkState_Normal)
        {
            readyFlag = TRUE;
        }
    }

    return readyFlag;
}
















