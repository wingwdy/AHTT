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
#include "Asw_IotProtoYKC21M.h"
#include "Asw_ErrorHandle.h" 
#include "Asw_IotProtoYKC21Send.h"
#include "Asw_IotProtoYKC21Recv.h"
#include "Asw_ChargeIf.h"
#include "MS_Nvm.h"
#include "SS_Tm.h"
#include "Asw_VoltCurHandle.h"
#include "Asw_ChargeIf.h"

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
IotYKC21Ctx_Struct *pIotYKC21Ctx = NULL;
const char Default_RsaKey[]={"MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKaTP4eBWYBh3JDnYa7h2nuYACREgmV1o250/36ebYwaUswQDbUdMoeRvRIWxhCtXEzVkMYtH07ctmpzMo8uTvMCAwEAAQ=="};
const char Default_TokenStr[] = {"54260225221003"};	

static const IotYKC21errMap_Struct Iot_Ykc21Error_map[] = 
{
    {eErr_CpVoltAbnor,        eIotYKC21ErrorState_Pile,	0x0324}, 
    {eErr_CpGroundFault,      eIotYKC21ErrorState_Pile,	0x0440},
    {eErr_PEBreakFault,       eIotYKC21ErrorState_Pile,	0x02E2},
    {eErr_EmergencyStop,      eIotYKC21ErrorState_Pile,	0x02C3},
    {eErr_InputLineReversed,  eIotYKC21ErrorState_Pile,	0x0441},
    {eErr_LeakageCurrErr,     eIotYKC21ErrorState_Pile,	0x031E},
    {eErr_ShortCircleErr,     eIotYKC21ErrorState_Pile,	0x0313},
    {eErr_RCDSelfcheckErr,    eIotYKC21ErrorState_Pile, 0x031E}, 
    {eErr_AphaseInputOverVol, eIotYKC21ErrorState_Pile,	0x02DE},

    {eErr_AphaseInputLessVol, eIotYKC21ErrorState_Pile,	0x02DE},

    {eErr_OutputOverCurr,     eIotYKC21ErrorState_Pile,	0x0303},
    {eErr_JcqMaloperation,    eIotYKC21ErrorState_Pile,	0x0326},
    {eErr_JcqSynechiaFault,   eIotYKC21ErrorState_Pile,	0x02C8},
    {eErr_HmiCommErr,         eIotYKC21ErrorState_Pile,	0x0332},
	{eErr_ReaderCommErr,      eIotYKC21ErrorState_Pile,	0x02C5},

    {eErr_MeterCommErr,   	  eIotYKC21ErrorState_Pile,	0x02C6},  
    {eErr_EnvOverTempErr,     eIotYKC21ErrorState_Pile,	0x02C9},
    {eErr_GunOverTempErr,     eIotYKC21ErrorState_Pile,	0x02C9},
    {eErr_POverTempErr,       eIotYKC21ErrorState_Pile,	0x02CA},
    {eErr_DatabaseErr,        eIotYKC21ErrorState_Other,0x0440},             
    {eErr_MeterCalcErr,   	  eIotYKC21ErrorState_Pile,	0x02C7},  
    {eErr_ChgStartTimeout,    eIotYKC21ErrorState_Pile,	0x0003},
    {eErr_DiodeStop,      	  eIotYKC21ErrorState_Pile,	0x02DA},
 
    {eSrc_LittleCurr,         eIotYKC21ErrorState_Null,	0x0441},
    {eSrc_S2BreakOff,         eIotYKC21ErrorState_Null,	0x0442},
    {eSrc_AppStop,      	  eIotYKC21ErrorState_Null,	0x0443},
    {eSrc_MannulStop,         eIotYKC21ErrorState_Null,	0x0444},
    {eSrc_CardStop,      	  eIotYKC21ErrorState_Null,	0x0445},
    {eSrc_InsuffBalance,      eIotYKC21ErrorState_Null,	0x0446},
    {eSrc_StopbyMoney,        eIotYKC21ErrorState_Null,	0x0447},
    {eSrc_StopbyTime,         eIotYKC21ErrorState_Null,	0x0448},
    {eSrc_StopbyEnergy,       eIotYKC21ErrorState_Null,	0x0449},
    {eErr_GunDisConn,         eIotYKC21ErrorState_Null,	0x044A},
   
    {eErr_CPBreakOff,         eIotYKC21ErrorState_Pile,	0x0440 },

    {eErr_NetNoSIMErr,        eIotYKC21ErrorState_Pile,	0x02E8 },  
    {eErr_PlatformOffline,    eIotYKC21ErrorState_Plat,	0x01B4 },

};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

 


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
 
static CommonSendCtrl_Struct* IotYKC21_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_YKC21_CMD_LOGIN_REQ:                   pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][0];   break;
        case IOT_YKC21_CMD_HEARTBEAT_REQ:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][1];   break;
        case IOT_YKC21_CMD_BILLMODE_VERIFY_REQ:		    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][2];   break;
        case IOT_YKC21_CMD_BILLMODE_REQ:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][3];   break;
        case IOT_YKC21_CMD_REPORT_REALDATA:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][4];   break;
        case IOT_YKC21_CMD_CALL_REALDATA_ACK:		    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][5];   break;
        case IOT_YKC21_CMD_REMOTE_STOP_CHARGE_RSP:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][6];   break;
        case IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][7];   break;
        case IOT_YKC21_CMD_MULTI_ORDER_RECORD_ACK:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][8];   break;
        case IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY_RSP:    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][9];   break;
        case IOT_YKC21_CMD_FAULTREST_REQ:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][10];  break;
        case IOT_YKC21_CMD_RECORD_RSP:				    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][11];  break;
        case IOT_YKC21_CMD_FAULT_REQ:				    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][12];  break;
        case IOT_YKC21_CMD_POWERCHANG_RSP:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][13];  break;
        case IOT_YKC21_CMD_SYNC_TIME_RSP:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][14];  break;
        case IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE_RSP:  pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][15];  break;
        case IOT_YKC21_CMD_POWERDEFAULT_MAX_RSP:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][16];  break;
        case IOT_YKC21_CMD_SET_QRCODE_RSP:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][17];  break;
        case IOT_YKC21_CMD_SET_PARAM_RSP:			    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][18];  break;
        case IOT_YKC21_CMD_REBOOT_RSP:				    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][19];  break;
        case IOT_YKC21_CMD_SET_FTP_RSP:				    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][20];  break;
        case IOT_YKC21_CMD_SET_KEY_RSP:				    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][21];  break;
        case IOT_YKC21_CMD_PILE_START_CHARGE_REQ:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][22];  break;
        case IOT_YKC21_CMD_REMOTE_START_CHARGE_RSP:	    pSendCtrl = &pIotYKC21Ctx->stSendCtrl[port][23];  break;

        default: break;
    }

    return pSendCtrl;
}

static CommonRecvCtrl_Struct* IotYKC21_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_YKC21_CMD_LOGIN_RSP:				    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][0];   break;
        case IOT_YKC21_CMD_HEARTBEAT_RSP:			    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][1];   break;
        case IOT_YKC21_CMD_BILLMODE_VERIFY_RSP:		    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][2];   break;
        case IOT_YKC21_CMD_BILLMODE_MUTIRATE_RSP:		pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][3];   break;
        case IOT_YKC21_CMD_CALL_REALDATA:			    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][4];   break;
        case IOT_YKC21_CMD_REMOTE_STOP_CHARGE:		    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][5];   break;
        case IOT_YKC21_CMD_ORDER_RECORD_RSP:		    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][6];   break;
        case IOT_YKC21_CMD_UPDATE_ACCOUNT_MONEY:	    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][7];   break;
        case IOT_YKC21_CMD_FAULT_RSP:				    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][8];   break;
        case IOT_YKC21_CMD_FAULTREST_RSP:			    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][9];   break;
        case IOT_YKC21_CMD_Call_RECORD:			        pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][10];  break;
        case IOT_YKC21_CMD_SET_POWERCHANG:			    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][11];  break;
        case IOT_YKC21_CMD_SYNC_TIME:				    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][12];  break;
        case IOT_YKC21_CMD_SET_BILLMODE_MULTIRATE:	    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][13];  break;
        case IOT_YKC21_CMD_SET_QRCODE:				    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][14];  break;
        case IOT_YKC21_CMD_SET_PARAM:				    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][15];  break;
        case IOT_YKC21_CMD_SET_POWERDEFAULT_MAX:	    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][16];  break;
        case IOT_YKC21_CMD_REBOOT:					    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][17];  break;
        case IOT_YKC21_CMD_SET_FTP:					    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][18];  break;
        case IOT_YKC21_CMD_SET_KEY:					    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][19];  break;
        case IOT_YKC21_CMD_PILE_START_CHARGE_RSP:	    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][20];  break;
        case IOT_YKC21_CMD_REMOTE_START_CHARGE:		    pRecvCtrl = &pIotYKC21Ctx->stRecvCtrl[port][21];  break;

        default: break;
    }
    return pRecvCtrl;
}

uint8_t IotYKC21_CompareRecordOrderNum(uint8_t *record, uint8_t *pCompara, uint16_t paraSize)
{
    MSNvmOrderInfo_Struct *pOrderInfo = (MSNvmOrderInfo_Struct *)record;
    MSNvmYKC21OrderInfo_Struct *pYKC21OrderInfo = &pOrderInfo->platOrderInfo.stYKC21OrderInfo;
    uint8_t ret = FALSE;

    if(0 == memcmp(pYKC21OrderInfo->orderTransactionNum, pCompara, paraSize))
    {
        ret = TRUE;
    }

    return ret;
}

static void IotYKC21_CycleReportRealData(void)
{
    uint32_t realDataReportCycle;
    uint8_t port;
    uint8_t curGunState = 0;
    uint8_t curGunConnectState = 0;
    uint8_t realDataReportFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunState = IotYKC21_GetGunState(port);
        curGunConnectState = AswChargeIf_CheckGunConnected(port);

        if (pIotYKC21Ctx->lastGunState[port] != curGunState)
        {
            realDataReportFlag = TRUE;
        }

        if (pIotYKC21Ctx->lastGunConnectState[port] != curGunConnectState)
        {
            realDataReportFlag = TRUE;
        }

        realDataReportCycle = (AswMonitor_IsOrderIdle(port) != TRUE) ? IOTYKC21_CFG_CHARGING_REALDATA_CYCLE : IOTYKC21_CFG_IDLE_REALDATA_CYCLE;
       
        if (Common_JudgeTimeoutMs(pIotYKC21Ctx->realDataReportTick[port], realDataReportCycle) == TRUE)
        {
            realDataReportFlag = TRUE;
        }

        if (realDataReportFlag == TRUE)
        {
            realDataReportFlag = FALSE;
            pIotYKC21Ctx->lastGunState[port] = curGunState;
            pIotYKC21Ctx->lastGunConnectState[port] = curGunConnectState;
            pIotYKC21Ctx->realDataReportTick[port] = Common_GetSystick();

            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_REPORT_REALDATA, TRUE);
        
        }
    }
}

static void IotYKC21_CycleDetectUnreporteRecord(void)
{
    uint8_t port = 0;
    uint8_t recordSendFlag = FALSE;

    if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) > 0)
    {
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ) ||
                Common_GetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, IOT_YKC21_CMD_ORDER_RECORD_RSP))
            {
                recordSendFlag = TRUE;
                break;
            }
        }

        if (recordSendFlag == FALSE)
        {
            if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord, (uint8_t *)&pIotYKC21Ctx->stOrderInfo, 
                sizeof(MSNvmOrderInfo_Struct), &pIotYKC21Ctx->time))
            {
                port = pIotYKC21Ctx->stOrderInfo.port;

               if (port >= SYSCFG_CFG_GUN_NUM || 
                    pIotYKC21Ctx->stOrderInfo.protocolType != eAswPlatCardType_YKC21 ||
                    pIotYKC21Ctx->stOrderInfo.orderSaveState != ASWMONITOR_ORDER_SAVE_STOP)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotYKC21Ctx->time);
                }
                else
                {
                    Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_MULTI_ORDER_RECORD_REQ, TRUE);
                }
            }
        }
    }
}

static void IotYKC21_UpError(void)
{
    uint8_t port = 0;
    uint8_t index = 0;
    uint8_t error_mapnumber = 0;
    uint8_t error_Exit = FALSE;
    static uint32_t erro_old_version[SYSCFG_CFG_GUN_NUM]= {0};
    uint32_t erro_now_version[SYSCFG_CFG_GUN_NUM] = {0};
    static uint64_t ErrStatus[SYSCFG_CFG_GUN_NUM] = {0}; /* 可记录64个错误类型 */

    error_mapnumber = (ARRAY_SIZE(Iot_Ykc21Error_map) > 64) ? ARRAY_SIZE(Iot_Ykc21Error_map) : 64;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {

        if (TRUE != Common_GetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_FAULT_REQ))
        {
            /* 故障产生或取消 */
            erro_now_version[port] = AswErrHandle_GetErrStatusVersion(port);
            if (erro_old_version[port] != erro_now_version[port])
            {
                erro_old_version[port] = erro_now_version[port];
                /* 循环查找变化故障 */
                for (index = 0; index < error_mapnumber; index++)
                {
                    error_Exit = AswErrHandle_CheckErrExit(port, Iot_Ykc21Error_map[index].err_localtype);

                    if ((ErrStatus[port] & (1 << index)) != error_Exit && eIotYKC21ErrorState_Null != Iot_Ykc21Error_map[index].err_plattype)
                    {
                        if (TRUE == error_Exit)
                        {
                            ErrStatus[port] |= (1 << index); /* 置1 */

                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorAppearTime = SSTM_GetSecTimestamp();
                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorAppearType = Iot_Ykc21Error_map[index].err_plattype;
                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorAppearId = Iot_Ykc21Error_map[index].err_codeid;

                            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_FAULT_REQ, TRUE);
                            IOTYKC21_CFG_LogPrint("[枪：%d]故障发生时间戳[%d],云快充2.1上报故障编码[0x%04x]\r\n", port, pIotYKC21Ctx->stProtoData[port].erroInfo.errorAppearTime, Iot_Ykc21Error_map[index].err_codeid);
                        }
                        else
                        {
                            ErrStatus[port] &= ~(1 << index); /* 置0 */

                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorDisppearTime = SSTM_GetSecTimestamp();
                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorDisppearType = Iot_Ykc21Error_map[index].err_plattype;
                            pIotYKC21Ctx->stProtoData[port].erroInfo.errorDisppearId = Iot_Ykc21Error_map[index].err_codeid;

                            Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_FAULTREST_REQ, TRUE);
                            IOTYKC21_CFG_LogPrint("[枪：%d]故障取消时间戳[%d],云快充2.1上报故障编码[0x%04x]\r\n", port, pIotYKC21Ctx->stProtoData[port].erroInfo.errorDisppearTime, Iot_Ykc21Error_map[index].err_codeid);
                        }

                        break;
                    }
                }
            }
        }
    }
}
static void IotYKC21_DetectPowerLimit(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    uint32_t currentTimeStamp = SSTM_GetSecTimestamp();
    uint8_t port = 0;
    uint8_t adjustFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        adjustFlag = FALSE;

        if (0x03 != IotYKC21_GetGunState(port))
        {
            IotYKC21_SetPowerControl(port, SYSCFG_CFG_MAX_OUTPUT_POWER);
            pIotYKC21Ctx->stProtoData[port].powerLimitFlag = FALSE;
            continue;
        }

        /* 优先功率修改策略 */
        if (pIotYKC21Ctx->stProtoData[port].powerLimitFlag == TRUE)
        {
            if (currentTimeStamp <= pIotYKC21Ctx->stProtoData[port].powerlimitEndTimeStamp)
            {
                IotYKC21_SetPowerControl(port, pIotYKC21Ctx->stProtoData[port].platLimitPower);
                adjustFlag = TRUE;
            }
            else
            {
                pIotYKC21Ctx->stProtoData[port].powerLimitFlag = FALSE;
            }
        }
        /* 默认最大功率策略 */
        else if (pPlatInfo->defaultMaxPowerLimitFlag[port] == TRUE)
        {
            if (currentTimeStamp >= pPlatInfo->deaultMaxPowerStartTimeStamp[port] && 
                currentTimeStamp <= pPlatInfo->deaultMaxPowerEndTimeStamp[port])
            {
                IotYKC21_SetPowerControl(port, pPlatInfo->defaultMaxPower[port]);
                adjustFlag = TRUE;
            }
            else if (currentTimeStamp > pPlatInfo->deaultMaxPowerEndTimeStamp[port])
            {
                pPlatInfo->defaultMaxPowerLimitFlag[port] = FALSE;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
            }
            else
            {}
        }

        /* 未触发功率限制，设置为最大功率 */
        if (adjustFlag == FALSE)
        {
            IotYKC21_SetPowerControl(port, SYSCFG_CFG_MAX_OUTPUT_POWER);
        }
    }
}

static void IotYKC21_DelayRefreshRSAKey(void)
{
    uint8_t gunNo = 0;
    uint8_t refreshFlag = TRUE;

    if (TRUE == pIotYKC21Ctx->rsaPublicKeyRefreshFlag)
    {
        if (TRUE == Common_JudgeTimeoutMs(pIotYKC21Ctx->rsaPubicKeyDelayRefreshTick, 2000))
        {
            if (pIotYKC21Ctx->rsaPubicKeyWaitIdleRefreshFlag == TRUE)
            {
                for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
                {
                    if (0x02 != IotYKC21_GetGunState(gunNo)) 
                    {
                        refreshFlag = FALSE;
                        break;
                    }
                }
            }

            if (refreshFlag == TRUE)
            {
                pIotYKC21Ctx->rsaPubicKeyWaitIdleRefreshFlag = FALSE;
                pIotYKC21Ctx->rsaPublicKeyRefreshFlag = FALSE;
                /* 执行重连 */
                IotYKC21_OfflineHandle();
            }
        }
    }
}

static void IotYKC21_CycleDetect(void)
{
    /* 实时报文传递 */
    IotYKC21_CycleReportRealData();         
    /* 记录上报 */
    IotYKC21_CycleDetectUnreporteRecord(); 
    /* 故障上报处理 */
    IotYKC21_UpError();
    /* RSA延迟更新密钥处理 */
    IotYKC21_DelayRefreshRSAKey(); 
}

static void IotYKC21_FunDeal(void)
{
    /* 功率调节 */
    IotYKC21_DetectPowerLimit(); 
}

static void IotYKC21_WSInitHandle(void)
{
    uint8_t port = 0;
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
    
    // 检测长度超过128，默认数值
    if (pPlatInfo->rsa_Keylength > 128)
    {
        pPlatInfo->rsa_Keylength = 128;
        memcpy(pPlatInfo->rsa_Key, Default_RsaKey, 128);
        memcpy(pPlatInfo->token, Default_TokenStr, 14);
    }

    pIotYKC21Ctx->eWorkState = eIOTYKC21WorkState_Offline;
}


static void IotYKC21_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct * pParam =  AswPlatM_GetPlatParamPtr();

    pIotYKC21Ctx->rsaPubicKeyWaitIdleRefreshFlag = FALSE;
    pIotYKC21Ctx->rsaPublicKeyRefreshFlag = FALSE;
    pIotYKC21Ctx->loginSucc = FALSE;
    pIotYKC21Ctx->queueBusyFlag = FALSE;
    pIotYKC21Ctx->waitQueueIdleTick = 0;

    pIotYKC21Ctx->sendIndex = 0;
    pIotYKC21Ctx->sendPort = 0;    
    pIotYKC21Ctx->reqSeq = 0;

    memset(pIotYKC21Ctx->realDataReportTick, 0x00, sizeof(pIotYKC21Ctx->realDataReportTick));
    memset(pIotYKC21Ctx->lastGunState, 0x00, sizeof(pIotYKC21Ctx->lastGunState));
    memset(pIotYKC21Ctx->lastGunConnectState, 0x00, sizeof(pIotYKC21Ctx->lastGunConnectState));

    memset(pIotYKC21Ctx->stSendCtrl, 0x00, sizeof(pIotYKC21Ctx->stSendCtrl));
    memset(pIotYKC21Ctx->stRecvCtrl, 0x00, sizeof(pIotYKC21Ctx->stRecvCtrl));

    FrameQueue_Reset(pIotYKC21Ctx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotYKC21Ctx->pileDnBCD, 14);

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotYKC21Ctx->eWorkState = eIOTYKC21WorkState_Login;
}


static void IotYKC21_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotYKC21Ctx->eWorkState = eIOTYKC21WorkState_Normal;
        Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, 0, IOT_YKC21_CMD_LOGIN_REQ, TRUE);
    }
}

static void IotYKC21_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotYKC21_OfflineHandle();
    }
    else
    {
        if (pIotYKC21Ctx->loginSucc == TRUE)
        {
            IotYKC21_CycleDetect();
        }

        /* 功率调节 */
        IotYKC21_DetectPowerLimit();

        IotYKC21_UpCtrlSendDeal();

        IotYKC21_UpCtrlRecvDeal();

        IotYKC21_TimeoutDetect();
    }
}


static IotYKC21StopReason_Enum Iot_ConverStopReason(AswErrorType_Enum errType)
{
    uint8_t index = 0;
    IotYKC21StopReason_Enum eStopReason = eIotYKC21StopReason_NoExpectedErr;

    const struct
    {
        AswErrorType_Enum errType;
        IotYKC21StopReason_Enum stopReason;
    }stopReasonMap[] = 
    {
        {eErr_CpVoltAbnor,        eIotYKC21StopReason_CpVoltAbnor},
        {eErr_CpGroundFault,      eIotYKC21StopReason_CpGroundFault},
        {eErr_PEBreakFault,       eIotYKC21StopReason_PEBreakFault},
        {eErr_EmergencyStop,      eIotYKC21StopReason_EmergencyStop},
        {eErr_InputLineReversed,  eIotYKC21StopReason_OtherErr},
        {eErr_LeakageCurrErr,     eIotYKC21StopReason_LeakageCurrErr},
        {eErr_ShortCircleErr,     eIotYKC21StopReason_ShortCut},
        {eErr_RCDSelfcheckErr,    eIotYKC21StopReason_OtherErr},

        {eErr_AphaseInputOverVol, eIotYKC21StopReason_VoltageErr},
        {eErr_AphaseInputLessVol, eIotYKC21StopReason_VoltageErr},
        {eErr_OutputOverCurr,     eIotYKC21StopReason_OverCurr},

        {eErr_JcqMaloperation,    eIotYKC21StopReason_JcqMaloperation},
        {eErr_JcqSynechiaFault,   eIotYKC21StopReason_JcqSynechiaFault},
        {eErr_ReaderCommErr,      eIotYKC21StopReason_OtherErr},
        {eErr_MeterCommErr,       eIotYKC21StopReason_MeterCommErr},
        {eErr_EnvOverTempErr,     eIotYKC21StopReason_TempErr},
        {eErr_GunOverTempErr,     eIotYKC21StopReason_GunTempErr},        
        {eErr_POverTempErr,       eIotYKC21StopReason_TempErr},  
        
        {eErr_DatabaseErr,        eIotYKC21StopReason_DataBaseErr},       
        {eErr_MeterCalcErr,       eIotYKC21StopReason_MeterCalcErr},     

        {eErr_ChgStartTimeout,    eIotYKC21StopReason_StartTimeout}, 
        
        {eErr_DiodeStop,          eIotYKC21StopReason_DiodeStop},  
        
        {eSrc_LittleCurr,         eIotYKC21StopReason_LittleCurr},   
        {eSrc_S2BreakOff,         eIotYKC21StopReason_CarStop},          
        {eSrc_AppStop,            eIotYKC21StopReason_AppStop},            
        {eSrc_MannulStop,         eIotYKC21StopReason_ManualStop},      
        {eSrc_CardStop,           eIotYKC21StopReason_ManualStop},   
        {eSrc_InsuffBalance,      eIotYKC21StopReason_SumNoEnough},  
        {eSrc_StopbyMoney,        eIotYKC21StopReason_StopByMoney},  
        {eSrc_StopbyTime,         eIotYKC21StopReason_StopByTime},  
        {eSrc_StopbyEnergy,       eIotYKC21StopReason_RechargEnergy}, 
        {eErr_GunDisConn,         eIotYKC21StopReason_GunDisconnect}, 
    };  

    for (index = 0; index < ARRAY_SIZE(stopReasonMap); index++)
    {
        if (errType == stopReasonMap[index].errType)
        {
            eStopReason = stopReasonMap[index].stopReason;
            break;
        }
    }

    return eStopReason;
}

void IotYKC21_SetPowerControl(uint8_t port, uint32_t powerLimit)
{
    if (pIotYKC21Ctx->stProtoData[port].lastSetPower != powerLimit)
    {
        AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, powerLimit);
        pIotYKC21Ctx->stProtoData[port].lastSetPower = powerLimit;
        IOTYKC21_CFG_LogPrint("[枪：%d]:云快充2.1协议, 功率调整为[%d]w\r\n", port, powerLimit);
    }
}

void IotYKC21_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21ParamBillMode_Struct *pYKC21BillMode = &pPrivateParam->stYKC21Param.stBillMode;
    uint8_t periodCount = 0;
    uint8_t index = 0;
    uint16_t startIndex = 0;
	uint16_t stopIndex = 0;

    if (pStandardBillMode != NULL)
    {
        memset(pStandardBillMode, 0x00, sizeof(AswMonitorBillMode_Struct));

        if (pYKC21BillMode->billnum < IOT_YKC21_BILLMODE_RATE_TYPE_MULT && pYKC21BillMode->billnum > 0)
        {
            pStandardBillMode->rateCount = pYKC21BillMode->billnum;
            pStandardBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_MULT;
        }
        else
        {}

        if (pStandardBillMode->rateCount != 0)
        {
            /* 转换计损比例 */
            pStandardBillMode->elecLossRate = pYKC21BillMode->elecLossRate;

            /* 转换费率内容 */
            memcpy(pStandardBillMode->rateElecPrice, pYKC21BillMode->elecPriceRate, pStandardBillMode->rateCount * sizeof(uint32_t));
            memcpy(pStandardBillMode->rateSeverPrice, pYKC21BillMode->servePriceRate, pStandardBillMode->rateCount * sizeof(uint32_t));

            for (index = 0; index < pStandardBillMode->rateCount; index++)
            {
                pStandardBillMode->totalPrice[index] = pStandardBillMode->rateElecPrice[index] + pStandardBillMode->rateSeverPrice[index];
            }

             /* 转换时段内容 */
            for (startIndex = 0; startIndex < MSNVM_YKC21_BILLMIDE_PERIOD_COUNT;startIndex++)
            {

             pStandardBillMode->periodRate[startIndex] = pYKC21BillMode->period_rate[startIndex]-1;
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

void IotYKC21_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    MSNvmYKC21OrderInfo_Struct *pOrderData = &pFlashRecord->stYKC21OrderInfo;
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
        memcpy(&pBuf[dataLen], pOrderData->pileDnBCD, 7);
        dataLen += 7;
        /* 枪号 */
        pBuf[dataLen++] = pOrderData->port + 1;
        /* 开始时间 */
        memcpy(&pBuf[dataLen], pOrderData->startTime, 7);
        dataLen += 7;
        /* 结束时间 */
        memcpy(&pBuf[dataLen], pOrderData->stopTime, 7);
        dataLen += 7;
        /* 电表表号 电表密文 电表协议版本号 加密方式 */
        memset(&pBuf[dataLen], 0, (6 + 34 + 2 + 1));
        dataLen += (6 + 34 + 2 + 1);
        /* 电表总起值 */
        memcpy(&pBuf[dataLen], pOrderData->startMeterVal, 5);
        dataLen += 5;
        /* 电表总止值 */
        memcpy(&pBuf[dataLen], pOrderData->stopMeterVal, 5);
        dataLen += 5;
        /* 总电量 */
        memcpy(&pBuf[dataLen], pOrderData->totalEnergy, 4);
        dataLen += 4;
        /* 总计损电量 */
        memcpy(&pBuf[dataLen], pOrderData->totalLossEnergy, 4);
        dataLen += 4;
        /* 总消费金额 */
        memcpy(&pBuf[dataLen], pOrderData->totalMoney, 4);
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
        /* 费率时段数量 */
        pBuf[dataLen++] = 48;
        /* 48h 分段电量 */
        memcpy(&pBuf[dataLen], pOrderData->time_power, 48 * 4);
        dataLen += (4 * 48);

        pRecordLen[0] = dataLen;
    }
}
uint8_t IotYKC21_GetGunState(uint8_t port)
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

void IotYKC21_OfflineHandle(void)
{
    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotYKC21Ctx->loginSucc = FALSE;
    pIotYKC21Ctx->eWorkState = eIOTYKC21WorkState_Offline;
}

void IotYKC21_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotYKC21Ctx != NULL)
    {
        strcpy(pLinkPara->stTcpPara.ip, pParam->platMainIp);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 2048, 2048, &pIotYKC21Ctx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotYKC21Ctx->frameQueueChannelID;
    }
}


uint8_t IotYKC21_SwipCardCharge(uint8_t port)
{
    uint8_t ret = FALSE;

    if (pIotYKC21Ctx->loginSucc == TRUE)
    {
        if (port < SYSCFG_CFG_GUN_NUM)
        {
            if ((TRUE != Common_GetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_PILE_START_CHARGE_REQ)) &&
                (TRUE != Common_GetRecvTimerEnable(pIotYKC21Ctx->pFuncRecvCtrl, port, IOT_YKC21_CMD_PILE_START_CHARGE_RSP)))
            {
                Common_SetSendEnable(pIotYKC21Ctx->pFuncSendCtrl, port, IOT_YKC21_CMD_PILE_START_CHARGE_REQ, TRUE);
                ret = TRUE;
                IOTYKC21_CFG_LogPrint("[枪：%d]刷卡成功，请求启动充电!\r\n", port);
            }
            else
            {
                IOTYKC21_CFG_LogPrint("[枪：%d]刷卡成功，但是已经有卡在申请启动充电，本次刷卡作废!\r\n", port);
            }
        }
    }

    return ret;
}
 
 /*更新密钥或token*/
uint8_t IotYKC21_RfreshYKC21key(char *YKC21key, uint16_t YKC21key_len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    IOTYKC21_CFG_LogPrint("ykc2.1平台RSA密钥长度变化：[\"%d\"]-->[\"%d\"]\r\n", pPlatInfo->rsa_Keylength, YKC21key_len);
    IOTYKC21_CFG_LogPrint("ykc2.1平台RSA密钥变化：[\"%.128s\"]-->[\"%s\"]\r\n", pPlatInfo->rsa_Key, YKC21key);
    pPlatInfo->rsa_Keylength = YKC21key_len;
    memset(pPlatInfo->rsa_Key,0,128);
    memcpy(pPlatInfo->rsa_Key,YKC21key,pPlatInfo->rsa_Keylength);

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));

    return TRUE;
}


uint8_t IotYKC21_RfreshYKC21token(char *YKC21token,uint16_t YKC21token_len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;
   
    IOTYKC21_CFG_LogPrint("ykc2.1平台token变化：[\"%s\"]-->[\"%s\"]\r\n", pPlatInfo->token, YKC21token);
    memset(pPlatInfo->token, 0, 14);
    memcpy(pPlatInfo->token, YKC21token, YKC21token_len); 

    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
 
    return TRUE;
}

void IotYKC21_PrintfYKC21KeyAndToken(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmYKC21_FlashPlatInfo_Struct *pPlatInfo = &pPrivateParam->stYKC21Param.platinfo;

    if (pIotYKC21Ctx != NULL)
    {
        IOTYKC21_CFG_LogPrint("RSA密钥[%d]：[\"%.128s\"]\r\n",pPlatInfo->rsa_Keylength,pPlatInfo->rsa_Key);
        IOTYKC21_CFG_LogPrint("token：[\"%.14s\"]\r\n", pPlatInfo->token);
    }
}


void IotYKC21_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{ 
    AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    AswMonitorChargeCtrl_Struct *pstChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmYKC21OrderInfo_Struct *pYkcOrder = &pOrderData->platOrderInfo.stYKC21OrderInfo;
    uint8_t index = 0;

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        memset(pYkcOrder, 0x00, sizeof(MSNvmYKC21OrderInfo_Struct));
        memcpy(pYkcOrder->pileDnBCD, pIotYKC21Ctx->pileDnBCD, 7);
        pYkcOrder->port = port;
        memcpy(pYkcOrder->orderTransactionNum, 
               pIotYKC21Ctx->stProtoData[port].curUsedOrderTransactionNum, 
               sizeof(pIotYKC21Ctx->stProtoData[port].curUsedOrderTransactionNum));
    
        //时间戳转换成 CP56Time2a 格式
        Common_TimestampToCp56Time2a(pChargeData->chargeStartTime, &pYkcOrder->startTime[0]);
        Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, &pYkcOrder->stopTime[0]);
    
        memset(& pYkcOrder->startMeterVal[0],0,5);
        Common_Uint32ToFourUint8(&pYkcOrder->startMeterVal[0],pChargeData->startMeterVal);

        memset(& pYkcOrder->stopMeterVal[0],0,5);
        Common_Uint32ToFourUint8(&pYkcOrder->stopMeterVal[0],pChargeData->stopMeterVal);
    
        memcpy(pYkcOrder->dealDate, pYkcOrder->startTime, 7);

        if (pstChargeCtrl->startSrc == ASWMONITOR_ORDER_START_SRC_APP)
        {
            pYkcOrder->dealFlag = 0x01;
        }
        else
        {
            pYkcOrder->dealFlag = 0x02;
            memcpy(pYkcOrder->logicCardNum, pIotYKC21Ctx->stProtoData[port].authCardID, 8);
        }

        pOrderData->orderLen = sizeof(MSNvmYKC21OrderInfo_Struct);
    
        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatCardType_YKC21;
        pOrderData->orderLen = sizeof(MSNvmYKC21OrderInfo_Struct);
        pYkcOrder->stopReason = eIotYKC21StopReason_PowerOff;
    }
 
    Common_TimestampToCp56Time2a(pChargeData->chargeStopTime, &pYkcOrder->stopTime[0]);

    memset(&pYkcOrder->stopMeterVal[0], 0, 5);
    Common_Uint32ToFourUint8(&pYkcOrder->stopMeterVal[0], pChargeData->stopMeterVal);
    Common_Uint32ToFourUint8(&pYkcOrder->totalEnergy[0], pChargeData->totalEnergy);
    Common_Uint32ToFourUint8(&pYkcOrder->totalLossEnergy[0],pChargeData->totalLossEnergy);
    Common_Uint32ToFourUint8(&pYkcOrder->totalMoney[0],pChargeData->totalMoney);

    pYkcOrder->fee_num = 48;
    for (index = 0; index < 48; index++)
    {
        pYkcOrder->time_power[index] = pChargeData->periodElePower[index]; // 48时段电量
    }

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        pYkcOrder->stopReason = Iot_ConverStopReason(pChargeData->eChargeStopReason);
    }
}

void IotYKC21_InitMemory(void)
{
    pIotYKC21Ctx = (IotYKC21Ctx_Struct *)malloc(sizeof(IotYKC21Ctx_Struct));
    if (pIotYKC21Ctx != NULL)
    {
        memset(pIotYKC21Ctx, 0, sizeof(IotYKC21Ctx_Struct));
    }

    pIotYKC21Ctx->pFuncSendCtrl = IotYKC21_GetSendCtrl;
    pIotYKC21Ctx->pFuncRecvCtrl = IotYKC21_GetRecvCtrl;
}

void IotYKC21_MainFunction(void)
{
    switch (pIotYKC21Ctx->eWorkState)
    {
        case eIOTYKC21WorkState_Init:
        {
            IotYKC21_WSInitHandle();
            break;
        }
        case eIOTYKC21WorkState_Offline:
        {
            IotYKC21_WSOfflineHandle();
            break;
        }
        case eIOTYKC21WorkState_Login:
        {
            IotYKC21_WSLoginHandle();
            break;
        }
        case eIOTYKC21WorkState_Normal:
        {
            IotYKC21_WSNormalHandle();
            break;
        }
        default:
        {
            pIotYKC21Ctx->eWorkState = eIOTYKC21WorkState_Init;
        }
    }
}





