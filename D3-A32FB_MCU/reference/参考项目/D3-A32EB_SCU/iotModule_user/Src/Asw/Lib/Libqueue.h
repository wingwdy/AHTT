//////////////////////////////////////////////////////////////////////////
//Copyright (C), 2010, 
//文件名：		GprsLib.h
//作者:			
//版本号:       V1.0
//创建日期:		2010-01-28
//说明:			模块操作
//函数列表:		
//修改记录:
//////////////////////////////////////////////////////////////////////////
#ifndef __LIB_QUEUE_H__
#define __LIB_QUEUE_H__

#include "globals.h"

enum {
    eDataType_TCP     	= 0,		//
    eDataType_MQTT,    				//
};

//#define eDataID_Socket1		((U8)eSocket_GPRS1)	//0
//#define eDataID_Socket2		((U8)eSocket_GPRS2)	//1
#define eDataID_Socket1		(0)	//0
#define eDataID_Socket2		(1)	//1
#define eDataID_Ble			2					//蓝牙

typedef enum {
    eDataID_1     	= eDataID_Socket1,		//
    eDataID_2     	= eDataID_Socket2,		//
    eDataID_3     	= eDataID_Ble,		//
    eDataID_CNT,    				//
}eDataQueueID;

U32 PalRecvPop(eDataQueueID eDataID, U8 u8DataType, void *pOutTopic, U16 *pTopicLen, void *pData, U16 *pDataLen, U32 u32MaxSize);

U32 PalRecvPush(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen);

U32 PushPalTxBuf(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen);

U32 QueueIsTxData(eDataQueueID eDataID, U8 u8DataType);
U32 PopPalTxBuf(eDataQueueID eDataID, U8 u8DataType, void *pOutTopic, U16 *pTopicLen, void *pOutBuf, U16 *pOutBufLen, U32 u32MaxSize);

U32 PopPalTxLen(eDataQueueID eDataID, U32 u32MaxSize);
U32 PopPalRxLen(eDataQueueID eDataID, U32 u32MaxSize);

U32 MTTxTopicData(eDataQueueID eDataID, U8 *pTopic, U32 u32Size, U32 *pPayloadLen);

void PalQueueInit(eDataQueueID eDataID);

#endif
