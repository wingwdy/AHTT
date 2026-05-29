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
*2026/05/21     V1.0.0       WDY        初版创建 - 骨架代码
*2026/05/22     V1.1.0       WDY        实现FillLinkPara/InitMemory/MainFunction FSM骨架
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
#include "MS_Nvm.h"
#include "myMalloc.h"
#include "SS_Tm.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_AP_B53_MAX_SEND_COUNT               (10U)


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

/* B47双缓冲计费模型全局变量(每枪独立一套A/B) */
IotAPBillModeSave_Struct g_stIotAPBillModeSave[SYSCFG_CFG_GUN_NUM];
uint8_t g_iotapBillActiveIndex[SYSCFG_CFG_GUN_NUM];     /* 当前活跃的A/B索引, 初始值0xFF=无效 */
uint8_t g_iotapB49SwitchFlag[SYSCFG_CFG_GUN_NUM];       /* B49上报判断标志 */


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
static void IotAP_Uint32ToThreeUint8(uint8_t *pData, uint32_t value);
static void IotAP_StopReasonToBcd(uint8_t *pData, IotAPStopReason_Enum stopReason);
static uint32_t IotAP_Cp56TimeToSeconds(const uint8_t *pCp56Time);  /* CP56Time2a转总秒数 */

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/*
 * 发送控制结构体查找函数
 * 根据 CMD 命令号返回对应端口的发送控制结构体指针
 * 索引分配(共19条):
 *  [0] F1 登录请求       [10] B34 功率控制上行
 *  [1] F3 U帧上报        [11] B38 零点示值上报
 *  [2] F5 心跳请求       [12] B40 FTP地址上行
 *  [3] F8 时钟同步应答   [13] B46 功率召测上行
 *  [4] B1 实时数据上报   [14] B48 分时计费模型上行
 *  [5] B5 启停控制结果   [15] B49 计费切换生效上行
 *  [6] B6 刷卡鉴权上行   [16] B52 计费召测上行
 *  [7] B10 启动通知上行  [17] B53 在线分时明细上传
 *  [8] B24 升级结果上报  [18] B57 功率实时状态上行
 *  [9] B31 SIM卡信息上行
 */
static CommonSendCtrl_Struct* IotAP_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    if ((pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return NULL;
    }

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
        case IOT_AP_CMD_B38_ZERO_METER_VALUE:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][11];  break;
        case IOT_AP_CMD_B40_FTP_ADDR_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][12];  break;
        case IOT_AP_CMD_B46_POWER_POLL_UP:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][13];  break;
        case IOT_AP_CMD_B48_TIMEBILL_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][14];  break;
        case IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP:     pSendCtrl = &pIotAPCtx->stSendCtrl[port][15];  break;
        case IOT_AP_CMD_B52_TIMEBILL_POLL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][16];  break;
        case IOT_AP_CMD_B53_ONLINE_DETAIL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][17];  break;
        case IOT_AP_CMD_B57_POWER_STATUS_UP:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][18];  break;

        default: break;
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

    if ((pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return NULL;
    }

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

    return pRecvCtrl;
}

/* ====== 初始化状态处理 ====== */
static void IotAP_WSInitHandle(void)
{
    pIotAPCtx->eWorkState = eIotAPWorkState_Offline;
}

/* ====== 离线状态处理 ====== */
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

/* ====== 登录状态处理 ====== */
static void IotAP_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotAPCtx->eWorkState = eIotAPWorkState_Normal;
        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_LOGIN_REQ, TRUE);
    }
}

/* ====== 正常工作状态处理 ====== */
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
            IotAP_CycleReportRealData();
            IotAP_CycleDetectUnreportedRecord();
        }

        /* 上行发送控制处理：遍历发送控制表，组帧并写入FrameQueue */
        IotAP_UpCtrlSendDeal();

        /* 上行接收解析处理：从FrameQueue取帧、校验、识别CMD并分发解析函数 */
        IotAP_UpCtrlRecvDeal();

        IotAP_TimeoutDetect();
    }
}

static void IotAP_Uint32ToThreeUint8(uint8_t *pData, uint32_t value)
{
    if (pData != NULL)
    {
        pData[0] = (uint8_t)(value & 0xFFU);
        pData[1] = (uint8_t)((value >> 8U) & 0xFFU);
        pData[2] = (uint8_t)((value >> 16U) & 0xFFU);
    }
}

static void IotAP_StopReasonToBcd(uint8_t *pData, IotAPStopReason_Enum stopReason)
{
    uint16_t reason = (uint16_t)stopReason;

    if (pData != NULL)
    {
        pData[0] = (uint8_t)((((reason / 10U) % 10U) << 4U) | (reason % 10U));
        pData[1] = (uint8_t)((((reason / 1000U) % 10U) << 4U) | ((reason / 100U) % 10U));
    }
}

static void IotAP_CycleDetectUnreportedRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if ((pIotAPCtx != NULL) && (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0U))
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
    IotAPStopReason_Enum eStopReason = eIotAPStopReason_NoExpectedErr;
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
        {eErr_InputLineReversed, eIotAPStopReason_OtherErr},
        {eErr_LeakageCurrErr,    eIotAPStopReason_LeakageCurrErr},
        {eErr_ShortCircleErr,    eIotAPStopReason_OtherErr},
        {eErr_RCDSelfcheckErr,   eIotAPStopReason_OtherErr},

        {eErr_AphaseInputOverVol, eIotAPStopReason_InputFault},
        {eErr_AphaseInputLessVol, eIotAPStopReason_InputFault},
        {eErr_OutputOverCurr,     eIotAPStopReason_InputFault},

        {eErr_JcqMaloperation,   eIotAPStopReason_JcqMaloperation},
        {eErr_JcqSynechiaFault,  eIotAPStopReason_JcqSynechiaFault},
        {eErr_HmiCommErr,        eIotAPStopReason_OtherErr},
        {eErr_ReaderCommErr,     eIotAPStopReason_OtherErr},
        {eErr_MeterCommErr,      eIotAPStopReason_MeterCommErr},
        {eErr_EnvOverTempErr,    eIotAPStopReason_TempErr},
        {eErr_GunOverTempErr,    eIotAPStopReason_TempErr},
        {eErr_POverTempErr,      eIotAPStopReason_TempErr},

        {eErr_DatabaseErr,       eIotAPStopReason_OtherErr},
        {eErr_MeterCalcErr,      eIotAPStopReason_MeterCalcErr},

        {eErr_ChgStartTimeout,   eIotAPStopReason_StartTimeout},

        {eErr_DiodeStop,         eIotAPStopReason_DiodeStop},

        {eSrc_LittleCurr,        eIotAPStopReason_OtherErr},
        {eSrc_S2BreakOff,        eIotAPStopReason_Full},
        {eSrc_AppStop,           eIotAPStopReason_AppStop},
        {eSrc_MannulStop,        eIotAPStopReason_KeyStop},
        {eSrc_CardStop,          eIotAPStopReason_KeyStop},
        {eSrc_InsuffBalance,     eIotAPStopReason_SumNoEnough},
        {eSrc_StopbyMoney,       eIotAPStopReason_SumNoEnough},
        {eSrc_StopbyTime,        eIotAPStopReason_Full},
        {eSrc_StopbyEnergy,      eIotAPStopReason_Full},
        {eErr_GunDisConn,        eIotAPStopReason_Full},
        {eErr_CPBreakOff,        eIotAPStopReason_CpFault},

        {eErr_NetNoSIMErr,       eIotAPStopReason_OtherErr},
        {eErr_PlatformOffline,   eIotAPStopReason_OtherErr},
    };

    for (index = 0; index < ARRAY_SIZE(stopReasonMap); index++)
    {
        if (errType == stopReasonMap[index].errType)
        {
            eStopReason = stopReasonMap[index].stopReason;
            findFlag = TRUE;
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

void IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotAPCtx != NULL)
    {
        strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pParam->platMainIp) - 1);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, IOT_AP_TXRX_BUFFER_SIZE, IOT_AP_TXRX_BUFFER_SIZE,
                         &pIotAPCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotAPCtx->frameQueueChannelID;
    }
}

void IotAP_InitMemory(void)
{
    pIotAPCtx = (IotAPCtx_Struct *)myMalloc(sizeof(IotAPCtx_Struct));

    if (pIotAPCtx == NULL)
    {
        return;
    }

    memset(pIotAPCtx, 0, sizeof(IotAPCtx_Struct));
    pIotAPCtx->pFuncSendCtrl = IotAP_GetSendCtrl;
    pIotAPCtx->pFuncRecvCtrl = IotAP_GetRecvCtrl;
    IotAP_ReadRateB47ModelFromNVM();
}

void IotAP_MainFunction(void)
{
    if (pIotAPCtx == NULL)
    {
        return;
    }

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

/**
 * @brief  将AP平台计费模式参数转换为标准计费模式结构体
 * @note   从平台私有参数中提取计费模式信息，将BCD编码的时间转换为BIN格式，
 *         并填充到标准计费模式结构体中。仅当AP计费模式有效
 *         （时段数合法、工作状态为[0,1]）时才进行转换，否则输出结构体保持清零状态。
 * @param[in]  port              枪号（当前未使用，预留接口）
 * @param[out] pStandardBillMode 标准计费模式结构体指针，转换结果写入此结构体；
 *                               若为NULL则不执行任何操作；调用前结构体内容会被清零
 * @return 无
 */
void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmAPParamBillMode_Struct *pAPBillMode = NULL;
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
            activeIndex = g_iotapBillActiveIndex[port];
            if ((activeIndex != IOTAP_B47_A) && (activeIndex != IOTAP_B47_B))
            {
                IotAP_RefreshNowbillModel(port);
                activeIndex = g_iotapBillActiveIndex[port];
            }

            if ((activeIndex == IOTAP_B47_A) || (activeIndex == IOTAP_B47_B))
            {
                pAPBillMode = &g_stIotAPBillModeSave[port].billModeData[activeIndex];
                if (((pAPBillMode->workState[0] == 0U) && (pAPBillMode->workState[1] == 1U)) ||
                    ((pAPBillMode->workState[0] == 1U) && (pAPBillMode->workState[1] == 0U)))
                {
                    workStateValid = TRUE;
                }
            }
        }

        if ((pAPBillMode != NULL) &&
            (pAPBillMode->periodCount > 0U) &&
            (pAPBillMode->periodCount <= MSNVM_AP_BILLMODE_PERIOD_COUNT) &&
            (workStateValid == TRUE))
        {
            memcpy(pStandardBillMode->billModeID, pAPBillMode->billModeID, sizeof(pStandardBillMode->billModeID));
            pStandardBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_MULT;
            pStandardBillMode->periodCount = pAPBillMode->periodCount;

            for (index = 0; index < pAPBillMode->periodCount; index++)
            {
                rateIndex = index;

                if (rateIndex < ASWMONITOR_BILLMODE_RATE_COUNT)
                {
                    Common_BCDToBIN(pAPBillMode->period[index].startTime, startTime, sizeof(startTime));
                    Common_BCDToBIN(pAPBillMode->period[index].stopTime, stopTime, sizeof(stopTime));

                    pStandardBillMode->periodRate[index] = rateIndex;
                    pStandardBillMode->startTime[index][0] = startTime[0];
                    pStandardBillMode->startTime[index][1] = startTime[1];
                    pStandardBillMode->stopTime[index][0] = stopTime[0];
                    pStandardBillMode->stopTime[index][1] = stopTime[1];

                    pStandardBillMode->rateElecPrice[rateIndex] = pAPBillMode->period[index].elecPrice;
                    pStandardBillMode->rateSeverPrice[rateIndex] = pAPBillMode->period[index].servePrice;
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
    MSNvmAPParamBillMode_Struct *pAPBillMode = NULL;
    AswMonitorBillMode_Struct *pBillMode = NULL;
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    AswMonitorChargeData_Struct *pChargeData = NULL;
    MSNvmAPOrderInfo_Struct *pAPOrder = NULL;
    uint8_t activeIndex = IOTAP_B47_INDEX_INVALID;
    uint8_t index = 0;
    uint8_t periodCount = 0;
    uint32_t tempVal = 0;

    if ((pIotAPCtx != NULL) && (pOrderData != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
        pChargeData = AswMonitor_GetChargeDataPtr(port);
        pAPOrder = &pOrderData->platOrderInfo.stAPOrderInfo;
        activeIndex = g_iotapBillActiveIndex[port];
        if ((activeIndex == IOTAP_B47_A) || (activeIndex == IOTAP_B47_B))
        {
            pAPBillMode = &g_stIotAPBillModeSave[port].billModeData[activeIndex];
        }

        if ((pBillMode != NULL) && (pChargeCtrl != NULL) && (pChargeData != NULL))
        {
            periodCount = pBillMode->periodCount;
            if (periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT)
            {
                periodCount = MSNVM_AP_BILLMODE_PERIOD_COUNT;
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
                Common_Uint32ToFourUint8(pAPOrder->startMeterVal, pChargeData->startMeterVal / 10U);
                Common_Uint32ToFourUint8(pAPOrder->stopMeterVal, pChargeData->stopMeterVal / 10U);

                if (pChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
                {
                    memcpy(pAPOrder->logicCardNum, pChargeCtrl->authCardID, sizeof(pAPOrder->logicCardNum));
                }
                else
                {
                    memcpy(pAPOrder->logicCardNum,
                           pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum,
                           sizeof(pAPOrder->logicCardNum));
                }

                pOrderData->port = port;
                pOrderData->protocolType = eAswPlatCardType_AP;
                pOrderData->orderLen = sizeof(MSNvmAPOrderInfo_Struct);
                IotAP_StopReasonToBcd(pAPOrder->stopReason, eIotAPStopReason_PowerOff);
            }

            pAPOrder->periodCount = periodCount;
            Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, pAPOrder->stopTime);
            Common_Uint16ToTwoUint8(pAPOrder->chargeTimeMin, (uint16_t)(pChargeData->chargeTime / 60U));

            for (index = 0; index < MSNVM_AP_BILLMODE_PERIOD_COUNT; index++)
            {
                if (index < periodCount)
                {
                    pAPOrder->periodInfo[index].timeSerialNumber = (uint8_t)(index + 1U);
                    if ((pAPBillMode != NULL) && (index < pAPBillMode->periodCount))
                    {
                        pAPOrder->periodInfo[index].timeKind = pAPBillMode->period[index].periodRate;
                    }
                    else
                    {
                        pAPOrder->periodInfo[index].timeKind = (uint8_t)(pBillMode->periodRate[index] + 1U);
                    }

                    tempVal = pChargeData->periodElePower[index] / 10U;
                    IotAP_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeEnergy, tempVal);
                    tempVal = pChargeData->periodEleMoney[index] / 100U;
                    IotAP_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeElecFee, tempVal);
                    tempVal = pChargeData->periodSerMoney[index] / 100U;
                    IotAP_Uint32ToThreeUint8(pAPOrder->periodInfo[index].chargeServeFee, tempVal);
                }
                else
                {
                    memset(&pAPOrder->periodInfo[index], 0x00, sizeof(MSNvmAPPeriodTradeInfo_Struct));
                }
            }

            IotAP_Uint32ToThreeUint8(pAPOrder->totalElecFee, pChargeData->totalElecMoney / 100U);
            IotAP_Uint32ToThreeUint8(pAPOrder->totalServeFee, pChargeData->totalServeMoney / 100U);
            IotAP_Uint32ToThreeUint8(pAPOrder->totalEnergy, pChargeData->totalEnergy / 10U);
            Common_Uint32ToFourUint8(pAPOrder->startMeterVal, pChargeData->startMeterVal / 10U);
            Common_Uint32ToFourUint8(pAPOrder->stopMeterVal, pChargeData->stopMeterVal / 10U);

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

    if ((pIotAPCtx != NULL) && (pFlashRecord != NULL) && (pProtocolRecord != NULL) && (pRecordLen != NULL))
    {
        pOrderData = &pFlashRecord->stAPOrderInfo;
        periodCount = pOrderData->periodCount;
        if (periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT)
        {
            periodCount = MSNVM_AP_BILLMODE_PERIOD_COUNT;
        }

        for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
        {
            pBuf[dataLen++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1U - index];
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

uint8_t IotAP_SwipCardCharge(uint8_t port)
{
    /* TODO: 刷卡启动充电处理 */
    return 0;
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
            DSLOGM_Debug(DSLogMModule_Proto, "[AP] port%d gunState chg: %d->%d \r\n",
                         port, pIotAPCtx->lastGunState[port], curGunState);
            realDataReportFlag = TRUE;
        }

        if (pIotAPCtx->lastGunConnectState[port] != curGunConnectState)
        {
            DSLOGM_Debug(DSLogMModule_Proto, "[AP] port%d connState chg: %d->%d \r\n",
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

/******************************************************************************
*    B47双缓冲计费模型管理函数实现
******************************************************************************/

/**
 * @brief  将CP56Time2a格式时间(7字节)转换为自1970-01-01以来的总秒数
 * @param[in] pCp56Time  CP56Time2a格式时间数组(msL msH 分 时 日 月 年)
 * @return 总秒数(32位), 若时间为全0则返回0
 */
static uint32_t IotAP_Cp56TimeToSeconds(const uint8_t *pCp56Time)
{
    uint32_t ret = 0U;

    if (pCp56Time != NULL)
    {
        ret = Common_Cp56Time2aToTimestamp(pCp56Time);
    }

    return ret;
}

/**
 * @brief  在指定枪的A/B两组计费模型中搜索匹配billModeID的组索引
 * @note   先比对A组全部8字节ID，若完全一致返回A组索引；
 *         否则再比对B组，一致返回B组索引；均不匹配返回0xFFFF
 *         对应旧代码 Serach_billingmodel_ID() 函数
 * @param[in]  port       枪号
 * @param[in] pSearchID   待搜索的计费模型ID(8字节)
 * @return 命中的组索引(IOTAP_B47_A=0 或 IOTAP_B47_B=1)，未命中返回0xFFFF
 */
uint16_t IotAP_SearchBillModeID(uint8_t port, const uint8_t *pSearchID)
{
    uint16_t ret = (uint16_t)(0x4700U + IOTAP_B47_A);  /* 默认先假设是A组 */

    if ((port < SYSCFG_CFG_GUN_NUM) && (pSearchID != NULL))
    {
        /* 逐字节比对A组的billModeID[8] */
        for (uint8_t i = 0; i < 8; i++)
        {
            if (pSearchID[i] != g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_A].billModeID[i])
            {
                ret = (uint16_t)(0x4700U + IOTAP_B47_B);  /* A不匹配 → 尝试B */

                /* 逐字节比对B组的billModeID[8] */
                for (uint8_t k = 0; k < 8; k++)
                {
                    if (pSearchID[k] != g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_B].billModeID[k])
                    {
                        ret = 0xFFFF;  /* A和B都不匹配 */
                        break;
                    }
                }
                break;  /* 无论B是否匹配都退出外层循环 */
            }
        }
    }
    else
    {
        ret = 0xFFFF;
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
    uint8_t ret = 0U;  /* 默认不同 */

    if ((pA != NULL) && (pB != NULL))
    {
        /* 比对头部固定字段 */
        if ((memcmp(pA->billModeID, pB->billModeID, sizeof(pA->billModeID)) == 0) &&
            (memcmp(pA->switchTime, pB->switchTime, sizeof(pA->switchTime)) == 0) &&
            (memcmp(pA->invalidTime, pB->invalidTime, sizeof(pA->invalidTime)) == 0) &&
            (memcmp(pA->workState, pB->workState, sizeof(pA->workState)) == 0) &&
            (pA->periodCount == pB->periodCount))
        {
            uint8_t allPeriodMatch = 1U;

            /* 逐个时段深度比对 */
            for (uint8_t i = 0; i < pA->periodCount; i++)
            {
                const MSNvmAPParamBillPeriod_Struct *pPeriodA = &pA->period[i];
                const MSNvmAPParamBillPeriod_Struct *pPeriodB = &pB->period[i];

                if ((pPeriodA->periodSerial != pPeriodB->periodSerial) ||
                    (pPeriodA->periodRate != pPeriodB->periodRate) ||
                    (memcmp(pPeriodA->startTime, pPeriodB->startTime, sizeof(pPeriodA->startTime)) != 0) ||
                    (memcmp(pPeriodA->stopTime, pPeriodB->stopTime, sizeof(pPeriodA->stopTime)) != 0) ||
                    (pPeriodA->elecPrice != pPeriodB->elecPrice) ||
                    (pPeriodA->servePrice != pPeriodB->servePrice))
                {
                    allPeriodMatch = 0U;
                    break;
                }
            }

            ret = allPeriodMatch;
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
    uint8_t ret = 0U;  /* 默认无效 */
    uint8_t minute = 0U;
    uint8_t hour = 0U;
    uint8_t day = 0U;
    uint8_t month = 0U;
    uint8_t year = 0U;

    if (pBillMode != NULL)
    {
        minute = pBillMode->switchTime[2] & 0x3FU;
        hour = pBillMode->switchTime[3] & 0x1FU;
        day = pBillMode->switchTime[4] & 0x1FU;
        month = pBillMode->switchTime[5] & 0x0FU;
        year = pBillMode->switchTime[6] & 0x7FU;

        /* 校验时段数量: 必须在[1, 12]范围内 */
        if ((pBillMode->periodCount < 1U) || (pBillMode->periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT))
        {
            /* ret保持0 → 无效 */
        }
        /* 校验切换时间各字段的合理性(CP56Time2a格式: msL msH 分 时 日 月 年) */
        else if (year > 99U)                              /* 年份上限2099 */
        {
            /* ret保持0 → 无效 */
        }
        else if ((month > 12U) ||                         /* 月: 1~12 */
                 (month == 0U))
        {
            /* ret保持0 → 无效 */
        }
        else if ((day > 31U) ||                           /* 日: 1~31 */
                 (day == 0U))
        {
            /* ret保持0 → 无效 */
        }
        else if (hour > 23U)                              /* 时: 0~23 */
        {
            /* ret保持0 → 无效 */
        }
        else if (minute > 59U)                            /* 分: 0~59 */
        {
            /* ret保持0 → 无效 */
        }
        else
        {
            ret = 1U;  /* 所有检查通过 → 有效 */
        }
    }

    return ret;
}

/**
 * @brief  保存B47计费模型到双缓冲区并持久化Flash
 * @note   完整流程(对应旧代码 save_rateB47_model_anpei):
 *         1. 获取当前活跃索引
 *         2. 按billModeID在A/B组中搜索是否已存在同ID模型
 *         3. 若同ID→深比较内容: 相同则跳过(减少Flash擦写), 不同则覆盖
 *         4. 若新ID→写入非活跃缓冲区(A/B交替)
 *         5. 写入Flash持久化(NVM PlatPrivateParam块)
 *         6. 设置B49上报标志; 若非充电中则立即触发Refresh刷新活跃模型
 * @param[in] pNewMode  新收到的B47计费模型数据指针(已解析完成)
 * @param[in] port      枪号
 */
void IotAP_SaveRateB47Model(const MSNvmAPParamBillMode_Struct *pNewMode, uint8_t port)
{
    uint8_t needSkip = 0U;  /* 0=正常执行, 1=提前退出(参数无效/内容相同) */

    if ((port < SYSCFG_CFG_GUN_NUM) && (pNewMode != NULL))
    {
        uint8_t nowActiveIndex = g_iotapBillActiveIndex[port];       /* 当前活跃索引 */
        uint16_t hitGroup = IotAP_SearchBillModeID(port, pNewMode->billModeID);  /* 搜索同ID */
        uint8_t hitIndex = (uint8_t)(hitGroup & 0xFFU);

        if ((hitIndex == IOTAP_B47_A) || (hitIndex == IOTAP_B47_B))
        {
            /* ====== 场景1: 已存在同ID → 深比较内容 ====== */
            if (IotAP_CompareContentBillMode(pNewMode,
                    &g_stIotAPBillModeSave[port].billModeData[hitIndex]) == 0U)
            {
                /* 同ID但内容不同 → 覆盖该组数据 */
                memset(&g_stIotAPBillModeSave[port].billModeData[hitIndex],
                       0x00, sizeof(MSNvmAPParamBillMode_Struct));
                memcpy(&g_stIotAPBillModeSave[port].billModeData[hitIndex],
                       pNewMode, sizeof(MSNvmAPParamBillMode_Struct));
                g_stIotAPBillModeSave[port].recentUpdateIndex = hitIndex;
                IOTAP_CFG_InfoPrint("AP,B47保存: 同ID不同内容,覆盖[%d]组\r\n", hitIndex);
            }
            else
            {
                /* 同ID且内容完全相同 → 跳过,避免不必要的Flash擦写 */
                IOTAP_CFG_InfoPrint("AP,B47保存: 内容相同,跳过Flash写入\r\n");
                needSkip = 1U;  /* 标记跳过后续Flash写入和刷新 */
            }
        }
        else
        {
            /* ====== 场景2: 全新ID → 写入非活跃缓冲区 ====== */
            uint8_t writeIndex = IOTAP_B47_B;  /* 默认写入B */

            if (nowActiveIndex == IOTAP_B47_INDEX_INVALID)
            {
                /* 首次无任何模型 → 清空AB两套,写入A */
                memset(&g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_A],
                       0x00, sizeof(MSNvmAPParamBillMode_Struct));
                memset(&g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_B],
                       0x00, sizeof(MSNvmAPParamBillMode_Struct));
                writeIndex = IOTAP_B47_A;
            }
            else if (nowActiveIndex == IOTAP_B47_A)
            {
                /* 当前活跃=A → 写入B(另一套) */
                writeIndex = IOTAP_B47_B;
            }
            else  /* nowActiveIndex == IOTAP_B47_B */
            {
                /* 当前活跃=B → 写入A(另一套) */
                writeIndex = IOTAP_B47_A;
            }

            memcpy(&g_stIotAPBillModeSave[port].billModeData[writeIndex],
                   pNewMode, sizeof(MSNvmAPParamBillMode_Struct));
            g_stIotAPBillModeSave[port].recentUpdateIndex = writeIndex;
            IOTAP_CFG_InfoPrint("AP,B47保存: 新ID写入[%d]组(当前活跃=%d)\r\n",
                                 writeIndex, nowActiveIndex);
        }

        /* ====== 将双缓冲数据同步到NVM平台私有参数区域 ====== */
        if (needSkip == 0U)
        {
            MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
            if (pPrivateParam != NULL)
            {
                memcpy(&pPrivateParam->stAPParam.stBillModeSave[port],
                       &g_stIotAPBillModeSave[port],
                       sizeof(IotAPBillModeSave_Struct));
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam,
                                     (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                IOTAP_CFG_InfoPrint("AP,B47保存: Flash持久化完成\r\n");
            }

            /* ====== 设置B49上报标志 + 触发刷新 ====== */
            g_iotapB49SwitchFlag[port] = 1;  /* 标记需要判断是否上报B49切换生效 */

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
 * @brief  刷新指定枪当前应使用的活跃计费模型(决策引擎)
 * @note   对应旧代码 Refresh_NowbillModel(), 完整实现以下逻辑:
 *
 *         Step1: 分别校验A/B两组的有效性(时段数+时间合法性)
 *         Step2: 根据有效性组合选择:
 *           - 双无效 → 返回FALSE(无可用费率表)
 *           - 单有效 → 直接选用唯一有效的那组
 *           - 双有效 → 进入时间窗口决策:
 *             a) 将A/B的switchTime转为总秒数进行比较
 *             b) 判断最近更新的是哪组(recentUpdateIndex):
 *                - 若更新组时间较早(倒序费率场景): 直接选更新组
 *                - 若更新组时间较晚:
 *                  - 当前时间>=更新组切换时间 → 选更新组(已到切换点)
 *                  - 当前时间<更新组切换时间 → 选另一组(未到切换点,继续用旧费率)
 *                - 两组切换时间相同 → 后下发优先(选更新组)
 *         Step3: 更新全局活跃索引 g_iotapBillActiveIndex[]
 * @param[in] port  枪号
 * @retval TRUE  成功确定活跃模型
 * @retval FALSE  无有效计费模型可供使用
 */
void IotAP_RefreshNowbillModel(uint8_t port)
{
    uint8_t validExec = 0U;   /* 0=跳过Step3(参数无效/双无效), 1=正常执行 */

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        uint8_t validFlag[2] = {0};  /* A/B各组有效性标记 */

        /* ====== Step1: 校验A/B两组计费模型的有效性 ====== */
        validFlag[IOTAP_B47_A] = IotAP_IsFeeModelValid(
            &g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_A]);
        validFlag[IOTAP_B47_B] = IotAP_IsFeeModelValid(
            &g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_B]);

        uint8_t selectedIdx = 0;  /* 最终选择的组索引 */

        /* ====== Step2: 根据有效性组合进行决策 ====== */
        if ((validFlag[IOTAP_B47_A] == 0U) && (validFlag[IOTAP_B47_B] == 0U))
        {
            /* 两组均无效 → 无可用费率表 */
            IOTAP_CFG_InfoPrint("AP,B47刷新: port%d 无有效费率表\r\n", port);
            /* validExec保持0 → 跳过Step3 */
        }
        else if ((validFlag[IOTAP_B47_A] == 1U) && (validFlag[IOTAP_B47_B] == 0U))
        {
            /* 仅A有效 → 选A */
            selectedIdx = IOTAP_B47_A;
            validExec = 1U;
        }
        else if ((validFlag[IOTAP_B47_A] == 0U) && (validFlag[IOTAP_B47_B] == 1U))
        {
            /* 仅B有效 → 选B */
            selectedIdx = IOTAP_B47_B;
            validExec = 1U;
        }
        else
        {
            /* ====== 双有效: 时间窗口决策 ====== */
            uint32_t secA = IotAP_Cp56TimeToSeconds(
                g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_A].switchTime);
            uint32_t secB = IotAP_Cp56TimeToSeconds(
                g_stIotAPBillModeSave[port].billModeData[IOTAP_B47_B].switchTime);

            /* 获取当前系统时间戳并转为CP56Time2a格式 */
            uint8_t currentTime[7] = {0};
            uint32_t secNowTimestamp = SSTM_GetSecTimestamp();
            Common_TimestampToCp56Time2a(secNowTimestamp, currentTime);
            uint32_t secNow = IotAP_Cp56TimeToSeconds(currentTime);
            uint8_t recentIdx = g_stIotAPBillModeSave[port].recentUpdateIndex;

            if (recentIdx == IOTAP_B47_A)
            {
                /* 最近更新的是A组 */
                if (secA < secB)
                {
                    /* A切换时间早于B(倒序费率场景:新下发的更早) → 选A */
                    selectedIdx = IOTAP_B47_A;
                }
                else if (secA == secB)
                {
                    /* 切换时间相同 → 后下发优先 → 选A(更新的那个) */
                    selectedIdx = IOTAP_B47_A;
                }
                else  /* secA > secB */
                {
                    /* A切换时间晚于B → 需要判断是否到了A的切换时间 */
                    if (secNow >= secA)
                    {
                        /* 当前时间已过A的切换时间 → 切换到A */
                        selectedIdx = IOTAP_B47_A;
                    }
                    else
                    {
                        /* 未到A的切换时间 → 继续用B */
                        selectedIdx = IOTAP_B47_B;
                    }
                }
            }
            else  /* recentIdx == IOTAP_B47_B */
            {
                /* 最近更新的是B组 */
                if (secB < secA)
                {
                    /* B切换时间早于A(倒序费率场景) → 选B */
                    selectedIdx = IOTAP_B47_B;
                }
                else if (secA == secB)
                {
                    /* 切换时间相同 → 后下发优先 → 选B */
                    selectedIdx = IOTAP_B47_B;
                }
                else  /* secB > secA */
                {
                    /* B切换时间晚于A → 需判断是否到了B的切换时间 */
                    if (secNow >= secB)
                    {
                        /* 当前时间已过B的切换时间 → 切换到B */
                        selectedIdx = IOTAP_B47_B;
                    }
                    else
                    {
                        /* 未到B的切换时间 → 继续用A */
                        selectedIdx = IOTAP_B47_A;
                    }
                }
            }

            validExec = 1U;  /* 双有效分支必然选中一组 → 执行Step3 */
        }

        /* ====== Step3: 更新全局活跃索引 ====== */
        if (validExec == 1U)
        {
            g_iotapBillActiveIndex[port] = selectedIdx;

            IOTAP_CFG_InfoPrint("AP,B47刷新: port%d 选择%d组(ID:%02X %02X %02X %02X %02X %02X %02X %02X...)\r\n",
                                 port, selectedIdx,
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[0],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[1],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[2],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[3],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[4],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[5],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[6],
                                 g_stIotAPBillModeSave[port].billModeData[selectedIdx].billModeID[7]);
        }
    }
    /* 参数无效(port越界) → validExec保持0, 直接退出 */
}

/**
 * @brief  启动时从NVM恢复B47双缓冲计费模型数据
 * @note   上电初始化阶段调用,逐枪完成以下4步:
 *
 *         Step 1 — 全局RAM清零: g_stIotAPBillModeSave[] / g_iotapBillActiveIndex[] / g_iotapB49SwitchFlag[]
 *         Step 2 — 从NVM读取双缓冲: stAPParam.stBillModeSave[port] → g_stIotAPBillModeSave[port]
 *         Step 3 — recentUpdateIndex合法性修正: 若不在[0,1]范围内则默认回退到A组(防NVM损坏)
 *         Step 4 — 逐枪触发 RefreshNowbillModel() 决策引擎,确定当前应使用的活跃费率
 *
 *         对应旧代码 Read_rateB47_model_anpei()
 */
void IotAP_ReadRateB47ModelFromNVM(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint8_t port = 0U;

    /* ====== Step 1: 全局RAM变量防御性清零 ====== */
    /* 无论NVM读取是否成功,先清零确保无脏数据 */
    memset(g_stIotAPBillModeSave, 0x00, sizeof(g_stIotAPBillModeSave));
    memset(g_iotapBillActiveIndex, IOTAP_B47_INDEX_INVALID, sizeof(g_iotapBillActiveIndex));
    memset(g_iotapB49SwitchFlag, 0x00, sizeof(g_iotapB49SwitchFlag));

    /* ====== Step 2~4: 逐枪从NVM恢复并触发决策 ====== */
    if (pPrivateParam != NULL)
    {
        for (port = 0U; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            /* Step 2: 从NVM读取该枪的IotAPBillModeSave_Struct到RAM */
            memcpy(&g_stIotAPBillModeSave[port],
                   &pPrivateParam->stAPParam.stBillModeSave[port],
                   sizeof(IotAPBillModeSave_Struct));

            /* Step 3: recentUpdateIndex合法性修正(防NVM损坏:非法值默认回退A组) */
            if ((g_stIotAPBillModeSave[port].recentUpdateIndex != IOTAP_B47_A) &&
                (g_stIotAPBillModeSave[port].recentUpdateIndex != IOTAP_B47_B))
            {
                g_stIotAPBillModeSave[port].recentUpdateIndex = IOTAP_B47_A;
            }

            /* Step 4: 触发决策引擎,确定当前活跃计费模型 */
            IotAP_RefreshNowbillModel(port);
        }
    }

    IOTAP_CFG_InfoPrint("AP,B47启动恢复: 双缓冲区已初始化\r\n");
}
