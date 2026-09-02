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

#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  COMMUNICATION_GLONALS
#define COMMUNCIATION_EXT
#else
#define COMMUNCIATION_EXT extern
#endif

#include "Libinclude.h"


//////////////////////////////////////////////////////////////////////////
#define MD_NULL				0x00							//无效模块
#define MD_SIM900A			0x01							//SIMCOMM_SIM900A
#define MD_EC600N			0x02							//移远EC600N

//////////////////////////////////////////////////////////////////////////
//操作模式
#define OPT_TYPE_NULL		0x00							//
#define OPT_TYPE_CMD		0x01							//指令模式
#define OPT_TYPE_DATA		0x02							//数据模式

//socket数量
#define SOCKET_CNT_0		0x00							//
#define SOCKET_CNT_1		0x01							//单链接
#define SOCKET_CNT_2		0x02							//双链接


#define OTA_URL_LEN			512								//
#define TCP_URL_LEN			256

//////////////////////////////////////////////////////////////////////////
#define YD_APN 				"CMIOT"
#define LT_APN 				"3GNET"
#define DX_APN 				"CTNET"

/* 联网方式*/
typedef enum {
    eNetType_GPRS     		= 0,	//GPRS
    eNetType_LAN,                   //LAN
} eNetType;


//服务商
enum 
{
	eOperator_NULL    		= 0x00, // 空
	eOperator_CMCC,					//移动
	eOperator_CUCC,					//联通
	eOperator_CTCC,					//电信
	eOperator_OTHER,				//其他
};
//////////////////////////////////////////////////////////////////////////
//链接控制属性
//////////////////////////////////////////////////////////////////////////
//SocketID
typedef enum {
    eSocket_GPRS1     		= 0,	//GPRS socket1
    eSocket_GPRS2,    				//
    eSocket_Cnt, 					//1单联接，2双链接，3三链接
    eSocket_Monitor,                //监管平台
}eNetSocket;

//Socket等级
typedef enum {
    eSocket_LvNull     		= 0,	//
    eSocket_LvMain,					//主
    eSocket_LvSlave,    			//从
}eSocketLevel;

//AT模块状态，AT异常(AT指令2min无回复或者长时间2h无数据交互)必须要重启模块
typedef enum {
    eATStatus_NULL = 0,
    eATStatus_Normal= 1,
    eATStatus_Abnormal = 2,
}eATStatus;

//Socket状态
typedef enum {
    eSocket_Idle     		= 0,	//
    eSocket_Neting,    				//网络连接中
//    eSocket_Hand,    				//握手
//    eSocket_Identify,    			//识别
//    eSocket_Config,    				//配置
    eSocket_Attach,    				//附着基站,当模块连接到eSocket_Attach后才可进行服务器连接等后续操作,基站连接无异常
    eSocket_Online,    				//服务器连接成功
    eSocket_Reconnect,				//
}eSocketState;

//Socket协议
typedef enum {
    eSocket_TCP     		= 0,	//
    eSocket_FTP,    				//
    eSocket_MQTT,    				//
    eSocket_HTTP,    				//
}eSocketType;

//AT模式
typedef enum {
    eSocket_MdNull     		= 0,	//
    eSocket_MdCmd,    				//指令模式
    eSocket_MdData,    				//数据模式
}eATMode;

//指令控制
typedef enum 
{
    eAT_Null             	= 0x00, // 空 
    eAT_PowerOn,     				// 上电开机

//    eAT_DataOK,     				// 数据模式OK
    eAT_OnQuery,     				// 开机查询
    eAT_SIMQuery,     				// SIM卡状态查询
    eAT_SIMStateQuery,     			// SIM卡识别状态查询
    eAT_SIMICCIDQuery,     			// SIM卡ICCID码查询
    eAT_CSQQuery,     				// 信号强度查询
    eAT_ClkQuery,     				// 实时时钟
    eAT_PSQuery,     				// PS服务网络连接状态查询
    eAT_OperatorQuery,     			// 运营商查询
    eAT_NetStateQuery,     			// EC20网络连接模式查询
    eAT_PDPSet,     				// EC20配置PDP上下文
    eAT_PDPAct,     				// 激活PDP上下文
    eAT_PDPStateQuery,     			// 激活状态查询
	eAT_StationtClose,				// 基站去激活
	
	//tcp
    eAT_ConnectSet,     			// 建立连接
    eAT_DataRead,     				// 数据读取
    eAT_DataWrite,     				// 数据发送
    eAT_ConnectQuery,     			// 查询连接
	eAT_SocketClose,				// 关闭Socket服务
	eAT_CFUNpythy,				    // 设置最小功能模式
	eAT_CFUNall,				    // 设置全功能模式
	
	//ftp
    eAT_FTPFlashQuery,     			// 查询flash
    eAT_FTPConfig,     				// ftp配置
    eAT_FTPNamePsw,     			// ftp用户名密码
    eAT_FTPFileType,     			// ftp文件类型
	eAT_FTPTransType,				// ftp主动传输模式
	eAT_FTPTimeout,					// ftp响应超时值
	eAT_FTPIpPort, 					// ftpIP和端口号
	eAT_FTPPath, 					// ftp路径
//	eAT_FTPBinSize, 				// ftp文件大小
	eAT_FTPClose,					// 关闭ftp服务
	
	//UFS及file操作
	eAT_UFSCatch, 					// ftp删除ufs文件
	eAT_UFSDelete, 					// ftp删除ufs文件
	eAT_FileDownload,				// 下载
	eAT_FileOpen,					// 打开
	eAT_FileClose,					// 关闭
	eAT_FileSeek,					// 偏移
	eAT_FileRead,					// 读取
	eAT_QFUpload,					// 上传文件
	eAT_QFUploadAck,				// 上传文件应答
	
	//mqtt
    eAT_MTCfgDNS,     				// MTDNS配置
    eAT_MTCfgHeartbeat,     		// MT心跳配置
    eAT_MTCfgKeepalive,     		// MT心跳超时配置
    eAT_MTRecvMode,     			// MT接收模式
    eAT_MTSsl,     					// MT设置ssl
    eAT_MTAliCfg,     				// MT阿里配置
    eAT_MTAliQuery,     			// MT阿里查询
	eAT_MTOPEN,     				// MT打开客户端
	eAT_MTCONN,     				// MT连接服务器
	eAT_MTSUBAck,     				// MT订阅应答
	eAT_MTSUBGet,     				// MT订阅下发
	eAT_MTSUBOta,     				// MT订阅OTA
	eAT_MTSUBUserGet,     			// MT订阅用户
	eAT_MTSUBNotify,     			// MT订阅通知
	//捷顺
	eAT_JSSUBDown,     				// MT订阅平台下行数据
	eAT_JSSUBDownReply,     		// MT订阅平台下行数据应答

	eAT_MTPUBEX,     				// MT推送
	eAT_MTPUBEXRET,					// MT推送结果
	eAT_MTRECV,     				// MT读取
	eAT_MTQueryConn, 				// MT查询连接
	eAT_MTDISC, 					// MT断开连接
	eAT_MTClose,					// MT关闭客户端
	
	//SSL
    eAT_SSLVer,     				// SSL设置 SSL 版本
    eAT_SSLSut,     				// SSL设置 SSL 加密套件
    eAT_SSLLvl,     				// SSL设置 SSL 验证级别
    eAT_SSLCAPath,     				// SSLCA 证书路径
//    eAT_SSLCAClient,     			// SSL客户端证书路径
//    eAT_SSLCAKey,     				// SSL客户端密钥
	
	//http
    eAT_HPSslId,     				// HTTP设置 SSL 上下文 ID
    eAT_HPSetPDP,     				// HTTP配置PDP
    eAT_HPRequestheader,     		// HTTP请求头
    eAT_HPSetURL,     				// HTTP配置URL
	eAT_HPDownload,					// HTTP文件下载
    eAT_HPRead,     				// HTTP读文件
    
	//
	eAT_Shutdown,					// 关机
	
    //补充一些AT指令，避免前面数字变化，跟到后面
    eAT_NTPClkQuery,     				// ntp服务器获取时间
    
	eAT_MAXCnt,
}AT_CMDOPT_E;

//捷顺http类型
enum 
{
	eJSHttp_Ota    			= 0x00, // ota
	eJSHttp_Param,					// 获取登录参数
};

#define JS_PARAM_SUCC 		"JCLOUD0001"	//捷顺一键登录参数获取成功
#define JS_PARAM_FAIL 		"JCLOUD0002"	//捷顺一键登录参数获取失败

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//应用层数据
#define AT_CMD_LEN            		36

#define AT_CMD_LEN_LONG            	64

#define AT_OPT_CNT        			(((U32)eAT_MAXCnt)*((U32)eSocket_Cnt))

//////////////////////////////////////////////////////////////////////////

typedef U32 (*ATPSend)(eNetSocket SocketID, U8* pBuf, U32 nATLen);

typedef U8 (*ATPRecv)(eNetSocket SocketID, U8* pData, U32 nDataLen);

typedef U8 (*ATPFail)(eNetSocket SocketID, U8 ATFunc);

//操作指令
typedef struct __AT_CMD_ITEM__
{
	char	cAT[80] ;									//AT指令
    ATPSend	pSend;
	char	cATAnswer[AT_CMD_LEN] ;						//AT指令预计回码
    ATPRecv	pRecv;
    ATPFail	pFail;
	
	U8		sendCnt ;									//AT指令尝试次数
	U32		waitDelay ;									//AT指令等待延时
	U8		ATFunc ;									//AT功能
	char	cMeanings[AT_CMD_LEN] ;						//AT指令含义
}ATCmdItem;

//上报数据对应处理函数
typedef U32 (*GprsUpDealFunc)(eNetSocket SocketID, U8* pData, U32 nDataLen, U8* *pDealHead, U32* pDealLen) ;

//GPRS/CDMA 数据打包函数
typedef U32 (*PackGprsDataFunc)(U8 *pData ,U32 nDataLen) ;

//上报数据
typedef struct __AT_UP_ITEM__
{
	GprsUpDealFunc	pFunc ;								//上报处理函数
	char			cAT[24] ;							//上报字符串
}ATUpItem ;

//模块操作信息
typedef struct __MODEM_AT_ITEM__
{
	U32		BaudRate ;									//波特率
	const ATCmdItem* pATCmd ;							//AT指令集
//	const ATCmdItem* pDataCmd ;							//数据指令集
	const ATUpItem* pUpCmd ;							//上报信息
	U8		CmdNum ;									//AT指令个数
//	U8		DataCmdNum ;								//数据指令个数
	U8		UpNum ;										//上报指令个数
	
	U8		modemType ;									//模块类型
}ModemItem ;
//////////////////////////////////////////////////////////////////////////

typedef struct __SOCKET_DCB__
{
	/*socket模块相关变量*/
	U8 SocketID;
	eSocketLevel SocketLevel;
	eSocketState ConnectState;
	eSocketType SocketType;								//连接类型
	eATMode ATMode;										//操作模式 1指令 2数据
	
	U8 ReconnectMaxCnt;									//socket最大重连次数，每个连接都达到最大连接次数后，重启模块，其他冲关联次数为0，重置0
	U8 ReconnectCnt;									//重连次数
	U32 ReconnectTime;								    //socket重连时间
	U32 ReconnectInterval;								//socket重连时间间隔，单位s
	U32 ATConnTick;										//Connect计数器
	
	U32 ATDataTaskTick; 								//AT数据任务暂停计数器
	U32 ATDataReadTick; 								//AT数据读暂停计数器
}SocketDCB;

typedef struct __AT_CMD_DCB__
{
	AT_CMDOPT_E ATCmdTask;
	eNetSocket SocketID;					//ID

}ATCmdDCB;

typedef enum {
    eATMDDataSta_Normal,    			//
    eATMDDataSta_Reconnect,    			//
}eATMDDataSta;

typedef enum {
    eModelRebootsta_Normal,    			//
    eModelRebootsta_ATReconnect,    			//
    eModelRebootsta_ModelReboot,    			//
}eModelRebootsta;

typedef struct __ATMD_DATA__
{
	U8 RebootSta;									    //0正常使用， 1ATCFUN重连中，2模组重启中
	U8 RebootNetCnt;									//模块重启次数,成功后清零
	U8 RebootExecute;									//执行重启指令
	U32 RebootTick;										//重启计数器
	// U8 RebootNetFlag;							    //模块需要重启

    //==================网络步骤
	eNetType NetWorkType;							    //联网类型，0:4G, 1:LAN


	U8	OperatorType;									//运营商
	
	char SIMID[25] ;									//SIM卡串号
	U8	Csq ;											//信号强度
}ATMDData;

typedef struct __FTP_INFO__
{
	char ftpIp[64];
	uint16_t ftpPort;
	char ftpUserName[32];
	char ftpPassword[32];
	char ftpPath[32];
	char ftpFileName[32];
}FTPinfo;

typedef struct __FTP_DATA__
{
	eDownloadState DownloadState;
	eFileType FileType;
	FTPinfo info;

	U8	u8TryCnt;										//重试次数
	//文件
	U32	u32FileHandle;									//文件句柄
	U32 u32FileSize;									//文件大小
	//分包
	U8	u8PackCnt;										//包数量
	U8	u8PackIndex;									//包序号
	U8	u8PackSize;										//包大小
	//分帧
	U32	u32FrameCnt;									//帧数量
	U32	u32FrameIndex;									//帧序号
	U32	u32FrameLen;									//包序号
	U32	u32FinalFrameLen;								//最后一帧长度
	//读
	U32 u32ReadOffDest;									//ftp文件偏移
	U32 u32ReadLen;										//ftp读取长度
}FtpDCB;

typedef struct __MQTT_DATA__
{
	U32 ATMTStateTick;									//连接状态查询计数器
	
}MqttDCB;

typedef struct __HTTP_DATA__
{
	U16		u16UrlLen;
	char	C8OtaUrl[OTA_URL_LEN];

	U8		u8JSOtaOrParam;
	U8		u8JSParamSucc;
}HttpDCB;

typedef struct __TCP_DATA__
{
	U16		u16UrlLen;
	char	C8OtaUrl[TCP_URL_LEN];
}TcpDCB;


//////////////////////////////////////////////////////////////////////////
//GPRS/CMDA 通讯总控制结构
typedef struct __GPRS_MODEM_DCB__
{
	U8	ModemIndex;										//模块识别序号
	//==================
	U8  SocketCnt;                                   	//当前socket的数量
	ATCmdDCB ATOptTask[AT_OPT_CNT];						//AT操作任务队列
	//==================电源控制
	U32 PowerStep;										//电源开机步骤
	//==================周期任务
	U32 ATCsqTick;										//Csq查询计数器
	U32 ATClkTick;										//Clk查询计数器
	//==================AT命令
	U8 ATWaitFlag;										//AT指令标志
	U8 ATTryCnt;										//AT指令尝试次数
	U32 ATWaitTick;										//AT指令等待
	U32 ATWaitDelay;									//AT指令等待延时
	//==================数据模式	
	//AT任务是阻塞的
	U8 UserDataTxState;									//用户数据发送状态
	eNetSocket UserDataTxSocket;
	
	U8 TCPDataTxState;									//用户数据发送状态
	eNetSocket TCPDataTxSocket;

	U8 UrlTxState;										//Url发送状态
	eNetSocket UrlTxSocket;
	
	U8 CATxState;										//CA发送状态
	eNetSocket CATxSocket;
	U32 u32CATxIndex;									//CA发送偏移索引
	U32 u32CATxLen;										//CA发送长度

	//==================ftp控制
	FtpDCB strFtpDCB;
	//==================mqtt控制
	MqttDCB strMqttDCB;
	//==================http控制
	HttpDCB strHttpDCB;
	//==================tcp控制
	TcpDCB strTcpDCB;

    
	//==================模块信息
	ATMDData strATMDData;								//模块信息数据
    
	U32 ATNormalTime;									//模块正常应答时间，1分钟无任何数据重启模块
    eATStatus ATAbnormal;								//模块异常无数据返回,0刚开机，1正常运行，2异常需要重启
	SocketDCB strSocketDCB[eSocket_Cnt];
}GprsModemDCB;
//////////////////////////////////////////////////////////////////////////

typedef struct __GPRS_MODEM_STA__ {
	U8	Socketx_IpNetSta[SOCKET_CNT_2];		//网络连接状态，0连接中(开关机开机置0)，1:ip连接中，2:ip连接成功
}GprsModemSTA;

//////////////////////////////////////////////////////////////////////////
//变量申明
//////////////////////////////////////////////////////////////////////////
COMMUNCIATION_EXT GprsModemDCB g_strGprsModemDCB;			//调制解调器控制数据


//////////////////////////////////////////////////////////////////////////
//方法:		ATSendWait
//全名:		ATSendWait
//函数说明: 
//访问:		  
//参数:		无
//返回值:	无
//注释:
//////////////////////////////////////////////////////////////////////////
void ATCmdWait(U8 falg, U32 u32WaitTick);

//////////////////////////////////////////////////////////////////////////
//函数名：		StartModemOpenTask
//功能描述：	启动开机任务
//入口参数：	U32 bNeedRePowerOn
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void StartModemOpenTask(void);
void StartModemReOpenTask(void);

U8 StartModemReconnectTask(eNetSocket SocketID);    //socket重连

U8 ReStartATModem(void);    //模组重连

//////////////////////////////////////////////////////////////////////////
//函数名：		MTRecvDecode
//功能描述：	接收信息处理
//入口参数：	U8 *pData	:收到的信息首地址
//				U32 nDataLen	:收到的信息长度
//函数返回值：	U32 :处理后的数据长度
//////////////////////////////////////////////////////////////////////////
void MTRecvDecode(U8 *pData ,U32 nDataLen);

//////////////////////////////////////////////////////////////////////////
//函数名：		CommunicationServer
//功能描述：	系统与主站通讯服务程序
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
void CommunicationServer(void);

extern void RunCommunicationTaskInit(void);
extern void DetectTaskInit(void);


void CommHttpsStart(eNetSocket Socket);

void CommFtpInit(eNetSocket Socket);

eSocketState GPRS_Getonlineflag(eNetSocket SocketID);

void GPRS_Setonlineflag(eNetSocket SocketID, eSocketState SocketState);

U8 Comm_FillHttpUrl(U8 *pUrl, U16 len);
U8 GPRS_SocketCnt(void);

U32 GPRSIsTxData(eNetSocket SocketID);

eSocketType GprsGetSocketType(void);

#define SOCKET_CNT	GPRS_SocketCnt()

U8 Comm_FillHttpUrl(U8 *pUrl, U16 len);


U8 tcpSwitchFtp(eNetSocket SocketID);
U8 ftpSwitchTcp(eNetSocket SocketID);

void runCommunicationTask(void);
#ifdef __cplusplus
}
#endif

#endif
