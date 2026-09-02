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
#include "Cdd_CardM.h"
#include "Cdd_ModeM.h"
#include "SS_Ucm.h"
#include "Cdd_NetM.h"
#include "Asw_IotProtoGNM.h"

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

    uint8_t swipCardSuccLedFlag;  /* 刷卡成功标记 */
    uint8_t swipCardFailLedFlag;  /* 刷卡失败标记 */

    MSNvmForbidState_Struct forbidParam;    /* 禁用状态 */
}AswMonitorCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswMonitorData_Struct g_stAswMonitorData[SYSCFG_CFG_GUN_NUM] = { 0 };
static AswMonitorCtx_Struct  g_stAswMonitorCtx = { 0 };
static AswMonitorCtrlPara_Struct g_stAswMonitorCtrlPara = { 0 };

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
    uint32_t oldPeriodEleMoney = 0, oldPeriodSerMoney = 0;
    uint32_t oldRateLossEnergy = 0;
    int32_t deltaEleMoney = 0, deltaServMoney = 0;
    uint32_t exactTotalElec = 0, exactTotalServ = 0;
    uint32_t lossFactor = 0;

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

    periodNum = AswMonitor_GetBillModePeriod(pBillMode);
    pChargeData->periodValidFlag[periodNum] = TRUE;
    pChargeData->currentPeriodNum = periodNum;
    
    if (incEnergy != 0)
    {
        rateNum = pBillMode->periodRate[periodNum];
        pChargeData->currentRateNum = rateNum;

        lossFactor = 100 + pBillMode->elecLossRate;
        oldRateLossEnergy = pChargeData->rateTotalLossEnergy[rateNum];

        pChargeData->totalEnergy += incEnergy;
        pChargeData->rateTotalEnergy[rateNum] += incEnergy;
        pChargeData->rateTotalLossEnergy[rateNum] = (uint64_t)pChargeData->rateTotalEnergy[rateNum] * lossFactor / 100;

        uint32_t incLossEnerge = (pChargeData->rateTotalLossEnergy[rateNum] - oldRateLossEnergy);
        pChargeData->totalLossEnergy += incLossEnerge;

        pChargeData->periodElePower[periodNum] += incEnergy;

        oldPeriodEleMoney = pChargeData->periodEleMoney[periodNum];
        oldPeriodSerMoney = pChargeData->periodSerMoney[periodNum];

        pChargeData->preciseElecTotalMoney += (uint64_t)incEnergy * pBillMode->rateElecPrice[rateNum] * lossFactor;
        pChargeData->preciseServTotalMoney += (uint64_t)incEnergy * pBillMode->rateSeverPrice[rateNum] * lossFactor;

        exactTotalElec = (uint32_t)((pChargeData->preciseElecTotalMoney + 5000000ULL) / 10000000ULL);
        exactTotalServ = (uint32_t)((pChargeData->preciseServTotalMoney + 5000000ULL) / 10000000ULL);

        pChargeData->periodEleMoney[periodNum] = exactTotalElec - (pChargeData->totalElecMoney - oldPeriodEleMoney);
        pChargeData->periodSerMoney[periodNum] = exactTotalServ - (pChargeData->totalServeMoney - oldPeriodSerMoney);
        pChargeData->periodTotalMoney[periodNum] = pChargeData->periodEleMoney[periodNum] + pChargeData->periodSerMoney[periodNum];

        deltaEleMoney  = (int32_t)(pChargeData->periodEleMoney[periodNum] - oldPeriodEleMoney);
        deltaServMoney = (int32_t)(pChargeData->periodSerMoney[periodNum] - oldPeriodSerMoney);

        pChargeData->rateEleMoney[rateNum]       += deltaEleMoney;
        pChargeData->rateSerMoney[rateNum]       += deltaServMoney;
        pChargeData->rateTotalMoney[rateNum]     += (deltaEleMoney + deltaServMoney);

        pChargeData->totalElecMoney              += deltaEleMoney;
        pChargeData->totalServeMoney             += deltaServMoney;
        pChargeData->totalMoney = pChargeData->totalElecMoney + pChargeData->totalServeMoney;
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

        if (diff < g_stAswMonitorCtrlPara.minAccountMoney)
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
    uint32_t diff = 0;

    if (totalMoney >= pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }
    else
    {
        diff = pstChargeCtrl->chargeCtrlVal - totalMoney;

        if (diff < ASWMONITOR_CFG_CHARGE_MIN_CHARGE_MONEY)
        {
            ret = TRUE;
        }
    }

    return ret;
}
static uint8_t AswMonitor_DetectChargeCtrlTime(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint8_t ret = FALSE;
    uint32_t diff = 0;

    if (pChargeData->chargeTime >= pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }
    else
    {
        diff = pstChargeCtrl->chargeCtrlVal - pChargeData->chargeTime;

        if (diff < ASWMONITOR_CFG_CHARGE_MIN_CHARGE_TIME)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t AswMonitor_DetectChargeCtrlEnergy(uint8_t port, AswMonitorChargeCtrl_Struct *pstChargeCtrl, AswMonitorChargeData_Struct *pChargeData)
{
    uint32_t totalLossEnergy = pChargeData->totalLossEnergy / 100;
    uint8_t ret = FALSE;
    uint32_t diff = 0;
    
    if (totalLossEnergy >= pstChargeCtrl->chargeCtrlVal)
    {
        ret = TRUE;
    }
    else
    {
        diff = pstChargeCtrl->chargeCtrlVal - totalLossEnergy;

        if (diff < ASWMONITOR_CFG_CHARGE_MIN_CHARGE_ENERGY)
        {
            ret = TRUE;
        }
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

static void AswMonitor_IdleHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    if (TRUE == CddModeM_IsFactoryMode())
    {
        if ((ASWCHARGEIF_WORKSTATE_READY == AswChargeIf_GetChargeState(port)) && (FALSE == SSUcm_IsUpdating()))
        {  
            if (eErrChargeCondition_Allow == AswErrHandle_GetChargeCondition(port))
            {
                AswMonitor_ChargeStart(port, ASWMONITOR_ORDER_START_SRC_PNC, TRUE);
            }
        }
    }
}

static void AswMonitor_OrderOngoingHandle(uint8_t port, AswMonitorData_Struct *pstAswMonitorData)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);

    AswMonitor_ProcessCostData(port, pstAswMonitorData);

    /* 订单结束 */
    if ((chargeState == ASWCHARGEIF_WORKSTATE_FINISH || chargeState == ASWCHARGEIF_WORKSTATE_IDLE) && 
        FALSE == AswChargeIf_GetAuthFlag(port))
    {
        pstAswMonitorData->chargeStart = FALSE;
        pstAswMonitorData->stOrderData.orderSaveState = ASWMONITOR_ORDER_SAVE_STOP;
        pstAswMonitorData->stChargeData.eChargeStopReason = AswChargeIf_GetStopReason(port);
        AswMonitor_SaveChargeRecord(port, ASWMONITOR_ORDER_SAVE_STOP);
        pstAswMonitorData->orderCtrl = ASWMONITOR_ORDER_CTRL_END;
    }
    else
    {
        AswMonitor_ChargeValDetect(port, pstAswMonitorData);
        /* 订单周期存储 */
        if (Common_JudgeTimeoutMs(pstAswMonitorData->orderDataSaveTick, ASWMONITOR_CFG_SAVE_CHARGE_RECORD_PERIOD))
        {
            AswMonitor_SaveChargeRecord(port, ASWMONITOR_ORDER_SAVE_PERIOD);
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
            AswMonitor_IdleHandle(port, pstAswMonitorData);
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
                if (Common_JudgeTimeoutMs(g_stAswMonitorCtx.rebootDelayTick, ASWMONITOR_CFG_REBOOT_DELAY_TIME) &&
                     FALSE == MSNvm_IsBusy())
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

void AswMonitor_SaveChargeRecord(uint8_t port, uint8_t orderSaveReason)
{
    AswMonitorData_Struct *pstAswMonitorData = &g_stAswMonitorData[port];

    /* 场内模式不保存订单 */
    if (FALSE == CddModeM_IsFactoryMode())
    {
        AswPlatM_PackChargeRecord(port, &pstAswMonitorData->stOrderData, orderSaveReason);
        ASWMONITOR_CFG_WriteBlockOrderInfo(port, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));

        if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
        {
            MSNvm_InsertNewRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
            MSNvm_InsertNewRecord(eMSNvmBlockID_OmOrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
        }
    }
}

void AswMonitor_PrintChargeData(void)
{
    AswMonitorChargeData_Struct *pChargeData = NULL;
    uint8_t port = 0;
    uint32_t voltage = 0;
    uint32_t current = 0;
    uint32_t power = 0;
    uint8_t gunTemp = 0;
    uint8_t envTemp = 0;
    uint32_t energy = 0;
    uint32_t chargeTime = 0;
    uint32_t money = 0;
    uint16_t cpVol = 0;
    uint16_t cpDuty = 0;
    uint64_t currMeterEnergyVal = 0;
    uint32_t temp1 = 0, temp2 = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        currMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);
        temp1 = currMeterEnergyVal / 10000;
        temp2 = currMeterEnergyVal % 10000;
        pChargeData = &g_stAswMonitorData[port].stChargeData;
        voltage = AswChargeIf_GetInputVoltage(port);
        current = AswChargeIf_GetOutputCurrent(port);
        gunTemp = AswChargeIf_GetGunTemperature(port);
        envTemp = AswChargeIf_GetEnvTemperature();
        power = AswChargeIf_GetOutputPower(port);
        energy = pChargeData->totalLossEnergy;
        money = pChargeData->totalMoney;
        chargeTime = pChargeData->chargeTime;
        cpVol = AswChargeIf_GetCpVoltage(port);
        cpDuty = AswChargeIf_GetCpDuty(port);
                                                                 
        ASWMONITOR_CFG_InfoPrint("---------------------------------[枪: %d]信息[CP:%d.%03dV, %d.%01d%%]------------------------------------\r\n", 
            port, cpVol / 1000, cpVol % 1000, cpDuty / 10, cpDuty % 10);
        ASWMONITOR_CFG_InfoPrint("电压：%d.%02d V,\t电流：%d.%03d A,\t功率：%d.%03d kW\r\n",
                                voltage / 100, voltage % 100, current / 1000, current % 1000, power / 1000, power % 1000);
        ASWMONITOR_CFG_InfoPrint("枪温：%d ℃,\t壳温：%d ℃,\t\t已充时间：%d s\r\n",
                                (gunTemp - 50), (envTemp - 50), chargeTime);                                
        ASWMONITOR_CFG_InfoPrint("已充电量：%d.%04d kWh,\t\t\t已充金额：%d.%04d 元,\r\n",
                                energy / 10000, energy % 10000, money / 10000, money % 10000);
        ASWMONITOR_CFG_InfoPrint("电表读数：%d.%04d kWh\r\n", temp1, temp2);
        ASWMONITOR_CFG_InfoPrint("----------------------------------------------------------------------------------------------\r\n");             
    }
}

static void AswMonitor_CardAuthHandle(void)
{
    /* 目前刷卡分不清到底是A枪刷卡，还是B枪刷卡，暂时所有的刷卡认为是A枪刷卡 */
    uint8_t port = 0;
    AswMonitorData_Struct *pstAswMonitorData = &g_stAswMonitorData[port];
    uint8_t bcdAuthCardID[ASWMONITOR_CARD_ID_LEN] = {0};
    uint8_t phyCardID[ASWMONITOR_CARD_ID_LEN + 1] = {0};
    uint8_t userCardID[ASWMONITOR_CARD_ID_LEN + 1] = {0};
    uint8_t randomNum[ASWMONITOR_RANDOM_LEN + 1] = {0};
    CddCardType_Enum eCardType = CddCardM_GetCardType();

    /* 读取物理卡号 UUID，用户ID，随机数 */
    CddCardM_GetCardUid(phyCardID);
    CddCardM_GetCardUserId(userCardID);
    CddCardM_GetCardInfoRandom(randomNum);

    /* 根据当前的卡类型，来给授权卡号赋值 */
    if (eCardType == eCddCardType_BullCard)
    {
        memcpy(bcdAuthCardID, userCardID, ASWMONITOR_CARD_ID_LEN);
    }
    else if (eCardType == eCddCardType_XiaojuCard)
    {
        


        memcpy(bcdAuthCardID, userCardID, ASWMONITOR_CARD_ID_LEN);
        /* 因为授权卡号、随机数，不参与刷卡启停逻辑，但是协议层要用，所以这里可以赋值 */
        memcpy(pstAswMonitorData->stChargeCtrl.randomNum, randomNum, ASWMONITOR_RANDOM_LEN);
        memcpy(pstAswMonitorData->stChargeCtrl.phyCardID, phyCardID, ASWMONITOR_CARD_ID_LEN);
    }
    else /* 默认按UUID 来处理*/
    {
        memcpy(bcdAuthCardID, phyCardID, ASWMONITOR_CARD_ID_LEN);
    }

    if (pstAswMonitorData->orderCtrl == ASWMONITOR_ORDER_CTRL_IDLE)
    {
        if (AswErrHandle_IsExsistError(port) == TRUE)
        {
            ASWMONITOR_CFG_InfoPrint("[枪：%d]刷卡成功，设备故障，拒绝充电!!\r\n", port);
        }
        else if (AswChargeIf_CheckGunConnected(port) != TRUE)
        {
            ASWMONITOR_CFG_InfoPrint("[枪：%d]刷卡成功，枪未连接，拒绝充电!!\r\n", port);
        }
        else if (AswMonitor_CheckBillModeValid(port) != TRUE)
        {
            ASWMONITOR_CFG_InfoPrint("[枪：%d]刷卡成功，计费模型无效，拒绝充电!!\r\n", port);
        }
        else if (SSUcm_IsUpdating() == TRUE)
        {
            ASWMONITOR_CFG_InfoPrint("刷卡成功，设备在升级，拒绝充电!!\r\n");
        }
        else if (TRUE == AswMonitor_CheckForbidState())
        {
            ASWMONITOR_CFG_InfoPrint("刷卡成功，设备禁用，拒绝充电!!\r\n");
        }
        else
        {
            /* 请求平台启动充电 */
            if (TRUE == AswPlatM_SwipCardCharge(port, bcdAuthCardID))
            {
                memcpy(pstAswMonitorData->stChargeCtrl.authCardID, bcdAuthCardID, ASWMONITOR_CARD_ID_LEN);
                ASWMONITOR_CFG_InfoPrint("[枪：%d]刷卡成功，请求启动充电!\r\n", port);
            }
        }
    }
    else if (pstAswMonitorData->orderCtrl == ASWMONITOR_ORDER_CTRL_ONGOING)
    {
        if (0 == memcmp(pstAswMonitorData->stChargeCtrl.authCardID, bcdAuthCardID, ASWMONITOR_CARD_ID_LEN))
        {
            AswErrhandle_SetErrExsitCallback(port, eSrc_CardStop);
        }
        else
        {
            ASWMONITOR_CFG_InfoPrint("刷卡成功，卡号不一致，拒绝停止充电!!\r\n");
        }
    }
    else
    {}
}

static void AswMonitor_SwipCardManage(void)
{
    CddCardEvent_Enum eCardEvent = CddCardM_GetCardEvent();

    if (eCardEvent == CddCardEvent_CardIdOK)
    {
        if (CddModeM_IsFactoryMode() == TRUE)
        {
            CddModeM_ExsitFactoryMode();
        }
        else
        {
            AswMonitor_CardAuthHandle();
        }

        g_stAswMonitorCtx.swipCardSuccLedFlag = TRUE;
    }
    else if (eCardEvent == CddCardEvent_CardIdError)
    {
        g_stAswMonitorCtx.swipCardFailLedFlag = TRUE;
    }
    else
    {}
}

void AswMonitor_SetMinAccountMoney(uint32_t minAccountMoney)
{
    g_stAswMonitorCtrlPara.minAccountMoney = minAccountMoney;
}

uint8_t AswMonitor_CheckSwipCardSuccEvent(void)
{
    uint8_t ret = g_stAswMonitorCtx.swipCardSuccLedFlag;

    g_stAswMonitorCtx.swipCardSuccLedFlag = FALSE;
    return ret;
}

uint8_t AswMonitor_CheckSwipCardFailEvent(void)
{
    uint8_t ret = g_stAswMonitorCtx.swipCardFailLedFlag;

    g_stAswMonitorCtx.swipCardFailLedFlag = FALSE;
    return ret;
}

uint8_t AswMonitor_CheckBillModeValid(uint8_t port)
{
    AswMonitorData_Struct *pstAswMonitorData = NULL;
    uint8_t ret = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstAswMonitorData = &g_stAswMonitorData[port];
        pstAswMonitorData->stBillMode.validFlag = FALSE;
        AswPlatM_TransformBillMode(port, &pstAswMonitorData->stBillMode);

        if (pstAswMonitorData->stBillMode.validFlag == TRUE)
        {
            ret = TRUE;
        }
    }

    return ret;
}

uint8_t AswMonitor_CheckForbidState(void)
{
    return g_stAswMonitorCtx.forbidParam.forbidState;
}

void AswMonitor_SetForbidState(uint8_t lockState, uint8_t lockReason)
{
    if (g_stAswMonitorCtx.forbidParam.forbidState != lockState ||
        g_stAswMonitorCtx.forbidParam.forbidReason != lockReason)
    {
        ASWMONITOR_CFG_InfoPrint("设备禁用状态变化：[%d]--->[%d]\r\n", g_stAswMonitorCtx.forbidParam.forbidState, lockState);
        g_stAswMonitorCtx.forbidParam.forbidState = lockState;
        g_stAswMonitorCtx.forbidParam.forbidReason = lockReason;
    }

    MSNvm_WriteParaBlock(eMSNvmBlockID_ForbidState, (uint8_t *)&g_stAswMonitorCtx.forbidParam, sizeof(MSNvmForbidState_Struct));
}

void AswMonitor_GetForbidState(uint8_t *pLockState, uint8_t *pLockReason)
{
    if (pLockState != NULL && pLockReason != NULL)
    {
        pLockState[0] = g_stAswMonitorCtx.forbidParam.forbidState;
        pLockReason[0] = g_stAswMonitorCtx.forbidParam.forbidReason;
    }
}

void AswMonitor_ChargeStart(uint8_t port, uint8_t startSrc, uint8_t clearFlag)
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

            /* 有的协议需要外部初始化，所以这里需要判断是否需要清空订单数据 */
            if (clearFlag == TRUE)
            {
                memset(&pstAswMonitorData->stOrderData, 0x00, sizeof(pstAswMonitorData->stOrderData));
            }
            
            pstAswMonitorData->chargeStart = TRUE;
            pstAswMonitorData->stChargeCtrl.startSrc = startSrc;

            if (pstAswMonitorData->stChargeCtrl.startSrc == ASWMONITOR_ORDER_START_SRC_PNC)
            {
                pstAswMonitorData->stChargeCtrl.eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                pstAswMonitorData->stChargeCtrl.accountMoney = 999999;
            }

            pChargeData->lastMeterEnergyVal = AswChargeIf_GetMeterEnergyVal(port);
            pChargeData->startMeterVal = pChargeData->lastMeterEnergyVal;
            pChargeData->stopMeterVal = pChargeData->startMeterVal;

            pChargeData->chargeStartTime = SSTM_GetSecTimestamp();
            pChargeData->chargeStopTime = pChargeData->chargeStartTime;
            pChargeData->eChargeStopReason = eErr_none;

            pstAswMonitorData->orderDataSaveTick = Common_GetSystick();
            pstAswMonitorData->orderCtrl = ASWMONITOR_ORDER_CTRL_ONGOING;

            pstAswMonitorData->stOrderData.orderSaveState = ASWMONITOR_ORDER_SAVE_START;
            AswChargeIf_ChargeStart(port);
            AswMonitor_SaveChargeRecord(port, ASWMONITOR_ORDER_SAVE_START);
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

MSNvmOrderInfo_Struct *AswMonitor_GerOrderDataPtr(uint8_t port)
{
    AswMonitorData_Struct *pstAswMonitorData = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pstAswMonitorData = &g_stAswMonitorData[port];
    }

    return &pstAswMonitorData->stOrderData;
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
                MSNvm_InsertNewRecord(eMSNvmBlockID_OmOrderRecord, (uint8_t *)&pstAswMonitorData->stOrderData, sizeof(MSNvmOrderInfo_Struct));
            }
        }
    }

    if (eGlobalRet_OK != MSNvm_ReadParaBlock(eMSNvmBlockID_ForbidState, (uint8_t *)&g_stAswMonitorCtx.forbidParam, sizeof(MSNvmForbidState_Struct)))
    {
        g_stAswMonitorCtx.forbidParam.forbidState = FALSE;
    }

    ASWMONITOR_CFG_InfoPrint("设备锁机状态：%s\r\n", (g_stAswMonitorCtx.forbidParam.forbidState == 0) ? "未锁机" : "已锁机");
    g_stAswMonitorCtrlPara.minAccountMoney = ASWMONITOR_CFG_CHARGE_MIN_ACCOUNT_MONEY;
}

void AswMonitor_MainFunction(void)
{
    uint8_t port = 0;
    
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswMonitor_OrderManage(port, &g_stAswMonitorData[port]);
    }

    AswMonitor_RebootManage();

    AswMonitor_SwipCardManage();
}
