/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
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
#include "Asw_Monitor.h"
#include "SysCfg.h"
#include "Asw_MonitorConfig.h"
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "Asw_ChargeIf.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t chargeStart; 
    uint8_t chargeState;
    AswMonitorBillMode_Struct stBillMode;
    MSNvmOrderInfo_Union stOrderData;
    AswMonitorChargeData_Struct stChargeData;
}AswMonitorData_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswMonitorData_Struct g_stAswMonitorData[SYSCFG_CFG_GUN_NUM] = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void AswMonitor_PackChargeRecord(uint8_t port, AswMonitorData_Struct *pstAswMonitorData, uint8_t orderSaveState)
{
    // 由平台提供打包函数
}

static uint8_t AswMonitor_GetBillModePeriod(AswMonitorBillMode_Struct *pBillMode)
{
    CommonDateTime_Struct dateTime;
    uint16_t curTime = 0;
    uint16_t startTime = 0;
    uint16_t endTime = 0;
    uint8_t periodIndex = 0;

    SSTM_GetDateTime(&dateTime);

    curTime = dateTime.hour * 60 + dateTime.minute;

    for (periodIndex = 0; periodIndex < pBillMode->periodCount; periodIndex++)
    {
        startTime = pBillMode->startTime[periodIndex][0] * 60 + pBillMode->startTime[periodIndex][1];
        endTime = pBillMode->stopTime[periodIndex][0] * 60 + pBillMode->stopTime[periodIndex][1];

        if (curTime >= startTime && curTime < endTime)
        {
            break;
        }
    }

    return periodIndex;
}

static void AswMonitor_ChargeInitHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    GlobalRet_Enum readResult = eGlobalRet_UnexpectedError;

    if (port == 0)
    {
        readResult = MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0OrderInfo, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Union));
    }

    if (eGlobalRet_OK == readResult)
    {
        if (TRUE == AswPlatM_FillChargeRecord(port, &pstAswMonitorData->stOrderData))
        {
            MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0OrderInfo, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Union));
            MSNvm_InsertNewRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Union));
        }
    }
}

static void AswMonitor_ProcessCostData(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    AswMonitorBillMode_Struct *pBillMode = &pstAswMonitorData->stBillMode;
    AswMonitorChargeData_Struct *pChargeData = &pstAswMonitorData->stChargeData;
    uint8_t periodNum = AswMonitor_GetBillModePeriod(pBillMode);
    uint8_t rateNum = pBillMode->periodRate[periodNum];
    uint32_t incEnergy = 0;
    uint64_t currMeterEnergyVal = 0;
    uint64_t temp1 = 0, temp2 = 0;
    uint8_t tempPeriod = 0, tempRateNum = 0;
    
    currMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);

    if (currMeterEnergyVal > pChargeData->lastMeterEnergyVal)
    {
        incEnergy = currMeterEnergyVal - pChargeData->lastMeterEnergyVal;
        pChargeData->lastMeterEnergyVal = currMeterEnergyVal;
    }

    if (incEnergy != 0)
    {
        pChargeData->totalEnergy += incEnergy;
        pChargeData->rateTotalEnergy[rateNum] += incEnergy;

        /* 计损电量todo 最后处理 */

        /* 计算时段计费信息 */
        pChargeData->periodElePower[periodNum] += incEnergy;

        temp1 = pChargeData->periodElePower[periodNum];

        temp2 = temp1 * pBillMode->rateElecPrice[rateNum];
        pChargeData->periodEleMoney[periodNum] = temp2 / 100000;

        temp2 = temp1 * pBillMode->rateSeverPrice[rateNum];
        pChargeData->periodSerMoney[periodNum] = temp2 / 100000;

        pChargeData->periodTotalMoney[periodNum] = pChargeData->periodEleMoney[periodNum] + pChargeData->periodSerMoney[periodNum];


        vTaskSuspendAll();

        memset(pChargeData->rateEleMoney, 0x00, sizeof(pChargeData->rateEleMoney));
        memset(pChargeData->rateSerMoney, 0x00, sizeof(pChargeData->rateSerMoney));
        memset(pChargeData->rateTotalMoney, 0x00, sizeof(pChargeData->rateTotalMoney));

        for (tempPeriod = 0; tempPeriod < pBillMode->periodCount; tempPeriod++)
        {
            tempRateNum = pBillMode->periodRate[tempPeriod];

            pChargeData->rateEleMoney[tempRateNum] += pChargeData->periodEleMoney[tempPeriod];
            pChargeData->rateSerMoney[tempRateNum] += pChargeData->periodSerMoney[tempPeriod];
            pChargeData->rateTotalMoney[tempRateNum] += pChargeData->periodTotalMoney[tempPeriod];
        }

        pChargeData->totalElecMoney = 0;
        pChargeData->totalServeMoney = 0;

        for (tempRateNum = 0; tempRateNum < pBillMode->rateCount; tempRateNum++)
        {
            pChargeData->totalElecMoney += pChargeData->rateEleMoney[tempRateNum];
            pChargeData->totalServeMoney += pChargeData->rateSerMoney[tempRateNum];
        }

        pChargeData->totalMoney = pChargeData->totalElecMoney + pChargeData->totalServeMoney;
        xTaskResumeAll();
    }
}


static void AswMonitor_ChargeIdleHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    if (pstAswMonitorData->chargeStart)
    {
        memset(&pstAswMonitorData->stChargeData, 0, sizeof(pstAswMonitorData->stChargeData));
        memset(&pstAswMonitorData->stOrderData, 0, sizeof(pstAswMonitorData->stOrderData));
        pstAswMonitorData->chargeState = ASWMONITOR_CHARGE_STATE_ONGOING;

        pstAswMonitorData->stChargeData.lastMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);
    }
}

static void AswMonitor_ChargeOngoingHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    AswMonitor_ProcessCostData(port, pstAswMonitorData);



}

static void AswMonitor_ChargeHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    switch (pstAswMonitorData->chargeState)
    {
        case ASWMONITOR_CHARGE_STATE_INIT:
        {
            AswMonitor_ChargeInitHandle(port, pstAswMonitorData);
            break;
        }

        case ASWMONITOR_CHARGE_STATE_IDLE:
        {
            AswMonitor_ChargeIdleHandle(port, pstAswMonitorData);
            break;
        }

        case ASWMONITOR_CHARGE_STATE_ONGOING:
        {
            AswMonitor_ChargeOngoingHandle(port, pstAswMonitorData);
            break;
        }

        default:
        {
            break;
        }
    }
}














void AswMonitor_InitMemory(void)
{


}

void AswMonitor_MainFunction(void)
{
    uint8_t port = 0;
    
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswMonitor_ChargeHandle(port, &g_stAswMonitorData[port]);
    }
}

















