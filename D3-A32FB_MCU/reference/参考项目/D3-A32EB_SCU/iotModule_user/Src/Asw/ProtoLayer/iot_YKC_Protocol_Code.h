#ifndef __PROTOCOL_CODEC_YKC_H__
#define __PROTOCOL_CODEC_YKC_H__

#include "protocol_ctrl.h"
#include "tcp_gn.h"

#define DD_PROTOCOL_VER				14				//东电
#define YKC_PROTOCOL_VER			16				//
#define TT_PROTOCOL_VER				17				//铁塔

#define YKC_FRAME_HEAD				0x68			//

enum {
    YKC_S_Identification = 0x01,   	//登录认证
    YKC_R_Identification = 0x02,   	//认证应答
    YKC_S_Heart = 0x03,           	//心跳包
    YKC_R_Heart = 0x04,           	//心跳应答
    YKC_S_Rate_Proving = 0x05,      //计费模型验证请求
    YKC_R_Rate_Proving = 0x06,     	//计费模型验证请求应答
    YKC_S_Rate_Ask = 0x09,      	//计费模型请求
    YKC_R_Rate_Ask = 0x0A,     		//计费模型请求应答
    
    YKC_R_RealData = 0x12,    		//平台读取实时检测数据
    YKC_S_RealData = 0x13,        	//上传实时检测数据
	
    YKC_S_Auth = 0x31,    			//申请启动充电-鉴权
    YKC_R_Auth = 0x32,   			//平台确认启动充电
    
    YKC_S_Start_Chg_Ack = 0x33,  	//开始充电回复
    YKC_R_Start_Chg = 0x34,        	//平台启动充电
    
    YKC_S_Stop_Chg_Ack = 0x35,  	//停止充电回复
    YKC_R_Stop_Chg = 0x36,        	//平台停止充电
	
    YKC_S_Chg_Record = 0x3B,     	//交易记录上传
    YKC_S_Chg_Record_DD = 0x3F,     //交易记录上传 东电
    YKC_R_Chg_Record = 0x40,     	//平台交易记录确认
    
    YKC_S_Sum_ACK = 0x41,        	//余额更新应答
    YKC_R_Sum_Update = 0x42,      	//账户余额更新
	
	YKC_S_Para_ACK = 0x51,			//充电桩工作参数设置应答
	YKC_R_Set_Para = 0x52, 			//充电桩工作参数设置
	
	YKC_S_TimeSyn_ACK = 0x55,		//对时设置应答
	YKC_R_TimeSyn = 0x56,			//对时设置
	
	YKC_S_Rate_ACK = 0x57,			//计费模型应答
	YKC_R_Set_Rate = 0x58,			//计费模型设置

	YKC_R_Ret_QR = 0x5A,			//设置二维码
	YKC_S_QR_ACK = 0x59,			//设置二维码应答
	
    YKC_R_set_reboot = 0x92,     	//平台设远程重启
    YKC_S_reboot_ACK = 0x91,      	//设备应答

    YKC_R_set_update_ftp = 0x94,	//平台设远程升级程序
    YKC_S_update_ACK = 0x93,    	//设备应答

	YKC_R_Set_QR_DD = 0x9C,			//设置二维码 东电
	YKC_S_QR_ACK_DD = 0x9B,			//设置二维码应答 东电
	
};

//启动充电失败原因
enum {
    eStart_Fail_NULL 			= 0,     	//无
    eStart_Fail_DevNumErr 		= 0x01,     //设备编号不匹配
    eStart_Fail_Working 		= 0x02,    	//枪已在充电
    eStart_Fail_DevErr 			= 0x03, 	//设备故障
    eStart_Fail_Offline 		= 0x04,		//设备离线
    eStart_Fail_NoConn 			= 0x05,		//未插枪
    eStart_Fail_ErrRate,					//计费模型错误
};

typedef struct
{
    uint8_t     head;
    uint8_t     len;
    uint8_t     ser[2];
    uint8_t     EncType;		//0x00-数据不加密，0x01-加密
    uint8_t     cmd;	
}YKC_HEAD_T;


typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t charge_login_result;         //充电登录认证结果 0x00：登陆成功 0x01:登陆失败
}YKC_Recv_Identification;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         //
    uint8_t ack;         //
}YKC_Recv_Heart;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t Rate_num[2];         //
    uint8_t result;         //0x00 桩计费模型与平台一致 0x01 桩计费模型与平台不一致
}YKC_Recv_Rate_Proving;

//公牛 云快充
typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 billing_model[2];        	//计费模型编号，首次链接到平台时置零
    
	U8 sharp_ele_fee[4];        	//尖
    U8 sharp_ser_fee[4];        	//
    U8 peak_ele_fee[4];        		//峰
    U8 peak_ser_fee[4];        		//
	U8 flat_ele_fee[4];        		//平
    U8 flat_ser_fee[4];        		//
	U8 valley_ele_fee[4];        	//谷
    U8 valley_ser_fee[4];        	//
    U8 measure_wastage_rates;       //计损比率
    U8 segmentation_rate[48];      	//一天分成48时间段，半个小时一段
}YKC_Recv_Rate_Model;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
}YKC_Recv_RealData_Rsq;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Logic_card_number[8];    	//逻辑卡号，BCD，卡面印刷卡号
    uint8_t account_balance[4];        	//账户余额，2位小数
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
}YKC_Recv_Auth_Ack;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Logic_card_number[8];    	//逻辑卡号，BCD，卡面印刷卡号
    uint8_t Physics_card_number[8];    	//物理卡号，BCD，卡面印刷卡号
    uint8_t account_balance[4];        	//账户余额，2位小数

}YKC_Recv_Start_Charge;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//

}YKC_Recv_Stop_Charge;

typedef struct
{
	uint8_t transaction_log_num[16]; 	//交易流水号，设备号（7bytes） +枪号（1byte）
    uint8_t result;         		//

}YKC_Recv_Record_Ack;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t gun;         		//
    uint8_t Physics_card_number[8];    	//物理卡号，BCD，卡面印刷卡号
    uint8_t account_balance[4];        	//账户余额，2位小数
}YKC_Recv_SumUpdata;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t service_flag;    	//0x00 表示允许正常工作 0x01 表示停止使用，锁定充电桩
    uint8_t power_per;        	//允许输出功率 1BIN 表示 1%，最大 100%，最小30%
}YKC_Recv_Para;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    cp56time2a cur_time;
}YKC_Recv_TimeSyn;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
    uint8_t platform_set_reboot;      	//0x01：立即执行 0x02：空闲执行
}YKC_Recv_Reboot;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
	uint8_t device_type;					//0x01：直流 0x02：交流
	uint8_t device_rate[2];				//设备功率
    uint8_t update_ip[16];               //升级服务器ip,ascii
    uint8_t update_port[2];              //设备功率
    uint8_t update_username[16];     	//用户名ascii
    uint8_t update_password[16];   		//密码ascii
    uint8_t update_file_path[32];      	//文件路径ascii
    uint8_t update_ctrl;               	//1升级立即执行，2空闲执行
    uint8_t update_timeout;           	//超时时间，min
}YKC_Recv_Update_ftp;

typedef struct
{
	uint8_t device_number[DEV_NUM_LEN];
	uint8_t gun;							//
	uint8_t QR_data[QR_MAX_SIZE];				//二维码

}YKC_Recv_QR;

typedef struct
{
	uint8_t gun;							//
	uint8_t QR_len[2];				//二维码长度
	uint8_t QR_data[QR_MAX_SIZE];				//二维码
}YKC_Recv_QR_DD;

typedef struct
{
	YKC_Recv_Identification 	strRecvIdenf;			//应答-登录认证
	YKC_Recv_Heart 				strRecvHeart;			//应答-心跳
	YKC_Recv_Rate_Proving 		strRecvRateProving;		//应答-计费模型版本
	YKC_Recv_Rate_Model 		strRecvRateModel;
	YKC_Recv_RealData_Rsq 		strRecvRealDataRsq;
	YKC_Recv_Auth_Ack 			strRecvAuthAck;
	YKC_Recv_Start_Charge		strRecvStartCharge;
	YKC_Recv_Stop_Charge 		strRecvStopCharge;
	YKC_Recv_Record_Ack 		strRecvRecordAck;
	YKC_Recv_SumUpdata 			strRecvSumUpdata;
	YKC_Recv_Para 				strRecvPara;
	YKC_Recv_TimeSyn 			strRecvTimeSyn;	
	YKC_Recv_Reboot 			strRecvReboot;
	YKC_Recv_Update_ftp 		strRecvUpdata;
	YKC_Recv_QR 				strRecvQR;
	YKC_Recv_QR_DD 				strRecvQRDD;
}YKC_RECV_Data;

void YKCUpProtocolDeal(void);

void YKC_CardAuthStart_Cmd(uint8_t u8Port);
void YKC_DealUpdate_Cmd(uint8_t u8Port);

#endif

