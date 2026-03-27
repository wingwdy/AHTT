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
#include "Asw_ErrorHandle.h"
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
static uint8_t IotXDT_RecvCredentialRsp_ITEM812(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvTimeSyncRsp_ITEM822(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvLinkRsp_ITEM824(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvSetReboot_ITEM825(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvRequestRateModeRsp_ITEM832(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvQueryRateMode_ITEM833(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvRateModeSet_ITEM834(uint8_t port, uint8_t *r_data, uint16_t len);
static uint8_t IotXDT_RecvPlieStateRsp_ITEM842(uint8_t port, uint8_t *r_data, uint16_t len);

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
		.cMeaning = "响应凭据查询",
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
		.pRecvParse = IotXDT_RecvSetReboot_ITEM825,
		.maxTimeout = 0,
		.maxTryCnt = 0,
		.matchCmd = IOT_XDT_CMD_SET_RESTART_RSP,
	},

	[1] ={
		.cmd = IOT_XDT_CMD_RATEMODE_SET,
		.matchStr = "rate_set",
		.pRecvParse = IotXDT_RecvRateModeSet_ITEM834,
		.maxTimeout = 0,
		.maxTryCnt = 0,
		.matchCmd = IOT_XDT_CMD_RATEMODE_SET_RSP,
	},

	[2] ={
		.cmd = IOT_XDT_CMD_QUERY_RATEMODE,
		.matchStr = "rate_get",
		.pRecvParse = IotXDT_RecvQueryRateMode_ITEM833,
		.maxTimeout = 0,
		.maxTryCnt = 0,
		.matchCmd = IOT_XDT_CMD_QUERY_RATEMODE_RSP,
	},

	// [3] ={
	// 	.cmd = IOT_XDT_CMD_CALL_REALDATA,
	// 	.matchStr = "req_real_data",
	// 	.pRecvParse = IotXDT_RecvCallRealData_ITEM846,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CMD_CALL_REALDATA_RSP,
	// },

	// [4] ={
	// 	.cmd = IOT_XDT_CHARGE_START,
	// 	.matchStr = "start_cmd",
	// 	.pRecvParse = IotXDT_RecvChargeStart_ITEM861,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_START_RSP,
	// },	

	// [5] ={
	// 	.cmd = IOT_XDT_CHARGE_STOP,
	// 	.matchStr = "stop_cmd",
	// 	.pRecvParse = IotXDT_RecvChargeStop_ITEM864,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_STOP_RSP,
	// },

	// [6] ={
	// 	.cmd = IOT_XDT_QUERY_CHARGE_RECORD,
	// 	.matchStr = "record_get",
	// 	.pRecvParse = IotXDT_RecvQueryChargeRecord_ITEM8615,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_CHARGE_RECORD_RSP,
	// },	

	// [7] ={
	// 	.cmd = IOT_XDT_CHARGE_PWRCTRL,
	// 	.matchStr = "control_power",
	// 	.pRecvParse = IotXDT_RecvPowerCtrl_ITEM8611,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_PWRCTRL_RSP,
	// },	

	// [8] ={
	// 	.cmd = IOT_XDT_CHARGE_CONTINUE_CHARGE,
	// 	.matchStr = "topup_cmd",
	// 	.pRecvParse = IotXDT_RecvContinueCharge_ITEM867,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP,
	// },	

	// [9] ={
	// 	.cmd = IOT_XDT_QUERY_BOARDINFO,
	// 	.matchStr = "control_info_get",
	// 	.pRecvParse = IotXDT_RecvQueryBoardInfo_ITEM871,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_QUERY_BOARDINFO_RSP,
	// },	

	// [10] ={
	// 	.cmd = IOT_XDT_PARA_SET,
	// 	.matchStr = "param_set",
	// 	.pRecvParse = IotXDT_RecvParaSet_ITEM873,
	// 	.maxTimeout = 0,
	// 	.maxTryCnt = 0,
	// 	.matchCmd = IOT_XDT_PARA_SET_RSP,
	// },	

	// [11] ={
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
		.cMeaning = "时间同步响应",
		.matchCmd = IOT_XDT_CMD_REQUEST_TIMESYNC,
	},
	
	[1] ={
		.cmd = IOT_XDT_CMD_REQUEST_LINK_RSP,
		.matchStr = "ts",
		.pRecvParse = IotXDT_RecvLinkRsp_ITEM824,
		.maxTimeout = 10 * 1000,
		.maxTryCnt = 3,
		.cMeaning = "上线请求响应",
		.matchCmd = IOT_XDT_CMD_REQUEST_LINK,
	},

	[2] ={
		.cmd = IOT_XDT_CMD_REQUEST_RATEMODE_RSP,
		.matchStr = "typeRule",
		.pRecvParse = IotXDT_RecvRequestRateModeRsp_ITEM832,
		.maxTimeout = 10 * 1000,
		.maxTryCnt = 0xFFFF,
		.matchCmd = IOT_XDT_CMD_REQUEST_RATEMODE,
	},

	[3] ={
		.cmd = IOT_XDT_CMD_PILE_STATE_RSP,
		.matchStr = NULL,
		.pRecvParse = IotXDT_RecvPlieStateRsp_ITEM842,
		.maxTimeout = 5 * 1000,
		.maxTryCnt = 21,
		.cMeaning = "电桩状态响应",
		.matchCmd = IOT_XDT_CMD_PILE_STATE,
	},

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
static uint8_t IotXDT_RecvCredentialRsp_ITEM812(uint8_t port, uint8_t *r_data, uint16_t len)
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

static uint8_t IotXDT_RecvTimeSyncRsp_ITEM822(uint8_t port, uint8_t *r_data, uint16_t len)
{
	cJSON *cRoot, *cTs;
	uint8_t ret = TRUE;
	IotXDTErrCodeList_Enum *pAns = NULL;

	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cTs = cJSON_GetObjectItem(cRoot, "ts");
	IOT_XDT_CheckKeyIsNull(cTs, "ts", FALSE, pAns);
	
	SSTM_SynTimeBySecTimeStamp(cTs->valueint + SSTM_BASE_TIMESTAMP_1970_BJT);
	cJSON_Delete(cRoot);
	return ret;
}

static uint8_t IotXDT_RecvLinkRsp_ITEM824(uint8_t port, uint8_t *r_data, uint16_t len)
{
	cJSON *cRoot,  *cAns;
	IotXDTErrCodeList_Enum *pAns = NULL;

	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cAns = cJSON_GetObjectItem(cRoot, "ans");
	IOT_XDT_CheckKeyIsNull(cAns, "ans", FALSE, pAns);
		
	if (cAns->valueint != 0)
	{
		IOTXDT_CFG_LogPrint("上线请求失败!!! 失败原因：%d!r\n", cAns->valueint);
        IotXDT_OfflineHandle();
	}
	else
	{
		pIotXDTCtx->loginSucc = TRUE;
		AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
	}

	cJSON_Delete(cRoot);
	return TRUE;
}

static void IotXDT_ConvertRateModeToGN(IotXDTRateMode_Struct *pLxRateMode, MSNvmXDTParamBillMode_Struct *pGnRateMode)
{
	IotXDTPeriodList_Struct *pIotXDTPeriodList = NULL;
	uint8_t index = 0;
	uint32_t tempVal = 0, tempVal2 = 0;
	uint8_t flag = 0;
	uint16_t startIndex = 0;
	uint16_t stopIndex = 0;
	char tempStr[3] = { 0 };
	uint8_t temp = 0;

	memset(pGnRateMode, 0x00, sizeof(MSNvmXDTParamBillMode_Struct));
	memcpy(pGnRateMode->billModeID, pLxRateMode->billModeID, sizeof(pLxRateMode->billModeID));

	pGnRateMode->period_count = pLxRateMode->periodCnt;
	pGnRateMode->typeRule = pLxRateMode->typeRule;
	
	if (pLxRateMode->typeRule == 0)
	{
		tempVal = round(atof(pLxRateMode->stdElec) * 10000) * 10;
		Common_Uint32ToFourUint8(pGnRateMode->sharp_ele_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->peak_ele_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->flat_ele_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->valley_ele_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->deep_ele_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->std_ele_fee, tempVal);
		tempVal = round(atof(pLxRateMode->stdSer) * 10000) * 10;
		Common_Uint32ToFourUint8(pGnRateMode->sharp_ser_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->peak_ser_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->flat_ser_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->valley_ser_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->deep_ser_fee, tempVal);
		Common_Uint32ToFourUint8(pGnRateMode->std_ser_fee, tempVal);

		pGnRateMode->period[0].validFlag = TRUE;
		pGnRateMode->period[0].startTime = 0;
		pGnRateMode->period[0].stopTime = 48;
		pGnRateMode->period[0].flag = 0;
	}
	else
	{
		tempVal = round(atof(pLxRateMode->stdElec) * 10000) * 10;
		Common_Uint32ToFourUint8(pGnRateMode->std_ele_fee, tempVal);
		tempVal = round(atof(pLxRateMode->stdSer) * 10000) * 10;
		Common_Uint32ToFourUint8(pGnRateMode->std_ser_fee, tempVal);
		
		for (index = 0; index < pLxRateMode->periodCnt; index++)
		{
			pIotXDTPeriodList = &pLxRateMode->periodListInfo[index];	
			flag = atoi(pIotXDTPeriodList->flag);
			tempVal = round(atof(pIotXDTPeriodList->elec) * 10000) * 10;
			tempVal2 = round(atof(pIotXDTPeriodList->ser) * 10000) * 10;

			switch (flag)
			{
				case 1:
				{
					Common_Uint32ToFourUint8(pGnRateMode->sharp_ele_fee, tempVal);
					Common_Uint32ToFourUint8(pGnRateMode->sharp_ser_fee, tempVal2);
					break;
				}
				case 2:
				{
					Common_Uint32ToFourUint8(pGnRateMode->peak_ele_fee, tempVal);
					Common_Uint32ToFourUint8(pGnRateMode->peak_ser_fee, tempVal2);
					break;
				}
				case 3:
				{
					Common_Uint32ToFourUint8(pGnRateMode->flat_ele_fee, tempVal);
					Common_Uint32ToFourUint8(pGnRateMode->flat_ser_fee, tempVal2);
					break;
				}
				case 4:
				{
					Common_Uint32ToFourUint8(pGnRateMode->valley_ele_fee, tempVal);
					Common_Uint32ToFourUint8(pGnRateMode->valley_ser_fee, tempVal2);
					break;
				}
				case 5:
				{
					Common_Uint32ToFourUint8(pGnRateMode->deep_ele_fee, tempVal);
					Common_Uint32ToFourUint8(pGnRateMode->deep_ser_fee, tempVal2);
					break;
				}
				default:
				{
					break;
				}
			}

			memcpy(tempStr, pIotXDTPeriodList->begin, 2);
			tempVal = atoi(tempStr);      
			memcpy(tempStr, pIotXDTPeriodList->begin + 3, 2);
			tempVal2 = atoi(tempStr);  
			startIndex = (tempVal * 2) +  (tempVal2 / 30);

			memcpy(tempStr, pIotXDTPeriodList->end, 2);
			tempVal = atoi(tempStr);      
			memcpy(tempStr, pIotXDTPeriodList->end + 3, 2);
			tempVal2 = atoi(tempStr);  
			stopIndex = (tempVal * 2) +  (tempVal2 / 30);

			for (temp = startIndex; temp < stopIndex; temp++)
			{
				pGnRateMode->segmentation_rate[temp] = (flag - 1);
			}	

			pGnRateMode->period[index].flag = (flag - 1);
			pGnRateMode->period[index].validFlag = TRUE;
			pGnRateMode->period[index].startTime = startIndex;
			pGnRateMode->period[index].stopTime = stopIndex;
		}
	}

	Common_Uint32ToFourUint8(pGnRateMode->validFlag, IOT_XDT_MAGIC_NUM);
}

static uint8_t IotXDT_RecvRequestRateModeRsp_ITEM832(uint8_t port, uint8_t *r_data, uint16_t len)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pBillMode = &pPrivateParam->stXDTParam.stBillMode;
	IotXDTRateMode_Struct strRateMode = {0};
	cJSON *cRoot, *cAns, *cType, *cId, *cTypeRule, *cElec, *cSer;
	cJSON *cBegin, *cEnd, *cFlag, *cPeriodList[MSNVM_XDT_BILLMODE_PERIOD_COUNT];
	cJSON *cPeriodListArray;
	uint8_t peroidCnt = 0;
	uint8_t ret = FALSE;
	uint8_t index = 0;
	IotXDTErrCodeList_Enum *pAns = NULL;

	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cAns = cJSON_GetObjectItem(cRoot, "ans");
	IOT_XDT_CheckKeyIsNull(cAns, "ans", FALSE, pAns);

	cType = cJSON_GetObjectItem(cRoot, "type");
	IOT_XDT_CheckKeyIsNull(cType, "type", FALSE, pAns);

	cId = cJSON_GetObjectItem(cRoot, "id");
	IOT_XDT_CheckKeyIsNull(cId, "id", FALSE, pAns);

	cTypeRule = cJSON_GetObjectItem(cRoot, "typeRule");
	IOT_XDT_CheckKeyIsNull(cTypeRule, "typeRule", FALSE, pAns);

	if (cAns->valueint == eIotXDTErrCode_Success)
	{
		if (cType->valueint == 0)
		{
			cElec = cJSON_GetObjectItem(cRoot, "elec");
			IOT_XDT_CheckKeyIsNull(cElec, "elec", FALSE, pAns);

			cSer = cJSON_GetObjectItem(cRoot, "ser");
			IOT_XDT_CheckKeyIsNull(cSer, "ser", FALSE, pAns);

			
			Common_Uint32ToFourUint8(strRateMode.billModeID, cId->valueint);
			strRateMode.typeRule = cTypeRule->valueint;
			
			strncpy(strRateMode.stdElec, cElec->valuestring, sizeof(strRateMode.stdElec) - 1);
			strncpy(strRateMode.stdSer, cSer->valuestring, sizeof(strRateMode.stdSer) - 1);
			
			if (cTypeRule->valueint == 1)
			{
				cPeriodListArray = cJSON_GetObjectItem(cRoot, "periodsList");
				IOT_XDT_CheckKeyIsNull(cPeriodListArray, "periodsList", FALSE, pAns);

				peroidCnt = cJSON_GetArraySize(cPeriodListArray);

				if (peroidCnt > MSNVM_XDT_BILLMODE_PERIOD_COUNT || peroidCnt == 0)
				{
					IOTXDT_CFG_LogPrint("[%s()]: rateMode period cnt[%d] is invalid...\r\n", __FUNCTION__, peroidCnt);
				}
				else
				{
					strRateMode.periodCnt = peroidCnt;
					for (index = 0; index < peroidCnt; index++)
					{
						cPeriodList[index] = cJSON_GetArrayItem(cPeriodListArray, index);

						if (cPeriodList[index] != NULL)
						{
							cBegin = cJSON_GetObjectItem(cPeriodList[index], "begin");
							IOT_XDT_CheckKeyIsNull(cBegin, "begin", FALSE, pAns);

							cEnd = cJSON_GetObjectItem(cPeriodList[index], "end");
							IOT_XDT_CheckKeyIsNull(cEnd, "end", FALSE, pAns);

							cFlag = cJSON_GetObjectItem(cPeriodList[index], "flag");
							IOT_XDT_CheckKeyIsNull(cFlag, "flag", FALSE, pAns);								

							cElec = cJSON_GetObjectItem(cPeriodList[index], "elec");
							IOT_XDT_CheckKeyIsNull(cEnd, "elec", FALSE, pAns);

							cSer = cJSON_GetObjectItem(cPeriodList[index], "ser");
							IOT_XDT_CheckKeyIsNull(cSer, "ser", FALSE, pAns);

							strncpy(strRateMode.periodListInfo[index].flag, cFlag->valuestring, sizeof(strRateMode.periodListInfo[index].flag) - 1);
							strncpy(strRateMode.periodListInfo[index].begin, cBegin->valuestring, sizeof(strRateMode.periodListInfo[index].begin) - 1);
							strncpy(strRateMode.periodListInfo[index].end, cEnd->valuestring, sizeof(strRateMode.periodListInfo[index].end) - 1);
							strncpy(strRateMode.periodListInfo[index].elec, cElec->valuestring, sizeof(strRateMode.periodListInfo[index].elec) - 1);
							strncpy(strRateMode.periodListInfo[index].ser, cSer->valuestring, sizeof(strRateMode.periodListInfo[index].ser) - 1);
						}
					}
				}
			}
			else
			{
				strRateMode.periodCnt = 1;
			}
			
			ret = TRUE;
		}
		else
		{
			IOTXDT_CFG_LogPrint("[%s()]: rateMode type is discharge ..\r\n", __FUNCTION__);
		}
	}
	else
	{
		IOTXDT_CFG_LogPrint("[%s()]: rateMode response Error, Errcode is %d ..\r\n", __FUNCTION__, cAns->valueint);
	}

	if (ret == TRUE)
	{
		if (TRUE != IotXDT_IsPileOnCharging())
		{
			IotXDT_ConvertRateModeToGN(&strRateMode, pBillMode);
			MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
		}
	}

	cJSON_Delete(cRoot);
	return ret;	
}


static uint8_t IotXDT_RecvRateModeSet_ITEM834(uint8_t port, uint8_t *r_data, uint16_t len)
{
	MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pBillMode = &pPrivateParam->stXDTParam.stBillMode;
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTErrCodeList_Enum *pAns = &pRecvDataInfo->offlineClearData.eAns_Item834;
	IotXDTRateMode_Struct strRateMode = { 0 };
	cJSON *cRoot, *cType, *cId, *cTypeRule, *cElec, *cSer,*cParams;
	cJSON *cBegin, *cEnd, *cFlag, *cPeriodList[IOT_XDT_RATE_MODE_MAX_PERIOD];
	cJSON *cPeriodListArray;
	uint8_t peroidCnt = 0;
	uint8_t index = 0;

	pAns[0] = eIotXDTErrCode_Success;

	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);

	cParams = cJSON_GetObjectItem(cRoot, "params");
	IOT_XDT_CheckKeyIsNull(cParams, "params", FALSE, pAns);

	cType = cJSON_GetObjectItem(cParams, "type");
	IOT_XDT_CheckKeyIsNull(cType, "type", FALSE, pAns);

	pRecvDataInfo->offlineClearData.type_ITEM834 = cType->valueint;
	
	cId = cJSON_GetObjectItem(cParams, "id");
	IOT_XDT_CheckKeyIsNull(cId, "id", FALSE, pAns);

	cTypeRule = cJSON_GetObjectItem(cParams, "typeRule");
	IOT_XDT_CheckKeyIsNull(cTypeRule, "typeRule", FALSE, pAns);

	if (cType->valueint != 0)
	{
		pAns[0] = eIotXDTErrCode_ParaInvalid;
		IOTXDT_CFG_LogPrint("[%s()]: rateMode type is discharge ..\r\n", __FUNCTION__);
	}

	cElec = cJSON_GetObjectItem(cParams, "elec");
	IOT_XDT_CheckKeyIsNull(cElec, "elec", FALSE, pAns);

	cSer = cJSON_GetObjectItem(cParams, "ser");
	IOT_XDT_CheckKeyIsNull(cSer, "ser", FALSE, pAns);

	strncpy(strRateMode.stdElec, cElec->valuestring, sizeof(strRateMode.stdElec) - 1);
	strncpy(strRateMode.stdSer, cSer->valuestring, sizeof(strRateMode.stdSer) - 1);
	Common_Uint32ToFourUint8(strRateMode.billModeID, cId->valueint);
	strRateMode.typeRule = cTypeRule->valueint;

	if (cTypeRule->valueint == 1)
	{
		cPeriodListArray = cJSON_GetObjectItem(cParams, "periodsList");
		IOT_XDT_CheckKeyIsNull(cPeriodListArray, "periodsList", FALSE, pAns);
		peroidCnt = cJSON_GetArraySize(cPeriodListArray);

		if (peroidCnt == 0)
		{
			pAns[0] = eIotXDTErrCode_RateModePeriodParaErr;
			IOTXDT_CFG_LogPrint("[%s()]: rateMode period cnt[%d] is invalid...\r\n", __FUNCTION__, peroidCnt);
		}
		else if (peroidCnt > IOT_XDT_RATE_MODE_MAX_PERIOD)
		{
			peroidCnt = IOT_XDT_RATE_MODE_MAX_PERIOD;
			IOTXDT_CFG_LogPrint("[%s()]: rateMode period cnt[%d] is invalid...\r\n", __FUNCTION__, peroidCnt);
		}
		else
		{}

		if (peroidCnt > 0)
		{
			strRateMode.periodCnt = peroidCnt;
			
			for (index = 0; index < peroidCnt; index++)
			{
				cPeriodList[index] = cJSON_GetArrayItem(cPeriodListArray, index);

				if (cPeriodList[index] != NULL)
				{
					cBegin = cJSON_GetObjectItem(cPeriodList[index], "begin");
					IOT_XDT_CheckKeyIsNull(cBegin, "begin", FALSE, pAns);

					cEnd = cJSON_GetObjectItem(cPeriodList[index], "end");
					IOT_XDT_CheckKeyIsNull(cEnd, "end", FALSE, pAns);

					cFlag = cJSON_GetObjectItem(cPeriodList[index], "flag");
					IOT_XDT_CheckKeyIsNull(cFlag, "flag", FALSE, pAns);
				
					cElec = cJSON_GetObjectItem(cPeriodList[index], "elec");
					IOT_XDT_CheckKeyIsNull(cEnd, "elec", FALSE, pAns);

					cSer = cJSON_GetObjectItem(cPeriodList[index], "ser");
					IOT_XDT_CheckKeyIsNull(cSer, "ser", FALSE, pAns);

					strncpy(strRateMode.periodListInfo[index].flag, cFlag->valuestring, sizeof(strRateMode.periodListInfo[index].flag) - 1);
					strncpy(strRateMode.periodListInfo[index].begin, cBegin->valuestring, sizeof(strRateMode.periodListInfo[index].begin) - 1);
					strncpy(strRateMode.periodListInfo[index].end, cEnd->valuestring, sizeof(strRateMode.periodListInfo[index].end) - 1);
					strncpy(strRateMode.periodListInfo[index].elec, cElec->valuestring, sizeof(strRateMode.periodListInfo[index].elec) - 1);
					strncpy(strRateMode.periodListInfo[index].ser, cSer->valuestring, sizeof(strRateMode.periodListInfo[index].ser) - 1);
				}
			}
		}
		else
		{
			strRateMode.periodCnt = peroidCnt;
		}
	}
	else
	{
		strRateMode.periodCnt = 1;
	}

	IotXDT_ConvertRateModeToGN(&strRateMode, &pRecvDataInfo->offlineClearData.strGnRate_Item834);

	if (pAns[0] == eIotXDTErrCode_Success)
	{
		if (TRUE != IotXDT_IsPileOnCharging())
		{
			memcpy(pBillMode, &pRecvDataInfo->offlineClearData.strGnRate_Item834, sizeof(MSNvmXDTParamBillMode_Struct));
			MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
		}
		else
		{
			pAns[0] = eIotXDTErrCode_OnCharging;
			IOTXDT_CFG_LogPrint("[%s()]: rateMode set failed, some gun is on charging ..\r\n", __FUNCTION__);
		}
	}

	cJSON_Delete(cRoot);
	return TRUE;
}

static uint8_t IotXDT_RecvSetReboot_ITEM825(uint8_t port, uint8_t *r_data, uint16_t len)
{
	cJSON *cRoot, *cParams, *cSnPlat, *cForce;
	IotXDTRecvData_Struct *pRecvData = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTErrCodeList_Enum *pAns = &pRecvData->offlineClearData.eAns_Item825;
	uint8_t ret = TRUE;

	pAns[0] =eIotXDTErrCode_Success;
	
	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cParams = cJSON_GetObjectItem(cRoot, "params");
	IOT_XDT_CheckKeyIsNull(cParams, "params", FALSE, pAns);
	
	cSnPlat = cJSON_GetObjectItem(cParams, "snPlat");
	IOT_XDT_CheckKeyIsNull(cSnPlat, "snPlat", FALSE, pAns);

	cForce = cJSON_GetObjectItem(cParams, "force");
	IOT_XDT_CheckKeyIsNull(cForce, "force", FALSE, pAns);

	pRecvData->offlineClearData.force_Item825 = cForce->valueint;
	return ret;
}

static uint8_t IotXDT_RecvQueryRateMode_ITEM833(uint8_t port, uint8_t *r_data, uint16_t len)
{
	IotXDTRecvData_Struct *pRecvDataInfo = &pIotXDTCtx->stProtoData.stRecvData[port];
	IotXDTErrCodeList_Enum *pAns = &pRecvDataInfo->offlineClearData.eAns_Item833;
	cJSON *cParams,*cRoot, *cType;

	pAns[0] = eIotXDTErrCode_Success;
	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);

	cParams = cJSON_GetObjectItem(cRoot, "params");
	IOT_XDT_CheckKeyIsNull(cParams, "params", FALSE, pAns);

	cType = cJSON_GetObjectItem(cParams, "type");
	IOT_XDT_CheckKeyIsNull(cType, "type", FALSE, pAns);

	pRecvDataInfo->offlineClearData.type_ITEM833 = cType->valueint;

	if (cType->valueint == 0)
	{
		pAns[0] = eIotXDTErrCode_Success;
	}
	else
	{
		pAns[0] = eIotXDTErrCode_ParaInvalid;
		IOTXDT_CFG_LogPrint("[%s()]: rateMode type is discharge ..\r\n", __FUNCTION__);
	}

	cJSON_Delete(cRoot);
	return TRUE;
}

static uint8_t IotXDT_RecvPlieStateRsp_ITEM842(uint8_t port, uint8_t *r_data, uint16_t len)
{
	cJSON *cRoot, *cStatus;
	uint8_t ret = FALSE;
	IotXDTErrCodeList_Enum *pAns = NULL;
	
	cRoot = cJSON_Parse((const char *)r_data);
	IOT_XDT_CheckObjIsNull(cRoot, FALSE);
	
	cStatus = cJSON_GetObjectItem(cRoot, "status");
	IOT_XDT_CheckKeyIsNull(cStatus, "status", FALSE, pAns);

	if (cStatus->valueint == eIotXDTErrCode_Success)
	{
		ret = TRUE;
	}
	else
	{
		IOTXDT_CFG_LogPrint("[%s()]: Platform response error[%d]\r\n", __FUNCTION__, cStatus->valueint);
	}

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

static void IotXDT_CmdTimeoutHandle3Times(uint8_t port, IotXDTRecvCtrl_Struct *pRecvCtrl)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmXDTParamBillMode_Struct *pBillMode = &pPrivateParam->stXDTParam.stBillMode;

	switch (pRecvCtrl->cmd)
	{
	case IOT_XDT_CMD_REQUEST_RATEMODE_RSP:
	{
		if (Common_FourUint8ToUint32(pBillMode->validFlag) == IOT_XDT_MAGIC_NUM)
		{
            Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
            Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
		}
		else
		{
            Common_ClearRptCount(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
		}

		break;
	}
	default:
	{
        Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
        Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
		break;
	}
	}
}

static void IotXDT_CmdTimeoutHandle(uint8_t port, uint16_t cmd)
{
	if (cmd == IOT_XDT_CMD_ERRINFO_RSP)
	{
//		IotXDT_CheckErrInfoQueueDuplicate(port);
	}
}

void IotXDT_TimeoutDetect(void)
{
	IotXDTRecvTopic_Struct *pRecvTopicTable = NULL;
	IotXDTRecvCtrl_Struct *pRecvCtrl = NULL;
	uint8_t index = 0, temp = 0, port = 0;
	uint8_t timeoutCount = 0;

	for (index = 0; index < ARRAY_SIZE(c_StrIotlXRecvTopicTable); index++)
	{
		pRecvTopicTable = &c_StrIotlXRecvTopicTable[index];

		if (pRecvTopicTable->cmdType != IOT_XDT_CMDTYPE_RESPONSE)
		{
			continue;
		}

		for (temp = 0; temp < pRecvTopicTable->memberCnt; temp++)
		{
			pRecvCtrl = &pRecvTopicTable->pStrRecvCtrlTable[temp];

			for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
			{
                if (Common_GetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd) != TRUE)
                {
                    continue;
                }

                if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotXDTCtx->pFuncRecvCtrl, port, 
                    pRecvCtrl->cmd), pRecvCtrl->maxTimeout) == TRUE)
				{
                    Common_SetRptCount(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);
                    timeoutCount = Common_GetRptCount(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd);

                    IOTXDT_CFG_LogPrint("[cmd:0x%04X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pRecvCtrl->cmd, 
                        pRecvCtrl->cMeaning, timeoutCount, pRecvCtrl->maxTimeout);

					if (pRecvCtrl->maxTryCnt == 0xFFFF)
					{
						if (timeoutCount >= 3)
						{
							IotXDT_CmdTimeoutHandle3Times(port, pRecvCtrl);
						}
						else
						{
                            Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
							IotXDT_CmdTimeoutHandle(port, pRecvCtrl->cmd);
						}
					}
					else
					{
						if (timeoutCount >= pRecvCtrl->maxTryCnt)
						{
							IotXDT_OfflineHandle();
						}
						else
						{
                            Common_SetRecvTimerEnable(pIotXDTCtx->pFuncRecvCtrl, port, pRecvCtrl->cmd, FALSE);
                            Common_SetSendEnable(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            Common_SetSendImmdFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, TRUE);
                            Common_SetSendFlag(pIotXDTCtx->pFuncSendCtrl, port, pRecvCtrl->matchCmd, FALSE);
							IotXDT_CmdTimeoutHandle(port, pRecvCtrl->cmd);
						}
					}
				}
			}
		}
	}
}


void IotXDT_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotXDTCtx->frameQueueChannelID, IotXDT_DecodeData);
}



