/******************************************************************************
* File Name          : template_Config.h
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
#ifndef CDD_NETM_CONFIG_H_
#define CDD_NETM_CONFIG_H_


/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Cdd_NetM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/



#define CCDD_NETM_CFG_LINK_COUNT                 3

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    void (*pFuncMainFunction)(void);
	CddNetMModuleState_Enum (*getModuleState)(void);
	CddNetMSocketState_Enum (*getSocketState)(uint8_t socketIndex);
	void (*setSocketDisconnect)(uint8_t socketIndex);
	uint8_t (*creatSocket)(CddNetMSocketType_Enum socketType, CddNetMSocketPara_Union *pSocketPara, 
        uint8_t *pSocketIndex, CddNetMPlatType_Enum ePlatType);
	void (*delAllSocket)(void);
	void (*delSingleSocket)(uint8_t socketIndex);
	void (*getIccid)(char *pICCID);
	uint8_t (*getCsq)(void);
	CddNetMOperator_Enum (*getOperator)(void);
	void (*getModuleTypeInfo)(char *ModuleType, uint16_t readLen);
	void (*updateMqttUserNamePassword)(uint8_t socketIndex, char *pUserName, char *pPassword);
	void (*updateIpPort)(uint8_t socketIndex, char *pIp, uint16_t port);
}CddNetMModuleOps_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddNetMModuleOps_Struct c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_COUNT];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* CDD_NETM_CONFIG_H_ */























