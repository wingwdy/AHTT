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
#include "SS_Snapshot.h"
#include "SS_SnapshotConfig.h"
#include "SS_Tm.h"

#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "Asw_ErrorHandle.h"
#include "Cdd_ModeM.h"
#include "Cdd_PE.h"
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
*    Global variables Declaration
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void SSSnapshot_Cfg_PackErrStr(uint8_t port, char *pStr, uint16_t bufLen)
{
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    uint32_t voltage = AswChargeIf_GetInputVoltage(port);
    uint32_t current = AswChargeIf_GetOutputCurrent(port);
    uint8_t gunTemp = AswChargeIf_GetGunTemperature(port);
    uint8_t envTemp = AswChargeIf_GetEnvTemperature();
    uint16_t cpVol = AswChargeIf_GetCpVoltage(port);
    uint16_t cpDuty = AswChargeIf_GetCpDuty(port);
    uint32_t chargeTime = pChargeData->chargeTime;
    CddPEState_Enum ePeState = CddPE_GetConnectState();
    char peInfo[32] = {0};

    if (ePeState == eCddPEState_PEConnect)
    {
        strcpy(peInfo, "接地正常");
    }
    else if (ePeState == eCddPEState_PEUnconn)
    {
        strcpy(peInfo, "接地异常");
    }
    else
    {
        strcpy(peInfo, "接地未知");
    }

    snprintf(pStr, bufLen, "电压:%d.%02d V, 电流:%d.%03d A, %s, %s, CP电压:%d.%03d V, CP占空比:%d.%01d%%, 枪温:%d ℃, 壳温:%d ℃, 充电时长:%d 秒, 网络状态:%s\r\n", 
        voltage / 100, voltage % 100, 
        current / 1000, current % 1000,
       (CddModeM_IsGBMode() == TRUE) ? "国标模式" : "兼容模式", 
        peInfo, cpVol / 1000, cpVol % 1000,
        cpDuty / 10, cpDuty % 10,
       gunTemp - 50, envTemp - 50,
       chargeTime,
       AswErrHandle_CheckErrExit(port, eErr_PlatformOffline) ? "离线" : "在线");
}


















