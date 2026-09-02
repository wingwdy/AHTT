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
#ifndef AT_MQTT_H_
#define AT_MQTT_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "AT_Describtor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

#define ATMQTT_CYCLE_WRITE_PERIOD         200

#define ATMQTT_WAIT_OPEN_TIMEOUT          90000

#define ATMQTT_WAIT_CONNECT_TIMEOUT       90000

#define ATMQTT_DECTECT_STATE_PERIOD       3000
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eATMQTTCmd_NULL,
    eATMQTTCmd_ConfigDataFormat,
    eATMQTTCmd_ConfigVersion,
    eATMQTTCmd_ConfigPing,
    eATMQTTCmd_ConfigKeepAlive,
    eATMQTTCmd_ConfigCleanSession,
    eATMQTTCmd_ConfigRecvMode,
    eATMQTTCmd_Open,
    eATMQTTCmd_Connect,
    eATMQTTCmd_Subscribe,
    eATMQTTCmd_Publish,
    eATMQTTCmd_QueryState,
    eATMQTTCmd_Close,
    eATMQTTCmd_Count,
}ATMQTTCmd_Enum;






/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const ATCmdDescribtor_Struct c_stMQTTATCmdDescribtor[eATMQTTCmd_Count];

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void ATMQTT_CloseSocket(void *socketCtrl);
void ATMQTT_StateHandle(uint8_t socketIndex, void *socketCtrl);
uint32_t ATMQTT_UrcQMTOpen(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATMQTT_UrcQMTConnect(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATMQTT_UrcQMTPubex(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATMQTT_UrcQMTStat(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATMQTT_UrcQMTRecv(uint8_t *pData, void * modulePara, uint16_t dataLen);
void ATMQTT_UpdateIpPort(void *socketPara, char *pIp, uint16_t port);
void ATMQTT_UpdateMqttUserNamePassword(void *socketPara, char *pUserName, char *pPassword);
#endif





















