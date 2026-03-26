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
#include "Asw_PlatM.h"
#include "SS_Tm.h"
#include "FrameQueue.h"
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
static uint8_t IotXDT_RecvCredentialRsp_ITEM812(uint8_t u8Port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvTimeSyncRsp_ITEM822(uint8_t u8Port, uint8_t *r_data, uint16_t len);



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotXDTCtx_Struct *pIotXDTCtx;

static IotXDTRecvCtrl_Struct c_IotXDTRecvctrlProvisonTable[] = 
{
	[0] ={
		.cmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP,
		.matchStr = "credentialsType",
		.pRecvParse = IotXDT_RecvCredentialRsp_ITEM812,
		.maxTimeout = 10 * 1000,
		.maxTryCnt = 3,
		.matchCmd = IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL,
	},
};

static IotXDTRecvCtrl_Struct c_IotXDTRecvctrlV2aTable[] = 
{
	[0] ={
		.cmd = IOT_XDT_CMD_OTA_ATTRIBUTE_SET,
		.matchStr = "fw_tag",
		.pRecvParse = NULL,
//		.pRecvParse = IotXDT_RecvOTAAttribute_ITEM881,
		.maxTimeout = 0,
		.maxTryCnt = 0,
		.matchCmd = IOT_XDT_CMD_NULL,
	},
};

static IotXDTRecvCtrl_Struct c_IotXDTRecvctrlV2rReqTable[] = 
{
	[0] ={
		.cmd = IOT_XDT_CMD_SET_RESTART,
		.matchStr = "reboot",
//		.pRecvParse = IotXDT_RecvSetReboot_ITEM825,
		.pRecvParse = NULL,
		.maxTimeout = 0,
		.maxTryCnt = 0,
		.matchCmd = IOT_XDT_CMD_SET_RESTART_RSP,
	},

	// [1] ={
	// 	.cmd = IOT_XDT_CMD_RATEMODE_SET,
	// 	.matchStr = "rate_set",
	// 	.pRecvParse = IotXDT_RecvRateModeSet_ITEM834,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CMD_RATEMODE_SET_RSP,
	// },

	// [2] ={
	// 	.cmd = IOT_XDT_QUERY_RATEMODE,
	// 	.matchStr = "rate_get",
	// 	.pRecvParse = IotXDT_RecvQueryRateMode_ITEM833,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_RATEMODE_RSP,
	// },

	// [3] ={
	// 	.cmd = IOT_XDT_CMD_CALL_REALDATA,
	// 	.matchStr = "req_real_data",
	// 	.pRecvParse = IotXDT_RecvCallRealData_ITEM846,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CMD_CALL_REALDATA_RSP,
	// },

	// [4] ={
	// 	.cmd = IOT_XDT_SET_ERCODE,
	// 	.matchStr = "qrcode_set",
	// 	.pRecvParse = IotXDT_RecvSetQrCode_ITEM857,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_SET_ERCODE_RSP,
	// },

	// [5] ={
	// 	.cmd = IOT_XDT_QUERY_ERCODE,
	// 	.matchStr = "qrcode_req",
	// 	.pRecvParse = IotXDT_RecvQueryQrCode_ITEM85A,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_ERCODE_RSP,
	// },

	// [6] ={
	// 	.cmd = IOT_XDT_CHARGE_START,
	// 	.matchStr = "start_cmd",
	// 	.pRecvParse = IotXDT_RecvChargeStart_ITEM861,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_START_RSP,
	// },	

	// [7] ={
	// 	.cmd = IOT_XDT_CHARGE_STOP,
	// 	.matchStr = "stop_cmd",
	// 	.pRecvParse = IotXDT_RecvChargeStop_ITEM864,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_STOP_RSP,
	// },

	// [8] ={
	// 	.cmd = IOT_XDT_QUERY_CHARGE_RECORD,
	// 	.matchStr = "record_get",
	// 	.pRecvParse = IotXDT_RecvQueryChargeRecord_ITEM8615,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_CHARGE_RECORD_RSP,
	// },	

	// [9] ={
	// 	.cmd = IOT_XDT_CHARGE_PWRCTRL,
	// 	.matchStr = "control_power",
	// 	.pRecvParse = IotXDT_RecvPowerCtrl_ITEM8611,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_PWRCTRL_RSP,
	// },	

	// [10] ={
	// 	.cmd = IOT_XDT_CHARGE_CONTINUE_CHARGE,
	// 	.matchStr = "topup_cmd",
	// 	.pRecvParse = IotXDT_RecvContinueCharge_ITEM867,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP,
	// },	

	// [11] ={
	// 	.cmd = IOT_XDT_QUERY_BOARDINFO,
	// 	.matchStr = "control_info_get",
	// 	.pRecvParse = IotXDT_RecvQueryBoardInfo_ITEM871,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_BOARDINFO_RSP,
	// },	

	// [12] ={
	// 	.cmd = IOT_XDT_PARA_SET,
	// 	.matchStr = "param_set",
	// 	.pRecvParse = IotXDT_RecvParaSet_ITEM873,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_PARA_SET_RSP,
	// },	

	// [13] ={
	// 	.cmd = IOT_XDT_PARA_QUERY,
	// 	.matchStr = "param_get",
	// 	.pRecvParse = IotXDT_RecvParaGet_ITEM875,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_PARA_QUERY_RSP,
	// },	
};

static IotXDTRecvCtrl_Struct c_IotXDTRecvctrlV2rResTable[] = 
{
	[0] ={
		.cmd = IOT_XDT_CMD_REQUEST_TIMESYNC_RSP,
		.matchStr = "ts",
		.pRecvParse = IotXDT_RecvTimeSyncRsp_ITEM822,
		.maxTimeout = 10 * 1000,
		.maxTryCnt = 3,
		.matchCmd = IOT_XDT_CMD_REQUEST_TIMESYNC,
	},
	
	[1] ={
		.cmd = IOT_XDT_CMD_REQUEST_LINK_RSP,
		.matchStr = "ts",
//		.pRecvParse = IotXDT_RecvLinkRsp_ITEM824,
		.pRecvParse = NULL,
		.maxTimeout = 10 * 1000,
		.maxTryCnt = 3,
		.matchCmd = IOT_XDT_CMD_REQUEST_LINK,
	},

	// [2] ={
	// 	.cmd = IOT_XDT_REQUEST_RATEMODE_RSP,
	// 	.matchStr = "typeRule",
	// 	.pRecvParse = IotXDT_RecvRequestRateModeRsp_ITEM832,
	// 	.maxTimeout = 10 * 1000,
	// 	.maxTryCnt = 0xFFFF,
	// 	.matchCmd = IOT_XDT_REQUEST_RATEMODE,
	// },

	// [3] ={
	// 	.cmd = IOT_XDT_CMD_PILESTATE_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvPlieStateRsp_ITEM842,
	// 	.maxTimeout = 5 * 1000,
	// 	.maxTryCnt = 21,
	// 	.matchCmd = IOT_XDT_CMD_PILESTATE,
	// },		

	// [4] ={
	// 	.cmd = IOT_XDT_CMD_ERRINFO_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvPlieErrInfoRsp_ITEM844,
	// 	.maxTimeout = 5 * 1000,
	// 	.maxTryCnt = 21,
	// 	.matchCmd = IOT_XDT_CMD_ERRINFO,
	// },	

	// [5] ={
	// 	.cmd = IOT_XDT_REQUEST_ERCODE_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvRequestErCodeRsp_ITEM856,
	// 	.maxTimeout = 10 * 1000,
	// 	.maxTryCnt = 0xFFFF,
	// 	.matchCmd = IOT_XDT_REQUEST_ERCODE,
	// },	

	// [6] ={
	// 	.cmd = IOT_XDT_CHARGE_RECORD_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvChargeRecordRsp_ITEM8614,
	// 	.maxTimeout = 5 * 1000,
	// 	.maxTryCnt = 21,
	// 	.matchCmd = IOT_XDT_CHARGE_RECORD,
	// },

	// [7] ={
	// 	.cmd = IOT_XDT_REQUEST_CARDAUTH_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvCardAuthRsp_ITEM852,
	// 	.maxTimeout = 10 * 1000,
	// 	.maxTryCnt = 3,
	// 	.matchCmd = IOT_XDT_REQUEST_CARDAUTH,
	// },

	// [8] ={
	// 	.cmd = IOT_XDT_SET_CATEGORY_RSP,
	// 	.matchStr = NULL,
	// 	.pRecvParse = IotXDT_RecvSetCategoryRsp_ITEM854,
	// 	.maxTimeout = 10 * 1000,
	// 	.maxTryCnt = 3,
	// 	.matchCmd = IOT_XDT_SET_CATEGORY,
	// },
};

// static IotXDTRecvCtrl_Struct c_IotXDTRecvctrlV2aResTable[] = 
// {
// 	[0] ={
// 		.cmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP,
// 		.matchStr = "fw_tag",
// 		.pRecvParse = IotXDT_RecvOTAAttributeRsp_ITEM883,
// 		.maxTimeout = 10 * 1000,
// 		.maxTryCnt = 3,
// 		.matchCmd = IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE
// 	},
// };

static IotXDTRecvTopic_Struct c_StrIotlXRecvTopicTable[] = 
{
	[0] ={
		.topic = IOT_XDT_PRE_TOPIC_PROVISION_RESPONSE,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.memberCnt = ARRAY_SIZE(c_IotXDTRecvctrlProvisonTable),
		.pStrRecvCtrlTable = c_IotXDTRecvctrlProvisonTable,
	},

	[1] ={
		.topic = IOT_XDT_PRE_TOPIC_V2A,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.memberCnt = ARRAY_SIZE(c_IotXDTRecvctrlV2aTable),
		.pStrRecvCtrlTable = c_IotXDTRecvctrlV2aTable,
	},

	[2] ={
		.topic = IOT_XDT_PRE_TOPIC_V2R_REQUEST,
		.cmdType = IOT_XDT_CMDTYPE_REQUSET,
		.memberCnt = ARRAY_SIZE(c_IotXDTRecvctrlV2rReqTable),
		.pStrRecvCtrlTable = c_IotXDTRecvctrlV2rReqTable,
	},

	[3] ={
		.topic = IOT_XDT_PRE_TOPIC_V2R_RESPONSE,
		.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
		.memberCnt = ARRAY_SIZE(c_IotXDTRecvctrlV2rResTable),
		.pStrRecvCtrlTable = c_IotXDTRecvctrlV2rResTable,
	},

	// [4] ={
	// 	.topic = IOT_XDT_PRE_TOPIC_V2A_RESPONSE,
	// 	.cmdType = IOT_XDT_CMDTYPE_RESPONSE,
	// 	.memberCnt = ARRAY_SIZE(c_IotXDTRecvctrlV2aResTable),
	// 	.pStrRecvCtrlTable = c_IotXDTRecvctrlV2aResTable,
	// },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotXDT_RecvCredentialRsp_ITEM812(uint8_t u8Port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTPlatInfo_Struct *pPlatInfo = &pPrivateParam->stXDTParam.platinfo;
	cJSON *cRoot, *cStatus, *cCredentialsValue, *cClientId, *cUserName, *cPassword, *cCredentialsType;
	uint8_t ret = FALSE;
	IotXDTErrCodeList_Enum *pAns = NULL;
	
	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cStatus = cJSON_GetObjectItem(cRoot, "status");
	IOT_XDT_CheckKeyIsNull(cStatus, "status", FALSE, pAns);
	
	cCredentialsType = cJSON_GetObjectItem(cRoot, "credentialsType");
	IOT_XDT_CheckKeyIsNull(cCredentialsType, "credentialsType", FALSE, pAns);
	
	if (0 == strcmp(cStatus->valuestring, "SUCCESS"))
	{
		if (0 == strcmp(cCredentialsType->valuestring, "MQTT_BASIC"))
		{
			cCredentialsValue = cJSON_GetObjectItem(cRoot, "credentialsValue");
			IOT_XDT_CheckKeyIsNull(cCredentialsValue, "credentialsValue", FALSE, pAns);
			
			cUserName = cJSON_GetObjectItem(cCredentialsValue, "userName");
			IOT_XDT_CheckKeyIsNull(cUserName, "userName", FALSE, pAns);
			
			cPassword = cJSON_GetObjectItem(cCredentialsValue, "password");
			IOT_XDT_CheckKeyIsNull(cPassword, "password", FALSE, pAns);
			
			if (0 != strcmp(pPlatInfo->cUserName, cUserName->valuestring) ||
				0 != strcmp(pPlatInfo->cPassword, cPassword->valuestring) ||
			    pPlatInfo->credentialSaveFlag != TRUE)
			{
				strncpy(pPlatInfo->cUserName, cUserName->valuestring, MSNVM_XDT_USER_NAME_LEN);
				strncpy(pPlatInfo->cPassword, cPassword->valuestring, MSNVM_XDT_PASSWORD_LEN);
				pPlatInfo->credentialSaveFlag = TRUE;
                pPlatInfo->credentialValidFlag = FALSE;
                MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
			}
			CddNetM_UpdateMqttUserNamePassword(eCddNetMPlatType_O, pPlatInfo->cUserName, pPlatInfo->cPassword);
			IotXDT_OfflineHandle();
			ret = TRUE;
		}
	}

	cJSON_Delete(cRoot);
	return ret;
}

static uint8_t IotXDT_RecvTimeSyncRsp_ITEM822(uint8_t u8Port, uint8_t *r_data, uint16_t len)
{
	cJSON *cRoot, *cTs;
	uint8_t ret = FALSE;
	IotXDTErrCodeList_Enum *pAns = NULL;

	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cTs = cJSON_GetObjectItem(cRoot, "ts");
	IOT_XDT_CheckKeyIsNull(cTs, "ts", FALSE, pAns);
	
	SSTM_SynTimeBySecTimeStamp(cTs->valueint);
	ret = TRUE;
	cJSON_Delete(cRoot);
	return ret;
}

static uint32_t IotXDT_ParseRecvRpc(char * recvTopic, uint8_t preTopicLen)
{
	char *pStart = NULL;
	uint32_t retRpc = 0;
	
	if (preTopicLen < strlen(recvTopic))
	{
		pStart = strrchr(recvTopic, '/');
		if (pStart != NULL)
		{
			retRpc = atoi(pStart+1);
		}
	}
	
	return retRpc;
}

static uint8_t IotXDT_ParseRecvPort(uint8_t *recvBuf, uint16_t dataLen, uint8_t *pEnsure)
{
	cJSON *params = NULL;
	cJSON *gunNo = NULL;
	uint8_t port = 0;
	cJSON *cRoot = cJSON_Parse((const char *)recvBuf);

	if (cRoot != NULL)
	{
		params = cJSON_GetObjectItem(cRoot, "params");
		
		if (params == NULL)
		{
			gunNo =  cJSON_GetObjectItem(cRoot, "gunNo");
			
			if (gunNo == NULL)
			{
				port = 0;
			}
			else
			{
				if (pEnsure != NULL)
				{
					pEnsure[0] = TRUE;
				}
				port = (((uint8_t)cJSON_GetNumberValue(gunNo)) == 1) ?  0 : 1;
			}
		}
		else
		{
			gunNo =  cJSON_GetObjectItem(params, "gunNo");
			
			if (gunNo == NULL)
			{
				port = 0;
			}
			else
			{
				if (pEnsure != NULL)
				{
					pEnsure[0] = TRUE;
				}
				port = (((uint8_t)cJSON_GetNumberValue(gunNo)) == 1) ?  0 : 1;
			}
		}
	}
	
	cJSON_Delete(cRoot);
	return port;
}

static uint8_t IotXDT_GetMatchRecvCtrlTable(uint8_t *pPort, IotXDTRecvTopic_Struct *pRecvTopicTable, 
	uint8_t *recvBuf, uint16_t dataLen, uint32_t rpc, uint8_t *pTabelIndex, uint8_t ensureGunNoFlag)
{
	IotXDTRecvCtrl_Struct *pRecvCtrlTable = NULL;
	uint8_t ret = FALSE;
	uint8_t index = 0;
	uint8_t gunNo = 0;

	if (pRecvTopicTable->cmdType == IOT_XDT_CMDTYPE_REQUSET)
	{
		for (index = 0; index < pRecvTopicTable->memberCnt; index++)
		{
			pRecvCtrlTable =  &pRecvTopicTable->pStrRecvCtrlTable[index];
		
			if (pRecvCtrlTable->matchStr != NULL)
			{
				if (Common_SearchData(recvBuf, dataLen, pRecvCtrlTable->matchStr, strlen(pRecvCtrlTable->matchStr)))
				{
					pTabelIndex[0] = index;
					ret = TRUE;
					break;
				}
			}	
		}
	}
	
	if (ret == FALSE)
	{
		for (index = 0; index < pRecvTopicTable->memberCnt; index++)
		{
			pRecvCtrlTable =  &pRecvTopicTable->pStrRecvCtrlTable[index];

			if (ensureGunNoFlag != TRUE)
			{
				for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
				{
                    if (TRUE == Common_GetSendFlag(pIotXDTCtx->pFuncSendCtrl, gunNo, pRecvCtrlTable->matchCmd) &&
                        rpc == Common_GetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, gunNo, pRecvCtrlTable->cmd))
					{
						pPort[0] = gunNo;
						pTabelIndex[0] = index;
						ret = TRUE;
						break;
					}
				}

				if (ret == TRUE)
				{
					break;
				}
			}
			else
			{
                if (TRUE == Common_GetSendFlag(pIotXDTCtx->pFuncSendCtrl, pPort[0], pRecvCtrlTable->matchCmd) &&
                    rpc == Common_GetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, pPort[0], pRecvCtrlTable->cmd))
				{
					pTabelIndex[0] = index;
					ret = TRUE;
					break;
				}
			}
		}
	}
	
	return ret;
}

static IotXDTRecvTopic_Struct* IotXDT_FindRecvTopicTablePointer(char *topic)
{
	IotXDTRecvTopic_Struct *pRecvTopicTable = NULL;
	IotXDTRecvCtrl_Struct *pRecvCtrl = NULL;
	uint8_t index = 0;
	
	for (index = 0; index < ARRAY_SIZE(c_StrIotlXRecvTopicTable); index++)
	{
		pRecvTopicTable = &c_StrIotlXRecvTopicTable[index];
		
		if (0 == memcmp(pRecvTopicTable->topic, topic, strlen(pRecvTopicTable->topic)))
		{
			if (Common_CalcCharCount(pRecvTopicTable->topic, strlen(pRecvTopicTable->topic), '/') == Common_CalcCharCount(topic, strlen(topic), '/'))
			{
				break;
			}
		}
		
		pRecvTopicTable = NULL;
	}
	
	return pRecvTopicTable;
}

static void IotXDT_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    uint16_t frameLen = 0;
	IotXDTRecvTopic_Struct *pRecvTopicTable = NULL;
	IotXDTRecvCtrl_Struct *pRecvCtrl = NULL;
	uint8_t index = 0;
	uint32_t rpc = 0;
	uint8_t port = 0;
	uint8_t ensureGunNoFlag = FALSE;

    if (dataLen > 0 && topicLen > 0)
    {
        pRecvTopicTable = IotXDT_FindRecvTopicTablePointer((char *)pTopic);
        
        if (pRecvTopicTable != NULL)
        {
            rpc = IotXDT_ParseRecvRpc((char *)pTopic, strlen(pRecvTopicTable->topic));
            port = IotXDT_ParseRecvPort(pData, dataLen, &ensureGunNoFlag);

            if (IotXDT_GetMatchRecvCtrlTable(&port, pRecvTopicTable, pData, dataLen, rpc, &index, ensureGunNoFlag))
            {
                pRecvCtrl = &pRecvTopicTable->pStrRecvCtrlTable[index];

                if (pRecvCtrl->pRecvParse != NULL)
                {
                    if (TRUE == pRecvCtrl->pRecvParse(port, pData, dataLen))
                    {
                        extern size_t xPortGetFreeHeapSize( void );
                        IOTXDT_CFG_LogPrint("[Recv]Remaining heap size: %zu bytes\n", xPortGetFreeHeapSize());

                        if (pRecvTopicTable->cmdType == IOT_XDT_CMDTYPE_RESPONSE)
                        {
                            Common_SetRecvEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, TRUE);
                            Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);

                            if (IotXDT_GetSendCmdSendCycle(pRecvCtrl->matchCmd) == 0)
                            {
                                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                            }
                        }
                        else
                        {	
                            if (pRecvCtrl->matchCmd != IOT_XDT_CMD_NULL)
                            {
                                Common_SetRecvSeq(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, rpc);
                                Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            }
                        }
                        
                        if (pRecvCtrl->matchCmd != IOT_XDT_CMD_NULL)
                        {
                            Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
                        }
                    }
                }
            }
            else
            {
                IOTXDT_CFG_LogPrint("\"%s\"parse failed...\r\n", (char *)pData);
            }
        }
        else
        {
            IOTXDT_CFG_LogPrint("\"%s\" topic parse failed...\r\n", pTopic);
        }
    }
}


void IotXDT_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotXDTCtx->frameQueueChannelID, IotXDT_DecodeData);
}



