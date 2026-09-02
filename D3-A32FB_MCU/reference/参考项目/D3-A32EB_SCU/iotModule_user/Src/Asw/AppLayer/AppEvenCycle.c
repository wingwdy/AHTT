#include "AppEvenCycle.h"
#include "FreeRTOS.h"
#include "task.h"

#include "mbsDataUpdate.h"
#include "CommInterface.h"
#include "mbsMaster.h"
#include "AppMidDataTrans.h"
#include "protocol_data.h"


//#define UPRINT(...)  printf(__VA_ARGS__)

/*------------------------------------------------------------------------------
Section: 共享内存
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Section: 设备状态控制
------------------------------------------------------------------------------*/
typedef EVE_CHECK_RESULT (*pCycleFunc)(uint8_t u8Port);

typedef struct
{
    E_ErrCode          eErr;           	// 故障类型
    uint32_t           	tlAppearTick;   	// 故障产生的滤波时间
    uint32_t           	tDisappearTick; 	// 故障恢复的滤波时间
    pCycleFunc    		CycleFunc;
    uint32_t           	tlCheckTimer[EVE_GUN_CNT];   // 故障产生/恢复计时器
}PROTECT_EVE_OBJ;


typedef struct
{
    DEVICE_STATE_E            eDevState;                      	// 设备状态
    E_ErrCode                firstEve;                       	// 第一个产生的可导致停电的停机原因
    REPORT_EVE_QUEUE          eveQueue[EVE_QUEUE_CNT];      	// 故障队列 用于上报
    uint8_t                   isExsit[eErr_Num];    			// 故障是否存在
}DEVSTATE_CTRL_T;

typedef struct
{
    E_ErrCode                eEveid;                     		// 故障编码
    EVE_LEVEL_E               eEveLevel;                  		// 故障等级,根据等级判断当前故障状态
    char *                    cEveString;                 		// 故障名
}EVE_ATTRIBUTE_T;

static const EVE_ATTRIBUTE_T EVE_ATTRIBUTE_MAP[] =
{                                                                              
    { eErr_none,                 eErr_LvN,     "无效值" }, 
	//1
    { eErr_CpVoltAbnor,          eErr_Lv1,     "CP电压异常" },
    { eErr_CpGroundFault,        eErr_Lv1,     "CP对地短路" },
    { eErr_PEBreakFault,         eErr_Lv1,     "PE接地故障" },
    { eErr_GunDisConnErr,        eErr_Lv1,     "充电中控制导引故障" },
    { eErr_EmergencyStop,        eErr_Lv1,     "急停故障" },
    { eErr_InputLineReversed,    eErr_Lv1,     "火零反接" },
    { eErr_LeakageCurrErr,       eErr_Lv1,     "漏电流故障" },
    { eErr_DiodeStop,            eErr_Lv1,     "二极管故障" },
    { eErr_ShortCircleErro,      eErr_Lv1,     "短路保护故障" },

    { eErr_AphaseInputOverVol,   eErr_Lv2,     "交流A相输入过压" },
    { eErr_BphaseInputOverVol,   eErr_Lv2,     "交流B相输入过压" },
    { eErr_CphaseInputOverVol,   eErr_Lv2,     "交流C相输入过压" },
    { eErr_AphaseInputLessVol,   eErr_Lv2,     "交流A相输入欠压" },
    { eErr_BphaseInputLessVol,   eErr_Lv2,     "交流B相输入欠压" },
    { eErr_CphaseInputLessVol,   eErr_Lv2,     "交流C相输入欠压" },
    { eErr_OutputOverCurr,       eErr_Lv2,     "交流输出过流" },
    { eErr_JcqMaloperation,      eErr_Lv2,     "交流输出接触器误动拒动" },
    { eErr_JcqSynechiaFault,     eErr_Lv2,     "交流输出接触器粘连" },
    { eErr_HmiCommErr,           eErr_Lv2,     "人机交互通信故障" },

    { eErr_ReaderCommErr,        eErr_Lv2,     "读卡器通信故障" },
    { eErr_8209CommErr,          eErr_Lv2,     "RN8209通信故障" },
    { eErr_MeterCommErr,         eErr_Lv2,     "电表通信故障" },
    { eErr_EnvOverTempErr,       eErr_Lv2,     "环境过温故障" },
    { eErr_GunOverTempErr,       eErr_Lv2,     "枪过温故障" },
    { eErr_POverTempErr,         eErr_Lv2,     "插头过温" },
    { eErr_DatabaseErr,          eErr_Lv2,     "数据库存储错误" },    
    { eErr_MeterCalcErr,         eErr_Lv2,     "电能表计量故障" }, 
    { eErr_PowerApplyFail,       eErr_Lv2,     "功率申请失败告警" },
    { eErr_ChgStartTimeout,      eErr_Lv2,     "充电启动超时" },

    { eErr_EnvOverTempWarn,       eErr_Lv3,     "环境过温警告" },
    { eErr_GunOverTempWarn,       eErr_Lv3,     "枪过温警告" },
    { eErr_PlugOverTempWarn,      eErr_LvN,     "插头过温警告" },
    { eErr_LittleCurr,           eErr_LvN,     "小电流状态" },
    { eSrc_ChargeFullStop,       eErr_LvN,     "充满停止" },
    { eSrc_AutoStop,             eErr_LvN,     "自动停止（车辆）" },
    { eSrc_ManualCtrlStop,       eErr_LvN,     "主动控制停止" },

    //下面为网络单元扩展故障
    { eErr_CCUSCUCommErr,        eErr_Lv1,     "充电单元通讯故障" },
    { eErr_PhaseLossErr,         eErr_Lv1,     "缺相故障" },
    { eErr_InputOverVol,         eErr_Lv1,     "交流输入过压" },
    { eErr_InputLessVol,         eErr_Lv1,     "交流输入欠压" },

    { eErr_NetNoSIMErr,           eErr_LvN,     "sim卡未插入" },
    { eErr_NetSIMErr,             eErr_LvN,     "网络连接异常" },
    { eErr_NetIPErr,              eErr_LvN,     "ip连接异常" },
    { eErr_PlatformOffline,       eErr_LvN,     "平台与平台通信异常" },
    { eErr_feemodelErr,           eErr_LvN,     "平台下发费率异常" },

    { eErr_Num,                  eErr_LvN,     "故障数 " },
};

static DEVSTATE_CTRL_T  s_devStateCtrl[EVE_GUN_CNT];

static uint8_t EVE_PushQueue(uint8_t u8Port, E_ErrCode eErr, uint8_t u8State)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];
	uint32_t index = 0;

	for (index = 0; index < EVE_QUEUE_CNT; index++)
	{
		//如果任务已经存在，那么不添加
		if (eErr_none != pdevStateCtrl->eveQueue[index].eErr)
			continue;
		
		pdevStateCtrl->eveQueue[index].eErr = eErr;
		pdevStateCtrl->eveQueue[index].state = u8State;
		return TRUE;
	}
	
    return FALSE;
}

uint8_t EVE_PopQueue(uint8_t u8Port, REPORT_EVE_QUEUE *pEveInfo)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];
	
	//如果任务已经存在，那么不添加
	if(eErr_none == pdevStateCtrl->eveQueue[0].eErr)
		return FALSE;
	
	pEveInfo->eErr = pdevStateCtrl->eveQueue[0].eErr;
	pEveInfo->state = pdevStateCtrl->eveQueue[0].state;
	
	memmove(&pdevStateCtrl->eveQueue[0], &pdevStateCtrl->eveQueue[1], (EVE_QUEUE_CNT-1)*sizeof(REPORT_EVE_QUEUE));
	memset(&pdevStateCtrl->eveQueue[EVE_QUEUE_CNT-1], 0, sizeof(REPORT_EVE_QUEUE));
	
	return TRUE;
}

uint8_t EVE_IsQueueEmpty(uint8_t u8Port)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];
	
	if(eErr_none == pdevStateCtrl->eveQueue[0].eErr)
	{
		return TRUE;
	}
	
	return FALSE;
}

/**
******************************************************************************
* @brief       判断故障是否存在
* @param[in]   ERR_CODE eErr 故障ID
* @param[out]  None
* @retval      bool TRUE-存在 FALSE-不存在
* @details
* @note
******************************************************************************
*/
uint8_t dev_getErrExsit(uint8_t u8Port, E_ErrCode eErr)
{
    return s_devStateCtrl[u8Port].isExsit[eErr];
}

/**
******************************************************************************
* @brief       故障置位
* @param[in]   E_ErrCode eErr 故障码
* @param[in]   uint16 line  故障产生代码行号
* @param[out]  None
* @retval
* @details
* @note
******************************************************************************
*/
uint8_t dev_setErrExsit(uint8_t u8Port, E_ErrCode eErr, uint16_t line)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];

    // 非法值滤掉
    if (eErr == eErr_none || eErr >= eErr_Num)
    {
        return FALSE;
    }

    if ((!pdevStateCtrl->isExsit[eErr]))
    {
    	pdevStateCtrl->isExsit[eErr] = TRUE;

#if(ERR_RCD_ENBLE)
		gn_packErrRecord(u8Port, eErr, TRUE);
#endif
        // 缓存住第一个可导致停止充电故障
        if ((eErr_Lv1 == EVE_ATTRIBUTE_MAP[eErr].eEveLevel)
			|| (eErr_Lv2 == EVE_ATTRIBUTE_MAP[eErr].eEveLevel)
			|| (eErr_Lv5 == EVE_ATTRIBUTE_MAP[eErr].eEveLevel))
        {/*used to record the first fault which cause stop*/
            if (eErr_none == pdevStateCtrl->firstEve)
            {
                pdevStateCtrl->firstEve = eErr;
            }
        }
		
		EVE_PushQueue(u8Port, eErr, TRUE);
        if (eErr < eErr_Num)
        {
			// LogPrintf(LVL_LOG_INFO, "\r\n<GUN:%d><ERR>[set]<0x%02X> -%s- line:%d!", u8Port, eErr, EVE_ATTRIBUTE_MAP[eErr].cEveString, line);
		}
        return TRUE;
    }

    return FALSE;
}

uint8_t dev_setErrExsit_all(E_ErrCode eErr, uint16_t line)
{
	uint8_t i = 0;

	for(i = 0; i < EVE_GUN_CNT; i++)
	{
		dev_setErrExsit(i, eErr, line);
	}

    return FALSE;
}
/**
******************************************************************************
* @brief       故障清除
* @param[in]   ERR_CODE_E eErr 故障码
* @param[in]   uint16_t line  故障清除代码行号
* @param[out]  None
* @retval
* @details
* @note
******************************************************************************
*/
uint8_t dev_clrErrExsit(uint8_t u8Port, E_ErrCode eErr, uint16_t line)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];

    // 非法值滤掉
    if (eErr == eErr_none || eErr >= eErr_Num)
    {
        return FALSE;
    }

    if (pdevStateCtrl->isExsit[eErr])
    {
    	pdevStateCtrl->isExsit[eErr] = FALSE;

#if(ERR_RCD_ENBLE)
		gn_packErrRecord(u8Port, eErr, FALSE);
#endif

        // 故障清除，清掉可导致停止充电的故障
        if ((eErr_none != pdevStateCtrl->firstEve) && (pdevStateCtrl->firstEve == eErr))
        {
            pdevStateCtrl->firstEve = eErr_none;
        }

		EVE_PushQueue(u8Port, eErr, FALSE);
        if (eErr < eErr_Num)
        {
            ;
			// LogPrintf(LVL_LOG_INFO, "\r\n<GUN:%d><ERR>[clr]<0x%02X> -%s- line:%d!", u8Port, eErr, EVE_ATTRIBUTE_MAP[eErr].cEveString, line);
		}

        return TRUE;
    }

    return FALSE;
}
uint8_t dev_clrErrExsit_all(E_ErrCode eErr, uint16_t line)
{
	uint8_t i = 0;

	for(i = 0; i < EVE_GUN_CNT; i++)
	{
		dev_clrErrExsit(i, eErr, line);
	}

    return FALSE;
}

/**
******************************************************************************
* @brief       获取设备状态
* @param[in]   None
* @param[out]  None
* @retval      DEVICE_STATE
* @details
* @note
******************************************************************************
*/
DEVICE_STATE_E dev_getDevState(uint8_t u8Port)
{
    return s_devStateCtrl[u8Port].eDevState;
}

//设备是否故障
uint8_t dev_getDevHardfalt(uint8_t u8Port)
{
	uint8_t flag = FALSE;

	if(dev_getDevState(u8Port) >= eDevState_MajorAlarm)
    {
		flag = TRUE;
    }

    return flag;
}

uint8_t EVE_isDevStop(uint8_t u8Port)
{
	DEVSTATE_CTRL_T *pdevStateCtrl = &s_devStateCtrl[u8Port];

    if (pdevStateCtrl->firstEve == 0) {
        return FALSE;
    }
    return TRUE;
}

/**
******************************************************************************
* @brief       获取最先产生的故障
* @param[in]   None
* @param[out]  None
* @retval      ERR_CODE
* @details
* @note        用于冻结停止原因和发起方
******************************************************************************
*/
E_ErrCode dev_getFirstStopErr(uint8_t u8Port)
{
    return s_devStateCtrl[u8Port].firstEve;
}
/**
******************************************************************************
* @brief       获取故障的对应的字符串
* @param[in]   ERR_CODE
* @param[out]  None
* @retval
* @details
* @note
******************************************************************************
*/
char *dev_getErrString(E_ErrCode eErr)
{
    if ((uint32_t)eErr >= FCNT(EVE_ATTRIBUTE_MAP))
    {
        return EVE_ATTRIBUTE_MAP[eErr_none].cEveString;
    }

    return EVE_ATTRIBUTE_MAP[eErr].cEveString;
}



static GN_PLATMOD  *errUsePlatmod = &sg_platmod;	//用于本文件中

//充电单元通讯故障
EVE_CHECK_RESULT eErr_CCUSCUCommErr_Func(uint8_t uPort)
{
    if (fgv_GetMbsCommSta()) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//读卡器通信故障
EVE_CHECK_RESULT eErr_ReaderCommErr_Func(uint8_t uPort)
{
    return eEveResult_Off;
}
//漏电流故障
EVE_CHECK_RESULT eErr_LeakageCurrErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultLeakage_1b) {
        return eEveResult_On; 
    } else {
        return eEveResult_Off;
    }
}
//缺相故障
EVE_CHECK_RESULT eErr_PhaseLossErr_Func(uint8_t uPort)
{
    return eEveResult_Off;
}
//急停故障
EVE_CHECK_RESULT eErr_EmergencyStop_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultEStop_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//CP电压异常
EVE_CHECK_RESULT eErr_CpVoltAbnor_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultCP_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//CP对地短路
EVE_CHECK_RESULT eErr_CpGroundFault_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultCPGround_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//PE接地故障
EVE_CHECK_RESULT eErr_PEBreakFault_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultGround_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//交流输入过压
EVE_CHECK_RESULT eErr_InputOverVol_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultVoltage_1b == 1) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//交流输入欠压
EVE_CHECK_RESULT eErr_InputLessVol_Func(uint8_t uPort)
{   
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultVoltage_1b == 2) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//交流输出过流
EVE_CHECK_RESULT eErr_OutputOverCurr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultCurrent_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//交流输出接触器粘连
EVE_CHECK_RESULT eErr_JcqSynechiaFault_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultRelay_1b == 1) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//交流输出接触器误动拒动
EVE_CHECK_RESULT eErr_JcqMaloperation_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultRelay_1b == 2) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//环境过温故障
EVE_CHECK_RESULT eErr_EnvOverTempErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultPileTemp_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//插头过温故障
EVE_CHECK_RESULT eErr_POverTempErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultPlugTemp_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//不存在二极管故障
EVE_CHECK_RESULT eErr_DiodeErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultDiode_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//短路故障
EVE_CHECK_RESULT eErr_ShortCErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultShortC_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//电池反接故障
EVE_CHECK_RESULT eErr_LFRvsErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultLFRevs_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}

//sim卡未插入
EVE_CHECK_RESULT eErr_NetNoSimErr_Func(uint8_t uPort)
{
       return eEveResult_Off;
       //下面为检测硬件引脚，未用到
    // if (fgu1_DiReadRoute(APP_SIM_STATUS_DI) == RESET) {
    //     return eEveResult_On;
    // } else {
    //     return eEveResult_Off;
    // }
}
//网络连接异常
EVE_CHECK_RESULT eErr_NetSIMErr_Func(uint8_t uPort)
{
        // return eEveResult_Off;
    if (Comm_getNetAbnormalSta(eSocket_GPRS1) == eNet_SimErro) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//ip连接异常
EVE_CHECK_RESULT eErr_NetIPErr_Func(uint8_t uPort)
{
        // return eEveResult_Off;
    if (Comm_getNetAbnormalSta(eSocket_GPRS1) == eNet_IPErro) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//平台与平台通信异常
EVE_CHECK_RESULT eErr_PlatformOffline_Func(uint8_t uPort)
{
    if (fgv_GetPileCfgOffLinChrg()) {
        return eEveResult_Off;
    }
    
    //心跳正常返回平台通讯正常
    if (Get_PlatConnectSta() == eOnline_Heart) {
        return eEveResult_Off;
    } else {
        return eEveResult_On;
    }
}
//电表通信故障
EVE_CHECK_RESULT eErr_MeterCommErr_Func(uint8_t uPort)
{
    U_GunFault *pFault = &errUsePlatmod->gun[uPort].gunRtInfo.hardfault;

    if (pFault->bit.faultEleMeasure_1b) {
        return eEveResult_On;
    } else {
        return eEveResult_Off;
    }
}
//电能表计量故障
EVE_CHECK_RESULT eErr_MeterCalcErr_Func(uint8_t uPort)
{
    //充电中判断
    if (GetPile_GunRelayOut(uPort) == 0) {
        return eEveResult_Off;
    }
    //电流值和电量值判断
    if (GetPile_ChgOutCur(uPort, 2) <= 10 && (GetPile_ChgTotalPower(uPort) <= 10)) {
        return eEveResult_On;
    }
    return eEveResult_Off;
}


/*****************************************************************************
 * 函 数 名  :
 * 负 责 人  :
 * 创建日期  : 2021年5月1日
 * 函数功能  :
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
static PROTECT_EVE_OBJ PROTECT_EVE_OBJ_TABLE[] =
{
  // 充电单元产生的故障
  { eErr_ReaderCommErr,    	    eTick_0ms,    	eTick_500ms,  	eErr_ReaderCommErr_Func}, // 读卡器通信故障
  { eErr_LeakageCurrErr,      	eTick_0ms,    	eTick_500ms,  	eErr_LeakageCurrErr_Func},// 漏电流故障 
  { eErr_PhaseLossErr,    	    eTick_0ms,    	eTick_500ms,  	eErr_PhaseLossErr_Func},  // 缺相故障
  { eErr_EmergencyStop,	        eTick_0ms,     	eTick_500ms,  	eErr_EmergencyStop_Func},// 急停故障
  { eErr_CpVoltAbnor,	        eTick_0ms,     	eTick_500ms,  	eErr_CpVoltAbnor_Func},// CP电压异常
  { eErr_CpGroundFault,   	    eTick_0ms,  	eTick_500ms,  	eErr_CpGroundFault_Func}, // CP对地短路
  { eErr_PEBreakFault, 	        eTick_0ms,     	eTick_500ms,  	eErr_PEBreakFault_Func},  // PE接地故障
  { eErr_InputOverVol,		    eTick_0ms,     	eTick_500ms,  	eErr_InputOverVol_Func},  // 交流输入过压
  { eErr_InputLessVol,		    eTick_0ms,     	eTick_500ms,  	eErr_InputLessVol_Func},  // 交流输入欠压

  { eErr_OutputOverCurr,        eTick_0ms,      eTick_1S,       eErr_OutputOverCurr_Func}, // 交流输出过流
  { eErr_JcqSynechiaFault,		eTick_0ms,    	eTick_500ms,  	eErr_JcqSynechiaFault_Func}, // 交流输出接触器粘连
  { eErr_JcqMaloperation,		eTick_0ms,    	eTick_500ms,  	eErr_JcqMaloperation_Func}, // 交流输出接触器误动拒动
  { eErr_EnvOverTempErr,		eTick_0ms,    	eTick_500ms,  	eErr_EnvOverTempErr_Func}, // 环境过温故障
  { eErr_POverTempErr,		    eTick_0ms,    	eTick_500ms,  	eErr_POverTempErr_Func}, // 插头过温故障
  { eErr_DiodeStop,		        eTick_0ms,     	eTick_500ms,  	eErr_DiodeErr_Func},  //不存在二极管故障
  { eErr_InputLineReversed,		eTick_0ms,     	eTick_500ms,  	eErr_LFRvsErr_Func},  //电池反接故障
  { eErr_ShortCircleErro,		eTick_0ms,     	eTick_500ms,  	eErr_ShortCErr_Func},  //短路故障
  { eErr_MeterCommErr,		    eTick_50ms,    	eTick_500ms,  	eErr_MeterCommErr_Func}, // 电表通信故障，硬件

  // 网络单元判断的故障需要进行停充处理
  { eErr_CCUSCUCommErr,     	eTick_5S,    	eTick_500ms,   	eErr_CCUSCUCommErr_Func}, // 充电单元通讯故障
  { eErr_MeterCalcErr,		    eTick_180S,    	eTick_100ms,  	eErr_MeterCalcErr_Func}, // 电能表计量故障,软件

  //以下故障不进行停充处理
  { eErr_NetNoSIMErr,		    eTick_200ms,    eTick_500ms,  	eErr_NetNoSimErr_Func}, // 网络连接异常，sim未插入
  { eErr_NetSIMErr,		        eTick_200ms,    eTick_500ms,  	eErr_NetSIMErr_Func}, // 网络连接异常，离线-SIM卡异常
  { eErr_NetIPErr,		        eTick_200ms,    eTick_500ms,  	eErr_NetIPErr_Func}, // 网络连接正常，ip连接异常 
  { eErr_PlatformOffline,		eTick_200ms,    eTick_500ms,  	eErr_PlatformOffline_Func}, // 平台未正常上线，平台通信异常

};

static void EVE_CycleCheck(void)
{
   PROTECT_EVE_OBJ *pObj = NULL;
   uint8_t index = 0, i = 0;
   uint32_t tDisappearTick = 0; 	// 故障恢复的滤波时间

	for (i = 0; i < EVE_GUN_CNT; i++)
   {
   	for (index = 0; index < FCNT(PROTECT_EVE_OBJ_TABLE); index++)
	    {
	        pObj = &PROTECT_EVE_OBJ_TABLE[index];

	        if (TRUE == dev_getErrExsit(i, pObj->eErr))
	        {
	            if (eEveResult_Off == pObj->CycleFunc(i))
	            {
	            	//故障恢复不能太快防止故障无法上报
	            	tDisappearTick = (pObj->tDisappearTick < eTick_200ms) ? eTick_200ms : pObj->tDisappearTick;
	                if (JudgeTimeOutMs(pObj->tlCheckTimer[i], tDisappearTick))
	                {
	                    dev_clrErrExsit(i, pObj->eErr, __LINE__);
	                    pObj->tlCheckTimer[i] = NOWTICK;
	                }
	            }
	            else
	            {
	                pObj->tlCheckTimer[i] = NOWTICK;
	            }
	        }
	        else
	        {
	            if (eEveResult_On == pObj->CycleFunc(i))
	            {
	                if (JudgeTimeOutMs(pObj->tlCheckTimer[i], pObj->tlAppearTick))
	                {
	                    dev_setErrExsit(i, pObj->eErr, __LINE__);
	                    pObj->tlCheckTimer[i] = NOWTICK;
	                }
	            }
	            else
	            {
	                pObj->tlCheckTimer[i] = NOWTICK;
	            }
	        }
	    }
	}
	return;
}

#define EVE_PWR_DELAY_1S 100

static void EVE_Cycle_task(void)
{
	static uint32_t pwr_delay = 0;
	
	//等待上电采样稳定
	if(pwr_delay < EVE_PWR_DELAY_1S)
	{
		pwr_delay++;
		return;
	}
	
	EVE_CycleCheck();
	return;
}

static void fgv_AppEvent_Init(void)
{

	return;
}

void fgv_AppEventCycleTask(void)
{
	fgv_AppEvent_Init();
	
    while(1)
    {
		
        EVE_Cycle_task();
		
        vTaskDelay(10);
    }
}

