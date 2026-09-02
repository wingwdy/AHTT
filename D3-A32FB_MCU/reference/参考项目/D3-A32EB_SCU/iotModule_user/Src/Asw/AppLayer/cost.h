#ifndef __COST_H__
#define __COST_H__

#include "AppHeaderSummary.h"
#include "tcp_gn.h"
#include "mbsDataUpdate.h"

#define MAX_RATE_PERIOD_CNT			48   //时段最大数量
#define MAX_RATE_NUMBER			    48   //各时段的费率类型，公牛平台按照半小时区分时段，一天共48个

enum {
    eCostType_SPVF,     //按照类型计费，例尖峰平谷
    eCostType_PerTime,  //按照时段计费
};

typedef struct _RATE_MODEL_T{
    uint8_t costType;   //多类电价计价类型，0尖峰平谷类型，1按照时间分时段类型

    uint8_t billing_model[2];			//计费模型编号，首次链接到平台时置零，gn平台
  
    uint8_t  billing_model_plat[8];      //计费模型编号，其他平台
    /*计费模式1 输入变量*/
    //类型电价
    uint8_t rateNumber;   //多类电价数量
    uint8_t rate_8u[MAX_RATE_PERIOD_CNT];        	//48费率段-每段的费率号   范围值0到3
    uint32_t price48_16u[MAX_RATE_PERIOD_CNT];     	//48费率段-每段电价费率   分辨率0.00001元
    uint32_t sever48_16u[MAX_RATE_PERIOD_CNT];     	//48费率段-服务费费率     分辨率0.00001元
    uint8_t elec_loss_ratio_32u;	//电损比例                分辨率0.01
    
    //时段电价
    uint8_t PeriodNumber;   //时段数量
    uint8_t PeriodRate_8u[MAX_RATE_NUMBER];        	        //48费率段-每段的费率号   范围值0到3
    uint32_t PeriodPrice48_16u[MAX_RATE_NUMBER];     	//48费率段-每段电价费率   分辨率0.00001元
    uint32_t PeriodSever48_16u[MAX_RATE_NUMBER];     	//48费率段-服务费费率     分辨率0.00001元
    uint8_t startTime[MAX_RATE_NUMBER][2];        	    //时间开始,时分BCD码
    uint8_t stopTime[MAX_RATE_NUMBER][2];        	    //时间结束,时分BCD码

  } RATE_MODEL_T;
  
typedef struct
{
	uint32_t last_value;		//（4位小数）
	uint32_t checkLastEle;		//（4位小数）,5s更新一次
	uint32_t checkLastMoney;	//（4位小数），5s更新一次

	uint32_t total_power;		// 总电量 小数点后四位
	uint32_t total_loss_power;	// 记损总电量 小数点后四位
	
	uint32_t allServerMoney;    // 服务费总金额           分辨率 （4位小数）
	uint32_t allEleMoney;       // 电费总金额             分辨率 （4位小数）
	uint32_t total_money;		// 总金额 小数点后四位
	
    //各费率的服务费和电费之和
	uint32_t ele_rate[MAX_RATE_NUMBER];		    //单价 小数点后五位（电费+服务费）
	uint32_t ele_loss_power[MAX_RATE_NUMBER];	//记损电量 小数点后四位
	uint32_t ele_power[MAX_RATE_NUMBER];	//电量 小数点后四位
	uint32_t perEleMoney[MAX_RATE_NUMBER];		//电费金额   小数点后四位
	uint32_t perSerMoney[MAX_RATE_NUMBER];		//服务费金额 小数点后四位
	uint32_t perMoney[MAX_RATE_NUMBER];		    //金额 小数点后四位

    //各时段
	uint32_t PeriodElePower[MAX_RATE_PERIOD_CNT];		//总电量 小数点后四位
	uint32_t PeriodEleMoney[MAX_RATE_PERIOD_CNT];		//时段电费金额   小数点后四位
	uint32_t PeriodSerMoney[MAX_RATE_PERIOD_CNT];		//时段服务费金额 小数点后四位
	uint32_t PeriodMoney[MAX_RATE_NUMBER];		    //时段金额 小数点后四位
	
	uint8_t eleAbnormalTimeCnt;                    //电量异常计数
}COST_GUN_DATA;

typedef struct
{
	/*控制用*/
//	uint8_t  pwr_flag[GUN_NUM_MAX]; 	  //上电标志
	uint8_t  start_chrg[GUN_NUM_MAX];	  //TRUE开始充电，开始计费
	
	/*计费输入*/
	RATE_MODEL_T  rate_model[GUN_NUM_MAX];
	/*计费数据*/
  	COST_GUN_DATA strCostGunData[GUN_NUM_MAX];
}COST_CTRL_T;

/***可供外部调用变量声明***/
extern COST_CTRL_T g_cost_ctrl;

/***可供外部调用函数声明***/

COST_GUN_DATA *CostGetOutput(uint8_t u8Port);

void Cost_stop_deal(uint8_t u8Port);
void Cost_Main( void );
void Cost_Main_Single(uint8_t u8Port);
void Cost_init(void);
void Cost_charge_init (uint8_t u8Port);
void set_rate_state(uint8_t u8Port, uint8_t flag);
void Cost_AppendExtEnergy(uint8_t u8Port, uint32_t extEnergy);

uint8_t CostGetRateModel(uint8_t u8Port);

#endif



