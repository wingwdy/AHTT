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
#include "FrameQueue.h"
#include "Asw_PlatM.h"
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
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t IotXDT_QueryAttachCredential_ITEM811(uint8_t ucPort, void *pBuf);


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotXDTCtx_Struct *pIotXDTCtx;

static IotXDTSendCtrl_Struct  c_IotXDTSendCtrlTable[] = 
{	
	[0] = {
		.topic = IOT_XDT_TOPIC_PROVISION_REQUEST,
		.cmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_QueryAttachCredential_ITEM811,
		.matchCmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP,
		.cMeaning = "请求获取凭据",
	}, 

/*
	[1] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_TIMESYNC,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = (2 * 60 * 60 * 1000),
		.pSendFunc = IotXDT_ReqeustTimeSync_ITEM821,
		.matchCmd = IOT_XDT_CMD_REQUEST_TIMESYNC_RSP,
	},

	[2] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_LINK,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestLink_ITEM823,
		.matchCmd = IOT_XDT_CMD_REQUEST_LINK_RSP,
	},

	[3] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_PILESTATE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 20 * 60 * 1000,
		.pSendFunc = IotXDT_ReportPileState_ITEM841,
		.matchCmd = IOT_XDT_CMD_PILESTATE_RSP,
	},

	[4] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_RATEMODE_SET_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportRateMode_ITEM835,
		.matchCmd = IOT_XDT_CMD_RATEMODE_SET,
	},

	[5] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportRateModeSetResponseEvent_ITEM836,
		.matchCmd = IOT_XDT_CMD_NULL,
	},

	[6] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_RATEMODE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_QueryRateModeRsp_ITEM837,
		.matchCmd = IOT_XDT_QUERY_RATEMODE,
	},

	[7] = {
		.topic = IOT_XDT_PRE_TOPIC_V2A_REQUEST,
		.cmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestOTAAttribute_ITEM882,
		.matchCmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP,
	},

	[8] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_CALL_REALDATA_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_CallRealDataResponse_ITEM847,
		.matchCmd = IOT_XDT_CMD_CALL_REALDATA,
	},

	[9] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_REQUEST_RATEMODE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestRateMode_ITEM831,
		.matchCmd = IOT_XDT_REQUEST_RATEMODE_RSP,
	},

	[10] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CMD_ERRINFO,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPileErrInfo_ITEM843,
		.matchCmd = IOT_XDT_CMD_ERRINFO_RSP,
	},

	[11] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_REQUEST_ERCODE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestErCode_ITEM855,
		.matchCmd = IOT_XDT_REQUEST_ERCODE_RSP,
	},

	[12] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_SET_ERCODE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportErCodeSetResponse_ITEM858,
		.matchCmd = IOT_XDT_SET_ERCODE,
	},

	[13] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_SET_ERCODE_RSP_EVENT,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportErCodeSetResponseEvent_ITEM859,
		.matchCmd = IOT_XDT_CMD_NULL,
	},

	[14] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_START_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStartReponse_ITEM862,
		.matchCmd = IOT_XDT_CHARGE_START,
	},

	[15] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_START_EVNET,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStartEvent_ITEM863,
		.matchCmd = IOT_XDT_CMD_NULL,
	},	

	[16] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_START_EVNETA,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStartEvent_ITEM86A,
		.matchCmd = IOT_XDT_CMD_NULL,
	},	

	[17] = {
		.topic = IOT_XDT_PRE_TOPIC_TSDATA,
		.cmd = IOT_XDT_CMD_PILEDATA,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPileData_ITEM845,
		.matchCmd = IOT_XDT_CMD_NULL,
	},

	[18] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_ERCODE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportQueryErCodeRsp_ITEM85B,
		.matchCmd = IOT_XDT_QUERY_ERCODE,
	},

	[19] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_STOP_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStopReponse_ITEM865,
		.matchCmd = IOT_XDT_CHARGE_STOP,
	},

	[20] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_STOP_EVNET,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeStopEvent_ITEM866,
		.matchCmd = IOT_XDT_CMD_NULL,
	},	

	[21] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_CHARGE_RECORD,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportChargeRecord_ITEM8613,
		.matchCmd = IOT_XDT_CHARGE_RECORD_RSP,
	},	

	[22] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_CHARGE_RECORD_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportQueryChargeRecordRsp_ITEM8616,
		.matchCmd = IOT_XDT_QUERY_CHARGE_RECORD,
	},		

	[23] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_PWRCTRL_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPowerControlResponse_ITEM8612,
		.matchCmd = IOT_XDT_CHARGE_PWRCTRL,
	},		

	[24] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportContinueChargeResponse_ITEM868,
		.matchCmd = IOT_XDT_CHARGE_CONTINUE_CHARGE,
	},	

	[25] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_REQUEST_CARDAUTH,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestAuth_ITEM851,
		.matchCmd = IOT_XDT_REQUEST_CARDAUTH_RSP,
	},	

	[26] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmd = IOT_XDT_SET_CATEGORY,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportCategory_ITEM853,
		.matchCmd = IOT_XDT_SET_CATEGORY_RSP,
	},	

	[27] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_QUERY_BOARDINFO_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportQueryBoardInfoRsp_ITEM872,
		.matchCmd = IOT_XDT_QUERY_BOARDINFO,
	},	

	[28] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_PARA_SET_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPataSetRsp_ITEM874,
		.matchCmd = IOT_XDT_PARA_SET,
	},	

	[29] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_PARA_QUERY_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportPataGetRsp_ITEM876,
		.matchCmd = IOT_XDT_PARA_QUERY,
	},

	[30] = {
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmd = IOT_XDT_CMD_RESTART_SET_RSP,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.sendCycle = 0,
		.pSendFunc = IotXDT_RequestResetResponse_ITEM826,
		.matchCmd = IOT_XDT_CMD_RESTART_SET,
	},
			
	[31] = {
		.topic = IOT_XDT_PRE_TOPIC_V2T,
		.cmd = IOT_XDT_CMD_FWWARE_STATE,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.sendCycle = 0,
		.pSendFunc = IotXDT_ReportFwState_ITEM886,
		.matchCmd = IOT_XDT_CMD_NULL,
	},	
 */   
};






/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotXDT_ReportCycleCheck(uint8_t port, uint32_t cmd, uint32_t	sendCyc)
{
	uint32_t startTick = Common_GetSendTick(pIotXDTCtx->pFuncSendCtrl, port, cmd);
	uint8_t sendImmdFlag = Common_GetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, cmd);
	uint8_t retFlag = FALSE;

	if (TRUE == sendImmdFlag) 
	{
		retFlag = TRUE;
	}
	else
	{
		if (Common_JudgeTimeoutMs(startTick, sendCyc))
		{
			retFlag = TRUE;
		}
	}
	
	return retFlag;
}


static uint16_t IotXDT_QueryAttachCredential_ITEM811(uint8_t ucPort, void *pBuf)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
	cJSON *cRoot = NULL;
	char cTempString[32 + 1] = { 0 };
	char *pJson = NULL;
	uint16_t dataLen = 0;

	cRoot = cJSON_CreateObject();
	IOT_XDT_CheckObjIsNull(cRoot, 0);

	snprintf(cTempString, 32, "%s-%s", pPlatInfo->cOperator, pIotXDTCtx->platDn);
	cJSON_AddStringToObject(cRoot, "deviceName", cTempString);
	cJSON_AddStringToObject(cRoot, "productKey", pPlatInfo->cProductKey);
	cJSON_AddStringToObject(cRoot, "productSecret", pPlatInfo->cProductSecret);
	cJSON_AddStringToObject(cRoot, "credentialsType", IOT_XDT_CREDENTIAL_TYPE);
	
	pJson = cJSON_Print(cRoot);
	IOT_XDT_CheckJsonPrint(cRoot, pJson, 0);
	
	dataLen = strlen(pJson);
	memcpy(pBuf, pJson, dataLen);
	cJSON_Delete(cRoot);
	myFree(pJson);
	return dataLen;
}

static void IotXDT_PackTopic(char *pInTopic, char *pOutTopic, uint32_t rpcSeq)
{
	char *pStart = NULL;

	if (pInTopic != NULL && pOutTopic != NULL)
	{
		if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2R_REQUEST, strlen(IOT_XDT_PRE_TOPIC_V2R_REQUEST)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2R_RESPONSE, strlen(IOT_XDT_PRE_TOPIC_V2R_RESPONSE)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else if (NULL != Common_SearchData((uint8_t *)pInTopic, strlen(pInTopic), IOT_XDT_PRE_TOPIC_V2A_REQUEST, strlen(IOT_XDT_PRE_TOPIC_V2A_REQUEST)))
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s%d", pInTopic, rpcSeq);
		}
		else
		{
			snprintf(pOutTopic, IOT_XDT_TOPIC_LEN + 1, "%s", pInTopic);	
		}
	}
}

uint32_t IotXDT_GetSendCmdSendCycle(uint32_t cmd)
{
	IotXDTSendCtrl_Struct *pSendCtrl = NULL;
	uint32_t retCycle = 0;
	uint8_t index = 0;

	for (index = 0; index < ARRAY_SIZE(c_IotXDTSendCtrlTable); index++)
	{
		pSendCtrl = &c_IotXDTSendCtrlTable[index];
		
		if (pSendCtrl->cmd == cmd)
		{
			retCycle = pSendCtrl->sendCycle;
			break;
		}
	}

	return retCycle;
}

void IotXDT_UpCtrlSendDeal(void)
{
	IotXDTSendCtrl_Struct *pSendCtrl = NULL;
	uint8_t port = 0;
	uint8_t index = 0;
	char cTopic[IOT_XDT_TOPIC_LEN + 1] = { 0 };
	uint16_t dataLen = 0;
	uint32_t rpcSeq = 0;
	uint8_t txbuf[IOT_XDT_TXRX_BUFFER_SIZE] = {0};

	if (pIotXDTCtx->queueBusyFlag == TRUE)
	{
		if (Common_JudgeTimeoutMs(pIotXDTCtx->waitQueueIdleTick, 500))
		{
			pIotXDTCtx->queueBusyFlag = FALSE;
		}
	}
	else
	{
		while (1)
		{
			if (pIotXDTCtx->sendIndex < ARRAY_SIZE(c_IotXDTSendCtrlTable))
			{
				index = pIotXDTCtx->sendIndex;
				port = pIotXDTCtx->sendPort;
					
				pSendCtrl = &c_IotXDTSendCtrlTable[index];

				if (Common_GetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd) == TRUE &&
			        IotXDT_ReportCycleCheck(port, pSendCtrl->cmd, pSendCtrl->sendCycle) == TRUE)
				{
					if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
					{
						rpcSeq = pIotXDTCtx->reqSeq;
						
						if (pSendCtrl->matchCmd != IOT_XDT_CMD_NULL)
						{
                            Common_SetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, rpcSeq);
						}
						
						pIotXDTCtx->reqSeq++;
					}
					else
					{
                        rpcSeq = Common_GetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd);
					}

					IotXDT_PackTopic(pSendCtrl->topic, cTopic, rpcSeq);

					if (NULL != pSendCtrl->pSendFunc)
					{
						dataLen = pSendCtrl->pSendFunc(port, txbuf);
					}

					if (dataLen > 0)
					{
						pIotXDTCtx->queueBusyFlag = TRUE;
						pIotXDTCtx->waitQueueIdleTick = Common_GetSystick();

                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotXDTCtx->frameQueueChannelID, cTopic, strlen(cTopic), txbuf, dataLen))
                        {
                            if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
                            {
                                pIotXDTCtx->reqSeq--;
                            }

                            IOTXDT_CFG_LogPrint("FrameQueue_PushTx fail...[cmd: 0x%03X][dataLen = %d]\r\n", pSendCtrl->cmd, dataLen);
                            break;
                        }
						
						extern size_t xPortGetFreeHeapSize( void );
						IOTXDT_CFG_LogPrint("Remaining heap size: %zu bytes\n", xPortGetFreeHeapSize());
                        IOTXDT_CFG_LogPrint("topic: %s\r\n, ", cTopic);
                        IOTXDT_CFG_LogPrint("[枪：%d]发送[cmd: 0x%04X, %s][%d]\r\n", port, pSendCtrl->cmd, pSendCtrl->cMeaning, dataLen);
                        
                        Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, Common_GetSystick());
                        
                        if (pSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pSendCtrl->cmd, FALSE);
                        }

                        if (pSendCtrl->cmdType == IOT_XDT_CMDTYPE_REQUSET)
                        {
                            if (pSendCtrl->matchCmd != IOT_XDT_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotXDTCtx->pFuncRecvCtrl, port, pSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }
					}
				}
			}

			pIotXDTCtx->sendIndex++;

			if (pIotXDTCtx->sendIndex >= ARRAY_SIZE(c_IotXDTSendCtrlTable))
			{
				pIotXDTCtx->sendIndex = 0;
				pIotXDTCtx->sendPort++;

				if (pIotXDTCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
				{
					pIotXDTCtx->sendPort = 0;
					break;
				}
			}
			
			if (pIotXDTCtx->queueBusyFlag == TRUE)
			{
				break;
			}
		}
	}
}




















