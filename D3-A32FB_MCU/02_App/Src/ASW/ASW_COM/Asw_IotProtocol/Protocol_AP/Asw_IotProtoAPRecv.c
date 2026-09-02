/******************************************************************************
* File Name          : Asw_IotProtoAPRecv.c
* Description        : 安培协议接收帧解析实现
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
#include <stdio.h>
#include "Asw_IotProtoAPRecv.h"
#include "Asw_IotProtoAPM.h"
#include "FrameQueue.h"
#include "Asw_ErrorHandle.h"
#include "SS_Tm.h"
#include "SS_Ucm.h"
#include "Asw_ChargeIf.h"
#include "Asw_Monitor.h"
#include "Asw_PlatM.h"
#include "MS_Nvm.h"

/*******************************************************************************
*    Static Function Prototypes
*******************************************************************************/
static uint8_t IotAP_ParseLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseUFrameAck(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseHeartbeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseSyncTimeReq(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseChgCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseCardAuthDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseStartNotifyDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseOnlineDetailDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseDeductConfirmDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseTimeBillDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseTimeBillSwitchDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseTimeBillPollDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParsePowerCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParsePowerPollDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseTerminalReqDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseFtpAddrDown(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotAP_ParseUpgradeStart(uint8_t *port, uint8_t *r_data, uint16_t len);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
#define IOT_AP_CLOCK_SYNC_HEAD_LEN              (14)
#define IOT_AP_B4_PILE_DN_OFFSET                (0)
#define IOT_AP_B4_PORT_OFFSET                   (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B4_CTRL_CMD_OFFSET               (IOT_AP_B4_PORT_OFFSET + 1)
#define IOT_AP_B4_START_CONDITION_OFFSET        (IOT_AP_B4_CTRL_CMD_OFFSET + 1)
#define IOT_AP_B4_START_WAY_OFFSET              (IOT_AP_B4_START_CONDITION_OFFSET + 1)
#define IOT_AP_B4_CTRL_DATA_OFFSET              (IOT_AP_B4_START_WAY_OFFSET + 1)
#define IOT_AP_B4_USER_NO_OFFSET                (IOT_AP_B4_CTRL_DATA_OFFSET + 4)
#define IOT_AP_B4_ORDER_NO_OFFSET               (IOT_AP_B4_USER_NO_OFFSET + 8)
#define IOT_AP_B4_DATA_LEN                      (IOT_AP_B4_ORDER_NO_OFFSET + 16)
#define IOT_AP_B4_CTRL_STOP                     (0)
#define IOT_AP_B4_CTRL_START                    (1)
#define IOT_AP_B4_START_CONDITION_NOW           (0)
#define IOT_AP_B4_START_WAY_ENERGY              (1)
#define IOT_AP_B4_START_WAY_TIME                (2)
#define IOT_AP_B4_START_WAY_MONEY               (3)
#define IOT_AP_B4_START_WAY_FULL                (4)
#define IOT_AP_B4_ACCOUNT_MONEY_MAX             (0xFFFFFFFF)
#define IOT_AP_B7_PORT_OFFSET                   (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B7_CARD_NO_OFFSET                (IOT_AP_B7_PORT_OFFSET + 1)
#define IOT_AP_B7_VIN_OFFSET                    (IOT_AP_B7_CARD_NO_OFFSET + 8)
#define IOT_AP_B7_BILL_MODE_ID_OFFSET           (IOT_AP_B7_VIN_OFFSET + 32)
#define IOT_AP_B7_ACCOUNT_BALANCE_OFFSET        (IOT_AP_B7_BILL_MODE_ID_OFFSET + 8)
#define IOT_AP_B7_AUTH_RESULT_OFFSET            (IOT_AP_B7_ACCOUNT_BALANCE_OFFSET + 4)
#define IOT_AP_B7_FAIL_REASON_OFFSET            (IOT_AP_B7_AUTH_RESULT_OFFSET + 1)
#define IOT_AP_B7_MIN_DATA_LEN                  (IOT_AP_B7_FAIL_REASON_OFFSET + 2 + 8)
#define IOT_AP_CARD_NO_LEN                       (8)
#define IOT_AP_B11_PORT_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B11_RESULT_OFFSET                (IOT_AP_B11_PORT_OFFSET + 1)
#define IOT_AP_B11_FAIL_REASON_OFFSET           (IOT_AP_B11_RESULT_OFFSET + 1)
#define IOT_AP_B11_ORDER_NO_OFFSET              (IOT_AP_B11_FAIL_REASON_OFFSET + 2)
#define IOT_AP_B11_MIN_DATA_LEN                 (IOT_AP_B11_ORDER_NO_OFFSET + 16)
#define IOT_AP_B54_PORT_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B54_RESULT_OFFSET                (IOT_AP_B54_PORT_OFFSET + 1)
#define IOT_AP_B54_ORDER_NO_OFFSET              (IOT_AP_B54_RESULT_OFFSET + 1)
#define IOT_AP_B54_MIN_DATA_LEN                 (IOT_AP_B54_RESULT_OFFSET + 1)
#define IOT_AP_B54_ORDER_DATA_LEN               (IOT_AP_B54_ORDER_NO_OFFSET + 16)
#define IOT_AP_B14_CARD_NO_OFFSET               (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B14_DEDUCT_AMOUNT_OFFSET         (IOT_AP_B14_CARD_NO_OFFSET + 8)
#define IOT_AP_B14_ACCOUNT_BALANCE_OFFSET       (IOT_AP_B14_DEDUCT_AMOUNT_OFFSET + 4)
#define IOT_AP_B14_DEDUCT_RESULT_OFFSET         (IOT_AP_B14_ACCOUNT_BALANCE_OFFSET + 4)
#define IOT_AP_B14_FAIL_REASON_OFFSET           (IOT_AP_B14_DEDUCT_RESULT_OFFSET + 1)
#define IOT_AP_B14_MIN_DATA_LEN                 (IOT_AP_B14_FAIL_REASON_OFFSET + 2 + (4 * 6))
#define IOT_AP_POWER_CTRL_PAUSE_NONE             (0U)
#define IOT_AP_POWER_CTRL_PAUSE                  (1U)
#define IOT_AP_POWER_CTRL_RESUME                 (2U)
#define IOT_AP_B47_PORT_LEN                     (1)
#define IOT_AP_B47_BILL_MODE_ID_LEN             (8)
#define IOT_AP_B47_SWITCH_TIME_LEN              (7)
#define IOT_AP_B47_INVALID_TIME_LEN             (7)
#define IOT_AP_B47_WORK_STATE_LEN               (2)
#define IOT_AP_B47_PERIOD_COUNT_LEN             (1)
#define IOT_AP_B47_PERIOD_SERIAL_OFFSET         (0)
#define IOT_AP_B47_PERIOD_RATE_OFFSET           (1)
#define IOT_AP_B47_PERIOD_LEN                   (14)
#define IOT_AP_B47_PORT_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B47_BILL_MODE_ID_OFFSET          (IOT_AP_B47_PORT_OFFSET + IOT_AP_B47_PORT_LEN)
#define IOT_AP_B47_PERIOD_COUNT_OFFSET          (IOT_AP_B47_BILL_MODE_ID_OFFSET + \
                                                IOT_AP_B47_BILL_MODE_ID_LEN + \
                                                IOT_AP_B47_SWITCH_TIME_LEN + \
                                                IOT_AP_B47_INVALID_TIME_LEN + \
                                                IOT_AP_B47_WORK_STATE_LEN)
#define IOT_AP_B47_PERIOD_DATA_OFFSET           (IOT_AP_B47_PERIOD_COUNT_OFFSET + IOT_AP_B47_PERIOD_COUNT_LEN)
#define IOT_AP_B47_MIN_DATA_LEN                 (IOT_AP_B47_PERIOD_DATA_OFFSET)
#define IOT_AP_B50_PORT_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B50_BILL_MODE_ID_OFFSET          (IOT_AP_B50_PORT_OFFSET + 1)
#define IOT_AP_B50_MIN_DATA_LEN                 (IOT_AP_B50_BILL_MODE_ID_OFFSET + 8)
#define IOT_AP_B51_PORT_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B51_TIME_OFFSET                  (IOT_AP_B51_PORT_OFFSET + 1)
#define IOT_AP_B51_TIME_LEN                     (7)
#define IOT_AP_B51_MIN_DATA_LEN                 (IOT_AP_B51_TIME_OFFSET + IOT_AP_B51_TIME_LEN)
#define IOT_AP_B32_KIND_OFFSET                  (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B32_KIND_LEN                     (2)
#define IOT_AP_B32_MIN_DATA_LEN                 (IOT_AP_B32_KIND_OFFSET + IOT_AP_B32_KIND_LEN)
#define IOT_AP_FTP_IP_LEN                       (4)
#define IOT_AP_FTP_PORT_LEN                     (2)
#define IOT_AP_FTP_USER_LEN                     (10)
#define IOT_AP_FTP_PASSWD_LEN                   (10)
#define IOT_AP_FTP_PATH_LEN                     (50)
#define IOT_AP_FTP_FILE_NAME_LEN                (50)
#define IOT_AP_B23_IP_OFFSET                    (IOT_AP_PILE_DN_LEN)
#define IOT_AP_B23_PORT_OFFSET                  (IOT_AP_B23_IP_OFFSET + IOT_AP_FTP_IP_LEN)
#define IOT_AP_B23_USER_OFFSET                  (IOT_AP_B23_PORT_OFFSET + IOT_AP_FTP_PORT_LEN)
#define IOT_AP_B23_PASSWD_OFFSET                (IOT_AP_B23_USER_OFFSET + IOT_AP_FTP_USER_LEN)
#define IOT_AP_B23_PATH_OFFSET                  (IOT_AP_B23_PASSWD_OFFSET + IOT_AP_FTP_PASSWD_LEN)
#define IOT_AP_B23_FILE_NAME_OFFSET             (IOT_AP_B23_PATH_OFFSET + IOT_AP_FTP_PATH_LEN)
#define IOT_AP_B23_MODE_OFFSET                  (IOT_AP_B23_FILE_NAME_OFFSET + IOT_AP_FTP_FILE_NAME_LEN)
#define IOT_AP_B23_SOFT_VERSION_OFFSET          (IOT_AP_B23_MODE_OFFSET + 1)
#define IOT_AP_B23_HARD_VERSION_OFFSET          (IOT_AP_B23_SOFT_VERSION_OFFSET + 30)
#define IOT_AP_B23_MIN_DATA_LEN                 (IOT_AP_B23_HARD_VERSION_OFFSET + 20)

const IotAPRecvCtrl_Struct c_stIotAPRecvctrlTable[IOT_AP_CMD_RECV_COUNT] =
{
    [0] =
    {
        .cmd = IOT_AP_CMD_LOGIN_RSP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登录验证应答",
    },
    [1] =
    {
        .cmd = IOT_AP_CMD_UFRAME_ACK,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseUFrameAck,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_UFRAME_AUTH,
        .printFlag = TRUE,
        .cMeaning = "U帧认证应答",
    },
    [2] =
    {
        .cmd = IOT_AP_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseHeartbeatRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_HEARTBEAT_REQ,
        .printFlag = TRUE,
        .cMeaning = "心跳应答",
    },
    [3] =
    {
        .cmd = IOT_AP_CMD_SYNC_TIME_REQ,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseSyncTimeReq,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_SYNC_TIME_RSP,
        .printFlag = TRUE,
        .cMeaning = "时钟同步请求",
    },
    [4] =
    {
        .cmd = IOT_AP_CMD_B04_CHG_CTRL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseChgCtrlDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B05_CHG_CTRL_RESULT,
        .printFlag = TRUE,
        .cMeaning = "充电启停控制下发",
    },
    [5] =
    {
        .cmd = IOT_AP_CMD_B07_CARD_AUTH_DOWN,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseCardAuthDown,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_B06_CARD_AUTH_UP,
        .printFlag = TRUE,
        .cMeaning = "刷卡鉴权下行",
    },
    [6] =
    {
        .cmd = IOT_AP_CMD_B11_START_NOTIFY_DOWN,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseStartNotifyDown,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_B10_START_NOTIFY_UP,
        .printFlag = TRUE,
        .cMeaning = "启动通知下行",
    },
    [7] =
    {
        .cmd = IOT_AP_CMD_B14_DEDUCT_CONFIRM,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseDeductConfirmDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_NULL,
        .printFlag = TRUE,
        .cMeaning = "扣款确认下行",
    },
    [8] =
    {
        .cmd = IOT_AP_CMD_B23_UPGRADE_START,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseUpgradeStart,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B24_UPGRADE_RESULT,
        .printFlag = TRUE,
        .cMeaning = "远程升级启动",
    },
    [9] =
    {
        .cmd = IOT_AP_CMD_B32_TERMINAL_REQ_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseTerminalReqDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B31_SIM_INFO_UP,
        .printFlag = TRUE,
        .cMeaning = "请求终端数据下行",
    },
    [10] =
    {
        .cmd = IOT_AP_CMD_B33_POWER_CTRL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParsePowerCtrlDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B34_POWER_CTRL_UP,
        .printFlag = TRUE,
        .cMeaning = "充电功率控制下发",
    },
    [11] =
    {
        .cmd = IOT_AP_CMD_B39_FTP_ADDR_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseFtpAddrDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B40_FTP_ADDR_UP,
        .printFlag = TRUE,
        .cMeaning = "FTP地址下发",
    },
    [12] =
    {
        .cmd = IOT_AP_CMD_B45_POWER_POLL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParsePowerPollDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B46_POWER_POLL_UP,
        .printFlag = TRUE,
        .cMeaning = "充电功率召测下发",
    },
    [13] =
    {
        .cmd = IOT_AP_CMD_B47_TIMEBILL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseTimeBillDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B48_TIMEBILL_UP,
        .printFlag = TRUE,
        .cMeaning = "分时服务费模型下发",
    },
    [14] =
    {
        .cmd = IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseTimeBillSwitchDown,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP,
        .printFlag = TRUE,
        .cMeaning = "分时服务费切换生效下行",
    },
    [15] =
    {
        .cmd = IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_REQUSET,
        .pRecvParse = IotAP_ParseTimeBillPollDown,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_AP_CMD_B52_TIMEBILL_POLL_UP,
        .printFlag = TRUE,
        .cMeaning = "分时服务费召测下发",
    },
    [16] =
    {
        .cmd = IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN,
        .cmdType = IOT_AP_CMDTYPE_RESPONSE,
        .pRecvParse = IotAP_ParseOnlineDetailDown,
        .maxTimeout = 60 * 1000,
        .maxTryCnt = 10,
        .matchCmd = IOT_AP_CMD_B53_ONLINE_DETAIL_UP,
        .printFlag = TRUE,
        .cMeaning = "在线分时交易明细下行",
    },
};

/*******************************************************************************
*    Static Local Functions Declaration
******************************************************************************/
static const IotAPRecvCtrl_Struct* IotAP_GetRecvCtrlPtr(uint16_t cmd);
static uint16_t IotAP_GetFrameTotalLen(const uint8_t *pFrame);
static IotAPInfoFrameHead_Struct* IotAP_FindValidFrame(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen);
static uint16_t IotAP_IdentifyCmd(IotAPInfoFrameHead_Struct *pHead, uint16_t frameTotalLen);
static void IotAP_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen);
static uint8_t IotAP_CheckPileDn(const uint8_t *pBuf, uint8_t offset);
static uint8_t IotAP_CheckChargeStart(uint8_t port, uint8_t *pFailReason,IotAPStopReason_Enum *pStopReason);
static void IotAP_FillChargeCtrlFromB4(uint8_t port, uint8_t startWay, uint32_t ctrlData, const uint8_t *pUserNo);
static uint8_t IotAP_FillFtpParaFromPayload(CddNetMSocketPara_Union *pFtpPara,
                                            const uint8_t *pData,
                                            uint16_t len);
static uint8_t IotAP_IsFtpParaValid(const CddNetMSocketPara_Union *pFtpPara);

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/* ====== 帧校验与CMD识别内部函数 ====== */

/*
 * 接收控制结构体查找：按 CMD 遍历接收控制表返回匹配项指针
 */
/**
 * @brief  根据命令字在接收控制表中查找对应的控制结构体指针
 * @param  cmd  待查找的AP命令字（如0x39等）
 * @return 找到则返回对应 IotAPRecvCtrl_Struct 指针，未找到返回 NULL
 */
static const IotAPRecvCtrl_Struct *IotAP_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotAPRecvCtrl_Struct *pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_AP_CMD_RECV_COUNT; index++)
    {
        if (c_stIotAPRecvctrlTable[index].cmd == cmd)
        {
            pCtrl = &c_stIotAPRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

/**
 * @brief  获取AP协议帧的总长度
 * @note   当帧长度字段(pFrame[1])为0x01时，视为固定短帧，总长度为12字节；
 *         否则总长度 = 帧长度字段值 + 起始符(1B) + 帧长度字段(1B)
 * @param  pFrame 指向AP协议帧数据的指针
 * @return 帧总长度(字节)；若pFrame为NULL则返回0
 */
static uint16_t IotAP_GetFrameTotalLen(const uint8_t *pFrame)
{
    uint16_t frameTotalLen = 0;

    if (pFrame != NULL)
    {
        if (pFrame[1] == 0x01) /*协议标识帧固定恢复01，其他帧为可变长度*/
        {
            frameTotalLen = 12;
        }
        else
        {
            frameTotalLen = (uint16_t)pFrame[1] + 2;
        }
    }    

    return frameTotalLen;
}

/**
 * @brief  根据接收命令类型获取日志记录长度
 *
 * 针对部分命令类型，限制日志输出长度以避免日志过长；
 * 未特殊处理的命令则使用帧总长度作为日志长度。
 *
 * @param  recvCmd       接收到的AP命令字
 * @param  frameTotalLen 帧数据总长度（字节数）
 * @return 实际用于日志记录的长度（不超过帧总长度）
 */
static uint16_t IotAP_GetRecvLogLen(uint16_t recvCmd, uint16_t frameTotalLen)
{
    uint16_t logLen = frameTotalLen;

    switch (recvCmd)
    {
        case IOT_AP_CMD_LOGIN_RSP:
        {
            logLen = 12;
            break;
        }

        case IOT_AP_CMD_UFRAME_ACK:
        case IOT_AP_CMD_HEARTBEAT_RSP:
        {
            logLen = 6;
            break;
        }

        default:
        {
            break;
        }
    }

    if (logLen > frameTotalLen)
    {
        logLen = frameTotalLen;
    }

    return logLen;
}

/**
 * @brief  校验缓冲区中指定偏移处的充电设备编号是否与本地存储的编号一致
 * @note   将缓冲区中从 offset 偏移处开始的 BCD 编码与上下文中存储的
 *         pileDnBCD 进行逐字节逆序比对。整帧场景传 2，数据域场景传
 *         IOT_AP_B4_PILE_DN_OFFSET。
 * @param[in] pBuf    指向待校验的缓冲区
 * @param[in] offset  充电设备编号在缓冲区中的起始偏移
 * @retval TRUE   编号一致
 * @retval FALSE  编号不一致或参数为空
 */
static uint8_t IotAP_CheckPileDn(const uint8_t *pBuf, uint8_t offset)
{
    uint8_t ret = FALSE;
    uint8_t index = 0;
    uint8_t dnByteLen = 0;

    if (pBuf != NULL)
    {
        dnByteLen = (uint8_t)sizeof(pIotAPCtx->pileDnBCD);
        ret = TRUE;
        for (index = 0; index < dnByteLen; index++)
        {
            if (pBuf[offset + index] != pIotAPCtx->pileDnBCD[dnByteLen - 1U - index])
            {
                ret = FALSE;
                break;
            }
        }
    }

    return ret;
}

/**
 * @brief  检查充电启动条件是否满足
 *
 * 依次检查端口号有效性、订单空闲状态、鉴权标志、充电允许条件、
 * 枪连接状态、计费模式有效性、固件升级状态及禁充状态，
 * 任一条件不满足则返回失败并给出对应原因码。
 *
 * 原因码定义：
 *   - 0x00: 无异常，允许启动
 *   - 0x01: 订单占用或已鉴权
 *   - 0x02: 充电条件不允许或计费模式无效
 *   - 0x03: 端口号无效、枪未连接、升级中或禁充状态
 *
 * @param[in]  port        充电端口索引
 * @param[out] pFailReason 失败原因码输出指针，可为NULL；非NULL时写入原因码
 * @return TRUE 允许启动充电，FALSE 不允许启动
 */
static uint8_t IotAP_CheckChargeStart(uint8_t port, uint8_t *pFailReason,
                                      IotAPStopReason_Enum *pStopReason)
{
    uint8_t reason = 0x00;
    IotAPStopReason_Enum stopReason = eIotAPStopReason_OtherErr;   /* 默认兜底 */
    AswErrChargeCondition_Enum chargeCondition = eErrChargeCondition_Allow;

    if (port >= SYSCFG_CFG_GUN_NUM)
    {
        reason = 0x03;
        stopReason = eIotAPStartFail_DataInvalid;          /* 200: 数据合法性校验失败 */
    }
    else if (AswMonitor_IsOrderIdle(port) != TRUE)
    {
        reason = 0x01;
        stopReason = eIotAPStartFail_PauseService;         /* 204: 充电桩暂停服务(订单占用) */
    }
    else if (AswChargeIf_GetAuthFlag(port) == TRUE)
    {
        reason = 0x01;
        stopReason = eIotAPStartFail_WorkingState;         /* 205: 充电桩工作状态(已鉴权) */
    }
    else if ((chargeCondition = AswErrHandle_GetChargeCondition(port)) != eErrChargeCondition_Allow)
    {
        reason = 0x02;
        stopReason = eIotAPStartFail_PauseState;           /* 206: 充电桩暂停状态 */
        IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动检查失败, 充电条件不允许, chargeCondition=%d\r\n",
                            port, chargeCondition);
    }
    else if (AswChargeIf_CheckGunConnected(port) != TRUE)
    {
        IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动检查失败, 枪未连接\r\n", port);
        reason = 0x03;
        stopReason = eIotAPStartFail_NotConnected;         /* 207: 车辆未连接 */
    }
    else if (AswMonitor_CheckBillModeValid(port) != TRUE)
    {
        reason = 0x02;
        stopReason = eIotAPStartFail_ParamNotReady;        /* 203: 充电参数未完成 */
        IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动检查失败, 计费模型无效\r\n", port);
    }
    else if (SSUcm_IsUpdating() == TRUE)
    {
        IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动检查失败, 升级中\r\n", port);
        reason = 0x03;
        stopReason = eIotAPStartFail_UpDating;             /* 234: 无空余充电机模块可用 */
    }
    else if (AswMonitor_CheckForbidState() == TRUE)
    {
        IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动检查失败, 禁充状态\r\n", port);
        reason = 0x03;
        stopReason = eIotAPStartFail_UpDating;             /* 235: 充电桩其它故障 */
    }
    else
    {
        stopReason = eIotAPStopReason_Null;                /* 启动成功时无停止原因 */
    }

    if (pFailReason != NULL)
    {
        pFailReason[0] = reason;
    }

    if (pStopReason != NULL)
    {
        pStopReason[0] = stopReason;
    }

    return (reason == 0x00) ? TRUE : FALSE;
}

/**
 * @brief  根据B4报文数据填充充电控制参数
 *
 * 将平台下发的启动方式及控制数值转换为内部充电控制结构体字段，
 * 包括用户编号、账户余额上限以及按电量/时间/金额/自动充电的
 * 控制类型与控制值设定。
 *
 * @param port      充电枪端口号，有效范围 [0, SYSCFG_CFG_GUN_NUM)
 * @param startWay  启动方式，取值为 IOT_AP_B4_START_WAY_ENERGY/TIME/MONEY 等
 * @param ctrlData  控制数值（电量kWh、时间min、金额元，由startWay决定语义）
 * @param pUserNo   用户编号指针，8字节ASCII，可为NULL表示无用户编号
 */
static void IotAP_FillChargeCtrlFromB4(uint8_t port, uint8_t startWay, uint32_t ctrlData, const uint8_t *pUserNo)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;

    if (port < SYSCFG_CFG_GUN_NUM)
    {
        pChargeCtrl = AswMonitor_GetChargeCtrlPtr(port);

        if (pChargeCtrl != NULL)
        {
            memset(pChargeCtrl->authCardID, 0x00, sizeof(pChargeCtrl->authCardID));

            if (pUserNo != NULL)
            {
                memcpy(pChargeCtrl->authCardID, pUserNo, 8);
            }

            pChargeCtrl->accountMoney = IOT_AP_B4_ACCOUNT_MONEY_MAX;

            if (startWay == IOT_AP_B4_START_WAY_ENERGY)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeEnergy;
                pChargeCtrl->chargeCtrlVal = ctrlData * 100;
            }
            else if (startWay == IOT_AP_B4_START_WAY_TIME)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeTime;
                pChargeCtrl->chargeCtrlVal = ctrlData * 60;
            }
            else if (startWay == IOT_AP_B4_START_WAY_MONEY)
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_JudgeMoney;
                pChargeCtrl->chargeCtrlVal = ctrlData * 100;
            }
            else
            {
                pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                pChargeCtrl->chargeCtrlVal = 0;
            }
        }
    }
}

/**
 * @brief  校验B4停止命令是否合法（订单号匹配且枪处于充电中）
 * @note   Step1: 校验上下文/枪号/订单号指针有效性，无效则返回失败原因0x03
 *         Step2: 校验枪是否为空闲态，空闲则返回失败原因0x03
 *         Step3: 比对本地保存的订单号与接收到的订单号是否一致，不一致返回失败原因0x03
 *         Step4: 全部通过则reason=0x00，输出失败原因并返回TRUE
 * @param[in]  port        枪号
 * @param[in]  pOrderNo    待校验的订单号（16字节）
 * @param[out] pFailReason 失败原因输出（1字节）
 * @retval   TRUE       校验通过，允许执行停止
 * @retval   FALSE      校验失败（参数非法/枪空闲/订单号不匹配）
 */
static uint8_t IotAP_CheckChargeStop(uint8_t port, const uint8_t *pOrderNo, uint8_t *pFailReason)
{
    uint8_t reason = 0x03;                          /* 默认失败 */

    do
    {
        if ((port >= SYSCFG_CFG_GUN_NUM) || (pOrderNo == NULL))
        {
            break;
        }
        if (AswMonitor_IsOrderIdle(port) == TRUE)
        {
            break;
        }
        if (memcmp(pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum,
                   pOrderNo,
                   sizeof(pIotAPCtx->stProtoData[port].curUsedOrderTransactionNum)) != 0)
        {
            break;
        }
        reason = 0x00;                              /* 正常情况 */
    } while (0);

    if (pFailReason != NULL)
    {
        pFailReason[0] = reason;
    }

    return (reason == 0x00) ? TRUE : FALSE;         /* 唯一出口 */
}

/*
 * AP帧有效性检查（单0x68格式）
 * 校验规则：
 *   ① 最小长度 >= 3 (0x68 + len >= 1字节)
 *   ② pBuf[0] == 0x68
 *   ③ dataLen >= pBuf[1] + 2 (len域 = 从控制域到数据域末尾的字节数)
 */
/**
 * @brief 在接收缓冲区中查找有效的 AP 协议帧头
 *
 * @param[in]  pData    接收数据缓冲区指针
 * @param[in]  dataLen  接收数据长度
 * @param[out] dealLen  输出已处理的数据长度（即跳过的无效字节数 + 完整帧长度，或仅跳过的无效字节数）
 *
 * @return 指向有效帧头的指针，若未找到则返回 NULL
 *
 * @note 该函数通过滑动窗口方式查找起始符 IOT_AP_HEAD (0x68)，
 *       并校验后续长度字段是否满足最小帧长要求。
 *       若找到完整帧，dealLen 更新为帧结束位置相对于 pData 的偏移量；
 *       若数据不足，dealLen 更新为当前已扫描到的位置，以便上层保留剩余数据等待拼接。
 */
static IotAPInfoFrameHead_Struct* IotAP_FindValidFrame(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint16_t remainLen = dataLen;
    IotAPInfoFrameHead_Struct *pHead = NULL;
    uint16_t frameTotalLen = 0;

    if ((pData != NULL) && (dataLen != 0) && (dealLen != NULL))
    {
        /* 初始化输出参数，防止未命中时包含垃圾值 */
        *dealLen = 0;

        while (remainLen >= 6) /*最短报文帧长度(U帧6字节)*/
        {
            /* 1. 查找帧头起始符 0x68 */
            if (pStart[0] != IOT_AP_HEAD)
            {
                pStart++;
                remainLen--;
                (*dealLen)++;
                continue;
            }

            /* 2. 根据帧头中的长度域计算整帧长度 */
            frameTotalLen = IotAP_GetFrameTotalLen(pStart);

            /* 3. 校验剩余数据是否足够容纳整帧 */
            if (remainLen >= frameTotalLen)
            {
                /* 找到完整帧：dealLen 记录从缓冲区起始到当前帧结束的总字节数 */
                *dealLen = (uint16_t)((uint32_t)pStart - (uint32_t)pData) + frameTotalLen;
                pHead = (IotAPInfoFrameHead_Struct *)pStart;
                break;
            }
            else
            {
                /* 数据不完整：break 退出循环，上层可根据 dealLen 判断已扫描过的无效字节进行缓存等待 */
                /* 此处不增加 dealLen，保持为已跳过的无效前缀长度，或者根据需求调整为当前 pStart 偏移 */
                /* 通常策略：如果头部正确但数据不足，应保留这部分数据等待后续包拼接，因此不消耗这部分数据 */
                break;
            }
        }
    }

    return pHead; 
}

/*
 * 从有效帧头中识别CMD号
 *   - F短帧(基础链路): 总帧长 <= sizeof(IotAPInfoFrameHead_Struct)，cmd取第3字节(pBuf[2])
 *   - B信息帧(业务数据): 总帧长 > sizeof(IotAPInfoFrameHead_Struct), cmd取recordKind字段
 */
static uint16_t IotAP_IdentifyCmd(IotAPInfoFrameHead_Struct *pHead, uint16_t frameTotalLen)
{
    uint16_t cmd = IOT_AP_CMD_NULL;

    if (pHead == NULL)
    {
        return IOT_AP_CMD_NULL;
    }

    if ((pHead->len == 0x01) && (frameTotalLen == 12))
    {
        cmd = IOT_AP_CMD_LOGIN_RSP;
    }
    else if ((pHead->len == 0x04) && (frameTotalLen == 6) && (pHead->control[0] == 0x0B))
    {
        cmd = IOT_AP_CMD_UFRAME_ACK;
    }
    else if ((pHead->len == 0x04) && (frameTotalLen == 6) && (pHead->control[0] == 0x83))
    {
        cmd = IOT_AP_CMD_HEARTBEAT_RSP;
    }
    else if (frameTotalLen < sizeof(IotAPInfoFrameHead_Struct))
    {
        cmd = IOT_AP_CMD_NULL;
    }
    else
    {
        if (pHead->typeId == IOT_AP_TYPE_CLOCK_SYNC)
        {
            cmd = IOT_AP_CMD_SYNC_TIME_REQ;
        }
        else if (pHead->typeId == IOT_AP_TYPE_UP_DATA)
        {
            if (pHead->recordKind == 2)
            {
                cmd = IOT_AP_CMD_B13_ONLINE_ORDER_DOWN;
            }
            else if (pHead->recordKind == 3)
            {
                cmd = IOT_AP_CMD_B16_OFFLINE_ORDER_DOWN;
            }
        }
        else if (pHead->typeId == IOT_AP_TYPE_DOWN_DATA)
        {
            switch (pHead->recordKind)
            {
                case 2:
                    cmd = IOT_AP_CMD_B07_CARD_AUTH_DOWN;
                    break;
                case 3:
                    cmd = IOT_AP_CMD_B14_DEDUCT_CONFIRM;
                    break;
                case 5:
                    cmd = IOT_AP_CMD_B02_BILLMODEL_DOWN;
                    break;
                case 12:
                    cmd = IOT_AP_CMD_B11_START_NOTIFY_DOWN;
                    break;
                case 15:
                    cmd = IOT_AP_CMD_B23_UPGRADE_START;
                    break;
                case 21:
                    cmd = IOT_AP_CMD_B04_CHG_CTRL_DOWN;
                    break;
                case 57:
                    cmd = IOT_AP_CMD_B32_TERMINAL_REQ_DOWN;
                    break;
                case 58:
                    cmd = IOT_AP_CMD_B33_POWER_CTRL_DOWN;
                    break;
                case 59:
                    cmd = IOT_AP_CMD_B35_BILLMODEL_POLL_DOWN;
                    break;
                case 60:
                    cmd = IOT_AP_CMD_B39_FTP_ADDR_DOWN;
                    break;
                case 63:
                    cmd = IOT_AP_CMD_B45_POWER_POLL_DOWN;
                    break;
                case 64:
                    cmd = IOT_AP_CMD_B47_TIMEBILL_DOWN;
                    break;
                case 65:
                    cmd = IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN;
                    break;
                case 66:
                    cmd = IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN;
                    break;
                case 67:
                    cmd = IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN;
                    break;
                case 68:
                    cmd = IOT_AP_CMD_B56_OFFLINE_DETAIL_DOWN;
                    break;
                default:
                    break;
            }
        }
    }

    return cmd;
}

/*
 * 数据解码回调入口，供 FrameQueue_ProcessRxData 调用
 * 处理流程：
 *   1. 找到有效帧头
 *   2. 根据F/B帧格式识别CMD
 *   3. 查找接收控制表获取解析入口
 *   4. 调用 pRecvParse 解析数据
 *   5. 解析成功后处理响应标志和匹配上行发送使能
 */
/**
 * @brief 解析并处理接收到的IoT AP协议数据帧
 *
 * 从原始数据流中查找有效帧，识别命令类型后调用对应的解析回调函数，
 * 并根据帧类型（响应帧/请求帧）执行不同的后续处理：
 * - 响应帧：清除对应上行帧的超时计时器和重试计数
 * - 下行请求帧：触发匹配的上行响应帧发送使能，并清除已发送标记
 *
 * @param[in]  pData    接收到的原始数据缓冲区指针
 * @param[in]  dataLen  原始数据长度（字节）
 * @param[in]  topicLen 主题长度（当前协议未使用）
 * @param[in]  pTopic   主题指针（当前协议未使用）
 * @param[out] dealLen  已处理的数据长度，无有效帧时置0
 */
static void IotAP_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotAPRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotAPInfoFrameHead_Struct *pFrameHead = NULL;
    static uint32_t unsupportedLogTick = 0;
    uint8_t port = 0;
    uint16_t recvCmd = 0;
    uint16_t frameTotalLen = 0;
    uint16_t logLen = 0;
    uint8_t parseResult = 0;
    uint8_t *pDataStart = NULL;
    uint16_t dataPartLen = 0;

    pFrameHead = IotAP_FindValidFrame(pData, dataLen, dealLen);

    if (pFrameHead != NULL)
    {
        frameTotalLen = IotAP_GetFrameTotalLen((uint8_t *)pFrameHead);
        recvCmd = IotAP_IdentifyCmd(pFrameHead, frameTotalLen);
        logLen = IotAP_GetRecvLogLen(recvCmd, frameTotalLen);
        pCmdRecvCtrl = IotAP_GetRecvCtrlPtr(recvCmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                /* 固定F帧/时钟同步F帧解析完整帧，B信息帧解析数据域 */
                pDataStart = (uint8_t *)pFrameHead;
                dataPartLen = frameTotalLen;

                if ((recvCmd < IOT_AP_CMD_LOGIN_REQ) &&
                    (frameTotalLen >= sizeof(IotAPInfoFrameHead_Struct)))
                {
                    pDataStart = (uint8_t *)pFrameHead + sizeof(IotAPInfoFrameHead_Struct);
                    dataPartLen = frameTotalLen - sizeof(IotAPInfoFrameHead_Struct);
                }

                parseResult = pCmdRecvCtrl->pRecvParse(&port, pDataStart, dataPartLen);

                if (parseResult != 0)
                {
                    /* 解析成功，打印日志 */
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTAP_CFG_DebugPrint("AP,[枪：%d]接收[cmd: 0x%02X, %s][%d]: \r\n", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, logLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, logLen);
                    }

                    /* 响应帧: 清除超时计时和重试计数 */
                    if (pCmdRecvCtrl->cmdType == IOT_AP_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        /* 下行请求帧: 触发匹配的上行响应发送使能 */
                        if (pCmdRecvCtrl->matchCmd != IOT_AP_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, pFrameHead->control[0]);
                            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    /* 清除匹配的上行帧的已发送标记（允许重新组帧） */
                    if (pCmdRecvCtrl->matchCmd != IOT_AP_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTAP_CFG_DebugPrint("AP,[枪：%d]接收[cmd: 0x%02X, %s][%d] 处理失败!\r\n", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, logLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, logLen);
                    }
                }
            }
        }
        else if (recvCmd != IOT_AP_CMD_NULL)
        {
            if (Common_JudgeTimeoutMs(unsupportedLogTick, 1000) == TRUE)
            {
                unsupportedLogTick = Common_GetSystick();
                IOTAP_CFG_InfoPrint("AP,接收到暂不支持帧[cmd:0x%02X][%d]，已忽略\r\n",
                                    recvCmd,
                                    logLen);
                DSLogM_HexOutput((uint8_t *)pFrameHead, logLen);
            }
        }
        else
        {
        }
    }
}

/* ====== 对外接收入口 ====== */

/**
 * @brief  上行接收解析总入口，从帧队列取出原始数据并分发到各解析函数
 * @note   调用 FrameQueue_ProcessRxData 遍历队列，对每帧数据回调 IotAP_DecodeData 进行 CMD 识别与解析
 * @param[in]  无
 * @retval  无
 */
void IotAP_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotAPCtx->frameQueueChannelID, IotAP_DecodeData);
}

/* ====== F帧接收解析函数 ====== */

/**
 * @brief  解析AP登录应答报文
 * @note   校验帧头、命令标识及桩号匹配，登录成功后清除平台离线故障并置位登录标志
 * @param[in]  port    枪号输出指针，登录成功时写0；可为NULL
 * @param[in]  r_data  接收到的原始报文数据指针
 * @param[in]  len     报文数据长度
 * @return TRUE-登录应答校验通过并处理成功, FALSE-参数无效或校验失败
 */
static uint8_t IotAP_ParseLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t gunNo = 0;
    uint8_t ret = FALSE;

    if (port != NULL)
    {
        port[0] = 0;
    }

    if ((r_data == NULL) || (len < 12))
    { 
        /* 参数无效 → ret保持FALSE */ 
    }
    else if ((r_data[0] != IOT_AP_HEAD) || (r_data[1] != 0x01) || (IotAP_CheckPileDn(r_data, 2) != TRUE))
    {
        IOTAP_CFG_InfoPrint("AP登录应答校验失败\r\n");
        /* 帧校验失败 → ret保持FALSE */
    }
    else
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
        pIotAPCtx->loginSucc = TRUE;
        ret = TRUE;
    }

    return ret;
}

/**
 * @brief  解析U帧确认报文
 * @param[in]   port    端口号指针，函数将其清零；可为NULL
 * @param[in]   r_data  接收到的原始报文数据
 * @param[in]   len     报文数据长度
 * @return  TRUE 解析成功（报文为合法的U帧确认帧），FALSE 报文非法或参数无效
 */
static uint8_t IotAP_ParseUFrameAck(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;

    if (port != NULL)
    {
        port[0] = 0;
    }

    if ((r_data == NULL) || (len < 6))
    { 
        /* 参数无效 → ret保持FALSE */ 
    }
    else if ((r_data[0] != IOT_AP_HEAD) || (r_data[1] != 0x04) || (r_data[2] != 0x0B))
    { 
        /* 帧头不匹配 → ret保持FALSE */ 
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}

/**
 * @brief  解析平台心跳响应报文
 *
 * 校验心跳响应帧头（起始符、长度、类型标识），通过后清除平台离线故障，
 * 并使能所有枪的B01实时数据上报。
 *
 * @param[in]  port   端口号指针，非空时置0
 * @param[in]  r_data 接收数据缓冲区指针
 * @param[in]  len    接收数据长度
 * @return     TRUE  解析成功
 * @return     FALSE 参数无效或帧头校验失败
 */
static uint8_t IotAP_ParseHeartbeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = TRUE;
    uint8_t i = 0;

    if (port != NULL)
    {
        port[0] = 0;
    }

    if ((r_data == NULL) || (len < 6))
    {
        ret = FALSE;
    }
    else if ((r_data[0] != IOT_AP_HEAD) || (r_data[1] != 0x04) || (r_data[2] != 0x83))
    {
        ret = FALSE;
    }
    else
    {
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);

        for (i = 0; i < SYSCFG_CFG_GUN_NUM; i++)
        {
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, i, IOT_AP_CMD_B01_REALTIME_DATA, TRUE);  // 实时数据上报使能
        }
    }

    return ret;
}

/**
 * @brief  解析平台下发的时钟同步请求，提取CP56Time2a时间并同步本地系统时钟
 * @param  port     输出参数，写入端口标识（若非NULL则置0）
 * @param  r_data   接收到的原始报文数据指针
 * @param  len      报文数据长度（字节）
 * @return uint8_t  解析成功返回TRUE，参数无效或长度不足返回FALSE
 * @note   解析成功后会自动触发时钟同步应答（IOT_AP_CMD_SYNC_TIME_RSP）
 */
static uint8_t IotAP_ParseSyncTimeReq(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    CommonDateTime_Struct dateTime = {0};
    uint8_t index = 0;
    uint8_t ret = FALSE;                          /* 默认失败 */

    if (port != NULL)
    {
        port[0] = 0;
    }

    if ((r_data != NULL) && (len >= (IOT_AP_CLOCK_SYNC_HEAD_LEN + sizeof(pIotAPCtx->syncTimeCp56))))
    {
        memcpy(pIotAPCtx->syncTimeCp56,&r_data[IOT_AP_CLOCK_SYNC_HEAD_LEN],sizeof(pIotAPCtx->syncTimeCp56));
        SSTM_SynTimeBySecTimeStamp(Common_Cp56Time2aToTimestamp(pIotAPCtx->syncTimeCp56));

        for (index = 0; index < SYSCFG_CFG_GUN_NUM; index++)
        {
            if (AswMonitor_IsOrderIdle(index) == TRUE)
            {
                IotAP_RefreshNowbillModel(index);
            }
        }

        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_SYNC_TIME_RSP, TRUE);
        ret = TRUE;
    }

    return ret;
}

/* ====== B帧接收解析函数 - 控制 ====== */

/**
 * @brief  AP B4: 充电启停控制命令下发下行数据，解析并执行平台远程启停充电指令
 * @note   第一阶段（字段解析）：提取枪号→控制命令→启动条件→启动方式→控制数据→用户编号
 *         第二阶段（启动命令处理）：条件=立即+方式合法+启动检查通过→填充控制参数→保存订单号→启动充电成功；否则标记失败
 *         第三阶段（停止命令处理）：调用IotAP_CheckChargeStop校验订单号→校验通过→执行停止充电；否则标记失败
 *         第四阶段（非法命令）：未知命令类型→标记失败
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败（参数非法/校验不通过）
 */
static uint8_t IotAP_ParseChgCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    uint8_t ctrlCmd = 0;
    uint8_t startCondition = 0;
    uint8_t startWay = 0;
    uint8_t failReason = 0x03;
    uint32_t ctrlData = 0;
    IotAPStopReason_Enum startFailStopReason = eIotAPStopReason_OtherErr;
    MSNvmOrderInfo_Struct stFailOrderInfo;
    IotAPProtoData_Struct *pProtoData = NULL;
    const MSNvmAPParamBillMode_Struct *pAPBillMode = NULL;
    MSNvmAPOrderInfo_Struct *pAPOrder = NULL;
    uint8_t idx = 0;
    uint8_t periodCount = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B4_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B4_PORT_OFFSET];
        ctrlCmd = r_data[IOT_AP_B4_CTRL_CMD_OFFSET];
        startCondition = r_data[IOT_AP_B4_START_CONDITION_OFFSET];
        startWay = r_data[IOT_AP_B4_START_WAY_OFFSET];
        ctrlData = Common_FourUint8ToUint32(&r_data[IOT_AP_B4_CTRL_DATA_OFFSET]);
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            pProtoData = &pIotAPCtx->stProtoData[recvPort];
            pProtoData->remoteCtrlCmd = ctrlCmd;
            memcpy(pProtoData->newRecvOrderTransactionNum, &r_data[IOT_AP_B4_ORDER_NO_OFFSET],
                   sizeof(pProtoData->newRecvOrderTransactionNum));

            if (ctrlCmd == IOT_AP_B4_CTRL_START)
            {
                if ((startCondition == IOT_AP_B4_START_CONDITION_NOW) &&
                    (startWay >= IOT_AP_B4_START_WAY_ENERGY) &&
                    (startWay <= IOT_AP_B4_START_WAY_FULL) &&
                    (IotAP_CheckChargeStart(recvPort, &failReason, &startFailStopReason) == TRUE))
                {
                    IotAP_FillChargeCtrlFromB4(recvPort, startWay, ctrlData, &r_data[IOT_AP_B4_USER_NO_OFFSET]);
                    memcpy(pProtoData->curUsedOrderTransactionNum,
                           pProtoData->newRecvOrderTransactionNum,
                           sizeof(pProtoData->curUsedOrderTransactionNum));
                    pProtoData->remoteStartResult = 0;
                    pProtoData->remoteStartFailReason = 0;
                    AswMonitor_ChargeStart(recvPort, ASWMONITOR_ORDER_START_SRC_APP, TRUE);
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动命令已接受, startWay=%d, ctrlData=%d\r\n",
                                        recvPort, startWay, ctrlData);
                }
                else
                {
                    pProtoData->remoteStartResult = 1;
                    pProtoData->remoteStartFailReason = failReason;

                    /* ===== 构建零费用交易记录供B53上报 ===== */
                    memset(&stFailOrderInfo, 0x00, sizeof(stFailOrderInfo));
                    stFailOrderInfo.port = recvPort;                                    /* 外层: 枪号 */
                    stFailOrderInfo.platOrderInfo.stAPOrderInfo.port = recvPort;       /* 内层: B53报文的充电接口标识 */

                    /* 从AP层B47三缓冲计费模型填充时段元数据(序号/类型),电量电费服务费保持零 */
                    {
                        pAPOrder = &stFailOrderInfo.platOrderInfo.stAPOrderInfo;
                        pAPBillMode = IotAP_GetActiveBillMode(recvPort);
                        if (pAPBillMode != NULL)
                        {
                            if ((pAPBillMode->workState[0] == 1) && (pAPBillMode->workState[1] == 0))
                            {
                                periodCount = pAPBillMode->periodCount;
                                if (periodCount > MSNVM_AP_BILLMODE_PERIOD_COUNT)
                                {
                                    periodCount = MSNVM_AP_BILLMODE_PERIOD_COUNT;
                                }
                                pAPOrder->periodCount = periodCount;

                                for (idx = 0; idx < periodCount; idx++)
                                {
                                    pAPOrder->periodInfo[idx].timeSerialNumber = idx + 1;
                                    pAPOrder->periodInfo[idx].timeKind =
                                        pAPBillMode->period[idx].periodRate + 1;
                                    /* chargeEnergy/chargeElecFee/chargeServeFee 保持memset的0 */
                                }

                                IOTAP_CFG_InfoPrint("AP,[枪:%d]零费订单-时段已填充, periodCount=%d\r\n",
                                                    recvPort, periodCount);
                            }
                            else
                            {
                                IOTAP_CFG_InfoPrint("AP,[枪:%d]零费订单-计费模型不可用: activeIdx=%d, workState=%02X%02X\r\n",
                                                    recvPort, pIotAPCtx->billActiveIndex[recvPort],
                                                    pAPBillMode->workState[0], pAPBillMode->workState[1]);
                            }
                        }
                        else
                        {
                            IOTAP_CFG_InfoPrint(
                                "AP,[枪:%d]零费订单-无活跃计费模型: activeIdx=%d(0xFF=无)\r\n",
                                recvPort, pIotAPCtx->billActiveIndex[recvPort]);
                        }
                        /* else: 无有效计费模型 → periodCount=0, 不输出时段明细 */
                    }

                    stFailOrderInfo.protocolType = eAswPlatCardType_AP;
                    stFailOrderInfo.orderSaveState = ASWMONITOR_ORDER_SAVE_STOP;
                    stFailOrderInfo.orderLen = sizeof(MSNvmAPOrderInfo_Struct);

                    /* 复制平台下发的订单号 */
                    memcpy(&stFailOrderInfo.platOrderInfo.stAPOrderInfo.orderTransactionNum,
                           pProtoData->newRecvOrderTransactionNum,
                           sizeof(pProtoData->newRecvOrderTransactionNum));

                    /* 时间: 开始/结束均为当前时刻 */
                    Common_TimestampToCp56Time2a(SSTM_GetSecTimestamp(),
                        &stFailOrderInfo.platOrderInfo.stAPOrderInfo.startTime[0]);
                    memcpy(&stFailOrderInfo.platOrderInfo.stAPOrderInfo.stopTime,
                           &stFailOrderInfo.platOrderInfo.stAPOrderInfo.startTime, 7);

                    /* 停止原因: 使用CheckChargeStart输出的细粒度枚举值(200/203~207/309/310) */
                    IotAP_StopReasonToBcd(
                        &stFailOrderInfo.platOrderInfo.stAPOrderInfo.stopReason[0],
                        startFailStopReason
                    );

                    IOTAP_CFG_InfoPrint("AP,[零费订单保存前]periodCount=%d, orderLen=%d, structSize=%d\r\n",
                        stFailOrderInfo.platOrderInfo.stAPOrderInfo.periodCount,
                        stFailOrderInfo.orderLen,
                        (int)sizeof(MSNvmOrderInfo_Struct));

                    /* 零费订单不保存NVM，直接填入上下文触发立即B53上报 */
                    memcpy(&pIotAPCtx->stOrderInfo, &stFailOrderInfo,
                           sizeof(MSNvmOrderInfo_Struct));
                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort,
                                         IOT_AP_CMD_B53_ONLINE_DETAIL_UP, TRUE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort,
                                           IOT_AP_CMD_B53_ONLINE_DETAIL_UP, TRUE);

                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B4启动命令拒绝, reason=%d, B53停止原因=%d, 零费订单直接触发B53, periodCount=%d\r\n",
                                        recvPort, failReason, startFailStopReason,
                                        stFailOrderInfo.platOrderInfo.stAPOrderInfo.periodCount);
                }

                ret = TRUE;
            }
            else if (ctrlCmd == IOT_AP_B4_CTRL_STOP)
            {
                if (IotAP_CheckChargeStop(recvPort, &r_data[IOT_AP_B4_ORDER_NO_OFFSET], &failReason) == TRUE)
                {
                    pProtoData->remoteStopResult = 0;
                    pProtoData->remoteStopFailReason = 0;
                    AswErrhandle_SetErrExsitCallback(recvPort, eSrc_AppStop);
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B4停止命令已接受\r\n", recvPort);
                }
                else
                {
                    pProtoData->remoteStopResult = 1;
                    pProtoData->remoteStopFailReason = failReason;
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B4停止命令拒绝, reason=%d\r\n",
                                        recvPort, failReason);
                }

                ret = TRUE;
            }
            else
            {
                pProtoData->remoteStartResult = 1;
                pProtoData->remoteStartFailReason = 0x03;
                ret = TRUE;
            }
        }
    }

    return ret;
}

/* ====== B帧接收解析函数 - 鉴权 ====== */

/**
 * @brief  AP B7: 刷卡鉴权下行，解析平台对刷卡认证请求的审批结果
 * @note   Step1: 入参合法性及最小长度校验，提取枪号
 *         Step2: 桩号BCD校验，获取充电控制器和协议上下文指针
 *         Step3: 提取鉴权结果/失败原因/账户余额/VIN码，停止B06发送
 *         Step4: 鉴权通过+余额非零+卡号匹配+启动检查通过→设置自动充电→启动B10启动通知；否则关闭B10发送
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败
 */
static uint8_t IotAP_ParseCardAuthDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    uint8_t failReason = 0x03;
    IotAPStopReason_Enum startFailStopReason = eIotAPStopReason_OtherErr;
    uint8_t reversedCardNo[IOT_AP_CARD_NO_LEN];
    uint8_t i = 0;
    uint8_t cardAuthPass = FALSE;

    if ((r_data != NULL) && (len >= IOT_AP_B7_MIN_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B7_PORT_OFFSET];
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            pChargeCtrl = AswMonitor_GetChargeCtrlPtr(recvPort);
            pProtoData = &pIotAPCtx->stProtoData[recvPort];

            if (pChargeCtrl != NULL)
            {
                pProtoData->cardAuthResult = r_data[IOT_AP_B7_AUTH_RESULT_OFFSET];
                memcpy(pProtoData->cardAuthFailReason, &r_data[IOT_AP_B7_FAIL_REASON_OFFSET],
                       sizeof(pProtoData->cardAuthFailReason));
                pProtoData->cardAccountBalance = Common_FourUint8ToUint32(&r_data[IOT_AP_B7_ACCOUNT_BALANCE_OFFSET]);
                memcpy(pProtoData->cardVin, &r_data[IOT_AP_B7_VIN_OFFSET], sizeof(pProtoData->cardVin));

                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);
                Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);
                Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B06_CARD_AUTH_UP, FALSE);

                /* B7卡号为逆向BCD序（B06发送时已做reverse），需翻转后再与本地authCardID比对 */
                for (i = 0; i < IOT_AP_CARD_NO_LEN; i++)
                {
                    reversedCardNo[i] = r_data[IOT_AP_B7_CARD_NO_OFFSET + IOT_AP_CARD_NO_LEN - 1u - i];
                }

                cardAuthPass = FALSE;

                if (pProtoData->cardAuthResult != 1)
                {
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B7刷卡鉴权失败(平台拒绝),platReason=[%02X,%02X]\r\n",
                                        recvPort,
                                        pProtoData->cardAuthFailReason[0],
                                        pProtoData->cardAuthFailReason[1]);
                }
                else if (pProtoData->cardAccountBalance == 0)
                {
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B7刷卡鉴权失败(余额为0)\r\n", recvPort);
                }
                else if (memcmp(pChargeCtrl->authCardID, reversedCardNo, IOT_AP_CARD_NO_LEN) != 0)
                {
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B7刷卡鉴权失败(卡号不匹配),balance=%d\r\n",
                                        recvPort, pProtoData->cardAccountBalance);
                }
                else if (IotAP_CheckChargeStart(recvPort, &failReason, &startFailStopReason) != TRUE)
                {
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B7刷卡鉴权失败(启动检查不通过),balance=%d,localReason=%d\r\n",
                                        recvPort, pProtoData->cardAccountBalance, failReason);
                }
                else
                {
                    cardAuthPass = TRUE;
                    pChargeCtrl->accountMoney = pProtoData->cardAccountBalance;
                    pChargeCtrl->eChargeCtrlType = eAswMonitorChargeCtrlType_AutoCharge;
                    pChargeCtrl->chargeCtrlVal = 0;
                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, TRUE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, TRUE);
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B7刷卡鉴权成功, 已启动并等待B10上报, balance=%d\r\n",
                                        recvPort, pProtoData->cardAccountBalance);
                }

                if (cardAuthPass == FALSE)
                {
                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
                    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
                    Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B11_START_NOTIFY_DOWN, FALSE);
                    Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B11_START_NOTIFY_DOWN);
                }

                ret = TRUE;
            }
        }
    }

    return ret;
}

/**
 * @brief  AP B11: 启动通知下行，解析平台对启动认证请求的审批结果
 * @note   Step1: 入参合法性及最小长度校验，提取枪号
 *         Step2: 校验桩号BCD合法性
 *         Step3: 提取结果和失败原因，停止B10启动通知发送
 *         Step4: 结果=1(成功)且卡鉴权通过→保存平台订单号，触发启动充电；否则设置停止充电错误
 * @param[out] port      解析出的枪号
 * @param[in]  r_data    接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败（参数非法/校验不通过/长度不足）
 */
static uint8_t IotAP_ParseStartNotifyDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    AswMonitorChargeCtrl_Struct *pChargeCtrl = NULL;
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B11_MIN_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B11_PORT_OFFSET];
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            pChargeCtrl = AswMonitor_GetChargeCtrlPtr(recvPort);
            pProtoData = &pIotAPCtx->stProtoData[recvPort];

            if (pChargeCtrl != NULL)
            {
                pProtoData->startNotifyResult = r_data[IOT_AP_B11_RESULT_OFFSET];
                memcpy(pProtoData->startNotifyFailReason, &r_data[IOT_AP_B11_FAIL_REASON_OFFSET],
                       sizeof(pProtoData->startNotifyFailReason));

                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
                Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);
                Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B10_START_NOTIFY_UP, FALSE);

                if ((pProtoData->startNotifyResult == 1) &&
                    (pProtoData->cardAuthResult == 1))
                {
                    memcpy(pProtoData->curUsedOrderTransactionNum, &r_data[IOT_AP_B11_ORDER_NO_OFFSET],
                           sizeof(pProtoData->curUsedOrderTransactionNum));
                    AswMonitor_ChargeStart(recvPort, ASWMONITOR_ORDER_START_SRC_CARD, TRUE); /*后面确定启动充电是放在B7还是B11*/
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B11启动通知成功, 平台订单号已保存\r\n", recvPort);
                }
                else
                {
                    if (AswMonitor_IsOrderIdle(recvPort) != TRUE)
                    {
                        AswErrhandle_SetErrExsitCallback(recvPort, eSrc_AppStop);
                    }

                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B11启动通知失败,result=%d,reason=[%02X,%02X]\r\n",
                                        recvPort, pProtoData->startNotifyResult,
                                        pProtoData->startNotifyFailReason[0], pProtoData->startNotifyFailReason[1]);
                }

                ret = TRUE;
            }
        }
    }

    return ret;
}

/* ====== B帧接收解析函数 - 交易 ====== */

/**
 * @brief  AP B54: 在线分时明细交易包下行数据，解析平台对交易记录的确认结果
 * @note   Step1: 入参合法性及最小长度校验，提取枪号和结果字段
 *         Step2: 校验桩号BCD合法性及订单号与本地下发的是否匹配
 *         Step3: 匹配→保存确认结果；成功时停止B53/B54收发控制并标记订单上报成功；不匹配→日志告警
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败（参数非法/校验不通过/订单号不匹配）
 */
static uint8_t IotAP_ParseOnlineDetailDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmAPOrderInfo_Struct *pOrderInfo = NULL;
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    uint8_t result = 1;

    if ((r_data != NULL) && (len >= IOT_AP_B54_ORDER_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B54_PORT_OFFSET];
        result = r_data[IOT_AP_B54_RESULT_OFFSET];
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            pOrderInfo = &pIotAPCtx->stOrderInfo.platOrderInfo.stAPOrderInfo;

            if (memcmp(pOrderInfo->orderTransactionNum,
                       &r_data[IOT_AP_B54_ORDER_NO_OFFSET],
                       sizeof(pOrderInfo->orderTransactionNum)) == 0)
            {
                pIotAPCtx->stProtoData[recvPort].onlineDetailResult = result;

                if (result == 0)
                {
                    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotAPCtx->time);
                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                    Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN, FALSE);
                    Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN);
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B53在线分时交易记录上报成功\r\n", recvPort);
                }
                else
                {
                    IOTAP_CFG_InfoPrint("AP,[枪:%d]B54在线分时交易记录确认失败,result=%d\r\n", recvPort, result);
                }

                ret = TRUE;
            }
            else
            {
                IOTAP_CFG_InfoPrint("AP,[枪:%d]B54订单号不匹配\r\n", recvPort);
            }
        }
    }

    return ret;
}

/**
 * @brief  AP B14: 充电扣款后下行数据，解析平台对充电扣款的确认通知
 * @note   Step1: 入参合法性及最小长度校验
 *         Step2: 桩号BCD校验，提取扣款结果和失败原因并打印日志
 *         Step3: 无论扣款成功或失败，若本枪正处于B53/B54上报流程则标记上报成功并停止收发控制
 * @param[out] port      解析出的枪号（固定为0）
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败（参数非法/校验不通过）
 */
static uint8_t IotAP_ParseDeductConfirmDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    uint8_t deductResult = 0;
    uint16_t failReason = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B14_MIN_DATA_LEN))
    {
        recvPort = 0;
        port[0] = recvPort;

        if (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE)
        {
            deductResult = r_data[IOT_AP_B14_DEDUCT_RESULT_OFFSET];
            failReason = Common_TwoUint8ToUint16(&r_data[IOT_AP_B14_FAIL_REASON_OFFSET]);

            if (deductResult == 1)
            {
                IOTAP_CFG_InfoPrint("AP,[枪:%d]B14扣款完成\r\n", recvPort);
            }
            else
            {
                IOTAP_CFG_InfoPrint("AP,[枪:%d]B14扣款失败,reason=0x%04X\r\n", recvPort, failReason);
            }

            if ((Common_GetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP) == TRUE) ||
                (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN) == TRUE))
            {
                MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotAPCtx->time);
                Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN, FALSE);
                Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, recvPort, IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN);
            }

            ret = TRUE;
        }
    }

    return ret;
}

/* ====== B帧接收解析函数 - 计费 ====== */

/**
 * @brief  解析B47计费模型下发报文(平台→桩)
 *         从下行数据中提取计费模式ID、切换时间、无效时间、工作状态及时段费率信息,
 *         校验通过后写入NVM持久化存储,供充电计费使用。
 * @param  [out]   port       充电枪号(输出参数)
 * @param  [in]    r_data     下行报文数据指针(帧头之后的有效载荷)
 * @param  [in]    len        有效载荷长度(字节)
 * @retval 0  解析成功且已保存到NVM
 * @retval 1  解析失败(参数非法/长度不足/时段费率越界等)
 * @note   报文结构(B47):
 *         [枪号][计费模式ID(8)][切换时间(8)][无效时间(8)][工作状态]
 *         [时段数N](时段序号|费率类型|开始时间(4)|结束时间(4)|电价(4)|服务费(4))×N
 *         费率类型有效范围: 1~8
 */
/**
 * @brief  AP B47: 下发计费模型下行数据-分时服务费，解析平台下发的分时费率表并写入NVM
 * @note   第一阶段（入参与基本校验）：检查上下文/数据指针有效→校验长度→提取枪号→根据时段数计算期望长度→遍历时段校验费率类型(1~8)
 *         第二阶段（字段解析→NVM写入）：提取模型ID/切换时间/失效时间/工作状态→逐时段解析序号/费率类型/起止时间/电费/服务费→调用多缓冲管理模块写入Flash
 *         第三阶段（结果回填）：打印日志→保存结果到协议上下文→通过输出参数返回枪号
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功（无论数据有效与否，均返回TRUE以通知上层）
 * @retval   FALSE      上下文或数据指针为空
 */
static uint8_t IotAP_ParseTimeBillDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmAPParamBillMode_Struct *pBillMode = NULL;
    uint8_t recvPort = 0;              /* 接收到的枪号 */
    uint8_t parseOk = FALSE;           /* 报文解析是否通过校验 */
    uint8_t result = 1;               /* 解析结果:0=成功,1=失败 */
    uint8_t periodCount = 0;           /* 时段数量 */
    uint8_t index = 0;                 /* 循环索引 */
    uint16_t dataOffset = 0;           /* 当前报文字段偏移量 */
    uint16_t expectLen = 0;            /* 根据时段数计算的期望报文总长 */
    MSNvmAPParamBillMode_Struct newBillMode;

    /* ====== 第一阶段: 入参合法性检查与基本校验 ====== */
    if (r_data == NULL)
    {
        parseOk = FALSE;                          /* 协议上下文或数据指针无效 → 解析失败 */
    }
    else if (len >= IOT_AP_B47_MIN_DATA_LEN)
    {        
        recvPort = r_data[IOT_AP_B47_PORT_OFFSET];  /* 提取枪号: 从报文固定偏移处读取 */
        if (recvPort >= SYSCFG_CFG_GUN_NUM)
        {
            recvPort = 0;                         /* 枪号越界 → 兜底为0号枪 */
        }
        else
        {
            dataOffset = IOT_AP_B47_PERIOD_COUNT_OFFSET;
            periodCount = r_data[dataOffset];       /* 获取时段数量 */
            expectLen = (uint16_t)(IOT_AP_B47_PERIOD_DATA_OFFSET + ((uint16_t)periodCount * IOT_AP_B47_PERIOD_LEN));    /* 计算完整报文长度 */
            /* 校验时段数范围 + 报文长度是否充足 */
            if ((periodCount > 0) && (periodCount <= MSNVM_AP_BILLMODE_PERIOD_COUNT) && (len == expectLen))  /* 严格等值匹配 */
            {
                dataOffset = IOT_AP_B47_PERIOD_DATA_OFFSET; /* 基本校验通过,跳转到时段数据区 */
                parseOk = TRUE;
                result = 0;

                for (index = 0; index < periodCount; index++)
                {
                    /* 校验费率类型字段: 必须在[1, 8]范围内 */
                    if ((r_data[dataOffset + IOT_AP_B47_PERIOD_RATE_OFFSET] < 1) ||
                        (r_data[dataOffset + IOT_AP_B47_PERIOD_RATE_OFFSET] > 8))
                    {
                        parseOk = FALSE;
                        result = 1;
                        break;
                    }
                    dataOffset += IOT_AP_B47_PERIOD_LEN;   /* 跳到下个时段 */
                }
            }
        }

        /* 无论解析成功与否,始终缓存计费模式ID到协议上下文(用于后续上报应答) */
        memcpy(pIotAPCtx->stProtoData[recvPort].timeBillModelId, &r_data[IOT_AP_B47_BILL_MODE_ID_OFFSET],
            sizeof(pIotAPCtx->stProtoData[recvPort].timeBillModelId));
    }
    else
    {
        memset(pIotAPCtx->stProtoData[recvPort].timeBillModelId, 0x00,
            sizeof(pIotAPCtx->stProtoData[recvPort].timeBillModelId));  /* 长度不足 → 清空计费模型ID */
    }

    /* ====== 第二阶段: 字段解析 → 交由多缓冲管理模块保存 ====== */
    if ((parseOk == TRUE) && (pPrivateParam != NULL))
    {
        dataOffset = IOT_AP_B47_BILL_MODE_ID_OFFSET;
        memset(&newBillMode, 0x00, sizeof(MSNvmAPParamBillMode_Struct));

        memcpy(newBillMode.billModeID, &r_data[dataOffset], sizeof(newBillMode.billModeID));   /* B47字段1: 计费模式ID(8B) */
        dataOffset += sizeof(newBillMode.billModeID);
        memcpy(newBillMode.switchTime, &r_data[dataOffset], sizeof(newBillMode.switchTime));  /* B47字段2: 切换时间(7B) */
        dataOffset += sizeof(newBillMode.switchTime);
        memcpy(newBillMode.invalidTime, &r_data[dataOffset], sizeof(newBillMode.invalidTime)); /* B47字段3: 无效时间(7B) */
        dataOffset += sizeof(newBillMode.invalidTime);
        memcpy(newBillMode.workState, &r_data[dataOffset], sizeof(newBillMode.workState));    /* B47字段4: 工作状态(2B) */
        dataOffset += sizeof(newBillMode.workState);

        newBillMode.periodCount = r_data[dataOffset++];   /* 时段数量 */

        /* 遍历提取每个时段的详细费率信息 */
        for (index = 0; index < newBillMode.periodCount; index++)
        {
            newBillMode.period[index].periodSerial = r_data[dataOffset++];   /* 时段序号 */
            newBillMode.period[index].periodRate = r_data[dataOffset++];     /* 费率类型(1~8) */
            memcpy(newBillMode.period[index].startTime, &r_data[dataOffset], sizeof(newBillMode.period[index].startTime));  /* 开始时间(2B) */
            dataOffset += sizeof(newBillMode.period[index].startTime);
            memcpy(newBillMode.period[index].stopTime, &r_data[dataOffset], sizeof(newBillMode.period[index].stopTime));    /* 结束时间(2B) */
            dataOffset += sizeof(newBillMode.period[index].stopTime);
            memcpy(newBillMode.period[index].elecPrice, &r_data[dataOffset], sizeof(newBillMode.period[index].elecPrice));  /* 电费(4字节) */
            dataOffset += sizeof(newBillMode.period[index].elecPrice);
            memcpy(newBillMode.period[index].servePrice, &r_data[dataOffset], sizeof(newBillMode.period[index].servePrice));  /* 服务费(4字节) */
            dataOffset += sizeof(newBillMode.period[index].servePrice);
        }

        /* 交由多缓冲管理模块处理: ID搜索→内容比对→选最优槽→Flash写入→触发Refresh */
        if (IotAP_IsFeeModelValid(&newBillMode) == TRUE)
        {
            IotAP_SaveRateB47Model(&newBillMode, recvPort);
        }
        else
        {
            result = 1;
            IOTAP_CFG_InfoPrint("AP,B47计费模型无效,拒绝保存\r\n");
        }
    }
    else if (parseOk == TRUE)
    {
        result = 1;                               /* 解析OK但NVM无效 → 标记失败 */
    }
    else
    {}  /* 解析未通过 → 保持result默认值1(失败) */

    /* ====== 第三阶段: 日志输出与结果回填 ====== */
    IOTAP_CFG_InfoPrint("AP,B47计费模型解析: len=%d, expectLen=%d, periodCount=%d, result=%d\r\n",
        len, expectLen, periodCount, result);
    pIotAPCtx->stProtoData[recvPort].timeBillResult = result;  /* 存入上下文,供B47应答上报使用 */

    if (port != NULL)                               /* 通过输出参数返回解析出的枪号 */
    {
        port[0] = recvPort;
    }

    return (r_data != NULL);
}

/**
 * @brief  AP B50: 计费模型切换生效下行-分时服务费，解析平台对切换确认的应答
 * @note   Step1: 入参合法性及最小长度校验，提取枪号
 *         Step2: 桩号BCD校验，核对计费模型ID是否与本地下发的切换目标一致
 *         Step3: 一致→清除切换标志，空枪时刷新活跃计费模型→日志输出；不一致→日志告警
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功且ID匹配
 * @retval   FALSE      解析失败或ID不匹配
 */
static uint8_t IotAP_ParseTimeBillSwitchDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B50_MIN_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B50_PORT_OFFSET];
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            if (memcmp(&r_data[IOT_AP_B50_BILL_MODE_ID_OFFSET],
                       pIotAPCtx->stProtoData[recvPort].timeBillSwitchModelId,
                       sizeof(pIotAPCtx->stProtoData[recvPort].timeBillSwitchModelId)) == 0)
            {
                if (AswMonitor_IsOrderIdle(recvPort) == TRUE)
                {
                    IotAP_RefreshNowbillModel(recvPort);
                }

                IOTAP_CFG_InfoPrint("AP,[枪:%d]B50分时服务费切换确认成功\r\n", recvPort);
                ret = TRUE;
            }
            else
            {
                IOTAP_CFG_InfoPrint("AP,[枪:%d]B50计费模型ID不匹配\r\n", recvPort);
            }
        }
    }

    return ret;
}

/**
 * @brief  AP B51: 计费模型召测下行数据-分时服务费，解析平台召测请求并保存召测时间戳
 * @note   Step1: 入参合法性及最小长度校验，提取枪号
 *         Step2: 桩号BCD校验，提取召测时间戳并保存到协议上下文
 *         Step3: 空枪时刷新当前活跃计费模型
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败
 */
static uint8_t IotAP_ParseTimeBillPollDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B51_MIN_DATA_LEN))
    {
        recvPort = r_data[IOT_AP_B51_PORT_OFFSET];
        port[0] = recvPort;

        if ((recvPort < SYSCFG_CFG_GUN_NUM) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
        {
            memcpy(pIotAPCtx->stProtoData[recvPort].timeBillPollTime,
                   &r_data[IOT_AP_B51_TIME_OFFSET],
                   sizeof(pIotAPCtx->stProtoData[recvPort].timeBillPollTime));

            if (AswMonitor_IsOrderIdle(recvPort) == TRUE)
            {
                IotAP_RefreshNowbillModel(recvPort);
            }

            ret = TRUE;
        }
    }

    return ret;
}

/* ====== B帧接收解析函数 - 功率控制 ====== */

/**
 * @brief  AP B33: 充电功率控制下行，解析平台下发的功率调节指令并执行
 * @note   第一阶段（字段解析）：提取枪号→CP56时间戳→功率控制类型→功率值→暂停控制→上报周期
 *         第二阶段（分发执行）：暂停/恢复仅对当前订单生效；普通功率指令按类型更新功率参数
 *         第三阶段：保存控制时间戳和功率值到上下文，通过输出参数返回枪号
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败（参数非法/长度不足）
 */
static uint8_t IotAP_ParsePowerCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t recvPort = 0;
    uint16_t dataOffset = 0;
    uint16_t reportCycle = 0;
    uint8_t ret = FALSE;
    IotAPProtoData_Struct *pProtoData = NULL;
    uint8_t valueValid = FALSE;
    uint8_t orderBusy = FALSE;
    uint8_t pauseCtrlFrameValid = FALSE;
    uint8_t applyOk = FALSE;

    if ((len >= (IOT_AP_PILE_DN_LEN + 1 + 15)))
    {
        recvPort = r_data[IOT_AP_PILE_DN_LEN];   /* 跳过桩号BCD后取枪号 */
        if (recvPort >= SYSCFG_CFG_GUN_NUM)
        {
            recvPort = 0;
        }

        dataOffset = IOT_AP_PILE_DN_LEN + 1;    /* 跳过桩号+枪号 */
        pProtoData = &pIotAPCtx->stProtoData[recvPort];

        /* 提取CP56时间戳 7字节 */
        memcpy(pProtoData->powerCtrlTimepower,
               &r_data[dataOffset], 7);
        dataOffset += 7;

        /* 功率控制类型 1字节 */
        pProtoData->powerCtrlKind = r_data[dataOffset++];

        /* 功率值(KW) 小数点后二位 大端4字节 -> uint32 */
        pProtoData->powerCtrlValue = Common_FourUint8ToUint32(&r_data[dataOffset]);
        dataOffset += 4;

        /* 暂停控制 1字节：0-普通功率指令，1-暂停，2-恢复 */
        pProtoData->powerCtrlPauseCmd = r_data[dataOffset++];

        /* 上报周期(秒) 大端2字节 -> uint16；EE EE 表示沿用原值，不覆盖 */
        reportCycle = Common_TwoUint8ToUint16(&r_data[dataOffset]);
        if (reportCycle != 0xEEEEU)
        {
            pProtoData->powerCtrlReportCycle = reportCycle;
        }

        /* 默认失败；仅合法功率才下发到 VoltCur/CP PWM 链路。 */
        pProtoData->powerCtrlResult = 1;

        IOTAP_CFG_InfoPrint("功率控制，枪号: %u, 类型: %u, 功率: 0x%08X, 暂停指令: %u\r\n",
                            recvPort, pProtoData->powerCtrlKind, pProtoData->powerCtrlValue,
                            pProtoData->powerCtrlPauseCmd);

        /* 0xEEEEEEEE表示本次不更新功率，仅暂停/恢复指令允许使用。 */
        valueValid = (pProtoData->powerCtrlValue <= (SYSCFG_CFG_MAX_OUTPUT_POWER / 10)) ? TRUE : FALSE;
        orderBusy = (AswMonitor_IsOrderIdle(recvPort) != TRUE) ? TRUE : FALSE;
        pauseCtrlFrameValid = ((pProtoData->powerCtrlKind == 3U) && (orderBusy == TRUE)) ? TRUE : FALSE;

        /* 暂停/恢复仅接受类型3和EEEEEEEE占位功率，且只作用于当前订单。 */
        if (pauseCtrlFrameValid == TRUE)
        {
            if (pProtoData->powerCtrlPauseCmd == IOT_AP_POWER_CTRL_PAUSE)
            {
                pProtoData->powerCtrlPauseActive = TRUE;
                pProtoData->powerCtrlResult = 0;
                applyOk = TRUE;
            }
            else if (pProtoData->powerCtrlPauseCmd == IOT_AP_POWER_CTRL_RESUME)
            {
                pProtoData->powerCtrlPauseActive = FALSE;
                pProtoData->powerCtrlResult = 0;
                applyOk = TRUE;
            }
        }

        if ((pProtoData->powerCtrlPauseCmd == IOT_AP_POWER_CTRL_PAUSE_NONE) &&
            (valueValid == TRUE))
        {
            if (pProtoData->powerCtrlKind == 1)
            {
                /* 默认功率设置：不校验枪状态 */
                IotAP_PowerCtrlSetDefaultValue(recvPort, pProtoData->powerCtrlValue);
                pProtoData->powerCtrlResult = 0;
                applyOk = TRUE;
            }
            else if ((pProtoData->powerCtrlKind == 2) ||
                     (pProtoData->powerCtrlKind == 3))
            {
                if (orderBusy == TRUE)
                {
                    if (pProtoData->powerCtrlKind == 2)
                    {
                        pProtoData->powerCtrlDynamicValue = pProtoData->powerCtrlValue;
                        pProtoData->powerCtrlDynamicTick = Common_GetSystick();
                        pProtoData->powerCtrlDynamicValid = TRUE;
                    }
                    else
                    {
                        pProtoData->powerCtrlControlValue = pProtoData->powerCtrlValue;
                        pProtoData->powerCtrlControlActive = TRUE;
                    }

                    pProtoData->powerCtrlResult = 0;

                    if (pProtoData->powerCtrlB57Enable == TRUE)
                    {
                        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B57_POWER_STATUS_UP, TRUE);
                        Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B57_POWER_STATUS_UP, TRUE);
                    }
                    applyOk = TRUE;
                }
                /* 当前无进行中订单 → 失败，走下方失败处理 */
            }
            /* 其他 kind → 失败，走下方失败处理 */
        }
        /* 失败处理：未成功下发则关闭 B57 上报 */
        if (applyOk == FALSE)
        {
            Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
            Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
            Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, recvPort, IOT_AP_CMD_B57_POWER_STATUS_UP, FALSE);
        }

        if (port != NULL)
        {
            port[0] = recvPort;
        }

        ret = TRUE;
    }

    return ret;
}

/**
 * @brief  AP B45: 充电功率召测下行，解析平台召测功率信息请求
 * @note   Step1: 入参合法性校验（上下文/数据指针/最小长度）
 *         Step2: 提取枪号并做越界保护（超过枪数上限则归零）
 *         Step3: 通过输出参数返回枪号，无额外数据解析（召测触发逻辑由上层在发送时处理）
 * @param[out] port      解析出的枪号
 * @param[in]  r_data   接收到的原始数据帧
 * @param[in]  len       帧数据长度
 * @retval   TRUE       解析成功
 * @retval   FALSE      解析失败
 */
static uint8_t IotAP_ParsePowerPollDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t recvPort = 0;
    uint8_t ret = FALSE;

    if (len >= (IOT_AP_PILE_DN_LEN + 1))
    {
        recvPort = r_data[IOT_AP_PILE_DN_LEN];
        if (recvPort >= SYSCFG_CFG_GUN_NUM)
        {
            recvPort = 0;
        }

        if (port != NULL)
        {
            port[0] = recvPort;
        }

        ret = TRUE;
    }

    return ret;
}

/* ====== B帧接收解析函数 - 其他扩展 ====== */

/* AP B32: 请求终端数据下行数据 */
static uint8_t IotAP_ParseTerminalReqDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;

    if ((r_data != NULL) && (len >= IOT_AP_B32_MIN_DATA_LEN) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
    {
        memcpy(pIotAPCtx->stProtoData[recvPort].terminalReqKind,
               &r_data[IOT_AP_B32_KIND_OFFSET],
               sizeof(pIotAPCtx->stProtoData[recvPort].terminalReqKind));
        port[0] = recvPort;
        ret = TRUE;
    }

    return ret;
}

static uint8_t IotAP_IsFtpParaValid(const CddNetMSocketPara_Union *pFtpPara)
{
    uint8_t ret = FALSE;

    if (pFtpPara != NULL)
    {
        if ((pFtpPara->stFtpPara.user[0] != '\0') &&
            (pFtpPara->stFtpPara.passwd[0] != '\0') &&
            (pFtpPara->stFtpPara.path[0] != '\0') &&
            (pFtpPara->stFtpPara.fileName[0] != '\0'))
        {
            ret = TRUE;
        }
    }

    return ret;
}

static uint8_t IotAP_FillFtpParaFromPayload(CddNetMSocketPara_Union *pFtpPara ,const uint8_t *pData,uint16_t len)
{
    uint8_t ret = FALSE;

    if (pFtpPara != NULL)
    {
        memset(pFtpPara, 0x00, sizeof(CddNetMSocketPara_Union));
    }

    if ((pFtpPara != NULL) && (pData != NULL) && (len >= IOT_AP_B23_MIN_DATA_LEN))
    {
        pFtpPara->stFtpPara.eFileFormat = eCddNetMFileType_BIN;
        pFtpPara->stFtpPara.eMode = eCddNetMFtpMode_Download;

        snprintf(pFtpPara->stFtpPara.ip, sizeof(pFtpPara->stFtpPara.ip),
                 "%u.%u.%u.%u",
                 pData[IOT_AP_B23_IP_OFFSET + 3],
                 pData[IOT_AP_B23_IP_OFFSET + 2],
                 pData[IOT_AP_B23_IP_OFFSET + 1],
                 pData[IOT_AP_B23_IP_OFFSET]);
        pFtpPara->stFtpPara.port = Common_TwoUint8ToUint16((uint8_t *)&pData[IOT_AP_B23_PORT_OFFSET]);
        strncpy(pFtpPara->stFtpPara.user, "gn_ftp_fw_reader", sizeof(pFtpPara->stFtpPara.user) - 1);
        strncpy(pFtpPara->stFtpPara.passwd, "d2aa28ee9a8693db", sizeof(pFtpPara->stFtpPara.passwd) - 1);        
        memcpy(pFtpPara->stFtpPara.path, &pData[IOT_AP_B23_PATH_OFFSET], sizeof(pFtpPara->stFtpPara.path) - 1);
        memcpy(pFtpPara->stFtpPara.fileName, &pData[IOT_AP_B23_FILE_NAME_OFFSET], IOT_AP_FTP_FILE_NAME_LEN);

        ret = IotAP_IsFtpParaValid(pFtpPara);
    }

    return ret;
}

/* AP B39: 平台ftp服务器地址下发 */
static uint8_t IotAP_ParseFtpAddrDown(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    IotAPProtoData_Struct *pProtoData = NULL;

    if ((r_data != NULL) && (len >= IOT_AP_PILE_DN_LEN) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
    {
        pProtoData = &pIotAPCtx->stProtoData[recvPort];
        pProtoData->ftpAddrResult = 0;

        port[0] = recvPort;
        ret = TRUE;
    }

    return ret;
}

/* AP B23: 远程升级启动 */
/**
 * @brief  解析平台下发的升级启动命令，提取FTP参数并请求启动OTA升级流程
 * @param  port  输出参数，用于返回接收端口号
 * @param  r_data 指向平台下发报文载荷数据的指针
 * @param  len   报文载荷数据长度
 * @return uint8_t 解析结果，TRUE表示解析成功，FALSE表示参数无效或校验未通过
 * @note   函数内部会检查升级条件（无正在进行的升级、更新条件满足），
 *         根据报文中B23字节的模式值决定立即执行还是等待空闲后执行；
 *         升级结果通过 pProtoData->upgradeResult 反映，0表示成功发起升级请求
 */
static uint8_t IotAP_ParseUpgradeStart(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t ret = FALSE;
    uint8_t recvPort = 0;
    IotAPProtoData_Struct *pProtoData = NULL;
    eSSUcmExcuteMode_Enum execMode = eSSUcmExcuteMode_WaitIdle;

    if ((r_data != NULL) && (len >= IOT_AP_PILE_DN_LEN) && (IotAP_CheckPileDn(r_data, IOT_AP_B4_PILE_DN_OFFSET) == TRUE))
    {
        pProtoData = &pIotAPCtx->stProtoData[recvPort];
        pIotAPCtx->upgradeResult = 1;

        if ((IotAP_FillFtpParaFromPayload(&pProtoData->upgradePara, r_data, len) == TRUE) &&
            (SSUcm_IsOngoging() != TRUE))
        {
            execMode = (r_data[IOT_AP_B23_MODE_OFFSET] == 1) ? eSSUcmExcuteMode_Immediate : eSSUcmExcuteMode_WaitIdle;
            SSUcm_GetResult();
            SSUcm_ReqStartOTA(&pProtoData->upgradePara, eSSUcmChannelType_FTP, execMode, 10 * 60 * 1000);
            pIotAPCtx->upgradeOngoing = TRUE;
            pIotAPCtx->upgradeResult = 0;
        }

        port[0] = recvPort;
        ret = TRUE;
    }

    return ret;
}

/**
 * @brief  IoT AP 协议层超时检测，轮询所有响应型命令的接收超时并执行重传或失败处理
 * @note   Step1: 遍历命令接收控制表，筛选需等待响应且配置了超时时间的命令
 *         Step2: 检查各枪对应命令的接收超时计数
 *         Step3: 超时次数未达上限则触发重传，达到上限则执行失败回调
 * @param[in]  无
 * @retval  无
 */
void IotAP_TimeoutDetect(void)
{
    const IotAPRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    /* Layer1: 遍历命令接收控制表, 筛选需要超时检测的命令 */
    for (index = 0; index < IOT_AP_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotAPRecvctrlTable[index];

        /* 仅处理"需等待响应"且"配置了超时时间"的命令, 其余跳过 */
        if ((pCmdRecvCtrl->cmdType != IOT_AP_CMDTYPE_RESPONSE) || (pCmdRecvCtrl->maxTimeout == 0))
        {
            continue;
        }

        /* Layer2: 遍历充电枪端口, 每把枪独立维护一套收发状态 */
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            /* 门控条件1: 该端口+命令组合已启用接收定时器才检测 */
            if (Common_GetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                continue;
            }

            /* 门控条件2: 判断从上次接收到现在是否已超过最大超时时间 */
            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd),
                                      pCmdRecvCtrl->maxTimeout) != TRUE)
            {
                continue;
            }

            /* 累加重试次数(SetRptCount内部会+1)并读取当前值 */
            Common_SetRptCount(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
            timeoutCount = Common_GetRptCount(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

            IOTAP_CFG_InfoPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n",
                                pCmdRecvCtrl->cmd,
                                pCmdRecvCtrl->cMeaning,
                                timeoutCount,
                                pCmdRecvCtrl->maxTimeout);

            /* 分支A: 重试次数耗尽 → 清理状态并执行失败后处理 */
            if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
            {
                /* 致命性失败: 登录/心跳响应超时 → 触发整个连接离线 */
                if ((pCmdRecvCtrl->cmd == IOT_AP_CMD_LOGIN_RSP) || (pCmdRecvCtrl->cmd == IOT_AP_CMD_HEARTBEAT_RSP))
                {
                    IotAP_OfflineHandle();
                }
                else
                {
                    /* 彻底清理: 清零重试计数, 关闭接收定时器 */
                    Common_ClearRptCount(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                    /* B53/B54专用: 彻底停止B53收发控制, 订单记录强行置为已上报(丢弃), 避免NVM队列卡死 */
                    if (pCmdRecvCtrl->cmd == IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN)
                    {
                        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                        Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, IOT_AP_CMD_B53_ONLINE_DETAIL_UP, FALSE);
                        MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, pIotAPCtx->time);
                        IOTAP_CFG_InfoPrint("AP,[枪:%d]B53在线分时交易记录上报%d次未确认, 强行置为成功!\r\n", port, timeoutCount);
                    }
                    /* 普通失败: 解除对应发送命令的占用锁(matchCmd), 允许上层重新发起 */
                    else
                    {
                        Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }                  
            }
            /* 分支B: 未达重试上限 → 立即触发重传机制 */
            else
            {
                /* 先关闭当前接收定时器, 避免在重发等待期间重复触发超时 */
                Common_SetRecvTimerEnable(pIotAPCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);

                if (pCmdRecvCtrl->matchCmd != IOT_AP_CMD_NULL)
                {
                    /* 重传三连击: 重新使能发送 + 标记立即发送 + 回退发送标志位 */
                    Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendImmdFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendFlag(pIotAPCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                }
            }
        }
    }
}

/* ====== 白名单 - 暂不实现(交流桩) ====== */
