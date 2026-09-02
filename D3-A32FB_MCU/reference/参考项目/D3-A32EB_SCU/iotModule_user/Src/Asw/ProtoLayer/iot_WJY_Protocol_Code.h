#ifndef __PROTOCOL_CODEC_WJY_H__
#define __PROTOCOL_CODEC_WJY_H__

#include "protocol_ctrl.h"
#include "tcp_gn.h"
#include "iot_GN_Protocol_Code.h"


#define WJY_FRAME_HEAD	        0x68	
#define WJY_PILE_LEN             monitor_getDevNumberlength_wjy()
#define WJY_HEAD_LEN            (8+WJY_PILE_LEN)//消息头长度(不含桩号)

#define WJY_PROTOCOL_ENCRYPT    1   //加密
#define WJY_PROTOCOL_NOENCRYPT  0   //不加密

#define Wjy_Reboot_SUCC       0;  //远程重启应答标志位 默认为1 操作失败
#define Wjy_Reboot_FAIL       1;  //远程重启应答标志位 默认为1 操作失败



enum {
    WJY_S_Aes_Key = 0x00,   	    //请求密钥
    WJY_R_Aes_Key = 0x80,   	    //请求密钥应答
    WJY_R_Aes_Err = 0x25,           //解密出错

    WJY_S_Identification = 0x01,   	//登录认证
    WJY_R_Identification = 0x81,   	//登录认证应答
    WJY_S_Auth = 0x04,    			//申请启动充电-鉴权
    WJY_R_Auth = 0x84,   			//平台确认启动充电

    WJY_S_Start_Chg_Ack = 0x86,  	//开始充电应答
    WJY_R_Start_Chg     = 0x06,     //平台启动充电    
    WJY_S_Stop_Chg_Ack  = 0x87,  	//停止充电应答
    WJY_R_Stop_Chg      = 0x07,     //平台停止充电

    WJY_S_Chg_Record = 0x08,     	//交易记录上传
    WJY_R_Chg_Record = 0x88,     	//平台交易记录确认
    WJY_S_RealData = 0x09,        	//枪状态定时/变更上报
    WJY_S_Rate_Ask = 0x8B,      	//计费规则设置应答   
    WJY_R_Rate     = 0x0B,     		//计费规则设置

    WJY_S_Heart = 0x0C,           	//心跳包
    WJY_R_Heart = 0x8C,           	//心跳应答
    WJY_S_Device_Fault = 0x0D,      //设备故障上送
    WJY_R_Device_Fault = 0x8D,      //设备故障上送回复确认   
	WJY_S_TimeSyn_ACK = 0x8E,		//时钟同步应答
	WJY_R_TimeSyn     = 0x0E,		//时钟同步	

	WJY_S_QR = 0x1C,			    //设置二维码
 	WJY_R_QR = 0x9C,			    //设置二维码应答   

    WJY_S_update_ACK = 0x9E,    	//远程升级应答	
    WJY_R_update_ftp = 0x1E,	    //平台设远程升级程序
    WJY_S_update_rst = 0x20,        //升级结果汇报
    WJY_R_update_rst = 0xA0,	    //升级结果汇报应答
    WJY_S_reboot_ACK = 0xA1,      	//远程重启应答
    WJY_R_set_reboot = 0x21,     	//平台设远程重启
        
    WJY_S_Sum = 0x26,        	//查询用户余额
    WJY_R_Sum = 0xA6,      	//查询用户余额应答
		
};

//wjy启动充电失败原因
typedef enum {
    WJY_Start_Fail_NULL 		= 0,     	//启动成功
    WJY_Start_Fail_DevNumErr    = 0x01,     //上传枪被预约，实际是设备编号错误
    WJY_Start_Fail_NoConn 		= 0x02,		//未插枪

    WJY_Start_Fail_Working 		= 0x03,    	//枪已在充电
    WJY_Start_Fail_GunErr 		= 0x04,		//上传枪故障，实际是需要重新插拔枪

    WJY_Start_Fail_DevErr 		= 0x05, 	//桩故障
    WJY_Start_Fail_Offline 		= 0x06,		//上传bms通讯故障，实际是设备离线
    WJY_Start_Fail_ErrRate		= 0x09,		//上传其他原因，实际是计费模型错误
}PLAT_FAIL_REASON;

//只能用于上行上报,不能用于流程判断
enum {
    WJY_Gun_State_Idle 		    = 0,      	//空闲
    WJY_Gun_State_Conn 			= 0x01,		//已插枪未充电
    WJY_Gun_State_Work 			= 0x02,   	//充电
    WJY_Gun_State_Finish 		= 0x03,		//充电完成未拔枪
    WJY_Gun_State_Err 			= 0x05, 	//故障
};

typedef struct
{
    uint8_t             Year;  		// 年 
    uint8_t             Month;  	// 月
    uint8_t             Day;  		// 日
    uint8_t             Hour;  		// 时
    uint8_t             Minute;  	// 分
    uint8_t             Second;		// 秒
}cp56time2a_wjy;




enum {
	Wjy_Reason_UnKnow 			    = 0,

	Wjy_Reason_Finish_Soc 			= 1,
	Wjy_Reason_Finish_Sum 			= 2,
	Wjy_Reason_Finish_Time 			= 3,
	Wjy_Reason_Finish_Ele 			= 4,

	Wjy_Reason_Stop_SumNoEnough 	= 5,        //充电异常中止，余额不足
	Wjy_Reason_Finish_App 			= 6,
	Wjy_Reason_Finish_Manual 		= 7,
	Wjy_Reason_Stop_GunBreak 		= 8, 	    //充电异常中止，导引断开
	Wjy_Reason_Stop_EmergencyStop 	= 9,  	    //充电异常中止，急停开入
	Wjy_Reason_StopLeak 			= 10,       //设备故障，漏电
	Wjy_Reason_StopComm 			= 11,       //网络故障 ，充电单元通信故障

	Wjy_Reason_Stop_GunTmpErr 		= 23, 	//枪温过高
	Wjy_Reason_Stop_PlugTmpErr 		= 34, 	//插头过温
	Wjy_Reason_Stop_TmpErr 		    = 37, 	//环境过温
    Wjy_Reason_StopCPErro           = 39,   //cp电压异常



	Wjy_Reason_Stop_ErrCurr 		= 41, 	//充电异常中止，总充电电流异常
	Wjy_Reason_Stop_ErrVal 		    = 42, 	//充电异常中止，总充电电压异常
	Wjy_Reason_Stop_MeterErr 		= 43, 	//充电异常中止，电表通信中断
	
    Wjy_Reason_StopEleErro 			= 51,
	Wjy_Reason_StopMoneyErro       	= 52,    //充电中金额异常
	Wjy_Reason_Stop_BreakErr 		= 56, 	//充电异常中止，断路器跳位
	Wjy_Reason_StopRelayMissTrip 	= 57,    //拒动
	Wjy_Reason_StopRelayCgltnt		= 58,    //粘连
 

    //平台没有的停止原因在71后面新增
    Wjy_Reason_StopCPGnd            = 72,
    Wjy_Reason_StoPEGnd             = 73,
	Wjy_Reason_StartDiode 	        = 74,
	Wjy_Reason_StartTimeout 	    = 75,

	Wjy_Reason_Interupt_PwOff 		= 76,		//充电异常中止，充电设备断电
	Wjy_Reason_StopSetKey 	        = 77,     //按键停止
    Wjy_Reason_Stop_TimeOut         = 78,


	Wjy_Reason_Finish_CarOk 		= 0x46,
	
	Wjy_Reason_Stop_Other 			= 0x65,		//其他原因
	
	Wjy_Reason_Stop_OverCur 		= 0x75, 	//充电异常中止，输出异常
	Wjy_Reason_Stop_LittleCurr 		= 0x76, 	//充电异常中止，充电无流
	
	
	Wjy_Reason_Finish 				= 0xFF,     //订单上报完成，不需要开机上报
};

typedef struct
{
    uint8_t upResult;                   //升级上报状态结果，0-成功，2-拒绝

    uint8_t Rate_Id[4];                 //计费规则id  上传充电记录时使用，充电开始记录，订单上报完成更新
    uint8_t Rate_Ver[4];                //计费规则版本号


    uint8_t SendSumFlag[GUN_NUM_MAX];   // 余额更新使能
    uint8_t SendSumTime[GUN_NUM_MAX];   // 余额更新上报次数

    uint8_t Wjy_Reboot_flag;            //重启标志
    uint8_t Wjy_Reboot_result;          //重启结果
    uint32_t reboot_tick;

} WJY_UpPlatInfo;

typedef struct
{
    uint8_t		PileStopReason;
	uint8_t		WjyStopReason;
}Pile_WjyStopReasonMap;

typedef struct
{
	U8 gun;						// gun
	U16 WarnId;					// warning id 
    U8 WarnPlatId[2];
	U8 StartTime[6];			// start time
	U8 StopTime[6];				// stop time
	U8 status;					// warning status, 0-normal, 1-warning
}WJY_Send_ErrSend;

typedef struct _WJY_charge_record{
    //交易记录信息，0x94 
    U8 gun_num;
    U8 transaction_log_num[16];     //订单号ascii 与启动命令(0x06，0x04)一致
    U8 trade_flag;           	    //交易标识 0x01：卡 启动 0x02：app启动
    U8 card_number[16];             //卡号ascii 与启动命令(0x04)一致 非卡充电置空
    U8 vin[17];          		    //电动汽车唯一标识vin码,正序上传，ASCII
    U8 start_soc;          		    //开始充电SOC 
    U8 stop_soc;          		    //结束充电SOC
    U8 stop_reason;                 //结束原因
    U8 chrg_start_time[6];         	//开始充电时间,bcd
    U8 chrg_stop_time[6];         	//结束充电时间
    U8 data_bit;         	        //数据精度位，0x04表示以下订单数据都为为4位小数 

	U8 total_start_elec[4];       	//总起示值
    U8 total_stop_elec[4];        	//总止示值
    U8 total_power[4];        		//总电量    
	U8 sharp_power[4];              //尖电量
	U8 peak_power[4];               //峰电量
	U8 flat_power[4];               //平电量
	U8 valley_power[4];             //谷电量

	U8 chg_money[4];                //充电费总金额
	U8 serve_money[4];              //服务费总金额
	U8 order_money[4];              //订单费总金额
	U8 park_money[4];               //停车费总金额
    U8 time_power[48][4];           //48时段电量 单位kwh，精度4位小数
    uint8_t rate_id[4];             //计费规则ID
    uint8_t rate_ver[4];            //计费规则版本号
}WJY_charge_record;

typedef struct
{
    uint8_t head;
    uint8_t cmd;	
    uint8_t ser[2];     
    uint8_t dev_len;        //设备编号长度
    uint8_t dev_variable[PLAT_NUMBER_LEN+1+2];    //设备编号（桩号,最多32位可变位 ）+EncType（是否加密）+len[2]（消息体长度）  
    // uint8_t EncType;	    //0x00-数据不加密，0x01-AES加密
    // uint8_t len[2];         //消息体长度
}WJY_HEAD_T;

typedef struct
{
    uint8_t aes_len;
    uint8_t aes_key[32];
}WJY_Recv_Aes;

typedef struct
{
    uint8_t aes_len;
    uint8_t aes_key[32];
}WJY_Recv_Aes_Err;

typedef struct
{
    uint8_t result;  //0x00-成功，0x01-失败
}WJY_Recv_Identification;

typedef struct
{
    uint8_t card_num[32];
    uint8_t account_balance[4];        	//账户余额，2位小数
	uint8_t transaction_log_num[16]; 	//订单号
    uint8_t Auth_success;				//鉴权充电成功标志，0成功
}WJY_Recv_Auth_Ack;

enum{
    E_WJY_GunState_Auto = 1,
    E_WJY_GunState_Money,
    E_WJY_GunState_Time,
    E_WJY_GunState_Ele,
};

typedef struct
{
    uint8_t gun;                    //枪号
    uint8_t chg_type;               //充电策略  1: 自动充满 2: 按金额充 3: 按时间充 4: 按电量充
    uint8_t type_data[4];           //充电策略数据  1: 填0 2: 元/2位小数 3: 时间长度/单位秒 4: 度/2位小数
    uint8_t stop_num[3];            //充电停止码
    uint8_t transaction_num[16];    //订单号
    uint8_t account_balance[4];     //用户余额 元/2位小数
}WJY_Recv_Start_Charge;

typedef struct
{
    uint8_t gun;
    uint8_t transaction_num[16];    //订单号
}WJY_Recv_Stop_Charge;

typedef struct
{
    uint8_t transaction_num[16];    //订单号
    uint8_t result;                 //0x00-成功，0x01-失败
}WJY_Recv_Record_Ack;

typedef struct
{
    //102字节
    uint8_t rate_id[4];             //计费规则ID
    uint8_t rate_ver[4];            //计费规则版本号
    uint8_t start_time[6];          //格式：YYMMDDHHMMSS
    uint8_t order_fee[4];           //预约费单价
    uint8_t park_fee[4];            //停车费单价
	uint8_t sharp_ele_fee[4];       //尖电费
    uint8_t peak_ele_fee[4];        //峰电费
	uint8_t flat_ele_fee[4];        //平电费
	uint8_t valley_ele_fee[4];      //谷电费
    uint8_t sharp_ser_fee[4];       //尖服务费
    uint8_t peak_ser_fee[4];        //峰服务费
    uint8_t flat_ser_fee[4];        //平服务费
    uint8_t valley_ser_fee[4];      //谷服务费
    uint8_t segmentation_rate[48];  //一天分成48时间段，当前时段的类型1尖 2峰 3平 4谷
}WJY_Recv_Rate_Model;

typedef struct
{
    uint8_t sever_time[6];          //YYMMDDHHMMSS，服务器时间 可根据此校时
}WJY_Recv_Heart;

// typedef struct 告警回复无消息体
// {
//     uint8_t     result;
// }WJY_Recv_Fault;

typedef struct
{
    uint8_t sever_time[6];          //YYMMDDHHMMSS，服务器时间 可根据此校时
}WJY_Recv_TimeSyn;

typedef struct
{
    uint8_t gun;
    uint8_t QR_len[2];              //二维码内容长度
	uint8_t QR_data[QR_MAX_SIZE];   //二维码内容
}WJY_Recv_QR;

typedef struct
{
    uint8_t soft_ver[16];           //软件版本
	uint8_t plat_ver[8];			//通信协议版本
	uint8_t md5[32];				//升级文件的MD5，用于校验文件是否完整
	uint8_t path_len[2];		    //升级文件下载地址长度
    uint8_t file_path[64];          //升级文件下载地址
    uint8_t username_len;     	    //用户名长度
    uint8_t username[16];     	    //用户名ascii
    uint8_t password_len;   		//密码长度
    uint8_t password[16];   		//密码ascii
}WJY_Recv_Update_ftp;


typedef struct
{
    uint8_t ctrl_type;              //控制类型
    uint8_t reserved[4];            //预留
}WJY_Recv_Reboot;

typedef struct
{
    uint8_t transaction_num[16];    //订单号
    uint8_t account_balance[4];     //余额
}WJY_Recv_SumUpdata;


typedef struct
{
	WJY_Recv_Aes                strRecvAes;
    WJY_Recv_Identification 	strRecvIdenf;			//应答-登录认证
	WJY_Recv_Auth_Ack 			strRecvAuthAck;
	WJY_Recv_Start_Charge		strRecvStartCharge;
	WJY_Recv_Stop_Charge 		strRecvStopCharge;
	WJY_Recv_Record_Ack 		strRecvRecordAck;
	WJY_Recv_Rate_Model 		strRecvRateModel;
	WJY_Recv_Heart 				strRecvHeart;			//应答-心跳
	// WJY_Recv_Fault 		        strRecvFault;
	WJY_Recv_TimeSyn 			strRecvTimeSyn;	
	WJY_Recv_QR 				strRecvQR;
	WJY_Recv_Update_ftp 		strRecvUpdata;
	WJY_Recv_Reboot 			strRecvReboot;
	WJY_Recv_SumUpdata 			strRecvSumUpdata;
}WJY_RECV_Data;




void WJYUpProtocolDeal(void);
void WJY_CardAuthStart_Cmd(uint8_t u8Port);
void wjy_packChgRecord(uint8_t u8Port, WJY_charge_record *pRecord);
void WJY_DealUpdate_Cmd(uint8_t u8Port);


#endif
