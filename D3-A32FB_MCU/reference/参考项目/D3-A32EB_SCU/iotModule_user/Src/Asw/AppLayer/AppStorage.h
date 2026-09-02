#ifndef __APP_STORAGE_H__
#define __APP_STORAGE_H__

#include "AppHeaderSummary.h"


#define STO_GUN_CNT					2

#define QRCODE_LEN  256     //二维码长度

#define   RATE_MODEL_MAX_LEN      		1020		//计费模型存储最大长度

/*ota*/
#define OTA_FRAME_SIZE        		1024		//下载一帧长度1024
#define OTA_MAX_SIZE        		(1024*384)	//




/*保存故障记录*/
#define ERR_RCD_ENBLE	  		  	FALSE	//

#define PLAT_CONN_TYPE_DOMAIN		0		//域名连接
#define PLAT_CONN_TYPE_IP	  		1	  	//ip连接

#define PLAT_DOMAIN_LEN	  			64	  	//

#define CHG_RCD_MAX_NUM	  		  	512	  	//充电记录最大条数

#define EVE_CACHE_MAX_CNT  			16      // 

/*参数结构体长度固定，防止升级后参数改变*/
#define STO_UPDATA_PARA_LEN	  		256	  	//
#define STO_SYSTEM_PARA_LEN	  		256	  	//
#define STO_CHARGE_PARA_LEN	  		1024	//

#define STO_CHARGE_RCD_SIZE	  		512		//
#define STO_RATE_MODEL_SIZE	  		200		
#define STO_QR_DATA_SIZE	  		256		//



//GNIot
#define IOT_GN_PKLEN				11		//ProductKey
#define IOT_GN_DNLEN	  			12	  	//DeviceName
#define IOT_GN_PSLEN	  			16	  	//ProductSecret
#define IOT_GN_DSLEN	  			32	  	//DevMsgSecret

//JSIot
#define IOT_JS_ACTLEN				16		//Account
#define IOT_JS_PSDLEN	  			13	  	//Password
#define IOT_JS_CIDLEN	  			16	  	//ClientId
#define IOT_JS_PTYPE	  			32	  	//product_type


#define JS_PERIOD_MAX_CNT			48		//捷顺计费时段最大数量

#define JS_LOGIN_PARAM_NULL			0		//捷顺一键注册未完成
#define JS_LOGIN_PARAM_OK			1		//捷顺一键注册完成


#define BLE_GN_PINLEN	  			6	  	//BLE PIN


//通用
#define CHG_RCD_MAX_NUM	  		  	512	  	//充电记录最大条数

#define FLA_QR_SIZE	  				256		//二维码长度
#define FLA_RATE_MODEL_SIZE	  		256		//计费模型长度
#define FLA_CHARGE_RCD_SIZE	  		512		//充电记录长度


enum 
{ 
    eModule_YFY             		= 0x00,     // 英飞源
    eModule_YKR                     = 0x01,     // 英科瑞
    eModule_YYLN                    = 0x02,     // 优优绿能
};
	
enum 
{ 
    eCardReader_GN             		= 0x00,     // 读卡器类型公牛
    eCardReader_MT            		= 0x01,     // 读卡器类型铭特
};





//以下关于平台参数配置信息
#define PLAT_DNS_LEN	64
#define FIX_NUMBER_LEN	32
#define PLAT_NUMBER_LEN	32
#define UPDATA_BODY_SIZE	123
#define RSA_KEY_LEN			128

typedef enum {
	ePlatType_GN,		                    //公牛平台
	ePlatType_GNP,		                    //快速直连平台
	ePlatType_YKC,		                    //云快充平台
	ePlatType_ANPEI,                        //浙江国网安培平台
	ePlatType_DKY,		                    //达克云平台
    ePlatType_YKC_V2,                       //云快充平台2.1
	ePlatType_TOWER,		                //铁塔平台
	ePlatType_DD,		                    //东电平台
	ePlatType_AHTT,		                    //安徽铁塔低速
	ePlatType_HaiNCT,		                //海宁城投
	ePlatType_WJY,		                	//蔚景云
	ePlatType_GNIOT,	                    //公牛智家
	ePlatType_JSIOT,		                //捷顺平台
	ePlatType_gwYKC,		                //国网云快充1.6接入平台


    ePlatType_WJYK,							// 蔚景云-快
	ePlatType_AHCHG,						// 安徽充换电	
	ePlatType_WLD,							// 卫莱电
	ePlatType_XJK,							// 小桔（快）
	ePlatType_YS,							// 云杉
	ePlatType_TLD,							// 特来电
	ePlatType_WJYB,							// 蔚景云（标）
	ePlatType_QWJ,							// 芊万佳
	ePlatType_XX,							// 小象
	ePlatType_ZQZC,							// 朱雀智充
	ePlatType_BJYCW,						// 北京易充网
	ePlatType_GWYCD,						// 国网易充电
	ePlatType_JNJTJT,						// 济南静态交通
	ePlatType_XJB,							// 小桔 （标）
	ePlatType_NFDW,							// 南方电网（易顺充）
	ePlatType_XXCD,							// 星星充电
	ePlatType_PTXNY,						// 普天新能源 
	ePlatType_FYJG,							// 阜阳监管 
	ePlatType_AHLLCD,						// 安徽洛洛充电
	ePlatType_BSC,							// 巴士充
	ePlatType_GWYC,							// 国五云充

   
	
	ePlatType_END,							// 结束	
} PLAT_TYPE;

typedef enum {
	CARD_GN,
	CARD_DKY,
	CARD_YKC,
    CARD_AHTT,
    CARD_HNCT,
    CARD_MAX,
} CARD_TYPE;



struct platECharge {
	uint8_t u8SN[64];
	uint8_t u8LicenseCode[64];

	uint8_t plat_Realese[128];			//运营平台参数预留
};
struct platJSData {
    //捷顺3元组--82bytes
    char C8JSAccount[IOT_JS_ACTLEN+1];
    char C8JSPassword[IOT_JS_PSDLEN+1];
    char C8JSClientId[IOT_JS_CIDLEN+1];
    char C8JSProductType[IOT_JS_PTYPE+1];
    uint8_t u8JSParamStep;				//一键注册状态

	uint8_t plat_Realese[174];			//运营平台参数预留
};

struct platGNData {
    //公牛智家/阿里5元组--80bytes
    char C8ProductKey[IOT_GN_PKLEN+1];
	char C8DeviceName[IOT_GN_DNLEN+1];
	char C8ProductSecret[IOT_GN_PSLEN+1];
	char C8DeviceSecret[IOT_GN_DSLEN+1];
	uint8_t U8Pid[4];
	uint8_t u8BindFlag;					//绑定标志 1已绑定

	uint8_t plat_Realese[176];			//运营平台参数预留
};


union PlatInfoStorage
{
    struct platECharge E_Info;  //北京e充网
    struct platJSData JS_Info;  //捷顺平台信息
    struct platGNData GN_Info;  //公牛智家信息
};

//关于平台相关信息存储
typedef struct _PlatCfgInfo {
	//4bytes
    uint8_t u8CtrlWord[4];                  //校验使用

	//186bytes
	char char16fixDeviceNumber[16];         //唯一桩编码，出厂不可更改, 废弃
	char pltDeviceNumber[PLAT_NUMBER_LEN];  //连接平台编码，可被更改
	uint8_t PltAuxiliaryIp[PLAT_DNS_LEN];	//辅平台ip
	uint16_t PltAuxiliaryPort;		        //辅平台端口
	uint8_t PltMainIp[PLAT_DNS_LEN];		//主平台ip
	uint16_t PltMainPort;				    //主平台端口
	uint8_t PltMainCardType;			    //主平台刷卡类型
	uint8_t PltMainType;				    //主平台类型
	uint8_t trans_type;						//传输层0-at,1-modbus，未使用，预留
    
	//256bytes
    union PlatInfoStorage platInfo ;        //预留总占位256

	//512 - (4 + 186 + 80 + 82) = 160
	uint8_t plat_Realese[33];			//运营平台参数预留

	char fixDeviceNumber[FIX_NUMBER_LEN];   //唯一桩编码，出厂不可更改，为了兼容老桩，结构体长度不变，从后面预留部分出来作为资产码
} PlatCfgInfo;

extern PlatCfgInfo g_pltCfgInfo;


//以下关于二维码信息
typedef struct
{
	//4bytes
    uint8_t u8CtrlWord[4];                  //校验使用
	char	qrcodeInfo[GUN_NUM_MAX][QRCODE_LEN];
    uint8_t au8_Res[508]; //预留
} stu_QrCodeInfo_t;
extern stu_QrCodeInfo_t g_QrCodeInfo;

//以下关于升级状态信息
typedef struct
{
	uint8_t	eBootOtaState;          //升级状态
	// uint8_t u8OtaState;					//升级状态
    uint32_t verBL;                     //bootloader软件版本        
    uint8_t au8_Res[UPDATA_BODY_SIZE]; //
} stu_UpdataCfg_t;

extern stu_UpdataCfg_t g_pstuUpdataCfg;

typedef struct _RATE_MODEL_PARA
{
    uint8_t u8CtrlWord[4];
    uint8_t RateModelData[RATE_MODEL_MAX_LEN];
	
}RATE_MODEL_PARA;


typedef struct _UPDATA_PARA{
    uint8_t UpFlag;
    uint8_t u8release[123];
} UPDATA_PARA;

typedef struct _F_UPDATA_PARA{
    uint8_t u8CtrlWord[4];
	UPDATA_PARA strUpdataPara;
	
} F_UPDATA_PARA;

typedef struct _S_CHARGE_RECORD
{
	uint8_t RcdType[2];									//记录类型
	uint8_t CurNum[4];									//记录数量
	uint8_t ChgRcdValidFlag[(CHG_RCD_MAX_NUM+7)>>3]; 	//有效标记
	uint8_t ChgRcdBillFlag[(CHG_RCD_MAX_NUM+7)>>3];		//结算标记
	uint8_t ChgRcdUpFlag[(CHG_RCD_MAX_NUM+7)>>3];		//上报标记
//	uint8_t ChgRcdCardNo[CHG_RCD_MAX_NUM*4];			//卡号
}S_CHARGE_RECORD;

typedef struct _SAMPLE_RECORD
{
    uint8_t u8CtrlWord[4];
	S_CHARGE_RECORD strSampleRecord;
//	uint8_t u8SecReserve[2048];					//未来记录开始结束时间等，应对记录查询需求
}SAMPLE_RECORD;

typedef struct _CHG_RCD_DATA{
    uint8_t u8Reserve[STO_CHARGE_RCD_SIZE];
} CHG_RCD_DATA;

typedef struct _CHARGE_RECORD{
    uint8_t u8CtrlWord[4];
	CHG_RCD_DATA strRcd;
} CHARGE_RECORD;

typedef struct _SYSTEM_PARAM
{
	char    C8BlePin[BLE_GN_PINLEN+1];//解锁密码
    uint8_t u8_ChargeMode;//充电模式
    uint8_t u8_GBQBMode;//国企标模式
    uint8_t u8_PilePECompMode;//桩体接地兼容模式
    uint8_t u8_ScheduleEnable;    //预约使能
    uint8_t u8_ScheduleEndedSelect; //预约结束选择充满即停标志
    uint8_t au8_ScheduleStartTime[2];//预约开始时间 00:00
    uint8_t au8_ScheduleEndedTime[2];//预约结束时间 23:59
    uint8_t u8Para[236];//max=252-16
}SYSTEM_PARAM;

typedef struct _F_SYSTEM_PARAM
{
    uint8_t u8CtrlWord[4];
    SYSTEM_PARAM strSysPara;
}F_SYSTEM_PARAM;

typedef struct _CHARGE_PARAM
{
	uint8_t     u8Para[252];
}CHARGE_PARAM;

typedef struct _F_CHARGE_PARAM
{
    uint8_t u8CtrlWord[4];
    CHARGE_PARAM strChargePara;
}F_CHARGE_PARAM;


typedef struct _UP_PLAT_PARAM
{
    uint8_t PlatAddr[PLAT_DOMAIN_LEN];	//平台地址
    uint8_t PlatPort[4];           	 	//平台端口
    uint8_t PlatType[4];           	 	//平台类型 
	
	char C8ProductKey[IOT_GN_PKLEN+1];
	char C8DeviceName[IOT_GN_DNLEN+1];
	char C8ProductSecret[IOT_GN_PSLEN+1];
	char C8DeviceSecret[IOT_GN_DSLEN+1];
	uint8_t U8Pid[4];
	uint8_t u8BindFlag;					//绑定标志 1已绑定
	
    uint8_t PlatRealese[100];         	//运营平台参数预留
    
}UP_PLAT_PARAM;

typedef struct _F_PLAT_PARAM
{
    uint8_t u8CtrlWord[4];
	UP_PLAT_PARAM strUpPlatPara;
}F_PLAT_PARAM;


typedef struct
{
	uint8_t QR_data[STO_GUN_CNT][STO_QR_DATA_SIZE];
}QR_DATA;

typedef struct _QR_PARA
{
	uint8_t u8CtrlWord[4];
	QR_DATA strQRData;
}QR_PARA;

typedef struct _S_EVE_RECORD
{
	uint8_t EveCnt[2];
	uint8_t CurNum[4];
	uint8_t EveValidFlag[(CHG_RCD_MAX_NUM+7)>>3]; 	//有效标记
	uint8_t EveStateFlag[(CHG_RCD_MAX_NUM+7)>>3];	//标记
	uint8_t EveUpFlag[(CHG_RCD_MAX_NUM+7)>>3];		//上报标记
}S_EVE_RECORD;

typedef struct _SAMPLE_EVERECORD
{
    uint8_t u8CtrlWord[4];
	S_EVE_RECORD strSampleEveRecord;
}SAMPLE_EVERECORD;


struct Eve_Time
{
    uint8_t             Sec	   : 6;  // 秒
    uint8_t             Min    : 6;  // 分
    uint8_t             Hour   : 5;  // 时
    uint8_t             Date   : 5;  // 日
    uint8_t             Month  : 4;  // 月
};

union FSH_EVE_TIME {
    uint8_t all[4];
    struct Eve_Time bit;
};

struct FSH_EVE_BITS {
    uint8_t u8BitRes:5;
	
    uint8_t u8BitEveState:1;
    uint8_t u8BitEveGun:2;
};
union FSH_EVE_INFO {
    uint8_t all;
    struct FSH_EVE_BITS bit;
};

typedef struct _EVE_RECORD{
    //交易记录信息，0x3F
    union FSH_EVE_TIME Eve_time;
	union FSH_EVE_INFO Eve_Info;
	
    uint8_t Eve_code[2];
    uint8_t Eve_data[8];		//故障数据
}EVE_RECORD;

typedef struct _F_EVE_RECORD{
    uint8_t u8CtrlWord[4];
    EVE_RECORD strErrRcd;
}F_EVE_RECORD;

//!!!!!所有数据都使用u8定义内存对齐,版本迭代要保证参数不变
typedef struct __SPI_DATA_MAP__
{
	/*前4096参数不能丢不能乱*/
	uint8_t					u8CtrlWord[4];
	uint8_t					u8ValidFlag;						//
	uint8_t					u8Res1[251];						//预留
	//256
	F_UPDATA_PARA			strFupDataPara;						//
    uint8_t 				u8Res2[128];
	//512
	F_SYSTEM_PARAM			strSystemParam;						//系统参数
    uint8_t 				u8Res3[256];
	//1024
	F_CHARGE_PARAM			strChargeParam;						//充电参数
    uint8_t 				u8Res4[256];
	//1536	
	F_PLAT_PARAM			strPlatParam;						//平台参数
    uint8_t 				u8Res5[256];
	//2048	
	RATE_MODEL_PARA			strRateModelPara;					//计费模型
    uint8_t 				u8Res6[108];
	//2560
	QR_PARA					strQRPara;							//二维码参数
	//3076
	uint8_t					u8Res7[1020];						//块1预留,使用中每个块擦的频率不一样
	//===========================================//
	//4096
	uint8_t					u8SecRes[4096];
	//===========================================//
	/*记录类数据,*/
	SAMPLE_RECORD			strSampleRecord;					//简易充电记录
	CHARGE_RECORD			strChargeRecord[CHG_RCD_MAX_NUM];	//充电记录
	
	SAMPLE_EVERECORD		strSampleEveRecord;					//简易故障记录
	F_EVE_RECORD			strEveRecord[CHG_RCD_MAX_NUM];		//故障记录
	
}STO_DATA_MAP;

#define STODM_ctrlWord			FPOS(STO_DATA_MAP, u8CtrlWord)		
#define STODM_UPDATA_PARA		FPOS(STO_DATA_MAP, strFupDataPara)
#define STODM_SYSTEMPARA		FPOS(STO_DATA_MAP, strSystemParam)			//系统参数
#define	STODM_CHARGE_PARAM		FPOS(STO_DATA_MAP, strChargeParam)			//充电参数
#define	STODM_RATE_PARAM		FPOS(STO_DATA_MAP, strRateModelPara)		//计费参数

#define	STODM_SEC_BLOCK			FPOS(STO_DATA_MAP, u8SecRes)				//

#define	STODM_SAMPLE_RECORD		FPOS(STO_DATA_MAP, strSampleRecord)			//充电记录
#define STODM_CHARGE_RECORD		FPOS(STO_DATA_MAP, strChargeRecord)			//充电记录
#define	STODM_SAMPLE_EVERECORD	FPOS(STO_DATA_MAP, strSampleEveRecord)			//充电记录
#define STODM_EVE_RECORD		FPOS(STO_DATA_MAP, strEveRecord)			//充电记录

#define STODM_END				(sizeof(STO_DATA_MAP))

#define STODM_FIRST_SEC_ADDR	(STODM_SAMPLE_RECORD)
//#define STODM_SECOND_SEC_ADDR	(STODM_SAMPLE_RECORD)

#define STO_USER_BACKUP_DATA_ADDR ((uint32_t)0x00100000)//升级备份区7M-7.5M 512k
#define STO_USER_UPDATA_DATA_ADDR ((uint32_t)0x00180000)//升级区7.5M-8M 512k

/*****************************************************************************
 * 
*****************************************************************************/

//extern CHARGE_PARAM g_ChargeParam;

//extern SYSTEM_PARAM g_SystemParam;

extern UP_PLAT_PARAM g_PlatParam;

//extern UPDATA_PARA g_UpdataParam;

/*****************************************************************************
 * 整片flash基本信息
*****************************************************************************/
#define FLASH_SIZE   	0x200000   	//flash大小
#define SECTOR_SIZE   	4096   		//flash sector大小

/*****************************************************************************
*
*****************************************************************************/
uint8_t STO_OTA_Write(uint32_t u32Dest, void *pSrc, uint32_t size);

uint8_t STO_OTA_Read(uint32_t u32Dest, void *pSrc, uint32_t size);

//写充电记录
uint8_t STO_W_ChgRcd(void *pRcdData, uint32_t RcdLen, uint32_t index);
//读充电记录
uint8_t STO_R_ChgRcd(void *pRcdData, uint32_t RcdLen, uint32_t index);
//写简易充电记录
uint8_t STO_W_SampleChgRcd(S_CHARGE_RECORD *pSRecord);
//读简易充电记录
uint8_t STO_R_SampleChgRcd(S_CHARGE_RECORD *pSRecord);
//存故障记录缓存，非一条故障
uint8_t STO_W_EveRcdPack(F_EVE_RECORD *pFRecord, uint32_t index, uint16_t cnt);
//读故障记录
uint8_t STO_R_EveRcd(EVE_RECORD *pRecord, uint32_t index);
//写简易故障记录
uint8_t STO_W_SampleEveRcd(S_EVE_RECORD *pSRecord);
//读简易故障记录
uint8_t STO_R_SampleEveRcd(S_EVE_RECORD *pSRecord);
//写计费模型
uint8_t STO_W_RateModel(uint8_t u8Port, void *pRate, uint16_t Ratelen);
//读计费模型
uint8_t STO_R_RateModel(uint8_t u8Port, void *pRate, uint16_t Ratelen);
//写系统参数
uint8_t STO_W_SystemParam(SYSTEM_PARAM *pSysPara);
//读系统参数
uint8_t STO_R_SystemParam(SYSTEM_PARAM *pSysPara);
//写充电参数
uint8_t STO_W_ChargeParam(CHARGE_PARAM *pChgPara);
//读充电参数
uint8_t STO_R_ChargeParam(CHARGE_PARAM *pChgPara);
//写二维码参数
uint8_t STO_W_QRPara(uint8_t u8Port, void *pQR, uint16_t len);
//读二维码参数
uint8_t STO_R_QRPara(uint8_t u8Port, void *pQR, uint16_t len);
//写升级参数
uint8_t STO_W_Updata(UPDATA_PARA *pUpdataPara);
//读升级参数
uint8_t STO_R_Updata(UPDATA_PARA *pUpdataPara);


uint8_t load_EEOP_Param(uint8_t *data, uint16_t u16Len);
uint8_t Set_EEOP_Param(uint8_t *data, uint16_t u16Len);
uint8_t Clear_EEOP_Param(void);

//写计费模型
// uint8_t DataFlashWrite_RateModel(RATE_MODEL_T *pRate);
//读计费模型
// uint8_t DataFlashRead_RateModel(RATE_MODEL_T *pRate);

//获取平台类型
uint8_t get_ChgParam_plat_type(void);
//获取卡类型
uint8_t get_ChgParam_Card_type(void);

//==============================================================
//存储初始化
uint8_t STO_Init(void);

#pragma pack()

#ifdef __cplusplus
extern "C"
{
#endif

//获取平台存储数据指针
PlatCfgInfo *fgv_GetPlatCfgInfo(void);

extern void load_AllParam(void);       //开机读取加载所有有效参数
extern uint8_t Set_updataParam(stu_UpdataCfg_t *pst_info);     //升级参数设置
extern uint8_t Set_platParam(PlatCfgInfo *pst_info);       //平台参数设置

void storage_qrCodeInfoStr(char *input);
void storage_PlatQRCodeInfoStr(uint8_t u8Port, char *input);
uint8_t load_qrCodeInfo(stu_QrCodeInfo_t *Qr_info);
uint8_t Set_qrCodeInfo(stu_QrCodeInfo_t *Qr_info);
void Update_qrCodeInfo();   //更新完桩号可以调用，仅仅有默认的二维码连接有效
uint8_t Check_qrCodeInfoValid(void);

void Save_rate_model(void *pRate, int len);
uint8_t Read_rate_model(void *pRate, int len);
void load_rate_model(void *pRateM);

void STO_EraseBillFlash(void);


uint8_t W25QXX_Erase_Sector(uint32_t u32Dest);
uint8_t W25QXX_Read(void *pSrc, uint32_t u32Dest, uint32_t size);
uint8_t W25QXX_Write_safety(void *pSrc, uint32_t u32Dest, uint32_t size);

#ifdef __cplusplus
}
#endif


//======================================================


#endif



