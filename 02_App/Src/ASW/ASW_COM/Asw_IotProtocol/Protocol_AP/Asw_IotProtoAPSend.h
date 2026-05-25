/******************************************************************************
* File Name          : Asw_IotProtoAPSend.h
* Description        : 安培协议发送帧处理头文件
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
#ifndef ASW_IOT_PROTO_AP_SEND_H_
#define ASW_IOT_PROTO_AP_SEND_H_

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

/* F帧发送函数 */
uint16_t IotAP_SendLoginReq(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendHeartbeatReq(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendSyncTimeRsp(uint8_t port, uint8_t *pBuf);

/* B帧发送函数 - 基础 */
uint16_t IotAP_SendRealtimeData(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendChgCtrlResult(uint8_t port, uint8_t *pBuf);

/* B帧发送函数 - 鉴权 */

/* B帧发送函数 - 交易 */
uint16_t IotAP_SendOnlineDetailUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendDeductConfirmRsp(uint8_t port, uint8_t *pBuf);

/* B帧发送函数 - 计费 */
uint16_t IotAP_SendTimeBillUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendTimeBillSwitchUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendTimeBillPollUp(uint8_t port, uint8_t *pBuf);

/* B帧发送函数 - 功率控制 */
uint16_t IotAP_SendPowerCtrlUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendPowerPollUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendPowerStatusUp(uint8_t port, uint8_t *pBuf);

/* B帧发送函数 - 其他扩展 */
uint16_t IotAP_SendSimInfoUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendZeroMeterValue(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendFtpAddrUp(uint8_t port, uint8_t *pBuf);
uint16_t IotAP_SendUpgradeResult(uint8_t port, uint8_t *pBuf);

/* 白名单 - 暂不实现(交流桩) */

#endif /* ASW_IOT_PROTO_AP_SEND_H_ */
