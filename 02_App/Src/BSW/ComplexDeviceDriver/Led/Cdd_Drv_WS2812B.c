/******************************************************************************
* File Name          : Cdd_Drv_WS2812B.c
* Description        : WS2812B的数据格式严格为绿→红→蓝，与常见的RGB顺序不同
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
#include "Cdd_Drv_WS2812B.h"
#include "Cdd_Drv_WS2812BConfig.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDDDRV_WS2812B_CTRL_STEP_0                   0X00
#define CDDDRV_WS2812B_CTRL_STEP_1                   0X01
#define CDDDRV_WS2812B_CTRL_STEP_2                   0X02
#define CDDDRV_WS2812B_CTRL_STEP_END                 0xFF
/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum 
{
    eCddDrvWS2812BbBightness_Off,
    eCddDrvWS2812BbBightness_On,
    eCddDrvWS2812BbBightness_Up,
    eCddDrvWS2812BbBightness_Down,
} eCddDrvWS2812BbBightness_Enum;

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t nextDispType;
    const CddDrvWS2812BConfig_Struct *pNextDispTypeConfig;
    uint8_t currentDispType;
    const CddDrvWS2812BConfig_Struct *pCurrentDispTypeConfig;
    uint8_t dispTimes;
    uint8_t brightDirection;
    uint8_t ctrlStep;
    uint32_t onStartTick;
    uint32_t offStartTick;
    uint32_t tempLedRgbBuf[CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT];
}CddDrvWS2812BCtrl_struct;

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static uint32_t g_CddDrvWS2812BLedRgbBuf[CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE] = {0};
static uint32_t g_LastCddDrvWS2812BLedRgbBuf[CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE] = {0};

static uint16_t g_CddDrvWS2812BLedDutyBuf[CDDDRV_WS2812B_CFG_DUTY_BUFF_SIZE] = {0};

static CddDrvWS2812BCtrl_struct g_stCddDrvWS2812BCtrl[eCddDrvWS2812BChannel_Count] = {0};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t CddDrvWS2812B_Controlbrightness(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl, 
    eCddDrvWS2812BbBightness_Enum eBrightnessCtrl, uint8_t variation)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;
    uint8_t index = 0;
    uint8_t pos = 0;
    uint8_t fullFlag = FALSE;
    uint8_t temp = 0;

    union
    {
        uint32_t rgbvalue;
        uint8_t rgb[3 + 1];
    }tempRgb;

    switch (eBrightnessCtrl)
    {
    case eCddDrvWS2812BbBightness_Off:
    {
        for (index = 0; index < CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT; index++)
        {
            pos = c_CddDrvWS2812BLedLocation[eCh][index];

            /* 位置是配置的，可能会配置错误，确保位置在有效范围内 */
            if (pos < CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE)
            {
                g_CddDrvWS2812BLedRgbBuf[pos] = CDDDRV_WS2812B_NONE;
            }
        }

        fullFlag = TRUE;

        break;
    }
    case eCddDrvWS2812BbBightness_On:
    {
        for (index = 0; index < CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT; index++)
        {
            pos = c_CddDrvWS2812BLedLocation[eCh][index];

            /* 位置是配置的，可能会配置错误，确保位置在有效范围内 */
            if (pos < CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE)
            {
                g_CddDrvWS2812BLedRgbBuf[pos] = c_stCddDrvWS2812BConfigTable[pCtrl->currentDispType].RGBArray[index];
            }
        }

        fullFlag = TRUE;

        break;
    }
    case eCddDrvWS2812BbBightness_Up:
    {
        for (index = 0; index < CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT; index++)
        {
            pos = c_CddDrvWS2812BLedLocation[eCh][index];

            /* 位置是配置的，可能会配置错误，确保位置在有效范围内 */
            if (pos < CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE)
            {
                tempRgb.rgbvalue = g_CddDrvWS2812BLedRgbBuf[pos];

                for (temp = 0; temp < 4; temp++)
                {
                    if (tempRgb.rgb[temp] > 0)
                    {
                        if ((0xFF -  tempRgb.rgb[temp]) <= variation)
                        {
                            tempRgb.rgb[temp] = 0xFF;
                            fullFlag = TRUE;
                        }
                        else
                        {
                            tempRgb.rgb[temp] = tempRgb.rgb[temp] + variation;
                        }
                    }
                }

                g_CddDrvWS2812BLedRgbBuf[pos] = tempRgb.rgbvalue;
            }
        }

        break;
    }
    
    case eCddDrvWS2812BbBightness_Down:
    {
        for (index = 0; index < CDDDRV_WS2812B_CFG_SINGLE_CHANNEL_LED_COUNT; index++)
        {
            pos = c_CddDrvWS2812BLedLocation[eCh][index];

            /* 位置是配置的，可能会配置错误，确保位置在有效范围内 */
            if (pos < CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE)
            {
                tempRgb.rgbvalue = g_CddDrvWS2812BLedRgbBuf[pos];

                for (temp = 0; temp < 4; temp++)
                {
                    if (tempRgb.rgb[temp] > 0)
                    {
                        if (tempRgb.rgb[temp] <= variation)
                        {
                            tempRgb.rgb[temp] = 0x01;
                            fullFlag = TRUE;
                        }
                        else
                        {
                            tempRgb.rgb[temp] = tempRgb.rgb[temp] - variation;
                        }
                    }
                }
            }

            g_CddDrvWS2812BLedRgbBuf[pos] = tempRgb.rgbvalue;
        }

        break;
    }
    default:
    {
        break;
    }
    }

    return fullFlag;
}

static uint8_t CddDrvWS2812B_CheckUpdateCondition(CddDrvWS2812BCtrl_struct *pCtrl, const CddDrvWS2812BConfig_Struct * pRecvLedDispTypecfg)
{
    uint8_t isCanOverride = FALSE;

    if (pRecvLedDispTypecfg->maxDispTimes != 0xFF)
    {
        isCanOverride = TRUE;
    }
    else
    {
        if (pCtrl->pNextDispTypeConfig == NULL)
        {
            isCanOverride = TRUE;
        }
        else
        {
            if (pCtrl->pNextDispTypeConfig->maxDispTimes == 0xFF)
            {
                isCanOverride = TRUE;
            }
            else
            {
                if (pCtrl->pNextDispTypeConfig->maxDispTimes <= pCtrl->dispTimes)
                {
                    isCanOverride = TRUE;
                }
            }
        }
    }

    return isCanOverride;
}

static void CddDrvWS2812B_ReloadLedDispCfg(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;

    pCtrl->pCurrentDispTypeConfig = pCtrl->pNextDispTypeConfig;
    pCtrl->currentDispType = pCtrl->nextDispType;
    pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_0;
    pCtrl->dispTimes = 0;
}

static void CddDrvWS2812B_LedDispReloadManage(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;
    const CddDrvWS2812BConfig_Struct *pCurrentDispCfg = pCtrl->pCurrentDispTypeConfig;

    if (pCtrl->nextDispType != pCtrl->currentDispType)
    {
        if (pCurrentDispCfg == NULL)
        {
            CddDrvWS2812B_ReloadLedDispCfg(eCh, pCtrl);
        }
        else
        {
            if (pCurrentDispCfg->maxDispTimes == 0xFF)
            {
                CddDrvWS2812B_ReloadLedDispCfg(eCh, pCtrl);
            }
            else if (pCtrl->dispTimes >= pCurrentDispCfg->maxDispTimes)
            {
                CddDrvWS2812B_ReloadLedDispCfg(eCh, pCtrl);
            }
        }
    }
}

static void CddDrvWS2812B_DispModeBlinkCtrl(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl, uint32_t OnTimeout, uint32_t OffTimeout)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;

    switch (pCtrl->ctrlStep)
    {
    case CDDDRV_WS2812B_CTRL_STEP_0:
    {
        CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_On, 0);
        pCtrl->onStartTick = Common_GetSystick();
        pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_1;
        break;
    }
    case CDDDRV_WS2812B_CTRL_STEP_1:
    {
        if (Common_JudgeTimeoutMs(pCtrl->onStartTick, OnTimeout))
        {
            CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_Off, 0);
            pCtrl->offStartTick = Common_GetSystick();
            pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_2;
        }

        break;
    }
    case CDDDRV_WS2812B_CTRL_STEP_2:
    {
        if (Common_JudgeTimeoutMs(pCtrl->offStartTick, OffTimeout))
        {
            CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_On, 0);
            pCtrl->onStartTick = Common_GetSystick();

            pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_1;

            if (pCtrl->pCurrentDispTypeConfig->maxDispTimes != 0xFF)
            {
                if (pCtrl->dispTimes < pCtrl->pCurrentDispTypeConfig->maxDispTimes)
                {
                    pCtrl->dispTimes++;

                    if (pCtrl->dispTimes == pCtrl->pCurrentDispTypeConfig->maxDispTimes)
                    {
                        pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_END;
                    }
                }
            }
        }

        break;
    }
    case CDDDRV_WS2812B_CTRL_STEP_END:
    {
        break;
    }
    default:
    {
        break;
    }
    }
}

static void CddDrvWS2812B_DispModeBreath(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;

    switch (pCtrl->ctrlStep)
    {
    case CDDDRV_WS2812B_CTRL_STEP_0:
    {
        CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_On, 0);
        pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_1;
        break;
    }
    case CDDDRV_WS2812B_CTRL_STEP_1:
    {
        if (TRUE == CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_Down, CDDDRV_WS2812B_CFG_BREATH_FOOTSTEP))
        {
            pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_2;
        }

        break;
    }
    case CDDDRV_WS2812B_CTRL_STEP_2:
    {
        if (TRUE == CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_Up, CDDDRV_WS2812B_CFG_BREATH_FOOTSTEP))
        {
            pCtrl->ctrlStep = CDDDRV_WS2812B_CTRL_STEP_1;

            if (pCtrl->pCurrentDispTypeConfig->maxDispTimes != 0xFF)
            {
                if (pCtrl->dispTimes < pCtrl->pCurrentDispTypeConfig->maxDispTimes)
                {
                    pCtrl->dispTimes++;
                }
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


static void CddDrvWS2812B_LedDispManage(uint8_t ch, CddDrvWS2812BCtrl_struct *pCtrl)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;

    if (pCtrl->pCurrentDispTypeConfig != NULL)
    {
        switch (pCtrl->pCurrentDispTypeConfig->eDispMode)
        {
            case eCddDrvWS2812BDispMode_Off:
            {
                CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_Off, 0);
                break;
            }
            case eCddDrvWS2812BDispMode_SlowBlink:
            {
                CddDrvWS2812B_DispModeBlinkCtrl(eCh, pCtrl, CDDDRV_WS2812B_CFG_SLOWBLINK_ON_TIMEOUT, CDDDRV_WS2812B_CFG_SLOWBLINK_OFF_TIMEOUT);
                break;
            } 
            case eCddDrvWS2812BDispMode_Blink:
            {
                CddDrvWS2812B_DispModeBlinkCtrl(eCh, pCtrl, CDDDRV_WS2812B_CFG_BLINK_ON_TIMEOUT, CDDDRV_WS2812B_CFG_BLINK_OFF_TIMEOUT);
                break;
            } 
            case eCddDrvWS2812BDispMode_FastBlink:
            {
                CddDrvWS2812B_DispModeBlinkCtrl(eCh, pCtrl, CDDDRV_WS2812B_CFG_FASTBLINK_ON_TIMEOUT, CDDDRV_WS2812B_CFG_FASTBLINK_OFF_TIMEOUT);
                break;
            } 
            case eCddDrvWS2812BDispMode_Breath:
            {
                CddDrvWS2812B_DispModeBreath(eCh, pCtrl);
                break;
            } 
            case eCddDrvWS2812BDispMode_On:
            {
                CddDrvWS2812B_Controlbrightness(eCh, pCtrl, eCddDrvWS2812BbBightness_On, 0);
                break;
            } 
            default:
            {
                break;
            }
        }
    }
}

static void CddDrvWS2812B_Show(void)
{ 
    uint8_t bit = 0;
    uint8_t word = 0;
    uint16_t dutyPos = 0;

    for (word = 0, dutyPos = 0; (word < CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE); word++)
    {
        for (bit = 24; (bit > 0) && (dutyPos < CDDDRV_WS2812B_CFG_DUTY_BUFF_SIZE); bit--, dutyPos++)
        {
            if (g_CddDrvWS2812BLedRgbBuf[word] & (1 << (bit - 1)))
            {
                g_CddDrvWS2812BLedDutyBuf[dutyPos] = CDDDRV_WS2812B_CFG_HIGH_BIT_DUTY;
            }
            else
            {
                g_CddDrvWS2812BLedDutyBuf[dutyPos] = CDDDRV_WS2812B_CFG_LOW_BIT_DUTY;
            }
        }
    }

    CddDrvWS2812BCfg_ShowLed(g_CddDrvWS2812BLedDutyBuf, CDDDRV_WS2812B_CFG_DUTY_BUFF_SIZE);
}

void CddDrvWS2812B_UpdateLedDispType(uint8_t ch, uint8_t ledDispType)
{
    CddDrvWS2812BChannel_Enum eCh = (CddDrvWS2812BChannel_Enum)ch;

    PARA_ASSERT(eCh < eCddDrvWS2812BChannel_Count);
    PARA_ASSERT(ledDispType < CDD_LEDM_DEVICE0_DISP_TYPE_COUNT);
    CddDrvWS2812BCtrl_struct *pCtrl = &g_stCddDrvWS2812BCtrl[eCh];
    const CddDrvWS2812BConfig_Struct * pRecvLedDispTypecfg = &c_stCddDrvWS2812BConfigTable[ledDispType];

    if (TRUE == CddDrvWS2812B_CheckUpdateCondition(pCtrl, pRecvLedDispTypecfg))
    {
        pCtrl->pNextDispTypeConfig = pRecvLedDispTypecfg;
        pCtrl->nextDispType = ledDispType;
    }
}

void CddDrvWS2812B_MainFunction(void)
{
    CddDrvWS2812BChannel_Enum eCh;
    static uint32_t rgbFreshTick = 0;

    for (eCh = eCddDrvWS2812BChannel_0; eCh < eCddDrvWS2812BChannel_Count; eCh++)
    {
        CddDrvWS2812B_LedDispReloadManage(eCh, &g_stCddDrvWS2812BCtrl[eCh]);
        CddDrvWS2812B_LedDispManage(eCh, &g_stCddDrvWS2812BCtrl[eCh]);
    }

    if (0 != memcmp(g_LastCddDrvWS2812BLedRgbBuf, g_CddDrvWS2812BLedRgbBuf, CDDDRV_WS2812B_CFG_RGB_BUFF_SIZE))
    {
        CddDrvWS2812B_Show();
        memcpy(g_LastCddDrvWS2812BLedRgbBuf, g_CddDrvWS2812BLedRgbBuf, sizeof(g_CddDrvWS2812BLedRgbBuf));
        rgbFreshTick = Common_GetSystick();
    }

    if (Common_JudgeTimeoutMs(rgbFreshTick, CDDDRV_WS2812B_CFG_REFRESH_PERIOD))
    {
        CddDrvWS2812B_Show();
    }
}



















