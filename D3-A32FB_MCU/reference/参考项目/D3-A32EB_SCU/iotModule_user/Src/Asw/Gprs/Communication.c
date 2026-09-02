//////////////////////////////////////////////////////////////////////////
//Copyright (C), 2010, 
//文件名：		Communication.h
//作者:			
//版本号:       V1.0
//创建日期:		2010-01-28
//说明:			GPRS/CDMA 通讯
//函数列表:		
//修改记录:
//				2010-06-15 
//////////////////////////////////////////////////////////////////////////

#define  COMMUNICATION_GLONALS

#include "Communication.h"
#include "Gprslib.h"
#include "SIM900A.h"
#include "Uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ProtoLayerHeaderSummary.h"


#define CommunicationPintf(level,fmt,args...)	\
		do {							\
            debug("\r\nCommunicationPintf: ");      \
            debugL(level, fmt ,##args); 	\
		} while(0)


//捷顺ca证书
const char JS_cacert_pem[] =
"-----BEGIN CERTIFICATE-----\r\nMIIHGDCCBQCgAwIBAgIJAILJM+YTdEGKMA0GCSqGSIb3DQEBDQUAMIGxMQswCQYD\r\nVQQGEwJDTjESMBAGA1UECAwJR3VhbmdEb25nMREwDwYDVQQHDAhTaGVuWmhlbjFD\r\nMEEGA1UECgw6U2hlblpoZW4gSmllc2h1biBTY2llbmNlIGFuZCBUZWNobm9sb2d5\r\nIEluZHVzdHJ5IENvLiwgTHRkLjEcMBoGA1UECwwTU3RhbmRhcmQgRGVwYXJ0bWVu\r\ndDEYMBYGA1UEAwwPSmllU2h1biBSb290IENBMCAXDTIyMDIxNTAyMDg1OVoYDzIx\r\nMjIwMTIyMDIwODU5WjCBsTELMAkGA1UEBhMCQ04xEjAQBgNVBAgMCUd1YW5nRG9u\r\nZzERMA8GA1UEBwwIU2hlblpoZW4xQzBBBgNVBAoMOlNoZW5aaGVuIEppZXNodW4g\r\nU2NpZW5jZSBhbmQgVGVjaG5vbG9neSBJbmR1c3RyeSBDby4sIEx0ZC4xHDAaBgNV\r\nBAsME1N0YW5kYXJkIERlcGFydG1lbnQxGDAWBgNVBAMMD0ppZVNodW4gUm9vdCBD\r\nQTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK+EFJmswFcXHWXrPLW1\r\nvesse5ocZdAFsuJCPa9ykv+98TjdhQMNYfKhQe1gvHxmgq1ENJzdolGO7lbloYXp\r\n6bvrpIF8J7KzRQlk/laMvGPPTIvtjukEQ0+9GItoQxXwpWDnKoo21R3jwtdXa+bA\r\nMBaqmDB8aErXMvQ4eXso+OWPIZEBL3GnEXiNH8sGg/fFe0SpkhiA2GfslapNueNH\r\n2Dph4tKe3XQ7xT9hDifUAe/ALP4oRicYhmfb1Is0wXzNuC5HddcJLLQMMcJW+yqU\r\nWAJfA6sDBeyj1RQP9oY6LMHsHVt2OuBcao/WOiGkdxz6Pf5Ba8U1KmUhdbgqJPJ/\r\n7pZieXpDxABknc2yC+Eiuwe6EgYdWMNNVIg/e5sOwI72eoRWWjRICD3vyJ5/2Zj8\r\nJS0o5Ma7H+jC5aoBzLV1euBrdYPzq5OEKvkBMSQtMTIysOICGLp3pp07pv1iPRbu\r\nWpEHGZZILJFpZ4oujHsPsKVPW7L7GA3VcC/qolMGTPQF8Q2LE2Kuj3h4cH47BtFj\r\nHi0tJ4C29yb10/2DABexzH63VNmwJPp2O5XzHhGCGwjigkzY1+mhQD8WP1UouXAI\r\nvEJ1W0LmvfO0B2VZ5M34kbuDrTQCEJGaZOZMSO590HKt+6lWaK3G21yMJgTvGbbH\r\n/1GOXAi5nHP9+QTXo7gvfE17AgMBAAGjggEtMIIBKTAdBgNVHQ4EFgQUB2HJpfkD\r\nxyXYxI+3356XWLxbFr0wgeYGA1UdIwSB3jCB24AUB2HJpfkDxyXYxI+3356XWLxb\r\nFr2hgbekgbQwgbExCzAJBgNVBAYTAkNOMRIwEAYDVQQIDAlHdWFuZ0RvbmcxETAP\r\nBgNVBAcMCFNoZW5aaGVuMUMwQQYDVQQKDDpTaGVuWmhlbiBKaWVzaHVuIFNjaWVu\r\nY2UgYW5kIFRlY2hub2xvZ3kgSW5kdXN0cnkgQ28uLCBMdGQuMRwwGgYDVQQLDBNT\r\ndGFuZGFyZCBEZXBhcnRtZW50MRgwFgYDVQQDDA9KaWVTaHVuIFJvb3QgQ0GCCQCC\r\nyTPmE3RBijAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG\r\n9w0BAQ0FAAOCAgEAArEBo9yyUPFTe2kVvUqvmjvmpGoJC/q0KiQ6DvGTMXALASAR\r\n6TtC6ljqHuxsSHT2H08SuDWShlQNQx9qzgyBfMPK0i2NZEZjB92e1wbl4PDIda/C\r\nVXVKfYVVFhlW89v08fO5nEEM80RDIE177GcwmVss/JmyP10Otjr090h9T1KJIqUV\r\n4hBnQAyiWYAYAGNRhhx68DmEeayLpBnVRkQAH7VLnnFqYzGc8V+33v56pXCplt3I\r\nf0beNH3PR8EYDvWWCgqcwZ7aVfiqi6ynYbHsrLGuODEShrh96Fzo3kLrq2hn0EYD\r\nKOrfyzxpLp37VLsLu94+Pjy24auDga1IBVnkAxNYSbzapJQjEtKFJ7w6RGeNNUY0\r\nD0Et8krWFMcy5hqusdNZYFhGrju1XDqffK4VZcBAi++FGFCib7P/OvGNBllesYzG\r\nzGaAo+ata3qcSOaYEI6VaUvtWXBC2R+JGIn2Na+zI1L4muc05ccaO/tFciJYRS2B\r\n11JJ4NBjKZX5mPBJtXfNH2f0RlLHkbiP/ADBpl/sXqcGvMhEEADme62II0KqErM5\r\n4N5ab0x2CcS+T/mCWlQ9UyPhWe47OglO8QefEYfEfnrQyTpchet39FKBQiYRQ8pE\r\nQW7/8JKWGJNlB2WBMJn0Q2RtT6NTxFkeBmLySiEJWlT4c1WCTnXSb2TJZo4=\r\n-----END CERTIFICATE-----\r\n";


GprsModemDCB g_strGprsModemDCB;			//调制解调器控制数据


static void GprsOptTaskInit(void)
{	
	ClrModemOptTask();
	return;
}
static U8 ATMqttConn(eNetSocket SocketID)
{
	if(ePlatType_GNIOT == get_ChgParam_plat_type())
	{
		AddModemOptTask(SocketID, eAT_MTCfgDNS);		//
		AddModemOptTask(SocketID, eAT_MTCfgHeartbeat); //
		AddModemOptTask(SocketID, eAT_MTCfgKeepalive); //
		AddModemOptTask(SocketID, eAT_MTRecvMode); 	//
		AddModemOptTask(SocketID, eAT_MTAliCfg);		//
		AddModemOptTask(SocketID, eAT_MTAliQuery); 	//
		AddModemOptTask(SocketID, eAT_MTOPEN); 		//
		AddModemOptTask(SocketID, eAT_MTCONN); 		//
		AddModemOptTask(SocketID, eAT_MTSUBAck);		//
		AddModemOptTask(SocketID, eAT_MTSUBGet);		//
		AddModemOptTask(SocketID, eAT_MTSUBOta);		//
		AddModemOptTask(SocketID, eAT_MTSUBUserGet);	//
		AddModemOptTask(SocketID, eAT_MTSUBNotify);	//
	}
	else if(ePlatType_JSIOT == get_ChgParam_plat_type())
	{
		AddModemOptTask(SocketID, eAT_MTCfgHeartbeat); //
		AddModemOptTask(SocketID, eAT_MTCfgKeepalive); //
		AddModemOptTask(SocketID, eAT_MTRecvMode); 	//
		AddModemOptTask(SocketID, eAT_MTSsl); 	//
		AddModemOptTask(SocketID, eAT_UFSDelete);			//
		AddModemOptTask(SocketID, eAT_QFUpload);			//
		AddModemOptTask(SocketID, eAT_QFUploadAck);			//
		AddModemOptTask(SocketID, eAT_MTOPEN); 		//
		AddModemOptTask(SocketID, eAT_MTCONN); 		//
		AddModemOptTask(SocketID, eAT_JSSUBDown);		//
		AddModemOptTask(SocketID, eAT_JSSUBDownReply);		//
	}
	else
	{
		AddModemOptTask(SocketID, eAT_MTCfgHeartbeat); //
		AddModemOptTask(SocketID, eAT_MTCfgKeepalive); //
		AddModemOptTask(SocketID, eAT_MTRecvMode); 	//
		AddModemOptTask(SocketID, eAT_MTOPEN); 		//
		AddModemOptTask(SocketID, eAT_MTCONN); 		//
	}
	
	return TRUE;
}
static U8 ATFtpConn(eNetSocket SocketID)
{
    AddModemOptTask(eSocket_GPRS1, eAT_FTPFlashQuery); 	//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPConfig); 		//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPNamePsw);		//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPFileType);	//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPTransType);	//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPTimeout);		//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPIpPort);		//
    AddModemOptTask(eSocket_GPRS1, eAT_FTPPath);		//
    AddModemOptTask(eSocket_GPRS1, eAT_UFSDelete);		//
	
	return TRUE;
}
U32 GPRSIsTxData(eNetSocket SocketID)
{
	U32 len = 0;
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	len = PopPalTxLen((eDataQueueID)SocketID, pUartDCB->u16TxBufSize);
	// LogPrintf(LVL_LOG_INFO, "GPRSIsTxData: len = %d \r\n%s", len);
	return len;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ATCmd
//功能描述：	发送AT命令
//入口参数：	const ATCmdItem *pATCmd
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static U8 ATCmd(eNetSocket SocketID, const ATCmdItem *pATCmd)
{
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	U32 nDataLen = 0;
	
	if(eUart_Idle != pUartDCB->RxState)
		return FALSE;
	
	memset(pUartDCB->pTxBuffer, 0x00, pUartDCB->u16TxBufSize);
	
	strcpy((char *)pUartDCB->pTxBuffer, pATCmd->cAT);
	nDataLen = strlen(pATCmd->cAT);
	
	if(NULL != pATCmd->pSend)
	{
		//填充字段替换
		nDataLen = pATCmd->pSend(SocketID, pUartDCB->pTxBuffer, strlen(pATCmd->cAT));
	}
	
	ATCmdWait(TRUE, pATCmd->waitDelay);
	if (nDataLen > 0)
	{	
		LogPrintf(LVL_LOG_INFO, "%s", pUartDCB->pTxBuffer);
		//发送数据
		UartSendData(UART_GPRS, nDataLen);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemCsqCheck
//功能描述：	CSQ检测
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static void ModemCsqCheck(void)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	if(TRUE == JudgeTimeOutMs(pGprsModemDCB->ATCsqTick, eTick_30S))
	{
		pGprsModemDCB->ATCsqTick = NOWTICK;
		AddModemOptTask(eSocket_GPRS1, eAT_CSQQuery);
	}
	
	return;
}
//////////////////////////////////////////////////////////////////////////
//函数名：		ModemAbnormalCheck
//功能描述：	模块异常检测，2分钟无任何数据必须要重启模块
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static void ModemAbnormalCheck(void)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	if(TRUE == JudgeTimeOutMs(pGprsModemDCB->ATNormalTime, 2*eTick_60S))
	{
        pGprsModemDCB->ATAbnormal = eATStatus_Abnormal;
	}
	
	return;
}

/*************************************************************************************
 *函数名：		ModemAbnormalReboot
 *功能描述：	网络重启
 *入口参数：	无
 *函数返回值：	无
 *作者：	    maxy
 *说明：        区分重连和重启
*************************************************************************************/
static void ModemAbnormalReboot(void)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	if(pGprsModemDCB->ATAbnormal == eATStatus_NULL) {
	    AddModemOptTask(eSocket_GPRS1, eAT_PowerOn);		//模块开机
        pGprsModemDCB->ATAbnormal = eATStatus_Normal;
	} else if(pGprsModemDCB->ATAbnormal == eATStatus_Normal) {
        pGprsModemDCB->strATMDData.RebootSta = eModelRebootsta_ATReconnect;
        StartModemReOpenTask();
    } else {
        pGprsModemDCB->strATMDData.RebootSta = eModelRebootsta_ModelReboot;
        printf("AT Shutdown\r\n");
        pGprsModemDCB->ATAbnormal = eATStatus_Normal;
        pGprsModemDCB->ATNormalTime = NOWTICK;
	    AddModemOptTask(eSocket_GPRS1, eAT_Shutdown);		//模块关机
    }
	
	return;
}
/*************************************************************************************
 *函数名：		AddModemGetRealTimeClk
 *功能描述：	获取时钟指令
 *入口参数：	无
 *函数返回值：	无
 *作者：	    maxy
 *说明：        联通和电信基站发送时间数据不对，取ntp服务器取数据，但太慢，所以移动还是基站下发
*************************************************************************************/
static void AddModemGetRealTimeClk(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
    

	if(ePlatType_gwYKC== get_ChgParam_plat_type())
	 return;

    AddModemOptTask(eSocket_GPRS1, eAT_NTPClkQuery);

    // if (pATMDData->OperatorType == eOperator_CMCC) {
	//     AddModemOptTask(eSocket_GPRS1, eAT_ClkQuery);			//时钟查询
    // } else {
	//     AddModemOptTask(eSocket_GPRS1, eAT_NTPClkQuery);
    // }
}

static uint32_t GetRealTimeClkInterval(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
    uint32_t clkInterval = 10*eTick_60S;
    
    if (GetRealTimeSucces() == 0) {
        clkInterval = 10*eTick_1S;
    // } else {
    //     if (pATMDData->OperatorType != eOperator_CMCC) {
    //         clkInterval = 1*eTick_60S;
    //     }
    }
    return clkInterval;
}


/*************************************************************************************
 *函数名：		ModemClkCheck
 *功能描述：	时钟
*************************************************************************************/
static void ModemClkCheck(void)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;
	
	if(ePlatType_JSIOT == get_ChgParam_plat_type())
		return;
	
    uint32_t clkInterval = GetRealTimeClkInterval();
    
	if(TRUE == JudgeTimeOutMs(pGprsModemDCB->ATClkTick, clkInterval))
	{
		pGprsModemDCB->ATClkTick = NOWTICK;
        AddModemGetRealTimeClk();
	}
	
	return;
}

/*************************************************************************************
 *函数名：		ModemConnectCheck
 *功能描述：	建立tcp链接，附着基站后
*************************************************************************************/
static void ModemConnectTCPTask(eNetSocket SocketID)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
    if (OtaGetUpdatingFlag()) {
		pSocketDCB->ATConnTick = NOWTICK;
        return;
    }
    if (eSocket_Attach != GPRS_Getonlineflag(SocketID)) {
		pSocketDCB->ATConnTick = NOWTICK;
    }

	if(eSocket_TCP != pSocketDCB->SocketType)
		return;
	
	//附着基站之后连服务器
	if(eSocket_Attach == GPRS_Getonlineflag(SocketID))
	{
		if(TRUE == JudgeTimeOutMs(pSocketDCB->ATConnTick, eTick_3S))
		{
			pSocketDCB->ATConnTick = NOWTICK;
			AddModemOptTask(SocketID, eAT_ConnectSet);
		}
	}
	
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemMTStateCheck
//功能描述：	mqtt状态
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static void ModemMTStateCheck(eNetSocket SocketID)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	if(eSocket_MQTT != pGprsModemDCB->strSocketDCB[SocketID].SocketType)
		return;
	
	if(eSocket_Attach != GPRS_Getonlineflag(SocketID))
		return;
	
	if(TRUE == JudgeTimeOutMs(pGprsModemDCB->strMqttDCB.ATMTStateTick, eTick_30S))
	{
		pGprsModemDCB->strMqttDCB.ATMTStateTick = NOWTICK;
        
        if(ePlatType_JSIOT == get_ChgParam_plat_type() && JS_LOGIN_PARAM_OK != pPlatParam->platInfo.JS_Info.u8JSParamStep)
        {
            Comm_JSHttpsParamStart();
        }
        ATMqttConn(eSocket_GPRS1);
		AddModemOptTask(SocketID, eAT_MTQueryConn);
	}
	
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemFTPStateCheck
//功能描述：	ftp升级需要
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static void ModemFTPStateCheck(eNetSocket SocketID)
{
    static uint32_t s_ftpTick[eSocket_Cnt] = {0};
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	if(eSocket_FTP != pGprsModemDCB->strSocketDCB[SocketID].SocketType) {
		s_ftpTick[SocketID] = NOWTICK;
		return;
    }
	
	if(eSocket_Attach != GPRS_Getonlineflag(SocketID))
		return;
	
	if(TRUE == JudgeTimeOutMs(s_ftpTick[SocketID], UPDATE_TIMEOUT_MS))
	{
		s_ftpTick[SocketID] = NOWTICK;
		AddModemOptTask(eSocket_GPRS1, eAT_FileClose);		//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPClose);		//
        ftpSwitchTcp(eSocket_GPRS1);
	}
	
	return;
}


//////////////////////////////////////////////////////////////////////////
//函数名：		MTGPRSDrvServer
//功能描述：	GPRS模块驱动服务主要完成模块公用的功能和进入相应分模块
//入口参数：	无
//函数返回值：	无
//说明：		1.每秒钟调用一次
//////////////////////////////////////////////////////////////////////////
static U8 MTGPRSServerDelay(eNetSocket SocketID, const ATCmdItem *pATCmd)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	//AT指令等待延时
	if(TRUE == pGprsModemDCB->ATWaitFlag)
	{
		if(TRUE == JudgeTimeOutMs(pGprsModemDCB->ATWaitTick, pGprsModemDCB->ATWaitDelay))
		{
			//如AT指令尝试次数为1次或者无错误处理，模块不应答继续执行以下AT指令
			if(1 == pATCmd->sendCnt || NULL == pATCmd->pFail)
			{
				ATDelOptDeal();
			}
			else
			{
				pGprsModemDCB->ATTryCnt++;
				if(pGprsModemDCB->ATTryCnt < pATCmd->sendCnt)
					return FALSE;
				
				ATDelOptDeal();
				pGprsModemDCB->ATTryCnt = 0;
				pATCmd->pFail(SocketID, pATCmd->ATFunc);
			}
		}
		return TRUE;
	}
	
	return FALSE;
}

static U8 MTGPRSCmdTask(void)
{
	
	ModemCsqCheck();		//信号强度查询任务
	ModemClkCheck();		//时钟查询任务
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemDataQuery
//功能描述：	周期性的抄读模块数据
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
//static U8 ModemDataRecvServer(U8 uartID, eNetSocket SocketID)
//{
//	AddModemOptTask(SocketID, eAT_DataRead);
//	return TRUE;
//}

static U8 GPRSDataTask(eNetSocket SocketID)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	U32 len = 0;
	U8 u8DataType = eDataType_TCP;
	
	//只要socket被打开
	if(eSocket_Online != GPRS_Getonlineflag(SocketID))
		return FALSE;
	
	//有数据正在被发送
	if(TRUE == pModemDCB->UserDataTxState || TRUE == pModemDCB->TCPDataTxState)
		return FALSE;
	
	u8DataType = eSocket_TCP == pSocketDCB->SocketType ? eDataType_TCP : eDataType_MQTT;
	
	//数据发送
	len = QueueIsTxData((eDataQueueID)SocketID, u8DataType);
	if(len <= 0)
		return FALSE;
	
	if(TRUE != JudgeTimeOutMs(pSocketDCB->ATDataTaskTick, eTick_200ms))
		return FALSE;
	
	pSocketDCB->ATDataTaskTick = NOWTICK;
	
	if(eSocket_MQTT == pSocketDCB->SocketType)
	{
		AddModemOptTask(SocketID, eAT_MTPUBEX);
	}
	else if(eSocket_TCP == pSocketDCB->SocketType)
	{
		AddModemOptTask(SocketID, eAT_DataWrite);
	}
	
	return TRUE;
}

static U8 GPRSTcpDataReadTask(eNetSocket SocketID)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	if(eSocket_TCP != pSocketDCB->SocketType)
		return FALSE;
	
	//只要socket被打开
	if(eSocket_Online != GPRS_Getonlineflag(SocketID))
		return FALSE;	

	if(TRUE != JudgeTimeOutMs(pSocketDCB->ATDataReadTick, eTick_3S))
		return FALSE;
	
	pSocketDCB->ATDataReadTick = NOWTICK;
	
	AddModemOptTask(SocketID, eAT_DataRead);
	
	return TRUE;
}

static U8 MTGPRSSocketCmdTask(eNetSocket SocketID)
{
	ModemConnectTCPTask(SocketID);  //tcp状态查询
	ModemMTStateCheck(SocketID);	//mqtt状态查询
	ModemFTPStateCheck(SocketID);	//ftp升级状态查询
	
	//数据模式任务管理
	GPRSDataTask(SocketID);
	GPRSTcpDataReadTask(SocketID);
	return FALSE;
}

static U8 MTGPRSDrvServer(eNetSocket SocketID)
{
	const ATCmdItem *pATCmd = GetCurrentATCmdItem(SocketID);
	
	//发送AT指令
	if (NULL == pATCmd)
		return FALSE;
	
	//AT等待延时
	if(TRUE == MTGPRSServerDelay(SocketID, pATCmd))
		return TRUE;
	
	return ATCmd(SocketID, pATCmd);
}

//////////////////////////////////////////////////////////////////////////
//以下为外部调用函数

static void CommMqttInit(eNetSocket Socket)
{
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
	
	//连接ID
	pModemDCB->strSocketDCB[Socket].SocketID = Socket;
	//连接等级
	pModemDCB->strSocketDCB[Socket].SocketLevel = eSocket_LvMain;
	//连接类型
	pModemDCB->strSocketDCB[Socket].SocketType = eSocket_MQTT;
	
	return;
}
static void MainCommunciationInit(void)
{
    //主平台初始化，根据平台不一样进行区别，需要按照平台不同进行更改
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;

    U8 Socket = eSocket_GPRS1;

    if (pModemDCB->SocketCnt <= Socket) {
        return;
    }
    pModemDCB->strSocketDCB[Socket].ReconnectCnt = 0;

	PalQueueInit((eDataQueueID)Socket);

	//连接ID
	pModemDCB->strSocketDCB[Socket].SocketID = Socket;
	//连接等级
	pModemDCB->strSocketDCB[Socket].SocketLevel = eSocket_LvMain;
    
	//重连时间间隔
	pModemDCB->strSocketDCB[Socket].ReconnectInterval = eTick_5S;
	//重连最大次数
	pModemDCB->strSocketDCB[Socket].ReconnectMaxCnt = 5;

	if(ePlatType_JSIOT == get_ChgParam_plat_type() || ePlatType_GNIOT == get_ChgParam_plat_type())
	{
        //连接类型
        pModemDCB->strSocketDCB[Socket].SocketType = eSocket_MQTT;
	}
	else if(ePlatType_ANPEI==get_ChgParam_plat_type())  //JJUNIVE
	{
        pModemDCB->strSocketDCB[Socket].SocketType = eSocket_TCP;
	}
	else
	{
        //连接类型
        pModemDCB->strSocketDCB[Socket].SocketType = eSocket_TCP;
	}
	
	return;
}

static void SlaveCommunciationInit(void)
{
    //运维平台初始化
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;

    U8 Socket = eSocket_GPRS2;
	
    if (pModemDCB->SocketCnt <= Socket) {
        return;
    }
    pModemDCB->strSocketDCB[Socket].ReconnectCnt = 0;
	//连接ID
	pModemDCB->strSocketDCB[Socket].SocketID = Socket;
	//连接等级
	pModemDCB->strSocketDCB[Socket].SocketLevel = eSocket_LvSlave;
	//连接类型
	pModemDCB->strSocketDCB[Socket].SocketType = eSocket_TCP;
    
	//重连时间间隔
	pModemDCB->strSocketDCB[Socket].ReconnectInterval = eTick_5S;
	//重连最大次数
	pModemDCB->strSocketDCB[Socket].ReconnectMaxCnt = 10;
	
	pModemDCB->u32CATxLen = strlen(JS_cacert_pem);

	PalQueueInit((eDataQueueID)Socket);

	return;
}

/*****************************************************************************
 * 函数名：		CommunciationInit
 * 功能描述：	多链接初始化，清除链接信息
 * 入口参数：	无
 * 函数返回值：	无
*****************************************************************************/
static void CommunciationInit(void)
{
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
	
	//Modem控制数据初始化
	memset(pModemDCB, 0x00, offsetof(GprsModemDCB, strATMDData));
	
	//双链接控制
	//连接数量
	if(ePlatType_gwYKC != get_ChgParam_plat_type())
	   pModemDCB->SocketCnt = eSocket_Cnt;
	else
	  pModemDCB->SocketCnt = 1;

    //提前初始化，否则任务不能加入
    GprsOptTaskInit();

    MainCommunciationInit();
    SlaveCommunciationInit();
    
	return;
}

eSocketType GprsGetSocketType()
{
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
	return pModemDCB->strSocketDCB[eSocket_GPRS1].SocketType;
}

//tcp切换到ftp
U8 tcpSwitchFtp(eNetSocket SocketID)
{
    eNetSocket Socket = SocketID;

	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
    
    if (pModemDCB->strSocketDCB[Socket].SocketType == eSocket_TCP) {
	    AddModemOptTask(eSocket_GPRS1, eAT_SocketClose);			//关闭tcp socket
    } else if (pModemDCB->strSocketDCB[Socket].SocketType == eSocket_MQTT) {
	    AddModemOptTask(SocketID, eAT_MTClose);                     //关闭mqtt socket
    } else {
	    return FALSE;
    }

	//连接类型
	pModemDCB->strSocketDCB[Socket].SocketType = eSocket_FTP;
	
	//tcp协议平台重连
	Plat_Reconnect(SocketID);

	GPRS_Setonlineflag(SocketID, eSocket_Attach);
    
    ATFtpConn(SocketID);

	printf("tcpSwitchFtp\r\n");

	return TRUE;
}

//ftp切换到tcp
U8 ftpSwitchTcp(eNetSocket SocketID)
{
    //升级完成后转tcp连接
    eNetSocket Socket = SocketID;

	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;
	//连接类型
	pModemDCB->strSocketDCB[Socket].SocketType = eSocket_TCP;
	
	//tcp协议平台重连
	Plat_Reconnect(SocketID);

	AddModemOptTask(eSocket_GPRS1, eAT_FTPClose);			//关闭ftp socket
    
	GPRS_Setonlineflag(SocketID, eSocket_Attach);

	printf("ftpSwitchTcp\r\n");

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//方法:		ATSendWait
//全名:		ATSendWait
//函数说明: 
//访问:		  
//参数:		无
//返回值:	无
//注释:
//////////////////////////////////////////////////////////////////////////
void ATCmdWait(U8 falg, U32 u32WaitTick)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	pGprsModemDCB->ATWaitFlag = falg;
	if(TRUE == pGprsModemDCB->ATWaitFlag)
	{
		pGprsModemDCB->ATWaitTick = NOWTICK;
		pGprsModemDCB->ATWaitDelay = u32WaitTick;
	}
	else
	{
		pGprsModemDCB->ATTryCnt = 0;
	}
	
	return;
}

static void StartModemATOpenList()
{
	AddModemOptTask(eSocket_GPRS1, eAT_OnQuery);			//模块识别
	AddModemOptTask(eSocket_GPRS1, eAT_SIMQuery);			//SIM卡状态查询
	AddModemOptTask(eSocket_GPRS1, eAT_SIMStateQuery);		//SIM卡识别状态查询
	AddModemOptTask(eSocket_GPRS1, eAT_SIMICCIDQuery);		//SIM卡ICCID码查询
	AddModemOptTask(eSocket_GPRS1, eAT_PSQuery);			//PS服务网络连接状态查询
	AddModemOptTask(eSocket_GPRS1, eAT_CSQQuery);
	AddModemOptTask(eSocket_GPRS1, eAT_OperatorQuery);		//
	AddModemOptTask(eSocket_GPRS1, eAT_NetStateQuery); 		//
	AddModemOptTask(eSocket_GPRS1, eAT_PDPSet); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_PDPAct); 			//
	AddModemOptTask(eSocket_GPRS1, eAT_PDPStateQuery);		//
    AddModemGetRealTimeClk();
}

void StartModemReOpenTask(void)
{
	AddModemOptTask(eSocket_GPRS1, eAT_CFUNpythy);		//
	AddModemOptTask(eSocket_GPRS1, eAT_CFUNall);		//
}

//////////////////////////////////////////////////////////////////////////
//函数名：		StartModemOpenTask
//功能描述：	启动开机任务
//入口参数：	U32 bNeedRePowerOn
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void StartModemOpenTask(void)
{
	GprsOptTaskInit();

    StartModemATOpenList();
	
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		StartModemCloseTask
//功能描述：	启动关机任务
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U8 ReStartATModem(void)
{
    CommunciationInit();

    ModemAbnormalReboot();

	return TRUE;
}
static U8 SocketDisConnectTask(eNetSocket SocketID)
{
	//1. 断开socket  2. 重新连接socket  3.主链接重连计次数，超过10次进行重启
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	if(eSocket_Reconnect != GPRS_Getonlineflag(SocketID))
		return FALSE;
	
    CommunicationPintf(DBG_INFO, "%d SocketDisConnectTask\r\n", SocketID);

    //关闭客户端连接
    if (pSocketDCB->SocketType == eSocket_TCP) {
	    AddModemOptTask(SocketID, eAT_SocketClose);
    } else if (pSocketDCB->SocketType == eSocket_MQTT) {
	    AddModemOptTask(SocketID, eAT_MTClose);
    } else if (pSocketDCB->SocketType == eSocket_FTP) {
	    AddModemOptTask(SocketID, eAT_FTPClose); 
        OtaSetState(eOtaSta_Idle);
        ftpSwitchTcp(SocketID);
    } else {
    }

	GPRS_Setonlineflag(SocketID, eSocket_Attach);

	return TRUE;
}

U8 StartModemReconnectTask(eNetSocket SocketID)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 ret = FALSE;
	
    CommunicationPintf(DBG_INFO, "\r\nsocket %d reconnect break!!!\r\n", SocketID);
    
	PalQueueInit((eDataQueueID)SocketID);
    ClrSocketModemOptTask(SocketID);

    SocketDisConnectTask(SocketID);         //socket断开连接

	return ret;
}

eSocketState GPRS_Getonlineflag(eNetSocket SocketID)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	return pSocketDCB->ConnectState;
}

void GPRS_Setonlineflag(eNetSocket SocketID, eSocketState SocketState)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];

	pSocketDCB->ConnectState = SocketState;
	
	return;
}

U8 GPRS_SocketCnt(void)
{
	return g_strGprsModemDCB.SocketCnt;
}

U8 Comm_FillHttpUrl(U8 *pUrl, U16 len)
{
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	if(0 == len || len > OTA_URL_LEN || NULL == pUrl)
		return FALSE;
	
	memset(pHttpDCB->C8OtaUrl, 0, OTA_URL_LEN);
	pHttpDCB->u16UrlLen = len;
	memcpy(pHttpDCB->C8OtaUrl, pUrl, len);
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		MTRecvDecode
//功能描述：	接收信息处理
//入口参数：	U8 *pData	:收到的信息首地址
//				U32 nDataLen	:收到的信息长度
//函数返回值：	U32 :处理后的数据长度
//////////////////////////////////////////////////////////////////////////
void MTRecvDecode(U8 *pData, U32 nDataLen)
{
	U8 i = 0;
	U32 u32RxLength = nDataLen;			//接收数据长度
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
    pModemDCB->ATNormalTime = NOWTICK;
    pModemDCB->ATAbnormal = eATStatus_Normal;
	
	for(i = 0; i < SOCKET_CNT; i++)
	{
		//指令解析,因为双链接单任务队列,避免误删,成功处理后直接退出
		if(TRUE == MTRecvDecodeProc((eNetSocket)i, pData, u32RxLength))
			break;
	}

	for(i = 0; i < SOCKET_CNT; i++)
	{
		//URC无阻塞限制
		MTRecvDecodeURC((eNetSocket)i, pData, &u32RxLength);
	}
	
	return;
}

static U8 MTGPRSUserDataSend(void)
{
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	if(TRUE != pModemDCB->UserDataTxState)
		return FALSE;
	
	//等待发送完成
	if (eUart_Idle != pUartDCB->TxState)
		return TRUE;
	
	PopPalTxBuf((eDataQueueID)pModemDCB->UserDataTxSocket, eDataType_MQTT, NULL, NULL, pUartDCB->pTxBuffer, &pUartDCB->TxLength, pUartDCB->u16TxBufSize);
	UartSendData(UART_GPRS, pUartDCB->TxLength);
	pModemDCB->UserDataTxState = FALSE;
	
	//发送完之后等结果
	AddModemOptTask(pModemDCB->UserDataTxSocket, eAT_MTPUBEXRET);

	return TRUE;
}


static U8 TCPGPRSUserDataSend(void)
{
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
    	
	if(TRUE != pModemDCB->UserDataTxState)
		return FALSE;
	
	//等待发送完成
	if (eUart_Idle != pUartDCB->TxState)
		return TRUE;
	//tcp发送队列中取数据发送
	PopPalTxBuf((eDataQueueID)pModemDCB->UserDataTxSocket, eDataType_TCP, NULL, NULL, pUartDCB->pTxBuffer, &pUartDCB->TxLength, pUartDCB->u16TxBufSize);
    hex_dump("tcp_send_data", pUartDCB->pTxBuffer, pUartDCB->TxLength);
	UartSendData(UART_GPRS, pUartDCB->TxLength);
	pModemDCB->UserDataTxState = FALSE;

	strcpy((char *)pUartDCB->pTxBuffer, "\r\n");
	//发送数据
	UartSendData(UART_GPRS, 2);

	return TRUE;
}

static U8 HPUrlDataSend(void)
{
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	if(TRUE != pModemDCB->UrlTxState)
		return FALSE;
	
	//等待发送完成
	if (eUart_Idle != pUartDCB->TxState)
		return TRUE;
	
	memset(pUartDCB->pTxBuffer, 0x00, pUartDCB->u16TxBufSize);
	memcpy(pUartDCB->pTxBuffer, pHttpDCB->C8OtaUrl, pHttpDCB->u16UrlLen);
	UartSendData(UART_GPRS, pHttpDCB->u16UrlLen);
	pModemDCB->UrlTxState = FALSE;
	
	return TRUE;
}

static U8 CADataSend(void)
{
	UartDCB *pUartDCB = GetUartDCB(UART_GPRS);
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	U32 u32SendCALen = 0;
	
	if(TRUE != pModemDCB->CATxState)
		return FALSE;
	
	//等待发送完成
	if (eUart_Idle != pUartDCB->TxState)
		return TRUE;
	
	if(pModemDCB->u32CATxLen >= (pModemDCB->u32CATxIndex+1024))
	{
		u32SendCALen = 1024;
	}
	else
	{
		if(pModemDCB->u32CATxLen >= pModemDCB->u32CATxIndex)
			u32SendCALen = pModemDCB->u32CATxLen - pModemDCB->u32CATxIndex;
		else
			u32SendCALen = 0;
	}
	
	memset(pUartDCB->pTxBuffer, 0x00, pUartDCB->u16TxBufSize);
	memcpy(pUartDCB->pTxBuffer, &JS_cacert_pem[pModemDCB->u32CATxIndex], u32SendCALen);
	
	UartSendData(UART_GPRS, u32SendCALen);
	
	pModemDCB->u32CATxIndex += u32SendCALen;
	if(pModemDCB->u32CATxIndex >= pModemDCB->u32CATxLen)
	{
		pModemDCB->CATxState = FALSE;
	}
	// UPRINT((char*)pUartDCB->pTxBuffer);
	
	return TRUE;
}

//监测Socket是否需要重连
static uint8_t ifSocketReconnect(eNetSocket SocketID)
{
    //主平台初始化，根据平台不一样进行区别，需要按照平台不同进行更改
	GprsModemDCB *pModemDCB = &g_strGprsModemDCB;

	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];

    if (pModemDCB->strATMDData.RebootSta != eModelRebootsta_Normal) {
        return 0;
    }
    if (pModemDCB->SocketCnt <= SocketID) {
        return 0;
    }
    if (pSocketDCB->ConnectState == eSocket_Online) {
        pSocketDCB->ReconnectTime = NOWTICK;
        pSocketDCB->ReconnectCnt = 0;
        return 0;
    }

    if (pSocketDCB->ConnectState != eSocket_Reconnect) {
        return 0;
    }

    //惰性重连机制，避免重复快速重连，减轻服务器压力
    uint32_t itv = pSocketDCB->ReconnectInterval * (pow(2, pSocketDCB->ReconnectCnt));
    if (JudgeTimeOutMs(pSocketDCB->ReconnectTime, itv) == FALSE) {
        return 0;
    }
	LogPrintf(LVL_LOG_WARN, "ifSocketReconnect: %d %d  %d %d\r\n", SocketID, pSocketDCB->ReconnectCnt, pSocketDCB->ReconnectTime, itv);

    pSocketDCB->ReconnectCnt++;
    pSocketDCB->ReconnectTime = NOWTICK;

    if (pSocketDCB->ReconnectCnt >= pSocketDCB->ReconnectMaxCnt) {
        pSocketDCB->ReconnectCnt = 0;
    }
    

    return 1;
}

void ModemResetCheck()
{
    //每个连接都断开，重连多次后重启模组
    uint8_t allReconnected = 1; // 假设所有连接都达到了重连次数
	U8 i = 0;
	GprsModemDCB *pGprsModemDCB = &g_strGprsModemDCB;

	U8 icnt= 0;
	for(i = 0; i < SOCKET_CNT; i++)
    {
	    SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[i];
        if (pSocketDCB->ReconnectCnt < 2) {
            allReconnected = 0; // 如果有任何一个连接的重连次数未达到2次，则标记为未全部重连
            break;
        }
    }
    if (allReconnected) {
        // 所有连接的重连次数都达到了2次以上，进行模组重连
	    printf("ATFUN Reboot\r\n");
        ReStartATModem();
    }
}

void SocketReconnectScan(eNetSocket SocketID)
{
    uint8_t rslt = ifSocketReconnect(SocketID);
    if (rslt == 0) {
        return;
    }
    CommunicationPintf(DBG_INFO, "SocketReconnectScan Reconnect: %d\r\n", SocketID);
	//socket重连
    StartModemReconnectTask(SocketID);
    
    GPRS_Setonlineflag(SocketID, eSocket_Attach);
}

//////////////////////////////////////////////////////////////////////////
//函数名：		CommunicationServer
//功能描述：	系统与主站通讯服务程序
//入口参数：	无
//函数返回值：	无
//				发送任务是单路队列,阻塞
//////////////////////////////////////////////////////////////////////////
void CommunicationServer(void)
{
	U8 i = 0;
	
	//OTA url发送
	// if(TRUE == HPUrlDataSend())
	// 	return;
	
	// //SSL证书发送
	// if(TRUE == CADataSend())
	// 	return;
		
	// //用户数据发送
	// if(TRUE == MTGPRSUserDataSend())
	// 	return;
	
	//TCP用户数据发送
	if(TRUE == TCPGPRSUserDataSend())
		return;
	
	for(i = 0; i < SOCKET_CNT; i++)
	{
		//模块指令任务管理
		MTGPRSSocketCmdTask((eNetSocket)i);
		
		//指令模式应答需要阻塞，结果不用阻塞
		MTGPRSDrvServer((eNetSocket)i);

		//socket重连惰性机制
        SocketReconnectScan((eNetSocket)i);
	}
	
	return;
}

//网络模块连接
void NetWorkConnectScan()
{
    //此处需要区分LAN和4G
    UartServer();   //串口收发状态转换以及数据处理，可单独
    
    CommunicationServer();
    
	//模块指令任务管理，时钟、信号定时获取
	MTGPRSCmdTask();
    
	//模块重启任务管理
    ModemAbnormalCheck();
    ModemResetCheck();
}

//充电桩运营平台
void CpopInteractionScan()
{
    // GNUpProtocolDeal();
}

//充电桩运维平台
void CpmpInteractionScan()
{
}

void runCommunicationTask()
{
    InitAllUart();      //串口初始化

    CommunciationInit();

    ReStartATModem();

    while(1)
    {
        NetWorkConnectScan();   //网络模块连接

        CpopInteractionScan();  //充电桩运营平台连接
        
        CpmpInteractionScan();  //充电桩运维平台连接

        vTaskDelay(20);
		
    }
}
