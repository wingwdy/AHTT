
//////////////////////////////////////////////////////////////////////////
//Copyright (C), 2010, 
//文件名：		SIM900A.c
//作者:			
//版本号:       V1.0
//创建日期:		2010-01-28
//说明:			SIM900A模块操作
//函数列表:		
//修改记录:
//				2010-06-15 
//				SIM900AOrder 函数，去掉保存更新点的AT命令
//////////////////////////////////////////////////////////////////////////
#include "SIM900A.h"
#include "Gprslib.h"
#include "Libqueue.h"
#include "CommInterface.h"
#include "RouteHeaderSummary.h"
#include "mbsDataUpdate.h"



//////////////////////////////////////////////////////////////////////////
//函数名：		ModemFtpInfoSetip
//功能描述：	设置升级ftp ip接口
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static void SIM900MainStartConnectAT(eNetSocket SocketID)
{
    GPRS_Setonlineflag(SocketID, eSocket_Neting);

    printf("\r\n---NET START CONNECT----\r\n\r\n");
}


//////////////////////////////////////////////////////////////////////////
//函数名：		ModemFtpInfoSetip
//功能描述：	设置升级ftp ip接口
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void SIM900FtpInfoSetip(char *ip, uint16_t port)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	memset(pGprsModemDCB->strFtpDCB.info.ftpIp, 0, sizeof(pGprsModemDCB->strFtpDCB.info.ftpIp));
	memcpy(pGprsModemDCB->strFtpDCB.info.ftpIp, ip, strlen(ip));
	pGprsModemDCB->strFtpDCB.info.ftpPort = port;

	printf("FTP Info: ip: %s\r\n", ip);
	printf("FTP Info: port: %d\r\n", port);
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemFtpInfoSetUserName
//功能描述：	设置升级ftp username接口
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void SIM900FtpInfoSetUserName(char *name, char *password)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	memset(pGprsModemDCB->strFtpDCB.info.ftpUserName, 0, sizeof(pGprsModemDCB->strFtpDCB.info.ftpUserName));
	memset(pGprsModemDCB->strFtpDCB.info.ftpPassword, 0, sizeof(pGprsModemDCB->strFtpDCB.info.ftpPassword));
	memcpy(pGprsModemDCB->strFtpDCB.info.ftpUserName, name, strlen(name));
	memcpy(pGprsModemDCB->strFtpDCB.info.ftpPassword, password, strlen(password));

	printf("FTP Info: name: %s\r\n", name);
	printf("FTP Info: password: %s\r\n", password);
	return;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ModemFtpInfoSetPath
//功能描述：	设置升级ftp path接口
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void SIM900FtpInfoSetPath(char *path, char *fileName)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	memset(pGprsModemDCB->strFtpDCB.info.ftpPath, 0, sizeof(pGprsModemDCB->strFtpDCB.info.ftpPath));
	memset(pGprsModemDCB->strFtpDCB.info.ftpFileName, 0, sizeof(pGprsModemDCB->strFtpDCB.info.ftpFileName));
	memcpy(pGprsModemDCB->strFtpDCB.info.ftpPath, path, strlen(path));
	memcpy(pGprsModemDCB->strFtpDCB.info.ftpFileName, fileName, strlen(fileName));

	printf("FTP Info: path: %s\r\n", path);
	printf("FTP Info: fileName: %s\r\n", fileName);
	return;
}




//////////////////////////////////////////////////////////////////////////
//宏定义

//////////////////////////////////////////////////////////////////////////
//方法:		MTPowerControl
//全名:		MTPowerControl
//函数说明: 模块电源控制
//访问:		public static 
//参数:		无
//返回值:	无
//注释:
//////////////////////////////////////////////////////////////////////////
static void MTPowerControl(void)
{
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	if(0 == pGprsModemDCB->PowerStep)
	{
		fgv_DoWriteRoute(APP_LTE_PWR_ENABLE_DO, RESET);
		pGprsModemDCB->PowerStep = 1;
        BspUartInitGprsIpd();
		LogPrintf(LVL_LOG_WARN, "GPRS poweroff !!!\r\n");
	}
	else if(1 == pGprsModemDCB->PowerStep)
	{
		fgv_DoWriteRoute(APP_LTE_PWR_ENABLE_DO, SET);
		pGprsModemDCB->PowerStep = 2;
        BspUartInitGprs();
		LogPrintf(LVL_LOG_WARN, "GPRS poweron !!!\r\n");
	}
	else if(2 == pGprsModemDCB->PowerStep)
	{
		fgv_DoWriteRoute(APP_LTE_PWR_KEY_DO, SET);
		pGprsModemDCB->PowerStep = 3;
		LogPrintf(LVL_LOG_WARN, "GPRS pwrkey 1 !!!\r\n");
	}
	else if(3 == pGprsModemDCB->PowerStep)
	{
		fgv_DoWriteRoute(APP_LTE_PWR_KEY_DO, RESET);
		pGprsModemDCB->PowerStep = 0;
		LogPrintf(LVL_LOG_WARN, "GPRS pwrkey 2 !!!\r\n");
	}
	return;
}

static U32 MPwrOn(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{	
	nATLen = 0;
	
	//给模块上电
	MTPowerControl();

    LogPrintf(LVL_LOG_WARN, "-------EC600 START UP-------\r\n");

    GPRS_Setonlineflag(eSocket_GPRS1, eSocket_Neting);
	
	return nATLen;
}


static U32 ATSQICSGP(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	char cApn[24] = {0};
	
	//替代APN字符
	if(eOperator_CMCC == pATMDData->OperatorType)
	{
		memcpy(cApn, YD_APN, strlen(YD_APN));
	}
	else if(eOperator_CUCC == pATMDData->OperatorType)
	{
		memcpy(cApn, YD_APN, strlen(LT_APN));
	}
	else if(eOperator_CTCC == pATMDData->OperatorType)
	{
		memcpy(cApn, YD_APN, strlen(DX_APN));
	}
	nATLen = ReplaceStr(pBuf, nATLen, "[APN]", cApn, strlen(cApn), "CMNET");
	
	return nATLen;
}

static U32 MPwrDown(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	printf("GPRS QPOWD !!!\r\n");
	return nATLen;
}
static U32 ATSQIOPEN(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];

	PlatCfgInfo *platCfgInfo = fgv_GetPlatCfgInfo();

	char cMainIP[40] = {0};
	U32 MainPort = 0;
	
	//替代ID
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, pSocketDCB->SocketID);
	
	if(eSocket_GPRS1 == SocketID)
	{
		memcpy(cMainIP, platCfgInfo->PltMainIp, strlen((char *)platCfgInfo->PltMainIp));
		//替代主站IP
		nATLen = ReplaceStr(pBuf, nATLen, "[MIP]", cMainIP, strlen(cMainIP), "pile.gongniu.cn");
		
		MainPort = platCfgInfo->PltMainPort;
		//替代主站端口
		nATLen = ReplaceNum(pBuf, nATLen, "[MPORT]", MainPort, 5455);
        
        printf("\r\n---SERVER %d: START CONNECT----%s, %d\r\n\r\n", SocketID, cMainIP, MainPort);
	}
	else
	{
		memcpy(cMainIP, platCfgInfo->PltAuxiliaryIp, strlen((char *)platCfgInfo->PltAuxiliaryIp));
		nATLen = ReplaceStr(pBuf, nATLen, "[MIP]", cMainIP, strlen(cMainIP), "120.195.64.42");
		
		MainPort = platCfgInfo->PltAuxiliaryPort;
		nATLen = ReplaceNum(pBuf, nATLen, "[MPORT]", MainPort, 7000);
	}
	return nATLen;
}

static U32 ATSQIRD(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	//替代ID
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, pSocketDCB->SocketID);
	
	return nATLen;
}

static U32 ATSQISEND(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	U32 Len = 0;
	
	Len = GPRSIsTxData(SocketID);
	
	if(Len <= 0)
		return FALSE;
	
	//替代ID
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", SocketID, SocketID);
	
	//替代长度
	nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", Len, Len);

	return nATLen;
}

static U32 ATSQISTATE(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	//替代ID
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", 2, 2);
	
	return nATLen;
}


static U32 ATSQICLOSE(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	//替代ID
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, pSocketDCB->SocketID);
	
	return nATLen;
}

static U32 ATSQFTPCFGa(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	FtpDCB* pFtpDCB = &g_strGprsModemDCB.strFtpDCB;

	// char cName[] = FTP_USER_NAME;
	// char cPsw[] = FTP_USER_PSW;

	//用户名
	nATLen = ReplaceStr(pBuf, nATLen, "[NAME]", pFtpDCB->info.ftpUserName, strlen(pFtpDCB->info.ftpUserName), FTP_USER_NAME);
	
	//密码
	nATLen = ReplaceStr(pBuf, nATLen, "[PSW]", pFtpDCB->info.ftpPassword, strlen(pFtpDCB->info.ftpPassword), FTP_USER_PSW);
	
	return nATLen;
}
static U32 ATSQFTPOPEN(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	FtpDCB* pFtpDCB = &g_strGprsModemDCB.strFtpDCB;

	// char cAddr[] = FTP_USER_IP;
	
	//IP
	nATLen = ReplaceStr(pBuf, nATLen, "[FIP]", pFtpDCB->info.ftpIp, strlen(pFtpDCB->info.ftpIp), FTP_USER_IP);
	
	//端口
	nATLen = ReplaceNum(pBuf, nATLen, "[FPORT]", pFtpDCB->info.ftpPort, FTP_USER_PORT);
	
	return nATLen;
}

static U32 ATSQFTPCWD(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	FtpDCB* pFtpDCB = &g_strGprsModemDCB.strFtpDCB;

	// char cPath[] = FTP_FILDER_PATH;
	
	//路径
	nATLen = ReplaceStr(pBuf, nATLen, "[PATH]", pFtpDCB->info.ftpPath, strlen(pFtpDCB->info.ftpPath), FTP_FILDER_PATH);
	
	return nATLen;
}

static U32 ATSQFTPSIZE(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	FtpDCB* pFtpDCB = &g_strGprsModemDCB.strFtpDCB;

	// char cFile[] = "D3-A64A Vx.xx.bin";
	
	//路径
	nATLen = ReplaceStr(pBuf, nATLen, "[FILE]", pFtpDCB->info.ftpFileName, strlen(pFtpDCB->info.ftpFileName), "D3-A64A Vx.xx.bin");
	
	return nATLen;
}

static U32 ATSQFTPGET(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	FtpDCB* pFtpDCB = &g_strGprsModemDCB.strFtpDCB;

//	char cBasicFileHead[] = "D3-A64A Vx.xx";
	char cBasicFileHead[32] = {0};
	char cBasicFileSuffix[] = ".bin";
	char cFile[32] = {0};
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	//查找.bin字符串
	char nameChar[32] = {0};
	memcpy(nameChar, pFtpDCB->info.ftpFileName, strlen(pFtpDCB->info.ftpFileName));

	char *ptr = strstr(nameChar, cBasicFileSuffix);

	if (ptr != NULL) {
		int index = ptr - nameChar;
		memcpy(cBasicFileHead, pFtpDCB->info.ftpFileName, index);
	} else {
		memcpy(cBasicFileHead, pFtpDCB->info.ftpFileName, strlen(pFtpDCB->info.ftpFileName));
	}
	
	sprintf(cFile, "%s%d%s", cBasicFileHead, pOtaDCB->u8PackIndex, cBasicFileSuffix);
	//文件名
	nATLen = ReplaceStr(pBuf, nATLen, "[FILE]", cFile, strlen(cFile), FTP_FILENAME);
	
	return nATLen;
}

static U32 ATSQFSEEK(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	//句柄
	nATLen = ReplaceNum(pBuf, nATLen, "[HANDLE]", pOtaDCB->u32FileHandle, 0);
	
	//偏移
	nATLen = ReplaceNum(pBuf, nATLen, "[OFFSET]", pOtaDCB->u32ReadOffDest, 0);
	
	return nATLen;
}

static U32 ATSQFREAD(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	//句柄
	nATLen = ReplaceNum(pBuf, nATLen, "[HANDLE]", pOtaDCB->u32FileHandle, 0);
	
	//长度
	nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", pOtaDCB->u32ReadLen, 1024);
	
	return nATLen;
}

static U32 ATSQFCLOSE(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	//句柄
	nATLen = ReplaceNum(pBuf, nATLen, "[HANDLE]", pOtaDCB->u32FileHandle, 0);
	
	return nATLen;
}

static U32 ATSQFUpl(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", pModemDCB->u32CATxLen, 0);
	return nATLen;
}
static U32 ATSDNSCFG(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char mDNS[] = "114.114.114.114";
	char sDNS[] = "8.8.8.8";
	
	nATLen = ReplaceStr(pBuf, nATLen, "[MDNS]", mDNS, strlen(mDNS), "114.114.114.114");
	nATLen = ReplaceStr(pBuf, nATLen, "[SDNS]", sDNS, strlen(sDNS), "8.8.8.8");
	
	return nATLen;
}

static U32 ATSMTSocketID(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	return nATLen;
}


static U32 ATSDNSCFGali(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();

	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceStr(pBuf, nATLen, "[KEY]", pPlatParam->platInfo.GN_Info.C8ProductKey, strlen(pPlatParam->platInfo.GN_Info.C8ProductKey), "g8uj0iGB8IZ");
	nATLen = ReplaceStr(pBuf, nATLen, "[NAME]", pPlatParam->platInfo.GN_Info.C8DeviceName, strlen(pPlatParam->platInfo.GN_Info.C8DeviceName), "587537D7B301");
	nATLen = ReplaceStr(pBuf, nATLen, "[SECRET]", pPlatParam->platInfo.GN_Info.C8DeviceSecret, strlen(pPlatParam->platInfo.GN_Info.C8DeviceSecret), "fdf3bfbf7e9f96fb51011b8413efed12");
	
	return nATLen;
}

static U32 ATSMTOPEN(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	#define MQTT_JS_URL					"zs-iot.jslife.net"						//捷顺
	#define MQTT_GNIOT_URL				"iot-as-mqtt.cn-shanghai.aliyuncs.com"	//公牛智家


	char cHostName[64] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	
	if(ePlatType_GNIOT == get_ChgParam_plat_type())
	{
		sprintf(cHostName, "%s.%s", pPlatParam->platInfo.GN_Info.C8ProductKey, MQTT_GNIOT_URL);
		nATLen = ReplaceStr(pBuf, nATLen, "[HOSTNAME]", cHostName, strlen(cHostName), MQTT_GNIOT_URL);
		nATLen = ReplaceNum(pBuf, nATLen, "[PORT]", 1883, 1883);
	}
	else if(ePlatType_JSIOT == get_ChgParam_plat_type())
	{
		nATLen = ReplaceStr(pBuf, nATLen, "[HOSTNAME]", pPlatParam->PltMainIp, strlen((char*)pPlatParam->PltMainIp), MQTT_GNIOT_URL);
		nATLen = ReplaceNum(pBuf, nATLen, "[PORT]", pPlatParam->PltMainPort, 23001);
	}
	else
	{
		sprintf(cHostName, "%s", MQTT_JS_URL);
		nATLen = ReplaceStr(pBuf, nATLen, "[HOSTNAME]", cHostName, strlen(cHostName), MQTT_GNIOT_URL);
		nATLen = ReplaceNum(pBuf, nATLen, "[PORT]", 1883, 1883);
	}
	
	return nATLen;
}

static U32 ATSMTCONN(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();

	U32 u32Pid = fourUint8ToUint32(pPlatParam->platInfo.GN_Info.U8Pid);
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	if(ePlatType_GNIOT == get_ChgParam_plat_type())
	{
	nATLen = ReplaceNum(pBuf, nATLen, "[PID]", u32Pid, 141);
	nATLen = ReplaceStr(pBuf, nATLen, "[NAME]", pPlatParam->platInfo.GN_Info.C8DeviceName, strlen((char*)pPlatParam->platInfo.GN_Info.C8DeviceName), "587537D7B301");
	nATLen = ReplaceStr(pBuf, nATLen, "[SECRET]", pPlatParam->platInfo.GN_Info.C8ProductSecret, strlen((char*)pPlatParam->platInfo.GN_Info.C8ProductSecret), "NQXP7ymHG0WyjS0Q");
	}
	else
	{
		nATLen = ReplaceStr(pBuf, nATLen, "[PID]", pPlatParam->platInfo.JS_Info.C8JSClientId, strlen(pPlatParam->platInfo.JS_Info.C8JSClientId), pPlatParam->platInfo.JS_Info.C8JSClientId);
		nATLen = ReplaceStr(pBuf, nATLen, "[NAME]", pPlatParam->platInfo.JS_Info.C8JSAccount, strlen(pPlatParam->platInfo.JS_Info.C8JSAccount), pPlatParam->platInfo.JS_Info.C8JSAccount);
		nATLen = ReplaceStr(pBuf, nATLen, "[SECRET]", pPlatParam->platInfo.JS_Info.C8JSPassword, strlen(pPlatParam->platInfo.JS_Info.C8JSPassword), pPlatParam->platInfo.JS_Info.C8JSPassword);
	}
	
	return nATLen;
}

static U32 ATSMTSUB(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	char cTopic_Suffix[] = "thing/event/property/post_reply";
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic, "/sys/%s/%s/%s", pPlatParam->platInfo.GN_Info.C8ProductKey, pPlatParam->platInfo.GN_Info.C8DeviceName, cTopic_Suffix);
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 1, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}

static U32 ATSMTSUBGet(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	char cTopic_Suffix[] = "thing/service/property/set";
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic, "/sys/%s/%s/%s", pPlatParam->platInfo.GN_Info.C8ProductKey, pPlatParam->platInfo.GN_Info.C8DeviceName, cTopic_Suffix);
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 2, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}

static U32 ATSMTSUBOta(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
//	sprintf(cTopic,"/ota/device/upgrade/%s/%s/", pPlatParam->C8ProductKey, pPlatParam->C8DeviceName);  // 
	sprintf(cTopic,"/%s/%s/user/ota/device/upgrade", pPlatParam->platInfo.GN_Info.C8ProductKey, pPlatParam->platInfo.GN_Info.C8DeviceName);  // 
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}

static U32 ATSMTSUBUser(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic,"/%s/%s/user/get", pPlatParam->platInfo.GN_Info.C8ProductKey, pPlatParam->platInfo.GN_Info.C8DeviceName);  // 
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}

static U32 ATSMTSUBNotify(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic,"/sys/%s/%s/_thing/event/notify", pPlatParam->platInfo.GN_Info.C8ProductKey, pPlatParam->platInfo.GN_Info.C8DeviceName);  // 
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}
static U32 ATSMTSUBDown(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic,"/cdz/%s/%s/thing/model/down_raw", pPlatParam->platInfo.JS_Info.C8JSProductType, pPlatParam->platInfo.JS_Info.C8JSAccount);  // 
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}

static U32 ATSMTSUBDownReply(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	char cTopic[128] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
	sprintf(cTopic,"/cdz/%s/%s/thing/model/up_raw_reply", pPlatParam->platInfo.JS_Info.C8JSProductType, pPlatParam->platInfo.JS_Info.C8JSAccount);  // 
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	
	return nATLen;
}


static U32 ATSMTPUBEX(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	char cTopic[128] = {0};
	U32 len = 0;
	
	MTTxTopicData((eDataQueueID)SocketID, (U8*)cTopic, sizeof(cTopic), &len);
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	nATLen = ReplaceNum(pBuf, nATLen, "[MSGID]", 3, 1);
	nATLen = ReplaceStr(pBuf, nATLen, "[TOPIC]", cTopic, strlen(cTopic), "/sys");
	nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", len, 1);
	
	return nATLen;
}


static U32 ATSMTRECV(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ID]", pSocketDCB->SocketID, 0);
	return nATLen;
}

//static U32 ATSLST(eNetSocket SocketID, U8* pBuf, U32 nATLen)
//{
//	char cFile[] = "D3-A64A Vx.xx.bin";
//
//	//文件名
//	nATLen = ReplaceStr(pBuf, nATLen, "[FILE]", cFile, strlen(cFile), "D3-A64A Vx.xx.bin");
//	
//	return nATLen;
//}
static U32 ATSUrl(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", pHttpDCB->u16UrlLen, 0);
	return nATLen;
}

static U32 ATSDownload(eNetSocket SocketID, U8* pBuf, U32 nATLen)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	nATLen = ReplaceNum(pBuf, nATLen, "[ADDR]", pOtaDCB->u32ReadOffDest, 0);	
	if(eJSHttp_Param == pHttpDCB->u8JSOtaOrParam)
		nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", 1024, 0);
	else
		nATLen = ReplaceNum(pBuf, nATLen, "[LEN]", pOtaDCB->u32ReadLen, 0);
	
	return nATLen;
}

//============================================
static U8 ATRAck(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;

    printf("%s\r\n", pData);

	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		ret = TRUE;
	}
	return ret;
}

static U8 ATRQSIMSTAT(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0};
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QSIMSTAT:", strlen("+QSIMSTAT:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, ",%d/r", &u32Temp[0]);
		if(0 == u32Temp[0])
			ret = TRUE;
	}
	return ret;
}

static U8 ATRQCCID(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U8 *pTail = NULL;
	U16 u16IDlen = 0;
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	
    printf("%s\r\n", pData);

	pTemp = SearchData((U8*)pData, nDataLen, "+QCCID:", strlen("+QCCID:"));
	if (NULL != pTemp)
	{

		pTemp = SearchData(pTemp, nDataLen, ":", strlen(":"));
		if (NULL == pTemp)
			return ret;
		pTemp += 2;
		pTail = SearchData(pTemp, nDataLen, "\r", strlen("\r"));
		if (NULL == pTemp)
			return ret;
		u16IDlen = pTail - pTemp;
		u16IDlen = u16IDlen > sizeof(pATMDData->SIMID) ? sizeof(pATMDData->SIMID) : u16IDlen;
		memcpy(pATMDData->SIMID, pTemp, u16IDlen);
		ret = TRUE;
	}
	return ret;
}

static U8 ATRCSQ(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0};
	
    printf("%s\r\n", pData);

	//信号强度
	pTemp = SearchData((U8*)pData, nDataLen, "+CSQ:", strlen("+CSQ:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+CSQ: %d,", &u32Temp[0]);
		pATMDData->Csq = u32Temp[0];
		ret = TRUE;
	}
	
	return ret;
}

static U8 ATRQLTS(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U8 u32Temp[7] = {0};
	U32 tmp_year = 2023;
	
    printf("%s\r\n", pData);
	//实时时钟
	pTemp = SearchData((U8*)pData, nDataLen, "+QLTS:", strlen("+QLTS:"));
	if (NULL != pTemp)
	{
          sscanf((char*)pTemp,"+QLTS: \"%d/%hhd/%hhd,%hhd:%hhd:%hhd+\r\n",&tmp_year,
                  &u32Temp[2],&u32Temp[3],&u32Temp[4],&u32Temp[5],&u32Temp[6]);
		//设置时钟
		u32Temp[0] = tmp_year / 100;
		u32Temp[1] = tmp_year % 100;
		setCurrentRunTime(u32Temp);
		ret = TRUE;
	}
	
	return ret;
}

static U8 ATRQNTP(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U8 u32Temp[7] = {0};
	U32 tmp_year = 2023;
	int s32temp = 0;
	
    printf("%s\r\n", pData);
	//实时时钟
	pTemp = SearchData((U8*)pData, nDataLen, "+QNTP:", strlen("+QNTP:"));
	if (NULL != pTemp)
	{
          sscanf((char*)pTemp,"+QNTP: %d,\"%d/%hhd/%hhd,%hhd:%hhd:%hhd+\r\n", &s32temp, &tmp_year,
                  &u32Temp[2],&u32Temp[3],&u32Temp[4],&u32Temp[5],&u32Temp[6]);
        if (s32temp > 0) {
            return ret;
        }
		//设置时钟
		u32Temp[0] = tmp_year / 100;
		u32Temp[1] = tmp_year % 100;
        
        //时区问题
        uint32_t currentStamp = 0;
        timToStamp(&currentStamp, (tm_struct *)u32Temp);
        currentStamp = currentStamp+28800;
        stampToTime((tm_struct *)u32Temp, currentStamp);
        
		setCurrentRunTime(u32Temp);
		ret = TRUE;
	}
	
	return ret;
}

static U8 ATRCGREG(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "+CGREG:", strlen("+CGREG:"));
	if (NULL != pTemp)
	{
		ret = TRUE;
	}
	
	return ret;
}

static U8 ATRCOPS(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
    printf("%s\r\n", pData);
    
	pTemp = SearchData((U8*)pData, nDataLen, "+COPS:", strlen("+COPS:"));
	if (NULL != pTemp)
	{
		if(NULL != SearchData((U8*)pData, nDataLen, "MOBILE", strlen("MOBILE")))
		{
			pATMDData->OperatorType = eOperator_CMCC;
			ret = TRUE;
		}
		else if(NULL != SearchData((U8*)pData, nDataLen, "UNICOM", strlen("UNICOM")))
		{
			pATMDData->OperatorType = eOperator_CUCC;
			ret = TRUE;
		}
		else if(NULL != SearchData((U8*)pData, nDataLen, "UNICOM", strlen("UNICOM")))
		{
			pATMDData->OperatorType = eOperator_CUCC;
			ret = TRUE;
		}
		else if(NULL != SearchData((U8*)pData, nDataLen, "CT", strlen("CT")))
		{
			pATMDData->OperatorType = eOperator_CTCC;
			ret = TRUE;
		}
	}
	
	return ret;
}

static U8 ATRQNWINFO(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QNWINFO:", strlen("+QNWINFO:"));
	if (NULL != pTemp)
	{
		ret = TRUE;
	}
	
	return ret;
}
static U8 ATRQIACT(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0};
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	PlatCfgInfo *pPlatParam = fgv_GetPlatCfgInfo();
	
    printf("%s\r\n", pData);
    
	pTemp = SearchData((U8*)pData, nDataLen, "+QIACT:", strlen("+QIACT:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QIACT: %d,%d,", &u32Temp[0], &u32Temp[1]);
		if(1 == u32Temp[1])
		{
            //socket可以进行连接
            for (int i = 0; i < SOCKET_CNT; i++) {
			    GPRS_Setonlineflag(i, eSocket_Attach);
                g_strGprsModemDCB.strATMDData.RebootSta = eModelRebootsta_Normal;
            }
			ret = TRUE;
		}
	}
	
	return ret;
}

static U8 ATRQPOWD(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	//固定走超时处理
	return ret;
}

static U8 ATRQIOPEN(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		ret = TRUE;
	}
	return ret;
}

//=====================================================================

static U8 ATRQIRD(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
//	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	U32 nUserDataLen = 0;
	U8 *pDest = NULL;
	
	pDest = SearchData((U8*)pData, nDataLen, "ERROR", strlen("ERROR"));
	if (NULL != pDest)
	{
	    return FALSE;
	}

	//寻找接收到数据标志
	pDest = SearchData(pData, nDataLen, "+QIRD:", strlen("+QIRD:"));
	if (NULL == pDest) 
		return TRUE;
	
	//判断长度
	if (sscanf((char*)pDest, "+QIRD: %d\r", &nUserDataLen) <= 0 || 0 == nUserDataLen)
		return TRUE;
	
	//指定数据起始
	//"AT+QIRD=0,1460\r\r\n+QIRD: xxx\r\n"
	pDest = SearchData(pDest, 30, "\r\n", strlen("\r\n"));
	if (NULL == pDest) 
		return TRUE;
	
	pDest += strlen("\r\n");
	
	//拷贝数据到缓存中		
	PalRecvPush((eDataQueueID)SocketID, eDataType_TCP, NULL, 0, pDest, nUserDataLen);
	
	return TRUE;
}

static U8 ATRQISEND(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 *pDest = NULL;
	pDest = SearchData((U8*)pData, nDataLen, "ERROR", strlen("ERROR"));
	if (NULL != pDest)
	{
	    return FALSE;
	}

	U8 ret = FALSE;
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;

	ret = TRUE;
	//等待串口空闲发送
	pModemDCB->UserDataTxState = TRUE;
	pModemDCB->UserDataTxSocket = SocketID;
	
	return ret;
}

static U8 ATRCFUN1(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
        SIM900MainStartConnectAT(SocketID);

        //启动开机任务
        StartModemOpenTask();

		ret = TRUE;
	}
	return ret;
}


static U8 ATRFTPCFGc(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		ret = TRUE;
	}
	return ret;
}

static U8 ATRQFTPOPEN(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRQFTPCWD(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	
	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		ret = TRUE;
		AddModemOptTask(eSocket_GPRS1, eAT_FileDownload);	//
	}
	
	return ret;
}

static U8 ATRQFTPSIZE(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0xff};
	char cExpect[AT_CMD_LEN] = {0};
	OtaDCB* pOtaDCB = &g_OtaDCB;
	SocketDCB *pSocketDCB = &g_strGprsModemDCB.strSocketDCB[SocketID];
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QFTPSIZE:", strlen("+QFTPSIZE:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QFTPSIZE: %d,%d\r", &u32Temp[0], &u32Temp[1]);
		if(SocketID == u32Temp[0])
		{
			if(u32Temp[1] > OTA_FRAME_SIZE && u32Temp[1] < OTA_MAX_SIZE)
			{
				pOtaDCB->u8PackSize = u32Temp[1];
				
				GPRS_Setonlineflag(SocketID, eSocket_Online);
				OtaSetState(eOtaSta_Start);
				ret = TRUE;
			}
			else
			{
				pOtaDCB->u8TryCnt = OTA_RETRY_CNT;
			}
			LogPrintf(LVL_LOG_WARN, "\r\nota file size %d !!!\r\n", u32Temp[1]);
		}
	}
	
	return ret;
}

static U8 ATRAckQFDEL(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U8 *pTemp1 = NULL;

    printf("%s\r\n", pData);

	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		 printf("Receive QFDEL OK\r\n");
		 ret = TRUE;
	}
	else //若未收到OK
	{
		   //只对418返回特殊打印
        	pTemp1 = SearchData((U8*)pData, nDataLen, "418", strlen("418"));
			if(NULL != pTemp1) 
			   printf("Receive QFDEL erro  than need reboot EC600N\r\n");
				

			 ret = FALSE;

	}

	return ret;
}
static U8 ATRQFTPGET(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0};
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QFTPGET:", strlen("+QFTPGET:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QFTPGET: %d,%d\r", &u32Temp[0], &u32Temp[1]);
		if(0 == u32Temp[0])
		{
			ret = TRUE;
			AddModemOptTask(eSocket_GPRS1, eAT_FileOpen); 	//
		}
//		else
//		{
//			Comm_PlatReconnect(SocketID, 2);
//		}
	}
	
	return ret;
}

static U8 ATRQFOPEN(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0};
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QFOPEN:", strlen("+QFOPEN:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QFOPEN: %d\r", &u32Temp[0]);
		pOtaDCB->u32FileHandle = u32Temp[0];
		ret = TRUE;
		AddModemOptTask(eSocket_GPRS1, eAT_FileSeek);	//
	}
	
	return ret;
}

static U8 ATRQFSEEK(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0};
	
	pTemp = SearchData((U8*)pData, nDataLen, "OK", strlen("OK"));
	if (NULL != pTemp)
	{
		ret = TRUE;
		AddModemOptTask(eSocket_GPRS1, eAT_FileRead);	//
	}
	
	return ret;
}

static U8 ATRQFREAD(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0xff};
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	pTemp = SearchData((U8*)pData, nDataLen, "CONNECT", strlen("CONNECT"));
	if (NULL == pTemp)
		return ret;
	
	pTemp = SearchData(pTemp, 30, "\r\n", strlen("\r\n"));
	if (NULL == pTemp) 
		return ret;
	pTemp += strlen("\r\n");
	
	ret = TRUE;

	ota_file_storage(pTemp, pOtaDCB->u32ReadLen);
	
	if(eOtaSta_FtpFinish == pOtaDCB->MOtaState)
	{//下载完成
		AddModemOptTask(eSocket_GPRS1, eAT_FileClose);		//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPClose);		//
		ftpSwitchTcp(eSocket_GPRS1);
	}
	else if(0 != ota_get_nextFile())
	{//下一包
		AddModemOptTask(eSocket_GPRS1, eAT_FileClose);		//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPClose);		//
		
		AddModemOptTask(eSocket_GPRS1, eAT_FTPConfig);		//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPNamePsw); 	//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPFileType);	//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPTransType);	//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPTimeout); 	//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPIpPort);		//
		AddModemOptTask(eSocket_GPRS1, eAT_FTPPath);		//
		AddModemOptTask(eSocket_GPRS1, eAT_UFSDelete);		//
	}
	else
	{//读定向
		AddModemOptTask(eSocket_GPRS1, eAT_FileSeek);	//
	}
	
	return ret;
}

static U8 ATRQFUpl(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	ret = TRUE;
	//等待串口空闲发送
	pModemDCB->CATxState = TRUE;
	pModemDCB->CATxSocket = SocketID;
	pModemDCB->u32CATxIndex = 0;
//	pModemDCB->u32CATxLen = strlen(JS_cacert_pem);
	
	return ret;
}

static U8 ATRQFTPCLOSE(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRMTOPEN(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;	
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0xff};
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QMTOPEN:", strlen("+QMTOPEN:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QMTOPEN: %d,%d\r", &u32Temp[0], &u32Temp[1]);
		if(SocketID == u32Temp[0])
		{
			if(0 != u32Temp[1])
			{
				Comm_PlatReconnect(SocketID, __LINE__);
			}
			else
			{
				ret = TRUE;
			}
		}
	}
	
	return ret;
}

static U8 ATRMTCONN(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRMTSUB(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRMTSUBSucc(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	GPRS_Setonlineflag(SocketID, eSocket_Online);
	
	return ret;
}

static U8 ATRMTPUBEX(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	ret = TRUE;
	//等待串口空闲发送
	pModemDCB->UserDataTxState = TRUE;
	pModemDCB->UserDataTxSocket = SocketID;
	
	return ret;
}

static U8 ATRPubRet(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};
	char cExpect[AT_CMD_LEN] = {0};
	
	sprintf(cExpect ,"+QMTPUBEX: %d", SocketID);
	pTemp = SearchData((U8*)pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL == pTemp)
		return ret;
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRMTRECV(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRQConn(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0xff};
	
	pTemp = SearchData((U8*)pData, nDataLen, "+QMTCONN:", strlen("+QMTCONN:"));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QMTCONN: %d,%d\r", &u32Temp[0], &u32Temp[1]);
		if(SocketID == u32Temp[0])
		{
			if(3 != u32Temp[1])
			{
				Comm_PlatReconnect(SocketID, __LINE__);
			}
			else
			{
				ret = TRUE;
			}
		}
	}
	
	return ret;
}

static U8 ATRDISC(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRCLOSE(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
	
	ret = TRUE;
	
	return ret;
}

static U8 ATRUrl(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
//	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};
	GprsModemDCB* pModemDCB = &g_strGprsModemDCB;
	
	ret = TRUE;
	pModemDCB->UrlTxState = TRUE;
	pModemDCB->UrlTxSocket = SocketID;
	
	return ret;
}

static U8 ATRHPGet(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};		
	
	ret = TRUE;
	AddModemOptTask(eSocket_GPRS1, eAT_HPRead);	//
	
	return ret;
}

static U8 Http_GetParam(U8* pData)
{
	cJSON *root = cJSON_Parse((char*)pData);
    cJSON *pResult = NULL;
    cJSON *pPlatParam = NULL;
    cJSON *pParamArray = NULL;
    cJSON *pParam = NULL;
    cJSON *pAccount = NULL;	
    cJSON *pAddr = NULL;
    cJSON *pID = NULL;
    cJSON *pPassword = NULL;
	PlatCfgInfo *pChargeParam = fgv_GetPlatCfgInfo();

	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	U8 *pHeadTemp = NULL, *pTailTemp = NULL;
	U32 u32Port = 0;
	U8 ret = FALSE;
	
	if (root == NULL)
		goto Result;
	
	pResult = cJSON_GetObjectItem(root, "result_code");
	if (NULL == pResult || FALSE == cJSON_IsString(pResult))
    {
    	goto Result;
    }
	
	if (0 != memcmp(pResult->valuestring, JS_PARAM_SUCC, strlen(JS_PARAM_SUCC)))
	{
		goto Result;
	}
	
	pPlatParam = cJSON_GetObjectItem(root, "platform_params");
	if (NULL == pPlatParam || FALSE == cJSON_IsArray(pPlatParam))
    {
		goto Result;
    }
	
	pParamArray = cJSON_GetArrayItem(pPlatParam, 0);
	if (NULL == pParamArray || FALSE == cJSON_IsObject(pParamArray))
    {
		goto Result;
    }
	
	pParam = cJSON_GetObjectItem(pParamArray, "params");
	if (NULL == pParam || FALSE == cJSON_IsObject(pParam))
    {
		goto Result;
    }
	
	pAccount = cJSON_GetObjectItem(pParam, "mqtt_account");
	if (NULL == pAccount || FALSE == cJSON_IsString(pAccount))
    {
    	goto Result;
	}
	
	pAddr = cJSON_GetObjectItem(pParam, "mqtt_server_addr");
	if (NULL == pAddr || FALSE == cJSON_IsString(pAddr))
    {
    	goto Result;
	}
	
	pID = cJSON_GetObjectItem(pParam, "mqtt_id");
	if (NULL == pID || FALSE == cJSON_IsString(pID))
    {
    	goto Result;
	}
	
	pPassword = cJSON_GetObjectItem(pParam, "mqtt_password");
	if (NULL == pPassword || FALSE == cJSON_IsString(pPassword))
    {
    	goto Result;
	}
	
	memset(pChargeParam->platInfo.JS_Info.C8JSAccount, 0, IOT_JS_ACTLEN+1);
	memset(pChargeParam->platInfo.JS_Info.C8JSPassword, 0, IOT_JS_PSDLEN+1);
	memset(pChargeParam->platInfo.JS_Info.C8JSClientId, 0, IOT_JS_CIDLEN+1);
	memset(pChargeParam->PltMainIp, 0, PLAT_DOMAIN_LEN);
	pChargeParam->PltMainPort = 0;
	
	pHeadTemp = SearchData((U8*)pAddr->valuestring, strlen(pAddr->valuestring), "//", strlen("//"));
	if (NULL == pHeadTemp)
	{
		goto Result;
	}
	pHeadTemp += 2;
	
	pTailTemp = SearchData((U8*)pHeadTemp, strlen((char*)pHeadTemp), ":", strlen(":"));
	if (NULL == pTailTemp)
	{
		goto Result;
	}
	
	if(pTailTemp-pHeadTemp > PLAT_DOMAIN_LEN)
	{
		goto Result;
	}
	
	memcpy(pChargeParam->PltMainIp, pHeadTemp, pTailTemp-pHeadTemp);
	
	sscanf((char*)pTailTemp, ":%d", &u32Port);
	pChargeParam->PltMainPort = u32Port;
	memcpy(pChargeParam->platInfo.JS_Info.C8JSAccount, pAccount->valuestring, IOT_JS_ACTLEN);
	memcpy(pChargeParam->platInfo.JS_Info.C8JSPassword, pPassword->valuestring, IOT_JS_PSDLEN);
	memcpy(pChargeParam->platInfo.JS_Info.C8JSClientId, pID->valuestring, IOT_JS_CIDLEN);
	
	ret = TRUE;
	
	if(TRUE == ret)
	{
		pChargeParam->platInfo.JS_Info.u8JSParamStep = JS_LOGIN_PARAM_OK;
		Set_platParam(pChargeParam);
		pHttpDCB->u8JSParamSucc = TRUE;
	}
	
Result:
	cJSON_Delete(root);
	return ret;
}

static U8 Http_Ota(U8* pData)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	U8 ret = FALSE;
	
	ota_file_storage(pData, pOtaDCB->u32ReadLen);
	
	pOtaDCB->u32FrameIndex++;

	if(eOtaSta_FtpFinish == pOtaDCB->MOtaState)
	{//下载完成
		ota_download_finish();
		return ret;
	}
	else
	{//读定向
		AddModemOptTask(eSocket_GPRS1, eAT_HPDownload);	//
	}
	
	return ret;
}

static U8 ATRHPREAD(eNetSocket SocketID, U8* pData, U32 nDataLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
//	U32 u32Temp[4] = {0xff};	
//	OtaDCB *pOtaDCB = &g_OtaDCB;
	HttpDCB* pHttpDCB = &g_strGprsModemDCB.strHttpDCB;
	
	pTemp = SearchData((U8*)pData, nDataLen, "CONNECT", strlen("CONNECT"));
	if (NULL == pTemp)
		return ret;
	
	pTemp = SearchData(pTemp, 30, "\r\n", strlen("\r\n"));
	if (NULL == pTemp) 
		return ret;
	pTemp += strlen("\r\n");
	
	if(ePlatType_JSIOT == get_ChgParam_plat_type() && eJSHttp_Param == pHttpDCB->u8JSOtaOrParam)
	{
		ret = Http_GetParam(pTemp);
	}
	else
	{
		ret = Http_Ota(pTemp);
	}
	
	return ret;
}

//================================================
static U8 MPwrOnF(eNetSocket SocketID, U8 ATFunc)
{
    SIM900MainStartConnectAT(SocketID);
	
	//启动开机任务
	StartModemOpenTask();
	return TRUE;
}

static U8 ATFail(eNetSocket SocketID, U8 ATFunc)
{	
	printf("ATFail...SocketID = %d  ATFunc = %d\r\n", SocketID, ATFunc);

	Comm_PlatReconnect(SocketID, __LINE__);
	
	return TRUE;
}
static U8 ATQFDELFail(eNetSocket SocketID, U8 ATFunc)
{	
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;

	printf("ATFail...SocketID = %d  ATFunc = %d\r\n", SocketID, ATFunc);

	Comm_PlatReconnect(SocketID, __LINE__);
	pGprsModemDCB->ATAbnormal=eATStatus_Abnormal;
	ReStartATModem();
	
	return TRUE;
}

static U8 ATRbt(eNetSocket SocketID, U8 ATFunc)
{	
	printf("ATRbt...SocketID = %d  ATFunc = %d\r\n", SocketID, ATFunc);

    ClrModemOptTask();
    StartModemReOpenTask();
	
	return TRUE;
}

static U8 ATMdlRbt(eNetSocket SocketID, U8 ATFunc)
{	
	printf("ATMdlRbt...SocketID = %d  ATFunc = %d\r\n", SocketID, ATFunc);

	AT_ImtlyReconnct(SocketID, __LINE__);
    
	return TRUE;
}

static U8 ATQIACT(eNetSocket SocketID, U8 ATFunc)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
    //失败Host可通过AT+QICLOSE断开Socket连接
	AddModemOptTask(SocketID, eAT_SocketClose); //关闭客户端连接
    
	return TRUE;
}

static U8 ATPWDF(eNetSocket SocketID, U8 ATFunc)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	//尝试次数
	if(pOtaDCB->u8TryCnt >= OTA_RETRY_CNT)
	{
//		CommFTPStop();
		fgv_AppSoftwareReset();
	}
	else
	{
		pGprsModemDCB->PowerStep = 0;
		AddModemOptTask(eSocket_GPRS1, eAT_PowerOn); 		//模块开机
	}
	return TRUE;
}
//==========================================
//SIM900A 拨号指令集合
const ATCmdItem SIM900A_DIALUPATCMD[] =
{
	{ ""                                                ,MPwrOn,		"",                	NULL,			MPwrOnF,4,    	eTick_2S, 		eAT_PowerOn,    	"模块开机" },
	
	{ "ATI\r\n"                                   		,NULL,			"ATI",         		ATRAck,			ATRbt,	3,    	eTick_5S,     	eAT_OnQuery,  		"识别模块" },
	{ "AT+QSIMSTAT?\r\n"                       			,NULL,			"+QSIMSTAT:",   	ATRQSIMSTAT,	ATRbt,	3,     	eTick_5S,     	eAT_SIMQuery,       "SIM卡状态查询" },
	{ "AT+CPIN?\r\n"                                    ,NULL,			"+CPIN: READY",   	NULL,			ATRbt,	3,    	eTick_10S,     	eAT_SIMStateQuery,  "SIM卡识别状态查询" },
	{ "AT+QCCID\r\n"                                    ,NULL,			"+QCCID:",         	ATRQCCID,		ATRbt,	3,   	eTick_10S,     	eAT_SIMICCIDQuery,	"SIM卡ICCID码查询" },
	{ "AT+CSQ\r\n"                                      ,NULL,			"+CSQ:",        	ATRCSQ,			ATRbt,	1,    	eTick_3S,     	eAT_CSQQuery,       "信号强度查询" },
	// { "AT+QLTS=2\r\n"                                   ,NULL,			"+QLTS:",        	ATRQLTS,		ATRbt,	1,    	eTick_3S,     	eAT_ClkQuery,       "实时时钟" },
	{ "AT+QNTP=1,\"ntp1.aliyun.com\"\r\n"               ,NULL,			"+QNTP:",        	NULL,		    ATRbt,	1,    	eTick_3S,     	eAT_NTPClkQuery,    "NTP实时时钟" },
	{ "AT+CGREG?\r\n"                                   ,NULL,			"+CGREG:",         	ATRCGREG,		ATRbt,	3,   	eTick_10S,     	eAT_PSQuery,       	"PS服务网络连接状态查询" },
	{ "AT+COPS?\r\n"                                    ,NULL,			"+COPS:",    		ATRCOPS,		ATRbt,	10,   	eTick_3S,     	eAT_OperatorQuery,	"运营商查询" },
	{ "AT+QNWINFO\r\n"                                  ,NULL,			"+QNWINFO:",   		ATRQNWINFO,		ATRbt,	3,   	eTick_10S,     	eAT_NetStateQuery,	"EC20网络连接模式查询" },
	{ "AT+QICSGP=1,1,\"[APN]\",\"\",\"\",0\r\n"      	,ATSQICSGP,		"+QICSGP",          ATRAck,			ATRbt,	3,   	eTick_10S,     	eAT_PDPSet,       	"EC20配置PDP上下文" },
	{ "AT+QIACT=1\r\n"                               	,NULL,			"+QIACT",          	ATRAck,			ATRbt,  3,   	eTick_10S,     	eAT_PDPAct,       	"激活PDP上下文" },
	{ "AT+QIACT?\r\n"                                   ,NULL,			"+QIACT",        	ATRQIACT,		ATRbt,	3,     	eTick_60S,    	eAT_PDPStateQuery,	"激活状态查询" },
	{ "AT+QIDEACT=1\r\n"                                ,NULL,			"+QIDEACT",        	ATRAck,			ATRbt,	3,     	eTick_40S,    	eAT_StationtClose,	"基站去激活" },
	{ "AT+QPOWD=0\r\n"                                	,MPwrDown,		"POWERED DOWN",		ATRQPOWD,		ATPWDF,	3,     	eTick_5S,    	eAT_Shutdown,		"关机" },
	
	//tcp
	{ "AT+QIOPEN=1,[ID],\"TCP\",\"[MIP]\",[MPORT],0,0\r\n" ,ATSQIOPEN,	"+QIOPEN", 			ATRQIOPEN,		ATFail, 3,		eTick_5S,		eAT_ConnectSet, 	"建立连接" },
	{ "AT+QIRD=[ID],1460\r\n" 							,ATSQIRD,		"+QIRD:", 			ATRQIRD,		ATFail,	2,		eTick_10S,		eAT_DataRead, 		"数据读取" },
	{ "AT+QISEND=[ID],[LEN]\r\n" 						,ATSQISEND,		"> ",       		ATRQISEND,		ATFail,	2,   	eTick_10S,    	eAT_DataWrite,   	"数据发送" },
	{ "AT+QICLOSE=[ID]\r\n" 							,ATSQICLOSE,	"+QICLOSE",       	ATRAck,			NULL,	3,   	eTick_3S,    	eAT_SocketClose,   	"关闭Socket服务" },
	{ "AT+CFUN=0\r\n" 							        ,NULL,	        "+CFUN",       	    ATRAck,			ATMdlRbt,	3,   	eTick_3S,    	eAT_CFUNpythy,   	"设置最小功能模式" },
	{ "AT+CFUN=1\r\n" 							        ,NULL,	        "+CFUN",       	    ATRCFUN1,		ATFail,	3,   	eTick_3S,    	eAT_CFUNall,   	    "设置全功能模式" },
	
	//ftp
	{ "AT+QFLDS=\"UFS\"\r\n"							,NULL,			"+QFLDS:",			NULL,			ATFail,	3,		eTick_3S, 		eAT_FTPFlashQuery,	"查询flash" },
	{ "AT+QFTPCFG=\"contextid\",1\r\n"					,NULL,			"contextid",		ATRFTPCFGc,		ATFail,	3,		eTick_3S, 		eAT_FTPConfig,		"ftp配置" },
	{ "AT+QFTPCFG=\"account\",\"[NAME]\",\"[PSW]\"\r\n" ,ATSQFTPCFGa,	"account",			ATRAck,			ATFail,	3,		eTick_3S, 		eAT_FTPNamePsw, 	"ftp用户名密码" },
	{ "AT+QFTPCFG=\"filetype\",0\r\n"					,NULL,			"filetype",			ATRAck,			ATFail,	3,		eTick_3S, 		eAT_FTPFileType,	"ftp文件类型" },
	{ "AT+QFTPCFG=\"transmode\",1\r\n"					,NULL,			"transmode",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_FTPTransType,	"ftp主动传输模式" },
	{ "AT+QFTPCFG=\"rsptimeout\",90\r\n"				,NULL,			"rsptimeout",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_FTPTimeout, 	"ftp响应超时值" },
	{ "AT+QFTPOPEN=\"[FIP]\",[FPORT]\r\n"				,ATSQFTPOPEN,	"+QFTPOPEN: 0,0",	ATRQFTPOPEN,	ATFail,	3,		eTick_30S, 		eAT_FTPIpPort,		"ftpIP和端口号" },
	{ "AT+QFTPCWD=\"[PATH]\"\r\n"						,ATSQFTPCWD,	"+QFTPCWD",			ATRQFTPCWD,		ATFail,	3,		eTick_3S, 		eAT_FTPPath,		"ftp路径" },
	{ "AT+QFTPCLOSE\r\n"								,NULL,			"+QFTPCLOSE: 0,0",	ATRQFTPCLOSE,	NULL,	3,		eTick_10S,		eAT_FTPClose,		"关闭ftp服务" },
	
	//UFS及file操作
	{ "AT+QFDEL=\"*\"\r\n"								,NULL,			"+QFDEL",			ATRAckQFDEL,    ATQFDELFail,	3,		eTick_3S, 		eAT_UFSDelete,		"删除ufs文件" },
	{ "AT+QFTPGET=\"[FILE]\",\"UFS:file\"\r\n"			,ATSQFTPGET,	"+QFTPGET:",		ATRQFTPGET,		ATFail,	3,		eTick_10S, 		eAT_FileDownload,	"file下载" },
	{ "AT+QFOPEN=\"UFS:file\",2\r\n"					,NULL,			"+QFOPEN:",			ATRQFOPEN, 		ATFail,	3,		eTick_1S,		eAT_FileOpen,		"file打开" },
	{ "AT+QFSEEK=[HANDLE],[OFFSET],0\r\n"				,ATSQFSEEK,		"+QFSEEK", 			ATRQFSEEK,		ATFail,	3,		eTick_1S,		eAT_FileSeek,		"file偏移" },
	{ "AT+QFREAD=[HANDLE],[LEN]\r\n"					,ATSQFREAD,		"CONNECT",			ATRQFREAD,		ATFail,	3,		eTick_3S,		eAT_FileRead,		"file读取" },
	{ "AT+QFCLOSE=[HANDLE]\r\n"							,ATSQFCLOSE,	"+QFCLOSE",			ATRAck,			ATFail,	3,		eTick_1S,		eAT_FileClose,		"file关闭" },
	{ "AT+QFUPL=cacert.pem,[LEN]\r\n"					,ATSQFUpl,		"CONNECT",			ATRQFUpl,		ATFail,	3,		eTick_1S,		eAT_QFUpload,		"上传文件" },
	{ ""												,NULL,			"+QFUPL:",			ATRAck,			ATFail, 1,		eTick_1S,		eAT_QFUploadAck,	"上传文件应答" },
	
	//SSL
	{ "AT+QSSLCFG=\"sslversion\",1,1\r\n"				,NULL,			"sslversion",		ATRAck,			ATFail,	3,		eTick_1S,		eAT_SSLVer,			"设置SSL版本" },
	{ "AT+QSSLCFG=\"ciphersuite\",1,0X0035\r\n"			,NULL,			"ciphersuite",		ATRAck,			ATFail,	3,		eTick_1S,		eAT_SSLSut,			"设置SSL加密套件" },
	{ "AT+QSSLCFG=\"seclevel\",1,1\r\n"					,NULL,			"seclevel",			ATRAck,			ATFail,	3,		eTick_1S,		eAT_SSLLvl,			"设置SSL验证级别" },
	{ "AT+QSSLCFG=\"cacert\",1,\"UFS:cacert.pem\"\r\n"	,NULL,			"cacert",			ATRAck,			ATFail,	3,		eTick_1S,		eAT_SSLCAPath,		"SSLCA证书路径" },
	
	//mqtt
	{ "AT+QIDNSCFG=1,\"[MDNS]\",\"[SDNS]\"\r\n"			,ATSDNSCFG,		"+QIDNSCFG",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTCfgDNS,		"MTDNS配置" },
	{ "AT+QMTCFG=\"qmtping\",[ID],30\r\n"				,ATSMTSocketID,	"qmtping",			ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTCfgHeartbeat,	"MT心跳配置" },
	{ "AT+QMTCFG=\"keepalive\",[ID],120\r\n"			,ATSMTSocketID,	"keepalive",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTCfgKeepalive,	"MT心跳超时配置" },
	{ "AT+QMTCFG=\"recv/mode\",[ID],0,1\r\n"			,ATSMTSocketID,	"recv/mode",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTRecvMode,		"MT接收模式" },
	{ "AT+QMTCFG=\"ssl\",[ID],1,1\r\n"					,ATSMTSocketID,	"ssl",				ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTSsl,			"MT设置ssl" },
	{ "AT+QMTCFG=\"aliauth\",[ID],\"[KEY]\",\"[NAME]\",\"[SECRET]\"\r\n",ATSDNSCFGali,"aliauth",ATRAck,		ATFail,	3,		eTick_3S, 		eAT_MTAliCfg,		"MT阿里配置" },
	{ "AT+QMTCFG=\"aliauth\",[ID]\r\n"					,ATSMTSocketID,	"aliauth",			ATRAck,			ATFail,	3,		eTick_3S, 		eAT_MTAliQuery,		"MT阿里查询" },
	{ "AT+QMTOPEN=[ID],\"[HOSTNAME]\",[PORT]\r\n"		,ATSMTOPEN,		"+QMTOPEN:",		ATRMTOPEN,		ATFail,	3,		eTick_30S, 		eAT_MTOPEN,			"MT打开客户端" },
	{ "AT+QMTCONN=[ID],\"[PID]\",\"[NAME]\",\"[SECRET]\"\r\n",ATSMTCONN,"+QMTCONN:",		ATRMTCONN,		ATFail,	3,		eTick_30S, 		eAT_MTCONN,			"MT连接服务器" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUB,		"+QMTSUB:",			ATRMTSUB,		ATFail,	3,		eTick_5S, 		eAT_MTSUBAck,		"MT订阅应答" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBGet,	"+QMTSUB:",			ATRMTSUB,		ATFail,	3,		eTick_5S, 		eAT_MTSUBGet,		"MT订阅下发" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBOta,	"+QMTSUB:",			ATRMTSUB,		ATFail,	3,		eTick_5S, 		eAT_MTSUBOta,		"MT订阅OTA" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBUser,	"+QMTSUB:",			ATRMTSUB,		ATFail,	3,		eTick_5S, 		eAT_MTSUBUserGet,	"MT订阅用户" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBNotify,"+QMTSUB:",			ATRMTSUBSucc,	ATFail,	3,		eTick_5S, 		eAT_MTSUBNotify,	"MT订阅通知" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBDown,	"+QMTSUB:",			ATRMTSUB,		ATFail,	3,		eTick_5S, 		eAT_JSSUBDown,		"MT订阅下行数据" },
	{ "AT+QMTSUB=[ID],[MID],\"[TOPIC]\",1\r\n"			,ATSMTSUBDownReply,"+QMTSUB:",		ATRMTSUBSucc,	ATFail,	3,		eTick_5S, 		eAT_JSSUBDownReply,	"MT订阅平台应答" },
	{ "AT+QMTPUBEX=[ID],[MSGID],1,0,\"[TOPIC]\",[LEN]\r\n",ATSMTPUBEX,	"> ",				ATRMTPUBEX,		ATFail,	1,		eTick_1S, 		eAT_MTPUBEX,		"MT推送" },
	{ ""												,NULL,			"+QMTPUBEX",		ATRPubRet, 		ATFail, 1,		eTick_3S,		eAT_MTPUBEXRET,		"MT推送结果" },
	{ "AT+QMTRECV=[ID],%d\r\n"							,ATSMTRECV,		"+QMTRECV:",		ATRMTRECV,		ATFail,	3,		eTick_3S, 		eAT_MTRECV,			"MT读取" },
	{ "AT+QMTCONN?\r\n"									,NULL,			"+QMTCONN:",		ATRQConn,		ATFail,	3,		eTick_3S, 		eAT_MTQueryConn,	"MT查询连接" },
	{ "AT+QMTDISC=[ID]\r\n"								,ATSMTSocketID,	"+QMTDISC:",		ATRDISC,		NULL,	3,		eTick_5S, 		eAT_MTDISC,			"MT断开连接" },
	{ "AT+QMTCLOSE=[ID]\r\n"							,ATSMTSocketID,	"+QMTCLOSE:",		ATRCLOSE,		NULL,	3,		eTick_5S, 		eAT_MTClose,		"MT关闭客户端" },
	
	//http
	{ "AT+QHTTPCFG=\"sslctxid\",1\r\n"					,NULL,			"sslctxid",			ATRAck,			ATFail,	3,		eTick_3S, 		eAT_HPSslId,		"HTTP配置设置SSL ID" },
	{ "AT+QHTTPCFG=\"contextid\",1\r\n"					,NULL,			"contextid",		ATRAck,			ATFail,	3,		eTick_3S, 		eAT_HPSetPDP,		"HTTP配置PDP" },
	{ "AT+QHTTPCFG=\"requestheader\",0\r\n"				,NULL,			"requestheader",	ATRAck,			ATFail,	3,		eTick_3S, 		eAT_HPRequestheader,"HTTP请求头" },
	{ "AT+QHTTPURL=[LEN],80\r\n"						,ATSUrl,		"CONNECT",			ATRUrl,			ATFail, 3,		eTick_3S,		eAT_HPSetURL,		"HTTP配置URL" },
	{ "AT+QHTTPGETEX=60,[ADDR],[LEN]\r\n"				,ATSDownload,	"+QHTTPGET:",		ATRHPGet,		ATFail, 3,		eTick_3S,		eAT_HPDownload,		"HTTP文件下载" },
	{ "AT+QHTTPREAD=60\r\n"								,NULL,			"CONNECT",			ATRHPREAD,		ATFail, 3,		eTick_3S,		eAT_HPRead,			"HTTP读文件" },

};


//////////////////////////////////////////////////////////////////////////
//函数名：		SIM900AUpDeal_ConnectOpen
//功能描述：	处理上报信息
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static U32 SIM900AUpDeal_ConnectOpen(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 ret = FALSE;
	U8 *pTemp = NULL;
	U32 u32Temp[4] = {0xff};
	char cExpect[AT_CMD_LEN] = {0};
	
	sprintf(cExpect ,"+QIOPEN: %d", SocketID);
	pTemp = SearchData((U8*)pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL != pTemp)
	{
		sscanf((char*)pTemp, "+QIOPEN: %d,%d\r", &u32Temp[0], &u32Temp[1]);
		if(SocketID == u32Temp[0] && 0 == u32Temp[1])
		{
			GPRS_Setonlineflag(SocketID, eSocket_Online);

            printf("\r\n---SERVER %d: CONNCT SUCC----\r\n\r\n", SocketID);

			LogPrintf(LVL_LOG_WARN, "\r\nsocket %d connect succ !!!\r\n", SocketID);
			ret = TRUE;
		}
		else
		{
			LogPrintf(LVL_LOG_ERR, "\r\nsocket %d connect fail reason %d !!!\r\n", SocketID, u32Temp[1]);
			Comm_PlatReconnect(SocketID, __LINE__);
		}
	}
	
	return ret;
}


//////////////////////////////////////////////////////////////////////////
//函数名：		SIM900AUpDeal_ConnectCloseOK
//功能描述：	处理上报信息
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
static U32 SIM900AUpDeal_URC_Close(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 *pDest = NULL;
	char cExpect[AT_CMD_LEN] = {0};
	
	sprintf(cExpect ,"\"closed\",%d\r", SocketID);
	pDest = SearchData(pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL == pDest) 
		return FALSE;
	
	LogPrintf(LVL_LOG_WARN, "\r\nURC recv closed %d ! \r\n", SocketID);
	Comm_PlatReconnect(SocketID, __LINE__);
	
	return TRUE;
}

static U32 SIM900AUpDeal_URC_Recv(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 *pDest = NULL;
	char cExpect[AT_CMD_LEN] = {0};
	
	sprintf(cExpect ,"\"recv\",%d\r", SocketID);
	
	//寻找接收到数据标志
	pDest = SearchData(pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL == pDest) 
		return FALSE;
	
	LogPrintf(LVL_LOG_INFO, "\r\n%s\r\n", cExpect);
	
	AddModemOptTask(SocketID, eAT_DataRead);
	return TRUE;
}
static U32 SIM900AUpDeal_URC(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 *pDest = NULL;
	
	pDest = SearchData(pData, nDataLen, "+QIURC:", strlen("+QIURC:"));
	if (NULL == pDest) 
		return FALSE;
	
	SIM900AUpDeal_URC_Recv(SocketID, pDest, nDataLen, pDealHead, pDealLen);

	SIM900AUpDeal_URC_Close(SocketID, pDest, nDataLen, pDealHead, pDealLen);
	
	return TRUE;
}


static U32 SIM900AUpDeal_RecvUserDataMT(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 *pDest = NULL, *pTemp = NULL, *pTopic = NULL, *pPayload = NULL, *pHead = NULL;
	S32 s32TopicLen = 0, s32PayloadLen = 0;
	U32 u32DealLen = 0;
	
	pDest = pData;
	pDealLen[0] = 0;
	
	while(nDataLen > strlen("+QMTRECV:"))
	{
		u32DealLen = 0;
		
		//寻找接收到数据标志
		pDest = SearchData(pDest, nDataLen, "+QMTRECV:", strlen("+QMTRECV:"));
		if (NULL == pDest) 
			break;
		//if (NULL == pHead)
			pHead = pDest;
		
		//topic "
		pDest = SearchData(pDest, nDataLen, "\"", strlen("\""));
		if (NULL == pDest) 
			break;
		pDest++;
		
	//	u32DealLen += ((U32)pDest - (U32)pData);
		
		pTemp = SearchData((pDest+1), nDataLen, "\"", strlen("\""));
		if (NULL == pDest) 
			break;
		
		//topic
		s32TopicLen = pTemp - pDest;
		if (s32TopicLen <= 0 || s32TopicLen > 128)
			break;
		//topic地址
		pTopic = pDest;
		
		//payload 长度
		pDest += s32TopicLen + 1;
		sscanf((char*)pDest, ",%d,", &s32PayloadLen);
		if (s32PayloadLen <= 0 || s32PayloadLen > 1024)
			break;
		
		//payload "
		pDest = SearchData(pDest, nDataLen, "\"", strlen("\""));
		if (NULL == pDest) 
			break;
		pDest++;
		pPayload = pDest;
		
	//	LogPrintf(LVL_LOG_INFO, "\r\n%s\r\n", cExpect);
		PalRecvPush((eDataQueueID)SocketID, eDataType_MQTT, pTopic, s32TopicLen, pPayload, s32PayloadLen);
		
	//	pDealHead[0] = pHead;
	//	pDealLen[0] += (((U32)pPayload - (U32)pHead) + s32PayloadLen);
		
		u32DealLen = (((U32)pDest - (U32)pHead) + s32PayloadLen);
		pDest += s32PayloadLen;
		
		if(nDataLen > u32DealLen)
		{
			nDataLen -= u32DealLen;
		}
		else
		{
			nDataLen = 0;
		}
	}
	
	return TRUE;
}

static U32 SIM900AUpDeal_RecvStateMT(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
	U8 *pDest = NULL;
	char cExpect[AT_CMD_LEN] = {0};
	U32 u32Temp[4] = {0xff};
	GprsModemDCB* pGprsModemDCB = &g_strGprsModemDCB;
	
	sprintf(cExpect ,"+QMTSTAT: %d,", SocketID);
	
	//寻找接收到数据标志
	pDest = SearchData(pData, nDataLen, cExpect, strlen(cExpect));
	if (NULL == pDest) 
		return FALSE;
	
	sscanf((char*)pDest, "+QIOPEN: %d,%d\r", &u32Temp[0], &u32Temp[1]);
	if(SocketID == u32Temp[0])
	{
		if(0 != u32Temp[1])
		{
			Comm_PlatReconnect(SocketID, __LINE__);
		}
	}
	
	return TRUE;
}

static U32 SIM900AUpDeal_RecvQNTP(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen)
{
    //会出现滞后回复的情况，所以需要轮询处理，滞后10s左右
    ATRQNTP(SocketID, pData, nDataLen);
	return TRUE;
}

//GPRS上报集合
const ATUpItem SIM900A_UPAT[] =
{
	{SIM900AUpDeal_ConnectOpen ,"+QIOPEN:"},
	// {SIM900AUpDeal_ConnectClose ,"+QIURC: \"closed\""},
	// {SIM900AUpDeal_RecvUserData ,"+QIURC: \"recv\""},
	{SIM900AUpDeal_RecvUserDataMT ,"+QMTRECV"},
	// {SIM900AUpDeal_RecvUserDataMT ,"+QMTRECV:"},
	// {SIM900AUpDeal_RecvStateMT ,"+QMTSTAT"},
	
//	{SIM900AUpDeal_StationtClose ,"+QIURC: \"pdpdeact\""},
//	{SIM900AUpDeal_Shutdown ,"POWERED DOWN"},
	{SIM900AUpDeal_RecvQNTP ,"+QNTP:"},
	{SIM900AUpDeal_URC ,"+QIURC"},
};

//模块列表
const ModemItem SIM900AOPTITEM = {
	115200 ,                                         			//波特率
	SIM900A_DIALUPATCMD ,                                    	//拨号指令控制集合
//	SIM900A_DATACMD ,                                         	//数据指令控制集合
	SIM900A_UPAT ,                                           	//GPRS上报集合
	FCNT(SIM900A_DIALUPATCMD) ,                       			//控制集合个数
//	FCNT(SIM900A_DATACMD) ,                       				//数据集合个数
	FCNT(SIM900A_UPAT) ,                                   		//上报集合个数
	MD_EC600N ,                                             	//模块类型
};

//////////////////////////////////////////////////////////////////////////
//以下为外部调用函数

//////////////////////////////////////////////////////////////////////////
//函数名：		GetSIM900AOptItem
//功能描述：	指定SIM900A操作元素
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
const ModemItem* GetSIM900AOptItem(void)
{
	return &SIM900AOPTITEM;
}
