#ifndef __PROTOCOL_CODEC_HaiNCT_H__
#define __PROTOCOL_CODEC_HaiNCT_H__

#include "protocol_ctrl.h"
#include "tcp_gn.h"
#include "iot_ANPEI_Protocol_Code.h"

#define HaiNCTFRAME_HEAD 0x68 //

#define BillMaster_A      0
#define BillSlave_B       1

//安培交易标识（启动方式）
enum {
    eUP_Start_Style_App_HaiNCT  		= 0x00,   	//app 启动
    eUP_Start_Style_CardOnline_HaiNCT  	= 0x01,		//卡启动
};

//安培启动充电失败原因
enum {
    eUP_Start_Fail_NULL_HaiNCT 		= 0,     	//无
    eUP_Start_Fail_DevNumErr_HaiNCT 	= 0x01,     //设备编号不匹配
    eUP_Start_Fail_Working_HaiNCT 		= 0x02,    	//枪已在充电
    eUP_Start_Fail_DevErr_HaiNCT 		= 0x03, 	//设备故障
    eUP_Start_Fail_Offline_HaiNCT 		= 0x04,		//设备离线
    eUP_Start_Fail_NoConn_HaiNCT		= 0x05,		//未插枪
    eUP_Start_Fail_Reconnect_HaiNCT 	= 0x06,		//超时不可启动，需要重新插拔枪
};

//停止充电失败原因
enum {
    eUP_Stop_Fail_NULL_HaiNCT 			= 0,    	//无
    eUP_Stop_Fail_DevNumErr_HaiNCT	= 0x01,   	//设备编号不匹配
    eUP_Stop_Fail_NoWorking_HaiNCT 	= 0x02,    	//枪未处于充电状态
    eUP_Stop_Fail_Other_HaiNCT 		= 0x03, 	//其他
};

/******************状态*******************************************************************/
// 只能用于实时报文中上行上报,不能用于流程判断 
enum
{
	eUP_Gun_State_Warning_HaiNCT = 1,		// 告警
	eUP_Gun_State_Standby_HaiNCT = 2,		// 待机
	eUP_Gun_State_Work_HaiNCT = 3,		// 工作
	eUP_Gun_State_OffLine_HaiNCT = 4,	// 离线
	eUP_Gun_State_Finish_HaiNCT = 5,		// 完成
};

enum{
    E_HNCT_GunState_Auto,
    E_HNCT_GunState_Ele,
    E_HNCT_GunState_Time,
    E_HNCT_GunState_Money,
};

//停止原因
enum{
    E_HNCT_StopReason_Card = 1,
    E_HNCT_StopReason_App = 2,
    E_HNCT_StopReason_Full,
    E_HNCT_StopReason_MnyOver,
    E_HNCT_StopReason_Haldfault,
    E_HNCT_StopReason_Estop = 7,
};
/*************************上报平台一些反馈公共使用****************************/
typedef struct
{
    U8 upGunState;      //上报平台充电桩状态，1告警，2待机，3工作，4离线，5完成

    U8 upResult;         //上报状态结果，可以公用
    U8 upReason;         //上报状态结果，可以公用
    
    U8 startCmdResult;   //1表示已经启动，需要等待启动结果，2表示启动失败，需要回复启动失败结果

    U8 rateErro;         //计费模型数据校验异常
    U8 A1code;           //实时检测数据上报传送原因，变化为3，周期为1
    
    U8 TradeFlag;       //海宁城投启动类型
    //关于功率
    U16 setPower;     //单位0.01kw

    U8 CrtBliiID[8];       //当前计费模型ID
    U8 UserID[8];       //充电用户ID
    U8 phyCard[8];      //物理卡号

    U32 acount;       //账户余额

} HaiNCT_UpPlatInfo;


typedef struct
{
    uint8_t powerCtrlEnable; 
    uint8_t powerPct[2];          //设置功率，百分比，nnn.n  单位为%
    uint8_t PowerEndTime[7];
} HaiNCT_FlashPlatInfo;

/*************************费率结构体*****费率变量******费率相关函数****************************/

// B47费率模型
typedef struct
{
	U8 rate_start[2];   //0分，1时
	U8 Serial_rate; // 1-4，尖峰平谷
	U8 UnitEleFee[4][4];    //尖峰平谷电价，小数点后5位
	U8 Ser_fee[4];      //服务费，小数点后5位
	U8 Perch_fee[4];    //占位费电价，小数点后5位
	U8 Order_fee[4];    //预约费单价，小数点后5位
} HaiNCT_Free_data;

// 约193字节
typedef struct
{   
	U8 billing_model[8]; // 计费模型编号
	U8 start_time[7] ;   // 生效时间
	U8 end_time[7];      // 失效时间
	U8 workstate[2]; // 1-有效 2-无效
	U8 measureType[2]; // 1充电量
	U8 time_allnum;	 // 时段数量(<=12)
	HaiNCT_Free_data B47modeldata[12];
} FeeModelA8; // 费率模型

typedef struct
{   
	U8 RecentUpdates_Nomber;   //记录最近一次更新的B47费率的是FeemodelB47save_data[]的下标 是(0：B47_A)还是(1:B47_B )
	FeeModelA8 FeemodelA8save_data[2];
} HnctFeeModel_Save_truct; // 存储的费率模型结构体


typedef struct
{
    U8 startEle[4]; //起始值
    U8 stopEle[4];  //终止值
} RecordEleValue;

typedef struct
{
    U8 unitPrice[4]; //单价，小数点后5位
    U8 perEle[4];  //时段电量，小数点后2位
    U8 perMny[4];  //时段金额，小数点后2位，四舍五入
} RecordInfoValue;

// 最多230个字节
typedef struct
{
	U8 transaction_log_num[16];
    U8 PayCard[8];
    U8 PhyCard[8];
    U8 TimePerFlag;
	U8 chrg_start_time[7];
	U8 chrg_stop_time[7];

    RecordEleValue EleValue[4];
    U8 costType[2];                 //计费类型，0001-充电量 0002-放电量
	U8 EnergyEle[2][4];             // 总起止值--0起  1止
    RecordInfoValue infoValue[4];
	U8 TatalEle[4];                 //总电量,精确到小数点后两位

	U8 BnsType[2];              //业务类型
	U8 WalletBalance[4];        //电子钱包余额
	U8 consumedUnit[4];         //消费单价
	U8 consumedMny[4];          //消费金额
	U8 vinID[17];               //vin码
	U8 dealType;               //交易标识， 0充电卡成功，1充电卡失败
    //空12个字节
	U8 zero0[12];               //vin码
	U8 dealMny[4];               //交易金额
	U8 dealTime[7];               //交易时间
    //空10个字节
	U8 zero1[14];               //vin码
	U8 chargeSerUnit[4];                 //充电服务费单价
	U8 chargeSerMny[4];                  //充电服务费金额，小数点后2位，四舍五入
	U8 chargeOrderUnit[4];               //预约费单价
	U8 chargeOrderMny[4];                //预约费金额，小数点后2位，四舍五入
	U8 chargePerchUnit[4];               //占位费单价
	U8 chargePerchMny[4];                //占位费金额，小数点后2位，四舍五入
	U8 chargeAllEleMny[4];               //总电费，小数点后2位，四舍五入

	U8 chargeBillID[8];                  //计费模型ID
	U8 chargeAllMny[4];                  //总费用，小数点后2位，四舍五入
	U8 stopReason[2];                    //停止原因
	U8 userID[8];                        //用户身份ID
} RecordA3;


/**************************************************/

/************************************************************************************/
// 协议里B1-B61 HaiNCT
enum
{

	HaiNCT_S_RealData = 0x01, // B1 充电过程实时监测数据   Type 134  cot 1(周期)/3(突变)  recordKind 无
    
    HaiNCT_S_AskerRateModel = 0x02,	  // A7. 请求下发计费模型数据Type 130  cot 6  recordKind 4
	HaiNCT_R_RateModel = 0x03,        // A8. 下发计费模型下行数据Type 133  cot 6  recordKind 6
	HaiNCT_S_RateModelAnswer = 0x04,  // A9. 下发计费模型结果数据Type 133  cot 6  recordKind 5

    HaiNCT_S_ChargingData = 0x07,	    // A20. 充电过程中上传数据Type 130  cot 6  recordKind 2
	HaiNCT_R_ChargingData = 0x08,        // A21. 充电过程中上传确认数据Type 133  cot 6  recordKind 3


	HaiNCT_R_Start_Chg = 0x11,	// B4. 充电启停控制命令下发下行数据（扫码充电）Type 133  cot 6  recordKind 21
	HaiNCT_S_Start_ChgAsk = 0x12, // B5. 充电启停控制命令结果确认（扫码充电）Type 133  cot 7  recordKind 21

	HaiNCT_R_Stop_Chg = 0x13,	// B4. 充电启停控制命令下发下行数据（扫码充电）Type 133  cot 6  recordKind 21
	HaiNCT_S_Stop_ChgAsk = 0x14, // B5. 充电启停控制命令结果确认（扫码充电）Type 133  cot 7  recordKind 21

	HaiNCT_S_Cardinf = 0x15,	    // A2. 刷卡鉴权上行（在线刷卡充电）Type 130  cot 6  recordKind 1
	HaiNCT_R_CardinfAck = 0x16,     // A10. 刷卡鉴权下行（在线刷卡充电）Type 133  cot 7  recordKind 2
	HaiNCT_S_CardStart_Chg = 0x17,	 // A25. 启动通知上报（在线刷卡充电/在线vin码充电）Type 130  cot 6  recordKind 14
	HaiNCT_R_CardStart_ChgAck = 0x18, // A26. 启动通知下行（在线刷卡充电/在线vin码充电）Type 133  cot 7  recordKind 12


    HaiNCT_S_OrderUp = 0x30,	    // A3. 充电记录上传数据Type 130  cot 6  recordKind 2
	HaiNCT_R_OrderUp = 0x31,        // A4. 充电记录确认数据Type 133  cot 6  recordKind 3

    
	HaiNCT_R_PowerCon = 0x51,	   // B33. 充电功率控制下行（扩展）Type 133  cot 7  recordKind 58
	HaiNCT_S_PowerConASK = 0x52,	   // B34. 充电功率控制上行（扩展）Type 130  cot 6  recordKind 28
    
    HaiNCT_R_ConfigPara = 0x53,	    // A22. 参数设置Type 133  cot 6  recordKind 17
	HaiNCT_S_ConfigPara = 0x54,     // A23. 设置结果Type 130  cot 7  recordKind 18
    
	HaiNCT_R_RemoteUpgrade = 0x5A,	 // B23. 远程升级启动（扩展）Type 133  cot 6  recordKind 15
	HaiNCT_S_RemoteUpgradeAck = 0x5B, // B24. 远程升级启动命令接收结果（扩展）Type 130  cot 7  recordKind 14
    
    HaiNCT_R_Qrcode = 0x5C,	    // A39. 二维码下发Type 133  cot 6  recordKind 24
	HaiNCT_S_Qrcode = 0x5D,     // A40. 二维码应答Type 130  cot 7  recordKind 24
    

	HaiNCT_S_Identification = 0xF1, // 登录认证
	HaiNCT_R_Identification = 0xF2, // 认证应答
	HaiNCT_S_Heart = 0xF3,		   // 心跳包
	HaiNCT_R_Heart = 0xF4,		   // 心跳应答
	HaiNCT_S_START_U = 0xF5,			   // U帧启动应答
	HaiNCT_R_START_U = 0xF6,			   // U帧启动
	HaiNCT_S_STOP_U = 0xF7,			   // U帧停止应答
	HaiNCT_R_STOP_U = 0xF8,			   // U帧停止
	HaiNCT_S_clocksyn = 0xF9,	   // 时钟同步  Type 103  cot 7  recordKind 无
	HaiNCT_R_clocksyn = 0xFA,	   // 时钟同步应答Type 103  cot 6 recordKind 无

};



// 认证
typedef struct
{
	uint8_t billModelID[8]; // 计费模型ID
	uint8_t device_number[DEV_NUM_LEN + 1]; // 充电桩编号 8个字节
} HaiNCT_Recv_Identification;

// 结构体
typedef struct
{
	uint8_t val;
} HaiNCT_Recv_U;

typedef struct
{

	uint8_t val;
} HaiNCT_Recv_Heart;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Rate_num[2]; //
	uint8_t result;		 // 0x00 桩计费模型与平台一致 0x01 桩计费模型与平台不一致
} HaiNCT_Recv_Rate_Proving;

typedef struct
{
	uint8_t head;
	uint8_t len[2];
	uint8_t control[4];
	uint8_t TypeIDE;	   // 类型标识
	uint8_t Vsq;		   // 可变结构限定词
	uint8_t Cot[2];		   // 传送原因
	uint8_t AppSerAddr[2]; // 应用服务数据单元公共地址，默认0；
	uint8_t InfAddr[3];	   // 信息对象地址,最高位未枪号，0x000000表示1枪，0x100000，表示2枪
	uint8_t recordKind;	   // 记录类型

} HNCT_HEAD_T;


typedef struct
{
	cp56time2a cur_time;
} HaiNCT_Recv_TimeSyn;


typedef struct
{
	U8 device_number[DEV_NUM_LEN + 1];
	U8 Interface_mark; // 0：一桩一充； 1-一桩多充的1号枪 2...

	FeeModelA8 billing_modelA8;

} HaiNCT_Recv_Rate_ModelA8;


typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];

	uint8_t update_ip[4]; // 升级服务器ip,
	uint8_t update_com[2];
	uint8_t update_username[10];  // 用户名ascii
	uint8_t update_password[10];  // 密码ascii
	uint8_t update_file_path[50]; // 文件路径ascii
} HaiNCT_Recv_Update_ftp;

typedef struct
{
    uint8_t VV; //AA有效
	uint8_t update_ip[4]; // 升级服务器ip,
	uint8_t update_com;
	uint8_t update_username[10];  // 用户名ascii
	uint8_t update_password[10];  // 密码ascii
	uint8_t update_file_path[50]; // 文件路径ascii
    uint8_t update_file_name[50];      	//文件名
    uint8_t updatemodel;
    uint8_t updateSoftver[30];
    uint8_t updateHalver[20];
} HaiNCT_Recv_RemUp_Cmd;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Interface_mark;
	uint8_t billing_model[8]; // 计费模型编号

	uint8_t success_mark; // 0成功 1失败
} HaiNCT_Recv_Rate_switchB50;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Interface_mark;
	uint8_t acount[4];        //余额
	uint8_t userID[8];      // 用户ID
	uint8_t start_type;     // 00 自动； 01 按电量； 02 按时间； 03 按金额
	uint8_t start_para[3];  //充电参数（3节 BCD 码）电量：单位 kWh，精确到0.01；时间：单位 min，精确到 0.01；金额：单位 元，精确到 0.01
} HaiNCT_Recv_Start_Charge;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Interface_mark;
	uint8_t stopUserID[8];      //终止ID
} HaiNCT_Recv_Stop_Charge;


typedef struct
{
	uint8_t Interface_mark;
	uint8_t resultask; // 0成功
} HaiNCT_Recv_Online_ask;

typedef struct
{
	uint8_t resultask; // 0成功
					   // uint8_t transaction_log_num[16]; 	//交易流水号，
} HaiNCT_Recv_Offline_ask;

typedef struct
{
	uint8_t Deduction_result; // 1 扣款成功
	uint8_t Fail_reason[2];	  //

} HaiNCT_Recv_DeductionRecord;

typedef struct
{
    uint8_t Physics_card_number[8];
    uint8_t CarOnlyTag[32];
    uint8_t billing_model[8];
    uint8_t account_balance[4];
    uint8_t check_card;         //1成功，0失败
    uint8_t failcheckreason[2]; //失败原因
} HaiNCT_Recv_Card_inf;

typedef struct
{
	uint8_t Interface_mark;

	uint8_t result; //0成功 1失败
	uint8_t failreason[2];
	uint8_t transaction_log_num[16]; // 交易流水号，

} HaiNCT_Recv_Card_start;

typedef struct
{
	uint8_t Interface_mark;
	uint8_t accountNumber[8];
	uint8_t chargefee[2];
	uint8_t transaction_log_num[16]; // 交易流水号，
	uint8_t state;//0开始预约, 1取消预约,  2开始定时, 3结束定时,
	uint8_t starttime[7];
	uint8_t endtime[7];

} HaiNCT_Recv_fixtime_cmd;

typedef struct
{
    uint8_t SetPower[2];
    uint8_t PowerEndTime[7];

} HaiNCT_Recv_Powercontrol;

typedef struct
{
	uint16_t upTimeInterval;
	uint16_t upTimeMin;
} HaiNCT_Recv_ParamSet;

typedef struct
{
	uint8_t codeGun;  //0x00 主屏幕二维码，0x01 充电枪 1 的二维码，0x02 充电枪 2 的二维码  
} HaiNCT_Recv_qrcode;

typedef struct
{
	HaiNCT_Recv_Identification strRecvIdenf;			   // 初始化协议帧的回送
	// HaiNCT_Recv_U strRecvU;							   // U帧-回复
	// HaiNCT_Recv_Heart strRecvHeart;					   // 心跳-回复
	HaiNCT_Recv_TimeSyn strRecvTimeSyn;				   // 时钟
    HaiNCT_Recv_Rate_ModelA8 strRecvRateModelA8;        //A8模型费率

	HaiNCT_Recv_RemUp_Cmd strRecvRemUp_Cmd;			   // B23
	HaiNCT_Recv_Start_Charge strRecvStartEndCharge;  // B4
    HaiNCT_Recv_Stop_Charge strRecvStopCharge;
	HaiNCT_Recv_Online_ask strRecvOnlinetrans_ask;	   // B13 B54
	HaiNCT_Recv_DeductionRecord strRecvDeductionRecord; // 扣款记录 B14
	HaiNCT_Recv_Card_inf strRecvCard_inf;			   // B7
	HaiNCT_Recv_Card_start strRecvCard_start;		   // B11
	HaiNCT_Recv_Powercontrol strRecvPowercontrol;	   // B33
    
	HaiNCT_Recv_ParamSet strRecvParamSet;	    // A22 数据上报参数设置
    
	HaiNCT_Recv_qrcode strRecvQrcode;	        //A39 二维码

} HaiNCT_RECV_Data;


void HaiNCTUpProtocolDeal(void);

void Set_powerinit(uint8_t u8Port);


bool Hnct_Refresh_NowbillModel(uint8_t port);
void save_rateB47_model_HaiNCT(FeeModelA8 *pRateM,uint8_t uport,uint16_t length);

void Read_rateB47_model_HaiNCT(void);
uint16_t get_Rate_HaiNCT_Priod(uint8_t port);
void HaiNCT_packChgRecord(uint8_t u8Port, RecordA3 *pRecord);

void HaiNCTDealUpdate_Cmd(uint8_t u8Port);


uint8_t HaiNCTCurrentRateType(uint8_t u8Port);



uint8_t HaiNCT_GetRecvTopicAndIndex(uint8_t cmd, uint8_t *index);
uint8_t HaiNCT_GetSendTopicAndIndex(uint8_t cmd, uint8_t *index);


void HaiNCT_CardAuthStart_Cmd(uint8_t u8Port);
#endif
