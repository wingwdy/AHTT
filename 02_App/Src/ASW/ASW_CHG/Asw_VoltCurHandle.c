/******************************************************************************
* File Name          : Asw_VoltCurHandle.c
* Description        : Code for VoltCurHandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      shenjc    初版创建
*
*******************************************************************************/
#include "Asw_VoltCurHandleConfig.h"
#include "SysCfg.h"
#include "Filter.h"
#include "Asw_ErrorHandle.h"

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#define ASWTEMP_WORK_STATE_1            0 /*额定状态*/
#define ASWTEMP_WORK_STATE_2            1 /*降流状态*/
#define ASWTEMP_WORK_STATE_3            2 /*停止状态*/


/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint16_t arSampleValue[AswVoltCur_EvtCnt];
    FilterProfile1_Struct arfilter[AswVoltCur_EvtCnt];
}AswVoltCurHandle_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswVoltCurHandle_Struct g_arAswVoltCurHandle[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

void AswVoltCur_InitMemory(void)
{
    memset(g_arAswVoltCurHandle, 0, sizeof(g_arAswVoltCurHandle));

}


void AswVoltCur_Manage(uint8_t port)
{
    uint8_t i = 0;
    uint8_t tempStatus = 0;
    AswVoltCurHandle_Struct *pHandle = &g_arAswVoltCurHandle[port];

    pHandle->arSampleValue[AswVoltCur_OV_Set] = 0;
    pHandle->arSampleValue[AswVoltCur_OV_Clr] = 0;
    pHandle->arSampleValue[AswVoltCur_UV_Set] = 0;
    pHandle->arSampleValue[AswVoltCur_UV_Clr] = 0;
    pHandle->arSampleValue[AswVoltCur_OC_Set] = 0;
    pHandle->arSampleValue[AswVoltCur_OC_Clr] = 0;

    for (i = 0; i < AswVoltCur_EvtCnt; i++)
    {
        switch(c_AswVoltCurHandleConfigTable[i].compareType)
        {
            case AswVoltCur_MaxEqu:
            {
                tempStatus = CHECK_MAX_EQU(pHandle->arSampleValue[i], c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCur_Max:
            {
                tempStatus = CHECK_MAX(pHandle->arSampleValue[i], c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCur_MinEqu:
            {
                tempStatus = CHECK_MIN_EQU(pHandle->arSampleValue[i], c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCur_Min:
            {
                tempStatus = CHECK_MIN(pHandle->arSampleValue[i], c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            case AswVoltCur_Equ:
            {
                tempStatus = CHECK_EQU(pHandle->arSampleValue[i], c_AswVoltCurHandleConfigTable[i].threshold);
                break;
            }
            default:
            {
                break;
            }
        }

        pHandle->arfilter[i].status = tempStatus;
        Filter_Profile1(&pHandle->arfilter[i], c_AswVoltCurHandleConfigTable[i].filterCount);

        if (pHandle->arfilter[i].validStatus == TRUE)
        {
            if(c_AswVoltCurHandleConfigTable[i].setErrorFlag == TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, c_AswVoltCurHandleConfigTable[i].errorType);
            }
            else
            {
                AswErrhandle_ResetErrExsitCallback(port, c_AswVoltCurHandleConfigTable[i].errorType);
            }
        }
    }
}

void AswVoltCurHandle_MainFunction(void)
{
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswVoltCur_Manage(port);
    }
}

