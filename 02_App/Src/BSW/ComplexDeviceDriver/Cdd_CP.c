/******************************************************************************
* File Name          : Cdd_CP.c
* Description        : Code for Control Pilot
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
#include "Cdd_CP.h"
#include "Common.h"
#include "SysCfg.h"
#include "Cdd_CPConfig.h"
#include "Filter.h"
#include "Asw_ErrorHandle.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDDCP_DIODE_DETECT_STEP0        0
#define CDDCP_DIODE_DETECT_STEP1        1
#define CDDCP_DIODE_DETECT_STEP2        2

#define CDDCP_WAKEUP_STEP0              0
#define CDDCP_WAKEUP_STEP1              1
#define CDDCP_WAKEUP_STEP2              2
#define CDDCP_WAKEUP_STEP3              3
/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/

typedef struct 
{
    uint16_t volStatefilerTimer;              /* cp状态滤波计数器 */
    CddCPVolState_Enum eTempCpVolState;       /* 滤波前的cp状态 */

    uint8_t wakeupStep;                       /* cp唤醒步骤 */
    uint32_t wakeupTick;                      /* cp唤醒计时 */

    uint8_t cpPWMOutputEnable;                /* cp PWM 输出使能 */

    uint8_t diodeDetectStep;                  /* 二极管检测步骤 */
    uint8_t diodeDetectResult;                /* 二极管检测结果 */
    uint32_t diodeDetectStartTick;            /* 二极管检测CP拉-12V计时 */
    FilterProfile1_Struct diodeFilter;        /* 二极管检测结果滤波 */

    uint16_t curSetCpDuty;                    /* 当前CP设置占空比 */ 
    uint16_t cpVol;                           /* CP电压，保留3位小数 */ 
    CddCPVolState_Enum eValidCpVolState;      /* CP电压状态 */
    uint16_t curAjustCurrent;                 /* 当前调节电流值，放大1000倍 */
}CddCPCtrl_Struct; 

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddCPCtrl_Struct g_stCddCPCtrl[SYSCFG_CFG_GUN_NUM];

const struct 
{
    char *cName;
}c_cpStateName[] = 
{
    {"接地态"},
    {"6V态"},
    {"9V态"},
    {"12V态"},
    {"故障态"},
    {"初始态"},
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void CddCP_SetPwmDuty(uint8_t port, uint16_t duty);
static float CddCP_GetVol(uint8_t port);
static void CddCP_WakeupHandle(uint8_t port, CddCPCtrl_Struct *pCPCtrl);
static void CddCP_DiodeExsitDetect(uint8_t port, CddCPCtrl_Struct *pCPCtrl);
static void CddCP_VolStateHandle(uint8_t port, CddCPCtrl_Struct *pCPCtrl);
static void CddCP_CPVolErrDetect(uint8_t port, CddCPCtrl_Struct *pCPCtrl);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void CddCP_CPVolErrDetect(uint8_t port, CddCPCtrl_Struct *pCPCtrl)
{
    if (pCPCtrl->eValidCpVolState == eCddCPVolState_Ground)
    {
        AswErrhandle_SetErrExsitCallback(port, eErr_CpGroundFault);
        AswErrhandle_ResetErrExsitCallback(port, eErr_CpVoltAbnor);
    }
    else if (pCPCtrl->eValidCpVolState == eCddCPVolState_Err)
    {
        AswErrhandle_SetErrExsitCallback(port, eErr_CpVoltAbnor);
        AswErrhandle_ResetErrExsitCallback(port, eErr_CpGroundFault);
    }
    else
    {
        AswErrhandle_ResetErrExsitCallback(port, eErr_CpVoltAbnor);
        AswErrhandle_ResetErrExsitCallback(port, eErr_CpGroundFault);
    }

    if (pCPCtrl->eValidCpVolState == eCddCPVolState_12V)
    {
        AswErrhandle_ResetErrExsitCallback(port, eErr_DiodeStop);
    }
}

static void CddCP_SetPwmDuty(uint8_t port, uint16_t duty)
{
    CddCPCtrl_Struct *pCpCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pCpCtrl = &g_stCddCPCtrl[port];

        if (pCpCtrl->curSetCpDuty != duty)
        {
            CDDCP_CFG_LogPrint("[枪：%d]CP PWM使能：%d, CP 输出占空比：%d.%d%%\r\n", port, 
                pCpCtrl->cpPWMOutputEnable, duty / 10, duty % 10);
            pCpCtrl->curSetCpDuty = duty;
        }

        if (c_stCddCPOpsConfigTable.pFunSetPwmDuty != NULL)
        {
            c_stCddCPOpsConfigTable.pFunSetPwmDuty(port, duty);
        }
    }
}

static float CddCP_GetVol(uint8_t port)
{
    float ret = 0.0;
    
    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (c_stCddCPOpsConfigTable.pFuncGetCpVol != NULL)
        {
            ret = c_stCddCPOpsConfigTable.pFuncGetCpVol(port);
        }
    }

    return ret;
}

static void CddCP_WakeupHandle(uint8_t port, CddCPCtrl_Struct *pCPCtrl)
{
    switch (pCPCtrl->wakeupStep)
    {
    case CDDCP_WAKEUP_STEP0:
    {
        break;
    }
    case CDDCP_WAKEUP_STEP1:
    {
        CddCP_SetPwmDuty(port, 0);
        pCPCtrl->wakeupTick = Common_GetSystick();
        pCPCtrl->wakeupStep = CDDCP_WAKEUP_STEP2;
        break;
    }
    case CDDCP_WAKEUP_STEP2:
    {
        if (Common_JudgeTimeoutMs(pCPCtrl->wakeupTick, CDDCP_CFG_WAKEUP_LOW_HOLDTIME))
        {
            CddCP_SetPwmDuty(port, 1000);
            pCPCtrl->wakeupTick = Common_GetSystick();
            pCPCtrl->wakeupStep = CDDCP_WAKEUP_STEP3;
        }

        break;
    }
    case CDDCP_WAKEUP_STEP3:
    {
        if (Common_JudgeTimeoutMs(pCPCtrl->wakeupTick, CDDCP_CFG_WAKEUP_HIGH_HOLDTIME))
        {
            CddCP_StartPWM(port);
            pCPCtrl->wakeupStep = CDDCP_WAKEUP_STEP0;
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

static void CddCP_DiodeExsitDetect(uint8_t port, CddCPCtrl_Struct *pCPCtrl)
{
    switch (pCPCtrl->diodeDetectStep)
    {
    case CDDCP_DIODE_DETECT_STEP0:
    {
        break;
    }

    case CDDCP_DIODE_DETECT_STEP1:
    {
        CddCP_SetPwmDuty(port, 0);
        pCPCtrl->diodeDetectStartTick = Common_GetSystick();
        pCPCtrl->diodeDetectStep = CDDCP_DIODE_DETECT_STEP2;
        break;
    }

    case CDDCP_DIODE_DETECT_STEP2:
    {
        if (Common_JudgeTimeoutMs(pCPCtrl->diodeDetectStartTick, CDDCP_CFG_DIODE_DETECT_TIMEOUT))
        {
            pCPCtrl->diodeDetectStep = CDDCP_DIODE_DETECT_STEP0;
            pCPCtrl->diodeDetectResult = GLOBAL_OPT_STATE_FAIL;
            CddCP_SetPwmDuty(port, 1000);
            CDDCP_CFG_LogPrint("[枪：%d]二极管存在性检测超时[%d]ms，该车不存在二极管！当前CP电压：%d.%d V\r\n",
            port, CDDCP_CFG_DIODE_DETECT_TIMEOUT, pCPCtrl->cpVol / 1000, pCPCtrl->cpVol % 1000);
            AswErrhandle_SetErrExsitCallback(port, eErr_DiodeStop);

        }
        else
        {
            if (pCPCtrl->cpVol > CDDCP_CFG_DIODE_THREOLD)
            {
                pCPCtrl->diodeFilter.status = TRUE;
            }
            else
            {
                pCPCtrl->diodeFilter.status = FALSE;
            }

            if (Filter_Profile1(&pCPCtrl->diodeFilter, CDDCP_CFG_DIODE_FILTER_POINT))
            {
                pCPCtrl->diodeDetectStep = CDDCP_DIODE_DETECT_STEP0;
                pCPCtrl->diodeDetectResult = GLOBAL_OPT_STATE_SUCCESS;
                CddCP_SetPwmDuty(port, 1000);
                CDDCP_CFG_LogPrint("[枪：%d]二极管存在性检测成功！\r\n", port);
            }
        }

        break;
    }

    default:
    {
        break;
    }
    }
}

static void CddCP_VolStateHandle(uint8_t port, CddCPCtrl_Struct *pCPCtrl)
{
    const CddCPVolStateFilter_Struct *pCurVolStateFilter = NULL;
    const CddCPVolStateFilter_Struct *pTempVolStateFilter = NULL;
    const CddCPVolStateFilter_Struct *pMap = NULL;
    uint8_t curState = (pCPCtrl->eTempCpVolState == eCddCPVolState_Err) ? 0 : ((uint8_t)(pCPCtrl->eTempCpVolState));
    uint8_t index = curState;
    uint8_t findFlag = FALSE;

    pCPCtrl->cpVol = abs((int16_t)(CddCP_GetVol(port) * 1000));

    if (pCPCtrl->curSetCpDuty != 0)
    {
        pMap = (CDDCP_CFG_IsQBStandardMode() == TRUE) ? c_stCddCPVolStateFilterQB : c_stCddCPVolStateFilterGB;
        pCurVolStateFilter = &pMap[curState];

        if (pCPCtrl->cpVol <= pCurVolStateFilter->upperVolLimit && pCPCtrl->cpVol >= pCurVolStateFilter->lowerVolLimit)
        {
            findFlag = TRUE;
        }
        else if (pCPCtrl->cpVol > pCurVolStateFilter->upperVolLimit)
        {
            for (index = curState; index < ARRAY_SIZE(c_stCddCPVolStateFilterGB); index++)
            {
                pTempVolStateFilter = &pMap[index];

                if (pCPCtrl->cpVol >= pTempVolStateFilter->lowerVolLimit && pCPCtrl->cpVol <= pTempVolStateFilter->upperVolLimit)
                {
                    findFlag = TRUE;
                    break;
                }
            }
        }
        else if (pCPCtrl->cpVol < pCurVolStateFilter->lowerVolLimit)
        {
            for (index = curState; index > 0; index--)
            {
                pTempVolStateFilter = &pMap[index];

                if (pCPCtrl->cpVol >= pTempVolStateFilter->lowerVolLimit && pCPCtrl->cpVol <= pTempVolStateFilter->upperVolLimit)
                {
                    findFlag = TRUE;
                    break;
                }
            }
        }
        else
        {}

        index = (findFlag == FALSE) ? ((uint8_t)(eCddCPVolState_Err)) : index;

        if ((CddCPVolState_Enum)index != pCPCtrl->eTempCpVolState)
        {
            if (index >= eCddCPVolState_Err)
            {
                pCPCtrl->volStatefilerTimer = (CDDCP_CFG_IsQBStandardMode() == TRUE) ? CDDCP_CFG_QB_FILERCNT : CDDCP_CFG_GB_FILERCNT;
            }
            else
            {
                pCPCtrl->volStatefilerTimer = pMap[index].statefiterCnt;
            }

            pCPCtrl->eTempCpVolState = (CddCPVolState_Enum)index;
        }

        if (pCPCtrl->volStatefilerTimer > 0)
        {
            pCPCtrl->volStatefilerTimer--;

            if (pCPCtrl->volStatefilerTimer == 0)
            {
                if (pCPCtrl->eValidCpVolState != pCPCtrl->eTempCpVolState)
                {
                    CDDCP_CFG_LogPrint("[枪：%d]CP电压状态：%s ---> %s, 当前CP电压值：%d.%d V\r\n",
                    port, c_cpStateName[pCPCtrl->eValidCpVolState], c_cpStateName[pCPCtrl->eTempCpVolState]
                    , pCPCtrl->cpVol/1000, pCPCtrl->cpVol % 1000);
                    pCPCtrl->eValidCpVolState = pCPCtrl->eTempCpVolState;
                    CddCP_CPVolErrDetect(port, pCPCtrl);
                }
            }
        }
    }
}

void CddCP_InitMemory(void)
{
    CddCPCtrl_Struct *pCpCtrl = NULL;
    uint8_t port = 0;

    memset(g_stCddCPCtrl, 0, sizeof(g_stCddCPCtrl));

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pCpCtrl = &g_stCddCPCtrl[port];

        CddCP_SetPwmDuty(port, 1000);
        pCpCtrl->curAjustCurrent = CDDCP_CFG_RATE_CURRENT;
        pCpCtrl->eValidCpVolState = eCddCPVolState_Init;
    }
}

void CddCP_MainFunction(void)
{
    CddCPCtrl_Struct *pCpCtrl = NULL;
    uint8_t port = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pCpCtrl = &g_stCddCPCtrl[port];
        CddCP_VolStateHandle(port, pCpCtrl);  
        CddCP_DiodeExsitDetect(port, pCpCtrl);
        CddCP_WakeupHandle(port, pCpCtrl);
    }
}

void CddCP_AdjustCurRateCurrent(uint8_t port, uint32_t current)
{
    CddCPCtrl_Struct *pCpCtrl = NULL;
    uint16_t duty = 0;
    uint8_t validFlag = FALSE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pCpCtrl = &g_stCddCPCtrl[port];

        if (current <= CDDCP_CFG_RATE_CURRENT)
        {
            if (current >= CDDCP_CFG_RATE_MIN_CURRENT && current <= CDDCP_CFG_RATE_THRESOLD_CURRENT)
            {
                duty = current  / 60;
                validFlag = TRUE;
            }
            else if (current < CDDCP_CFG_RATE_MIN_CURRENT)
            {
                duty = 1000;
                validFlag = TRUE;
            }
            else
            {}

            if (validFlag == TRUE)
            {
                if (duty != 0 && duty != 1000)
                {
                    if (pCpCtrl->curAjustCurrent != current)
                    {
                        CDDCP_CFG_LogPrint("[枪：%d]CP PWM使能：%d, CP额定电流值变化：[%d.%d A] ---> [%d.%d A] \r\n",
                            port, pCpCtrl->cpPWMOutputEnable, pCpCtrl->curAjustCurrent / 1000, 
                            pCpCtrl->curAjustCurrent % 1000, current / 1000, current % 1000);

                        pCpCtrl->curAjustCurrent = current;
                    }

                    if (pCpCtrl->cpPWMOutputEnable == TRUE)
                    {
                        CddCP_SetPwmDuty(port, duty);
                    }
                }
                else
                {
                    CddCP_SetPwmDuty(port, duty);
                }
            }
        }
    }
}

void CddCP_SetErrNotice(uint8_t port)
{
    if (port < SYSCFG_CFG_GUN_NUM)
    {
        CddCP_SetPwmDuty(port, 0);
    }
}

CddCPVolState_Enum CddCP_GetVolState(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];
    CddCPVolState_Enum eRet = eCddCPVolState_Ground;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        eRet = pCpCtrl->eValidCpVolState;
    }

    return eRet;
}

uint16_t CddCP_GetVoltage(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];
    uint16_t cpVol = 0;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        cpVol = pCpCtrl->cpVol;
    }

    return cpVol;
}

void CddCP_SetReqStartWakeup(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->wakeupStep == CDDCP_WAKEUP_STEP0)
        {
            pCpCtrl->wakeupStep = CDDCP_WAKEUP_STEP1;
            CDDCP_CFG_LogPrint("[枪：%d]请求尝试CP唤醒!\r\n");
        }
    }
}

void CddCP_SetReqStopWakeUp(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->wakeupStep != CDDCP_WAKEUP_STEP0)
        {
            pCpCtrl->wakeupStep = CDDCP_WAKEUP_STEP0;

            if (pCpCtrl->curSetCpDuty != 0)
            {
                CddCP_SetPwmDuty(port, 1000);
            }

            CDDCP_CFG_LogPrint("[枪：%d]请求中断CP唤醒!\r\n");
        }
    }
}

void CddCP_SetReqStartDiodeExsitDetect(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->diodeDetectStep == CDDCP_DIODE_DETECT_STEP0)
        {
            pCpCtrl->diodeDetectStep = CDDCP_DIODE_DETECT_STEP1;
            pCpCtrl->diodeDetectResult = GLOBAL_OPT_STATE_PROCESS;
            memset(&pCpCtrl->diodeFilter, 0x00, sizeof(FilterProfile1_Struct));
            CDDCP_CFG_LogPrint("[枪：%d]请求执行二极管存在性检测!\r\n");
        }
    }
}

void CddCP_SetReqStopDiodeExsitDetect(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->diodeDetectStep != CDDCP_DIODE_DETECT_STEP0)
        {
            pCpCtrl->diodeDetectStep = CDDCP_DIODE_DETECT_STEP0;
            pCpCtrl->diodeDetectResult = GLOBAL_OPT_STATE_IDLE;
            CDDCP_CFG_LogPrint("[枪：%d]请求中断二极管存在性检测!\r\n");
        }
    }
}

uint8_t CddCP_GetDiodeExsitDetectResult(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];
    uint8_t ret = GLOBAL_OPT_STATE_IDLE;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        ret = pCpCtrl->diodeDetectResult;
    }

    return ret;
}

void CddCP_StartPWM(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->cpPWMOutputEnable != TRUE)
        {
            pCpCtrl->cpPWMOutputEnable = TRUE;
            CddCP_AdjustCurRateCurrent(port, pCpCtrl->curAjustCurrent);
            CDDCP_CFG_LogPrint("[枪：%d]CP开始发送PWM! PWM对应电流值：%d.%d\r\n",port, pCpCtrl->curAjustCurrent / 1000,
            pCpCtrl->curAjustCurrent % 1000);
        }
    }
}

void CddCP_StopPWM(uint8_t port)
{
    CddCPCtrl_Struct *pCpCtrl = &g_stCddCPCtrl[port];

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        if (pCpCtrl->cpPWMOutputEnable != FALSE)
        {
            pCpCtrl->cpPWMOutputEnable = FALSE;
            CddCP_AdjustCurRateCurrent(port, 0);
            CDDCP_CFG_LogPrint("[枪：%d]CP停止发送PWM!\r\n",port);
        }
    }
}




















