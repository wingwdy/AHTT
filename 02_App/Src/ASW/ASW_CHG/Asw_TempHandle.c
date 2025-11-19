/******************************************************************************
* File Name          : Asw_TempHandle.c
* Description        : Code for TempHandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      chenls    初版创建
*
*******************************************************************************/
#include "Asw_TempHandleConfig.h"
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
typedef enum
{
    AswGunTempThr60 = 0,
    AswGunTempThr75,
    AswGunTempThr90,
    AswGunTempThr105,
    AswGunTempThrMax,
} AswGunTempThr_Enum;

typedef enum
{
    AswEnvTempThr65 = 0,
    AswEnvTempThr85,
    AswEnvTempThrMax,
} AswEnvTempThr_Enum;

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t temperatue;
    uint8_t workState;
    uint8_t preWorkState;
    uint8_t limitCurrentFlag;
    uint32_t level0_Tick;
    uint32_t level1_Tick;
    uint8_t arFilterThr[AswEnvTempThrMax];
    FilterProfile1_Struct arFilterState[AswEnvTempThrMax];
}AswEnvTempHandle_Struct;

typedef struct 
{
    uint8_t temperatue;
    uint8_t workState;
    uint8_t preWorkState;
    uint8_t limitCurrentFlag;
    uint32_t level0_Tick;
    uint32_t level1_Tick;
    uint8_t arFilterThr[AswGunTempThrMax];
    FilterProfile1_Struct arFilterState[AswGunTempThrMax];
}AswGunTempHandle_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswEnvTempHandle_Struct g_arAswEnvTempHandle[SYSCFG_CFG_GUN_NUM] = {0};
static AswGunTempHandle_Struct g_arAswGunTempHandle[SYSCFG_CFG_GUN_NUM] = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

void AswTempHandle_InitMemory(void)
{
    uint8_t i = 0;
    memset(g_arAswEnvTempHandle, 0, sizeof(g_arAswEnvTempHandle));
    memset(g_arAswGunTempHandle, 0, sizeof(g_arAswGunTempHandle));
    for(i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
    {
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr60] = ASWTEMP_CFG_GUN_OTEMP_60_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr75] = ASWTEMP_CFG_GUN_OTEMP_75_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr90] = ASWTEMP_CFG_GUN_OTEMP_90_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr105] = ASWTEMP_CFG_GUN_OTEMP_105_THR;

        g_arAswEnvTempHandle[i].arFilterThr[AswEnvTempThr65] = ASWTEMP_CFG_ENV_OTEMP_65_THR;
        g_arAswEnvTempHandle[i].arFilterThr[AswEnvTempThr85] = ASWTEMP_CFG_ENV_OTEMP_85_THR;
    }
}


void AswGunTemp_Manage(uint8_t port)
{
    uint8_t i = 0;
    AswGunTempHandle_Struct *pTempHandle = &g_arAswGunTempHandle[port];
    
    pTempHandle->temperatue = ASWTEMP_CFG_GetGunTemp(port);
    
    for (i = 0; i < AswGunTempThrMax; i++)
    {
        pTempHandle->arFilterState[i].status = (pTempHandle->temperatue >= pTempHandle->arFilterThr[i]) ? TRUE : FALSE;
        Filter_Profile1(&pTempHandle->arFilterState[i], ASWTEMP_CFG_GUN_TEMP_FILTER_COUNT);
    }

    switch(pTempHandle->workState)
    {
        case ASWTEMP_WORK_STATE_1:
        {
            if (pTempHandle->arFilterState[AswGunTempThr90].validStatus == TRUE)
            {/* 大于阈值 */
                if (pTempHandle->arFilterState[AswGunTempThr105].validStatus == TRUE)
                {/* 大于阈值 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_3;
                }
                else
                {
                    pTempHandle->limitCurrentFlag = TRUE;
                    pTempHandle->workState = ASWTEMP_WORK_STATE_2;
                    pTempHandle->level1_Tick = Common_GetSystick();
                    pTempHandle->level0_Tick = Common_GetSystick();
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_2:
        {
            if (pTempHandle->arFilterState[AswGunTempThr105].validStatus == TRUE)
            {/* 大于阈值*/
                pTempHandle->workState = ASWTEMP_WORK_STATE_3;
            }

            if (pTempHandle->limitCurrentFlag == FALSE && pTempHandle->arFilterState[AswGunTempThr90].validStatus == FALSE)
            {/* 小于阈值 */
                pTempHandle->level1_Tick = Common_GetSystick();
            }
            /*温度高于90,持续5分钟*/
            if (Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_GUN_ABOVE_90_KEEP_TIME))
            {/* 5分钟后 */
                if (pTempHandle->arFilterState[AswGunTempThr90].validStatus == TRUE)
                {/* 大于阈值 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_3;
                }
                pTempHandle->limitCurrentFlag = FALSE;
            }

            if (pTempHandle->arFilterState[AswGunTempThr75].validStatus == TRUE)
            {/* 大于阈值*/
                pTempHandle->level0_Tick = Common_GetSystick();
            }
            /* 温度低于75,持续5分钟 */
            if (Common_JudgeTimeoutMs(pTempHandle->level0_Tick, ASWTEMP_CFG_GUN_BELOW_75_KEEP_TIME))
            {/* 持续5分钟 */
                if (pTempHandle->arFilterState[AswGunTempThr75].validStatus == FALSE)
                {/* 小于阈值 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_1;
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_3:
        {
            if (pTempHandle->arFilterState[AswGunTempThr60].validStatus == FALSE)
            {/* 小于阈值 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_1;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if(pTempHandle->workState != pTempHandle->preWorkState)
    {
        pTempHandle->preWorkState = pTempHandle->workState;
        if(pTempHandle->workState == ASWTEMP_WORK_STATE_3)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_GunOverTempErr);
        }
        else if(pTempHandle->workState == ASWTEMP_WORK_STATE_1)
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_GunOverTempErr);
        }
        else
        {}
    }
}

void AswEnvTemp_Manage(uint8_t port)
{
    uint8_t i = 0;
    AswEnvTempHandle_Struct *pTempHandle = &g_arAswEnvTempHandle[port];
    
    pTempHandle->temperatue = ASWTEMP_CFG_GetEnvTemp(port);
    for (i = 0; i < AswEnvTempThrMax; i++)
    {
        pTempHandle->arFilterState[i].status = pTempHandle->temperatue >= pTempHandle->arFilterThr[i]? TRUE:FALSE;
        Filter_Profile1(&pTempHandle->arFilterState[i], ASWTEMP_CFG_ENV_TEMP_FILTER_COUNT);
    }

    switch(pTempHandle->workState)
    {
        case ASWTEMP_WORK_STATE_1:
        {
            if (pTempHandle->arFilterState[AswEnvTempThr85].validStatus == TRUE)
            {/* 大于阈值 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_2;
                pTempHandle->level1_Tick = Common_GetSystick();
                pTempHandle->level0_Tick = Common_GetSystick();
                pTempHandle->limitCurrentFlag = TRUE;
            }
            break;
        }
        case ASWTEMP_WORK_STATE_2:
        {
            if (pTempHandle->limitCurrentFlag == FALSE && pTempHandle->arFilterState[AswEnvTempThr85].validStatus == FALSE)
            {/* 小于阈值 */
                pTempHandle->level1_Tick = Common_GetSystick();
            }
            /*温度高于85,持续5分钟*/
            if (Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_ENV_ABOVE_85_KEEP_TIME))
            {/* 5分钟后 */
                if (pTempHandle->arFilterState[AswEnvTempThr85].validStatus == TRUE)
                {/* 大于阈值 停机 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_3;
                }
                pTempHandle->limitCurrentFlag = FALSE;
            }

            if (pTempHandle->arFilterState[AswEnvTempThr65].validStatus == TRUE)
            {/* 大于阈值*/
                pTempHandle->level0_Tick = Common_GetSystick();
            }
            /*温度低于65,持续5分钟*/
            if (Common_JudgeTimeoutMs(pTempHandle->level0_Tick, ASWTEMP_CFG_ENV_BELOW_65_KEEP_TIME))
            {/* 持续5分钟 */
                if(pTempHandle->arFilterState[AswEnvTempThr65].validStatus == FALSE)
                {/* 小于阈值 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_1;
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_3:
        {
            if (pTempHandle->arFilterState[AswEnvTempThr65].validStatus == FALSE)
            {/* 小于阈值 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_1;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}


uint16_t AswTempHandle_SetCurrent(uint8_t port)
{
    uint16_t current = 32000;/*32A*/

    AswGunTempHandle_Struct *pGunTempHandle = &g_arAswGunTempHandle[port];
    AswEnvTempHandle_Struct *pEnvTempHandle = &g_arAswEnvTempHandle[port];

    if (pGunTempHandle->workState == ASWTEMP_WORK_STATE_2 
    || pEnvTempHandle->workState == ASWTEMP_WORK_STATE_2)
    {
        current = 25000;
    }
    else if(pGunTempHandle->workState == ASWTEMP_WORK_STATE_3)
    {
        current = 0;
    }
    else
    {
        current = 32000;   
    }

    return current;
}


void AswTempHandle_MainFunction(void)
{
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswGunTemp_Manage(port);
        AswEnvTemp_Manage(port);
    }
}


