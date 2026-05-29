/******************************************************************************
* File Name          : Asw_IotProtoAPSend.c
* Description        : 安培协议发送帧实现
* -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
*------------    --------     -------   ----------------------------------------
*2026/05/21     V1.0.0       WDY        初版创建 - 骨架代码
*
******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoAPSend.h"
#include "Asw_IotProtoAPM.h"
#include "FrameQueue.h"
#include "SS_Tm.h"
#include "Asw_ChargeIf.h"
#include "Asw_ErrorHandle.h"
#include "Asw_Monitor.h"
#include "Cdd_NetM.h"

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
#define IOT_AP_CLOCK_SYNC_HEAD_LEN              (14U)
#define IOT_AP_CLOCK_REALTIME_HEAD_LEN          (11U)
#define IOT_AP_B53_MAX_SEND_COUNT               (10U)

typedef struct
{
    uint16_t cmd;
    uint8_t type;
    uint8_t cot;
    uint8_t recordKind;
}IotAPCmdHeadInfo_Struct;

static const IotAPCmdHeadInfo_Struct c_stIotAPCmdHeadInfoTable[] =
{
    { IOT_AP_CMD_B01_REALTIME_DATA,         134U, 6U,  0U  },
    { IOT_AP_CMD_B02_BILLMODEL_DOWN,        133U, 6U,  5U  },
    { IOT_AP_CMD_B03_BILLMODEL_RESULT,      130U, 7U,  6U  },
    { IOT_AP_CMD_B04_CHG_CTRL_DOWN,         133U, 6U,  21U },
    { IOT_AP_CMD_B05_CHG_CTRL_RESULT,       133U, 7U,  21U },
    { IOT_AP_CMD_B06_CARD_AUTH_UP,          130U, 6U,  1U  },
    { IOT_AP_CMD_B07_CARD_AUTH_DOWN,        133U, 7U,  2U  },
    { IOT_AP_CMD_B10_START_NOTIFY_UP,       130U, 6U,  14U },
    { IOT_AP_CMD_B11_START_NOTIFY_DOWN,     133U, 7U,  12U },
    { IOT_AP_CMD_B12_ONLINE_ORDER_UP,       130U, 6U,  2U  },
    { IOT_AP_CMD_B13_ONLINE_ORDER_DOWN,     130U, 7U,  2U  },
    { IOT_AP_CMD_B14_DEDUCT_CONFIRM,        133U, 6U,  3U  },
    { IOT_AP_CMD_B15_OFFLINE_ORDER_UP,      130U, 6U,  3U  },
    { IOT_AP_CMD_B16_OFFLINE_ORDER_DOWN,    130U, 7U,  3U  },
    { IOT_AP_CMD_B23_UPGRADE_START,         133U, 6U,  15U },
    { IOT_AP_CMD_B24_UPGRADE_RESULT,        130U, 7U,  14U },
    { IOT_AP_CMD_B25_RESERVE_CMD_DOWN,      133U, 6U,  54U },
    { IOT_AP_CMD_B26_RESERVE_RESULT_UP,     130U, 7U,  24U },
    { IOT_AP_CMD_B31_SIM_INFO_UP,           130U, 7U,  27U },
    { IOT_AP_CMD_B32_TERMINAL_REQ_DOWN,     133U, 6U,  57U },
    { IOT_AP_CMD_B33_POWER_CTRL_DOWN,       133U, 7U,  58U },
    { IOT_AP_CMD_B34_POWER_CTRL_UP,         130U, 6U,  28U },
    { IOT_AP_CMD_B35_BILLMODEL_POLL_DOWN,   133U, 6U,  59U },
    { IOT_AP_CMD_B36_BILLMODEL_POLL_UP,     130U, 7U,  29U },
    { IOT_AP_CMD_B37_VEHICLE_MONITOR,       130U, 6U,  30U },
    { IOT_AP_CMD_B38_ZERO_METER_VALUE,      130U, 7U,  31U },
    { IOT_AP_CMD_B39_FTP_ADDR_DOWN,         133U, 6U,  60U },
    { IOT_AP_CMD_B40_FTP_ADDR_UP,           130U, 7U,  32U },
    { IOT_AP_CMD_B45_POWER_POLL_DOWN,       133U, 6U,  63U },
    { IOT_AP_CMD_B46_POWER_POLL_UP,         130U, 7U,  35U },
    { IOT_AP_CMD_B47_TIMEBILL_DOWN,         133U, 6U,  64U },
    { IOT_AP_CMD_B48_TIMEBILL_UP,           130U, 6U,  36U },
    { IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP,    130U, 7U,  37U },
    { IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN,  133U, 6U,  65U },
    { IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN,    133U, 6U,  66U },
    { IOT_AP_CMD_B52_TIMEBILL_POLL_UP,      130U, 7U,  38U },
    { IOT_AP_CMD_B53_ONLINE_DETAIL_UP,      130U, 6U,  39U },
    { IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN,    133U, 6U,  67U },
    { IOT_AP_CMD_B55_OFFLINE_DETAIL_UP,     130U, 6U,  40U },
    { IOT_AP_CMD_B56_OFFLINE_DETAIL_DOWN,   133U, 6U,  68U },
    { IOT_AP_CMD_B57_POWER_STATUS_UP,       130U, 6U,  41U },
    { IOT_AP_CMD_SYNC_TIME_REQ,             103U, 6U,  0U  },
    { IOT_AP_CMD_SYNC_TIME_RSP,             103U, 7U,  0U  },
    { IOT_AP_CMD_LOGIN_REQ,                 0U,   0U,  0U  },
    { IOT_AP_CMD_LOGIN_RSP,                 0U,   0U,  0U  },
    { IOT_AP_CMD_UFRAME_AUTH,               0U,   0U,  0U  },
    { IOT_AP_CMD_UFRAME_ACK,                0U,   0U,  0U  },
    { IOT_AP_CMD_HEARTBEAT_REQ,             0U,   0U,  0U  },
    { IOT_AP_CMD_HEARTBEAT_RSP,             0U,   0U,  0U  },
};

const IotAPSendCtrl_Struct c_stIotAPSendctrlTable[IOT_AP_CMD_SEND_COUNT] =
{
    [0] =
    {
        .cmd = IOT_AP_CMD_LOGIN_REQ,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendLoginReq,
        .matchCmd = IOT_AP_CMD_LOGIN_RSP,
        .printFlag = TRUE,
        .cMeaning = "登录验证请求",
    },
    [1] =
    {
        .cmd = IOT_AP_CMD_UFRAME_AUTH,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendUFrameAuth,
        .matchCmd = IOT_AP_CMD_UFRAME_ACK,
        .printFlag = TRUE,
        .cMeaning = "U帧认证请求",
    },
    [2] =
    {
        .cmd = IOT_AP_CMD_HEARTBEAT_REQ,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 30 * 1000,
        .pSendFunc = IotAP_SendHeartbeatReq,
        .matchCmd = IOT_AP_CMD_HEARTBEAT_RSP,
        .printFlag = TRUE,
        .cMeaning = "心跳上报",
    },
    [3] =
    {
        .cmd = IOT_AP_CMD_SYNC_TIME_RSP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendSyncTimeRsp,
        .matchCmd = IOT_AP_CMD_SYNC_TIME_REQ,
        .printFlag = TRUE,
        .cMeaning = "时钟同步应答",
    },
    [4] =
    {
        .cmd = IOT_AP_CMD_B01_REALTIME_DATA,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = IOTAP_CFG_IDLE_REALDATA_CYCLE,
        .pSendFunc = IotAP_SendRealtimeData,
        .matchCmd = IOT_AP_CMD_NULL,
        .printFlag = TRUE,
        .cMeaning = "实时监测数据上报",
    },
    [5] =
    {
        .cmd = IOT_AP_CMD_B05_CHG_CTRL_RESULT,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendChgCtrlResult,
        .matchCmd = IOT_AP_CMD_B04_CHG_CTRL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "充电启停控制结果",
    },
    [6] =
    {
        .cmd = IOT_AP_CMD_B06_CARD_AUTH_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendCardAuthUp,
        .matchCmd = IOT_AP_CMD_B07_CARD_AUTH_DOWN,
        .printFlag = TRUE,
        .cMeaning = "刷卡鉴权上行",
    },
    [7] =
    {
        .cmd = IOT_AP_CMD_B10_START_NOTIFY_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendStartNotifyUp,
        .matchCmd = IOT_AP_CMD_B11_START_NOTIFY_DOWN,
        .printFlag = TRUE,
        .cMeaning = "远程启动通知上报",
    },
    [8] =
    {
        .cmd = IOT_AP_CMD_B24_UPGRADE_RESULT,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendUpgradeResult,
        .matchCmd = IOT_AP_CMD_B23_UPGRADE_START,
        .printFlag = TRUE,
        .cMeaning = "远程升级启动结果",
    },
    [9] =
    {
        .cmd = IOT_AP_CMD_B31_SIM_INFO_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendSimInfoUp,
        .matchCmd = IOT_AP_CMD_B32_TERMINAL_REQ_DOWN,
        .printFlag = TRUE,
        .cMeaning = "SIM卡信息上报",
    },
    [10] =
    {
        .cmd = IOT_AP_CMD_B34_POWER_CTRL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendPowerCtrlUp,
        .matchCmd = IOT_AP_CMD_B33_POWER_CTRL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "充电功率控制结果",
    },
    [11] =
    {
        .cmd = IOT_AP_CMD_B38_ZERO_METER_VALUE,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendZeroMeterValue,
        .matchCmd = IOT_AP_CMD_NULL,
        .printFlag = TRUE,
        .cMeaning = "零点示值上报",
    },
    [12] =
    {
        .cmd = IOT_AP_CMD_B40_FTP_ADDR_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendFtpAddrUp,
        .matchCmd = IOT_AP_CMD_B39_FTP_ADDR_DOWN,
        .printFlag = TRUE,
        .cMeaning = "FTP地址下发结果",
    },
    [13] =
    {
        .cmd = IOT_AP_CMD_B46_POWER_POLL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendPowerPollUp,
        .matchCmd = IOT_AP_CMD_B45_POWER_POLL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "充电功率召测上行",
    },
    [14] =
    {
        .cmd = IOT_AP_CMD_B48_TIMEBILL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillUp,
        .matchCmd = IOT_AP_CMD_B47_TIMEBILL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费模型下发结果",
    },
    [15] =
    {
        .cmd = IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillSwitchUp,
        .matchCmd = IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费切换生效上报",
    },
    [16] =
    {
        .cmd = IOT_AP_CMD_B52_TIMEBILL_POLL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillPollUp,
        .matchCmd = IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费召测上行",
    },
    [17] =
    {
        .cmd = IOT_AP_CMD_B53_ONLINE_DETAIL_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 60 * 1000,
        .pSendFunc = IotAP_SendOnlineDetailUp,
        .matchCmd = IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "在线分时交易明细上报",
    },
    [18] =
    {
        .cmd = IOT_AP_CMD_B57_POWER_STATUS_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 15 * 1000,
        .pSendFunc = IotAP_SendPowerStatusUp,
        .matchCmd = IOT_AP_CMD_NULL,
        .printFlag = TRUE,
        .cMeaning = "功率控制实时状态上报",
    },
};

/*******************************************************************************
 *    Function Source Code
 ******************************************************************************/

/**
 * @brief  根据命令字在命令头信息表中查找对应的表项
 * @param  cmd  待查找的命令字
 * @return 找到则返回对应表项的指针，未找到则返回 NULL
 */
static const IotAPCmdHeadInfo_Struct *IotAP_FindCmdHeadInfo(uint16_t cmd)
{
    uint16_t index = 0;

    for (index = 0; index < ARRAY_SIZE(c_stIotAPCmdHeadInfoTable); index++)
    {
        if (c_stIotAPCmdHeadInfoTable[index].cmd == cmd)
        {
            return &c_stIotAPCmdHeadInfoTable[index];
        }
    }

    return NULL;
}

/**
 * @brief  判断指定命令是否为固定帧命令（F帧）
 * @param  cmd  AP协议命令字
 * @retval TRUE  该命令为固定帧命令，无需组包头
 * @retval FALSE 该命令为可变帧命令，需要组包头
 */
static uint8_t IotAP_IsFixedFrameCmd(uint16_t cmd)
{
    uint8_t ret = FALSE;

    switch (cmd)
    {
        case IOT_AP_CMD_LOGIN_REQ:
        case IOT_AP_CMD_UFRAME_AUTH:
        case IOT_AP_CMD_HEARTBEAT_REQ:
        {
            ret = TRUE;
            break;
        }

        default:
        {
            break;
        }
    }

    return ret;
}

/**
 * @brief  为AP协议数据组装信息帧头，将裸数据打包为带帧头的完整帧
 * @param  cmd     命令字，用于查表获取帧头信息（type/cot/recordKind）
 * @param  pBuf    数据缓冲区，输入时包含裸数据，输出时为带帧头的完整帧
 * @param  dataLen 输入数据长度（字节）
 * @return 组包后的总长度（含帧头）；若参数无效、为登录帧/U帧或长度溢出则返回0
 * @note   登录帧和U帧（type=0且cot=0）不参与组包，直接返回0；
 *         时间同步帧采用独立的短帧头格式（IOT_AP_CLOCK_SYNC_HEAD_LEN），无recordKind字段；
 *         B1实时监测数据帧采用短帧头格式，无infAddr和recordKind字段；
 *         其余命令帧使用标准的IotAPInfoFrameHead_Struct帧头格式
 */
static uint16_t IotAP_PackHead(uint16_t cmd, uint8_t *pBuf, uint16_t dataLen)
{
    const IotAPCmdHeadInfo_Struct *pHeadInfo = IotAP_FindCmdHeadInfo(cmd);
    IotAPInfoFrameHead_Struct *pFrameHead = (IotAPInfoFrameHead_Struct *)pBuf;
    uint16_t ret = 0;                          /* 单一返回值，默认0(失败) */
    uint16_t totalLen;
    uint16_t lenField;

    if ((pBuf != NULL) && (pHeadInfo != NULL))  /* 前置校验通过才继续 */
    {
        totalLen = dataLen + sizeof(IotAPInfoFrameHead_Struct);
        lenField = totalLen - 2U;

        if ((pHeadInfo->type == 0U) && (pHeadInfo->cot == 0U))
        { 
            /* 登录帧和U帧 → ret=0 */ 
        }
        else if (lenField > 0xFFU)
        { 
            /* 长度溢出 → ret=0 */ 
        }
        else if ( cmd == IOT_AP_CMD_SYNC_TIME_RSP)
        {   /* 短帧头分支：时间同步，无recordKind */
            uint16_t headLen = IOT_AP_CLOCK_SYNC_HEAD_LEN;
            totalLen = dataLen + headLen;
            lenField = totalLen - 2U;
            if (lenField <= 0xFFU)
            {
                memmove(&pBuf[headLen], pBuf, dataLen);
                memset(pBuf, 0x00, headLen);
                pBuf[0] = IOT_AP_HEAD;
                pBuf[1] = (uint8_t)lenField;
                pBuf[6] = pHeadInfo->type;
                pBuf[7] = IOT_AP_VSQ_DEFAULT;
                pBuf[8] = pHeadInfo->cot;
                ret = totalLen;
            }
        }
        else if ( cmd == IOT_AP_CMD_B01_REALTIME_DATA )
        {   /* 短帧头分支：B1实时监测数据，无infAddr和recordKind */
            uint16_t headLen = IOT_AP_CLOCK_REALTIME_HEAD_LEN;
            totalLen = dataLen + headLen;
            lenField = totalLen - 2U;
            if (lenField <= 0xFFU)
            {
                memmove(&pBuf[headLen], pBuf, dataLen);
                memset(pBuf, 0x00, headLen);
                pBuf[0] = IOT_AP_HEAD;
                pBuf[1] = (uint8_t)lenField;
                pBuf[6] = pHeadInfo->type;
                pBuf[7] = IOT_AP_VSQ_DEFAULT;
                pBuf[8] = pHeadInfo->cot;
                ret = totalLen;
            }
        }
        else
        {   /* 标准信息帧头 */
            memmove(&pBuf[sizeof(IotAPInfoFrameHead_Struct)], pBuf, dataLen);
            memset(pFrameHead, 0x00, sizeof(IotAPInfoFrameHead_Struct));
            pFrameHead->head = IOT_AP_HEAD;
            pFrameHead->len = (uint8_t)lenField;
            pFrameHead->typeId = pHeadInfo->type;
            pFrameHead->vsq = 0U;
            pFrameHead->cot = pHeadInfo->cot;
            pFrameHead->recordKind = pHeadInfo->recordKind;
            ret = totalLen;
        }
    }

    return ret;
}

/**
 * @brief  将桩号下传BCD码以字节逆序方式拷贝到发送缓冲区
 * @param  pBuf     发送缓冲区指针
 * @param  pDataLen 指向当前数据长度的指针，拷贝完成后自动递增
 * @return 无
 * @note   将 pileDnBCD 数组从末尾至首字节逐个写入 pBuf，实现大端序转换；
 *         当 pBuf、pDataLen 或 pIotAPCtx 为 NULL 时直接返回
 */
static void IotAP_CopyPileDnReverse(uint8_t *pBuf, uint16_t *pDataLen)
{
    uint8_t index = 0;

    if ((pBuf == NULL) || (pDataLen == NULL) || (pIotAPCtx == NULL))
    {
        return;
    }

    for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
    {
        pBuf[(*pDataLen)++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1U - index];
    }
}

/**
 * @brief  获取充电口实时状态码
 * @param  port  充电口端口号
 * @return uint16_t 状态码：
 *         - 0x0001: 存在故障
 *         - 0x0002: 空闲或就绪
 *         - 0x0003: 启动中/唤醒/充电中/暂停A/暂停B
 *         - 0x0009: 停止中或已完成
 *         - 0x0000: 其他未知状态
 */
static uint16_t IotAP_GetRealtimeState(uint8_t port)
{
    uint8_t chargeState = AswChargeIf_GetChargeState(port);
    uint16_t retState = 0x0000U;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        retState = 0x0001U;
    }
    else if (chargeState == ASWCHARGEIF_WORKSTATE_IDLE || chargeState == ASWCHARGEIF_WORKSTATE_READY)
    {
        retState = 0x0002U;
    }
    else if ((chargeState == ASWCHARGEIF_WORKSTATE_STARTING) ||
             (chargeState == ASWCHARGEIF_WORKSTATE_WAKEUP) ||
             (chargeState == ASWCHARGEIF_WORKSTATE_CHARGING) ||
             (chargeState == ASWCHARGE_WORKSTATE_PAUSEA) ||
             (chargeState == ASWCHARGE_WORKSTATE_PAUSEB))
    {
        retState = 0x0003U;
    }

    else if ((chargeState == ASWCHARGE_WORKSTATE_STOPPING) ||
             (chargeState == ASWCHARGE_WORKSTATE_FINISH))
    {
        retState = 0x0009U;
    }
    else
    {
        retState = 0x0000U;
    }   

    return retState;
}

/* ====== F帧发送函数 ====== */

uint16_t IotAP_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint8_t index = 0;

    if ((pBuf == NULL) || (pIotAPCtx == NULL))
    {
        return 0;
    }

    pBuf[dataLen++] = IOT_AP_HEAD;
    pBuf[dataLen++] = 0x01;

    for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
    {
        pBuf[dataLen++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1U - index];
    }

    pBuf[dataLen++] = 0x00;
    pBuf[dataLen++] = 0x00;

    return dataLen;
}

uint16_t IotAP_SendUFrameAuth(uint8_t port, uint8_t *pBuf)
{
    if (pBuf == NULL)
    {
        return 0;
    }

    pBuf[0] = IOT_AP_HEAD;
    pBuf[1] = 0x04;
    pBuf[2] = 0x07;
    pBuf[3] = 0x00;
    pBuf[4] = 0x00;
    pBuf[5] = 0x00;

    return 6;
}

uint16_t IotAP_SendHeartbeatReq(uint8_t port, uint8_t *pBuf)
{
    if (pBuf == NULL)
    {
        return 0;
    }

    pBuf[0] = IOT_AP_HEAD;
    pBuf[1] = 0x04;
    pBuf[2] = 0x43;
    pBuf[3] = 0x00;
    pBuf[4] = 0x00;
    pBuf[5] = 0x00;

    return 6;
}

uint16_t IotAP_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
{
    CommonDateTime_Struct dateTime = { 0 };
    uint8_t cp56Time[7] = { 0 };
    uint8_t *pSyncTime = cp56Time;

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    if ((pIotAPCtx->syncTimeCp56[0] == 0U) &&
        (pIotAPCtx->syncTimeCp56[1] == 0U) &&
        (pIotAPCtx->syncTimeCp56[2] == 0U) &&
        (pIotAPCtx->syncTimeCp56[3] == 0U) &&
        (pIotAPCtx->syncTimeCp56[4] == 0U) &&
        (pIotAPCtx->syncTimeCp56[5] == 0U) &&
        (pIotAPCtx->syncTimeCp56[6] == 0U))
    {
        SSTM_GetDateTime(&dateTime);
        Common_TimestampToCp56Time2a(Common_DateTimeToTimestamp(&dateTime), cp56Time);
    }
    else
    {
        pSyncTime = pIotAPCtx->syncTimeCp56;
    }

    

    memcpy(pBuf, pSyncTime, sizeof(cp56Time));

    return sizeof(cp56Time);
}

/* ====== B帧发送函数 - 基础 ====== */

uint16_t IotAP_SendRealtimeData(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeData_Struct *pChargeData = NULL;
    uint16_t dataLen = 0;
    uint32_t tempVal = 0;
    uint8_t index = 0;
    uint8_t errBytes[8] = { 0 };
    uint8_t zeroOrder[16] = { 0 };

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    pChargeData = AswMonitor_GetChargeDataPtr(port);
    if (pChargeData == NULL)
    {
        return 0;
    }

    IotAP_CopyPileDnReverse(pBuf, &dataLen);

    pBuf[dataLen++] = port;
    pBuf[dataLen++] = (AswChargeIf_CheckGunConnected(port) == TRUE) ? 0x01U : 0x00U;
    Common_Uint16ToTwoUint8(&pBuf[dataLen], IotAP_GetRealtimeState(port));
    dataLen += 2;

    memset(&pBuf[dataLen], 0x00, 21);
    dataLen += 21; /* 序号5-16当前无来源，填0 */

    tempVal = AswChargeIf_GetOutputVoltage(port) / 10U;
    Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)tempVal);
    dataLen += 2;
    tempVal = AswChargeIf_GetOutputCurrent(port) / 100U;
    Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)tempVal);
    dataLen += 2;

    pBuf[dataLen++] = (AswChargeIf_GetRelayState(port) == ASWCHARGEIF_RELAYSTATE_ON) ? 0x01U : 0x00U;
    pBuf[dataLen++] = 0x00U; /* BMS 通信状态，当前无独立来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 是否连接电池，当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4; /* 单体电池最高/最低电压当前交流桩无来源，填0 */

    Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalEnergy); /* 总电量 */
    dataLen += 4;
    for (index = 0; index < 4U; index++)    /* 尖峰平谷电量 */
    {
        Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->rateTotalEnergy[index]);
        dataLen += 4;
    }

    memset(&pBuf[dataLen], 0x00, 2);
    dataLen += 2; /* SOC当前交流桩无来源，填0 */
    Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)(pChargeData->chargeTime / 60U));
    dataLen += 2;

    memset(&pBuf[dataLen], 0x00, 32);
    dataLen += 32; /* 车辆唯一标识/VIN当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 3);
    dataLen += 3; /* BMS相关，当前无来源，填0 */

    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_EmergencyStop) == TRUE) ? 0x01U : 0x00U;
    pBuf[dataLen++] = 0x00U; /* 直流侧开关跳闸/熔断器熔断，交流桩无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 充电机过温告警当前无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 交流输入异常当前无独立来源，填0 */
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_MeterCommErr) == TRUE) ? 0x01U : 0x00U;
    pBuf[dataLen++] = 0x00U; /* 缺相保护当前无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 反接保护当前无来源，填0 */
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_AphaseInputOverVol) == TRUE) ? 0x01U : 0x00U;
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr) == TRUE) ? 0x01U : 0x00U;
    pBuf[dataLen++] = 0x00U; /* 风扇故障当前无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 温度传感器故障当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 5);
    dataLen += 5; /* 电池组/单体故障字段当前交流桩无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 集中器与桩通信故障当前无来源，填0 */
    pBuf[dataLen++] = 0x00U; /* 充电监控单元故障当前无来源，填0 */

    Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalElecMoney / 100U);
    dataLen += 4;
    Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalServeMoney / 100U);
    dataLen += 4;
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4; /* 剩余时长当前无来源，填0 */

    if (memcmp(pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum, zeroOrder, sizeof(zeroOrder)) != 0)
    {
        memcpy(&pBuf[dataLen], pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum, 16);
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 16); /* 当前无订单流水号时填0 */
    }
    dataLen += 16;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        if (AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr) == TRUE)
        {
            errBytes[0] = 0x02U;
        }
        if (AswErrHandle_CheckErrExit(port, eErr_ShortCircleErr) == TRUE)
        {
            errBytes[2] |= (1U << 1);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr) == TRUE)
        {
            errBytes[2] |= (1U << 2);
        }
        if ((AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor) == TRUE) ||
            (AswErrHandle_CheckErrExit(port, eErr_CpGroundFault) == TRUE))
        {
            errBytes[2] |= (1U << 6);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_GunOverTempErr) == TRUE)
        {
            errBytes[3] |= (1U << 3);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_EnvOverTempErr) == TRUE)
        {
            errBytes[4] |= (1U << 1);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_GunOverTempErr) == TRUE)
        {
            errBytes[4] |= (1U << 2);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation) == TRUE)
        {
            errBytes[4] |= (1U << 4);
        }
        if (AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault) == TRUE)
        {
            errBytes[4] |= (1U << 5);
        }
    }
    memcpy(&pBuf[dataLen], errBytes, sizeof(errBytes));
    dataLen += sizeof(errBytes);

    return dataLen;
}

uint16_t IotAP_SendChgCtrlResult(uint8_t port, uint8_t *pBuf)
{
    CommonDateTime_Struct dateTime = { 0 };
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t result = 1U;
    uint8_t failReason = 0x03U;
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (pIotAPCtx != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pProtoData = &pIotAPCtx->stProtoData[port];

        if (pProtoData->remoteCtrlCmd == 0U)
        {
            result = pProtoData->remoteStopResult;
            failReason = pProtoData->remoteStopFailReason;
        }
        else
        {
            result = pProtoData->remoteStartResult;
            failReason = pProtoData->remoteStartFailReason;
        }

        IotAP_CopyPileDnReverse(pBuf, &dataLen);
        pBuf[dataLen++] = port;
        pBuf[dataLen++] = result;
        pBuf[dataLen++] = 0x00U;
        pBuf[dataLen++] = (uint8_t)(((failReason / 10U) << 4U) | (failReason % 10U));
        pBuf[dataLen++] = pProtoData->remoteCtrlCmd;

        SSTM_GetDateTime(&dateTime);
        Common_TimestampToCp56Time2a(Common_DateTimeToTimestamp(&dateTime), &pBuf[dataLen]);
        dataLen += 7U;
    }

    return dataLen;
}

/* ====== B帧发送函数 - 鉴权 ====== */

uint16_t IotAP_SendCardAuthUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装刷卡鉴权上行 B6 */
    return 0;
}

uint16_t IotAP_SendStartNotifyUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装启动通知上报 B10 */
    return 0;
}

/* ====== B帧发送函数 - 交易 ====== */

uint16_t IotAP_SendDeductConfirmRsp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装充电扣款后下行数据应答 B14 */
    return 0;
}

uint16_t IotAP_SendOnlineDetailUp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (pIotAPCtx != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        IotAP_TransformChargeRecord(&pIotAPCtx->stOrderInfo.platOrderInfo, pBuf, &dataLen);
    }

    return dataLen;
}

/* ====== B帧发送函数 - 计费 ====== */

uint16_t IotAP_SendTimeBillUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[18] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    IotAP_CopyPileDnReverse(dataBuf, &dataLen);
    dataBuf[dataLen++] = port;
    memcpy(&dataBuf[dataLen], pIotAPCtx->stProtoData[port].timeBillModelId, sizeof(pIotAPCtx->stProtoData[port].timeBillModelId));
    dataLen += sizeof(pIotAPCtx->stProtoData[port].timeBillModelId);
    dataBuf[dataLen++] = pIotAPCtx->stProtoData[port].timeBillResult;

    memcpy(pBuf, dataBuf, dataLen);

    return dataLen;
}

uint16_t IotAP_SendTimeBillSwitchUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装计费模型切换生效上行-分时服务费 B49 */
    return 0;
}

uint16_t IotAP_SendTimeBillPollUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装计费模型召测上行数据-分时服务费 B52 */
    return 0;
}

/* ====== B帧发送函数 - 功率控制 ====== */

uint16_t IotAP_SendPowerCtrlUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[17] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    /* 桩号BCD逆序 */
    IotAP_CopyPileDnReverse(dataBuf, &dataLen);

    /* 充电接口标识(枪号) */
    dataBuf[dataLen++] = port;

    /* 时间戳原样回传 */
    memcpy(&dataBuf[dataLen],
           pIotAPCtx->stProtoData[port].powerCtrlTimepower,
           sizeof(pIotAPCtx->stProtoData[port].powerCtrlTimepower));
    dataLen += 7;

    /* 成功标志: 0成功 1失败 */
    dataBuf[dataLen++] = pIotAPCtx->stProtoData[port].powerCtrlResult;

    memcpy(pBuf, dataBuf, dataLen);
    return dataLen;
}

uint16_t IotAP_SendPowerPollUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装充电功率召测上行 B46 */
    return 0;
}

uint16_t IotAP_SendPowerStatusUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装充电功率控制过程中的扩展实时状态 B57 */
    return 0;
}

/* ====== B帧发送函数 - 其他扩展 ====== */

uint16_t IotAP_SendSimInfoUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[39] = { 0 };
    uint16_t dataLen = 0;
    uint8_t iccid[20] = { 0 };

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    CddNetM_GetIccid(iccid);
    IotAP_CopyPileDnReverse(dataBuf, &dataLen);
    memcpy(&dataBuf[dataLen], iccid, sizeof(iccid));
    dataLen += sizeof(iccid);
    memset(&dataBuf[dataLen], 0x00, 11);
    dataLen += 11; /* 手机号当前工程无来源，填0 */

    memcpy(pBuf, dataBuf, dataLen);

    return dataLen;
}

uint16_t IotAP_SendZeroMeterValue(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装零点示值上报 B38 */
    return 0;
}

uint16_t IotAP_SendFtpAddrUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[9] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf == NULL) || (pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return 0;
    }

    IotAP_CopyPileDnReverse(dataBuf, &dataLen);
    dataBuf[dataLen++] = 0x00U;

    memcpy(pBuf, dataBuf, dataLen);

    return dataLen;
}

uint16_t IotAP_SendUpgradeResult(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装远程升级启动命令接收结果 B24 */
    return 0;
}

/* ====== 白名单 - 暂不实现(交流桩) ====== */









/*
 * 上行发送控制处理
 * 参照GN的IotGN_UpCtrlSendDeal模式：
 *   1. queueBusyFlag检测（500ms超时自清）
 *   2. while循环遍历c_stIotAPSendctrlTable (sendIndex/sendPort轮转)
 *   3. 检查发送使能 + 周期/立即标志
 *   4. 调用pSendFunc组帧
 *   5. FrameQueue_PushTx入队
 *   6. 更新控制标志(发送标记/周期计时/使能清除/接收超时启动)
 *
 * AP与GN的关键差异：AP组帧函数已包含0x68帧头，无需额外PackHead封装
 */
void IotAP_UpCtrlSendDeal(void)
{
    const IotAPSendCtrl_Struct *pCmdSendCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint16_t reqSeq = 0;
    uint16_t dataLen = 0;
    uint8_t txBuf[IOT_AP_TXRX_BUFFER_SIZE] = { 0 };

    if (pIotAPCtx->queueBusyFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pIotAPCtx->waitQueueIdleTick, 500) == TRUE)
        {
            pIotAPCtx->queueBusyFlag = FALSE;
        }
    }
    else
    {
        while (1)
        {
            if (pIotAPCtx->sendIndex < ARRAY_SIZE(c_stIotAPSendctrlTable))
            {
                index = pIotAPCtx->sendIndex;
                port = pIotAPCtx->sendPort;

                pCmdSendCtrl = &c_stIotAPSendctrlTable[index];

                if ((Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) == TRUE) &&
                    (TRUE == Common_GetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd) ||
                     Common_JudgeTimeoutMs(Common_GetSendTick(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd), pCmdSendCtrl->sendCycle) == TRUE))
                {
                    /* 区分请求帧/响应帧获取序列号 */
                    if (pCmdSendCtrl->cmdType == IOT_AP_CMDTYPE_REQUSET)
                    {
                        reqSeq = pIotAPCtx->reqSeq;

                        if (pCmdSendCtrl->matchCmd != IOT_AP_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotAPCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, reqSeq);
                        }

                        pIotAPCtx->reqSeq++;
                    }
                    else
                    {
                        if (pCmdSendCtrl->matchCmd != IOT_AP_CMD_NULL)
                        {
                            reqSeq = Common_GetRecvSeq(pIotAPCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd);
                        }
                    }

                    /* 调用组帧函数：固定短帧返回完整帧长度，信息帧返回数据域长度 */
                    if (pCmdSendCtrl->pSendFunc != NULL)
                    {
                        dataLen = pCmdSendCtrl->pSendFunc(port, txBuf);
                    }

                    /* 非固定帧命令添加头部信息 */
                    if ((dataLen > 0) && (IotAP_IsFixedFrameCmd(pCmdSendCtrl->cmd) == FALSE))
                    {
                        dataLen = IotAP_PackHead(pCmdSendCtrl->cmd, txBuf, dataLen);
                    }
                    
                    /* 组帧成功则入队发送 */
                    if (dataLen > 0)
                    {
                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotAPCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen))
                        {
                            /* 入队失败，回退请求帧序号 */
                            if (pCmdSendCtrl->cmdType == IOT_AP_CMDTYPE_REQUSET)
                            {
                                pIotAPCtx->reqSeq--;
                            }

                            break;
                        }

                        /* 打印日志 */
                        if (pCmdSendCtrl->printFlag)
                        {
                            IOTAP_CFG_DebugPrint("AP,[枪：%d]发送[cmd: 0x%02X, %s][%d]: \r\n", port, (uint8_t)pCmdSendCtrl->cmd, pCmdSendCtrl->cMeaning, dataLen);
                            DSLogM_HexOutput(txBuf, dataLen);
                        }

                        /* 更新控制标志 */
                        Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, TRUE);
                        Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        Common_SetSendTick(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, Common_GetSystick());

                        if ((pCmdSendCtrl->cmd == IOT_AP_CMD_B40_FTP_ADDR_UP) ||
                            (pCmdSendCtrl->cmd == IOT_AP_CMD_B48_TIMEBILL_UP))
                        {
                            if ((Common_GetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B40_FTP_ADDR_UP) == TRUE) &&
                                (Common_GetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B48_TIMEBILL_UP) == TRUE))
                            {
                                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_UFRAME_AUTH, TRUE);
                            }
                        }

                        /* 建连流程：F8发送成功后再使能B31，B31发送成功后再使能F5 */
                        if (pCmdSendCtrl->cmd == IOT_AP_CMD_SYNC_TIME_RSP)
                        {
                            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B31_SIM_INFO_UP, TRUE);
                        }
                        else if (pCmdSendCtrl->cmd == IOT_AP_CMD_B31_SIM_INFO_UP)
                        {
                            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_HEARTBEAT_REQ, TRUE);
                            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_HEARTBEAT_REQ, TRUE);
                        }

                        /* 一次性帧(无发送周期)清除使能，避免重复发送 */
                        if (pCmdSendCtrl->sendCycle == 0)
                        {
                            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, pCmdSendCtrl->cmd, FALSE);
                        }

                        /* 请求帧+有匹配应答: 启动接收超时计时 */
                        if (pCmdSendCtrl->cmdType == IOT_AP_CMDTYPE_REQUSET)
                        {
                            if (pCmdSendCtrl->matchCmd != IOT_AP_CMD_NULL)
                            {
                                Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, TRUE);
                                Common_SetRecvTick(pIotAPCtx->pFuncRecvCtrl, port, pCmdSendCtrl->matchCmd, Common_GetSystick());
                            }
                        }

                        if (pCmdSendCtrl->cmd == IOT_AP_CMD_B53_ONLINE_DETAIL_UP)
                        {
                            Common_SetRptCount(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN);

                            if (Common_GetRptCount(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN) >= IOT_AP_B53_MAX_SEND_COUNT)
                            {
                                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                                Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                                Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                                Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN, FALSE);
                                IOTAP_CFG_InfoPrint("AP,[枪:%d]B53在线分时交易记录上报10次未确认，暂停本次重发\r\n", port);
                            }
                        }
                    }
                }
            }

            /* 轮转到下一条发送表项 */
            pIotAPCtx->sendIndex++;

            if (pIotAPCtx->sendIndex >= ARRAY_SIZE(c_stIotAPSendctrlTable))
            {
                pIotAPCtx->sendIndex = 0;
                pIotAPCtx->sendPort++;

                if (pIotAPCtx->sendPort >= SYSCFG_CFG_GUN_NUM)
                {
                    pIotAPCtx->sendPort = 0;
                    break;
                }
            }

            if (pIotAPCtx->queueBusyFlag == TRUE)
            {
                break;
            }
        }
    }
}
