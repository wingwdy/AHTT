/******************************************************************************
* File Name          : Asw_IotProtoAPM.c
* Description        : 安培协议主模块源文件
* -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
*------------    --------     -------   ----------------------------------------
*2026/05/21     V1.0.0       WDY        初版创建
*
******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoAPM.h"
#include "Asw_ErrorHandle.h"
#include "Asw_IotProtoAPSend.h"
#include "Asw_IotProtoAPRecv.h"
#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "MS_Nvm.h"
#include "myMalloc.h"
#include "SS_Tm.h"
#include "SS_Ucm.h"
#include <stddef.h>

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_AP_B53_MAX_SEND_COUNT               (10)
#define IOT_AP_POWER_CTRL_DYNAMIC_TIMEOUT       (15 * 1000)
#define IOT_AP_POWER_CTRL_KIND_DEFAULT          (1)
#define IOT_AP_POWER_CTRL_KIND_DYNAMIC          (2)
#define IOT_AP_POWER_CTRL_KIND_CONTROL          (3)
#define IOT_AP_CP56_YEAR_2026                   (26U)


/*******************************************************************************
*    Enum Definition
******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
IotAPCtx_Struct *pIotAPCtx = NULL;

/* B47三缓冲计费模型全局变量(每枪独立一套A/B/C) */


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotAP_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotAP_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotAP_WSInitHandle(void);
static void IotAP_WSOfflineHandle(void);
static void IotAP_WSLoginHandle(void);
static void IotAP_WSNormalHandle(void);
static IotAPStopReason_Enum IotAP_ConverStopReason(AswErrorType_Enum errType);
static void IotAP_CycleDetectUnreportedRecord(void);
static void IotAP_CycleReportPowerStatus(void);
static void IotAP_CyclePowerCtrlManage(void);
static void IotAP_PowerCtrlLoadDefaultFromNVM(void);
static uint32_t IotAP_PowerCtrlGetDefaultValue(uint8_t port);
static void IotAP_PowerCtrlApplyDefault(uint8_t port);
static void IotAP_PowerCtrlDisableB57(uint8_t port);
static void IotAP_CycleCheckTimeBillSwitch(void);
static void IotAP_CycleReportUpgradeResult(void);
void IotAP_StopReasonToBcd(uint8_t *pData, IotAPStopReason_Enum stopReason);
static void IotAP_ClearSwipCardCtrl(uint8_t port);

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/*
 * 发送控制结构体查找函数
 * 根据 CMD 命令号返回对应端口的发送控制结构体指针
 * 索引分配(共18条):
 *  [0] F1 登录请求       [10] B34 功率控制上行
 *  [1] F3 U帧上报        [11] B40 FTP地址上行
 *  [2] F5 心跳请求       [12] B46 功率召测上行
 *  [3] F8 时钟同步应答   [13] B48 分时计费模型上行
 *  [4] B1 实时数据上报   [14] B49 计费切换生效上行
 *  [5] B5 启停控制结果   [15] B52 计费召测上行
 *  [6] B6 刷卡鉴权上行   [16] B53 在线分时明细上传
 *  [7] B10 启动通知上行  [17] B57 功率实时状态上行
 *  [8] B24 升级结果上报
 *  [9] B31 SIM卡信息上行
 */
static CommonSendCtrl_Struct* IotAP_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        switch (cmd)
        {
            case IOT_AP_CMD_LOGIN_REQ:                  pSendCtrl = &pIotAPCtx->stSendCtrl[port][0];   break;
            case IOT_AP_CMD_UFRAME_AUTH:                pSendCtrl = &pIotAPCtx->stSendCtrl[port][1];   break;
            case IOT_AP_CMD_HEARTBEAT_REQ:              pSendCtrl = &pIotAPCtx->stSendCtrl[port][2];   break;
            case IOT_AP_CMD_SYNC_TIME_RSP:              pSendCtrl = &pIotAPCtx->stSendCtrl[port][3];   break;
            case IOT_AP_CMD_B01_REALTIME_DATA:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][4];   break;
            case IOT_AP_CMD_B05_CHG_CTRL_RESULT:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][5];   break;
            case IOT_AP_CMD_B06_CARD_AUTH_UP:           pSendCtrl = &pIotAPCtx->stSendCtrl[port][6];   break;
            case IOT_AP_CMD_B10_START_NOTIFY_UP:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][7];   break;
            case IOT_AP_CMD_B24_UPGRADE_RESULT:         pSendCtrl = &pIotAPCtx->stSendCtrl[port][8];   break;
            case IOT_AP_CMD_B31_SIM_INFO_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][9];   break;
            case IOT_AP_CMD_B34_POWER_CTRL_UP:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][10];  break;
            case IOT_AP_CMD_B40_FTP_ADDR_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][11];  break;
            case IOT_AP_CMD_B46_POWER_POLL_UP:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][12];  break;
            case IOT_AP_CMD_B48_TIMEBILL_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][13];  break;
            case IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP:     pSendCtrl = &pIotAPCtx->stSendCtrl[port][14];  break;
            case IOT_AP_CMD_B52_TIMEBILL_POLL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][15];  break;
            case IOT_AP_CMD_B53_ONLINE_DETAIL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][16];  break;
            case IOT_AP_CMD_B57_POWER_STATUS_UP:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][17];  break;

            default: break;
        }
    }    

    return pSendCtrl;
}

/*
 * 接收控制结构体查找函数
 * 根据 CMD 命令号返回对应端口的接收控制结构体指针
 * 索引分配(共17条):
 *  [0] F2 登录应答       [9]  B32 终端数据请求下发
 *  [1] F4 U帧回复        [10] B33 功率控制下发
 *  [2] F6 心跳应答       [11] B39 FTP地址下发
 *  [3] F7 时钟同步请求   [12] B45 功率召测下发
 *  [4] B4 启停控制下发   [13] B47 分时计费模型下发
 *  [5] B7 刷卡鉴权下行   [14] B50 计费切换生效下发
 *  [6] B11 启动通知下行  [15] B51 计费召测下发
 *  [7] B14 扣款确认下行  [16] B54 在线分时明细下行
 *  [8] B23 远程升级启动
 */
static CommonRecvCtrl_Struct* IotAP_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        switch (cmd)
        {
            case IOT_AP_CMD_LOGIN_RSP:                 pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][0];   break;
            case IOT_AP_CMD_UFRAME_ACK:                pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][1];   break;
            case IOT_AP_CMD_HEARTBEAT_RSP:             pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][2];   break;
            case IOT_AP_CMD_SYNC_TIME_REQ:             pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][3];   break;
            case IOT_AP_CMD_B04_CHG_CTRL_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][4];   break;
            case IOT_AP_CMD_B07_CARD_AUTH_DOWN:        pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][5];   break;
            case IOT_AP_CMD_B11_START_NOTIFY_DOWN:     pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][6];   break;
            case IOT_AP_CMD_B14_DEDUCT_CONFIRM:        pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][7];   break;
            case IOT_AP_CMD_B23_UPGRADE_START:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][8];   break;
            case IOT_AP_CMD_B32_TERMINAL_REQ_DOWN:     pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][9];   break;
            case IOT_AP_CMD_B33_POWER_CTRL_DOWN:       pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][10];  break;
            case IOT_AP_CMD_B39_FTP_ADDR_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][11];  break;
            case IOT_AP_CMD_B45_POWER_POLL_DOWN:       pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][12];  break;
            case IOT_AP_CMD_B47_TIMEBILL_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][13];  break;
            case IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN:  pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][14];  break;
            case IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN:    pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][15];  break;
            case IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN:    pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][16];  break;

            default: break;
        }
    }

    return pRecvCtrl;
}

/* ====== 初始化状态处理 ====== */
static void IotAP_WSInitHandle(void)
{
    pIotAPCtx->eWorkState = eIotAPWorkState_Offline;
}

/**
 * @brief  平台离线状态处理，复位协议层所有运行时上下文
 * @note   进入离线态后执行以下清理：
 *         Step1: 清除登录/排队标志，重置发送序号和请求序列号
 *         Step2: 清零实时数据上报计时器及各枪状态快照
 *         Step3: 复位收发控制结构体，重置帧队列
 *         Step4: 桩编号 ASCII→BCD 重新转换，上报离线错误，转登录态等待重连
 * @param[in]  无
 * @retval  无
 */
static void IotAP_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    /* 清除登录和队列状态 */
    pIotAPCtx->loginSucc = FALSE;
    pIotAPCtx->queueBusyFlag = FALSE;
    pIotAPCtx->waitQueueIdleTick = 0;

    /* 重置发送状态 */
    pIotAPCtx->sendIndex = 0;
    pIotAPCtx->sendPort = 0;
    pIotAPCtx->reqSeq = 0;

    /* 清零实时数据上报计时和枪状态记录 */
    memset(pIotAPCtx->realDataReportTick, 0x00, sizeof(pIotAPCtx->realDataReportTick));
    memset(pIotAPCtx->lastGunState, 0x00, sizeof(pIotAPCtx->lastGunState));
    memset(pIotAPCtx->lastGunConnectState, 0x00, sizeof(pIotAPCtx->lastGunConnectState));

    /* 清零所有收发控制结构体 */
    memset(pIotAPCtx->stSendCtrl, 0x00, sizeof(pIotAPCtx->stSendCtrl));
    memset(pIotAPCtx->stRecvCtrl, 0x00, sizeof(pIotAPCtx->stRecvCtrl));

    /* 重置帧队列并转换桩编号为BCD码 */
    FrameQueue_Reset(pIotAPCtx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotAPCtx->pileDnBCD, 16);

    /* 设置平台离线错误回调 */
    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);

    pIotAPCtx->eWorkState = eIotAPWorkState_Login;
}

static void IotAP_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotAPCtx->eWorkState = eIotAPWorkState_Normal;
        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_LOGIN_REQ, TRUE);
    }
}

/**
 * @brief  正常工作状态处理，执行平台上线后的周期性任务
 * @note   Step1: 检测网络链路断开则切离线态
 *         Step2: 已登录后执行周期性任务（实时数据上报、功率状态上报、未上报记录检测、计费切换检查）
 *         Step3: 上行发送控制处理（遍历发送控制表、组帧写入队列）
 *         Step4: 上行接收解析处理（取帧校验、CMD识别分发）
 *         Step5: 超时检测
 * @param[in]  无
 * @retval  无
 */
static void IotAP_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotAP_OfflineHandle();
    }
    else
    {
        /* 已登录后的周期性任务 */
        if (pIotAPCtx->loginSucc == TRUE)
        {
            IotAP_CyclePowerCtrlManage();
            IotAP_CycleReportRealData();
            IotAP_CycleReportPowerStatus();
            IotAP_CycleDetectUnreportedRecord();
            IotAP_CycleCheckTimeBillSwitch();
            IotAP_CycleReportUpgradeResult();
        }

        /* 上行发送控制处理：遍历发送控制表，组帧并写入FrameQueue */
        IotAP_UpCtrlSendDeal();

        /* 上行接收解析处理：从FrameQueue取帧、校验、识别CMD并分发解析函数 */
        IotAP_UpCtrlRecvDeal();

        IotAP_TimeoutDetect();
    }
}

void IotAP_StopReasonToBcd(uint8_t *pData, IotAPStopReason_Enum stopReason)
{
    uint16_t reason = (uint16_t)stopReason;

    if (pData != NULL)
    {
        pData[0] = (uint8_t)((((reason / 10) % 10) << 4) | (reason % 10));
        pData[1] = (uint8_t)((((reason / 1000) % 10) << 4) | ((reason / 100) % 10));
    }
}

/**
 * @brief  周期检查计费模式定时切换是否到达触发时刻
 * @details 在系统时间已校准（年份≥2026）的前提下，逐枪检查定时切换标志；
 *          当切换时刻已到达且对应计费模型有效时，刷新当前计费模型并上报
 *          B49 定时切换通知。仅在订单空闲时执行刷新，避免干扰正在进行的订单。
 * @param  无（内部使用全局上下文 pIotAPCtx 及枪号数组）
 * @return 无
 */
static void IotAP_CycleCheckTimeBillSwitch(void)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t port = 0;
    uint8_t nowCp56[7] = {0};
    uint8_t cp56Year = 0;
    uint32_t nowTimestamp = 0;
    uint32_t nextSwitchSec = 0;
    uint8_t activeIdx = 0;
    const MSNvmAPParamBillMode_Struct *pModel = NULL;

    nowTimestamp = SSTM_GetSecTimestamp();
    Common_TimestampToCp56Time2a(nowTimestamp, nowCp56);
    cp56Year = (uint8_t)(nowCp56[6] & 0x7FU);

    /* 系统时间尚未校准时不做切换判断，避免上电早期误触发B49 */
    if (cp56Year >= IOT_AP_CP56_YEAR_2026)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            nextSwitchSec = pIotAPCtx->nextSwitchSec[port];
            if ((nextSwitchSec == 0U) ||
                (nowTimestamp < nextSwitchSec))
            {
                continue;
            }

            if (AswMonitor_IsOrderIdle(port) != TRUE)
            {
                continue;
            }

            if ((Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, port,IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP) == TRUE) ||
                (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port,IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN) == TRUE))
            {
                continue;
            }

            /* 切换时间到，刷新模型决策（内部会更新 pIotAPCtx->nextSwitchSec） */
            IotAP_RefreshNowbillModel(port);

            /* 填充 B49 上报数据（取当前活跃模型） */
            {
                activeIdx = pIotAPCtx->billActiveIndex[port];
                pModel = IotAP_GetActiveBillMode(port);

                if (pModel != NULL)     /*存在生效有效模型*/
                {
                    pProtoData = &pIotAPCtx->stProtoData[port];
                    memcpy(pProtoData->timeBillSwitchModelId,pModel->billModeID,sizeof(pProtoData->timeBillSwitchModelId));
                    memcpy(pProtoData->timeBillSwitchTime,pModel->switchTime,sizeof(pProtoData->timeBillSwitchTime));
                    pProtoData->timeBillSwitchIndex = activeIdx;
                    pProtoData->timeBillSwitchResult = 0;

                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port,
                                            IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP, TRUE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port,
                                            IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP, TRUE);
                    IOTAP_CFG_InfoPrint("AP,B49切换到达: port%d 组%d\r\n", port, activeIdx);
                }
            }
        }
    }
}

/**
 * @brief  周期检测NVM中未上报的订单记录，并在满足条件时触发B53在线详情上行补传
 *
 * 在IoT登录成功后由主循环周期性调用。依次执行以下逻辑：
 * 1. 检查是否存在未上报记录（轻量级计数查询）；
 * 2. 检查所有枪口是否正在执行B53/B54收发流程（全局互斥保护）；
 * 3. 从NVM取出最早未上报的订单记录；
 * 4. 校验记录有效性：枪号越界、非AP协议、非STOP状态的记录将被标记为已上报并丢弃；
 * 5. 对有效记录且重试次数未超限的情况下，设置B53立即发送标志触发补传。
 *
 * @note  仅在所有枪口B53/B54均空闲时才取新记录，保证同一时刻仅一份订单在发送流程中
 * @note  重试次数上限为IOT_AP_B53_MAX_SEND_COUNT，防止无限重发占用资源
 * @note  非STOP状态（START/PERIOD）的残留记录会被安全丢弃，具备NVM队列自清理能力
 */
static void IotAP_CycleDetectUnreportedRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if ((Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP) == TRUE) ||
                (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN) == TRUE))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord,
                                                                    (uint8_t *)&pIotAPCtx->stOrderInfo,
                                                                    sizeof(MSNvmOrderInfo_Struct),
                                                                    &pIotAPCtx->time))
            {
                // IOTAP_CFG_InfoPrint("AP,[NVM取出]port=%d, protoType=%d, saveState=%d, orderLen=%d, periodCount=%d\r\n", //订单上送10次失败之后会一直刷屏
                //     pIotAPCtx->stOrderInfo.port,
                //     pIotAPCtx->stOrderInfo.protocolType,
                //     pIotAPCtx->stOrderInfo.orderSaveState,
                //     pIotAPCtx->stOrderInfo.orderLen,
                //     pIotAPCtx->stOrderInfo.platOrderInfo.stAPOrderInfo.periodCount);

                port = pIotAPCtx->stOrderInfo.port;

                if ((port >= SYSCFG_CFG_GUN_NUM) ||
                    (pIotAPCtx->stOrderInfo.protocolType != eAswPlatCardType_AP) ||
                    (pIotAPCtx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP))
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotAPCtx->time);
                }
                else
                {
                    if (Common_GetRptCount(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN) < IOT_AP_B53_MAX_SEND_COUNT)
                    {
                        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, TRUE);
                        Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, TRUE);
                        Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                    }
                }
            }
        }
    }
}

static IotAPStopReason_Enum IotAP_ConverStopReason(AswErrorType_Enum errType)
{
    uint8_t index = 0;
    IotAPStopReason_Enum eStopReason = eIotAPStopReason_OtherErr;
    uint8_t findFlag = FALSE;

    const struct
    {
        AswErrorType_Enum errType;
        IotAPStopReason_Enum stopReason;
    } stopReasonMap[] =
    {
        {eErr_none,              eIotAPStopReason_Null},
        {eErr_CpVoltAbnor,       eIotAPStopReason_CpFault},
        {eErr_CpGroundFault,     eIotAPStopReason_CpFault},
        {eErr_PEBreakFault,      eIotAPStopReason_PEBreakFault},
        {eErr_EmergencyStop,     eIotAPStopReason_EmergencyStop},
        {eErr_InputLineReversed, eIotAPStopReason_LNReversed},
        {eErr_LeakageCurrErr,    eIotAPStopReason_LeakageCurrErr},
        {eErr_ShortCircleErr,    eIotAPStopReason_ShortCircleErr},
        {eErr_RCDSelfcheckErr,   eIotAPStopReason_RCDSelfcheckErr},

        {eErr_AphaseInputOverVol, eIotAPStopReason_InputFault},
        {eErr_AphaseInputLessVol, eIotAPStopReason_InputFault},
        {eErr_OutputOverCurr,     eIotAPStopReason_InputFault},

        {eErr_JcqMaloperation,   eIotAPStopReason_JcqMaloperation},
        {eErr_JcqSynechiaFault,  eIotAPStopReason_JcqSynechiaFault},
        {eErr_HmiCommErr,        eIotAPStopReason_HmiCommErr},
        {eErr_ReaderCommErr,     eIotAPStopReason_ReaderCommErr},
        {eErr_MeterCommErr,      eIotAPStopReason_MeterCommErr},
        {eErr_EnvOverTempErr,    eIotAPStopReason_TempErr},
        {eErr_GunOverTempErr,    eIotAPStopReason_TempErr},
        {eErr_POverTempErr,      eIotAPStopReason_TempErr},

        {eErr_DatabaseErr,       eIotAPStopReason_eErr_DatabaseErr},
        {eErr_MeterCalcErr,      eIotAPStopReason_MeterCalcErr},

        {eErr_ChgStartTimeout,   eIotAPStopReason_StartTimeout},

        {eErr_DiodeStop,         eIotAPStopReason_DiodeStop},

        {eSrc_LittleCurr,        eIotAPStopReason_BMSStop},
        {eSrc_S2BreakOff,        eIotAPStopReason_Full},
        {eSrc_AppStop,           eIotAPStopReason_AppStop},
        {eSrc_MannulStop,        eIotAPStopReason_KeyStop},
        {eSrc_CardStop,          eIotAPStopReason_Card},
        {eSrc_InsuffBalance,     eIotAPStopReason_InsuffBalance},
        {eSrc_StopbyMoney,       eIotAPStopReason_SumNoEnough},
        {eSrc_StopbyTime,        eIotAPStopReason_TimeEnough},
        {eSrc_StopbyEnergy,      eIotAPStopReason_EnergyEnough},
        {eErr_GunDisConn,        eIotAPStopReason_Full},
        {eErr_CPBreakOff,        eIotAPStopReason_Full},

        {eErr_NetNoSIMErr,       eIotAPStopReason_NetNoSIMErr},
        {eErr_PlatformOffline,   eIotAPStopReason_PlatformOffline},
    };

    for (index = 0; index < ARRAY_SIZE(stopReasonMap); index++)
    {
        if (errType == stopReasonMap[index].errType)
        {
            eStopReason = stopReasonMap[index].stopReason;
            findFlag = TRUE;
            IOTAP_CFG_InfoPrint("安培结束原因转换，原始类型：%d, 转换后停止原因：%d\r\n", errType, eStopReason);
            break;
        }
    }

    if (findFlag == FALSE)
    {
        IOTAP_CFG_InfoPrint("安培结束原因转换，未找到对应原因，原始原因为：%d!\r\n", errType);
    }

    return eStopReason;
}

/* ====== 断线离线处理 ====== */
/**
 * @brief  IoT AP离线处理函数，执行断网及状态清理操作
 *
 * 断开网络连接，清除登录成功标志，并将工作状态切换为离线，
 * 后续清理工作由WSOfflineHandle完成。
 *
 * @param  无
 * @return 无
 */
void IotAP_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    
    /* 清除登录成功标志 */
    pIotAPCtx->loginSucc = FALSE;

    /* 切换到离线状态，由WSOfflineHandle完成剩余清理 */
    pIotAPCtx->eWorkState = eIotAPWorkState_Offline;
}

/******************************************************************************
 *    对外接口函数实现
 ******************************************************************************/

/**
 * @brief  填充IoT AP平台TCP连接参数并创建帧队列通道
 * @param  pLinkPara  指向网络连接参数联合体的指针，用于存储TCP连接所需参数
 * @note   从平台参数中读取主IP和端口，同时为该连接创建TCP类型的帧队列通道，
 *         并将通道ID写入连接参数中；当输入指针或上下文指针为NULL时跳过填充
 * @retval 无返回值
 */
uint8_t IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pParam->platMainIp) - 1);
    pLinkPara->stTcpPara.port = pParam->platMainPort;
    FrameQueue_Creat(eFrameQueueType_TCP, IOT_AP_TXRX_BUFFER_SIZE, IOT_AP_TXRX_BUFFER_SIZE,
                        &pIotAPCtx->frameQueueChannelID);
    pLinkPara->stTcpPara.frameQueueChannelID = pIotAPCtx->frameQueueChannelID;
    
    return TRUE;
}

/**
 * @brief  初始化IOT AP模块的内存上下文
 * @note   分配IotAPCtx_Struct结构体内存并将其清零，设置收发控制指针，
 *         从NVM加载费率B47模型及功率控制默认配置
 * @return 无返回值；若内存分配失败则直接返回
 */
void IotAP_InitMemory(void)
{
    pIotAPCtx = (IotAPCtx_Struct *)myMalloc(sizeof(IotAPCtx_Struct));

    memset(pIotAPCtx, 0, sizeof(IotAPCtx_Struct));
    pIotAPCtx->pFuncSendCtrl = IotAP_GetSendCtrl;
    pIotAPCtx->pFuncRecvCtrl = IotAP_GetRecvCtrl;
    IotAP_ReadRateB47ModelFromNVM();
    IotAP_PowerCtrlLoadDefaultFromNVM();
}


static uint32_t IotAP_PowerCtrlGetDefaultValue(uint8_t port)
{
    uint32_t defaultValue = (SYSCFG_CFG_MAX_OUTPUT_POWER / 10);

    if ((pIotAPCtx->stProtoData[port].powerCtrlDefaultValue <= (SYSCFG_CFG_MAX_OUTPUT_POWER / 10)))
    {
        defaultValue = pIotAPCtx->stProtoData[port].powerCtrlDefaultValue;
    }

    return defaultValue;
}

static void IotAP_PowerCtrlLoadDefaultFromNVM(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint32_t defaultValue = 0;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        defaultValue = pPrivateParam->stAPParam.powerCtrlDefaultValue[port];
        if ((defaultValue == 0) || (defaultValue > (SYSCFG_CFG_MAX_OUTPUT_POWER / 10)))
        {
            defaultValue = (SYSCFG_CFG_MAX_OUTPUT_POWER / 10);
        }

        pIotAPCtx->stProtoData[port].powerCtrlDefaultValue = defaultValue;
        pIotAPCtx->stProtoData[port].powerCtrlActiveValue = defaultValue;
        pIotAPCtx->stProtoData[port].powerCtrlActiveKind = IOT_AP_POWER_CTRL_KIND_DEFAULT;
    }
}

void IotAP_PowerCtrlSetDefaultValue(uint8_t port, uint32_t powerValue)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();

    if ((port < SYSCFG_CFG_GUN_NUM) &&
        (powerValue >= 0) &&
        (powerValue <= (SYSCFG_CFG_MAX_OUTPUT_POWER / 10)))
    {
        pIotAPCtx->stProtoData[port].powerCtrlDefaultValue = powerValue;
        if (pPrivateParam->stAPParam.powerCtrlDefaultValue[port] != powerValue)
        {
            pPrivateParam->stAPParam.powerCtrlDefaultValue[port] = powerValue;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                                 (uint8_t *)pPrivateParam,
                                 sizeof(MSNvmPlatPrivateParam_Union));
        }
    }
}

static void IotAP_PowerCtrlDisableB57(uint8_t port)
{
    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
    pIotAPCtx->stProtoData[port].powerCtrlB57Enable = FALSE;
}

static void IotAP_PowerCtrlApplyDefault(uint8_t port)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint32_t defaultValue = 0;

    pProtoData = &pIotAPCtx->stProtoData[port];
    defaultValue = IotAP_PowerCtrlGetDefaultValue(port);
    pProtoData->powerCtrlActiveValue = defaultValue;
    pProtoData->powerCtrlActiveKind = IOT_AP_POWER_CTRL_KIND_DEFAULT;
    pProtoData->powerCtrlB57Enable = FALSE;
    AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, defaultValue * 10);
    IotAP_PowerCtrlDisableB57(port);
}

/**
 * @brief 清除指定枪口的功率控制充电值，将所有功率控制相关参数复位至默认状态
 * @param port 枪口号，有效范围为 [0, SYSCFG_CFG_GUN_NUM)
 * @note 该函数会将动态功率值、控制功率值清零，动态有效标志置为无效，
 *       控制激活标志置为无效，激活功率值恢复为默认值，
 *       激活类型恢复为默认类型，并禁用B57功率控制上报
 */
void IotAP_PowerCtrlClearChargeValue(uint8_t port)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint32_t defaultValue = 0;

    pProtoData = &pIotAPCtx->stProtoData[port];
    defaultValue = IotAP_PowerCtrlGetDefaultValue(port);
    pProtoData->powerCtrlDynamicValue = 0;
    pProtoData->powerCtrlControlValue = 0;
    pProtoData->powerCtrlDynamicTick = 0;
    pProtoData->powerCtrlDynamicValid = FALSE;
    pProtoData->powerCtrlControlActive = FALSE;
    pProtoData->powerCtrlPauseCmd = 0;
    pProtoData->powerCtrlPauseActive = FALSE;
    pProtoData->powerCtrlActiveValue = defaultValue;
    pProtoData->powerCtrlActiveKind = IOT_AP_POWER_CTRL_KIND_DEFAULT;
    AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, defaultValue * 10);
    IotAP_PowerCtrlDisableB57(port);
}

/**
 * @brief  判断某功率限值是否低于当前生效值（可作为更优先的生效限值）
 * @note   限值需 >= 0 为有效（允许以 0 强制限功率），且小于当前生效值才生效
 * @param[in] limit       待比较的功率限值
 * @param[in] activeValue 当前已生效的功率值
 * @retval TRUE  限值有效且低于当前生效值；FALSE  否则
 */
static uint8_t IotAP_PowerCtrlIsLower(uint32_t limit, uint32_t activeValue)
{
    uint8_t ret = FALSE;
    if ((limit >= 0) && (limit < activeValue))
    {
        ret = TRUE;
    }
    return ret;
}

/**
 * @brief  功率控制策略申请，根据动态限值和控制限值计算当前生效功率并下发
 *
 * 优先级从低到高：默认功率 < 动态限值 < 控制限值（取三者中最小有效值）。
 * 当动态限值超时后自动失效回退；当生效值发生变化时调用充电接口调整输出电流；
 * 若控制限值生效则使能B57功率状态上报，否则关闭B57上报。
 *
 * @param  port  充电枪端口号，有效范围 [0, SYSCFG_CFG_GUN_NUM)
 * @return 无
 */
void IotAP_PowerCtrlApply(uint8_t port)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint32_t defaultValue = 0;
    uint32_t activeValue = 0;
    uint8_t activeKind = IOT_AP_POWER_CTRL_KIND_DEFAULT;  /* 初始化为默认类型 */
    uint8_t b57Enable = FALSE;                             /* 控制限值生效时才使能 */

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pProtoData = &pIotAPCtx->stProtoData[port];
        defaultValue = IotAP_PowerCtrlGetDefaultValue(port);
        activeValue = defaultValue;  /* 初始值取默认功率 */

        /* Step1: 动态功率超时检查——超过15s未刷新则自动失效 */
        if ((pProtoData->powerCtrlDynamicValid == TRUE) &&
            (Common_JudgeTimeoutMs(pProtoData->powerCtrlDynamicTick, IOT_AP_POWER_CTRL_DYNAMIC_TIMEOUT) == TRUE))
        {
            pProtoData->powerCtrlDynamicValid = FALSE;
            pProtoData->powerCtrlDynamicValue = 0;
        }

        /* Step2: 三级优先级仲裁——默认功率 < 动态限值 < 控制限值（取最小值） */
        /* 动态限值低于当前生效值时，进一步降功率 */
        if ((pProtoData->powerCtrlDynamicValid == TRUE) &&
            (IotAP_PowerCtrlIsLower(pProtoData->powerCtrlDynamicValue, activeValue) == TRUE))
        {            
            activeValue = pProtoData->powerCtrlDynamicValue;
            activeKind = IOT_AP_POWER_CTRL_KIND_DYNAMIC;
        }

        /* 控制限值低于当前生效值（默认或动态后的最小值）时，进一步降功率 */
        if ((pProtoData->powerCtrlControlActive == TRUE) &&
            (IotAP_PowerCtrlIsLower(pProtoData->powerCtrlControlValue, activeValue) == TRUE))
        {
            activeValue = pProtoData->powerCtrlControlValue;
            activeKind = IOT_AP_POWER_CTRL_KIND_CONTROL;
            b57Enable = TRUE;  /* 控制限值生效，需使能B57上报 */
        }

        /* 平台暂停只作用于当前订单，优先级高于所有功率控制。 */
        if (pProtoData->powerCtrlPauseActive == TRUE)
        {
            activeValue = 0;
        }

        /* Step3: 生效值发生变化时记录时间戳并下发到充电接口 */
        if (pProtoData->powerCtrlActiveValue != activeValue)
        {
            AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, activeValue * 10);
        }

        /* 更新状态变量（每次调用都刷新，即使值未变化） */
        pProtoData->powerCtrlActiveValue = activeValue;
        pProtoData->powerCtrlActiveKind = activeKind;
        pProtoData->powerCtrlB57Enable = b57Enable;
    }
}

/**
 * @brief  周期性功率控制管理，根据各枪订单状态变化执行功率控制策略
 *
 * 遍历所有充电枪，检测订单忙闲状态的跳变沿，并在以下三种情形下
 * 执行相应处理：
 * - 上升沿（空闲→忙）：清除充电累计值并应用默认功率配置
 * - 下降沿（忙→空闲）：清除充电累计值
 * - 持续忙：应用当前功率控制指令
 *
 * 每次循环结束后更新上一次订单忙闲状态，用于下次边沿检测。
 *
 * @note  依赖全局上下文指针 pIotAPCtx 非空
 */
static void IotAP_CyclePowerCtrlManage(void)
{
    uint8_t port = 0;
    uint8_t orderBusy = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        orderBusy = (AswMonitor_IsOrderIdle(port) != TRUE) ? TRUE : FALSE;

        if ((orderBusy != pIotAPCtx->stProtoData[port].powerCtrlLastOrderBusy)) /*充电状态改变*/
        {
            IotAP_PowerCtrlClearChargeValue(port);
            pIotAPCtx->stProtoData[port].powerCtrlLastOrderBusy = orderBusy;
        }

        if (orderBusy == TRUE)  /*充电中*/
        {
            IotAP_PowerCtrlApply(port);
        }
    }
}

/**
 * @brief  将AP平台计费模式参数转换为标准计费模式结构体
 * @note   从平台私有参数中提取计费模式信息，将BCD编码的时间转换为BIN格式，
 *         并填充到标准计费模式结构体中。仅当AP计费模式有效
 *         （时段数合法、工作状态为[0,1]）时才进行转换，否则输出结构体保持清零状态。
 * @param[in]  port              枪号
 * @param[out] pStandardBillMode 标准计费模式结构体指针，转换结果写入此结构体；
 *                               若为NULL则不执行任何操作；调用前结构体内容会被清零
 * @return 无
 */
void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    const MSNvmAPParamBillMode_Struct *pAPBillMode = NULL;
    uint8_t activeIndex = IOTAP_B47_INDEX_INVALID;
    uint8_t index = 0;
    uint8_t rateIndex = 0;
    uint8_t workStateValid = FALSE;
    uint8_t startTime[2] = {0};
    uint8_t stopTime[2] = {0};

    if (pStandardBillMode != NULL)
    {
        memset(pStandardBillMode, 0x00, sizeof(AswMonitorBillMode_Struct));

        if (port < SYSCFG_CFG_GUN_NUM)
        {
            activeIndex = pIotAPCtx->billActiveIndex[port];
            if ((AswMonitor_IsOrderIdle(port) == TRUE) ||
                (!IOTAP_IS_VALID_BILL_INDEX(activeIndex)))
            {
                IotAP_RefreshNowbillModel(port);
                activeIndex = pIotAPCtx->billActiveIndex[port];
            }

            if (IOTAP_IS_VALID_BILL_INDEX(activeIndex))
            {
                pAPBillMode = IotAP_GetActiveBillMode(port);
                if ((pAPBillMode != NULL) &&
                    ((pAPBillMode->workState[0] == 1) && (pAPBillMode->workState[1] == 0)))
                {
                    workStateValid = TRUE;
                }
            }
        }

        if ((pAPBillMode != NULL) &&
            (pAPBillMode->periodCount > 0) &&
            (pAPBillMode->periodCount <= MSNVM_AP_BILLMODE_PERIOD_COUNT) &&
            (workStateValid == TRUE))
        {
            memcpy(pStandardBillMode->billModeID, pAPBillMode->billModeID, sizeof(pStandardBillMode->billModeID));
            pStandardBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_MULT;
            pStandardBillMode->periodCount = pAPBillMode->periodCount;

            for (index = 0; index < pAPBillMode->periodCount; index++)
            {
                rateIndex = pAPBillMode->period[index].periodRate - 1;

                if (rateIndex < ASWMONITOR_BILLMODE_RATE_COUNT)
                {
                    Common_BCDToBIN((uint8_t *)pAPBillMode->period[index].startTime, startTime, sizeof(startTime));
                    Common_BCDToBIN((uint8_t *)pAPBillMode->period[index].stopTime, stopTime, sizeof(stopTime));

                    pStandardBillMode->periodRate[index] = pAPBillMode->period[index].periodRate - 1;
                    pStandardBillMode->startTime[index][0] = startTime[1];
                    pStandardBillMode->startTime[index][1] = startTime[0];
                    pStandardBillMode->stopTime[index][0] = stopTime[1];
                    pStandardBillMode->stopTime[index][1] = stopTime[0];

                    memcpy(&pStandardBillMode->rateElecPrice[rateIndex],
                           pAPBillMode->period[index].elecPrice,
                           sizeof(pAPBillMode->period[index].elecPrice));
                    memcpy(&pStandardBillMode->rateSeverPrice[rateIndex],
                           pAPBillMode->period[index].servePrice,
                           sizeof(pAPBillMode->period[index].servePrice));
                    pStandardBillMode->totalPrice[rateIndex] = pStandardBillMode->rateElecPrice[rateIndex] + 
                                                               pStandardBillMode->rateSeverPrice[rateIndex];
                }
            }

            pStandardBillMode->rateCount = pAPBillMode->periodCount;
            pStandardBillMode->validFlag = TRUE;
        }
        else if (pAPBillMode != NULL)
        {
            IOTAP_CFG_InfoPrint("AP,B47计费模型转换无效: periodCount=%d, workState=%02X %02X\r\n",
                pAPBillMode->periodCount, pAPBillMode->workState[0], pAPBillMode->workState[1]);
        }
        else
        {
            IOTAP_CFG_InfoPrint("AP,B47计费模型转换无效: activeIndex=%d\r\n", activeIndex);
        }
    }
}

/**
 * @brief  打包充电订单记录数据，将充电监控数据转换为AP平台订单存储格式
 * @note   当 orderSaveReason 为启动保存时，初始化订单结构体并填充启动信息；
 *         为停止保存时，更新停止原因；每次调用均刷新费率时段、费用及电量等实时数据。
 *         电量单位由 Wh 转换为 0.1kWh，金额单位由 分 转换为 元。
 * @param[in]  port             充电枪端口号，范围 [0, SYSCFG_CFG_GUN_NUM)
 * @param[in]  pOrderData       订单数据存储结构体指针，用于存放打包后的订单信息
 * @param[in]  orderSaveReason  订单保存原因，取值 ASWMONITOR_ORDER_SAVE_START(启动保存)
 *                              或 ASWMONITOR_ORDER_SAVE_STOP(停止保存)
 * @retval 无
 */
void IotAP_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{
    AswMonitorBillMode_Struct *pBillMode = NULL;
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    AswMonitorChargeData_Struct *pChargeData = NULL;
    MSNvmAPOrderInfo_Struct *pAPOrder = NULL;
    uint8_t index = 0;
    uint8_t periodCount = 0;
    uint32_t tempVal = 0;
    uint32_t totalEnergyVal = 0;
    uint32_t totalElecFeeVal = 0;
    uint32_t totalServeFeeVal = 0;
    uint32_t periodEnergyVal[MSNVM_AP_BILLMODE_PERIOD_COUNT] = { 0 };
    uint8_t periodEnergyRemainder[MSNVM_AP_BILLMODE_PERIOD_COUNT] = { 0 };
    uint8_t periodEnergyAdjusted[MSNVM_AP_BILLMODE_PERIOD_COUNT] = { 0 };
    uint32_t startMeterVal = 0;
    uint32_t stopMeterVal = 0;
    uint32_t targetEnergyVal = 0;
    uint32_t deltaEnergyVal = 0;
    uint8_t adjustIndex = 0;
    uint8_t selectedIndex = 0;
    uint8_t maxRemainder = 0;
    uint8_t selectedFlag = FALSE;

    if ((pOrderData != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
        pChargeData = AswMonitor_GetChargeDataPtr(port);
        pAPOrder = &pOrderData->platOrderInfo.stAPOrderInfo;

        if ((pBillMode != NULL) && (pChargeCtrl != NULL) && (pChargeData != NULL))
        {
            periodCount = pBillMode->periodCount;
            if (periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT)
            {
                periodCount = MSNVM_AP_BILLMODE_PERIOD_COUNT;
            }
            startMeterVal = pChargeData->startMeterVal / 10;
            stopMeterVal = pChargeData->stopMeterVal / 10;
            if (stopMeterVal >= startMeterVal)
            {
                targetEnergyVal = stopMeterVal - startMeterVal;
            }

            if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
            {
                memset(pAPOrder, 0x00, sizeof(MSNvmAPOrderInfo_Struct));
                pAPOrder->port = port;
                memcpy(pAPOrder->orderTransactionNum,
                       pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum,
                       sizeof(pAPOrder->orderTransactionNum));
                Common_TimestampToCp56Time2a(pChargeData->chargeStartTime, pAPOrder->startTime);
                Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, pAPOrder->stopTime);
                Common_Uint32ToFourUint8(pAPOrder->startMeterVal, startMeterVal);
                Common_Uint32ToFourUint8(pAPOrder->stopMeterVal, stopMeterVal);

                if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
                {
                    /* authCardID为大端BCD序，协议帧要求小端序，此处逆序拷贝 */
                    uint8_t i;
                    for (i = 0; i < sizeof(pAPOrder->logicCardNum); i++)
                    {
                        pAPOrder->logicCardNum[i] =
                            pChargeCtrl->authCardID[sizeof(pAPOrder->logicCardNum) - 1u - i];
                    }
                }
                else
                {
                    memset(pAPOrder->logicCardNum, 0x00, sizeof(pAPOrder->logicCardNum));
                }

                pOrderData->port = port;
                pOrderData->protocolType = eAswPlatCardType_AP;
                pOrderData->orderLen = sizeof(MSNvmAPOrderInfo_Struct);
                IotAP_StopReasonToBcd(pAPOrder->stopReason, eIotAPStopReason_PowerOff);
            }

            pAPOrder->periodCount = periodCount;
            Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, pAPOrder->stopTime);
            Common_Uint16ToTwoUint8(pAPOrder->chargeTimeMin, (uint16_t)(pChargeData->chargeTime / 60));

            /* 电能电费服务费使用时段的值累加，避免出现和时段累加值不相等的情况 */
            for (index = 0; index < MSNVM_AP_BILLMODE_PERIOD_COUNT; index++)
            {
                if (index < periodCount)
                {
                    pAPOrder->periodInfo[index].timeSerialNumber = (uint8_t)(index + 1);
                    pAPOrder->periodInfo[index].timeKind = pBillMode->periodRate[index] + 1;

                    periodEnergyVal[index] = pChargeData->periodElePower[index] / 10;
                    periodEnergyRemainder[index] = pChargeData->periodElePower[index] % 10;
                    totalEnergyVal += periodEnergyVal[index];

                    tempVal = pChargeData->periodEleMoney[index] / 100;
                    Common_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeElecFee, tempVal);
                    totalElecFeeVal += tempVal;

                    tempVal = pChargeData->periodSerMoney[index] / 100;
                    Common_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeServeFee, tempVal);
                    totalServeFeeVal += tempVal;
                }
                else
                {
                    memset(&pAPOrder->periodInfo[index], 0x00, sizeof(MSNvmAPPeriodTradeInfo_Struct));
                }
            }

            /*正常情况 diff <= (9*periodCount)/10 < periodCount*/
            if ((targetEnergyVal > totalEnergyVal) && ((targetEnergyVal - totalEnergyVal) <= periodCount)) 
            {
                deltaEnergyVal = targetEnergyVal - totalEnergyVal;
                while (deltaEnergyVal > 0)
                {
                    selectedFlag = FALSE;
                    selectedIndex = 0;
                    maxRemainder = 0;

                    for (adjustIndex = 0; adjustIndex < periodCount; adjustIndex++)
                    {
                        if ((periodEnergyAdjusted[adjustIndex] == FALSE) &&
                            ((selectedFlag == FALSE) || (periodEnergyRemainder[adjustIndex] > maxRemainder)))
                        {
                            selectedFlag = TRUE;
                            selectedIndex = adjustIndex;
                            maxRemainder = periodEnergyRemainder[adjustIndex];
                        }
                    }

                    if (selectedFlag == TRUE)
                    {
                        periodEnergyVal[selectedIndex]++;
                        periodEnergyAdjusted[selectedIndex] = TRUE;
                        totalEnergyVal++;
                        deltaEnergyVal--;
                        IOTAP_CFG_DebugPrint("AP订单电量调整: period[%d]增加1, 当前总电量=%d, 剩余需调整=%d\r\n",
                            selectedIndex, totalEnergyVal, deltaEnergyVal);
                    }
                    else
                    {
                        deltaEnergyVal = 0;
                    }
                }
            }

            for (index = 0; index < periodCount; index++)
            {
                Common_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeEnergy, periodEnergyVal[index]);
            }

            Common_Uint32ToThreeUint8(pAPOrder->totalEnergy, totalEnergyVal);
            Common_Uint32ToThreeUint8(pAPOrder->totalElecFee, totalElecFeeVal);
            Common_Uint32ToThreeUint8(pAPOrder->totalServeFee, totalServeFeeVal);
            Common_Uint32ToFourUint8(pAPOrder->startMeterVal, startMeterVal);
            Common_Uint32ToFourUint8(pAPOrder->stopMeterVal, stopMeterVal);

            if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
            {
                IotAP_StopReasonToBcd(pAPOrder->stopReason, IotAP_ConverStopReason(pChargeData->eChargeStopReason));
            }
        }
    }
}

void IotAP_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmAPOrderInfo_Struct *pOrderData = NULL;
    uint8_t *pBuf = pProtocolRecord;
    uint16_t dataLen = 0;
    uint8_t index = 0;
    uint8_t periodCount = 0;

    if ((pFlashRecord != NULL) && (pProtocolRecord != NULL) && (pRecordLen != NULL))
    {
        pOrderData = &pFlashRecord->stAPOrderInfo;
        periodCount = pOrderData->periodCount;
        if (periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT)
        {
            periodCount = MSNVM_AP_BILLMODE_PERIOD_COUNT;
        }

        for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
        {
            pBuf[dataLen++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1 - index];
        }

        pBuf[dataLen++] = pOrderData->port;
        memcpy(&pBuf[dataLen], pOrderData->orderTransactionNum, sizeof(pOrderData->orderTransactionNum));
        dataLen += sizeof(pOrderData->orderTransactionNum);
        pBuf[dataLen++] = periodCount;

        for (index = 0; index < periodCount; index++)
        {
            pBuf[dataLen++] = pOrderData->periodInfo[index].timeSerialNumber;
            pBuf[dataLen++] = pOrderData->periodInfo[index].timeKind;
            memcpy(&pBuf[dataLen], pOrderData->periodInfo[index].chargeEnergy, sizeof(pOrderData->periodInfo[index].chargeEnergy));
            dataLen += sizeof(pOrderData->periodInfo[index].chargeEnergy);
            memcpy(&pBuf[dataLen], pOrderData->periodInfo[index].chargeElecFee, sizeof(pOrderData->periodInfo[index].chargeElecFee));
            dataLen += sizeof(pOrderData->periodInfo[index].chargeElecFee);
            memcpy(&pBuf[dataLen], pOrderData->periodInfo[index].chargeServeFee, sizeof(pOrderData->periodInfo[index].chargeServeFee));
            dataLen += sizeof(pOrderData->periodInfo[index].chargeServeFee);
        }

        memcpy(&pBuf[dataLen], pOrderData->startTime, sizeof(pOrderData->startTime));
        dataLen += sizeof(pOrderData->startTime);
        memcpy(&pBuf[dataLen], pOrderData->stopTime, sizeof(pOrderData->stopTime));
        dataLen += sizeof(pOrderData->stopTime);
        memcpy(&pBuf[dataLen], pOrderData->chargeTimeMin, sizeof(pOrderData->chargeTimeMin));
        dataLen += sizeof(pOrderData->chargeTimeMin);
        memcpy(&pBuf[dataLen], pOrderData->totalElecFee, sizeof(pOrderData->totalElecFee));
        dataLen += sizeof(pOrderData->totalElecFee);
        memcpy(&pBuf[dataLen], pOrderData->totalServeFee, sizeof(pOrderData->totalServeFee));
        dataLen += sizeof(pOrderData->totalServeFee);
        memcpy(&pBuf[dataLen], pOrderData->totalEnergy, sizeof(pOrderData->totalEnergy));
        dataLen += sizeof(pOrderData->totalEnergy);
        memcpy(&pBuf[dataLen], pOrderData->startMeterVal, sizeof(pOrderData->startMeterVal));
        dataLen += sizeof(pOrderData->startMeterVal);
        memcpy(&pBuf[dataLen], pOrderData->stopMeterVal, sizeof(pOrderData->stopMeterVal));
        dataLen += sizeof(pOrderData->stopMeterVal);
        memcpy(&pBuf[dataLen], pOrderData->startSoc, sizeof(pOrderData->startSoc));
        dataLen += sizeof(pOrderData->startSoc);
        memcpy(&pBuf[dataLen], pOrderData->stopSoc, sizeof(pOrderData->stopSoc));
        dataLen += sizeof(pOrderData->stopSoc);
        memcpy(&pBuf[dataLen], pOrderData->logicCardNum, sizeof(pOrderData->logicCardNum));
        dataLen += sizeof(pOrderData->logicCardNum);
        memcpy(&pBuf[dataLen], pOrderData->vin, sizeof(pOrderData->vin));
        dataLen += sizeof(pOrderData->vin);
        memcpy(&pBuf[dataLen], pOrderData->stopReason, sizeof(pOrderData->stopReason));
        dataLen += sizeof(pOrderData->stopReason);

        pRecordLen[0] = dataLen;
    }
}

uint8_t IotAP_SwipCardCharge(uint8_t port, uint8_t *pCardID)
{
    uint8_t ret = FALSE;

    if ((pIotAPCtx->loginSucc == TRUE) && (port < SYSCFG_CFG_GUN_NUM))
    {
        if ((Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP) != TRUE) &&
            (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B07_CARD_AUTH_DOWN) != TRUE) &&
            (Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B10_START_NOTIFY_UP) != TRUE) &&
            (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B11_START_NOTIFY_DOWN) != TRUE))
        {
            IotAP_ClearSwipCardCtrl(port);
            pIotAPCtx->stProtoData[port].cardAuthResult = 0;
            memset(pIotAPCtx->stProtoData[port].cardAuthFailReason, 0x00,
                   sizeof(pIotAPCtx->stProtoData[port].cardAuthFailReason));
            pIotAPCtx->stProtoData[port].cardAccountBalance = 0;
            memset(pIotAPCtx->stProtoData[port].cardVin, 0x00,
                   sizeof(pIotAPCtx->stProtoData[port].cardVin));
            pIotAPCtx->stProtoData[port].startNotifyResult = 0;
            memset(pIotAPCtx->stProtoData[port].startNotifyFailReason, 0x00,
                   sizeof(pIotAPCtx->stProtoData[port].startNotifyFailReason));
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP, TRUE);
            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP, TRUE);
            ret = TRUE;
        }
    }

    return ret;
}

static void IotAP_ClearSwipCardCtrl(uint8_t port)
{
    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);
    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);
    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);
    Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B07_CARD_AUTH_DOWN, FALSE);
    Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B07_CARD_AUTH_DOWN);

    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
    Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B11_START_NOTIFY_DOWN, FALSE);
    Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B11_START_NOTIFY_DOWN);
}

/* ====== 实时数据周期上报检测 ====== */

uint8_t IotAP_GetGunState(uint8_t port)
{
    uint8_t gunState = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (AswErrHandle_IsExsistError(port) == TRUE)
        {
            gunState = 0x01;  /* 故障 */
        }
        else if (AswMonitor_IsOrderIdle(port) != TRUE)
        {
            gunState = 0x03;  /* 充电中 */
        }
        else
        {
            gunState = 0x02;  /* 空闲 */
        }
    }

    return gunState;
}

const MSNvmAPParamBillMode_Struct *IotAP_GetActiveBillMode(uint8_t port)
{
    const MSNvmAPParamBillMode_Struct *pBillMode = NULL;
    uint8_t activeIndex = IOTAP_B47_INDEX_INVALID;


    activeIndex = pIotAPCtx->billActiveIndex[port];
    if (IOTAP_IS_VALID_BILL_INDEX(activeIndex))
    {
        pBillMode = &pIotAPCtx->stBillModeSave[port].billModeData[activeIndex];
    }

    return pBillMode;
}

/**
 * @brief  周期性上报实时监测数据
 *
 * 遍历所有充电枪，当检测到枪状态变化、插拔枪状态变化或上报周期超时时，
 * 触发B01实时监测数据上报。充电中与空闲状态使用不同的上报周期。
 */
static void IotAP_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotAP_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        if (pIotAPCtx->lastGunState[port] != curGunState)
        {
            IOTAP_CFG_DebugPrint("[AP] 端口%d 枪状态变化: %d->%d \r\n",
                         port, pIotAPCtx->lastGunState[port], curGunState);
            realDataReportFlag = TRUE;
        }

        if (pIotAPCtx->lastGunConnectState[port] != curGunConnectState)
        {
            IOTAP_CFG_DebugPrint("[AP] 端口%d 连接状态变化: %d->%d \r\n",
                         port, pIotAPCtx->lastGunConnectState[port], curGunConnectState);
            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTAP_CFG_CHARGING_REALDATA_CYCLE : IOTAP_CFG_IDLE_REALDATA_CYCLE;

        if (Common_JudgeTimeoutMs(pIotAPCtx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == TRUE)
        {
            realDataReportFlag = FALSE;
            pIotAPCtx->lastGunState[port] = curGunState;
            pIotAPCtx->lastGunConnectState[port] = curGunConnectState;
            pIotAPCtx->realDataReportTick[port] = Common_GetSystick();
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B01_REALTIME_DATA, TRUE);
            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B01_REALTIME_DATA, TRUE);
        }
    }
}

/**
 * @brief  周期性上报充电功率状态（B57命令）
 *
 * 按协议规定"从每分钟第0秒起按配置间隔上报"：
 * 取系统运行秒数 % 60 得到当前分钟内的秒数，
 * 当 secInMin % powerCtrlReportCycle == 0 且与上一上报秒不同时触发上报。
 *
 * @note   协议要求 powerCtrlReportCycle > 0，为0时跳过不触发。
 */
static void IotAP_CycleReportPowerStatus(void)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint32_t curSec    = 0;
    uint32_t secInMin  = 0;
    uint16_t periodSec = 0;
    uint8_t  port      = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pProtoData = &pIotAPCtx->stProtoData[port];

        if ((pProtoData->powerCtrlB57Enable == TRUE) &&
            (IotAP_GetGunState(port) == 0x03))  /* 充电中 */
        {
            periodSec = pProtoData->powerCtrlReportCycle;
            if (periodSec == 0)
            {
                continue;  /* 协议要求大于0，非法值不触发 */
            }

            curSec   = SSTM_GetSecTimestamp();
            secInMin = curSec % 60;

            if ((secInMin % periodSec == 0) &&
                (secInMin != pProtoData->powerCtrlStatusTick))
            {
                pProtoData->powerCtrlStatusTick = secInMin;
                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, TRUE);
                Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, TRUE);
            }
        }
        else
        {
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
            Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
        }
    }
}

/**
 * @brief  周期性轮询升级结果，当检测到升级完成时更新状态并触发B24上报
 * @note   升级为整桩固件级操作(不区分枪号)，全局查询一次SSU结果；
 *         成功时upgradeResult置0，失败置1，并立即使能B24上行发送(port 0)
 */
static void IotAP_CycleReportUpgradeResult(void)
{
    SSUcmResult_Enum ucmResult = eSSUcmResult_None;

    if (pIotAPCtx->upgradeOngoing == TRUE)
    {
        ucmResult = SSUcm_GetResult();

        if (ucmResult != eSSUcmResult_None)
        {
            pIotAPCtx->upgradeOngoing = FALSE;
            pIotAPCtx->upgradeResult = (ucmResult == eSSUcmResult_Succ) ? 0 : 1;
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_B24_UPGRADE_RESULT, TRUE);
            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_B24_UPGRADE_RESULT, TRUE);
        }
    }
}

/******************************************************************************
*    B47三缓冲计费模型管理函数实现
******************************************************************************/

/**
 * @brief  在指定枪的A/B/C计费模型中搜索匹配billModeID的组索引
 * @note   逐组比对billModeID[8]直到找到完全一致的那组, 按A→B→C顺序搜索
 *         对应旧代码 Serach_billingmodel_ID() 函数
 * @param[in]  port       枪号
 * @param[in] pSearchID   待搜索的计费模型ID(8字节)
 * @return 命中的组索引(0x4700+IOTAP_B47_A/B/C), 未命中返回0xFFFF
 */
uint16_t IotAP_SearchBillModeID(uint8_t port, const uint8_t *pSearchID)
{
    uint16_t ret = 0xFFFF;  /* 默认未找到 */

    if ((port < SYSCFG_CFG_GUN_NUM) && (pSearchID != NULL))
    {
        uint8_t group = 0;
        uint8_t i = 0;
        uint8_t match = 1;

        for (group = 0; group < MSNVM_AP_BILLMODE_MAX_NUM; group++)
        {
            match = 1;

            for (i = 0; i < 8; i++)
            {
                if (pSearchID[i] != pIotAPCtx->stBillModeSave[port].billModeData[group].billModeID[i])
                {
                    match = 0;
                    break;
                }
            }

            if (match == 1)
            {
                ret = (uint16_t)(0x4700 + group);
                break;
            }
        }
    }

    return ret;
}

/**

 * @brief  深度比较两个B47计费模型的所有字段内容是否完全相同
 * @note   逐一比对: billModeID(8B) + switchTime(7B) + invalidTime(7B) +
 *         workState(2B) + periodCount + N个period字段(periodSerial/periodRate/
 *         startTime/stopTime/elecPrice/servePrice)。对应旧代码 Comapare_content_billingModel()
 * @param[in] pA  计费模型A指针
 * @param[in] pB  计费模型B指针
 * @return 1=内容完全相同, 0=存在差异或入参无效
 */
uint8_t IotAP_CompareContentBillMode(const MSNvmAPParamBillMode_Struct *pA,
                                     const MSNvmAPParamBillMode_Struct *pB)
{
    uint8_t ret = 0;  /* 默认不同 */
    uint8_t i = 0;
    const MSNvmAPParamBillPeriod_Struct *pPeriodA = NULL;
    const MSNvmAPParamBillPeriod_Struct *pPeriodB = NULL;

    if ((pA != NULL) && (pB != NULL))
    {
        /* 比对头部固定字段(含periodCount) - 到period数组前均为连续uint8_t无填充, 共25字节 */
        if (memcmp(pA, pB, offsetof(MSNvmAPParamBillMode_Struct, period)) == 0)
        {
            ret = 1;

            /* 逐个时段深度比对 (防御性限制i不超过数组容量, 防止periodCount异常越界) */
            for (i = 0; (i < pA->periodCount) && (i < MSNVM_AP_BILLMODE_PERIOD_COUNT); i++)
            {
                pPeriodA = &pA->period[i];
                pPeriodB = &pB->period[i];

                /* 整段比较单个时段(14字节连续无填充, 含elecPrice/servePrice数组内容) */
                if (memcmp(pPeriodA, pPeriodB, sizeof(MSNvmAPParamBillPeriod_Struct)) != 0)
                {
                    ret = 0;
                    break;
                }
            }
        }
    }

    return ret;
}

/**
 * @brief  校验B47计费模型的合法性
 * @note   检查项: 时段数范围[1,12]、切换时间的年/月/日/时/分合法性。
 *         对应旧代码 ANPEI_Is_FeeModel_Valid()
 * @param[in] pBillMode  待校验的计费模型指针
 * @return 1=合法有效, 0=非法无效
 */
uint8_t IotAP_IsFeeModelValid(const MSNvmAPParamBillMode_Struct *pBillMode)
{
    uint8_t ret = 0;  /* 默认无效 */
    uint8_t minute = 0;
    uint8_t hour = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    uint8_t year = 0;

    if (pBillMode != NULL)
    {
        /* CP56Time2a格式: msL msH 分 时 日 月 年, 时间字节含标志位需掩码提取(与Asw_IotProtoAPRecv.c解析一致) */
        minute = pBillMode->switchTime[2] & 0x3F;   /* 分: bit7=IV, bit6保留 */
        hour   = pBillMode->switchTime[3] & 0x1F;   /* 时: bit7=SU, bit5-6保留 */
        day    = pBillMode->switchTime[4] & 0x1F;   /* 日: bit5-6=星期, bit7=DOWF */
        month  = pBillMode->switchTime[5] & 0x0F;   /* 月: bit4=WDF */
        year   = pBillMode->switchTime[6] & 0x7F;   /* 年: bit7保留, 0~99 */

        /* 校验时段数量: 必须在[1, 12]范围内 */
        if ((pBillMode->periodCount < 1) || (pBillMode->periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT))
        {
            /* ret保持0 → 无效 */
        }
        /* 校验切换时间各字段的合理性(CP56Time2a格式: msL msH 分 时 日 月 年) */
        else if (year > 99)                              /* 年份上限2099 */
        {
            
        }
        else if ((month > 12) || (month == 0))           /* 月: 1~12 */                 
        {

        }
        else if ((day > 31) ||  (day == 0))              /* 日: 1~31 */
        {

        }
        else if (hour > 23)                              /* 时: 0~23 */
        {

        }
        else if (minute > 59)                            /* 分: 0~59 */
        {

        }
        else
        {
            ret = 1;  /* 所有检查通过 → 有效 */
        }
    }

    return ret;
}

/**
 * @brief  保存B47计费模型到多缓冲区并持久化Flash
 * @note   完整流程(对应旧代码 save_rateB47_model_anpei):
 *         1. 获取当前活跃索引
 *         2. 按billModeID在全部槽中搜索是否已存在同ID模型
 *         3. 若同ID→深比较内容: 相同则跳过(减少Flash擦写), 不同则覆盖
 *         4. 若新ID→优先写入无效槽, 其次写入非活跃槽
 *         5. 写入Flash持久化(NVM PlatPrivateParam块)
 *         6. 若非充电中则立即刷新活跃模型及未来切换缓存
 * @param[in] pNewMode  新收到的B47计费模型数据指针(已解析完成)
 * @param[in] port      枪号
 */
void IotAP_SaveRateB47Model(const MSNvmAPParamBillMode_Struct *pNewMode, uint8_t port)
{
    uint8_t needSkip = 0;  /* 0=正常执行, 1=提前退出(参数无效/内容相同) */

    if ((port < SYSCFG_CFG_GUN_NUM) && (pNewMode != NULL))
    {
        uint8_t nowActiveIndex = pIotAPCtx->billActiveIndex[port];       /* 当前活跃索引 */
        uint16_t hitGroup = IotAP_SearchBillModeID(port, pNewMode->billModeID);  /* 搜索同ID */
        uint8_t hitIndex = (uint8_t)(hitGroup & 0xFF);

        if (IOTAP_IS_VALID_BILL_INDEX(hitIndex))    /* ====== 场景1: 已存在同ID → 深比较内容 ====== */
        {            
            if (IotAP_CompareContentBillMode(pNewMode,
                    &pIotAPCtx->stBillModeSave[port].billModeData[hitIndex]) == 0)
            {
                /* 同ID但内容不同 → 覆盖该组数据 */
                memcpy(&pIotAPCtx->stBillModeSave[port].billModeData[hitIndex],
                       pNewMode, sizeof(MSNvmAPParamBillMode_Struct));
                pIotAPCtx->stBillModeSave[port].recentUpdateIndex = hitIndex;
                IOTAP_CFG_InfoPrint("AP,B47保存: 同ID不同内容,覆盖[%d]组\r\n", hitIndex);
            }
            else
            {
                /* 同ID且内容完全相同 → 跳过,避免不必要的Flash擦写 */
                IOTAP_CFG_InfoPrint("AP,B47保存: 内容相同,跳过Flash写入\r\n");
                needSkip = 1;  /* 标记跳过后续Flash写入和刷新 */
            }
        }
        else    /* ====== 场景2: 全新ID → 选择最优写入槽 ====== */
        {
            uint8_t writeIndex = IOTAP_B47_A;  /* 默认A */
            uint8_t foundSlot = 0;
            uint8_t group = 0;

            /* 第1优先级: 寻找无效空槽; 无活跃模型不代表缓存中没有未来模型 */
            for (group = 0; group < MSNVM_AP_BILLMODE_MAX_NUM; group++)
            {
                if ((group != nowActiveIndex) &&
                    (IotAP_IsFeeModelValid(&pIotAPCtx->stBillModeSave[port].billModeData[group]) == 0))
                {
                    writeIndex = group;
                    foundSlot = 1;
                    break;
                }
            }

            /* 第2优先级: 所有槽都有效 → 从最近更新槽的下一槽开始轮换,并保护活跃槽 */
            if (foundSlot == 0)
            {
                uint8_t offset = 0;

                for (offset = 1; offset <= MSNVM_AP_BILLMODE_MAX_NUM; offset++)
                {
                    group = (uint8_t)((pIotAPCtx->stBillModeSave[port].recentUpdateIndex + offset) %
                                      MSNVM_AP_BILLMODE_MAX_NUM);
                    if (group != nowActiveIndex)
                    {
                        writeIndex = group;
                        break;
                    }
                }
            }

            memcpy(&pIotAPCtx->stBillModeSave[port].billModeData[writeIndex],
                   pNewMode, sizeof(MSNvmAPParamBillMode_Struct));
            pIotAPCtx->stBillModeSave[port].recentUpdateIndex = writeIndex;
            IOTAP_CFG_InfoPrint("AP,B47保存: 新ID写入[%d]组(当前活跃=%d)\r\n",
                                 writeIndex, nowActiveIndex);
        }

        /* ====== 将多缓冲数据同步到NVM平台私有参数区域 ====== */
        if (needSkip == 0)
        {
            MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
            if (pPrivateParam != NULL)
            {
                memcpy(&pPrivateParam->stAPParam.stBillModeSave[port],
                       &pIotAPCtx->stBillModeSave[port],
                       sizeof(IotAPBillModeSave_Struct));
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                                     (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                IOTAP_CFG_InfoPrint("AP,B47保存: Flash持久化完成\r\n");
            }

            /* ====== B47写入完成, 触发刷新 + 更新唤醒缓存 ====== */

            /* 如果该枪不在充电中, 立即刷新当前活跃计费模型 */
            if (AswMonitor_IsOrderIdle(port) == TRUE)
            {
                IotAP_RefreshNowbillModel(port);
            }
        }
    }
    /* 参数无效 → needSkip保持0, 直接返回 */
}

/**
 * @brief  刷新指定枪当前应使用的活跃计费模型(N槽通用决策引擎)
 * @note   对应旧代码 Refresh_NowbillModel(), 升级为通用多槽决策:
 *
 *         Step1: 逐槽校验有效性并转换switchTime
 *         Step2: 仅从switchTime <= 当前时间的模型中选择切换时间最大的一套
 *         Step3: 更新全局活跃索引;所有switchTime均在未来时置为无效,
 *                确保到达切换时间前计费模型校验不通过、不能充电
 *         Step4: 缓存最早未来切换时间,供周期任务到点后重新刷新
 * @param[in] port  枪号
 * @return 无
 */
void IotAP_RefreshNowbillModel(uint8_t port)
{
    uint8_t validCount = 0;
    uint8_t selectedIdx = IOTAP_B47_INDEX_INVALID;

    uint8_t validFlag[MSNVM_AP_BILLMODE_MAX_NUM];
    uint32_t switchSec[MSNVM_AP_BILLMODE_MAX_NUM];
    uint8_t group = 0;
    uint32_t secNow = SSTM_GetSecTimestamp();
    uint32_t latestPastSec = 0;
    uint8_t foundPast = 0;

    /* ====== Step1: 逐槽校验有效性 ====== */
    for (group = 0; group < MSNVM_AP_BILLMODE_MAX_NUM; group++)
    {
        validFlag[group] = IotAP_IsFeeModelValid(&pIotAPCtx->stBillModeSave[port].billModeData[group]);
        if (validFlag[group] == 1)
        {
            validCount++;
            switchSec[group] = Common_Cp56Time2aToTimestamp(
                pIotAPCtx->stBillModeSave[port].billModeData[group].switchTime);
        }
        else
        {
            switchSec[group] = 0;
        }
    }

    /* ====== Step2: 仅选择已到达切换时间的模型 ====== */
    if (validCount == 0)
    {
        IOTAP_CFG_InfoPrint("AP,B47刷新: port%d 无有效费率表\r\n", port);
    }
    else
    {
        /* 找最近已生效的切换时间(switchTime <= now 中最大的) */
        for (group = 0; group < MSNVM_AP_BILLMODE_MAX_NUM; group++)
        {
            if ((validFlag[group] == 1) && (switchSec[group] <= secNow))    /* 执行状态有效 */
            {
                if ((foundPast == 0) || (switchSec[group] >= latestPastSec))
                {
                    latestPastSec = switchSec[group];
                    selectedIdx = group;
                    foundPast = 1;
                }
            }
        }
    }

    /* ====== Step3: 更新全局活跃索引 ====== */
    {
        uint8_t oldIdx = pIotAPCtx->billActiveIndex[port];

        /* 没有已到切换时间的模型时，清空旧索引，禁止未来费率提前生效 */
        pIotAPCtx->billActiveIndex[port] = selectedIdx;

        /* 仅在计费模型组真正切换时才打印日志，避免B49/B52轮询时刷屏 */
        if ((foundPast == 1) && (selectedIdx != oldIdx))
        {
            IOTAP_CFG_InfoPrint("AP,B47刷新: port%d 选择%d组(ID:%02X %02X %02X %02X %02X %02X %02X %02X...)\r\n",
                                    port, selectedIdx,
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[0],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[1],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[2],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[3],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[4],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[5],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[6],
                                    pIotAPCtx->stBillModeSave[port].billModeData[selectedIdx].billModeID[7]);
        }
        else if ((foundPast == 0) && (oldIdx != IOTAP_B47_INDEX_INVALID))
        {
            IOTAP_CFG_InfoPrint("AP,B47刷新: port%d 无当前可生效计费模型\r\n", port);
        }
    }

    /* ====== Step4: 缓存下次最早切换唤醒时间 ====== */
    {
        uint32_t nextSec = 0;
        uint8_t group;

        for (group = 0; group < MSNVM_AP_BILLMODE_MAX_NUM; group++)
        {
            if ((validFlag[group] == 1) && (switchSec[group] > secNow)) /*未来生效的有效计费模型*/
            {
                if ((nextSec == 0) || (switchSec[group] < nextSec)) /*取未来生效时间的最小值*/
                {
                    nextSec = switchSec[group];
                }
            }
        }
        pIotAPCtx->nextSwitchSec[port] = nextSec;
    }
    /* 参数无效(port越界) → 直接退出 */
}

/**
 * @brief  启动时从NVM恢复B47多缓冲计费模型数据
 * @note   上电初始化阶段调用,逐枪完成以下4步:
 *
 *         Step 1 — 全局RAM清零: pIotAPCtx->stBillModeSave[] / pIotAPCtx->billActiveIndex[] / pIotAPCtx->nextSwitchSec[]
 *         Step 2 — 从NVM读取多缓冲: stAPParam.stBillModeSave[port] → pIotAPCtx->stBillModeSave[port]
 *         Step 3 — recentUpdateIndex合法性修正: 若不在[0,2]范围内则默认回退到A组(防NVM损坏)
 *         Step 4 — 逐枪触发 RefreshNowbillModel() 决策引擎,确定当前应使用的活跃费率
 *
 *         对应旧代码 Read_rateB47_model_anpei()
 */
void IotAP_ReadRateB47ModelFromNVM(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint8_t port = 0;

    /* ====== Step 1: 全局RAM变量防御性清零 ====== */
    /* 无论NVM读取是否成功,先清零确保无脏数据 */
    memset(pIotAPCtx->stBillModeSave, 0x00, sizeof(pIotAPCtx->stBillModeSave));
    memset(pIotAPCtx->billActiveIndex, IOTAP_B47_INDEX_INVALID, sizeof(pIotAPCtx->billActiveIndex));
    memset(pIotAPCtx->nextSwitchSec, 0x00, sizeof(pIotAPCtx->nextSwitchSec));

    /* ====== Step 2~4: 逐枪从NVM恢复并触发决策 ====== */
    if (pPrivateParam != NULL)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            /* Step 2: 从NVM读取该枪的IotAPBillModeSave_Struct到RAM */
            memcpy(&pIotAPCtx->stBillModeSave[port],
                &pPrivateParam->stAPParam.stBillModeSave[port],
                sizeof(IotAPBillModeSave_Struct));

            /* Step 3: recentUpdateIndex合法性修正(防NVM损坏:非法值默认回退A组) */
            if (!IOTAP_IS_VALID_BILL_INDEX(pIotAPCtx->stBillModeSave[port].recentUpdateIndex))
            {
                pIotAPCtx->stBillModeSave[port].recentUpdateIndex = IOTAP_B47_A;
            }

            /* Step 4: 触发决策引擎,确定当前活跃计费模型 */
            IotAP_RefreshNowbillModel(port);
        }
    }
    IOTAP_CFG_InfoPrint("AP,B47启动恢复: 多缓冲区已初始化\r\n");
}

void IotAP_MainFunction(void)
{
    if (pIotAPCtx != NULL)
    {
        switch (pIotAPCtx->eWorkState)
        {
            case eIotAPWorkState_Init:
            {
                IotAP_WSInitHandle();
                break;
            }
            case eIotAPWorkState_Offline:
            {
                IotAP_WSOfflineHandle();
                break;
            }
            case eIotAPWorkState_Login:
            {
                IotAP_WSLoginHandle();
                break;
            }
            case eIotAPWorkState_Normal:
            {
                IotAP_WSNormalHandle();
                break;
            }
            default:
            {
                pIotAPCtx->eWorkState = eIotAPWorkState_Init;
            }
        }
    }
}
