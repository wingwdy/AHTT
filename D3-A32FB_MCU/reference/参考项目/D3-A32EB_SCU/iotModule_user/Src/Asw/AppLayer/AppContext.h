#ifndef _APP_CONTEXT_H_
#define _APP_CONTEXT_H_

#include "ProtocolCCU.h"
#include "ProtocolBLE.h"

typedef struct
{
	E_UART_CHANNEL_LIST e_UartChnId;
    CProtoCCU* p_proto_ccu;
 	uint8_t    u8_dev_info_is_send[2];
 	uint32_t   u32_tick_yx_data[2];
 	uint32_t   u32_tick_yc_data[2];

	E_UART_CHANNEL_LIST e_BLEUartChnId;
	CProtoBLE* p_proto_ble;
	
} app_context_t;


extern app_context_t app_ctx;

#endif /*_APP_CONTEXT_H_*/
