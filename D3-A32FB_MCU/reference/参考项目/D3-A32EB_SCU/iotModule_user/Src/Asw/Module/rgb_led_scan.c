#include "rgb_led_scan.h"

#include "cmsis_os2.h"
#include "AppHeaderSummary.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mbsDataUpdate.h"
#include "tcp_gn.h"
#include "card_user.h"
#include "AppMidDataTrans.h"

static const uint32_t led_Off[Led_MAX_NUM] = {eColor_Off};

#define UNSCREEN_LED	1		//1:无屏版本，0:有屏版本

//根据各个灯的属性进行统一刷新--常亮、闪烁、呼吸、流水
#define LED_INTERVAL_NUM	1	//间隔个数，0表示不间隔

static GN_PLATMOD  *rgbUsePlatmod = &sg_platmod;	//用于本文件中


//0表示不操作，1表示可以操作

//灯珠不是按照正常顺序排列，需要自己定义位置
// const uint8_t gun1ContinuesLedStation[Led_MAX_NUM] = {3,2,1,15,14,13,12,0,0,0,0,0,0,0,0};	//双枪枪1灯位置
// const uint8_t gun2ContinuesLedStation[Led_MAX_NUM] = {11,10,9,8,7,6,5,0,0,0,0,0,0,0,0};	//双枪枪2灯位置
const uint8_t gun1ContinuesLedStation[Led_MAX_NUM] = {13,14,15,1,2,3,4,0,0,0,0,0,0,0,0};	//双枪枪1灯位置
const uint8_t gun2ContinuesLedStation[Led_MAX_NUM] = {6,7,8,9,10,11,12,0,0,0,0,0,0,0,0};	//双枪枪2灯位置
const uint8_t singleContinuesLedStation[Led_MAX_NUM] = {1,2,3,4,5,6,7,8,9,0,0,0,0,0,0};	//单枪灯位置
// const uint8_t singleContinuesLedStation[Led_MAX_NUM] = {10,9,8,7,6,5,4,3,2,1,13,12,11};	//全部测试使用


//下面灯语顺序都是从左到右顺序
static const uint32_t led_WhiteTest[Led_ARRY_MAX_NUM] = {eColor_White, eColor_White, eColor_White, eColor_White};

static const uint32_t led_StaStandby[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Green, eColor_Green, eColor_Green};					//绿绿绿绿，正常模式
static const uint32_t led_StaStandbyCNS[Led_ARRY_MAX_NUM] = {eColor_Blue, eColor_Blue, eColor_Blue, eColor_Blue};					//蓝蓝蓝蓝，国标模式
static const uint32_t led_StaStandbyFac[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Yellow, eColor_Yellow};			//黄黄黄黄，厂内模式

static const uint32_t led_StaInsertgun[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Yellow, eColor_Yellow};	//黄黄黄黄
static const uint32_t led_StaStarting[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Blue, eColor_Blue, eColor_Blue};			//绿绿蓝蓝
static const uint32_t led_StaCharging[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Green, eColor_Green, eColor_Green};			//绿绿绿绿
static const uint32_t led_StaPause[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Yellow, eColor_Yellow, eColor_Yellow};			//绿绿黄黄
static const uint32_t led_StaStopping[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Red, eColor_Red, eColor_Red};				//绿绿红红
static const uint32_t led_StaStopfinish[Led_ARRY_MAX_NUM] = {eColor_Green, eColor_Green, eColor_Green, eColor_Green};		//绿绿绿绿

static const uint32_t led_FaultComm[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_Red};	
				//红红红红
static const uint32_t led_FaultCCU[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_Yellow};					//红红红黄
static const uint32_t led_FaultCard[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_Pink};					//红红红粉
static const uint32_t led_FaultLeak[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_Blue};				//红红红蓝
static const uint32_t led_FaultPhaseLs[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_White};				//红红红白
static const uint32_t led_FaultEStop[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Red, eColor_Green};					//红红红绿
static const uint32_t led_FaultCpAbnmal[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Yellow, eColor_Yellow};			//红红黄黄
static const uint32_t led_FaultCpGnd[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Yellow, eColor_Blue};				//红红黄蓝

static const uint32_t led_FaultPe[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Yellow, eColor_Green};					//红红黄绿
static const uint32_t led_FaultOvVol[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Blue, eColor_Blue};					//红红蓝蓝
static const uint32_t led_FaultUndVol[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Blue, eColor_Yellow};				//红红蓝黄
static const uint32_t led_FaultOvCrt[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Blue, eColor_Green};				//红红蓝绿
static const uint32_t led_FaultSynechia[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Green, eColor_Yellow};			//红红绿黄
static const uint32_t led_FaultMsTrip[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Green, eColor_Blue};				//红红绿蓝
static const uint32_t led_FaultAirOvTemp[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Green, eColor_Green};			//红红绿绿
static const uint32_t led_FaultPlgOvTemp[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Yellow, eColor_Yellow, eColor_Yellow};		//红黄黄黄
static const uint32_t led_FaultIDiode[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Yellow, eColor_Red};		//红红黄红，二极管
static const uint32_t led_FaultLFRevs[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Blue, eColor_Red};		//红红蓝红，反接
static const uint32_t led_FaultShortCircle[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Red, eColor_Green, eColor_Red};	//红红绿红，短路

static const uint32_t led_FaultNosim[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Blue, eColor_Blue, eColor_Blue};	 			//红蓝蓝蓝
static const uint32_t led_FaultSim[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Green, eColor_Green, eColor_Green};	 			//红绿绿绿
static const uint32_t led_FaultIp[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Green, eColor_Blue, eColor_Blue};					//红绿蓝蓝
static const uint32_t led_FaultPlatComm[Led_ARRY_MAX_NUM] = {eColor_Red, eColor_Green, eColor_Yellow, eColor_Yellow};		//红绿黄黄
static const uint32_t led_feemodelillegal[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Blue, eColor_Blue, eColor_Yellow};		//黄蓝蓝黄


static const uint32_t led_UpdateA[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Blue, eColor_Blue, eColor_Blue};			//黄蓝蓝蓝
static const uint32_t led_UpdateB[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Blue, eColor_Blue};			//黄黄蓝蓝
static const uint32_t led_UpdateFTP[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Yellow, eColor_Blue};		//黄黄黄蓝

//card led
static const uint32_t led_CardExsit[Led_ARRY_MAX_NUM] = {eColor_Blue, eColor_Blue, eColor_Blue, eColor_Blue};		//蓝蓝蓝蓝
static const uint32_t led_CardErro[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Yellow, eColor_Red};	//黄黄黄红

//card result
static const uint32_t led_warning[Led_ARRY_MAX_NUM] = {eColor_Yellow, eColor_Yellow, eColor_Yellow, eColor_Yellow};	//黄黄黄黄

led_all_sta g_led_var;


/************************************************
 * 根据单双枪状态选择需要执行的灯
 ************************************************/
static uint8_t *fsv_chooseLedStation(uint8_t uPort)
{
	uint8_t *finalLed = NULL;
	uint8_t F_uPort = 0;
	if (GUN_NUM == GUN_NUM_MAX) {
		F_uPort = uPort + 1;
	}
	if (F_uPort == 0) {
		finalLed = (uint8_t *)singleContinuesLedStation;
	} else if (F_uPort == 1) {
		finalLed = (uint8_t *)gun1ContinuesLedStation;
	} else if (F_uPort == 2) {
		finalLed = (uint8_t *)gun2ContinuesLedStation;
	}
	return finalLed;
}

/************************************************
 * 计算流水灯逐渐降低亮度的颜色
 ************************************************/
static void fsv_CalGrduChangeData(uint32_t *targetLed, uint32_t *originLed, uint8_t ledCnt)
{
	//流行灯珠从左到右依次滑动，右边最亮，左边最暗
	int i;
	uint8_t step = 0;

	// uint8_t ligntPer = 100 / (ledCnt * 3 / 4 + 0.5);
	uint8_t ligntPer = 100 / (ledCnt * 2 / 3 + 0.5);

	COLOR_RGB rgb_v;

	for (i = 0; i < ledCnt; i++) {

		rgb_v.B = originLed[i] >> 16;
		rgb_v.G = originLed[i] >> 8;
		rgb_v.R = originLed[i];

		step += ligntPer;

		led_reduce_fun(step, &rgb_v);
		
    	uint32_t fixLed = (rgb_v.B << 16) | (rgb_v.G << 8) | (rgb_v.R << 0);
		targetLed[ledCnt - i - 1] = fixLed;
	}
}


/**************************************************************************
 * 流水灯--状态改变时，只需要计算一次灯色从亮到暗的变化，后续流水只需要移动数据即可
 **************************************************************************/
void fgv_SetGrduChangeParaInit(uint8_t uPort, uint32_t itvTime)
{
	//连续渐变灯珠，状态改变调用一次即可
	led_all_sta *pLed = &g_led_var;
	led_running *runLed = &pLed->run[uPort];

	uint8_t *t_LedStation = NULL;
	uint8_t t_LedIndex = 0;

	t_LedStation = fsv_chooseLedStation(uPort);

	uint32_t originLed[Led_MAX_NUM];
	int t_cntFnl = 0;
	int i;

	for (i = 0; i < Led_MAX_NUM; i++) {
		if (t_LedStation[i] == 0) {
			continue;
		}
		t_LedIndex = t_LedStation[i] - 1;
		
		//非流水灯状态不进行赋值
		if (pLed->sta[t_LedIndex] != eRunType_Running) {
			continue;
		}

		originLed[t_cntFnl] = pLed->ledSta[t_LedIndex].target_rgb.rgb;
		
		t_cntFnl++;
	}

	//流水灯个数
	runLed->LedMaxCnt = t_cntFnl;		//流水灯个数
	if (runLed->LedMaxCnt > Led_MAX_NUM) {
		runLed->LedMaxCnt = Led_MAX_NUM;
	}
	/*控制流星移动速度, 150ms移动一次*/
	runLed->IntervalTime = itvTime;

	//计算渐变颜色
	fsv_CalGrduChangeData(runLed->RunBuffer, originLed, t_cntFnl);
}


/*********************************************************
 * 流水灯按位移动，制造流水效果，按照初始化配置的时间进行调用
 *********************************************************/
static void fsv_IdpdGrduLedArrayScan(uint8_t uPort, led_all_sta *pLed)
{
	led_running *runLed = &pLed->run[uPort];
	//环形流星灯
	uint8_t circleLedLen = runLed->LedMaxCnt;

	static uint32_t t_RunLed[GUN_NUM_MAX][Led_MAX_NUM];/*最终显示的颜色*/
    	
	uint8_t *t_LedStation = NULL;
	uint8_t t_LedIndex = 0;
	t_LedStation = fsv_chooseLedStation(uPort);

	//将分割开的灯语先整理--》进行流水移位--》再导入当前需要亮的数据中
	memset(t_RunLed, 0, GUN_NUM_MAX * circleLedLen * sizeof(uint32_t));

	//需要顺序整体调换，比如原始数据从左到右是亮到暗，需要最亮的是火车头，从左到右
	for (int i = 0; i < circleLedLen; i++) {
		t_RunLed[uPort][i] = runLed->RunBuffer[i?(i - 1):(circleLedLen - 1)];
	}
	memcpy(runLed->RunBuffer, t_RunLed[uPort], circleLedLen * 4);
	
	int t_cntFnl = 0;
	for (int i = 0; i < Led_MAX_NUM; i++) {
		if (t_LedStation[i] == 0) {
			continue;
		}
		t_LedIndex = t_LedStation[i] - 1;

		//非流水灯状态不进行赋值
		if (pLed->sta[t_LedIndex] != eRunType_Running) {
			continue;
		}
		
		pLed->ledSta[t_LedIndex].current_rgb.rgb = t_RunLed[uPort][t_cntFnl];
		t_cntFnl++;
	}

}

void fgv_RunningLedArray(led_all_sta *pLed, uint8_t uPort)
{
	//连续渐变灯珠，状态改变调用一次即可
	led_running *runLed = &pLed->run[uPort];
	
	if (JudgeTimeOutMs(runLed->StartTick, runLed->IntervalTime) == 0) {
		return;
	} 
	runLed->StartTick = NOWTICK;

	fsv_IdpdGrduLedArrayScan(uPort, pLed);
}

static void fsv_IdpdOffLedData(led_all_sta *pLed, uint8_t i)
{
	if (pLed->sta[i] != eRunType_Off) {
		return;
	}
	pLed->ledSta[i].current_rgb.rgb = eColor_Off;
}


static void fsv_IdpdLightLedData(led_all_sta *pLed, uint8_t i)
{
	if (pLed->sta[i] != eRunType_Light) {
		return;
	}
	pLed->ledSta[i].current_rgb.rgb = pLed->ledSta[i].target_rgb.rgb;
}

static void fsv_IdpdBreathLedData(led_all_sta *pLed, uint8_t i)
{
	if (pLed->sta[i] != eRunType_Breath) {
		return;
	}
	ledStaInfo *ledInfo = &pLed->ledSta[i];
	led_breath *ledBrt = &ledInfo->brt;
	
	if (!JudgeTimeOutMs(ledBrt->StartTick, ledBrt->IntervalTime)) {
		return;
	}

	//防止时间不同步
	ledBrt->StartTick = NOWTICK;

	//5ms进行一次，是呼吸的效果
	//更新hs,为了不更改颜色需要每次更新，因为HSV_TO_RGB里面更改过hs
	led_breath_fun(ledBrt->step, &ledBrt->rhythmFlag, &ledBrt->rgb_v);
	
	ledInfo->current_rgb.color.rgb_r = ledBrt->rgb_v.R;
	ledInfo->current_rgb.color.rgb_g = ledBrt->rgb_v.G;
	ledInfo->current_rgb.color.rgb_b = ledBrt->rgb_v.B;
}

static void fsv_IdpdBlinkLedData(led_all_sta *pLed, uint8_t i)
{
	if (pLed->sta[i] != eRunType_Blink) {
		return;
	}
	pLed->ledSta[i].blk.IntervalTime = TIME_BLINKING_STEP;

	pLed->ledSta[i].blk.rgbBlink[0] = pLed->ledSta[i].target_rgb;
	pLed->ledSta[i].blk.rgbBlink[1].rgb = eColor_Off;

	if (JudgeTimeOutMs(pLed->ledSta[i].blk.StartTick, pLed->ledSta[i].blk.IntervalTime)) {
		pLed->ledSta[i].blk.StartTick = NOWTICK;

		pLed->ledSta[i].current_rgb.rgb = pLed->ledSta[i].blk.rgbBlink[pLed->ledSta[i].blk.blinkCnt].rgb;
		pLed->ledSta[i].blk.blinkCnt++;
		
		if (pLed->ledSta[i].blk.blinkCnt >= BLINK_CNT) {
			pLed->ledSta[i].blk.blinkCnt = 0;
		}
	}
}


static void fsv_ledRunningInit(led_all_sta *pLed, uint8_t uPort)
{
	static uint32_t preLightColor[GUN_NUM_MAX] = {0};
	uint32_t LightColor = pLed->LightArrayColor[uPort];

	//状态改变需要初始化流水灯语颜色
	if (preLightColor[uPort] != LightColor) {
		fgv_SetGrduChangeParaInit(uPort, TIME_RUNNING_STEP);		//流水灯颜色改变或者状态改变配置参数
		preLightColor[uPort] = LightColor;
	}
}

static void fsv_ledBreathingInit(led_all_sta *pLed, uint8_t i)
{
	ledStaInfo *ledInfo = &pLed->ledSta[i];
	led_breath *ledBrt = &ledInfo->brt;

	static COLOR_UN breathCrtColor[Led_MAX_NUM] = {0};

	if (pLed->sta[i] != eRunType_Breath) {
		return;
	}

	//流水呼吸等于变化需要初始化灯语颜色	
	if (breathCrtColor[i].rgb != ledInfo->target_rgb.rgb) {
		ledBrt->rgb_v.R = ledInfo->target_rgb.color.rgb_r;
		ledBrt->rgb_v.G = ledInfo->target_rgb.color.rgb_g;
		ledBrt->rgb_v.B = ledInfo->target_rgb.color.rgb_b;
		ledBrt->rhythmFlag = 0;
		ledBrt->step = 2;
		ledBrt->IntervalTime = TIME_BREATH_STEP;

		breathCrtColor[i].rgb = ledInfo->target_rgb.rgb;
	}
}




void fgv_SetSingleLedColor(uint8_t num, const uint32_t color)
{
	led_all_sta *pLed = &g_led_var;
	
	pLed->sta[num] = eRunType_Light;
	pLed->ledSta[num].target_rgb.rgb = color;

}
void fgv_SetWholeLedColor(const uint32_t color)
{
	led_all_sta *pLed = &g_led_var;
	
	for (int i = 0; i < Led_MAX_NUM; i++) {
		pLed->sta[i] = eRunType_Light;
		pLed->ledSta[i].target_rgb.rgb = color;
	}

}




//uPort = 0表示单枪， 1,2表示双枪-枪1枪2
void fgv_SetSingleLedType(uint8_t uPort, const uint32_t *color, uint8_t itv, eLedRunType type)
{
	//分割方式,itv=1需要分割，itv=0不需要分割
	#define INTERNAL_OFF	Led_ARRY_MAX_NUM

	const uint8_t normalSta[Led_GROUP_NUM] = {0,0,1,1,2,2,3};
	const uint8_t normalSingleSta[Led_SINGLE_NUM] = {0,0,0,1,1,2,2,3,3};
	const uint8_t internalSta[Led_GROUP_NUM] = {0, INTERNAL_OFF, 1, INTERNAL_OFF, 2, INTERNAL_OFF, 3};
	const uint8_t internalSingleSta[Led_SINGLE_NUM] = {INTERNAL_OFF, 0, INTERNAL_OFF, 1, INTERNAL_OFF, 2, INTERNAL_OFF, 3, INTERNAL_OFF};

	uint8_t itvlLedStation[Led_SINGLE_NUM] = {0};
	
	uint8_t l_GroupNum = Led_GROUP_NUM;
	if (GUN_NUM == 1) {
		l_GroupNum = Led_SINGLE_NUM;
	}

	if (itv) {
		(GUN_NUM == 1) ? memcpy(itvlLedStation, internalSingleSta, l_GroupNum) : memcpy(itvlLedStation, internalSta, l_GroupNum);
	} else {
		(GUN_NUM == 1) ? memcpy(itvlLedStation, normalSingleSta, l_GroupNum) : memcpy(itvlLedStation, normalSta, l_GroupNum);
	}
	uint8_t t_ivtCnt = 0;
	uint8_t t_LedIndex = 0;

	led_all_sta *pLed = &g_led_var;

	// pLed->LightArraySta[uPort] = type;
	pLed->LightArrayColor[uPort] = (uint32_t)color;

	uint8_t *t_LedStation = NULL;

	t_LedStation = fsv_chooseLedStation(uPort);

	for (int i = 0; i < Led_MAX_NUM; i++) {
		if (t_LedStation[i] == 0) {
			continue;
		}
		
		//异常过滤掉
		if (itvlLedStation[t_ivtCnt] > Led_ARRY_MAX_NUM) {
			continue;
		}
		if (t_ivtCnt >= l_GroupNum) {
			continue;
		}

		t_LedIndex = t_LedStation[i] - 1;
		pLed->sta[t_LedIndex] = type;
		pLed->ledSta[t_LedIndex].target_rgb.rgb = color[itvlLedStation[t_ivtCnt]];

		if (itvlLedStation[t_ivtCnt] == INTERNAL_OFF) {	//不操作
			// pLed->sta[t_LedIndex] = eRunType_Off;
            pLed->ledSta[t_LedIndex].target_rgb.rgb = eColor_Off;
		}
		
		t_ivtCnt++;
	}
}




//灯语数据进行刷新，最后统一进行展示
void fsv_LedDataRefresh()
{
	led_all_sta *pLedVar = &g_led_var;

	for (int i = 0; i < GUN_NUM; i++) {
		fsv_ledRunningInit(pLedVar, i);
		fgv_RunningLedArray(pLedVar, i);	//流水灯
	}

	for (int i = 0; i < Led_MAX_NUM; i++) {
		fsv_ledBreathingInit(pLedVar, i);
		fsv_IdpdBreathLedData(pLedVar, i);	//呼吸
	}

	for (int i = 0; i < Led_MAX_NUM; i++) {
		fsv_IdpdOffLedData(pLedVar, i);		//关闭
		fsv_IdpdLightLedData(pLedVar, i);	//常亮
		fsv_IdpdBlinkLedData(pLedVar, i);	//闪烁
	}


	for (int i = 0; i < Led_MAX_NUM; i++) {
		pLedVar->current_rgb[i] = pLedVar->ledSta[i].current_rgb;
	}
}

uint8_t fsv_UpdateLed(uint8_t i)
{
	uint8_t interval_flag = 1;	//1隔断
	uint8_t rslt = 1;
    uint8_t updateN = fgv_getUpdataObj();
    
    if (OtaGetUpdatingFlag() == 0) {
        return 0;
    }
    if (updateN == eUpdateObj_A) {
		fgv_SetSingleLedType(i, led_UpdateA, interval_flag, eRunType_Blink);
    } else if (updateN == eUpdateObj_B) {
		fgv_SetSingleLedType(i, led_UpdateB, interval_flag, eRunType_Blink);
    } else {
		fgv_SetSingleLedType(i, led_UpdateFTP, interval_flag, eRunType_Blink);
    }
	return rslt;
}


uint8_t fsv_hardfaultLed(uint8_t i)
{
	uint8_t interval_flag = 1;	//1隔断
	uint8_t rslt = 1;

	if (dev_getErrExsit(i, eErr_CCUSCUCommErr)) {
		fgv_SetSingleLedType(i, led_FaultCCU, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_ReaderCommErr)) {
		fgv_SetSingleLedType(i, led_FaultCard, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_LeakageCurrErr)) {
		fgv_SetSingleLedType(i, led_FaultLeak, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_PhaseLossErr)) {
		fgv_SetSingleLedType(i, led_FaultPhaseLs, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_EmergencyStop)) {
		fgv_SetSingleLedType(i, led_FaultEStop, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_CpGroundFault)) {
		fgv_SetSingleLedType(i, led_FaultCpGnd, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_CpVoltAbnor)) {
		fgv_SetSingleLedType(i, led_FaultCpAbnmal, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_PEBreakFault)) {
		fgv_SetSingleLedType(i, led_FaultPe, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_InputOverVol)) {
		fgv_SetSingleLedType(i, led_FaultOvVol, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_InputLessVol)) {
		fgv_SetSingleLedType(i, led_FaultUndVol, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_OutputOverCurr)) {
		fgv_SetSingleLedType(i, led_FaultOvCrt, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_JcqSynechiaFault)) {
		fgv_SetSingleLedType(i, led_FaultSynechia, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_JcqMaloperation)) {
		fgv_SetSingleLedType(i, led_FaultMsTrip, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_EnvOverTempErr)) {
		fgv_SetSingleLedType(i, led_FaultAirOvTemp, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_POverTempErr)) {
		fgv_SetSingleLedType(i, led_FaultPlgOvTemp, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_DiodeStop)) {
		fgv_SetSingleLedType(i, led_FaultIDiode, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_InputLineReversed)) {
		fgv_SetSingleLedType(i, led_FaultLFRevs, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_ShortCircleErro)) {
		fgv_SetSingleLedType(i, led_FaultShortCircle, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_NetNoSIMErr)) {
		fgv_SetSingleLedType(i, led_FaultNosim, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_NetSIMErr)) {
		fgv_SetSingleLedType(i, led_FaultSim, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_NetIPErr)) {
		fgv_SetSingleLedType(i, led_FaultIp, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_PlatformOffline)) {
		fgv_SetSingleLedType(i, led_FaultPlatComm, interval_flag, eRunType_Blink);
	}
	else if (dev_getErrExsit(i, eErr_feemodelErr)) {
		fgv_SetSingleLedType(i, led_feemodelillegal, interval_flag, eRunType_Blink);
	}
	else if (dev_getDevHardfalt(i)) {
		fgv_SetSingleLedType(i, led_FaultComm, interval_flag, eRunType_Blink);
	} else {
		rslt = 0;
	}
	return rslt;
}

uint8_t fsv_cardLed(uint8_t i)
{
	uint8_t rslt = 1;
	IC_T *pLed_ic = Get_IC_Msg(i);
	if (pLed_ic->opt_sts.ValidCard_ok_2b == 2) {
		fgv_SetSingleLedType(i, led_CardErro, 0, eRunType_Light);
	}
	else if (pLed_ic->opt_sts.ExistCard_ok_1b) {
		fgv_SetSingleLedType(i, led_CardExsit, 0, eRunType_Light);
	}  else {
		rslt = 0;
	}
	return rslt;
}
uint8_t fsv_Warning(uint8_t i)
{
	uint8_t rslt = 1;
    uint8_t sta = GetPlat_CardChargeFaild(i);
    if (sta) {
		fgv_SetSingleLedType(i, led_warning, 0, eRunType_Blink);
    } else {
		rslt = 0;
	}
	return rslt;
}



uint8_t fsv_LedStaRefresh(uint8_t i)
{
	uint8_t chargeSta = 0;
	uint8_t rslt = 1;

	chargeSta = rgbUsePlatmod->gun[i].gunRtInfo.gun_ChrgSta;

	if (chargeSta == eChargeState_Idle) {
		if (rgbUsePlatmod->pileCfgInfo.fct_cfg.bit.cgfPlugChrg) {
			fgv_SetSingleLedType(i, led_StaStandbyFac, 0, eRunType_Breath);
		} else if (fgv_GetPileCfgNationalStandard()) {
			fgv_SetSingleLedType(i, led_StaStandbyCNS, 0, eRunType_Breath);
		} else {
			fgv_SetSingleLedType(i, led_StaStandby, 0, eRunType_Breath);
		}
	} else if (chargeSta == eChargeState_Waiting) {
		fgv_SetSingleLedType(i, led_StaInsertgun, 0, eRunType_Light);
	} else if (chargeSta == eChargeState_Starting) {
		fgv_SetSingleLedType(i, led_StaStarting, 0, eRunType_Running);
	} else if (chargeSta == eChargeState_Charging) {
		fgv_SetSingleLedType(i, led_StaCharging, 0, eRunType_Running);
	} else if (chargeSta == eChargeState_PauseB) {
		fgv_SetSingleLedType(i, led_StaPause, 0, eRunType_Running);
	} else if (chargeSta == eChargeState_Stoping) {
		fgv_SetSingleLedType(i, led_StaStopping, 0, eRunType_Running);
	} else if (chargeSta == eChargeState_StopFinish) {
		fgv_SetSingleLedType(i, led_StaStopfinish, 0, eRunType_Light);
	} else {
		rslt = 0;
	}
	return rslt;
}


void LedDataTrans_Scan()
{
	//50ms刷新一次
	static uint32_t ledTrans_sTick = 0;

	if (JudgeTimeOutMs(ledTrans_sTick, eTick_50ms) == FALSE) {
		return;
	}
	ledTrans_sTick = NOWTICK;

	led_all_sta *pLedVar = &g_led_var;

	ws281x_BufferUpdate(&pLedVar->current_rgb->rgb, Led_MAX_NUM);

	ws281x_BufferTransfer((uint8_t *)pixelBuffer,sizeof(pixelBuffer));
}

//测试灯板灯珠是否正常
static void rgbTest()
{
#if Led_RGB_TEST == 0
    return;
#endif
    static uint8_t clrCnt = 0;    //0红，1绿， 2蓝
    static uint32_t t_time = 0;    //计时，1s跳一次颜色
    uint32_t clrValue = eColor_Red;    //0红，1绿， 2蓝
    if (clrCnt == 0) {
        clrValue = eColor_Red;
    } else if (clrCnt == 1) {
        clrValue = eColor_Green;
    } else if (clrCnt == 2) {
        clrValue = eColor_Blue;
    }
    led_all_sta *pLedVar = &g_led_var;
    for (int i = 0; i < Led_MAX_NUM; i++) {
        pLedVar->sta[i] = eRunType_Light;
        fgv_SetSingleLedColor(i, clrValue);
    }
    
    t_time++;
    if (t_time > (1000 / 20)) {
        t_time = 0;
        clrCnt++;
        if (clrCnt >= 3) {
            clrCnt = 0;
        }
    }
}

/*****************************************************************************
 * 函 数 名  : RGB_LED_Main
 * 负 责 人  : WEEN
 * 创建日期  : 2021年5月1日
 * 函数功能  : 灯带控制主函数
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  : called by main
 * 其    它  :
*****************************************************************************/
void RGB_LED_Main()
{
	LED_Timer_Init();

	while(1)
	{
		osDelay(20);
		
		for (int i = 0; i < GUN_NUM; i++) {
			//升级
			if (fsv_UpdateLed(i)) {
				continue;
			}
			//警示
			if (fsv_Warning(i)) {
				continue;
			}
			//刷卡
			if (fsv_cardLed(i)) {
				continue;
			}            
			//故障
			if (fsv_hardfaultLed(i)) {
				continue;
			}
			//充电
			if (fsv_LedStaRefresh(i)) {
				continue;
			}
		}

        rgbTest();

		//灯语数据统一刷新
		fsv_LedDataRefresh();

		//灯语数据统一传输
		LedDataTrans_Scan();
	}
}
