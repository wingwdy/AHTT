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
#include "Asw_IotProtoXJM.h"
#include "Asw_IotProtoXJSend.h"
#include "Asw_IotProtoXJRecv.h"
#include "Asw_PlatM.h"
#include "FrameQueue.h"
#include "Asw_ChargeIf.h"
#include "SS_Ucm.h"
#include "Asw_Monitor.h"
#include "Version.h"
#include "SS_Tm.h"



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
IotXJCtx_Struct *pIotXJCtx = NULL;

static IotXJErrDesc_Struct c_IotXJErrDescTable[] = 
{
	{eErr_RCDSelfcheckErr,	         eXJErrCode_LeakageCurrErr },
/*  没办法多对一，上报不好处理，在下面进行特殊处理 
	{eErr_LeakageCurrErr,	         eXJErrCode_LeakageCurrErr },
	{eErr_CpVoltAbnor,	             eXJErrCode_CPVolErr   },
*/
	{eErr_CpGroundFault,	     	 eXJErrCode_CPVolErr   },
	{eErr_PEBreakFault,	         	 eXJErrCode_PEBreakFault   },
	{eErr_AphaseInputOverVol, 	     eXJErrCode_AphaseInputOverVol   },
	{eErr_AphaseInputLessVol, 	     eXJErrCode_AphaseInputLessVol   },
	{eErr_OutputOverCurr, 	         eXJErrCode_OutputOverCurr   },
	{eErr_JcqMaloperation, 	         eXJErrCode_JcqMaloperation   },
    {eErr_JcqSynechiaFault, 	     eXJErrCode_JcqSynechiaFault   },
	{eErr_EnvOverTempErr, 	         eXJErrCode_PileOverTemp   },	
	{eErr_GunOverTempErr,	         eXJErrCode_GunOverTemp	  }, 
	{eErr_InputLineReversed,         eXJErrCode_InputLineReversed	  }, 
	{eErr_ShortCircleErr,	         eXJErrCode_ShortCircleErr },
	{eErr_MeterCommErr,	         	 eXJErrCode_MeterCommErr  },
    {eErr_ReaderCommErr,             eXJErrCode_CardReaderErr },
    {eErr_MeterCalcErr,              eXJErrCode_MeterCalcErr  },
/*  第 16 个预留给设备禁用，平台要求设备禁用，按118上报 */
    {eErr_none,                      eXJErrCode_DevForbid  },
};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotXJ_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotXJ_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotXJ_CycleReportSignHeartState(void);
static void IotXJ_CycleReportEvent(void);
static void IotXJ_WSInitHandle(void);
static void IotXJ_WSOfflineHandle(void);
static void IotXJ_WSLoginHandle(void);
static void IotXJ_CycleDetectPileStatus(void);
static void IotXJ_CycleDetectPileData(void);
static void IotXJ_CycleDetectUnreporteRecord(void);
static void IotXJ_CycleDetectErrInfo(void);
static void IotXJ_CycleDetect(void);
static void IotXJ_WSNormalHandle(void);
static uint8_t IotXJ_CheckPileErrInfoReport(uint8_t port);
static void IotXJ_GunStatusManage(void);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotXJ_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_XJ_CMD_SET_INTEGER_PARA_RESPONSE:          pSendCtrl = &pIotXJCtx->stSendCtrl[port][0];   break;
        case IOT_XJ_CMD_SET_COMMON_PARA_RESPONSE:           pSendCtrl = &pIotXJCtx->stSendCtrl[port][1];   break;
        case IOT_XJ_CMD_REMOTE_CONFIG_RESPONSE:             pSendCtrl = &pIotXJCtx->stSendCtrl[port][2];   break;
        case IOT_XJ_CMD_QUERY_COMMON_PARA_RESPONSE:         pSendCtrl = &pIotXJCtx->stSendCtrl[port][3];   break;
        case IOT_XJ_CMD_SET_START_CHARGE_RESPONSE:          pSendCtrl = &pIotXJCtx->stSendCtrl[port][4];   break;
        case IOT_XJ_CMD_SET_STOP_CHARGE_RESPONSE:           pSendCtrl = &pIotXJCtx->stSendCtrl[port][5];   break;
        case IOT_XJ_CMD_SET_POWER_ALLOC_RESPONSE:           pSendCtrl = &pIotXJCtx->stSendCtrl[port][6];   break;
        case IOT_XJ_CMD_SEND_HEART:                         pSendCtrl = &pIotXJCtx->stSendCtrl[port][7];   break;
        case IOT_XJ_CMD_SEND_STATE_INFO:                    pSendCtrl = &pIotXJCtx->stSendCtrl[port][8];   break;
        case IOT_XJ_CMD_SEND_SIGN_INFO:                     pSendCtrl = &pIotXJCtx->stSendCtrl[port][9];   break;
        case IOT_XJ_CMD_SEND_ORDER_INFO:                    pSendCtrl = &pIotXJCtx->stSendCtrl[port][10];  break;
        case IOT_XJ_CMD_SET_RATEMODE_RESPONSE:              pSendCtrl = &pIotXJCtx->stSendCtrl[port][11];  break;
        case IOT_XJ_CMD_SEND_EVENT:                         pSendCtrl = &pIotXJCtx->stSendCtrl[port][12];  break;
        case IOT_XJ_CMD_SEND_ERROR_INFO:                    pSendCtrl = &pIotXJCtx->stSendCtrl[port][13];  break;
        case IOT_XJ_CMD_SEND_WARN_INFO:                     pSendCtrl = &pIotXJCtx->stSendCtrl[port][14];  break;
        case IOT_XJ_CMD_SET_OTA_RESPONSE:                   pSendCtrl = &pIotXJCtx->stSendCtrl[port][15];  break;
        case IOT_XJ_CMD_REQUEST_CARD_AUTH:                  pSendCtrl = &pIotXJCtx->stSendCtrl[port][16];  break;
        case IOT_XJ_CMD_REQUEST_CARD_CHARGE:                pSendCtrl = &pIotXJCtx->stSendCtrl[port][17];  break;
        default:
        {
            break;
        }
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotXJ_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_XJ_CMD_SET_INTEGER_PARA:                   pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][0];   break;
        case IOT_XJ_CMD_SET_COMMON_PARA:                    pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][1];   break;
        case IOT_XJ_CMD_REMOTE_CONFIG:                      pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][2];   break;
        case IOT_XJ_CMD_QUERY_COMMON_PARA:                  pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][3];   break;
        case IOT_XJ_CMD_SET_START_CHARGE:                   pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][4];   break;
        case IOT_XJ_CMD_SET_STOP_CHARGE:                    pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][5];   break;
        case IOT_XJ_CMD_SET_POWER_ALLOC:                    pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][6];   break;
        case IOT_XJ_CMD_SEND_HEART_RESPONSE:                pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][7];   break;
        case IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE:           pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][8];   break;
        case IOT_XJ_CMD_SEND_SIGN_INFO_RESPONSE:            pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][9];   break;
        case IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE:           pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][10];  break;
        case IOT_XJ_CMD_SET_RATEMODE:                       pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][11];  break;
        case IOT_XJ_CMD_SEND_EVENT_RESPONSE:                pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][12];  break;
        case IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE:           pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][13];  break;
        case IOT_XJ_CMD_SEND_WARN_INFO_RESPONSE:            pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][14];  break;
        case IOT_XJ_CMD_SET_OTA:                            pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][15];  break;
        case IOT_XJ_CMD_REQUEST_CARD_AUTH_RESPONSE:         pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][16];  break;
        case IOT_XJ_CMD_REQUEST_CARD_CHARGE_RESPONSE:       pRecvCtrl = &pIotXJCtx->stRecvCtrl[port][17];  break;
        default:
        {
            break;
        }
    }

    return pRecvCtrl;
}

void IotXJ_TransfromErrCode(AswErrorType_Enum eGnErrCode, IotXJErrCode_Enum *pXjErrCode)
{
    uint8_t index = 0;
    uint8_t findFlag = FALSE;

    struct 
    {
        AswErrorType_Enum eInputErrCode;
        IotXJErrCode_Enum eOutputErrCode;
    } stErrCodeMap[] = 
    {
        { eErr_none,                 eXJErrCode_Succ },
        { eErr_CpVoltAbnor,          eXJErrCode_CPVolErr},
        { eErr_CpGroundFault,        eXJErrCode_CPVolErr},
        { eErr_PEBreakFault,         eXJErrCode_PEBreakFault},
        { eErr_InputLineReversed,    eXJErrCode_InputLineReversed},
        { eErr_LeakageCurrErr,       eXJErrCode_LeakageCurrErr},
        { eErr_ShortCircleErr,       eXJErrCode_ShortCircleErr},
        { eErr_RCDSelfcheckErr,      eXJErrCode_LeakageCurrErr},
        { eErr_AphaseInputOverVol,   eXJErrCode_AphaseInputOverVol},
        { eErr_AphaseInputLessVol,   eXJErrCode_AphaseInputLessVol},

        { eErr_OutputOverCurr,       eXJErrCode_OutputOverCurr},
        { eErr_JcqMaloperation,      eXJErrCode_JcqMaloperation},
        { eErr_JcqSynechiaFault,     eXJErrCode_JcqSynechiaFault},
        { eErr_ReaderCommErr,        eXJErrCode_CardReaderErr},

        { eErr_MeterCommErr,         eXJErrCode_MeterCommErr},
        { eErr_EnvOverTempErr,       eXJErrCode_PileOverTemp},
        { eErr_GunOverTempErr,       eXJErrCode_GunOverTemp},
     
        { eErr_MeterCalcErr,         eXJErrCode_MeterCalcErr},

        { eErr_ChgStartTimeout,      eXJErrCode_S2ActTimeout},
        { eSrc_LittleCurr,           eXJErrCode_CarStop},
        { eSrc_S2BreakOff,           eXJErrCode_CarStop},
        { eSrc_AppStop,              eXJErrCode_AppStop},
        { eSrc_MannulStop,           eXJErrCode_CardStop},
        { eSrc_CardStop,             eXJErrCode_CardStop},
    
        { eSrc_StopbyTime,           eXJErrCode_StopByTime},
        { eSrc_StopbyEnergy,         eXJErrCode_StopByEnergy},
        { eErr_GunDisConn,           eXJErrCode_GunDisconnect},
        { eErr_CPBreakOff,           eXJErrCode_GunDisconnect},
    };

    for (index = 0; ARRAY_SIZE(stErrCodeMap); index++)
    {
        if (stErrCodeMap[index].eInputErrCode == eGnErrCode)
        {
            *pXjErrCode = stErrCodeMap[index].eOutputErrCode;
            findFlag = TRUE;
            break;
        }
    }

    if (findFlag == FALSE)
    {
        *pXjErrCode = eXJErrCode_OtherErr + eGnErrCode;
    }
}


static void IotXJ_WSInitHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    IotXJProtoData_Struct *pProtoData = &pIotXJCtx->stProtoData;
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;

    if (pPrivateParam->stXJParam.platinfo.frame106Interval == 0 || 
        pPrivateParam->stXJParam.platinfo.frame104Interval == 0 ||
        pPrivateParam->stXJParam.platinfo.frame102Interval == 0 ||
        pPrivateParam->stXJParam.platinfo.frame102MaxTimeoutTimes == 0)
    {
        pPrivateParam->stXJParam.platinfo.frame106Interval = 30;
        pPrivateParam->stXJParam.platinfo.frame104Interval = 30;
        pPrivateParam->stXJParam.platinfo.frame102Interval = 30;
        pPrivateParam->stXJParam.platinfo.frame102MaxTimeoutTimes = 3;
    }

    pIotXJCtx->eWorkState = eIotXJWorkState_Offline;
    pPrivateParam->stXJParam.platinfo.rebootCount++;

    MSNvm_ReadParaBlock(eMSNvmBlockID_Gun0OrderInfo, (uint8_t *)&pIotXJCtx->stOrderInfo, sizeof(MSNvmOrderInfo_Struct));
    pIotXJCtx->latestChargeTimestamp = pIotXJCtx->stOrderInfo.platOrderInfo.stXJOrderInfo.charge_start_time;
    memset(&pIotXJCtx->stOrderInfo, 0x00, sizeof(pIotXJCtx->stOrderInfo));
}

static void IotXJ_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    memset(pIotXJCtx->stProtoData.stEventType, 0x00, sizeof(pIotXJCtx->stProtoData.stEventType));
    memset(pIotXJCtx->stProtoData.stCurrentEventType, 0x00, sizeof(pIotXJCtx->stProtoData.stCurrentEventType));
    memset(pIotXJCtx->stProtoData.stErrInfoReport, 0x00, sizeof(pIotXJCtx->stProtoData.stErrInfoReport));
    memset(pIotXJCtx->stProtoData.lastConnectState, 0x00, sizeof(pIotXJCtx->stProtoData.lastConnectState));

    memset(pIotXJCtx->errVersion, 0x00, sizeof(pIotXJCtx->errVersion));
    pIotXJCtx->loginSucc = FALSE;
    pIotXJCtx->queueBusyFlag = FALSE;
    pIotXJCtx->waitQueueIdleTick = 0;

    pIotXJCtx->sendIndex = 0;
    pIotXJCtx->sendPort = 0;    
    pIotXJCtx->reqSeq = 0;
    pIotXJCtx->heartBeatSeq = 0;
    
    memset(pIotXJCtx->stSendCtrl, 0x00, sizeof(pIotXJCtx->stSendCtrl));
    memset(pIotXJCtx->stRecvCtrl, 0x00, sizeof(pIotXJCtx->stRecvCtrl));

    FrameQueue_Reset(pIotXJCtx->frameQueueChannelID);
    memcpy(pIotXJCtx->platDn, pParam->platPileDn, 32);

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotXJCtx->eWorkState = eIotXJWorkState_Login;
}

static void IotXJ_WSLoginHandle(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXJParam.platinfo;
    uint8_t port = 0;

    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO, TRUE);
        Common_SetSendImmdFlag(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO, TRUE);
        pIotXJCtx->eWorkState = eIotXJWorkState_Normal;
    }
}

static void IotXJ_CycleReportSignHeartState(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint32_t sendTick = 0;
    uint8_t port = 0;

    sendTick = Common_GetSendTick(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO);

    if (Common_JudgeTimeoutMs(sendTick, pPrivateParam->stXJParam.platinfo.frame106Interval * 60 * 1000))
    {
        if (Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO_RESPONSE) == FALSE)
        {
            Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO, TRUE);
            Common_SetSendTick(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_SIGN_INFO, Common_GetSystick());
        }
    }

    sendTick = Common_GetSendTick(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_HEART);

    if (Common_JudgeTimeoutMs(sendTick, pPrivateParam->stXJParam.platinfo.frame102Interval * 1000))
    {
        if (Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, 0, IOT_XJ_CMD_SEND_HEART_RESPONSE) == FALSE)
        {
            Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_HEART, TRUE);
            Common_SetSendTick(pIotXJCtx->pFuncSendCtrl, 0, IOT_XJ_CMD_SEND_HEART, Common_GetSystick());
        }
    }

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        sendTick = Common_GetSendTick(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO);

        if (Common_JudgeTimeoutMs(sendTick, pPrivateParam->stXJParam.platinfo.frame104Interval * 1000))
        {
            if (Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO_RESPONSE) == FALSE)
            {
                Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO, TRUE);
                Common_SetSendTick(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_STATE_INFO, Common_GetSystick());
            }
        }
    }
}

static void IotXJ_CycleReportEvent(void)
{
    uint8_t port = 0;
    IotXJEventType_Struct *pEventQueue = NULL;
    uint8_t gunConnectState = 0;
    uint8_t orderNo[32 + 1] = {0x00};

    /* 检测是否有枪连接事件 */
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        gunConnectState = AswChargeIf_CheckGunConnected(port);

        if (gunConnectState != pIotXJCtx->stProtoData.lastConnectState[port])
        {
            pIotXJCtx->stProtoData.lastConnectState[port] = gunConnectState;

            if (TRUE != AswMonitor_IsOrderIdle(port))
            {
                memcpy(orderNo, pIotXJCtx->stProtoData.curUsedOrderNo[port], 32);
            }
            else
            {
                memset(orderNo, 0x00, sizeof(orderNo));
            }

            if (gunConnectState == TRUE)
            {
                IotXJ_AddEventQueue(port, eIotXJEventType_GunPlugIn, 0, orderNo);
            }
            else
            {
                IotXJ_AddEventQueue(port, eIotXJEventType_GunPlugOut, 0, orderNo);
            }
        }
    }

    /* 检测是否有事件需要上报 */
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pEventQueue = IotXJ_GetFirstEventQueue(port);

        if (pEventQueue->eventType == 0)
        {
            continue;
        }

        if (Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_EVENT) ||
            Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_SEND_EVENT_RESPONSE))
        {
            continue;
        }

        Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_EVENT, TRUE);
    }
}

static void IotXJ_CycleDetectPileStatus(void)
{
 
}

static void IotXJ_CycleDetectPileData(void)
{

}

static void IotXJ_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_ORDER_INFO) ||
                Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_SEND_ORDER_INFO_RESPONSE) ||
                /* 尽量确保停止事件先发送 */
                Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_EVENT) ||
                Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_SEND_EVENT_RESPONSE))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pIotXJCtx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotXJCtx->time))
            {
                port = pIotXJCtx->stOrderInfo.port;

                /* 避免当数据库存在脏数据时，脏数据有问题，持续进入到这边 */
                if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotXJCtx->stOrderInfo.protocolType != eAswPlatCardType_XJ ||
                    pIotXJCtx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotXJCtx->time);
                }
                else
                {
                    Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_ORDER_INFO, TRUE);
                }
            }
        }
    }
}

static void IotXJ_CycleDetectErrInfo(void)
{
    uint8_t port = 0;
    uint8_t checkFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (pIotXJCtx->errVersion[port] != AswErrHandle_GetErrStatusVersion(port))
        {
            pIotXJCtx->errVersion[port] = AswErrHandle_GetErrStatusVersion(port);
            checkFlag = TRUE;
        }
    }

    if (pIotXJCtx->forbidState != AswMonitor_CheckForbidState())
    {
        pIotXJCtx->forbidState = AswMonitor_CheckForbidState();
        checkFlag = TRUE;
    }

    if (checkFlag == TRUE)
    {
        IotXJ_CheckErrStatus();
    }

	for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
	{
        if (TRUE == IotXJ_CheckPileErrInfoReport(port))
        {
            Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_SEND_ERROR_INFO, TRUE);
        }
	}
}

static void IotXJ_GunStatusManage(void)
{
    uint8_t chargeState;
	IotXJGunStatus_Enum eXJGunStatus;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        chargeState = AswChargeIf_GetChargeState(port);

        if (chargeState != ASWCHARGEIF_WORKSTATE_READY)
        {
            pIotXJCtx->stProtoData.chargeStarFailFlag[port] = FALSE;
        }

        switch (chargeState)
        {
            case ASWCHARGEIF_WORKSTATE_IDLE:
            {
                eXJGunStatus = eIotXJGunStatus_Idle;
                break;
            }

            case ASWCHARGEIF_WORKSTATE_READY:
            {
                if (pIotXJCtx->stProtoData.chargeStarFailFlag[port] == TRUE)
                {
                    eXJGunStatus = eIotXJGunStatus_ChargeFinish;
                }
                else
                {
                    eXJGunStatus = eIotXJGunStatus_Connected;
                }
                break;
            }
            case ASWCHARGE_WORKSTATE_STARTING:
            case ASWCHARGE_WORKSTATE_WAKEUP:
            case ASWCHARGE_WORKSTATE_CHARGING:
            case ASWCHARGE_WORKSTATE_PAUSEA:
            case ASWCHARGE_WORKSTATE_PAUSEB:
            {
                eXJGunStatus = eIotXJGunStatus_Charging;
                break;
            }
            case ASWCHARGE_WORKSTATE_FINISH:
            {
                eXJGunStatus = eIotXJGunStatus_ChargeFinish;
                break;
            }
            case ASWCHARGE_WORKSTATE_STOPPING:
            {
                eXJGunStatus = eIotXJGunStatus_Stopping;
                break;
            }
            default:
            {
                eXJGunStatus = eIotXJGunStatus_Idle;
                break;
            }
        }

        pIotXJCtx->stProtoData.eGunStatus[port] = eXJGunStatus;
    }
}


static void IotXJ_CycleDetect(void)
{
    IotXJ_CycleReportEvent();

    IotXJ_CycleReportSignHeartState();
    
    IotXJ_CycleDetectPileStatus();

    IotXJ_CycleDetectPileData();

    IotXJ_CycleDetectUnreporteRecord();

    IotXJ_CycleDetectErrInfo();
}

static void IotXJ_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotXJ_WSOfflineHandle();
    }
    else
    {
        if (pIotXJCtx->loginSucc == TRUE)
        {           
            IotXJ_CycleDetect();
        }

        IotXJ_UpCtrlSendDeal();

        IotXJ_UpCtrlRecvDeal();

        IotXJ_TimeoutDetect();
    }
}

static uint8_t IotXJ_CheckPileErrInfoReport(uint8_t port)
{
	IotXJErrInfoReport_Struct *pErrInfo = pIotXJCtx->stProtoData.stErrInfoReport[port];
	uint8_t index = 0,  ret = FALSE;

	if (pErrInfo[0].eReportState != eXJReportState_Null)
	{
		if (TRUE != Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE))
		{
			ret = TRUE;
		}
	}

	return ret;
}

void IotXJ_AddEventQueue(uint8_t port, uint8_t eventType, uint16_t eventParam, uint8_t *pOrderNo)
{
    uint8_t index = 0;

    for (index = 0; index < ARRAY_SIZE(pIotXJCtx->stProtoData.stEventType[port]); index++)
    {
        if (pIotXJCtx->stProtoData.stEventType[port][index].eventType == eventType)
        {
            pIotXJCtx->stProtoData.stEventType[port][index].eventParam = eventParam;
            memcpy(pIotXJCtx->stProtoData.stEventType[port][index].orderNo, pOrderNo, 32);
            break;
        }
        else if (pIotXJCtx->stProtoData.stEventType[port][index].eventType == 0)
        {
            pIotXJCtx->stProtoData.stEventType[port][index].eventType = eventType;
            pIotXJCtx->stProtoData.stEventType[port][index].eventParam = eventParam;
            memcpy(pIotXJCtx->stProtoData.stEventType[port][index].orderNo, pOrderNo, 32);
            break;
        }
        else
        {}
    }
}

void IotXJ_DelEventQueue(uint8_t port)
{
    memmove(pIotXJCtx->stProtoData.stEventType[port], pIotXJCtx->stProtoData.stEventType[port] + 1, sizeof(IotXJEventType_Struct) * (ARRAY_SIZE(pIotXJCtx->stProtoData.stEventType[port]) - 1));
    memset(&pIotXJCtx->stProtoData.stEventType[port][ARRAY_SIZE(pIotXJCtx->stProtoData.stEventType[port]) - 1], 0x00, sizeof(IotXJEventType_Struct));
}

IotXJEventType_Struct *IotXJ_GetFirstEventQueue(uint8_t port)
{
    return &pIotXJCtx->stProtoData.stEventType[port][0];
}

void IotXJ_DateTimeToBcdTime(CommonDateTime_Struct *pdateTime, uint8_t *pBcdTime)
{
    uint8_t temp = 0;

    if (pdateTime != NULL && pBcdTime != NULL)
    {
        temp = pdateTime->year / 100;
        Common_BINToBCD(&temp, &pBcdTime[0], 1);
        temp = pdateTime->year % 100;
        Common_BINToBCD(&temp, &pBcdTime[1], 1);
        Common_BINToBCD(&pdateTime->month, &pBcdTime[2], 1);
        Common_BINToBCD(&pdateTime->day, &pBcdTime[3], 1);
        Common_BINToBCD(&pdateTime->hour, &pBcdTime[4], 1);
        Common_BINToBCD(&pdateTime->minute, &pBcdTime[5], 1);
        Common_BINToBCD(&pdateTime->second, &pBcdTime[6], 1);
        pBcdTime[7] = 0xff;
    }
}

void IotXJ_BcdTimeToDateTime(uint8_t *pBcdTime, CommonDateTime_Struct *pdateTime)
{
    uint8_t temp = 0;

    if (pdateTime != NULL && pBcdTime != NULL)
    {
        Common_BCDToBIN(&pBcdTime[0], &temp, 1);
        pdateTime->year = temp * 100;
        Common_BCDToBIN(&pBcdTime[1], &temp, 1);
        pdateTime->year += temp;
        Common_BCDToBIN(&pBcdTime[2], &temp, 1);
        pdateTime->month = temp;
        Common_BCDToBIN(&pBcdTime[3], &temp, 1);
        pdateTime->day = temp;
        Common_BCDToBIN(&pBcdTime[4], &temp, 1);
        pdateTime->hour = temp;
        Common_BCDToBIN(&pBcdTime[5], &temp, 1);
        pdateTime->minute = temp;
        Common_BCDToBIN(&pBcdTime[6], &temp, 1);
        pdateTime->second = temp;
    }
}

void IotXJ_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmXJOrderInfo_Struct *pOrderData = &pFlashRecord->stXJOrderInfo;
    uint16_t dataLen = 0;
    CommonDateTime_Struct dateTime = {0};

    if (pFlashRecord != NULL && pProtocolRecord != NULL && pRecordLen != NULL)
    {
        /* 预留 */
        memset(&pProtocolRecord[dataLen], 0x00, 2);
        dataLen += 2;
        /* 充放电类型 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->charge_type, 2);
        dataLen += 2;
        /* 充电桩编码 */
        memcpy(&pProtocolRecord[dataLen], pOrderData->pileDn, 32);
        dataLen += 32;
        /* 充电枪类型 */
        pProtocolRecord[dataLen++] = pOrderData->gun_type;
        /* 充电枪口 */
        pProtocolRecord[dataLen++] = pOrderData->port + 1;
        /* 订单号 */
        memcpy(&pProtocolRecord[dataLen], pOrderData->charge_user_id, 32);
        dataLen += 32;
        /* 充电开始时间 */
        Common_TimestampToDateTime(pOrderData->charge_start_time, &dateTime);
        IotXJ_DateTimeToBcdTime(&dateTime, &pProtocolRecord[dataLen]);
        dataLen += 8;
        /* 充电结束时间 */
        Common_TimestampToDateTime(pOrderData->charge_end_time, &dateTime);
        IotXJ_DateTimeToBcdTime(&dateTime, &pProtocolRecord[dataLen]);
        dataLen += 8;
        /* 充电时间 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->charge_time, 4);
        dataLen += 4;
        /* 开始Soc */
        pProtocolRecord[dataLen++] = pOrderData->start_soc;
        /* 结束Soc */
        pProtocolRecord[dataLen++] = pOrderData->end_soc;
        /* 充电结束原因 */
        sprintf((char *)&pProtocolRecord[dataLen], "%04X", pOrderData->err_no);
        dataLen += 4;
        /* 本次充电电量 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->charge_kwh_amount, 4);
        dataLen += 4;
        /* 充电前电表读数 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->start_charge_kwh_meter, 8);
        dataLen += 8;
        /* 充电后电表读数 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->end_charge_kwh_meter, 8);
        dataLen += 8;
        /* 本次充电金额 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->total_charge_fee, 4);
        dataLen += 4;
        /* 是否不刷卡结束 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->is_not_stoped_by_card, 4);
        dataLen += 4;
        /* 充电前卡余额 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->start_card_money, 4);
        dataLen += 4;
        /* 充电后卡余额 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->end_card_money, 4);
        dataLen += 4;
        /* 服务费 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->total_service_fee, 4);
        dataLen += 4;
        /* 是否线下支付 */
        pProtocolRecord[dataLen++] = pOrderData->is_paid_by_offline;
        /* 充电策略 */
        pProtocolRecord[dataLen++] = pOrderData->charge_policy;
        /* 充电策略参数 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->charge_policy_param, 4);
        dataLen += 4;
        /* 车辆VIN */
        memcpy(&pProtocolRecord[dataLen], pOrderData->car_vin, 17);
        dataLen += 17;
        /* 车牌号 */
        memcpy(&pProtocolRecord[dataLen], pOrderData->car_plate_no, 8);
        dataLen += 8;
        /* 电量 */
        memcpy(&pProtocolRecord[dataLen], pOrderData->kwh_amount, sizeof(pOrderData->kwh_amount));
        dataLen += sizeof(pOrderData->kwh_amount);
        /* 充电枪类型 */
        pProtocolRecord[dataLen++] = pOrderData->start_charge_type;
        /* 卡号 */
        memcpy(&pProtocolRecord[dataLen], pOrderData->card_id, 16);
        dataLen += 16;
        /* 放电前电表读数 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->start_discharge_kwh_meter, 8);
        dataLen += 8;
        /* 放电后电表读数 */
        memcpy(&pProtocolRecord[dataLen], &pOrderData->end_discharge_kwh_meter, 8);
        dataLen += 8;

        *pRecordLen = dataLen;
    }
}

void IotXJ_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam)
{
    uint8_t index = 0;

    pPrivateParam->stXJParam.platinfo.frame106Interval = 30;
    pPrivateParam->stXJParam.platinfo.frame104Interval = 30;
    pPrivateParam->stXJParam.platinfo.frame102Interval = 30;
    pPrivateParam->stXJParam.platinfo.frame102MaxTimeoutTimes = 3;

    /* 小桔要求没有计费模型也能充电，初始化时默认一套计费模型，但是不算钱 */
    pPrivateParam->stXJParam.stBillMode.periodCount = MSNVM_XJ_BILLMIDE_PERIOD_COUNT;

    for (index = 0; index < MSNVM_XJ_BILLMIDE_PERIOD_COUNT; index++)
    {
        pPrivateParam->stXJParam.stBillMode.periodDetail[index].startPeriod = index;
        pPrivateParam->stXJParam.stBillMode.periodDetail[index].continuesPeriodCount = index + 1;
    }
}

void IotXJ_AddErrInfoQueue(uint8_t port, uint16_t errIndex, uint8_t status)
{
	IotXJErrInfoReport_Struct *pErrInfo = pIotXJCtx->stProtoData.stErrInfoReport[port];
	uint8_t index = 0;

	for (index = 0; index < IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE; index++)
	{
		if (pErrInfo[index].eReportState == eXJReportState_Null)
		{
			pErrInfo[index].eReportState = eXJReportState_ToReport;
			pErrInfo[index].errIndex = errIndex;
			pErrInfo[index].status = status;
			break;
		}
		
		if (index == (IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE - 1))
		{
			IOTXJ_CFG_InfoPrint("[%s()]: Failed to push into the error info queue\r\n", __FUNCTION__);
		}
	}
}

void IotXJ_DelErrInfoQueue(uint8_t port)
{
	IotXJErrInfoReport_Struct *pErrInfo = pIotXJCtx->stProtoData.stErrInfoReport[port];
	uint8_t index = 0;
	uint8_t count = 0;

	for (index = 0; index < IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE; index++)
	{
		if (pErrInfo[index].eReportState == eXJReportState_Reporting)
		{
			count++;
		}
	}

	if (count > 0)
	{
		memmove(&pErrInfo[0], &pErrInfo[count], (IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE - count) * sizeof(IotXJErrInfoReport_Struct));
		memset(&pErrInfo[IOT_XJ_ERRINFO_REPORT_QUEUE_SIZE - count], 0x00, sizeof(IotXJErrInfoReport_Struct) * count);
	}
}

uint8_t IotXJ_CheckErrInfoReportStatusFree(void)
{
	IotXJErrInfoReport_Struct *pErrInfo = NULL;
	uint8_t gunNo = 0;
	uint8_t ret = TRUE;
	
	for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
	{
		pErrInfo = pIotXJCtx->stProtoData.stErrInfoReport[gunNo];

		if (pErrInfo[0].eReportState != eXJReportState_Null ||
			TRUE == Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, gunNo, IOT_XJ_CMD_SEND_ERROR_INFO_RESPONSE))
		{
			ret = FALSE;
			break;
		}
	}

	return ret;
}

void IotXJ_CheckErrStatus(void)
{
	IotXJErrDesc_Struct *pErrDesc = NULL;
	uint8_t index = 0, gunNo = 0, status = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXJErrDescTable); index++)
	{
		for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
		{
            pErrDesc = &c_IotXJErrDescTable[index];

            if (index == 0)
            {
                if (AswErrHandle_CheckErrExit(gunNo, eErr_RCDSelfcheckErr) || 
                    AswErrHandle_CheckErrExit(gunNo, eErr_LeakageCurrErr))
                {
                    status = TRUE;
                }
                else
                {
                    status = FALSE;
                }
            }
            else if (index == 1)
            {
                if (AswErrHandle_CheckErrExit(gunNo, eErr_CpVoltAbnor) || 
                    AswErrHandle_CheckErrExit(gunNo, eErr_CpGroundFault))
                {
                    status = TRUE;
                }
                else
                {
                    status = FALSE;
                }
            }
            else if (index == 15)
            {
                if (AswMonitor_CheckForbidState())
                {
                    status = TRUE;
                }
                else
                {
                    status = FALSE;
                }
            }
            else
            {
                status = AswErrHandle_CheckErrExit(gunNo, pErrDesc->eErrorCode);
            }

			if (status != pErrDesc->lastStatus[gunNo])
			{
                IotXJ_AddErrInfoQueue(gunNo, pErrDesc->errIndex, status);
                pErrDesc->lastStatus[gunNo] = status;
			}
		}
	}
}

void IotXJ_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{ 
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmXJOrderInfo_Struct *pXJOrder = &pOrderData->platOrderInfo.stXJOrderInfo;
    uint8_t index = 0;
    uint32_t totalPeriodChargeEnergy = 0;
    int32_t deltaEnergy = 0;

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        pIotXJCtx->chargeEnergy[port] = 0;
        pXJOrder->charge_type = 0;
        memcpy(pXJOrder->pileDn, pIotXJCtx->platDn, 32);
        pXJOrder->gun_type = 2;
        pXJOrder->port = port;
        memcpy(pXJOrder->charge_user_id, pIotXJCtx->stProtoData.curUsedOrderNo[port], 32);
        pXJOrder->charge_start_time = pChargeData->chargeStartTime;
        pIotXJCtx->latestChargeTimestamp = pXJOrder->charge_start_time;
        pXJOrder->start_soc = 0;
        pXJOrder->end_soc = 0;
        pXJOrder->err_no = eXJErrCode_PowerOff;
        pXJOrder->start_charge_kwh_meter = pChargeData->startMeterVal / 10;

        pXJOrder->total_charge_fee = 0;
        pXJOrder->total_service_fee = 0;
        pXJOrder->is_not_stoped_by_card = 0;
        pXJOrder->start_card_money = 0;
        pXJOrder->end_card_money = 0;
        pXJOrder->is_paid_by_offline = 0;

        if (pstChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeTime)
        {
            pXJOrder->charge_policy = 1;
        }
        else if (pstChargeCtrl->eChargeCtrlType == eAswMonitorChargeCtrlType_JudgeEnergy)
        {
            pXJOrder->charge_policy = 3;
        }
        else
        {
            pXJOrder->charge_policy = 0;
        }

        pXJOrder->charge_policy_param = pstChargeCtrl->chargeCtrlVal;
        memset(pXJOrder->car_vin, 0x00, sizeof(pXJOrder->car_vin));

        if (pstChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_CARD)
        {
            memcpy(pXJOrder->card_id, pIotXJCtx->stProtoData.authCardNo[port], 16);
            pXJOrder->start_charge_type = 0;
        }
        else
        {
            memset(pXJOrder->card_id, 0x00, sizeof(pXJOrder->card_id));
            pXJOrder->start_charge_type = 1;
        }

        memset(pXJOrder->car_plate_no, 0x00, sizeof(pXJOrder->car_plate_no));

        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatCardType_XJ;
        pOrderData->orderLen = sizeof(MSNvmXJOrderInfo_Struct);

        IotXJ_AddEventQueue(port, eIotXJEventType_ChargeStart, 0, pXJOrder->charge_user_id);
    }
    else if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        IotXJ_TransfromErrCode(pChargeData->eChargeStopReason, (IotXJErrCode_Enum *)&pXJOrder->err_no);
        IotXJ_AddEventQueue(port, eIotXJEventType_ChargeStop, pXJOrder->err_no, pXJOrder->charge_user_id);
    }
    else
    {}

    pXJOrder->end_charge_kwh_meter = pChargeData->stopMeterVal / 10;
    pIotXJCtx->chargeEnergy[port] = pXJOrder->end_charge_kwh_meter - pXJOrder->start_charge_kwh_meter;
    pXJOrder->charge_end_time = pChargeData->chargeStopTime;
    pXJOrder->charge_time = pChargeData->chargeTime;
    pXJOrder->charge_kwh_amount = pIotXJCtx->chargeEnergy[port];

    for (index = 0; index < 48; index++)
    {
        pXJOrder->kwh_amount[index] = pChargeData->periodElePower[index] / 10;
        totalPeriodChargeEnergy += pXJOrder->kwh_amount[index];
    }

    deltaEnergy = pIotXJCtx->chargeEnergy[port] - totalPeriodChargeEnergy;

    if (deltaEnergy != 0)
    {
        pXJOrder->kwh_amount[pChargeData->currentPeriodNum] += deltaEnergy;
    }
}
   

uint8_t IotXJ_SwipCardCharge(uint8_t port, uint8_t *pCardID)
{
    uint8_t ret = FALSE;

    if (pIotXJCtx->loginSucc == TRUE)
    {
        if (port < SYSCFG_CFG_GUN_NUM)
        {
            if ((TRUE != Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_REQUEST_CARD_AUTH)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_REQUEST_CARD_AUTH_RESPONSE)) &&
                (TRUE != Common_GetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_REQUEST_CARD_CHARGE)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotXJCtx->pFuncRecvCtrl, port, IOT_XJ_CMD_REQUEST_CARD_CHARGE_RESPONSE)))
            {
                Common_SetSendEnable(pIotXJCtx->pFuncSendCtrl, port, IOT_XJ_CMD_REQUEST_CARD_AUTH, TRUE);
                ret = TRUE;
            }
            else
            {
                IOTXJ_CFG_InfoPrint("[枪：%d]刷卡成功，但是已经有卡在申请启动充电，本次刷卡作废!\r\n", port);
            }
        }
    }
    else
    {
        IOTXJ_CFG_InfoPrint("[枪：%d]刷卡成功，设备离线，拒绝充电!!\r\n", port);
    }

    return ret;
}

uint8_t IotXJ_IsPileOnCharging(void)
{
    uint8_t port = 0;
    uint8_t ret = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (AswMonitor_IsOrderIdle(port) != TRUE)
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

void IotXJ_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotXJCtx->loginSucc = FALSE;
    pIotXJCtx->eWorkState = eIotXJWorkState_Offline;
}

void IotXJ_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXJParamBillMode_Struct *pRecvBillingModel = &pPrivateParam->stXJParam.stBillMode;
    uint8_t index = 0;
    uint8_t stdPeriodIndex = 0;
    uint8_t ret = TRUE;
    uint16_t startPeriod = 0;
    uint16_t continuesCount = 0;
    uint8_t xjPeriodIndex = 0;

    if (pRecvBillingModel->periodCount == 0 || pRecvBillingModel->periodCount > MSNVM_XJ_BILLMIDE_PERIOD_COUNT)
    {
        ret = FALSE;
        IOTXJ_CFG_InfoPrint("[%s()]计费模型时段数错误！\r\n", __FUNCTION__);
    }
    else
    {
        /* 初始化标准计费模型 */
        pStandardBillMode->rateCount = ASWMONITOR_BILLMODE_RATE_COUNT;
        pStandardBillMode->periodCount = ASWMONITOR_BILLMODE_PERIOD_COUNT;
        pStandardBillMode->elecLossRate = 0;

        /* 遍历所有48个标准时段 */
        for (stdPeriodIndex = 0; stdPeriodIndex < ASWMONITOR_BILLMODE_PERIOD_COUNT; stdPeriodIndex++)
        {
            /* 对第stdPeriodIndex个标准时段，找到对应的XJ时段 */
            xjPeriodIndex = 0;
            for (index = 0; index < pRecvBillingModel->periodCount; index++)
            {
                /* 获取当前XJ时段的起始时段和连续时段数 */
                startPeriod = pRecvBillingModel->periodDetail[index].startPeriod;
                continuesCount = pRecvBillingModel->periodDetail[index].continuesPeriodCount;
                
                /* 检查stdPeriodIndex是否在这个XJ时段的范围内 */
                if (stdPeriodIndex >= startPeriod && stdPeriodIndex < startPeriod + continuesCount)
                {
                    xjPeriodIndex = index;
                    break;
                }
            }
            
            /* 给这个标准时段创建对应的费率 */
            /* XJ费率是小数点后2位，标准是5位，需要乘以1000 */
            pStandardBillMode->rateElecPrice[stdPeriodIndex] = (uint32_t)pRecvBillingModel->periodDetail[xjPeriodIndex].elecPrice * 1000;
            pStandardBillMode->rateSeverPrice[stdPeriodIndex] = (uint32_t)pRecvBillingModel->periodDetail[xjPeriodIndex].servePrice * 1000;
            
            /* 这个标准时段使用费率号stdPeriodIndex（一一对应） */
            pStandardBillMode->periodRate[stdPeriodIndex] = stdPeriodIndex;
            
            /* 设置这个时段的时间 */
            pStandardBillMode->startTime[stdPeriodIndex][0] = stdPeriodIndex / 2;
            pStandardBillMode->startTime[stdPeriodIndex][1] = (stdPeriodIndex % 2) * 30;
            pStandardBillMode->stopTime[stdPeriodIndex][0] = (stdPeriodIndex + 1) / 2;
            pStandardBillMode->stopTime[stdPeriodIndex][1] = ((stdPeriodIndex + 1) % 2) * 30;
        }
    }

    if (ret == TRUE)
    {
        pStandardBillMode->validFlag = TRUE;
    }
    else
    {
        memset(pStandardBillMode, 0x00, sizeof(AswMonitorBillMode_Struct));
    }
}

uint8_t IotXJ_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint8_t envFlag = 0;

    if (pLinkPara != NULL && pIotXJCtx != NULL)
    {
        strncpy(pLinkPara->stMqttPara.ip, pParam->platMainIp, sizeof(pLinkPara->stMqttPara.ip));
        pLinkPara->stMqttPara.port = pParam->platMainPort;

        pLinkPara->stMqttPara.eVersion = eCddNetMMqttVersion_V3_1_1;
        pLinkPara->stMqttPara.keepAliveTime = 180;

        AswPlatM_GetEnvFlag(&envFlag);

        if (envFlag == 0)
        {
            strcpy(pLinkPara->stMqttPara.userName, IOTXJ_CFG_PRD_USERNAME);
            strcpy(pLinkPara->stMqttPara.password, IOTXJ_CFG_PRD_PASSWORD);
        }
        else
        {
            strcpy(pLinkPara->stMqttPara.userName, IOTXJ_CFG_UAT_USERNAME);
            strcpy(pLinkPara->stMqttPara.password, IOTXJ_CFG_UAT_PASSWORD);
        }
        
        memcpy(pLinkPara->stMqttPara.pid, pParam->platPileDn, 32);

        pLinkPara->stMqttPara.topicCount = 0;
        memcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], pParam->platPileDn, 32);

        FrameQueue_Creat(eFrameQueueType_MQTT, 3072, 3072, &pIotXJCtx->frameQueueChannelID);
        pLinkPara->stMqttPara.frameQueueChannelID = pIotXJCtx->frameQueueChannelID;
    }

    return TRUE;
}

void IotXJ_InitMemory(void)
{
    pIotXJCtx = (IotXJCtx_Struct *)myMalloc(sizeof(IotXJCtx_Struct));

    if (pIotXJCtx != NULL)
    {
        memset(pIotXJCtx, 0, sizeof(IotXJCtx_Struct));
    }

    pIotXJCtx->pFuncSendCtrl = IotXJ_GetSendCtrl;
    pIotXJCtx->pFuncRecvCtrl = IotXJ_GetRecvCtrl;
}

void IotXJ_MainFunction(void)
{
    switch (pIotXJCtx->eWorkState)
    {
        case eIotXJWorkState_Init:
        {
            IotXJ_WSInitHandle();
            break;
        }
        case eIotXJWorkState_Offline:
        {
            IotXJ_WSOfflineHandle();
            break;
        }
        case eIotXJWorkState_Login:
        {
            IotXJ_WSLoginHandle();
            break;
        }
        case eIotXJWorkState_Normal:
        {
            IotXJ_WSNormalHandle();
            break;
        }
        default:
        {
            pIotXJCtx->eWorkState = eIotXJWorkState_Init;
        }
    }

    IotXJ_GunStatusManage();
}





















