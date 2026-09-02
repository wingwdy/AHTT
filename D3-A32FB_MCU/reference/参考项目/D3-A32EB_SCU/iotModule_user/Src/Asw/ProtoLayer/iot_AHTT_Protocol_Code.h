#ifndef __IOT_AHTT_PROTOCOL_CODE_H__
#define __IOT_AHTT_PROTOCOL_CODE_H__

#include "tcp_gn.h"

#define AHTT_PROTOCOL_VER				14				//安徽铁塔低速

#define AHTT_FRAME_HEAD                 14             //

#define AHTT_PROTOCOL_VERSION   "0.1"

#define AHTTDATA_PHYCARD_LEN   	    4      	// 物理卡号长度
#define AHTTDATA_CARD_LEN   		8      	// 安徽tt卡号长度
#define AHTTDATA_TRDNUM_LEN   		16    	// ahtt流水号长度


#define AHTT_RATE_NUM_MAX   	    8      	// 安徽铁塔资费数据最大段数

enum{
    E_AHTT_GunState_Free,
    E_AHTT_GunState_Work,
    E_AHTT_GunState_Fault,
    E_AHTT_GunState_Offline,
};

enum{
    E_AHTT_GunCharge_Free,
    E_AHTT_GunCharge_Charging,
    E_AHTT_GunCharge_Finish,
    E_AHTT_GunCharge_Pause,
};

enum{
    E_AHTT_ChargeMode_Start = 1,
    E_AHTT_ChargeMode_Full,
    E_AHTT_ChargeMode_Max,
};

typedef enum {
    E_AHTT_ChargeResult_Sucs = 1,           // 1-成功
    E_AHTT_ChargeResult_ChannelFault,       // 2-通道故障
    E_AHTT_ChargeResult_ChannelBusy,         // 3-通道不在空闲
    E_AHTT_ChargeResult_DeviceIDErr,         // 4-设备ID错误
    E_AHTT_ChargeResult_AutoStopOff,         // 5-未开启充满自停
    E_AHTT_ChargeResult_LineOverTemp,        // 6-进线接线过温
    E_AHTT_ChargeResult_VoltageLow,          // 7-输入电压过低
    E_AHTT_ChargeResult_VoltageHigh,         // 8-输入电压过高
    E_AHTT_ChargeResult_PhaseGroundFault,    // 9-相序/接地异常
    E_AHTT_ChargeResult_RelayLStuck,         // 10-L继电器粘连
    E_AHTT_ChargeResult_RelayNStuck,         // 11-N继电器粘连
    E_AHTT_ChargeResult_OutputShort,         // 12-输出负载短路
    E_AHTT_ChargeResult_MeterFault,          // 13-计量电路故障
    E_AHTT_ChargeResult_PilotVolAbnormal,    // 14-充电引导电压异常
    E_AHTT_ChargeResult_RelayOverTemp,       // 15-继电器过温
    E_AHTT_ChargeResult_GunNotConn,          // 16-充电枪未插入车辆
    E_AHTT_ChargeResult_EmergStop            // 17-急停按钮按下
} E_AHTT_ChargeResult;

typedef enum {
    E_AHTT_StopReason_MnyOver = 0,               // 0-金额用完
    E_AHTT_StopReason_DisGun = 1,               // 1-拔掉自停
    E_AHTT_StopReason_FullAuto,                 // 2-自动充满
    E_AHTT_StopReason_LoadOver,                 // 3-过载自停
    E_AHTT_StopReason_CardCost,                 // 4-刷卡板退费引起的通道停止
    E_AHTT_StopReason_NoTime,                 // 5-通道没有时间引起的通道停止
    E_AHTT_StopReason_EletricFault,             // 6-电表通讯异常结束
    E_AHTT_StopReason_DrawGun,                  // 7-拔枪停止
    E_AHTT_StopReason_Estop,                    // 8-急停按钮按下
    E_AHTT_StopReason_StartFail,                // 9-车端未能正常启动充电
    E_AHTT_StopReason_TimeOver,                 // 10-时间用完
    E_AHTT_StopReason_EleOver,                  // 11-电量用完
    E_AHTT_StopReason_EleMax,                   // 12-电量达到最大值
    E_AHTT_StopReason_PeFault,                  // 13-充电桩接地故障停止
    E_AHTT_StopReason_OverVol,                  // 14-过压结束
    E_AHTT_StopReason_UnderVol,                 // 15-欠压结束
    E_AHTT_StopReason_RmtFns,                   // 16-远程结束
    E_AHTT_StopReason_CardStop,                 // 17-刷卡主动结束
    E_AHTT_StopReason_CarStop,                  // 18-车端结束充电
    E_AHTT_StopReason_LeakStop,                 // 19-漏电结束
    E_AHTT_StopReason_RelayTp,                  // 20-继电器过温停止
    E_AHTT_StopReason_VolFault,                 // 21-CP电压异常
    E_AHTT_StopReason_NLTemp,                   // 22-接线座过温停止
    E_AHTT_StopReason_Power,                    // 23-掉电故障
    
	E_AHTT_Unknow 				        = 0xFD,     //未知错误
	E_AHTT_Other 				        = 0xFE,     //其他故障
	E_AHTT_Reason_Finish 				= 0xFF,     //订单上报完成，不需要开机上报
} E_AHTT_StopReason;


typedef struct
{
    uint8_t     head;
    uint8_t     len[2];
    uint8_t     ver;
    uint8_t     device_num[5];
    uint8_t     ser[2];
    uint8_t     cmd;
}AHTT_HEAD_T;

enum{
    AHTT_Identification = 0x01,   //签到

    AHTT_Heart_Set = 0x02,        //设置心跳周期
    AHTT_Heart_Search = 0x03,     //平台查询心跳周期
    AHTT_Heart = 0x81,            //心跳包

    AHTT_Port_Domain = 0x04,      //平台下发域名与端口

    AHTT_MaxChgTime = 0x0A,        //平台下发最大充电时长
    AHTT_Sea_MaxChgTime = 0x0B,    //平台查询最大充电时长

    AHTT_Auth = 0x4D,             //平台请求充电

    AHTT_RealData = 0x93,         //平台读取实时检测数据
    AHTT_Stop_Chg_Ack = 0x4B,     //平台下发停止充电

    AHTT_Chg_Record = 0x94,       //交易记录上传

    AHTT_ChgCard_Record = 0x4C,   //刷卡订单交易记录上传

    AHTT_Equipara = 0x84,         //下发设备参数
    AHTT_Sea_Equipara = 0x85,     //平台查询设备参数

    AHTT_GetPower = 0x87,          //获取设备功率

    AHTT_TimeSyn_ACK = 0x95,		//获取平台时间

    AHTT_State = 0x96,            //平台请求设备状态

    AHTT_Alarm = 0xC1,            //上报当前设备状态
    AHTT_Network_Alarm = 0xC2,    //上报网络状态

    AHTT_Temper_Alarm = 0xC3,     //上报当前温
    AHTT_SetTemper = 0xC4,        //平台下发温度告警值
    
    AHTT_set_update_ftp = 0xD1,	//平台设远程升级程序
};

//安徽铁塔交易标识（启动方式）
enum {
    eUP_Start_Style_NULL_AHTT 		= 0,  		//无
    eUP_Start_Style_App_AHTT  		= 0x01,   	//app 启动
    eUP_Start_Style_CardOnline_AHTT  	= 0x02,		//卡启动
    eUP_Start_Style_CardOffline_AHTT  = 0x04,		//离线卡启动
    eUP_Start_Style_VIN_AHTT  		= 0x05,		// vin 码启动充电
    eUP_Start_Style_Test_AHTT 		= 0x06,		// 厂内测试启动
	eUP_Start_Style_Fixtime_AHTT 		= 0x07,		// 定时启动
};

//安徽铁塔启动充电失败原因
enum {
    eUP_Start_Fail_NULL_AHTT = 0,							//无
    eUP_Start_Fail_DevNumErr_AHTT = 0x01, 		//设备编号不匹配
    eUP_Start_Fail_Working_AHTT = 0x02, 			//枪已在充电
    eUP_Start_Fail_DevErr_AHTT = 0x03, 				//设备故障
    eUP_Start_Fail_Offline_AHTT = 0x04, 			//设备离线
    eUP_Start_Fail_NoConn_AHTT = 0x05,				//未插枪
    eUP_Start_Fail_Reconnect_AHTT = 0x06, 		//超时不可启动，需要重新插拔枪
};

//停止充电失败原因
enum {
    eUP_Stop_Fail_NULL_AHTT 			= 0,    	//无
    eUP_Stop_Fail_DevNumErr_AHTT	= 0x01,   	//设备编号不匹配
    eUP_Stop_Fail_NoWorking_AHTT 	= 0x02,    	//枪未处于充电状态
    eUP_Stop_Fail_Other_AHTT 		= 0x03, 	//其他
};

typedef struct _AHTT_charge_record{
    //交易记录信息，0x94
    U8 stop_reason;
    U8 gun_num;
    U8 total_power[3];        		// 总电量
    U8 Remaining_sum[2];            //剩余金额
    U8 total_money[2];        		// 总金额
    U8 total_sever48[2];            //总服务费
} AHTT_charge_record;

typedef struct {
    uint16_t number;  // 单号
    uint8_t channel_number;  // 通道号
    uint8_t card_number[5];  // 卡号（5字节）
    uint16_t swipe_amount;   // 刷卡金额（单位：元）
} STU_AHTT_ChgCard_Record;

typedef struct
{

}AHTT_Recv_Identification;

typedef struct
{
	U8 head;
}AHTT_Recv_Heart;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 billing_model[2];        	//计费模型编号，首次链接到平台时置零
    U8 billing_model_result;     	//0一致，1不一致
}AHTT_Recv_Billing_Verify;


typedef struct
{
	U8 endTime; /* 结束时间 单位：小时 */
	U8 ele_fee; /* 电价 单位：分 */
	U8 ser_fee; /* 服务费 单位：分 */
}AHTT_CostModel;

typedef struct
{
	U8 odd_number[2];     //订单号
    U8 gun_no;                      //通道号
    U8 balance[4];                    //余额
    U8 startResult;                 //启动结果
    U8 rateType;                   //资费类型
    U8 ChargePara[3];               //充电参数,4A、 .4D、 4E 传递金额（分）

    U8 baseEleMny;                  //基础电费
    U8 power_Num;                   //资费个数

    /* 功率阶梯 6个字节 */

    U8 gun_chrg_step;               //充电时段阶梯个数
    AHTT_CostModel ahCostModel[AHTT_RATE_NUM_MAX];  //最大5个
}AHTT_Recv_Auth_Ack;

typedef struct
{
	U8 odd_number[2];  //单号
    U8 gun_no;                      //通道号
    U8 gun_chrg_mode;               //充电模式，1-开启充电 2-充满自停
    U8 gun_chrg_mode_param[2];         //充电参数
    U8 gun_chrg_step;               //充电时段阶梯个数
    AHTT_CostModel ahCostModel[AHTT_RATE_NUM_MAX];  //最大5个
}AHTT_Recv_Start_Charge;

typedef struct
{
	U8 odd_number[2];  //单号
    U8 gun_no;
}AHTT_Recv_Stop_Charge;

typedef struct
{
    U8 gunNum;						//0x00 上传成功 0x01 非法账单
}AHTT_Recv_Record_Ack;

typedef struct
{
    uint8_t Report_results;
} AHTT_Recv_AHTT_State;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 gun_no;						//
    U8 Logic_card_number[8];        //逻辑卡号，BCD，卡面印刷卡号
    U8 account_balance[4];        	//账户余额，2位小数
}AHTT_Recv_Sum_Update;

typedef struct
{
	U8 cardmoney;   //单位：0.1元
    U8 cardtime;
    U8 slottime;
    U8 stacur;
    U8 maxcurr;
    U8 discondelay;
    U8 fulldellay;
    U8 uploadcyc;

}AHTT_Recv_Set_Param;

typedef struct
{
	U8 device_number[DEV_NUM_LEN];
    U8 platform_set_reboot;          //远程设置重启，1，立即执行，2空闲执行
}AHTT_Recv_Reboot;

typedef struct
{
	U8 UpdateFileName[8];
	U8 md5check[16];
} AHTT_Recv_Update_ftp;

typedef struct
{
	uint8_t cyctime;        //单位min
}AHTT_Recv_SetCyc;

typedef struct
{
	char domain[24];
    char port[6];
}AHTT_Recv_Port_Domain;

typedef struct
{
    U8 Temalarm;        //偏移50，当前值60表示10
}AHTT_Recv_SetTemper;

typedef struct
{
    U8 Ahtt_PhyCard_number[4];        //卡号，方便2min内后面刷其他卡做判断
    U8 CardCnt;                     //刷卡次数
    U8 CardAuthIng;                 //0可以计次，1准备鉴权, 2刷卡鉴权等待结果中，2充电中
    U8 revs;                        //预留
    U32 CardTimeInit;               //刷卡计时
}AHTT_CardStartData;



typedef struct
{
    U8 upResult;         //上报状态结果，可以公用

    U8 overTempAlm;        //告警温度值
    U8 overTempValue;        //告警温度值
    
    U8 netSgnAlm;           //网络信号告警
    U8 devErrAlm[2];           //设备故障告警

    U8 chargeResult_4D;                 //启动充电结果
    U8 chargeMode_4D;                 //工作模式

    U8 ahttGunState[GUN_NUM_MAX];

    STU_AHTT_ChgCard_Record cardSendData;

    AHTT_CardStartData CardStartData[GUN_NUM_MAX];

    //充电资费部分，刷卡和扫码统一讲数据整理至此处
    U8 gun_chrg_step;               //充电时段阶梯个数
    AHTT_CostModel ahCostModel[AHTT_RATE_NUM_MAX];

} AHTT_UpPlatInfo;

typedef struct
{
    uint8_t u8ChargeMaxTime;          //充电最大时长,单位h；
    AHTT_Recv_Set_Param Param_0x84;   //参数设置保存
    AHTT_Recv_SetCyc cyc_0x03;
} AHTT_FlashPlatInfo;


typedef struct
{
	AHTT_Recv_Identification 	strRecvIdenf;			//应答-登录认证
	AHTT_Recv_Heart 			strRecvHeart;			//应答-心跳
	AHTT_Recv_Billing_Verify 	strRecvBillingVer;		//应答-计费模型版本
	AHTT_Recv_Auth_Ack 		    strRecvAuthAck;
	AHTT_Recv_Start_Charge	    strRecvStartCharge;
	AHTT_Recv_Stop_Charge 	    strRecvStopCharge;
	AHTT_Recv_Record_Ack 		strRecvRecordAck;
    AHTT_Recv_AHTT_State        strRecvDevState;
	AHTT_Recv_Sum_Update 		strRecvSumUpdate;
	AHTT_Recv_Set_Param 		strRecvSetParam;
	AHTT_Recv_SetCyc 		    strRecvSetTime;
	AHTT_Recv_Reboot 			strRecvReboot;
	AHTT_Recv_Update_ftp 		strRecvFtp;
    AHTT_Recv_Port_Domain       strRevPorDOmain;
    AHTT_Recv_SetTemper         strRecvSetTemper;
}AHTT_RECV_Data;

void AHTTUpProtocolDeal(void);


AHTT_UpPlatInfo *Get_AhttAllInfo(void);
void AHTT_CardAuthStart_Cmd(uint8_t u8Port);

void AHTT_DealUpdate_Cmd(uint8_t u8Port);
uint8_t ahtt_packChgRecord(uint8_t u8Port, AHTT_charge_record *pRecord);

#endif
