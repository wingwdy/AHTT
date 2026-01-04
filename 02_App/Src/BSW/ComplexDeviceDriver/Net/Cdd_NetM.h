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
#include "Cdd_NetMConfig.h"
#include "Global.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_NETM_DEV_4G              0
#define CDD_NETM_DEV_DEFAULT         CDD_NETM_DEV_4G
// #define CDD_NETM_DEV_ETH          1 
#define CDD_NETM_DEV_COUNT           1

/******************************************************************************
*    Enum Definition
******************************************************************************/

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
	eCddNetMSocketType_Count,
}CddNetMSocketType_Enum;

typedef enum 
{
	eCddNetMSocketState_Init,
	eCddNetMSocketState_Connecting,
	eCddNetMSocketState_ConnectOK,
	eCddNetMSocketState_Abnormal,
    eCddNetMSocketState_Delete,
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
    uint8_t fileType;
    uint16_t port;
	char ip[CDD_NETM_CFG_IP_LEN + 1];
	char user[CDD_NETM_CFG_FTP_FILENAME_LEN + 1];
	char passwd[CDD_NETM_CFG_FTP_PATH_LEN + 1];
    char fileName[CDD_NETM_CFG_FTP_USERNAME_LEN + 1];
	char path[CDD_NETM_CFG_FTP_PASSWD_LEN + 1];
}CddNetMFtpPara_Struct;

typedef struct
{
    uint8_t frameQueueChannelID;
    CddNetMMqttVersion_Enum eVersion;
    uint16_t keepAliveTime;
    uint8_t topicCount;
    char topic[CDD_NETM_CFG_MQTT_TOPIC_COUNT][CDD_NETM_CFG_MQTT_TOPIC_LEN + 1];
    char deviceName[CDD_NETM_CFG_MQTT_DEVICE_NAME_LEN + 1];
    char productKey[CDD_NETM_CFG_MQTT_PRODUCT_SECRET_LEN + 1];
    char productSecret[CDD_NETM_CFG_MQTT_PRODUCT_SECRET_LEN + 1];
    char pid[CDD_NETM_CFG_MQTT_PID_LEN + 1];
}CddNetMMqttPara_Struct;

typedef union 
{
    CddNetMMqttPara_Struct stMqttPara;
    CddNetMFtpPara_Struct stFtpPara;
    CddNetMTcpPara_Struct stTcpPara;
    uint8_t socketParaBuf[CDD_NETM_CFG_LINKPARA_LEN];
}CddNetMSocketPara_Union;


typedef struct
{
	CddNetMModuleState_Enum (*getModuleState)(void);
	CddNetMSocketState_Enum (*getSocketState)(uint8_t socketIndex);
	void (*setSocketDisconnect)(uint8_t socketIndex);
	uint8_t (*creatSocket)(CddNetMSocketType_Enum socketType, CddNetMSocketPara_Union *pSocketPara, uint8_t *pSocketIndex);
	void (*delAllSocket)(void);
	void (*delSingleSocket)(uint8_t socketIndex);
}CddNetMModuleOps_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
GlobalRet_Enum CddNetM_CreatLink(CddNetMSocketType_Enum eSocketType, CddNetMSocketPara_Union socketPara, CddNetMPlatType_Enum ePlatType);



#endif /* CDD_NETM_H_ */























