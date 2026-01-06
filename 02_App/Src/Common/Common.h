/******************************************************************************
* File Name          : Common.h
* Description        : Code for Common function
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef COMMON_H_
#define COMMON_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "gd32e50x.h"
#include "Global.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
uint16_t Common_CalcCRC16(uint8_t *pData, uint16_t dataLen);
void Common_InsertSort(uint16_t *pData, uint16_t n);
void Common_BubbleSort(uint16_t *pData, uint16_t size);
uint8_t Common_JudgeTimeoutMs(uint32_t startTick, uint32_t threshold);
uint16_t Common_MedianU16Filter(uint16_t *pData, uint16_t sample_num, uint16_t discard_num);
uint32_t Common_GetSystick(void);
void Common_Uint32ToFourUint8(uint8_t *pData, uint32_t curVal);
void Common_Uint32ToTwoUint8(uint8_t *pData, uint32_t curVal);
uint32_t Common_FourUint8ToUint32(uint8_t *pData);
uint16_t Common_TwoUint8ToUint16(uint8_t *pData);
void Common_Uint16ToTwoUint8(uint8_t *pData, uint16_t curVal);

uint8_t* Common_SearchData(uint8_t *pData, uint16_t dataLen, void *pString, uint16_t stringLen);
uint16_t Common_ReplaceStr(uint8_t* pData, uint16_t nDataLen, char* cDestStr, void* pReplace, uint16_t nReplaceLen, char* pDefault);
uint16_t Common_ReplaceNum(uint8_t* pData, uint16_t nDataLen, char* cDestStr, uint16_t replace, uint16_t defaultNum);
#endif /* COMMON_H_ */


























