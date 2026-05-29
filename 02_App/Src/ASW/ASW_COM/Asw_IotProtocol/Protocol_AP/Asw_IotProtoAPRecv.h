/******************************************************************************
* File Name          : Asw_IotProtoAPRecv.h
* Description        : 安培协议接收帧解析头文件
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
#ifndef ASW_IOT_PROTO_AP_RECV_H_
#define ASW_IOT_PROTO_AP_RECV_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Asw_IotProtoAPTypes.h"

/******************************************************************************
*    Macro Definition
*******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/

/* F帧接收解析函数 */
extern const IotAPRecvCtrl_Struct c_stIotAPRecvctrlTable[IOT_AP_CMD_RECV_COUNT];

uint8_t IotAP_ParseLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseUFrameAck(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseHeartbeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseSyncTimeReq(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 控制 */
uint8_t IotAP_ParseChgCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 鉴权 */
uint8_t IotAP_ParseCardAuthDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseStartNotifyDown(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 交易 */
uint8_t IotAP_ParseOnlineDetailDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseDeductConfirmDown(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 计费 */
uint8_t IotAP_ParseTimeBillDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseTimeBillSwitchDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseTimeBillPollDown(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 功率控制 */
uint8_t IotAP_ParsePowerCtrlDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParsePowerPollDown(uint8_t *port, uint8_t *r_data, uint16_t len);

/* B帧接收解析函数 - 其他扩展 */
uint8_t IotAP_ParseTerminalReqDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseFtpAddrDown(uint8_t *port, uint8_t *r_data, uint16_t len);
uint8_t IotAP_ParseUpgradeStart(uint8_t *port, uint8_t *r_data, uint16_t len);

void IotAP_TimeoutDetect(void);

/* 以下为暂不实现的帧(直流桩/交流不开发) */

/* 白名单 - 暂不实现 */

#endif /* ASW_IOT_PROTO_AP_RECV_H_ */
