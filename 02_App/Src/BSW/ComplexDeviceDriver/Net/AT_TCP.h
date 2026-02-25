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
#ifndef AT_TCP_H_
#define AT_TCP_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "AT_Describtor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ATTCP_CYCLE_READ_PERIOD          3000

#define ATTCP_CYCLE_WRITE_PERIOD         200

#define ATTCP_WAIT_IPOPEN_TIMEOUT        90000

#define ATTCP_DECTECT_STATE_PERIOD       3000

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eATTCPCmd_Null,
    eATTCPCmd_Open,
    eATTCPCmd_Read,
    eATTCPCmd_Write,
    eATTCPCmd_Close,
    eATTCPCmd_QueryState,
    eATTCPCmd_Count,
}ATTcpCmd_Enum;












/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const ATCmdDescribtor_Struct c_stTCPATCmdDescribtor[eATTCPCmd_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void ATTCP_StateHandle(uint8_t socketIndex, void *socketCtrl);
void ATTCP_CloseSocket(void *socketCtrl);

void ATTCP_UrcQIPOpen(uint8_t *pData, void * modulePara, uint16_t dataLen);
void ATTCP_UrcSendOK(uint8_t *pData, void * modulePara, uint16_t dataLen);
void ATTCP_UrcClose(uint8_t *pData, void * modulePara, uint16_t dataLen);
void ATTCP_UrcRecv(uint8_t *pData, void * modulePara, uint16_t dataLen);
#endif





















