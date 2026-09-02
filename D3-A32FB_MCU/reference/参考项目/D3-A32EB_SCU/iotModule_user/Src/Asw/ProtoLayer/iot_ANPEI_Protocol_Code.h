#ifndef __PROTOCOL_CODEC_ANPEI_H__
#define __PROTOCOL_CODEC_ANPEI_H__

 #include "protocol_ctrl.h"

//#define TEST_FAKE_open

#define GUN_NUM_MAX_ANPEI   GUN_NUM_MAX
#define GUN_NUM_ANPEI   	GUN_NUM

#define ANPEI_FRAME_HEAD 0x68 //

#define ANPEI_FLASH_RATE_MODEL_ADDR  EXT_FLASH_RATE_MODEL_ADDR //安培存储费率地址
//安培交易标识（启动方式）
enum
{
	eUP_Start_Style_NULL_ANPEI = 0,			  // 无
	eUP_Start_Style_App_ANPEI = 0x01,		  // app 启动
	eUP_Start_Style_CardOnline_ANPEI = 0x02,  // 卡启动
	eUP_Start_Style_CardOffline_ANPEI = 0x04, // 离线卡启动
	eUP_Start_Style_VIN_ANPEI = 0x05,		  // vin 码启动充电
	eUP_Start_Style_Test_ANPEI = 0x06,		  // 厂内测试启动
	eUP_Start_Style_Fixtime_ANPEI = 0x07,	  // 定时启动
};

//安培启动充电失败原因
enum {
    eUP_Start_Fail_NULL_ANPEI 		= 0,     	//无
    eUP_Start_Fail_DevNumErr_ANPEI 	= 0x01,     //设备编号不匹配
    eUP_Start_Fail_Working_ANPEI 		= 0x02,    	//枪已在充电
    eUP_Start_Fail_DevErr_ANPEI 		= 0x03, 	//设备故障
    eUP_Start_Fail_Offline_ANPEI 		= 0x04,		//设备离线
    eUP_Start_Fail_NoConn_ANPEI		= 0x05,		//未插枪
    eUP_Start_Fail_Reconnect_ANPEI 	= 0x06,		//超时不可启动，需要重新插拔枪
};

//停止充电失败原因
enum {
    eUP_Stop_Fail_NULL_ANPEI 			= 0,    	//无
    eUP_Stop_Fail_DevNumErr_ANPEI	= 0x01,   	//设备编号不匹配
    eUP_Stop_Fail_NoWorking_ANPEI 	= 0x02,    	//枪未处于充电状态
    eUP_Stop_Fail_Other_ANPEI 		= 0x03, 	//其他
};



/******************状态*******************************************************************/
// 只能用于上行上报,不能用于流程判断 
enum
{
	eUP_Gun_State_Offline_anpei = 0,		// 离线
	eUP_Gun_State_Err_anpei = 0x01,			// 故障
	eUP_Gun_State_Idle_anpei = 0x02,		// 空闲
	eUP_Gun_State_Work_anpei = 0x03,		// 充电
	eUP_Gun_State_Conn_anpei = 0x04,		// 已插枪未充电
	eUP_Gun_State_Finish_anpei = 0x05,		// 充电完成未拔枪
	eUP_Gun_State_reservation_anpei = 0x06, // 预约车位
	eUP_Gun_State_Fixtime_anpei = 0x07,		// 定时充电

};

/*************************费率结构体*****费率变量******费率相关函数****************************/

// B2的费率模型
// 约95字节
typedef struct
{
	U8 billing_model[8]; // 计费模型编号
	U8 start_time[7];
	U8 end_time[7];
	U8 workstate[2];		  // 1-有效 2-无效
	U8 Meterkind[2];		  // 1-里程，2-充电量 3-放电量
	U8 segmentation_rate[48]; // 一天分成48时间段，半个小时一段
	U8 fee_Num;				  // 费率数 目前只支持4个
	U8 sharp_ele_fee[4];	  // 尖
	U8 peak_ele_fee[4];		  // 峰
	U8 flat_ele_fee[4];		  // 平
	U8 valley_ele_fee[4];	  // 谷
	U8 sever_ser_fee[4];	  // 充电服务费单价

} FeeModelB2; // 费率模型

// B47费率模型
typedef struct
{
	U8 Serial_number;
	U8 Serial_rate; // 1-8
	U8 rate_start[2];
	U8 rate_end[2];
	U8 ele_fee[4];
	U8 ser_fee[4]; //

} Fee_data;

// 约193字节
typedef struct
{   
	U8 billing_model[8]; // 计费模型编号
	U8 start_time[7] ;
	U8 end_time[7];
	U8 workstate[2]; // 1-有效 2-无效
	U8 time_allnum;	 // 时段数量(<=12)
	Fee_data B47modeldata[12];
} FeeModelB47; // 费率模型


#define B47_A   0
#define B47_B   1
typedef struct
{   
	U8 RecentUpdates_Nomber;   //记录最近一次更新的B47费率的是FeemodelB47save_data[]的下标 是(0：B47_A)还是(1:B47_B )
	FeeModelB47 FeemodelB47save_data[2];
} FeeModel_Save_truct; // 存储的费率模型结构体


/******计量电量结构体******************************************************************************/
// 实时电量记录 电量 费用； B53交易记录里数据可直接调用该结构体里数据
typedef struct
{
	U8 transaction_log_num[16];	 // 订单号  成功启动才更新 B1阶段使用 
	U8 anpei_card_number[8];     //鉴权卡号，BCD，卡面印刷卡号; 平台交易流水后16 位编码，用做在线交易记录中的物理卡号	

	U8 start_waykind;			 //  交易标识(启动方式) 0无； 0x01：app 启动； 0x02：卡启动； 0x04：离线卡启动 0x05: vin 码启动充电 0x06;厂内测试 0x07:app定时启动
	U8 controlfee[4];			 // 控制策略 0.01,APP情况下表示充电金额 ，定时APP模式下，表示预充金额 ；

	U8 start_time[7];			 // 预约/定时开始时间 仅预约/定时有效  年H 年L 月 日 时 分 秒  B25 B26阶段使用
	U8 end_time[7];				 // 预约/定时结束时间 仅预约/定时有效  年H 年L 月 日 时 分 秒  B25 B26阶段使用

    
    
	U8 total_EE[4];				 // 0.001 当前订单的总电量     B1 B53阶段使用
	U8 total_sharpEE[4];		 // 0.001 当前订单的累计尖     B1阶段使用
	U8 total_peakEE[4];			 // 0.001                	  B1阶段使用
	U8 total_flatEE[4];			 // 0.001                     B1阶段使用
	U8 total_valleyEE[4];		 // 0.001 当前订单的累计谷电量 B1阶段使用

	U8 total_chrg_time[2];		 // min    当前订单充电时间    B1 B53阶段使用
	// U8 total_chrg_EEmomey[4];	 // 0.01   当前订单充电费      B1  B53阶段使用
	// U8 total_chrg_severmomey[4]; // 0.01  当前订单服务费       B1  B53阶段使用
	uint64_t total_chrg_EEmomey ;	 // 电费 4位        B1  /100使用 
	uint64_t total_chrg_severmomey ; // 服务费4位         B1  /100使用

	// U32 Time_electricity[12] ;   //分时段电量0.0001    		   B53阶段/10使用
	// uint64_t Time_elecbill[12] ; //分时段电费 4位    		   B53阶段/100使用
	// uint64_t Time_severbill[12] ;//分时段服务费4位  		   B53阶段/100使用
} chrg_EE_Money_struct;


// 更新里面数据
enum
{
	No_transaction_log_num = 0, //
	No_anpei_card_number,
	No_start_way , //
	No_controlfee,//
	No_start_time ,
	No_end_time,
	No_total_EE,//
	No_total_sharpEE,//
	No_total_peakEE,//
	No_total_flatEE,//
	No_total_valleyEE,//
	No_total_chrg_time,//
	// No_total_chrg_EEmomey,//
	// No_total_chrg_severmomey,//

	No_Time_electricity,
	No_Time_elecbill,
	No_Time_severbill,

	


};

typedef struct
{
	U8 total_00_EE[4]; //0.001 
	U8 total_00_sharpEE[4];		 // 0.001  累计尖     
	U8 total_00_peakEE[4];			 // 0.001                	  
	U8 total_00_flatEE[4];			 // 0.001                      
	U8 total_00_valleyEE[4];		 // 0.001  累计谷电量 
	

} UP_0000_clock_struct;

/******交易记录实时更新和存储的 结构体******************************************************************************/
// B12 交易记录

typedef struct
{
	U8 transaction_log_num[16];
	U8 Logic_card_number[8];
	U8 chrg_start_time[7];
	U8 chrg_stop_time[7];
	U8 chrg_chrg_totaltime[2]; // min

	U8 sharp_start_power[4];
	U8 sharp_stop_power[4];

	U8 peak_start_power[4];
	U8 peak_stop_power[4];

	U8 flat_start_power[4];
	U8 flat_stop_power[4];

	U8 valley_start_power[4];
	U8 valley_stop_power[4];

	U8 sharp_total_power[4];
	U8 peak_total_power[4];
	U8 flat_total_power[4];
	U8 valley_total_power[4];

	U8 total_total_power[4];
	U8 total_start_power[4];
	U8 total_stop_power[4];

	U8 soc_start[2];
	U8 soc_end[2];

	U8 Car_onlycode[32];
	U8 stop_reason[2];
	U8 chrg_total_money[4];
	U8 sver_total_money[4];

	U8 card_number[8];
	U8 card_YN_open;

	U8 UPB12_cnt;		   // 上传次数
	U8 ExistChargeDealB12; // 1订单进行中，
} RecordB12;



typedef struct
{
	U8 time_serrnumber;
	U8 time_kind;
	U8 chrg_totalpower[3]; // 0.001
	U8 chrg_totalmoney[3]; // 0.01
	U8 chrg_servemomey[3]; // 0.01
} Fee_B53;

// 最多230个字节
typedef struct
{
	U8 transaction_log_num[16];

	U8 FeeModel_timenum;
	Fee_B53 Fee_B53data[12];

	U8 chrg_start_time[7];
	U8 chrg_stop_time[7];
	U8 chrg_chrg_totaltime[2]; // min

	U8 chrg_total_money[3];
	U8 sver_total_money[3];

	U8 total_total_power[3];
	U8 total_start_power[4];
	U8 total_stop_power[4];

	U8 soc_start[2];
	U8 soc_end[2];

	U8 Logic_card_number[8];
	U8 Car_onlycode[32];
	U8 stop_reason[2];


} RecordB53;



/**********************************************************************************/
/******功率调节******************************************************************************/
// 理论此刻应该输出的功率类型 0.01
typedef enum {
    E_platPowerDefault = 1,
    E_platPowerDynamic = 2,
    E_platPowerControl = 3,
    E_platPowerMax,
}E_platPower;

typedef struct {
    uint32_t CtrlPower[E_platPowerMax];
	uint8_t Powerkind; // 1 默认 2 动态 3 控制
    uint32_t ActPower;      //实际使用功率
    uint32_t DynamicTime;   //动态功率超时时间
	uint16_t timepre;  // 控制周期（仅针对控制功率类型时，该值有效）
} Stu_GunPowerCtrArry;


/**************************************************/

/************************************************************************************/
// 协议里B1-B61 ANPEI
enum
{

	ANPEI_S_RealData = 0x01, // B1 充电过程实时监测数据   Type 134  cot 6  recordKind 无

	ANPEI_R_Rate_SETB2 = 0x02,	  // B2. 下发计费模型下行数据 Type 133  cot 6  recordKind 5
	ANPEI_S_Rate_SETAskB3 = 0x03, // B3. 下发计费模型结果数据（基础）Type 130  cot 7  recordKind 6

	ANPEI_R_StartEnd_Chg = 0x04,	// B4. 充电启停控制命令下发下行数据（扫码充电）Type 133  cot 6  recordKind 21
	ANPEI_S_StartEnd_ChgAsk = 0x05, // B5. 充电启停控制命令结果确认（扫码充电）Type 133  cot 7  recordKind 21

	ANPEI_S_Cardinf = 0x06,	   // B6. 刷卡鉴权上行（在线刷卡充电）Type 130  cot 6  recordKind 1
	ANPEI_R_CardinfAck = 0x07, // B7. 刷卡鉴权下行（在线刷卡充电）Type 133  cot 7  recordKind 2

	ANPEI_S_CardStart_Chg = 0x10,	 // B10. 启动通知上报（在线刷卡充电/在线vin码充电）Type 130  cot 6  recordKind 14
	ANPEI_R_CardStart_ChgAck = 0x11, // B11. 启动通知下行（在线刷卡充电/在线vin码充电）Type 133  cot 7  recordKind 12

	ANPEI_S_onlineEnd_ChgInfB12 = 0x12,	   // B12. 在线情况下停止充电时上传记录数据（基础）Type 130  cot 6  recordKind 2
	ANPEI_R_onlineEnd_ChgInfAckB13 = 0x13, // B13. 在线交易包下行数据（基础）Type 130  cot 7  recordKind 2

	ANPEI_R_ChgDeduction_Inf = 0x14, // B14. 充电扣款后下行数据（基础）Type 133  cot 6  recordKind 3

	ANPEI_S_offlineEnd_ChgInfB15 = 0x15,	// B15. 离线交易上线后上传交易记录数据（基础）	Type 130  cot 6  recordKind 3
	ANPEI_R_offlineEnd_ChgInfAckB16 = 0x16, // B16. 离线交易包下行数据（基础）Type 130  cot 7  recordKind 3

	ANPEI_R_RemoteUpgrade = 0x23,	 // B23. 远程升级启动（扩展）Type 133  cot 6  recordKind 15
	ANPEI_S_RemoteUpgradeAck = 0x24, // B24. 远程升级启动命令接收结果（扩展）Type 130  cot 7  recordKind 14

	ANPEI_R_FixtimeCmd = 0x25,	  // B25. 预约/定时命令下行数据（扩展）	Type 133  cot 6  recordKind 54
	ANPEI_S_FixtimeCmdAsk = 0x26, // B26. 桩回复预约/定时结果上行数据（扩展）Type 130  cot 7  recordKind 24

	ANPEI_S_SIMInfAck = 0x31,  // B31. SIM卡信息上行数据（扩展）Type 130  cot 7  recordKind 27
	ANPEI_R_NeedSIMInf = 0x32, //	B32. 请求终端数据下行数据（扩展）Type 133  cot 6  recordKind 57

	ANPEI_R_PowerCon = 0x33,	   // B33. 充电功率控制下行（扩展）Type 133  cot 7  recordKind 58
	ANPEI_S_PowerConASK = 0x34,	   // B34. 充电功率控制上行（扩展）Type 130  cot 6  recordKind 28
	ANPEI_R_FeeModelB35 = 0x35,	   // B35. 计费模型召测下行数据（扩展）Type 133  cot 6  recordKind 59
	ANPEI_S_FeeModelB36Ask = 0x36, // B36. 计费模型召测上行数据（扩展）Type 130  cot 7  recordKind 29
	ANPEI_S_ChgCarInf = 0x37,	   // B37. 充电中车辆监测数据（扩展）Type 130  cot 6  recordKind 30
	ANPEI_S_2400Inf = 0x38,		   // B38. 零点示值上报(扩展)Type 130  cot 7  recordKind 31

	ANPEI_R_ftpInf = 0x39,	  // B39. 平台ftp服务器地址下发（扩展）Type 133  cot 6  recordKind 60
	ANPEI_S_ftpInfAsk = 0x40, // B40. 平台ftp服务器地址上行（扩展）Type 130  cot 7  recordKind 32

	ANPEI_R_PowerVal = 0x45,	// B45. 充电功率召测下行（扩展）Type 133  cot 6  recordKind 63
	ANPEI_S_PowerValAsk = 0x46, // B46. 充电功率召测上行（扩展）Type 130  cot 7  recordKind 35

	ANPEI_R_Rate_SETB47 = 0x47,	   // B47. 下发计费模型下行数据—分时服务费Type 133  cot 6  recordKind 64
	ANPEI_S_Rate_SETAskB48 = 0x48, // B48. 下发计费模型上行数据—分时服务费Type 130  cot 7  recordKind 36

	ANPEI_S_Rate_Swtich = 0x49,	   // B49. 计费模型切换生效上行—分时服务费Type 130  cot 7  recordKind 37
	ANPEI_R_Rate_SwtichAsk = 0x50, // B50. 计费模型切换生效下行—分时服务费Type 133  cot 6  recordKind 65

	ANPEI_R_FeeModelB51 = 0x51,	   // B51. 计费模型召测下行数据-分时服务费Type 133  cot 6  recordKind 66
	ANPEI_S_FeeModelB52Ask = 0x52, // B52. 计费模型召测上行数据—分时服务费Type 130  cot 7  recordKind 38

	ANPEI_S_onlineEnd_ChgInfB53 = 0x53,	   // B53. 在线情况下停止充电上传分时交易明细数据Type 130  cot 6  recordKind 39
	ANPEI_R_onlineEnd_ChgInfAckB54 = 0x54, // B54. 在线分时明细交易包下行数据Type 133  cot 6  recordKind 67

	ANPEI_S_offlineEnd_ChgInfB55 = 0x55,	// B55. 离线情况下停止充电上传分时交易明细数据Type 130  cot 6  recordKind 40
	ANPEI_R_offlineEnd_ChgInfAckB56 = 0x56, // B56. 离线分时明细交易包下行数据Type 133  cot 6  recordKind 68

	ANPEI_S_ChgPowerCon_inf = 0x57, // B57. 充电功率控制过程中的扩展实时状态（扩展）Type 130  cot 6  recordKind 41

	ANPEI_S_Identification = 0xF1, // 登录认证
	ANPEI_R_Identification = 0xF2, // 认证应答
	ANPEI_S_U = 0xF3,			   // U帧
	ANPEI_R_U = 0xF4,			   // U帧应答
	ANPEI_S_Heart = 0xF5,		   // 心跳包
	ANPEI_R_Heart = 0xF6,		   // 心跳应答
	ANPEI_S_clocksyn = 0xF7,	   // 时钟同步  Type 103  cot 7  recordKind 无
	ANPEI_R_clocksyn = 0xF8,	   // 时钟同步应答Type 103  cot 6 recordKind 无

};



// 认证
typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1]; // 充电桩编号 8个字节
} ANPEI_Recv_Identification;

// 结构体
typedef struct
{
	uint8_t val;
} ANPEI_Recv_U;

typedef struct
{

	uint8_t val;
} ANPEI_Recv_Heart;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Rate_num[2]; //
	uint8_t result;		 // 0x00 桩计费模型与平台一致 0x01 桩计费模型与平台不一致
} ANPEI_Recv_Rate_Proving;

typedef struct
{
	uint8_t head;
	uint8_t len;
	uint8_t control[4];
	uint8_t TypeIDE;	   // 类型标识
	uint8_t Vsq;		   // 可变结构限定词
	uint8_t Cot;		   // 传送原因
	uint8_t AppSerAddr[2]; // 应用服务数据单元公共地址，；
	uint8_t InfAddr[3];	   // 信息对象地址
	uint8_t recordKind;	   // 记录类型

} ANPEI_HEAD_T;

typedef struct
{
	uint8_t MilliSec[2]; // 毫秒
	uint8_t Minute;		 // 分
	uint8_t Hour;		 // 时;
	uint8_t Date;		 // 日 /
	uint8_t Month;		 // 月
	uint8_t Year;		 // 年

} cp56timeanpei;

typedef struct
{
	cp56timeanpei cur_time;
	uint8_t first_flag; // 首次对时
} ANPEI_Recv_TimeSyn;

typedef struct
{
	U8 device_number[DEV_NUM_LEN + 1];
	FeeModelB2 billing_modelB2;

} ANPEI_Recv_Rate_ModelB2;

typedef struct
{
	U8 device_number[DEV_NUM_LEN + 1];
	U8 Interface_mark; // 0：一桩一充； 1-一桩多充的1号枪 2...

	FeeModelB47 billing_modelB47;

} ANPEI_Recv_Rate_ModelB47;
typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];

	uint8_t update_ip[4]; // 升级服务器ip,
	uint8_t update_com[2];
	uint8_t update_username[10];  // 用户名ascii
	uint8_t update_password[10];  // 密码ascii
	uint8_t update_file_path[50]; // 文件路径ascii
} ANPEI_Recv_Update_ftp;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];

	uint8_t update_ip[4]; // 升级服务器ip,
	uint8_t update_com[2];
	uint8_t update_username[10];  // 用户名ascii
	uint8_t update_password[10];  // 密码ascii
	uint8_t update_file_path[50]; // 文件路径ascii
    uint8_t update_file_name[50];      	//文件名
    uint8_t updatemodel;
    uint8_t updateSoftver[30];
    uint8_t updateHalver[20];
} ANPEI_Recv_RemUp_Cmd;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Interface_mark;
	uint8_t billing_model[8]; // 计费模型编号

	uint8_t success_mark; // 0成功 1失败
} ANPEI_Recv_Rate_switchB50;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN + 1];
	uint8_t Interface_mark;
	uint8_t cntrol_cmd;
	uint8_t start_condition;
	uint8_t start_way;
	uint8_t start_Controldata[4];
	uint8_t user_number[DEV_NUM_LEN + 1];
	uint8_t transaction_log_num[16]; // 交易流水号，

} ANPEI_Recv_StartEnd_Charge;

// 启动充电失败原因
// enum
// {
// 	eStart_Fail_NULL_anpei = 0,		 // 无
// 	eStart_Fail_Working_anpei = 0x01, // 枪已在充电
// 	eStart_Fail_DevErr_anpei = 0x02,	 // 设备故障
// 	eStart_Fail_Errother_anpei,		 // 其他
// };

typedef struct
{
	uint8_t Interface_mark;
	uint8_t resultask; // 0成功
} ANPEI_Recv_Online_ask;

typedef struct
{
	uint8_t resultask; // 0成功
					   // uint8_t transaction_log_num[16]; 	//交易流水号，
} ANPEI_Recv_Offline_ask;

typedef struct
{
	uint8_t Deduction_result; // 1 扣款成功
	uint8_t Fail_reason[2];	  //

} ANPEI_Recv_DeductionRecord;

typedef struct
{
	//	uint8_t Interface_mark;
	//  uint8_t Logic_card_number[8];    	//逻辑卡号，BCD，卡面印刷卡号
	//	uint8_t billing_model[8];
	//  uint8_t Physics_card_number[8];
	uint8_t account_balance[4];
	uint8_t check_card;
	uint8_t failcheckreason[2];

} ANPEI_Recv_Card_inf;

typedef struct
{
	uint8_t Interface_mark;

	uint8_t result;
	uint8_t failreason[2];
	uint8_t transaction_log_num[16]; // 交易流水号，

} ANPEI_Recv_Card_start;

typedef struct
{
	uint8_t Interface_mark;
	uint8_t accountNumber[8];
	uint8_t chargefee[2];
	uint8_t transaction_log_num[16]; // 交易流水号，
	uint8_t state;//0开始预约, 1取消预约,  2开始定时, 3结束定时,
	uint8_t starttime[7];
	uint8_t endtime[7];

} ANPEI_Recv_fixtime_cmd;

typedef struct
{
	uint8_t timepower[7];
	uint8_t kind;
	uint8_t powerVual[4];
	uint8_t dafaultVal;
	uint8_t timeuppower[2];

} ANPEI_Recv_Powercontrol;

typedef struct
{
	uint8_t interface;

} ANPEI_Recv_PowerUP;

typedef struct
{
	uint8_t CP56time[7];
} ANPEI_Recv_FeeModelUPB35;

typedef struct
{
	uint8_t intreface;
	uint8_t CP56time[7];
} ANPEI_Recv_FeeModelUPB51;

typedef struct
{
	uint8_t kind[2];
	uint8_t receiveflag;//非协议里的
} ANPEI_Recv_SIMinf_up;

typedef struct
{
	ANPEI_Recv_Identification strRecvIdenf;			   // 初始化协议帧的回送
	ANPEI_Recv_U strRecvU;							   // U帧-回复
	ANPEI_Recv_Heart strRecvHeart;					   // 心跳-回复
	ANPEI_Recv_TimeSyn strRecvTimeSyn;				   // 时钟
	ANPEI_Recv_Rate_ModelB2 strRecvRateModelB2;		   // B2模型费率
	ANPEI_Recv_Rate_ModelB47 strRecvRateModelB47;	   // B47模型费率
	ANPEI_Recv_Rate_switchB50 strRecvRateswitchB50;	   // 费率模型切换B50
	ANPEI_Recv_Update_ftp strRecvUpdata;			   // B39
	ANPEI_Recv_RemUp_Cmd strRecvRemUp_Cmd;			   // B23
	ANPEI_Recv_StartEnd_Charge strRecvStartEndCharge;  // B4
	ANPEI_Recv_Online_ask strRecvOnlinetrans_ask;	   // B13 B54
	ANPEI_Recv_Offline_ask strRecvOfflinetrans_ask;	   // B16 B56
	ANPEI_Recv_DeductionRecord strRecvDeductionRecord; // 扣款记录 B14
	ANPEI_Recv_Card_inf strRecvCard_inf;			   // B7
	ANPEI_Recv_Card_start strRecvCard_start;		   // B11
	ANPEI_Recv_fixtime_cmd strRecvCard_fixtime_cmd;	   // B25
	ANPEI_Recv_Powercontrol strRecvPowercontrol;	   // B33
	ANPEI_Recv_PowerUP strRecvPowerUP;				   // B45
	ANPEI_Recv_FeeModelUPB35 strRecvFeeModelUPB35;	   // b35的命令
	ANPEI_Recv_FeeModelUPB51 strRecvFeeModelUPB51;	   // b51的命令
	ANPEI_Recv_SIMinf_up strRecvSIMinf_up;			   // B32

} ANPEI_RECV_Data;


//协议外相关函数
void AnpeiGet_PlatNumberBCD(uint8_t *pNum);
uint8_t AnpeiCurrentRateType(uint8_t u8Port);
void anpeiCostUpdate(uint8_t u8Port);

//实时数据处理
void Refresh_chrg_EE_Money_data(uint8_t port,uint8_t numkind, uint8_t *data,uint8_t databytenum);
void Clear_chrg_EE_Money_data(uint8_t u8Port);
//费率保存和读取
void save_rateB2_model_anpei(void *pRateM);
uint16_t  Serach_billingmodel_ID(uint8_t uport,uint8_t *needsearchID);
uint8_t compare_Fee_data(const Fee_data *a, const Fee_data *b) ;
uint8_t Comapare_content_billingModel(FeeModelB47 *a,FeeModelB47 *b);
void save_rateB47_model_anpei(FeeModelB47 *pRateM,uint8_t uport,uint8_t length);
void Read_rateB47_model_anpei(void);
//费率更新策略
void Set_FeemodelREE(uint8_t uport);
void Clear_FeemodelREE(uint8_t uport);
uint8_t FeeModel_errIixst(uint8_t uport);
bool ANPEI_Is_FeeModel_Valid( FeeModelB47 *FeeModelB47val);
 bool Refresh_NowbillModel(uint8_t port);
extern bool Read_rate_model_anpei(uint8_t port,uint8_t *pRate);
uint16_t Now_model_get_RatePriod(uint8_t port,uint8_t a_or_b,uint8_t hour,uint8_t minute);
uint16_t get_Rate_anpei_Priod(uint8_t u8Port);
//更新存储的记录
void anpei_packChgRecord(uint8_t u8Port, RecordB53 *pRecord);
void anpei_packChgRecord_init(uint8_t u8Port,RecordB53 *pRecord);
void anpei_packChgRecord_startFailinitData(uint8_t u8Port,uint8_t *transaction_log ,uint8_t stopenreason, RecordB53 *pRecord);
void ANpeiUpChargeRecordUpDealOffline(void);
uint8_t ANpeiUpChargeStorageDeal(uint8_t u8Port, void *deal, uint8_t len);

//枪状态
void ANPEIUpGunStateCheck(uint8_t u8Port);

//启动判断
uint8_t anpei_monitor_charge_start(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney,uint8_t *starttime,uint8_t *endtime);
extern void ANPEI_DealUpdate_Cmd(uint8_t u8Port);
//安培协议1.41收发
//........
//安培协议1.41收发

//启动认证
void ANPEIUpLogin(void);

// 任务状态处理
void ANPEIUpCtrlTaskDeal(void); 

//命令对应接收报文处理
void ANPEIUpCtrlRecvDeal(ANPEI_HEAD_T *pHead, uint8_t cmd, void *pindata, uint16_t inlen);
void ANPEIfrom_buffer_data(U8 *recv_buf, U16 *len);
void ANPEIPackConnectHandle(U8 *recv_buf, int totalLen);
void ANPEIUpRecvDeal(void);
void ANPEIRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd);
void ANPEIUpCtrlRecvOutTime(void);

void ANPEIUpProtocolDeal(void);


#endif
