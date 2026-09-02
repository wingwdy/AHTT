
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

#include "Libqueue.h"
#include "common.h"

//协议解析发送缓存最大长度
#define Q1_TXBUF_MAXLEN      		(1024*2) 	//云快充2.1订单数据长度为1120字节  缓存由由1024改为2048
#define Q1_RXBUF_MAXLEN      		(1024*2)
#define Q2_TXBUF_MAXLEN      		(1024*2)
#define Q2_RXBUF_MAXLEN      		(1024*2)
#define Q3_TXBUF_MAXLEN      		(384)
#define Q3_RXBUF_MAXLEN      		(384)
//协议解析接收缓存最大长度
//#define Q1_RXBUF_MAXLEN      		(256)
static U8 U8Q1TXBuf[Q1_TXBUF_MAXLEN] = {0};		//Q1发送缓存
static U8 U8Q1RXBuf[Q1_RXBUF_MAXLEN] = {0};		//Q1接收缓存
static U8 U8Q2TXBuf[Q2_TXBUF_MAXLEN] = {0};		//Q2发送缓存
static U8 U8Q2RXBuf[Q2_RXBUF_MAXLEN] = {0};		//Q2接收缓存
static U8 U8Q3TXBuf[Q3_TXBUF_MAXLEN] = {0};		//Q3发送缓存
static U8 U8Q3RXBuf[Q3_RXBUF_MAXLEN] = {0};		//Q3接收缓存

//////////////////////////////////////////////////////////////////////////
//协议解析数据包头
typedef struct __PAL_DATAPKT_HEAD__
{
	U8 	U8CtrlWord[4];						//控制字
	
	U8  u8PalType;							//协议解析类型
	U8  U8Len[4];							//长度(mqtt 高2字节topic 低2字节load)
}PAL_DATAPKT_HEAD;

//协议解析收发控制
typedef struct __PAL_TRX_DCB__
{
	U8		u8SktType;						//每个socket的通讯类型,非初始化通讯类型不进行存储，避免异常

	U32		u32TXLen;						//发送数据长度
	U32		u32TXBufSize;					//发送缓存长度
	U8		*pTXBuf;						//发送缓存指针
	
	U32		u32RXLen;						//接收数据长度
	U32		u32RXBufSize;					//接收缓存长度
	U8		*pRXBuf;						//接收缓存指针
}PAL_TRX_DCB;

static PAL_TRX_DCB g_PalTRxDCB[eDataID_CNT];

static PAL_TRX_DCB* GetPalTRxDCB(eDataQueueID eDataID)
{
	if (eDataID >= eDataID_CNT)
		return NULL;
	
	return &g_PalTRxDCB[eDataID];
}


//////////////////////////////////////////////////////////////////////////
//名称:		PushPalRxBuf_
//功能:		插入协议解析接收缓冲区
//入口参数:	u32PalComID 协议解析口ID
//			u32PtlTypeCfg 协议类型配置
//			pData 数据指针
//			u32DataLen 数据长度
//出口参数:   	无
//返回:	    U32 TRUE/FASLE
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
static U32 PushPalRxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD strDataPktHead;
	U8 *pPtlData = (U8*)pData;
	U32 u32PtlDataLen = u32DataLen;
	U32 u32TempLen = 0;
	
	if (NULL == pPalTRxDCB)
		return FALSE;
	
	if (sizeof(PAL_DATAPKT_HEAD)+u32PtlDataLen > pPalTRxDCB->u32RXBufSize)
		return FALSE;
	
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	strDataPktHead.u8PalType = u8DataType;
	uint32ToFourUint8(strDataPktHead.U8Len, u32PtlDataLen);
	PackData(&strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
	u32TempLen += sizeof(PAL_DATAPKT_HEAD);
	
	u32TempLen += u32PtlDataLen;
	
	if(pPalTRxDCB->u32RXLen+u32TempLen <= pPalTRxDCB->u32RXBufSize)
	{
		memcpy(&pPalTRxDCB->pRXBuf[pPalTRxDCB->u32RXLen], &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pRXBuf[pPalTRxDCB->u32RXLen+sizeof(PAL_DATAPKT_HEAD)], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32RXLen += u32TempLen;
	}
	else
	{
		memset(pPalTRxDCB->pRXBuf, 0x00, pPalTRxDCB->u32RXBufSize);
		memcpy(pPalTRxDCB->pRXBuf, &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32RXLen = u32TempLen;
	}
	
	return TRUE;
}

static U32 MTPushPalRxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD strDataPktHead;
	U8 *pPtlTopic = (U8*)pTopic;
	U32 u32PtlTopicLen = u32TopicLen;
	U8 *pPtlData = (U8*)pData;
	U32 u32PtlDataLen = u32DataLen;
	U32 u32TempLen = 0;
	
	if (NULL == pPalTRxDCB)
		return FALSE;
	
	if (sizeof(PAL_DATAPKT_HEAD)+u32PtlDataLen+u32PtlTopicLen > pPalTRxDCB->u32RXBufSize)
		return FALSE;
	
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	strDataPktHead.u8PalType = u8DataType;
	uint32ToTwoUint8(&strDataPktHead.U8Len[0], u32PtlTopicLen);
	uint32ToTwoUint8(&strDataPktHead.U8Len[2], u32PtlDataLen);
	PackData(&strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
	u32TempLen += sizeof(PAL_DATAPKT_HEAD);
	
	u32TempLen += u32PtlTopicLen+u32PtlDataLen;
	
	if(pPalTRxDCB->u32RXLen+u32TempLen <= pPalTRxDCB->u32RXBufSize)
	{
		memcpy(&pPalTRxDCB->pRXBuf[pPalTRxDCB->u32RXLen], &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pRXBuf[pPalTRxDCB->u32RXLen+sizeof(PAL_DATAPKT_HEAD)], pPtlTopic, u32PtlTopicLen);
		memcpy(&pPalTRxDCB->pRXBuf[pPalTRxDCB->u32RXLen+sizeof(PAL_DATAPKT_HEAD)+u32PtlTopicLen], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32RXLen += u32TempLen;
	}
	else
	{
		memset(pPalTRxDCB->pRXBuf, 0x00, pPalTRxDCB->u32RXBufSize);
		memcpy(pPalTRxDCB->pRXBuf, &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)], pPtlTopic, u32PtlTopicLen);
		memcpy(&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+u32PtlTopicLen], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32RXLen = u32TempLen;
	}
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//名称:		PushPalRxBuf
//功能:		插入协议解析接收缓冲区
//入口参数:	u32PalComID 协议解析口ID
//			u32PtlTypeCfg 协议类型配置
//			pData 数据指针
//			u32DataLen 数据长度
//出口参数:   	无
//返回:	    U32 TRUE/FASLE
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
U32 PushPalRxBuf(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	U32 u32Rtr = FALSE;

	if (u8DataType != pPalTRxDCB->u8SktType) {
		printf("PushPalRxBuf erro.eDataID = %d  u8DataType = %d, %d\r\n",  eDataID, u8DataType, pPalTRxDCB->u8SktType);
		return u32Rtr;
	}
	
	if(eDataType_MQTT == u8DataType)
	{
		u32Rtr = MTPushPalRxBuf_(eDataID, u8DataType, pTopic, u32TopicLen, pData, u32DataLen);
	}
	else
	{
		u32Rtr = PushPalRxBuf_(eDataID, u8DataType, pData, u32DataLen);
	}
	
	return u32Rtr;
}

//////////////////////////////////////////////////////////////////////////
//名称:		PopPalRxBuf_
//功能:		从规约解析接收缓冲区取出数据
//入口参数:	u32PalComID 协议解析端口号
//			u32MaxSize 最大空间
//出口参数: pDataPktHead 数据包头
//			pOutBuf 输出缓冲区
//返回:	    U32 取出数据长度
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
static U32 PopPalRxBuf_(eDataQueueID eDataID, U8 u8DataType, PAL_DATAPKT_HEAD *pDataPktHead, void *pOutBuf, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32IsPop = FALSE;
	U32 len = 0;
	
	if (NULL==pPalTRxDCB)
		return 0;
	
	while(pPalTRxDCB->u32RXLen > sizeof(PAL_DATAPKT_HEAD))
	{
        pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pRXBuf[0];
		
		if (FALSE == CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
		{
			memmove(&pPalTRxDCB->pRXBuf[0] ,&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)],pPalTRxDCB->u32RXLen-sizeof(PAL_DATAPKT_HEAD));
			pPalTRxDCB->u32RXLen -= sizeof(PAL_DATAPKT_HEAD);	
			continue;
		}
		
		len = fourUint8ToUint32(pTempHead->U8Len);
		if (len > u32MaxSize)
		{
            printf("PopPalRxBuf erro--ID:%d len:%d MaxLen:%d\r\n", eDataID, len, u32MaxSize);
			memmove(&pPalTRxDCB->pRXBuf[0], &pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32RXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
			pPalTRxDCB->u32RXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);
			continue;
		}

		*pDataPktHead = *pTempHead;
		memcpy(pOutBuf,&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)], len);
		memmove(&pPalTRxDCB->pRXBuf[0] ,&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+len],pPalTRxDCB->u32RXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
		pPalTRxDCB->u32RXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);
		
		u32IsPop=TRUE;
		break;
	}
	
	if (FALSE==u32IsPop)
	{
		return 0;
	}
	return len;
}

static U32 MTPopPalRxBuf_(eDataQueueID eDataID, U8 u8DataType, PAL_DATAPKT_HEAD *pDataPktHead, void *pOutTopic, void *pOutBuf, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32IsPop = FALSE;
	U32 len = 0, u32TopicLen = 0, u32PayloadLen = 0;
	
	if (NULL==pPalTRxDCB)
		return 0;
	
	while(pPalTRxDCB->u32RXLen > sizeof(PAL_DATAPKT_HEAD))
	{
        pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pRXBuf[0];
		
		if (FALSE == CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
		{
			memmove(&pPalTRxDCB->pRXBuf[0] ,&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)],pPalTRxDCB->u32RXLen-sizeof(PAL_DATAPKT_HEAD));
			pPalTRxDCB->u32RXLen -= sizeof(PAL_DATAPKT_HEAD);	
			continue;
		}
		
		u32TopicLen = twoUint8ToUint16(&pTempHead->U8Len[0]);
		u32PayloadLen = twoUint8ToUint16(&pTempHead->U8Len[2]);
		len = u32TopicLen + u32PayloadLen;
		if (len > u32MaxSize)
		{
			memmove(&pPalTRxDCB->pRXBuf[0], &pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32RXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
			pPalTRxDCB->u32RXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);
			continue;
		}
		
		*pDataPktHead = *pTempHead;
		memcpy(pOutTopic, &pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)], u32TopicLen);
		memcpy(pOutBuf, &pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+u32TopicLen], u32PayloadLen);
		memmove(&pPalTRxDCB->pRXBuf[0] ,&pPalTRxDCB->pRXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32RXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
		pPalTRxDCB->u32RXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);
		
		u32IsPop = TRUE;
		break;
	}
	
	if (FALSE == u32IsPop)
	{
		return 0;
	}
	return len;
}

//////////////////////////////////////////////////////////////////////////
//名称:		PopPalRxBuf
//功能:		从规约解析接收缓冲区取出数据
//入口参数:	u32PalComID 协议解析端口号
//			u32MaxSize 最大空间
//出口参数: pDataPktHead 数据包头
//			pOutBuf 输出缓冲区
//返回:	    U32 取出数据长度
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
static U32 PopPalRxBuf(eDataQueueID eDataID, U8 u8DataType, PAL_DATAPKT_HEAD *pDataPktHead, void *pOutTopic, void *pOutBuf, U32 u32MaxSize)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	U32 u32Rtr = 0;
	
	if (u8DataType != pPalTRxDCB->u8SktType) {
		printf("PopPalRxBuf erro.eDataID = %d  u8DataType = %d, %d\r\n",  eDataID, u8DataType, pPalTRxDCB->u8SktType);
		return u32Rtr;
	}

	if(eDataType_MQTT == u8DataType)
	{
		u32Rtr = MTPopPalRxBuf_(eDataID, u8DataType, pDataPktHead, pOutTopic, pOutBuf, u32MaxSize);
	}
	else
	{
		u32Rtr = PopPalRxBuf_(eDataID, u8DataType, pDataPktHead, pOutBuf, u32MaxSize);
	}
	
	return u32Rtr;
}

U32 PalRecvPop(eDataQueueID eDataID, U8 u8DataType, void *pOutTopic, U16 *pTopicLen, void *pData, U16 *pDataLen, U32 u32MaxSize)
{
	PAL_DATAPKT_HEAD strDataPktHead;
	U32 len = 0;
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	PopPalRxBuf(eDataID, u8DataType, &strDataPktHead, pOutTopic, pData, u32MaxSize);
	
	if(NULL != pTopicLen)
		pTopicLen[0] = twoUint8ToUint16(&strDataPktHead.U8Len[0]);
	
	if(NULL != pDataLen)
	{
		if(eDataType_MQTT == u8DataType)
			pDataLen[0] = twoUint8ToUint16(&strDataPktHead.U8Len[2]);
		else
			pDataLen[0] = (U16)fourUint8ToUint32(strDataPktHead.U8Len);
	}
	
	if(NULL == pTopicLen)
		len = pDataLen[0];
	else
		len = (pTopicLen[0]+pDataLen[0]);
	return len;
}

U32 PalRecvPush(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen)
{
	PushPalRxBuf(eDataID, u8DataType, pTopic, u32TopicLen, pData, u32DataLen);
	
	return TRUE;
}

U32 PopPalRxLen(eDataQueueID eDataID, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	U32 len = 0;
	
	if (NULL == pPalTRxDCB)
		return 0;
	
	len = pPalTRxDCB->u32TXLen;
	if (len > u32MaxSize)
	{
		len = u32MaxSize;
	}
	return len;
}
//////////////////////////////////////////////////////////////////////////
//名称:		PushPalTxBuf_
//功能:		插入协议解析发送缓冲区
//入口参数:	u32PalComID 协议解析口ID
//			u32PtlTypeCfg 协议类型配置
//			pData 数据指针
//			u32DataLen 数据长度
//出口参数:   	无
//返回:	    U32 TRUE/FASLE
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
static U32 PushPalTxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD strDataPktHead;
	U8 *pPtlData = (U8*)pData;
	U32 u32PtlDataLen = u32DataLen;
	U32 u32TempLen = 0;
	
	if (NULL == pPalTRxDCB)
		return FALSE;
	
	if (u32TempLen+u32PtlDataLen > pPalTRxDCB->u32TXBufSize)
		return FALSE;
	
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	strDataPktHead.u8PalType = u8DataType;
	uint32ToFourUint8(strDataPktHead.U8Len, u32PtlDataLen);
	PackData(&strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
	u32TempLen += sizeof(PAL_DATAPKT_HEAD);	
	
	u32TempLen += u32PtlDataLen;
	
	if(pPalTRxDCB->u32TXLen+u32TempLen <= pPalTRxDCB->u32TXBufSize)
	{		
		memcpy(&pPalTRxDCB->pTXBuf[pPalTRxDCB->u32TXLen], &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pTXBuf[pPalTRxDCB->u32TXLen+sizeof(PAL_DATAPKT_HEAD)], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32TXLen += u32TempLen;
	}
	else
	{
		memset(pPalTRxDCB->pTXBuf, 0x00, pPalTRxDCB->u32TXBufSize);
		memcpy(pPalTRxDCB->pTXBuf, &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32TXLen = u32TempLen;
	}
	
	return TRUE;
}

static U32 MTPushPalTxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen,  void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	PAL_DATAPKT_HEAD strDataPktHead;
	U8 *pPtlTopic = (U8*)pTopic;
	U32 u32PtlTopicLen = u32TopicLen;
	U8 *pPtlData = (U8*)pData;
	U32 u32PtlDataLen = u32DataLen;
	U32 u32TempLen = 0;
	
	if (NULL == pPalTRxDCB)
		return FALSE;
	
	if (u32TempLen+u32PtlDataLen+u32PtlTopicLen > pPalTRxDCB->u32TXBufSize)
		return FALSE;
	
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	strDataPktHead.u8PalType = u8DataType;
	uint32ToTwoUint8(&strDataPktHead.U8Len[0], u32PtlTopicLen);
	uint32ToTwoUint8(&strDataPktHead.U8Len[2], u32PtlDataLen);
	PackData(&strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
	u32TempLen += sizeof(PAL_DATAPKT_HEAD);	
	
	u32TempLen += u32PtlDataLen+u32PtlTopicLen;
	
	if(pPalTRxDCB->u32TXLen+u32TempLen <= pPalTRxDCB->u32TXBufSize)
	{		
		memcpy(&pPalTRxDCB->pTXBuf[pPalTRxDCB->u32TXLen], &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
		memcpy(&pPalTRxDCB->pTXBuf[pPalTRxDCB->u32TXLen+sizeof(PAL_DATAPKT_HEAD)], pPtlTopic, u32PtlTopicLen);
		memcpy(&pPalTRxDCB->pTXBuf[pPalTRxDCB->u32TXLen+sizeof(PAL_DATAPKT_HEAD)+u32PtlTopicLen], pPtlData, u32PtlDataLen);
		pPalTRxDCB->u32TXLen += u32TempLen;
	}
	else
	{
		return FALSE;
//		memset(pPalTRxDCB->pTXBuf, 0x00, pPalTRxDCB->u32TXBufSize);
//		memcpy(pPalTRxDCB->pTXBuf, &strDataPktHead, sizeof(PAL_DATAPKT_HEAD));
//		memcpy(&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], pPtlTopic, u32PtlTopicLen);
//		memcpy(&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+u32PtlTopicLen], pPtlData, u32PtlDataLen);
//		pPalTRxDCB->u32TXLen = u32TempLen;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//名称:		PushPalTxBuf
//功能:		插入协议解析发送缓冲区
//入口参数:	u32PalComID 协议解析口ID
//			u32PtlTypeCfg 协议类型配置
//			pData 数据指针
//			u32DataLen 数据长度
//出口参数:   	无
//返回:	    U32 TRUE/FASLE
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
U32 PushPalTxBuf(eDataQueueID eDataID, U8 u8DataType, void *pTopic, U32 u32TopicLen, void *pData, U32 u32DataLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);
	U32 u32Rtr = FALSE;
	
	if(eDataType_MQTT == u8DataType)
	{
		u32Rtr = MTPushPalTxBuf_(eDataID, u8DataType, pTopic, u32TopicLen, pData, u32DataLen);
	}
	else
	{
		u32Rtr = PushPalTxBuf_(eDataID, u8DataType, pData,u32DataLen);	
	}
	
	return u32Rtr;
}

//////////////////////////////////////////////////////////////////////////
//名称:		PopPalTxBuf_
//功能:		从规约解析发送缓冲区取出数据
//入口参数:	u32PalComID 协议解析端口号
//			u32MaxSize 最大空间
//出口参数: pDataPktHead 数据包头
//			pOutBuf 输出缓冲区
//返回:	    U32 取出数据长度
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
static U32 PopPalTxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pOutBuf, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32IsPop = FALSE;
	U32 len = 0;
	
	if (NULL == pPalTRxDCB)
		return 0;
	
	while(pPalTRxDCB->u32TXLen > sizeof(PAL_DATAPKT_HEAD))
	{
        pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pTXBuf[0];
		
		if (FALSE==CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
		{
			memmove(&pPalTRxDCB->pTXBuf[0] ,&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)],pPalTRxDCB->u32TXLen-sizeof(PAL_DATAPKT_HEAD));
			pPalTRxDCB->u32TXLen -= sizeof(PAL_DATAPKT_HEAD);			
			continue;
		}
		
		len = fourUint8ToUint32(pTempHead->U8Len);
		if (len > u32MaxSize)
		{
			memmove(&pPalTRxDCB->pTXBuf[0], &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32TXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
			pPalTRxDCB->u32TXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);	
			continue;
		}
		
		memcpy(pOutBuf,&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], len);
		memmove(&pPalTRxDCB->pTXBuf[0], &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32TXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
		pPalTRxDCB->u32TXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);

		u32IsPop = TRUE;
		break;
	}
	
	if (FALSE == u32IsPop)
	{
		return 0;
	}
	return len;
}

static U32 MTPopPalTxBuf_(eDataQueueID eDataID, U8 u8DataType, void *pOutTopic, U16 *pTopicLen, void *pOutBuf, U16 *pOutBufLen, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32IsPop = FALSE;
	U32 len = 0;
	U16 u16Topiclen = 0, u16Datalen = 0;
	
	if (NULL == pPalTRxDCB)
		return 0;
	
	while(pPalTRxDCB->u32TXLen > sizeof(PAL_DATAPKT_HEAD))
	{
        pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pTXBuf[0];
		
		if (FALSE==CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
		{
			memmove(&pPalTRxDCB->pTXBuf[0] ,&pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)],pPalTRxDCB->u32TXLen-sizeof(PAL_DATAPKT_HEAD));
			pPalTRxDCB->u32TXLen -= sizeof(PAL_DATAPKT_HEAD);			
			continue;
		}
		
		u16Topiclen = twoUint8ToUint16(&pTempHead->U8Len[0]);
		u16Datalen = twoUint8ToUint16(&pTempHead->U8Len[2]);
		len = u16Topiclen + u16Datalen;
		if (len > u32MaxSize)
		{
			memmove(&pPalTRxDCB->pTXBuf[0], &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32TXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
			pPalTRxDCB->u32TXLen -= (sizeof(PAL_DATAPKT_HEAD)+len); 
			continue;
		}
		
		if(NULL != pOutTopic && NULL != pTopicLen)
		{
			pTopicLen[0] = u16Topiclen;
			memcpy(pOutTopic, &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], u16Topiclen);
		}
		if(NULL != pOutBuf && NULL != pOutBufLen)
		{
			pOutBufLen[0] = u16Datalen;
			memcpy(pOutBuf, &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+u16Topiclen], u16Datalen);
		}
		
		memmove(&pPalTRxDCB->pTXBuf[0], &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)+len], pPalTRxDCB->u32TXLen-(sizeof(PAL_DATAPKT_HEAD)+len));
		pPalTRxDCB->u32TXLen -= (sizeof(PAL_DATAPKT_HEAD)+len);

		u32IsPop = TRUE;
		break;
	}
	
	if (FALSE == u32IsPop)
	{
		return 0;
	}
	return len;
}


//////////////////////////////////////////////////////////////////////////
//名称:		PopPalTxBuf
//功能:		从规约解析发送缓冲区取出数据
//入口参数:	u32PalComID 协议解析端口号
//			u32MaxSize 最大空间
//出口参数: pDataPktHead 数据包头
//			pOutBuf 输出缓冲区
//返回:	    U32 取出数据长度
//作者:	    YQ-2019-02-28
//修改说明:
//////////////////////////////////////////////////////////////////////////
U32 PopPalTxBuf(eDataQueueID eDataID, U8 u8DataType, void *pOutTopic, U16 *pTopicLen, void *pOutBuf, U16 *pOutBufLen, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	U32 u32Rtr = 0;
	
	if(eDataType_MQTT == u8DataType)
	{
		u32Rtr = MTPopPalTxBuf_(eDataID, u8DataType, pOutTopic, pTopicLen, pOutBuf, pOutBufLen, u32MaxSize);
	}
	else
	{
		u32Rtr = PopPalTxBuf_(eDataID, u8DataType, pOutBuf, u32MaxSize);
		pOutBufLen[0] = u32Rtr;
	}
	
	return u32Rtr;
}


U32 PopPalTxLen(eDataQueueID eDataID, U32 u32MaxSize)
{	
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32IsPop = FALSE;
	U32 len = 0;
	
	if (NULL == pPalTRxDCB)
		return 0;
	
	pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pTXBuf[0];
	
	len = fourUint8ToUint32(pTempHead->U8Len);
	if (len > u32MaxSize)
	{
		len = u32MaxSize;
	}
	return len;
}


U32 MTTxTopicData(eDataQueueID eDataID, U8 *pTopic, U32 u32Size, U32 *pPayloadLen)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U16 u16TopicLen = 0;
	
	if (NULL == pPalTRxDCB || NULL == pTopic)
		return 0;
	
	while(pPalTRxDCB->u32TXLen > sizeof(PAL_DATAPKT_HEAD))
	{
        pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pTXBuf[0];
		
		if (FALSE == CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
		{
			memmove(&pPalTRxDCB->pTXBuf[0], &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], pPalTRxDCB->u32TXLen-sizeof(PAL_DATAPKT_HEAD));
			pPalTRxDCB->u32TXLen -= sizeof(PAL_DATAPKT_HEAD);			
			continue;
		}
		
		u16TopicLen = twoUint8ToUint16(&pTempHead->U8Len[0]);
		pPayloadLen[0] = twoUint8ToUint16(&pTempHead->U8Len[2]);
		if(u16TopicLen < u32Size)
		{
			memcpy(pTopic, &pPalTRxDCB->pTXBuf[sizeof(PAL_DATAPKT_HEAD)], u16TopicLen);
		}
		else
		{
			u16TopicLen = 0;
		}
		break;
	}
	
	return u16TopicLen;
}

U32 QueueIsTxData(eDataQueueID eDataID, U8 u8DataType)
{
	PAL_TRX_DCB *pPalTRxDCB = GetPalTRxDCB(eDataID);	
	PAL_DATAPKT_HEAD *pTempHead = NULL;
	U32 u32Len = 0;
	
	if (NULL == pPalTRxDCB)
		return 0;
	
	if(0 == pPalTRxDCB->u32TXLen)
		return 0;
	
	pTempHead = (PAL_DATAPKT_HEAD*)&pPalTRxDCB->pTXBuf[0];
	
	if (FALSE == CheckDataPack(pTempHead, sizeof(PAL_DATAPKT_HEAD)))
	{
		return 0;
	}
	
	if(u8DataType != pTempHead->u8PalType)
		return 0;
	
	if(eDataType_MQTT == u8DataType)
	{
		u32Len = (twoUint8ToUint16(&pTempHead->U8Len[0]) + twoUint8ToUint16(&pTempHead->U8Len[2]));
	}
	else
	{
		u32Len = fourUint8ToUint32(pTempHead->U8Len);
	}
	
	return u32Len;
}


/*
U32 GPRSPopTxData(eNetSocket SocketID, U8 u8DataType, void *pOutTopic, void *pOutBuf,U32 u32MaxSize)
{
	PAL_DATAPKT_HEAD strDataPktHead;
	
	memset(&strDataPktHead, 0, sizeof(PAL_DATAPKT_HEAD));
	
	return PopPalTxBuf(SocketID, u8DataType, &strDataPktHead, pOutTopic, pOutBuf, u32MaxSize);
}*/

void PalQueueInit(eDataQueueID eDataID)
{
	PAL_TRX_DCB *pPalTRxDCB = NULL;
	
	if(eDataID >= eDataID_CNT)
		return;
	
	pPalTRxDCB = &g_PalTRxDCB[eDataID];
	memset(pPalTRxDCB, 0, sizeof(PAL_TRX_DCB));
	
	if(eDataID_1 == eDataID)
	{
		pPalTRxDCB->u8SktType = eDataType_TCP;
		memset(U8Q1TXBuf, 0, Q1_TXBUF_MAXLEN);
		memset(U8Q1RXBuf, 0, Q1_RXBUF_MAXLEN);
		pPalTRxDCB->u32TXBufSize = Q1_TXBUF_MAXLEN;
		pPalTRxDCB->pTXBuf = U8Q1TXBuf;
		pPalTRxDCB->u32RXBufSize = Q1_RXBUF_MAXLEN;
		pPalTRxDCB->pRXBuf = U8Q1RXBuf;
	}
	else if(eDataID_2 == eDataID)
	{
		pPalTRxDCB->u8SktType = eDataType_TCP;
		memset(U8Q2TXBuf, 0, Q2_TXBUF_MAXLEN);
		memset(U8Q2RXBuf, 0, Q2_RXBUF_MAXLEN);
		pPalTRxDCB->u32TXBufSize = Q2_TXBUF_MAXLEN;
		pPalTRxDCB->pTXBuf = U8Q2TXBuf;
		pPalTRxDCB->u32RXBufSize = Q2_RXBUF_MAXLEN;
		pPalTRxDCB->pRXBuf = U8Q2RXBuf;
	}
	else if(eDataID_3 == eDataID)
	{
		pPalTRxDCB->u8SktType = eDataType_TCP;
		memset(U8Q3TXBuf, 0, Q3_TXBUF_MAXLEN);
		memset(U8Q3RXBuf, 0, Q3_RXBUF_MAXLEN);
		pPalTRxDCB->u32TXBufSize = Q3_TXBUF_MAXLEN;
		pPalTRxDCB->pTXBuf = U8Q3TXBuf;
		pPalTRxDCB->u32RXBufSize = Q3_RXBUF_MAXLEN;
		pPalTRxDCB->pRXBuf = U8Q3RXBuf;
	}
	
	return;
}


