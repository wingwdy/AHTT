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
#include "Asw_ErrorHandle.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
/* 订单控制状态定义 */
#define ASWMONITOR_ORDER_CTRL_IDLE                0
#define ASWMONITOR_ORDER_CTRL_ONGOING             1
#define ASWMONITOR_ORDER_CTRL_END                 2

/* 订单保存原因 */
#define ASWMONITOR_ORDER_SAVE_NULL                0
#define ASWMONITOR_ORDER_SAVE_START               1
#define ASWMONITOR_ORDER_SAVE_PERIOD              2
#define ASWMONITOR_ORDER_SAVE_STOP                3

/* 订单号长度 */
#define ASWMONITOR_ORDER_TRANSACTION_NUM_LEN      20

/* 卡号长度 */
#define ASWMONITOR_CARD_ID_LEN                    16

/* 随机数长度 */
#define ASWMONITOR_RANDOM_LEN                     48

/* 订单保存状态 */
#define ASWMONITOR_ORDER_STATE_NULL               0
#define ASWMONITOR_ORDER_STATE_START              1

/* 复位状态定义 */
#define ASWMONITOR_REBOOT_STATE_IDLE              0
#define ASWMONITOR_REBOOT_STATE_WAITING           1
#define ASWMONITOR_REBOOT_STATE_ONGOING           2

/* 标准计费模型相关定义 */
#define ASWMONITOR_BILLMODE_PERIOD_COUNT          96
#define ASWMONITOR_BILLMODE_RATE_COUNT            96

#define ASWMONITOR_BILLMODE_TYPE_FOUR             0
#define ASWMONITOR_BILLMODE_TYPE_MULT             1

/* 启动充电发起方 */
#define ASWMONITOR_ORDER_START_SRC_NULL           0         /* 无效的方式 */
#define ASWMONITOR_ORDER_START_SRC_PNC            1         /* 即插即充 */
#define ASWMONITOR_ORDER_START_SRC_CARD           2         /* 刷卡授权 */
#define ASWMONITOR_ORDER_START_SRC_APP            3         /* APP授权 */

/******************************************************************************
*    Enum Definition
*******************************************************************************/
/* 充电控制方式 */
typedef enum
{
    eAswMonitorChargeCtrlType_AutoCharge,             /* 自动充满 */
    eAswMonitorChargeCtrlType_JudgeTime,              /* 按时间充电 */
    eAswMonitorChargeCtrlType_JudgeMoney,             /* 按金额充电 */
    eAswMonitorChargeCtrlType_JudgeEnergy,            /* 按电量充电 */
}eAswMonitorChargeCtrlType_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t billModeID[17];                                    /* 计费模型ID (选填)*/

    uint8_t billmodeType;                                      /* 计费模型4类电价或者多类电价（选填）*/
    uint8_t validFlag;
    uint32_t totalPrice[ASWMONITOR_BILLMODE_RATE_COUNT];       /* 费率总单价 小数点后五位(电费+服务费) */

    uint8_t rateCount;                                         /* 费率个数 */
    uint32_t rateElecPrice[ASWMONITOR_BILLMODE_RATE_COUNT];    /* 费率电费单电价 小数点后五位*/
    uint32_t rateSeverPrice[ASWMONITOR_BILLMODE_RATE_COUNT];   /* 费率服务费单价 小数点后五位*/

    uint8_t periodCount;                                       /* 时段数 */
    uint8_t periodRate[ASWMONITOR_BILLMODE_PERIOD_COUNT];      /* 时段费率号 */
    uint8_t startTime[ASWMONITOR_BILLMODE_PERIOD_COUNT][2];    /* 时段起始时间 */
    uint8_t stopTime[ASWMONITOR_BILLMODE_PERIOD_COUNT][2];     /* 时段结束时间 */
    uint8_t elecLossRate;                                      /* 计损比率 小数点后两位 */
}AswMonitorBillMode_Struct;

typedef struct
{
    uint8_t startSrc;                                /* 发起源 */
    eAswMonitorChargeCtrlType_Enum eChargeCtrlType;  /* 充电控制方式 */
    uint32_t chargeCtrlVal;                          /* 充电控制变量，时间:秒, 金额：0.01 元，电量：0.01度*/
    uint32_t accountMoney;                           /* 账户余额 0.01 元 */
    uint8_t authCardID[ASWMONITOR_CARD_ID_LEN];      /* 授权卡号 */
    uint8_t phyCardID[ASWMONITOR_CARD_ID_LEN];       /* 物理卡号 */
    uint8_t randomNum[ASWMONITOR_RANDOM_LEN];        /* 随机数 */
}AswMonitorChargeCtrl_Struct;

typedef enum
{
    eAswMonitorRebootType_Null,
    eAswMonitorRebootType_Immediate, /* 立即重启 */       
    eAswMonitorRebootType_WaitIdle,  /* 等待空闲 */ 
}AswMonitorRebootType_Enum;


typedef struct 
{
    uint64_t lastMeterEnergyVal;     /* 上一次计算电量，小数点后4位 */

    AswErrorType_Enum eChargeStopReason; /* 充电停止原因 */

    uint32_t chargeStartTime;       /* 充电开始时间 时间戳 */
    uint32_t chargeStopTime;        /* 充电结束时间 时间戳 */
    uint32_t chargeTime;            /* 充电时长 单位：秒 */

    uint32_t startMeterVal;         /* 充电开始电量，小数点后4位 */
    uint32_t stopMeterVal;          /* 充电结束电量，小数点后4位 */

    uint32_t totalMoney;            /* (计损后)总金额，小数点后4位 */
    uint32_t totalElecMoney;        /* (计损后)电费金额，小数点后4位 */
    uint32_t totalServeMoney;       /* (计损后)服务费金额，小数点后4位 */

    uint32_t totalEnergy;           /* (计损前)充电总电量, 小数点后4位，单位：度 */ 
    uint32_t totalLossEnergy;       /* (计损后)总电量  小数点后4位，单位：度 */

    uint8_t currentRateNum;          /* 当前费率号 */
    uint8_t currentPeriodNum;        /* 当前时段号 */

    /* 各费率 */
    uint32_t rateTotalEnergy[ASWMONITOR_BILLMODE_RATE_COUNT];           /* (计损前)费率的总电量, 小数点后四位 */
    uint32_t rateTotalLossEnergy[ASWMONITOR_BILLMODE_RATE_COUNT];       /* (计损后)费率的计损电量, 小数点后四位 */
	uint32_t rateEleMoney[ASWMONITOR_BILLMODE_RATE_COUNT];		        /* (计损后)费率的电费金额, 小数点后四位 */
	uint32_t rateSerMoney[ASWMONITOR_BILLMODE_RATE_COUNT];		        /* (计损后)费率的服务费金额, 小数点后四位 */
	uint32_t rateTotalMoney[ASWMONITOR_BILLMODE_RATE_COUNT];	        /* (计损后)费率的总金额, 小数点后四位 */

    /* 各时段 */
    uint8_t periodValidFlag[ASWMONITOR_BILLMODE_PERIOD_COUNT];          /* 置TRUE,表示该时段在本次充电经历过 */
	uint32_t periodElePower[ASWMONITOR_BILLMODE_PERIOD_COUNT];          /* (计损前)时段的总电量, 小数点后四位 */
	uint32_t periodEleMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];	        /* (计损后)时段的电费金额, 小数点后四位 */
	uint32_t periodSerMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];	        /* (计损后)时段的服务费金额, 小数点后四位 */
	uint32_t periodTotalMoney[ASWMONITOR_BILLMODE_PERIOD_COUNT];        /* (计损后)时段的总金额 小数点后四位 */

    uint64_t preciseElecTotalMoney;                                     /* (计损后)精确电费 */
    uint64_t preciseServTotalMoney;                                     /* (计损后)精确服务费 */
}AswMonitorChargeData_Struct;

typedef struct
{
    uint32_t minAccountMoney;    /* 最小账户余额 0.01 元 */
}AswMonitorCtrlPara_Struct;



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void AswMonitor_InitMemory(void);
void AswMonitor_MainFunction(void);

uint8_t AswMonitor_IsOrderIdle(uint8_t port);
AswMonitorChargeData_Struct *AswMonitor_GetChargeDataPtr(uint8_t port);
AswMonitorChargeCtrl_Struct *AswMonitor_GetChargeCtrlPtr(uint8_t port);
AswMonitorBillMode_Struct *AswMonitor_GetCurUsedBillModePtr(uint8_t port);
MSNvmOrderInfo_Struct *AswMonitor_GerOrderDataPtr(uint8_t port);

uint8_t AswMonitor_CheckBillModeValid(uint8_t port);
void AswMonitor_ChargeStart(uint8_t port, uint8_t startSrc, uint8_t clearFlag);
void AswMonitor_SetReboot(AswMonitorRebootType_Enum eRebootType);
void AswMonitor_PrintChargeData(void);
void AswMonitor_SaveChargeRecord(uint8_t port, uint8_t orderSaveReason);
uint8_t AswMonitor_CheckSwipCardSuccEvent(void);
uint8_t AswMonitor_CheckSwipCardFailEvent(void);
void AswMonitor_SetMinAccountMoney(uint32_t minAccountMoney);
uint8_t AswMonitor_CheckForbidState(void);
void AswMonitor_SetForbidState(uint8_t lockState, uint8_t lockReason);
void AswMonitor_GetForbidState(uint8_t *pLockState, uint8_t *pLockReason);

#endif





















