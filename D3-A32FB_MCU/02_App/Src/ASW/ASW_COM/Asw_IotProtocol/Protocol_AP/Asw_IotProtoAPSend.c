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
*2026/05/21     V1.0.0       WDY        初版创建
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
*    Static Function Prototypes
*******************************************************************************/
static uint16_t IotAP_SendLoginReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendUFrameAuth(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendHeartbeatReq(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendRealtimeData(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendChgCtrlResult(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendCardAuthUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendStartNotifyUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendDeductConfirmRsp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendOnlineDetailUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendTimeBillUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendTimeBillSwitchUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendTimeBillPollUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendPowerCtrlUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendPowerPollUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendPowerStatusUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendSimInfoUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendFtpAddrUp(uint8_t port, uint8_t *pBuf);
static uint16_t IotAP_SendUpgradeResult(uint8_t port, uint8_t *pBuf);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
#define IOT_AP_CLOCK_SYNC_HEAD_LEN              (14)
#define IOT_AP_CLOCK_REALTIME_HEAD_LEN          (11)
#define IOT_AP_CARD_NO_LEN                      (8)
#define IOT_AP_CARD_PASSWORD_LEN                (16)
#define IOT_AP_CARD_BALANCE_LEN                 (4)
#define IOT_AP_CARD_VIN_LEN                     (32)
#define IOT_AP_START_WAY_FULL                   (4)
#define IOT_AP_B52_PORT_LEN                     (1)
#define IOT_AP_B52_TIME_LEN                     (7)
#define IOT_AP_B52_BILL_MODE_ID_LEN             (8)
#define IOT_AP_B52_SWITCH_TIME_LEN              (7)
#define IOT_AP_B52_INVALID_TIME_LEN             (7)
#define IOT_AP_B52_PERIOD_COUNT_LEN             (1)
#define IOT_AP_B52_PERIOD_LEN                   (14)
#define IOT_AP_B52_RESULT_LEN                   (1)
#define IOT_AP_B52_FAIL_MODEL_LEN               (IOT_AP_B52_BILL_MODE_ID_LEN + \
                                                IOT_AP_B52_SWITCH_TIME_LEN + \
                                                IOT_AP_B52_INVALID_TIME_LEN + \
                                                IOT_AP_B52_PERIOD_COUNT_LEN)
#define IOT_AP_B52_MAX_DATA_LEN                 (IOT_AP_PILE_DN_LEN + \
                                                IOT_AP_B52_PORT_LEN + \
                                                IOT_AP_B52_TIME_LEN + \
                                                IOT_AP_B52_FAIL_MODEL_LEN + \
                                                (IOT_AP_B52_PERIOD_LEN * MSNVM_AP_BILLMODE_PERIOD_COUNT) + \
                                                IOT_AP_B52_RESULT_LEN)

typedef struct
{
    uint16_t cmd;
    uint8_t type;
    uint8_t cot;
    uint8_t recordKind;
}IotAPCmdHeadInfo_Struct;

static const IotAPCmdHeadInfo_Struct c_stIotAPCmdHeadInfoTable[] =
{
    { IOT_AP_CMD_B01_REALTIME_DATA,         134, 6,  0  },
    { IOT_AP_CMD_B02_BILLMODEL_DOWN,        133, 6,  5  },
    { IOT_AP_CMD_B03_BILLMODEL_RESULT,      130, 7,  6  },
    { IOT_AP_CMD_B04_CHG_CTRL_DOWN,         133, 6,  21 },
    { IOT_AP_CMD_B05_CHG_CTRL_RESULT,       133, 7,  21 },
    { IOT_AP_CMD_B06_CARD_AUTH_UP,          130, 6,  1  },
    { IOT_AP_CMD_B07_CARD_AUTH_DOWN,        133, 7,  2  },
    { IOT_AP_CMD_B10_START_NOTIFY_UP,       130, 6,  14 },
    { IOT_AP_CMD_B11_START_NOTIFY_DOWN,     133, 7,  12 },
    { IOT_AP_CMD_B12_ONLINE_ORDER_UP,       130, 6,  2  },
    { IOT_AP_CMD_B13_ONLINE_ORDER_DOWN,     130, 7,  2  },
    { IOT_AP_CMD_B14_DEDUCT_CONFIRM,        133, 6,  3  },
    { IOT_AP_CMD_B15_OFFLINE_ORDER_UP,      130, 6,  3  },
    { IOT_AP_CMD_B16_OFFLINE_ORDER_DOWN,    130, 7,  3  },
    { IOT_AP_CMD_B23_UPGRADE_START,         133, 6,  15 },
    { IOT_AP_CMD_B24_UPGRADE_RESULT,        130, 7,  14 },
    { IOT_AP_CMD_B25_RESERVE_CMD_DOWN,      133, 6,  54 },
    { IOT_AP_CMD_B26_RESERVE_RESULT_UP,     130, 7,  24 },
    { IOT_AP_CMD_B31_SIM_INFO_UP,           130, 7,  27 },
    { IOT_AP_CMD_B32_TERMINAL_REQ_DOWN,     133, 6,  57 },
    { IOT_AP_CMD_B33_POWER_CTRL_DOWN,       133, 7,  58 },
    { IOT_AP_CMD_B34_POWER_CTRL_UP,         130, 6,  28 },
    { IOT_AP_CMD_B35_BILLMODEL_POLL_DOWN,   133, 6,  59 },
    { IOT_AP_CMD_B36_BILLMODEL_POLL_UP,     130, 7,  29 },
    { IOT_AP_CMD_B37_VEHICLE_MONITOR,       130, 6,  30 },
    { IOT_AP_CMD_B39_FTP_ADDR_DOWN,         133, 6,  60 },
    { IOT_AP_CMD_B40_FTP_ADDR_UP,           130, 7,  32 },
    { IOT_AP_CMD_B45_POWER_POLL_DOWN,       133, 6,  63 },
    { IOT_AP_CMD_B46_POWER_POLL_UP,         130, 7,  35 },
    { IOT_AP_CMD_B47_TIMEBILL_DOWN,         133, 6,  64 },
    { IOT_AP_CMD_B48_TIMEBILL_UP,           130, 6,  36 },
    { IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP,    130, 7,  37 },
    { IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN,  133, 6,  65 },
    { IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN,    133, 6,  66 },
    { IOT_AP_CMD_B52_TIMEBILL_POLL_UP,      130, 7,  38 },
    { IOT_AP_CMD_B53_ONLINE_DETAIL_UP,      130, 6,  39 },
    { IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN,    133, 6,  67 },
    { IOT_AP_CMD_B55_OFFLINE_DETAIL_UP,     130, 6,  40 },
    { IOT_AP_CMD_B56_OFFLINE_DETAIL_DOWN,   133, 6,  68 },
    { IOT_AP_CMD_B57_POWER_STATUS_UP,       130, 6,  41 },
    { IOT_AP_CMD_SYNC_TIME_REQ,             103, 6,  0  },
    { IOT_AP_CMD_SYNC_TIME_RSP,             103, 7,  0  },
    { IOT_AP_CMD_LOGIN_REQ,                 0,   0,  0  },
    { IOT_AP_CMD_LOGIN_RSP,                 0,   0,  0  },
    { IOT_AP_CMD_UFRAME_AUTH,               0,   0,  0  },
    { IOT_AP_CMD_UFRAME_ACK,                0,   0,  0  },
    { IOT_AP_CMD_HEARTBEAT_REQ,             0,   0,  0  },
    { IOT_AP_CMD_HEARTBEAT_RSP,             0,   0,  0  },
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
        .cmd = IOT_AP_CMD_B40_FTP_ADDR_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendFtpAddrUp,
        .matchCmd = IOT_AP_CMD_B39_FTP_ADDR_DOWN,
        .printFlag = TRUE,
        .cMeaning = "FTP地址下发结果",
    },
    [12] =
    {
        .cmd = IOT_AP_CMD_B46_POWER_POLL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendPowerPollUp,
        .matchCmd = IOT_AP_CMD_B45_POWER_POLL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "充电功率召测上行",
    },
    [13] =
    {
        .cmd = IOT_AP_CMD_B48_TIMEBILL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillUp,
        .matchCmd = IOT_AP_CMD_B47_TIMEBILL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费模型下发结果",
    },
    [14] =
    {
        .cmd = IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillSwitchUp,
        .matchCmd = IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费切换生效上报",
    },
    [15] =
    {
        .cmd = IOT_AP_CMD_B52_TIMEBILL_POLL_UP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .sendCycle = 0,
        .pSendFunc = IotAP_SendTimeBillPollUp,
        .matchCmd = IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "分时服务费召测上行",
    },
    [16] =
    {
        .cmd = IOT_AP_CMD_B53_ONLINE_DETAIL_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 60 * 1000,
        .pSendFunc = IotAP_SendOnlineDetailUp,
        .matchCmd = IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN,
        .printFlag = TRUE,
        .cMeaning = "在线分时交易明细上报",
    },
    [17] =
    {
        .cmd = IOT_AP_CMD_B57_POWER_STATUS_UP,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .sendCycle = 0,
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
        lenField = totalLen - 2;

        if ((pHeadInfo->type == 0) && (pHeadInfo->cot == 0))
        { 
            /* 登录帧和U帧 → ret=0 */ 
        }
        else if (lenField > 0xFF)
        { 
            /* 长度溢出 → ret=0 */ 
        }
        else if ( cmd == IOT_AP_CMD_SYNC_TIME_RSP)
        {   /* 短帧头分支：时间同步，无recordKind */
            uint16_t headLen = IOT_AP_CLOCK_SYNC_HEAD_LEN;
            totalLen = dataLen + headLen;
            lenField = totalLen - 2;
            if (lenField <= 0xFF)
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
            lenField = totalLen - 2;
            if (lenField <= 0xFF)
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
            pFrameHead->vsq = 0;
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

    if ((pBuf != NULL) && (pDataLen != NULL))
    {
        for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
        {
            pBuf[(*pDataLen)++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1 - index];
        }
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
    uint16_t retState = 0x0000;

    if (AswErrHandle_IsExsistError(port) == TRUE)
    {
        retState = 0x0001;                 /* 故障优先 */
    }
    else
    {
        switch (chargeState)
        {
            case ASWCHARGEIF_WORKSTATE_IDLE:
            case ASWCHARGEIF_WORKSTATE_READY:
                retState = 0x0002;         /* 空闲/准备 */
                break;

            case ASWCHARGEIF_WORKSTATE_STARTING:
            case ASWCHARGEIF_WORKSTATE_WAKEUP:
            case ASWCHARGEIF_WORKSTATE_CHARGING:
            case ASWCHARGEIF_WORKSTATE_PAUSEA:
            case ASWCHARGEIF_WORKSTATE_PAUSEB:
                retState = 0x0003;         /* 充电中 */
                break;

            case ASWCHARGEIF_WORKSTATE_STOPPING:
            case ASWCHARGEIF_WORKSTATE_FINISH:
                retState = 0x0009;         /* 停止/完成 */
                break;

            default:
                retState = 0x0000;
                break;
        }
    }

    return retState;
}

/* ====== F帧发送函数 ====== */

static uint16_t IotAP_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;
    uint8_t index = 0;

    if (pBuf != NULL)
    {
        pBuf[dataLen++] = IOT_AP_HEAD;
        pBuf[dataLen++] = 0x01;

        for (index = 0; index < sizeof(pIotAPCtx->pileDnBCD); index++)
        {
            pBuf[dataLen++] = pIotAPCtx->pileDnBCD[sizeof(pIotAPCtx->pileDnBCD) - 1 - index];
        }

        pBuf[dataLen++] = 0x00;
        pBuf[dataLen++] = 0x00;
    }

    return dataLen;
}

static uint16_t IotAP_SendUFrameAuth(uint8_t port, uint8_t *pBuf)
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

static uint16_t IotAP_SendHeartbeatReq(uint8_t port, uint8_t *pBuf)
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

static uint16_t IotAP_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
{
    uint8_t cp56Time[7] = { 0 };
    const uint8_t zeroCp56[7] = { 0 };
    uint8_t *pSyncTime = cp56Time;
    uint16_t ret = 0;                          /* 默认失败值 */

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        if (memcmp(pIotAPCtx->syncTimeCp56, zeroCp56, sizeof(pIotAPCtx->syncTimeCp56)) == 0)
        {
            Common_TimestampToCp56Time2a(SSTM_GetSecTimestamp(), cp56Time);
        }
        else
        {
            pSyncTime = pIotAPCtx->syncTimeCp56;
        }

        memcpy(pBuf, pSyncTime, sizeof(cp56Time));
        ret = sizeof(cp56Time);
    }

    return ret;
}

/* ====== B帧发送函数 - 基础 ====== */

/**
 * @brief  AP B1: 组装充电实时监测数据帧体
 * @note   将端口充电数据按协议格式逐字段填入缓冲区
 *         Step1: 固定字段：桩号BCD逆序→枪号→连接状态→实时状态字(2B)→预留(21B)
 *         Step2: 电气参数：电压/电流(各2B)→继电器状态→BMS通信状态→预留字段
 *         Step3: 电量数据：总电量(4B)→尖峰平谷分时电量(各4B×4)
 *         Step4: 时间/标识字段：SOC/充电时长→VIN/订单号/错误码等
 *         Step5: 故障告警字段（多个bit位对应各类型故障）
 * @param[in]  port       枪号
 * @param[out] pBuf       组帧输出缓冲区
 * @param[in]  pChargeData 充电数据指针（调用方已校验非空）
 * @retval   >0  成功组包的帧长度
 */
static uint16_t IotAP_AssembleRealtimeData(uint8_t port, uint8_t *pBuf,
                                            AswMonitorChargeData_Struct *pChargeData)
{
    uint16_t dataLen = 0;
    uint32_t tempVal = 0;
    uint8_t index = 0;
    uint8_t errBytes[8] = { 0 };
    uint8_t zeroOrder[16] = { 0 };
    uint32_t totalEnergyVal = 0;
    uint32_t energyVals[4] = { 0 };
    uint8_t isIdle;

    isIdle = AswMonitor_IsOrderIdle(port);

    IotAP_CopyPileDnReverse(pBuf, &dataLen);

    pBuf[dataLen++] = port;
    pBuf[dataLen++] = (AswChargeIf_CheckGunConnected(port) == TRUE) ? 0x01 : 0x00;
    Common_Uint16ToTwoUint8(&pBuf[dataLen], IotAP_GetRealtimeState(port));
    dataLen += 2;

    memset(&pBuf[dataLen], 0x00, 21);
    dataLen += 21; /* 序号5-16当前无来源，填0 */

    tempVal = AswChargeIf_GetOutputVoltage(port) / 10;
    Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)tempVal);
    dataLen += 2;
    IOTAP_CFG_DebugPrint("AP,[枪:%d]B01 输出电压=%lu(0.1V)\r\n", port, tempVal);

    tempVal = AswChargeIf_GetOutputCurrent(port) / 10;
    // tempVal = 3200;  /* TODO: wdy测试用，16A(0.01A单位) 后面删除 */
    Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)tempVal);
    dataLen += 2;
    IOTAP_CFG_DebugPrint("AP,[枪:%d]B01 输出电流=%lu(0.01A)\r\n", port, tempVal);

    pBuf[dataLen++] = (AswChargeIf_GetRelayState(port) == ASWCHARGEIF_RELAYSTATE_ON) ? 0x01 : 0x00;
    pBuf[dataLen++] = 0x00; /* BMS 通信状态，当前无独立来源，填0 */
    pBuf[dataLen++] = 0x00; /* 是否连接电池，当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4; /* 单体电池最高/最低电压当前交流桩无来源，填0 */

    /* 计算尖峰平谷分时电量 + 累加有功总电量 */
    if (isIdle == FALSE)
    {
        for (index = 0; index < 4; index++)
        {
            energyVals[index] = pChargeData->rateTotalEnergy[index] / 10;
            if (index < 3)  /* 尖/峰/平需累加扩展电量 */
            {
                energyVals[index] += pChargeData->rateTotalEnergy[index + 5] / 10;
            }
            else  /* 谷需累加深谷电量 */
            {
                energyVals[index] += pChargeData->rateTotalEnergy[index + 1] / 10;
            }
            totalEnergyVal += energyVals[index];
        }
    }

    /* 有功总电量(4B) — 参考文档 B01 报文电量段第1字段 */
    Common_Uint32ToFourUint8(&pBuf[dataLen], totalEnergyVal);
    dataLen += 4;

    /* 尖/峰/平/谷电量(各4B, 共16B) — 参考文档 B01 报文电量段第2~5字段 */
    for (index = 0; index < 4; index++)
    {
        Common_Uint32ToFourUint8(&pBuf[dataLen], energyVals[index]);
        dataLen += 4;
    }
    IOTAP_CFG_DebugPrint("AP,[枪:%d]B01 总电量=%lu, 尖=%lu, 峰=%lu, 平=%lu, 谷=%lu(0.001度)\r\n",
                         port,
                         (pChargeData->rateTotalEnergy[0] + pChargeData->rateTotalEnergy[5]
                          + pChargeData->rateTotalEnergy[1] + pChargeData->rateTotalEnergy[6]
                          + pChargeData->rateTotalEnergy[2] + pChargeData->rateTotalEnergy[7]
                          + pChargeData->rateTotalEnergy[3] + pChargeData->rateTotalEnergy[4]) / 10,
                         (pChargeData->rateTotalEnergy[0] + pChargeData->rateTotalEnergy[5]) / 10,
                         (pChargeData->rateTotalEnergy[1] + pChargeData->rateTotalEnergy[6]) / 10,
                         (pChargeData->rateTotalEnergy[2] + pChargeData->rateTotalEnergy[7]) / 10,
                         (pChargeData->rateTotalEnergy[3] + pChargeData->rateTotalEnergy[4]) / 10);

    memset(&pBuf[dataLen], 0x00, 2);
    dataLen += 2; /* SOC当前交流桩无来源，填0 */
    if (isIdle == FALSE)
    {
        Common_Uint16ToTwoUint8(&pBuf[dataLen], (uint16_t)(pChargeData->chargeTime / 60));
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 2);
    }
    dataLen += 2;

    memset(&pBuf[dataLen], 0x00, 32);
    dataLen += 32; /* 车辆唯一标识/VIN当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 3);
    dataLen += 3; /* BMS相关，当前无来源，填0 */

    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_EmergencyStop) == TRUE) ? 0x01 : 0x00;
    pBuf[dataLen++] = 0x00; /* 直流侧开关跳闸/熔断器熔断，交流桩无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 充电机过温告警当前无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 交流输入异常当前无独立来源，填0 */
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_MeterCommErr) == TRUE) ? 0x01 : 0x00;
    pBuf[dataLen++] = 0x00; /* 缺相保护当前无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 反接保护当前无来源，填0 */
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_AphaseInputOverVol) == TRUE) ? 0x01 : 0x00;
    pBuf[dataLen++] = (AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr) == TRUE) ? 0x01 : 0x00;
    pBuf[dataLen++] = 0x00; /* 风扇故障当前无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 温度传感器故障当前无来源，填0 */
    memset(&pBuf[dataLen], 0x00, 5);
    dataLen += 5; /* 电池组/单体故障字段当前交流桩无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 集中器与桩通信故障当前无来源，填0 */
    pBuf[dataLen++] = 0x00; /* 充电监控单元故障当前无来源，填0 */

    if (isIdle == FALSE)
    {
        Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalElecMoney / 100);
        dataLen += 4;
        Common_Uint32ToFourUint8(&pBuf[dataLen], pChargeData->totalServeMoney / 100);
        dataLen += 4;
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 8);
        dataLen += 8;
    }
    memset(&pBuf[dataLen], 0x00, 4);
    dataLen += 4; /* 剩余时长当前无来源，填0 */

    if (isIdle == FALSE)
    {
        if (memcmp(pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum, zeroOrder, sizeof(zeroOrder)) != 0)
        {
            memcpy(&pBuf[dataLen], pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum, 16);
        }
        else
        {
            memset(&pBuf[dataLen], 0x00, 16); /* 当前无订单流水号时填0 */
        }
    }
    else
    {
        memset(&pBuf[dataLen], 0x00, 16);
    }
    dataLen += 16;

    /*
     * 平台其他故障码按字节顺序发送，每字节从最高位开始编号：
     * bit0 = 0x80，bit7 = 0x01。
     */
    if (AswErrHandle_CheckErrExit(port, eErr_ReaderCommErr) == TRUE)
    {
        errBytes[0] |= (1U << 6);  /* bit1：读卡器通信故障 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_ShortCircleErr) == TRUE)
    {
        errBytes[2] |= (1U << 6);  /* bit17：充电模块输出短路故障 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_OutputOverCurr) == TRUE)
    {
        errBytes[2] |= (1U << 5);  /* bit18：充电模块输出过流告警 */
    }
    if ((AswErrHandle_CheckErrExit(port, eErr_CpVoltAbnor) == TRUE) ||
        (AswErrHandle_CheckErrExit(port, eErr_CpGroundFault) == TRUE))
    {
        errBytes[2] |= (1U << 1);  /* bit22：充电中车辆控制导引告警 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_GunOverTempErr) == TRUE)
    {
        errBytes[3] |= (1U << 4);  /* bit27：充电枪过温故障 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_EnvOverTempErr) == TRUE)
    {
        errBytes[4] |= (1U << 6);  /* bit33：充电桩过温告警 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_JcqMaloperation) == TRUE)
    {
        errBytes[4] |= (1U << 3);  /* bit36：交流输入接触器误动/拒动 */
    }
    if (AswErrHandle_CheckErrExit(port, eErr_JcqSynechiaFault) == TRUE)
    {
        errBytes[4] |= (1U << 2);  /* bit37：交流输入接触器粘连 */
    }
    memcpy(&pBuf[dataLen], errBytes, sizeof(errBytes));
    dataLen += sizeof(errBytes);

    return dataLen;
}


/**
 * @brief  AP B1: 充电过程实时监测数据，组帧上报当前充电过程的全部状态信息
 * @note   校验参数合法性后，委托 IotAP_AssembleRealtimeData 完成组包
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败（参数非法或数据指针为空）
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendRealtimeData(uint8_t port, uint8_t *pBuf)
{
    uint16_t ret = 0;
    AswMonitorChargeData_Struct *pChargeData = NULL;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pChargeData = AswMonitor_GetChargeDataPtr(port);
        if (pChargeData != NULL)
        {
            ret = IotAP_AssembleRealtimeData(port, pBuf, pChargeData);
        }
    }

    return ret;
}

/**
 * @brief  AP B5: 充电启停控制命令结果确认，组帧应答平台下发的启停控制指令执行结果
 * @note   Step1: 参数合法性校验，获取协议上下文指针
 *         Step2: 根据remoteCtrlCmd区分命令类型，取对应结果和失败原因
 *         Step3: 组包：桩号BCD逆序→枪号→结果→预留→失败原因BCD化→命令类型→CP56时间戳(7B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendChgCtrlResult(uint8_t port, uint8_t *pBuf)
{
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t result = 1;
    uint8_t failReason = 0x03;
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pProtoData = &pIotAPCtx->stProtoData[port];

        if (pProtoData->remoteCtrlCmd == 0)
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
        // pBuf[dataLen++] = 0x00;
        // pBuf[dataLen++] = (uint8_t)(((failReason / 10) << 4) | (failReason % 10));
        pBuf[dataLen++] = failReason;   /* 失败原因低字节 */
        pBuf[dataLen++] = 0x00;        /* 高字节 */
        pBuf[dataLen++] = pProtoData->remoteCtrlCmd;

        Common_TimestampToCp56Time2a(SSTM_GetSecTimestamp(), &pBuf[dataLen]);
        dataLen += 7;

        memcpy(&pBuf[dataLen], pProtoData->newRecvOrderTransactionNum, 16);
        dataLen += 16;
    }

    return dataLen;
}

/* ====== B帧发送函数 - 鉴权 ====== */

/* AP B6: 刷卡鉴权上行 */
static uint16_t IotAP_SendCardAuthUp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    uint16_t dataLen = 0;
    uint8_t index = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);

        if (pChargeCtrl != NULL)
        {
            IotAP_CopyPileDnReverse(pBuf, &dataLen);
            pBuf[dataLen++] = port;
            /* AP card number uses 8-byte BCD, low-order byte first. */
            for (index = 0; index < IOT_AP_CARD_NO_LEN; index++)
            {
                pBuf[dataLen++] = pChargeCtrl->authCardID[IOT_AP_CARD_NO_LEN - 1u - index];
            }
            memset(&pBuf[dataLen], 0x00, IOT_AP_CARD_PASSWORD_LEN);
            dataLen += IOT_AP_CARD_PASSWORD_LEN;
            memset(&pBuf[dataLen], 0x00, IOT_AP_CARD_BALANCE_LEN);
            dataLen += IOT_AP_CARD_BALANCE_LEN;
            memset(&pBuf[dataLen], 0x00, IOT_AP_CARD_VIN_LEN);
            dataLen += IOT_AP_CARD_VIN_LEN;
        }
    }

    return dataLen;
}

/**
 * @brief  AP B10: 启动通知上报，组帧并填充启动认证信息
 * @note   Step1: 参数合法性校验（缓冲区/上下文/枪号）
 *         Step2: 从协议上下文获取充电控制器指针
 *         Step3: 按协议逐字段组包：桩号BCD逆序→枪号→卡号→密码→余额→VIN码→启动方式→时间戳
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败（参数非法或枪号越界）
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendStartNotifyUp(uint8_t port, uint8_t *pBuf)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    IotAPProtoData_Struct *pProtoData = NULL;
    uint16_t dataLen = 0;
    uint8_t index = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);
        pProtoData = &pIotAPCtx->stProtoData[port];

        if (pChargeCtrl != NULL)
        {
            IotAP_CopyPileDnReverse(pBuf, &dataLen);
            pBuf[dataLen++] = port;
            /* AP card number uses 8-byte BCD, low-order byte first. */
            for (index = 0; index < IOT_AP_CARD_NO_LEN; index++)
            {
                pBuf[dataLen++] = pChargeCtrl->authCardID[IOT_AP_CARD_NO_LEN - 1u - index];
            }
            memset(&pBuf[dataLen], 0x00, IOT_AP_CARD_PASSWORD_LEN);
            dataLen += IOT_AP_CARD_PASSWORD_LEN;
            Common_Uint32ToFourUint8(&pBuf[dataLen], pProtoData->cardAccountBalance);
            dataLen += IOT_AP_CARD_BALANCE_LEN;
            memcpy(&pBuf[dataLen], pProtoData->cardVin, IOT_AP_CARD_VIN_LEN);
            dataLen += IOT_AP_CARD_VIN_LEN;
            pBuf[dataLen++] = IOT_AP_START_WAY_FULL;
            Common_Uint32ToFourUint8(&pBuf[dataLen], 0);
            dataLen += 4;
        }
    }

    return dataLen;
}

/* ====== B帧发送函数 - 交易 ====== */

/* AP B14: 充电扣款后下行数据 */
static uint16_t IotAP_SendDeductConfirmRsp(uint8_t port, uint8_t *pBuf)
{
    /* B14 is a platform downlink settlement notice and has no AP upstream response. */
    return 0;
}

/* AP B53: 在线情况下停止充电上传分时交易明细数据 */
static uint16_t IotAP_SendOnlineDetailUp(uint8_t port, uint8_t *pBuf)
{
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        IotAP_TransformChargeRecord(&pIotAPCtx->stOrderInfo.platOrderInfo, pBuf, &dataLen);
    }

    return dataLen;
}

/* ====== B帧发送函数 - 计费 ====== */

/**
 * @brief  AP B48: 下发计费模型上行数据-分时服务费，应答平台的计费模型下发
 * @note   Step1: 参数合法性校验
 *         Step2: 组包：桩号BCD逆序→枪号→计费模型ID(8B)→结果(1B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendTimeBillUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[18] = { 0 };
    uint16_t ret = 0;                          /* 默认失败值 */

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        uint16_t dataLen = 0;

        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = port;
        memcpy(&dataBuf[dataLen], pIotAPCtx->stProtoData[port].timeBillModelId, sizeof(pIotAPCtx->stProtoData[port].timeBillModelId));
        dataLen += sizeof(pIotAPCtx->stProtoData[port].timeBillModelId);
        dataBuf[dataLen++] = pIotAPCtx->stProtoData[port].timeBillResult;

        memcpy(pBuf, dataBuf, dataLen);
        ret = dataLen;                         /* 组包成功 → 返回帧长度 */
    }
    /* 参数无效 → ret保持0 */

    return ret;                                /* 唯一出口 */
}

/**
 * @brief  AP B49: 计费模型切换生效上行-分时服务费，应答平台切换确认
 * @note   Step1: 参数合法性校验
 *         Step2: 组包：桩号BCD逆序→枪号→切换目标计费模型ID(8B)→切换时间(7B)→结果(1B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendTimeBillSwitchUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[25] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = port;
        memcpy(&dataBuf[dataLen],
               pIotAPCtx->stProtoData[port].timeBillSwitchModelId,
               sizeof(pIotAPCtx->stProtoData[port].timeBillSwitchModelId));
        dataLen += sizeof(pIotAPCtx->stProtoData[port].timeBillSwitchModelId);
        memcpy(&dataBuf[dataLen],
               pIotAPCtx->stProtoData[port].timeBillSwitchTime,
               sizeof(pIotAPCtx->stProtoData[port].timeBillSwitchTime));
        dataLen += sizeof(pIotAPCtx->stProtoData[port].timeBillSwitchTime);
        dataBuf[dataLen++] = pIotAPCtx->stProtoData[port].timeBillSwitchResult;

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
}

/**
 * @brief  AP B52: 计费模型召测上行数据-分时服务费，响应平台召测请求上报当前活跃费率
 * @note   Step1: 空枪时刷新当前计费模型
 *         Step2: 检查活跃索引及费率有效性
 *         Step3: 组包：桩号BCD逆序→枪号→召测时间→计费模型ID→切换/失效时间→时段数
 *         Step4: 逐时段组包：序号→费率类型→开始/结束时间→电费→服务费
 *         Step5: 无有效模型时填0填充，结果字段置1
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendTimeBillPollUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[IOT_AP_B52_MAX_DATA_LEN] = { 0 };
    uint16_t dataLen = 0;
    uint8_t result = 1;
    const MSNvmAPParamBillMode_Struct *pBillMode = NULL;
    const MSNvmAPParamBillPeriod_Struct *pPeriod = NULL;
    uint8_t index = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        if (AswMonitor_IsOrderIdle(port) == TRUE)
        {
            IotAP_RefreshNowbillModel(port);
        }

        pBillMode = IotAP_GetActiveBillMode(port);
        if (pBillMode != NULL)
        {
            result = 0;
        }

        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = port;
        memcpy(&dataBuf[dataLen],
               pIotAPCtx->stProtoData[port].timeBillPollTime,
               sizeof(pIotAPCtx->stProtoData[port].timeBillPollTime));
        dataLen += sizeof(pIotAPCtx->stProtoData[port].timeBillPollTime);

        /*当前存在有效计费模型*/
        if ((result == 0) && (pBillMode != NULL))
        {
            memcpy(&dataBuf[dataLen], pBillMode->billModeID, sizeof(pBillMode->billModeID));
            dataLen += sizeof(pBillMode->billModeID);
            memcpy(&dataBuf[dataLen], pBillMode->switchTime, sizeof(pBillMode->switchTime));
            dataLen += sizeof(pBillMode->switchTime);
            memcpy(&dataBuf[dataLen], pBillMode->invalidTime, sizeof(pBillMode->invalidTime));
            dataLen += sizeof(pBillMode->invalidTime);
            dataBuf[dataLen++] = pBillMode->periodCount;

            for (index = 0; index < pBillMode->periodCount; index++)
            {
                pPeriod = &pBillMode->period[index];
                dataBuf[dataLen++] = pPeriod->periodSerial;
                dataBuf[dataLen++] = pPeriod->periodRate;
                memcpy(&dataBuf[dataLen], pPeriod->startTime, sizeof(pPeriod->startTime));
                dataLen += sizeof(pPeriod->startTime);
                memcpy(&dataBuf[dataLen], pPeriod->stopTime, sizeof(pPeriod->stopTime));
                dataLen += sizeof(pPeriod->stopTime);
                memcpy(&dataBuf[dataLen], pPeriod->elecPrice, sizeof(pPeriod->elecPrice));
                dataLen += sizeof(pPeriod->elecPrice);
                memcpy(&dataBuf[dataLen], pPeriod->servePrice, sizeof(pPeriod->servePrice));
                dataLen += sizeof(pPeriod->servePrice);
            }
        }
        else
        {
            /* 无有效计费模型: 占位填充, 维持帧结构长度一致 */
            dataLen += IOT_AP_B52_FAIL_MODEL_LEN;
        }

        dataBuf[dataLen++] = result;
        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
}

/* ====== B帧发送函数 - 功率控制 ====== */

/**
 * @brief  AP B34: 充电功率控制上行，应答平台的功率调节指令执行结果
 * @note   Step1: 参数合法性校验
 *         Step2: 组包：桩号BCD逆序→枪号→时间戳原样回传(7B)→执行结果(1B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendPowerCtrlUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[17] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
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
    }

    return dataLen;
}

/**
 * @brief  AP B46: 充电功率召测上行，响应平台召测当前功率状态
 * @note   Step1: 参数合法性校验
 *         Step2: 获取协议上下文指针，处理默认值为0时的补偿逻辑
 *         Step3: 组包：桩号BCD逆序→枪号→控制值(4B)→默认值(4B)→动态值(4B)→预留(1B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendPowerPollUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[22] = { 0 };
    uint16_t dataLen = 0;
    uint32_t defaultValue = 0;
    IotAPProtoData_Struct *pProtoData = NULL;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pProtoData = &pIotAPCtx->stProtoData[port];
        defaultValue = (pProtoData->powerCtrlDefaultValue == 0) ?
                       (SYSCFG_CFG_MAX_OUTPUT_POWER / 10) : pProtoData->powerCtrlDefaultValue;
        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = port;
        Common_Uint32ToFourUint8(&dataBuf[dataLen], pProtoData->powerCtrlControlValue);
        dataLen += 4;
        Common_Uint32ToFourUint8(&dataBuf[dataLen], defaultValue);
        dataLen += 4;
        Common_Uint32ToFourUint8(&dataBuf[dataLen], pProtoData->powerCtrlDynamicValue);
        dataLen += 4;
        dataBuf[dataLen++] = 0x00;

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
}

/**
 * @brief  AP B57: 充电功率控制过程中的扩展实时状态，周期性上报当前实际功率状态
 * @note   Step1: 参数合法性校验
 *         Step2: 获取充电接口输出电压/电流及协议上下文指针
 *         Step3: 组包：桩号BCD逆序→枪号→CP56时间戳(7B)→电压(2B)→电流(2B)→预留(1B)→当前功率(2B)→预留(6B)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendPowerStatusUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[29] = { 0 };
    uint16_t dataLen = 0;
    uint32_t tempVal = 0;
    IotAPProtoData_Struct *pProtoData = NULL;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pProtoData = &pIotAPCtx->stProtoData[port];
        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = port;
        Common_TimestampToCp56Time2a(SSTM_GetSecTimestamp(), &dataBuf[dataLen]);
        dataLen += 7;

        tempVal = AswChargeIf_GetOutputVoltage(port) / 10;
        Common_Uint16ToTwoUint8(&dataBuf[dataLen], (uint16_t)tempVal);
        dataLen += 2;
        tempVal = AswChargeIf_GetOutputCurrent(port) / 10;
        Common_Uint16ToTwoUint8(&dataBuf[dataLen], (uint16_t)tempVal);
        dataLen += 2;
        dataBuf[dataLen++] = 0;
        /* 当前功率(2B, 协议0.01kW/位) - 取实际输出功率, 接口返回0.001kW → /10 */
        tempVal = AswChargeIf_GetOutputPower(port) / 10;
        Common_Uint16ToTwoUint8(&dataBuf[dataLen], (uint16_t)tempVal);
        dataLen += 2;
        memset(&dataBuf[dataLen], 0x00, 6);
        dataLen += 6;

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
}

/* ====== B帧发送函数 - 其他扩展 ====== */

/**
 * @brief  AP B31: SIM卡信息上行数据，上报桩内SIM卡的ICCID等信息
 * @note   Step1: 参数合法性校验（参数非法直接早期返回0）
 *         Step2: 调用网络驱动获取ICCID
 *         Step3: 组包：桩号BCD逆序→ICCID(20B)→手机号(11B填0)
 * @param[in]  port   枪号
 * @param[out] pBuf   组帧输出缓冲区
 * @retval   0       组包失败
 * @retval   >0      成功组包的帧长度
 */
static uint16_t IotAP_SendSimInfoUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[39] = { 0 };
    uint16_t dataLen = 0;
    uint8_t iccid[20] = { 0 };

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        CddNetM_GetIccid(iccid);
        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        memcpy(&dataBuf[dataLen], iccid, sizeof(iccid));
        dataLen += sizeof(iccid);
        memset(&dataBuf[dataLen], 0x00, 11);
        dataLen += 11; /* 手机号当前工程无来源，填0 */

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;  /* 唯一出口 */
}

/* AP B40: 平台ftp服务器地址上行 */
static uint16_t IotAP_SendFtpAddrUp(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[9] = { 0 };
    uint16_t dataLen = 0;
    IotAPProtoData_Struct *pProtoData = NULL;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        pProtoData = &pIotAPCtx->stProtoData[port];

        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = pProtoData->ftpAddrResult;

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
}

/* AP B24: 远程升级启动命令接收结果 */
static uint16_t IotAP_SendUpgradeResult(uint8_t port, uint8_t *pBuf)
{
    uint8_t dataBuf[9] = { 0 };
    uint16_t dataLen = 0;

    if ((pBuf != NULL) && (port < SYSCFG_CFG_GUN_NUM))
    {
        IotAP_CopyPileDnReverse(dataBuf, &dataLen);
        dataBuf[dataLen++] = pIotAPCtx->upgradeResult;

        memcpy(pBuf, dataBuf, dataLen);
    }

    return dataLen;
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
                        pIotAPCtx->queueBusyFlag = TRUE;
 			            pIotAPCtx->waitQueueIdleTick = Common_GetSystick();
                        
                        if (eGlobalRet_OK != FrameQueue_PushTx(pIotAPCtx->frameQueueChannelID, NULL, 0, txBuf, dataLen, 1))
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
                            if ((Common_GetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B40_FTP_ADDR_UP) == TRUE) )
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
