/******************************************************************************
* File Name          : template.h
* Description        : Code for xxxxxxxxxxx
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef ASW_IOT_PROTO_XDT_TYPES_H_
#define ASW_IOT_PROTO_XDT_TYPES_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"
#include "cJSON.h"
#include "myMalloc.h"
#include "Ms_Nvm.h"
#include "Asw_ErrorHandle.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

#define IOT_XDT_PROTOCOL_VER                      "v1.6.9"

/* 通信buff缓存定义 */
#define IOT_XDT_TXRX_BUFFER_SIZE                 (3072U)

#define IOT_XDT_RATE_MODE_MAX_PERIOD               16
#define IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE          16
#define IOT_XDT_ORDERNUM_LEN                       32
#define IOT_XDT_USERNUM_LEN                        17
#define IOT_XDT_FAULTCODE_LEN                      7
#define IOT_XDT_TOPIC_LEN                          32
#define IOT_XDT_PASSWDSTOP_LEN                     16
#define IOT_XDT_FWVERSION_LEN                      32
#define IOT_XDT_PARAM_MAX_LEN                      64
#define IOT_XDT_FWSTATE_LEN                        16

#define IOT_XDT_SINGLE_FRAME_MAX_ERROR_COUNT       10

/* 最小账户余额 0.5元，预留5分钱 */
#define IOT_XDT_MIN_ACCOUNT_MONEY                  (50 + 5)

/* 协议Topic 定义 */
#define IOT_XDT_TOPIC_PROVISION_REQUEST      	  "/provision/request"
#define IOT_XDT_TOPIC_PROVISION_RESPONSE     	  "/provision/response"
#define IOT_XDT_TOPIC_V2R_REQUEST				  "v2/r/req/+"
#define IOT_XDT_TOPIC_V2R_RESPONSE				  "v2/r/res/+"
#define IOT_XDT_TOPIC_V2A				          "v2/a"
#define IOT_XDT_TOPIC_V2A_REQUEST			      "v2/a/req/+"
#define IOT_XDT_TOPIC_V2A_RESPONSE	    	      "v2/a/res/+"
#define IOT_XDT_TOPIC_V2FW_ERROR  	    	      "v2/fw/error"

#define IOT_XDT_PRE_TOPIC_PROVISION_RESPONSE	  "/provision/response"
#define IOT_XDT_PRE_TOPIC_V2A					  "v2/a"
#define IOT_XDT_PRE_TOPIC_V2R_REQUEST             "v2/r/req/"
#define IOT_XDT_PRE_TOPIC_V2R_RESPONSE       	  "v2/r/res/" 
#define IOT_XDT_PRE_TOPIC_V2A_REQUEST       	  "v2/a/req/"  
#define IOT_XDT_PRE_TOPIC_V2A_RESPONSE      	  "v2/a/res/"
#define IOT_XDT_PRE_TOPIC_TSDATA                  "tsdata/run_process"
#define IOT_XDT_PRE_TOPIC_V2T                     "v2/t"


#define IOT_XDT_CREDENTIAL_TYPE                   "MQTT_BASIC"

#define IOT_XDT_MAGIC_NUM                         (0x55AAAA55U)

/* 协议CMD 定义 */
#define IOT_XDT_CMDTYPE_REQUSET			         (0x00U)
#define IOT_XDT_CMDTYPE_RESPONSE                 (0x01U)
#define IOT_XDT_CMD_NULL                         (0xFFFFU)             

/* 协议CMD 发送定义 */
#define IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL      (0x811U)        /* 查询接入凭据 */
#define IOT_XDT_CMD_REQUEST_TIMESYNC             (0x821U)        /* 请求时间同步 */
#define IOT_XDT_CMD_REQUEST_LINK                 (0x823U)        /* 请求链接 */
#define IOT_XDT_CMD_SET_RESTART_RSP              (0x826U)        /* 重启设置响应 */
#define IOT_XDT_CMD_REQUEST_RATEMODE             (0x831U)        /* 请求计费模型 */
#define IOT_XDT_CMD_QUERY_RATEMODE_RSP           (0x837U)        /* 查询计费模型响应 */
#define IOT_XDT_CMD_RATEMODE_SET_RSP             (0x835U)        /* 计费模型设置响应 */
#define IOT_XDT_CMD_RATEMODE_SET_RSP_EVENT       (0x836U)        /* 计费模型设置响应事件 */
#define IOT_XDT_CMD_PILE_STATE                   (0x841U)        /* 桩状态 */
#define IOT_XDT_CMD_ERRINFO                      (0x843U)        /* 错误信息 */
#define IOT_XDT_CMD_PILE_DATA                    (0x845U)        /* 桩数据 */
#define IOT_XDT_CMD_CALL_REALDATA                (0x846U)        /* 召测实时数据 */
#define IOT_XDT_CMD_REQUEST_CARDAUTH             (0x851U)        /* 请求卡认证 */
#define IOT_XDT_SET_CATEGORY                     (0x853U)        /* 策略设置 */
#define IOT_XDT_CHARGE_START_RSP		         (0x862U)        /* 充电启动响应 */
#define IOT_XDT_CHARGE_START_EVNET               (0x863U)        /* 充电启动事件 */
#define IOT_XDT_CHARGE_STOP_RSP					 (0x865U)        /* 充电停止响应 */
#define IOT_XDT_CHARGE_STOP_EVNET                (0x866U)        /* 充电停止事件 */
#define IOT_XDT_CHARGE_CONTINUE_CHARGE_RSP       (0x868U)        /* 续充命令响应 */
#define IOT_XDT_QUERY_BOARDINFO_RSP              (0x872U)        /* 查询控制板信息响应 */
#define IOT_XDT_PARA_SET_RSP                     (0x874U)        /* 参数设置响应 */
#define IOT_XDT_PARA_QUERY_RSP                   (0x876U)        /* 参数查询响应 */
#define IOT_XDT_CHARGE_PWRCTRL_RSP               (0x8612U)       /* 功率控制请求响应 */
#define IOT_XDT_CHARGE_RECORD                    (0x8613U)       /* 充电记录 */
#define IOT_XDT_QUERY_CHARGE_RECORD_RSP          (0x8616U)       /* 查询充电记录响应 */
#define IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE        (0x882U)        /* 升级属性请求 */
#define IOT_XDT_CMD_FIRMWARE_STATE               (0x886U)        /* 固件过程数据 */

#define IOT_XDT_CMD_SEND_COUNT                   (27U)

/* 协议CMD 接收定义 */
#define IOT_XDT_CMD_QUERY_ATTACH_CREDENTIAL_RSP  (0x812U)        /* 查询接入凭据响应 */
#define IOT_XDT_CMD_REQUEST_TIMESYNC_RSP         (0x822U)        /* 请求时间同步响应 */
#define IOT_XDT_CMD_REQUEST_LINK_RSP             (0x824U)        /* 请求链接响应 */
#define IOT_XDT_CMD_SET_RESTART                  (0x825U)        /* 重启设置响*/
#define IOT_XDT_CMD_REQUEST_RATEMODE_RSP         (0x832U)        /* 请求计费模型响应 */
#define IOT_XDT_CMD_QUERY_RATEMODE               (0x833U)        /* 查询计费模型 */
#define IOT_XDT_CMD_RATEMODE_SET                 (0x834U)        /* 计费模型设置 */
#define IOT_XDT_CMD_PILE_STATE_RSP               (0x842U)        /* 桩状态响应 */
#define IOT_XDT_CMD_ERRINFO_RSP                  (0x844U)        /* 错误信息响应 */   
#define IOT_XDT_CMD_CALL_REALDATA_RSP            (0x847U)        /* 召测实时数据响应 */
#define IOT_XDT_CMD_REQUEST_CARDAUTH_RSP         (0x852U)        /* 请求卡认证响应 */
#define IOT_XDT_SET_CATEGORY_RSP                 (0x854U)        /* 策略设置响应 */
#define IOT_XDT_CHARGE_START					 (0x861U)        /* 充电启动 */
#define IOT_XDT_CHARGE_STOP						 (0x864U)        /* 充电停止 */
#define IOT_XDT_CHARGE_CONTINUE_CHARGE           (0x867U)        /* 续充命令 */
#define IOT_XDT_QUERY_BOARDINFO					 (0x871U)        /* 控制板信息请求 */
#define IOT_XDT_PARA_SET                         (0x873U)        /* 参数设置 */
#define IOT_XDT_PARA_QUERY                       (0x875U)        /* 参数查询 */
#define IOT_XDT_CHARGE_PWRCTRL                   (0x8611U)       /* 功率控制请求 */
#define IOT_XDT_CHARGE_RECORD_RSP                (0x8614U)       /* 充电记录响应 */
#define IOT_XDT_QUERY_CHARGE_RECORD              (0x8615U)       /* 查询充电记录 */
#define IOT_XDT_CMD_OTA_ATTRIBUTE_SET            (0x881U)        /* 升级属性订阅 */
#define IOT_XDT_CMD_REQUEST_OTA_ATTRIBUTE_RSP    (0x883U)        /* 升级属性请求响应 */

#define IOT_XDT_CMD_RECV_COUNT                   (23U)


/* 日志接口函数定义 */
#define IOTXDT_CFG_LogPrint(fmt, ...)             DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)


#define IOT_XDT_CheckKeyIsNull(key, keyName, ret, pAns)                   \
{\
	if (key == NULL)\
	{\
		if (pAns != NULL)\
		{\
			pAns[0] = eIotXDTErrCode_ParaMissing;\
		}\
		cJSON_Delete(cRoot);\
		IOTXDT_CFG_LogPrint("[%s()]: Failed to find the key [%s]\r\n", __FUNCTION__, keyName);\
		return ret;\
	} \
}

#define IOT_XDT_CheckObjIsNull(obj, ret)                   \
{\
	if (obj == NULL)\
	{\
		cJSON_Delete(cRoot);\
		IOTXDT_CFG_LogPrint("[%s()]: Failed to Creat JSON object\r\n", __FUNCTION__);\
		return ret;\
	} \
}

#define  IOT_XDT_CheckJsonPrint(cRoot, pJson, ret)    \
{\
	if (pJson == NULL)\
	{\
		IOTXDT_CFG_LogPrint("[%s()]: Failed to print JSON object\r\n", __FUNCTION__);\
		cJSON_Delete(cRoot);\
		return ret;\
	} \
}

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eIotXDTErrCode_Success = 0,
	eIotXDTErrCode_PileCodeNoRegister = 1,
	eIotXDTErrCode_Foibid = 2,
	eIotXDTErrCode_OnCharging = 3,
	eIotXDTErrCode_RateModePeriodParaErr = 4,
	eIotXDTErrCode_ParaMissing = 5,	
	eIotXDTErrCode_ParaInvalid = 6,		
	eIotXDTErrCode_RecvAbnormal = 7,		
	eIotXDTErrCode_PsWErr = 8,
	eIotXDTErrCode_ErcodeNotSet = 9,
	eIotXDTErrCode_ErcodeFormatErr = 10,
	eIotXDTErrCode_PileError = 11,
	eIotXDTErrCode_FuncNotSupport = 12,
	eIotXDTErrCode_PlatformError = 13,
	eIotXDTErrCode_PileNotStartCharge = 14,
	eIotXDTErrCode_GunNotConnect = 15,
	eIotXDTErrCode_OrderNumIsInvalid = 16,
	eIotXDTErrCode_PlatformParaErr = 17,
	eIotXDTErrCode_AccountIsNotExsist = 18,
	eIotXDTErrCode_AccountIsFreeze = 19,
	eIotXDTErrCode_InsufficientBalance = 20,
	eIotXDTErrCode_AccountIsCance = 21,
	eIotXDTErrCode_VinIsNotExsist = 22,
	eIotXDTErrCode_GunCantUse = 23,
	eIotXDTErrCode_CardCantUseInDevice = 24,
	eIotXDTErrCode_AccountCantUse = 25,
	eIotXDTErrCode_BulidOrderFailure = 26,
	eIotXDTErrCode_ParashortPileNum = 27,
	eIotXDTErrCode_ParaCardNumIsNull = 28,
	eIotXDTErrCode_VinCardNotBindAccount = 29,
	eIotXDTErrCode_AccountVinFuncNotEnable = 30,
	eIotXDTErrCode_ThirdPartyAuthFailure = 31,
	eIotXDTErrCode_Cnt = 32,
}IotXDTErrCodeList_Enum;

typedef enum 
{
	eXDTReportState_Null = 0,
	eXDTReportState_ToReport = 1,
	eXDTReportState_Reporting = 2,	
}IotXDTReportState_Enum;

typedef enum
{
	eIotXDTParamType_t1,
	eIotXDTParamType_t2,	
	eIotXDTParamType_t18,
	eIotXDTParamType_t40,
	eIotXDTParamType_t41,
	eIotXDTParamType_t42,
	eIotXDTParamType_Count,
}IotXDTParamType_Enum;

typedef enum
{
	eIotXDTParamOptType_Read,
	eIotXDTParamOptType_Write,
}IotXDTParamOptType_Enum;

typedef enum
{
	eIotXDTPileStatus_Idle = 0,
	eIotXDTPileStatus_Work = 1,
	eIotXDTPileStatus_Fix = 2,
    eIotXDTPileStatus_Error = 3,
}IotXDTPileStatus_Enum;

typedef enum	
{
	eIotXDTGunStatus_Idle = 0,
	eIotXDTGunStatus_Connected = 1,
	eIotXDTGunStatus_Charging = 2,
	eIotXDTGunStatus_ChargeFinish = 3,
	eIotXDTGunStatus_AppointMent = 7,
	eIotXDTGunStatus_Error = 255,
}IotXDTGunStatus_Enum;

typedef enum
{
	eIotXDTOtaState_Idle,
	eIotXDTOtaState_Starting,
	eIotXDTOtaState_Succ,
	eIotXDTOtaState_Fail,
}IotXDTOtaState_Enum;

typedef enum 
{
	eIotXDTErrorLevel_Ctrtical = 0,
	eIotXDTErrorLevel_Major = 1,
	eIotXDTErrorLevel_Minor = 2,
	eIotXDTErrorLevel_Warning = 3,
}IOTXDTErrorLevel_Enum;

typedef enum 
{
	eIotXDTEntityType_Pile = 0,
	eIotXDTEntityType_Gun = 1,
}IOTXDTEntityType_Enum;

typedef enum 
{
	eIotXDTStopReason_Null = 0,
	eIotXDTStopReason_PlatformStop = 1,
	eIotXDTStopReason_ChargeFull = 2,
	eIotXDTStopReason_EmergeStop = 3,
	eIotXDTStopReason_CarStop = 4,
	eIotXDTStopReason_ReachMoney = 5,
	eIotXDTStopReason_ReachElec = 6,
	eIotXDTStopReason_ReachTime = 7,
	eIotXDTStopReason_LocalStop = 8,
	eIotXDTStopReason_PileErr = 9,
	eIotXDTStopReason_InsuffcientFund = 10,
	eIotXDTStopReason_ReachSOC = 11,
	eIotXDTStopReason_CarErr = 12,
	eIotXDTStopReason_Other = 13,
	eIotXDTStopReason_GunDisconnect = 14,
}eIotXDTStopReason_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
	char *topic;
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    uint16_t (*pSendFunc)(uint8_t port, void *pBuf);
	uint16_t matchCmd;
    char *cMeaning;
}IotXDTSendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	char *matchStr;
	uint8_t cmdType; 
    uint8_t (*pRecvParse)(uint8_t port, uint8_t *r_data, uint16_t len);
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    char *cMeaning;
}IotXDTRecvCtrl_Struct;

typedef struct
{
	char *topic;
	uint8_t memberCnt;
	uint8_t cmdType;
	IotXDTRecvCtrl_Struct *pStrRecvCtrlTable;
}IotXDTRecvTopic_Struct;

typedef struct
{
	char begin[6];
	char end[6];
	char flag[2];
	char elec[8];
	char ser[8];
}IotXDTPeriodList_Struct;

typedef struct
{
	uint8_t billModeID[4];
	uint8_t typeRule;
	char stdElec[8]; 
	char stdSer[8];
	uint8_t periodCnt;
	IotXDTPeriodList_Struct periodListInfo[MSNVM_XDT_BILLMODE_PERIOD_COUNT];
}IotXDTRateMode_Struct;

typedef struct
{
	uint8_t optFlag;
	char keyName[6];
	IotXDTParamType_Enum eParaType;
	char paramString[IOT_XDT_PARAM_MAX_LEN + 1];
	IotXDTErrCodeList_Enum eAns;
}IOTXDTParamOpt_Struct;

typedef struct
{
	uint8_t errIndex;
	uint8_t *p;
	uint8_t callFlag;
	uint8_t status;
	IotXDTReportState_Enum eReportState;
}IotXDTErrInfoReport_Struct;

typedef struct 
{
	IotXDTErrCodeList_Enum eAns_Item825;  
	uint8_t force_Item825;

	IotXDTErrCodeList_Enum eAns_Item833;    
	uint8_t type_ITEM833;

	IotXDTErrCodeList_Enum eAns_Item834;
	uint8_t type_ITEM834;
	MSNvmXDTParamBillMode_Struct strGnRate_Item834;

	IotXDTErrInfoReport_Struct errInfoReportQueue[IOT_XDT_ERRINFO_REPORT_QUEUE_SIZE];
	uint8_t errClearInfoReportFlag;

	uint32_t chargeTotalMoney;
	
	IotXDTErrCodeList_Enum eAns_ITEM846;
	uint8_t type_ITEM846;
	uint8_t mode_ITEM846;

	double balance_ITEM852;

	IotXDTErrCodeList_Enum eAns_ITEM85A;
	uint8_t gunNo_ITEM85A;

	IotXDTErrCodeList_Enum eAns_ITEM861;
	uint8_t gunNo_ITEM861;
	uint8_t initiator_ITEM861;
	uint8_t type_ITEM861;
	uint8_t typeComsume_ITEM861;
	double comsume_ITEM861;
	char user_ITEM861[IOT_XDT_USERNUM_LEN + 1];
	uint8_t plan_ITEM861;
	uint8_t typePlan_ITEM861;
	uint8_t typeStart_ITEM861;
	uint32_t tsStart_ITEM861;
	double value_ITEM861;
	char passwdStop[IOT_XDT_PASSWDSTOP_LEN + 1];
	char orderNo_ITEM861[IOT_XDT_ORDERNUM_LEN + 1];
	char fDetail_ITEM861[IOT_XDT_FAULTCODE_LEN + 1];

	IotXDTErrCodeList_Enum eAns_ITEM864;
	uint8_t gunNo_ITEM864;
	char orderNo_ITEM864[IOT_XDT_ORDERNUM_LEN + 1];
	char  fDetail_ITEM865[IOT_XDT_FAULTCODE_LEN + 1];

	IotXDTErrCodeList_Enum eAns_ITEM867;
	uint8_t gunNo_ITEM867; 
	uint8_t initiator_ITEM867;
	uint8_t typePlan_ITEM867;
	double value_ITEM867;
	char user_ITEM867[IOT_XDT_USERNUM_LEN + 1];
	char orderNo_ITEM867[IOT_XDT_ORDERNUM_LEN + 1];
	
	IotXDTErrCodeList_Enum eAns_ITEM8611;
	uint8_t gunNo_ITEM8611;
	double powerControl_ITEM8611;
	uint8_t type_ITEM8611;
	uint8_t typeControl_ITEM8611;
	
	IotXDTErrCodeList_Enum eAns_ITEM8615;
	uint8_t gunNo_ITEM8615;
	char orderNo_ITEM8615[IOT_XDT_ORDERNUM_LEN + 1];
	MSNvmOrderInfo_Struct queryChargeRecord;

	IotXDTErrCodeList_Enum eAns_ITEM871;

	IOTXDTParamOpt_Struct  paramOpt[eIotXDTParamType_Count];
}IotXDTDataOfflineClr_Struct;

typedef struct
{
	char fw_state_ITEM886[IOT_XDT_FWSTATE_LEN + 1];
}IotXDTDataOfflineNotClear_Struct;


typedef struct
{
	IotXDTDataOfflineClr_Struct offlineClearData;
	IotXDTDataOfflineNotClear_Struct offlineNotClearData;
}IotXDTRecvData_Struct;

typedef struct
{
	AswErrorType_Enum eErrorCode;
	IOTXDTEntityType_Enum eEntity;
	uint8_t errNo;
	IOTXDTErrorLevel_Enum eLevel;
	char * alarmDesc;
	uint8_t lastStatus[SYSCFG_CFG_GUN_NUM];
}IotXDTErrDesc_Struct;

typedef struct 
{
	AswErrorType_Enum eCommonStopReason;
	eIotXDTStopReason_Enum eXDTStopReason;
}IotXDTStopReasonMap_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* ASW_IOT_PROTO_XDT_TYPES_H_ */





















