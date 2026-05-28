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
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoYKC16M.h"
#include "Asw_ErrorHandle.h" 
#include "Asw_IotProtoYKC16Send.h"
#include "Asw_IotProtoYKC16Recv.h"
#include "Asw_ChargeIf.h"
#include "MS_Nvm.h"
#include "myMalloc.h"
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
IotYKC16Ctx_Struct *pIotYKC16Ctx = NULL;



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotYKC16_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotYKC16_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotYKC16_CycleReportRealData(void);
static void IotYKC16_CycleDetectUnreporteRecord(void);
static void IotYKC16_CycleDetect(void);
static void IotYKC16_WSInitHandle(void);
static void IotYKC16_WSOfflineHandle(void);
static void IotYKC16_WSLoginHandle(void);
static void IotYKC16_WSNormalHandle(void);
static IotYKC16StopReason_Enum IotYKC16_ConverStopReason(AswErrorType_Enum errType);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotYKC16_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_YKC16_CMD_LOGIN_REQ:                  pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][0];   break;
        case IOT_YKC16_CMD_HEARTBEAT_REQ:              pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][1];   break;
        case IOT_YKC16_CMD_BILLMODE_VERIFY_REQ:        pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][2];   break;
        case IOT_YKC16_CMD_BILLMODE_REQ:               pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][3];   break;
        case IOT_YKC16_CMD_REPORT_REALDATA:            pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][4];   break;
        case IOT_YKC16_CMD_CALL_REALDATA_ACK:          pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][5];   break;
        case IOT_YKC16_CMD_REMOTE_START_CHARGE_RSP:    pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][6];   break;
        case IOT_YKC16_CMD_REMOTE_STOP_CHARGE_RSP:     pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][7];   break;
        case IOT_YKC16_CMD_ORDER_RECORD_REQ:           pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][8];   break;
        case IOT_YKC16_CMD_PILE_START_CHARGE_REQ:      pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][9];   break;
        case IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY_RSP:   pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][10];  break;
        case IOT_YKC16_CMD_Para_RSP:                   pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][11];  break;
        case IOT_YKC16_CMD_SYNC_TIME_RSP:              pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][12];  break;
        case IOT_YKC16_CMD_SET_BILLMODE_4RATE_RSP:     pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][13];  break;
        case IOT_YKC16_CMD_SET_QRCODE_RSP:             pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][14];  break;
        case IOT_YKC16_CMD_REBOOT_RSP:                 pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][15];  break;
        case IOT_YKC16_CMD_UPDATE_RSP:                 pSendCtrl = &pIotYKC16Ctx->stSendCtrl[port][16];  break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotYKC16_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_YKC16_CMD_LOGIN_RSP:                  pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][0];   break;
        case IOT_YKC16_CMD_HEARTBEAT_RSP:              pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][1];   break;
        case IOT_YKC16_CMD_BILLMODE_VERIFY_RSP:        pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][2];   break;
        case IOT_YKC16_CMD_BILLMODE_4RATE_RSP:         pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][3];   break;
        case IOT_YKC16_CMD_CALL_REALDATA:              pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][4];   break;
        case IOT_YKC16_CMD_REMOTE_START_CHARGE:        pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][5];   break;
        case IOT_YKC16_CMD_REMOTE_STOP_CHARGE:         pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][6];   break;
        case IOT_YKC16_CMD_ORDER_RECORD_RSP:           pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][7];   break;
        case IOT_YKC16_CMD_PILE_START_CHARGE_RSP:      pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][8];   break;
        case IOT_YKC16_CMD_UPDATE_ACCOUNT_MONEY:       pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][9];  break;
        case IOT_YKC16_CMD_Para_REQ:                   pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][10];  break;
        case IOT_YKC16_CMD_SYNC_TIME:                  pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][11];  break;
        case IOT_YKC16_CMD_SET_BILLMODE_4RATE:         pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][12];  break;
        case IOT_YKC16_CMD_SET_QRCODE:                 pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][13];  break;
        case IOT_YKC16_CMD_REBOOT:                     pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][14];  break;
        case IOT_YKC16_CMD_UPDATE:                     pRecvCtrl = &pIotYKC16Ctx->stRecvCtrl[port][15];  break;
        default: break;
    }
    return pRecvCtrl;
}

static void IotYKC16_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;
    uint8_t billMmodelReportFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotYKC16_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        if (pIotYKC16Ctx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        if (pIotYKC16Ctx->lastGunConnectState[port] != curGunConnectState)
        {
            /* tt2.4插枪请求计费 */
            if (AswPlatM_GetPlatType() == eAswPlatType_TT24)
            {
                if (curGunConnectState == TRUE)
                {
                    billMmodelReportFlag = TRUE;
                }
            }

            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTYKC16_CFG_CHARGING_REALDATA_CYCLE : IOTYKC16_CFG_IDLE_REALDATA_CYCLE;
       
        if (Common_JudgeTimeoutMs(pIotYKC16Ctx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == TRUE)
        {
            realDataReportFlag = FALSE;
            pIotYKC16Ctx->lastGunState[port] = curGunState;
            pIotYKC16Ctx->lastGunConnectState[port] = curGunConnectState;
            pIotYKC16Ctx->realDataReportTick[port] = Common_GetSystick();
            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_REPORT_REALDATA, TRUE);
        }

        if (billMmodelReportFlag == TRUE)
        {
            billMmodelReportFlag = FALSE;
            Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_BILLMODE_REQ, TRUE);
        }
    }
}

/* 未上报交易记录补发轮询 */
static void IotYKC16_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    /* 存在未上报的记录 */
    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            /* 有枪正在发送/等待应答 */
            if (Common_GetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ) ||
                Common_GetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_RSP))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pIotYKC16Ctx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotYKC16Ctx->time))
            {
                port = pIotYKC16Ctx->stOrderInfo.port;

                /* 避免当数据库存在脏数据时，脏数据有问题，持续进入到这边 */
                if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotYKC16Ctx->stOrderInfo.protocolType != eAswPlatCardType_YKC16 ||
                    pIotYKC16Ctx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC16Ctx->time);
                }
                else
                {
                    Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_ORDER_RECORD_REQ, TRUE);
                }
            }
        }
    }
}


static void IotYKC16_CycleDetect(void)
{ 
    IotYKC16_CycleReportRealData();

    IotYKC16_CycleDetectUnreporteRecord();
}

static void IotYKC16_WSInitHandle(void)
{
    pIotYKC16Ctx->eWorkState = eIOTYKC16WorkState_Offline;
}

static void IotYKC16_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    pIotYKC16Ctx->loginSucc = FALSE;
    pIotYKC16Ctx->queueBusyFlag = FALSE;
    pIotYKC16Ctx->waitQueueIdleTick = 0;

    pIotYKC16Ctx->sendIndex = 0;
    pIotYKC16Ctx->sendPort = 0;    
    pIotYKC16Ctx->reqSeq = 0;

    memset(pIotYKC16Ctx->realDataReportTick, 0x00, sizeof(pIotYKC16Ctx->realDataReportTick));
    memset(pIotYKC16Ctx->lastGunState, 0x00, sizeof(pIotYKC16Ctx->lastGunState));
    memset(pIotYKC16Ctx->lastGunConnectState, 0x00, sizeof(pIotYKC16Ctx->lastGunConnectState));

    memset(pIotYKC16Ctx->stSendCtrl, 0x00, sizeof(pIotYKC16Ctx->stSendCtrl));
    memset(pIotYKC16Ctx->stRecvCtrl, 0x00, sizeof(pIotYKC16Ctx->stRecvCtrl));

    FrameQueue_Reset(pIotYKC16Ctx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotYKC16Ctx->pileDnBCD, 14);

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotYKC16Ctx->eWorkState = eIOTYKC16WorkState_Login;
}

static void IotYKC16_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotYKC16Ctx->eWorkState = eIOTYKC16WorkState_Normal;
        Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, 0, IOT_YKC16_CMD_LOGIN_REQ, TRUE);
    }
}

static void IotYKC16_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotYKC16_OfflineHandle();
    }
    else
    {
        if (pIotYKC16Ctx->loginSucc == TRUE)
        {
            IotYKC16_CycleDetect();
        }

        IotYKC16_UpCtrlSendDeal();

        IotYKC16_UpCtrlRecvDeal();

        IotYKC16_TimeoutDetect();
    }
}

static IotYKC16StopReason_Enum IotYKC16_ConverStopReason(AswErrorType_Enum errType)
{
    uint8_t index = 0;
    IotYKC16StopReason_Enum eStopReason = eIotYKC16StopReason_NoExpectedErr;
    uint8_t findFlag = FALSE;

    const struct
    {
        AswErrorType_Enum errType;
        IotYKC16StopReason_Enum stopReason;
    }stopReasonMap[] = 
    {
        {eErr_CpVoltAbnor,        eIotYKC16StopReason_CpVoltAbnor},
        {eErr_CpGroundFault,      eIotYKC16StopReason_CpGroundFault},
        {eErr_PEBreakFault,       eIotYKC16StopReason_PEBreakFault},
        {eErr_EmergencyStop,      eIotYKC16StopReason_EmergencyStop},
        {eErr_InputLineReversed,  eIotYKC16StopReason_OtherErr},
        {eErr_LeakageCurrErr,     eIotYKC16StopReason_LeakageCurrErr},
        {eErr_ShortCircleErr,     eIotYKC16StopReason_ShortCut},
        {eErr_RCDSelfcheckErr,    eIotYKC16StopReason_OtherErr},

        {eErr_AphaseInputOverVol, eIotYKC16StopReason_VoltageErr},
        {eErr_AphaseInputLessVol, eIotYKC16StopReason_VoltageErr},
        {eErr_OutputOverCurr,     eIotYKC16StopReason_OverCurr},

        {eErr_JcqMaloperation,    eIotYKC16StopReason_JcqMaloperation},
        {eErr_JcqSynechiaFault,   eIotYKC16StopReason_JcqSynechiaFault},
        {eErr_ReaderCommErr,      eIotYKC16StopReason_OtherErr},
        {eErr_MeterCommErr,       eIotYKC16StopReason_MeterCommErr},
        {eErr_EnvOverTempErr,     eIotYKC16StopReason_TempErr},
        {eErr_GunOverTempErr,     eIotYKC16StopReason_GunTempErr},        
        {eErr_POverTempErr,       eIotYKC16StopReason_TempErr},  
        
        {eErr_DatabaseErr,        eIotYKC16StopReason_DataBaseErr},       
        {eErr_MeterCalcErr,       eIotYKC16StopReason_MeterCalcErr},     

        {eErr_ChgStartTimeout,    eIotYKC16StopReason_StartTimeout}, 
        
        {eErr_DiodeStop,          eIotYKC16StopReason_DiodeStop},  
        
        {eSrc_LittleCurr,         eIotYKC16StopReason_LittleCurr},   
        {eSrc_S2BreakOff,         eIotYKC16StopReason_CarStop},          
        {eSrc_AppStop,            eIotYKC16StopReason_AppStop},            
        {eSrc_MannulStop,         eIotYKC16StopReason_ManualStop},      
        {eSrc_CardStop,           eIotYKC16StopReason_ManualStop},   
        {eSrc_InsuffBalance,      eIotYKC16StopReason_SumNoEnough},  
        {eSrc_StopbyMoney,        eIotYKC16StopReason_StopByMoney},  
        {eSrc_StopbyTime,         eIotYKC16StopReason_StopByTime},  
        {eSrc_StopbyEnergy,       eIotYKC16StopReason_StopByEnergy}, 
        {eErr_GunDisConn,         eIotYKC16StopReason_ManualStop}, 
        {eErr_CPBreakOff,         eIotYKC16StopReason_GunDisconnect}, 
    };  

    for (index = 0; index < ARRAY_SIZE(stopReasonMap); index++)
    {
        if (errType == stopReasonMap[index].errType)
        {
            /* 铁塔2.4刷卡停止码*/
            if (AswPlatM_GetPlatType() == eAswPlatType_TT24 && errType == eSrc_CardStop)
            {
                eStopReason = eIotTT24StopReason_CardStop;
            }
            else
            {
                eStopReason = stopReasonMap[index].stopReason;
            }
            
            findFlag = TRUE;
            break;
        }
    }

    if (findFlag == FALSE)
    {
        IOTYKC16_CFG_InfoPrint("云快充1.6结束原因转换，未找到对应原因，原始原因为：%d!\r\n", errType);
    }
    
    return eStopReason;
}

void IotYKC16_SetPowerControl(uint8_t port, uint8_t powerRate)
{
    AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_RELATIVE, powerRate * 10);
}

void IotYKC16_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC16ParamBillMode_Struct *pYKC16BillMode = &pPrivateParam->stYKC16Param.stBillMode;
    uint8_t periodCount = 0;
    uint8_t index = 0;
    uint16_t startIndex = 0;
	uint16_t stopIndex = 0;

    if (pStandardBillMode != NULL)
    {
        memset(pStandardBillMode, 0x00, sizeof(AswMonitorBillMode_Struct));
        
        pStandardBillMode->rateCount = 4;

        if (pStandardBillMode->rateCount != 0)
        {
            /* 转换计损比例 */
            pStandardBillMode->elecLossRate = pYKC16BillMode->elecLossRate;

            /* 转换费率内容 */
            /* 尖*/
            pStandardBillMode->rateElecPrice[0] = Common_FourUint8ToUint32(pYKC16BillMode->sharp_ele_fee);
            pStandardBillMode->rateSeverPrice[0] = Common_FourUint8ToUint32(pYKC16BillMode->sharp_ser_fee);
            /* 峰*/
            pStandardBillMode->rateElecPrice[1] = Common_FourUint8ToUint32(pYKC16BillMode->peak_ele_fee);
            pStandardBillMode->rateSeverPrice[1] = Common_FourUint8ToUint32(pYKC16BillMode->peak_ser_fee);
            /* 平*/
            pStandardBillMode->rateElecPrice[2] = Common_FourUint8ToUint32(pYKC16BillMode->flat_ele_fee);
            pStandardBillMode->rateSeverPrice[2] = Common_FourUint8ToUint32(pYKC16BillMode->flat_ser_fee);
            /* 谷*/
            pStandardBillMode->rateElecPrice[3] = Common_FourUint8ToUint32(pYKC16BillMode->valley_ele_fee);
            pStandardBillMode->rateSeverPrice[3] = Common_FourUint8ToUint32(pYKC16BillMode->valley_ser_fee);

            for (index = 0; index < pStandardBillMode->rateCount; index++)
            {
                pStandardBillMode->totalPrice[index] = pStandardBillMode->rateElecPrice[index] + pStandardBillMode->rateSeverPrice[index];
            }

             /* 转换时段内容 */
            for (startIndex = 0; startIndex < MSNVM_YKC16_BILLMIDE_PERIOD_COUNT;startIndex++)
            {

             pStandardBillMode->periodRate[startIndex] = pYKC16BillMode->period_rate[startIndex];
             pStandardBillMode->startTime[startIndex][0] = startIndex / 2;
             pStandardBillMode->startTime[startIndex][1] = (startIndex % 2) * 30;

             stopIndex = startIndex + 1;

             pStandardBillMode->stopTime[startIndex][0] = stopIndex / 2;
             pStandardBillMode->stopTime[startIndex][1] = (stopIndex % 2) * 30;

            }

            pStandardBillMode->periodCount = 48;
            pStandardBillMode->validFlag = TRUE;
        }
    }
}

void IotYKC16_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmYKC16OrderInfo_Struct *pOrderData = &pFlashRecord->stYKC16OrderInfo;
    uint8_t *pBuf = pProtocolRecord;
    uint16_t dataLen = 0;
    uint8_t index = 0;
    CommonDateTime_Struct dateTime;
    uint16_t temp = 0;

    if (pFlashRecord != NULL && pProtocolRecord != NULL && pRecordLen != NULL)
    {
        /* 交易流水号 */
        memcpy(&pBuf[dataLen], pOrderData->orderTransactionNum, 16);
        dataLen += 16;
        /* 设备编码 */
        memcpy(&pBuf[dataLen], pIotYKC16Ctx->pileDnBCD, 7);
        dataLen += 7;
        /* 枪号 */
        pBuf[dataLen++] = pOrderData->port + 1;
        /* 开始时间 */
        memcpy(&pBuf[dataLen], pOrderData->startTime, 7);
        dataLen += 7;
        /* 结束时间 */
        memcpy(&pBuf[dataLen], pOrderData->stopTime, 7);
        dataLen += 7;
        /* 单价、电量、计损电量、金额 */
        memcpy(&pBuf[dataLen], pOrderData->billInfo, 64);
        dataLen += 64;
        /* 电表总起值 */
        memcpy(&pBuf[dataLen], pOrderData->startMeterVal, 5);
        dataLen += 5;
        /* 电表总止值 */
        memcpy(&pBuf[dataLen], pOrderData->stopMeterVal, 5);
        dataLen += 5;
        /* 总电量 */
        Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->totalEnergy);
        dataLen += 4;
        /* 总计损电量 */
        Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->totalLossEnergy);
        dataLen += 4;
        /* 总消费金额 */
        Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->totalMoney);
        dataLen += 4;
        /* 电动汽车唯一标识 */
        memset(&pBuf[dataLen], 0, 17);
        dataLen += 17;
        /*  交易标识 */
        pBuf[dataLen++] = pOrderData->dealFlag;
        /* 交易日期 */
        memcpy(&pBuf[dataLen], pOrderData->dealDate, 7);
        dataLen += 7;
        /*  停止原因 */
        pBuf[dataLen++] = pOrderData->stopReason;
        /* 逻辑卡号 */
        memcpy(&pBuf[dataLen], pOrderData->logicCardNum, 8);
        dataLen += 8;
    }

    pRecordLen[0] = dataLen;
}


uint8_t IotYKC16_GetGunState(uint8_t port)
{
    uint8_t gunState = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (AswErrHandle_IsExsistError(port) == TRUE)
        {
            gunState = 0x01; /* 故障 */
        }
        else if (AswMonitor_IsOrderIdle(port) != TRUE)
        {
            gunState = 0x03; /* 充电中 */
        }
        else
        {
            gunState = 0x02; /* 空闲 */
        }
    }

    return gunState;
}

void IotYKC16_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotYKC16Ctx->loginSucc = FALSE;
    pIotYKC16Ctx->eWorkState = eIOTYKC16WorkState_Offline;
}

void IotYKC16_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotYKC16Ctx != NULL)
    {
        strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pParam->platMainIp) - 1);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 3072, 3072, &pIotYKC16Ctx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotYKC16Ctx->frameQueueChannelID;
    }
}

uint8_t IotYKC16_SwipCardCharge(uint8_t port)
{
    uint8_t ret = FALSE;

    if (pIotYKC16Ctx->loginSucc == TRUE)
    {
        if (port < SYSCFG_CFG_GUN_NUM)
        {
            if ((TRUE != Common_GetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_PILE_START_CHARGE_REQ)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotYKC16Ctx->pFuncRecvCtrl, port, IOT_YKC16_CMD_PILE_START_CHARGE_RSP)))
            {
                Common_SetSendEnable(pIotYKC16Ctx->pFuncSendCtrl, port, IOT_YKC16_CMD_PILE_START_CHARGE_REQ, TRUE);
                ret = TRUE;
            }
        }
    }

    return ret;
}

void IotYKC16_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{ 
    AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmYKC16OrderInfo_Struct *pYKC16Order = &pOrderData->platOrderInfo.stYKC16OrderInfo;
    uint8_t index = 0;

    /* 订单开始*/
    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        memcpy(pYKC16Order->pileDnBCD, pIotYKC16Ctx->pileDnBCD, 7);
        pYKC16Order->port = port;
        memcpy(pYKC16Order->orderTransactionNum, 
               pIotYKC16Ctx->stProtoData[port].curUsedOrderTransactionNum,
               sizeof(pIotYKC16Ctx->stProtoData[port].curUsedOrderTransactionNum));

        /* 时间戳转换成 CP56Time2a 格式 */
        Common_TimestampToCp56Time2a(pChargeData->chargeStartTime, &pYKC16Order->startTime[0]);
        Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, &pYKC16Order->stopTime[0]);
    
        memset(&pYKC16Order->startMeterVal[0], 0, 5);
        Common_Uint32ToFourUint8(&pYKC16Order->startMeterVal[0],pChargeData->startMeterVal);

        memset(&pYKC16Order->stopMeterVal[0], 0, 5);
        Common_Uint32ToFourUint8(&pYKC16Order->stopMeterVal[0],pChargeData->stopMeterVal);

        memcpy(pYKC16Order->dealDate, pYKC16Order->startTime, 7);

        if (pstChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_APP)
        {
            pYKC16Order->dealFlag = 0x01;
        }
        else
        {
            pYKC16Order->dealFlag = 0x02;
            memcpy(pYKC16Order->logicCardNum, pIotYKC16Ctx->stProtoData[port].authCardID, 8);
        }

        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatCardType_YKC16;
        pOrderData->orderLen = sizeof(MSNvmYKC16OrderInfo_Struct);
        pYKC16Order->stopReason = eIotYKC16StopReason_PowerOff;
    }

    Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, &pYKC16Order->stopTime[0]);
    
    memset(&pYKC16Order->stopMeterVal[0], 0, 5);
    Common_Uint32ToFourUint8(&pYKC16Order->stopMeterVal[0], pChargeData->stopMeterVal);

    pYKC16Order->totalEnergy = pChargeData->totalEnergy;
    pYKC16Order->totalLossEnergy = pChargeData->totalLossEnergy;
    pYKC16Order->totalMoney = pChargeData->totalMoney;

    

    for (index = 0; index < 4; index++)
    {
        pYKC16Order->billInfo[index][0] = pBillMode->totalPrice[index];            // 单价
        pYKC16Order->billInfo[index][1] = pChargeData->rateTotalEnergy[index];     // 电量     
        pYKC16Order->billInfo[index][2] = pChargeData->rateTotalLossEnergy[index]; // 计损电量              
        pYKC16Order->billInfo[index][3] = pChargeData->rateTotalMoney[index];      // 总金额  
    }
    /* 订单停止*/
    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        pYKC16Order->stopReason = IotYKC16_ConverStopReason(pChargeData->eChargeStopReason);
    }
}

void IotYKC16_InitMemory(void)
{
    pIotYKC16Ctx = (IotYKC16Ctx_Struct *)myMalloc(sizeof(IotYKC16Ctx_Struct));

    if (pIotYKC16Ctx != NULL)
    {
        memset(pIotYKC16Ctx, 0, sizeof(IotYKC16Ctx_Struct));
    }

    pIotYKC16Ctx->pFuncSendCtrl = IotYKC16_GetSendCtrl;
    pIotYKC16Ctx->pFuncRecvCtrl = IotYKC16_GetRecvCtrl;
}

void IotYKC16_MainFunction(void)
{
    switch (pIotYKC16Ctx->eWorkState)
    {
        case eIOTYKC16WorkState_Init:
        {
            IotYKC16_WSInitHandle();
            break;
        }
        case eIOTYKC16WorkState_Offline:
        {
            IotYKC16_WSOfflineHandle();
            break;
        }
        case eIOTYKC16WorkState_Login:
        {
            IotYKC16_WSLoginHandle();
            break;
        }
        case eIOTYKC16WorkState_Normal:
        {
            IotYKC16_WSNormalHandle();
            break;
        }
        default:
        {
            pIotYKC16Ctx->eWorkState = eIOTYKC16WorkState_Init;
        }
    }
}