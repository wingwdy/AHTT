/***********************************************************************************
 * 文 件 名  : iot_HaiNCT_Protocol.c
 * 版 本 号  : V0.1
 * 负 责 人  : JI JUN
 * 创建日期  : 2025-02-20
 * 文件描述  : 海宁城投平台
 * 版权说明  : 
 * 函数列表  :    
 * 其    他  :
 * 修改日志  :
***********************************************************************************/
#include "iot_HNCT_Protocol_Code.h"
#include "iot_GN_Protocol_Code.h"
#include "protocol_data.h"
#include "mbsMaster.h"
#include "maths.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "cost.h"
#include "AppDealFlash.h"
#include "AppCfg.h"


#define GUN_NUM_MAX_HaiNCT   GUN_NUM_MAX

#define HaiNCT_PROTOCOL_FRAME   22  //不包含启动字符和长度域
#define HaiNCT_FAC_NUM   0x9501   //厂家编码,BCD码
#define HaiNCT_BNS_TYPE   1     //业务类型
#define HaiNCT_MEASURE_TYPE   2     //计量类型，2充电量，3放电量

#define HaiNCT_PROTOCOL_HEAD   0x68
//上报类型标识
enum {
    eHaiNCT_type_103 = 103,     //时钟
    eHaiNCT_type_130 = 130,
    eHaiNCT_type_133 = 133, 
    eHaiNCT_type_134 = 134,     //数据上报
};
enum {
    eHaiNCT_U_START = 0x07,
    eHaiNCT_U_STARTACK = 0x0B,
    eHaiNCT_U_STOP = 0x13,
    eHaiNCT_U_STOPACK = 0x23,
    eHaiNCT_U_TEST = 0x43,
    eHaiNCT_U_TESTACK = 0x83,
};

static HaiNCT_UpPlatInfo s_HnctUpInfo[GUN_NUM_MAX];
static HaiNCT_FlashPlatInfo s_HnctFlashInfo[GUN_NUM_MAX];

/*************************费率结构体*****费率变量*************************************/
//开机读取费率更新；收到新费率报文更新
HnctFeeModel_Save_truct HaiNCT_feeModel_save={0};

/******协议外相关函数******************************************************************************************/
/********************************************************************
 * @brief 	   anpei协议获取桩编号(32位)，截断成16位（8个字节）
 * @param[in]	 
 * @return 	   pNum  8个字节桩编号的BCD
 *********************************************************************/
void Get_PlatNumberBCD(uint8_t *pNum)
{
	char arry[PLAT_NUMBER_LEN]={0};
   Get_PlatNumberString(arry);   //例如 3100000000010019000000000000000
   
   for(uint8_t i=0;i<8;i++)
   {
        pNum[i]=(arry[2*i]-0x30)*16+arry[2*i+1]-0x30; //0x31 0x00 ..... 0x19

   }
}

//连接平台桩号转换
uint8_t HaiNCT_Get_PlatNumberBCD(uint8_t *pNum)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
    uint8_t tempNum[8] = {0};

    int fixedLength = 8; //sizeof(fixedArray);
	
    String2bin(tempNum, pst_cfgInfo->pltDeviceNumber, fixedLength);

	BINToBCD(tempNum, tempNum, fixedLength);

    //颠倒顺序,tempNum转pNum里面
    for(uint8_t i = 0; i < fixedLength; i++) {
        pNum[i] = tempNum[fixedLength - 1 - i];
    }

	return TRUE;
}

static bool Compare_Array(uint8_t *number1, uint8_t *number2, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		if (number1[i] != number2[i])
			return FALSE;
	}
	return TRUE;
}


uint8_t Compare_HnctPile(uint8_t *number) {
    uint8_t plieNumber[8] = {0};
    HaiNCT_Get_PlatNumberBCD(plieNumber);
    // 判断跟桩编号是否一致
     return Compare_Array(plieNumber, number, 8);
}

uint8_t HaiNct_getTradeFlag(uint8_t u8Port)
{
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	return pUpGunData->TradeFlag;
}


/********************************************************************
 * @brief 	   anpei获取当前费率模型号
 * @param[in]	 
 * @return 	   pNum  8个字节桩编号的BCD
 *********************************************************************/
void HaiNCTCurrentRateNum(uint8_t *billNum)
{
    memcpy(billNum, HaiNCT_feeModel_save.FeemodelA8save_data[B47_A].billing_model, 8);
}

static FeeModelA8* HaiNCT_Get_BillID()
{
    return &HaiNCT_feeModel_save.FeemodelA8save_data[B47_A];
}
/******实时数据处理******************************************************************************************/
/*
 * void Refresh_chrg_EE_Money_data(...)
 * void Refresh_chrg_EE_Money_piecewise_data (...)
 * void Clear_chrg_EE_Money_data(...)
*/


//发送数据组包头
#define HaiNCT_HeadLen 17   //不包含头和长度
static void HaiNCT_PackHeadData(uint8_t *headData, uint8_t u8Port, uint16_t len, uint8_t TypeIDE, uint8_t Cot,uint8_t recordKind)
{
    HNCT_HEAD_T *pHead = (HNCT_HEAD_T *)headData;
    pHead->head = HaiNCT_PROTOCOL_HEAD;
    uint16_t u16Len = len - 3;
    memcpy(pHead->len, &u16Len, 2);
    // pHead->len[0] = len - 3;
    // pHead->len[1] = 0;      //标准104协议没有这个位置
    memset(pHead->control, 0, 4);
    pHead->TypeIDE = TypeIDE;
    pHead->Vsq = 0;
    pHead->Cot[0] = Cot;
    pHead->Cot[1] = 0;
    memset(pHead->AppSerAddr, 0, 2);
    pHead->InfAddr[0] = (u8Port << 4) & 0x0F;
    pHead->InfAddr[1] = 0;
    pHead->InfAddr[2] = 0;
    pHead->recordKind = recordKind;
}
static uint8_t HaiNCT_PackUFrameData(uint8_t *data, uint8_t uType)
{
	uint16_t i = 0;
	data[i++] = HaiNCT_PROTOCOL_HEAD;
	data[i++] = 0x04;
	data[i++] = 0x00;   //标准104协议没有这个位置
	data[i++] = uType;
	data[i++] = 0x00;
	data[i++] = 0x00;
	data[i++] = 0x00;
    return i;
}
/**************************************************************************************************************************
 * 这部分属于订单数据更新
 * hnct_packStopReasonChgRecord()   停止原因更新
 * printf_HaiNCT_CalSerPerMoney()   订单信息打印
 * HaiNCT_CalSerPerMoney()      订单金额计算，每个平台计算方式不一致
 * HaiNCT_RealDataUpdate()      订单数据更新
 * HaiNCT_packChgRecord()       一分钟存储一次，更新停止原因，外部调用
 * HaiNCT_packChgRecord_Scan()  实时上报数据前更新一次总数据，避免数据滞后
 * HaiNCT_MoneyUpdate_Scan()    金额更新，屏幕需要实时显示，所以需要实时更新这部分
************************************************************************************************************************ */

typedef struct
{
    uint32_t		gnReason;
	uint32_t		PlatReason;
}Hnct_StopReasonMap;

// 安徽铁塔故障对应表
const Hnct_StopReasonMap StrHNCTStopReasonMap[] = {
	{Pile_Stop_Reason_Card	        ,E_HNCT_StopReason_Card          },
	{Pile_Stop_Reason_APP	        ,E_HNCT_StopReason_App          },
	{Pile_Stop_Reason_CarOk	        ,E_HNCT_StopReason_Full          },
	{Pile_Stop_Reason_LittleCrt	    ,E_HNCT_StopReason_Full          },
	{Pile_Stop_Reason_OverSum	    ,E_HNCT_StopReason_MnyOver          },
	{Pile_Stop_Reason_EStop	        ,E_HNCT_StopReason_Estop          },
};

//订单数据更新
static void hnct_packStopReasonChgRecord(uint8_t u8Port, RecordA3 *pRecord)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    U8 pileReason = pChgGunData->DealRecord.PileStopReason;
	const Hnct_StopReasonMap *pHNCTStopMap = NULL;

    for (uint32_t u32i = 0; u32i < ARRAY_SIZE(StrHNCTStopReasonMap); u32i++) {
        pHNCTStopMap = &StrHNCTStopReasonMap[u32i];
        if (pileReason == pHNCTStopMap->gnReason) {
            pRecord->stopReason[1] = 0x0;  
            pRecord->stopReason[0] = pHNCTStopMap->PlatReason;  
            printf("hnct_stop reason: 0x%x %d\r\n", pHNCTStopMap->gnReason, pHNCTStopMap->PlatReason);
            return;
        }
    }
    printf("Else OgrReason: %d\r\n", pileReason);
    if (pileReason == Pile_Stop_Reason_PwOff) {
        pRecord->stopReason[1] = 2;     //断电补传
        pRecord->stopReason[0] = 0;  
    } else {
        pRecord->stopReason[1] = 0xFF;  
        pRecord->stopReason[0] = pileReason;  
    }
}
static void printf_HaiNCT_CalSerPerMoney(uint8_t u8Port, RecordA3 *pRecord)
{
    uint32_t value_U321 = 0;
    uint32_t value_U322 = 0;
    uint32_t value_U323 = 0;
    //尖峰平谷,起止值
    for(uint8_t i = 0; i < 4; i++)
    {
        value_U321 = 0;
        memcpy(&value_U321, pRecord->EleValue[i].startEle, 4);
        value_U322 = 0;
        memcpy(&value_U322, pRecord->EleValue[i].stopEle, 4);
        printf("start-stop:%d--%d\r\n", value_U321, value_U322);
    }
    //尖峰平谷,单价电量金额
    for(uint8_t i = 0; i < 4; i++)
    {
        value_U321 = 0;
        memcpy(&value_U321, pRecord->infoValue[i].unitPrice, 4);
        value_U322 = 0;
        memcpy(&value_U322, pRecord->infoValue[i].perEle, 4);
        value_U323 = 0;
        memcpy(&value_U323, pRecord->infoValue[i].perMny, 4);
        printf("jfpg:%d %d %d\r\n", value_U321, value_U322, value_U323);
    }
    value_U321 = 0;
    memcpy(&value_U321, &pRecord->EnergyEle[0][0], 4);
    value_U322 = 0;
    memcpy(&value_U322, &pRecord->EnergyEle[1][0], 4);
    printf("total:%d--%d\r\n", value_U321, value_U322);

    
    value_U321 = 0;
    memcpy(&value_U321, pRecord->chargeAllEleMny, 4);
    printf("EleMny:%d\r\n", value_U321);
    
    value_U321 = 0;
    memcpy(&value_U321, pRecord->chargeSerMny, 4);
    printf("SerMny:%d\r\n", value_U321);
    
    value_U321 = 0;
    memcpy(&value_U321, pRecord->chargePerchMny, 4);
    printf("PerchMny:%d\r\n", value_U321);
    
    value_U321 = 0;
    memcpy(&value_U321, pRecord->chargeOrderMny, 4);
    printf("OrderMny:%d\r\n", value_U321);
    
    value_U321 = 0;
    memcpy(&value_U321, pRecord->chargeAllMny, 4);
    printf("AllMny:%d\r\n", value_U321);



}
//分别计算电费服务费占位费预约费
static void HaiNCT_CalSerPerMoney(uint8_t u8Port, RecordA3 *pRecord)
{
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	RATE_MODEL_T *pcostInData = &g_cost_ctrl.rate_model[u8Port];
    FeeModelA8 *pFreeModel = HaiNCT_Get_BillID();

    uint8_t num = pcostInData->rateNumber;
    uint64_t tChargeEleMny = 0;
    uint64_t tChargeSerMny = 0;
    uint64_t tChargeOrderMny = 0;
    uint64_t tChargePerMny = 0;
    uint32_t tChargePerEleMny = 0;    //分时电费金额
    uint32_t tChargePerSerMny = 0;    //分时服务费金额
    for (int i = 0; i < num; i++) {
        uint32_t tData = 0;
        uint64_t tu64Data = 0;
        //总电费
        tu64Data = (uint64_t)pcostdata->ele_power[i] * pcostInData->price48_16u[i];
        tu64Data = tu64Data / 1000000 + 5;
        tu64Data = tu64Data / 10;
        tChargeEleMny += tu64Data;
        tChargePerEleMny = tu64Data;
        //总服务费
        tu64Data = (uint64_t)pcostdata->ele_power[i] * pcostInData->sever48_16u[i];
        tu64Data = tu64Data / 1000000 + 5;
        tu64Data = tu64Data / 10;
        tChargeSerMny += tu64Data;
        tChargePerSerMny = tu64Data;

        //预约费用
        memcpy(&tData, pFreeModel->B47modeldata[i].Order_fee, 4);
        tChargeOrderMny += (uint64_t)pcostdata->ele_power[i] * tData;
        
        //占位费用
        memcpy(&tData, pFreeModel->B47modeldata[i].Perch_fee, 4);
        tChargePerMny += (uint64_t)pcostdata->ele_power[i] * tData;

        uint32_t value_U32 = tChargePerEleMny + tChargePerSerMny;
        memcpy(pRecord->infoValue[i].perMny, &value_U32, 4);
    }
    //总电费
    // tChargeEleMny = pcostdata->allEleMoney;
    //总服务费
    // tChargeSerMny = pcostdata->allServerMoney;

    //四舍五入
    // tChargeEleMny = tChargeEleMny / 10 + 5;
    // tChargeSerMny = tChargeSerMny / 10 + 5;
    tChargeOrderMny = tChargeOrderMny / 1000000 + 5;
    tChargePerMny = tChargePerMny / 1000000 + 5;

    // tChargeEleMny = tChargeEleMny / 10;
    // tChargeSerMny = tChargeSerMny / 10;
    tChargeOrderMny = tChargeOrderMny / 10;
    tChargePerMny = tChargePerMny / 10;

    memcpy(pRecord->chargeAllEleMny,&tChargeEleMny, 4);
    memcpy(pRecord->chargeSerMny, &tChargeSerMny, 4);
    memcpy(pRecord->chargeOrderMny, &tChargeOrderMny, 4);
    memcpy(pRecord->chargePerchMny, &tChargePerMny, 4);

    uint64_t tChargeAllMny = tChargeEleMny + tChargeSerMny + tChargeOrderMny + tChargePerMny;
    SetPlat_ChgTotalMoney(u8Port, tChargeAllMny * 100);
    memcpy(pRecord->chargeAllMny, &tChargeAllMny, 4);

    memcpy(pRecord->consumedMny, &tChargeAllMny, 4);    //消费金额
    memcpy(pRecord->dealMny, &tChargeAllMny, 4);        //交易金额
}

static void HaiNCT_RealDataUpdate(uint8_t u8Port, RecordA3 *pRecord)
{
    HaiNCT_Recv_Rate_ModelA8 *pRecvBillCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRateModelA8;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    
    FeeModelA8 *pFreeModel = HaiNCT_Get_BillID();
    memcpy(pRecord->transaction_log_num, pFreeModel->billing_model, 16);
    
    memcpy(pRecord->transaction_log_num, pChgGunData->transaction_log_num, 16);
    //支付卡号
    if (HaiNct_getTradeFlag(u8Port) == eUP_Start_Style_App_HaiNCT) {
        memcpy(pRecord->PayCard, pUpGunData->UserID, 8);
    } else {
        memcpy(pRecord->PayCard, pUpGunData->phyCard, 8);
    }

    //物理卡号
    if (HaiNct_getTradeFlag(u8Port) == eUP_Start_Style_CardOnline_HaiNCT) {
        memcpy(pRecord->PhyCard, pUpGunData->phyCard, 8);
    } else {
        memset(pRecord->PhyCard, 0xFF, 8);
    }

	pRecord->TimePerFlag = 0;

    //开始结束时间
	uint8_t timeCP56Time[7]={0};
    Bin_to_Cp56time2a(&pChgGunData->chrg_start_time[1], (cp56time2a*)&timeCP56Time);
	memcpy(pRecord->chrg_start_time,timeCP56Time,7);

    Bin_to_Cp56time2a(&pChgGunData->chrg_stop_time[1], (cp56time2a*)&timeCP56Time);
	memcpy(pRecord->chrg_stop_time,timeCP56Time,7); //停止时间
	memcpy(pRecord->dealTime,timeCP56Time,7);       //交易时间
    
    uint32_t value_U32 = 0;
    //尖峰平谷 起止值
    for(uint8_t i = 0; i < 4; i++)
    {
        value_U32 = 0;
        memcpy(pRecord->EleValue[i].startEle, &value_U32, 4);
        value_U32 = pcostdata->ele_power[i]/10;
        memcpy(pRecord->EleValue[i].stopEle, &value_U32, 4);
    }

	pRecord->costType[0] = 1;
	pRecord->costType[1] = 0;
    //总起止值
    value_U32 = pChgGunData->total_start_elec / 10;
    memcpy(pRecord->EnergyEle[0], &value_U32, 4);
    value_U32 = pChgGunData->total_stop_elec / 10;
    memcpy(pRecord->EnergyEle[1], &value_U32, 4);

    //尖峰平谷单价、电量、金额
    for(uint8_t i = 0; i < 4; i++)
    {
        value_U32 = pcostdata->ele_rate[i];
        memcpy(pRecord->infoValue[i].unitPrice, &value_U32, 4);
        value_U32 = pcostdata->ele_power[i] / 10;
        memcpy(pRecord->infoValue[i].perEle, &value_U32, 4);
    }

    //总电量
    value_U32 = pcostdata->total_power / 10;
    memcpy(pRecord->TatalEle, &value_U32, 4);
	pRecord->BnsType[0] = HaiNCT_BNS_TYPE & 0xFF;
	pRecord->BnsType[1] = HaiNCT_BNS_TYPE >> 8;
    
    value_U32 = pChgGunData->sum_balance - pcostdata->total_money / 100;
    memcpy(pRecord->WalletBalance, &value_U32, 4);

    //总电费  预约费  占位费  服务费
    HaiNCT_CalSerPerMoney(u8Port, pRecord);

    memcpy(pRecord->userID, pUpGunData->UserID, 8);

    printf_HaiNCT_CalSerPerMoney(u8Port, pRecord);
}

//一分钟存储一次，更新停止原因，外部调用
void HaiNCT_packChgRecord(uint8_t u8Port, RecordA3 *pRecord)
{
    HaiNCT_RealDataUpdate(u8Port, pRecord);
    //停止原因
    hnct_packStopReasonChgRecord(u8Port, pRecord);
}

//实时上报数据需要用到，这部分数据，所以需要实时更新
void HaiNCT_packChgRecord_Scan(uint8_t u8Port)
{
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
    HaiNCT_RealDataUpdate(u8Port, &UpRecord->HaiNCTChgRecord);
}
//平台总费用需要实时更新并显示
static void HaiNCT_MoneyUpdate_Scan(uint8_t u8Port)
{
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
    //不充电不更新，否则会在上报离线记录的时候将金额都归零
    if (logic_get_gun_Uncharged(u8Port) == 0) {
        return;
    }
    
    //总电费  预约费  占位费  服务费
    HaiNCT_CalSerPerMoney(u8Port, &UpRecord->HaiNCTChgRecord);
}



//安徽铁塔参数存储
static void HaiNCT_ReadStoragePara()
{
    if (load_EEOP_Param((uint8_t *)s_HnctFlashInfo, sizeof(s_HnctFlashInfo)) == FALSE) {
        printf("HaiNCT_ReadStoragePara init.\r\n");
        memset(s_HnctFlashInfo, 0, sizeof(s_HnctFlashInfo));
    };
}
//海宁城投参数存储
static void HaiNCT_WriteStoragePara()
{
    Set_EEOP_Param((uint8_t *)s_HnctFlashInfo, sizeof(s_HnctFlashInfo));
}


void printf_HaiNCTBill(FeeModelA8 *pbillInfo)
{
    printf("billID: ");
    for (int i = 0; i < 8; i++) {
        printf("%x", pbillInfo->billing_model[i]);
    }
    printf("\r\n");
    
    tm_struct SysTime;
    SysTime.yearH = 20;
    Cp56time2a_to_Bin((uint8_t *)&SysTime.yearL, (cp56time2a *)pbillInfo->start_time);
    printf("startTime: %d%d:%d:%d %d:%d:%d\r\n", SysTime.yearH,SysTime.yearL,SysTime.month,SysTime.day,
    SysTime.hour,SysTime.minute,SysTime.second);
    Cp56time2a_to_Bin((uint8_t *)&SysTime.yearL, (cp56time2a *)pbillInfo->end_time);
    printf("stopTime: %d%d:%d:%d %d:%d:%d\r\n", SysTime.yearH,SysTime.yearL,SysTime.month,SysTime.day,
    SysTime.hour,SysTime.minute,SysTime.second);

    printf("timeNum: %d\r\n", pbillInfo->time_allnum);

    for (int i = 0; i < pbillInfo->time_allnum; i++) {
        printf(" timeStart: %d %d\r\n", pbillInfo->B47modeldata[i].rate_start[0],
            pbillInfo->B47modeldata[i].rate_start[1]);
        printf(" Serial_rate: %d\r\n", pbillInfo->B47modeldata[i].Serial_rate);
        uint32_t tempU32_1 = 0, tempU32_2 = 0, tempU32_3 = 0, tempU32_4 = 0;
        tempU32_1 = fourUint8ToUint32(pbillInfo->B47modeldata[i].UnitEleFee[0]);
        tempU32_2 = fourUint8ToUint32(pbillInfo->B47modeldata[i].UnitEleFee[1]);
        tempU32_3 = fourUint8ToUint32(pbillInfo->B47modeldata[i].UnitEleFee[2]);
        tempU32_4 = fourUint8ToUint32(pbillInfo->B47modeldata[i].UnitEleFee[3]);
        printf(" UnitEle: %d %d %d %d\r\n", tempU32_1, tempU32_2, tempU32_3, tempU32_4);
        
        tempU32_1 = fourUint8ToUint32(pbillInfo->B47modeldata[i].Ser_fee);
        printf(" SerUnit: %d\r\n", tempU32_1);
        tempU32_1 = fourUint8ToUint32(pbillInfo->B47modeldata[i].Perch_fee);
        printf(" PerchUnit: %d\r\n", tempU32_1);
        tempU32_1 = fourUint8ToUint32(pbillInfo->B47modeldata[i].Order_fee);
        printf(" OrderUnit: %d\r\n", tempU32_1);
        printf("\r\n");
    }
}


/********************************************************************
 * @brief B47费率保存   
 *       1.需求是每块枪有单独的自己的两套费率，
 *       2.每写入新的一套，需擦除整块区域后重新写入4套（双枪各双套，共四套）      
 * @param[in] pRateM  需要保存的费率结构体指针
 *            uport  枪号
 *            length 费率的总字节数  
 * @return  
 *********************************************************************/
void save_rateB47_model_HaiNCT(FeeModelA8 *pRateM,uint8_t uport,uint16_t length)
{
	uint8_t SameID_UnContentFlag = 0;
	uint8_t NeedCoverSameID_UnContent_No = 0;

    //直接存储到备份区
    memset(&HaiNCT_feeModel_save.FeemodelA8save_data[B47_B],0,sizeof(FeeModelA8));
    memcpy(&HaiNCT_feeModel_save.FeemodelA8save_data[B47_B],pRateM,length);

    //将上述更新的HaiNCT_feeModel_save[uport]存储到flash
    Save_rate_model(&HaiNCT_feeModel_save, sizeof(HaiNCT_feeModel_save));

    printf(".........Bill......Save.............\r\n");
}
/********************************************************************
 * @brief 	   读取当前桩内4套的B47费率(双枪各两套 2*2=4)
 * @param[in]	 
 * @return 	   
 *********************************************************************/	
void Read_rateB47_model_HaiNCT(void)
{
	memset(&HaiNCT_feeModel_save, 0, sizeof(HaiNCT_feeModel_save));
	
	uint32_t u32Dest = EXT_FLASH_RATE_MODEL_ADDR ;
    Read_rate_model((void *)&HaiNCT_feeModel_save,sizeof(HaiNCT_feeModel_save));
    printf_HaiNCTBill(&HaiNCT_feeModel_save.FeemodelA8save_data[B47_A]);
    printf_HaiNCTBill(&HaiNCT_feeModel_save.FeemodelA8save_data[B47_B]);
}

/*****************费率更新策略*******************************************************************************/
/*
*uint16_t get_Rate_HaiNCT_Priod(...)
*bool Hnct_Refresh_NowbillModel(...)
*/
/*
*static void get_Rate_HaiNCT_Priod(...)
*static void CP56Time2a_to_Time(...)
*static void Time_to_CP56Time2a(...)
*/
/********************************************************************
 * @brief 	   CP56格式转常规格式
 * @param[in]	毫秒L 毫秒H 分 时 日 月 年 
 * @return 	    (年H(0X14) 年L(0X19) 月 日 时 分 秒) //2025
 *********************************************************************/	
static void CP56Time2a_to_Time(uint8_t *timeCP56Time,uint8_t *Outorgtime)
{
	uint32_t SS=0;
	Outorgtime[0]=20;
	Outorgtime[1]=timeCP56Time[6];
	Outorgtime[2]=timeCP56Time[5];
	Outorgtime[3]=timeCP56Time[4];
	Outorgtime[4]=timeCP56Time[3];
	Outorgtime[5]=timeCP56Time[2];
	 SS = ((timeCP56Time[1]<<8)+timeCP56Time[0])/ 1000;
	 Outorgtime[6]=SS&0xFF;
}
/********************************************************************
 * @brief 	  常规格式转 CP56格式
 * @param[in]	(年H(0X14) 年L(0X19) 月 日 时 分 秒) //2025
 * @return 	     毫秒L 毫秒H 分 时 日 月 年
 *********************************************************************/	
static void Time_to_CP56Time2a(uint8_t *orgtime,uint8_t *OuttimeCP56Time)
{
	uint32_t SS=orgtime[6]*1000;

	OuttimeCP56Time[0]=SS&0XFF;
	OuttimeCP56Time[1]=(SS>>8)&0XFF;
	OuttimeCP56Time[2]=orgtime[5];
	OuttimeCP56Time[3]=orgtime[4];
	OuttimeCP56Time[4]=orgtime[3];
	OuttimeCP56Time[5]=orgtime[2];
	OuttimeCP56Time[6]=orgtime[1];

}


/********************************************************************
 * @brief 	 更新当前理论费率,每次开始充电时调用刷新
 * @param[in]	 
 * @return 	   true 当前时间段存在费率 
 * 
 *      更新U8 Now_billingmodel_and_num[port][10]； 
 *      当前应该运行的计费模型ID[0-7]   
 *      [8]当前ID是A套还是B套 
 *      [9]当前费率的套数含有的时段个数
 *  调用时间: 1.每次启动时调用 
 *            2.下发费率召测时B51，非充电中状态时调用    
 *            3.切换费率上B49报时，非充电中状态时调用  
 *********************************************************************/	
bool Hnct_Refresh_NowbillModel(uint8_t port)
{
    //统一将费率时间段格式（毫秒L 毫秒H 分 时 日 月 年）转换成 (年H 年L 月 日 时 分 秒)
	U8 timebill_0[7] = {0};//(年H 年L 月 日 时 分 秒)
	U8 timebill_1[7] = {0};
    uint32_t SS=0;
    
    //充电中不进行费率更新
    if (logic_get_gun_Uncharged(port)) {
        return false;
    }

    //两套模型一模一样不需要更新
    if (Compare_Array((uint8_t *)&HaiNCT_feeModel_save.FeemodelA8save_data[B47_A], 
                    (uint8_t *)&HaiNCT_feeModel_save.FeemodelA8save_data[B47_B], sizeof(FeeModelA8)) == TRUE) {
        return false;
    }
	//A套
	CP56Time2a_to_Time(HaiNCT_feeModel_save.FeemodelA8save_data[B47_A].start_time,timebill_0);
	//B套
	CP56Time2a_to_Time(HaiNCT_feeModel_save.FeemodelA8save_data[B47_B].start_time,timebill_1);

	 //先判断上述数据是否有效,只需验证月份是不是为0；即可确定哪套费率表是存在
	uint8_t Update_local_Nomber=0;//理论上当前时间的费率属于 HaiNCT_feeModel_save.FeemodelA8save_data[]的哪套下标

    if(timebill_0[2]==0&&timebill_1[2]==0)//说明不存在费率表
    {
        printf("BILLRATE: NOT BILLRATE.\r\n");
        return false;
    }
    
    //存在费率，检查主用和备用的生效时间
    //生效时间相同，不充电的时候更新备用-》主用
    //生效时间不同，判断何时到达，到达备用生效时间后更新备用-》主用
    //备用为空
    if (timebill_1[2] == 0) {
        return false;
    }
    //两套都存在，判断生效时间
    //比较两套费率先后的费率的切换时间-转换成自 1970 年 1 月 1 日以来的总秒数
    //秒数大----切换时间靠后
    uint32_t total_seconds_A = 0;
    uint32_t total_seconds_B = 0;
    timToStamp(&total_seconds_A, (tm_struct *)timebill_0);
    timToStamp(&total_seconds_B, (tm_struct *)timebill_1);

    //获取当前时间 -转换成自 1970 年 1 月 1 日以来的总秒数
    uint8_t timeOrgin[7] = {0};
    getRunTimeYYMDHMS(timeOrgin); 
    uint32_t total_seconds_Now = 0;     //当前时间戳
    timToStamp(&total_seconds_Now, (tm_struct *)timeOrgin);

    uint8_t updateMaster = 0;
    if (total_seconds_A == total_seconds_B) {
        updateMaster = 1;       //以最新下发的为准
    } else {
        //到达备份生效时间，备份切换到主用,备用是比较新的，时间到达直接切换
        if (total_seconds_Now < total_seconds_B) {
            updateMaster = 1;
        }
    }
    if (updateMaster == 0) {
        return false;
    }

    //存储更新过的费率
    memcpy(&HaiNCT_feeModel_save.FeemodelA8save_data[B47_A], &HaiNCT_feeModel_save.FeemodelA8save_data[B47_B], sizeof(FeeModelA8));
    //将上述更新的HaiNCT_feeModel_save[uport]存储到flash
    Save_rate_model(&HaiNCT_feeModel_save, sizeof(HaiNCT_feeModel_save));
    
    printf("BILLRATE: The Rate switch was successful.\r\n");

    return true;
}

/*****************更新存储的记录 **********************************************************************/
/********************************************************************
 * 设备生成交易流水号
 * 终端机器编码（ 16 位） + 序列号(16 位)， 
 * 序列号要确保唯一性； 序列号产生规则：
 * 年（ 两位） +月（ 两位） +日（ 两位） +时（ 两位） +序列号（ 8 位， 可以采用自增的方式， 必须保证交易流水号的的唯一性。 ）
 *********************************************************************/	
void HaiNCT_TransLogNumber_Update(uint8_t u8Port, uint8_t *ChargeNumber, uint8_t *logNumber)
{
    uint8_t preHour = 0;
    static uint32_t tLogNumber[GUN_NUM_MAX] = {0};
    uint8_t number[8] = {0};
	tm_struct strCurTime = get_current_time();
    
	BINToBCD(&number[0], &strCurTime.yearL, 1);
	BINToBCD(&number[1], &strCurTime.month, 1);
	BINToBCD(&number[2], &strCurTime.day, 1);
	BINToBCD(&number[3], &strCurTime.hour, 1);
    
    number[4] = u8Port;
    if (preHour != strCurTime.hour) {
        preHour = strCurTime.hour;
        memset(tLogNumber, 0, GUN_NUM_MAX);
    }

    tLogNumber[u8Port]++;
    uint8_t tLog[4] = {0};
	uint32ToTwoUint8(&tLog[0], tLogNumber[u8Port]);
	BINToBCD(&number[5], tLog, 3);

    memcpy(logNumber, ChargeNumber, 8);
    memcpy(&logNumber[8], number, 8);
}

 /********************************************************************
 * @brief 	 读取最新的记录并判断是否上传成功
 * @param[in]	
 * @return 	
 *********************************************************************/	
 void HaiNCTUpChargeRecordUpDealOffline(void)
 {
	for (uint8_t i = 0; i < GUN_NUM; i++ ) {
		uint8_t uGun = i;

        uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }

        uint8_t cmd = HaiNCT_S_OrderUp;
    
		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("HaiNCTUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}

	    RecordA3 *UpRecord = &g_chgData[uGun].DealRecord.ChgRecord.HaiNCTChgRecord;

        //停止原因
        hnct_packStopReasonChgRecord(i, UpRecord);

		SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
        Send_Immediately(uGun, cmd);
	}

    return ;

 }
/********************************************************************
 * @brief 	写入记录，在收到平台下发的B14 后调用
 * @param[in] 	u8Port 枪号
 *              *deal 记录的数据
 *              len  记录数据长度（默认直接写是0xFF即可） 	   
 *********************************************************************/	
static uint8_t HnctUpChargeStorageDeal(uint8_t u8Port, void *deal, uint8_t len)
{
    PlatDealRecord *pRecord = &g_chgData[u8Port].DealRecord;
    DealData_write(u8Port, (uint8_t *)&pRecord, sizeof(PlatDealRecord));
    return TRUE;
}


/****************枪状态**************************************************************************/

/********************************************************************
 * @brief 	更新枪状态
 * @param[in] 	u8Port 枪号
 *              
 *********************************************************************/	
void HaiNCTUpGunStateCheck(uint8_t u8Port)
{
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	static uint8_t gun_state[GUN_NUM_MAX_HaiNCT] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX_HaiNCT] = {0};
	uint8_t report_flag[GUN_NUM_MAX_HaiNCT] = {0};

    //没有心跳不需要传输实时数据
    if (Get_PlatConnectSta() != eOnline_Heart) {
        return;
    }
	if (logic_get_gun_charging(u8Port))
	{
		pUpGunData->upGunState = eUP_Gun_State_Work_HaiNCT;
	}
	else
	{
		if (TRUE == dev_getErrState(u8Port))
		{
			pUpGunData->upGunState = eUP_Gun_State_Warning_HaiNCT;
		}
        else if (logic_get_gun_StopFinish(u8Port))
        {
            pUpGunData->upGunState = eUP_Gun_State_Finish_HaiNCT;
        } 
		else
		{
			pUpGunData->upGunState = eUP_Gun_State_Standby_HaiNCT;
		}
	}

	if (gun_state[u8Port] != pUpGunData->upGunState)
	{
		gun_state[u8Port] = pUpGunData->upGunState;
		report_flag[u8Port] = TRUE;
	}

	if (gun_conn_state[u8Port] != GetPile_gun_connect(u8Port))
	{
		gun_conn_state[u8Port] = GetPile_gun_connect(u8Port);
		report_flag[u8Port] = TRUE;
	}

	if (TRUE == report_flag[u8Port])//已经登录，确保首次发送B1信息是重新登录时，解决上线首次会跳到这一步发送
	{
        pUpGunData->A1code = 3;
		SetSendEnable(u8Port, HaiNCT_S_RealData, SEND_ENABLE_ON);
		Send_Immediately(u8Port, HaiNCT_S_RealData);
	}

	return;
}

void HaiNCTUp_A20_ChangeCheck(uint8_t u8Port)
{
    //变化超过0.1kWh立即上送A.20充电过程中上传数据
    //充电后开启上报，停止充电关闭上报
    COST_GUN_DATA *gCostOut = CostGetOutput(u8Port);
    
    //充电中开启AHTT_RealData上报
    uint8_t chrgFlag = logic_get_gun_charging(u8Port);
    if (chrgFlag) {
        if (SEND_ENABLE_ON != GetSendEnable(u8Port, HaiNCT_S_ChargingData)) {
            SetSendEnable(u8Port, HaiNCT_S_ChargingData, SEND_ENABLE_ON);
            Send_Immediately(u8Port, HaiNCT_S_ChargingData);
        }
    } else {
        SetSendEnable(u8Port, HaiNCT_S_ChargingData, SEND_ENABLE_OFF);
    }
    
    if (SEND_ENABLE_ON != GetSendEnable(u8Port, HaiNCT_S_ChargingData)) {
        return;
    }

    static uint32_t sPrePower[GUN_NUM_MAX] = {0};
    if (gCostOut->total_power - sPrePower[u8Port] > 1000) {
		SetSendEnable(u8Port, HaiNCT_S_ChargingData, SEND_ENABLE_ON);
		Send_Immediately(u8Port, HaiNCT_S_ChargingData);
		SetSendEnable(u8Port, HaiNCT_S_RealData, SEND_ENABLE_ON);
		Send_Immediately(u8Port, HaiNCT_S_RealData);
        
        sPrePower[u8Port] = gCostOut->total_power;
    }
}
/****************启动判断**************************************************************************/

/********************************************************************
 * @brief 	   启动判断 
 * @param[in]	 u8Port 抢号
 *              up_fail_reason  返回的失败原因
 *              trade_flag  交易标识
 *              pCardNo  卡号
 *              pTrdNum  交易订单号
 *              BlMoney  余额
 *              starttime 定时开始时间
 *              endtime   定时结束时间
 * 
 * @return 	   true  启动成功
 *********************************************************************/	
uint8_t HaiNCT_monitor_charge_start(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney)
{
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	HaiNCT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;
	HaiNCT_Recv_Card_inf *pRecvStartCardCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_inf;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
    uint8_t reason = 0;

    Hnct_Refresh_NowbillModel(u8Port);
    
    pUpGunData->TradeFlag = trade_flag;     //启动类型,刷卡或者扫码
    uint8_t trade_flag_pile = 0;

    if (trade_flag == eUP_Start_Style_App_HaiNCT) {
        memcpy(pUpGunData->UserID, pRecvStartCharge->userID, 8);
        memset(pUpGunData->phyCard, 0xEE, 8);
        trade_flag_pile = eUP_Start_Style_App;
    } else {
        memset(pUpGunData->UserID, 0xFF, 8);
        memcpy(&pUpGunData->phyCard, pRecvStartCardCharge->Physics_card_number, 8);
        trade_flag_pile = eUP_Start_Style_CardOnline;
    }

    //卡号倒叙
    uint8_t cardN[8] = {0};
    reverse(pCardNo, cardN, 8);

    uint8_t ret = monitor_charge_start(u8Port, &reason, trade_flag_pile, cardN, pTrdNum, BlMoney);
    //停止原因转换
    if (reason == eUP_Start_Fail_NoConn) {
        *up_fail_reason = 1;
    } else {
        *up_fail_reason = 4 + reason;
    }

    //启动时存储当前计费模型号
	RecordA3 *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord.HaiNCTChgRecord;
    memset(UpRecord, 0, sizeof(RecordA3));
    HaiNCTCurrentRateNum(UpRecord->chargeBillID);
    return ret;
}


/*********海宁城投协议1.41收发**************************************************************************/
//注意: 桩号3100000000010019----------报文里是反位 1900010000000031

/***********************************************************************
*@brief 协议帧
【上行】：68 19 00 01 00 00 00 00 31 00 00
【下行】：68 01 19 00 01 00 00 00 00 31 00 00
*************************************************************************/

// 协议帧
uint16_t send_Identification_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};

	uint8_t *data = (uint8_t *)pdata;
	uint16_t datalen = 0;

	data[datalen++] = 0x68;
	data[datalen++] = HaiNCT_PROTOCOL_FRAME;
	data[datalen++] = 0;
	data[datalen++] = 0xFF;
	data[datalen++] = 2;        //协议版本
    //计费模型ID
    
	datalen += 8;
    //设备编号
	HaiNCT_Get_PlatNumberBCD(&data[datalen]);
	datalen += 8;
    
	data[datalen++] = GUN_NUM;
	data[datalen++] = 0x0B;     // 支持按电量、按时间、按金额

    //桩厂家编码
	data[datalen++] = HaiNCT_FAC_NUM >> 8;
	data[datalen++] = HaiNCT_FAC_NUM & 0xFF;

	return datalen;
}

void send_Identification_HaiNCT_Succ(uint8_t u8Port)
{
	// if (RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_Identification))
	// {
	// 	SetRecvEnable(u8Port, HaiNCT_R_Identification, RECV_ENABLE_ON);
	// 	SetRecvTick(u8Port, HaiNCT_R_Identification, Get_Systick());
	// }

	return;
}

// 协议帧收 //680119000100000000310000
uint8_t recv_login_data_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	HaiNCT_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvIdenf;

	if (u8Port >= GUN_NUM_MAX_HaiNCT)
		return FALSE;

	gun[0] = u8Port;

    if (len != 25) {
        printf("recv--login len err\r\n");
		return FALSE;
    }
	memcpy(pRecvIdenf, r_data + 5, sizeof(HaiNCT_Recv_Identification));

	return TRUE;
}

void recv_login_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvIdenf;

	uint8_t plieNumber[8] = {0};
	HaiNCT_Get_PlatNumberBCD(plieNumber);
	// 判断跟桩编号是否一致
 	if (true == Compare_Array(plieNumber, (uint8_t *)pRecvIdenf->device_number, 8))
 	{
		SetSendEnable(u8Port, HaiNCT_S_Identification, SEND_ENABLE_OFF);
		// SetSendEnable(u8Port, HaiNCT_S_U, SEND_ENABLE_ON);
        // Send_Immediately(u8Port, HaiNCT_S_U);
 	} else {
        printf("recv--login pileNum err\r\n");
    }

	 HaiNCT_Recv_TimeSyn *pRecvSYn = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;
    
	return;
}

/*****************************************************************
 * @brief  U帧
【上行】：68 04 07 00 00 00
【下行】：68 04 0B 00 00 00
******************************************************************/
// U帧发送 680407000000
uint16_t send_U_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;
	uint16_t i = 0;
    i = HaiNCT_PackUFrameData(data, eHaiNCT_U_STARTACK);
	return i;
}

void send_U_HaiNCT_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, HaiNCT_S_START_U, SEND_ENABLE_OFF);
    SetSendEnable(u8Port, HaiNCT_S_Heart, SEND_ENABLE_ON);
    Send_Immediately(u8Port, HaiNCT_S_Heart);
	return;
}
// U帧认证应答解析
uint8_t recv_U_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	return TRUE;
}

void recv_U_HaiNCT_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, HaiNCT_S_START_U, SEND_ENABLE_ON);
    Send_Immediately(u8Port, HaiNCT_S_START_U);

	return;
}

/*****************************************************************
 * @brief  心跳帧发送
【上行】：68 04 43 00 00 00
【下行】：68 04 83 00 00 00
******************************************************************/
// 心跳帧发送 68 04 43 00 00 00
uint16_t send_Heart_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = 0;
    
    data_len = HaiNCT_PackUFrameData(data, eHaiNCT_U_TEST);
	return data_len;
}

void send_Heart_HaiNCT_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_Heart))
	{
		SetRecvEnable(u8Port, HaiNCT_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, HaiNCT_R_Heart, Get_Systick());
	}

	return;
}
// 心跳认证应答解析
uint8_t recv_R_Heart_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;

	if (u8Port >= GUN_NUM_MAX_HaiNCT)
		return FALSE;

	gun[0] = u8Port;

	return TRUE;
}

void recv_R_Heart_HaiNCT_Succ(uint8_t u8Port)
{
    PlatHeartTickRefresh();

	Set_PlatConnectSta(eOnline_Heart);

	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_Heart))
	{
		SetRecvEnable(u8Port, HaiNCT_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, HaiNCT_R_Heart, Get_Systick());
	}

	return;
}

uint8_t recv_clocksyn_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	HaiNCT_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;

	if (u8Port >= GUN_NUM_MAX_HaiNCT)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data+16, 7);

	return TRUE;
}
void recv_clocksyn_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;

	tm_struct SysTime;
    SysTime.yearH = 20;
    Cp56time2a_to_Bin((uint8_t *)&SysTime.yearL, &pRecvTimeSyn->cur_time);
    setCurrentRunTime((uint8_t *)&SysTime);

	SetSendEnable(u8Port, HaiNCT_S_clocksyn, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_clocksyn);

	return;
}
//	对时发 68 13 00 00 00 00 67 00  07  00 00 00 00 00   FF B9 0D 0B 1D 02 14
uint16_t send_clocksyn_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	HaiNCT_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;

	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = HaiNCT_HeadLen - 1; //校时信息对象中没有记录类型

	tm_struct strCurTime = get_current_time();
    Bin_to_Cp56time2a((uint8_t *)(&strCurTime.yearL), (cp56time2a*)&data[data_len]);
    data_len += 7;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_103, 7, 0);

	return data_len;
}

void send_clocksyn_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;

	SetSendEnable(u8Port, HaiNCT_S_clocksyn, SEND_ENABLE_OFF);
    for(uint8_t i=0;i<GUN_NUM_MAX_HaiNCT;i++)
        Hnct_Refresh_NowbillModel(i);//首次上线对时 后刷新费率

    printf("Online refresh...billModel\r\n");

    HaiNCTUpChargeRecordUpDealOffline();
    
    SetSendEnable(u8Port, HaiNCT_S_AskerRateModel, SEND_ENABLE_ON);//一桩一计费模型，不用区分枪
    SetSendTick(u8Port, HaiNCT_S_AskerRateModel, Get_Systick() - eTick_1S);

	return;
}


//设备请求计费模型
uint16_t send_askRateModel_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = u8Port; // 充电接口标识

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 6, 4);
    
	return data_len;
}

void send_askRateModel_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvTimeSyn;

	SetRecvEnable(u8Port, HaiNCT_R_RateModel, RECV_ENABLE_ON);

	return;
}


/*****************************************************************
*@brief  B1充电过程实时监测数据 
        空闲2min 发送一次;
		充电30s 发送一次;
		状态变换发送一次(非协议里提及的);
******************************************************************/
uint16_t send_RealData_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;
    COST_GUN_DATA *gCostOut = CostGetOutput(u8Port);

	uint16_t data_len = HaiNCT_HeadLen;

	HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = u8Port; // 充电接口标识

	if (TRUE == GetPile_gun_connect(u8Port))
		data[data_len++] = 0x01; // 连接确认开关状态
	else
		data[data_len++] = 0x00;

	// 工作状态BCD码 2Byte
    data[data_len++] = pUpGunData->upGunState;
    data[data_len++] = 0;

    //过压警告
	if(1== dev_getErrExsit(u8Port, eErr_InputOverVol))
    {
        data[data_len] = 1;
    }
    data_len++;
    //欠压警告
    if(1== dev_getErrExsit(u8Port, eErr_InputLessVol))
    {
        data[data_len] = 1;
    }
    data_len++;
    //过流警告
    if(1== dev_getErrExsit(u8Port, eErr_OutputOverCurr))
    {
        data[data_len] = 1;
    }
    data_len++;

	uint32_t tempval = 0;
	// 充电输出电压
	tempval = GetPile_ChgOutVol(u8Port, 1);
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 充电输出电流
	tempval = GetPile_ChgOutCur(u8Port, 2);
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 输出继电器状态
    data[data_len++] = GetPile_GunRelayOut(u8Port);

	// 有功总电量
    uint32_t value = gCostOut->total_power / 10;
    uint32ToFourUint8(&data[data_len], value);
    data_len += 4;

    //三相信息
    data_len += 12;

    //温度过高保护
    if (dev_getErrExsit(u8Port, eErr_GunOverTempWarn) 
    || dev_getErrExsit(u8Port, eErr_EnvOverTempWarn)
    || dev_getErrExsit(u8Port, eErr_PlugOverTempWarn)) {
        data[data_len] = 1;
    }
    data_len++;

    data[data_len++] = dev_getErrExsit(u8Port, eErr_ShortCircleErro);
    data[data_len++] = dev_getErrExsit(u8Port, eErr_LeakageCurrErr);
    data[data_len++] = dev_getErrExsit(u8Port, eErr_EmergencyStop);

    //自定义故障
    // data[data_len++] = dev_getFirstStopErr(u8Port);

    // data_len += 4;
    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_134, pUpGunData->A1code, 1);

	return data_len;
}

void send_RealData_HaiNCT_Succ(uint8_t u8Port)
{
	return;
}


//A.20 充电中数据上报
uint16_t send_ChargeData_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;
    COST_GUN_DATA *gCostOut = CostGetOutput(u8Port);
    
    HaiNCT_packChgRecord_Scan(u8Port);    //上报前立即更新计费信息，需要上报

	uint16_t data_len = HaiNCT_HeadLen;

	HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = u8Port; // 充电接口标识
    
    //交易流水号
    memcpy(&data[data_len], pChgGunData->transaction_log_num, 16);
	data_len += 16;
    
	data[data_len++] = HaiNCT_BNS_TYPE & 0xFF;
	data[data_len++] = HaiNCT_BNS_TYPE >> 8;

    //用户身份ID
    memcpy(&data[data_len], pUpGunData->UserID, 8);
	data_len += 8;
    
    if (pUpGunData->TradeFlag == eUP_Start_Style_App_HaiNCT) {
        memset(&data[data_len], 0xEE, 8);
    } else {
        memcpy(&data[data_len], pUpGunData->phyCard, 8);
    }
	data_len += 8;

    //开始结束时间
	Bin_to_Cp56time2a(&pChgGunData->chrg_start_time[1], (cp56time2a*)&data[data_len]);
	data_len += 7;
    
    tm_struct tTime = get_current_time();
	Bin_to_Cp56time2a(&tTime.yearL, (cp56time2a*)&data[data_len]);
	data_len += 7;

    //尖峰平谷起止值
    RecordA3 *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord.HaiNCTChgRecord;
    memcpy(&data[data_len], UpRecord->EleValue, sizeof(UpRecord->EleValue));
	data_len += sizeof(UpRecord->EleValue);

    //尖峰平谷电量
    for (int i = 0; i < 4; i++) {
        memcpy(&data[data_len], UpRecord->infoValue[i].perEle, 4);
        data_len += 4;
    }

    memcpy(&data[data_len], UpRecord->TatalEle, 4);
    data_len += 4;

     //尖峰平谷电费
     uint32_t totalEleMny = 0;
     for (int i = 0; i < 4; i++) {
        memcpy(&data[data_len], UpRecord->infoValue[i].perMny, 4);
        data_len += 4;
        totalEleMny += fourUint8ToUint32(UpRecord->infoValue[i].perMny);
    }

    //总电费
    memcpy(&data[data_len], UpRecord->chargeAllEleMny, 4);
    data_len += 4;
    
    //总服务费
    memcpy(&data[data_len], UpRecord->chargeSerMny, 4);
    data_len += 4;

    //总占位费
    memcpy(&data[data_len], UpRecord->chargePerchMny, 4);
    data_len += 4;

	data[data_len++] = HaiNCT_MEASURE_TYPE & 0xFF;
	data[data_len++] = HaiNCT_MEASURE_TYPE >> 8;

    memcpy(&data[data_len], UpRecord->EnergyEle[0], 4);
    data_len += 4;

    memcpy(&data[data_len], UpRecord->EnergyEle[1], 4);
    data_len += 4;

	data[data_len++] = 0;
    
    memset(&data[data_len], 0, 17);
    data_len += 17;

    U32 temp = pChgGunData->chg_timer / 60;
    memcpy(&data[data_len], &temp, 2);
    data_len += 2;
    
    data_len += 2;
    memcpy(&data[data_len], UpRecord->chargeAllMny, 4);
    data_len += 4;
    
    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 3, 17);

    return data_len;
}

void send_ChargeData_HaiNCT_Succ(uint8_t u8Port)
{
	return;
}


// B4 充电启停控制命令下发下行数据（扫码充电）
uint8_t recv_Start_Chg_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_Start_Charge *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data, sizeof(HaiNCT_Recv_Start_Charge));

	return TRUE;
}

void HaiNCT_StartChargeCmdScan(uint8_t u8Port)
{
    //有启动指令，则开始轮询启动结果，成功开始充电立即上报，失败也立即上送
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
    if (pUpGunData->startCmdResult == 0) {
        return;
    }
    pUpGunData->upResult = 0;

    if (pUpGunData->startCmdResult == 2) {
        pUpGunData->upResult = 2;                //结果为失败
    } else if (pUpGunData->startCmdResult == 1) {
        //等待启动结果，15s
        if (eChargeState_Charging == GetPile_gun_state(u8Port)) {
            pUpGunData->upResult = 1;                //启动失败
        } else if ((eChargeState_StopFinish == GetPile_gun_state(u8Port) || eChargeState_Idle == GetPile_gun_state(u8Port))) {
            pUpGunData->upResult = 2;                //启动失败
        }
    }
    
    if (pUpGunData->upResult) {
        SetSendEnable(u8Port, HaiNCT_S_Start_ChgAsk, SEND_ENABLE_ON);
        Send_Immediately(u8Port, HaiNCT_S_Start_ChgAsk);
        pUpGunData->startCmdResult = 0;          //清零
    }
}
void recv_Start_Chg_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
	
	uint32_t sum_balance = 0;
	uint8_t StartType = pRecvStartCharge->start_type;
	uint32_t StartPara = pRecvStartCharge->start_para[0] | pRecvStartCharge->start_para[1] << 8 | pRecvStartCharge->start_para[2] << 16;
    
    pUpGunData->upResult = 0;

	SetSendEnable(u8Port, HaiNCT_S_Start_ChgAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_Start_ChgAsk);

    //启动下发桩号不一致
    if (Compare_HnctPile(pRecvStartCharge->device_number) == FALSE) {
        pUpGunData->upReason = 10;
        return;
    }

    printf("HaiNCT charge mode = %d para:%d\r\n", pRecvStartCharge->start_type, StartPara);
    if (StartType == E_HNCT_GunState_Money) {
        SetDetectModeParam(u8Port, eDetectMode_Count, StartPara / 10);
    } else if (StartType == E_HNCT_GunState_Time) {
        SetDetectModeParam(u8Port, eDetectMode_Time, StartPara / 100);
    } else if (StartType == E_HNCT_GunState_Ele) {
        SetDetectModeParam(u8Port, eDetectMode_Ele, StartPara / 100);
    } else if (StartType == E_HNCT_GunState_Auto) {
        sum_balance = 0xFFFF;
    } else {
        pUpGunData->upReason = 11;
        return;
    }

	sum_balance = fourUint8ToUint32(pRecvStartCharge->acount);//下发的是整数

	monitor_set_MonitorState(u8Port, eMonitorState_Service);

    uint8_t transLogNum[16] = {0};
    HaiNCT_TransLogNumber_Update(u8Port, pRecvStartCharge->device_number, transLogNum);
    //收到启动命令先清空后续充电过程中的实时记录的结构体
    memcpy(pUpGunData->UserID, pRecvStartCharge->userID, 8);
    
    if (TRUE == HaiNCT_monitor_charge_start(u8Port,
                                        &pUpGunData->upReason,
                                        eUP_Start_Style_App_HaiNCT,
                                        NULL,
                                        transLogNum,
                                        &sum_balance))
    {
        pUpGunData->startCmdResult = 1;
        fgv_CtrlStartCharge(u8Port);

        printf("----------A12--------------start success--------------\r\n");
    }
    else
    {
        pUpGunData->startCmdResult = 2;
        fgv_CtrlStopCharge(u8Port);
        printf("-----------A12-------------start fail--------------\r\n");
    }

	return;
}

// 停止充电
uint8_t recv_Stop_Chg_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_Stop_Charge *pRecvStop = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStopCharge;

    printf("HaiNCT stop charge.\r\n");

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

    memcpy(pRecvStop->device_number, r_data, 17);

    return TRUE;
}

void recv_Stop_Chg_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Stop_Charge *pRecvStop = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStopCharge;
	HaiNCT_Recv_Start_Charge *pRecvStart = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

    uint8_t reasonElse = 127;
    
	uint8_t dataNum[8] = {0};
	HaiNCT_Get_PlatNumberBCD(dataNum);

    uint8_t stopFlag = 1;
    if (Compare_Array(pRecvStop->device_number, dataNum, 8) == FALSE) {
        stopFlag = 0;
    }
    //刷卡启动，手机停止时没法判断，所以统一取消掉
    // if (Compare_Array(pRecvStop->stopUserID, pRecvStart->userID, 8) == FALSE) {
    //     stopFlag = 0;
    // }
    if (stopFlag) {
        pUpGunData->upResult = 0;
        stopPileCharge(u8Port, Pile_Stop_Reason_APP);
        fgv_CtrlStopCharge(u8Port);     //停止充电
    } else {
        printf("HaiNCT stop charge. erro.\r\n");
        pUpGunData->upResult = 1 << 7 & reasonElse;
    }
    
	SetSendEnable(u8Port, HaiNCT_S_Stop_ChgAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_Stop_ChgAsk);

	return;
}

//	B5. 充电启停控制命令结果确认（扫码充电）
// 68 31 00 00 00 00 8500070000000000151900010000000031 000000000180BB330E1D021462313016511429022020000000000000
uint16_t send_Start_ChgAsk_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;
	HaiNCT_Recv_Start_Charge *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;
	//	 充电接口标识 1
	data[data_len++] = u8Port;

    //交易流水号
    memcpy(&data[data_len], pChgGunData->transaction_log_num, 16);
	data_len += 16;

	// 计费模型编码
    HaiNCTCurrentRateNum(&data[data_len]);
	data_len += 8;

	//启动充电方式，0远程启动，1刷卡启动
	data[data_len++] = pUpGunData->TradeFlag;

	//执行结果
	data[data_len++] = pUpGunData->upResult;
	//异常原因
	data[data_len++] = pUpGunData->upReason;

    //允许充电总电压
    Uint16ToTwoUint8(&data[data_len], GetPile_ChgMaxVol(u8Port));
	data_len += 2;
    //允许充电总电流
    Uint16ToTwoUint8(&data[data_len], GetPile_ChgMaxCurrent(u8Port));
	data_len += 2;
    //电池标称总能量
	data_len += 2;
    //最高充电温度
	data[data_len++] = GetPile_ChgMaxTemp(u8Port);
    //单体电池允许最高充电电压
    Uint16ToTwoUint8(&data[data_len], GetPile_ChgMaxVol(u8Port));
	data_len += 2;
    //当前电池整车电压
    Uint16ToTwoUint8(&data[data_len], GetPile_ChgMaxVol(u8Port));
	data_len += 2;
    //用户身份ID
    memcpy(&data[data_len], pRecvIdenf->userID, sizeof(pRecvIdenf->userID));
	data_len += sizeof(pRecvIdenf->userID);
	
    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 13);

	return data_len;
}

void send_Start_ChgAsk_HaiNCT_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, HaiNCT_S_Start_ChgAsk, SEND_ENABLE_OFF);

	return;
}


//远程停止充电应答
uint16_t send_Stop_ChgAsk_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{	
	uint8_t *data = (uint8_t *)pdata;
	HaiNCT_Recv_Start_Charge *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;
	//	 充电接口标识 1
	data[data_len++] = u8Port;
    
	data[data_len++] = pUpGunData->upResult;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 14);
	return data_len;
}

void send_Stop_ChgAsk_HaiNCT_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, HaiNCT_S_Stop_ChgAsk, SEND_ENABLE_OFF);
}

void Hnct_GetCardNumber(uint8_t u8Port, uint8_t *data)
{
	uint8_t cardnumber[4] = {0};//物理卡号
	// uint8_t tcardnumber[4] = {0};//反转后
	GetPile_CardPhyNumber(u8Port, cardnumber);
	// reverse(cardnumber, tcardnumber, 4);
    uint32_t cardN = 0;
    memcpy(&cardN, cardnumber, 4);
    char chars[11] = {0};
    snprintf(chars, 11, "%u", cardN);

    uint8_t cardData[5] = {0};
    AsciiPToBCD(chars, (char *)cardData, 10);

	reverse(cardData, &data[0], 5);
}

/********************************************************************
 * @brief 	 B6. 刷卡鉴权上行（在线刷卡充电）
 *           B7刷卡鉴权下行（在线刷卡充电）
 *********************************************************************/	
//B6. 刷卡鉴权上行（在线刷卡充电）
uint16_t send_Cardinf_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t data_len = HaiNCT_HeadLen;
    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	//	 充电接口标识 1
	data[data_len++] = u8Port;

	// 物理卡号 8byte
    Hnct_GetCardNumber(u8Port, &data[data_len]);
	data_len += 8;

	// 密码 16
	data_len += 16;
    // 输入密码
	data_len += 16;

	// 卡余额 4
	memset(&data[data_len], 0, 4);
	data_len += 4;
    
    //卡状态
	data_len += 2;

	// 电动汽车唯一标识 32
	data_len += 32;

	// 计费模型编码
    HaiNCTCurrentRateNum(&data[data_len]);
	data_len += 8;
    
	data_len += 8;
	data_len += 8;
    
    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 6, 1);

	return data_len;
}
void send_Cardinf_HaiNCT_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_CardinfAck))
	{
		SetRecvEnable(u8Port, HaiNCT_R_CardinfAck, RECV_ENABLE_ON);
		SetRecvTick(u8Port, HaiNCT_R_CardinfAck, Get_Systick());
	}
    
	SetSendEnable(u8Port, HaiNCT_S_Cardinf, SEND_ENABLE_OFF);

	return;
}

// B7刷卡鉴权下行（在线刷卡充电）
uint8_t recv_CardinfAck_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_Card_inf *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_inf;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 9, sizeof(HaiNCT_Recv_Card_inf));

	return TRUE;
}

void recv_CardinfAck_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Card_inf *pRecvStartCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_inf;
	uint32_t sum_balance = 0;
	sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);

    //判断物理卡号是否一致
	uint8_t cardnumber[8] = {0};
    Hnct_GetCardNumber(u8Port, cardnumber);
    if (Compare_Array(pRecvStartCharge->Physics_card_number, cardnumber, 8) == FALSE) {
        SetPlat_CardChargeFaild(u8Port, 1);
        return;
    }
    //计费模型是否与当前一致
    
    //是否鉴权成功
    if (pRecvStartCharge->check_card == 0) {
        SetPlat_CardChargeFaild(u8Port, 2);
        printf("CARD_CHARGE: FAILD--%x\r\n", pRecvStartCharge->failcheckreason[1] << 8 | pRecvStartCharge->failcheckreason[0]);
        return;
    }

    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

    memcpy(&pUpGunData->acount, pRecvStartCharge->account_balance, 4);

	if (1 == pRecvStartCharge->check_card && sum_balance)
	{
		SetSendEnable(u8Port, HaiNCT_S_CardStart_Chg, SEND_ENABLE_ON); // 发送B10信息
		Send_Immediately(u8Port, HaiNCT_S_CardStart_Chg);
	}
    SetRecvEnable(u8Port, HaiNCT_R_CardinfAck, RECV_ENABLE_OFF);
}
/********************************************************************
 * @brief B10. 启动通知上报（在线刷卡充电/在线vin码充电）
          B11 启动通知下行（在线刷卡充电/在线vin码充电）
 *********************************************************************/	
//	B10. 启动通知上报（在线刷卡充电/在线vin码充电）
uint16_t send_CardStart_Chg_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	// CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t data_len = HaiNCT_HeadLen;

	HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;
	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 物理卡号 8byte
    Hnct_GetCardNumber(u8Port, &data[data_len]);
	data_len += 8;

	// 密码 16
	memset(&data[data_len], 0, 16);
	data_len += 16;
	data_len += 16;

	// 电动汽车唯一标识 32
	memset(&data[data_len], 0, 32);
	data_len += 32;

    //计费模型编码
    HaiNCTCurrentRateNum(&data[data_len]);
	data_len += 8;

    //用户身份
	data_len += 8;
    //充电方式
	data[data_len++] = 0;
    
	data_len += 3;  //参数为0

	// 启动充电控制方式
	// data[data_len++] = 4;
	// 启动充电控制数据
	// data[data_len++] = 0;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 6, 20);

	return data_len;
}
void send_CardStart_Chg_HaiNCT_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_CardStart_ChgAck))
	{
		SetRecvEnable(u8Port, HaiNCT_R_CardStart_ChgAck, RECV_ENABLE_ON);
		SetRecvTick(u8Port, HaiNCT_R_CardStart_ChgAck, Get_Systick());
	}

	return;
}

// B11 启动通知下行（在线刷卡充电/在线vin码充电）
uint8_t recv_CardStart_ChgAck_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_Card_start *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_start;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, sizeof(HaiNCT_Recv_Card_start));

	return TRUE;
}


void recv_CardStart_ChgAck_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Card_start *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_start;
	HaiNCT_Recv_Card_inf *pRecvStartCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvCard_inf;
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
    
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	uint32_t Card_sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);
 
     monitor_set_MonitorState(u8Port, eMonitorState_Service);

    uint8_t transLogNum[16] = {0};

	uint32_t sum_balance = pUpGunData->acount;
    memcpy(transLogNum, pRecvIdenf->transaction_log_num, sizeof(transLogNum));
    
	if (0 == pRecvIdenf->result)
	{
        if (TRUE == HaiNCT_monitor_charge_start(u8Port,
                                                &pUpGunData->upReason,
                                                eUP_Start_Style_CardOnline_HaiNCT,
                                                pRecvStartCharge->Physics_card_number,
                                                transLogNum,
                                                &sum_balance))
		{
			fgv_CtrlStartCharge(u8Port);
		}
		else
		{
			fgv_CtrlStopCharge(u8Port);
		}

	}

    SetSendEnable(u8Port, HaiNCT_S_CardStart_Chg, SEND_ENABLE_OFF);
    SetRecvEnable(u8Port, HaiNCT_R_CardStart_ChgAck, RECV_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	B23 远程升级启动（扩展） ftp升级
            B24 远程升级启动命令接收结果（扩展）
 *********************************************************************/	
uint8_t recv_RemoteUpgrade_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	HaiNCT_Recv_RemUp_Cmd *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRemUp_Cmd;

	if (u8Port >= GUN_NUM_MAX_HaiNCT)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data+8, sizeof(HaiNCT_Recv_RemUp_Cmd));
	return TRUE;
}

void recv_RemoteUpgrade_HaiNCT_Succ(uint8_t u8Port)
{
	U8 recv_update_ip[4] = {0};
	char recip[17] = {0};
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
	HaiNCT_Recv_RemUp_Cmd *pRecvFtp = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRemUp_Cmd;

	SetSendEnable(u8Port, HaiNCT_S_RemoteUpgradeAck, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_RemoteUpgradeAck); // B24

    if (pRecvFtp->VV != 0xAA) {
        return;
    }
	ptcp_data->up_update_ret = 0;
	g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;		  // 升级
	g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick(); // 超时时间

	// 如：C0 02 03 04 转换成IP 为192.2.3.4
	recv_update_ip[0] = pRecvFtp->update_ip[3];
	recv_update_ip[1] = pRecvFtp->update_ip[2];
	recv_update_ip[2] = pRecvFtp->update_ip[1];
	recv_update_ip[3] = pRecvFtp->update_ip[0];

	sprintf(recip, "%d.%d.%d.%d", recv_update_ip[0], recv_update_ip[1], recv_update_ip[2], recv_update_ip[3]);
    uint16_t u16Port = pRecvFtp->update_com;

    g_PileUpdateInterface(recip, u16Port, (char *)pRecvFtp->update_username, (char *)pRecvFtp->update_password, 
                            (char *)pRecvFtp->update_file_path, (char *)pRecvFtp->update_file_name);

	return;
}
//	B24. 远程升级启动命令接收结果（扩展）
uint16_t send_RemoteUpgradeAck_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = HaiNCT_HeadLen;

	HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = 0x00;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 16);

	return data_len;
}

void send_RemoteUpgradeAck_HaiNCT_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, HaiNCT_S_RemoteUpgradeAck, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief B33	 充电功率控制下行（扩展）
          B34.   充电功率控制上行（扩展） 
 *********************************************************************/	
// 充电功率控制下行（扩展） B33
uint32_t HaiNCTGet_devpow(void) // 获取桩默认功率
{
	return sg_platmod.pileCfgInfo.pow_limit;
}

void HaiNCT_Set_devpow(void) // 获取桩默认功率
{
	return;
}

// 初始化运行时的功率值
void HaiNCT_Set_powerinit(uint8_t u8Port)
{
}

uint8_t recv_PowerCon_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	HaiNCT_Recv_Powercontrol *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvPowercontrol;

	if (u8Port >= GUN_NUM_MAX_HaiNCT)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8 + 1, sizeof(HaiNCT_Recv_Powercontrol));

	return TRUE;
}

/*******************************************************关于功率控制部分*************************************************************************** */

//功率控制超时后自动取消限制
void HaiNCTSetPowerTimeoutScan(uint8_t u8Port)
{
    HaiNCT_FlashPlatInfo *pFlashGunData = &s_HnctFlashInfo[u8Port];
    if ((pFlashGunData->powerCtrlEnable == 0) || (pFlashGunData->powerCtrlEnable == 0xFF)) {
        return;
    }
	tm_struct SysTime;
    SysTime.yearH = 20;
    Cp56time2a_to_Bin((uint8_t *)&SysTime.yearL, (cp56time2a*)&pFlashGunData->PowerEndTime);
    //超时时间戳
    uint32_t outTime = 0;
    timToStamp(&outTime, (tm_struct *)&SysTime);
    //当前时间戳
    uint32_t currentTime = 0;
    tm_struct SysTime_current = get_current_time();
    timToStamp(&currentTime, &SysTime_current);

    if (currentTime > outTime) {
        memset(pFlashGunData, 0, sizeof(HaiNCT_FlashPlatInfo));
        HaiNCT_WriteStoragePara();
    }
}
//timer:单位s
static void SetPlatSetPower(uint8_t u8Port)
{
    #define DEFAULT_POWER   700 //7kw
    HaiNCT_FlashPlatInfo *pFlashGunData = &s_HnctFlashInfo[u8Port];
    if ((pFlashGunData->powerCtrlEnable == 0) || (pFlashGunData->powerCtrlEnable == 0xFF)) {
        fgv_CtrlChargeCrt(u8Port, DEFAULT_POWER);
    }

	uint16_t tPercent = twoUint8ToUint16(pFlashGunData->powerPct);        //百分比，338表示33.8%

    uint16_t powerVual = 7 * tPercent / 10;
    
    if(powerVual >= 132 && powerVual <= 700) {
        
        fgv_CtrlChargeCrt(u8Port, powerVual);
    
        printf("SetPlatSetPower: %d %d\r\n", u8Port, tPercent);
    } else {
        fgv_CtrlChargeCrt(u8Port, DEFAULT_POWER);
    }
}

void recv_PowerCon_HaiNCT_Succ(uint8_t u8Port)
{
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
    HaiNCT_FlashPlatInfo *pFlashGunData = &s_HnctFlashInfo[u8Port];
	HaiNCT_Recv_Powercontrol *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvPowercontrol;
	// 更新功率
	uint16_t tPercent = twoUint8ToUint16(pRecvIdenf->SetPower);        //百分比，338表示33.8%

    pUpGunData->upResult = 0xFF;   //失败

    if(tPercent <= 1000) {
        memcpy(pFlashGunData->powerPct, pRecvIdenf->SetPower, 9);
        pFlashGunData->powerCtrlEnable = 1;
        HaiNCT_WriteStoragePara();
        SetPlatSetPower(u8Port);
        pUpGunData->upResult = 0;   //成功
    }

	SetSendEnable(u8Port, HaiNCT_S_PowerConASK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_PowerConASK);    

	return;
}
//	B34. 充电功率控制上行（扩展）
uint16_t send_PowerConASK_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;
	
    data[data_len++] = pUpGunData->upResult;
     
    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 15);

	return data_len;
}
void send_PowerConASK_HaiNCT_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, HaiNCT_S_PowerConASK, SEND_ENABLE_OFF);
	return;
}


/********************************************************************
 * @brief 	B47.下发计费模型下行数据—分时服务费
            B48.下发计费模型上行数据—分时服务费
 *********************************************************************/	
// 费率下发B47
uint8_t recv_Rate_SETB47_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
    uint8_t u8Port = r_data[8];
    HaiNCT_Recv_Rate_ModelA8 *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRateModelA8;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

    if (u8Port >= GUN_NUM)
        return FALSE;

    gun[0] = u8Port;

    pUpGunData->rateErro = 0;

    memset(pRecvIdenf,0,sizeof(HaiNCT_Recv_Rate_ModelA8));
    memcpy(&pRecvIdenf->device_number, r_data, len);//此处长度不定,不用sizeof(结构体); 

    //判断理论长度跟实际接收的对不对
    uint8_t  num=pRecvIdenf->billing_modelA8.time_allnum;//收到的时段数量
    uint16_t theoretical_length =8+1+8+7+7+2+2+1+num*sizeof(HaiNCT_Free_data);//理论长度

    memcpy(&pRecvIdenf->billing_modelA8.B47modeldata, &r_data[8+1+8+7+7+2+2+1], sizeof(HaiNCT_Free_data)*num);

    if (theoretical_length != len) {
        printf("BILLRATE: ERR--len = %d\r\n", len);
        pUpGunData->rateErro = 1;
    }

	return TRUE;
}

void recv_Rate_SETB47_HaiNCT_Succ(uint8_t u8Port)
{
    HaiNCT_Recv_Rate_ModelA8 *pRecvRateModel = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRateModelA8;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];
	// 验证费率的正确性
	uint8_t  num=pRecvRateModel->billing_modelA8.time_allnum;

	if(pUpGunData->rateErro != 1)
    {			
        for (uint8_t i = 0; i < num; i++)
        {
            if ((pRecvRateModel->billing_modelA8.B47modeldata[i].Serial_rate > 4)
            || (pRecvRateModel->billing_modelA8.B47modeldata[i].Serial_rate == 0))
            {
                printf("BILLRATE: ERR--rateType = %d\r\n", pRecvRateModel->billing_modelA8.B47modeldata[i].Serial_rate);
                pUpGunData->rateErro = 1;
                break;
            }
        }
    }
	
	if (1 != pUpGunData->rateErro)
	{
        SetSendEnable(u8Port, HaiNCT_S_AskerRateModel, SEND_ENABLE_OFF);

        printf_HaiNCTBill(&pRecvRateModel->billing_modelA8);    //打印计费细节

        //下发下来的新计费模型和旧计费模型一致不进行更新
        if (Compare_Array((uint8_t *)&pRecvRateModel->billing_modelA8, (uint8_t *)&HaiNCT_feeModel_save.FeemodelA8save_data[B47_B], sizeof(FeeModelA8)) == TRUE) {
            printf("BILLRATE: NOT CHANGE.\r\n");
        } else { 
            uint16_t length=8+7+7+2+1+num*sizeof(HaiNCT_Free_data); //FeeModelA8 实际用到的长度 billing_model 8个字节+ start_time 7个字节 以此类推
            save_rateB47_model_HaiNCT(&pRecvRateModel->billing_modelA8,u8Port,length); // 保存费率B47//
        }
	}
		
	SetSendEnable(u8Port, HaiNCT_S_RateModelAnswer, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_RateModelAnswer);

	return;
}

//	B48. 下发计费模型上行数据—分时服务费
uint16_t send_Rate_SETAskB48_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;
	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = u8Port; // 充电接口标识
    
    HaiNCTCurrentRateNum(&data[data_len]);
	data_len += 8;

	data[data_len++] = pUpGunData->rateErro;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 5);

	return data_len;
}
void send_Rate_SETAskB48_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Rate_ModelA8 *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvRateModelA8;

	SetSendEnable(u8Port, HaiNCT_S_RateModelAnswer, SEND_ENABLE_OFF);

	return;
}



/********************************************************************
 * @brief 	B53. 在线情况下停止充电上传分时交易明细数据 //////长度
            B54  在线分时明细交易包下行数据   
 *********************************************************************/	
//	B53. 在线情况下停止充电上传分时交易明细数据 //////长度
void HaiNCTDealUpdate_Cmd(uint8_t u8Port)
{
	SetSendEnable(u8Port, HaiNCT_S_OrderUp, SEND_ENABLE_ON);
	Send_Immediately(u8Port, HaiNCT_S_OrderUp);

}
uint16_t send_onlineEnd_ChgInfB53_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
    HaiNCT_Recv_Start_Charge *pRecvStartCharge = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvStartEndCharge;

	uint8_t *data = (uint8_t *)pdata;
	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	// 接口标识
	data[data_len++] = u8Port;

    uint16_t tlen = sizeof(RecordA3);
    memcpy(&data[data_len], &UpRecord->HaiNCTChgRecord, tlen);
    data_len += tlen;
    // data_len += 20; //协议中需要补充20个字节的0

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 6, 2);

	return data_len;
}
void send_onlineEnd_ChgInfB53_HaiNCT_Succ(uint8_t u8Port)
{
   CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, HaiNCT_R_OrderUp))
	{
		SetRecvEnable(u8Port, HaiNCT_R_OrderUp, RECV_ENABLE_ON);
		SetRecvTick(u8Port, HaiNCT_R_OrderUp, Get_Systick());
	}


	pChgGunData->upDealCnt++;

	if (pChgGunData->upDealCnt >= 3)
	{
		// 上报3次未回复，停止上报,重连
		if (SEND_ENABLE_ON == GetSendEnable(u8Port, HaiNCT_S_OrderUp))
		{
			SetSendEnable(u8Port, HaiNCT_S_OrderUp, SEND_ENABLE_OFF);
		}
	}
	else if (pChgGunData->upDealCnt == 1)
	{
		pChgGunData->ExistChargeDeal = 0;
	}

	return;
}


uint16_t send_replayQrcode_HaiNCT(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	HaiNCT_Recv_qrcode *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvQrcode;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	uint16_t data_len = HaiNCT_HeadLen;

    HaiNCT_Get_PlatNumberBCD(&data[data_len]);
	data_len += 8;

	data[data_len++] = pRecvIdenf->codeGun; 

	data[data_len++] = pUpGunData->upResult;

    HaiNCT_PackHeadData(data, u8Port, data_len, eHaiNCT_type_130, 7, 24);

	return data_len;
}

void send_replayQrcode_HaiNCT_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, HaiNCT_S_Qrcode, SEND_ENABLE_OFF);
    return;
}

// B54  在线分时明细交易包下行数据
uint8_t recv_onlineEnd_ChgInfAckB54_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvOnlinetrans_ask;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, sizeof(HaiNCT_Recv_Online_ask));

	return TRUE;
}

void recv_onlineEnd_ChgInfAckB54_HaiNCT_Succ(uint8_t u8Port)
{
	HaiNCT_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvOnlinetrans_ask;
	RecordA3 *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.HaiNCTChgRecord;

    SetSendEnable(u8Port, HaiNCT_S_OrderUp, SEND_ENABLE_OFF);

    updatePileStopReason(u8Port, Pile_Stop_Reason_Finish);
    GNUpChargeStorageDeal(u8Port, (void *)&g_chgData[u8Port].DealRecord, sizeof(PlatDealRecord));

    // HnctUpChargeStorageDeal(u8Port, (void *)pRecord, 0xFF);

	return;
}

// 参数设置
uint8_t recv_ParamSet_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	HaiNCT_Recv_ParamSet *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvParamSet;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 9, sizeof(HaiNCT_Recv_ParamSet));

	return TRUE;
}

void recv_ParamSet_HaiNCT_Succ(uint8_t u8Port)
{
	return;
}

uint8_t recv_ChargingData_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
    //处理账户余额、扣款信息
	return TRUE;
}
void recv_ChargingData_HaiNCT_Succ(uint8_t u8Port)
{

	return;
}


// 二维码设置
uint8_t recv_SetQrcode_HaiNCT(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = 0;
    if (r_data[8] == 0) {
        u8Port = 0;
    } else {
        u8Port = r_data[8] - 1;
    }
	gun[0] = u8Port;

	HaiNCT_Recv_qrcode *pRecvIdenf = &g_ProtocolDCB.pHaiNCTRecvData[u8Port].strRecvQrcode;
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

    pUpGunData->upResult = 1;   //失败

    uint8_t gunRcv = r_data[8];
    if (gunRcv > GUN_NUM) {
        return FALSE;
    } else if (gunRcv == 0) {
        return FALSE;
    }
    pUpGunData->upResult = 0;       //成功

    pRecvIdenf->codeGun = gunRcv;

    storage_PlatQRCodeInfoStr(gunRcv, (char *)&r_data[9]);

	return TRUE;
}

void recv_SetQrcode_HaiNCT_Succ(uint8_t u8Port)
{
    SetSendEnable(u8Port, HaiNCT_S_Qrcode, SEND_ENABLE_ON);
    Send_Immediately(u8Port, HaiNCT_S_Qrcode);

	return;
}

/********************************************************************
 * @brief 	 接收等待时间判断处理
 * @return   false 未超时  true 超时     
 *********************************************************************/	
uint8_t HaiNCTUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);

	if ((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;

	if (HaiNCT_R_Heart == cmd)
		Cyc *= 1;

	Cyc += eTick_5S;

	if (JudgeTimeOutMs(start_tick, Cyc))
	{
		return TRUE;
	}
		

	return FALSE;
}



/********************************************************************
 * @brief 	 发送轮询时间判断处理
 * @return   false 未超时  true 超时     
 *********************************************************************/	
uint8_t UpCtrlSendCycHaiNCT(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	uint32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
    HaiNCT_UpPlatInfo *pUpGunData = &s_HnctUpInfo[u8Port];

	if (TRUE == u8SendImmdFlag)
		return TRUE;

	if (JudgeTimeOutMs(start_tick, Cyc)) {
        if (HaiNCT_S_RealData == cmd ) {
            pUpGunData->A1code = 1;
        }
		return TRUE;
    }

	return FALSE;
}


void HaiNCT_CardAuthStart_Cmd(uint8_t u8Port)
{
	//插枪状态下刷有效卡，进行充电鉴权
    if(SEND_ENABLE_ON == GetSendEnable(u8Port, HaiNCT_S_Cardinf)) {
        return;
    }
    SetSendEnable(u8Port, HaiNCT_S_Cardinf, SEND_ENABLE_ON);

    Send_Immediately(u8Port, HaiNCT_S_Cardinf);
}




/***************************发送系列函数********************************************************************************/
typedef uint8_t (*PSendCyc)(uint8_t u8Port, uint32_t cmd, uint32_t Cyc);
typedef uint16_t (*PSend)(uint8_t u8Port, void *pBuf, uint16_t u32BufSize);
typedef void (*PSendSucc)(uint8_t u8Port);

typedef struct
{
	uint32_t cmd;
	uint32_t cyc;
	PSendCyc pSendCyc;
	PSend pSend;
	PSendSucc pSendSucc;
} HaiNCT_Send_ctrl;

// #define  HaiNCT_SEND_IMMD 0

const HaiNCT_Send_ctrl StrHaiNCT_SendCtrl[] = {
	{HaiNCT_S_Identification,      eTick_30S, UpCtrlSendCycHaiNCT, send_Identification_HaiNCT,  send_Identification_HaiNCT_Succ}, //
	{HaiNCT_S_START_U,             eTick_15S, UpCtrlSendCycHaiNCT, send_U_HaiNCT,               send_U_HaiNCT_Succ},
	{HaiNCT_S_Heart,               eTick_30S, UpCtrlSendCycHaiNCT, send_Heart_HaiNCT,           send_Heart_HaiNCT_Succ}, //
	{HaiNCT_S_clocksyn,            eTick_15S, UpCtrlSendCycHaiNCT, send_clocksyn_HaiNCT,        send_clocksyn_HaiNCT_Succ},

	{HaiNCT_S_RealData,            eTick_60S*5, UpCtrlSendCycHaiNCT, send_RealData_HaiNCT,      send_RealData_HaiNCT_Succ},
	{HaiNCT_S_ChargingData,        eTick_60S*1, UpCtrlSendCycHaiNCT, send_ChargeData_HaiNCT,     send_ChargeData_HaiNCT_Succ},

	{HaiNCT_S_AskerRateModel,      eTick_10S, UpCtrlSendCycHaiNCT, send_askRateModel_HaiNCT,    send_askRateModel_HaiNCT_Succ},
	{HaiNCT_S_RateModelAnswer,     eTick_15S, UpCtrlSendCycHaiNCT, send_Rate_SETAskB48_HaiNCT,  send_Rate_SETAskB48_HaiNCT_Succ},

	{HaiNCT_S_Start_ChgAsk,        eTick_15S, UpCtrlSendCycHaiNCT, send_Start_ChgAsk_HaiNCT,     send_Start_ChgAsk_HaiNCT_Succ},
	{HaiNCT_S_Stop_ChgAsk,         eTick_15S, UpCtrlSendCycHaiNCT, send_Stop_ChgAsk_HaiNCT,     send_Stop_ChgAsk_HaiNCT_Succ},
    
	{HaiNCT_S_Cardinf,             eTick_15S, UpCtrlSendCycHaiNCT, send_Cardinf_HaiNCT,         send_Cardinf_HaiNCT_Succ},
	{HaiNCT_S_CardStart_Chg,       eTick_15S, UpCtrlSendCycHaiNCT, send_CardStart_Chg_HaiNCT,   send_CardStart_Chg_HaiNCT_Succ},

	{HaiNCT_S_OrderUp,             eTick_10S, UpCtrlSendCycHaiNCT, send_onlineEnd_ChgInfB53_HaiNCT, send_onlineEnd_ChgInfB53_HaiNCT_Succ},

	{HaiNCT_S_RemoteUpgradeAck,    eTick_15S, UpCtrlSendCycHaiNCT, send_RemoteUpgradeAck_HaiNCT, send_RemoteUpgradeAck_HaiNCT_Succ},
	{HaiNCT_S_PowerConASK,         eTick_15S, UpCtrlSendCycHaiNCT, send_PowerConASK_HaiNCT,     send_PowerConASK_HaiNCT_Succ},
    
	{HaiNCT_S_Qrcode,              eTick_15S, UpCtrlSendCycHaiNCT, send_replayQrcode_HaiNCT,     send_replayQrcode_HaiNCT_Succ},

};
/*********************接收系列函数*****************************************************************************************************/
typedef uint8_t (*PRecvTimer)(uint8_t u8Port, uint32_t cmd, uint32_t OutTimer);
typedef uint8_t (*PRecv)(uint8_t *r_data, int len, uint8_t *gun);
typedef void (*PRecvSucc)(uint8_t u8Port);

typedef struct
{
	uint32_t cmd;
	uint32_t timer; // 平台主动下发需要桩回（0xFFFFFFF)， 桩主动，需要平台回复（eTick_xxS）
	PRecvTimer pRecvTimer;
	PRecv pRecv;
	PRecvSucc pRecvSucc;
} HaiNCT_Recv_ctrl;

const HaiNCT_Recv_ctrl StrHaiNCT_RecvCtrl[] = {

	{HaiNCT_R_Identification,   eTick_20S,      HaiNCTUpCtrlRecvTimer, recv_login_data_HaiNCT,      recv_login_HaiNCT_Succ},		 //
	{HaiNCT_R_START_U,          eTick_10S,      HaiNCTUpCtrlRecvTimer, recv_U_HaiNCT,               recv_U_HaiNCT_Succ},									 //
	{HaiNCT_R_Heart,            eTick_30S*5,    HaiNCTUpCtrlRecvTimer, recv_R_Heart_HaiNCT,         recv_R_Heart_HaiNCT_Succ},					 //
	{HaiNCT_R_clocksyn,         0xFFFFFFF,      HaiNCTUpCtrlRecvTimer, recv_clocksyn_HaiNCT,        recv_clocksyn_HaiNCT_Succ},				 //
	{HaiNCT_R_Start_Chg,        0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_Start_Chg_HaiNCT,       recv_Start_Chg_HaiNCT_Succ}, //
	{HaiNCT_R_Stop_Chg,         0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_Stop_Chg_HaiNCT,        recv_Stop_Chg_HaiNCT_Succ}, //
																													 //
	{HaiNCT_R_CardinfAck,       eTick_30S,      HaiNCTUpCtrlRecvTimer, recv_CardinfAck_HaiNCT,      recv_CardinfAck_HaiNCT_Succ},
	//
	{HaiNCT_R_CardStart_ChgAck, eTick_30S,      HaiNCTUpCtrlRecvTimer, recv_CardStart_ChgAck_HaiNCT,recv_CardStart_ChgAck_HaiNCT_Succ}, //
	//
    {HaiNCT_R_RemoteUpgrade,    0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_RemoteUpgrade_HaiNCT,   recv_RemoteUpgrade_HaiNCT_Succ}, //
	{HaiNCT_R_PowerCon,         0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_PowerCon_HaiNCT,        recv_PowerCon_HaiNCT_Succ},
	{HaiNCT_R_RateModel,        0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_Rate_SETB47_HaiNCT,     recv_Rate_SETB47_HaiNCT_Succ},
	{HaiNCT_R_OrderUp,          eTick_30S,      HaiNCTUpCtrlRecvTimer, recv_onlineEnd_ChgInfAckB54_HaiNCT, recv_onlineEnd_ChgInfAckB54_HaiNCT_Succ},

	{HaiNCT_R_ChargingData,     eTick_10S,      HaiNCTUpCtrlRecvTimer, recv_ChargingData_HaiNCT,     recv_ChargingData_HaiNCT_Succ},
	// {HaiNCT_R_ConfigPara,       eTick_30S,      HaiNCTUpCtrlRecvTimer, recv_ParamSet_HaiNCT,        recv_ParamSet_HaiNCT_Succ}, //

	{HaiNCT_R_Qrcode,           0xffffffff,     HaiNCTUpCtrlRecvTimer, recv_SetQrcode_HaiNCT,     recv_SetQrcode_HaiNCT_Succ},
};

uint8_t HaiNCT_GetRecvTopicAndIndex(uint8_t cmd, uint8_t *index)
{
	const HaiNCT_Recv_ctrl *pHaiNCT_RecvCtrl = NULL;

    for (int u32i = 0; u32i < ARRAY_SIZE(StrHaiNCT_RecvCtrl); u32i++) {
        
        pHaiNCT_RecvCtrl = &StrHaiNCT_RecvCtrl[u32i];

        if (pHaiNCT_RecvCtrl->cmd == cmd) {
            *index = u32i;
            return 0;
        }
    }
    
    return 1;   //未找到
}
uint8_t HaiNCT_GetSendTopicAndIndex(uint8_t cmd, uint8_t *index)
{
	const HaiNCT_Send_ctrl *pHaiNCT_SendCtrl = NULL;

    for (int u32i = 0; u32i < ARRAY_SIZE(StrHaiNCT_SendCtrl); u32i++) {
        
        pHaiNCT_SendCtrl = &StrHaiNCT_SendCtrl[u32i];

        if (pHaiNCT_SendCtrl->cmd == cmd) {
            *index = u32i;
            return 0;
        }
    }
    
    return 1;   //未找到
}

/********************************************************************
 * @brief 	 启动认证
 * @return   
 *********************************************************************/	
void HaiNCTUpLogin(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;

	if (!Comm_getIpSuces(eSocket_GPRS1))
	{
		return;
	}

	if (eOnline_Off == Get_PlatConnectSta())
	{
		Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, HaiNCT_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, HaiNCT_S_Identification);
	}
}


uint8_t HaiNCTUpChargeRecordUpDeal(void)
{
	return FALSE;
}

void HaiNCTUpCtrlTaskDeal(void) // 任务状态处理
{
	uint8_t i = 0;

	HaiNCTUpLogin();

	for (i = 0; i < GUN_NUM; i++)
	{
		HaiNCTUpGunStateCheck(i);   //实时检测数据变位上送
        HaiNCTUp_A20_ChangeCheck(i);//变化超过0.1kWh立即上送A.20充电过程中上传数据
	}

	HaiNCTUpChargeRecordUpDeal();

	return;
}

/********************************************************************
 * @brief 	 发送任务处理
 * @return   
 *********************************************************************/	
static uint16_t HaiNCTUpCtrlSendDeal(void *pBuf, uint32_t u32BufSize)
{
	const HaiNCT_Send_ctrl *pHaiNCT_SendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;

	uint8_t *pData = (uint8_t *)pBuf;
	for (i = 0; i < GUN_NUM; i++)  
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrHaiNCT_SendCtrl); u32i++)
		{
			pHaiNCT_SendCtrl = &StrHaiNCT_SendCtrl[u32i];

			if (SEND_ENABLE_ON != GetSendEnable(i, pHaiNCT_SendCtrl->cmd))
				continue;

			if (TRUE == pHaiNCT_SendCtrl->pSendCyc(i, pHaiNCT_SendCtrl->cmd, pHaiNCT_SendCtrl->cyc))
			{

				if ((outLen = pHaiNCT_SendCtrl->pSend(i, pData, u32BufSize)) > 0)
				{

					pHaiNCT_SendCtrl->pSendSucc(i);
					SetSendTick(i, pHaiNCT_SendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pHaiNCT_SendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pHaiNCT_SendCtrl->cmd, FALSE);

					UPRINT("\r\nUpProtocol --> GUN: %d, SendDealcmd: %x \r\n", i, pHaiNCT_SendCtrl->cmd);
					return outLen;
				}
			}
		}
	}

	return outLen;
}


static void HaiNCTUpSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;

	if (eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;

	outLen = HaiNCTUpCtrlSendDeal(pbuf, sizeof(pbuf));

	if (0 == outLen)
		return;

	PushPalTxBuf(eDataID_1, eDataType_TCP, NULL, 0, pbuf, outLen);

	return;
}

/********************************************************************
 * @brief 	 接收任务系列函数处理
 * @return   
 *********************************************************************/	
//根据传来数据的特征判断对应cmd
static uint8_t Deal_ANSYfromRec(uint8_t *recvData, uint16_t *len)
{
	HNCT_HEAD_T *pHead = NULL;
    uint8_t TypeIDE = 0;
	uint8_t cmd = 0;
    
    pHead = (HNCT_HEAD_T *)(recvData);
    *len = (pHead->len[1] << 8 | pHead->len[0]) + 3;

    //收到协议帧
	// 协议帧，U帧，心跳帧为特殊格式
    if ((recvData[1] == HaiNCT_PROTOCOL_FRAME) && (recvData[3] == 0xFF)) {
        printf("protocol frame.\r\n");
		cmd = HaiNCT_R_Identification;					 // 认证应答 例如 68 01 1900010000000031 0000
    } else if (recvData[1] == 4) {
        if (recvData[3] == eHaiNCT_U_START) {
            cmd = HaiNCT_R_START_U;	                // U帧启动指令 68 04 0B000000
        } else if (recvData[3] == eHaiNCT_U_STOP) {
            cmd = HaiNCT_R_STOP_U;	                // U帧停止指令 68 04 0B000000
        } else if (recvData[3] == eHaiNCT_U_TESTACK) {
            cmd = HaiNCT_R_Heart;					        // 心跳帧应答 680483000000
        }				            
    } else {
        uint8_t TypeIDE = pHead->TypeIDE;
        uint8_t recordKind = pHead->recordKind;

		if (TypeIDE == 103)
			cmd = HaiNCT_R_clocksyn;
        //下行数据
		else if (TypeIDE == 130)
		{
			// if (recordKind == 2)
			// 	cmd = HaiNCT_R_OrderUp;
		}
		else if (TypeIDE == 133)
		{
			if (recordKind == 12)
				cmd = HaiNCT_R_Start_Chg;   //启动充电
			else if (recordKind == 13)
				cmd = HaiNCT_R_Stop_Chg;    //终止充电
			else if (recordKind == 14)
				cmd = HaiNCT_R_PowerCon;    //功率控制
			else if (recordKind == 15)
				cmd = HaiNCT_R_RemoteUpgrade;//远程升级
			else if (recordKind == 17)
				cmd = HaiNCT_R_ConfigPara;  //配置参数
			else if (recordKind == 24)
				cmd = HaiNCT_R_Qrcode;      //二维码下发
            //以下为应答
			else if (recordKind == 2)
                cmd = HaiNCT_R_CardinfAck;      //充电鉴权下行数据
			else if (recordKind == 3)
                cmd = HaiNCT_R_OrderUp;          //充电记录确认数据
			else if (recordKind == 6)
                cmd = HaiNCT_R_RateModel;        //下发计费模型下行数据
			else if (recordKind == 16)
                cmd = HaiNCT_R_ChargingData;     //充电过程中上传确认数据
			else if (recordKind == 20)
                cmd = HaiNCT_R_CardStart_ChgAck; //刷卡启动充电通知确认
		}
	}

	return cmd;
}
//命令对应接收报文处理
void HaiNCTUpCtrlRecvDeal(HNCT_HEAD_T *pHead, uint8_t cmd, void *pindata, uint16_t inlen)
{
	const HaiNCT_Recv_ctrl *pHaiNCT_RecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = pHead->InfAddr[0] >> 4;      //端口号

	for (u32i = 0; u32i < ARRAY_SIZE(StrHaiNCT_RecvCtrl); u32i++)
	{
		pHaiNCT_RecvCtrl = &StrHaiNCT_RecvCtrl[u32i];
		//
		if (cmd == pHaiNCT_RecvCtrl->cmd)
		{
			if (TRUE == pHaiNCT_RecvCtrl->pRecv(pindata, inlen, &port))
			{
				pHaiNCT_RecvCtrl->pRecvSucc(port);

				SetRecvTick(port, cmd, Get_Systick());

				UPRINT("\r\nUpProtocol --> GUN: %d, RecvDealcmd: 0x%x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}
// 判断tcp接收到的所有数据是否合法
static int HaiNCTTcp_Read_Data_Check(uint8_t *r_data)
{
	if (r_data[0] != HaiNCT_PROTOCOL_HEAD)
	{
		printf("Check head erro  0x%x\r\n", r_data[0]);
		return -1;
	}

	return 0;
}

void HaiNCTfrom_buffer_data(U8 *recv_buf, U16 *len)
{
	U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, len, TCP_DATA_LEN_MAX);

	if (recv_buf[0] == HaiNCT_PROTOCOL_HEAD)
	{
		// 继续寻找len
		read_len = *len;
		if (read_len > TCP_DATA_LEN_MAX)
		{
			printf("\r\nprotocol--> recv buf full ! ");
			return;
		}
		*len = read_len;
	}
}



void HaiNCTPackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	HNCT_HEAD_T *pHead = NULL;
	uint8_t HaiNCTCmd = 0;
	uint8_t  tProtocolFrame = 0;
	uint8_t  *tHandleData = recv_buf;

	while (surplusLen)
	{
        tHandleData = recv_buf + currentIndex;
        if (HaiNCTTcp_Read_Data_Check(tHandleData) < 0)
		{
			return;
		}
        
        //收到协议帧
		uint16_t packLen = 0;
        HaiNCTCmd = Deal_ANSYfromRec(tHandleData, &packLen);

		// 防止乱数据导致程序死掉
		if (packLen > surplusLen)
		{
			return;
		}
		surplusLen = surplusLen - packLen;

		hex_dump("tcp_recv_data", tHandleData, packLen);

        if(HaiNCTCmd!=0)
		{
		  if (HaiNCTCmd >= 0xF1 && HaiNCTCmd <= 0xFF) // 这部分协议非正常格式，需要额外处理
			HaiNCTUpCtrlRecvDeal(pHead, HaiNCTCmd, tHandleData, packLen);
		  else 
			HaiNCTUpCtrlRecvDeal(pHead, HaiNCTCmd, tHandleData + sizeof(HNCT_HEAD_T), packLen-sizeof(HNCT_HEAD_T)); // packlen:		
		}
		
		currentIndex = currentIndex + packLen;
	}
}

void HaiNCTUpRecvDeal(void)
{
	uint8_t from_tcp_data[TCP_DATA_LEN_MAX]={0};
	U16 r_len = 0;
	HNCT_HEAD_T *pHead = NULL;

	// PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, from_tcp_data, (U16 *)&r_len, TCP_DATA_LEN_MAX);

	HaiNCTfrom_buffer_data(from_tcp_data, &r_len);
	if (r_len == 0)
		return;
	if (r_len > TCP_DATA_LEN_MAX)
	{
		printf("\r\nprotocol--> recv buf full ! ");
		return;
	}
	// 粘包处理
	HaiNCTPackConnectHandle(from_tcp_data, r_len);

	return;
}
/********************************************************************
 * @brief 	 接收超时处理
 * @return   
 *********************************************************************/	
void HaiNCT_RecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
  CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (HaiNCT_R_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}

	if(HaiNCT_R_OrderUp==cmd) 
	{
       if(pChgGunData->upDealCnt>=10)
	   {
		pChgGunData->upDealCnt=0;
	   }
	     
	}

   if(HaiNCT_R_CardStart_ChgAck == cmd)
	{
		SetSendEnable(u8Port, HaiNCT_S_CardStart_Chg, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, HaiNCT_R_CardStart_ChgAck, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
	}
   
	if(HaiNCT_R_CardinfAck==cmd)
	{
		SetSendEnable(u8Port, HaiNCT_S_Cardinf, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, HaiNCT_R_CardinfAck, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);

	}

	return;
}

void HaiNCTUpCtrlRecvOutTime(void)
{
	const HaiNCT_Recv_ctrl *pHaiNCT_RecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;

	for (i = 0; i < GUN_NUM; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrHaiNCT_RecvCtrl); u32i++)
		{
			pHaiNCT_RecvCtrl = &StrHaiNCT_RecvCtrl[u32i];

			if (RECV_ENABLE_ON != GetRecvEnable(i, pHaiNCT_RecvCtrl->cmd))
				continue;

			if (TRUE == pHaiNCT_RecvCtrl->pRecvTimer(i, pHaiNCT_RecvCtrl->cmd, pHaiNCT_RecvCtrl->timer))
			{
				HaiNCT_RecvOutTimeDeal(i, pHaiNCT_RecvCtrl->cmd);
			}
		}
	}
	return;
}



/********************************************************************
 * @brief 	主函数任务
 * @return   
 *********************************************************************/	

 static void HaiNCT_TaskInit(void)
{
    static uint8_t firstRead = 0;
    if (firstRead) {
        return;
    }
    firstRead = 1;

    HaiNCT_ReadStoragePara();     //配置信息读取
    Read_rateB47_model_HaiNCT();//开机从指定位置读取费率并更新出当前应该所在的费率ID
}


void HaiNCTUpProtocolDeal(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    
    HaiNCT_TaskInit();

	if (NULL == pProtocolDCB->pHaiNCTRecvData)
		return;

	HaiNCTUpCtrlTaskDeal();	  // 任务状态处理
	HaiNCTUpRecvDeal();		  // 接收处理
	HaiNCTUpSendDeal();		  // 发送处理
	HaiNCTUpCtrlRecvOutTime(); // 超时处理

    for (int i = 0; i < GUN_NUM_MAX; i++) {
        HaiNCT_StartChargeCmdScan(i);    //启动充电是否成功刷新
        HaiNCTSetPowerTimeoutScan(i);    //功率控制
        Hnct_Refresh_NowbillModel(i);    //计费模型更新
        HaiNCT_MoneyUpdate_Scan(i);
    }
	return;
}