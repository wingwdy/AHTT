// #include "Iot_gn.h"
#include "protocol_data.h"
#include "cost.h"
#include "maths.h"
#include "mbsDataUpdate.h"
#include "tcp_gn.h"
#include "AppDealFlash.h"
#include "mbsMaster.h"
#include "card_user.h"
#include "AppMidDataTrans.h"
#include "iot_GN_Protocol_Code.h"
#include "iot_YKC_Protocol_Code.h"
#include "iot_ANPEI_Protocol_Code.h"
#include "iot_HNCT_Protocol_Code.h"
#include "iot_WJY_Protocol_Code.h"


static GN_PLATMOD  *ltcpCharge = &sg_platmod;	//用于本文件中


//设备型号，单双枪区分
uint8_t monitor_getDevName(uint8_t *pName, uint8_t len)
{
    memset(pName, 0, len);
    if (GUN_NUM == 1) {
        memcpy(pName, DEVICE_MODEL_CODE_ONE, strlen(DEVICE_MODEL_CODE_ONE) < len ? strlen(DEVICE_MODEL_CODE_ONE) : len);
    } else if (GUN_NUM == 2) {
        memcpy(pName, DEVICE_MODEL_CODE_TWO, strlen(DEVICE_MODEL_CODE_TWO) < len ? strlen(DEVICE_MODEL_CODE_TWO) : len);
    }
	return TRUE;
}

//平台对外接口
uint8_t UpOnlineFlag(void)
{
	if (Get_PlatConnectSta() == eOnline_Heart) {
		return 1;
	}
	return 0;
}

//连接平台桩号转换
uint8_t monitor_getDevNumber(uint8_t *pNum, uint8_t len)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;

    int fixedLength = 7; //sizeof(fixedArray);
	
    String2bin(pNum, pst_cfgInfo->pltDeviceNumber, fixedLength);

	BINToBCD(pNum, pNum, fixedLength);

	return TRUE;
}

//偏移量 -50,交流没枪温采样用插销和壳体温度代替
int32_t monitor_getGunTem(uint8_t u8Port)
{
	int32_t temp = 0;
	
	temp = ltcpCharge->gun[u8Port].gunRtInfo.plugTemp;
	
	temp += 50;
	return temp;
}
//获取充电时间s
uint32_t monitor_getChgTimer(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t temp = 0;
	
	temp = pChgGunData->chg_timer;
	
	return temp;
}

//获取实际充电电量（如果出现计量异常，为异常前的总电量）
uint32_t monitor_getChgTotalEnergy(uint8_t u8Port)
{
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	uint32_t temp = 0;

    temp = pcostdata->total_power;
	
	return temp;
}

//获取充电电量
uint32_t monitor_getChgTotalPower(uint8_t u8Port)
{
	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t temp = 0;
	
	temp = pChgGunData->chrg_ele;

	return temp;
}

//获取起始充电量
uint32_t monitor_getChgStartEle(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t temp = 0;
	
	temp = pChgGunData->total_start_elec;
	
	return temp;
}
//获取终止充电量
uint32_t monitor_getChgStoptEle(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t temp = 0;
	
	temp = pChgGunData->total_stop_elec;
	
	return temp;
}

//获取充电损耗电量
uint32_t monitor_getChgTotalLossPower(uint8_t u8Port)
{
	uint32_t temp = 0;
	
	// temp = ltcpCharge->gun[u8Port].chrgingInfo.charge_ele;
	
	return temp;
}
//获取充电金额
uint32_t monitor_getChgTotalMoney(uint8_t u8Port)
{
	COST_GUN_DATA *pRecord = &g_cost_ctrl.strCostGunData[u8Port];

	return pRecord->total_money;
}
//获取桩停止原因
uint16_t monitor_getChgStopReason(uint8_t u8Port)
{
	uint32_t temp = 0;

	temp = ltcpCharge->gun[u8Port].gunRtInfo.pileStopReason;
	
	return temp;
}

//获取充电枪故障
uint16_t get_hard_err_bit(uint8_t u8Port)
{
	uint16_t temp = 0;
	
	if(ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bit.faultEStop_1b)
	{
		SetBitFlag(&temp, 0);
	}

	if(ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bit.faultPileTemp_1b
		|| ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bit.faultPlugTemp_1b)
	{
		SetBitFlag(&temp, 2);
	}
	
	if(ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bit.faultEleMeasure_1b)
	{
		SetBitFlag(&temp, 6);
	}
	
	// if(dev_getErrExsit(u8Port, eErr_ReaderCommErr))
	// {
	// 	SetBitFlag(&temp, 7);
	// }
	
	//为了方便故障定位
	if(0 == temp) 
	{
		temp = ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bits;
		if (temp) {
			SetBitFlag(&temp, 15);
		}
	}
	
	return temp;
}
//充电枪状态
CHARGE_STATE_E logic_get_gun_state(uint8_t u8Port)
{
	return ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
}
//充电枪是否充电阶段,继电器闭合，包括暂停
uint8_t logic_get_gun_charging(uint8_t u8Port)
{
	uint8_t sta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	if ((sta > eChargeState_Starting) && (sta < eChargeState_StopFinish)) {
		return 1;
	}
	return 0;
}
//充电枪是否充电阶段,pwm发波
uint8_t logic_get_gun_pwmEnable(uint8_t u8Port)
{
	uint8_t sta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	if ((sta >= eChargeState_Starting) && (sta < eChargeState_Stoping)) {
		return 1;
	}
	return 0;
}
//充电枪是否未充电阶段
uint8_t logic_get_gun_Uncharged(uint8_t u8Port)
{
	uint8_t sta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	if ((sta >= eChargeState_Starting) && (sta < eChargeState_StopFinish)) {
		return 1;
	}
	return 0;
}
//充电枪是否待机状态
uint8_t logic_get_gun_Stanby(uint8_t u8Port)
{
	uint8_t sta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	if (sta == eChargeState_Idle) {
		return 1;
	}
	return 0;
}
//充电枪是否充电完成
uint8_t logic_get_gun_StopFinish(uint8_t u8Port)
{
	uint8_t sta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	if (sta == eChargeState_StopFinish) {
		return 1;
	}
	return 0;
}

//充电桩是否有报警
uint8_t dev_getErrState(uint8_t u8Port)
{
	uint8_t flag = FALSE;

	// if(ltcpCharge->gun[u8Port].gunRtInfo.gunWarn.bits)
	// 	flag = TRUE;
	if(ltcpCharge->gun[u8Port].gunRtInfo.hardfault.bits)
		flag = TRUE;
	
    return flag;
}

//更新桩端停止原因
void updatePileStopReason(uint8_t u8Port, uint16_t reason)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->DealRecord.PileStopReason = reason;
	printf("updatePileStopReason = %d\r\n", pChgGunData->DealRecord.PileStopReason);
}

//停止充电，并更新停止原因
uint8_t stopPileCharge(uint8_t u8Port, uint16_t reason)
{
    printf("stopPileCharge: %d,0x%x\r\n", u8Port, reason);

	//充电开始时需要清空停止原因
	if (reason != Pile_Stop_Reason_None) {
		//下发停止指令
		fgv_CtrlStopCharge(u8Port);
	}
	//更新停止原因
    updatePileStopReason(u8Port, reason);

    return 0;
}

typedef struct
{
    uint8_t		pileDefault;
	uint8_t		pileStopReason;
}Pile_StopReasonMap;

// 桩停止码对应停止原因对应表
const Pile_StopReasonMap StrPileStopReasonMap[] = {
	{eErr_CCUSCUCommErr		    ,Pile_Stop_Reason_Comm              },
	{eErr_LeakageCurrErr		,Pile_Stop_Reason_Leak              },
	{eErr_EmergencyStop		    ,Pile_Stop_Reason_EStop             },
	{eErr_CpGroundFault		    ,Pile_Stop_Reason_CPGnd             },
	{eErr_CpVoltAbnor		    ,Pile_Stop_Reason_CPErro            },
	{eErr_PEBreakFault	        ,Pile_Stop_Reason_PEGnd             },
	{eErr_AphaseInputOverVol	,Pile_Stop_Reason_VolOver           },
	{eErr_BphaseInputOverVol	,Pile_Stop_Reason_VolOver           },
	{eErr_CphaseInputOverVol	,Pile_Stop_Reason_VolOver           },
	{eErr_AphaseInputLessVol	,Pile_Stop_Reason_VolUnder          },
	{eErr_BphaseInputLessVol	,Pile_Stop_Reason_VolUnder          },
	{eErr_CphaseInputLessVol	,Pile_Stop_Reason_VolUnder          },
	{eErr_OutputOverCurr	    ,Pile_Stop_Reason_CrtUnder         },
	{eErr_JcqMaloperation	    ,Pile_Stop_Reason_RlyRfs          },
	{eErr_JcqSynechiaFault	    ,Pile_Stop_Reason_RlySyn            },
	{eErr_EnvOverTempErr	    ,Pile_Stop_Reason_AirTempOver       },
	{eErr_GunOverTempErr	    ,Pile_Stop_Reason_GunTempOver       },
	{eErr_POverTempErr	        ,Pile_Stop_Reason_PlugTempOver      },
	{eErr_ShortCircleErro	    ,Pile_Stop_Reason_ShortCircle      },
	{eSrc_AutoStop	            ,Pile_Stop_Reason_CarOk             },
	{eErr_GunDisConnErr	        ,Pile_Stop_Reason_GunBreak          },
	{eErr_DiodeStop	            ,Pile_Stop_Reason_StartDiode        },
	{eErr_MeterCommErr	        ,Pile_Stop_Reason_EleCommFault      },
	{eErr_MeterCalcErr	        ,Pile_Stop_Reason_Ele      },
	{eSrc_SetKeyStop	        ,Pile_Stop_Reason_StopKey           },
};


//桩本体的停止原因，不关乎平台，平台可以从这个函数里面去转换
void Update_PileStopReason(uint8_t u8Port)
{
    //充电中没任何停止原因，
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	//桩正常充电中，网络这边的状态有故障停止
	if (pChgGunData->DealRecord.PileStopReason) {
		return;
	}

	//获取桩停止原因，并转换
	uint16_t stopRs = monitor_getChgStopReason(u8Port);
	const Pile_StopReasonMap *pPileStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrPileStopReasonMap); u32i++) {
        pPileStopMap = &StrPileStopReasonMap[u32i];
        if (stopRs == pPileStopMap->pileDefault) {
            pChgGunData->DealRecord.PileStopReason = pPileStopMap->pileStopReason;
            printf("Pile_stop reason: %d %d\r\n", pPileStopMap->pileDefault, pPileStopMap->pileStopReason);
            return;
        }
    }
	if (stopRs != eErr_none) {
		printf("\r\nPile_stop reason else: 0x%x\r\n", stopRs);
		pChgGunData->DealRecord.PileStopReason = Pile_Stop_Reason_Other;
	}

	printf("\r\nUpdate_PileStopReason %d\r\n", pChgGunData->DealRecord.PileStopReason);
}



//桩停止原因对应公牛平台需要上报的停止原因
const Pile_StopReasonMap StrGnStopReasonMap[] = {
	{Pile_Stop_Reason_Comm		        ,Reason_StopComm              },    //充电单元通信故障
	{Pile_Stop_Reason_Leak		        ,Reason_StopLeak              },    //漏电故障
	{Pile_Stop_Reason_EStop		        ,Reason_Stop_EmergencyStop    },    //急停
	{Pile_Stop_Reason_CPGnd		        ,Reason_StopCPGnd             },    //CP接地
	{Pile_Stop_Reason_CPErro		    ,Reason_StopCPErro            },    //CP异常
	{Pile_Stop_Reason_PEGnd	            ,Reason_StoPEGnd              },    //PE接地故障
	{Pile_Stop_Reason_VolOver	        ,Reason_Stop_ErrVal           },    //电压异常
	{Pile_Stop_Reason_VolUnder	        ,Reason_Stop_ErrVal           },    //电压异常
	{Pile_Stop_Reason_CrtUnder	        ,Reason_Stop_ErrCurr          },    //过流
	{Pile_Stop_Reason_RlyRfs	        ,Reason_StopRelayMissTrip     },    //拒动
	{Pile_Stop_Reason_RlySyn	        ,Reason_StopRelayCgltnt       },    //粘连
	{Pile_Stop_Reason_AirTempOver	    ,Reason_Stop_TmpErr           },
	{Pile_Stop_Reason_GunTempOver	    ,Reason_Stop_TmpErr           },
	{Pile_Stop_Reason_PlugTempOver	    ,Reason_Stop_TmpErr           },
	{Pile_Stop_Reason_CarOk	            ,Reason_Finish_Soc             },   //自动停止，S2断开
	{Pile_Stop_Reason_GunBreak	        ,Reason_Stop_GunBreak          },   //导引断开
	{Pile_Stop_Reason_StartDiode	    ,Reason_StartDiode             },   //二极管检测无，新国标
	{Pile_Stop_Reason_EleCommFault	    ,Reason_Stop_MeterErr          },   //电表通信异常
	{Pile_Stop_Reason_StopKey	        ,Reason_StopSetKey             },
	{Pile_Stop_Reason_S2TimeOut	        ,Reason_StartTimeout           },
    //以下非故障停止
	{Pile_Stop_Reason_Card	            ,Reason_Finish_Manual          },
	{Pile_Stop_Reason_Ele	            ,Reason_StopEleErro            },
	{Pile_Stop_Reason_Money	            ,Reason_StopMoneyErro          },
	{Pile_Stop_Reason_APP	            ,Reason_Finish_App             },
	{Pile_Stop_Reason_OverBalance	    ,Reason_Stop_SumNoEnough       },
	{Pile_Stop_Reason_OverTime	        ,Reason_Finish_Time            },
	{Pile_Stop_Reason_OverSum	        ,Reason_Finish_Sum             },
	{Pile_Stop_Reason_OverEle	        ,Reason_Finish_Ele             },
	{Pile_Stop_Reason_PwOff	            ,Reason_Interupt_PwOff         },
    //无法判断故障
	{Pile_Stop_Reason_None	            ,Reason_UnKnow                 },
	{Pile_Stop_Reason_Other		        ,Reason_UnKnow                 },
};

//上传订单时更新停止原因,获取桩的停止原因
uint16_t gnUpdate_stopReason(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t stopRs = pChgGunData->DealRecord.PileStopReason;
	uint16_t gnStopRs = 0;

    const Pile_StopReasonMap *pPileStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrGnStopReasonMap); u32i++) {
        pPileStopMap = &StrGnStopReasonMap[u32i];
        if (stopRs == pPileStopMap->pileDefault) {
            printf("Pile_stop reason: %d, gn stop reason: %d\r\n", pPileStopMap->pileDefault, pPileStopMap->pileStopReason);
            gnStopRs = pPileStopMap->pileStopReason;
            return gnStopRs;
        }
    }

    printf("Else OgrReason: %d\r\n", stopRs);
    gnStopRs = stopRs;  
    return gnStopRs;
}

//桩停止原因对应云快充2.1平台需要上报的停止原因
const Pile_StopReasonMap StrYkcV21StopReasonMap[] = {
	{Pile_Stop_Reason_EStop		        ,YKC21_Reason_Stop_EmergencyStop    	},  //急停
	{Pile_Stop_Reason_GunTempOver	    ,YKC21_Reason_Stop_TmpErr            	},	//枪头过温
	{Pile_Stop_Reason_VolOver	        ,YKC21_Reason_Stop_ErrVal           	},  //过压
	{Pile_Stop_Reason_CrtUnder	        ,YKC21_Reason_Stop_ErrCurr           	},  //过流
	{Pile_Stop_Reason_EleCommFault	    ,YKC21_Reason_Stop_MeterErr          	},  //电表通信异常

	{Pile_Stop_Reason_GunBreak	        ,YKC21_Reason_Stop_GunBreak          	},  //导引断开
	{Pile_Stop_Reason_APP	            ,YKC21_Reason_Finish_App             	},	//APP远程停止
	{Pile_Stop_Reason_CarOk	            ,YKC21_Reason_Finish_Soc             	},  //自动停止，S2断开

	{Pile_Stop_Reason_OverEle	        ,YKC21_Reason_Finish_Ele             	},	//本次请求充电电量不足停止
	{Pile_Stop_Reason_OverSum	        ,YKC21_Reason_Finish_Sum             	},	//本次请求充电金额不足停止
	{Pile_Stop_Reason_OverTime	        ,YKC21_Reason_Finish_Time            	},	//本次请求充电时间不足停止
	{Pile_Stop_Reason_Card	            ,YKC21_Reason_Finish_Manual          	},	//刷卡停止
	{Pile_Stop_Reason_OverBalance	    ,YKC21_Reason_Stop_SumNoEnough       	},	//总账户余额不足停止
	{Pile_Stop_Reason_PwOff	            ,YKC21_Reason_Interupt_PwOff         	},	//充电异常中止，充电设备断电

    //无法判断故障
	{Pile_Stop_Reason_None	            ,YKC21_Reason_UnKnow                 	},
	{Pile_Stop_Reason_Other		        ,YKC21_Reason_UnKnow                 	},	//其他原因
};

//云快充2.1上传订单时更新停止原因,获取桩的停止原因
uint16_t ykcV21Update_stopReason(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t stopRs = pChgGunData->DealRecord.PileStopReason;
	uint16_t ykcStopRs = 0;

    const Pile_StopReasonMap *pPileStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrYkcV21StopReasonMap); u32i++) {
        pPileStopMap = &StrYkcV21StopReasonMap[u32i];
        if (stopRs == pPileStopMap->pileDefault) {
            printf("ykc2.1_Pile_stop reason: %02x, ykc2.1 stop reason: %02x\r\n", pPileStopMap->pileDefault, pPileStopMap->pileStopReason);
            ykcStopRs = pPileStopMap->pileStopReason;
            return ykcStopRs;
        }
    }
	if (stopRs)		//如果停止原因不为0且平台无对应停止原因，偏移0x90上报
	{
		stopRs = stopRs + 0x90;
	}

    printf("ykc2.1 Else OgrReason: %d\r\n", stopRs);
    ykcStopRs = stopRs;  
    return ykcStopRs;
}

uint8_t monitor_charge_stop(uint8_t u8Port)
{
	if(ltcpCharge->gun[u8Port].gunRtInfo.u_platChrgsta.bit.charging_1b)
    	return FALSE;
	
    return TRUE;
}


/*------------------------------------------
-------------------------------------------*/
MONITOR_STATE_E monitor_get_MonitorState(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	return pChgGunData->eMOnitorState;
}

void monitor_set_MonitorState(uint8_t u8Port, MONITOR_STATE_E state)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	pChgGunData->eMOnitorState = state;

	// debug_printf("\r\nmonitor -->[%d] State set: %d !", u8Port, state);
}


void monitor_setTradeFlag(uint8_t u8Port, uint8_t flag)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	pChgGunData->trade_flag = flag;
	
	return;
}
uint8_t monitor_getTradeFlag(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	return pChgGunData->trade_flag;
}

//设置充电鉴权卡号
uint8_t monitor_setChgCardNum(uint8_t u8Port, uint8_t *pNum, uint8_t len)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint8_t card_len = len > GNDATA_CARD_LEN ? GNDATA_CARD_LEN:len;
	
    memcpy(pChgGunData->Auth_card_number, pNum, card_len);

	return TRUE;
}
//获取充电鉴权卡号、卡面卡号
uint8_t monitor_getChgCardNum(uint8_t u8Port, uint8_t *pNum, uint8_t len)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    memcpy(pNum, pChgGunData->Auth_card_number, len);
	
	return TRUE;
}

void monitor_chgrcd_report_succ(void)
{
	
	// DataFlashWrite_SamplchgRcd(pSampleRcdCtrl);
}

void UpAskRateModel(uint8_t u8Port)
{
	if(TRUE != UpOnlineFlag()) return;

	SetSendEnable(u8Port, CMD_Request_billing_model, SEND_ENABLE_ON);
	Send_Immediately(u8Port, CMD_Request_billing_model);
	
	return;
}

//插抢时间过长不允许充电
void ChargeConnectGunTimeScan(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	static uint32_t connectTick[GUN_NUM_MAX] = {0};

	if (GetPile_gun_connect(u8Port) == 0) {
		connectTick[u8Port] = Get_Systick();
	}

	if(ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta != eChargeState_Waiting)
	{
		return;
	}

	if (GetPile_gun_connect(u8Port)) {
		//插抢超时1分钟不可启动
		if (JudgeTimeOutMs(connectTick[u8Port], eTick_60S)) {
			fgv_CtrlStopCharge(u8Port);
		}
	}
}

// 车辆15s不启动，充电完成, 订单上报
void StartingTimeOutScan(uint8_t u8Port)
{
	static uint32_t CtnTick[GUN_NUM_MAX] = {0};
    uint32_t S2_timetick = eTick_15S;
	uint8_t crtSta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;

	if (crtSta != eChargeState_Starting) {
		CtnTick[u8Port] = NOWTICK;
		return;
	}

	if (ePlatType_ANPEI == get_ChgParam_plat_type())
	{
		S2_timetick = 3*eTick_30S;
	}
	else if (ePlatType_AHTT == get_ChgParam_plat_type())
	{
		S2_timetick = eTick_30S;
	}
	else
	{
		S2_timetick=eTick_15S;
	}

	if (JudgeTimeOutMs(CtnTick[u8Port], S2_timetick)) {
		stopPileCharge(u8Port, Pile_Stop_Reason_S2TimeOut);
		CtnTick[u8Port] = NOWTICK;
	}
}

// 暂停超时，s1闭合后，s2是否需要断开，根据平台定义
void PauseTimeOutScan(uint8_t u8Port)
{
	static uint32_t CtnTick[GUN_NUM_MAX] = {0};
	uint8_t crtSta = ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta;
	uint32_t timeOutTick = eTick_60S;
	
	if (ePlatType_AHTT == get_ChgParam_plat_type())
	{
		timeOutTick = eTick_10S;
	}

	if (crtSta != eChargeState_PauseB) {
		CtnTick[u8Port] = NOWTICK;
		return;
	}
	if (JudgeTimeOutMs(CtnTick[u8Port], timeOutTick)) {
		stopPileCharge(u8Port, Pile_Stop_Reason_CarOk);
		CtnTick[u8Port] = NOWTICK;
	}
}


//刷卡鉴权失败或者拔枪，清除刷卡信息
void g_CardInfoClear(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    memset(pChgGunData->LogicCard_number, 0, GNDATA_CARD_LEN);
    memset(pChgGunData->PhyCard_number, 0, GNDATA_PHYCARD_LEN);
    memset(pChgGunData->Auth_card_number, 0, GNDATA_CARD_LEN);
    memset(pChgGunData->Swip_PhyCard_number, 0, GNDATA_PHYCARD_LEN);
}
/*************************************************************
 * 插抢刷卡进行鉴权充电，或者刷卡停止充电
 *************************************************************/
void CardStartCharge(uint8_t u8Port)
{
	IC_T *pPlat_ic = Get_IC_Msg(u8Port);
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (logic_get_gun_Uncharged(u8Port)) {
		return;
	}
	// if (logic_get_gun_StopFinish(u8Port)) {
	// 	return;
	// }
    
	printf("\r\nUser CardStartCharge\r\n");
	
	//刷卡卡号
	memcpy(pChgGunData->LogicCard_number, &pPlat_ic->LogicNumbers[8], GNDATA_CARD_LEN);
	memcpy(pChgGunData->PhyCard_number, pPlat_ic->SeriNumbers, GNDATA_PHYCARD_LEN);

	if((ePlatType_GN == get_ChgParam_plat_type())
    || (ePlatType_GNP == get_ChgParam_plat_type())) {
        GN_CardAuthStart_Cmd(u8Port);
    } else if(ePlatType_YKC_V2 == get_ChgParam_plat_type()) {
		YKC_CardAuthStart_Cmd_V2(u8Port);
	} else if(ePlatType_YKC == get_ChgParam_plat_type()
	        || ePlatType_gwYKC == get_ChgParam_plat_type()
            || ePlatType_DD == get_ChgParam_plat_type()
            || ePlatType_TOWER == get_ChgParam_plat_type()) {
        YKC_CardAuthStart_Cmd(u8Port);
    } else if(ePlatType_AHTT == get_ChgParam_plat_type()) {
        AHTT_CardAuthStart_Cmd(u8Port);
    } else if(ePlatType_HaiNCT == get_ChgParam_plat_type()) {
        HaiNCT_CardAuthStart_Cmd(u8Port);
	} else if(ePlatType_WJY == get_ChgParam_plat_type()) {
        WJY_CardAuthStart_Cmd(u8Port);	
    }
}
void CardStopCharge(uint8_t u8Port)
{
	IC_T *pPlat_ic = Get_IC_Msg(u8Port);
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	//刷卡充电才可以刷卡停止
	if (monitor_getTradeFlag(u8Port) != eUP_Start_Style_CardOnline) {
		return;
	}
	if (logic_get_gun_pwmEnable(u8Port) == 0) {
		return;
	}
	//判断卡号是否一致
	if (memcmp(pPlat_ic->SeriNumbers, pChgGunData->Swip_PhyCard_number, GNDATA_PHYCARD_LEN) != 0) {
		return;
	}
	
	//停止充电
	fgv_CtrlStopCharge(u8Port);

	stopPileCharge(u8Port, Pile_Stop_Reason_Card);

	printf("\r\nUser CardStopCharge\r\n");
}

//存在一次刷卡动作，并非一直存在
uint8_t GetChargeExistSwipingCard(uint8_t u8Port)
{
    //避免刷卡开始充电卡不拿开一直鉴权的情况
    static uint8_t l_CardOper[GUN_NUM_MAX] = {0};	//0不操作，1新的一次刷卡动作
    static uint8_t l_ExistSwiping[GUN_NUM_MAX] = {0};	//0不操作，1新的一次刷卡动作

    if (Get_IC_ValidCard(u8Port) == 0) {
        l_CardOper[u8Port] = 0;
        l_ExistSwiping[u8Port] = 0;
    } else {
        if (l_CardOper[u8Port] == 0) {
            l_ExistSwiping[u8Port] = 1; //存在刷卡
        } else {
            l_ExistSwiping[u8Port] = 0;
        }
        l_CardOper[u8Port] = 1;
    }
    if (l_ExistSwiping[u8Port] == 1) {
        return 1;
    }
    return 0;
}


void ChargeConnectGunWaitCardScan(uint8_t u8Port)
{
	//避免刷卡开始充电卡不拿开的情况
	static uint8_t l_CardOper[GUN_NUM_MAX] = {0};	//0不操作，1新的一次刷卡动作

	if (GetPile_gun_connect(u8Port) == 0) {
		return;
	}
	if (Get_IC_ValidCard(u8Port) == 0) {
		l_CardOper[u8Port] = 1;
		return;
	}

	if (l_CardOper[u8Port] == 0) {
		return;
	}

	//平台连接正常之后可进行充电鉴权
	if(TRUE == UpOnlineFlag()) {
		CardStartCharge(u8Port);
	}

	CardStopCharge(u8Port);
	
	l_CardOper[u8Port] = 0;
}

/*------------------------------*/
/*----------------------------
*up_fail_reason out 失败原因
*pCardNo in 卡号
*pTrdNum in 记录流水号
*BlMoney in 账户余额
-------------------------------------------*/
uint8_t monitor_charge_start(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    g_ClearChargingInfo(u8Port);

	//开始充电，清零停止原因
	stopPileCharge(u8Port, Pile_Stop_Reason_None);

	//一次充电完成后不允许再次开启充电，需要重新插拔枪
	// if(ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta == eChargeState_StopFinish)
	// {
	// 	up_fail_reason[0] = eUP_Start_Fail_Reconnect;
	// 	return FALSE;
	// }
    
    //订单进行中不进行充电处理，避免平台在上一笔订单传输前下发启动充电
	if(pChgGunData->ExistChargeDeal)
	{
		up_fail_reason[0] = eUP_Start_Fail_Reconnect;
        printf("monitor_charge_start---last order not been completed\r\n");
		return FALSE;
	}
	
	if ((ePlatType_AHTT == get_ChgParam_plat_type()) && (trade_flag == eUP_Start_Style_CardOnline))
	{/*安徽铁塔刷卡策略：刷卡先立即开启充电，10秒后首次向平台发起鉴权时，桩可能已经在充电中，因此不判断是否在充电中*/
		;
	}
	else
	{
		if (ltcpCharge->gun[u8Port].gunRtInfo.u_platChrgsta.bit.charging_1b)
		{
			up_fail_reason[0] = eUP_Start_Fail_Working;
			return FALSE;
		}
	}

	if(eMonitorState_Auth == monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Working;
		return FALSE;
	}

	if(eMonitorState_Forbid == monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Offline;
		return FALSE;
	}
	
	if(eMonitorState_UpState == monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Offline;
		return FALSE;
	}

	if(eMonitorState_Service != monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Working;
		return FALSE;
	}
	
	if(TRUE == dev_getErrState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_DevErr;
		return FALSE;
	}
	
	if(TRUE != GetPile_gun_connect(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_NoConn;
		return FALSE;
	}

	//计费模型校验失败,请求计费模型
	if(TRUE != CostGetRateModel(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Rate;
        printf("monitor_charge_start---计费模型异常\r\n");
		return FALSE;
	}

	getRunTimeYYMDHMS(pChgGunData->chrg_start_time);

	// if(TRUE == logic_charge_start(u8Port))
	{
		memcpy(pChgGunData->Swip_PhyCard_number, pChgGunData->PhyCard_number, GNDATA_PHYCARD_LEN);
		memset(pChgGunData->transaction_log_num, 0, GNDATA_TRDNUM_LEN);
		pChgGunData->sum_balance = 0;
        pChgGunData->ExistChargeDeal = 1;   //订单开始

		monitor_setTradeFlag(u8Port, trade_flag);
		
		if(NULL != pCardNo)
			monitor_setChgCardNum(u8Port, pCardNo, GNDATA_CARD_LEN);
		if(NULL != pTrdNum)
		{
			memcpy(pChgGunData->transaction_log_num, pTrdNum, GNDATA_TRDNUM_LEN);
		}
		printf("\r\n monitor_charge_start,transaction_log_num\r\n");

		for (size_t i = 0; i < GNDATA_TRDNUM_LEN; i++)
		{
			printf("%02x ", pChgGunData->transaction_log_num[i]);
		}
		
		if(NULL != BlMoney)
		{
			pChgGunData->sum_balance = BlMoney[0];
		}
		// monitor_chargeRecordSave(u8Port, CHARGE_RECORD_START);
		return TRUE;
	}
	
	return FALSE;
}



//平台重连
void UpOfflineDeal(eNetSocket SocketID)
{
    Plat_Reconnect(SocketID);
    //socket重连
	Comm_PlatReconnect(SocketID, __LINE__);
}
//运营平台重连
void DB_UpOfflineDeal()
{
    //数据清除
    // if (Get_PlatConnectSta() == eOnline_Off) {
    //     return;
    // }
    UpOfflineDeal(eSocket_GPRS1);
}



//充电信息实时更新
void ChargingRecordUpdateScan(uint8_t u8Port)
{
	if (logic_get_gun_Uncharged(u8Port) == 0) {
        return;
	}
    //其他时间不需要更新充电时间
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    
	memcpy(pChgGunData->chrg_start_time, ltcpCharge->gun[u8Port].chrgingInfo.start_time, 7);
	memcpy(pChgGunData->chrg_stop_time, ltcpCharge->gun[u8Port].chrgingInfo.stop_time, 7);

	pChgGunData->total_start_elec = GetPile_ChgStartEle(u8Port);
	//总止电量=总起电量+充电电量
	pChgGunData->total_stop_elec = pChgGunData->total_start_elec + monitor_getChgTotalEnergy(u8Port);
	//pChgGunData->total_stop_elec = GetPile_ChgStoptEle(u8Port);
	pChgGunData->chrg_ele = GetPile_ChgTotalPower(u8Port);
	pChgGunData->chg_timer = GetPile_ChgTimer(u8Port);
}

//订单数据更新
static void ykc_packStopReasonChgRecord(uint8_t u8Port, charge_record_ykcv2 *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    U8 pileReason = pChgGunData->DealRecord.PileStopReason;
    if (pileReason == Reason_Updata) {
        pRecord->stop_reason = Reason_Interupt_PwOff;
    } else {
        pRecord->stop_reason = pileReason;  
    }
	printf("ykc_packStopReasonChgRecord,stop_reason = %d\r\n",pRecord->stop_reason);
}

//订单数据更新
static void gn_packChgRecord(uint8_t u8Port, charge_record *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];

	monitor_getDevNumber(pRecord->device_number, sizeof(pRecord->device_number));
	
	pRecord->gun_num = u8Port+1;
	
	memcpy(pRecord->transaction_log_num, pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);	 //交易流水号
	
	BINToBCD(pRecord->chrg_start_time, pChgGunData->chrg_start_time, 7);
	
	BINToBCD(pRecord->chrg_stop_time, pChgGunData->chrg_stop_time, 7);

	uint32ToFourUint8(pRecord->sharp_rate, pcostdata->ele_rate[0]);
	uint32ToFourUint8(pRecord->sharp_power, pcostdata->ele_power[0]);
	uint32ToFourUint8(pRecord->sharp_loss_power, pcostdata->ele_loss_power[0]);
	uint32ToFourUint8(pRecord->sharp_money, pcostdata->perMoney[0]);
	
	uint32ToFourUint8(pRecord->peak_rate, pcostdata->ele_rate[1]);
	uint32ToFourUint8(pRecord->peak_power, pcostdata->ele_power[1]);
	uint32ToFourUint8(pRecord->peak_loss_power, pcostdata->ele_loss_power[1]);
	uint32ToFourUint8(pRecord->peak_money, pcostdata->perMoney[1]);
	
	uint32ToFourUint8(pRecord->flat_rate, pcostdata->ele_rate[2]);
	uint32ToFourUint8(pRecord->flat_power, pcostdata->ele_power[2]);
	uint32ToFourUint8(pRecord->flat_loss_power, pcostdata->ele_loss_power[2]);
	uint32ToFourUint8(pRecord->flat_money, pcostdata->perMoney[2]);
	
	uint32ToFourUint8(pRecord->valley_rate, pcostdata->ele_rate[3]);
	uint32ToFourUint8(pRecord->valley_power, pcostdata->ele_power[3]);
	uint32ToFourUint8(pRecord->valley_loss_power, pcostdata->ele_loss_power[3]);
	uint32ToFourUint8(pRecord->valley_money, pcostdata->perMoney[3]);

	if(pRecord->order_model_type == RATE_MODEL_9_TYPE)
	{
		uint32ToFourUint8(pRecord->deep_valley_rate, pcostdata->ele_rate[4]);
		uint32ToFourUint8(pRecord->deep_valley_power, pcostdata->ele_power[4]);
		uint32ToFourUint8(pRecord->deep_valley_loss_power, pcostdata->ele_loss_power[4]);
		uint32ToFourUint8(pRecord->deep_valley_money, pcostdata->perMoney[4]);

		uint32ToFourUint8(pRecord->six_rate, pcostdata->ele_rate[5]);
		uint32ToFourUint8(pRecord->six_power, pcostdata->ele_power[5]);
		uint32ToFourUint8(pRecord->six_loss_power, pcostdata->ele_loss_power[5]);
		uint32ToFourUint8(pRecord->six_money, pcostdata->perMoney[5]);

		uint32ToFourUint8(pRecord->seven_rate, pcostdata->ele_rate[6]);
		uint32ToFourUint8(pRecord->seven_power, pcostdata->ele_power[6]);
		uint32ToFourUint8(pRecord->seven_loss_power, pcostdata->ele_loss_power[6]);
		uint32ToFourUint8(pRecord->seven_money, pcostdata->perMoney[6]);

		uint32ToFourUint8(pRecord->eight_rate, pcostdata->ele_rate[7]);
		uint32ToFourUint8(pRecord->eight_power, pcostdata->ele_power[7]);
		uint32ToFourUint8(pRecord->eight_loss_power, pcostdata->ele_loss_power[7]);
		uint32ToFourUint8(pRecord->eight_money, pcostdata->perMoney[7]);

		uint32ToFourUint8(pRecord->nine_rate, pcostdata->ele_rate[8]);
		uint32ToFourUint8(pRecord->nine_power, pcostdata->ele_power[8]);
		uint32ToFourUint8(pRecord->nine_loss_power, pcostdata->ele_loss_power[8]);
		uint32ToFourUint8(pRecord->nine_money, pcostdata->perMoney[8]);
	}

	uint32ToFourUint8(pRecord->total_start_elec, pChgGunData->total_start_elec);
	uint32ToFourUint8(pRecord->total_stop_elec, pChgGunData->total_stop_elec);
	uint32ToFourUint8(pRecord->total_power, pcostdata->total_power);
	uint32ToFourUint8(pRecord->total_loss_power, pcostdata->total_loss_power);
	uint32ToFourUint8(pRecord->total_money, pcostdata->total_money);
	
	memset(pRecord->vin, 0, 17);
	
	pRecord->trade_flag = monitor_getTradeFlag(u8Port);
	
	BINToBCD(pRecord->trade_time, pChgGunData->chrg_stop_time, 7);

	monitor_getChgCardNum(u8Port, pRecord->card_number, GNDATA_CARD_LEN);

    pRecord->stop_reason = gnUpdate_stopReason(u8Port);      //公牛停止原因更新

	return;
}

//云快充2.1 订单数据更新
static void ykc_v2_packChgRecord(uint8_t u8Port, charge_record_ykcv2 *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
	YKC_V2_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvStartCharge;
	YKC_V2_Recv_Auth_Ack *pRecvAuth = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvAuthAck;
	uint32_t ele_limit_plat = 0;
	uint32_t ele_limit_card = 0;

	monitor_getDevNumber(pRecord->device_number, sizeof(pRecord->device_number));
	
	pRecord->gun_num = u8Port+1;
	
	memcpy(pRecord->transaction_log_num, pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);	 //交易流水号
	
	BINToBCD(pRecord->chrg_start_time, pChgGunData->chrg_start_time, 7);
	
	BINToBCD(pRecord->chrg_stop_time, pChgGunData->chrg_stop_time, 7);

	pRecord->fee_num = prate->rateNumber;
	for (uint8_t i = 0; i < pRecord->fee_num; i++)
	{
		uint32ToFourUint8(pRecord->fee_rate[i], pcostdata->ele_rate[i]);
		uint32ToFourUint8(pRecord->fee_ele[i], pcostdata->ele_power[i]);
		uint32ToFourUint8(pRecord->loss_fee_ele[i], pcostdata->ele_loss_power[i]);
	}
	for (int i = 0; i < 48; i++)	//时段恒为48
	{
		uint32ToFourUint8(pRecord->time_power[i], pcostdata->PeriodElePower[i]);
	}
    
	uint32ToFourUint8(pRecord->total_start_elec, pChgGunData->total_start_elec);
	uint32ToFourUint8(pRecord->total_stop_elec, pChgGunData->total_stop_elec);
	uint32ToFourUint8(pRecord->total_power, pcostdata->total_power);
	uint32ToFourUint8(pRecord->total_loss_power, pcostdata->total_loss_power);
	uint32ToFourUint8(pRecord->total_money, pcostdata->total_money);
	
	memset(pRecord->vin, 0, 17);
	
	pRecord->trade_flag = monitor_getTradeFlag(u8Port);
	
	BINToBCD(pRecord->trade_time, pChgGunData->chrg_stop_time, 7);

	reverse(&pChgGunData->PhyCard_number, &pRecord->card_number, GNDATA_PHYCARD_LEN);		//与平台交互用物理卡号

	pRecord->stop_reason = ykcV21Update_stopReason(u8Port);      //云快充2.1停止原因更新

	printf("\r\n ykc_v2_packChgRecord,stop_reason = 0x%02x\r\n",pRecord->stop_reason);
	if (pRecord->stop_reason == YKC21_Reason_Finish_Ele)
	{
		ele_limit_plat = fourUint8ToUint32LH(pRecvStartCharge->chg_ele_limit);		//app启动电量限制
		ele_limit_card = fourUint8ToUint32LH(pRecvAuth->Charge_capacity_limit);		//卡启动电量限制
		if (ele_limit_plat || ele_limit_card)		//如果有电量限制则对停止原因进行修改		
		{
			pRecord->stop_reason = YKC21_Reason_Limit_Ele;
			printf("\r\n ykc_v2_packChgRecord: YKC21_Reason_Limit_Ele = 0x%02x\r\n", YKC21_Reason_Limit_Ele);
		}		
	}

	return;
}

uint8_t GetPlatRecordData(uint8_t u8Port)
{
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
	uint8_t ret = TRUE;

    Update_PileStopReason(u8Port);

    if (ePlatType_GN == get_ChgParam_plat_type()
    || ePlatType_GNP == get_ChgParam_plat_type()
    || ePlatType_YKC == get_ChgParam_plat_type()
	|| ePlatType_gwYKC == get_ChgParam_plat_type()
    || ePlatType_TOWER == get_ChgParam_plat_type()
    || ePlatType_DD == get_ChgParam_plat_type()) {
        gn_packChgRecord(u8Port, &UpRecord->GnChgRecord);
    }
	else if(ePlatType_ANPEI== get_ChgParam_plat_type())//JJUNIVE anpei平台
	{
        anpei_packChgRecord(u8Port, &UpRecord->AnpeiChgRecord);
    }
    else if(ePlatType_AHTT== get_ChgParam_plat_type())
	{
        ret = ahtt_packChgRecord(u8Port, &UpRecord->AhttChgRecord);
    } else if (ePlatType_YKC_V2 == get_ChgParam_plat_type()) {
		ykc_v2_packChgRecord(u8Port,&UpRecord->YkcRecord);
    } else if (ePlatType_HaiNCT == get_ChgParam_plat_type()) {
		HaiNCT_packChgRecord(u8Port,&UpRecord->HaiNCTChgRecord);
    } else if (ePlatType_WJY == get_ChgParam_plat_type()) {
		wjy_packChgRecord(u8Port,&UpRecord->WjyChgRecord);
    }

	return ret;
}

void PlatCmdDealUpdateEnable(uint8_t u8Port)
{
    if ((ePlatType_GN == get_ChgParam_plat_type())
      ||(ePlatType_GNP == get_ChgParam_plat_type())) {
        GN_DealUpdate_Cmd(u8Port);
    } else if (ePlatType_YKC == get_ChgParam_plat_type()
	        ||ePlatType_gwYKC == get_ChgParam_plat_type()
            || ePlatType_TOWER == get_ChgParam_plat_type()
            || ePlatType_DD == get_ChgParam_plat_type()) {
        YKC_DealUpdate_Cmd(u8Port);
    } else if(ePlatType_ANPEI== get_ChgParam_plat_type()){
		ANPEI_DealUpdate_Cmd(u8Port);
	} else if(ePlatType_AHTT== get_ChgParam_plat_type()){
		AHTT_DealUpdate_Cmd(u8Port);
	} else if(ePlatType_YKC_V2== get_ChgParam_plat_type()){
		YKC_DealUpdate_Cmd_V2(u8Port);
	} else if(ePlatType_HaiNCT== get_ChgParam_plat_type()){
		HaiNCTDealUpdate_Cmd(u8Port);
	}else if(ePlatType_WJY== get_ChgParam_plat_type()){
		WJY_DealUpdate_Cmd(u8Port);
	}
}

//充电中订单存储
uint8_t UpChargeRecordingStorage(uint8_t u8Port, uint8_t imd)
{
    uint8_t l_strg = imd;
    
	static uint32_t start_tick[GUN_NUM_MAX] = {0};
	PlatDealRecord *UpRecord = &g_chgData[u8Port].DealRecord;
    
    if (logic_get_gun_charging(u8Port))
    {
        //1分钟存储一次
        if(JudgeTimeOutMs(start_tick[u8Port], eTick_60S) == TRUE)
        {  
            l_strg = 1;
            printf("gun %d 1min charging storage...\r\n", u8Port);
        }
    }

    if (l_strg) {
		if (TRUE == GetPlatRecordData(u8Port))
		{
			DealData_write(u8Port, (uint8_t *)UpRecord, sizeof(PlatDealRecord));
		}
        start_tick[u8Port] = Get_Systick();
    }

	return 0;
}

//0x40应答之后清除掉，避免重新上报继续上报
uint8_t GNUpChargeStorageDeal(uint8_t u8Port, void *deal, uint16_t len)
{

	DealData_write(u8Port, (uint8_t *)deal, len);

    return TRUE;
}

//设置计费模式和参数，平台启动充电后调用；
void SetDetectModeParam(uint8_t u8Port, eDetectMode mode, uint32_t param)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    pChgGunData->gun_chrg_mode = mode;
    pChgGunData->gun_chrg_mode_param = param;
    printf("SetDetectModeParam: %d %d\r\n", pChgGunData->gun_chrg_mode, pChgGunData->gun_chrg_mode_param);
}
void SetDetectChargeMaxTime(uint8_t u8Port, uint32_t param)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    pChgGunData->chargeMaxTime = param;
}


//余额检测，上位机下发余额更新指令，需要根据充电的具体情况实时检测当前余额是否足够充电，余额不足则停止充电
int account_balance_detect(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    //已充金额
    U32 money = monitor_getChgTotalMoney(u8Port) / 100;  //小数点后四位to小数点后两位
    int money_diff = pChgGunData->sum_balance - money;
	
    static uint16_t last_print_time = 0;
    uint16_t current_time = monitor_getChgTimer(u8Port);

    // 每隔 10 秒打印一次s
    if (current_time - last_print_time >= 10)
    {
        printf("account_balance_detect: %d  %d\r\n", money, pChgGunData->sum_balance);
        last_print_time = current_time;  
    }

    //低于0.3元停止充电
    //功率最大7kw，1min 0.1167度  1s 0.00194度  强制断开时间6s，可能会消耗0.01167度电
    if (money_diff < 5) {
        printf("account_balance_detect: not sufficient funds-- %d  %d\r\n", money, pChgGunData->sum_balance);
        return 1;
    }
    return 0;
}

//时间检测
int charge_time_detect(uint8_t u8Port)
{
    //已充时间-十六进制单位s gun_chrg_mode_param单位min
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    if (pChgGunData->gun_chrg_mode != eDetectMode_Time) {
        return 0;
    }
    U32 time = monitor_getChgTimer(u8Port);
    U32 time_diff = pChgGunData->gun_chrg_mode_param * 60 - time;
    if (time_diff < 1) {
        debugL(DBG_ERRO, "charge_time_detect:-- %d  %d  %d", time, pChgGunData->gun_chrg_mode_param, time_diff);
        return 1;
    }
    return 0;
}

//金额检测
int charge_sum_detect(uint8_t u8Port)
{
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    if (pChgGunData->gun_chrg_mode != eDetectMode_Count) {
        return 0;
    }
	//已充金额
    U32 money = monitor_getChgTotalMoney(u8Port) / 100;;
    U32 money_diff = pChgGunData->gun_chrg_mode_param * 10 - money;
    if (money_diff < 5) {
        //set_charge_stop_reason(u8Port, Reason_Finish_Sum);
        debugL(DBG_ERRO, "charge_sum_detect:-- %d  %d  %d", money, pChgGunData->gun_chrg_mode_param, money_diff);
        return 1;
    }
    return 0;
}

int charge_ele_detect(uint8_t u8Port)
{
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    if (pChgGunData->gun_chrg_mode != eDetectMode_Ele) {
        return 0;
    }
	//已充电量
    U32 ele = monitor_getChgTotalEnergy(u8Port);    //单位度，0.0001kWh 
    U32 target_ele = pChgGunData->gun_chrg_mode_param * 10000;    //单位度，0.0001kWh
    U32 ele_diff = target_ele  - ele;
    //低于0.1度停止充电
    if (ele_diff < 100) {
        //set_charge_stop_reason(u8Port, Reason_Finish_Ele);
        debugL(DBG_ERRO, "charge_ele_detect:-- %d  %d  %d", ele, pChgGunData->gun_chrg_mode_param, ele_diff);
        return 1;
    }
    return 0;
}

//最长充电时间检测
int charge_MaxTime_detect(uint8_t u8Port)
{
    //已充时间-十六进制单位s gun_chrg_mode_param单位min
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    if (pChgGunData->chargeMaxTime == 0) {
        return 0;
    }

    U32 time = monitor_getChgTimer(u8Port);
    U32 time_diff = pChgGunData->chargeMaxTime * 60 - time;
    if (time_diff < 1) {
        debugL(DBG_ERRO, "charge_MaxTime_detect:-- %d  %d  %d", time, pChgGunData->chargeMaxTime, time_diff);
        return 1;
    }
    return 0;
}


void DetectAbnormalScan(uint8_t u8Port)
{
	//充电中检测异常需要停止
	if (logic_get_gun_Uncharged(u8Port) == 0) {
		return;
	}
	//余额不足停止充电
	if (account_balance_detect(u8Port)) {
		stopPileCharge(u8Port, Pile_Stop_Reason_OverBalance);
	}
	//时间用完
	if (charge_time_detect(u8Port)) {
		stopPileCharge(u8Port, Pile_Stop_Reason_OverTime);
	}
	//金额用完
	if (charge_sum_detect(u8Port)){
		stopPileCharge(u8Port, Pile_Stop_Reason_OverSum);
	}
	//电量用完
	if (charge_ele_detect(u8Port)){
		stopPileCharge(u8Port, Pile_Stop_Reason_OverEle);
	}
	//最长时间到达
	if (charge_MaxTime_detect(u8Port)){
		stopPileCharge(u8Port, Pile_Stop_Reason_MaxTime);
	}
}
//订单信息存储
uint8_t GNUpChargeRecordStorage(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    static U8 preSta[GUN_NUM_MAX] = {0};
    U8 crtSta[GUN_NUM_MAX] = {0};

	crtSta[u8Port] = logic_get_gun_state(u8Port);
	
	//开启充电和结束充电分别立即存储一次
	if (preSta[u8Port] != crtSta[u8Port]) {
		printf("gun%d change: %d %d\r\n",u8Port, preSta[u8Port], crtSta[u8Port]);
		//开启充电存储一次
		if ((crtSta[u8Port] == eChargeState_Charging) || (crtSta[u8Port] == eChargeState_Starting)) {
			printf("gun %d start storage...\r\n", u8Port);
	        UpChargeRecordingStorage(u8Port, 1);
		}
		//结束充电存储一次
		if (((preSta[u8Port] >= eChargeState_Starting) && (preSta[u8Port] <= eChargeState_Stoping))
		&& ((crtSta[u8Port] == eChargeState_StopFinish) || (crtSta[u8Port] == eChargeState_Idle))) {
			printf("gun %d stop storage...\r\n", u8Port);
			
            PlatCmdDealUpdateEnable(u8Port);

			pChgGunData->upDealCnt = 0;

	        UpChargeRecordingStorage(u8Port, 1);
		}
		preSta[u8Port] = crtSta[u8Port];
	} else {
	    UpChargeRecordingStorage(u8Port, 0);
    }

	
    return TRUE;
}
//启动命令下发后的5s,判断是否A版反应，
//背景：A版老版程序第二次启动时需要插拔枪（A新程序中已改）。若桩内A老版程序，B版是此版程序。
//目的：B是此版程序情况下，下发成功后，应能判断出A版不响应，从而结束此次订单。
uint8_t GNUpChargeRecordJudgestart(uint8_t u8Port)
{
	 static U32 waittimestick[GUN_NUM_MAX] = {0};
	 CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	 if(1==fgv_get_gun_startCmd(u8Port)&&eChargeState_StopFinish==GetPile_gun_state(u8Port))//存在启动命令且是停止状态：上一笔订单结束，再次启动
	  {
		if(waittimestick[u8Port]==0)
			waittimestick[u8Port] = NOWTICK;
		
        //5s内状态一直是停止，没有启动订单，也没有更新订单
		if(JudgeTimeOutMs(waittimestick[u8Port], 5*eTick_1S) == TRUE)
		{
			fgv_clr_gun_startCmd(u8Port);//分析结束，清除启动命令位的标志
			//结束该笔订单
			PlatCmdDealUpdateEnable(u8Port);

            stopPileCharge(u8Port, Pile_Stop_Reason_MaxTime);//说明是二次启动，但A版是老版程序

			pChgGunData->upDealCnt = 0;

	        UpChargeRecordingStorage(u8Port, 1);
			waittimestick[u8Port]=0;
		}
		
	  }
	  else //不存在启动或状态发生改变,说明正常
	  {
		waittimestick[u8Port]=0; 
		fgv_clr_gun_startCmd(u8Port);//分析结束，清除启动命令位的标志

	  }
		
		return 0;
}
void ChargeFaultActiveStop(uint8_t u8Port)
{
    //充电中故障主动停止充电
    if (EVE_isDevStop(u8Port) == FALSE) {
        return;
    }
    //充电中判断
    if (logic_get_gun_pwmEnable(u8Port) == 0) {
        return;
    }
    uint8_t stopErr = dev_getFirstStopErr(u8Port);
	const Pile_StopReasonMap *pPileStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrPileStopReasonMap); u32i++) {
        pPileStopMap = &StrPileStopReasonMap[u32i];
        if (stopErr == pPileStopMap->pileDefault) {
            stopErr = pPileStopMap->pileStopReason;
            printf("ActiveStop: %d %d\r\n", pPileStopMap->pileDefault, pPileStopMap->pileStopReason);
            break;
        }
    }

    //停止充电
    stopPileCharge(u8Port, stopErr);
}


//是否需要离线上报
uint8_t UpChargeRecordUpDealOffline(uint8_t u8Port)
{
    PlatDealRecord tChgGunDat;
    PlatDealRecord *pDealData = &tChgGunDat;

    printf("DealOffline: gun = %d\r\n", u8Port);

    int ret = DealData_Read(u8Port, (uint8_t *)pDealData, sizeof(PlatDealRecord), 1);

    
    if (ret < 0) {
        printf("DealOffline: ret < 0\r\n");
        return FALSE;
    }

    if (logic_get_gun_charging(u8Port)) {
        printf("DealOffline charging\r\n");
        return FALSE;
    }
    //正常结束，不进行离线订单上传
    if (pDealData->PileStopReason == Pile_Stop_Reason_Finish) {
        printf("DealOffline Reason_Finish\r\n");
        return FALSE;
    }

    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    PlatDealRecord *pActDealRecord = &pChgGunData->DealRecord;

    memcpy(pActDealRecord, pDealData, sizeof(PlatDealRecord));
    
    //如果停止原因未知，上报为断电停止
    if (pDealData->PileStopReason == Pile_Stop_Reason_None) {
        pActDealRecord->PileStopReason = Pile_Stop_Reason_PwOff;
    }

    printf("DealOffline stop_reason = 0x%x\r\n", pActDealRecord->PileStopReason);


    return TRUE;
}

//离线上报
uint8_t GNUpChargeRecordUpDealOffline()
{
    U32 u32i = 0, temp = 0;
    uint8_t tPileReason = 0;
	
	for (uint8_t i = 0; i < GUN_NUM; i++ ) {
		uint8_t uGun = i;
        
		uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }
        charge_record *UpRecord = &g_chgData[uGun].DealRecord.ChgRecord.GnChgRecord;
        //区分云快充东电和公牛
        uint8_t cmd = CMD_Request_deal_log;
        if(ePlatType_DD == get_ChgParam_plat_type()) {
            cmd = YKC_S_Chg_Record_DD;
        } else if (ePlatType_YKC == get_ChgParam_plat_type() ||ePlatType_gwYKC==get_ChgParam_plat_type()|| ePlatType_TOWER == get_ChgParam_plat_type()) {
            cmd = YKC_S_Chg_Record;
        } else if (ePlatType_GN == get_ChgParam_plat_type())
		{
			if(UpRecord->order_model_type == RATE_MODEL_9_TYPE)
			{
				cmd  = CMD_Request_multi_deal_log;
			}
            else
			{
				cmd = CMD_Request_deal_log;
			}
        }
    
		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("GNUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}

		UpRecord->stop_reason = gnUpdate_stopReason(uGun);

		SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
		Send_Immediately(uGun, cmd);
	}

    return TRUE;
}
