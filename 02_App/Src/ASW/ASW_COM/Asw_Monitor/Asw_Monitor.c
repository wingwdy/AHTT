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
#include "Mcal_Mcu.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eAswMonitorRebootStep_Null,
    eAswMonitorRebootStep_WaitIdle,
    eAswMonitorRebootStep_LastDelay,
    eAswMonitorRebootStep_Finish,    
}AswMonitorRebootStep_Enum;



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t forbidState;                            /* 禁用状态 */
    uint8_t chargeStart;                            /* 启动充电控制 */
    uint8_t orderCtrl;                              /* 订单控制 */
    uint32_t orderDataSaveTick;                     /* 订单周期存储计时 */
    AswMonitorChargeCtrl_Struct stChargeCtrl;       /* 充电控制变量 */
    AswMonitorBillMode_Struct stBillMode;           /* 充电计费模型 */
    MSNvmOrderInfo_Struct stOrderData;              /* 订单数据 */
    AswMonitorChargeData_Struct stChargeData;       /* 充电变量 */
}AswMonitorData_Struct;

typedef struct 
{
    uint32_t rebootDelayTick;                        /* 复位延时计时 */
    AswMonitorRebootType_Enum eAswMonitorRebootType; /* 复位类型 */
    AswMonitorRebootStep_Enum eAswMonitorRebootStep; /* 复位控制步骤 */
}AswMonitorCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswMonitorData_Struct g_stAswMonitorData[SYSCFG_CFG_GUN_NUM] = { 0 };
static AswMonitorCtx_Struct  g_stAswMonitorCtx = { 0 };

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t AswMonitor_GetBillModePeriod(AswMonitorBillMode_Struct *pBillMode);
static void AswMonitor_ProcessCostData(uint8_t port, AswMonitorData_Struct *pstAswMonitorData);
static uint8_t AswMonitor_DetectAccountMoney(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData);
static uint8_t AswMonitor_DetectChargeCtrlMoney(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData);
static uint8_t AswMonitor_DetectChargeCtrlTime(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData);
static uint8_t AswMonitor_DetectChargeCtrlEnergy(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData);
static void AswMonitor_ChargeValDetect(uint8_t port, AswMonitorData_Struct *pstAswMonitorData);
static void AswMonitor_SaveChargeRecord(uint8_t port, AswMonitorData_Struct *pstAswMonitorData, uint8_t orderSaveReason);
static void AswMonitor_OrderOngoingHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData);
static void AswMonitor_OrderEndHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData);
static void AswMonitor_OrderManage(uint8_t port, AswMonitorData_Struct *pstAswMonitorData);
static void AswMonitor_RebootManage(void);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
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

static void AswMonitor_ProcessCostData(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    uint32_t curTime = SSTM_GetSecTimestamp();
    AswMonitorBillMode_Struct *pBillMode = &pstAswMonitorData->stBillMode;
    AswMonitorChargeData_Struct *pChargeData = &pstAswMonitorData->stChargeData;
    uint8_t periodNum = 0;
    uint8_t rateNum = 0;
    uint32_t incEnergy = 0;
    uint64_t currMeterEnergyVal = 0;
    uint64_t temp1 = 0, temp2 = 0;
    uint8_t tempPeriod = 0, tempRateNum = 0;
    
    currMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);

    if (currMeterEnergyVal > pChargeData->lastMeterEnergyVal)
    {
        incEnergy = currMeterEnergyVal - pChargeData->lastMeterEnergyVal;
        pChargeData->lastMeterEnergyVal = currMeterEnergyVal;
        pChargeData->stopMeterVal = currMeterEnergyVal;
    }

    if (curTime > pChargeData->chargeStartTime)
    {
        pChargeData->chargeTime = curTime - pChargeData->chargeStartTime;
        pChargeData->chargeStopTime = curTime;
    }

    if (incEnergy != 0)
    {
        rateNum = pBillMode->periodRate[periodNum];
        periodNum = AswMonitor_GetBillModePeriod(pBillMode);

        pChargeData->totalEnergy += incEnergy;
        pChargeData->totalLossEnergy = ((uint64_t)pChargeData->totalEnergy * pBillMode->elecLossRate / 100) + pChargeData->totalEnergy;
        pChargeData->rateTotalEnergy[rateNum] += incEnergy;
        pChargeData->rateTotalLossEnergy[rateNum] = ((uint64_t)pChargeData->rateTotalEnergy[rateNum] * pBillMode->elecLossRate / 100) + 
            pChargeData->rateTotalEnergy[rateNum];

        /* 计算时段计费信息 */
        pChargeData->periodElePower[periodNum] += incEnergy;

        temp1 = ((uint64_t)pChargeData->periodElePower[periodNum] * pBillMode->elecLossRate / 100) + pChargeData->periodElePower[periodNum];

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

static uint8_t AswMonitor_DetectAccountMoney(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint32_t totalMoney = pChargeData->totalMoney / 100;
    uint8_t ret = FALSE;
    uint32_t diff = 0;

    if (pstChargeCtrl->accountMoney < totalMoney)
    {
        ret = TRUE;
    }
    else
    {
        diff = pstChargeCtrl->accountMoney - totalMoney;

        if (diff < ASWMONITOR_CFG_CHARGE_MIN_ACCOUNT_MONEY)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t AswMonitor_DetectChargeCtrlMoney(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint32_t totalMoney = pChargeData->totalMoney / 100;
    uint8_t ret = FALSE;

    if (totalMoney >= pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }

    return ret;
}
static uint8_t AswMonitor_DetectChargeCtrlTime(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint8_t ret = FALSE;

    if (pChargeData->chargeTime > pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }

    return ret;
}
static uint8_t AswMonitor_DetectChargeCtrlEnergy(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint32_t totalLossEnergy = pChargeData->totalLossEnergy / 100;
    uint8_t ret = FALSE;
    
    if (totalLossEnergy >= pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }

    return ret;
}

static void AswMonitor_ChargeValDetect(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{ 
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = &pstAswMonitorData->stChargeCtrl;
    AswMonitorChargeData_Struct *pChargeData = &pstAswMonitorData->stChargeData;

    /* 余额检测 */
    if (TRUE == AswMonitor_DetectAccountMoney(port, pstChargeCtrl, pChargeData))
    {
        AswErrhandle_SetErrExsitCallback(port, eSrc_InsuffBalance);
    }
    else
    {
        /* 基于充电方式进行检测 */
        if (pstChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeTime)
        {
            if (TRUE == AswMonitor_DetectChargeCtrlTime(port, pstChargeCtrl, pChargeData))
            {
                AswErrhandle_SetErrExsitCallback(port, eSrc_StopbyTime);
            }
        }
        else if (pstChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeEnergy)
        {
            if (TRUE == AswMonitor_DetectChargeCtrlEnergy(port, pstChargeCtrl, pChargeData))
            {
                AswErrhandle_SetErrExsitCallback(port, eSrc_StopbyEnergy);
            }
        }
        else if (pstChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeMoney)
        {
            if (TRUE == AswMonitor_DetectChargeCtrlMoney(port, pstChargeCtrl, pChargeData))
            {
                AswErrhandle_SetErrExsitCallback(port, eSrc_StopbyMoney);
            }
        }
        else
        {}
    }
}

static void AswMonitor_SaveChargeRecord(uint8_t port, AswMonitorData_Struct *pstAswMonitorData, uint8_t orderSaveReason)
{
    AswPlatM_PackChargeRecord(port, &pstAswMonitorData->stOrderData, orderSaveReason);
    ASWMONITOR_CFG_WriteBlockOrderInfo(port, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        MSNvm_InsertNewRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
    }
}

static void AswMonitor_OrderOngoingHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);

    AswMonitor_ProcessCostData(port, pstAswMonitorData);

    /* 订单结束 */
    if (chargeState == ASWCHARGEIF_WORKSTATE_FINISH ||
        chargeState == ASWCHARGEIF_WORKSTATE_IDLE)
    {
        pstAswMonitorData->chargeStart = FALSE;
        pstAswMonitorData->stOrderData.orderSaveState = ASWMONITOR_ORDER_SAVE_STOP;
        pstAswMonitorData->stChargeData.eChargeStopReason = AswChargeIf_GetStopReason(port);
        AswMonitor_SaveChargeRecord(port, pstAswMonitorData, ASWMONITOR_ORDER_SAVE_STOP);
        pstAswMonitorData->orderCtrl = ASWMONITOR_ORDER_CTRL_END;
    }
    else
    {
        AswMonitor_ChargeValDetect(port, pstAswMonitorData);
        /* 订单周期存储 */
        if (Common_JudgeTimeoutMs(pstAswMonitorData->orderDataSaveTick, ASWMONITOR_CFG_SAVE_CHARGE_RECORD_PERIOD))
        {
            AswMonitor_SaveChargeRecord(port, pstAswMonitorData, ASWMONITOR_ORDER_SAVE_PERIOD);
            pstAswMonitorData->orderDataSaveTick = Common_GetSystick();
        }
    }
}

static void AswMonitor_OrderEndHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    pstAswMonitorData->orderCtrl = ASWMONITOR_ORDER_CTRL_IDLE;
}

static void AswMonitor_OrderManage(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    switch (pstAswMonitorData->orderCtrl)
    {
        case ASWMONITOR_ORDER_CTRL_IDLE:
        {
            break;
        }

        case ASWMONITOR_ORDER_CTRL_ONGOING:
        {
            AswMonitor_OrderOngoingHandle(port, pstAswMonitorData);
            break;
        }

        case ASWMONITOR_ORDER_CTRL_END:
        {
            AswMonitor_OrderEndHandle(port, pstAswMonitorData);
            break;
        }

        default:
        {
            break;
        }
    }
}

static void AswMonitor_RebootManage(void)
{
    uint8_t port = 0;
    uint8_t orderIdle = TRUE;

    if (g_stAswMonitorCtx.eAswMonitorRebootType != eAswMonitorRebootType_Null)
    {
        switch (g_stAswMonitorCtx.eAswMonitorRebootStep)
        {
            case eAswMonitorRebootStep_Null:
            {
                break;
            }
            case eAswMonitorRebootStep_WaitIdle:
            {
                for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
                {
                    if (AswMonitor_IsOrderIdle(port) != TRUE)
                    {
                        orderIdle = FALSE;
                        break;
                    }
                }

                if (orderIdle == TRUE)
                {
                    g_stAswMonitorCtx.eAswMonitorRebootStep = eAswMonitorRebootStep_LastDelay;
                    g_stAswMonitorCtx.rebootDelayTick = Common_GetSystick();
                }

                break;
            }
            case eAswMonitorRebootStep_LastDelay:
            {
                if (Common_JudgeTimeoutMs(g_stAswMonitorCtx.rebootDelayTick, ASWMONITOR_CFG_REBOOT_DELAY_TIME))
                {
                    g_stAswMonitorCtx.eAswMonitorRebootStep = eAswMonitorRebootStep_Finish;
                }

                break;
            }
            case eAswMonitorRebootStep_Finish:
            {
                McalMcu_SystemReset();
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

uint8_t AswMonitor_CheckBillModeValid(uint8_t port)
{
    AswMonitorData_Struct *pstAswMonitorData = NULL;
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstAswMonitorData = &g_stAswMonitorData[port];
        AswPlatM_TransformBillMode(port, &pstAswMonitorData->stBillMode);

        if (pstAswMonitorData->stBillMode.validFlag == TRUE)
        {
            ret = TRUE;
        }
    }

    return ret;
}

uint8_t AswMonitor_CheckForbidState(uint8_t port)
{
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        ret = g_stAswMonitorData[port].forbidState;
    }

    return ret;
}

void AswMonitor_ChargeStart(uint8_t port, uint8_t startSrc)
{
    AswMonitorChargeData_Struct *pChargeData = NULL;
    AswMonitorData_Struct *pstAswMonitorData = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstAswMonitorData = &g_stAswMonitorData[port];
        pChargeData = &pstAswMonitorData->stChargeData;

        if (pstAswMonitorData->chargeStart == FALSE && pstAswMonitorData->orderCtrl == ASWMONITOR_ORDER_CTRL_IDLE)
        {
            memset(pChargeData, 0x00, sizeof(AswMonitorChargeData_Struct));
            memset(&pstAswMonitorData->stOrderData, 0x00, sizeof(pstAswMonitorData->stOrderData));

            pstAswMonitorData->chargeStart = TRUE;
            pstAswMonitorData->stChargeCtrl.startSrc = startSrc;

            pChargeData->lastMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);
            pChargeData->startMeterVal = pChargeData->lastMeterEnergyVal;
            pChargeData->stopMeterVal = pChargeData->startMeterVal;

            pChargeData->chargeStartTime = SSTM_GetSecTimestamp();
            pChargeData->chargeStopTime = pChargeData->chargeStartTime;
            pChargeData->eChargeStopReason = eErr_none;

            pstAswMonitorData->orderDataSaveTick = Common_GetSystick();
            pstAswMonitorData->orderCtrl = ASWMONITOR_ORDER_CTRL_ONGOING;

            pstAswMonitorData->stOrderData.orderSaveState = ASWMONITOR_ORDER_SAVE_START;
            AswMonitor_SaveChargeRecord(port, pstAswMonitorData, ASWMONITOR_ORDER_SAVE_START);
            AswChargeIf_ChargeStart(port);
        }
    }
}

void AswMonitor_SetReboot(AswMonitorRebootType_Enum eRebootType)
{
    uint8_t changeFlag = FALSE; 

    if (eRebootType != eAswMonitorRebootType_Null && eRebootType != g_stAswMonitorCtx.eAswMonitorRebootType)
    {
        if (g_stAswMonitorCtx.eAswMonitorRebootType == eAswMonitorRebootType_Null)
        {
            if (eRebootType == eAswMonitorRebootType_Immediate)
            {
                g_stAswMonitorCtx.eAswMonitorRebootType = eRebootType;
                g_stAswMonitorCtx.eAswMonitorRebootStep = eAswMonitorRebootStep_LastDelay;
                g_stAswMonitorCtx.rebootDelayTick = Common_GetSystick();
            }
            else if (eRebootType == eAswMonitorRebootType_WaitIdle)
            {
                g_stAswMonitorCtx.eAswMonitorRebootType = eRebootType;
                g_stAswMonitorCtx.eAswMonitorRebootStep = eAswMonitorRebootStep_WaitIdle;
                g_stAswMonitorCtx.rebootDelayTick = Common_GetSystick();
            }
            else
            {}
        }
        else if (g_stAswMonitorCtx.eAswMonitorRebootType == eAswMonitorRebootType_WaitIdle)
        { 
            if (eRebootType == eAswMonitorRebootType_Immediate)
            {
                g_stAswMonitorCtx.eAswMonitorRebootType = eRebootType;
                g_stAswMonitorCtx.eAswMonitorRebootStep = eAswMonitorRebootStep_LastDelay;
                g_stAswMonitorCtx.rebootDelayTick = Common_GetSystick();
            }
        }
        else
        {}
    }
}

AswMonitorChargeCtrl_Struct *AswMonitor_GetChargeCtrlPtr(uint8_t port)
{
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstChargeCtrl = &g_stAswMonitorData[port].stChargeCtrl;
    }

    return pstChargeCtrl;
}

AswMonitorBillMode_Struct *AswMonitor_GetCurUsedBillModePtr(uint8_t port)
{
    AswMonitorBillMode_Struct *pstBillMode = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstBillMode = &g_stAswMonitorData[port].stBillMode;
    }

    return pstBillMode;
}

AswMonitorChargeData_Struct *AswMonitor_GetChargeDataPtr(uint8_t port)
{
    AswMonitorChargeData_Struct *pstChargeData = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstChargeData = &g_stAswMonitorData[port].stChargeData;
    }

    return pstChargeData;
}

uint8_t AswMonitor_IsOrderIdle(uint8_t port)
{
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (g_stAswMonitorData[port].orderCtrl == ASWMONITOR_ORDER_CTRL_IDLE)
        {
            ret = TRUE;
        }
    }

    return ret;
}

void AswMonitor_InitMemory(void)
{
    AswMonitorData_Struct *pstAswMonitorData = NULL;
    uint8_t port = 0;
    GlobalRet_Enum readResult;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {  
        pstAswMonitorData = &g_stAswMonitorData[port];
        memset(pstAswMonitorData, 0x00, sizeof(AswMonitorData_Struct));

        ASWMONITOR_CFG_ReadBlockOrderInfo(port, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct), readResult);

        if (eGlobalRet_OK == readResult)
        {
            if (pstAswMonitorData->stOrderData.orderSaveState == ASWMONITOR_ORDER_SAVE_START ||
                pstAswMonitorData->stOrderData.orderSaveState == ASWMONITOR_ORDER_SAVE_PERIOD)
            {
                pstAswMonitorData->stOrderData.orderSaveState = ASWMONITOR_ORDER_SAVE_STOP;
                ASWMONITOR_CFG_WriteBlockOrderInfo(port, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
                MSNvm_InsertNewRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
            }
        }
    }
}

void AswMonitor_MainFunction(void)
{
    uint8_t port = 0;
    
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswMonitor_OrderManage(port, &g_stAswMonitorData[port]);
    }

    AswMonitor_RebootManage();
}

















