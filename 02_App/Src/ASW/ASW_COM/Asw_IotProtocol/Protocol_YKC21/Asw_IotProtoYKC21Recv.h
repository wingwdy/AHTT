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
#ifndef ASW_IOT_PROTO_YKC21M_RECV_H_
#define ASW_IOT_PROTO_YKC21M_RECV_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/


/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
	uint16_t flow;						 
	uint8_t WarnType;				  
	uint16_t WarnId;					 
	uint8_t status;						 
	uint8_t StartTime[7];			 
	uint8_t StopTime[7];					 
    uint8_t ErrorIdx[7];
}IotYKC21Err_Struct;



typedef struct
{  
 
    uint16_t DefaultPower_max;          //当前默认最大功率 kw
    uint32_t DeaultMaxPowerStartTimess; //到达此时间后按配置最大功率执行时间戳
    uint32_t DeaultMaxPowerEndTimess;   //到达此时间后解除最大功率执行时间戳

 
    uint8_t  instruct_rsp_priority;     //指令响应优先级
    uint16_t power_running;             //运行中功率 kw
    uint16_t Limittimess;               //运行中功率执行时间
    uint32_t LimitEndtimess;            //运行中功率结束时间戳
}IotYKC21_PowerChange_Struct;




typedef struct
{  
    uint8_t Call_orderTransactionNum[16]; //交易记录召唤的订单号
    uint8_t receive_newRSAflg;            //指令下收到新的RSA密钥

}IotYKC21_CmdControl_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern IotYKC21Err_Struct ErrSendPlatform[SYSCFG_CFG_GUN_NUM];
extern IotYKC21_PowerChange_Struct IotYKC21_PowerChangeConfig[SYSCFG_CFG_GUN_NUM];
extern IotYKC21_CmdControl_Struct IotYKC21_CmdControl;




/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void IotYKC21_UpCtrlRecvDeal(void);
void IotYKC21_TimeoutDetect(void);
#endif /* ASW_IOT_PROTO_YKC21M_RECV_H_ */























