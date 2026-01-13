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
#ifndef ASW_IOT_PROTO_GN_TYPES_H_
#define ASW_IOT_PROTO_GN_TYPES_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ASWGN_CFG_LogPrint(fmt, ...)            DSLOGM_Debug(DSLogMModule_Proto, fmt, ##__VA_ARGS__)

#define IOT_GN_TXRX_BUFFER_SIZE                 (3072U)


#define IOT_GN_PROTOCOL_VERSION                 (10008U)

#define IOT_GN_PLUS_HEAD1                       (0x5AU)                
#define IOT_GN_PLUS_HEAD2                       (0xA5U)

#define IOT_GN_HEAD1                            (0xFAU)                
#define IOT_GN_HEAD2                            (0xAFU)


#define IOT_GN_CMDTYPE_REQUSET			        (0x00U)
#define IOT_GN_CMDTYPE_RESPONSE                 (0x01U)

#define IOT_GN_CMD_NULL                         (0x00U)             /* 无效 */

#define IOT_GN_CMD_LOGIN_REQ                    (0x01U)             /* 登陆 */
#define IOT_GN_CMD_SEND_COUNT                   (0x01U)

#define IOT_GN_CMD_LOGIN_RSP                    (0x02U)             /* 登陆应答 */
#define IOT_GN_CMD_RECV_COUNT                   (0x01U)

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef uint16_t (*IotGN_pSendPackFuncType)(uint8_t port, uint8_t *pBuf);
typedef uint8_t (*IotGN_pRecvParseFuncType)(uint8_t *port, uint8_t *r_data, uint16_t len);

typedef struct
{
    uint16_t cmd;
	uint8_t cmdType;
    uint32_t sendCycle;
    IotGN_pSendPackFuncType pSendFunc;
	uint16_t matchCmd;
    char *cMeaning;
}IotGNSendCtrl_Struct;

typedef struct 
{
	uint16_t cmd;
	uint8_t cmdType; 
	IotGN_pRecvParseFuncType pRecvParse;
	uint16_t maxTimeout;
	uint16_t maxTryCnt;
	uint16_t matchCmd;
    char *cMeaning;
}IotGNRecvCtrl_Struct;

typedef struct 
{
    uint8_t head[2];
    uint8_t version[2];
    uint8_t seq[2];
    uint8_t encryptFlag;
    uint8_t cmd;
    uint8_t dataLen[2];
}IotGNFrameHead_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

#endif /* ASW_IOT_PROTO_GN_TYPES_H_ */






















