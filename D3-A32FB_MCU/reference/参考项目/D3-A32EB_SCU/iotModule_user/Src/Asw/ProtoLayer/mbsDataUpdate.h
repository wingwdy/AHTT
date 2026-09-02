#ifndef __MBSDATAUPDATE_H__
#define __MBSDATAUPDATE_H__

#include "stdbool.h"
#include "globals.h"
#include <stdio.h>
#include <stdint.h>

//typedef enum {FALSE = 0, TRUE = !FALSE} Bool;
#define UPDATE_PAGESIZE           512     //升级页长度

#define SLAVE_PILE_ADR       		0

#define MODBUS_READ_REG       		0X03
#define MODBUS_WRITE_SINGLE_REG     0X06
#define MODBUS_WRITE_MULTIPLE_REGS  0X10
#define MODBUS_UPGRADATE_REGS  		0X15


//modbus地址分配
#define PILE_INF_OFFSET_ADR      	0x0
#define PILE_NET_INF_OFFSET_ADR     0x10
#define PILE_CONFIG_OFFSET_ADR   	0x30
#define PILE_PLAT_CFG_OFFSET_ADR   	0x50
#define PILE_RT_INFO_OFFSET_ADR   	0xA0
#define PILE_NET_RT_OFFSET_ADR   	0xB0
#define PILE_CTRL_OFFSET_ADR   	    0xC0

#define PILE_WRITE_OFFSET_ADR   	0xC0

#define GUN_QRCODE_OFFSET_ADR      	0x0
#define GUN_RT_INF_OFFSET_ADR      	0x80
#define GUN_CTRL_OFFSET_ADR   		0x90
#define GUN_AUTHN_OFFSET_ADR      	0x92
#define GUN_CHARGE_OFFSET_ADR   	0xB0
#define GUN_ORDER_OFFSET_ADR   		0xD0

typedef enum {
	E_NetStepOff = 0,
	E_NetStepSimFind,
	E_NetStepSimAuth,
	E_NetStepSimIccid,
	E_NetStepSimSignal,
	E_NetStepPS,
	E_NetStepFindOptor,
	E_NetStepFindMode,
	E_NetStepCfgPS,
	E_NetStepActivePS,
	E_NetStepPSStatu,
	E_NetStepTime,
	E_NetStepConnect,

	E_NetStepSrvSucs = 31,		//sim卡连接正常
	E_NetStepPlatSuc = 32,		//平台连接正常
	E_NetStepPlatHeart = 33,	//心跳正常

	E_NetStepReconnet = 99,		//重连
} E_NetStep;


typedef enum {
	E_AuthFailUnknow = 0,		//未知错误
	E_AuthFailInexist,			//账户不存在
	E_AuthFailArrearage,		//账户余额不足
	E_AuthFailFrezed,			//账户被冻结
	E_AuthFailBlackList,		//账户为黑名单用户
	E_AuthFailUnAccount,		//账户存在未结账记录
	E_AuthFailNotCard,			//此设备不支持刷卡
	E_AuthFailOffline,			//平台失连
} E_AuthFailReason;


#pragma pack(1)

typedef struct {
	uint8_t soft_ver[4];			//桩软件版本
	uint8_t protocol_ver[4];			//协议版本
	uint8_t hw_version[4];			//协议版本
	uint8_t dev_type;				//设备类型，0直流，1交流，2电瓶车
    uint8_t	IotMod;
	uint16_t dev_pow;				//设备总功率
}GN_PLATMOD_PILE_INF;

typedef struct {
	uint8_t soft_ver[4];			//桩软件版本
	uint8_t protocol_ver[4];		//协议版本
	uint8_t hw_version[4];			//协议版本
	uint8_t netType;				//网络类型
	uint8_t oprtType;				//运营商类型
    uint8_t	SIMCard[20];			//SIM卡号
    uint8_t	IMEICard[20];			//IMEI卡号
}GN_PLATMOD_NET_INF;

/*桩的参数设置变量*/
typedef struct
{
  uint8_t   cgfEStop          :1; //急停使能      0,禁止  1,允许
  uint8_t   cgfForbid         :1; //禁止充电      0,禁止  1,允许
  uint8_t   cgfPlugChrg       :1; //即插即充模式  0,禁止  1,允许
  uint8_t   cgfModeCmp        :1; //国标模式      0,企标  1,国标
  uint8_t   cgfNetChrg        :1; //运营模式      0,禁止  1,允许
  uint8_t   cgfOfflineChrg    :1; //离线充电      0,禁止  1,允许
  uint8_t   cgfOnWorkPile     :1; //是否在工桩上   0,否  1,是
  uint8_t   cgfNeedFacMode    :1; //需要进入工厂模式   0,否  1,是

  uint8_t   rsvd_8b           :8;

} _16BIT_Stru_FunConfigInfo;

typedef union 
{
  uint16_t                      bits;
  _16BIT_Stru_FunConfigInfo     bit;
} U_FunConfigInfo;

typedef struct 
{
  uint8_t gunNum;				    //枪数量, 配置重启生效
  uint8_t	res1;
  U_FunConfigInfo fct_cfg;        //功能配置
  uint16_t pow_limit;							//功率限制大小，0为不限制
  uint16_t max_voltage;						//电压上限
  uint16_t min_voltage;						//电压下限
  uint16_t max_current;						//电流上限
  uint16_t target_current;				//设置目标电流
  uint8_t max_gun_temp;						//枪温上限
  uint8_t min_gun_temp;						//枪温下限
  uint8_t max_pile_temp;					//环温上限
  uint8_t min_pile_temp;					//环温下限
  uint16_t warning_vol;						//过压阀值
  uint16_t warning_cur;						//过流阀值
} GN_PILE_CFG_INF;


typedef struct {
	uint8_t devNumber[16];
	uint8_t platNumber[32];
	uint8_t ipOrDomain;
	uint8_t ip[4];
	uint8_t domain[64];
	uint16_t port;
	uint8_t cardType;
	uint8_t platType;
}GN_PLATMOD_CFG_INF;

typedef struct {
    uint8_t	pile_ack;
    uint8_t	revs1;
	uint8_t pile_sta;
	uint8_t key;
	uint16_t inVoltageA;
	uint16_t inVoltageB;
	uint16_t inVoltageC;
	int16_t pileTemp;
	uint8_t stopKey;
	uint8_t revs;
} GN_PILE_RT_INF;

typedef struct {
	/* 网络模块状态(0联网过程中, 1主连接正常，7辅连接正常(配置)，8ftp获取文件中，9升级中(包括文件传输中)) */
	uint8_t netCfgAck;
	uint8_t netCfgAckRevs;
	uint8_t netSta;
	uint8_t updateObj;
	uint8_t netStep;
	uint8_t simSta;
	uint32_t signalVal;
	uint8_t signalLevel;
	uint8_t resv;
} GN_PILE_NETRT_INF;

typedef struct {
	uint8_t pileTime[8];
	uint8_t ctrlCmd;
	uint8_t ctrlRvs;
} GN_PILE_CTRL_INF;


typedef struct {
	uint8_t disPlatNumber[32];
	uint8_t qrCode[200];
} GUN_PLTMD_QRCODE_INF;


//充电状态结构体类型
typedef struct
{
	uint8_t connect_1b      :1;//充电枪连接:1,有效
	uint8_t charging_1b     :1;//充电状态:1,有效
	uint8_t relay_out     	:1;//继电器状态，1闭合
	uint8_t pwm_en_1b       :1;//PWM使能:1,有效
	uint8_t low_rate_1b     :1;//降额中:1,有效
	uint8_t revd     		:3;//预留
} _16BIT_PlatChargeSta;
//充电状态联合体类型
typedef union
{ 
  uint16_t         bits;
  _16BIT_PlatChargeSta bit;
} U_PlatChargeSta;

/* 充电枪故障警告信息结构体类型 */
typedef struct
{
	uint8_t alrmGround_1b           :1;//接地警告:1,有效
	uint8_t alrmPhaseLoss_1b        :1;//缺相警告:1,有效
	uint8_t alrmTemp_1b             :1;//枪过温警告:1,有效
	uint8_t alrmRevd1     		    :5;//预留
    
	uint8_t alrmRevd2     		      :8;//预留
} Stru_16BIT_GunAlarmList;

typedef struct
{
	uint8_t faultLeakage_1b         :1;//漏电故障:1,有效
	uint8_t faultEleMeasure_1b      :1;//计量异常:1,有效
	uint8_t faultEStop_1b           :1;//急停故障:1,有效
	uint8_t faultPileTemp_1b        :1;//环温过温故障:1,有效
	uint8_t faultPlugTemp_1b        :1;//枪头过温故障:1,有效
	uint8_t faultRelay_1b           :2;//继电器故障:1,黏连，2输出异常
	uint8_t faultCP_1b              :1;//CP异常:1,有效

	uint8_t faultCurrent_1b         :1;//过流:1,有效
	uint8_t faultVoltage_1b         :2;//过压欠压:1,过压，2欠压
	uint8_t faultCPGround_1b        :1;//cp接地故障:1,有效
	uint8_t faultGround_1b        	:1;//接地故障:1,有效
	uint8_t faultDiode_1b        	:1;//二极管不存在故障：1不存在
	uint8_t faultLFRevs_1b        	:1;//火零反接：1反接
	uint8_t faultShortC_1b        	:1;//短路故障：1存在
    
	uint8_t faultRevd2     		    :8;//预留

	uint8_t faultRevd3     		    :8;//预留

} Stru_32BIT_GunFaultList;


typedef union
{ 
  uint16_t                bits;
  Stru_16BIT_GunAlarmList bit;
} U_GunAlarm;

typedef union
{ 
  uint32_t                bits;
  Stru_32BIT_GunFaultList bit;
} U_GunFault;


typedef struct {
	uint8_t gunAck;		//
	uint8_t revs1;		//
	uint8_t gun_sta;		//枪状态(0正常，1故障，2警告)
	uint8_t revs2;
	uint8_t gun_ChrgSta;		//枪充电状态(0待机，1等待启动, 2启动中，3充电中，4暂停充电，5停止中，6停止完成)
	uint8_t gun_insert; 	//枪是否归位(bit0-bit3)、是否插抢(bit4-bit7)
	uint16_t cpValue;
	U_PlatChargeSta u_platChrgsta;			//bit15插抢，bit14充电，bit13继电器，bit12pwm输出，bit11降额，
	int8_t plugTemp;
	uint8_t revs;
	U_GunAlarm gunWarn;
	U_GunFault hardfault;
	uint16_t pileStopReason;
}GUN_PLTMD_RT_INF;

typedef struct {
	uint8_t cmd;	//启停命令
	uint8_t stopReason;	//停止原因
	uint16_t setPower;			    //功率控制, 0.01kW
}GUN_PLTMD_CARD_INF;

typedef struct {
	uint8_t start_time[8];	
	uint8_t stop_time[8];	
	uint32_t start_ele;	
	uint32_t stop_ele;	
	uint32_t charge_ele;	
	uint16_t out_vol;
	uint16_t out_cur;
	uint32_t charge_time;
	uint16_t pwm;
}GUN_PLTMD_CHARGE_INF;

typedef struct {
	uint32_t amount;	
	uint32_t lossEle;
}GUN_PLTMD_ORDER_INF;

typedef struct {
	GUN_PLTMD_QRCODE_INF disQrcode;
	GUN_PLTMD_RT_INF gunRtInfo;
	GUN_PLTMD_CARD_INF cardCmd;
	GUN_PLTMD_CHARGE_INF chrgingInfo;	//充电桩充电信息
	GUN_PLTMD_ORDER_INF orderInfo;		//
} GN_PLATMOD_GUN;



typedef struct {
	GN_PLATMOD_PILE_INF pileInf;			//桩信息
	GN_PILE_CFG_INF pileCfgInfo;            //桩配置信息
	GN_PILE_RT_INF  pile_rt_inf;			//充电桩实时信息
	GN_PILE_NETRT_INF pileNetRtInf;         //网络单元实时信息
	GN_PILE_CTRL_INF pileCtrlInf;           //桩控制指令
	GN_PLATMOD_GUN gun[GUN_NUM_MAX];		//枪信息
}GN_PLATMOD;

extern GN_PLATMOD  sg_platmod;


#pragma pack()






enum {
	eMbsScanOff,
	eMbsScanOn
};
enum {
	eMbsStanby,
	eMbsSend,
	eMbsWaitRecv
};

typedef enum
{
	//读取桩信息
	E_ReadCmd_PileInfo,				//读取桩基本信息
	E_ReadCmd_PileNetInfo,			//读取网络模块基本信息
	E_ReadCmd_PileCfgAck,			//读取桩配置信息ack
	E_ReadCmd_PileCfgInfo,			//读取桩配置信息
	E_ReadCmd_PilePlatCfgInfo,		//读取平台配置信息
	E_ReadCmd_PileRtInfo,			//读取桩实时信息
	E_ReadCmd_PileNetRtInfo,		//读取网络实时信息
	//写入桩信息
	E_WriteCmd_PileCfgInfo,			//更改写入桩配置信息
	E_WriteCmd_PileCfgAck,			//写入桩配置信息ack
	E_WriteCmd_TimeSync,			//同步时间
	E_WriteCmd_CtrlDev,				//控制桩重启、恢复出厂设置

	E_PileInfoMax,			//同步时间


	//读取枪信息
	E_ReadCmd_GunQrCodeInfo,		//读取二维码信息
	E_ReadCmd_GunRtInfo,			//读取枪实时状态信息
	E_ReadCmd_GunAuthnInfo,			//读取刷卡鉴权状态
	E_ReadCmd_GunChrgingInfo,		//读取枪实时充电信息，充电中有效
	E_ReadCmd_GunOrderInfo,			//读取枪充电完成，充电完成有效
	//写入桩信息
	E_WriteCmd_GunCtrlCmd,			//启停命令
	E_WriteCmd_GunCrtInfo,		    //电流控制

	E_InteractionMaxCmd,
} E_MbsCmd;



typedef struct {
	uint8_t					SlaveAddr;		//从机地址
	E_MbsCmd				CommCmd;		//读取写入命令
	uint8_t					Sta;			//主机 0空闲，1发送，2等待接收
	uint8_t					GunNum;			//枪号
	uint32_t				SendTick;		//发送时间
}Stru_MbsMasterSta;


typedef bool (*ModbusExecuteFunc)(uint8_t ch);
typedef bool (*ModbusHandleFunc)(uint8_t ch,uint8_t *pData);
typedef bool (*ModbusRegSendFunc)(uint8_t ch,uint8_t *pData);

typedef struct {
	uint8_t					excuteType;		//1初始化需要开启
	uint32_t				TimeOutTick;	//轮询间隔
	E_MbsCmd				CommCmd;		//命令
	uint8_t					funCode;
	uint16_t				beginRegAddr_u16;
	uint8_t					num_8u;			//寄存器个数
	ModbusRegSendFunc		pileRegSendFunc;	
	ModbusRegSendFunc		pileRegHandleFunc;	
	ModbusExecuteFunc		pileExecuteFunc;	
}Stru_MbsInteraction;

typedef enum
{
	E_Disable,
	E_NeedInit,
	E_Circle,
} E_ItrcMode;



typedef enum 
{ 
	E_PLAT_MOD_DEAL_NONE,
	E_PLAT_MOD_DEAL_FAULT,
	E_PLAT_MOD_DEAL_OK,
	E_PLAT_MOD_DEAL_END,
}E_PLAT_MOD_DEAL;
	
typedef enum 
{ 
	E_PLATMOD_CMD_NONE,
	E_PLATMOD_CMD_START = 0x1,	
	E_PLATMOD_CMD_STOP  = 0x2,
	E_PLATMOD_CMD_NOSTART  = 0x3,
}E_PLATMOD_CMD;

typedef enum 
{ 
	E_DEV_CTRL_CMD_NONE,
	E_DEV_CTRL_CMD_REBOOT  = 0x5,	//控制设备重启
	E_DEV_CTRL_CMD_FACRST = 0x6,	//恢复出厂设置
}E_DEV_CTRL_CMD;

typedef enum 
{ 
	E_DEV_STA_NONE,			//正常
	E_DEV_STA_FAULT  = 0x1,	//故障
	E_DEV_STA_WARN = 0x2,	//警告
	E_DEV_STA_CHECK = 0x3,	//自检中
	E_DEV_STA_UPD = 0x4,	//升级中
}E_DEV_STA;

typedef enum 
{ 
	E_PLAT_NET_ING,			//联网中
	E_PLAT_NET_MAIN,		//主连接正常
	E_PLAT_NET_AUXILIARY = 7,	//辅连接正常
	E_PLAT_NET_FTPGET = 8,		//ftp获取文件中
	E_PLAT_NET_UPDATING = 9,	//升级中，包括文件传输
}E_PLAT_NET_STA;

uint8_t fgv_GetPileCfgGunNum(void);

uint8_t fgv_GetPileCfgOffLinChrg(void);
uint8_t fgv_GetPileCfgNationalStandard(void);

void MbsThreadScan(void);


#endif
