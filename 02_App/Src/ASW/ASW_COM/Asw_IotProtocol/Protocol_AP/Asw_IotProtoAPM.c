/******************************************************************************
* File Name          : Asw_IotProtoAPM.c
* Description        : 安培协议主模块源文件
* -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
*------------    --------     -------   ----------------------------------------
*2026/05/21     V1.0.0       WDY        初版创建 - 骨架代码
*2026/05/22     V1.1.0       WDY        实现FillLinkPara/InitMemory/MainFunction FSM骨架
*
******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoAPM.h"
#include "Asw_ErrorHandle.h"
#include "Asw_IotProtoAPSend.h"
#include "Asw_IotProtoAPRecv.h"
#include "Asw_ChargeIf.h"
#include "MS_Nvm.h"
#include "myMalloc.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
IotAPCtx_Struct *pIotAPCtx = NULL;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotAP_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotAP_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotAP_WSInitHandle(void);
static void IotAP_WSOfflineHandle(void);
static void IotAP_WSLoginHandle(void);
static void IotAP_WSNormalHandle(void);
static void IotAP_OfflineHandle(void);

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/*
 * 发送控制结构体查找函数
 * 根据 CMD 命令号返回对应端口的发送控制结构体指针
 * 索引分配(共19条):
 *  [0] F1 登录请求       [10] B49 计费切换生效上行
 *  [1] F5 心跳请求       [11] B52 计费召测上行
 *  [2] F8 时钟同步应答   [12] B34 功率控制上行
 *  [3] B1 实时数据上报   [13] B46 功率召测上行
 *  [4] B5 启停控制结果   [14] B57 功率实时状态上行
 *  [5] B6 刷卡鉴权上行   [15] B31 SIM卡信息上行
 *  [6] B10 启动通知上行  [16] B38 零点示值上报
 *  [7] B14 扣款确认应答  [17] B40 FTP地址上行
 *  [8] B53 在线分时明细上传 [18] B24 升级结果上报
 *  [9] B48 分时计费模型上行
 */
static CommonSendCtrl_Struct* IotAP_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    if ((pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return NULL;
    }

    switch (cmd)
    {
        case IOT_AP_CMD_LOGIN_REQ:                  pSendCtrl = &pIotAPCtx->stSendCtrl[port][0];   break;
        case IOT_AP_CMD_HEARTBEAT_REQ:              pSendCtrl = &pIotAPCtx->stSendCtrl[port][1];   break;
        case IOT_AP_CMD_SYNC_TIME_RSP:              pSendCtrl = &pIotAPCtx->stSendCtrl[port][2];   break;
        case IOT_AP_CMD_B01_REALTIME_DATA:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][3];   break;
        case IOT_AP_CMD_B05_CHG_CTRL_RESULT:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][4];   break;
        case IOT_AP_CMD_B06_CARD_AUTH_UP:           pSendCtrl = &pIotAPCtx->stSendCtrl[port][5];   break;
        case IOT_AP_CMD_B10_START_NOTIFY_UP:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][6];   break;
        case IOT_AP_CMD_B14_DEDUCT_CONFIRM:         pSendCtrl = &pIotAPCtx->stSendCtrl[port][7];   break;
        case IOT_AP_CMD_B53_ONLINE_DETAIL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][8];   break;

        case IOT_AP_CMD_B48_TIMEBILL_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][9];   break;
        case IOT_AP_CMD_B49_TIMEBILL_SWITCH_UP:     pSendCtrl = &pIotAPCtx->stSendCtrl[port][10];  break;
        case IOT_AP_CMD_B52_TIMEBILL_POLL_UP:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][11];  break;
        case IOT_AP_CMD_B34_POWER_CTRL_UP:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][12];  break;
        case IOT_AP_CMD_B46_POWER_POLL_UP:          pSendCtrl = &pIotAPCtx->stSendCtrl[port][13];  break;
        case IOT_AP_CMD_B57_POWER_STATUS_UP:        pSendCtrl = &pIotAPCtx->stSendCtrl[port][14];  break;
        case IOT_AP_CMD_B31_SIM_INFO_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][15];  break;
        case IOT_AP_CMD_B38_ZERO_METER_VALUE:       pSendCtrl = &pIotAPCtx->stSendCtrl[port][16];  break;
        case IOT_AP_CMD_B40_FTP_ADDR_UP:            pSendCtrl = &pIotAPCtx->stSendCtrl[port][17];  break;
        case IOT_AP_CMD_B24_UPGRADE_RESULT:         pSendCtrl = &pIotAPCtx->stSendCtrl[port][18];  break;

        default: break;
    }

    return pSendCtrl;
}

/*
 * 接收控制结构体查找函数
 * 根据 CMD 命令号返回对应端口的接收控制结构体指针
 * 索引分配(共16条):
 *  [0] F2 登录应答       [9]  B47 分时计费模型下发
 *  [1] F6 心跳应答       [10] B50 计费切换生效下发
 *  [2] F7 时钟同步请求   [11] B51 计费召测下发
 *  [3] B4 启停控制下发   [12] B33 功率控制下发
 *  [4] B7 刷卡鉴权下行   [13] B45 功率召测下发
 *  [5] B11 启动通知下行  [14] B32 终端数据请求下发
 *  [6] B54 在线分时明细下行 [15] B39 FTP地址下发
 *  [7] B14 扣款确认下行  [8]  B47 分时计费模型下发
 *  [15] B23 远程升级启动
 */
static CommonRecvCtrl_Struct* IotAP_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    if ((pIotAPCtx == NULL) || (port >= SYSCFG_CFG_GUN_NUM))
    {
        return NULL;
    }

    switch (cmd)
    {
        case IOT_AP_CMD_LOGIN_RSP:                 pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][0];   break;
        case IOT_AP_CMD_HEARTBEAT_RSP:             pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][1];   break;
        case IOT_AP_CMD_SYNC_TIME_REQ:             pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][2];   break;
        case IOT_AP_CMD_B04_CHG_CTRL_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][3];   break;
        case IOT_AP_CMD_B07_CARD_AUTH_DOWN:        pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][4];   break;
        case IOT_AP_CMD_B11_START_NOTIFY_DOWN:     pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][5];   break;
        case IOT_AP_CMD_B54_ONLINE_DETAIL_DOWN:    pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][6];   break;
        case IOT_AP_CMD_B14_DEDUCT_CONFIRM:        pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][7];   break;

        case IOT_AP_CMD_B47_TIMEBILL_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][8];   break;
        case IOT_AP_CMD_B50_TIMEBILL_SWITCH_DOWN:  pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][9];   break;
        case IOT_AP_CMD_B51_TIMEBILL_POLL_DOWN:    pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][10];  break;
        case IOT_AP_CMD_B33_POWER_CTRL_DOWN:       pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][11];  break;
        case IOT_AP_CMD_B45_POWER_POLL_DOWN:       pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][12];  break;
        case IOT_AP_CMD_B32_TERMINAL_REQ_DOWN:     pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][13];  break;
        case IOT_AP_CMD_B39_FTP_ADDR_DOWN:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][14];  break;
        case IOT_AP_CMD_B23_UPGRADE_START:         pRecvCtrl = &pIotAPCtx->stRecvCtrl[port][15];  break;

        default: break;
    }

    return pRecvCtrl;
}

/* ====== 初始化状态处理 ====== */
static void IotAP_WSInitHandle(void)
{
    pIotAPCtx->eWorkState = eIotAPWorkState_Offline;
}

/* ====== 离线状态处理 ====== */
static void IotAP_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    /* 清除登录和队列状态 */
    pIotAPCtx->loginSucc = FALSE;
    pIotAPCtx->queueBusyFlag = FALSE;
    pIotAPCtx->waitQueueIdleTick = 0;

    /* 重置发送状态 */
    pIotAPCtx->sendIndex = 0;
    pIotAPCtx->sendPort = 0;
    pIotAPCtx->reqSeq = 0;

    /* 清零实时数据上报计时和枪状态记录 */
    memset(pIotAPCtx->realDataReportTick, 0x00, sizeof(pIotAPCtx->realDataReportTick));
    memset(pIotAPCtx->lastGunState, 0x00, sizeof(pIotAPCtx->lastGunState));
    memset(pIotAPCtx->lastGunConnectState, 0x00, sizeof(pIotAPCtx->lastGunConnectState));

    /* 清零所有收发控制结构体 */
    memset(pIotAPCtx->stSendCtrl, 0x00, sizeof(pIotAPCtx->stSendCtrl));
    memset(pIotAPCtx->stRecvCtrl, 0x00, sizeof(pIotAPCtx->stRecvCtrl));

    /* 重置帧队列并转换桩编号为BCD码 */
    FrameQueue_Reset(pIotAPCtx->frameQueueChannelID);
    Common_AsciiToBCD(pParam->platPileDn, pIotAPCtx->pileDnBCD, 14);

    /* 设置平台离线错误回调 */
    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);

    pIotAPCtx->eWorkState = eIotAPWorkState_Login;
}

/* ====== 登录状态处理 ====== */
static void IotAP_WSLoginHandle(void)
{
    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotAPCtx->eWorkState = eIotAPWorkState_Normal;
        Common_SetSendEnable(pIotAPCtx->pFuncSendCtrl, 0, IOT_AP_CMD_LOGIN_REQ, TRUE);
    }
}

/* ====== 正常工作状态处理 ====== */
static void IotAP_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotAP_OfflineHandle();
    }
    else
    {
        /* TODO: 已登录后的周期性任务（后续实现）
        if (pIotAPCtx->loginSucc == TRUE)
        {
            IotAP_CycleDetect();
        }
        */

        /* TODO: 上行发送控制处理
        IotAP_UpCtrlSendDeal();
        */

        /* TODO: 上行接收解析处理
        IotAP_UpCtrlRecvDeal();
        */

        /* TODO: 超时检测处理
        IotAP_TimeoutDetect();
        */
    }
}

/* ====== 断线离线处理 ====== */
static void IotAP_OfflineHandle(void)
{
    /* 清除登录成功标志 */
    pIotAPCtx->loginSucc = FALSE;

    /* 切换到离线状态，由WSOfflineHandle完成剩余清理 */
    pIotAPCtx->eWorkState = eIotAPWorkState_Offline;
}

/******************************************************************************
*    对外接口函数实现
******************************************************************************/

void IotAP_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotAPCtx != NULL)
    {
        strncpy(pLinkPara->stTcpPara.ip, pParam->platMainIp, sizeof(pParam->platMainIp) - 1);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, IOT_AP_TXRX_BUFFER_SIZE, IOT_AP_TXRX_BUFFER_SIZE,
                         &pIotAPCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotAPCtx->frameQueueChannelID;
    }
}

void IotAP_InitMemory(void)
{
    pIotAPCtx = (IotAPCtx_Struct *)myMalloc(sizeof(IotAPCtx_Struct));

    if (pIotAPCtx == NULL)
    {
        return;
    }

    memset(pIotAPCtx, 0, sizeof(IotAPCtx_Struct));
    pIotAPCtx->pFuncSendCtrl = IotAP_GetSendCtrl;
    pIotAPCtx->pFuncRecvCtrl = IotAP_GetRecvCtrl;
}

void IotAP_MainFunction(void)
{
    if (pIotAPCtx == NULL)
    {
        return;
    }

    switch (pIotAPCtx->eWorkState)
    {
        case eIotAPWorkState_Init:
        {
            IotAP_WSInitHandle();
            break;
        }
        case eIotAPWorkState_Offline:
        {
            IotAP_WSOfflineHandle();
            break;
        }
        case eIotAPWorkState_Login:
        {
            IotAP_WSLoginHandle();
            break;
        }
        case eIotAPWorkState_Normal:
        {
            IotAP_WSNormalHandle();
            break;
        }
        default:
        {
            pIotAPCtx->eWorkState = eIotAPWorkState_Init;
        }
    }
}

void IotAP_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pStandardBillMode)
{
    /* TODO: 计费模式转换为安培格式 */
}

void IotAP_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{
    /* TODO: 打包充电记录为安培协议格式 */
}

void IotAP_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    /* TODO: 充电记录转换为安培上报格式 */
}

uint8_t IotAP_SwipCardCharge(uint8_t port)
{
    /* TODO: 刷卡启动充电处理 */
    return 0;
}
