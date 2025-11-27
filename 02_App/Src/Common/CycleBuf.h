/******************************************************************************
* File Name          : CycleBuf.h
* Description        : Code for Circular memory management algorithm
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
#ifndef CYCLEBUF_H_
#define CYCLEBUF_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define  CYCLEBUF_MAX_CHANNEL_COUNT                    10

#define  CYCLEBUF_INVALID_ID                           (0xFFu)

#define  CYCLEBUF_PROFILE_CIRCLE                       0
#define  CYCLEBUF_PROFILE_SINGLE                       1

#define  CYCLEBUF_ENTER_CRITICAL_AREA()                uint32_t primask = __get_PRIMASK(); __set_PRIMASK(1)
                                        
#define  CYCLEBUF_EXIT_CRITICAL_AREA()                 __set_PRIMASK(primask)

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
void CycleBuf_Init(void);
GlobalRet_Enum CycleBuf_ResetBuf(uint8_t channel);
GlobalRet_Enum CycleBuf_CheckDataLen(uint8_t channel, uint16_t* pRemainLen);
GlobalRet_Enum CycleBuf_CheckDataLenIsr(uint8_t channel, uint16_t* pRemainLen);
GlobalRet_Enum CycleBuf_PreviewReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize);
GlobalRet_Enum CycleBuf_ReadData(uint8_t channel, uint8_t * pOutData, uint16_t readSize);
GlobalRet_Enum CycleBuf_ReadDataIsr(uint8_t channel, uint8_t * pOutData, uint16_t readSize);
GlobalRet_Enum CycleBuf_WriteData(uint8_t channel, uint8_t * pSrcData, uint16_t dataSize);
GlobalRet_Enum CycleBuf_WriteDataIsr(uint8_t channel, uint8_t * pSrcData, uint16_t dataSize);
GlobalRet_Enum CycleBuf_CreatChannel(uint8_t* pChannel, uint8_t* pDataBuf, uint32_t bufSize, uint8_t porfile);
GlobalRet_Enum CycleBuf_RemoveData(uint8_t channel, uint16_t dataLen);

#endif /* CYCLEBUF_H_ */






















