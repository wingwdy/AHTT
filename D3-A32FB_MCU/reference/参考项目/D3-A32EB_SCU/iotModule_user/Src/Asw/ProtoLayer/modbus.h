#ifndef __MODBUS_H__
#define __MODBUS_H__
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "AppHeaderSummary.h"

#define MODBUS_BUFSZ  512

#define PLATMOD_DATA_RECVLEN_MAX    	256          //Modbus数据长度max
#define PLATMOD_DATA_SENDLEN_MAX    	256          //Modbus数据长度max

typedef struct
{
    E_UART_CHANNEL_LIST e_UartChnId;
	uint8_t m_aucRcvBuf[PLATMOD_DATA_RECVLEN_MAX];
	uint8_t m_aucSendBuf[PLATMOD_DATA_SENDLEN_MAX];
	uint16_t m_uRcvLen;

} STRU_MBUS_COMM_CONTEXT;


typedef struct
{
  uint8_t   dev;    /* STD */
  uint8_t   func;   /* STD */
  
  uint8_t   buf[1];
} ModbusFrame;

uint16_t CRC16(uint8_t *puchMsg, uint16_t usDataLen);
uint16_t ModbusCRC(uint8_t *pData, uint32_t len);

ModbusFrame *MbsComm_ParsePack(STRU_MBUS_COMM_CONTEXT *g_pstuMbsCommHandle);
int32_t MbsComm_SendPack(STRU_MBUS_COMM_CONTEXT *g_pstuMbsCommHandle, uint8_t *p_data, uint16_t len);
void MbsCommModuleInit(STRU_MBUS_COMM_CONTEXT *g_pstuMbsCommHandle, E_UART_CHANNEL_LIST channel);

#endif
