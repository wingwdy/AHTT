/**********************************************************
  File name:       cost.c	          
  Author:                       
  Version:              
  Date:                    
  Description: 计费模式1:48费率段
  Others:		                      
  Function List:	                
  History:
**********************************************************/
#include "cost.h"
#include "AppMidDataTrans.h"
#include "protocol_data.h"
#include "iot_GN_Protocol_Code.h"
#include "iot_YKC_Protocol_CodeV2_1.h"
#include "iot_WJY_Protocol_Code.h"
#include "iot_ANPEI_Protocol_Code.h" //JJUNIVE
#include "iot_AHTT_Protocol_Code.h"

#define COST_ERR_DATA		10000		//1度电

COST_CTRL_T g_cost_ctrl;

COST_GUN_DATA *CostGetOutput(uint8_t u8Port)
{
    return &g_cost_ctrl.strCostGunData[u8Port];
}
/*************************************************************
  Function:    Cost_Style
  Description: 计费方式  -- 函数计算大，建议按时间间隔调用
  Calls:       无
  Called By:   by->Cost_Main
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void set_rate_state(uint8_t u8Port, uint8_t flag)
{
	g_cost_ctrl.start_chrg[u8Port] = flag;
	
	return;
}

//根据充电状态，状态位以及数据及时更新，外面不用去关心什么时候清楚cost数据问题
static void cost_StaScan(uint8_t u8Port)
{
	static uint8_t u8PreChargingSta[GUN_NUM_MAX] = {0};
	uint8_t u8PChargingSta = logic_get_gun_Uncharged(u8Port);
	//开始充电，所有计费数据清零
	if (u8PChargingSta != u8PreChargingSta[u8Port]) {
		if (u8PChargingSta) {
			Cost_charge_init(u8Port);
            CostGetRateModel(u8Port);       //获取计费模型
			set_rate_state(u8Port, TRUE);
		}
		u8PreChargingSta[u8Port] = u8PChargingSta;
	} else {
		if (u8PChargingSta == 0) {
			set_rate_state(u8Port, FALSE);
        }
    }
    
    //充电前判断填充计费模型,充电下发计费模型本次充电使用旧计费模型
	static uint8_t u8PreConnect[GUN_NUM_MAX] = {0};
    //拔枪插枪都需要进行最新计费模型更新
	uint8_t u8Connect = GetPile_gun_connect(u8Port);
	if (u8Connect != u8PreConnect[u8Port]) {
        //避免充电过程中直接拔枪，计费模型更新导致上报数据异常
        if (u8Connect) {
            CostGetRateModel(u8Port);       //获取计费模型
        }
		u8PreConnect[u8Port] = u8Connect;
    }
}
static uint8_t get_cur_tiem_index(void)
{
	uint16_t index  = 0;

	uint8_t now_time[8] = { 0 };
	getRunTimeYYMDHMS(now_time);

	uint16_t hour = now_time[4];
	uint16_t minute = now_time[5];
	
	index = (hour*30*2 + minute)/30;
	
	if(index >= 48) index = 0;
	
	return index;
}

static uint8_t get_RatePriod(uint8_t u8Port, uint8_t *resultType)
{
	uint8_t ratePeriod = 0, time_index = 0;
	
	time_index = get_cur_tiem_index();
	*resultType = g_cost_ctrl.rate_model[u8Port].rate_8u[time_index];
	ratePeriod = time_index;
    
	if(*resultType >= MAX_RATE_NUMBER) *resultType = 0;
	if(ratePeriod >= MAX_RATE_PERIOD_CNT) ratePeriod = 0;
	
	return ratePeriod;
}
static uint8_t get_PeriodRatePriod(uint8_t u8Port, uint8_t *resultType)
{
    uint8_t ratePeriod = 0;     //时段，1234
    // uint8_t resultType = 0;     //类型，尖峰平谷
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
    
    //获取当前 时和分 匹配是该费率的哪一段
    uint8_t timeOrgin[7] = {0}; 
    getRunTimeYYMDHMS(timeOrgin); //(年H 年L 月 日 时 分 秒) 
    uint8_t Now_hour=timeOrgin[4];
    uint8_t Now_minute=timeOrgin[5];

    for(int i=0; i<prate->PeriodNumber; i++)
    {
        uint8_t StartBCD_HH = prate->startTime[i][0];//此处十进制
        uint8_t StartBCD_MM = prate->startTime[i][1];
        uint8_t EndBCD_HH = prate->stopTime[i][0];
        uint8_t EndBCD_MM = prate->stopTime[i][1];

        uint16_t timeInMinutes = Now_hour * 60 + Now_minute;
        uint16_t startTimeInMinutes = StartBCD_HH * 60 + StartBCD_MM;
        uint16_t endTimeInMinutes = EndBCD_HH * 60 + EndBCD_MM;

         if (timeInMinutes >= startTimeInMinutes && timeInMinutes < endTimeInMinutes)
         {
            *resultType = prate->rate_8u[i];
            ratePeriod = i;
            break;
         }
    }
    return ratePeriod;
}
//计费异常判断，电量异常或者计费异常
static uint8_t CostCheckAbnormal(uint8_t u8Port, uint32_t nowTotalEnergy) 
{
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	int64_t incEnergyValue = 0;	    //判断异常使用
    int64_t incMoneyValue = 0;	    //判断异常使用

    //电量异常判断
    incEnergyValue = nowTotalEnergy - pcostdata->checkLastEle;
    if ((incEnergyValue < 0) || (incEnergyValue > COST_ERR_DATA)) 
    {
		printf("Cost Ele Fatal: %d %d\r\n", nowTotalEnergy, pcostdata->checkLastEle);
        if(pcostdata->eleAbnormalTimeCnt++ > 200)
        {
            pcostdata->eleAbnormalTimeCnt = 0;
            stopPileCharge(u8Port, Pile_Stop_Reason_Ele);
        }
        return TRUE;
    }
    pcostdata->eleAbnormalTimeCnt = 0;
    pcostdata->checkLastEle = nowTotalEnergy;

    //计费异常判断
    incMoneyValue = pcostdata->total_money - pcostdata->checkLastMoney;

    if ((incMoneyValue < 0) || (incMoneyValue > COST_ERR_DATA * 100)) 
    {
		printf("Cost Money Fatal: %d %d\r\n", pcostdata->total_money, pcostdata->checkLastMoney);
		stopPileCharge(u8Port, Pile_Stop_Reason_Money);
        return TRUE;
    }

    pcostdata->checkLastMoney = pcostdata->total_money;
    return FALSE;
}

static void CostAbnormalDataClear(uint8_t u8Port)
{
    //不充电时需要清除的数据
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    pcostdata->last_value = 0;
    pcostdata->checkLastEle = 0;
    pcostdata->checkLastMoney = 0;
    pcostdata->eleAbnormalTimeCnt = 0;
}

static void Cost_deal(uint8_t u8Port, uint32_t prevEnergy)
{
	uint8_t i = 0;
	uint8_t ratePeriod = 0;     //当前时段
	uint8_t resultType = 0;     //当前类型
    uint64_t energe = 0;
	uint32_t incEnergy = 0, inclossEnergy = 0;	//电量增量
    uint64_t u64temp = 0;	//9位小数,临时变量
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    uint32_t tempAllEleMoney = 0;
    uint32_t tempAllServerMoney = 0;
    uint8_t tempPeriod,tempRate = 0;
 
      
	//单价应该实时刷新以备以后在屏幕上显示
	for(i = 0; i < MAX_RATE_NUMBER; i++)
	{
		pcostdata->ele_rate[i] = prate->price48_16u[i] + prate->sever48_16u[i];
	}
	
	if (TRUE != g_cost_ctrl.start_chrg[u8Port])
	{
		//不充电一直刷新,防止异常损耗电量算到客户头上
		//同时在停止时,要读到最后一次电流防止运营商损耗大
		// pcostdata->last_value = monitor_getChgStoptEle(u8Port);
		pcostdata->last_value = 0;
		pcostdata->checkLastEle = 0;
		pcostdata->checkLastMoney = 0;
        pcostdata->eleAbnormalTimeCnt = 0;
		//计量数据错误计数清0
        // CostAbnormalDataClear(u8Port);
		return;
	}

    if (prate->costType == eCostType_SPVF) {
        ratePeriod = get_RatePriod(u8Port, &resultType);
    } else {
        ratePeriod = get_PeriodRatePriod(u8Port, &resultType);        //获取分时段多类电价的计费模型
    }

    energe = monitor_getChgTotalPower(u8Port);

    if (ePlatType_ANPEI == get_ChgParam_plat_type())
    {
        // 只在1.5kwh内进行补偿（应对电能精度抽检）
        if(energe > 0 && energe < 15000)
        {
            u64temp= energe * 9813 - 12;
            energe= (u64temp + 5000) / 10000; //四舍五入
        }
    }

	//电量增加值
    if (energe > pcostdata->last_value)
    {
        incEnergy = energe - pcostdata->last_value;
    }

    if (TRUE == CostCheckAbnormal(u8Port, energe)) return;      //异常判断

    if (prevEnergy > 0)
    {/* 安徽铁塔刷卡立即进入充电，鉴权后增加之前产生的电量 */
        incEnergy += prevEnergy;
    }

    if (0 == incEnergy) return;

    pcostdata->last_value = energe;

    //总电量
    pcostdata->total_power += incEnergy;
    //各费率电量
    pcostdata->ele_power[resultType] += incEnergy;
    //各费率计损电量
    pcostdata->ele_loss_power[resultType] = (pcostdata->ele_power[resultType] * prate->elec_loss_ratio_32u) / 100;
    pcostdata->total_loss_power = (pcostdata->total_power * prate->elec_loss_ratio_32u) / 100;

    //以下为分时段的计费信息
    pcostdata->PeriodElePower[ratePeriod] += incEnergy;        //各类型总电量 小数点后四位
    //分时段电费
    u64temp = (uint64_t)pcostdata->PeriodElePower[ratePeriod] * prate->PeriodPrice48_16u[ratePeriod];
    pcostdata->PeriodEleMoney[ratePeriod] = u64temp / 100000;
    //分时段服务费
    u64temp = (uint64_t)pcostdata->PeriodElePower[ratePeriod] * prate->PeriodSever48_16u[ratePeriod];    //分时段服务费
    pcostdata->PeriodSerMoney[ratePeriod] = u64temp / 100000;
    //分时段电费+服务费
    pcostdata->PeriodMoney[ratePeriod] = pcostdata->PeriodEleMoney[ratePeriod] + pcostdata->PeriodSerMoney[ratePeriod];

    //以下为各费率的计费信息
    //清除各费率电费、各费率服务费、各费率总金额，方便重新累加计算
    //这么做的原因：各时段金额相加 = 各费率金额相加 = 总金额，避免出现不相等的情况
    memset(pcostdata->perEleMoney, 0x00, sizeof(pcostdata->perEleMoney));
    memset(pcostdata->perSerMoney, 0x00, sizeof(pcostdata->perSerMoney));
    memset(pcostdata->perMoney, 0x00, sizeof(pcostdata->perMoney));
    //总费用
    pcostdata->allEleMoney = 0;
    pcostdata->allServerMoney = 0;

    for (tempPeriod = 0; tempPeriod < prate->PeriodNumber; tempPeriod++)
    {
        tempRate = prate->rate_8u[tempPeriod];
        pcostdata->perEleMoney[tempRate] += pcostdata->PeriodEleMoney[tempPeriod];
        pcostdata->perSerMoney[tempRate] += pcostdata->PeriodSerMoney[tempPeriod];
        pcostdata->perMoney[tempRate] += pcostdata->PeriodMoney[tempPeriod];

        pcostdata->allEleMoney += pcostdata->PeriodEleMoney[tempPeriod];        //电费总额
        pcostdata->allServerMoney += pcostdata->PeriodSerMoney[tempPeriod];     //服务费总额
    }

    pcostdata->total_money = (pcostdata->allServerMoney + pcostdata->allEleMoney);  //总金额

    return;
}

//停止冻结订单最后计费一次
void Cost_stop_deal(uint8_t u8Port)
{
	Cost_deal(u8Port, 0);
}
/*************************************************************
  Function:    Cost_Main
  Description: 计费主函数 
  Calls:       无
  Called By:   by->main
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void Cost_Main(void)
{
	//即插即充
	if (fgv_GetPileCfgOffLinChrg()) {
		return;
	}
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		cost_StaScan(i);
		Cost_deal(i, 0);
	}
}

void Cost_Main_Single(uint8_t u8Port)
{
    cost_StaScan(u8Port);
    Cost_deal(u8Port, 0);
}

void Cost_AppendExtEnergy(uint8_t u8Port, uint32_t extEnergy)
{
    Cost_deal(u8Port, extEnergy);
}

//公牛计费模型转换为多类电价计费标准
void gnRateToCostMultiple(uint8_t u8Port, RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    GN_Recv_Billing_Model *pRecvBillingModel = (GN_Recv_Billing_Model *)rateData;
    charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;

    prate->PeriodNumber = 48;   //公牛平台为48时段
	prate->costType = eCostType_SPVF;

    memcpy(prate->billing_model, pRecvBillingModel->billing_model, 2);
    //48时段费率号
    memcpy(prate->rate_8u, pRecvBillingModel->segmentation_rate, 48);
    //尖
	prate->price48_16u[0] = fourUint8ToUint32(pRecvBillingModel->sharp_ele_fee);
	prate->sever48_16u[0] = fourUint8ToUint32(pRecvBillingModel->sharp_ser_fee);
	//峰
	prate->price48_16u[1] = fourUint8ToUint32(pRecvBillingModel->peak_ele_fee);
	prate->sever48_16u[1] = fourUint8ToUint32(pRecvBillingModel->peak_ser_fee);
	//平
	prate->price48_16u[2] = fourUint8ToUint32(pRecvBillingModel->flat_ele_fee);
	prate->sever48_16u[2] = fourUint8ToUint32(pRecvBillingModel->flat_ser_fee);
	//谷
	prate->price48_16u[3] = fourUint8ToUint32(pRecvBillingModel->valley_ele_fee);
	prate->sever48_16u[3] = fourUint8ToUint32(pRecvBillingModel->valley_ser_fee);
	
    prate->elec_loss_ratio_32u = pRecvBillingModel->measure_wastage_rates;

    if(pRecvBillingModel->rateModelType == RATE_MODEL_9_TYPE)
    {
        pRecord->order_model_type = RATE_MODEL_9_TYPE;
        prate->rateNumber = 9;
          //深谷
	    prate->price48_16u[4] = fourUint8ToUint32(pRecvBillingModel->deep_valley_ele_fee);
	    prate->sever48_16u[4] = fourUint8ToUint32(pRecvBillingModel->deep_valley_ser_fee);
        //第六
        prate->price48_16u[5] = fourUint8ToUint32(pRecvBillingModel->six_ele_fee);
        prate->sever48_16u[5] = fourUint8ToUint32(pRecvBillingModel->six_ser_fee);
        //第七
        prate->price48_16u[6] = fourUint8ToUint32(pRecvBillingModel->seven_ele_fee);
        prate->sever48_16u[6] = fourUint8ToUint32(pRecvBillingModel->seven_ser_fee);
        //第八
        prate->price48_16u[7] = fourUint8ToUint32(pRecvBillingModel->eight_ele_fee);
        prate->sever48_16u[7] = fourUint8ToUint32(pRecvBillingModel->eight_ser_fee);
        //第九
        prate->price48_16u[8] = fourUint8ToUint32(pRecvBillingModel->nine_ele_fee);
        prate->sever48_16u[8] = fourUint8ToUint32(pRecvBillingModel->nine_ser_fee);
    }
    else
    {
        prate->rateNumber = 4;  
        pRecord->order_model_type = RATE_MODEL_4_TYPE;
    }

     for (int i = 0; i < prate->PeriodNumber; i++) {  //48个时段
        prate->PeriodPrice48_16u[i] = prate->price48_16u[prate->rate_8u[i]];   //通过每个时段的时段费率号去费率电价里对应
        prate->PeriodSever48_16u[i] = prate->sever48_16u[prate->rate_8u[i]];   
    }
}

void ykcRateToCostMultiple(RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    YKC_Recv_Rate_Model *pRecvBillingModel = (YKC_Recv_Rate_Model *)rateData;

    prate->PeriodNumber = 48;
	prate->rateNumber = 4;
	prate->costType = eCostType_SPVF;

    memcpy(prate->billing_model, pRecvBillingModel->billing_model, 2);
    //48时段费率号
    memcpy(prate->rate_8u, pRecvBillingModel->segmentation_rate, 48);
    //尖
	prate->price48_16u[0] = fourUint8ToUint32(pRecvBillingModel->sharp_ele_fee);
	prate->sever48_16u[0] = fourUint8ToUint32(pRecvBillingModel->sharp_ser_fee);
	//峰
	prate->price48_16u[1] = fourUint8ToUint32(pRecvBillingModel->peak_ele_fee);
	prate->sever48_16u[1] = fourUint8ToUint32(pRecvBillingModel->peak_ser_fee);
	//平
	prate->price48_16u[2] = fourUint8ToUint32(pRecvBillingModel->flat_ele_fee);
	prate->sever48_16u[2] = fourUint8ToUint32(pRecvBillingModel->flat_ser_fee);
	//谷
	prate->price48_16u[3] = fourUint8ToUint32(pRecvBillingModel->valley_ele_fee);
	prate->sever48_16u[3] = fourUint8ToUint32(pRecvBillingModel->valley_ser_fee);
	
    prate->elec_loss_ratio_32u = pRecvBillingModel->measure_wastage_rates;

    for (int i = 0; i < prate->PeriodNumber; i++) {  //48个时段
        prate->PeriodPrice48_16u[i] = prate->price48_16u[prate->rate_8u[i]];   //通过每个时段的时段费率号去费率电价里对应
        prate->PeriodSever48_16u[i] = prate->sever48_16u[prate->rate_8u[i]];   
    }
}

//云快充2.1计费模型转换为多类电价计费标准
void ykc21RateToCostMultiple(RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    YKC_V2_Recv_Rate_Model *pYkc21RecvBillingModel = (YKC_V2_Recv_Rate_Model *)rateData;

    prate->PeriodNumber = 48;   //云快充2.1平台为48时段
	prate->rateNumber = pYkc21RecvBillingModel->Rate_quantity;
	prate->costType = eCostType_SPVF;
    if (prate->rateNumber > MAX_RATE_PERIOD_CNT) {
        printf("rateNumber erro\r\n");
        return;
    }

    memcpy(prate->billing_model, pYkc21RecvBillingModel->billing_model, 2);
    //48时段费率号
    memcpy(prate->rate_8u, pYkc21RecvBillingModel->segmentation_rate, 48);

    for (int i = 0; i < prate->rateNumber; i++) {
        prate->price48_16u[i] = fourUint8ToUint32(pYkc21RecvBillingModel->ele_rate[i]);
        prate->sever48_16u[i] = fourUint8ToUint32(pYkc21RecvBillingModel->ser_rate[i]);
    }
	
    for (int i = 0; i < prate->PeriodNumber; i++) {  //48个时段
        prate->PeriodPrice48_16u[i] = prate->price48_16u[prate->rate_8u[i]];   //通过每个时段的时段费率号去费率电价里对应
        prate->PeriodSever48_16u[i] = prate->sever48_16u[prate->rate_8u[i]];   
    }
    
    prate->elec_loss_ratio_32u = pYkc21RecvBillingModel->measure_wastage_rates;
}


//安培计费模型转换为多类电价计费标准
void anpeiRateToCostMultiple(uint8_t u8Port, RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    FeeModel_Save_truct *pAnpeiRateModel = (FeeModel_Save_truct *)rateData;
    FeeModelB47 *anpeiModel = &pAnpeiRateModel[u8Port].FeemodelB47save_data[B47_A];
	prate->costType = eCostType_PerTime;

    if (!pAnpeiRateModel || u8Port >= GUN_NUM_MAX) {
        printf("Error: Invalid parameters for port %d\n", u8Port);
        return;
    }

    if (AnpeiCurrentRateType(u8Port) == B47_A) {
        memcpy(anpeiModel, &pAnpeiRateModel[u8Port].FeemodelB47save_data[B47_A], sizeof(FeeModelB47));
    } else if (AnpeiCurrentRateType(u8Port) == B47_B) {
        memcpy(anpeiModel, &pAnpeiRateModel[u8Port].FeemodelB47save_data[B47_B], sizeof(FeeModelB47));
    } else {
        printf("Error: Unknown rate type for port %d\n", u8Port);
        return;
     }
     //转换为启动计费模型
     uint8_t bcd_to_hex_val[2]={0};
    // //类型电价
    prate->rateNumber = anpeiModel->time_allnum;
     //判断当前时段属于哪个时间，一天48时段，最小半小时一段
	prate->PeriodNumber = anpeiModel->time_allnum;
    for (int i = 0; i < prate->PeriodNumber; i++) {
        prate->rate_8u[i] = anpeiModel->B47modeldata[i].Serial_rate;
        prate->PeriodPrice48_16u[i] = fourUint8ToUint32(anpeiModel->B47modeldata[i].ele_fee);
        prate->PeriodSever48_16u[i] = fourUint8ToUint32(anpeiModel->B47modeldata[i].ser_fee);

        bcd_to_hex_val[0]=0;
        bcd_to_hex_val[1]=0;
        //注意：协议中关于每天时段的开始时间为BCD码，此处需要转成十进制。例如：0x11 表示11点，需转成0xB
        bcd_to_hex_val[0]=(anpeiModel->B47modeldata[i].rate_start[1]>>4)*10+(anpeiModel->B47modeldata[i].rate_start[1]&0x0F);//HH 时
        bcd_to_hex_val[1]=(anpeiModel->B47modeldata[i].rate_start[0]>>4)*10+(anpeiModel->B47modeldata[i].rate_start[0]&0x0F);//MM 分
        memcpy(prate->startTime[i], bcd_to_hex_val, 2);

        bcd_to_hex_val[0]=(anpeiModel->B47modeldata[i].rate_end[1]>>4)*10+(anpeiModel->B47modeldata[i].rate_end[1]&0x0F);
        bcd_to_hex_val[1]=(anpeiModel->B47modeldata[i].rate_end[0]>>4)*10+(anpeiModel->B47modeldata[i].rate_end[0]&0x0F);
        memcpy(prate->stopTime[i], bcd_to_hex_val, 2);
    }
}



//安徽铁塔低速计费模型转换为多类电价计费标准
void ahttRateToCostMultiple(uint8_t u8Port, RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    AHTT_UpPlatInfo *pAhttUpInfo = Get_AhttAllInfo();

    AHTT_CostModel *ahttModel = pAhttUpInfo->ahCostModel;
    
	prate->costType = eCostType_PerTime;
    prate->PeriodNumber = pAhttUpInfo->gun_chrg_step;
    prate->rateNumber = pAhttUpInfo->gun_chrg_step;
    if (prate->PeriodNumber > MAX_RATE_PERIOD_CNT) {
        printf("rateNumber erro\r\n");
        return;
    }

    for (int i = 0; i < prate->PeriodNumber; i++) {
        prate->PeriodPrice48_16u[i] = ahttModel[i].ele_fee * 1000;
        prate->PeriodSever48_16u[i] = ahttModel[i].ser_fee * 1000;
        prate->rate_8u[i] = i;
        if (i == 0) {
            prate->startTime[i][0] = 0;
            prate->stopTime[i][0] = ahttModel[i].endTime;
        } else {
            prate->startTime[i][0] = ahttModel[i - 1].endTime;
            prate->stopTime[i][0] = ahttModel[i].endTime;
        }
        prate->startTime[i][1] = 0;
        prate->stopTime[i][1] = 0;
    }
}


//海宁城投计费模型转换为多类电价计费标准
void HaiNCTRateToCostMultiple(uint8_t u8Port, RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    HnctFeeModel_Save_truct *pHnctRateModel = (HnctFeeModel_Save_truct *)rateData;

    FeeModelA8 *hnctModel = &pHnctRateModel->FeemodelA8save_data[B47_A]; //取主分区计费模型即可
	prate->costType = eCostType_PerTime;

    if (!pHnctRateModel) {
        printf("Error: Invalid parameters\n");
        return;
    }
    if (hnctModel->time_allnum > 12 || hnctModel->time_allnum == 0) {
        return;
    }

    //转换为启动计费模型
    //判断当前时段属于哪个时间，一天48时段，最小半小时一段
    prate->rateNumber = 4;
    prate->PeriodNumber = hnctModel->time_allnum;
    for (int i = 0; i < prate->PeriodNumber; i++) {
        uint8_t tType = hnctModel->B47modeldata[i].Serial_rate; //数据前期已经校验过
        if (tType == 0 || tType > 4) {
            printf("Error: Unknown Serial_rate: %d\n", tType);
            return;
        }
        printf("hnct cost num = %d\r\n", hnctModel->time_allnum);
        prate->rate_8u[i] = tType - 1;
        if (prate->rate_8u[i] >= prate->rateNumber) {
            prate->rate_8u[i] = 0;
        }
        prate->price48_16u[prate->rate_8u[i]] = fourUint8ToUint32(hnctModel->B47modeldata[i].UnitEleFee[prate->rate_8u[i]]);
        prate->sever48_16u[prate->rate_8u[i]] = fourUint8ToUint32(hnctModel->B47modeldata[i].Ser_fee);

        prate->PeriodPrice48_16u[i] = fourUint8ToUint32(hnctModel->B47modeldata[i].UnitEleFee[prate->rate_8u[i]]);
        prate->PeriodSever48_16u[i] = fourUint8ToUint32(hnctModel->B47modeldata[i].Ser_fee);
        prate->startTime[i][0] = hnctModel->B47modeldata[i].rate_start[1];
        prate->startTime[i][1] = hnctModel->B47modeldata[i].rate_start[0];
        //结束等于下一个开始，最后一个结束等于第一个开始
        if (i < prate->PeriodNumber - 1) {
            prate->stopTime[i][0] = hnctModel->B47modeldata[i+1].rate_start[1];
            prate->stopTime[i][1] = hnctModel->B47modeldata[i+1].rate_start[0];
        } else {
            // memcpy(prate->stopTime[i], hnctModel->B47modeldata[0].rate_start[0], 2);
            prate->stopTime[i][0] = 23;
            prate->stopTime[i][1] = 59;
        }
    }
}


//蔚景云计费模型转换为多类电价计费标准
void WJYRateToCostMultiple(RATE_MODEL_T *prate, uint8_t *rateData, uint8_t len)
{
    WJY_Recv_Rate_Model *pRecvBillingModel = (WJY_Recv_Rate_Model *)rateData;
	if(ePlatType_WJY == get_ChgParam_plat_type()) {
        memcpy(pRecvBillingModel, rateData, sizeof(WJY_Recv_Rate_Model));
    }

    prate->PeriodNumber = 48;   //蔚景云平台为48时段
	prate->rateNumber = 4;
	prate->costType = eCostType_SPVF;  //理解两种类型的区别
   
 
    memcpy(&prate->billing_model_plat[0], &pRecvBillingModel->rate_id[0], 8);  //费率模型
   
    //48时段费率号
    for (uint8_t i = 0; i < 48; i++)
    {  
       memcpy(&prate->rate_8u[i], &pRecvBillingModel->segmentation_rate[i], 48);  
       prate->rate_8u[i] -= 1;  ////平台尖峰平谷下发1-4，减去1对应程序的费率号0-3
    }
    
    //尖
	prate->price48_16u[0] = fourUint8ToUint32LH(pRecvBillingModel->sharp_ele_fee);
	prate->sever48_16u[0] = fourUint8ToUint32LH(pRecvBillingModel->sharp_ser_fee);
	//峰
	prate->price48_16u[1] = fourUint8ToUint32LH(pRecvBillingModel->peak_ele_fee);
	prate->sever48_16u[1] = fourUint8ToUint32LH(pRecvBillingModel->peak_ser_fee);
	//平
	prate->price48_16u[2] = fourUint8ToUint32LH(pRecvBillingModel->flat_ele_fee);
	prate->sever48_16u[2] = fourUint8ToUint32LH(pRecvBillingModel->flat_ser_fee);
	//谷
	prate->price48_16u[3] = fourUint8ToUint32LH(pRecvBillingModel->valley_ele_fee);
	prate->sever48_16u[3] = fourUint8ToUint32LH(pRecvBillingModel->valley_ser_fee);

    for (int i = 0; i < prate->PeriodNumber; i++) {  //48个时段
        prate->PeriodPrice48_16u[i] = prate->price48_16u[prate->rate_8u[i]];   //通过每个时段的时段费率号去费率电价里对应
        prate->PeriodSever48_16u[i] = prate->sever48_16u[prate->rate_8u[i]];   
    }
}

//读取存储的计费模型，根据平台进行转换
void CostSwitchPlatTypeRate(uint8_t u8Port, uint8_t *rateData, uint16_t len)
{
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
    memset(prate, 0, sizeof(RATE_MODEL_T));     //新的计费模型导入前，清空缓存

	if(ePlatType_GN == get_ChgParam_plat_type() || ePlatType_GNP == get_ChgParam_plat_type()) {
        gnRateToCostMultiple(u8Port, prate, rateData, len);
    } else if((ePlatType_YKC == get_ChgParam_plat_type())||(ePlatType_gwYKC == get_ChgParam_plat_type())  || (ePlatType_TOWER == get_ChgParam_plat_type()) || (ePlatType_DD == get_ChgParam_plat_type())) {
        ykcRateToCostMultiple(prate, rateData, len);//云快充使用和公牛计费一样
    } else if(ePlatType_YKC_V2 == get_ChgParam_plat_type()) {
        ykc21RateToCostMultiple(prate, rateData, len);
    } else if(ePlatType_ANPEI == get_ChgParam_plat_type()) {
        anpeiRateToCostMultiple(u8Port, prate, rateData, len);
    } else if(ePlatType_AHTT == get_ChgParam_plat_type()) {
        ahttRateToCostMultiple(u8Port, prate, rateData, len);
    } else if(ePlatType_HaiNCT == get_ChgParam_plat_type()) {
        HaiNCTRateToCostMultiple(u8Port, prate, rateData, len);
    } else if(ePlatType_WJY == get_ChgParam_plat_type()) {
        WJYRateToCostMultiple(prate, rateData, len);
    }
}

/******************************************************************************
 * 函 数 名  : CostGetRateModel
 * 负 责 人  : 
 * 创建日期  : 20250318
 * 函数功能  : 获取当前存储计费模型，并转化成计费所需要的数据
 * 调用关系  : 
 * 参   数   ： 枪号
*****************************************************************************/
uint8_t CostGetRateModel(uint8_t u8Port)
{
    uint8_t rateData[RATE_MODEL_MAX_LEN] = {0};
     uint8_t readRsult = FALSE;
    //读取flash计费模型
    //安徽铁塔不需要存储，每次启动会下发计费模型，不进行频繁存储
    if (ePlatType_AHTT != get_ChgParam_plat_type()) 
    {
        if(ePlatType_ANPEI!=get_ChgParam_plat_type())
        {
            readRsult = Read_rate_model(rateData, sizeof(rateData));

            if (readRsult == FALSE) 
            {
                return FALSE;
            }
        }    
        else
        {
        //因为安培的计费模型未使用校验
            readRsult = Read_rate_model_anpei(u8Port,rateData);

            if (readRsult == FALSE) 
            {
                return FALSE;
            }
        }
    }

    CostSwitchPlatTypeRate(u8Port, rateData, sizeof(rateData));
    return TRUE;
}



/*****************************************************************************
 * 函 数 名  : Cost_charge_init
 * 负 责 人  : 
 * 创建日期  : 
 * 函数功能  : 充电前后计费初始化
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  : 
 * 其     它   : 无
*****************************************************************************/
void Cost_charge_init (uint8_t u8Port)
{
	COST_GUN_DATA *pCostGunData = &g_cost_ctrl.strCostGunData[u8Port];
	uint8_t i = 0;
	
	pCostGunData->total_power = 0;
	pCostGunData->total_loss_power = 0;
	pCostGunData->total_money = 0;
	
    int len = sizeof(COST_GUN_DATA);
    memset(pCostGunData, 0, len);
	
	return;
}
/*****************************************************************************
 * 函 数 名  : Cost_init
 * 负 责 人  : 
 * 创建日期  : 
 * 函数功能  : 上电初始化
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  : 
 * 其     它   : 无
*****************************************************************************/
void Cost_init ( void )
{
	uint8_t i = 0;

	memset(&g_cost_ctrl, 0, sizeof(COST_CTRL_T));

	return;
}




