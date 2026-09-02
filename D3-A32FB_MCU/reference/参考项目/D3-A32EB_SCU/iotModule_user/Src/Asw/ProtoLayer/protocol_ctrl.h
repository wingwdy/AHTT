#ifndef __PROTOCOL_CTRL_H__
#define __PROTOCOL_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "Libinclude.h"
#include "Comminclude.h"
#include "AppHeaderSummary.h"

//==============================================================
//充电控制发送使能
#define SEND_ENABLE_NULL		0x00		//无效
#define SEND_ENABLE_ON			0x55		//打开
#define SEND_ENABLE_OFF			0xAA		//关闭

#define SEND_FLAG_NULL			0x00		//无效
#define SEND_FLAG_YES			0x55		//已发送
#define SEND_FLAG_NO			0xAA		//未发送

#define RECV_ENABLE_NULL		0x00		//无效
#define RECV_ENABLE_ON			0x55		//打开
#define RECV_ENABLE_OFF			0xAA		//关闭

#define TCP_DATA_LEN_MAX    	2048    	//tcp协议包数据长度max  云快充2.1交易记录超过1024 改成2048
#define QR_MAX_SIZE				200			//二维码最大长度
#define TCP_DATA_LEN_MAX_AHCHG  512    		//tcp协议包数据长度max


#define DEV_NUM_LEN				7				//


//
struct BLE_HEAD_8BIT
{
	uint8_t   btMsgID   	:4;			//消息ID 每发送一条消息ID加1；如果消息有应答，应答消息的ID和请求消息ID匹配；超过15则自动循环到1
	uint8_t   btEncrypt    	:1;			//数据加密指示
	uint8_t   btVersion    	:3;			//版本信息
};

union BLE_HEAD {
    uint8_t all;
    struct BLE_HEAD_8BIT bit;
};

//===================================================================运营平台
//对平台文件的接口函数
void SetSendTick(uint8_t u8Port, uint32_t cmd, int32_t tick);

void SetSendEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);

uint8_t GetSendEnable(uint8_t u8Port, uint32_t cmd);

void Send_Immediately(uint8_t u8Port, uint32_t cmd);

void SetSendImmdFlag(uint8_t u8Port, uint32_t cmd, uint8_t Flag);

uint8_t GetSendImmdFlag(uint8_t u8Port, uint32_t cmd);

int32_t GetSendTick(uint8_t u8Port, uint32_t cmd);

void SetSendFlag(uint8_t u8Port, uint32_t cmd, uint8_t flag);

void SetSendSrm(uint8_t u8Port, uint32_t cmd, uint16_t Srm);

uint16_t GetSendSrm(uint8_t u8Port, uint32_t cmd);

void SetRecvTick(uint8_t u8Port, uint32_t cmd, uint32_t tick);

int32_t GetRecvTick(uint8_t u8Port, uint32_t cmd);

void SetRecvEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);

uint8_t GetRecvEnable(uint8_t u8Port, uint32_t cmd);


//===================================================================蓝牙
void SetBleSTick(uint8_t u8Port, uint32_t cmd, int32_t tick);

void SetBleSEnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);

uint8_t GetBleSEnable(uint8_t u8Port, uint32_t cmd);

void BleSend_Immediately(uint8_t u8Port, uint32_t cmd);

int32_t GetBleSTick(uint8_t u8Port, uint32_t cmd);

void SetBleSFlag(uint8_t u8Port, uint32_t cmd, uint8_t flag);

uint8_t GetBleSFlag(uint8_t u8Port, uint32_t cmd);

void SetBleRTick(uint8_t u8Port, uint32_t cmd, uint32_t tick);

int32_t GetBleRTick(uint8_t u8Port, uint32_t cmd);

void SetBleREnable(uint8_t u8Port, uint32_t cmd, uint8_t flag);

uint8_t GetBleREnable(uint8_t u8Port, uint32_t cmd);

void PlatHeartTickRefresh(void);

uint8_t GetRecvRptCnt(uint8_t u8Port, uint32_t cmd);
void SetRecvRptUpt(uint8_t u8Port, uint32_t cmd);
void SetClearRecvRptUpt(uint8_t u8Port, uint32_t cmd);

void g_UpdatePathToName(char *inputPath, char *filePathOutput, char *fileNameOutput);
void g_PileUpdateInterface(char *ip, uint16_t port, char *name, char *password, char *path, char *fileName);

//===================================================================加密
/* 仅用于16byte字节的加解密 */
void aes128_sevice(uint8_t mode, const unsigned char *key, unsigned char *iv, const unsigned char *input, unsigned char *output);

//===================================================================
void Plat_Reconnect(eNetSocket SocketID);

void GNUpProtocolDeal(void);

char const *NormProtocolName(void);
void NormProtocolInit(void);
void NormProtocolMalloc(void);

void runProtocolTask(void);

#ifdef __cplusplus
}
#endif

#endif

