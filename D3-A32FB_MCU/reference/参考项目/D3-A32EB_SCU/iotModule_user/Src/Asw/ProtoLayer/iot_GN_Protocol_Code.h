#ifndef __IOT_GN_PROTOCOL_CODE_H__
#define __IOT_GN_PROTOCOL_CODE_H__

#include "globals.h"
#include "tcp_gn.h"

#define DEVICE_TYPE    		1          	//0直流桩，1交流桩，2电瓶车桩

//公牛tcp头
#define TCP_HEAD_1      0xFA
#define TCP_HEAD_2      0xAF
//外发版本
#define TCP_HEAD_GNP_1      0x5A
#define TCP_HEAD_GNP_2      0xA5

#define UP_RESULT_FAIL 			0
#define UP_RESULT_SUCC 			1

#define UP_S_FRAME_ACK     		0      		//发送应答
#define UP_S_FRAME_SELF    		1         	//主动上报

#define PROTOCOL_ENCRYPT    0           //是否加密


#define GNDATA_PHYCARD_LEN   	    4      	// 物理卡号长度
#define GNDATA_CARD_LEN   			8      	// 公牛卡号长度
#define GNDATA_TRDNUM_LEN   		16    	// 公牛流水号长度

typedef struct
{
    uint8_t     head[2];
    uint8_t     ver[2];
    uint8_t     ser[2];
    uint8_t     EncType;		//0x00-数据不加密，0x01-加密
    uint8_t     cmd;	
    uint8_t     len[2];
}GN_HEAD_T;


enum {
    CMD_Request_Identification = 0x01,      //登录认证
    CMD_Response_Identification = 0x02,     //认证应答
    CMD_Request_Heart = 0x03,               //心跳包
    CMD_Response_Heart = 0x04,              //心跳应答
    CMD_Request_billing_verify = 0x05,      //计费模型验证请求
    CMD_Response_billing_verify = 0x06,     //计费模型验证请求应答
    CMD_Request_billing_model = 0x09,      	//计费模型请求
    CMD_Response_billing_model = 0x0A,     	//计费模型请求应答
    CMD_Response_multi_billing_model = 0x0B,//多类电价模型验证请求应答

    CMD_Request_realTime_gun = 0x13,        //上传实时检测数据
    CMD_Response_realTime_gun = 0x12,      	//平台读取实时检测数据
    
    CMD_Request_apply_statr_chrg = 0x31,    //申请启动充电
    CMD_Response_apply_statr_chrg = 0x32,   //平台确认启动充电

    CMD_Response_statr_chrg = 0x34,        	//平台启动充电
    CMD_Request_statr_chrg = 0x33,        	//开始充电回复
    CMD_Response_stop_chrg = 0x36,        	//平台停止充电
    CMD_Request_stop_chrg = 0x35,        	//停止充电回复

    CMD_Request_deal_log = 0x3F,        	//交易记录上传
    CMD_Request_multi_deal_log = 0x3E,      //九类交易记录上传
    CMD_Response_deal_log = 0x40,        	//平台交易记录确认

    
    CMD_Response_sum_update= 0x42,        	//账户余额更新
    CMD_Request_sum_update = 0x41,        	//余额更新应答
    CMD_Response_set_device_param= 0x52,    //平台对充电设备工作参数设置
    CMD_Request_set_device_param= 0x51,    	//设备应答
    CMD_Response_set_timing= 0x56,        	//平台对时设置
    CMD_Request_set_timing= 0x55,        	//设备应答
    CMD_Response_set_billing_model = 0x58,  //平台设置计费模型
    CMD_Response_set_multi_billing = 0x54,  //平台设置多类别计费模型
    CMD_Request_set_billing_model = 0x57,  	//设备应答
    
	CMD_Request_R_Ret_QR = 0x5A,			//设置二维码
	CMD_Response_S_QR_ACK = 0x59,			//设置二维码应答

    CMD_Response_set_reboot = 0x92,     	//平台设远程重启
    CMD_Request_set_reboot = 0x91,      	//设备应答

    CMD_Response_set_update_ftp = 0x94,     //平台设远程升级程序
    CMD_Request_set_update_ftp = 0x93,      //设备应答

};

	
//交易标识
enum {
    eUP_Start_Style_NULL 		= 0,  		//无
    eUP_Start_Style_App 		= 0x01,   	//app 启动
    eUP_Start_Style_CardOnline 	= 0x02,		//卡启动
    eUP_Start_Style_CardOffline = 0x04,		//离线卡启动
    eUP_Start_Style_VIN 		= 0x05,		// vin 码启动充电
    eUP_Start_Style_Test		= 0x06,		// 厂内测试启动
};

//停止充电失败原因
enum {
    eUP_Stop_Fail_NULL 			= 0,    	//无
    eUP_Stop_Fail_DevNumErr 	= 0x01,   	//设备编号不匹配
    eUP_Stop_Fail_NoWorking 	= 0x02,    	//枪未处于充电状态
    eUP_Stop_Fail_Other 		= 0x03, 	//其他
};

//只能用于上行上报,不能用于流程判断
enum {
    eUP_Gun_State_Offline 		= 0,      	//离线
    eUP_Gun_State_Err 			= 0x01, 	//故障
    eUP_Gun_State_Idle 			= 0x02,  	//空闲
    eUP_Gun_State_Work 			= 0x03,   	//充电
	//东电
    eUP_Gun_State_Conn 			= 0x04,		//已插枪未充电 东电
    eUP_Gun_State_Finish 		= 0x05,		//充电完成未拔枪 东电
};



enum {
	Reason_Updata 			    = 0x00,
	Reason_StopCPErro 			= 0x01,
	Reason_StopCPGnd 			= 0x02,
	Reason_StoPEGnd 			= 0x03,
	Reason_StopPhase 			= 0x04,

	Reason_StopComm 			= 0x17,
	Reason_StopLeak 			= 0x07,
	Reason_StopRelayMissTrip 	= 0x13,    //拒动
	Reason_StopRelayCgltnt		= 0x14,    //粘连

	Reason_StopEleErro       	= 0x0B,    //电表数据异常
	Reason_StopMoneyErro       	= 0x0C,    //充电中金额异常
	Reason_StartTimeout 	    = 0x22,
	Reason_StartDiode 	        = 0x25,
	Reason_StopSetKey 	        = 0x27,     //按键停止

	Reason_Finish_App 			= 0x40,
	Reason_Finish_Soc 			= 0x41,		//soc = 100%
	Reason_Finish_Ele 			= 0x42,
	Reason_Finish_Sum 			= 0x43,
	Reason_Finish_Time 			= 0x44,
	Reason_Finish_Manual 		= 0x45,
	Reason_Finish_CarOk 		= 0x46,
	
	Reason_Stop_Other 			= 0x65,		//其他原因
	
	Reason_Stop_GunBreak 		= 0x6B, 	//充电异常中止，导引断开
	Reason_Stop_BreakErr 		= 0x6C, 	//充电异常中止，断路器跳位
	Reason_Stop_MeterErr 		= 0x6D, 	//充电异常中止，电表通信中断
	Reason_Stop_SumNoEnough 	= 0x6E,     //充电异常中止，余额不足
	Reason_Stop_EmergencyStop 	= 0x72,  	//充电异常中止，急停开入
	Reason_Stop_TmpErr 			= 0x74, 	//充电异常中止，温度异常
	Reason_Stop_OverCur 		= 0x75, 	//充电异常中止，输出异常
	Reason_Stop_LittleCurr 		= 0x76, 	//充电异常中止，充电无流
	Reason_Stop_ErrVal 		    = 0x79, 	//充电异常中止，总充电电压异常
	Reason_Stop_ErrCurr 		= 0x7A, 	//充电异常中止，总充电电流异常
	
	Reason_Interupt_PwOff 		= 0x83,		//充电异常中止，充电设备断电
	
	Reason_UnKnow 				= 0x90,
	Reason_Finish 				= 0xFF,     //订单上报完成，不需要开机上报
};

//充电桩标准故障，不关任何运营平台，运营平台使用需要自动转换
enum {
	Pile_Stop_Reason_None 		    = 0x00,     //开始充电，故障码清零；未知原因--无故障; 开机之后如果有未上报订单，需要处理，判断到此故障可以当作断电异常处理
    //充电过程中充电桩桩端故障
	Pile_Stop_Reason_CPErro 		= 0x01,     //CP电压异常
	Pile_Stop_Reason_CPGnd 			= 0x02,     //CP接地停止
	Pile_Stop_Reason_PEGnd 			= 0x03,     //PE接地故障停止
	Pile_Stop_Reason_Phase 			= 0x04,     //缺相故障
	Pile_Stop_Reason_Leak 			= 0x05,     //漏电故障停止
	Pile_Stop_Reason_RlySyn 		= 0x06,     //继电器粘连
	Pile_Stop_Reason_RlyRfs 		= 0x07,     //继电器拒动，输出异常
	Pile_Stop_Reason_EStop 			= 0x08,     //急停开入故障
	Pile_Stop_Reason_PlugTempOver 	= 0x09,     //插头过温
	Pile_Stop_Reason_GunTempOver 	= 0x0A,     //枪头过温
	Pile_Stop_Reason_AirTempOver 	= 0x0B,     //环境过温
	Pile_Stop_Reason_VolOver 	    = 0x0C,     //过压
	Pile_Stop_Reason_VolUnder 	    = 0x0D,     //欠压
	Pile_Stop_Reason_CrtUnder 	    = 0x0E,     //过流
	Pile_Stop_Reason_EleCommFault 	= 0x0F,     //电表通信异常

    //启动阶段故障
	Pile_Stop_Reason_StartDiode 	= 0x20,     //二极管异常
	Pile_Stop_Reason_ShortCircle 	= 0x21,     //短路检测故障
	Pile_Stop_Reason_S2TimeOut 	    = 0x23,     //S2一定时间内不闭合，车辆拒绝，目前暂定15s

    //充电中异常保护
	Pile_Stop_Reason_Door 			= 0x30,     //门禁故障
	Pile_Stop_Reason_Ele 			= 0x31,     //电表数据异常
	Pile_Stop_Reason_Comm 			= 0x32,     //CCU通信异常
	Pile_Stop_Reason_GunBreak 		= 0x33,     //导引断开，充电中拔枪
	Pile_Stop_Reason_Money 			= 0x34,     //充电中金额异常
	// Pile_Stop_Reason_S2OverCnt 		= 0x30,     //充电过程中S2闭合断开超过一定次数

    //安全保护停止
	Pile_Stop_Reason_APP 			    = 0x40,     //APP远程停止
    Pile_Stop_Reason_CarOk              = 0x41,     //车辆充满停止，S2断开超时不闭合
	Pile_Stop_Reason_OverEle 			= 0x42,     //本次请求充电电量不足停止
	Pile_Stop_Reason_OverSum 			= 0x43,     //本次请求充电金额不足停止
	Pile_Stop_Reason_OverTime 			= 0x44,     //本次请求充电时间不足停止
	Pile_Stop_Reason_Card 			    = 0x45,     //刷卡停止
	Pile_Stop_Reason_SocFull 			= 0x46,     //soc达到，暂不使用
	Pile_Stop_Reason_OverBalance 		= 0x47,     //总账户余额不足停止
	Pile_Stop_Reason_LittleCrt 			= 0x48,     //小电流停止
	Pile_Stop_Reason_StopKey 			= 0x49,     //按键停止
	Pile_Stop_Reason_MaxTime 			= 0x50,     //超过最长时间停止

	Pile_Stop_Reason_PwOff 		        = 0x83,		//充电异常中止，充电设备断电

	Pile_Stop_Reason_Other 			    = 0x90,		//其他原因--存在故障，但未分类

	Pile_Stop_Reason_Finish 			= 0xFF,		//故障已经上报完成，如果是这个值，不需要离线上报
};


// 费率模型类型定义
enum {
    RATE_MODEL_NONE = 0,      // 无模型
    RATE_MODEL_4_TYPE = 1,    // 4类电价
    RATE_MODEL_9_TYPE = 2     // 9类电价
};

typedef struct _charge_record{
    U8 order_model_type;
    //交易记录信息，0x3F
    U8 device_number[7];
    U8 gun_num;
    U8 transaction_log_num[GNDATA_TRDNUM_LEN];
    //上传交易记录0x3F
    U8 chrg_start_time[7];         	//开始充电时间,bcd
    U8 chrg_stop_time[7];         	//结束充电时间
    U8 total_start_elec[4];       	// 总起示值
    U8 total_stop_elec[4];        	// 总止示值
    U8 total_power[4];        		// 总电量
    U8 total_loss_power[4];        	// 总计损电量
    U8 total_money[4];        		// 总金额
    U8 vin[17];          		//电动汽车唯一标识vin码,正序上传，ASCII
    U8 trade_flag;           	//交易标识 0x01：app 启动 0x02：卡启动
    U8 trade_time[7];           //交易日期，年月日时分秒
    U8 stop_reason;       		//交易停止原因
    U8 card_number[GNDATA_CARD_LEN];    //逻辑卡号，BCD，卡面印刷卡号

    U8 sharp_rate[4]; 				//尖
	U8 sharp_power[4];
	U8 sharp_loss_power[4];
	U8 sharp_money[4];

	U8 peak_rate[4]; 				//峰
	U8 peak_power[4];
	U8 peak_loss_power[4];
	U8 peak_money[4];

	U8 flat_rate[4]; 				//平
	U8 flat_power[4];
	U8 flat_loss_power[4];
	U8 flat_money[4];

	U8 valley_rate[4];				//谷
	U8 valley_power[4];
	U8 valley_loss_power[4];
	U8 valley_money[4];

    U8 deep_valley_rate[4];				//谷
	U8 deep_valley_power[4];
	U8 deep_valley_loss_power[4];
	U8 deep_valley_money[4];

    U8 six_rate[4];
    U8 six_power[4];
    U8 six_loss_power[4];
    U8 six_money[4];

    U8 seven_rate[4];
    U8 seven_power[4];
    U8 seven_loss_power[4];
    U8 seven_money[4];

    U8 eight_rate[4];
    U8 eight_power[4];
    U8 eight_loss_power[4];
    U8 eight_money[4];

    U8 nine_rate[4];
    U8 nine_power[4];  
    U8 nine_loss_power[4];
    U8 nine_money[4];
} charge_record;

//==================================================
//公牛达克云接收数据结构体
//
typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 charge_logon_result;         //充电登录认证结果 0x00：登陆成功 0x01:登陆失败
    U8 charge_logon_reason;         //充电登录认证失败原因	
}GN_Recv_Identification;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;         				//终端号验证
    U8 ack;         				//心跳应答	
}GN_Recv_Heart;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 billing_model[2];        	//计费模型编号，首次链接到平台时置零
    U8 billing_model_result;     	//0一致，1不一致
}GN_Recv_Billing_Verify;

typedef struct
{
	U8 rateModelType;
    U8 device_number[DEV_NUM_LEN];
    U8 billing_model[2];        	//计费模型编号，首次链接到平台时置零
    U8 measure_wastage_rates;       //计损比率
    U8 segmentation_rate[48];      	//一天分成48时间段，半个小时一段

    U8 sharp_ele_fee[4];        	//尖
    U8 sharp_ser_fee[4];        	//
    U8 peak_ele_fee[4];        		//峰
    U8 peak_ser_fee[4];        		//
	U8 flat_ele_fee[4];        		//平
    U8 flat_ser_fee[4];        		//
	U8 valley_ele_fee[4];        	//谷
    U8 valley_ser_fee[4];        	//

    U8 deep_valley_ele_fee[4];      //深谷
    U8 deep_valley_ser_fee[4];      //

    U8 six_ele_fee[4];              //第六
    U8 six_ser_fee[4];
    U8 seven_ele_fee[4];            //第七
    U8 seven_ser_fee[4];
    U8 eight_ele_fee[4];            //第八
    U8 eight_ser_fee[4];
    U8 nine_ele_fee[4];             //第九
    U8 nine_ser_fee[4];
}GN_Recv_Billing_Model;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;
}GN_Recv_RealTime_Rsq;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;
    U8 transaction_log_num[16];     //交易流水号，设备号（7bytes） +枪号（1byte） +年月日时分秒（6bytes） +自增序号（2bytes）
    U8 Logic_card_number[8];        //逻辑卡号，BCD，卡面印刷卡号
    U8 account_balance[4];        	//账户余额，2位小数
    U8 Auth_success;				//鉴权充电成功标志，0失败，1成功
    U8 Auth_Fail_Reason;			//鉴权启动充电失败原因	BCD 码
}GN_Recv_Auth_Ack;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;
    U8 transaction_log_num[16];     //交易流水号，设备号（7bytes） +枪号（1byte） +年月日时分秒（6bytes） +自增序号（2bytes）
    U8 Logic_card_number[8];        //逻辑卡号，BCD，卡面印刷卡号
    U8 account_balance[4];        	//账户余额，2位小数
}GN_Recv_Start_Charge;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;
}GN_Recv_Stop_Charge;

typedef struct
{
    U8 transaction_log_num[16];     //交易流水号，设备号（7bytes） +枪号（1byte） +年月日时分秒（6bytes） +自增序号（2bytes）
    U8 result;						//0x00 上传成功 0x01 非法账单
}GN_Recv_Record_Ack;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;						//
    U8 Logic_card_number[8];        //逻辑卡号，BCD，卡面印刷卡号
    U8 account_balance[4];        	//账户余额，2位小数
}GN_Recv_Sum_Update;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 chrg_device_forbid_use;        //设备是否停止使用，0正常，1不允许使用
    U8 chrg_device_max_rate;          //充电设备最大允许输出功率，1BIN 表示 1%，最大 100%，最小 30%
}GN_Recv_Set_Param;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 device_current_time[7];        //设置当前时间
}GN_Recv_Set_Time;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 platform_set_reboot;          //远程设置重启，1，立即执行，2空闲执行
}GN_Recv_Reboot;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
	U8 device_type;					//0x01：直流 0x02：交流
	U8 device_rate[2];				//设备功率
    U8 update_ip[16];               //升级服务器ip,ascii
    U8 update_port[2];              //设备功率
    U8 update_username[16];     	//用户名ascii
    U8 update_password[16];   		//密码ascii
    U8 update_file_path[32];      	//文件路径ascii
    U8 update_ctrl;               	//1升级立即执行，2空闲执行
    U8 update_timeout;           	//超时时间，min
}GN_Recv_Update_ftp;



typedef struct
{
	GN_Recv_Identification 	strRecvIdenf;			//应答-登录认证
	GN_Recv_Heart 			strRecvHeart;			//应答-心跳
	GN_Recv_Billing_Verify 	strRecvBillingVer;		//应答-计费模型版本
	GN_Recv_Billing_Model 	strRecvBillingModel;
	GN_Recv_RealTime_Rsq 	strRecvRealTimeRsq;
	GN_Recv_Auth_Ack 		strRecvAuthAck;
	GN_Recv_Start_Charge	strRecvStartCharge;
	GN_Recv_Stop_Charge 	strRecvStopCharge;
	GN_Recv_Record_Ack 		strRecvRecordAck;
	GN_Recv_Sum_Update 		strRecvSumUpdate;
	GN_Recv_Set_Param 		strRecvSetParam;
	GN_Recv_Set_Time 		strRecvSetTime;
	GN_Recv_Reboot 			strRecvReboot;
	GN_Recv_Update_ftp 		strRecvFtp;
}RECV_Data;


//====================================================================
//启动充电失败原因
enum {
    eUP_Start_Fail_NULL 		= 0,     	//无
    eUP_Start_Fail_DevNumErr 	= 0x01,     //设备编号不匹配
    eUP_Start_Fail_Working 		= 0x02,    	//枪已在充电
    eUP_Start_Fail_DevErr 		= 0x03, 	//设备故障
    eUP_Start_Fail_Offline 		= 0x04,		//设备离线
    eUP_Start_Fail_NoConn 		= 0x05,		//未插枪
    eUP_Start_Fail_Reconnect 	= 0x06,		//超时不可启动，需要重新插拔枪
    eUP_Start_Fail_Rate 	    = 0x07,     //计费异常
};

typedef enum
{
    eChgType_Auto              	=  0,      	// 自动启动
    eChgType_Ennergy           	=  1,      	// 按电量
    eChgType_Time              	=  2,      	// 按时间
    eChgType_Money             	=  3,      	// 按金额
}CHG_TYPE_E;



void ClearGNUpCnt(void);

void GN_CardAuthStart_Cmd(uint8_t u8Port);
void GN_DealUpdate_Cmd(uint8_t u8Port);

#endif
