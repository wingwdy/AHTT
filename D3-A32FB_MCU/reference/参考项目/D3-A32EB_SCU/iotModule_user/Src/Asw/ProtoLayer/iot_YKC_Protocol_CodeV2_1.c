#include "iot_YKC_Protocol_CodeV2_1.h"
#include "iot_GN_Protocol_Code.h"
#include "protocol_data.h"
#include "mbsMaster.h"
#include "maths.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "AppEvenCycle.h"
#include "cost.h"
#include "rsaOwn.h"
#include <stdlib.h>
#include <tcp_gn.h>
#include "AppDealFlash.h"

/*==============================================================*/
#define PROTOCOL_ENCRYPT_YKC    1           //加密
#define PROTOCOL_NO_ENCRYPT_YKC    0           //不加密

static YKC21_UpPlatInfo s_YKC21UpInfo;
YKC21_FlashPlatInfo s_YkcFlashInfo;

//==================================================
void Time_to_Cp56time2a_v2(cp56time2a_v2 *pCp56)
{
	tm_struct strCurTime = get_current_time();
		
	pCp56->Year = strCurTime.yearL;
	pCp56->Month = strCurTime.month;
	pCp56->Date = strCurTime.day;
	pCp56->Hour = strCurTime.hour;
	pCp56->Minute = strCurTime.minute;
	Uint16ToTwoUint8(pCp56->MilliSec, strCurTime.second*1000);
	return;
}
void Cp56time2a_Set_Time_v2(cp56time2a_v2 *pCp56)
{
	tm_struct SysTime;
	tm_struct *pSysTime = &SysTime;

	pSysTime->yearH = 20;
	pSysTime->yearL = pCp56->Year;
	pSysTime->month = pCp56->Month;
	pSysTime->day = pCp56->Date;
	pSysTime->hour = pCp56->Hour;
	pSysTime->minute = pCp56->Minute;
	pSysTime->second = twoUint8ToUint16(pCp56->MilliSec)/1000;
    
    setCurrentRunTime((uint8_t *)pSysTime);
	
	return;
}

uint32_t Cp56time2a_To_Time_v2(cp56time2a_v2 *pCp56)
{
	tm_struct SysTime;
	tm_struct *pSysTime = &SysTime;
	uint32_t ulSecond = 0;

	pSysTime->yearH = 20;
	pSysTime->yearL = pCp56->Year;
	pSysTime->month = pCp56->Month;
	pSysTime->day = pCp56->Date;
	pSysTime->hour = pCp56->Hour;
	pSysTime->minute = pCp56->Minute;
	pSysTime->second = twoUint8ToUint16(pCp56->MilliSec)/1000;
    
    timToStamp(&ulSecond, (tm_struct*)pSysTime);
	return ulSecond;
}
void Bin_to_Cp56time2a_v2(uint8_t *pTime, cp56time2a_v2 *pCp56)
{		
	pCp56->Year = pTime[0];
	pCp56->Month = pTime[1];
	pCp56->Date = pTime[2];
	pCp56->Hour = pTime[3];
	pCp56->Minute = pTime[4];
	Uint16ToTwoUint8(pCp56->MilliSec, pTime[5]*1000);
	return;
}

void Bcd_to_Cp56time2a_v2(uint8_t *pTime, cp56time2a_v2 *pCp56)
{
	pCp56->Year = U8BcdToBin(pTime[0]);
	pCp56->Month = U8BcdToBin(pTime[1]);
	pCp56->Date = U8BcdToBin(pTime[2]);
	pCp56->Hour = U8BcdToBin(pTime[3]);
	pCp56->Minute = U8BcdToBin(pTime[4]);
	Uint16ToTwoUint8(pCp56->MilliSec, U8BcdToBin(pTime[5])*1000);
	return;
}


//离线上报 云快充2.1
static uint8_t YKCUpChargeRecordUpDealOffline()
{
	for (uint8_t i = 0; i < GUN_NUM; i++ ) {
		charge_record_ykcv2 *pRecord = &g_chgData[i].DealRecord.ChgRecord.YkcRecord;		
		uint8_t uGun = i;

		uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }
        
        uint8_t cmd = YKC_V2_S_Chg_Record;

		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("\r\n YKCUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}
        
        pRecord->stop_reason = ykcV21Update_stopReason(uGun);
		printf("\r\n YKCUpChargeRecordUpDealOffline gun = %d stop_reason = %d\r\n", uGun, pRecord->stop_reason);
		
		SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
		Send_Immediately(uGun, cmd);
	}

    return TRUE;
}


/*******************************************************
*
*
*
*
*
*
*******************************************************/
static uint8_t Power_Limit_Flag[GUN_NUM_MAX];		//功率限制标志
static uint8_t Max_Power_Flag[GUN_NUM_MAX];			//最大功率限制标志

static uint8_t first_trigger[GUN_NUM_MAX] = {0};
static uint16_t Max_Power[GUN_NUM_MAX] = {0};

static unsigned char Default_Rsa_Key[] = { 				//公钥DER编码默认值
    "MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKaTP4eBWYBh3JDnYa7h2nuYACREgmV1o250/36ebYwaUswQDbUdMoeRvRIWxhCtXEzVkMYtH07ctmpzMo8uTvMCAwEAAQ=="
};
char TokenStr[15] = "90250314441003";			//集测默认token 与生产一致

static unsigned char random_key_own[16];            	// 随机密钥A
static unsigned char b64_buf[88];				 		// 64位base64编码
static unsigned char rsa_pub_key[128] ;					//公钥DER编码

struct AES_ctx g_ex;
static uint8_t test_iv[] = "m4uqsb0lm7ha6wr1";

//读取ykc21保存数据
void YKC21_ReadStoragePara(YKC21_FlashPlatInfo *pFlashInfo)
{
    if (load_EEOP_Param((uint8_t *)pFlashInfo, sizeof(YKC21_FlashPlatInfo)) == FALSE) {		//如果Flash为空，则写入默认值        
		memcpy(pFlashInfo->Rsa_Key, Default_Rsa_Key, RSA_KEY_LEN);
		memcpy(pFlashInfo->Token, TokenStr, 14);		
		printf("\r\n YKC21_ReadStoragePara init key.\r\n%s\r\n",pFlashInfo->Rsa_Key);
		printf("\r\n YKC21_ReadStoragePara init token.\r\n%s\r\n",pFlashInfo->Token);		
    }
	else
	{
		printf("\r\n YKC21_ReadStoragePara have key.\r\n%s\r\n",pFlashInfo->Rsa_Key);
		printf("\r\n YKC21_ReadStoragePara have token.\r\n%s\r\n",pFlashInfo->Token);
	}
}

//存储ykc21数据
void YKC21_WriteStoragePara(YKC21_FlashPlatInfo *pFlashInfo)
{
    Set_EEOP_Param((uint8_t *)pFlashInfo, sizeof(YKC21_FlashPlatInfo));
	printf("\r\n YKC21_WriteStoragePara\r\n");
}

//RSA公钥存储
void YKC21_WriteRsaKey(char *key)
{
	YKC21_FlashPlatInfo *pChangeKey = &s_YkcFlashInfo;
	memcpy(pChangeKey->Rsa_Key, key, RSA_KEY_LEN);

	YKC21_WriteStoragePara(&s_YkcFlashInfo);
}

//Token存储
void YKC21_WriteToken(char *token)
{
	YKC21_FlashPlatInfo *pChangeToken = &s_YkcFlashInfo;	
	memcpy(pChangeToken->Token, token, 14);

	YKC21_WriteStoragePara(&s_YkcFlashInfo);
}

//版本号最后一位补0
void pad_version(char* version, char* output) {
    char* last_dot = strrchr(version, '.'); // 找到最后一个'.'
    if (last_dot && strlen(last_dot + 1) == 1) { // 最后一段长度为1时才补零
        snprintf(output, strlen(version) + 2, "%.*s.0%s", 
                (int)(last_dot - version), version, last_dot + 1);
    } else {
        strcpy(output, version);
    }
}

uint16_t send_login_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	uint8_t u8VerLne = 0;
	uint8_t u8SoftVer = 0;
	char cSimID[20] = {0};
	YKC21_FlashPlatInfo *ykc_cfgInfo = &s_YkcFlashInfo;
	
	YKC21_ReadStoragePara(ykc_cfgInfo);
	
	encrypt_and_decrypt_data(ykc_cfgInfo->Rsa_Key,b64_buf,random_key_own);
	memcpy(&data[data_len], b64_buf, 88);
	data_len += 88;

    hex_dump("----------send Random key ----------", random_key_own, KEY_LEN);

	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    //设备类型
    data[data_len] = DEVICE_TYPE;
	data_len++;
	
    data[data_len] = GetPile_CfgGunNum();// 修改枪数量上传
	data_len++;

    data[data_len]   = 2; 
    data[data_len+1] = 1; 
    data[data_len+2] = 0; 
    data_len += 3; 	
	
	u8VerLne = strlen(SOFTWARE_VERSION) > 8 ? 8 : strlen(SOFTWARE_VERSION);
	
	memcpy(&data[data_len], SOFTWARE_VERSION, u8VerLne);//版本号修改
	data_len += u8VerLne;
	if (u8VerLne == 7)	 //不足8位补0  1.3.2.1->1.3.2.01
	{
		uint8_t temp = 0;
		temp = data[data_len-1];
		data[data_len - 1] = '0';
		data[data_len] = temp;
		printf("\r\n data[data_len - 1] = %d,data[data_len] = %d",data[data_len - 1],data[data_len]);
		data_len ++;
	}

	data[data_len] = 0;
	data_len++;
	
	GetNet_Comm_SimID((uint8_t*)cSimID, 20);
	AsciiPToBCD(cSimID, (char*)&data[data_len], 20);
	data_len += 10;
	
	//1,移动 2,联通 3,电信
	if(eOperator_CMCC == GetNet_Comm_Operator())
	{
		data[data_len] = 0;
	}
	else if(eOperator_CUCC == GetNet_Comm_Operator())
	{
		data[data_len] = 3;
	}
	else if(eOperator_CTCC == GetNet_Comm_Operator())
	{
		data[data_len] = 2;
	}
	else
	{
		data[data_len] = 4;
	}
	data_len++;

	//Token
	printf("\r\n send_login_data_ykc_v2,Token :%s\n", ykc_cfgInfo->Token);
	AsciiPToBCD((char*)&ykc_cfgInfo->Token, (char*)&data[data_len], 14);
	// AsciiPToBCD(TokenStr, (char*)&data[data_len], 14);
	data_len+=7;

	//手机号码
	memset(&data[data_len], 0, 11);
	data_len+=11;

	//支持网络制式
	data[data_len] = NET_TYPE;
	data_len++;

	//当前网络制式
	data[data_len] = NET_TYPE_NOW;
	data_len++;

	//经度
	memset(&data[data_len], 0, 4);
	data_len += 4;

	//纬度
	memset(&data[data_len], 0, 4);
	data_len += 4;
	
	return data_len;
}

void send_login_ykc_v2_Succ(uint8_t u8Port)
{
	return;
}

uint16_t send_heart_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    data[data_len] = u8Port + 1;
	data_len++;
	
    data[data_len] = dev_getErrState(u8Port);
	data_len++;
	
	return data_len;
}

void send_heart_ykc_v2_Succ(uint8_t u8Port)
{
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_V2_R_Heart))
	{
		SetRecvEnable(u8Port, YKC_V2_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_V2_R_Heart, Get_Systick());
	}
	
	return;
}


//计费模型验证请求
uint16_t send_Rate_Proving_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    
    YKC_V2_Recv_Rate_Model pRecvykc21RateModel;
	
    Read_rate_model((void *)&pRecvykc21RateModel, sizeof(YKC_V2_Recv_Rate_Model));

    //设备编码
    //设备编码需要转换成BIN码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
    //计费模型编码
    // memcpy(&data[data_len], rate_model.billing_model, 2);
    memset(&data[data_len], 0, 2);
	data_len += 2;

    return data_len;
}

void send_Rate_Proving_ykc_v2_Succ(uint8_t u8Port)
{
	
	return;
}

//充电设备计费模型请求
uint16_t send_Rate_Ask_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	

    return data_len;
}

void send_Rate_Ask_ykc_v2_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_real_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	memcpy(&data[data_len], pChgGunData->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pProtocolDCB->strUpGunData[u8Port].up_gun_state;
	data_len++;
	
	data[data_len] = 2;
	data_len++;

	data[data_len] = GetPile_gun_connect(u8Port);
	data_len++;
	
	Uint16ToTwoUint8(&data[data_len], GetPile_ChgOutVol(u8Port, 1));
	data_len += 2;
	// printf("GetPile_ChgOutVol(%d, 1) = %d\r\n", u8Port, GetPile_ChgOutVol(u8Port, 1));

	Uint16ToTwoUint8(&data[data_len], GetPile_ChgOutCur(u8Port, 2)/10);//0.1A
	data_len += 2;
	// printf("GetPile_ChgOutCur(%d, 2) = %d\r\n", u8Port, GetPile_ChgOutCur(u8Port, 2)/10);

	data[data_len] = monitor_getGunTem(u8Port);
	data_len++;

	memset(&data[data_len], 0, 8);
	data_len += 8;
	
	//soc
	data[data_len] = 0;
	data_len++;
	
	data[data_len] = 0;
	data_len++;
	
	Uint16ToTwoUint8(&data[data_len], (monitor_getChgTimer(u8Port)/60));
	data_len += 2;

	Uint16ToTwoUint8(&data[data_len], 0);
	data_len += 2;

	uint32ToFourUint8(&data[data_len], monitor_getChgTotalEnergy(u8Port));
	data_len += 4;
	// printf("gun = %d,monitor_getChgTotalEnergy = %d\r\n", u8Port, monitor_getChgTotalEnergy(u8Port));
	
	uint32ToFourUint8(&data[data_len], monitor_getChgTotalLossPower(u8Port));
	data_len += 4;

	uint32ToFourUint8(&data[data_len], monitor_getChgTotalMoney(u8Port));
	data_len += 4;
	
	Uint16ToTwoUint8(&data[data_len], get_hard_err_bit(u8Port));
	data_len += 2;
	
	//桩体温度
	data[data_len] =0;
	data_len++;

	//烟感状态
	data[data_len] = 0;
	data_len++;

	//电表示值
	memset(&data[data_len], 0, 5);
	data_len += 5;
	
    return data_len;
}

void send_real_ykc_v2_Succ(uint8_t u8Port)
{
	
	return;
}

//
uint16_t send_auth_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;	
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = 1;
	data_len++;
	
	data[data_len] = 0;
	data_len++;
	
	data_len += 4;
	reverse(&pChgGunData->PhyCard_number, &data[data_len], GNDATA_PHYCARD_LEN);
	// memcpy(&data[data_len], pChgGunData->PhyCard_number, GNDATA_PHYCARD_LEN);
	data_len += 4;
	
	memset(&data[data_len], 0, 16);
	data_len += 16;
	
	memset(&data[data_len], 0, 17);
	data_len += 17;

    return data_len;
}

void send_auth_ykc_v2_Succ(uint8_t u8Port)
{
	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_V2_R_Auth))
	{
		SetRecvEnable(u8Port, YKC_V2_R_Auth, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_V2_R_Auth, Get_Systick());
	}
	
	return;
}

uint16_t send_start_ack_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	YKC_V2_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvStartCharge;
	
	memcpy(&data[data_len], pRecvStartCharge->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pUpGunData->up_start_ret;
	data_len++;
	
	data[data_len] = pUpGunData->up_start_fail_reason;
	data_len++;

    return data_len;
}

void send_start_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Start_Chg_Ack, SEND_ENABLE_OFF);
	return;
}

uint16_t send_stop_ack_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
    //设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = pUpGunData->up_stop_ret;
	data_len++;
	
	data[data_len] = pUpGunData->up_stop_fail_reason;
	data_len++;
	
    return data_len;
}

void send_stop_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Stop_Chg_Ack, SEND_ENABLE_OFF);
	return;
}

//交易记录上传
uint16_t send_chg_record_data_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	charge_record_ykcv2 *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.YkcRecord;
	YKC_V2_Recv_Rate_Model *pRateModel = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateModel;
	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	memcpy(&data[data_len], pRecord->transaction_log_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
	memcpy(&data[data_len], pRecord->device_number, 7);
	data_len += 7;
	
	data[data_len] = pRecord->gun_num;
	data_len++;
	
	Bcd_to_Cp56time2a_v2(&pRecord->chrg_start_time[1], (cp56time2a_v2*)&data[data_len]);
	data_len += sizeof(cp56time2a_v2);
	
	Bcd_to_Cp56time2a_v2(&pRecord->chrg_stop_time[1], (cp56time2a_v2*)&data[data_len]);
	data_len += sizeof(cp56time2a_v2);
	
	//电表表号
	memset(&data[data_len], 0, 6);
	data_len += 6;

	//电表密文
	memset(&data[data_len], 0, 34);
	data_len += 34;

	//电表协议版本号
	memset(&data[data_len], 0, 2);
	data_len += 2;

	//加密方式
	memset(&data[data_len], 0, 1);
	data_len ++;

	memcpy(&data[data_len], pRecord->total_start_elec, 4);
	data_len += 4;
	data_len++;
		
	memcpy(&data[data_len], pRecord->total_stop_elec, 4);
	data_len += 4;
	data_len++;

	memcpy(&data[data_len], pRecord->total_power, 4);
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("%d ", data[data_len + i]);
	}
	
	data_len += 4;

	memcpy(&data[data_len], pRecord->total_loss_power, 4);
	data_len += 4;

	memcpy(&data[data_len], pRecord->total_money, 4);
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("%d ", data[data_len + i]);
	}
	data_len += 4;
	
	memcpy(&data[data_len], pRecord->vin, 17);
	data_len += 17;
	
	data[data_len] = pRecord->trade_flag;
	data_len++;
	
	Bcd_to_Cp56time2a_v2(&pRecord->trade_time[1], (cp56time2a_v2*)&data[data_len]);
	data_len += sizeof(cp56time2a_v2);
	
	data[data_len] = pRecord->stop_reason;
	data_len++;
	
	data_len += 4;
	reverse(&pChgGunData->PhyCard_number, &data[data_len], GNDATA_PHYCARD_LEN);		//与平台交互用物理卡号
	data_len += 4;
	
	data[data_len] = pRecord->fee_num;
	data_len++;

	for (uint8_t i = 0; i < pRecord->fee_num; i++)
	{
		memcpy(&data[data_len], pRecord->fee_rate[i], 4);//费率单价
		data_len += 4;
		memcpy(&data[data_len], pRecord->fee_ele[i], 4);//费率电量
		data_len += 4;
		memcpy(&data[data_len], pRecord->loss_fee_ele[i], 4);//费率计损电量
		data_len += 4;

		uint64_t fee_money = (uint64_t)fourUint8ToUint32(pRecord->fee_rate[i])*fourUint8ToUint32(pRecord->fee_ele[i]);
		uint64_t loss_fee_money = (uint64_t)fourUint8ToUint32(pRecord->fee_rate[i])*fourUint8ToUint32(pRecord->loss_fee_ele[i]);
		uint32_t all_fee_money = (fee_money + loss_fee_money)/100000;
		uint8_t total_fee_money[4]={0};
		uint32ToFourUint8(&total_fee_money[0],all_fee_money);
		memcpy(&data[data_len], total_fee_money, 4);//费率金额
		data_len += 4;
	}

	for (uint8_t i = 0; i < 48; i++)	//时段恒为48个
	{
		memcpy(&data[data_len], pRecord->time_power[i], 4);//48时段电量
		data_len += 4;
	}

    
    uint32_t tTotalMoney = 0;
    memcpy(&tTotalMoney, &pRecord->total_money, 4);
    SetPlat_ChgTotalMoney(u8Port, tTotalMoney);

    return data_len;
}

uint16_t send_charge_record_data_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	data_len += send_chg_record_data_v2(u8Port, data, inlen);
	
    return data_len;
}

void send_charge_record_ykc_v2_Succ(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	pChgGunData->upDealCnt++;
	
	if (pChgGunData->upDealCnt >= 3) {
		//上报三次未回复，停止上报
        uint8_t t_Chg_Record = YKC_V2_S_Chg_Record;
        
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, t_Chg_Record))
		{
			SetSendEnable(u8Port, t_Chg_Record, SEND_ENABLE_OFF);
			pChgGunData->upDealCnt = 0;		
			printf("\r\n stop send record! upDealCnt = %d\n",pChgGunData->upDealCnt);
		}
	} else if (pChgGunData->upDealCnt == 1) {
        pChgGunData->ExistChargeDeal = 0;
    }
    
	return;
}

uint16_t send_Sum_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	YKC_V2_Recv_SumUpdata *pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvSumUpdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	memcpy(&data[data_len], pRecvSumUpdata->Physics_card_number, GNDATA_CARD_LEN);
	data_len += GNDATA_CARD_LEN;
	
	data[data_len] = pChgGunData->sum_updata_ret;
	data_len++;
	
    return data_len;
}

void send_Sum_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Sum_ACK, SEND_ENABLE_OFF);
	return;
}

typedef struct {
    E_ErrCode err_type;
    uint8_t err_plat_type;
    uint16_t err_code;
} err_map_t;

YKC_V2_Send_ErrSend ErrSendPlatform[GUN_NUM_MAX];

//故障类型
static const err_map_t error_map[] = {
    {eErr_CpVoltAbnor,       ePlatType_D,	0x0324},
    {eErr_CpGroundFault,     ePlatType_D,	0x0440},
    {eErr_PEBreakFault,      ePlatType_D,	0x02E2},
    {eErr_EmergencyStop,     ePlatType_D,	0x02C3},
    {eErr_InputLineReversed, ePlatType_D,	0x0441},
    {eErr_LeakageCurrErr,    ePlatType_D,	0x031E},
    {eErr_DiodeStop,      	 ePlatType_D,	0x02DA},
    {eErr_ShortCircleErro,   ePlatType_D,	0x0313},  
    
    {eErr_OutputOverCurr,    ePlatType_D,	0x0303},
    {eErr_JcqMaloperation,   ePlatType_D,	0x0326},
	{eErr_JcqSynechiaFault,  ePlatType_D,	0x02C8},

	{eErr_ReaderCommErr,     ePlatType_D,	0x02C5},
    {eErr_MeterCommErr,   	 ePlatType_D,	0x02C6},  
    {eErr_EnvOverTempErr,    ePlatType_D,	0x02C9},
    {eErr_POverTempErr,      ePlatType_D,	0x02CA},
    {eErr_MeterCalcErr,   	 ePlatType_D,	0x02C7}, 
    
    {eErr_CCUSCUCommErr,     ePlatType_D,	0x030B},
	{eErr_PhaseLossErr,      ePlatType_D,	0x02E1},
    {eErr_InputOverVol,      ePlatType_D,	0x02DE},
    {eErr_InputLessVol,      ePlatType_D,	0x02DE},

    {eErr_NetNoSIMErr,       ePlatType_D,	0x02E8},  
    {eErr_NetSIMErr,         ePlatType_D,	0x02C1},  
    {eErr_NetIPErr,          ePlatType_D,	0x02F3},  
    {eErr_PlatformOffline,   ePlatType_C,	0x01B4}
};

//获取故障平台上传类型
uint8_t get_hard_err_type(uint8_t u8Port ,uint8_t err)
{
    for (uint16_t i = 0; i < sizeof(error_map)/sizeof(error_map[0]); ++i) {
        if (err == error_map[i].err_type) {
			return error_map[i].err_plat_type;
        }
    }

	return NULL;
}

//获取故障编码
uint16_t get_hard_err_code(uint8_t u8Port,uint8_t err)
{
    for (uint16_t i = 0; i < sizeof(error_map)/sizeof(error_map[0]); ++i) {
        if (dev_getErrExsit(u8Port, error_map[i].err_type)) {
            return error_map[i].err_code;
        }
    }

	return NULL;
}

//设备故障上送
uint16_t send_Device_Fault_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	YKC_V2_REcv_Device_Fault *pRecvDevFault = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceFault;
	YKC_V2_Send_ErrSend *ykc21ErrorSend = &ErrSendPlatform[u8Port];
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;

	data[data_len] = ykc21ErrorSend->WarnType;//故障类型
	data_len++;
	printf("\r\n GUN = %d, ykc21ErrorSend.WarnType = %d\r\n",u8Port,ykc21ErrorSend->WarnType);

	Uint16ToTwoUint8(&data[data_len], ykc21ErrorSend->WarnId);//故障编码
	data_len += 2;
	printf("\r\n GUN = %d, ykc21ErrorSend.WarnId = 0x%02x\r\n",u8Port,ykc21ErrorSend->WarnId);

	memcpy(&data[data_len], &ykc21ErrorSend->StartTime, 7);
	data_len += 7;
	
    return data_len;
}

void send_Device_Fault_ykc_v2_Succ(uint8_t u8Port)
{
    YKC21_UpPlatInfo *pYKC21UpInfo = &s_YKC21UpInfo;
	pYKC21UpInfo->upFaultCnt++;
	
	if (pYKC21UpInfo->upFaultCnt >= 3) {
		//上报三次未回复，停止上报
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_V2_S_Device_Fault))
		{
			SetSendEnable(u8Port, YKC_V2_S_Device_Fault, SEND_ENABLE_OFF);
			pYKC21UpInfo->upFaultCnt = 0;		
			printf("\r\n stop send fault! upFaultCnt = %d\n",pYKC21UpInfo->upFaultCnt);
		}
	} else if (pYKC21UpInfo->upFaultCnt == 1) {
        pYKC21UpInfo->ExistChargeFault = 0;
    }

	return;
}

//设备故障复位上送
uint16_t send_Device_Reset_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	YKC_V2_REcv_Device_Reset *pRecvDevReset = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceReset;
	YKC_V2_Send_ErrSend *ykc21ErrorReset = &ErrSendPlatform[u8Port];
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;

	data[data_len] = ykc21ErrorReset->WarnType;//故障类型
	data_len++;
	printf("\r\n GUN = %d, ykc21ResetErrorSend.WarnType = %d\r\n",u8Port, ykc21ErrorReset->WarnType);

	Uint16ToTwoUint8(&data[data_len], ykc21ErrorReset->WarnId);//故障编码
	data_len += 2;
	printf("\r\n GUN = %d, ykc21ResetErrorSend.WarnId = 0x%02x\r\n",u8Port, ykc21ErrorReset->WarnId);

	memcpy(&data[data_len], &ykc21ErrorReset->StopTime, 7);
	data_len += 7;
	
    return data_len;
}

void send_Device_Reset_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Device_Reset, SEND_ENABLE_OFF);
	return;
}

//从Flash中读取订单数据存入YkcRecord
uint8_t Ykc21_Record_Reload(uint8_t u8Port)
{
	charge_record_ykcv2 *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.YkcRecord;
	
	int ret = DealData_Read(u8Port, (uint8_t *)pRecord, sizeof(charge_record_ykcv2), 1);
	
	if (ret < 0) {
		printf("Ykc21_Record_Reload gun = %d ret < 0\r\n", u8Port);
		return FALSE;
	}
	return TRUE;
}

//检查是否能发送订单
uint8_t Ykc21_Record_Enable(uint8_t u8Port)
{
	YKC_V2_Recv_Deal *pRecvDeal = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeal;
	charge_record_ykcv2 *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.YkcRecord;

	if (logic_get_gun_Uncharged(u8Port) == 1)
	{
		printf("\r\n Ykc21_Record_Enable, gun not Idle\r\n");
    	return 2;		//召唤交易正在充电中
	}
	else if (pRecord->trade_flag == 0)
	{
		printf("\r\n Ykc21_Record_Enable,trade_flag  = 0!\r\n");
    	return 1;		//交易记录不存在
	}
	else if (memcmp(( const void*)pRecord->transaction_log_num, (const void*)pRecvDeal->deal_num,GNDATA_TRDNUM_LEN) != 0)
	{
		printf("\r\n Ykc21_Record_Enable,memcmp error\r\n");
		printf("\r\n pRecvDeal->deal_num = ");
		for (uint8_t i = 0; i < GNDATA_TRDNUM_LEN; i++)
		{
			printf("%02x ", pRecvDeal->deal_num[i]);
		}
		printf("\r\n");
    	return 1;		//交易记录不存在
	}
	
	printf("\r\n Ykc21_Record_Enable,send record on!\r\n");
    return 0;
}

//交易记录召唤确认
uint16_t send_Deal_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	YKC_V2_Recv_Deal *pRecvDeal = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeal;	
	memcpy(&data[data_len], pRecvDeal->deal_num, GNDATA_TRDNUM_LEN);
	data_len += GNDATA_TRDNUM_LEN;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port + 1;
	data_len++;

	uint8_t ret = Ykc21_Record_Enable(u8Port);
	if (ret != 0)
	{
		data[data_len] = CALL_FAIL;	//召唤失败
		data_len++;

		data[data_len] = ret;		//失败原因
		data_len++;

		printf("\r\n send_Deal_ACK_ykc_v2,record send fail reason = %d\n", ret);
	}
	else
	{
		data[data_len] = CALL_SUCC;//召唤成功
		data_len++;

		data[data_len] = 0;	
		data_len ++;
	}
	
    return data_len;
}

void send_Deal_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Deal_ACK, SEND_ENABLE_OFF);
	return;
}

//功率修改应答
uint16_t send_Power_Change_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	YKC_V2_Recv_Power_Change *pRecvPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPower;
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;

	data[data_len] = pRecvPower->gun;
	data_len++;

	if (logic_get_gun_charging(u8Port))		//功率修改仅能在充电中进行设置
	{
		data[data_len] = SET_SUCC;
		data_len++;
	}
	else
	{
		data[data_len] = SET_FAIL;	
		data_len++;		
	}
	
    return data_len;
}

void send_Power_Change_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port,YKC_V2_S_Power_Change_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_TimeSyn_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	YKC_V2_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvTimeSyn;
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	memcpy(&data[data_len], pRecvTimeSyn->device_number, 7);
	data_len += 7;
	
	Time_to_Cp56time2a_v2((cp56time2a_v2*)&data[data_len]);
	data_len += 7;
	
    return data_len;
}

void send_TimeSyn_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_TimeSyn_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_Max_Power_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;

	data[data_len] = u8Port + 1;
	data_len++;
	
	data[data_len] = SET_SUCC;
	data_len++;

    return data_len;
}

void send_Max_Power_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Max_Power_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_Rate_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = 1;
	data_len++;
	
    return data_len;
}

void send_Rate_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Rate_ACK, SEND_ENABLE_OFF);
	return;
}

uint16_t send_reboot_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = 1;
	data_len++;

    return data_len;
}

void send_reboot_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	
	SetSendEnable(u8Port, YKC_V2_S_reboot_ACK, SEND_ENABLE_OFF);
	
	return;
}

uint16_t send_update_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = ptcp_data->up_update_ret;
	data_len++;

    return data_len;
}

void send_update_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	SetSendEnable(u8Port, YKC_V2_S_update_ACK, SEND_ENABLE_OFF);
	
	//应答之后再升级
	pProtocolDCB->PlatTask.updata_delay_tick = Get_Systick();
	
	return;
}

uint16_t send_Key_Update_Ack_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = SET_SUCC;
	data_len++;
    
    return data_len;
}

void send_Key_Update_Ack_ykc_v2_Succ(uint8_t u8Port)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	SetSendEnable(u8Port, YKC_V2_S_Key_Update_Ack, SEND_ENABLE_OFF);

	return;
}

uint16_t send_QR_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port+1;
	data_len++;

	data[data_len] = 1;
	data_len++;

    return data_len;
}

void send_QR_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_QR_ACK, SEND_ENABLE_OFF);
	return;
}

void send_QR_ACK_v2_Succ_DD(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_S_QR_ACK_DD, SEND_ENABLE_OFF);
	return;
}

uint16_t send_S_Para_ACK_ykc_v2(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    uint16_t data_len = 0;  //此命令数据包长度
    uint8_t* data = (uint8_t*)pdata;
	
	//设备编码
	monitor_getDevNumber(&data[data_len], 7);
	data_len += 7;
	
	data[data_len] = u8Port+1;
	data_len++;

	data[data_len] = 1;
	data_len++;

    return data_len;
}

void send_S_Para_ACK_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_Para_ACK, SEND_ENABLE_OFF);
	return;
}


uint8_t UpCtrlSendCyc_V2(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	uint32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	if(TRUE == u8SendImmdFlag)
		return TRUE;
	
	if(YKC_V2_S_RealData == cmd)
	{
		if(eUP_Gun_State_Work == pUpGunData->up_gun_state)
			Cyc = eTick_15S;
	}
	
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
}YKC_V2_Send_ctrl;

#define  JX_SEND_IMMD 0

const YKC_V2_Send_ctrl StrYKCSendCtrl_v2[]={
    {YKC_V2_S_Identification	,UP_S_FRAME_SELF	,eTick_60S,		UpCtrlSendCyc_V2	,send_login_data_ykc_v2			,send_login_ykc_v2_Succ				},		//
	{YKC_V2_S_Heart 			,UP_S_FRAME_SELF	,eTick_10S,		UpCtrlSendCyc_V2 	,send_heart_data_ykc_v2			,send_heart_ykc_v2_Succ				},
	
	{YKC_V2_S_Rate_Proving		,UP_S_FRAME_SELF	,eTick_15S,		UpCtrlSendCyc_V2	,send_Rate_Proving_ykc_v2		,send_Rate_Proving_ykc_v2_Succ		},		//
	{YKC_V2_S_Rate_Ask			,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc_V2 	,send_Rate_Ask_data_ykc_v2		,send_Rate_Ask_ykc_v2_Succ			},
	{YKC_V2_S_RealData			,UP_S_FRAME_SELF 	,(eTick_60S*5),	UpCtrlSendCyc_V2 	,send_real_data_ykc_v2			,send_real_ykc_v2_Succ				},
	
	{YKC_V2_S_Auth				,UP_S_FRAME_SELF	,eTick_15S,		UpCtrlSendCyc_V2 	,send_auth_data_ykc_v2			,send_auth_ykc_v2_Succ				},
	
	{YKC_V2_S_Start_Chg_Ack		,UP_S_FRAME_ACK		,eTick_15S,		UpCtrlSendCyc_V2 	,send_start_ack_ykc_v2			,send_start_ykc_v2_Succ				},
	{YKC_V2_S_Stop_Chg_Ack		,UP_S_FRAME_ACK		,eTick_15S,		UpCtrlSendCyc_V2 	,send_stop_ack_ykc_v2			,send_stop_ykc_v2_Succ				},
	{YKC_V2_S_Chg_Record		,UP_S_FRAME_SELF	,eTick_30S,		UpCtrlSendCyc_V2	,send_charge_record_data_ykc_v2	,send_charge_record_ykc_v2_Succ		},
	
	{YKC_V2_S_Sum_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Sum_ACK_ykc_v2			,send_Sum_ACK_ykc_v2_Succ			},
	{YKC_V2_S_Device_Fault		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Device_Fault_ykc_v2		,send_Device_Fault_ykc_v2_Succ		},
	{YKC_V2_S_Device_Reset		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Device_Reset_ykc_v2		,send_Device_Reset_ykc_v2_Succ		},
	{YKC_V2_S_Deal_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Deal_ACK_ykc_v2			,send_Deal_ACK_ykc_v2_Succ			},

	{YKC_V2_S_Power_Change_ACK	,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Power_Change_ACK_ykc_v2	,send_Power_Change_ACK_ykc_v2_Succ	},
	{YKC_V2_S_TimeSyn_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_TimeSyn_ACK_ykc_v2		,send_TimeSyn_ACK_ykc_v2_Succ		},
	{YKC_V2_S_Rate_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Rate_ACK_ykc_v2			,send_Rate_ACK_ykc_v2_Succ			},
	{YKC_V2_S_Max_Power_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Max_Power_ACK_ykc_v2		,send_Max_Power_ACK_ykc_v2_Succ		},
	{YKC_V2_S_reboot_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_reboot_ACK_ykc_v2			,send_reboot_ACK_ykc_v2_Succ		},
	{YKC_V2_S_update_ACK		,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_update_ACK_ykc_v2			,send_update_ACK_ykc_v2_Succ		},
	{YKC_V2_S_QR_ACK			,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_QR_ACK_ykc_v2				,send_QR_ACK_ykc_v2_Succ			},
	{YKC_V2_S_QR_ACK_DD			,UP_S_FRAME_ACK 	,eTick_30S,		UpCtrlSendCyc_V2	,send_QR_ACK_ykc_v2				,send_QR_ACK_v2_Succ_DD				},
	{YKC_V2_S_Para_ACK			,UP_S_FRAME_ACK 	,eTick_30S,		UpCtrlSendCyc_V2	,send_S_Para_ACK_ykc_v2			,send_S_Para_ACK_ykc_v2_Succ		},
	{YKC_V2_S_Key_Update_Ack	,UP_S_FRAME_ACK		,eTick_30S,		UpCtrlSendCyc_V2	,send_Key_Update_Ack_ykc_v2		,send_Key_Update_Ack_ykc_v2_Succ	},
};

static void Ykc_Send_Data_Encrypt(uint32_t cmd, uint8_t* origin_data, uint16_t* origin_length)
{
	if ((YKC_V2_S_Identification == cmd) || 
		(YKC_V2_S_Heart == cmd) || 
		(YKC_V2_R_Heart == cmd) || 
		(YKC_V2_R_TimeSyn == cmd) || 
		(YKC_V2_S_TimeSyn_ACK == cmd)) 
	{
		return;
	}
	else
	{
		uint16_t data_length = *origin_length;
		// printf("\r\nYkc_send_data_encrypt,origin_length = %d\r\n",data_length);
		uint16_t padded_len = ((data_length + AES_BLOCKLEN - 1) / AES_BLOCKLEN) * AES_BLOCKLEN;
		uint8_t pad_value = padded_len - data_length;
		memset(&origin_data[data_length], pad_value, pad_value);
		data_length = padded_len;
		*origin_length = data_length;

		// printf("\r\n----------Ykc_send_data_encrypt,cmd = 0x%02x,data_length = %d----------\r\n",cmd,*origin_length);

		hex_dump("----------Ykc_send_data_encrypt,origin_data----------", origin_data, data_length);
		AES_init_ctx_iv(&g_ex, random_key_own, test_iv);
		AES_CBC_encrypt_buffer(&g_ex, origin_data, data_length);
	}		
}

static uint16_t YKC_dataEncode_V2(uint8_t u8Port, uint8_t *p, uint8_t cmd, uint8_t type, uint16_t *data_len)
{
	YKC_V2_HEAD_T *pHeart = (YKC_V2_HEAD_T*)p;
	uint16_t all_len = data_len[0] + 14 + 2;
    uint16_t crc_len = data_len[0] + 11;
	uint16_t crc = 0;
    // printf("YKC_dataEncode_V2_crc_len = %d\r\n",crc_len);
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
    //number自增
    //前导域、版本域、序号域、加密标志、命令字、长度域、数据域、校验域
    pHeart->head = YKC_FRAME_HEAD;
	
	if(UP_S_FRAME_ACK == type)
	{
		Uint16ToTwoUint8(pHeart->ser, GetSendSrm(u8Port, cmd));
	}
	else
	{
		Uint16ToTwoUint8LH(pHeart->ser, pUpGunData->up_srm);
        pUpGunData->up_srm++;
	}
	
	Time_to_Cp56time2a_v2((cp56time2a_v2*)&(pHeart->send_time));
	
	if ((YKC_V2_S_Identification == cmd) || 
		(YKC_V2_S_Heart == cmd) || 
		(YKC_V2_R_Heart == cmd) || 
		(YKC_V2_R_TimeSyn == cmd) || 
		(YKC_V2_S_TimeSyn_ACK == cmd)) 
	{
    	pHeart->EncType = PROTOCOL_NO_ENCRYPT_YKC;
	}
	else
	{
		pHeart->EncType = PROTOCOL_ENCRYPT_YKC;
	}	

	pHeart->cmd = cmd;
	
	Uint16ToTwoUint8LH(pHeart->len, crc_len);
	
	uint16_t crc_num=crc_len-2;
    //对校验位之前的数据进行CRC校验
    crc = CRC16(pHeart->ser, crc_len);

	if(ePlatType_DD == get_ChgParam_plat_type())
		Uint16ToTwoUint8(&p[crc_len+2], crc);
	else
    	memcpy(&p[crc_len+3], &crc, 2);
	
	data_len[0] = all_len;
	
    return all_len;
}

static uint16_t YKCUpCtrlSendDeal_V2(void *pBuf ,uint32_t u32BufSize)
{
	const YKC_V2_Send_ctrl *pYKCSendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;
	YKC_V2_HEAD_T *pHead = (YKC_V2_HEAD_T*)pBuf;
	uint8_t *pData = (uint8_t*)pBuf + sizeof(YKC_V2_HEAD_T);
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrYKCSendCtrl_v2); u32i++)
		{
			pYKCSendCtrl = &StrYKCSendCtrl_v2[u32i];
			
			if (SEND_ENABLE_ON != GetSendEnable(i, pYKCSendCtrl->cmd))
				continue;
			
			if (TRUE == pYKCSendCtrl->pSendCyc(i, pYKCSendCtrl->cmd, pYKCSendCtrl->cyc))
			{
				if ((outLen = pYKCSendCtrl->pSend(i, pData, u32BufSize)) > 0)
	            {	
					Ykc_Send_Data_Encrypt( pYKCSendCtrl->cmd, pData, &outLen);
	            	YKC_dataEncode_V2(i, (uint8_t*)pHead, pYKCSendCtrl->cmd, pYKCSendCtrl->FType, &outLen);
					pYKCSendCtrl->pSendSucc(i);
					SetSendTick(i, pYKCSendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pYKCSendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pYKCSendCtrl->cmd, FALSE);
					
					UPRINT("\r\nYKC_2.1_UpProtocol --> GUN: %d, SendDealcmd: 0x%02x \r\n", i, pYKCSendCtrl->cmd);
					return outLen;
				}
			}
		}
	}
	
	return outLen;
}

static void YKCUpSendDeal_V2(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;
	
	if(eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;
	
	outLen = YKCUpCtrlSendDeal_V2(pbuf, sizeof(pbuf));
	
	if (0 == outLen) return;
	
	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);
	
	return;
}

static void Ykc_Recv_Data_Decrypt(uint8_t *r_data, int len)
{
	AES_init_ctx_iv(&g_ex, random_key_own, test_iv);
    AES_CBC_decrypt_buffer(&g_ex, r_data, len);
    hex_dump("----------Ykc_Recv_Data_Decrypt,Decrypt_data----------", r_data, len);	
}

/*******************************************************/
//登录认证应答解析
uint8_t recv_login_data_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;

	uint16_t r_data_len = len-16;				//加密数据长度	
	YKC21_FlashPlatInfo *pYkcFlashInfo = &s_YkcFlashInfo;      
	
	memcpy(test_iv,random_key_own,16);			//初始向量与密钥一致

	Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	YKC_V2_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvIdenf;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvIdenf, r_data, sizeof(YKC_V2_Recv_Identification));
    
	YKC21_WriteRsaKey((char*)&pRecvIdenf->New_RSA_key);	
	
	
	return TRUE;
}

void recv_login_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvIdenf;
	
	if(0 == pRecvIdenf->charge_login_result)
	{
		SetSendEnable(u8Port, YKC_V2_S_Identification, SEND_ENABLE_OFF);

		SetSendEnable(u8Port, YKC_V2_S_Rate_Proving, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_V2_S_Rate_Proving);
		for(uint8_t i = 0; i < GUN_NUM; i++)
		{
			if(SEND_ENABLE_ON != GetSendEnable(i, YKC_V2_S_RealData))
			{
				SetSendEnable(i, YKC_V2_S_RealData, SEND_ENABLE_ON);
				Send_Immediately(i, YKC_V2_S_RealData);
			}
		}	
	}
	return;
}

uint8_t recv_Heart_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;
	YKC_V2_Recv_Heart *pRecvHeart = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvHeart = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvHeart;
	
	memcpy(pRecvHeart, r_data, sizeof(YKC_V2_Recv_Heart));

	return TRUE;
}

void recv_heart_ykc_v2_Succ(uint8_t u8Port)
{
	uint8_t i = 0;
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
    Set_PlatConnectSta(eOnline_Heart);

	dev_clrErrExsit_all(eErr_PlatformOffline, __LINE__);
	
	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, YKC_V2_R_Heart))
	{
		SetRecvEnable(u8Port, YKC_V2_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, YKC_V2_R_Heart, Get_Systick());
	}
	
	if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_V2_S_Identification))
	{
		SetSendEnable(u8Port, YKC_V2_S_Identification, SEND_ENABLE_OFF);
	}
	
	//实时数据所有枪都上报
	for(i = 0; i < GUN_NUM; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, YKC_V2_S_RealData))
		{
			SetSendEnable(i, YKC_V2_S_RealData, SEND_ENABLE_ON);
			Send_Immediately(i, YKC_V2_S_RealData);
		}
	}
	
	pProtocolDCB->PlatSta.no_Comm_tick = Get_Systick();
	
	return;
}

uint8_t recv_Rate_Proving_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Rate_Proving *pRecvRateProving = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateProving;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t r_data_len = len-16;				//加密数据长度	


    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	memcpy(pRecvRateProving, r_data, sizeof(YKC_Recv_Rate_Proving));

	return TRUE;
}

void recv_Rate_Proving_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Rate_Proving *pRecvRateProving = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateProving;
	uint8_t i = 0;

	SetSendEnable(u8Port, YKC_V2_S_Rate_Proving, SEND_ENABLE_OFF);
	
	//桩计费模型与平台不一致-向平台请求新计费模型
	if(pRecvRateProving->result == 1)
	{
		if(SEND_ENABLE_ON != GetSendEnable(u8Port, YKC_V2_S_Rate_Ask))
		{
			SetSendEnable(u8Port, YKC_V2_S_Rate_Ask, SEND_ENABLE_ON);
			Send_Immediately(u8Port, YKC_V2_S_Rate_Ask);
		}
	}

	//桩计费模型与平台一致-发送心跳
	else
	{
		for(i = 0; i < GUN_NUM; i++)
		{
			if(SEND_ENABLE_ON != GetSendEnable(i, YKC_V2_S_Heart))
			{
				SetSendEnable(i, YKC_V2_S_Heart, SEND_ENABLE_ON);
				Send_Immediately(i, YKC_V2_S_Heart);
			}
		}
	}	
	return;
}

uint8_t recv_Rate_Ask_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateModel;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t data_len = 0;
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);;
		
	memcpy(pRecvRateModel->device_number, &r_data[data_len], DEV_NUM_LEN);
	data_len += DEV_NUM_LEN;

	memcpy(pRecvRateModel->billing_model, &r_data[data_len], 2);
	data_len += 2;

	pRecvRateModel->Rate_quantity = r_data[data_len];
	data_len += 1;

	//计费模型数量
	uint8_t rate_num = (pRecvRateModel->Rate_quantity > 48) ? 48 : pRecvRateModel->Rate_quantity;

	for (uint8_t i = 0; i < rate_num; i++) {

		memcpy(pRecvRateModel->ele_rate[i], &r_data[data_len], 4);
		data_len += 4;
		
		memcpy(pRecvRateModel->ser_rate[i], &r_data[data_len], 4);
		data_len += 4;
	}

	pRecvRateModel->measure_wastage_rates =r_data[data_len];
	data_len += 1;

	for (uint8_t i = 0; i < 48; i++) {//48时段费率号
		pRecvRateModel->segmentation_rate[i] = r_data[data_len];
		(pRecvRateModel->segmentation_rate[i])--;//让第一个费率号对应第0个时段
		data_len += 1;
	}

	return TRUE;
}

void recv_Rate_Ask_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateModel;
	uint8_t i = 0;
	
	SetSendEnable(u8Port, YKC_V2_S_Rate_Ask, SEND_ENABLE_OFF);
	
	//双枪各自发心跳
	for(i = 0; i < GUN_NUM; i++)
	{
		if(SEND_ENABLE_ON != GetSendEnable(i, YKC_V2_S_Heart))
		{
			SetSendEnable(i, YKC_V2_S_Heart, SEND_ENABLE_ON);
			Send_Immediately(i, YKC_V2_S_Heart);
		}
	}
	YKCUpChargeRecordUpDealOffline();	//离线记录上报
	
	Save_rate_model(pRecvRateModel, sizeof(YKC_V2_Recv_Rate_Model));
	
	return;
}

uint8_t recv_RealData_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = r_data[7] - 1;

	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	return TRUE;
}

void recv_RealData_ykc_v2_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, YKC_V2_S_RealData, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_RealData);
	
	return;
}

uint8_t recv_Auth_Ack_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	uint8_t u8Port = r_data[23] - 1;
	YKC_V2_Recv_Auth_Ack *pRecvAuthAck = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;

	gun[0] = u8Port;
	
	pRecvAuthAck = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvAuthAck;
	
	memcpy(pRecvAuthAck, r_data, sizeof(YKC_V2_Recv_Auth_Ack));
	
	return TRUE;
}

static uint16_t power_limit_card[GUN_NUM_MAX] = {0};		//卡启动功率限制 默认0
static uint32_t ele_limit_card[GUN_NUM_MAX] = {0};		//卡启动充电电量限制 默认0

void recv_Auth_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Auth_Ack *pRecvAuthAck = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvAuthAck;
	uint8_t up_fail_reason = 0;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	uint32_t sum_balance = 0;
	uint8_t gum_num = pRecvAuthAck->gun - 1;
	
	//鉴权状态退出
	monitor_set_MonitorState(u8Port, eMonitorState_Service);

	sum_balance = fourUint8ToUint32(pRecvAuthAck->account_balance);
	power_limit_card[u8Port] = twoUint8ToUint16(pRecvAuthAck->Max_Power);				
	ele_limit_card[u8Port] = fourUint8ToUint32(pRecvAuthAck->Charge_capacity_limit);		
	printf("\r\n sum_balance = %d, power_limit = %d\r\n", sum_balance, power_limit_card[u8Port]);
	//云快充物理卡号鉴权,逻辑卡号应答
	if(1 == pRecvAuthAck->Auth_success)
	{
		monitor_charge_start(u8Port, &up_fail_reason, eUP_Start_Style_CardOnline, \
			pRecvAuthAck->Logic_card_number, \
			pRecvAuthAck->transaction_log_num, \
			&sum_balance);
		
		fgv_CtrlStartCharge(u8Port);
		if(power_limit_card[u8Port])		//功率限制
		{
			fgv_CtrlChargeCrt(gum_num, power_limit_card[u8Port]/100);
		}
		if (ele_limit_card[u8Port])		//充电电量限制
		{
			SetDetectModeParam(u8Port, eDetectMode_Ele, ele_limit_card[u8Port] / 10000);
			printf("\r\n recv_Auth_ykc_v2_Succ,ele_limit = %d \r\n", ele_limit_card[u8Port]);
		}		
	} else {
        SetPlat_CardChargeFaild(u8Port, 1);
    }
	
	SetSendEnable(u8Port, YKC_V2_S_Auth, SEND_ENABLE_OFF);
	SetRecvEnable(u8Port, YKC_V2_R_Auth, RECV_ENABLE_OFF);
	
	return;
}

uint8_t recv_Start_Charge_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	uint8_t u8Port = r_data[23] - 1;
	YKC_V2_Recv_Start_Charge *pRecvStartCharge = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvStartCharge;
	
	memcpy(pRecvStartCharge, r_data, sizeof(YKC_V2_Recv_Start_Charge));
	
	return TRUE;
}

static uint16_t power_limit_plat[GUN_NUM_MAX] = {0};		//APP启动功率限制 默认0
static uint32_t ele_limit_plat[GUN_NUM_MAX] = {0};		//APP启动充电电量限制 默认0

void recv_Start_Charge_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvStartCharge;
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	uint32_t sum_balance = 0;
	uint8_t gum_num = pRecvStartCharge->gun - 1;
	
	sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);
	power_limit_plat[u8Port] = twoUint8ToUint16(pRecvStartCharge->chg_max_power);		
	ele_limit_plat[u8Port] = fourUint8ToUint32LH(pRecvStartCharge->chg_ele_limit);		
	printf("\r\nsum_balance = %d, chg_ele_value = %d ,power_limit = %d\r\n", sum_balance, power_limit_plat[u8Port], ele_limit_plat[u8Port]);
	//云快充下发的逻辑卡号,无法用于停止充电,没有屏也不需要显示
	if(TRUE == monitor_charge_start(u8Port, \
		&pUpGunData->up_start_fail_reason, \
		eUP_Start_Style_App, \
		NULL, \
		pRecvStartCharge->transaction_log_num, \
		&sum_balance))
	{
		pUpGunData->up_start_ret = UP_RESULT_SUCC;
		pUpGunData->up_start_fail_reason = eStart_Fail_NULL_V2;
		fgv_CtrlStartCharge(u8Port);
		if(power_limit_plat[u8Port])		//功率限制
		{
			fgv_CtrlChargeCrt(gum_num, power_limit_plat[u8Port]/100);
		}
		if (ele_limit_plat[u8Port])		//充电电量限制
		{
			SetDetectModeParam(u8Port, eDetectMode_Ele, ele_limit_plat[u8Port] / 10000);
			printf("\r\n recv_Start_Charge_ykc_v2_Succ,ele_limit = %d \r\n", ele_limit_plat[u8Port]);
		}	
	}
	else
	{
		pUpGunData->up_start_ret = UP_RESULT_FAIL;
		fgv_CtrlStopCharge(u8Port);
	}
	
	SetSendEnable(u8Port, YKC_V2_S_Start_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Start_Chg_Ack);
	
	return;
}

uint8_t recv_Stop_Charge_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	uint8_t u8Port = r_data[7] - 1;
	YKC_V2_Recv_Stop_Charge *pRecvStopCharge = NULL;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvStopCharge = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvStopCharge;
	
	memcpy(pRecvStopCharge, r_data, sizeof(YKC_V2_Recv_Stop_Charge));
  
	return TRUE;
}

void recv_Stop_Charge_ykc_v2_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	
	stopPileCharge(u8Port, Reason_Finish_App);
	
	pUpGunData->up_stop_ret = UP_RESULT_SUCC;
	pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL;
	
	SetSendEnable(u8Port, YKC_V2_S_Stop_Chg_Ack, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Stop_Chg_Ack);
	
	return;
}

uint8_t recv_Record_Ack_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
    U8 u8Port = GUN_A;
	YKC_V2_Recv_Record_Ack *pRecvRecordAck = NULL;

    gun[0] = u8Port;
	pRecvRecordAck = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRecordAck;

	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);	

	memcpy(pRecvRecordAck, r_data, sizeof(YKC_V2_Recv_Record_Ack));

    uint8_t result = 1;
    //查看账单应答属于哪个枪
    for (int i = 0; i < GUN_NUM; i++) {
        charge_record_ykcv2 *pRecord = &g_chgData[i].DealRecord.ChgRecord.YkcRecord;
        result = memcmp(pRecord->transaction_log_num, pRecvRecordAck->transaction_log_num, 16);
        if (result == 0) {
            *gun = i;
            // pRecord->send_flag = FALSE;			//0x40回复后置为0，重启不上传离线订单
            updatePileStopReason(i, Pile_Stop_Reason_Finish);
            GNUpChargeStorageDeal(i, (void *)&g_chgData[i].DealRecord, sizeof(PlatDealRecord));		//存储更新的订单数据至flash，可通过日志召唤上报
			
			Power_Limit_Flag[i] = FALSE;	//功率修改复位
			// Max_Power_Flag[u8Port] = FALSE;		//默认最大功率修改复位
			
			power_limit_card[i] = 0;		//卡启动功率限制清零
			power_limit_plat[i] = 0;		//APP启动功率限制清零


			printf("\r\n recv_Record_Ack_ykc_v2,gun=%d,transaction_log_num=", *gun);
			printf("\r\n [ykc]Charge transaction_log_num");
			for (uint8_t i = 0; i < 16; i++)
			{
				printf("%02x ", pRecvRecordAck->transaction_log_num[i]);
			}	



            break;
        }
    }
    if(u8Port >= GUN_NUM) return FALSE;

    return TRUE;
}

void recv_Record_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Record_Ack *pRecvRecordAck = NULL;

	pRecvRecordAck = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRecordAck;

	if(0 == pRecvRecordAck->result)
	{
		SetSendEnable(u8Port, YKC_V2_S_Chg_Record, SEND_ENABLE_OFF);
	}
	
	return;
}

uint8_t recv_Sum_Update_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	uint8_t u8Port = r_data[7] - 1;
	
	YKC_V2_Recv_SumUpdata *pRecvSumUpdata = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvSumUpdata;
	
	memcpy(pRecvSumUpdata, r_data, sizeof(YKC_V2_Recv_SumUpdata));
	
	return TRUE;
}

void recv_Sum_Update_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_SumUpdata *pRecvSumUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvSumUpdata;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	SetSendEnable(u8Port, YKC_V2_S_Sum_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Sum_ACK);
	
    pChgGunData->sum_updata_ret = 0;
    pChgGunData->sum_balance = fourUint8ToUint32(pRecvSumUpdata->account_balance);
	printf("\r\n recv_Sum_Update_ykc_v2_Succ,sum_balance = %d\r\n", pChgGunData->sum_balance);

	return;
}

//设备故障上送回复确认
uint8_t recv_Device_Fault_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);	
	
	uint8_t u8Port = r_data[7] - 1;
	YKC_V2_REcv_Device_Fault *pRecvDeviceFault = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	pRecvDeviceFault = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceFault;
	
	memcpy(pRecvDeviceFault, r_data, sizeof(YKC_V2_REcv_Device_Fault));
	
	return TRUE;
}
 
void recv_Device_Fault_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_REcv_Device_Fault *pRecvDeviceFault = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceFault;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	if (0 == pRecvDeviceFault->rec_mark)	//设备故障上送成功
		printf("\r\n gun %d ,recv_Device_Fault_ykc_v2_Succ \r\n" , u8Port);
	else
		printf("\r\n gun %d ,recv_Device_Fault_ykc_v2_Fail ,illegal failure\r\n" , u8Port);
	
	SetSendEnable(u8Port, YKC_V2_S_Device_Fault, SEND_ENABLE_OFF);

	return;
}

//设备故障复位上送回复确认
uint8_t recv_Device_Reset_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	uint8_t u8Port = r_data[7] - 1;
	YKC_V2_REcv_Device_Reset *pRecvDeviceReset = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvDeviceReset = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceReset;

	memcpy(pRecvDeviceReset, r_data, sizeof(YKC_V2_REcv_Device_Reset));
	
	return TRUE;
}
 
void recv_Device_Reset_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_REcv_Device_Reset *pRecvDeviceReset = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeviceReset;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	
	if ( 0 == pRecvDeviceReset->rec_mark)//设备故障复位上送成功
		printf("\r\n gun %d ,recv_Device_Reset_ykc_v2_Succ \r\n" , u8Port);
	else
		printf("\r\n gun %d ,recv_Device_Reset_ykc_v2_Fail ,illegal failure\r\n" , u8Port);
	
	SetSendEnable(u8Port, YKC_V2_S_Device_Reset, SEND_ENABLE_OFF);

	return;
}

//交易记录召唤
uint8_t recv_Deal_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	uint8_t u8Port = r_data[23] - 1;
	printf("\r\n recv_Deal_ykc_v2, GUN = %d\r\n",u8Port);
	YKC_V2_Recv_Deal *pRecvDeal = NULL;

	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	pRecvDeal = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvDeal;
	
	memcpy(pRecvDeal, r_data, sizeof(YKC_V2_Recv_Deal));
	
	return TRUE;
}
 
void recv_Deal_ykc_v2_Succ(uint8_t u8Port)
{
	Ykc21_Record_Reload(u8Port);

	SetSendEnable(u8Port, YKC_V2_S_Deal_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Deal_ACK);
	
	if (0 == Ykc21_Record_Enable(u8Port))	//发送订单
	{
		printf("\r\n recv_Deal_ykc_v2_Succ ,Send Record!\r\n");
		SetSendEnable(u8Port, YKC_V2_S_Chg_Record, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_V2_S_Chg_Record);
	}

	return;
}

uint8_t recv_Power_Change_Para_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	uint8_t u8Port = r_data[7] - 1;
	if(u8Port >= GUN_NUM) return FALSE;
	gun[0] = u8Port;

	YKC_V2_Recv_Power_Change *pRecvPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPower;
	
	memcpy(pRecvPower, r_data, sizeof(YKC_V2_Recv_Power_Change));
	
	return TRUE;
}

void recv_Power_Change_Para_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Power_Change *pRecvPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPower;

	if (logic_get_gun_charging(u8Port))		//只能在充电中进行功率修改
	{
		uint8_t Limit_time = twoUint8ToUint16(pRecvPower->limit_time)*60;//单位秒

		Power_Limit_Flag[u8Port] = TRUE;
	}
	

	SetSendEnable(u8Port, YKC_V2_S_Power_Change_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Power_Change_ACK);
	
	return;
}

uint8_t recv_TimeSyn_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvTimeSyn;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
	memcpy(pRecvTimeSyn, r_data, sizeof(YKC_V2_Recv_TimeSyn));
	
	return TRUE;
}

void recv_TimeSyn_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvTimeSyn;
	
	Cp56time2a_Set_Time_v2(&pRecvTimeSyn->cur_time);
	
	SetSendEnable(u8Port, YKC_V2_S_TimeSyn_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_TimeSyn_ACK);
	return;
}

uint8_t recv_Set_Rate_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateModel;
	uint8_t i = 0;
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t data_len = 0;
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	memcpy(pRecvRateModel->device_number, &r_data[data_len], DEV_NUM_LEN);
	data_len += DEV_NUM_LEN;

	memcpy(pRecvRateModel->billing_model, &r_data[data_len], 2);
	data_len += 2;

	pRecvRateModel->Rate_quantity = r_data[data_len];
	data_len += 1;

	//计费模型数量
	uint8_t rate_num = (pRecvRateModel->Rate_quantity > 48) ? 48 : pRecvRateModel->Rate_quantity;

	for (uint8_t i = 0; i < rate_num; i++) {

		memcpy(pRecvRateModel->ele_rate[i], &r_data[data_len], 4);
		data_len += 4;
		
		memcpy(pRecvRateModel->ser_rate[i], &r_data[data_len], 4);
		data_len += 4;
	}

	pRecvRateModel->measure_wastage_rates =r_data[data_len];
	data_len += 1;

	for (uint8_t i = 0; i < 48; i++) {		//rate_num改48 时段固定是48个
		pRecvRateModel->segmentation_rate[i] = r_data[data_len];
		(pRecvRateModel->segmentation_rate[i])--;//让第一个费率号对应第0个时段
		// printf("\r\nsegmentation_rate[%d] = %d", i, pRecvRateModel->segmentation_rate[i]);
		data_len += 1;
	}

	return TRUE;
}

void recv_Set_Rate_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Rate_Model *pRecvRateModel = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvRateModel;
	
	SetSendEnable(u8Port, YKC_V2_S_Rate_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Rate_ACK);
	
	Save_rate_model(pRecvRateModel, sizeof(YKC_V2_Recv_Rate_Model));
	return;
}

uint8_t recv_Set_Max_Power_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	uint8_t u8Port = r_data[7] - 1 ;
	if(u8Port >= GUN_NUM) return FALSE;
	gun[0] = u8Port;

	YKC_V2_Recv_Max_Power *pRecvMaxPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvMaxPower;
	
	memcpy(pRecvMaxPower, r_data, sizeof(YKC_V2_Recv_Max_Power));
	
	return TRUE;
}

void recv_Set_Max_Power_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Max_Power *pRecvMaxPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvMaxPower;
	Max_Power[u8Port] = pRecvMaxPower->default_max_power[0];
	printf("\r\n recv_Set_Max_Power_ykc_v2_Succ ,Max_Power[%d] = %d\r\n", u8Port,Max_Power[u8Port]);
	
	Max_Power_Flag[u8Port] = TRUE;

	SetSendEnable(u8Port, YKC_V2_S_Max_Power_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Max_Power_ACK);
	
	return;
}

uint8_t recv_set_reboot_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Reboot *pRecvReboot = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvReboot;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	memcpy(pRecvReboot, r_data, sizeof(YKC_V2_Recv_Reboot));
	
	return TRUE;
}

void recv_set_reboot_ykc_v2_Succ(uint8_t u8Port)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	pProtocolDCB->PlatTask.reboot_flag = E_Reboot_Idle;
	pProtocolDCB->PlatTask.reboot_tick = Get_Systick();
	
	SetSendEnable(u8Port, YKC_V2_S_reboot_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_reboot_ACK);
	
	return;
}

uint8_t recv_update_ftp_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Update_ftp *pRecvUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvUpdata;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	memcpy(pRecvUpdata, r_data, sizeof(YKC_V2_Recv_Update_ftp));
	
	return TRUE;
}

void recv_update_ftp_ykc_v2_Succ(uint8_t u8Port)
{
    up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
    YKC_V2_Recv_Update_ftp *pRecvFtp = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvUpdata;

	SetSendEnable(u8Port, YKC_V2_S_update_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_update_ACK);

    uint16_t rate = 0;
    rate = pRecvFtp->device_rate[1] << 8 | pRecvFtp->device_rate[0];
    //设备型号以及设备功率异常不升级,应答失败
    if ((pRecvFtp->device_type != 2) || (rate != (7 * GUN_NUM))) {
        // 应答失败且不升级
        // ptcp_data->up_update_ret = 2;
        // pRecvFtp->update_ctrl = 0;
        // return;
    }

    ptcp_data->up_update_ret = 0;
    g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;				//升级
    g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick();		//超时时间

    char p_ip[16+1] = {0};
    char p_username[16+1] = {0};
    char p_update_password[16+1] = {0};
	memcpy(p_ip, pRecvFtp->update_ip, sizeof(pRecvFtp->update_ip));
	memcpy(p_username, pRecvFtp->update_username, sizeof(pRecvFtp->update_username));
	memcpy(p_update_password, pRecvFtp->update_password, sizeof(pRecvFtp->update_password));
	printf("\r\n ip:%s\r\n username:%s\r\n password:%s\r\n",p_ip,p_username,p_update_password);

    uint16_t u16Port = pRecvFtp->update_port[1] << 8 | pRecvFtp->update_port[0];
    g_PileUpdateInterface((char *)p_ip, u16Port, (char *)p_username, 
                          (char *)p_update_password, (char *)pRecvFtp->update_file_path, (char *)pRecvFtp->update_file_name);
	
	return;
}

uint8_t recv_QR_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);

	uint8_t u8Port = r_data[7] - 1;
	YKC_V2_Recv_QR *pRecvQR = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvQR;

	printf("recv_QR_ykc_v2, u8Port = %d\n", u8Port);
	uint16_t copy_size = r_data[9] + 10;//二维码长度
	if (copy_size > sizeof(YKC_V2_Recv_QR)) {
		printf("Error: Copy size %u > struct size %u\n", copy_size, sizeof(YKC_V2_Recv_QR));
		return FALSE;
	}
	memcpy(pRecvQR, r_data, copy_size);

	if(u8Port > GUN_NUM) return FALSE;
	
	gun[0] = u8Port;
	
    storage_PlatQRCodeInfoStr(u8Port+1, (char*)pRecvQR->QR_data);		//更新哪把枪传哪吧枪的二维码信息


	return TRUE;
}

void recv_QR_ykc_v2_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, YKC_V2_S_QR_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_QR_ACK);
	
	return;
}

uint8_t recv_Set_Para_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Para *pRecvPara = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPara;
	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	memcpy(pRecvPara, r_data, sizeof(YKC_V2_Recv_Para));
	
	return TRUE;
}

void recv_Set_Para_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Para *pRecvPara = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPara;
	
	SetSendEnable(u8Port, YKC_V2_S_Para_ACK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, YKC_V2_S_Para_ACK);
	
	return;
}

uint8_t recv_Key_Update_ykc_v2(uint8_t *r_data, int len, uint8_t* gun)
{
	uint8_t u8Port = GUN_A;
	YKC_V2_Recv_Key_Update *pRecvKeyUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvKeyUpdate;
	// PlatCfgInfo *pst_cfgInfo = fgv_GetPlatCfgInfo();
	YKC21_FlashPlatInfo *pYkcFlashInfo = &s_YkcFlashInfo;      

	
	if(u8Port >= GUN_NUM) return FALSE;
	
	gun[0] = u8Port;

	uint16_t r_data_len = len-16;				//加密数据长度	

    Ykc_Recv_Data_Decrypt(r_data,r_data_len);
	
	memcpy(pRecvKeyUpdata, r_data, sizeof(YKC_V2_Recv_Key_Update));

	YKC21_WriteRsaKey((char*)pRecvKeyUpdata->new_key);	
	
	return TRUE;
}

void recv_Key_Update_ykc_v2_Succ(uint8_t u8Port)
{
	YKC_V2_Recv_Key_Update *pRecvKeyUpdata = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvKeyUpdate;
	
	SetSendEnable(u8Port, YKC_V2_S_Key_Update_Ack, SEND_ENABLE_ON);

	Send_Immediately(u8Port, YKC_V2_S_Key_Update_Ack);
	
	if (1 == pRecvKeyUpdata->exe_control)
	{
		SetSendEnable(u8Port, YKC_V2_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_V2_S_Identification);
	}

	return;
}


void SetSendSrmSet(uint8_t u8Port, uint32_t cmd, uint16_t Srm)
{
	if (cmd == YKC_V2_R_Chg_Record ) 
		SetSendSrm(u8Port, YKC_V2_S_Chg_Record, Srm);    
	else if (cmd == YKC_V2_R_Device_Fault_ACK)
		SetSendSrm(u8Port, YKC_V2_S_Device_Fault, Srm);    
	else if (cmd == YKC_V2_R_Device_Reset_ACK)
		SetSendSrm(u8Port, cmd + 1, Srm);    
	else
		SetSendSrm(u8Port, cmd - 1, Srm);    //此处-1是因为命令字不一样原因
}

//===================================================================
uint8_t UpCtrlRecvTimer_V2(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);
	
	if((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;
	
	if(YKC_R_Heart == cmd)
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
}YKC_V2_Recv_ctrl;

const YKC_V2_Recv_ctrl StrYKCRecvCtrl_v2[]={
    {YKC_V2_R_Identification	,eTick_30S		,UpCtrlRecvTimer_V2		,recv_login_data_ykc_v2			,recv_login_ykc_v2_Succ				},//
    {YKC_V2_R_Heart				,eTick_60S		,UpCtrlRecvTimer_V2		,recv_Heart_ykc_v2				,recv_heart_ykc_v2_Succ				},//
    {YKC_V2_R_Rate_Proving		,eTick_20S		,UpCtrlRecvTimer_V2		,recv_Rate_Proving_ykc_v2		,recv_Rate_Proving_ykc_v2_Succ		},//
    {YKC_V2_R_Rate_Ask			,eTick_30S		,UpCtrlRecvTimer_V2		,recv_Rate_Ask_ykc_v2			,recv_Rate_Ask_ykc_v2_Succ			},//
	
    {YKC_V2_R_RealData			,0xffffffff		,UpCtrlRecvTimer_V2		,recv_RealData_ykc_v2			,recv_RealData_ykc_v2_Succ				},//
    {YKC_V2_R_Auth				,eTick_60S		,UpCtrlRecvTimer_V2		,recv_Auth_Ack_ykc_v2			,recv_Auth_ykc_v2_Succ				},//
	
	{YKC_V2_R_Start_Chg 		,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Start_Charge_ykc_v2		,recv_Start_Charge_ykc_v2_Succ 		},
	
    {YKC_V2_R_Stop_Chg			,0xffffffff		,UpCtrlRecvTimer_V2		,recv_Stop_Charge_ykc_v2		,recv_Stop_Charge_ykc_v2_Succ		},//
	{YKC_V2_R_Chg_Record		,eTick_30S 		,UpCtrlRecvTimer_V2		,recv_Record_Ack_ykc_v2			,recv_Record_ykc_v2_Succ			},
	
    {YKC_V2_R_Sum_Update		,0xffffffff		,UpCtrlRecvTimer_V2		,recv_Sum_Update_ykc_v2			,recv_Sum_Update_ykc_v2_Succ		},//
    {YKC_V2_R_Device_Fault_ACK	,0xffffffff		,UpCtrlRecvTimer_V2		,recv_Device_Fault_ykc_v2		,recv_Device_Fault_ykc_v2_Succ		},//
    {YKC_V2_R_Device_Reset_ACK	,0xffffffff		,UpCtrlRecvTimer_V2		,recv_Device_Reset_ykc_v2		,recv_Device_Reset_ykc_v2_Succ		},//
    {YKC_V2_R_Deal				,0xffffffff		,UpCtrlRecvTimer_V2		,recv_Deal_ykc_v2				,recv_Deal_ykc_v2_Succ				},//
	{YKC_V2_R_Power_Change_Para	,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Power_Change_Para_ykc_v2	,recv_Power_Change_Para_ykc_v2_Succ		},
	{YKC_V2_R_Max_Power			,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Set_Max_Power_ykc_v2		,recv_Set_Max_Power_ykc_v2_Succ		},
    {YKC_V2_R_TimeSyn			,0xffffffff		,UpCtrlRecvTimer_V2		,recv_TimeSyn_ykc_v2			,recv_TimeSyn_ykc_v2_Succ			},//
	{YKC_V2_R_Set_Rate			,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Set_Rate_ykc_v2			,recv_Set_Rate_ykc_v2_Succ			},
    {YKC_V2_R_set_reboot		,0xffffffff		,UpCtrlRecvTimer_V2		,recv_set_reboot_ykc_v2			,recv_set_reboot_ykc_v2_Succ		},//
	{YKC_V2_R_set_update_ftp	,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_update_ftp_ykc_v2			,recv_update_ftp_ykc_v2_Succ		},
	{YKC_V2_R_Ret_QR			,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_QR_ykc_v2					,recv_QR_ykc_v2_Succ				},
	{YKC_V2_R_Para				,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Set_Para_ykc_v2			,recv_Set_Para_ykc_v2_Succ			},
	{YKC_V2_R_Key_Update		,0xffffffff 	,UpCtrlRecvTimer_V2		,recv_Key_Update_ykc_v2			,recv_Key_Update_ykc_v2_Succ		}
};

void YKCUpCtrlRecvDeal_V2(YKC_V2_HEAD_T *pHead, uint32_t cmd, void *pindata, uint16_t inlen)
{
	const YKC_V2_Recv_ctrl *pYKCRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;
	
	for (u32i = 0; u32i < ARRAY_SIZE(StrYKCRecvCtrl_v2); u32i++)
    {
		pYKCRecvCtrl = &StrYKCRecvCtrl_v2[u32i];
		
		if (cmd == pYKCRecvCtrl->cmd)
		{
			if(TRUE == pYKCRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pYKCRecvCtrl->pRecvSucc(port);
				
				SetSendSrmSet(port, cmd, twoUint8ToUint16(pHead->ser));			
				
				SetRecvTick(port, cmd, Get_Systick());
				
				UPRINT("\r\nYKCv2.1_UpProtocol --> GUN: %d, RecvDealcmd: 0x%02x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}

uint16_t swap_bytes(uint16_t value) { return (value >> 8) | (value << 8); }

//判断tcp接收到的所有数据是否合法
static int ykc_Tcp_Read_Data_Check_V2(uint8_t *r_data)
{
    if (r_data[0] != 0x68) {
        printf("Check head erro  0x%x\r\n", r_data[0]);
        return -1;
    }
    //检查校验，读取所有数据长度
    // uint16_t r_len = r_data[2];
	uint16_t r_len = swap_bytes(twoUint8ToUint16(&r_data[1]));
    uint16_t crc_len = r_len + 3;

    //对校验位之前的数据进行CRC校验
    uint16_t c_crc = CRC16(&r_data[3], r_len);

    uint16_t r_crc = 0;
	r_crc = r_data[crc_len] << 8 | r_data[crc_len + 1];
	
    // memcpy(&r_crc, &r_data[crc_len], 2);

    if (c_crc !=  r_crc) {
        printf("\r\nCheck crc erro  0x%04x  0x%04x\r\n", r_crc, c_crc);
        return -2;
    }
    return 0;
}

void ykcfrom_buffer_data_V2(U8 *recv_buf, U16 *len)
{
    //从buffer里查找合法数据进行校验
    U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, len, TCP_DATA_LEN_MAX);
    
    if (recv_buf[0] == 0x68) {
        //继续寻找len
		read_len = *len;
        if (read_len > TCP_DATA_LEN_MAX) {
			printf("\r\nykc2.1protocol--> recv buf full ! ");
            return;
        }
        *len = read_len;

    }
}


void ykcPackConnectHandle_V2(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	YKC_V2_HEAD_T *pHead = NULL;

	while(surplusLen) {
		pHead = (YKC_V2_HEAD_T*)(recv_buf + currentIndex);

		int packLen = swap_bytes(twoUint8ToUint16(&pHead->len[0]))+5;
		//防止乱数据导致程序死掉
		if (packLen > surplusLen) {
			printf("\r\n----------packLen>surplusLen----------,surplusLen = %d\r\n",surplusLen);
			return;
		}
		surplusLen = surplusLen - packLen;

		// printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);
		
		if (ykc_Tcp_Read_Data_Check_V2(recv_buf + currentIndex) < 0) {
			printf("Check_data error\r\n");
			return;
		}

		hex_dump("\r\nykc2.1_tcp_recv_data", recv_buf + currentIndex, packLen);

	    YKCUpCtrlRecvDeal_V2(pHead, pHead->cmd, recv_buf+currentIndex+sizeof(YKC_V2_HEAD_T), packLen);
		
		currentIndex = currentIndex + packLen;
		}
}

// uint8_t from_tcp_data[TCP_DATA_LEN_MAX];

void YKCUpRecvDeal_V2(void)
{
    uint8_t from_tcp_data[TCP_DATA_LEN_MAX];
    U16 r_len = 0;
	YKC_V2_HEAD_T *pHead = NULL;

    // PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, from_tcp_data, (U16 *)&r_len, TCP_DATA_LEN_MAX);

    ykcfrom_buffer_data_V2(from_tcp_data, &r_len);
    if (r_len == 0)
        return;
    if (r_len > TCP_DATA_LEN_MAX) {
        printf("\r\nykc2.1protocol--> recv buf full ! ");
        return;
    }
	//粘包处理
	ykcPackConnectHandle_V2(from_tcp_data, r_len);

	return;
}

void YKCRecvOutTimeDeal_V2(uint8_t u8Port, uint32_t cmd)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if(YKC_V2_R_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}
	
	if(YKC_V2_R_Auth == cmd)
	{
		SetSendEnable(u8Port, YKC_V2_S_Auth, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, YKC_V2_R_Auth, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
//		pChgGunData->Card_err = eCardErr_NULL;
	}
	
	return;
}

void YKCUpCtrlRecvOutTime_V2(void)
{
	const YKC_V2_Recv_ctrl *pYKCRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;
	
	for(i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrYKCRecvCtrl_v2); u32i++)
	    {
			pYKCRecvCtrl = &StrYKCRecvCtrl_v2[u32i];
			
			if(RECV_ENABLE_ON != GetRecvEnable(i, pYKCRecvCtrl->cmd))
				continue;
			
			if (TRUE == pYKCRecvCtrl->pRecvTimer(i, pYKCRecvCtrl->cmd, pYKCRecvCtrl->timer))
			{
				YKCRecvOutTimeDeal_V2(i, pYKCRecvCtrl->cmd);
			}
		}
	}
	return;
}
//===========================================================================
//===========================================================================

void YKC_CardAuthStart_Cmd_V2(uint8_t u8Port)
{
	//插枪状态下刷有效卡，进行充电鉴权
    if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_V2_S_Auth)) {
        return;
    }
    SetSendEnable(u8Port, YKC_V2_S_Auth, SEND_ENABLE_ON);

    Send_Immediately(u8Port, YKC_V2_S_Auth);
}

void YKC_DealUpdate_Cmd_V2(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    if(eOnline_Off == Get_PlatConnectSta())
	{
		printf("YKC_DealUpdate_Cmd_V2:offline\r\n");
		return;
	}	
	printf("-----YKC_DealUpdate_Cmd_V2,YKC_V2_S_Chg_Record_Send-----\r\n");

    SetSendEnable(u8Port, YKC_V2_S_Chg_Record, SEND_ENABLE_ON);
    Send_Immediately(u8Port, YKC_V2_S_Chg_Record);

}

void YKCUpLogin_V2(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
    if (!Comm_getIpSuces(eSocket_GPRS1)) {
		return;
	}
    
	if(eOnline_Off == Get_PlatConnectSta())
	{
        Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, YKC_V2_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, YKC_V2_S_Identification);
	}
}

uint8_t YKCUpChargeRecordUpDeal_V2(void)
{
    return FALSE;
}

void YKCUpGunStateCheck_V2(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	static uint8_t gun_state[GUN_NUM_MAX] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX] = {0};
	uint8_t report_flag[GUN_NUM_MAX] = {0};
	
	if (logic_get_gun_charging(u8Port)) {
		pUpGunData->up_gun_state = eUP_Gun_State_Work;
	} else {
		if(TRUE == dev_getErrState(u8Port)) {
			pUpGunData->up_gun_state = eUP_Gun_State_Err;
        } else {
			pUpGunData->up_gun_state = eUP_Gun_State_Idle;
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
		Send_Immediately(u8Port, YKC_V2_S_RealData);
	}
	
	return;
}


// 故障上报处理
static void Ykcv21UpError(uint8_t u8Port)
{
	// uint8_t u8Port;
	uint8_t SendFaultFlag = FALSE;		// 故障发生发送标记
	uint8_t SendResetFlag = FALSE;		// 故障复位发送标记
	static uint8_t ErrSts[GUN_NUM_MAX][ARRAY_SIZE(error_map)] = {0};
	uint8_t i;						// source error index
	static uint16_t flow = 0;
		
	if(TRUE != UpOnlineFlag())
		return;

	// one by one	
	for(u8Port=0; u8Port < GUN_NUM; u8Port++)
	{
		if(SEND_ENABLE_ON == GetSendEnable(u8Port, YKC_V2_S_Device_Fault))
			return;
	}

	for(u8Port=0; u8Port < GUN_NUM; u8Port++)
	{
		for(i=0; i<ARRAY_SIZE(error_map); i++)
		{
			if(ErrSts[u8Port][i] != dev_getErrExsit(u8Port, error_map[i].err_type)) //故障状态改变
			{
				ErrSts[u8Port][i] = dev_getErrExsit(u8Port, error_map[i].err_type);
				YKC_V2_Send_ErrSend *ykc21ErrorSwap = &ErrSendPlatform[u8Port];
				if(ErrSts[u8Port][i] == TRUE)		// error happen
				{	
					Time_to_Cp56time2a_v2((cp56time2a_v2*)ykc21ErrorSwap->StartTime);
					memset(&ykc21ErrorSwap->StopTime, 0, 7);
					ykc21ErrorSwap->status = 1;
					SendFaultFlag = TRUE;
					ykc21ErrorSwap->flow = flow;
					flow ++;

					ykc21ErrorSwap->WarnType = get_hard_err_type(u8Port ,error_map[i].err_type);
					ykc21ErrorSwap->WarnId = get_hard_err_code(u8Port ,error_map[i].err_type);
					printf("\r\n GUN = %d ,WarnType = %d , WarnId = %d \r\n",u8Port,ErrSendPlatform[u8Port].WarnType ,ErrSendPlatform[u8Port].WarnId);
					break;		// up one error every time
				}
				else			// error cancel
				{					
					Time_to_Cp56time2a_v2((cp56time2a_v2*)ykc21ErrorSwap->StopTime);
					printf("\r\n GUN = %d , error_cancel , err_type = %d \r\n",u8Port,error_map[i].err_type);
					ykc21ErrorSwap->status = 0;					
					SendResetFlag = TRUE;
					break;				
				}
			}
		}	
		if(SendFaultFlag == TRUE ||  SendResetFlag == TRUE)
			break;		// up one error every time
	}

	if(SendFaultFlag == TRUE)//故障产生发送
	{
		SendFaultFlag = FALSE;
		SetSendEnable(u8Port, YKC_V2_S_Device_Fault, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_V2_S_Device_Fault);
	}
	
	if(SendResetFlag == TRUE)//故障清除发送
	{
		SendResetFlag = FALSE;
		SetSendEnable(u8Port, YKC_V2_S_Device_Reset, SEND_ENABLE_ON);
		Send_Immediately(u8Port, YKC_V2_S_Device_Reset);
	}
}

static uint32_t Last_time[GUN_NUM_MAX] = {0};
//功率控制处理
static void Ykc21PowerLimit(uint8_t u8Port)
{
	if (u8Port >= GUN_NUM) return;

	YKC_V2_Recv_Power_Change *pRecvPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvPower;
	
	uint16_t Limit_time = twoUint8ToUint16(pRecvPower->limit_time)*60;//单位秒
	cp56time2a_v2 Current_time[GUN_NUM_MAX];
	uint8_t gun_num = pRecvPower->gun - 1;
	uint32_t Cur_Stamp;


	if (u8Port != gun_num || 0 == Limit_time) 	//枪号不匹配或者无功率控制时间
		return;

	if (TRUE == Power_Limit_Flag[u8Port])		//功率控制使能
	{
		Time_to_Cp56time2a_v2((cp56time2a_v2*)&Current_time[u8Port]);		//将当前时间转化为cp56time2a_v2格式
		Cur_Stamp = Cp56time2a_To_Time_v2(&Current_time[u8Port]);			//cp56time2a_v2格式转化为时间戳 单位秒
		if ( 0 == Last_time[u8Port])	//开始功率调节时更新一次时间	
		{
			fgv_CtrlChargeCrt(gun_num, twoUint8ToUint16(pRecvPower->power_max)*100);		//开始功率控制
			Last_time[u8Port] = Cur_Stamp;
			printf("\r\n Start_Power_Limit, gun:%d,Power_Limit_Flag = %d,power = %d,Limit_time = %d\n",
					u8Port,Power_Limit_Flag[u8Port],twoUint8ToUint16(pRecvPower->power_max)*100,Limit_time);
			printf("\r\n Start_Power_Limit, current_time[%d] = %d, last_time[%d] = %d\r\n",u8Port,Cur_Stamp,u8Port,Last_time[u8Port]);
			printf("\r\n Ykc21PowerLimit,gun:%d,OutVol = %d,OutCur = %d\r\n", u8Port, GetPile_ChgOutVol(u8Port, 1), GetPile_ChgOutCur(u8Port, 2)/10);
		}	
		else if (Cur_Stamp - Last_time[u8Port] >= Limit_time)		//达到功率调节时间
		{
			fgv_CtrlChargeCrt(gun_num, 700);		//恢复默认7KW
			Power_Limit_Flag[u8Port] = FALSE;		//功率控制使能复位
			Last_time[u8Port] = 0;
			printf("\r\n Stop_Power_Limit,gun:%d,Power_Limit_Flag = %d,current_time[%d] = %d, last_time[%d] = %d\r\n",
					u8Port,Power_Limit_Flag[u8Port],u8Port,Cur_Stamp,u8Port,Last_time[u8Port]);
		}
	}
}


//默认最大功率控制处理
static void Ykc21MaxPowerLimit(uint8_t u8Port)
{
	if (u8Port >= GUN_NUM) return;
	
	YKC_V2_Recv_Max_Power *pRecvMaxPower = &g_ProtocolDCB.pYKCRecvData_v2[u8Port].strRecvMaxPower;
	// uint8_t gun_num = pRecvMaxPower->gun - 1;
	cp56time2a_v2 Current_time[GUN_NUM_MAX] = {0};
	// Max_Power[u8Port] = twoUint8ToUint16(pRecvMaxPower->default_max_power)*100;

	if (0 == Max_Power[u8Port])
		return;

	if (TRUE == Max_Power_Flag[u8Port])		//最大功率控制使能
	{
		Time_to_Cp56time2a_v2((cp56time2a_v2*)&Current_time[u8Port]);			//获取cp56time2a_v2格式的当前时间
		uint32_t Cur_Stamp = Cp56time2a_To_Time_v2(&Current_time[u8Port]);		//cp56time2a_v2格式转化为时间戳 单位秒
		uint32_t Power_Start = Cp56time2a_To_Time_v2(&pRecvMaxPower->start_time);		
		uint32_t Power_Stop = Cp56time2a_To_Time_v2(&pRecvMaxPower->stop_time);		

		if (Power_Limit_Flag[u8Port] || power_limit_card[u8Port] || power_limit_plat[u8Port])		//如果正在进行功率控制则默认最大功率不生效
		{
			// printf("\r\n Default_Max_Power_Not_Work,gun=%d,Power_Limit_Flag=%d,power_limit_card=%d,power_limit_plat=%d \n", 
			// 		u8Port,Power_Limit_Flag[u8Port] , power_limit_card[u8Port] , power_limit_plat[u8Port]);			
			return;
		}
		if ( !first_trigger[u8Port] && (Cur_Stamp >= Power_Start))	//开始最大功率调节
		{	
			fgv_CtrlChargeCrt(u8Port, Max_Power[u8Port]*100);
			printf("\r\n Start_Max_Power_Limit,Current_time = %d,Power_Start = %d,Power_Stop = %d,Max_Power = %d \n",Cur_Stamp,Power_Start,Power_Stop,Max_Power[u8Port]);
			printf("\r\n Ykc21MaxPowerLimit,gun:%d,OutVol = %d,OutCur = %d\r\n", u8Port, GetPile_ChgOutVol(u8Port, 1), GetPile_ChgOutCur(u8Port, 2)/10);
			first_trigger[u8Port] = true;
		}
		else if (Cur_Stamp >= Power_Stop)	//达到最大功率调节时间
		{
			fgv_CtrlChargeCrt(u8Port, 700);		//恢复默认7KW
			Max_Power[u8Port] = 0;		//最大默认功率清零
			Max_Power_Flag[u8Port] = FALSE;		//最大功率控制使能复位
			first_trigger[u8Port] = FALSE;
			printf("\r\n Stop_Max_Power_Limit,gun=%d,Current_time = %d,Power_Start = %d,Power_Stop = %d,Max_Power = %d ,Max_Power_Flag = %d\n", 
					u8Port,Cur_Stamp, Power_Start, Power_Stop, Max_Power[u8Port],Max_Power_Flag[u8Port]);
		}
	}
}

void YKCUpCtrlTaskDeal_V2(void)
{
	uint8_t i = 0;
	
	YKCUpLogin_V2();
	
	for (i = 0; i < GUN_NUM; i++)
	{
		YKCUpGunStateCheck_V2(i);
		Ykcv21UpError(i);			//故障上报处理
		Ykc21PowerLimit(i);			
		Ykc21MaxPowerLimit(i);		
	}
	
	YKCUpChargeRecordUpDeal_V2();
	
	return;
}

void YKCUpProtocolDeal_V2(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
	
	if(NULL == pProtocolDCB->pYKCRecvData_v2)
		return;
	
	YKCUpCtrlTaskDeal_V2();	//任务状态处理
	YKCUpRecvDeal_V2();		//接收处理
	YKCUpSendDeal_V2();		//发送处理
	YKCUpCtrlRecvOutTime_V2();	//超时处理
	
	return;
}


