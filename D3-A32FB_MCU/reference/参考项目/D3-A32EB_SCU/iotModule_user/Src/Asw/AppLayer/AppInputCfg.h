#ifndef __APP_INPUTCFG_H__
#define __APP_INPUTCFG_H__

#include "AppHeaderSummary.h"


#define INPUTBUF_LEN	256      //云快充RSA密钥为128字节，buf大小由128改为256字节


typedef struct _inputCmdBuf {
	uint8_t u8_PreInputCmdBuf[INPUTBUF_LEN];
	uint8_t u8_InputCmdBuf[INPUTBUF_LEN];
	uint8_t u8_InputCnt;
	uint8_t u8_OutputCnt;
} inputCmdBuf;

#ifdef __cplusplus
extern "C" {
#endif

void CmdHandle_PrintfAll(void);
	
void pushInputCmd(uint8_t *inBuf, uint16_t len);

void Get_CurrentPlatTypeName(char *name);
void Get_CurrentCardTypeName(char *name);

#ifdef __cplusplus
}
#endif

#endif



