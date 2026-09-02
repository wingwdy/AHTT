/**********************************************************************************************************
  * FileName         : AppCommon.h
  * Author           : sjc
  * Version          : V1.0.0
  * Description      :
  * Date             : 2023.10.16 
**********************************************************************************************************/
#ifndef __APP_COMMON_H_
#define __APP_COMMON_H_

#include <stdint.h>

#define DEC_BIT_WAN			5
#define DEC_BIT_QIAN		4
#define DEC_BIT_BAI			3
#define DEC_BIT_SHI			2
#define DEC_BIT_GE			1


#ifdef __cplusplus
extern "C" {
#endif

extern uint16_t getCrc16(uint8_t *pucData, uint16_t uLen);
extern uint16_t foundString(const uint8_t *pucStr, const char *puctag, uint16_t uLen);
extern int MyStrstr(char *str, char *substr);

#ifdef __cplusplus
}
#endif

#endif
