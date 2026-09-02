#include "iot_WJY_Protocol_Code.h"
#include "iot_GN_Protocol_Code.h"
#include "protocol_data.h"
#include "mbsMaster.h"
#include "maths.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "cost.h"

static WJY_UpPlatInfo s_WjyUpInfo;
static WJY_Send_ErrSend WjyErrSend[GUN_NUM_MAX];

#define FOUR_UINT8_TO_UINT32(pu8) \
    (((uint32_t)(pu8)[0] << 24) | \
     ((uint32_t)(pu8)[1] << 16) | \
     ((uint32_t)(pu8)[2] << 8)  | \
     (uint32_t)(pu8)[3])

void Bcd_to_Cp56time2a_wjy(uint8_t *pTime, uint8_t *pCp56)
{
	pTime[0] = U8BcdToBin(pCp56[0]);  	//Year
	pTime[1] = U8BcdToBin(pCp56[1]);  	//Month
	pTime[2] = U8BcdToBin(pCp56[2]);  	//Day
	pTime[3] = U8BcdToBin(pCp56[3]);  	//Hour
	pTime[4] = U8BcdToBin(pCp56[4]);  	//Minute
	pTime[5] = U8BcdToBin(pCp56[5]);  	//Second
	return;
}

void Time_to_YYMMDDHHMMSS(uint8_t *pTime)
{
	tm_struct strCurTime = get_current_time();
		
	pTime[0] = strCurTime.yearL;
	pTime[1] = strCurTime.month;
	pTime[2] = strCurTime.day;
	pTime[3] = strCurTime.hour;
	pTime[4] = strCurTime.minute;
	pTime[5] = strCurTime.second;
	return;
}

uint8_t monitor_getDevNumber_wjy(uint8_t *pNum, uint8_t len)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
	
	memcpy(pNum, pst_cfgInfo->pltDeviceNumber, len);
	
	return TRUE;
}
//获取桩号长度
uint8_t monitor_getDevNumberlength_wjy(void)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
	
	uint8_t length=strlen(pst_cfgInfo->pltDeviceNumber);

	if(length>0&&length<=PLAT_NUMBER_LEN)
	return length;
	else
	 return 0;

	
	
}
void stringToAscii(const uint8_t *str, int *asciiArray) {
    if (str == NULL || asciiArray == NULL) return;
    for (int i = 0; str[i] != '\0'; i++) {
        asciiArray[i] = (int)str[i]; // 将字符转换为ASCII码
    }
}

void stampToCharArray(uint8_t *dest, uint32_t stamp) {
	if (dest == NULL) return;
    snprintf((char *)dest, 11, "%010lu", (unsigned long)stamp); // 10位，不足补0
}


//将8位bcd码转换成16位ascii码
void CardNum8ToAscii16(uint8_t *input, uint8_t *output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[2*i] = (input[i] / 16) + 48;    // 十位数存入偶数索引 ascii码
        output[2*i + 1] = (input[i] % 16) + 48; // 个位数存入奇数索引
    }
}


/*****************桩端生成订单号 **********************************************************************/
/********************************************************************
 * 设备生成16位交易流水号
 * 时间戳（10 位） + 桩号（4位）序列号（2位）
 * 序列号产生规则：0-99递增
 *********************************************************************/	
void WJY_TransLogNumber_Update(uint8_t u8Port, uint8_t *ChargeNumber, uint8_t *logNumber)
{
	if(u8Port >= GUN_NUM_MAX || logNumber == NULL) {
        printf("Invalid parameters!\n");
        return;
    }	

    static uint16_t tLogNumber[GUN_NUM_MAX] = {0};
    uint32_t cur_stamp = getRunTimeS();
    uint8_t stamp_str[10+1] = {0};  // 存储时间戳的字符串（10位字符 + '\0'）
    uint8_t number[6] = {0};  //后6位
	uint8_t dev_num[PLAT_NUMBER_LEN+1] = {0};  //设备编号

      uint8_t length=monitor_getDevNumberlength_wjy();

	stampToCharArray(stamp_str,cur_stamp); //时间戳转字符串 为流水号前10位
	monitor_getDevNumber_wjy(&dev_num[0], length);		//设备编号（桩号）

	memcpy(&number[0], &dev_num[8], 4);  //流水号的11-14位是桩号第9-12位
	
    
	// tLogNumber[u8Port]++;  //序列号自增

	// if (tLogNumber[u8Port] >= 99) tLogNumber[u8Port] = 0;  //序列号超过99归零

	// uint8_t tens = tLogNumber[u8Port] / 10;  // 十位
	// uint8_t units = tLogNumber[u8Port] % 10; // 个位
	number[4] =  '0';
	number[5] = u8Port + '0';

    memcpy(&logNumber[0], &stamp_str[0], 10);  //前10位是时间戳
    memcpy(&logNumber[10],&number[0], 6);  //后6位是桩号后4位加序列号2位
	
    printf("\r\n WJY_TransLogNumber_Update: ");  //打印流水号
	for (uint8_t i = 0; i < 16; i++)
	{
		printf("%02x ", logNumber[i]);
	}
}


//桩停止原因对应蔚景云平台需要上报的停止原因
const Pile_WjyStopReasonMap StrWjyStopReasonMap[] = {
	{Pile_Stop_Reason_CarOk	            ,Wjy_Reason_Finish_Soc              },  //充满停止
	{Pile_Stop_Reason_OverSum	        ,Wjy_Reason_Finish_Sum              },  //金额截止
	{Pile_Stop_Reason_OverTime	        ,Wjy_Reason_Finish_Time             },  //时间截止
	{Pile_Stop_Reason_OverEle	        ,Wjy_Reason_Finish_Ele              },  //电量截止
	{Pile_Stop_Reason_OverBalance	    ,Wjy_Reason_Stop_SumNoEnough     	},  //余额截止
	{Pile_Stop_Reason_APP	            ,Wjy_Reason_Finish_App             	},  //APP停止     
	{Pile_Stop_Reason_Card	            ,Wjy_Reason_Finish_Manual         	},  //刷卡停止
	{Pile_Stop_Reason_GunBreak	        ,Wjy_Reason_Stop_GunBreak          	},  //拔枪停止
	{Pile_Stop_Reason_EStop		        ,Wjy_Reason_Stop_EmergencyStop    	},  //急停
	{Pile_Stop_Reason_Leak		        ,Wjy_Reason_StopLeak              	},  //设备故障，漏电故障
	{Pile_Stop_Reason_Comm		        ,Wjy_Reason_StopComm                },  //网络故障，充电单元通信故障

	{Pile_Stop_Reason_GunTempOver	    ,Wjy_Reason_Stop_GunTmpErr          },	//枪温过高
	{Pile_Stop_Reason_PlugTempOver	    ,Wjy_Reason_Stop_PlugTmpErr         },  //输出连接器过温故障
	{Pile_Stop_Reason_AirTempOver	    ,Wjy_Reason_Stop_TmpErr           	},  //输出连接器过温故障
	{Pile_Stop_Reason_CPErro		    ,Wjy_Reason_StopCPErro            	},  //检测点2电压检测故障

	{Pile_Stop_Reason_CrtUnder	        ,Wjy_Reason_Stop_ErrCurr          	},  //过流
	{Pile_Stop_Reason_VolOver	        ,Wjy_Reason_Stop_ErrVal             },  //电压异常 过压
	{Pile_Stop_Reason_VolUnder	        ,Wjy_Reason_Stop_ErrVal           	},  //电压异常 欠压
	{Pile_Stop_Reason_EleCommFault	    ,Wjy_Reason_Stop_MeterErr           },  //电能表通信异常
	{Pile_Stop_Reason_Ele	            ,Wjy_Reason_StopEleErro            	},  //电量上传超时

	{Pile_Stop_Reason_Money	            ,Wjy_Reason_StopMoneyErro          	},	//余额不足
	{Pile_Stop_Reason_ShortCircle	    ,Wjy_Reason_Stop_BreakErr           },	//短路检测故障
	{Pile_Stop_Reason_RlyRfs	        ,Wjy_Reason_StopRelayMissTrip     	},  //拒动
	{Pile_Stop_Reason_RlySyn	        ,Wjy_Reason_StopRelayCgltnt       	},  //粘连

	// //以下是自定义原因
	{Pile_Stop_Reason_CPGnd		    	,Wjy_Reason_StopCPGnd             	},  //CP接地  72
	{Pile_Stop_Reason_PEGnd	        	,Wjy_Reason_StoPEGnd              	},  //PE接地故障  73
	{Pile_Stop_Reason_StartDiode	    ,Wjy_Reason_StartDiode              },  //二极管检测无，新国标  74
	{Pile_Stop_Reason_S2TimeOut	    	,Wjy_Reason_StartTimeout           	},  //S2闭合超时，车辆拒绝  75
	{Pile_Stop_Reason_PwOff	        	,Wjy_Reason_Interupt_PwOff         	},  //充电异常中止，充电设备断电 76
	{Pile_Stop_Reason_StopKey	    	,Wjy_Reason_StopSetKey             	},  //按键停止  77
	{Pile_Stop_Reason_MaxTime	    	,Wjy_Reason_Stop_TimeOut           	},
    
	//无法判断故障
	{Pile_Stop_Reason_None	            ,Wjy_Reason_UnKnow                 	},
	{Pile_Stop_Reason_Other		        ,Wjy_Reason_UnKnow                 	},
};

//上传订单时更新停止原因,获取桩的停止原因
uint16_t WjyUpdate_stopReason(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t stopRs = pChgGunData->DealRecord.PileStopReason;
	uint16_t wjyStopRs = 0;

    const Pile_WjyStopReasonMap *pPileStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrWjyStopReasonMap); u32i++) {
        pPileStopMap = &StrWjyStopReasonMap[u32i];
        if (stopRs == pPileStopMap->PileStopReason) {
            printf("Pile_stop reason: %d, wjy_stop_reason: %d\r\n", pPileStopMap->PileStopReason, pPileStopMap->WjyStopReason);
            wjyStopRs = pPileStopMap->WjyStopReason;
            return wjyStopRs;
        }
    }

    printf("Else OgrReason: %d\r\n", stopRs);
    wjyStopRs = stopRs;  
    return wjyStopRs;
}

//蔚景云订单数据更新
void wjy_packChgRecord(uint8_t u8Port, WJY_charge_record *pRecord)
{
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;  //读当前存储计费模型 
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
	pRecord->gun_num = u8Port+1;
	
	memcpy(pRecord->transaction_log_num, pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);	 //交易流水号
	

	if (1 == monitor_getTradeFlag(u8Port))
		pRecord->trade_flag = 2;  //app启动
	else 
		pRecord->trade_flag = 1;  //卡启动	

	memset(pRecord->card_number, 0, GNDATA_CARD_LEN);
	    uint8_t card_num[16] = {0};  //上传卡号
	
	CardNum8ToAscii16(&pChgGunData->LogicCard_number[0], &pRecord->card_number[0], 8);  //wjy card

	memset(pRecord->vin, 0, 17);
	pRecord->start_soc = 0;
	pRecord->stop_soc = 0;

    pRecord->stop_reason = WjyUpdate_stopReason(u8Port);      

	WJY_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvTimeSyn;
		

	BINToBCD(&pRecord->chrg_start_time[0],&pChgGunData->chrg_start_time[1],6);
	BINToBCD(&pRecord->chrg_stop_time[0],&pChgGunData->chrg_stop_time[1],6);

	pRecord->data_bit = 4;      //小数点后4位

	uint32ToFourUint8LH(pRecord->total_start_elec, pChgGunData->total_start_elec);
	uint32ToFourUint8LH(pRecord->total_stop_elec, pChgGunData->total_stop_elec);
	uint32ToFourUint8LH(pRecord->total_power, pcostdata->total_power);  //小数点后4位
	uint32ToFourUint8LH(pRecord->sharp_power, pcostdata->ele_power[0]);	
	uint32ToFourUint8LH(pRecord->peak_power, pcostdata->ele_power[1]);	
	uint32ToFourUint8LH(pRecord->flat_power, pcostdata->ele_power[2]);	
	uint32ToFourUint8LH(pRecord->valley_power, pcostdata->ele_power[3]);
	

	uint32ToFourUint8LH(pRecord->chg_money, pcostdata->allEleMoney);
	uint32ToFourUint8LH(pRecord->serve_money, pcostdata->allServerMoney);
	memset(pRecord->order_money, 0, 4);
	memset(pRecord->park_money, 0, 4);
	
	printf("\r\n wjy_packChgRecord = ");
	for (int i = 0; i < 48; i++)	//时段恒为48
	{
		uint32ToFourUint8LH(pRecord->time_power[i], pcostdata->PeriodElePower[i]);
		// printf("\r\n time_power[%d] = %02x %02x %02x %02x", i, pRecord->time_power[i][0],pRecord->time_power[i][1],pRecord->time_power[i][2],pRecord->time_power[i][3]);
	}
	printf("\r\n");
	
	memcpy(pRecord->rate_id, &prate->billing_model_plat[0], 8);  //wdy 用起充时的费率id和版本号
	

	return;
}

//离线上报 蔚景云
static uint8_t WJYUpChargeRecordUpDealOffline()
{
	for (uint8_t i = 0; i < GUN_NUM; i++ ) {
		WJY_charge_record *pRecord = &g_chgData[i].DealRecord.ChgRecord.WjyChgRecord;		
		uint8_t uGun = i;

		uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }
        
        uint8_t cmd = WJY_S_Chg_Record;

		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("\r\n WJYUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}
        
        pRecord->stop_reason = WjyUpdate_stopReason(uGun);
		printf("\r\n WJYUpChargeRecordUpDealOffline gun = %d stop_reason = %d\r\n", uGun, pRecord->stop_reason);
		
		SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
		Send_Immediately(uGun, cmd);
	}

    return TRUE;
}


uint16_t send_login_data_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	uint16_t power = 700;	//默认功率
	uint8_t u8SoftVer = 0;
	uint8_t u8HardVer = 0;
	char PlatVer[4] = "2.2";	//通信协议版本号
	// WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;

    uint8_t rateData[128] = {0};  //根据计费模型大小调整
    WJY_Recv_Rate_Model *pRecvRateModel = (WJY_Recv_Rate_Model *)rateData;


    data[data_len] = 1;
	data_len ++;	//通信协议版本号

	data_len += 2;
	Uint16ToTwoUint8LH(&data[data_len],power);
    data_len += 2;

    data[data_len] = fgv_GetPileCfgGunNum();  //充电枪数量 如果不回02则改成1
    data_len ++;

	// Read_rate_model(rateData, sizeof(rateData));  //读上一个存的计费模型	
	memcpy(&data[data_len], &pRecvRateModel->rate_id, 4);
	data_len += 4;	//计费规则ID 首次登录填0，后续登录填桩当前正在使用的计费规则ID

	memcpy(&data[data_len], &pRecvRateModel->rate_ver, 4);
	data_len += 4;	//计费规则版本号 首次登录填0，后续登录填桩当前正在使用的计费规则版本号

	data_len += 4;	

	data_len += 3;	

	data_len += 8;	//设备软件版本号
	u8SoftVer = strlen(SOFTWARE_VERSION) > 8 ? 8 : strlen(SOFTWARE_VERSION);
	memcpy(&data[data_len], SOFTWARE_VERSION, u8SoftVer);
	data_len += u8SoftVer;
	if (u8SoftVer == 7)	 //不足8位补0  1.3.2.1->1.3.2.01
	{
		uint8_t temp = 0;
		temp = data[data_len-1];
		data[data_len - 1] = '0';
		data[data_len] = temp;
		data_len ++;
	}

	data_len += 8;	//设备硬件版本号
	u8HardVer = strlen(HARDWARE_VERSION) > 8 ? 8 : strlen(HARDWARE_VERSION);
	memcpy(&data[data_len], HARDWARE_VERSION, u8HardVer);
	data_len += u8HardVer;
	if (u8SoftVer == 7)	 //不足8位补0   1.3.2.1->1.3.2.01
	{
		uint8_t temp = 0;
		temp = data[data_len-1];
		data[data_len - 1] = '0';
		data[data_len] = temp;
		data_len ++;
	}

	data_len += 5;	//通信协议版本号	
	memcpy(&data[data_len], &PlatVer, 3);
    data_len += 3; 	


    return data_len;
}

void send_login_wjy_Succ(uint8_t u8Port)
{
	return;
}



uint16_t send_auth_data_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	data[data_len] = u8Port + 1;
	data_len ++;

	data[data_len] = 1;  //充电方式，1刷卡充电
	data_len ++;
	
	data[data_len] = 1;  //充电模式，1自动充满
	data_len ++;

	data_len += 4;		//充电数据，对应充电模式 直到充满，填0

	data_len += 16;
    uint8_t card_num[16] = {0};  //上传卡号
	
	CardNum8ToAscii16(&pChgGunData->LogicCard_number[0], &card_num[0], 8);
	
	memcpy(&data[data_len], card_num, 16);
	data_len += 16;

	data_len += 32;

	uint8_t dev_num[9] = {0};
    uint8_t transLogNum[16] = {0};
	monitor_getDevNumber_wjy(&dev_num[0], 7);		//设备编号（桩号）	
	dev_num[7] = u8Port;

	WJY_TransLogNumber_Update(u8Port,&dev_num[0],transLogNum);
	
	memcpy(&data[data_len],transLogNum,16);
	data_len += 16;  //订单号

	data_len ++ ;	//不校验密码

    return data_len;
}

void send_auth_wjy_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, WJY_R_Auth))
	{
		SetRecvEnable(u8Port, WJY_R_Auth, RECV_ENABLE_ON);
		SetRecvTick(u8Port, WJY_R_Auth, Get_Systick());
	}
	return;
}


uint16_t send_start_ack_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	U8 chg_rst = pUpGunData->up_start_fail_reason;

	memcpy(&data[data_len], &pChgGunData->transaction_log_num, 16);
	data_len += 16;

	switch (chg_rst)  //充电失败原因转换
	{
		case eUP_Start_Fail_NULL:  //启动成功
			data[data_len] = eUP_Start_Fail_NULL;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_DevNumErr:  //设备号错误
			data[data_len] = WJY_Start_Fail_DevNumErr;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_Working:  //设备正在充电
			data[data_len] = WJY_Start_Fail_Working;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_DevErr:  //设备故障
			data[data_len] = WJY_Start_Fail_DevErr;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_Offline:  	//设备离线
			data[data_len] = WJY_Start_Fail_Offline;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_NoConn:  //未插枪
			data[data_len] = WJY_Start_Fail_NoConn;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_Reconnect:  //需重新插枪
			data[data_len] = WJY_Start_Fail_GunErr;
			data_len ++ ;	
			break;
		case eUP_Start_Fail_Rate:  //充电费率错误
			data[data_len] = WJY_Start_Fail_ErrRate;
			data_len ++ ;	
			break;
		default:
			break;
	}
	
	printf("\r\n send_start_ack_wjy,gun: %d,SendSumFlag = %d,SendSumTime = %d\n",u8Port, pWjyUpInfo->SendSumFlag[u8Port],pWjyUpInfo->SendSumTime[u8Port] = 0);//wdy测试

    return data_len;
}

void send_start_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_Start_Chg_Ack, SEND_ENABLE_OFF);

	return;
}

uint16_t send_stop_ack_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	memcpy(&data[data_len], &pChgGunData->transaction_log_num, 16);
	data_len += 16;
	
	data[data_len] = pUpGunData->up_stop_ret;
	data_len++;

    return data_len;
}

void send_stop_wjy_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, WJY_S_Stop_Chg_Ack, SEND_ENABLE_OFF);
	return;
}


uint16_t send_charge_record_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;
	WJY_Recv_Auth_Ack *pRecvAuth = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvAuthAck;
	WJY_charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.WjyChgRecord;

	data[data_len] = u8Port + 1;
	data_len ++;
	
	memcpy(&data[data_len], &pRecord->transaction_log_num, 16);
	
	printf("\r\n [WJY]send_charge_record_wjy,transaction_log_num:");
	for (uint8_t i = 0; i < 16; i++)
	{
		printf("%02x ", pRecord->transaction_log_num[i]);
	}	

	data_len += 16;
	
	data[data_len] = pRecord->trade_flag;  //充电方式
	data_len ++;
	
	if (1 == pRecord->trade_flag)  //卡启动上传卡号
	{
		memcpy(&data[data_len], pRecord->card_number, 16);
		data_len += 16;		
	}
	else
	{
		data_len += 16;
	}
		
	data_len += 17;  //vin码
	data_len ++;
	data_len ++;

	data[data_len] = pRecord->stop_reason;
	data_len ++;

	memcpy(&data[data_len], &pRecord->chrg_start_time[0], 6);
	printf("\r\n send_charge_record_wjy, start Time: %02x-%02x-%02x %02x:%02x:%02x\n", 
			data[data_len], data[data_len+1],data[data_len+2], data[data_len+3], data[data_len+4], data[data_len+5]);
	data_len += 6;  //开始时间 YYMMDDHHMMSS

	memcpy(&data[data_len], &pRecord->chrg_stop_time[0], 6);
	printf("\r\n send_charge_record_wjy, start Time: %02x-%02x-%02x %02x:%02x:%02x\n", 
			data[data_len], data[data_len+1], data[data_len+2], data[data_len+3], data[data_len+4], data[data_len+5]);
	data_len += 6;  //结束时间 YYMMDDHHMMSS

	data[data_len] = 4;  //数据精度位 如0x04：表示精度为4位小数	
	data_len ++;  

	memcpy(&data[data_len], &pRecord->total_start_elec[0], 4);  
	data_len += 4;

	memcpy(&data[data_len], &pRecord->total_stop_elec[0], 4);  
	data_len += 4;

	memcpy(&data[data_len], &pRecord->total_power[0], 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->sharp_power, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->peak_power, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->flat_power, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->valley_power, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->chg_money, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->serve_money, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->order_money, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->park_money, 4);
	data_len += 4;

	for (uint8_t i = 0; i < 48; i++)  //48时段电量
	{
		memcpy(&data[data_len], &pRecord->time_power[i][0], 4);
		printf("\r\n send_charge_record_wjy, time_power[%d]: %02x %02x %02x %02x", i, data[data_len], data[data_len+1], data[data_len+2], data[data_len+3]);
		data_len += 4;
	}

	memcpy(&data[data_len], &pRecord->rate_id, 4);
	data_len += 4;

	memcpy(&data[data_len], &pRecord->rate_ver, 4);
	data_len += 4;

    return data_len;
}

void send_charge_record_wjy_Succ(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;

	if (pChgGunData->upDealCnt >= 5) {
		//上报五次未回复，停止上报
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, WJY_S_Chg_Record))
		{
			SetSendEnable(u8Port, WJY_S_Chg_Record, SEND_ENABLE_OFF);
		}
	} else if (pChgGunData->upDealCnt == 1) {
        pChgGunData->ExistChargeDeal = 0;
    }

	return;
}

uint16_t send_real_data_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	// WJY_charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.WjyChgRecord;
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];

	data[data_len] = u8Port+1;  //枪号
	data_len ++;

	memcpy(&data[data_len], &pChgGunData->transaction_log_num, 16);  //订单号
	data_len += 16;

	if (1 == pChgGunData->trade_flag)
	{
		data[data_len] = 2;  //app启动
		data_len ++;
		data_len += 16;		//卡号0
	}
	else 
	{
		data[data_len] = 1;  //卡启动
		data_len ++;
    	uint8_t card_num[16] = {0};

		CardNum8ToAscii16(&pChgGunData->LogicCard_number[0], &card_num[0], 8);  //卡号转为ascii码上传
		memcpy(&data[data_len], card_num, 16);
		data_len += 16;
	}

	data_len += 17;		
	data_len ++;

	// memcpy(&data[data_len], &pChgGunData->chrg_start_time[1], 6);
	BINToBCD(&data[data_len], &pChgGunData->chrg_start_time[1], 6);
	printf("\r\n real_data_wjy, start Time: %x-%x-%x %x:%x:%x\n", 
			data[data_len], data[data_len+1], data[data_len+2], data[data_len+3], data[data_len+4], data[data_len+5]);
	data_len += 6;  //开始时间 YYMMDDHHMMSS

	// memcpy(&data[data_len], &pChgGunData->chrg_stop_time[1], 6);
	BINToBCD(&data[data_len], &pChgGunData->chrg_stop_time[1], 6);
	printf("\r\n real_data_wjy, stop Time: %x-%x-%x %x:%x:%x\n", 
			data[data_len], data[data_len+1], data[data_len+2], data[data_len+3], data[data_len+4], data[data_len+5]);
	data_len += 6;  //结束时间 YYMMDDHHMMSS

	data[data_len] = 4;  //数据精度位 如0x04：表示精度为4位小数	
	data_len ++;  

	//实时数据中的总电量和尖峰平谷变量用strCostGunData中的数据，否则订单中的尖峰平谷电量加起来不等于实时充电电量
	uint32ToFourUint8LH(&data[data_len], pcostdata->total_power);  //总电量
	printf("\r\n wjy,total_power: ");
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("%02x ", data[data_len + i]);
	}
	data_len += 4;
		
	//尖峰平谷电量
	uint32ToFourUint8LH(&data[data_len], pcostdata->ele_power[0]);
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], pcostdata->ele_power[1]);
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], pcostdata->ele_power[2]);
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], pcostdata->ele_power[3]);
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], pcostdata->allEleMoney);  //总电费
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], pcostdata->allServerMoney);  //总服务费
	data_len += 4;

	// memcpy(&data[data_len], &pRecord->order_money, 4);
	data_len += 4;

	// memcpy(&data[data_len], &pRecord->park_money, 4);
	data_len += 4;

	data_len += 4;
	
	uint32ToFourUint8LH(&data[data_len], (monitor_getGunTem(u8Port)-50)*10000);  //枪头温度 4位小数
	printf("\r\n wjy,gun_temp:%d ",(monitor_getGunTem(u8Port)-50)*10000);
	data_len += 4;

	data_len += 16;  //充电机相关

	data_len += 4;
	// data[data_len] = 220;  //电压需求

	data_len += 4;
	// data[data_len] = 32;  //电流需求

	uint32ToFourUint8LH(&data[data_len], GetPile_ChgOutVol(u8Port, 1)*1000);  //A相电压 4位小数
	printf("\r\n wjy,OutVol:%d ",GetPile_ChgOutVol(u8Port, 1)*1000);
	data_len += 4;

	data_len += 4;
	data_len += 4;

	uint32ToFourUint8LH(&data[data_len], GetPile_ChgOutCur(u8Port, 2)*100);  //A相电流 4位小数
	printf("\r\n wjy,current:%d ",GetPile_ChgOutCur(u8Port, 2)*100);
	data_len += 4;

	data_len += 4;
	data_len += 4;

    return data_len;
}

void send_real_wjy_Succ(uint8_t u8Port)
{
	return;
}


uint16_t send_Rate_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;

	memcpy(&data[data_len],pRecvRateModel->rate_id,sizeof(pRecvRateModel->rate_id));
	data_len += 4;

	memcpy(&data[data_len],pRecvRateModel->rate_ver,sizeof(pRecvRateModel->rate_ver));
	data_len += 4; 
	
	data[data_len] = 0;  //设置成功
	data_len ++;
	
    return data_len;
}

void send_Rate_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_Rate_Ask, SEND_ENABLE_OFF);
	return;
}


uint16_t send_heart_data_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;

	// uint8_t gun_num = GetPile_CfgGunNum();

	data[data_len] = GUN_NUM;
	data_len ++;

	for (uint8_t i = 0; i < GUN_NUM; i++)
	{
		up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[i];
		data[data_len] = pUpGunData->up_gun_state;
		printf("\r\n send_heart_data_wjy,gun:%d, up_gun_state= %d",i,pUpGunData->up_gun_state);
		data_len ++;
	}
	
    return data_len;
}

void send_heart_wjy_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, WJY_R_Heart))
	{
		SetRecvEnable(u8Port, WJY_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, WJY_R_Heart, Get_Systick());
	}
	return;
}


void uint16_to_bcd(uint16_t num, uint8_t *bcd) {
    // 提取每一位
    uint8_t thousands = (num / 1000) % 10;  // 千位
    uint8_t hundreds = (num / 100) % 10;    // 百位
    uint8_t tens = (num / 10) % 10;         // 十位
    uint8_t ones = num % 10;                // 个位

    // 组合为压缩BCD码（每个字节存储两位十进制数）
    bcd[0] = (thousands << 4) | hundreds;  // 高字节：千位和百位
    bcd[1] = (tens << 4) | ones;           // 低字节：十位和个位
}

typedef struct {
    E_ErrCode err_type;
    uint16_t err_code;
} err_map_wjy;


static const err_map_wjy error_map[] = {
	{eErr_ReaderCommErr,     	1002},	//读卡器通信故障
    {eErr_MeterCalcErr,   	 	1004}, 	//电能表计量故障
    {eErr_MeterCommErr,   	 	1006},  //电表通信故障    
    {eErr_EnvOverTempErr,    	1011},	//环境过温
    {eErr_DiodeStop,      	 	1012},	//不存在二极管
    {eErr_JcqMaloperation,   	1022},  //拒动
 
    {eErr_EmergencyStop,     	2003},	//急停
    {eErr_ShortCircleErro,   	2005},  //短路故障
    {eErr_LeakageCurrErr,    	2006},	//漏电故障
    {eErr_POverTempErr,      	2007},	//插头过温

    {eErr_InputOverVol,      	3104},	//输入过压
    {eErr_InputLessVol,      	3105},	//输入欠压
	{eErr_PhaseLossErr,      	3174}, 	//缺相故障

    {eErr_NetNoSIMErr,       	5002},  //无SIM卡
    {eErr_PEBreakFault,      	5003},  //pe接地
    {eErr_OutputOverCurr,    	5005},	//输出过流
	{eErr_JcqSynechiaFault,  	5006},	//粘连	
    {eErr_InputLineReversed, 	5009},	//火零反接
	{eErr_CpVoltAbnor,       	5011},  //cp电压异常

    {eErr_NetIPErr,          	6005},  //ip连接异常
    {eErr_CCUSCUCommErr,     	6013},	//充电单元通信故障
    {eErr_CpGroundFault,     	6020},  //cp接地故障
    {eErr_PlatformOffline,   	6021},	//平台通信异常
    {eErr_NetSIMErr,         	6025},  //SIM卡异常 
};

//获取故障编码
uint8_t get_hard_err_code_wjy(uint8_t u8Port)
{
	WJY_Send_ErrSend *wjyErrorSwap = &WjyErrSend[u8Port];
    for (uint16_t i = 0; i < sizeof(error_map)/sizeof(error_map[0]); ++i) {
        if (dev_getErrExsit(u8Port, error_map[i].err_type)) {
			uint16_to_bcd(error_map[i].err_code,wjyErrorSwap->WarnPlatId);
			printf("r\n get_hard_err_code_wjy,gun:%d id:%02x %02x\n", u8Port,wjyErrorSwap->WarnPlatId[0],wjyErrorSwap->WarnPlatId[1]);
            return TRUE;
        }
    }
	return NULL;
}



uint16_t send_Device_Fault_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	WJY_Send_ErrSend *wjyErrorSwap = &WjyErrSend[u8Port];

	data[data_len] = wjyErrorSwap->gun;
	data_len ++;

	memcpy(&data[data_len],wjyErrorSwap->WarnPlatId,2);
	data_len += 2;

	if (1 == wjyErrorSwap->status)  //告警上报发故障发送时间
	{
		memcpy(&data[data_len],wjyErrorSwap->StartTime,6);
		data_len += 6;
	}
	else	//告警恢复上报发故障恢复时间
	{
		memcpy(&data[data_len],wjyErrorSwap->StopTime,6);
		data_len += 6;
	}

	data[data_len] = wjyErrorSwap->status;
	data_len ++;

	data_len += 4;

    return data_len;
}

void send_Device_Fault_wjy_Succ(uint8_t u8Port)
{
	// SetSendEnable(u8Port, WJY_S_Device_Fault, SEND_ENABLE_OFF);
	return;
}


uint16_t send_TimeSyn_ACK_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;

	data[data_len] = 0;
	data_len ++;

    return data_len;
}

void send_TimeSyn_ACK_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_TimeSyn_ACK, SEND_ENABLE_OFF);
	return;
}


uint16_t send_QR_ACK_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;

	data[data_len] = u8Port + 1;
	data_len ++;

    return data_len;
}

void send_QR_ACK_wjy_Succ(uint8_t u8Port)
{
	return;
}


uint16_t send_update_ACK_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;

	data[data_len] = pWjyUpInfo ->upResult;
	data_len ++;

	data[data_len] = 3;  //ftp方式下载
	data_len ++;

	data_len += 2;			

    return data_len;
}

void send_update_ACK_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_update_ACK, SEND_ENABLE_OFF);

	return;
}

uint16_t send_update_rst_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	u8Port = GUN_A;
    WJY_Recv_Update_ftp *pRecvUpdata = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvUpdata;
	WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;

	data[data_len] = pWjyUpInfo ->upResult;
	data_len ++;

	memcpy(&data[data_len],pRecvUpdata->soft_ver,16);
	data_len += 16;			

	memcpy(&data[data_len],pRecvUpdata->plat_ver,16);
	data_len += 8;		

    return data_len;
}

void send_update_rst_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_update_rst, SEND_ENABLE_OFF);

	return;
}


uint16_t send_reboot_ACK_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	WJY_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvReboot;
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;

	data[data_len] = pRecvReboot->ctrl_type;
	data_len ++;

	data[data_len] = pWjyUpInfo->Wjy_Reboot_result;  //执行结果
	printf("\r\n send_reboot_ACK_wjy,Reboot_result:%d\n", pWjyUpInfo->Wjy_Reboot_result);
	data_len ++;

    return data_len;
}

void send_reboot_ACK_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_reboot_ACK, SEND_ENABLE_OFF);
	return;
}


uint16_t send_Sum_ACK_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	memcpy(&data[data_len],pChgGunData->transaction_log_num,16);
	data_len += 16;

	if (eUP_Start_Style_CardOnline == pChgGunData->trade_flag)  //卡启动上报卡号
	{
		data[data_len] = 1;
		data_len ++;

		uint8_t card_num[16] = {0};
		CardNum8ToAscii16(&pChgGunData->Auth_card_number[0], &card_num[0], 8);
		memcpy(&data[data_len],card_num,8);
		data_len += 8;
	}
	else		//app启动置0
	{
		data[data_len] = 2;
		data_len ++;

		memset(&data[data_len],0,8);
		data_len += 8;
	}
	data_len += 24;
	
    return data_len;
}

void send_Sum_ACK_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_Sum, SEND_ENABLE_OFF);
	
	return;
}



uint16_t send_Aes_Key_wjy(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度

    return data_len;
}

void send_Aes_Key_wjy_Succ(uint8_t u8Port)
{
	printf("send_Aes_Key_wjy_Succ\n");
	return;
}








static uint8_t UpCtrlSendCyc(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	uint32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	if(TRUE == u8SendImmdFlag)
		return TRUE;
	
	// if(YKC_S_RealData == cmd)
	// {
	// 	if(eUP_Gun_State_Work == pUpGunData->up_gun_state)
	// 		Cyc = eTick_15S;
	// }
	
	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

typedef uint8_t (*PSendCyc)(uint8_t u8Port, uint32_t cmd, uint32_t	Cyc);
typedef uint16_t (*PSend)(uint8_t u8Port, void *pBuf ,uint16_t u32BufSize);
typedef void (*PSendSucc)(uint8_t u8Port);

typedef struct
{
    uint32_t		cmd;
	uint8_t			FType;
    uint32_t     	cyc;
    PSendCyc 		pSendCyc;
    PSend 			pSend;
    PSendSucc 		pSendSucc;
}WJY_Send_ctrl;

const WJY_Send_ctrl StrWJYSendCtrl[]={
    {WJY_S_Identification	,UP_S_FRAME_SELF	,eTick_60S,		UpCtrlSendCyc	,send_login_data_wjy		,send_login_wjy_Succ},		
	{WJY_S_Auth 			,UP_S_FRAME_SELF	,eTick_60S,		UpCtrlSendCyc 	,send_auth_data_wjy			,send_auth_wjy_Succ},	
	
	{WJY_S_Start_Chg_Ack	,UP_S_FRAME_ACK	,   eTick_15S,		UpCtrlSendCyc	,send_start_ack_wjy			,send_start_wjy_Succ},		
	{WJY_S_Stop_Chg_Ack		,UP_S_FRAME_ACK	,   eTick_30S,		UpCtrlSendCyc 	,send_stop_ack_wjy			,send_stop_wjy_Succ},

	{WJY_S_Chg_Record		,UP_S_FRAME_SELF 	,eTick_30S,	    UpCtrlSendCyc 	,send_charge_record_wjy	    ,send_charge_record_wjy_Succ},	
	{WJY_S_RealData			,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc 	,send_real_data_wjy			,send_real_wjy_Succ},
	{WJY_S_Rate_Ask     	,UP_S_FRAME_ACK		,eTick_15S,		UpCtrlSendCyc 	,send_Rate_wjy				,send_Rate_wjy_Succ},
	
	{WJY_S_Heart		    ,UP_S_FRAME_SELF	,eTick_20S,		UpCtrlSendCyc 	,send_heart_data_wjy		,send_heart_wjy_Succ},
    
	{WJY_S_Device_Fault		,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc	,send_Device_Fault_wjy		,send_Device_Fault_wjy_Succ},
	{WJY_S_TimeSyn_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_TimeSyn_ACK_wjy		,send_TimeSyn_ACK_wjy_Succ},

	{WJY_S_QR			    ,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc	,send_QR_ACK_wjy			,send_QR_ACK_wjy_Succ},
	{WJY_S_update_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_update_ACK_wjy		,send_update_ACK_wjy_Succ},
	{WJY_S_update_rst		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_update_rst_wjy		,send_update_rst_wjy_Succ},
	{WJY_S_reboot_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc	,send_reboot_ACK_wjy		,send_reboot_ACK_wjy_Succ},
	{WJY_S_Sum		        ,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc	,send_Sum_ACK_wjy			,send_Sum_ACK_wjy_Succ},

};


uint16_t Calculate_CRC_CCITT(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0x0000;  // 初始值
    const uint16_t polynomial = 0x1021;  // CRC-CCITT标准多项式
    
    // 处理每个字节
    for (uint32_t i = 0; i < length; i++) 
    {
        // 处理每个bit（从高位到低位）
        for (uint8_t j = 0; j < 8; j++) 
        {
            bool bit = (data[i] >> (7 - j)) & 0x01;
            bool c15 = (crc >> 15) & 0x01;
            
            crc <<= 1;
            
            if (c15 ^ bit) 
            {
                crc ^= polynomial;
            }
        }
    }
    
    return crc & 0xFFFF;  // 确保返回16位结果
}

uint16_t Verify_Message_CRC(const uint8_t *message, uint32_t msg_length)
{
    // 至少需要2字节CRC + 1字节数据
    if (msg_length < 3) 
    {
        return false;
    }
    
    // 计算除最后2字节CRC外的数据CRC
    uint16_t calculated_crc = Calculate_CRC_CCITT(message, msg_length);
	// printf("\r\n calculated_crc = 0x%04x\r\n", calculated_crc);
    return calculated_crc;
}

static uint16_t WJY_dataEncode(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	WJY_HEAD_T *pHeart = (WJY_HEAD_T*)p;
    uint16_t crc_len = data_len[0] + WJY_HEAD_LEN;    //消息体+消息头
	uint16_t all_len = crc_len + 2;         //加上crc校验码的总长度

	uint16_t crc = 0;
    printf("\r\n wjy_dataEncode_crc_len = %d\r\n",crc_len);
    
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
    //number自增
    //起始标志、命令码、消息序号、设备编号长度、设备编号、加密标志、消息体长度
    pHeart->head = WJY_FRAME_HEAD;
	
    if ((cmd == WJY_S_RealData) && (GetSendSrm(u8Port, cmd))) {
        Uint16ToTwoUint8(pHeart->ser, GetSendSrm(u8Port, cmd));
        SetSendSrm(u8Port, cmd, 0);
    }
	else if(UP_S_FRAME_ACK == type)
	{
		Uint16ToTwoUint8(pHeart->ser, GetSendSrm(u8Port, cmd));		//消息序号
	}
	else
	{
		Uint16ToTwoUint8LH(pHeart->ser, pUpGunData->up_srm);
        pUpGunData->up_srm++;
	}

    pHeart->dev_len = WJY_PILE_LEN ;		//设备编号长度

	monitor_getDevNumber_wjy(&pHeart->dev_variable[0], pHeart->dev_len);		//设备编号（桩号）

	// if (WJY_S_Aes_Key == cmd)   //除了请求秘钥其余命令帧都加密
    pHeart->dev_variable[WJY_PILE_LEN] = WJY_PROTOCOL_NOENCRYPT;	//先做不加密
    // else
    //     pHeart->EncType = WJY_PROTOCOL_ENCRYPT;
	
	pHeart->cmd = cmd;
	
	Uint16ToTwoUint8LH(&pHeart->dev_variable[WJY_PILE_LEN+1], data_len[0]);   //消息体长度
	
    crc = Verify_Message_CRC(&pHeart->head, crc_len);  //对校验位之前的数据进行CRC校验
	Uint16ToTwoUint8LH(&p[crc_len], crc);   //消息体长度
	
    all_len = crc_len + 2;      

	data_len[0] = all_len;
	
    return all_len;
}
static uint16_t WJYUpCtrlSendDeal(void *pBuf ,uint32_t u32BufSize)
{
	const WJY_Send_ctrl *pWJYSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	WJY_HEAD_T *pHead = (WJY_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + WJY_HEAD_LEN;
	
	for(i = 0; i < GUN_NUM; i++)	
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrWJYSendCtrl); u32i++)
		{
			pWJYSendCtrl = &StrWJYSendCtrl[u32i];
			
			if (SEND_ENABLE_ON != GetSendEnable(i, pWJYSendCtrl->cmd))
				continue;
			
			if (TRUE == pWJYSendCtrl->pSendCyc(i, pWJYSendCtrl->cmd, pWJYSendCtrl->cyc))
			{
				if ((outLen = pWJYSendCtrl->pSend(i, pData, u32BufSize)) >= 0)
	            {	
					// WJY_Send_Data_Encrypt( pWJYSendCtrl->cmd, pData, &outLen);
	            	WJY_dataEncode(i, (uint8_t*)pHead, pWJYSendCtrl->cmd, pWJYSendCtrl->FType, &outLen);
					pWJYSendCtrl->pSendSucc(i);
					SetSendTick(i, pWJYSendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pWJYSendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pWJYSendCtrl->cmd, FALSE);
					
					UPRINT("\r\n WJY_UpProtocol --> GUN: %d, SendDealcmd: 0X%02x \r\n", i, pWJYSendCtrl->cmd);
					return outLen;
				}
			}
		}
	}
	
	return outLen;
}

static void WJYUpSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;
	
	outLen = WJYUpCtrlSendDeal(pbuf, sizeof(pbuf));
	
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);
	
	return;
}


























uint8_t recv_Aes_Key_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_Aes *pRecvAes = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvAes;
	
	hex_dump("----------    WJY_Recv_Aes_Key,Recv_data----------",r_data,len);
	memcpy(pRecvAes, r_data, sizeof(WJY_Recv_Aes));
	
	return TRUE;
}

void recv_Aes_Key_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Aes *pRecvAes = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvAes;
	static unsigned char random_key_own[32] = {0};            	// 平台下发密钥

	memcpy(random_key_own, pRecvAes->aes_key, pRecvAes->aes_len);
	hex_dump("----------    WJY_Recv_Aes_Key,Recv_data----------",pRecvAes->aes_key,pRecvAes->aes_len);


	SetSendEnable(GUN_A, WJY_S_Identification, SEND_ENABLE_ON);
	Send_Immediately(GUN_A, WJY_S_Identification);

	return;
}

uint8_t recv_Aes_Err_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;

	
	return TRUE;
}

void recv_Aes_Err_wjy_Succ(uint8_t u8Port)
{

	return;
}


uint8_t recv_login_data_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvIdenf;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvIdenf, r_data, sizeof(WJY_Recv_Identification));
	return TRUE;
}

void recv_login_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvIdenf;
	
	if(0 == pRecvIdenf->result)
	{
		SetSendEnable(u8Port, WJY_S_Identification, SEND_ENABLE_OFF);
		printf("\r\n WJY_Identification Succ\n");

		for(uint8_t i = 0; i < GUN_NUM; i++)	//登录成功发心跳
		{
			SetSendEnable(i, WJY_S_Heart, SEND_ENABLE_ON);
			Send_Immediately(i, WJY_S_Heart);
		}
        Set_PlatConnectSta(eOnline_Auth);
	}
	
	return;
}



uint8_t recv_auth_data_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
   uint8_t u8Port=0;
   WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;

   if(r_data[51] < '0' || r_data[51] > '1')
     return FALSE;
    else
	  u8Port = r_data[51] - 0x30;//订单号最后一位表示枪号
    printf("recv_auth_data_wjy:u8Port=%d\r\n", u8Port);
	if (u8Port >= GUN_NUM_MAX)
		return FALSE;
	
	WJY_Recv_Auth_Ack *pRecvAuth = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvAuthAck;
	memcpy(pRecvAuth, r_data, sizeof(WJY_Recv_Auth_Ack));

	gun[0] = u8Port;

	return TRUE;
}


void recv_auth_wjy_Succ(uint8_t u8Port)
{
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	WJY_Recv_Auth_Ack *pRecvAuth = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvAuthAck;
	WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;
	uint8_t up_fail_reason = 0;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t sum_balance = 0;
    char card_str[16+1] = {0};
    uint8_t card_num[8] = {0};

	memcpy(card_str, pRecvAuth->card_num, 16);  //取平台下发卡号前16位

    AsciiPToBCD(card_str, (char *)card_num, 16);  //转成bcd码
	printf("\r\n recv_auth_wjy_Succ,card_num = \r\n");
	for (uint8_t i = 0; i < 8; i++)
	{
		printf("%02x", card_num[i]);
	}
	printf("\r\n");
	
	//鉴权状态退出
	monitor_set_MonitorState(u8Port, eMonitorState_Service);
	printf("\r\n recv_auth_wjy_Succ,sum_balance = %d \r\n",FOUR_UINT8_TO_UINT32(pRecvAuth->account_balance));


	//蔚景云
	if(0 == pRecvAuth->Auth_success)
	{
		sum_balance = FOUR_UINT8_TO_UINT32(pRecvAuth->account_balance);

		if (TRUE == monitor_charge_start(u8Port, &up_fail_reason, eUP_Start_Style_CardOnline,
										 (uint8_t *)card_num,
										 pRecvAuth->transaction_log_num,
										 &sum_balance))
		{
			fgv_CtrlStartCharge(u8Port);

			memcpy(&pWjyUpInfo->Rate_Id[0], &pRecvRateModel->rate_id[0], 4); // 开始充电保存当前的费率id到全局变量中
			memcpy(&pWjyUpInfo->Rate_Ver[0], &pRecvRateModel->rate_ver[0], 4);

			SetSendEnable(u8Port, WJY_S_RealData, SEND_ENABLE_ON);
		}
	}

	SetSendEnable(u8Port, WJY_S_Auth, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, WJY_R_Auth, RECV_ENABLE_OFF);
	
	return;
}


uint8_t recv_start_ack_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[0] - 1;
	WJY_Recv_Start_Charge *pRecvStartCharge = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStartCharge = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvStartCharge;
	
	memcpy(pRecvStartCharge, r_data, sizeof(WJY_Recv_Start_Charge));
	return TRUE;
}

void recv_start_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvStartCharge;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	// uint32_t sum_balance = 0;
	uint32_t sum_balance = FOUR_UINT8_TO_UINT32(pRecvStartCharge->account_balance);
	printf("\r\n recv_start_wjy_Succ,sum_balance = %d \r\n",sum_balance);

	uint8_t StartType = pRecvStartCharge->chg_type;  //充电策略
	uint32_t StartPara = FOUR_UINT8_TO_UINT32(pRecvStartCharge->type_data);  //充电策略数据
	printf("recv_start_wjy_Succ,charge_mode = %d para:%d\r\n", StartType, StartPara);
	
	if (StartType == E_WJY_GunState_Money) {  //按金额 单位0.01元
        SetDetectModeParam(u8Port, eDetectMode_Count, StartPara / 10);
    } else if (StartType == E_WJY_GunState_Time) {  //按时间 单位分钟
        SetDetectModeParam(u8Port, eDetectMode_Time, StartPara / 60);
    } else if (StartType == E_WJY_GunState_Ele) {  //按电量 单位0.01度
        SetDetectModeParam(u8Port, eDetectMode_Ele, StartPara / 100);
    } else if (StartType == E_WJY_GunState_Auto) {	//直到充满
        SetDetectModeParam(u8Port, eDetectMode_Auto, StartPara); //默认则传0
        // sum_balance = 0xFFFF;
    } 

	if(TRUE == monitor_charge_start(u8Port, \
		&pUpGunData->up_start_fail_reason, \
		eUP_Start_Style_App, \
		NULL, \
		pRecvStartCharge->transaction_num, \
		&sum_balance))
	{
		pUpGunData->up_start_ret = UP_RESULT_SUCC;
		pUpGunData->up_start_fail_reason = eStart_Fail_NULL;
		
		fgv_CtrlStartCharge(u8Port);

		SetSendEnable(u8Port, WJY_S_RealData, SEND_ENABLE_ON);
	}
	else
	{
		pUpGunData->up_start_ret = UP_RESULT_FAIL;
		fgv_CtrlStopCharge(u8Port);
	}

	SetSendEnable(u8Port, WJY_S_Start_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_Start_Chg_Ack);

	return;
}


uint8_t recv_stop_ack_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[0] - 1;
	printf("\r\n recv_stop_ack_wjy,u8Port = %d \r\n",u8Port);
	WJY_Recv_Stop_Charge *pRecvStopCharge = NULL;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStopCharge = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvStopCharge;
	
	memcpy(pRecvStopCharge, r_data, sizeof(WJY_Recv_Stop_Charge));
  
	return TRUE;
}

void recv_stop_wjy_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	WJY_Recv_Stop_Charge* pRecvStopCharge = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvStopCharge;
	uint8_t cmp_result = memcmp(pChgGunData->transaction_log_num, pRecvStopCharge->transaction_num, 16);
	printf("\r\n recv_stop_wjy_Succ,cmp_result = %d\r\n",cmp_result);

	if (cmp_result == 0)  //和当前充电流水号一致
	{
		stopPileCharge(u8Port, Pile_Stop_Reason_APP);
		pUpGunData->up_stop_ret = 0;
		pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL;
	}
	else
	{
		pUpGunData->up_stop_ret = 1;	
		pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_DevNumErr;  //失败原因订单号错误
	}
	
	SetSendEnable(u8Port, WJY_S_Stop_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_Stop_Chg_Ack);
	
	return;
}


uint8_t recv_charge_record_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	WJY_Recv_Record_Ack *pRecvRecordAck = NULL;
	

    gun[0] = u8Port;
	pRecvRecordAck = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRecordAck;

	memcpy(pRecvRecordAck, r_data, sizeof(WJY_Recv_Record_Ack));

    uint8_t result = 1;
    //查看账单应答属于哪个枪
    for (int i = 0; i < GUN_NUM_MAX; i++) {
		WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[i].strRecvRateModel;
	    CHG_DATA_T *pChgGunData = &g_chgData[i];
        WJY_charge_record *pRecord = &g_chgData[i].DealRecord.ChgRecord.WjyChgRecord;
        result = memcmp(pRecord->transaction_log_num, pRecvRecordAck->transaction_num, 16);
        if (result == 0) {
            *gun = i;
            updatePileStopReason(i, Pile_Stop_Reason_Finish);
            GNUpChargeStorageDeal(i, (void *)&g_chgData[i].DealRecord, sizeof(PlatDealRecord));		//存储更新的订单数据至flash，可通过日志召唤上报

			memcpy(&pWjyUpInfo->Rate_Id[0], &pRecvRateModel->rate_id[0], 4);  //订单结束更新当前费率id
			memcpy(&pWjyUpInfo->Rate_Ver[0], &pRecvRateModel->rate_ver[0], 4);

			pChgGunData->ExistChargeDeal = 0;  //如果平台回复则交易标志位清零
			
			pWjyUpInfo->SendSumFlag[i] = FALSE;		// 余额更新使能复位
			pWjyUpInfo->SendSumTime[i] = 0;		

			SetSendEnable(i, WJY_S_RealData, SEND_ENABLE_OFF);

			printf("\r\n [WJY]Charge transaction_log_num");
			for (uint8_t i = 0; i < 16; i++)
			{
				printf("%02x ",  pRecvRecordAck->transaction_num[i]);
			}			

				printf("\r\n [WJY]Charge record success,gun = %d,SendSumFlag=%d,SendSumTime=%dExistChargeDeal=%d \n",
					*gun,pWjyUpInfo->SendSumFlag[i],pWjyUpInfo->SendSumTime[i],pChgGunData->ExistChargeDeal);
            break;
        }
    }
  


	return TRUE;
}

void recv_charge_record_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Record_Ack *pRecvRecordAck = NULL;

	pRecvRecordAck = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRecordAck;

	if(0 == pRecvRecordAck->result)
	{
		SetSendEnable(u8Port, WJY_S_Chg_Record, SEND_ENABLE_OFF);
		printf("\r\n [WJY]Charge record success\n");
	}

	return;
}

uint8_t recv_Rate_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvRateModel, r_data, sizeof(WJY_Recv_Rate_Model));

	return TRUE;
}

void recv_Rate_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvRateModel;
	
	SetSendEnable(u8Port, WJY_S_Rate_Ask, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_Rate_Ask);
	
	WJYUpChargeRecordUpDealOffline();	//离线记录上报

	Save_rate_model(pRecvRateModel, sizeof(WJY_Recv_Rate_Model));

	return;
}

uint8_t recv_heart_data_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;

	WJY_Recv_Heart *pRecvHeart = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvHeart;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvHeart, r_data, sizeof(WJY_Recv_Heart));
	
	return TRUE;
}
void recv_heart_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Heart *pRecvHeart = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvHeart;
	
	Set_PlatConnectSta(eOnline_Heart);
	dev_clrErrExsit_all(eErr_PlatformOffline, __LINE__);

	for(uint8_t i=0;i<GUN_NUM;i++)
	{
		SetRecvEnable(i, WJY_R_Heart, RECV_ENABLE_OFF);
		SetRecvTick(i, WJY_R_Heart, Get_Systick());
	}

	// 心跳报文接收到后更新tick,用做超时重连时间
    PlatHeartTickRefresh();

	return;
}


uint8_t recv_Device_Fault_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;

	
	return TRUE;
}

void recv_Device_Fault_wjy_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, WJY_S_Device_Fault, SEND_ENABLE_OFF);

	return;
}



uint8_t recv_TimeSyn_ACK_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvTimeSyn;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvTimeSyn, r_data, sizeof(WJY_Recv_TimeSyn));
	
	return TRUE;
}
void recv_TimeSyn_ACK_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvTimeSyn;
	
	tm_struct SysTime;
    SysTime.yearH = 20;
	Bcd_to_Cp56time2a_wjy(&SysTime.yearL, &pRecvTimeSyn->sever_time[0]);
	printf("Time Syn Succ, Time: %d-%d-%d-%d %d:%d:%d\n", 
			SysTime.yearH, SysTime.yearL, SysTime.month, SysTime.day, SysTime.hour, SysTime.minute, SysTime.second);
    setCurrentRunTime((uint8_t *)&SysTime);

	SetSendEnable(u8Port, WJY_S_TimeSyn_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_TimeSyn_ACK);

	for(uint8_t i = 0; i < GUN_NUM; i++)	//收到校时后发送二维码请求
	{
		SetSendEnable(i, WJY_S_QR, SEND_ENABLE_ON);
		Send_Immediately(i, WJY_S_QR);
	}

	return;
}

uint8_t recv_QR_ACK_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[0] - 1;
	uint8_t QR_data[QR_MAX_SIZE] = {0};
	// WJY_Recv_QR *pRecvQR = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvQR;

	printf("recv_QR_ACK_wjy, u8Port = %d\n", u8Port);
	uint16_t copy_size = (r_data[1] << 8) + r_data[2];//二维码长度
	printf("recv_QR_ACK_wjy, copy_size = %02x\n", copy_size);
	if (copy_size > 200) {
		printf("Error: Copy size %u > qr_max_len \n", copy_size);
		return FALSE;
	}
	memcpy(QR_data, &r_data[3], copy_size);

	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
    storage_PlatQRCodeInfoStr(u8Port+1, (char*)QR_data);		//更新哪把枪传哪吧枪的二维码信息

	return TRUE;
}

void recv_QR_ACK_wjy_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, WJY_S_QR, SEND_ENABLE_OFF);
	return;
}

uint8_t recv_update_ACK_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_Update_ftp *pRecvUpdata = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvUpdata;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	uint8_t data_len = 0;

	memcpy(pRecvUpdata, &r_data[data_len], 58);  //前58字节固定
	data_len += 58;

    uint16_t filename_len = pRecvUpdata->path_len[0] << 8 | pRecvUpdata->path_len[1];

	memcpy(pRecvUpdata->file_path, &r_data[data_len], filename_len);  //文件路径
	data_len += filename_len;

	// memcpy(pRecvUpdata->username_len, &r_data[58 + filename_len], 1);  //用户名长度
	pRecvUpdata->username_len = r_data[data_len];		//用户名长度
	data_len ++;

	memcpy(pRecvUpdata->username, &r_data[data_len], pRecvUpdata->username_len);  //用户名
	data_len += pRecvUpdata->username_len;

	pRecvUpdata->password_len = r_data[data_len];	//密码长度
	data_len ++;
	
	memcpy(pRecvUpdata->password, &r_data[data_len], pRecvUpdata->password_len);	//密码
	
	return TRUE;

}

void recv_update_ACK_wjy_Succ(uint8_t u8Port)
{
	WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
    pWjyUpInfo->upResult = 0;
    up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    WJY_Recv_Update_ftp *pRecvUpdata = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvUpdata;

	g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;				//升级
    g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间

	char aUpd[8] = "aUpdate";
    char bUpd[8] = "bUpdate";

    char password[9] = {'\0'};
    char tPsw[9] = {'\0'};

    memcpy(password, pRecvUpdata->password, sizeof(password)-1);  //通过密码看升级a板还是b板
	printf("password = %s\r\n", password);				
    
	//充电不升级
	if(eChargeState_Idle != logic_get_gun_state(u8Port))
	{
		pWjyUpInfo->upResult = 2;      //失败
		g_ProtocolDCB.PlatTask.updata_flag = E_Update_Null;				//升级
		g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间
		printf("Error: Upgrade fail,upResult = %d\n",pWjyUpInfo->upResult);
	}	
	else  //空闲状态升级
	{
		if (memcmp(password, aUpd, 4) == 0) 
		{
			memcpy(tPsw, aUpd, sizeof(aUpd));
			printf("aUpd,tPsw = %s\r\n", tPsw);	
		} 
		else if (memcmp(password, bUpd, 4) == 0) 
		{
			memcpy(tPsw, bUpd, sizeof(bUpd));
			printf("bUpd,tPsw = %s\r\n", tPsw);	
		} 
		else 
		{
			pWjyUpInfo->upResult = 2;      //失败
			g_ProtocolDCB.PlatTask.updata_flag = E_Update_Null;				//升级
			g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间
			printf("Error: Upgrade file name error,tPsw = %s,result = %d\n",tPsw,pWjyUpInfo->upResult);
		}
	}  

    g_PileUpdateInterface("admin", 21, "wjyroot", tPsw, NULL, NULL);
	
	SetSendEnable(u8Port, WJY_S_update_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_update_ACK);

	SetSendEnable(u8Port, WJY_S_update_rst, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_update_rst);

	return;
}

//升级结果汇报
uint8_t recv_update_result_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;

	
	return TRUE;
}

void recv_update_result_wjy_Succ(uint8_t u8Port)
{

	return;
}


//远程控制 当前平台只支持重启操作
uint8_t recv_reboot_ACK_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvReboot;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvReboot, r_data, sizeof(WJY_Recv_Reboot));
	
	return TRUE;
}

//设备重启 故障状态也会重启
void Wjy_PileRebootCheck(void)
{
	WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	uint8_t i = 0;
	
	if(pWjyUpInfo->Wjy_Reboot_flag == E_Reboot_Null)
		return;

	for(i = 0; i < GUN_NUM; i++)
	{
		//充电不重启
		if(eChargeState_Idle != logic_get_gun_state(i) && eChargeState_StopFinish != logic_get_gun_state(i))
		{
			printf("logic_get_gun:%d , state = %d, can not reboot\r\n",i,logic_get_gun_state(i));
            if (pWjyUpInfo->Wjy_Reboot_flag == E_Reboot_Idle) {
			    pWjyUpInfo->Wjy_Reboot_flag = E_Reboot_Null;
			    return;
            }
		}
	}

	if (JudgeTimeOutMs(pWjyUpInfo->reboot_tick, eTick_5S) == TRUE) {

		printf("wjy_charge board reboot...\r\n");

		//主板重新启动
		fgv_CtrlPileOpr(E_DEV_CTRL_CMD_REBOOT);
		
		//控制主板重启完成后，网络单元重启
		osDelay(100);
		
		printf("wjy_network board reboot...\r\n");

		NVIC_SystemReset();
	}
	
	return;
}

void recv_reboot_ACK_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvReboot;
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
	
	pWjyUpInfo->Wjy_Reboot_result = 1;   //默认重启失败
	
	if (pRecvReboot->ctrl_type == 3)  
	{
		if (eChargeState_Idle == logic_get_gun_state(u8Port) || eChargeState_StopFinish== logic_get_gun_state(u8Port)) //枪空闲或停止完成状态才能重启   
		{
			pWjyUpInfo->Wjy_Reboot_result = 0;  //成功
			pWjyUpInfo->Wjy_Reboot_flag = E_Reboot_Idle;	//重启使能
			pWjyUpInfo->reboot_tick = Get_Systick();
		}
		else/*  */
			pWjyUpInfo->Wjy_Reboot_result = 1;  	
	}
	else  //非重启功能则操作失败
		pWjyUpInfo->Wjy_Reboot_result = 1;  
		
	SetSendEnable(u8Port, WJY_S_reboot_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, WJY_S_reboot_ACK);

	return;
}

uint8_t recv_Sum_ACK_wjy(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	WJY_Recv_SumUpdata *pRecvSum = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvSumUpdata;
	
	if(u8Port >= GUN_NUM_MAX) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvSum, r_data, sizeof(WJY_Recv_SumUpdata));
	
	return TRUE;
}

void recv_Sum_ACK_wjy_Succ(uint8_t u8Port)
{
	WJY_Recv_SumUpdata *pRecvSum = &g_ProtocolDCB.pWJYRecvData[u8Port].strRecvSumUpdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint8_t card_bcd[8] = {0};
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;

	uint32_t sum_balance = FOUR_UINT8_TO_UINT32(pRecvSum->account_balance);

	uint8_t card_result = memcmp(pRecvSum->transaction_num, pChgGunData->transaction_log_num, 16);
	if (0 == card_result)  //卡号一致
	{
		pChgGunData->sum_balance = sum_balance;
		pWjyUpInfo->SendSumFlag[u8Port] = TRUE;		//余额发送使能关闭
		printf("\r\n Sum Updata Succ, sum_balance: %d ,SendSumFlag = %d \n", pChgGunData->sum_balance,pWjyUpInfo->SendSumFlag[u8Port]);
	}
	SetSendEnable(u8Port, WJY_S_Sum, SEND_ENABLE_OFF);
	return;
}














void SetSendSrmWJY(uint8_t u8Port, uint32_t cmd, uint16_t Srm)
{
	if (cmd == WJY_R_Start_Chg || cmd == WJY_R_Stop_Chg || cmd == WJY_R_Rate 
		|| cmd == WJY_R_TimeSyn || cmd == WJY_R_update_ftp || cmd == WJY_R_set_reboot)
	{
		// printf("\r\n SetSendSrmWJY,WJY_R_0x%02x\n", cmd); 
		SetSendSrm(u8Port, cmd + 0x80, Srm);   //桩应答帧在原有基础上加0x80
	}
	else
		SetSendSrm(u8Port, cmd - 0x80, Srm);
}


static uint8_t WJYUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	if(WJY_R_Heart == cmd)
		Cyc *= 1;
	
	Cyc += eTick_5S;
	
	if(JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;
	
	return FALSE;
}

typedef uint8_t (*PRecvTimer)(uint8_t u8Port, uint32_t cmd, uint32_t OutTimer);
typedef uint8_t (*PRecv)(uint8_t *r_data, int len, uint8_t* gun);
typedef void (*PRecvSucc)(uint8_t u8Port);

typedef struct
{
    uint32_t		cmd;
    uint32_t     	timer;
    PRecvTimer 		pRecvTimer;
    PRecv 			pRecv;
    PRecvSucc 		pRecvSucc;
}WJY_Recv_ctrl;


const WJY_Recv_ctrl StrWJYRecvCtrl[]={
    {WJY_R_Identification	,eTick_60S,		WJYUpCtrlRecvTimer	,recv_login_data_wjy		,recv_login_wjy_Succ},		
	{WJY_R_Auth 			,eTick_30S,		WJYUpCtrlRecvTimer 	,recv_auth_data_wjy			,recv_auth_wjy_Succ},	
	
	{WJY_R_Start_Chg	    ,eTick_15S,		WJYUpCtrlRecvTimer	,recv_start_ack_wjy			,recv_start_wjy_Succ},		
	{WJY_R_Stop_Chg		    ,eTick_30S,		WJYUpCtrlRecvTimer 	,recv_stop_ack_wjy			,recv_stop_wjy_Succ},

	{WJY_R_Chg_Record		,eTick_30S,		WJYUpCtrlRecvTimer 	,recv_charge_record_wjy	    ,recv_charge_record_wjy_Succ},	
	{WJY_R_Rate     	    ,eTick_15S,		WJYUpCtrlRecvTimer 	,recv_Rate_wjy				,recv_Rate_wjy_Succ},
	
	{WJY_R_Heart		    ,eTick_60S,		WJYUpCtrlRecvTimer 	,recv_heart_data_wjy		,recv_heart_wjy_Succ},
    
	{WJY_R_Device_Fault		,eTick_30S,		WJYUpCtrlRecvTimer	,recv_Device_Fault_wjy		,recv_Device_Fault_wjy_Succ},
	{WJY_R_TimeSyn		    ,eTick_30S,		WJYUpCtrlRecvTimer	,recv_TimeSyn_ACK_wjy		,recv_TimeSyn_ACK_wjy_Succ},

	{WJY_R_QR			    ,eTick_30S,		WJYUpCtrlRecvTimer	,recv_QR_ACK_wjy			,recv_QR_ACK_wjy_Succ},
	{WJY_R_update_ftp		,eTick_30S,		WJYUpCtrlRecvTimer	,recv_update_ACK_wjy		,recv_update_ACK_wjy_Succ},
	{WJY_R_update_rst		,eTick_30S,		WJYUpCtrlRecvTimer	,recv_update_result_wjy		,recv_update_result_wjy_Succ},
	{WJY_R_set_reboot		,eTick_30S,		WJYUpCtrlRecvTimer	,recv_reboot_ACK_wjy		,recv_reboot_ACK_wjy_Succ},
	{WJY_R_Sum		        ,eTick_30S,		WJYUpCtrlRecvTimer	,recv_Sum_ACK_wjy			,recv_Sum_ACK_wjy_Succ},

};

void WJYUpCtrlRecvDeal(WJY_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const WJY_Recv_ctrl *pWJYRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(StrWJYRecvCtrl); u32i++)
    {
		pWJYRecvCtrl = &StrWJYRecvCtrl[u32i];
		
		if (cmd == pWJYRecvCtrl->cmd)
		{
			if(TRUE == pWJYRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pWJYRecvCtrl->pRecvSucc(port);
				
				SetSendSrmWJY(port, cmd, twoUint8ToUint16(pHead->ser));			
				
				SetRecvTick(port, cmd, Get_Systick());
				
				UPRINT("\r\n WJY_UpProtocol --> GUN: %d, RecvDealcmd: 0x%02x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}

//判断tcp接收到的所有数据是否合法
static int WJY_Tcp_Read_Data_Check(uint8_t *r_data)
{
    if (r_data[0] != 0x68) {
        printf("Check head erro  0x%x\r\n", r_data[0]);
        return -1;
    }
	
    uint16_t r_len = (r_data[WJY_HEAD_LEN-2] << 8 | r_data[WJY_HEAD_LEN-1]) ;	//读取消息头中消息体的长度
	printf("\r\n r_len: %d\r\n", r_len);

    uint16_t crc_len = r_len + WJY_HEAD_LEN;	//crc校验码地址
    
	uint16_t c_crc = Verify_Message_CRC(&r_data[0], crc_len);   //对校验位之前的数据进行CRC校验	
	// printf("\r\n Verify_Message_CRC,c_crc = 0x%04x\r\n", c_crc);

    uint16_t r_crc = 0;
	r_crc = r_data[crc_len] << 8 | r_data[crc_len + 1];
	
    if (c_crc !=  r_crc) {
        printf("Check crc erro  0x%x  0x%x\r\n", r_crc, c_crc);
        return -2;
    }
    return 0;
}

void WJYfrom_buffer_data(U8 *recv_buf, U16 *len)
{
    //从buffer里查找合法数据进行校验
    U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, len, TCP_DATA_LEN_MAX);
    
    if (recv_buf[0] == 0x68) {
		// printf("\r\n WJYfrom_buffer_data cmd: 0x%02x\r\n ",recv_buf[1]);
		// for (uint16_t i = 0; i < *len; i++)
		// {
		// 	printf("%02x ", recv_buf[i]);
		// }
		
        //继续寻找len
		read_len = *len;
        if (read_len > TCP_DATA_LEN_MAX) {
			printf("\r\n wjy_protocol--> recv buf full ! ");
            return;
        }
        *len = read_len;
    }
}

void WJYPackConnectHandle(U8 *recv_buf, int totalLen)
{
	uint16_t surplusLen = totalLen;
	uint16_t currentIndex = 0;
	WJY_HEAD_T *pHead = NULL;
	uint16_t datalen = 0;

	while(surplusLen) {	
		pHead = (WJY_HEAD_T*)(recv_buf + currentIndex);
		
		datalen = (pHead->dev_variable[WJY_PILE_LEN+1] << 8 | pHead->dev_variable[WJY_PILE_LEN+2] );
		// datalen = twoUint8ToUint16(&pHead->len[0]);	//消息体长度
		// printf("\r\n pHead->len[0]: %d,pHead->len[1]: %d,datalen = %d\r\n", pHead->len[0],pHead->len[1],datalen);
		// printf("\r\n datalen:%d\n", datalen);
		int packLen = datalen + WJY_HEAD_LEN + 2;	//包总长度 消息头+消息体+crc校验
		//防止乱数据导致程序死掉
		if (packLen > surplusLen) {
			printf("\r\n----------packLen>surplusLen----------,surplusLen = %d\r\n",surplusLen);
			return;
		}
		surplusLen = surplusLen - packLen;  //包剩余长度

		printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);
		
		if (WJY_Tcp_Read_Data_Check(recv_buf + currentIndex) < 0) {
			return;
		}

		hex_dump("tcp_recv_data", recv_buf + currentIndex, packLen);

	    WJYUpCtrlRecvDeal(pHead, pHead->cmd, recv_buf+currentIndex+WJY_HEAD_LEN ,packLen);
		
		currentIndex = currentIndex + packLen;
		}
}

void WJYUpRecvDeal(void)
{
    uint8_t from_tcp_data[TCP_DATA_LEN_MAX] = {0};
    U16 r_len = 0;
	YKC_HEAD_T *pHead = NULL;

    // PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, from_tcp_data, (U16 *)&r_len, TCP_DATA_LEN_MAX);

    WJYfrom_buffer_data(from_tcp_data, &r_len);
    if (r_len == 0)
        return;
    if (r_len > TCP_DATA_LEN_MAX) {
        printf("\r\nprotocol--> recv buf full ! ");
        return;
    }
	//粘包处理
	WJYPackConnectHandle(from_tcp_data, r_len);

	return;
}

void WJYRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if(WJY_R_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}
	
	if(WJY_R_Auth == cmd)
	{
		SetSendEnable(u8Port, WJY_S_Auth, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, WJY_R_Auth, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
//		pChgGunData->Card_err = eCardErr_NULL;
	}
	
	return;
}

void WJYUpCtrlRecvOutTime(void)
{
	const WJY_Recv_ctrl *pWJYRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrWJYRecvCtrl); u32i++)
	    {
			pWJYRecvCtrl = &StrWJYRecvCtrl[u32i];
			
			if(RECV_ENABLE_ON != GetRecvEnable(i, pWJYRecvCtrl->cmd))
				continue;
			
			if (TRUE == pWJYRecvCtrl->pRecvTimer(i, pWJYRecvCtrl->cmd, pWJYRecvCtrl->timer))
			{
				WJYRecvOutTimeDeal(i, pWJYRecvCtrl->cmd);
			}
		}
	}
	return;
}
//===========================================================================
//===========================================================================

void WJYUpLogin(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
    if (!Comm_getIpSuces(eSocket_GPRS1)) {

		return;
	}
    
	if(eOnline_Off == Get_PlatConnectSta())
	{
		printf("\r\n WJYUpLogin\n");
        Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, WJY_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, WJY_S_Identification);
	}
}

//当充电枪状态发生改变时，应立即上报一次心跳包
void WJYUpGunStateCheck(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	static uint8_t gun_state[GUN_NUM_MAX] = {0};  
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};  
	uint8_t report_flag[GUN_NUM_MAX] = {0};  
	
	if (logic_get_gun_charging(u8Port)) {
		pUpGunData->up_gun_state = WJY_Gun_State_Work;
	} else {
		if(TRUE == dev_getErrState(u8Port)) {
			pUpGunData->up_gun_state = WJY_Gun_State_Err;
        } 
		else 
		{
            if (TRUE == GetPile_gun_connect(u8Port)) 
			    pUpGunData->up_gun_state = WJY_Gun_State_Conn;
			else if (eChargeState_StopFinish == logic_get_gun_state(u8Port)) 
			    pUpGunData->up_gun_state = WJY_Gun_State_Finish;
			else
				pUpGunData->up_gun_state = WJY_Gun_State_Idle;
        }
	}
	
	if(gun_state[u8Port] != pUpGunData->up_gun_state)
	{
		gun_state[u8Port] = pUpGunData->up_gun_state;
		report_flag[u8Port] = TRUE;
	}
	
	if(gun_conn_state[u8Port] != GetPile_gun_connect(u8Port))
	{
		gun_conn_state[u8Port] = GetPile_gun_connect(u8Port);
		report_flag[u8Port] = TRUE;
	}
	
	if(TRUE == report_flag[u8Port])
	{
		printf("\r\n WJY_S_Heart,gun: %d,up_gun_state = %d\n", u8Port, pUpGunData->up_gun_state);
		Send_Immediately(u8Port, WJY_S_Heart);
	}
	

	return;
}

void WJY_CardAuthStart_Cmd(uint8_t u8Port)
{
	
	//插枪状态下刷有效卡，进行充电鉴权
    if(SEND_ENABLE_ON == GetSendEnable(u8Port, WJY_S_Auth)) {
        return;
    }

	printf("\r\n WJY_CardAuthStart_Cmd,gun: %d\n", u8Port);
    SetSendEnable(u8Port, WJY_S_Auth, SEND_ENABLE_ON);

    Send_Immediately(u8Port, WJY_S_Auth);
}

void WJY_DealUpdate_Cmd(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    if(eOnline_Off == Get_PlatConnectSta())
	{
		printf("WJY_DealUpdate_Cmd_V2:offline\r\n");
		return;
	}	
	printf("-----WJY_DealUpdate_Cmd_V2,WJY_S_Chg_Record-----\r\n");

    SetSendEnable(u8Port, WJY_S_Chg_Record, SEND_ENABLE_ON);
    Send_Immediately(u8Port, WJY_S_Chg_Record);
}




// 故障上报处理
static void WJYUpError(uint8_t u8Port)
{
	// uint8_t u8Port;
	static uint8_t SendFaultFlag[GUN_NUM_MAX] = {0};		// 故障发生发送标记
	static uint8_t SendResetFlag[GUN_NUM_MAX] = {0};		// 故障复位发送标记
	static uint8_t ErrSts[GUN_NUM_MAX][ARRAY_SIZE(error_map)] = {0};
	uint8_t i;						// source error index
	static uint16_t flow = 0;
		
	if(TRUE != UpOnlineFlag())
		return;

	// one by one	
	for(u8Port=0; u8Port<GUN_NUM; u8Port++)
	{
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, WJY_S_Device_Fault))
			return;
	}

	for(u8Port=0; u8Port<1; u8Port++)
	{
		for(i=0; i<ARRAY_SIZE(error_map); i++)
		{
			if(ErrSts[u8Port][i] != dev_getErrExsit(u8Port, error_map[i].err_type)) //故障状态改变
			{
				ErrSts[u8Port][i] = dev_getErrExsit(u8Port, error_map[i].err_type);
				WJY_Send_ErrSend *wjyErrorSwap = &WjyErrSend[u8Port];
				if(ErrSts[u8Port][i] == TRUE)		// error happen
				{	
					uint8_t StartNum[6] = {0};
					Time_to_YYMMDDHHMMSS(StartNum);
					BINToBCD(wjyErrorSwap->StartTime, StartNum, sizeof(wjyErrorSwap->StartTime));

					printf("\r\n Error_time: %x-%x-%x-%x:%x:%x \n", wjyErrorSwap->StartTime[0],wjyErrorSwap->StartTime[1],wjyErrorSwap->StartTime[2],
												wjyErrorSwap->StartTime[3],wjyErrorSwap->StartTime[4],wjyErrorSwap->StartTime[5]);
					wjyErrorSwap->status = 1;
					SendFaultFlag[i] = TRUE;
					wjyErrorSwap->gun = u8Port + 1;

					get_hard_err_code_wjy(u8Port);
					printf("\r\n GUN = %d , WarnId = %02x %02x\r\n",wjyErrorSwap->gun ,wjyErrorSwap->WarnPlatId[0],wjyErrorSwap->WarnPlatId[1]);
					break;		// up one error every time
				}
				else			// error cancel
				{					
					uint8_t StopNum[6] = {0};
					Time_to_YYMMDDHHMMSS(StopNum);
					BINToBCD(wjyErrorSwap->StopTime, StopNum, sizeof(wjyErrorSwap->StopTime));
					printf("\r\n error_cancel_time: %x-%x-%x-%x:%x:%x \n", wjyErrorSwap->StopTime[0],wjyErrorSwap->StopTime[1],wjyErrorSwap->StopTime[2],
												wjyErrorSwap->StopTime[3],wjyErrorSwap->StopTime[4],wjyErrorSwap->StopTime[5]);
					wjyErrorSwap->status = 0;		
					printf("\r\n GUN = %d , error_cancel , err_type = %d \r\n",wjyErrorSwap->gun,error_map[i].err_type);
					SendResetFlag[i] = TRUE;
					break;				
				}
			}
		}	
		if(SendFaultFlag[i] == TRUE ||  SendResetFlag[i] == TRUE)
			break;		// up one error every time
	}

	if(SendFaultFlag[i] == TRUE)//故障产生发送
	{
		SendFaultFlag[i] = FALSE;
		SetSendEnable(u8Port, WJY_S_Device_Fault, SEND_ENABLE_ON);
		Send_Immediately(u8Port, WJY_S_Device_Fault);
	}
	
	if(SendResetFlag[i] == TRUE)//故障清除发送
	{
		SendResetFlag[i] = FALSE;
		SetSendEnable(u8Port, WJY_S_Device_Fault, SEND_ENABLE_ON);
		Send_Immediately(u8Port, WJY_S_Device_Fault);
	}
}


void wjy_charge_sum_detect(uint8_t u8Port)
{
    WJY_UpPlatInfo *pWjyUpInfo = &s_WjyUpInfo;
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (1 != logic_get_gun_charging(u8Port))  return;  //不在充电状态不进行检测

	if (pWjyUpInfo->SendSumTime[u8Port] >= 3) //连续发送三次后停止发送
	{
		printf("wjy_charge_sum_detect,u8Port = %d,SendSumTime = %d,",u8Port,pWjyUpInfo->SendSumTime[u8Port]);
		pWjyUpInfo->SendSumTime[u8Port] = 0;		
		pWjyUpInfo->SendSumFlag[u8Port] = TRUE;		
	}

	//已充金额
    U32 money = monitor_getChgTotalMoney(u8Port) / 100;  //小数点后两位
	U32 sum = pChgGunData->sum_balance;
    U32 money_diff = pChgGunData->sum_balance - money;
    if (money_diff < 200 && FALSE == pWjyUpInfo->SendSumFlag[u8Port]) { //余额小于2元时向平台请求最新余额 只上报一次
		printf("\r\n WJY_S_Sum,gun: %d,money = %d , SendSumFlag = %d\n", u8Port, money,pWjyUpInfo->SendSumFlag[u8Port]);
		pWjyUpInfo->SendSumFlag[u8Port] = TRUE;		// 
		pWjyUpInfo->SendSumTime[u8Port] ++;		
		
        SetSendEnable(u8Port, WJY_S_Sum, SEND_ENABLE_ON);
		Send_Immediately(u8Port, WJY_S_Sum);
    }
	return;
}


void WJYUpCtrlTaskDeal(void)
{
	uint8_t i = 0;
	
	WJYUpLogin();
	
	for (i = 0; i < GUN_NUM; i++)   
	{
		WJYUpGunStateCheck(i);  	//枪变位上报
		WJYUpError(i);				//故障上报处理
		wjy_charge_sum_detect(i);  	//请求最新余额
		Wjy_PileRebootCheck();		//远程重启
	}
		
	return;
}

void WJYUpProtocolDeal(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	if(NULL == pProtocolDCB->pWJYRecvData)
		return;
	
	WJYUpCtrlTaskDeal();	//任务状态处理
	WJYUpRecvDeal();		//接收处理
	WJYUpSendDeal();		//发送处理
	WJYUpCtrlRecvOutTime();	//超时处理
	
	return;
}





















