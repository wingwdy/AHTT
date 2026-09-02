
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
#ifndef __GPRS_LIB_C__
#define __GPRS_LIB_C__


#include "Gprslib.h"
#include "SIM900A.h"
#include "common.h"
#include "Uart.h"

//GPRS/CDMA 数据打包函数
typedef const ModemItem* (*GetModemItem)(void);

const GetModemItem MODEMTABLE[] = { GetSIM900AOptItem };

//////////////////////////////////////////////////////////////////////////
//以下为外部调用函数
//////////////////////////////////////////////////////////////////////////
//函数名：		CmpModemAnswer
//功能描述：	对比会码
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U8 CmpModemAnswer(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	const ATCmdItem *pATCmd = GetCurrentATCmdItem(SocketID);
	U32 bAnswerOK = FALSE;
	char cExpect[AT_CMD_LEN] = {0};
	U8 *pDest = NULL;
	
	if (NULL == pATCmd) return bAnswerOK;
	//AT 应答
	//判断模块应答与预期回码对比
	if (strlen(pATCmd->cATAnswer) <= 0) return bAnswerOK;
	
	strcpy(cExpect, pATCmd->cATAnswer);
	
	pDest = SearchData(pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL == pDest) return bAnswerOK;
	
	//回码内容处理
	if(NULL == pATCmd->pRecv)
		bAnswerOK = TRUE;
	else
		bAnswerOK = pATCmd->pRecv(SocketID, pDest, nDataLen);
	
    
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;

    if (pATCmd->ATFunc != pModemDCB->ATOptTask->ATCmdTask) {
        return bAnswerOK;
    }
	if (TRUE == bAnswerOK)
	{
		ATDelOptDeal();
	}
	
	return bAnswerOK;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		MTRecvDecodeProc
//功能描述：	接收数据处理
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void MTRecvDecodeURC(eNetSocket SocketID, U8 *pData, U32 *pDataLen)
{
	U32 index;
	const ModemItem* pModem = GetCURModemItem();
	U32 u32Len = pDataLen[0];
	U8* pDealData = pData;
	U8* pDealHead = NULL;
	U32 u32DealLen = 0;
	
	//URC被动接收任务
	for (index = 0; index < pModem->UpNum; index++)
	{
		if (NULL != SearchData(pDealData, u32Len, (void*)pModem->pUpCmd[index].cAT,
			strlen(pModem->pUpCmd[index].cAT)))
		{
			pDealHead = NULL;
			u32DealLen = 0;
			pModem->pUpCmd[index].pFunc(SocketID, pDealData, u32Len, &pDealHead, &u32DealLen);
			//处理过得数据删除,暂不生效
			if((NULL != pDealHead) && (0 != u32DealLen) && (u32DealLen <= u32Len))
			{
//				memmove(pDealHead, (pDealHead + u32DealLen), u32DealLen);
//				u32Len -= u32DealLen;
			}
		}
	}
	
	return;
}

U32 MTRecvDecodeProc(eNetSocket SocketID, U8 *pData, U32 nDataLen)
{	
	//主动接收任务,同一时间只会执行一个任务,收到正确应答就要退出,防止多删除任务
	return CmpModemAnswer(SocketID, pData, nDataLen);
}

//////////////////////////////////////////////////////////////////////////
//函数名：		GetCurrentATCmdItem
//功能描述：	指定当前AT指令
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
const ATCmdItem* GetCurrentATCmdItem(eNetSocket SocketID)
{
	U32 index;
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	ATCmdDCB *pFirstOptDCB = GetModemFirstOptDCB();
	const ModemItem* pModem;
	
	//判断模块是否存在
	if (pModemDCB->ModemIndex >= FCNT(MODEMTABLE) || SocketID != pFirstOptDCB->SocketID)
	{
		return NULL;
	}
	
	pModem = GetCURModemItem();
	
	//判断模块是否需要操作
	while (pFirstOptDCB->ATCmdTask > eAT_Null)
	{
		for (index = 0; index < pModem->CmdNum; index++)
		{
			if (pModem->pATCmd[index].ATFunc == pFirstOptDCB->ATCmdTask
				&& SocketID == pFirstOptDCB->SocketID)
			{
				return &pModem->pATCmd[index];
			}
		}
		
		//移出无效操作
//		DelModemCurOptTask();
		
		break;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ATSuccedDeal
//功能描述：	拨号成功处理
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void ATDelOptDeal(void)
{	
	ATCmdWait(FALSE, 0);
	DelModemCurOptTask();
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		GetCURModemItem
//功能描述：	返回当前模块的处理结构
//入口参数：	无
//函数返回值：	ModemItem
//////////////////////////////////////////////////////////////////////////
const ModemItem* GetCURModemItem(void)
{
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
	return MODEMTABLE[pModemDCB->ModemIndex]();
}

//////////////////////////////////////////////////////////////////////////
//方法:		AddModemOptTask
//全名:		AddModemOptTask
//函数说明: 增加模块操作任务
//访问:		U32 optFunc
//参数:		无
//返回值:	无
//注释:
//////////////////////////////////////////////////////////////////////////
void AddModemOptTask(eNetSocket SocketID, AT_CMDOPT_E optCmd)
{
	U32 index;
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;

    if ((optCmd != eAT_DataRead) && (optCmd != eAT_DataWrite)) {
	    printf("AddModemOptTask: %d %d\r\n", SocketID, optCmd);
    }

	for (index = 0; index < AT_OPT_CNT; index++)
	{
		if (optCmd == pModemDCB->ATOptTask[index].ATCmdTask
			&& SocketID == pModemDCB->ATOptTask[index].SocketID)
		{
			return;
		}
		else 
			if (eAT_Null == pModemDCB->ATOptTask[index].ATCmdTask
			&& eSocket_Cnt == pModemDCB->ATOptTask[index].SocketID)
		{
			pModemDCB->ATOptTask[index].ATCmdTask = optCmd;
			pModemDCB->ATOptTask[index].SocketID = SocketID;
			return;
		}
	}
	
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		DelModemCurOptTask
//功能描述：	移出当前操作任务
//入口参数：	const ATCmdItem *pATCmd
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void DelModemCurOptTask(void)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	//移出无效操作
	memmove(pModemDCB->ATOptTask, pModemDCB->ATOptTask + 1, (AT_OPT_CNT - 1)*sizeof(ATCmdDCB));
//	memmove(pModemDCB->ATOptTask, pModemDCB->ATOptTask + 1, (AT_OPT_CNT - 1));
	pModemDCB->ATOptTask[AT_OPT_CNT - 1].ATCmdTask = eAT_Null;
	pModemDCB->ATOptTask[AT_OPT_CNT - 1].SocketID = eSocket_Cnt;
	
	return;
}

void ClrSocketModemOptTask(eNetSocket SocketID)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
    printf("ClrSocketModemOptTask\r\n");
    
	U32 t_start = SocketID * eAT_MAXCnt;
	U32 t_stop = (SocketID + 1) * eAT_MAXCnt;
	
	for(U32 i = t_start; i < t_stop; i++)
	{
		pModemDCB->ATOptTask[i].ATCmdTask = eAT_Null;
		pModemDCB->ATOptTask[i].SocketID = eSocket_Cnt;
	}
	
	return;
}

void ClrModemOptTask(void)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	U32 i = 0;
    printf("ClrModemOptTask\r\n");
	
	for(i = 0; i < AT_OPT_CNT; i++)
	{
		pModemDCB->ATOptTask[i].ATCmdTask = eAT_Null;
		pModemDCB->ATOptTask[i].SocketID = eSocket_Cnt;
	}
	
	return;
}

ATCmdDCB* GetModemFirstOptDCB(void)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	return &pModemDCB->ATOptTask[0];
}

#endif	//__GPRS_LIB_C__
