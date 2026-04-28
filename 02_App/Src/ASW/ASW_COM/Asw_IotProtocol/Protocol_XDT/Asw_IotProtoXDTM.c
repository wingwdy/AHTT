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
#include "Asw_IotProtoXDTM.h"
#include "Asw_IotProtoXDTSend.h"
#include "Asw_IotProtoXDTRecv.h"
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
IotXDTCtx_Struct *pIotXDTCtx = NULL;

const IotXDTStopReasonMap_Struct c_IotXDTStopReasonTable[] = 
{
    { eErr_CpVoltAbnor,                 eIotXDTStopReason_PileErr },
    { eErr_CpGroundFault,               eIotXDTStopReason_PileErr },
    { eErr_PEBreakFault,                eIotXDTStopReason_PileErr },
    { eErr_EmergencyStop,               eIotXDTStopReason_EmergeStop },
    { eErr_InputLineReversed,           eIotXDTStopReason_PileErr },
    { eErr_LeakageCurrErr,              eIotXDTStopReason_PileErr },
    { eErr_ShortCircleErr,              eIotXDTStopReason_PileErr },
    { eErr_RCDSelfcheckErr,             eIotXDTStopReason_PileErr },
    { eErr_AphaseInputOverVol,          eIotXDTStopReason_PileErr },
    { eErr_AphaseInputLessVol,          eIotXDTStopReason_PileErr },
    { eErr_OutputOverCurr,              eIotXDTStopReason_PileErr },
    { eErr_JcqMaloperation,             eIotXDTStopReason_PileErr },
    { eErr_JcqSynechiaFault,            eIotXDTStopReason_PileErr },
    { eErr_MeterCommErr,                eIotXDTStopReason_PileErr },
    { eErr_EnvOverTempErr,              eIotXDTStopReason_PileErr },
    { eErr_GunOverTempErr,              eIotXDTStopReason_PileErr },
    { eErr_POverTempErr,                eIotXDTStopReason_PileErr },
    { eErr_DatabaseErr,                 eIotXDTStopReason_PileErr },
    { eErr_MeterCalcErr,                eIotXDTStopReason_PileErr },
    { eErr_ChgStartTimeout,             eIotXDTStopReason_Other },
    { eErr_DiodeStop,                   eIotXDTStopReason_CarErr },
    { eSrc_LittleCurr,                  eIotXDTStopReason_ChargeFull },
    { eSrc_S2BreakOff,                  eIotXDTStopReason_ChargeFull },
    { eSrc_AppStop,                     eIotXDTStopReason_PlatformStop },
    { eSrc_MannulStop,                  eIotXDTStopReason_LocalStop },
    { eSrc_CardStop,                    eIotXDTStopReason_CarStop },
    { eSrc_InsuffBalance,               eIotXDTStopReason_InsuffcientFund },
    { eSrc_StopbyMoney,                 eIotXDTStopReason_ReachMoney },
    { eSrc_StopbyTime,                  eIotXDTStopReason_ReachTime },
    { eSrc_StopbyEnergy,                eIotXDTStopReason_ReachElec },
    { eErr_GunDisConn,                  eIotXDTStopReason_GunDisconnect },
    { eErr_CPBreakOff,                  eIotXDTStopReason_GunDisconnect },
};

static IotXDTErrDesc_Struct c_IotXDTErrDescTable[] = 
{
	{eErr_RCDSelfcheckErr,	     eIotXDTEntityType_Gun,      1,   eIotXDTErrorLevel_Ctrtical,  "RCD自检故障"},
	{eErr_LeakageCurrErr,	     eIotXDTEntityType_Gun,      2,   eIotXDTErrorLevel_Ctrtical,  "漏电故障"},
	{eErr_EmergencyStop,	     eIotXDTEntityType_Pile,     3,   eIotXDTErrorLevel_Ctrtical,  "急停故障"},
	{eErr_CpVoltAbnor,	         eIotXDTEntityType_Gun,      4,   eIotXDTErrorLevel_Major,     "CP电压异常"},
	{eErr_CpGroundFault,	     eIotXDTEntityType_Gun,	     5,   eIotXDTErrorLevel_Major,	   "CP对地短路"},
	{eErr_PEBreakFault,	         eIotXDTEntityType_Pile,	 6,   eIotXDTErrorLevel_Ctrtical,  "PE接地故障"},
	{eErr_AphaseInputOverVol, 	 eIotXDTEntityType_Pile,     7,   eIotXDTErrorLevel_Major,	   "交流输入过压"},
	{eErr_AphaseInputLessVol, 	 eIotXDTEntityType_Pile,     8,   eIotXDTErrorLevel_Major,	   "交流输入欠压"},
	{eErr_OutputOverCurr, 	     eIotXDTEntityType_Gun,      9,   eIotXDTErrorLevel_Ctrtical,  "交流输出过流"},
	{eErr_JcqSynechiaFault,	     eIotXDTEntityType_Gun,     10,   eIotXDTErrorLevel_Ctrtical,  "交流输出接触器粘连"},
	{eErr_JcqMaloperation, 	     eIotXDTEntityType_Gun,     11,   eIotXDTErrorLevel_Ctrtical,  "交流输出接触器误动拒动"},
	{eErr_EnvOverTempErr, 	     eIotXDTEntityType_Pile,    12,   eIotXDTErrorLevel_Major,	   "环境过温故障"},	
	{eErr_POverTempErr,	         eIotXDTEntityType_Pile,    13,	  eIotXDTErrorLevel_Major,	   "插头过温故障"}, 
	{eErr_GunOverTempErr,	     eIotXDTEntityType_Gun,     14,	  eIotXDTErrorLevel_Major,	   "枪过温故障"}, 
	{eErr_DiodeStop,	         eIotXDTEntityType_Gun,     15,	  eIotXDTErrorLevel_Minor,	   "未检测到二极管"}, 
	{eErr_InputLineReversed,     eIotXDTEntityType_Pile,    16,	  eIotXDTErrorLevel_Ctrtical,  "火零反接"}, 
	{eErr_ShortCircleErr,	     eIotXDTEntityType_Gun,     17,	  eIotXDTErrorLevel_Ctrtical,  "充电前输出短路故障"},
	{eErr_MeterCommErr,	         eIotXDTEntityType_Gun,	    18,	  eIotXDTErrorLevel_Major,     "电表通信故障"},
	{eErr_DatabaseErr,	         eIotXDTEntityType_Pile,    19,	  eIotXDTErrorLevel_Ctrtical,  "数据库存储错误"},
};
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static CommonSendCtrl_Struct* IotXDT_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL:      pSendCtrl = &pIotXDTCtx->stSendCtrl[port][0];   break;
        case IOT_XDT_CMD_REQUEST_TIMESYNC:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][1];   break;
        case IOT_XDT_CMD_REQUEST_LINK:                 pSendCtrl = &pIotXDTCtx->stSendCtrl[port][2];   break;
        case IOT_XDT_CMD_SET_RESTART_RSP:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][3];   break;
        case IOT_XDT_CMD_REQUEST_RATEMODE:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][4];   break;
        case IOT_XDT_CMD_QUERY_RATEMODE_RSP:           pSendCtrl = &pIotXDTCtx->stSendCtrl[port][5];   break;
        case IOT_XDT_CMD_RATEMODE_SET_RSP:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][6];   break;
        case IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT:       pSendCtrl = &pIotXDTCtx->stSendCtrl[port][7];   break;
        case IOT_XDT_CMD_PILE_STATE:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][8];   break;
        case IOT_XDT_CMD_ERRINFO:                      pSendCtrl = &pIotXDTCtx->stSendCtrl[port][9];   break;
        case IOT_XDT_CMD_PILE_DATA:                    pSendCtrl = &pIotXDTCtx->stSendCtrl[port][10];  break;
        case IOT_XDT_CMD_CALL_REALDATA_RSP:            pSendCtrl = &pIotXDTCtx->stSendCtrl[port][11];  break;
        case IOT_XDT_CMD_REQUEST_CARDAUTH:             pSendCtrl = &pIotXDTCtx->stSendCtrl[port][12];  break;
        case IOT_XDT_SET_CATEGORY:                     pSendCtrl = &pIotXDTCtx->stSendCtrl[port][13];  break;
        case IOT_XDT_CHARGE_START_RSP:                 pSendCtrl = &pIotXDTCtx->stSendCtrl[port][14];  break;
        case IOT_XDT_CHARGE_START_EVNET:               pSendCtrl = &pIotXDTCtx->stSendCtrl[port][15];  break;
        case IOT_XDT_CHARGE_STOP_RSP:                  pSendCtrl = &pIotXDTCtx->stSendCtrl[port][16];  break;
        case IOT_XDT_CHARGE_STOP_EVNET:                pSendCtrl = &pIotXDTCtx->stSendCtrl[port][17];  break;
        case IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP:       pSendCtrl = &pIotXDTCtx->stSendCtrl[port][18];  break;
        case IOT_XDT_CMD_FIRMWARE_STATE:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][19];  break;
        case IOT_XDT_QUERY_BOARDINFO_RSP:              pSendCtrl = &pIotXDTCtx->stSendCtrl[port][20];  break;
        case IOT_XDT_PARA_SET_RSP:                     pSendCtrl = &pIotXDTCtx->stSendCtrl[port][21];  break;
        case IOT_XDT_PARA_QUERY_RSP:                   pSendCtrl = &pIotXDTCtx->stSendCtrl[port][22];  break;
        case IOT_XDT_CHARGE_PWRCTRL_RSP:               pSendCtrl = &pIotXDTCtx->stSendCtrl[port][23];  break;
        case IOT_XDT_CHARGE_RECORD:                    pSendCtrl = &pIotXDTCtx->stSendCtrl[port][24];  break;
        case IOT_XDT_QUERY_CHARGE_RECORD_RSP:          pSendCtrl = &pIotXDTCtx->stSendCtrl[port][25];  break;
        case IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE:        pSendCtrl = &pIotXDTCtx->stSendCtrl[port][26];  break;
        default:
        {
            break;
        } 
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotXDT_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP:  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][0];   break;
        case IOT_XDT_CMD_REQUEST_TIMESYNC_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][1];   break;
        case IOT_XDT_CMD_REQUEST_LINK_RSP:             pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][2];   break;
        case IOT_XDT_CMD_SET_RESTART:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][3];   break;
        case IOT_XDT_CMD_REQUEST_RATEMODE_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][4];   break;
        case IOT_XDT_CMD_QUERY_RATEMODE:               pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][5];   break;
        case IOT_XDT_CMD_RATEMODE_SET:                 pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][6];   break;
        case IOT_XDT_CMD_PILE_STATE_RSP:               pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][7];   break;
        case IOT_XDT_CMD_ERRINFO_RSP:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][8];   break;
        case IOT_XDT_CMD_CALL_REALDATA:                pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][9];   break;
        case IOT_XDT_CMD_REQUEST_CARDAUTH_RSP:         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][10];  break;
        case IOT_XDT_SET_CATEGORY_RSP:                 pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][11];  break;
        case IOT_XDT_CHARGE_START:                     pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][12];  break;
        case IOT_XDT_CHARGE_STOP:                      pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][13];  break;
        case IOT_XDT_QUERY_BOARDINFO:                  pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][14];  break;
        case IOT_XDT_PARA_SET:                         pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][15];  break;
        case IOT_XDT_PARA_QUERY:                       pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][16];  break;
        case IOT_XDT_CHARGE_PWRCTRL:                   pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][17];  break;
        case IOT_XDT_CHARGE_RECORD_RSP:                pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][18];  break;
        case IOT_XDT_QUERY_CHARGE_RECORD:              pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][19];  break;
        case IOT_XDT_CMD_OTA_ATTRIBUTE_SET:            pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][20];  break;
        case IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP:    pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][21];  break;
        case IOT_XDT_CHARGE_CONTINUE_CHARGE:           pRecvCtrl = &pIotXDTCtx->stRecvCtrl[port][22];  break;
        default:
        {
            break;
        }
    }

    return pRecvCtrl;
}

static eIotXDTStopReason_Enum IotXDT_ConvertStopReason(AswErrorType_Enum eChargeStopReason)
{
	eIotXDTStopReason_Enum eXDTStopReason = eIotXDTStopReason_Other;
	uint8_t index = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTStopReasonTable); index++)
	{
		if (c_IotXDTStopReasonTable[index].eCommonStopReason == eChargeStopReason)
		{
			eXDTStopReason = c_IotXDTStopReasonTable[index].eXDTStopReason;
			break;
		}
	}

	return eXDTStopReason;
}
static void IotXDT_WSInitHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    IotXDTProtoData_Struct *pProtoData = &pIotXDTCtx->stProtoData;
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    float amountChangeThreshold = 0.5f;

    pIotXDTCtx->stProtoData.powerOnTick = Common_GetSystick();

    memcpy(pProtoData->mainIp, pParam->platMainIp, MSNVM_PLAT_IP_LEN);
    snprintf(pProtoData->mainPort, sizeof(pProtoData->mainPort), "%d", pParam->platMainPort);

    pIotXDTCtx->eWorkState = eIotXDTWorkState_Offline;

    if (pPlatInfo->pileDataReportCycle == 0)
    {
        pPlatInfo->pileDataReportCycle = 60;

        if (pPlatInfo->pileDataCycleReportEnable == FALSE)
        {
            pPlatInfo->pileDataCycleReportEnable = TRUE;
        }
    }

    if (pPlatInfo->amountChangeThreshold == 0)
    {
        memcpy(&pPlatInfo->amountChangeThreshold, &amountChangeThreshold, 4);
    }

    pPlatInfo->resetCount++;
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    AswMonitor_SetMinAccountMoney(IOT_XDT_MIN_ACCOUNT_MONEY);
    AswChargeIf_SetProfile(ASWCHARGEIF_PROFILE_XDT);
}

static void IotXDT_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    uint8_t port = 0;

	for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
	{
		memset(&pIotXDTCtx->stProtoData.stRecvData[port].offlineClearData, 0x00, sizeof(IotXDTDataOfflineClr_Struct));
	}

    pIotXDTCtx->loginSucc = FALSE;
    pIotXDTCtx->queueBusyFlag = FALSE;
    pIotXDTCtx->waitQueueIdleTick = 0;

    pIotXDTCtx->sendIndex = 0;
    pIotXDTCtx->sendPort = 0;    
    pIotXDTCtx->reqSeq = 0;

    memset(pIotXDTCtx->errVersion, 0x00, sizeof(pIotXDTCtx->errVersion));

    pIotXDTCtx->stProtoData.t1SetFlag = FALSE;
    pIotXDTCtx->stProtoData.t2SetFlag = FALSE;

    memset(pIotXDTCtx->stSendCtrl, 0x00, sizeof(pIotXDTCtx->stSendCtrl));
    memset(pIotXDTCtx->stRecvCtrl, 0x00, sizeof(pIotXDTCtx->stRecvCtrl));

    FrameQueue_Reset(pIotXDTCtx->frameQueueChannelID);
    memcpy(pIotXDTCtx->platDn, pParam->platPileDn, 16);

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++) 
    {
        pIotXDTCtx->stProtoData.s2Status[port] = 0xFF;
    }

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotXDTCtx->eWorkState = eIotXDTWorkState_Login;
}

static void IotXDT_WSLoginHandle(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t port = 0;

    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        if (pPlatInfo->credentialSaveFlag != TRUE)
        {
            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL, TRUE);
        }
        else
        {
            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_TIMESYNC, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_TIMESYNC, TRUE);
            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_LINK, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_LINK, TRUE);

            if (IotXDT_IsPileOnCharging() != TRUE)
            {
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_RATEMODE, TRUE);
                Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_RATEMODE, TRUE);
            }

            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_PILE_STATE, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_PILE_STATE, TRUE);

            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE, TRUE);

            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_FIRMWARE_STATE, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_FIRMWARE_STATE, TRUE);

            for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
            {
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_ERRINFO, TRUE);
                Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_ERRINFO, TRUE);
            }

            if (pPlatInfo->otaState == eIotXDTOtaState_Starting && pIotXDTCtx->stProtoData.otaStartFlag == FALSE)
            {
                if (strcmp(APP_SW_VERSION_STRING, pPlatInfo->otaSoftwareVersion) == 0)
                {
                    pPlatInfo->otaState = eIotXDTOtaState_Succ;
                }
                else
                {
                    pPlatInfo->otaState = eIotXDTOtaState_Fail;
                }

                memcpy(pPlatInfo->lastOtaSoftwareVersion, pPlatInfo->otaSoftwareVersion, MSNVM_XDT_VERSION_LEN);
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            }
        }

        pIotXDTCtx->eWorkState = eIotXDTWorkState_Normal;
    }
}

static void IotXDT_CycleDetectPileStatus(void)
{
    IotXDTPileStatus_Enum ePileStatus = IotXDT_GetPileStatus();
    IotXDTGunStatus_Enum eGunStatus;
    uint8_t s2Status = 0;
    uint8_t statusChangeFlag = FALSE;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        eGunStatus = IotXDT_GetGunStatus(port);
        
        if (eGunStatus != pIotXDTCtx->stProtoData.eGunStatus[port])
        {
            pIotXDTCtx->stProtoData.eGunStatus[port] = eGunStatus;
            statusChangeFlag = TRUE;
        }

        s2Status = AswChargeIf_CheckS2Closed(port);

        if (s2Status != pIotXDTCtx->stProtoData.s2Status[port])
        {
            pIotXDTCtx->stProtoData.s2Status[port] = s2Status;
            statusChangeFlag = TRUE;
        }
    }

    if (ePileStatus != pIotXDTCtx->stProtoData.ePileStatus)
    {
        pIotXDTCtx->stProtoData.ePileStatus = ePileStatus;
        statusChangeFlag = TRUE;
    }

    if (statusChangeFlag == TRUE)
    {
        Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_PILE_STATE, TRUE);
        Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_PILE_STATE, TRUE);
    }
}

static void IotXDT_CycleDetectPileData(void)
{
    AswMonitorChargeData_Struct *pChargeData = NULL;
    IotXDTRecvData_Struct *pRecvData = NULL;
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t index = 0;
    uint8_t item845Flag[SYSCFG_CFG_GUN_NUM] = {0};
    uint32_t chargeTotalMoney[SYSCFG_CFG_GUN_NUM] = {0};
    float fAmountThreshold = 0.0f;

	/* detect the transmmit condition of pile data */
	for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)
	{
		pRecvData = &pIotXDTCtx->stProtoData.stRecvData[index];
        pChargeData = AswMonitor_GetChargeDataPtr(index);

		if (AswMonitor_IsOrderIdle(index) != TRUE)
		{
			if (pPlatInfo->pileDataCycleReportEnable)
			{
                if (Common_JudgeTimeoutMs(Common_GetSendTick(pIotXDTCtx->pFuncSendCtrl, index, IOT_XDT_CMD_PILE_DATA), 
                    pPlatInfo->pileDataReportCycle * 1000))
                {
                    item845Flag[index] = TRUE;
                }
			}

            memcpy(&fAmountThreshold, &pPlatInfo->amountChangeThreshold, sizeof(float));

			chargeTotalMoney[index] = pChargeData->totalMoney;

			if ((chargeTotalMoney[index] > pRecvData->offlineClearData.chargeTotalMoney) &&
				(chargeTotalMoney[index] - pRecvData->offlineClearData.chargeTotalMoney) > 
				((uint32_t)(fAmountThreshold * 10000)))
			{
				pRecvData->offlineClearData.chargeTotalMoney = chargeTotalMoney[index];
				item845Flag[index] = TRUE;
			}

			if (item845Flag[index] == TRUE)
			{
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, index, IOT_XDT_CMD_PILE_DATA, TRUE);
                Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, index, IOT_XDT_CMD_PILE_DATA, TRUE);
				item845Flag[index] = FALSE;
			}
		}
		else
		{
			pRecvData->offlineClearData.chargeTotalMoney = 0;
			chargeTotalMoney[index] = 0;
			item845Flag[index] = TRUE;
		}
	}
}

static void IotXDT_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CHARGE_RECORD) ||
                Common_GetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, IOT_XDT_CHARGE_RECORD_RSP))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pIotXDTCtx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotXDTCtx->time))
            {
                port = pIotXDTCtx->stOrderInfo.port;

                /* 避免当数据库存在脏数据时，脏数据有问题，持续进入到这边 */
                if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotXDTCtx->stOrderInfo.protocolType != eAswPlatCardType_XDT ||
                    pIotXDTCtx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotXDTCtx->time);
                }
                else
                {
                    Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CHARGE_RECORD, TRUE);
                    Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CHARGE_RECORD, TRUE);
                }
            }
        }
    }
}

static void IotXDT_CycleDetectErrInfo(void)
{
    IotXDTRecvData_Struct *pRecvData = NULL;
    uint8_t port = 0;
    uint8_t checkFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (pIotXDTCtx->errVersion[port] != AswErrHandle_GetErrStatusVersion(port))
        {
            pIotXDTCtx->errVersion[port] = AswErrHandle_GetErrStatusVersion(port);
            checkFlag = TRUE;
        }
    }

    if (checkFlag == TRUE)
    {
        IotXDT_CheckErrStatus();
    }

	for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
	{
		pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
		
		if (pRecvData->offlineClearData.errClearInfoReportFlag == TRUE)
		{
			if (TRUE == IotXDT_CheckPileErrInfoReport(port))
			{
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_ERRINFO, TRUE);
                Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_ERRINFO, TRUE);
			}
		}
	}
}

static void IotXDT_CycleDetect(void)
{
    IotXDT_CycleDetectPileStatus();

    IotXDT_CycleDetectPileData();

    IotXDT_CycleDetectUnreporteRecord();

    IotXDT_CycleDetectErrInfo();
}

static void IotXDT_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotXDT_WSOfflineHandle();
    }
    else
    {
        if (pIotXDTCtx->loginSucc == TRUE)
        {           
            IotXDT_CycleDetect();
        }

        IotXDT_UpCtrlSendDeal();

        IotXDT_UpCtrlRecvDeal();

        IotXDT_TimeoutDetect();
    }
}

static void IotXDT_MqttConnectCallback(uint8_t connectResult, uint8_t *pCredential)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t flag = FALSE;

    if (connectResult == TRUE)
    {
        /* 如果已获取到凭据,且拿着新凭据连接成功 */
        if (pPlatInfo->credentialSaveFlag == TRUE)
        {
            if (pPlatInfo->credentialValidFlag != TRUE)
            {
                pPlatInfo->credentialValidFlag = TRUE;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            }
        }

        flag = pPlatInfo->credentialSaveFlag;
    }
    else
    {
        if (pPlatInfo->credentialSaveFlag == TRUE || pPlatInfo->credentialValidFlag == TRUE)
        {
            pPlatInfo->credentialSaveFlag = FALSE;
            pPlatInfo->credentialValidFlag = FALSE;
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        }
    }

    if (pCredential != NULL)
    {
        pCredential[0] = flag;
    }
}

void IotXDT_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmXDTOrderInfo_Struct *pOrderData = &pFlashRecord->stXDTOrderInfo;

    if (pFlashRecord != NULL && pProtocolRecord != NULL && pRecordLen != NULL)
    {
        memcpy(pProtocolRecord, pOrderData, sizeof(MSNvmXDTOrderInfo_Struct));
        pRecordLen[0] = sizeof(MSNvmXDTOrderInfo_Struct);
    }
}


uint8_t IotXDT_CheckPileErrInfoReport(uint8_t port)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	uint8_t index = 0,  ret = FALSE;

	if (pRecvDataInfo->offlineClearData.errInfoReportQueue[0].eReportState != eXDTReportState_Null)
	{
		if (TRUE != Common_GetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, IOT_XDT_CMD_ERRINFO_RSP))
		{
			ret = TRUE;
		}
	}

	return ret;
}

void IotXDT_AddErrInfoQueue(uint8_t port, uint8_t errIndex, IotXDTErrDesc_Struct *pErrDesc, uint8_t status, uint8_t callFlag)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	uint8_t index = 0;

	for (index = 0; index < IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE; index++)
	{
		if (pRecvDataInfo->offlineClearData.errInfoReportQueue[index].eReportState == eXDTReportState_Null)
		{
			pRecvDataInfo->offlineClearData.errInfoReportQueue[index].eReportState = eXDTReportState_ToReport;
			pRecvDataInfo->offlineClearData.errInfoReportQueue[index].callFlag = callFlag;
			pRecvDataInfo->offlineClearData.errInfoReportQueue[index].p = (uint8_t *)pErrDesc;
			pRecvDataInfo->offlineClearData.errInfoReportQueue[index].errIndex = errIndex;
			pRecvDataInfo->offlineClearData.errInfoReportQueue[index].status = status;
			break;
		}
		
		if (index == (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - 1))
		{
			IOTXDT_CFG_InfoPrint("[%s()]: Failed to push into the error info queue\r\n", __FUNCTION__);
		}
	}
}

void IotXDT_DelErrInfoQueue(uint8_t port)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	uint8_t index = 0;
	uint8_t count = 0;

	for (index = 0; index < IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE; index++)
	{
		if (pRecvDataInfo->offlineClearData.errInfoReportQueue[index].eReportState == eXDTReportState_Reporting)
		{
			count++;
		}
	}

	if (count > 0)
	{
		memmove(&pRecvDataInfo->offlineClearData.errInfoReportQueue[0], 
			&pRecvDataInfo->offlineClearData.errInfoReportQueue[count], (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - count) * sizeof(IotXDTErrInfoReport_Struct));
		memset(&pRecvDataInfo->offlineClearData.errInfoReportQueue[IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - count], 0x00, sizeof(IotXDTErrInfoReport_Struct) * count);
	}
}

uint8_t IotXDT_CheckErrInfoReportStatusFree(void)
{
	IotXDTRecvData_Struct *pRecvDataInfo = NULL;
	uint8_t gunNo = 0;
	uint8_t ret = TRUE;
	
	for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
	{
		pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[gunNo];

		if (pRecvDataInfo->offlineClearData.errInfoReportQueue[0].eReportState != eXDTReportState_Null ||
			TRUE == Common_GetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, gunNo, IOT_XDT_CMD_ERRINFO_RSP))
		{
			ret = FALSE;
			break;
		}
	}

	return ret;
}

IotXDTErrDesc_Struct *IotXDT_CheckFirstErr(uint8_t port)
{
	IotXDTErrDesc_Struct *pErrDesc = NULL;
	uint8_t index = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTErrDescTable); index++)
	{
		pErrDesc = &c_IotXDTErrDescTable[index];

		if (AswErrHandle_CheckErrExit(port, pErrDesc->eErrorCode))
		{
			break;
		}
		
		pErrDesc = NULL;
	}

	return pErrDesc;
}

static uint8_t IotXDT_CheckErrInfoReportForCall(void)
{
	IotXDTRecvData_Struct *pRecvDataInfo = NULL;
	uint8_t gunNo = 0;
	uint8_t ret = FALSE;
	
	for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
	{
		pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[gunNo];

		if (pRecvDataInfo->offlineClearData.errInfoReportQueue[0].callFlag == TRUE)
		{
			ret = TRUE;
			break;
		}
	}

	return ret;
}

void IotXDT_CheckErrStatus(void)
{
	IotXDTErrDesc_Struct *pErrDesc = NULL;
	uint8_t index = 0, gunNo = 0, status = 0;
	uint8_t callFlag = IotXDT_CheckErrInfoReportForCall();;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTErrDescTable); index++)
	{
		for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
		{
			pErrDesc = &c_IotXDTErrDescTable[index];
			status = AswErrHandle_CheckErrExit(gunNo, pErrDesc->eErrorCode);

			if (status != pErrDesc->lastStatus[gunNo])
			{
				if (callFlag == TRUE)
				{
					if (status == TRUE)
					{
						if (TRUE != Common_GetSendFlag(pIotXDTCtx->pFuncSendCtrl, gunNo, IOT_XDT_CMD_ERRINFO))
						{
							IotXDT_AddErrInfoQueue(gunNo, pErrDesc->errNo, pErrDesc, status, TRUE);
							pErrDesc->lastStatus[gunNo] = status;
							IOTXDT_CFG_InfoPrint("[%s()]: port: %d, status: %d, errdesc: %s\r\n", __FUNCTION__, gunNo, status, pErrDesc->alarmDesc);
						}
					}
				}
				else
				{
					IotXDT_AddErrInfoQueue(gunNo, pErrDesc->errNo, pErrDesc, status, FALSE);
					pErrDesc->lastStatus[gunNo] = status;
					IOTXDT_CFG_InfoPrint("[%s()]: port: %d, status: %d, errdesc: %s\r\n", __FUNCTION__, gunNo, status, pErrDesc->alarmDesc);
				}
			}
		}
	}
}

void IotXDT_RefreshErrStatusForCall(void)
{
	IotXDTErrDesc_Struct *pErrDesc = NULL;
	uint8_t index = 0, gunNo = 0, status = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTErrDescTable); index++)
	{
		for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
		{
			pErrDesc = &c_IotXDTErrDescTable[index];

			status = AswErrHandle_CheckErrExit(gunNo, pErrDesc->eErrorCode);

			if (status == TRUE)
			{
				pErrDesc->lastStatus[gunNo] = status;
				IotXDT_AddErrInfoQueue(gunNo, pErrDesc->errNo, pErrDesc, status, TRUE);
			}
		}
	}
}

void IotXDT_CheckErrInfoQueueDuplicate(uint8_t port)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	uint8_t startCheckPos = 0, endCheckPos = 0;
	uint8_t nullCount = 0;

	while (startCheckPos < (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - 1))
	{
		nullCount = 0;
		
		for (endCheckPos = startCheckPos + 1; endCheckPos < IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE; endCheckPos++)
		{
			if (pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos].p != NULL)
			{
				if (pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos].errIndex ==
					pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos].errIndex)
				{
					if (pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos].status !=
						pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos].status)
					{
						memset(&pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos], 0x00, sizeof(IotXDTErrInfoReport_Struct));
						memset(&pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos], 0x00, sizeof(IotXDTErrInfoReport_Struct));
					}
					else
					{
						memset(&pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos], 0x00, sizeof(IotXDTErrInfoReport_Struct));
					}
				}
			}
			else
			{
				nullCount++;
			}
		}
		
		if (nullCount >= (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - startCheckPos - 1))
		{
			break;
		}

		startCheckPos++;
	}

	startCheckPos = 0;
	endCheckPos = 0;

	while (startCheckPos < (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE - 1))
	{
		if (pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos].p == NULL)
		{
			nullCount = 0;
			
			for (endCheckPos = startCheckPos + 1; endCheckPos < IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE; endCheckPos++)
			{
				if (pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos].p != NULL)
				{
					memcpy(&pRecvDataInfo->offlineClearData.errInfoReportQueue[startCheckPos], 
						&pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos], sizeof(IotXDTErrInfoReport_Struct));
					memset(&pRecvDataInfo->offlineClearData.errInfoReportQueue[endCheckPos], 0x00, sizeof(IotXDTErrInfoReport_Struct));
				}
				else
				{
					nullCount++;
				}
			}
			
			if (nullCount == (IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE- startCheckPos - 1))
			{
				break;
			}
		}

		startCheckPos++;
	}
}


void IotXDT_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{ 
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmXDTOrderInfo_Struct *pXDTOrder = &pOrderData->platOrderInfo.stXDTOrderInfo;
    uint8_t index = 0;

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        Common_Uint32ToFourUint8(pXDTOrder->beginTs, pChargeData->chargeStartTime - SSTM_BASE_TIMESTAMP_1970_BJT);
        Common_Uint32ToFourUint8(pXDTOrder->beginMr, pChargeData->startMeterVal);
        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatCardType_XDT;
        pOrderData->orderLen = sizeof(MSNvmXDTOrderInfo_Struct);
        pXDTOrder->stopReason = eIotXDTStopReason_Other;
        pPlatInfo->orderCount++;
        Common_Uint32ToFourUint8(pXDTOrder->indexRec, pPlatInfo->orderCount);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        pXDTOrder->typeRec = 1;
    }
    else if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        if (pXDTOrder->stopReason == eIotXDTStopReason_Other)
        {
            pXDTOrder->stopReason = IotXDT_ConvertStopReason(pChargeData->eChargeStopReason);
        }

        if (pIotXDTCtx->loginSucc == TRUE)
        {
            pXDTOrder->typeRec = 0;
            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CHARGE_STOP_EVNET, TRUE);
            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CHARGE_STOP_EVNET, TRUE);
        }
    }
    else
    {}

    Common_Uint32ToFourUint8(pXDTOrder->endTs, pChargeData->chargeStopTime - SSTM_BASE_TIMESTAMP_1970_BJT);
    memcpy(pXDTOrder->ts, pXDTOrder->endTs, 4);
    Common_Uint32ToFourUint8(pXDTOrder->endMr, pChargeData->stopMeterVal);
    Common_Uint32ToFourUint8(pXDTOrder->tPq, pChargeData->totalLossEnergy);
    Common_Uint32ToFourUint8(pXDTOrder->elecAmt, pChargeData->totalElecMoney);
    Common_Uint32ToFourUint8(pXDTOrder->serMt, pChargeData->totalServeMoney);
    Common_Uint32ToFourUint8(pXDTOrder->amt, pChargeData->totalMoney);
    memset(pXDTOrder->pqTotal, 0x00, 4);

    for (index = 0; index < MSNVM_XDT_BILLMODE_PERIOD_COUNT; index++)
    {
        if (pChargeData->periodValidFlag[index] == TRUE)
        {
            pXDTOrder->periodInfoArray[index].valid = TRUE;
            pXDTOrder->periodInfoArray[index].sn = index + 1;
            Common_Uint32ToFourUint8(pXDTOrder->periodInfoArray[index].pq, pChargeData->periodElePower[index]);
        }
    }
}

uint8_t IotXDT_SwipCardCharge(uint8_t port)
{
    uint8_t ret = FALSE;

    if (pIotXDTCtx->loginSucc == TRUE)
    {
        if (port < SYSCFG_CFG_GUN_NUM)
        {
            if ((TRUE != Common_GetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_REQUEST_CARDAUTH)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, IOT_XDT_CMD_REQUEST_CARDAUTH_RSP)))
            {
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, IOT_XDT_CMD_REQUEST_CARDAUTH, TRUE);
                ret = TRUE;
            }
        }
    }

    return ret;
}

uint8_t IotXDT_CompareRecordOrderNum(uint8_t *record, uint8_t *pCompara, uint16_t paraSize)
{
    MSNvmOrderInfo_Struct *pOrderInfo = (MSNvmOrderInfo_Struct *)record;
    MSNvmXDTOrderInfo_Struct *pXDTOrderInfo = &pOrderInfo->platOrderInfo.stXDTOrderInfo;
    uint8_t ret = FALSE;

    if(0 == memcmp(pXDTOrderInfo->orderNo, pCompara, paraSize))
    {
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_IsPileOnCharging(void)
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

IotXDTGunStatus_Enum IotXDT_GetGunStatus(uint8_t port)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);
	IotXDTGunStatus_Enum eXDTGunStatus;

	if (TRUE == AswErrHandle_IsExsistError(port))
	{
		eXDTGunStatus = eIotXDTGunStatus_Error;
	}
	else
	{
		if (chargeState == ASWCHARGEIF_WORKSTATE_IDLE)
		{
			eXDTGunStatus = eIotXDTGunStatus_Idle;
		}
		else if (chargeState == ASWCHARGEIF_WORKSTATE_READY)
		{
			eXDTGunStatus = eIotXDTGunStatus_Connected;
		}
		else if (chargeState == ASWCHARGE_WORKSTATE_STARTING ||
				 chargeState == ASWCHARGE_WORKSTATE_WAKEUP ||
				 chargeState == ASWCHARGE_WORKSTATE_CHARGING ||
				 chargeState == ASWCHARGE_WORKSTATE_PAUSEA ||
                 chargeState == ASWCHARGE_WORKSTATE_PAUSEB ||
                 chargeState == ASWCHARGE_WORKSTATE_STOPPING)
		{
			eXDTGunStatus = eIotXDTGunStatus_Charging;
		}
		else if (chargeState == ASWCHARGE_WORKSTATE_FINISH)
		{
			eXDTGunStatus = eIotXDTGunStatus_ChargeFinish;
		}
		else
		{
			eXDTGunStatus = eIotXDTGunStatus_Idle;
		}		 
	}

	return eXDTGunStatus;
}


IotXDTPileStatus_Enum IotXDT_GetPileStatus(void)
{
	IotXDTPileStatus_Enum ePileStatus;
	uint8_t index = 0;
	uint8_t gunChargeStatus[SYSCFG_CFG_GUN_NUM];
    uint8_t gunErrStatus[SYSCFG_CFG_GUN_NUM];
	uint8_t errorFlag = FALSE;
	uint8_t workFlag = FALSE;
    uint8_t fixFlag = FALSE;

	for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)
	{
		gunChargeStatus[index] = AswChargeIf_GetChargeState(index);
        gunErrStatus[index] = AswErrHandle_IsExsistError(index);

        if (SSUcm_IsUpdating() == TRUE || AswMonitor_CheckForbidState() == TRUE)
        {
            fixFlag = TRUE;
        }
        else if (gunErrStatus[index] == TRUE)
		{
			errorFlag = TRUE;
		}
        else if (gunChargeStatus[index] != ASWCHARGEIF_WORKSTATE_IDLE)
        {
            workFlag = TRUE;
        }
        else
        {}
	}

    if (fixFlag == TRUE)
    {
        ePileStatus = eIotXDTPileStatus_Fix;
    }
	else if (errorFlag == TRUE)
	{
		ePileStatus = eIotXDTPileStatus_Error;
	}
	else if (workFlag == TRUE)
	{
		ePileStatus = eIotXDTPileStatus_Work;
	}
	else
	{
		ePileStatus = eIotXDTPileStatus_Idle;
	}

	return ePileStatus;
}

void IotXDT_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotXDTCtx->loginSucc = FALSE;
    pIotXDTCtx->eWorkState = eIotXDTWorkState_Offline;
}

static void IotXDT_RebootCheck(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();
    IotXDTProtoData_Struct *pProtoData = &pIotXDTCtx->stProtoData;
	uint8_t index = 0;
	uint8_t rebootFlag = FALSE;

	if (pProtoData->rebootFlag == TRUE)
	{
		if (IotXDT_IsPileOnCharging() == TRUE)
		{
			if (Common_JudgeTimeoutMs(pProtoData->rebootTick, 6000))
			{
				rebootFlag = TRUE;
			}
		}
		else
		{
			rebootFlag = TRUE;
		}
	}

	if (rebootFlag == TRUE)
	{
        if (pProtoData->rebootFlag == TRUE)
        {
            pProtoData->rebootFlag = FALSE;

			if (pProtoData->t1SetFlag == TRUE || pProtoData->t2SetFlag == TRUE)
			{
				if (pProtoData->t1SetFlag == TRUE)
				{
					memcpy(pParam->platMainIp, pProtoData->mainIp, MSNVM_PLAT_IP_LEN);
				}

				if (pProtoData->t2SetFlag == TRUE)
				{
					pParam->platMainPort = atoi(pProtoData->mainPort);
				}

                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatParam, (uint8_t *)pParam, sizeof(MSNvmPlatParam_Struct));
			}

            AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);
        }
	}
}

static void IotXDT_OtaCheck(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    SSUcmResult_Enum otaResult;

    if (pPlatInfo->otaState == eIotXDTOtaState_Starting && pIotXDTCtx->stProtoData.otaStartFlag == TRUE)
    {
        otaResult = SSUcm_GetResult();

        if (otaResult != eSSUcmResult_None && otaResult != eSSUcmResult_Succ)
        {
            pPlatInfo->otaState = eIotXDTOtaState_Fail;
            memcpy(pPlatInfo->lastOtaSoftwareVersion, pPlatInfo->otaSoftwareVersion, MSNVM_XDT_VERSION_LEN);
            MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            pIotXDTCtx->stProtoData.otaStartFlag = FALSE;

             if (pIotXDTCtx->loginSucc == TRUE)
             {
                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_FIRMWARE_STATE, TRUE);
                Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, 0, IOT_XDT_CMD_FIRMWARE_STATE, TRUE);
             }
        }
    }
}

static void IotXDT_CalcStaticInfo(void)
{
    AswMonitorChargeData_Struct *pChargeData = NULL;
	IotXDTGunStatus_Enum gunStatus;
	uint8_t index = 0;
	uint32_t delta = 0;
	uint32_t chargeEnergy = 0, chargeTimeSec = 0;
    uint32_t currentSysTick = Common_GetSystick();
	static uint32_t s_lastChargeEnergy[SYSCFG_CFG_GUN_NUM] = {0};
	static uint32_t s_lastChargeTimeSec[SYSCFG_CFG_GUN_NUM] = {0};
	static IotXDTGunStatus_Enum s_lastGunStatus[SYSCFG_CFG_GUN_NUM] = {0};

	for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)
	{
        pChargeData = AswMonitor_GetChargeDataPtr(index);
		gunStatus = IotXDT_GetGunStatus(index);
	
		if (gunStatus == eIotXDTGunStatus_Charging)
		{
			chargeEnergy = pChargeData->totalLossEnergy;
			
			if (chargeEnergy > s_lastChargeEnergy[index])
			{
				delta = chargeEnergy - s_lastChargeEnergy[index];

				if (delta < 10000)
				{
					pIotXDTCtx->stProtoData.totalChargeEnergy += delta;
				}
			}

			s_lastChargeEnergy[index] = chargeEnergy;

			chargeTimeSec = pChargeData->chargeTime;
			
			if (chargeTimeSec > s_lastChargeTimeSec[index])
			{
				delta = chargeTimeSec - s_lastChargeTimeSec[index];
				pIotXDTCtx->stProtoData.totalChargeTimeSec += delta;
			}

			s_lastChargeTimeSec[index] = chargeTimeSec;

			if (s_lastGunStatus[index] != gunStatus)
			{
				pIotXDTCtx->stProtoData.totalChargeTimes++; 
			}
		}
		else
		{
			s_lastChargeEnergy[index] = 0;
			s_lastChargeTimeSec[index] = 0;
		}

		s_lastGunStatus[index] = gunStatus;
	}

	if (currentSysTick > pIotXDTCtx->stProtoData.powerOnTick)
	{
		pIotXDTCtx->stProtoData.runTime = (currentSysTick - pIotXDTCtx->stProtoData.powerOnTick) / 1000;
	}
	else
	{
		pIotXDTCtx->stProtoData.runTime = (0xFFFFFFFF - pIotXDTCtx->stProtoData.powerOnTick + currentSysTick + 1) / 1000;
	}
}


uint8_t IotXDT_SetProductKey(char *pProductKey, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_PRODUCT_KEY_LEN)
    {
        memset(pPlatInfo->cProductKey, 0, MSNVM_XDT_PRODUCT_KEY_LEN);
        memcpy(pPlatInfo->cProductKey, pProductKey, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetProductKey(char *pProductKey, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t productKeyLen = 0;

    if (pProductKey != NULL && pOutLen != NULL)
    {
        productKeyLen = strlen(pPlatInfo->cProductKey);
        productKeyLen = (productKeyLen >= MSNVM_XDT_PRODUCT_KEY_LEN) ? MSNVM_XDT_PRODUCT_KEY_LEN : productKeyLen;
        memcpy(pProductKey, pPlatInfo->cProductKey, productKeyLen);
        *pOutLen = productKeyLen;
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_SetProductSecret(char *pProductSecret, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_PRODUCT_SECRET_LEN)
    {
        memset(pPlatInfo->cProductSecret, 0, MSNVM_XDT_PRODUCT_SECRET_LEN);
        memcpy(pPlatInfo->cProductSecret, pProductSecret, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetProductSecret(char *pProductSecret, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t productSecretLen = 0;

    if (pProductSecret != NULL && pOutLen != NULL)
    {
        productSecretLen = strlen(pPlatInfo->cProductSecret);
        productSecretLen = (productSecretLen >= MSNVM_XDT_PRODUCT_SECRET_LEN) ? MSNVM_XDT_PRODUCT_SECRET_LEN : productSecretLen;
        memcpy(pProductSecret, pPlatInfo->cProductSecret, productSecretLen);
        *pOutLen = productSecretLen;
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_SetDevOperator(char *pDevOperator, uint8_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;

    if (len <= MSNVM_XDT_DEV_OPERATOR_LEN)
    {
        memset(pPlatInfo->cOperator, 0, MSNVM_XDT_DEV_OPERATOR_LEN);
        memcpy(pPlatInfo->cOperator, pDevOperator, len);
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
        ret = TRUE;
    }

    return ret;
}

uint8_t IotXDT_GetDevOperator(char *pDevOperator, uint8_t *pOutLen)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
    uint8_t ret = FALSE;
    uint8_t devOperatorLen = 0;

    if (pDevOperator != NULL && pOutLen != NULL)
    {
        devOperatorLen = strlen(pPlatInfo->cOperator);
        devOperatorLen = (devOperatorLen >= MSNVM_XDT_DEV_OPERATOR_LEN) ? MSNVM_XDT_DEV_OPERATOR_LEN : devOperatorLen;
        memcpy(pDevOperator, pPlatInfo->cOperator, devOperatorLen);
        *pOutLen = devOperatorLen;
        ret = TRUE;
    }

    return ret;
}

void IotXDT_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pRecvBillingModel = &pPrivateParam->stXDTParam.stBillMode;
	uint8_t tempTimeH = 0, tempTimeL = 0;
	uint8_t ret = TRUE;
	uint8_t index = 0;

	if (Common_FourUint8ToUint32(pRecvBillingModel->validFlag) != IOT_XDT_MAGIC_NUM)
	{
		ret = FALSE;
		IOTXDT_CFG_InfoPrint("[%s()]magic num error\r\n", __FUNCTION__);
	}
	else if (pRecvBillingModel->period_count > MSNVM_XDT_BILLMODE_PERIOD_COUNT || 
		pRecvBillingModel->period_count == 0)
	{
		ret = FALSE;
		IOTXDT_CFG_InfoPrint("[%s()]period count error1\r\n", __FUNCTION__);
	}	
	else if (pRecvBillingModel->typeRule != 0 && pRecvBillingModel->typeRule != 1)
	{
		ret = FALSE;
		IOTXDT_CFG_InfoPrint("[%s()]type rule error\r\n", __FUNCTION__);
	}
	else
	{
        /* 计费模型ID */
        memcpy(pStandardBillMode->billModeID, pRecvBillingModel->billModeID, 4);
        pStandardBillMode->billmodeType = pRecvBillingModel->typeRule;

		// 48时段费率号
        pStandardBillMode->rateCount = pRecvBillingModel->period_count;
		pStandardBillMode->periodCount = pRecvBillingModel->period_count;
		pStandardBillMode->elecLossRate = pRecvBillingModel->measure_wastage_rates;

		if (pRecvBillingModel->typeRule == 0)
		{
			pStandardBillMode->rateElecPrice[0] = Common_FourUint8ToUint32(pRecvBillingModel->sharp_ele_fee);
			pStandardBillMode->rateSeverPrice[0] = Common_FourUint8ToUint32(pRecvBillingModel->sharp_ser_fee);
			tempTimeH = 0 / 2;
			tempTimeL = (0 % 2) * 30;
            pStandardBillMode->startTime[0][0] = tempTimeH;
            pStandardBillMode->startTime[0][1] = tempTimeL;
            tempTimeH = 48 / 2;
			tempTimeL = (48 % 2) * 30;
            pStandardBillMode->stopTime[0][0] = tempTimeH;
            pStandardBillMode->stopTime[0][1] = tempTimeL;
		}
		else
		{
			for (index = 0; index < pRecvBillingModel->period_count; index++)
			{
				if (pRecvBillingModel->period[index].validFlag == TRUE)
				{
					tempTimeH = pRecvBillingModel->period[index].startTime / 2;
					tempTimeL = (pRecvBillingModel->period[index].startTime % 2) * 30;
                    pStandardBillMode->startTime[index][0] = tempTimeH;
                    pStandardBillMode->startTime[index][1] = tempTimeL;

					tempTimeH = pRecvBillingModel->period[index].stopTime / 2;
					tempTimeL = (pRecvBillingModel->period[index].stopTime % 2) * 30;
                    pStandardBillMode->stopTime[index][0] = tempTimeH;
                    pStandardBillMode->stopTime[index][1] = tempTimeL;
                    pStandardBillMode->periodRate[index] = pRecvBillingModel->period[index].flag;

					switch (pRecvBillingModel->period[index].flag)
					{
						case 0:  // 尖
						{
							pStandardBillMode->rateElecPrice[0] = Common_FourUint8ToUint32(pRecvBillingModel->sharp_ele_fee);
							pStandardBillMode->rateSeverPrice[0] = Common_FourUint8ToUint32(pRecvBillingModel->sharp_ser_fee);
							break;
						}
						case 1:  // 峰 
						{
							pStandardBillMode->rateElecPrice[1] = Common_FourUint8ToUint32(pRecvBillingModel->peak_ele_fee);
							pStandardBillMode->rateSeverPrice[1] = Common_FourUint8ToUint32(pRecvBillingModel->peak_ser_fee);
							break;
						}
						case 2:  // 平
						{
							pStandardBillMode->rateElecPrice[2] = Common_FourUint8ToUint32(pRecvBillingModel->flat_ele_fee);
							pStandardBillMode->rateSeverPrice[2] = Common_FourUint8ToUint32(pRecvBillingModel->flat_ser_fee);
							break;				
						}
						case 3:  // 谷
						{
							pStandardBillMode->rateElecPrice[3] = Common_FourUint8ToUint32(pRecvBillingModel->valley_ele_fee);
							pStandardBillMode->rateSeverPrice[3] = Common_FourUint8ToUint32(pRecvBillingModel->valley_ser_fee);
							break;			
						}
						case 4:  // 深
						{
							pStandardBillMode->rateElecPrice[4] = Common_FourUint8ToUint32(pRecvBillingModel->deep_ele_fee);
							pStandardBillMode->rateSeverPrice[4] = Common_FourUint8ToUint32(pRecvBillingModel->deep_ser_fee);
							break;
						}
						default:
						{
							ret = FALSE;
							IOTXDT_CFG_InfoPrint("[%s()]period flag erro\r\n", __FUNCTION__);
							break;
						}
					}
				}
			}
		}
	}

    if (ret == TRUE)
    {
        pStandardBillMode->validFlag = TRUE;
    }
}

void IotXDT_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();

    if (pLinkPara != NULL && pIotXDTCtx != NULL)
    {
        strncpy(pLinkPara->stMqttPara.ip, pParam->platMainIp, sizeof(pLinkPara->stMqttPara.ip));
        pLinkPara->stMqttPara.port = pParam->platMainPort;

        pLinkPara->stMqttPara.eVersion = eCddNetMMqttVersion_V3_1_1;
        pLinkPara->stMqttPara.keepAliveTime = 180;

        if (pPrivateParam->stXDTParam.platinfo.credentialSaveFlag == TRUE)
        {
            strncpy(pLinkPara->stMqttPara.userName, pPrivateParam->stXDTParam.platinfo.cUserName, MSNVM_XDT_USER_NAME_LEN);
            strncpy(pLinkPara->stMqttPara.password, pPrivateParam->stXDTParam.platinfo.cPassword, MSNVM_XDT_PASSWORD_LEN);
        }
        else
        {
            strcpy(pLinkPara->stMqttPara.userName, "provision");
            strcpy(pLinkPara->stMqttPara.password, "provision");
        }

        memcpy(pLinkPara->stMqttPara.pid, pParam->platPileDn, 16);

        pLinkPara->stMqttPara.topicCount = 0;
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/r/res/+");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/r/req/+");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/fw/error");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/a");
        strcpy(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], "v2/a/res/+");

        pLinkPara->stMqttPara.pFuncMqttConnectCallback = IotXDT_MqttConnectCallback;

        FrameQueue_Creat(eFrameQueueType_MQTT, 3072, 3072, &pIotXDTCtx->frameQueueChannelID);
        pLinkPara->stMqttPara.frameQueueChannelID = pIotXDTCtx->frameQueueChannelID;
    }
}

void IotXDT_InitMemory(void)
{
    pIotXDTCtx = (IotXDTCtx_Struct *)myMalloc(sizeof(IotXDTCtx_Struct));

    if (pIotXDTCtx != NULL)
    {
        memset(pIotXDTCtx, 0, sizeof(IotXDTCtx_Struct));
    }

    pIotXDTCtx->pFuncSendCtrl = IotXDT_GetSendCtrl;
    pIotXDTCtx->pFuncRecvCtrl = IotXDT_GetRecvCtrl;
}

void IotXDT_MainFunction(void)
{
    switch (pIotXDTCtx->eWorkState)
    {
        case eIotXDTWorkState_Init:
        {
            IotXDT_WSInitHandle();
            break;
        }
        case eIotXDTWorkState_Offline:
        {
            IotXDT_WSOfflineHandle();
            break;
        }
        case eIotXDTWorkState_Login:
        {
            IotXDT_WSLoginHandle();
            break;
        }
        case eIotXDTWorkState_Normal:
        {
            IotXDT_WSNormalHandle();
            break;
        }
        default:
        {
            pIotXDTCtx->eWorkState = eIotXDTWorkState_Init;
        }
    }

    IotXDT_CalcStaticInfo();

    IotXDT_RebootCheck();

    IotXDT_OtaCheck();
}





















