/******************************************************************************
* File Name          : template.h
* Description        : Code for Net Manage
*                      架构设计预留以太网的可能性
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
#ifndef CDD_NETM_H_
#define CDD_NETM_H_


/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Global.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_NETM_CFG_DEV_4G                      0

#if 0
#define CDD_NETM_CFG_DEV_ETH                     1
#endif

#define CDD_NETM_CFG_DEV_COUNT                   1


#define CDD_NETM_CFG_IP_LEN						 72

/* config for FTP */
#define CDD_NETM_CFG_FTP_FILENAME_LEN            64
#define CDD_NETM_CFG_FTP_PATH_LEN                48
#define CDD_NETM_CFG_FTP_USERNAME_LEN            24
#define CDD_NETM_CFG_FTP_PASSWD_LEN              24

/* config for MQTT*/
#define CDD_NETM_CFG_MQTT_TOPIC_COUNT            7
#define CDD_NETM_CFG_MQTT_TOPIC_LEN              128
#define CDD_NETM_CFG_MQTT_DEVICE_NAME_LEN        32
#define CDD_NETM_CFG_MQTT_USER_NAME_LEN          64
#define CDD_NETM_CFG_MQTT_PASSWORD_LEN           256
#define CDD_NETM_CFG_MQTT_PID_LEN                192

/* config for HTTP */
#define CDD_NETM_CFG_HTTP_URL_LEN                512 /* 国网e充电OTA请求url巨长(390字节左右) */
#define CDD_NETM_CFG_HTTP_BODY_LEN               1024
/******************************************************************************
*    Enum Definition
******************************************************************************/
//服务商
typedef enum 
{
	eCddNetMOperator_Null,
	eCddNetMOperator_CMCC,					/* 移动 */
	eCddNetMOperator_CUCC,					/* 联通 */
	eCddNetMOperator_CTCC,					/* 电信 */
	eCddNetMOperator_Other,				    /* 其他 */
}CddNetMOperator_Enum;


typedef enum
{
	eCddNetMModuleState_Init,
	eCddNetMModuleState_Cfg,
	eCddNetMModuleState_Work,
	eCddNetMModuleState_AbNormal,
}CddNetMModuleState_Enum;

typedef enum
{
    eCddNetMSocketType_Null,
	eCddNetMSocketType_TCP,
    eCddNetMSocketType_MQTT,
    eCddNetMSocketType_FTP,
    eCddNetMSocketType_HTTP,
	eCddNetMSocketType_Count,
}CddNetMSocketType_Enum;

typedef enum 
{
	eCddNetMSocketState_Init,
	eCddNetMSocketState_Connecting,
	eCddNetMSocketState_ConnectOK,
	eCddNetMSocketState_Abnormal,
    eCddNetMSocketState_WaitReconnect,
}CddNetMSocketState_Enum;

typedef enum
{
	eCddNetMFtpMode_Download,
	eCddNetMFtpMode_Upload,
}CddNetMFtpMode_Enum;

typedef enum
{
    eCddNetMMqttVersion_V3_1,
    eCddNetMMqttVersion_V3_1_1,
}CddNetMMqttVersion_Enum;

typedef enum
{
    eCddNetMPlatType_Null,
    eCddNetMPlatType_O,
    eCddNetMPlatType_OM,
    eCddNetMPlatType_File,
}CddNetMPlatType_Enum;

typedef enum
{
    eCddNetMFileType_Null,
    eCddNetMFileType_BIN,     /* 二进制文件 建议用这个*/
    eCddNetMFileType_ASCLL,   /* 文本文件，windows系统会进行转义*/
}CddNetMFileType_Enum;

typedef enum
{
    eCddNetMHttpType_Null = 0,
    eCddNetMHttpType_GET,       /* 全量请求 */
    eCddNetMHttpType_SegGET,    /* 分段请求 */
    eCddNetMHttpType_POST,

}CddNetMHttpType_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    uint8_t frameQueueChannelID;
	char ip[CDD_NETM_CFG_IP_LEN + 1];
	uint16_t port;
}CddNetMTcpPara_Struct;

typedef struct
{
    CddNetMFtpMode_Enum eMode;
    CddNetMFileType_Enum eFileFormat;
    uint16_t port;
	char ip[CDD_NETM_CFG_IP_LEN + 1];
	char user[CDD_NETM_CFG_FTP_USERNAME_LEN + 1];
	char passwd[CDD_NETM_CFG_FTP_PASSWD_LEN + 1];
    char fileName[CDD_NETM_CFG_FTP_FILENAME_LEN + 1];
	char path[CDD_NETM_CFG_FTP_PATH_LEN + 1];
}CddNetMFtpPara_Struct;

typedef struct
{
    uint8_t frameQueueChannelID;
    CddNetMMqttVersion_Enum eVersion;
    uint16_t keepAliveTime;
    uint16_t port;
    char ip[CDD_NETM_CFG_IP_LEN + 1];
    uint8_t topicCount;
    char topic[CDD_NETM_CFG_MQTT_TOPIC_COUNT][CDD_NETM_CFG_MQTT_TOPIC_LEN + 1];
    char deviceName[CDD_NETM_CFG_MQTT_DEVICE_NAME_LEN + 1];
    char userName[CDD_NETM_CFG_MQTT_USER_NAME_LEN + 1];
    char password[CDD_NETM_CFG_MQTT_PASSWORD_LEN + 1];
    char pid[CDD_NETM_CFG_MQTT_PID_LEN + 1];
    void (*pFuncMqttConnectCallback)(uint8_t connectResult, uint8_t *pCredential);
}CddNetMMqttPara_Struct;

typedef struct
{
    CddNetMHttpType_Enum type;
    uint16_t urlLen;
    char     url[CDD_NETM_CFG_HTTP_URL_LEN + 1];
    uint16_t bodyLen;
    char     body[CDD_NETM_CFG_HTTP_BODY_LEN + 1];
    uint8_t  dataReady;
    uint32_t segPos;        /* 分段下载偏移(SegGET专用) */
    uint16_t segLen;        /* 分段下载长度(SegGET专用) */
}CddNetMHttpPara_Struct;


typedef union 
{
    CddNetMMqttPara_Struct stMqttPara;
    CddNetMFtpPara_Struct stFtpPara;
    CddNetMTcpPara_Struct stTcpPara;
    CddNetMHttpPara_Struct stHttpPara;
}CddNetMSocketPara_Union;




/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint8_t CddNetM_CheckLinkConnectOK(CddNetMPlatType_Enum ePlatType);
void CddNetM_SwitchPhyChannel(uint8_t moduleDev);
GlobalRet_Enum CddNetM_CreatLink(CddNetMSocketType_Enum eSocketType, CddNetMSocketPara_Union socketPara, CddNetMPlatType_Enum ePlatType);
void CddNetM_DeleteLink(CddNetMPlatType_Enum ePlatType);
void CddNetM_SetLinkDisconnect(CddNetMPlatType_Enum ePlatType);
void CddNetM_MainFunction(void);
uint16_t CddNetM_GetCsq(void);
void CddNetM_GetIMEI(uint8_t *pIMEI);
uint8_t CddNetM_CheckFileLinkExsit(void);
void CddNetM_GetIccid(uint8_t *pICCID);
CddNetMOperator_Enum CddNetM_GetOperatorType(void);
void CddNetM_GetModuleTypeInfo(char *ModuleTypeInfo, uint16_t readLen);
void CddNetM_UpdateMqttUserNamePassword(CddNetMPlatType_Enum ePlatType, char *pUserName, char *pPassword);
void CddNetM_UpdateIpPort(CddNetMPlatType_Enum ePlatType, char *pIp, uint16_t port);
void CddNetM_SetSimNet(uint8_t simNet);
CddNetMHttpPara_Struct* CddNetM_GetHttpPara(CddNetMPlatType_Enum ePlatType);
#endif /* CDD_NETM_H_ */























