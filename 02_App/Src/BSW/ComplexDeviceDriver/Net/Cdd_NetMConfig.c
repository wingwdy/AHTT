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
#include "Cdd_NetMConfig.h"
#include "Cdd_Drv_EG800AK.h"


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



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/

const CddNetMModuleOps_Struct c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_COUNT] = {
    [CDD_NETM_CFG_DEV_4G] = 
    {
        .pFuncMainFunction = CddDrvEG800AK_MainFunction,
        .getModuleState = CddDrvEG800AK_GetModuleState,
        .getSocketState = CddDrvEG800AK_GetSocketState,
        .setSocketDisconnect = CddDrvEG800AK_SetSocketDisconnect,
        .creatSocket = CddDrvEG800AK_CreatSocket,
        .delAllSocket = CddDrvEG800AK_DelAllSocket,
        .delSingleSocket = CddDrvEG800AK_DelSingleSocket,
        .getIccid = CddDrvEG800AK_GetIccid,
        .getCsq = CddDrvEG800AK_GetCsq,
        .getOperator = CddDrvEG800AK_GetOperatorType,
        .getModuleTypeInfo = CddDrvEG800AK_GetModuleTypeInfo,
        .updateMqttUserNamePassword = CddDrvEG800AK_UpdateMqttUserNamePassword,
        .updateIpPort = CddDrvEG800AK_UpdateIpPort,
    },
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
























