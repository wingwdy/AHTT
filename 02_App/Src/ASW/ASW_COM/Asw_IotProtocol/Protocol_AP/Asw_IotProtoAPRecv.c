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
*2026/05/21     V1.0.0       AI        初版创建 - 骨架代码
*
******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoAPRecv.h"
#include "Asw_IotProtoAPM.h"

/*******************************************************************************
*    Function Source Code
******************************************************************************/

/* ====== F帧接收解析函数 ====== */

uint8_t IotAP_ParseLoginRsp(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析登录验证应答 F2 */
    return 0;
}

uint8_t IotAP_ParseHeartbeatRsp(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析心跳应答 F6 */
    return 0;
}

uint8_t IotAP_ParseSyncTimeReq(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析时钟同步请求 F7 */
    return 0;
}

/* ====== B帧接收解析函数 - 控制 ====== */

uint8_t IotAP_ParseChgCtrlDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析充电启停控制命令下发下行数据 B4 */
    return 0;
}

/* ====== B帧接收解析函数 - 鉴权 ====== */

uint8_t IotAP_ParseCardAuthDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析刷卡鉴权下行 B7 */
    return 0;
}

uint8_t IotAP_ParseStartNotifyDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析启动通知下行 B11 */
    return 0;
}

/* ====== B帧接收解析函数 - 交易 ====== */

uint8_t IotAP_ParseOnlineDetailDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析在线分时明细交易包下行数据 B54 */
    return 0;
}

uint8_t IotAP_ParseDeductConfirmDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析充电扣款后下行数据 B14 */
    return 0;
}

/* ====== B帧接收解析函数 - 计费 ====== */

uint8_t IotAP_ParseTimeBillDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析下发计费模型下行数据-分时服务费 B47 */
    return 0;
}

uint8_t IotAP_ParseTimeBillSwitchDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析计费模型切换生效下行-分时服务费 B50 */
    return 0;
}

uint8_t IotAP_ParseTimeBillPollDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析计费模型召测下行数据-分时服务费 B51 */
    return 0;
}

/* ====== B帧接收解析函数 - 功率控制 ====== */

uint8_t IotAP_ParsePowerCtrlDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析充电功率控制下行 B33 */
    return 0;
}

uint8_t IotAP_ParsePowerPollDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析充电功率召测下行 B45 */
    return 0;
}

/* ====== B帧接收解析函数 - 其他扩展 ====== */

uint8_t IotAP_ParseTerminalReqDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析请求终端数据下行数据 B32 */
    return 0;
}

uint8_t IotAP_ParseFtpAddrDown(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析平台ftp服务器地址下发 B39 */
    return 0;
}

uint8_t IotAP_ParseUpgradeStart(uint8_t port, uint8_t *r_data, uint16_t len)
{
    /* TODO: 解析远程升级启动 B23 */
    return 0;
}

/* ====== 白名单 - 暂不实现(交流桩) ====== */
