/******************************************************************************
* File Name          : template.h
* Description        : Code for xxxxxxxxxxx
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef ASW_MONITOR_H_
#define ASW_MONITOR_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "MS_Nvm.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 充电状态定义 */
#define ASWMONITOR_CHARGE_STATE_INIT              0
#define ASWMONITOR_CHARGE_STATE_IDLE              1
#define ASWMONITOR_CHARGE_STATE_ONGOING           2
#define ASWMONITOR_CHARGE_STATE_END               3

/* 订单保存状态 */
#define ASWMONITOR_ORDER_SAVE_NULL                0
#define ASWMONITOR_ORDER_SAVE_START               1
#define ASWMONITOR_ORDER_SAVE_PERIOD              2
#define ASWMONITOR_ORDER_SAVE_STOP                3

#define ASWMONITOR_ORDER_TRANSACTION_NUM_LEN      20

/* 复位状态定义 */
#define ASWMONITOR_REBOOT_STATE_IDLE              0
#define ASWMONITOR_REBOOT_STATE_WAITING           1
#define ASWMONITOR_REBOOT_STATE_ONGOING           2

/* 标准计费模型相关定义 */
#define ASWMONITOR_BILLMODE_PERIOD_COUNT          48
#define ASWMONITOR_BILLMODE_RATE_COUNT            48

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t validFlag;
    uint32_t totalPrice[ASWMONITOR_BILLMODE_RATE_COUNT];       /* 费率总单价 小数点后五位(电费+服务费) */

    uint8_t rateCount;                                         /* 费率个数 */
    uint32_t rateElecPrice[ASWMONITOR_BILLMODE_RATE_COUNT];    /* 费率电费单电价 小数点后五位*/
    uint32_t rateSeverPrice[ASWMONITOR_BILLMODE_RATE_COUNT];   /* 费率服务费单价 小数点后五位*/

    uint8_t periodCount;                                       /* 时段数 */
    uint8_t periodRate[ASWMONITOR_BILLMODE_PERIOD_COUNT];      /* 时段费率号 */
    uint8_t startTime[ASWMONITOR_BILLMODE_PERIOD_COUNT][2];    /* 时段起始时间 */
    uint8_t stopTime[ASWMONITOR_BILLMODE_PERIOD_COUNT][2];     /* 时段结束时间 */
    uint8_t elecLossRate;                                      /* 计损比率 */
}AswMonitorBillMode_Struct;

typedef struct 
{
    uint64_t lastMeterEnergyVal;

    uint32_t chargeStartTime;       /* 充电开始时间 时间戳 */
    uint32_t chargeStopTime;        /* 充电结束时间 时间戳 */
    uint32_t chargeTime;            /* 充电时长 单位：秒 */

    uint32_t startMeterVal;         /* 充电开始电量，小数点后4位 */
    uint32_t stopMeterVal;          /* 充电结束电量，小数点后4位 */

    uint32_t totalMoney;            /* 总金额，小数点后4位 */
    uint32_t totalElecMoney;        /* 电费金额，小数点后4位 */
    uint32_t totalServeMoney;       /* 服务费金额，小数点后4位 */

    uint32_t totalEnergy;           /* 充电总电量, 小数点后4位，单位：度 */ 
    uint32_t totalLossEnergy;       /* 计损总电量  小数点后4位，单位：度 */

    /* 各费率 */
    uint32_t rateTotalEnergy[ASWMONITOR_BILLMODE_RATE_COUNT];           /* 费率的总电量, 小数点后四位 */
    uint32_t rateTotalLossEnergy[ASWMONITOR_BILLMODE_RATE_COUNT];       /* 费率的计损电量, 小数点后四位 */
	uint32_t rateEleMoney[ASWMONITOR_BILLMODE_RATE_COUNT];		        /* 费率的电费金额, 小数点后四位 */
	uint32_t rateSerMoney[ASWMONITOR_BILLMODE_RATE_COUNT];		        /* 费率的服务费金额, 小数点后四位 */
	uint32_t rateTotalMoney[ASWMONITOR_BILLMODE_RATE_COUNT];	        /* 费率的总金额, 小数点后四位 */

    /* 各时段 */
	uint32_t periodElePower[ASWMONITOR_BILLMODE_PERIOD_COUNT];          /* 时段的总电量, 小数点后四位 */
	uint32_t periodEleMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];	        /* 时段的电费金额, 小数点后四位 */
	uint32_t periodSerMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];	        /* 时段的服务费金额, 小数点后四位 */
	uint32_t periodTotalMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];        /* 时段的总金额 小数点后四位 */

    uint32_t orderTransactionNum[ASWMONITOR_ORDER_TRANSACTION_NUM_LEN]; /* 订单编号 */
}AswMonitorChargeData_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void AswMonitor_InitMemory(void);
void AswMonitor_MainFunction(void);



#endif





















