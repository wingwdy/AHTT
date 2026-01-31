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
*2025/11/12      V1.0.0      shenjc    初版创建
*
*******************************************************************************/
#include "Asw_TempHandleConfig.h"
#include "Asw_ErrorHandle.h"
#include "SysCfg.h"
#include "Filter.h"
#include "Cdd_CP.h"

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#define ASWTEMP_WORK_STATE_NORMAL               0 /* 正常状态 */
#define ASWTEMP_WORK_STATE_LIMITCURR_A          1 /* 限流状态A */
#define ASWTEMP_WORK_STATE_LIMITCURR_B          2 /* 限流状态B */
#define ASWTEMP_WORK_STATE_FAULT                3 /* 停充状态 */


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
    uint32_t level0_Tick;
    uint32_t level1_Tick;
    uint8_t arFilterThr[AswGunTempThrMax];
    FilterProfile1_Struct arFilterState[AswGunTempThrMax];
}AswGunTempHandle_Struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static AswEnvTempHandle_Struct g_arAswEnvTempHandle = {0};
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
    memset(&g_arAswEnvTempHandle, 0, sizeof(g_arAswEnvTempHandle));
    memset(g_arAswGunTempHandle, 0, sizeof(g_arAswGunTempHandle));
    for(i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
    {
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr60] = ASWTEMP_CFG_GUN_OTEMP_60_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr75] = ASWTEMP_CFG_GUN_OTEMP_75_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr90] = ASWTEMP_CFG_GUN_OTEMP_90_THR;
        g_arAswGunTempHandle[i].arFilterThr[AswGunTempThr105] = ASWTEMP_CFG_GUN_OTEMP_105_THR;
    }
    g_arAswEnvTempHandle.arFilterThr[AswEnvTempThr65] = ASWTEMP_CFG_ENV_OTEMP_65_THR;
    g_arAswEnvTempHandle.arFilterThr[AswEnvTempThr85] = ASWTEMP_CFG_ENV_OTEMP_85_THR;
}


void AswGunTemp_Manage(uint8_t port)
{
    uint8_t i = 0;
    AswGunTempHandle_Struct *pTempHandle = &g_arAswGunTempHandle[port];
    
    pTempHandle->temperatue = ASWTEMP_CFG_GetGunTemp(port);
    
    for (i = 0; i < AswGunTempThrMax; i++)
    {
        pTempHandle->arFilterState[i].status = (pTempHandle->temperatue > pTempHandle->arFilterThr[i]) ? TRUE : FALSE;
        Filter_Profile1(&pTempHandle->arFilterState[i], ASWTEMP_CFG_GUN_TEMP_FILTER_COUNT);
    }

    switch(pTempHandle->workState)
    {
        case ASWTEMP_WORK_STATE_NORMAL:
        {
            if (pTempHandle->arFilterState[AswGunTempThr90].validStatus == TRUE)
            {/* 温度大于90 */
                if (pTempHandle->arFilterState[AswGunTempThr105].validStatus == TRUE)
                {/* 温度大于105 进入停充 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                    ASWTEMP_CFG_LogPrint("[枪：%d]当前充电枪温度%d℃大于%d℃,[正常] --> [故障]\r\n", port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr105] - 50);
                }
                else
                {/* 温度大于90且小于105 进入限流 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_LIMITCURR_A;
                    pTempHandle->level1_Tick = Common_GetSystick();
                    pTempHandle->level0_Tick = Common_GetSystick();
                    ASWTEMP_CFG_LogPrint("[枪：%d]当前充电枪温度%d℃大于%d℃,[正常] --> [限流A]\r\n", port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr90] - 50);
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_LIMITCURR_A:
        {
            if (pTempHandle->arFilterState[AswGunTempThr105].validStatus == TRUE)
            {/* 温度大于105，进入停充*/
                pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                ASWTEMP_CFG_LogPrint("[枪：%d]当前充电枪温度%d℃大于%d℃,[限流A] --> [故障]\r\n", port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr105] - 50);
            }
            if (Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_GUN_ABOVE_90_KEEP_TIME))
            {
                if (pTempHandle->arFilterState[AswGunTempThr90].validStatus == TRUE)
                { /* 5分钟后温度大于90，进入停充 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                    ASWTEMP_CFG_LogPrint("[枪：%d]5分钟后,当前充电枪温度%d℃大于%d℃, [限流A] --> [故障]\r\n", port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr90] - 50);
                }
                else
                {/* 温度小于90，继续限流 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_LIMITCURR_B;
                    pTempHandle->level1_Tick = Common_GetSystick();
                    pTempHandle->level0_Tick = Common_GetSystick();
                    ASWTEMP_CFG_LogPrint("[枪：%d]5分钟后,当前充电枪温度%d℃小于%d℃, [限流A] --> [限流B]\r\n", port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr90] - 50);
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_LIMITCURR_B:
        {
            if (pTempHandle->arFilterState[AswGunTempThr90].validStatus == FALSE)
            {/* 温度小于90，重置停充计时器 */
                pTempHandle->level1_Tick = Common_GetSystick();
            }

            /* 温度大于90持续5分钟或温度大于105，进入停充 */
            if (pTempHandle->arFilterState[AswGunTempThr105].validStatus == TRUE ||
                Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_GUN_ABOVE_90_KEEP_TIME))
            {
                pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                ASWTEMP_CFG_LogPrint("[枪：%d]当前充电枪温度%d℃大于%d℃且持续5分钟 或 温度大于%d℃, [限流B] --> [故障]\r\n", 
                    port,pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr90] - 50,
                    g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr105] - 50);
            }

            if (pTempHandle->arFilterState[AswGunTempThr75].validStatus == TRUE)
            {/* 温度大于75，重置计时器 */
                pTempHandle->level0_Tick = Common_GetSystick();
            }
            else
            {
                /* 温度小于75，持续5分钟，恢复正常 */
                if (Common_JudgeTimeoutMs(pTempHandle->level0_Tick, ASWTEMP_CFG_GUN_BELOW_75_KEEP_TIME))
                {
                    pTempHandle->workState = ASWTEMP_WORK_STATE_NORMAL;
                    ASWTEMP_CFG_LogPrint("[枪：%d]当前充电枪温度%d℃小于%d℃且持续5分钟, [限流B] --> [正常]\r\n", 
                        port, pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr75] - 50);  
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_FAULT:
        {
			CddCPVolState_Enum eCPState = CddCP_GetVolState(port);
            if (pTempHandle->arFilterState[AswGunTempThr60].validStatus == FALSE && eCPState == eCddCPVolState_12V)
            {/* 温度小于60度，恢复正常 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_NORMAL;
                ASWTEMP_CFG_LogPrint("[枪：%d]拔枪且当前充电枪温度%d℃小于%d℃, [故障] --> [正常]\r\n", 
                    port,pTempHandle->temperatue - 50, g_arAswGunTempHandle[port].arFilterThr[AswGunTempThr60] - 50); 
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (pTempHandle->workState != pTempHandle->preWorkState)
    {
        pTempHandle->preWorkState = pTempHandle->workState;
        if (pTempHandle->workState == ASWTEMP_WORK_STATE_FAULT)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_GunOverTempErr);
        }
        else
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_GunOverTempErr);
        }
    }
}

void AswEnvTemp_Manage(void)
{
    uint8_t i = 0;
	uint8_t port = 0;
    AswEnvTempHandle_Struct *pTempHandle = &g_arAswEnvTempHandle;
    
    pTempHandle->temperatue = ASWTEMP_CFG_GetEnvTemp();
    for (i = 0; i < AswEnvTempThrMax; i++)
    {
        pTempHandle->arFilterState[i].status = pTempHandle->temperatue > pTempHandle->arFilterThr[i]? TRUE:FALSE;
        Filter_Profile1(&pTempHandle->arFilterState[i], ASWTEMP_CFG_ENV_TEMP_FILTER_COUNT);
    }

    switch(pTempHandle->workState)
    {
        case ASWTEMP_WORK_STATE_NORMAL:
        {
            if (pTempHandle->arFilterState[AswEnvTempThr85].validStatus == TRUE)
            {/* 温度大于85度，进入限流 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_LIMITCURR_A;
                pTempHandle->level1_Tick = Common_GetSystick();
                pTempHandle->level0_Tick = Common_GetSystick();
                ASWTEMP_CFG_LogPrint("当前环境温度%d℃大于%d℃, [正常] --> [限流A]\r\n", 
                    pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr85] - 50);
            }
            break;
        }
        case ASWTEMP_WORK_STATE_LIMITCURR_A:
        {
            if (Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_ENV_ABOVE_85_KEEP_TIME))
            {
                if (pTempHandle->arFilterState[AswEnvTempThr85].validStatus == TRUE)
                {/* 5分钟后，温度大于85度，进入停充 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                    ASWTEMP_CFG_LogPrint("5分钟后当前环境温度%d℃大于%d℃, [限流A] --> [故障]\r\n", 
                        pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr85] - 50);
                }
                else
                {/* 5分钟后，温度小于85度，继续限流 */
                    pTempHandle->workState = ASWTEMP_WORK_STATE_LIMITCURR_B;
                    pTempHandle->level1_Tick = Common_GetSystick();
                    pTempHandle->level0_Tick = Common_GetSystick();
                    ASWTEMP_CFG_LogPrint("5分钟后当前环境温度%d℃小于%d℃, [限流A] --> [限流B]\r\n", 
                        pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr85] - 50);
                }
            }
            break;
        }        
        case ASWTEMP_WORK_STATE_LIMITCURR_B:
        {
            if (pTempHandle->arFilterState[AswEnvTempThr85].validStatus == FALSE)
            {
                pTempHandle->level1_Tick = Common_GetSystick();
            }
            else
            {
                /* 温度大于85度，持续5分钟， 进入停充 */
                if (Common_JudgeTimeoutMs(pTempHandle->level1_Tick, ASWTEMP_CFG_ENV_ABOVE_85_KEEP_TIME))
                {
                    pTempHandle->workState = ASWTEMP_WORK_STATE_FAULT;
                    ASWTEMP_CFG_LogPrint("5分钟后当前环境温度%d℃大于%d℃, [限流B] --> [故障]\r\n", 
                        pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr85] - 50);
                }
            }

            if (pTempHandle->arFilterState[AswEnvTempThr65].validStatus == TRUE)
            {
                pTempHandle->level0_Tick = Common_GetSystick();
            }
            else
            {
                /* 温度小于65度，持续5分钟， 恢复正常*/
                if (Common_JudgeTimeoutMs(pTempHandle->level0_Tick, ASWTEMP_CFG_ENV_BELOW_65_KEEP_TIME))
                {
                    pTempHandle->workState = ASWTEMP_WORK_STATE_NORMAL;
                    ASWTEMP_CFG_LogPrint("5分钟后当前环境温度%d℃小于%d℃, [限流B] --> [正常]\r\n", 
                        pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr65] - 50);
                }
            }
            break;
        }
        case ASWTEMP_WORK_STATE_FAULT:
        {
			CddCPVolState_Enum eCPState = CddCP_GetVolState(port); /* 双枪时注意拔哪个枪 或者双枪都要拔*/
            if (pTempHandle->arFilterState[AswEnvTempThr65].validStatus == FALSE && eCPState == eCddCPVolState_12V)
            {/* 温度小于65度， 恢复正常 */
                pTempHandle->workState = ASWTEMP_WORK_STATE_NORMAL;
                ASWTEMP_CFG_LogPrint("拔枪且当前环境温度%d℃小于%d℃, [故障] --> [正常]\r\n", 
                    pTempHandle->temperatue - 50, pTempHandle->arFilterThr[AswEnvTempThr65] - 50);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (pTempHandle->workState != pTempHandle->preWorkState)
    {
        pTempHandle->preWorkState = pTempHandle->workState;
        if (pTempHandle->workState == ASWTEMP_WORK_STATE_FAULT)
        {
            AswErrhandle_SetErrExsitCallback(port, eErr_EnvOverTempErr);
        }
        else
        {
            AswErrhandle_ResetErrExsitCallback(port, eErr_EnvOverTempErr);
        }
    }

}

LimitCurrentLevel_Enum AswTempHandle_GetLimitCurrentLevel(uint8_t port)
{
    AswGunTempHandle_Struct *pGunTempHandle = &g_arAswGunTempHandle[port];
    AswEnvTempHandle_Struct *pEnvTempHandle = &g_arAswEnvTempHandle;
    
    if ((pGunTempHandle->workState == ASWTEMP_WORK_STATE_LIMITCURR_A)
    ||  (pGunTempHandle->workState == ASWTEMP_WORK_STATE_LIMITCURR_B)
    ||  (pEnvTempHandle->workState == ASWTEMP_WORK_STATE_LIMITCURR_A)
    ||  (pEnvTempHandle->workState == ASWTEMP_WORK_STATE_LIMITCURR_B))
    {
        return AswLimitCurrLevelOne;
    }

    return AswLimitCurrLevelZero;
}


void AswTempHandle_MainFunction(void)
{
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        AswGunTemp_Manage(port);
    }
    AswEnvTemp_Manage();
}


