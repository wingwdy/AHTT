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
*2026/05/21     V1.0.0       AI        初版创建 - 骨架代码
*
******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoAPSend.h"
#include "Asw_IotProtoAPM.h"

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/* ====== F帧发送函数 ====== */

uint16_t IotAP_SendLoginReq(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装登录验证请求 F1 */
    return 0;
}

uint16_t IotAP_SendHeartbeatReq(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装心跳上报 F5 */
    return 0;
}

uint16_t IotAP_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装时钟同步应答 F8 */
    return 0;
}

/* ====== B帧发送函数 - 基础 ====== */

uint16_t IotAP_SendRealtimeData(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装充电过程实时监测数据上报 B1 */
    return 0;
}

uint16_t IotAP_SendChgCtrlResult(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装充电启停控制命令结果确认 B5 */
    return 0;
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
    /* TODO: 组装在线情况下停止充电上传分时交易明细数据 B53 */
    return 0;
}

/* ====== B帧发送函数 - 计费 ====== */

uint16_t IotAP_SendTimeBillUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装下发计费模型上行数据-分时服务费 B48 */
    return 0;
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
    /* TODO: 组装充电功率控制上行 B34 */
    return 0;
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
    /* TODO: 组装SIM卡信息上行数据 B31 */
    return 0;
}

uint16_t IotAP_SendZeroMeterValue(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装零点示值上报 B38 */
    return 0;
}

uint16_t IotAP_SendFtpAddrUp(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装平台ftp服务器地址上行 B40 */
    return 0;
}

uint16_t IotAP_SendUpgradeResult(uint8_t port, uint8_t *pBuf)
{
    /* TODO: 组装远程升级启动命令接收结果 B24 */
    return 0;
}

/* ====== 白名单 - 暂不实现(交流桩) ====== */
