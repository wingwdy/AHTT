#ifndef __PROTOCOL_CODEC_YKC_V2_H__
#define __PROTOCOL_CODEC_YKC_V2_H__

#include "protocol_ctrl.h"
#include "tcp_gn.h"

#define DD_PROTOCOL_VER				14				//东电
#define YKC_PROTOCOL_VER			16				//
#define TT_PROTOCOL_VER				17				//铁塔

#define YKC_FRAME_HEAD				0x68			//

#define CALL_SUCC                   0               //召唤成功
#define CALL_FAIL                   1               //召唤失败

#define CHG_SUCC                    0               //充电机启动成功
#define CHG_FAIL                    1               //充电机启动失败


#define SET_SUCC                    1               //设置成功
#define SET_FAIL                    0               //设置失败

#define NET_TYPE                    4               //支持网络制式（只支持4G）
#define NET_TYPE_NOW                4               //当前网络制式（当前使用4G）


/*************************上报平台一些反馈公共使用****************************/
typedef struct
{
	U8 upFaultCnt;				    //故障上报次数，上限3次，3次之后停止上报
	U8 ExistChargeFault;			//1订单进行中，0不存在订单，上报完成, 有的平台需要订单未上报之前实时订单交易流水号不清零，需要状态变位

} YKC21_UpPlatInfo;


enum {
    YKC_V2_S_Identification = 0x01,   	//登录认证
    YKC_V2_R_Identification = 0x02,   	//认证应答
    YKC_V2_S_Heart = 0x03,           	//心跳包
    YKC_V2_R_Heart = 0x04,           	//心跳应答
    YKC_V2_S_Rate_Proving = 0x05,      //计费模型验证请求
    YKC_V2_R_Rate_Proving = 0x06,     	//计费模型验证请求应答
    YKC_V2_S_Rate_Ask = 0x09,      	    //计费模型请求
    YKC_V2_R_Rate_Ask = 0x0A,     		//计费模型请求应答
    
    YKC_V2_R_RealData = 0x12,    		//平台读取实时检测数据                  
    YKC_V2_S_RealData = 0x13,        	//上传实时检测数据 
	
    YKC_V2_S_Auth = 0xA5,    			//充电桩主动申请启动充电
    YKC_V2_R_Auth = 0xA6,   			//运营平台确认启动充电
    
    YKC_V2_S_Start_Chg_Ack = 0xA7,  	//远程启机命令回复
    YKC_V2_R_Start_Chg = 0xA8,         //运营平台远程控制启机
    
    YKC_V2_S_Stop_Chg_Ack = 0x35,  	    //停止充电回复
    YKC_V2_R_Stop_Chg = 0x36,        	//平台停止充电
	
    YKC_V2_S_Chg_Record = 0x3D,     	//交易记录上传
    YKC_V2_R_Chg_Record = 0x40,     	//平台交易记录确认
    
    YKC_V2_S_Sum_ACK = 0x41,        	//余额更新应答
    YKC_V2_R_Sum_Update = 0x42,      	//账户余额更新

    YKC_V2_S_Device_Fault = 0x50,      //设备故障上送
    YKC_V2_R_Device_Fault_ACK = 0x49,  //设备故障上送回复确认

    YKC_V2_S_Device_Reset = 0x4B,      //设备故障复位上送
    YKC_V2_R_Device_Reset_ACK = 0x4A,  //设备故障复位上送回复确认
   
    YKC_V2_S_Deal_ACK = 0x4C,          //交易记录召唤
    YKC_V2_R_Deal = 0x4D,              //交易记录召唤确认

    YKC_V2_S_Chg_Finish = 0x4F,         //充电机启动完成
    YKC_V2_R_Chg_Finish_ACK = 0x4E,     //充电机启动完成应答

	YKC_V2_S_Power_Change_ACK = 0x51,	//功率修改应答
	YKC_V2_R_Power_Change_Para = 0x52, //功率修改
	
	YKC_V2_S_TimeSyn_ACK = 0x55,		//对时设置应答
	YKC_V2_R_TimeSyn = 0x56,			//对时设置
	
	YKC_V2_S_Rate_ACK = 0x57,			//计费模型应答
	YKC_V2_R_Set_Rate = 0x58,			//计费模型设置

	YKC_V2_S_Max_Power_ACK = 0x59,	    //默认最大功率下发应答
	YKC_V2_R_Max_Power = 0x60,	        //默认最大功率下发

	YKC_V2_S_QR_ACK = 0x5A,			//二维码设置应答
	YKC_V2_R_Ret_QR = 0x5B,			//二维码设置

	YKC_V2_S_Para_ACK = 0x5E,			//参数设置应答
    YKC_V2_R_Para = 0x5F,			    //参数设置            

    YKC_V2_R_set_reboot = 0x92,     	//平台设远程重启
    YKC_V2_S_reboot_ACK = 0x91,      	//设备应答

    YKC_V2_R_set_update_ftp = 0x94,	//平台设远程升级程序
    YKC_V2_S_update_ACK = 0x93,    	//设备应答

    YKC_V2_S_Key_Update_Ack = 0x95,    //密钥更新应答 
    YKC_V2_R_Key_Update = 0x96,        //密钥更新

	YKC_V2_R_Set_QR_DD = 0x9C,			//设置二维码 东电
	YKC_V2_S_QR_ACK_DD = 0x9B,			//设置二维码应答 东电
	
};

typedef struct _platinfo{
	unsigned char Rsa_Key[RSA_KEY_LEN+1];	//云快充2.1 rsa公钥
	uint8_t Token[14+1];			//token
} YKC21_FlashPlatInfo;

typedef enum
{
    ePlatType_N                = 0x00,     // 无效值 */
    ePlatType_A                = 0x01,     // 车故障
    ePlatType_B                = 0x02,     // 车桩交互故障
    ePlatType_C                = 0x03,     // 桩/平台故障
    ePlatType_D                = 0x04,     // 桩故障
    ePlatType_E                = 0x05,     // 自定义故障
}ERR_PLATFORM;

typedef struct _rate_model{
    //多类电价计费模型
    U8 rate_num;
    U8 rate_price[48][4];
    U8 rate_ele[48][4];
    U8 rate_loss_ele[48][4];
    U8 rate_amount[48][4];
    U8 rate_period_ele[48][4];
} rate_model_ykc_v2;


enum {
	YKC21_Reason_Updata 			        = 0x00,
	YKC21_Reason_StopCPErro 			    = 0x01,
	YKC21_Reason_StopCPGnd 			        = 0x02,
	YKC21_Reason_StoPEGnd 			        = 0x03,
	YKC21_Reason_StopPhase 			        = 0x04,

	YKC21_Reason_StopLeak 			        = 0x07,
	YKC21_Reason_StopRelayMissTrip 	        = 0x13,    //拒动
	YKC21_Reason_StopRelayCgltnt		    = 0x14,    //粘连

	YKC21_Reason_StartDiode 	            = 0x25,

	YKC21_Reason_Finish_App 			    = 0x40,
	YKC21_Reason_Finish_Soc 			    = 0x41,		//soc = 100%
	YKC21_Reason_Finish_Ele 			    = 0x42,
	YKC21_Reason_Finish_Sum 			    = 0x43,
	YKC21_Reason_Finish_Time 			    = 0x44,
	YKC21_Reason_Finish_Manual 		        = 0x45,
	YKC21_Reason_Finish_CarOk 		        = 0x46,
	
	YKC21_Reason_Stop_Other 			    = 0x65,		//其他原因
	
	YKC21_Reason_Stop_GunBreak 		        = 0x6B, 	//充电异常中止，导引断开
	YKC21_Reason_Stop_BreakErr 		        = 0x6C, 	//充电异常中止，断路器跳位
	YKC21_Reason_Stop_MeterErr 		        = 0x6D, 	//充电异常中止，电表通信中断
	YKC21_Reason_Stop_SumNoEnough 	        = 0x6E,     //充电异常中止，余额不足
	YKC21_Reason_Stop_EmergencyStop 	    = 0x72,  	//充电异常中止，急停开入
	YKC21_Reason_Stop_TmpErr 			    = 0x74, 	//充电异常中止，温度异常
	YKC21_Reason_Stop_OverCur 		        = 0x75, 	//充电异常中止，输出异常
	YKC21_Reason_Stop_LittleCurr 		    = 0x76, 	//充电异常中止，充电无流
	YKC21_Reason_Stop_ErrVal 		        = 0x79, 	//充电异常中止，总充电电压异常
	YKC21_Reason_Stop_ErrCurr 		        = 0x7A, 	//充电异常中止，总充电电流异常
	
	YKC21_Reason_Interupt_PwOff 		    = 0x83,		//充电异常中止，充电设备断电	
    
    YKC21_Reason_Limit_Ele  		        = 0x8A,     //充电异常中止，可充电量余额不足

	YKC21_Reason_UnKnow 				    = 0x90,
	YKC21_Reason_Finish 				    = 0xFF,     //订单上报完成，不需要开机上报

    //平台无对应停止原因，以下停止原因减去0x90为桩的原始停止原因 
    // YKC21_Reason_Stop_Reason_CPErro 		= 0x91,     //CP电压异常
    // YKC21_Reason_Stop_CPGnd                 = 0x92,     //CP接地停止
    // YKC21_Reason_Stop_PEGnd                 = 0x93,     //PE接地故障停止
    // YKC21_Reason_Stop_Leak                  = 0x95,     //漏电故障停止
    // YKC21_Reason_Stop_RlySyn                = 0x96,     //继电器粘连
    // YKC21_Reason_Stop_RlyRfs                = 0x97,     //继电器拒动，输出异常
    // YKC21_Reason_Stop_Reason_PlugTempOver   = 0x99,     //插头过温
    // YKC21_Reason_Stop_Reason_AirTempOver    = 0x9B,     //环境过温
    // YKC21_Reason_Stop_Reason_VolUnder       = 0x9D,     //欠压

    // YKC21_Reason_Stop_Reason_StartDiode     = 0xB0,     //二极管异常
    // YKC21_Reason_Stop_Reason_ShortCircle    = 0xB1,     //短路检测故障

    // YKC21_Reason_Stop_Reason_MaxTime        = 0xE0,     //超过最长时间停止 
	// YKC21_Reason_StartTimeout 	            = 0xB3,     //S2一定时间内不闭合，车辆拒绝，目前暂定15s
    // YKC21_Reason_StopEleErro       	        = 0xC1,     //电表数据异常
	// YKC21_Reason_StopComm 			        = 0xC2,    //充电单元通信故障
	// YKC21_Reason_StopMoneyErro       	    = 0xC4,    //充电中金额异常
	// YKC21_Reason_StopSetKey 	            = 0xD9,     //按键停止

};

typedef struct {
    //交易记录信息，0x3D  共858字节
    U8 transaction_log_num[16];
    U8 device_number[7];
    U8 gun_num;
    U8 chrg_start_time[7];         	//开始充电时间,bcd
    U8 chrg_stop_time[7];         	//结束充电时间
 
    U8 fee_num;                     //费率个数
	U8 fee_rate[48][4];        	    //费率单价 电费+服务费 小数点后五位
    U8 fee_ele[48][4];              //费率电量 小数点后四位
    U8 loss_fee_ele[48][4];         //费率计损电量 小数点后四位
    U8 time_power[48][4];           //48时段电量 小数点后四位
	
	U8 total_start_elec[4];       	// 总起示值 小数点后四位
    U8 total_stop_elec[4];        	// 总止示值
    U8 total_power[4];        		// 总电量
    U8 total_loss_power[4];        	// 总计损电量
    U8 total_money[4];        		// 总金额
	
    U8 vin[17];          		    //电动汽车唯一标识vin码,正序上传，ASCII
    U8 trade_flag;           	    //交易标识 0x01：app 启动 0x02：卡启动
    							    //0x04：离线卡启动 0x05: vin 码启动充电
    U8 trade_time[7];               //交易日期，年月日时分秒
    U8 stop_reason;       		    //交易停止原因
    U8 send_flag;                   //发送标识
    U8 card_number[4];              //物理卡号
} charge_record_ykcv2;

//启动充电失败原因
enum {
    eStart_Fail_NULL_V2 			= 0,     	//无
    eStart_Fail_DevNumErr_V2 		= 0x01,     //设备编号不匹配
    eStart_Fail_Working_V2 		= 0x02,    	//枪已在充电
    eStart_Fail_DevErr_V2 			= 0x03, 	//设备故障
    eStart_Fail_Offline_V2 		= 0x04,		//设备离线
    eStart_Fail_NoConn_V2 			= 0x05,		//未插枪
    eStart_Fail_ErrRate_V2,					//计费模型错误
};


//云快充铁塔东电接收数据结构体
typedef struct
{
    uint8_t             MilliSec[2]; // 毫秒
    uint8_t             Minute : 6;  // 分
    uint8_t             res1   : 2;                    
    uint8_t             Hour   : 5;  // 时
    uint8_t             res2   : 3;                    
    uint8_t             Date   : 5;  // 日
    uint8_t             Day    : 3;  // 周                   
    uint8_t             Month  : 4;  // 月
    uint8_t             res4   : 4;                    
    uint8_t             Year   : 7;  // 年
    uint8_t             res5   : 1;
}cp56time2a_v2;

typedef struct
{
    uint8_t     head;
    uint8_t     len[2];
    uint8_t     ser[2];
    uint8_t     send_time[7];    
    uint8_t     EncType;		//0x00-数据不加密，0x01-加密
    uint8_t     cmd;	
}YKC_V2_HEAD_T;


typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t charge_login_result;         //充电登录认证结果 0x00：登陆成功 0x01:登陆失败
    uint8_t key_len;                     //固定128
    char New_RSA_key[128];               //最新RSA配置公钥
}YKC_V2_Recv_Identification;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         //
    uint8_t ack;         //
}YKC_V2_Recv_Heart;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t Rate_num[2];         //
    uint8_t result;         //0x00 桩计费模型与平台一致 0x01 桩计费模型与平台不一致
}YKC_V2_Recv_Rate_Proving;

//公牛 云快充
typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 billing_model[2];        	//计费模型编号，首次链接到平台时置零
    
    U8 Rate_quantity;               //最多48个计费模型
	U8 ele_rate[48][4];        	    //电费费率 精确到五位小数
    U8 ser_rate[48][4];             //服务费费率
    U8 measure_wastage_rates;       //计损比率
    U8 segmentation_rate[48];       //0：00～0：30 时段费率号
}YKC_V2_Recv_Rate_Model;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
}YKC_V2_Recv_RealData_Rsq;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Logic_card_number[8];    	//逻辑卡号，BCD，卡面印刷卡号
    uint8_t account_balance[4];        	//账户余额，2位小数
    uint8_t Max_Power[2];               //本次充电当前允许的最大功率
    uint8_t SOC_limit[1];               //SOC限制
    uint8_t Charge_capacity_limit[4];   //充电电量限制
    uint8_t Auth_success;				//鉴权充电成功标志，0失败，1成功
    uint8_t Auth_Fail_Reason;			//鉴权启动充电失败原因	BCD 码
    //0x01 账户不存在
	//0x02 账户冻结
	//0x03 账户余额不足
	//0x04 该卡存在未结账记录
	//0x05 桩停用
	//0x06 该账户不能在此桩上充电
	//0x07 密码错误
	//0x08 电站电容不足
	//0x09 系统中 vin 码不存在
	//0x0A 该桩存在未结账记录
	//0x0B 该桩不支持刷卡
}YKC_V2_Recv_Auth_Ack;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Logic_card_number[8];    	//逻辑卡号，BCD，卡面印刷卡号
    uint8_t Physics_card_number[8];    	//物理卡号，BCD，卡面印刷卡号
    uint8_t account_balance[4];        	//账户余额，2位小数
    uint8_t chg_max_power[2];        	//单位:kW 默认值 0000；当值为0000时按默认最大功率报文下发的功率限制，如无默认最大功率限制则按无限制执行
    uint8_t soc_limit;        	        //默认 0x00，不限制
    uint8_t chg_ele_limit[4];        	//精确到小数点后四位；默认全 0，不限制
}YKC_V2_Recv_Start_Charge;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//

}YKC_V2_Recv_Stop_Charge;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
    uint8_t result;         		//

}YKC_V2_Recv_Record_Ack;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Physics_card_number[8];    	//物理卡号，BCD
    uint8_t account_balance[4];        	//账户余额，2位小数
}YKC_V2_Recv_SumUpdata;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun; 
    uint8_t rec_mark;        	//0x00 上传成功 0x01 非法故障
}YKC_V2_REcv_Device_Fault;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;    	
    uint8_t rec_mark;        	
}YKC_V2_REcv_Device_Reset;

typedef struct
{
    uint8_t deal_num[16];                //交易流水号
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;    		
}YKC_V2_Recv_Deal;

typedef struct
{
    uint8_t deal_num[16];                //交易流水号
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;
    uint8_t start_result;                //启动结果
}YKC_V2_Recv_Chg_Finish;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;
    uint8_t power_max[2];
    uint8_t instruct_rsp_priority;        //指令响应优先级
    uint8_t limit_time[2];                //限制时间
}YKC_V2_Recv_Power_Change;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    cp56time2a_v2 cur_time;
}YKC_V2_Recv_TimeSyn;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;
    uint8_t default_max_power[2];
    cp56time2a_v2 start_time;
    cp56time2a_v2 stop_time;
}YKC_V2_Recv_Max_Power;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t platform_set_reboot;      	//0x01：立即执行 0x02：空闲执行
}YKC_V2_Recv_Reboot;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
	uint8_t device_type;					//0x01：直流 0x02：交流
	uint8_t device_rate[2];				//设备功率
    uint8_t update_ip[16];               //升级服务器ip,ascii
    uint8_t update_port[2];              //升级服务器端口
    uint8_t update_username[16];     	//用户名ascii
    uint8_t update_password[16];   		//密码ascii
    uint8_t update_file_path[32];      	//文件路径ascii
    uint8_t update_file_name[32];      	//文件名称
    uint8_t update_ctrl;               	//1升级立即执行，2空闲执行
    uint8_t update_timeout;           	//超时时间，min
    uint8_t MD5_signature;           	//升级文件的MD5校验码ascii
}YKC_V2_Recv_Update_ftp;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
	uint8_t gun;	
	uint8_t QR_type;				            //二维码码制
    uint8_t QR_len;                             //二维码长度						//
	uint8_t QR_data[QR_MAX_SIZE];				//二维码

}YKC_V2_Recv_QR;

typedef struct
{
	uint8_t gun;							
	uint8_t QR_len[2];				            //二维码长度
	uint8_t QR_data[QR_MAX_SIZE];				//二维码
}YKC_V2_Recv_QR_DD;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];						
	uint8_t gun;
	uint8_t PlugChrg;                           //即插即充开关  0x00 支持，默认 0x01 不支持
    uint8_t auth_timeout;                       //鉴权超时时间  单位：秒
    uint8_t offline_chrg;                       //离线充电时间  桩和平台离线后，达到配置时间，停止充电；单位：秒；默认 0，不限制。
}YKC_V2_Recv_Para;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];						
	uint8_t key_len;                        //密钥长度
	uint8_t new_key[128];				    //最新密钥
    uint8_t exe_control;                    //执行控制 0x01：立即执行 0x02：空闲执行，默认
}YKC_V2_Recv_Key_Update;

typedef struct
{
	U16 flow;						// flow
	U8 WarnType;				    // warning type
	U16 WarnId;						// warning id 
	U8 status;						// warning status, 0-normal, 1-warning
	U8 StartTime[7];				// start time
	U8 StopTime[7];					// stop time
    U8 ErrorIdx[7];
}YKC_V2_Send_ErrSend;

typedef struct
{
	YKC_V2_Recv_Identification 	    strRecvIdenf;			//应答-登录认证
	YKC_V2_Recv_Heart 				strRecvHeart;			//应答-心跳
	YKC_V2_Recv_Rate_Proving 		strRecvRateProving;		//应答-计费模型版本
	YKC_V2_Recv_Rate_Model 		    strRecvRateModel;
	YKC_V2_Recv_RealData_Rsq 		strRecvRealDataRsq;
	YKC_V2_Recv_Auth_Ack 			strRecvAuthAck;
	YKC_V2_Recv_Start_Charge		strRecvStartCharge;
	YKC_V2_Recv_Stop_Charge 		strRecvStopCharge;
	YKC_V2_Recv_Record_Ack 		    strRecvRecordAck;
	YKC_V2_Recv_SumUpdata 			strRecvSumUpdata;
    YKC_V2_REcv_Device_Fault        strRecvDeviceFault;
    YKC_V2_REcv_Device_Reset        strRecvDeviceReset;
    YKC_V2_Recv_Deal                strRecvDeal;
    YKC_V2_Recv_Chg_Finish          strRecvChg_Finish;
	YKC_V2_Recv_Power_Change 		strRecvPower;
	YKC_V2_Recv_TimeSyn 			strRecvTimeSyn;	
    YKC_V2_Recv_Max_Power           strRecvMaxPower;	
	YKC_V2_Recv_Reboot 			    strRecvReboot;
	YKC_V2_Recv_Update_ftp 		    strRecvUpdata;
	YKC_V2_Recv_QR 				    strRecvQR;
    YKC_V2_Recv_Para                strRecvPara;
    YKC_V2_Recv_Key_Update          strRecvKeyUpdate;
	YKC21_FlashPlatInfo 		    strSetParam;
}YKC_RECV_Data_V2;

// void YKC21_WriteStoragePara(unsigned char *pFlashInfo);

void YKCUpProtocolDeal_V2(void);

void YKC_CardAuthStart_Cmd_V2(uint8_t u8Port);
void YKC_DealUpdate_Cmd_V2(uint8_t u8Port);

void YKC21_WriteRsaKey(char *key);
void YKC21_WriteToken(char *token);

#endif

