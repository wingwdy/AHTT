/***********************************************************************************
 * 文 件 名  : AppMidDataTrans.c
 * 版 本 号  : V1.0
 * 负 责 人  : maxy
 * 创建日期  : 2024-7-1
 * 文件描述  : 获取数据接口
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  : 
 * 其    他  : 
 * 修改日志  : 初版
***********************************************************************************/
#include "CommInterface.h"
#include "RouteHeaderSummary.h"
#include "protocol_ctrl.h"
#include "AppStorage.h"
#include "mbsMaster.h"
#include "cost.h"
#include "ProtoLayerHeaderSummary.h"
#include "protocol_data.h"
#include "screenUart.h"

static GN_PLATMOD  *lpMidPile = &sg_platmod;	//用于本文件中
static PlatCfgInfo *lpMidcfgInfo = &g_pltCfgInfo;

/***************************************************************************************************************
 * ***************************************************************************************************************
 * 基本信息
 * ***************************************************************************************************************
 * ***************************************************************************************************************/

void Get_SoftVersion_A(uint8_t *version)
{
	static GN_PLATMOD  *InputMod = &sg_platmod;	//为了获取A板版本
    memcpy(version, InputMod->pileInf.soft_ver, 4);
}
void Get_HardVersion_A(uint8_t *version)
{
	static GN_PLATMOD  *InputMod = &sg_platmod;	//为了获取A板版本
    memcpy(version, InputMod->pileInf.hw_version, 4);
}


/***************************************************************************************************************
 * ***************************************************************************************************************
 * 以下为网络部分接口
 * ***************************************************************************************************************
 * ***************************************************************************************************************/
/* 获取网络信号 */
U8 GetNet_Comm_CSQ(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	
	return pATMDData->Csq;
}
/* 获取网络信号强度 */
U8 GetNet_SignalLevel(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 u8Csq = 0;
	if(pATMDData->Csq == 99)
    	u8Csq = 99;
	else if(pATMDData->Csq == 0)
    	u8Csq = 0;
	else if(pATMDData->Csq >=1 && pATMDData->Csq <= 14)
    	u8Csq  = 1;
	else if(pATMDData->Csq >=15 && pATMDData->Csq <= 19)
    	u8Csq = 2;
	else if(pATMDData->Csq >=20 && pATMDData->Csq <= 24)
    	u8Csq = 3;
//	else if(pATMDData->Csq >=25 && pATMDData->Csq <= 29)
	else if(pATMDData->Csq >=25)
    	u8Csq = 4;
	return u8Csq;
}
/* 获取网络信号dBm值 */
int8_t GetNet_SignaldBm(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U8 u8Csq = pATMDData->Csq;
	int8_t dBm = 1;
    int8_t step = 0;
    if (u8Csq == 0) {
        dBm = -113;
    } else if (u8Csq == 1) {
        dBm = -111;
    } else if ((u8Csq >= 2) && (u8Csq <= 30)) {
        //-109 ~ -53 dBm
        step = (109 - 53) / (30-2);
        dBm = -109 + (u8Csq - 2) * step;
    } else if ((u8Csq >= 31) && (u8Csq <= 98)) {
        //-53 ~ -1 dBm
        // step = (53 - 1) / (98-31);
        step = 1;
        dBm = -53 + (u8Csq - 31) * step;  
    }
    return dBm;
}


/* 获取运营商 */
U32 GetNet_Comm_Operator(void)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	
	return pATMDData->OperatorType;
}

/* 获取SIM卡号 */
U8 GetNet_Comm_SimID(U8 *p, U16 u16Size)
{
	ATMDData* pATMDData = &g_strGprsModemDCB.strATMDData;
	U16 u16len = 0;
	
	u16len = u16Size > sizeof(pATMDData->SIMID) ? sizeof(pATMDData->SIMID) : u16Size;
	memcpy(p, pATMDData->SIMID, u16len);
	
	return TRUE;
}



/***************************************************************************************************************
 * ***************************************************************************************************************
 * 以下为平台部分相关接口
 * ***************************************************************************************************************
 * ***************************************************************************************************************/

//获取平台服务器参数
void Get_PlatIServer(char *ser, uint16_t *port)
{
    memcpy(ser, lpMidcfgInfo->PltMainIp, PLAT_DNS_LEN);
    *port = lpMidcfgInfo->PltMainPort;
}
void Get_OMPlatIServer(char *ser, uint16_t *port)
{
    memcpy(ser, lpMidcfgInfo->PltAuxiliaryIp, PLAT_DNS_LEN);
    *port = lpMidcfgInfo->PltAuxiliaryPort;
}

typedef struct
{
    PLAT_TYPE		  type;
    char *platTypeName;
    char *cardTypeName;
} CharPlatName;

//获取二维码字符串
void Get_QrCodeString(uint8_t u8Port, char *str)
{
    //固定字符串+桩号+枪号
}


//连接平台桩号转换
void Get_PlatNumberString(char *pNum)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;

    memcpy(pNum, pst_cfgInfo->pltDeviceNumber, PLAT_NUMBER_LEN);
}

//桩资产码
void Get_DevNumberString(char *pNum)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;

    memcpy(pNum, pst_cfgInfo->fixDeviceNumber, FIX_NUMBER_LEN);
}

uint8_t Get_OrderTradeFlag(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	return pChgGunData->trade_flag;
}
//设置平台ip端口信息
void Set_PlatIpPort(char *ip, uint16_t port)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;

    memset(pst_cfgInfo->PltMainIp, 0, PLAT_DNS_LEN);
    memcpy(pst_cfgInfo->PltMainIp, ip, PLAT_DNS_LEN);

    pst_cfgInfo->PltMainPort = port;

    PlatDevNumberChange(1);
}
//设置平台类型
void Set_PlatType(uint8_t type)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
    
    pst_cfgInfo->PltMainType = type;
    
    //更新二维码
    Update_qrCodeInfo();
    PlatDevNumberChange(1);
}



//设置运营平台连接状态
void Set_PlatConnectSta(uint8_t sta)
{
    g_ProtocolDCB.PlatSta.eOnlineType = sta;
}
//获取运营平台连接状态
uint8_t Get_PlatConnectSta(void)
{
    return g_ProtocolDCB.PlatSta.eOnlineType;
}




/***************************************************************************************************************
 * ***************************************************************************************************************
 * 以下为桩端信息接口
 * ***************************************************************************************************************
 * ***************************************************************************************************************/

//获取枪数量
uint8_t GetPile_CfgGunNum()
{
    return lpMidPile->pileCfgInfo.gunNum;
}
//获取是否即插即充状态
uint8_t GetPile_CfgOffLinChrg()
{
    return lpMidPile->pileCfgInfo.fct_cfg.bit.cgfPlugChrg;
}
//获取国企标状态
uint8_t GetPile_CfgNationalStandard()
{
    return lpMidPile->pileCfgInfo.fct_cfg.bit.cgfModeCmp;
}

//获取是否在工桩上
uint8_t GetPile_CfgOnWorkPile()
{
    // return 0;
    return lpMidPile->pileCfgInfo.fct_cfg.bit.cgfOnWorkPile;
}

//获取cp value,0.01v
uint16_t GetPile_GunRtCpValue(uint8_t u8Port)
{
    return lpMidPile->gun[u8Port].gunRtInfo.cpValue;
}
//获取pwm en
uint16_t GetPile_GunPwnEn(uint8_t u8Port)
{
    return lpMidPile->gun[u8Port].gunRtInfo.u_platChrgsta.bit.pwm_en_1b;
}
//获取relay out
uint16_t GetPile_GunRelayOut(uint8_t u8Port)
{
    return lpMidPile->gun[u8Port].gunRtInfo.u_platChrgsta.bit.relay_out;
}
//获取pwm
uint16_t GetPile_GunChargingPwm(uint8_t u8Port)
{
    if (GetPile_GunPwnEn(u8Port)) {
        return lpMidPile->gun[u8Port].chrgingInfo.pwm;
    }
    return 0;
}


//获取按键状态
uint8_t Get_PileBtnSta()
{
	return lpMidPile->pile_rt_inf.stopKey;
}

//充电桩版本号
void GetPile_StrSoftVer(char *strVer)
{
    sprintf(strVer, "%d.%d.%d.%d", lpMidPile->pileInf.soft_ver[0],lpMidPile->pileInf.soft_ver[1],lpMidPile->pileInf.soft_ver[2],lpMidPile->pileInf.soft_ver[3]);
}
void GetPile_U8SoftVer(uint8_t *u8Ver)
{
    memcpy(u8Ver, lpMidPile->pileInf.soft_ver, 4);
}

//充电枪状态
CHARGE_STATE_E GetPile_gun_state(uint8_t u8Port)
{
	return lpMidPile->gun[u8Port].gunRtInfo.gun_ChrgSta;
}

//充电枪连接状态, 0x00 否 0x01 是 插抢状态
uint8_t GetPile_gun_connect(uint8_t u8Port)
{
    if (lpMidPile->gun[u8Port].gunRtInfo.u_platChrgsta.bit.connect_1b) {
	    return 1;
    } else {
	    return 0;
    }
}

//获取输入电压, 0.1V
uint32_t GetPile_ChgInVol()
{
	uint32_t temp = 0;
    
	// temp = lpMidPile->gun[u8Port].chrgingInfo.out_vol;
    temp = lpMidPile->pile_rt_inf.inVoltageA;
	
	return temp;
}
//获取电压, 0.1V
uint32_t GetPile_ChgOutVol(uint8_t u8Port, uint8_t point)
{
	uint32_t temp = 0;

	//正常充电中输出电压，其他为0，包括暂停也为0，按照交流电是否输出为根据
    if (lpMidPile->gun[u8Port].gunRtInfo.u_platChrgsta.bit.relay_out == 0) {
		return 0;
    }
    if (point == 1) {
	    temp = (lpMidPile->gun[u8Port].chrgingInfo.out_vol + 5) / 10;
    } else {
	    temp = lpMidPile->gun[u8Port].chrgingInfo.out_vol;
    }
	
	return temp;
}

//获取最大允许充电总电压
uint16_t GetPile_ChgMaxVol(uint8_t u8Port)
{
    uint16_t temp = 0;
    
	temp = lpMidPile->pileCfgInfo.max_voltage;
	
	return temp;
}

//获取最大允许充电总电流
uint16_t GetPile_ChgMaxCurrent(uint8_t u8Port)
{
    uint16_t temp = 0;
    
	temp = lpMidPile->pileCfgInfo.max_current;
	
	return temp;
}
//获取最大允许温度
uint8_t GetPile_ChgMaxTemp(uint8_t u8Port)
{
    uint8_t temp = 0;
    
	temp = lpMidPile->pileCfgInfo.max_gun_temp;
	
	return temp;
}

//获取电流
uint32_t GetPile_ChgOutCur(uint8_t u8Port, uint8_t point)
{
	uint32_t temp = 0;

    if (lpMidPile->gun[u8Port].gunRtInfo.u_platChrgsta.bit.relay_out == 0) {
		return 0;
    }

    if (point == 1) {
	    temp = (lpMidPile->gun[u8Port].chrgingInfo.out_cur + 5) / 10;
    } else {
	    temp = lpMidPile->gun[u8Port].chrgingInfo.out_cur;
    }
	
	return temp;
}

//交流没枪温采样用插销和壳体温度代替
int8_t GetPile_GunTem(uint8_t u8Port)
{
	int8_t temp = 0;
	
	temp = lpMidPile->gun[u8Port].gunRtInfo.plugTemp;

	return temp;
}
//交流环境温度
int8_t GetPile_EvnTem()
{
	int8_t temp = 0;
	
	temp = lpMidPile->pile_rt_inf.pileTemp / 10;

	return temp;
}

//获取充电时间s
uint32_t GetPile_ChgTimer(uint8_t u8Port)
{
	uint32_t temp = 0;
	
	temp = lpMidPile->gun[u8Port].chrgingInfo.charge_time;
	
	return temp;
}
//获取充电电量
// uint32_t GetPile_ChgTotalPower(uint8_t u8Port)
// {
// 	uint32_t temp = 0;

//     //即插即充，无订单
//     if (GetPile_CfgOffLinChrg()) {

// 	    temp = lpMidPile->gun[u8Port].chrgingInfo.charge_ele;

//     } else {
//         //运营模式，有订单
// 	    COST_GUN_DATA *pCostGunData = &g_cost_ctrl.strCostGunData[u8Port];
        
// 	    temp = pCostGunData->total_power;
//     }
	
// 	return temp;
// }
uint32_t GetPile_ChgTotalPower(uint8_t u8Port)
{
	uint32_t temp = 0;

	temp = lpMidPile->gun[u8Port].chrgingInfo.charge_ele;

	return temp;
}

//获取起始充电量
uint32_t GetPile_ChgStartEle(uint8_t u8Port)
{
	uint32_t temp = 0;
	
	temp = lpMidPile->gun[u8Port].chrgingInfo.start_ele;
	
	return temp;
}
//获取终止充电量
uint32_t GetPile_ChgStoptEle(uint8_t u8Port)
{
	uint32_t temp = 0;
	
	temp = lpMidPile->gun[u8Port].chrgingInfo.stop_ele;
	
	return temp;
}

//获取充电损耗电量
uint32_t GetPile_ChgTotalLossPower(uint8_t u8Port)
{
	uint32_t temp = 0;
	
	// temp = lpMidPile->gun[u8Port].chrgingInfo.charge_ele;
	
	return temp;
}


/***************************************************************************************************************
 * ***************************************************************************************************************
 * 以下为订单部分数据接口
 * ***************************************************************************************************************
 * ***************************************************************************************************************/

//获取充电金额
uint32_t GetPile_ChgTotalMoney(uint8_t u8Port)
{
    // 四位小数
	COST_GUN_DATA *pRecord = &g_cost_ctrl.strCostGunData[u8Port];

	return pRecord->total_money;
}
//设置平台充电总金额
void SetPlat_ChgTotalMoney(uint8_t u8Port, uint32_t money)
{
    // 四位小数
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    pChgGunData->chargeAllMny = money;
}

//获取屏幕显示充电金额,单位0.01元
uint32_t GetPile_ChgTotalMoneyDisplay(uint8_t u8Port)
{
    // 2位小数
	COST_GUN_DATA *pRecord = &g_cost_ctrl.strCostGunData[u8Port];
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    
    if (pChgGunData->chargeAllMny) {
        return ((pChgGunData->chargeAllMny + 50) / 100);
    }
    return ((pRecord->total_money + 50) / 100);
}

//设置平台刷卡异常原因
uint8_t GetPlat_CardChargeFaild(uint8_t u8Port)
{
    // 四位小数
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    if (pChgGunData->CardChargeFaild) {
        if (JudgeTimeOutMs(pChgGunData->CardFaildTick, eTick_3S) == TRUE) {
            pChgGunData->CardChargeFaild = 0;
        }
    }

    return pChgGunData->CardChargeFaild;
}
void SetPlat_CardChargeFaild(uint8_t u8Port, uint8_t reason)
{
    // 四位小数
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    pChgGunData->CardChargeFaild = reason;
    pChgGunData->CardFaildTick = Get_Systick();
}



//获取结算后账户余额
uint32_t GetPile_SettleAccountBalance(uint8_t u8Port)
{
    //两位小数
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    return ((pChgGunData->sum_balance * 100 - GetPile_ChgTotalMoney(u8Port)) / 100);
}

//获取结算前账户余额
uint32_t GetPile_AccountBalance(uint8_t u8Port)
{
    //两位小数
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

    return pChgGunData->sum_balance;
}

//获取充电卡号
void GetPile_ChgCarNumber(uint8_t u8Port, uint8_t *card_number)
{	
	monitor_getChgCardNum(u8Port, card_number, GNDATA_CARD_LEN);
    
	return;
}

//获取物理卡号
void GetPile_CardPhyNumber(uint8_t u8Port, uint8_t *card_number)
{	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    
    memcpy(card_number, pChgGunData->PhyCard_number, 4);
    
	return;
}
//获取逻辑卡号
void GetPile_CardLogNumber(uint8_t u8Port, uint8_t *card_number)
{	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    
	memcpy(card_number, pChgGunData->LogicCard_number, GNDATA_CARD_LEN);
    
	return;
}


//获取桩停止原因
uint16_t GetPile_ChgStopReason(uint8_t u8Port)
{
	uint32_t temp = 0;

	temp = lpMidPile->gun[u8Port].gunRtInfo.pileStopReason;
	
	return temp;
}

//充电桩是否有故障
uint8_t GetPile_ErrState(uint8_t u8Port)
{
	uint8_t flag = FALSE;

	if(lpMidPile->gun[u8Port].gunRtInfo.hardfault.bits)
		flag = TRUE;
	
    return flag;
}


//充电桩故障,下面故障只有桩存在
uint8_t Get_OnlayPileErr()
{
	uint8_t flag = FALSE;
    for (int u8Port = 0; u8Port < GUN_NUM; u8Port++) {
        if(lpMidPile->gun[u8Port].gunRtInfo.hardfault.bit.faultEStop_1b)
            flag = TRUE;
            break;
        if(lpMidPile->gun[u8Port].gunRtInfo.hardfault.bit.faultPileTemp_1b)
            flag = TRUE;
            break;
        if(lpMidPile->gun[u8Port].gunRtInfo.hardfault.bit.faultLFRevs_1b)
            flag = TRUE;
            break;
        if(lpMidPile->gun[u8Port].gunRtInfo.hardfault.bit.faultGround_1b)
            flag = TRUE;
            break;
    }
    return flag;
}

//获取急停按键状态
uint8_t Get_PileEstopBtnSta()
{
	return lpMidPile->gun[0].gunRtInfo.hardfault.bit.faultEStop_1b;
}

//充电桩故障信息
uint32_t GetPile_ErrInfo(uint8_t u8Port)
{
	return lpMidPile->gun[u8Port].gunRtInfo.hardfault.bits;
}

//获取桩是否空闲，非枪
uint8_t GetPile_Idlet()
{
    uint8_t ret = 1;
    for (int i = 0; i < GUN_NUM_MAX; i++) {
        if (GetPile_gun_state(i)) {
            ret = 0;
        }
    }
    return ret;
}


//获取充电起始时间
void GetPile_ChargeBgEndTime(uint8_t u8Port, uint32_t *startStamp, uint32_t *stopStamp)
{
    //即插即充，无订单
    if (GetPile_CfgOffLinChrg()) {
        timToStamp(startStamp, (tm_struct *)lpMidPile->gun[u8Port].chrgingInfo.start_time);
        timToStamp(stopStamp, (tm_struct *)lpMidPile->gun[u8Port].chrgingInfo.stop_time);
    } else {
        //运营模式，有订单
        CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
        uint32_t l_stamp = 0;
        timToStamp(startStamp, (tm_struct *)pChgGunData->chrg_start_time);
        timToStamp(stopStamp, (tm_struct *)pChgGunData->chrg_stop_time);
    }
}

void g_ClearChargingInfo(uint8_t u8Port)
{
    COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    memset(&lpMidPile->gun[u8Port].chrgingInfo, 0, offsetof(GUN_PLTMD_CHARGE_INF, pwm));           
    memset(pcostdata, 0, sizeof(COST_GUN_DATA));

    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    memset(pChgGunData->transaction_log_num, 0, GNDATA_TRDNUM_LEN);

    memset(pChgGunData->chrg_start_time, 0, 14);
    pChgGunData->total_start_elec = 0;
    pChgGunData->total_stop_elec = 0;
    pChgGunData->chg_timer = 0;
    pChgGunData->chrg_ele = 0;
    pChgGunData->sum_balance = 0;
    pChgGunData->chargeAllMny = 0;
    pChgGunData->trade_flag = eUP_Start_Style_NULL;
}

//充电完成拔枪500ms后清除充电以及订单信息
void GunLeaveCarChargeDataClear(uint8_t u8Port, uint8_t CnctSta)
{
    static uint32_t DisconnectGunTime[GUN_NUM_MAX] = {0};
    static uint8_t DisconnectFlag[GUN_NUM_MAX] = {0};
    static uint8_t preCnctSta[GUN_NUM_MAX] = {0};
    
    
    if (DisconnectFlag[u8Port]) {
        if (JudgeTimeOutMs(DisconnectGunTime[u8Port], eTick_500ms) == TRUE) {
            DisconnectFlag[u8Port] = 0;

            g_ClearChargingInfo(u8Port);    //清除充电数据

            g_CardInfoClear(u8Port);
            
            //拔枪之后计费部分数据处理,有些平台需要读取当前计费模型，需要更新掉，或者屏幕显示
			Cost_charge_init(u8Port);
            CostGetRateModel(u8Port);       //获取计费模型
        }
    }

    if (preCnctSta[u8Port] == CnctSta) {
        return;
    }
    preCnctSta[u8Port] = CnctSta;

    if (CnctSta == 0) {
        DisconnectGunTime[u8Port] = NOWTICK;
        DisconnectFlag[u8Port] = 1;
    }
}

