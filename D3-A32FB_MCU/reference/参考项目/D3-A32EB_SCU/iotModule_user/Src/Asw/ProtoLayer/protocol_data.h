#ifndef __PROTOCOL_DATA_H__
#define __PROTOCOL_DATA_H__

#include "Libinclude.h"
#include "Comminclude.h"
#include "protocol_ctrl.h"
#include "iot_GN_Protocol_Code.h"
#include "iot_YKC_Protocol_Code.h"
#include "iot_Monitor_Code.h"
#include "iot_ANPEI_Protocol_Code.h"
#include "iot_YKC_Protocol_CodeV2_1.h"
#include "iot_AHTT_Protocol_Code.h"
#include "iot_HNCT_Protocol_Code.h"
#include "iot_WJY_Protocol_Code.h"

#define GNIOT_QUEUE_CNT				8
#define GNIOT_OTA_TASKIDLEN			32

typedef enum {
    E_Reboot_Null,
    E_Reboot_Idle,
    E_Reboot_Now,
} E_RebootType;

typedef enum {
    E_Update_Null,
    E_Update_Ftp,
    E_Update_Http,
} E_UpdateType;


//==================================================
//收发控制
typedef struct __SEND_CTRL__
{
	int32_t CycTimer;					//周期计时器
	uint8_t u8ImmdFlag;					//立即发送标志
	uint8_t u8SendEnable;				//发送使能
	uint8_t u8SendFlag;					//发送标记
	uint16_t u16UpSrm;					//序列号
}SEND_CTRL;
 
typedef struct __RECV_CTRL__
{
	uint32_t u32CycTimer;				//周期计时器
	uint8_t u8RecvEnable;				//接收使能
	uint8_t u8TimerEnable;				//定时使能
	uint8_t u8RptCnt;					//重复超时次数
	uint8_t u8RecvFlag;					//接收标记
}RECV_CTRL;
typedef struct
{
	U8	u8Gun;
	U32	u32Cmd;
	U8	U8MsgID[32];
}GNIOT_QUEUE;
//==================================================
//蓝牙控制

typedef struct
{
	U8	u8Type;
	
	union BLE_HEAD unHead;
	
	char CPid_string[8];
	char CMac_Addr[12];
	U8	aes_key[16];
//	U8	aes_key[32];

	U8	aes_iv[16];
}BLEProtocolDCB;

typedef struct
{
	U8 u8LoginState;
	U8 u8OtaFlag;
	U8 u8WorkState;

	U32 u32EveReportCyc;
	U8 u8StopRealInfoFlag;
	char cRRpcMsgID[32];			//rrpc  topic id
	char cOtaTaskId[GNIOT_OTA_TASKIDLEN+1];			//ota  task id
	
	REPORT_EVE_QUEUE strEveInfo;
}IOTProtocolDCB;


typedef struct _up_gun_data_ctrl {
    U8 up_srm;            		//上报index
    
    U8 up_gun_state;            		//

    U8 up_start_ret;            		//
    U8 up_start_fail_reason;            //

	U8 up_stop_ret;            		//
    U8 up_stop_fail_reason;            //

    uint8_t up_update_ret;            			//升级结果，0成功，1编号错误，2程序与设备类型不符合，3下载文件超时
    
}up_gun_data_ctrl;


typedef struct
{
    U8 update_ip[18];               //升级服务器ip,ascii
    U16 update_port;                //升级端口
    U8 update_username[32];     	//用户名ascii
    U8 update_password[32];   		//密码ascii
    U8 update_file_path[32];      	//文件路径ascii
    U8 update_file_name[32];      	//文件名称  云快充2.1文件名和路径分开发送 增加一个变量
}GN_Ftp_Info;

typedef enum {
    eOnline_Off,
    eOnline_Start,  //ip连接成功
    eOnline_Auth,   //认证成功
    eOnline_Heart,  //心跳正常
} E_OnlineType;


union PileUpdateUnion {
	GN_Ftp_Info ftpInfo;
};

union ChargeRecordUnion {
    uint8_t RecordData[DEAL_RECORD_MAXLEN - 4];
	charge_record GnChgRecord;
	RecordB53    AnpeiChgRecord;//JJUNIVE anpei
    AHTT_charge_record AhttChgRecord; //Ahtt
	charge_record_ykcv2 YkcRecord; //ykc v2.1
	RecordA3    HaiNCTChgRecord;
    WJY_charge_record WjyChgRecord; //wjy
};

typedef struct _PlatDealRecord {
	U8 PileStopReason;				//桩端的停止原因，不关乎平台上报，固定，平台部分需要自己转换
    //记录
    union ChargeRecordUnion ChgRecord;
} PlatDealRecord;


/* 平台连接情况*/
typedef struct __PLATCONNECTSTA__ {
	uint32_t no_Comm_tick;

    E_OnlineType eOnlineType;
    
	uint8_t login_start;				//登录控制
	uint8_t online_flag;				//登录成功标识
    uint8_t revs[5];
} PlatConnectSta;


/* 平台下发任务需要执行*/
typedef struct __PLATTASKEXCUTE__ {
	E_UpdateType updata_flag;
    union PileUpdateUnion u_updateInfo;
	uint32_t updata_delay_tick;			//开始升级延时

	E_RebootType reboot_flag;
	uint32_t reboot_tick;
} PlatTaskExcute;


//关于平台部分公用此结构体
typedef struct _PROTOCOL_DCB__ {
	//=======================================
    PlatConnectSta OmPlatSta;       //运维平台连接状态
    PlatConnectSta PlatSta;         //运营平台连接状态
    PlatTaskExcute PlatTask;        //平台下发任务需要等待执行，比如升级、重启，针对于桩而非枪

	//公牛云快充枪数据
    up_gun_data_ctrl strUpGunData[GUN_NUM_MAX];
	
	//=======================================
	//接收数据,根据平台类型动态分配,上电的时候分配一次
	OM_RECV_Data *pOMRecvData;  //运维平台
	//全部U8
	RECV_Data *pRecvData;
	YKC_RECV_Data *pYKCRecvData;
	YKC_RECV_Data_V2 *pYKCRecvData_v2;
	ANPEI_RECV_Data *pANPEIRecvData;
    AHTT_RECV_Data  *pAHTTRecvData;
    HaiNCT_RECV_Data  *pHaiNCTRecvData;
	WJY_RECV_Data	*pWJYRecvData;

    //运维平台相关数据
	RECV_CTRL strOmRecvCtrl[GUN_NUM_MAX][15];
	SEND_CTRL strOmSendCtrl[GUN_NUM_MAX][15];

    RECV_CTRL strRecvCtrl[GUN_NUM_MAX][25];//JJUNIVE 20改为25
	SEND_CTRL strSendCtrl[GUN_NUM_MAX][30];
}ProtocolDCB;

extern ProtocolDCB g_ProtocolDCB;

//===========================
typedef struct
{
    MONITOR_STATE_E    	eMOnitorState;    	// 监控状态,所有状态都要从Normal跳转
	
    U8 transaction_log_num[GNDATA_TRDNUM_LEN];    	//交易流水号，设备号（7bytes） +枪号（1byte） +年月日时分秒（6bytes） +自增序号（2bytes）
	U8 LogicCard_number[GNDATA_CARD_LEN];    		//鉴权卡号，BCD，卡面印刷卡号 
	U8 PhyCard_number[GNDATA_PHYCARD_LEN];    	//鉴权卡号，BIN，物理卡号，刷卡鉴权赋值，刷卡停止充电使用

	U8 Auth_card_number[GNDATA_CARD_LEN];    	//鉴权卡号，屏幕显示以及平台交互需要，例如云快充鉴权卡号为物理卡号，但应答卡号是逻辑卡号
	U8 Swip_PhyCard_number[GNDATA_PHYCARD_LEN];    	//充电卡号，BIN，物理卡号，开始充电时更新，刷卡停止充电使用，比如安徽铁塔特殊
	
	U8 sum_updata_ret;				//账户余额更新结果0x00-修改成功0x01-设备编号错误0x02-卡号错误
	U32 sum_balance;				//账户余额小数点两位
	U16 chargeMaxTime;               //最大充电时长单位min
    U32 chargeAllMny;                //充电总金额，4位小数；平台算法不一致会导致不一样，需要按照每个平台的算法

	U32 gun_chrg_mode_param;        // 充电参数 时间模式（单位分钟），金额模式（单位0.1元），电量模式（单位kWh）
	U8 gun_chrg_mode;               // 充电模式 0自动充满，1时间模式，2金额模式，3电量模式
	
	U8 chrg_start_time[7];         	//开始充电时间,bcd
    U8 chrg_stop_time[7];         	//结束充电时间
	U32 total_start_elec;       	// 总起示值0.0001 掉电不记录示值,用U32勉强够
    U32 total_stop_elec;        	// 总止示值0.0001 掉电不记录示值,用U32勉强够
    U32 chrg_ele;        	        // 充电电量0.0001 掉电不记录示值,用U32勉强够
    U32 chg_timer;        			// 充电时长 秒
    
    U8 trade_flag;           		//交易标识 0x01：app 启动 0x02：卡启动
    								//0x04：离线卡启动 0x05: vin 码启动充电

                                    
    U8 CardChargeFaild;           		//刷卡失败，异常原因；0无异常;1无效卡号；2余额不足；
    U32 CardFaildTick;           		//时间更新，3s后消除或者再次刷卡电故障消除

    PlatDealRecord DealRecord;

	U8 upDealCnt;				    //账单上报次数，上限5次，5次之后订单彻底结束
	U8 ExistChargeDeal;				//1订单进行中，0不存在订单，上报完成, 有的平台需要订单未上报之前实时订单交易流水号不清零，需要状态变位
}CHG_DATA_T;

extern CHG_DATA_T g_chgData[GUN_NUM_MAX];


#endif

