#ifndef __IOT_MONITOR_CODE_H__
#define __IOT_MONITOR_CODE_H__
#ifndef __TCPDATA_OM_H__
#define __TCPDATA_OM_H__

#include "globals.h"
#include "AppHeaderSummary.h"
// #include "protocol_data.h"

#define OM_DEVICE_TYPE    		1          	//0直流桩，1交流桩，2电瓶车桩
#define OM_GUN_TYPE        	2        	//1一代枪头，2二代枪头（12v灯亮）

#define OM_DEV_NUM_LEN				32				//

//公牛运维协议tcp头
#define OM_TCP_HEAD_1      0xEB
#define OM_TCP_HEAD_2      0xBE

#define GNDATA_PHYCARD_LEN   	    4      	// 物理卡号长度
#define GNDATA_CARD_LEN   			8      	// 公牛卡号长度
#define GNDATA_TRDNUM_LEN   		16    	// 公牛流水号长度


#define OM_UP_RESULT_FAIL 			0
#define OM_UP_RESULT_SUCC 			1

#define OM_LOCKDOWN_FAIL            1       //锁机/解锁失败
#define OM_LOCKDOWN_SUCC            2       //锁机/解锁成功

#define OM_LOCK_STATE_SUCC          1       //已锁机
#define OM_LOCK_STATE_FAIL          2       //已锁机

#define OM_TCP_DATA_LEN_MAX    	256    		//tcp协议包数据长度max
#define QR_MAX_SIZE				200			//二维码最大长度

//只能用于上行上报,不能用于流程判断
enum {
    eUP_OM_Gun_State_Idle 		    = 0,      	//空闲
    eUP_OM_Gun_State_Start 		    = 0x01, 	//启动充电
    eUP_OM_Gun_State_Work 			= 0x02,  	//充电中
    eUP_OM_Gun_State_Finish 		= 0x03,   	//结束
    eUP_OM_Gun_State_Err 			= 0x04, 	//故障
};


enum {
    OM_CMD_Request_Identification = 0x01,          //登录认证上报
    OM_CMD_Response_Identification = 0x02,         //登录认证应答
    OM_CMD_Request_Heart = 0x03,                   //心跳包上报
    OM_CMD_Response_Heart = 0x04,                  //心跳包应答
    OM_CMD_Request_Network_module_info = 0x05,     //网络模块信息上报
    OM_CMD_Response_Network_module_info = 0x06,    //网络模块信息获取

    OM_CMD_Request_realTime_gun = 0x13,            //实时监测数据上报
    OM_CMD_Response_realTime_gun = 0x12,         //实时监测数据读取

    OM_CMD_Request_Meter_base_number = 0x15,       //电表底数上报

    OM_CMD_Request_order_info = 0x17,              //订单信息上报
    OM_CMD_Response_order_info = 0x16,             //订单上报应答

    OM_CMD_Request_set_device_param = 0x51,    	   //充电设备工作参数设置应答
    OM_CMD_Response_set_device_param = 0x52,       //充电设备工作参数设置

    OM_CMD_Request_Qrcode_update = 0x59,           //二维码更新应答
    OM_CMD_Response_Qrcode_update = 0x5A,          //二维码更新
   
    OM_CMD_Request_set_reboot = 0x91,              //远程重启应答
    OM_CMD_Response_set_reboot = 0x92,     	       //远程重启

    OM_CMD_Request_set_update_ftp_ = 0x93,         //远程更新应答   
    OM_CMD_Response_set_update_ftp = 0x94,         //远程更新

    OM_CMD_Request_Lock_machine = 0x95,            //锁机应答
    OM_CMD_Response_Lock_machine = 0x96,           //设备锁机/解锁

    OM_CMD_Request_Lock_state = 0x97,              //本地锁机状态上传
    OM_CMD_Response_Lock_state = 0x98,             //本地锁机应答

    OM_CMD_Response_Log_read = 0xB0,               //日志读取
    OM_CMD_Request_Log_read = 0xB1,                //日志读取应答
};


typedef struct
{
    uint8_t     head[2];
    uint8_t     ver[2];
    uint8_t     ser[2];
    uint8_t     EncType;		//0x00-数据不加密，0x01-加密
    uint8_t     cmd;	
    uint8_t     len[2];
}OM_GN_HEAD_T;



//6.1充电设备登录认证
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 pileNumber[32];                  //平台桩编号
    U8 device_type;                     //设备类型
    U8 terminals_num;                   //终端数量
    U8 program_version[4];              //程序版本 版本为1.00.00 表示0x00,0x01,0x00,0x00
    U8 A_Hardware_version[4];           //充电模块硬件版本
    U8 A_Software_version[4];           //充电模块软件版本
    U8 B_Hardware_version[4];           //灯板模块硬件版本
    U8 B_Software_version[4];           //灯板模块软件版本
    U8 C_Hardware_version[4];           //网络模块硬件版本
    U8 C_Software_version[4];           //网络模块软件版本
    U8 Signature_value[16];
}OM_GN_Send_Identfication;


//6.2登录认证应答
//登录认证失败原因
enum {
    login_auth_Fail_NULL 		= 0,     	   //无
    login_auth_Fail_Overlap     = 0x01,        //设备编码重叠
    login_auth_Fail_KeyErr      = 0x02,        //验证密钥错误
    login_auth_Fail_TypeErr     = 0x03,        //设备类型错误
    login_auth_Fail_ExistErr    = 0x04,        //设备不存在
    login_auth_Fail_Other       = 0x05,        //其他原因
};
typedef struct
{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 charge_logon_result;             //充电登录认证结果 0x00：登陆成功 0x01:登陆失败
    U8 charge_logon_reason;             //充电登录认证失败原因	
}OM_GN_Recv_Identification;


//6.3充电设备心跳包
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_no;         				    //终端号验证
    U8 device_state;                    //设备状态 0x00：正常 0x01：故障
}OM_GN_Send_Heart;


//6.4心跳包应答
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 ack;         				    //心跳应答	
}OM_GN_Recv_Heart;


//6.5获取网络模块信息
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 ack;         				    //心跳应答	
}OM_GN_Recv_Network_Module_Info;

//6.6网络模块信息上传
//网络链接类型
typedef enum{
    link_type_SIM           = 0,           
    link_type_LAN           = 0x01,        
    link_type_WAN           = 0x02,        
    link_type_LORA          = 0x03,        
    link_type_OTHER         = 0xFF,        
}SIMtype;
//运营商
typedef enum{
    operator_mobile         = 0,            //移动
    operator_telecom        = 0x01,         //电信
    operator_unicom        = 0x02,         //联通
    operator_other         = 0xFF,         //其他
}Operator;

typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 link_type;     	                   //网络链接类型
    U8 operator;                           //运营商
    U8 SIM_num[20];                        //SIM卡卡号
    U8 network_module_model[20];           //网络模块型号
    U8 mac_address[20];                    //mac地址
}OM_GN_Send_Network_Module_Info;


//7.1读取实时监测数据
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_num;                      //0：A枪，1：B枪
}OM_GN_Recv_RealTime_Gun;


//7.2上传实时监测数据
//充电状态 需做到变位上送
typedef enum{
    Gun_State_Charge_free        = 0,           //空闲
    Gun_State_Charge_strat       = 0x01,        //启动充电
    Gun_State_Charging           = 0x02,        //充电中
    Gun_State_Charge_End         = 0x03,        //充电结束
    Gun_State_Charge_Fault       = 0x04,        //充电故障
}ChargingStatus;
//硬件故障
enum{
    OM_Hardware_Other 				= 0, 		//其他故障
    OM_Hardware_Stop, 				 		    //急停按钮动作故障
    OM_Hardware_Rectification,             	//无可用整流模块
    OM_Hardware_AirOutlet_Temp,            	//出风口温度过高
    OM_Hardware_AntiThunder,               	//交流防雷故障
    OM_Hardware_BC20,                      	//交直流模块DC20通信中断
    OM_Hardware_FC08,                      	//绝缘监测模块FC08通信中断

    OM_Hardware_Ammeter,                   	//电度表通信中断
    OM_Hardware_Read_ID,                   	//读卡器通信中断 
    OM_Hardware_RC10,                      	//rc10通信中断
    OM_Hardware_AirFan,                    	//风扇调速板故障
    OM_Hardware_Fuse,                      	//直流熔断器故障
    OM_Hardware_Contactor,                 	//高压接触器故障
    OM_Hardware_Door_Open                  	//门打开
};
typedef struct
{
	U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_no;    				            //终端号验证
    U8 gun_state;                           //充电状态
    U8 gun_exist;                           //是否插枪  0x00否 0x01是 需做到变位上送
    U8 gun_temp;                            //枪线温度整形，偏移量-50;待机置零（以正负极柱中最高温度为准）
    U8 chg_timerp[2];                       //累计充电时间  单位：min；待机置零
    U8 chg_capacity[4];                     //累计充电电量  小数点后3位
    U8 chg_amount[4];                       //充电金额  小数点后 4 位
    U8 hardware_fault[2];                   //硬件故障
    U8 Module_fault[4];                     //模块故障
    U8 Fault_code;                          //故障码
}OM_GN_Send_RealTime_Gun;


//7.3电表底数上报
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_no;    				            //终端号验证
    U8 active_power[4];                     //有功功率 
    U8 total_active_energy[4];              //有功总电能  小数后3位
}OM_GN_Send_Meter_Base_Number;


//7.4订单信息上报
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 order_data_len[4];                   //订单数据长度
    U8 order_data[32];                      //业务平台的完整订单数据。运维平台只原样保存。
    U8 shutdown_reason;                     //停机原因
}OM_GN_Send_Order_Info;


//7.5订单信息应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 handling_result;                     //处理结果 1：成功 2：失败
}OM_GN_Recv_Order_Info;


//8.1充电设备工作参数设置
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];  
    U16 business_platform_name;             //业务平台名称
    char business_platform_address[64];       //业务平台地址 
    U16 business_platform_port;             //业务平台端口
    U16 supervision_platform_name;          //监管平台名称
    char supervision_platform_address[64];    //监管平台地址
    U16 supervision_platform_port;          //监管平台端口
}OM_GN_Recv_Set_Device_Param;


//8.2充电设备工作参数设置应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 set_results;                         //0x00 失败 0x01 成功
}OM_GN_Send_Set_Device_Param;


//8.3二维码更新
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_num;                             //0：A 枪，1：B 枪
    U8 qrcode_str[200];                     //下发二维码固定字段，不足补0
}OM_GN_Recv_Qrcode_Update;


//8.4二维码更新应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 gun_no;    				            //终端号验证
    U8 set_results;                         //0x00 失败 0x01 成功
}OM_GN_Send_Qrcode_Update;


//9.1远程重启
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 execution_control;                   //0x01：立即执行 0x02：空闲执行
}OM_GN_Recv_Set_Reboot;


//9.2远程重启应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 set_results;                         //0x00 失败 0x01 成功
}OM_GN_Send_Set_Reboot;


//9.3远程更新
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 device_type;                         //0x01：直流 0x02：交流
    U8 device_power[2];                     //不足2位补零
    U8 updata_server_address[16];           //不足16位补零
    U8 updata_server_port[2];           
    U8 user_name[16];
    U8 user_password[16];
    U8 file_path[32];                       //不足32位补零，文件路径名由平台定义
    U8 updata_objects;                      //0x01：A板   0x02：B板   0x03：C板
    U8 download_timeout_time;               //单位：min
    U8 update_ctrl;               	        //1升级立即执行，2空闲执行
}OM_GN_Recv_Set_Update_Ftp;


//9.4远程更新应答
enum{
    updata_status_success           = 0,    //成功
    updata_status_err_no            = 0x01, //编号错误
    updata_status_err_type          = 0x02, //程序与设备型号不符
    updata_status_err_timeout       = 0x03, //下载更新文件超时
};

typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 updata_status;                       
}OM_GN_Send_Set_Update_Ftp;


//9.5远程锁机应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 lockdown_result;                     // 0x00：无效 0x01：锁机失败 0x02：锁机成功
    U8 unlock_result;                       
    U8 lockdown_fail_reason;                //0x00：无原因 0x01：已插枪 0x02：已插枪且充电中 0x03：其他原因   
    U8 unlock_fail_reason;                  //0x00：无原因 0x01：解锁超时 0x02：其他原因   
}OM_GN_Send_Lock_machine;


//9.6远程设备锁机/解锁
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 remote_command;                     // 0x00：无效 0x01：锁机 0x02：解锁
}OM_GN_Recv_Lock_machine;


//9.7本地锁机状态上传
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 lockdown_state;                     // 0x00：无效 0x01：已锁机 0x02：未锁机 0x03：其他（预留）
    U8 lockdown_reason;                    //0x00：无效 0x01：本地人工锁机 0x02：其他原因
    U8 unlock_reason;                      //0x00：无效 0x01：本地人工解锁 0x02：其他原因
}OM_GN_Send_Lock_machine_state;


//9.8本地锁机应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
}OM_GN_Recv_Lock_machine_state;


//9.9日志读取
enum{
    log_type_run_log                = 0x00, //运行日志
    log_type_fault_log              = 0x01, //故障日志
    log_type_bms_log                = 0x02, //BMS日志
    log_type_other_log              = 0x03, //其它
};
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 log_type;                            
}OM_GN_Recv_Log_read;


//9.10日志读取应答
typedef struct{
    U8 device_number[OM_DEV_NUM_LEN];
    U8 log_len[2];                      //日志数据长度<=512 字节
    U8 log_data[32];
}OM_GN_Send_Log_read;


typedef struct
{
	OM_GN_Recv_Identification 	       om_strRecvIdenf;			    //应答-登录认证
	OM_GN_Recv_Heart 			       om_strRecvHeart;			    //应答-心跳
    OM_GN_Recv_Network_Module_Info     om_strRecvNetModuleInfo;     //应答-网络模块信息
    OM_GN_Recv_RealTime_Gun            om_strRecvRealTimeGun;       //应答-实时充电枪状态
    OM_GN_Recv_Order_Info              om_strRecvOrderInfo;         //应答-订单信息
    OM_GN_Recv_Set_Device_Param        om_strRecvSetDeviceParam;    //应答-设备参数设置
    OM_GN_Recv_Qrcode_Update           om_strRecvQrcodeUpdate;      //应答-二维码更新
    OM_GN_Recv_Set_Reboot              om_strRecvSetReboot;         //应答-远程重启
    OM_GN_Recv_Set_Update_Ftp          om_strRecvSetUpdateFtp;      //应答-远程更新
    OM_GN_Recv_Lock_machine            om_strRecvLockMachine;       //应答-远程锁机
    OM_GN_Recv_Lock_machine_state      om_strRecvLockMachineState;  //应答-本地锁机状态
    OM_GN_Recv_Log_read                om_strRecvLogRead;           //应答-日志读取
}OM_RECV_Data;



typedef struct  {
    U8 up_gun_state;            		//

    U8 up_start_ret;            		//
    U8 up_start_fail_reason;            //

	U8 up_stop_ret;            		//
    U8 up_stop_fail_reason;            //
    
}om_up_gun_data_ctrl;


/*----------------------------*/
typedef struct _om_up_data_ctrl {
	//桩数据
    uint8_t A_hardware_version[4];       //充电模块硬件版本
    uint8_t A_soft_version[4];           //充电模块软件版本
    uint8_t B_hardware_version[4];       //灯板硬件版本
    uint8_t B_soft_version[4];           //灯板软件版本
    uint8_t C_hardware_version[4];       //网络模块硬件版本
    uint8_t C_soft_version[4];           //灯板模块软件版本
    uint8_t Signature_value[16];         //签名值

    uint8_t Order_data_len[4];           //订单数据长度
    uint8_t Order_data[32];              //订单数据

    uint8_t log_len[2];                  //帧长度<=512 字节
    uint8_t log_data[32];                //日志数据
    
    uint8_t up_update_ret;            	 //升级结果，0成功，1编号错误，2程序与设备类型不符合，3下载文件超时S
    uint8_t remote_command;             // 0x00：无效 0x01：锁机 0x02：解锁

    OM_GN_Send_Network_Module_Info network_info;
    OM_GN_Recv_Set_Device_Param Set_Device_Param;
    OM_GN_Send_RealTime_Gun RealTime_Gun;
    OM_GN_Send_Order_Info Order_Info;
    OM_GN_Recv_Qrcode_Update Qrcode_Update;
    OM_GN_Send_Lock_machine Lock_machine;
    OM_GN_Send_Lock_machine_state machine_state;
    
	//枪数据
    om_up_gun_data_ctrl om_strUpGunData[GUN_NUM_MAX];
	
}om_up_data_ctrl;


extern om_up_data_ctrl om_tcp_data_ctrl;



void OM_GNUpProtocolDeal(void);
uint32_t monitor_getChgTotalPower(uint8_t u8Port);
void om_tcp_data_ctrl_init(void);

#endif

#endif
