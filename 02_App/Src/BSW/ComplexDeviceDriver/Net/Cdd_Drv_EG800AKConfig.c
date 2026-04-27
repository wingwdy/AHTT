/******************************************************************************
* File Name          : Cdd_Drv_EG800AK.c
* Description        : Code for EG800AK Driver
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
#include "Cdd_Drv_EG800AK.h"
#include "Cdd_Drv_EG800AKConfig.h"
#include "AT_Describtor.h"
#include "AT_Module.h"
#include "AT_TCP.h"
#include "AT_FTP.h"
#include "AT_MQTT.h"
/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/



/**********************CDDDRV_EG800AK**************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
const CddDrvEG800AKSocketConfig_Struct c_stCddDrvEG800AKSocketConfigTable[eCddNetMSocketType_Count] =
{
    [eCddNetMSocketType_Null] = 
    {
        .cmdTaskCount = eATModuleCmd_QueryCount,
        .pATCmdDescribtorTable = c_stModuleATCmdDescribtor,
        .stateHandle = NULL,
        .socketCloseHandle = NULL,
        .recvTransparentData = NULL,
    },

    [eCddNetMSocketType_TCP] = 
    {
        .cmdTaskCount = eATTCPCmd_Count,
        .pATCmdDescribtorTable = c_stTCPATCmdDescribtor,
        .stateHandle = ATTCP_StateHandle,
        .socketCloseHandle = ATTCP_CloseSocket,
        .recvTransparentData = NULL,
    },

    [eCddNetMSocketType_MQTT] = 
    {
        .cmdTaskCount = eATMQTTCmd_Count,
        .pATCmdDescribtorTable = c_stMQTTATCmdDescribtor,
        .stateHandle = ATMQTT_StateHandle,
        .socketCloseHandle = ATMQTT_CloseSocket,
        .recvTransparentData = NULL,
        .updateIpPort = ATMQTT_UpdateIpPort,
        .updateMqttUserNamePassword = ATMQTT_UpdateMqttUserNamePassword,
    },

    [eCddNetMSocketType_FTP] = 
    {
        .cmdTaskCount = eATFTPCmd_Count,
        .pATCmdDescribtorTable = c_stFTPATCmdDescribtor,
        .stateHandle = ATFTP_StateHandle,
        .socketCloseHandle = ATFTP_CloseSocket,
        .recvTransparentData = ATFTP_ReavTransparentData,
    },
};

void CDDDRVEG800AK_CFG_WriteData(uint8_t *pData, uint16_t len, void *userData)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)userData;
    /*
        CDDDRV_EG800AK_CFG_DebugPrint("[socket: %d]Send Data[%d]: ", pSocketCtrl->socketIndex, len);
        DSLogM_HexOutput(pData, len);
    */
    CDDDRV_EG800AK_CFG_WriteData(pData, len);
}



/*******************************************************************************
*    Function Source Code
*******************************************************************************/



























