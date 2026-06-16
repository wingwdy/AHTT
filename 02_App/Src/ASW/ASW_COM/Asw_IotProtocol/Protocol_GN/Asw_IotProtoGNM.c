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
#include "Asw_IotProtoGNM.h"
#include "Asw_ErrorHandle.h" 
#include "Asw_IotProtoGNSend.h"
#include "Asw_IotProtoGNRecv.h"
#include "Asw_ChargeIf.h"
#include "MS_Nvm.h"
#include "myMalloc.h"
#include "Asw_IotProtoOMM.h"
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
IotGNCtx_Struct *pIotGNCtx = NULL;



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotGN_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotGN_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotGN_CycleReportRealData(void);
static void IotGN_CycleDetectUnreporteRecord(void);
static void IotGN_CycleDetect(void);
static void IotGN_WSInitHandle(void);
static void IotGN_WSOfflineHandle(void);
static void IotGN_WSLoginHandle(void);
static void IotGN_WSNormalHandle(void);
static IotGNStopReason_Enum IotGN_ConverStopReason(AswErrorType_Enum errType);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotGN_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_REQ:                  pSendCtrl = &pIotGNCtx->stSendCtrl[port][0];   break;
        case IOT_GN_CMD_HEARTBEAT_REQ:              pSendCtrl = &pIotGNCtx->stSendCtrl[port][1];   break;
        case IOT_GN_CMD_BILLMODE_VERIFY_REQ:        pSendCtrl = &pIotGNCtx->stSendCtrl[port][2];   break;
        case IOT_GN_CMD_BILLMODE_REQ:               pSendCtrl = &pIotGNCtx->stSendCtrl[port][3];   break;
        case IOT_GN_CMD_REPORT_REALDATA:            pSendCtrl = &pIotGNCtx->stSendCtrl[port][4];   break;
        case IOT_GN_CMD_CALL_REALDATA_ACK:          pSendCtrl = &pIotGNCtx->stSendCtrl[port][5];   break;
        case IOT_GN_CMD_REMOTE_START_CHARGE_RSP:    pSendCtrl = &pIotGNCtx->stSendCtrl[port][6];   break;
        case IOT_GN_CMD_REMOTE_STOP_CHARGE_RSP:     pSendCtrl = &pIotGNCtx->stSendCtrl[port][7];   break;
        case IOT_GN_CMD_MULTI_ORDER_RECORD_REQ:     pSendCtrl = &pIotGNCtx->stSendCtrl[port][8];   break;
        case IOT_GN_CMD_ORDER_RECORD_REQ:           pSendCtrl = &pIotGNCtx->stSendCtrl[port][9];   break;
        case IOT_GN_CMD_PILE_START_CHARGE_REQ:      pSendCtrl = &pIotGNCtx->stSendCtrl[port][10];  break;
        case IOT_GN_CMD_UPDATE_ACCOUNT_MONEY_RSP:   pSendCtrl = &pIotGNCtx->stSendCtrl[port][11];  break;
        case IOT_GN_CMD_SYNC_TIME_RSP:              pSendCtrl = &pIotGNCtx->stSendCtrl[port][12];  break;
        case IOT_GN_CMD_SET_BILLMODE_4RATE_RSP:     pSendCtrl = &pIotGNCtx->stSendCtrl[port][13];  break;
        case IOT_GN_CMD_SET_BILLMODE_MULTIRATE_RSP: pSendCtrl = &pIotGNCtx->stSendCtrl[port][14];  break;
        case IOT_GN_CMD_SET_QRCODE_RSP:             pSendCtrl = &pIotGNCtx->stSendCtrl[port][15];  break;
        case IOT_GN_CMD_REBOOT_RSP:                 pSendCtrl = &pIotGNCtx->stSendCtrl[port][16];  break;
        case IOT_GN_CMD_UPDATE_RSP:                 pSendCtrl = &pIotGNCtx->stSendCtrl[port][17];  break;
        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotGN_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_GN_CMD_LOGIN_RSP:                  pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][0];   break;
        case IOT_GN_CMD_HEARTBEAT_RSP:              pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][1];   break;
        case IOT_GN_CMD_BILLMODE_VERIFY_RSP:        pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][2];   break;
        case IOT_GN_CMD_BILLMODE_4RATE_RSP:         pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][3];   break;
        case IOT_GN_CMD_BILLMODE_MUTIRATE_RSP:      pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][4];   break;
        case IOT_GN_CMD_CALL_REALDATA:              pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][5];   break;
        case IOT_GN_CMD_REMOTE_START_CHARGE:        pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][6];   break;
        case IOT_GN_CMD_REMOTE_STOP_CHARGE:         pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][7];   break;
        case IOT_GN_CMD_ORDER_RECORD_RSP:           pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][8];   break;
        case IOT_GN_CMD_PILE_START_CHARGE_RSP:      pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][9];   break;
        case IOT_GN_CMD_UPDATE_ACCOUNT_MONEY:       pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][10];  break;
        case IOT_GN_CMD_SYNC_TIME:                  pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][11];  break;
        case IOT_GN_CMD_SET_BILLMODE_4RATE:         pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][12];  break;
        case IOT_GN_CMD_SET_BILLMODE_MULTIRATE:     pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][13];  break;
        case IOT_GN_CMD_SET_QRCODE:                 pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][14];  break;
        case IOT_GN_CMD_REBOOT:                     pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][15];  break;
        case IOT_GN_CMD_UPDATE:                     pRecvCtrl = &pIotGNCtx->stRecvCtrl[port][16];  break;
        default: break;
    }
    return pRecvCtrl;
}

static void IotGN_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;
    uint8_t curErrInfo[32] = {0};

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotGN_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        /* 故障变位上报 */
        if (pIotGNCtx->errVersion[port] != AswErrHandle_GetErrStatusVersion(port))
        {
            pIotGNCtx->errVersion[port] = AswErrHandle_GetErrStatusVersion(port);
            IotOM_SetRealDataErrBit(port, curErrInfo);

            if (0 != memcmp(curErrInfo, pIotGNCtx->lastErrInfo[port], 32))
            {
                realDataReportFlag = TRUE;
            }
        }
        
        if (pIotGNCtx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        if (pIotGNCtx->lastGunConnectState[port] != curGunConnectState)
        {
            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTGN_CFG_CHARGING_REALDATA_CYCLE : IOTGN_CFG_IDLE_REALDATA_CYCLE;
       
        if (Common_JudgeTimeoutMs(pIotGNCtx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }
            
        if (realDataReportFlag == TRUE)
        {
            realDataReportFlag = FALSE;
            pIotGNCtx->lastGunState[port] = curGunState;
            pIotGNCtx->lastGunConnectState[port] = curGunConnectState;
            pIotGNCtx->realDataReportTick[port] = Common_GetSystick();
            memcpy(pIotGNCtx->lastErrInfo[port], curErrInfo, 32);
            memset(curErrInfo, 0x00, 32);
            Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_REPORT_REALDATA, TRUE);
        }
    }
}

static void IotGN_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ) ||
                Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ) ||
                Common_GetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, IOT_GN_CMD_ORDER_RECORD_RSP))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pIotGNCtx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotGNCtx->time))
            {
                port = pIotGNCtx->stOrderInfo.port;

                /* 避免当数据库存在脏数据时，脏数据有问题，持续进入到这边 */
                if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotGNCtx->stOrderInfo.protocolType != eAswPlatCardType_GN ||
                    pIotGNCtx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotGNCtx->time);
                }
                else
                {
                    if (pIotGNCtx->stOrderInfo.platOrderInfo.stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                    }
                    else
                    {
                        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_ORDER_RECORD_REQ, TRUE);
                    }
                }
            }
        }
    }
}

static void IotGN_CycleDetect(void)
{ 
    IotGN_CycleReportRealData();

    IotGN_CycleDetectUnreporteRecord();
}

static void IotGN_WSInitHandle(void)
{
    pIotGNCtx->eWorkState = eIOTGNWorkState_Offline;
}

static void IotGN_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    pIotGNCtx->loginSucc = FALSE;
    pIotGNCtx->queueBusyFlag = FALSE;
    pIotGNCtx->waitQueueIdleTick = 0;

    pIotGNCtx->sendIndex = 0;
    pIotGNCtx->sendPort = 0;    
    pIotGNCtx->reqSeq = 0;

    memset(pIotGNCtx->realDataReportTick, 0x00, sizeof(pIotGNCtx->realDataReportTick));
    memset(pIotGNCtx->lastGunState, 0x00, sizeof(pIotGNCtx->lastGunState));
    memset(pIotGNCtx->lastGunConnectState, 0x00, sizeof(pIotGNCtx->lastGunConnectState));
    memset(pIotGNCtx->lastErrInfo, 0x00, sizeof(pIotGNCtx->lastErrInfo));
    memset(pIotGNCtx->errVersion, 0x00, sizeof(pIotGNCtx->errVersion));


    memset(pIotGNCtx->stSendCtrl, 0x00, sizeof(pIotGNCtx->stSendCtrl));
    memset(pIotGNCtx->stRecvCtrl, 0x00, sizeof(pIotGNCtx->stRecvCtrl));

    FrameQueue_Reset(pIotGNCtx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotGNCtx->pileDnBCD, 14);

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotGNCtx->eWorkState = eIOTGNWorkState_Login;
}

static void IotGN_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotGNCtx->eWorkState = eIOTGNWorkState_Normal;
        Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, 0, IOT_GN_CMD_LOGIN_REQ, TRUE);
    }
}

static void IotGN_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotGN_OfflineHandle();
    }
    else
    {
        if (pIotGNCtx->loginSucc == TRUE)
        {
            IotGN_CycleDetect();
        }

        IotGN_UpCtrlSendDeal();

        IotGN_UpCtrlRecvDeal();

        IotGN_TimeoutDetect();
    }
}

static IotGNStopReason_Enum IotGN_ConverStopReason(AswErrorType_Enum errType)
{
    uint8_t index = 0;
    IotGNStopReason_Enum eStopReason = eIotGNStopReason_NoExpectedErr;
    uint8_t findFlag = FALSE;

    const struct
    {
        AswErrorType_Enum errType;
        IotGNStopReason_Enum stopReason;
    }stopReasonMap[] = 
    {
        {eErr_CpVoltAbnor,        eIotGNStopReason_CpVoltAbnor},
        {eErr_CpGroundFault,      eIotGNStopReason_CpGroundFault},
        {eErr_PEBreakFault,       eIotGNStopReason_PEBreakFault},
        {eErr_EmergencyStop,      eIotGNStopReason_EmergencyStop},
        {eErr_InputLineReversed,  eIotGNStopReason_OtherErr},
        {eErr_LeakageCurrErr,     eIotGNStopReason_LeakageCurrErr},
        {eErr_ShortCircleErr,     eIotGNStopReason_ShortCut},
        {eErr_RCDSelfcheckErr,    eIotGNStopReason_OtherErr},

        {eErr_AphaseInputOverVol, eIotGNStopReason_VoltageErr},
        {eErr_AphaseInputLessVol, eIotGNStopReason_VoltageErr},
        {eErr_OutputOverCurr,     eIotGNStopReason_OverCurr},

        {eErr_JcqMaloperation,    eIotGNStopReason_JcqMaloperation},
        {eErr_JcqSynechiaFault,   eIotGNStopReason_JcqSynechiaFault},
        {eErr_ReaderCommErr,      eIotGNStopReason_OtherErr},
        {eErr_MeterCommErr,       eIotGNStopReason_MeterCommErr},
        {eErr_EnvOverTempErr,     eIotGNStopReason_TempErr},
        {eErr_GunOverTempErr,     eIotGNStopReason_GunTempErr},        
        {eErr_POverTempErr,       eIotGNStopReason_TempErr},  
        
        {eErr_DatabaseErr,        eIotGNStopReason_DataBaseErr},       
        {eErr_MeterCalcErr,       eIotGNStopReason_MeterCalcErr},     

        {eErr_ChgStartTimeout,    eIotGNStopReason_StartTimeout}, 
        
        {eErr_DiodeStop,          eIotGNStopReason_DiodeStop},  
        
        {eSrc_LittleCurr,         eIotGNStopReason_LittleCurr},   
        {eSrc_S2BreakOff,         eIotGNStopReason_CarStop},          
        {eSrc_AppStop,            eIotGNStopReason_AppStop},            
        {eSrc_MannulStop,         eIotGNStopReason_ManualStop},      
        {eSrc_CardStop,           eIotGNStopReason_ManualStop},   
        {eSrc_InsuffBalance,      eIotGNStopReason_SumNoEnough},  
        {eSrc_StopbyMoney,        eIotGNStopReason_StopByMoney},  
        {eSrc_StopbyTime,         eIotGNStopReason_StopByTime},  
        {eSrc_StopbyEnergy,       eIotGNStopReason_StopByEnergy}, 
        {eErr_GunDisConn,         eIotGNStopReason_ManualStop}, 
        {eErr_CPBreakOff,         eIotGNStopReason_GunDisconnect}, 
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
        IOTGN_CFG_InfoPrint("公牛结束原因转换，未找到对应原因，原始原因为：%d!\r\n", errType);
    }
    
    return eStopReason;
}

void IotGN_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGNParamBillMode_Struct *pGnBillMode = &pPrivateParam->stGNParam.stBillMode;
    uint8_t periodCount = 0;
    uint8_t index = 0;
    uint16_t startIndex = 0;
	uint16_t stopIndex = 0;

    if (pStandardBillMode != NULL)
    {
        memset(pStandardBillMode, 0x00, sizeof(AswMonitorBillMode_Struct));

        if (pGnBillMode->billType == IOT_GN_BILLMODE_RATE_TYPE_4)
        {
            pStandardBillMode->rateCount = 4;
            pStandardBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_FOUR;
        }
        else if (pGnBillMode->billType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
        {
            pStandardBillMode->rateCount = 9;
            pStandardBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_MULT;
        }
        else
        {}

        if (pStandardBillMode->rateCount != 0)
        {
            /* 转换计损比例 */
            pStandardBillMode->elecLossRate = pGnBillMode->elecLossRate;

            /* 转换费率内容 */
            memcpy(pStandardBillMode->rateElecPrice, pGnBillMode->elecPriceRate, pStandardBillMode->rateCount * sizeof(uint32_t));
            memcpy(pStandardBillMode->rateSeverPrice, pGnBillMode->servePriceRate, pStandardBillMode->rateCount * sizeof(uint32_t));

            for (index = 0; index < pStandardBillMode->rateCount; index++)
            {
                pStandardBillMode->totalPrice[index] = pStandardBillMode->rateElecPrice[index] + pStandardBillMode->rateSeverPrice[index];
            }

             /* 转换时段内容 */
            for (startIndex = 0; startIndex < MSNVM_GN_BILLMIDE_PERIOD_COUNT;)
            {
                if ((startIndex == 0) || (pGnBillMode->period_rate[startIndex] != pGnBillMode->period_rate[startIndex - 1]))
                {
                    if (periodCount < ASWMONITOR_BILLMODE_PERIOD_COUNT)
                    {
                        pStandardBillMode->periodRate[periodCount] = pGnBillMode->period_rate[startIndex];
                        pStandardBillMode->startTime[periodCount][0] = startIndex / 2;
                        pStandardBillMode->startTime[periodCount][1] = (startIndex % 2) * 30;
                    }

                    stopIndex = startIndex + 1;

                    while (stopIndex < MSNVM_GN_BILLMIDE_PERIOD_COUNT &&
                    pGnBillMode->period_rate[startIndex] == pGnBillMode->period_rate[stopIndex])
                    {
                        stopIndex++;
                    }

                    pStandardBillMode->stopTime[periodCount][0] = stopIndex / 2;
                    pStandardBillMode->stopTime[periodCount][1] = (stopIndex % 2) * 30;

                    periodCount++;
                    startIndex = stopIndex;
                }
                else
                { 
                    startIndex++;
                }
            }

            pStandardBillMode->periodCount = periodCount;
            pStandardBillMode->validFlag = TRUE;
        }
    }
}

void IotGN_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmGNOrderInfo_Struct *pOrderData = &pFlashRecord->stGNOrderInfo;
    uint8_t *pBuf = pProtocolRecord;
    uint16_t dataLen = 0;
    uint8_t index = 0;
    CommonDateTime_Struct dateTime;
    uint16_t temp = 0;

    if (pFlashRecord != NULL && pProtocolRecord != NULL && pRecordLen != NULL)
    {
        /* 设备编码 */
        memcpy(&pBuf[dataLen], pIotGNCtx->pileDnBCD, 7);
        dataLen += 7;
        /* 枪号 */
        pBuf[dataLen++] = pOrderData->port + 1;
        /* 交易流水号 */
        memcpy(&pBuf[dataLen], pOrderData->orderTransactionNum, 16);
        dataLen += 16;
        /* 开始时间 */
        Common_TimestampToDateTime(pOrderData->startTime, &dateTime);
        temp = Common_uintBINToBCD(dateTime.year);
        pBuf[dataLen++] = (temp >> 8) & 0xFF;
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.month, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.day, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.hour, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.minute, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.second, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        /* 结束时间 */
        Common_TimestampToDateTime(pOrderData->stopTime, &dateTime);
        temp = Common_uintBINToBCD(dateTime.year);
        pBuf[dataLen++] = (temp >> 8) & 0xFF;
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.month, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.day, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.hour, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.minute, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);
        Common_BINToBCD(&dateTime.second, (uint8_t *)&temp, 1);
        pBuf[dataLen++] = (uint8_t)(temp);

        if (pFlashRecord->stGNOrderInfo.billmodeType == IOT_GN_BILLMODE_RATE_TYPE_MULT)
        {
            /* 电表总起值 */
            Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->startMeterVal);
            dataLen += 4;
            /* 电表总止值 */
            Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->stopMeterVal);
            dataLen += 4;
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
            memcpy(&pBuf[dataLen], pOrderData->vin, 17);
            dataLen += 17;
            /*  交易标识 */
            pBuf[dataLen++] = pOrderData->dealFlag;
            /* 交易日期 */
            Common_TimestampToDateTime(pOrderData->dealDate, &dateTime);
            temp = Common_uintBINToBCD(dateTime.year);
            pBuf[dataLen++] = (temp >> 8) & 0xFF;
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.month, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.day, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.hour, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.minute, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.second, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            /*  停止原因 */
            pBuf[dataLen++] = pOrderData->stopReason;
            /* 逻辑卡号 */
            memcpy(&pBuf[dataLen], pOrderData->logicCardNum, 8);
            dataLen += 8;
            /* 单价、电量、计损电量、金额 */
            memcpy(&pBuf[dataLen], pOrderData->billInfo, 144);
            dataLen += 144;
        }
        else
        {
            /* 单价、电量、计损电量、金额 */
            memcpy(&pBuf[dataLen], pOrderData->billInfo, 64);
            dataLen += 64;
            /* 电表总起值 */
            Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->startMeterVal);
            dataLen += 4;
            /* 电表总止值 */
            Common_Uint32ToFourUint8(&pBuf[dataLen], pOrderData->stopMeterVal);
            dataLen += 4;
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
            memcpy(&pBuf[dataLen], pOrderData->vin, 17);
            dataLen += 17;
            /*  交易标识 */
            pBuf[dataLen++] = pOrderData->dealFlag;
            /* 交易日期 */
            Common_TimestampToDateTime(pOrderData->dealDate, &dateTime);
            temp = Common_uintBINToBCD(dateTime.year);
            pBuf[dataLen++] = (temp >> 8) & 0xFF;
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.month, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.day, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.hour, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.minute, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            Common_BINToBCD(&dateTime.second, (uint8_t *)&temp, 1);
            pBuf[dataLen++] = (uint8_t)(temp);
            /*  停止原因 */
            pBuf[dataLen++] = pOrderData->stopReason;
            /* 逻辑卡号 */
            memcpy(&pBuf[dataLen], pOrderData->logicCardNum, 8);
            dataLen += 8;
        }

        pRecordLen[0] = dataLen;
    }
}


uint8_t IotGN_GetGunState(uint8_t port)
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

void IotGN_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotGNCtx->loginSucc = FALSE;
    pIotGNCtx->eWorkState = eIOTGNWorkState_Offline;
}

void IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotGNCtx != NULL)
    {
        strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pParam->platMainIp) - 1);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 3072, 3072, &pIotGNCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotGNCtx->frameQueueChannelID;
    }
}

uint8_t IotGN_SwipCardCharge(uint8_t port)
{
    uint8_t ret = FALSE;

    if (pIotGNCtx->loginSucc == TRUE)
    {
        if (port < SYSCFG_CFG_GUN_NUM)
        {
            if ((TRUE != Common_GetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_PILE_START_CHARGE_REQ)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotGNCtx->pFuncRecvCtrl, port, IOT_GN_CMD_PILE_START_CHARGE_RSP)))
            {
                Common_SetSendEnable(pIotGNCtx->pFuncSendCtrl, port, IOT_GN_CMD_PILE_START_CHARGE_REQ, TRUE);
                ret = TRUE;
            }
        }
    }

    return ret;
}

void IotGN_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{ 
    AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmGNOrderInfo_Struct *pGnOrder = &pOrderData->platOrderInfo.stGNOrderInfo;
    uint8_t index = 0;

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        if (pBillMode->billmodeType == ASWMONITOR_BILLMODE_TYPE_FOUR)
        {
            pGnOrder->billmodeType = IOT_GN_BILLMODE_RATE_TYPE_4;
        }
        else
        {
            pGnOrder->billmodeType = IOT_GN_BILLMODE_RATE_TYPE_MULT;
        }

        memcpy(pGnOrder->pileDnBCD, pIotGNCtx->pileDnBCD, 7);
        pGnOrder->port = port;
        memcpy(pGnOrder->orderTransactionNum, 
               pIotGNCtx->stProtoData[port].curUsedOrderTransactionNum,
               sizeof(pIotGNCtx->stProtoData[port].curUsedOrderTransactionNum));

        pGnOrder->startTime = pChargeData->chargeStartTime;
        pGnOrder->stopTime = pChargeData->chargeStopTime;
        pGnOrder->startMeterVal = pChargeData->startMeterVal;
        pGnOrder->stopMeterVal = pChargeData->stopMeterVal;
        pGnOrder->dealDate = pGnOrder->startTime;

        if (pstChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_APP)
        {
            pGnOrder->dealFlag = 0x01;
        }
        else
        {
            pGnOrder->dealFlag = 0x02;
            memcpy(pGnOrder->logicCardNum, pIotGNCtx->stProtoData[port].authCardID, 8);
        }

        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatCardType_GN;
        pOrderData->orderLen = sizeof(MSNvmGNOrderInfo_Struct);
        pGnOrder->stopReason = eIotGNStopReason_PowerOff;
    }

    pGnOrder->stopTime = pChargeData->chargeStopTime;
    pGnOrder->stopMeterVal = pChargeData->stopMeterVal;
    pGnOrder->totalEnergy = pChargeData->totalEnergy;
    pGnOrder->totalLossEnergy = pChargeData->totalLossEnergy;
    pGnOrder->totalMoney = pChargeData->totalMoney;

    for (index = 0; index < 9; index++)
    {
        pGnOrder->billInfo[index][0] = pBillMode->totalPrice[index];            // 单价
        pGnOrder->billInfo[index][1] = pChargeData->rateTotalEnergy[index];     // 电量     
        pGnOrder->billInfo[index][2] = pChargeData->rateTotalLossEnergy[index]; // 计损电量              
        pGnOrder->billInfo[index][3] = pChargeData->rateTotalMoney[index];      // 总金额  
    }

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        pGnOrder->stopReason = IotGN_ConverStopReason(pChargeData->eChargeStopReason);
    }
}

void IotGN_InitMemory(void)
{
    pIotGNCtx = (IotGNCtx_Struct *)myMalloc(sizeof(IotGNCtx_Struct));

    if (pIotGNCtx != NULL)
    {
        memset(pIotGNCtx, 0, sizeof(IotGNCtx_Struct));
    }

    pIotGNCtx->pFuncSendCtrl = IotGN_GetSendCtrl;
    pIotGNCtx->pFuncRecvCtrl = IotGN_GetRecvCtrl;
}

void IotGN_MainFunction(void)
{
    switch (pIotGNCtx->eWorkState)
    {
        case eIOTGNWorkState_Init:
        {
            IotGN_WSInitHandle();
            break;
        }
        case eIOTGNWorkState_Offline:
        {
            IotGN_WSOfflineHandle();
            break;
        }
        case eIOTGNWorkState_Login:
        {
            IotGN_WSLoginHandle();
            break;
        }
        case eIOTGNWorkState_Normal:
        {
            IotGN_WSNormalHandle();
            break;
        }
        default:
        {
            pIotGNCtx->eWorkState = eIOTGNWorkState_Init;
        }
    }
}























