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
#ifndef ASW_IOT_PROTO_GNM_H_
#define ASW_IOT_PROTO_GNM_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoGNTypes.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
	eIOTGNWorkState_Init,
	eIOTGNWorkState_Offline,
	eIOTGNWorkState_Login,
	eIOTGNWorkState_Normal,
}IotGNWorkState_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t frameQueueChannelID;
    IotGNWorkState_Enum eWorkState;
    typeFuncSendCtrl pFuncSendCtrl;
    typeFuncRecvCtrl pFuncRecvCtrl;
    CommonSendCtrl_Struct stSendCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_SEND_COUNT];
    CommonRecvCtrl_Struct stRecvCtrl[SYSCFG_CFG_GUN_NUM][IOT_GN_CMD_RECV_COUNT];
}IotGNCtx_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

void IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara);
void IotGN_InitMemory(void);
void IotGN_MainFunction(void);
#endif





















