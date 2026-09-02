/***********************************************************************************
 * 文 件 名  : iot_ANPEI_Protocol.c
 * 版 本 号  : V0.1
 * 负 责 人  : JI JUN
 * 创建日期  : 2025-02-20
 * 文件描述  : 浙江国网安培协议1.41版本开发
 * 版权说明  : 
 * 函数列表  :    
 * 其    他  :
 * 修改日志  :
***********************************************************************************/
#include "iot_ANPEI_Protocol_Code.h"
#include "protocol_ctrl.h"
#include "tcp_gn.h"
#include "AppCfg.h" 
#include "protocol_data.h"
#include "mbsMaster.h"
#include "maths.h"
#include "modbus.h"
#include "AppMidDataTrans.h"
#include "cost.h"
#include "AppDealFlash.h"




uint8_t B1_first_aftre_online_flag=0;//首次登录是在开机上线发送SIM信息后

/*************************费率结构体*****费率变量*************************************/

//[0-7]当前应该运行的计费模型ID  
//[8]当前ID是A套还是B套(0xFF,不存在) 
//[9]当前费率的套数含有的时段个数
U8 Now_billingmodel_and_num[GUN_NUM_MAX_ANPEI][10] = {{0,0,0,0,0,0,0,0,0xFF,0},{0,0,0,0,0,0,0,0,0xFF,0}};

//最近一次切换上报的费率([0-9]定义同上)
U8 Rate_Swtich_datamodel_and_num[GUN_NUM_MAX_ANPEI][10] = {{0,0,0,0,0,0,0,0,0xFF,0},{0,0,0,0,0,0,0,0,0xFF,0}}; 
//开机读取费率更新；收到新费率报文更新
FeeModel_Save_truct anpei_feeModel_save[GUN_NUM_MAX_ANPEI]={0};

U8 B49_upjudge_flag[GUN_NUM_MAX_ANPEI]={0};//B49切换上报标志位 1：收到新费率 需要判断是否到切换时间
/******计量实时电量结构体刷新*********************************************************/

chrg_EE_Money_struct chrg_EE_Money[GUN_NUM_MAX_ANPEI];
UP_0000_clock_struct UP_0000_clock_data[GUN_NUM_MAX_ANPEI];//零点上报电量数值
/******功率调节******************************************************************************/
Stu_GunPowerCtrArry stuGunPowerCtr[GUN_NUM_MAX_ANPEI];

/******协议外相关函数******************************************************************************************/
/********************************************************************
 * @brief 	   anpei协议获取桩编号(32位)，截断成16位（8个字节）
 * @param[in]	 
 * @return 	   pNum  8个字节桩编号的BCD
 *********************************************************************/
void AnpeiGet_PlatNumberBCD(uint8_t *pNum)
{
	char arry[PLAT_NUMBER_LEN]={0};
   Get_PlatNumberString(arry);   //例如 3100000000010019000000000000000
   
   for(uint8_t i=0;i<8;i++)
   {
        pNum[i]=(arry[2*i]-0x30)*16+arry[2*i+1]-0x30; //0x31 0x00 ..... 0x19

   }
}
/********************************************************************
 * @brief 	   anpei获取当前费率模型
 * @param[in]	 
 * @return 	   pNum  8个字节桩编号的BCD
 *********************************************************************/
uint8_t AnpeiCurrentRateType(uint8_t u8Port)
{
    //1.匹配当前运行的费率ID 是 A套 还是 B套 ，肯定有值
    uint8_t AorB= Now_billingmodel_and_num[u8Port][8];
    if(AorB==0xff) return 0; //说明没有时段费率，返回0
    return AorB;
}

void anpeiCostUpdate(uint8_t u8Port)
{
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];//rate_8u

    uint8_t arryvalue[4]={0};
    uint32ToFourUint8(arryvalue,(pcostdata->total_power)/10);
    Refresh_chrg_EE_Money_data(u8Port,No_total_EE,arryvalue,4);//当前订单的充电总电量 0.001

    uint32_t u32value[4]={0};//尖峰平谷
   
	for(uint8_t i=0;i<prate->PeriodNumber;i++)
	{
       if(prate->rate_8u[i]==1||prate->rate_8u[i]==6) //尖总=尖+尖扩展(j总=j+j扩展)
	     u32value[0]+=pcostdata->PeriodElePower[i];
		else if(prate->rate_8u[i]==2||prate->rate_8u[i]==7)//f总=f+f扩展
		  u32value[1]+=pcostdata->PeriodElePower[i];
		else if(prate->rate_8u[i]==3||prate->rate_8u[i]==8)//p总=p+p扩展
		  u32value[2]+=pcostdata->PeriodElePower[i];
		else if(prate->rate_8u[i]==4||prate->rate_8u[i]==5)//g总=g+g扩展
		  u32value[3]+=pcostdata->PeriodElePower[i];
	}

    uint32ToFourUint8(arryvalue,u32value[0]/10);//尖总 0.001
    Refresh_chrg_EE_Money_data(u8Port,No_total_sharpEE,arryvalue,4);


    uint32ToFourUint8(arryvalue,u32value[1]/10);//f总 0.001
    Refresh_chrg_EE_Money_data(u8Port,No_total_peakEE,arryvalue,4);


    uint32ToFourUint8(arryvalue,u32value[2]/10);//p总 0.001
    Refresh_chrg_EE_Money_data(u8Port,No_total_flatEE,arryvalue,4);

    uint32ToFourUint8(arryvalue,u32value[3]/10);//g总 0.001
    Refresh_chrg_EE_Money_data(u8Port,No_total_valleyEE,arryvalue,4);
}

/******实时数据处理******************************************************************************************/
/*
 * void Refresh_chrg_EE_Money_data(...)
 * void Refresh_chrg_EE_Money_piecewise_data (...)
 * void Clear_chrg_EE_Money_data(...)
*/

/********************************************************************
 * @brief 	   chrg_EE_Money[port]数据更新
 * @param[in]  port 枪口号 0 1 （不应超出当前使用的枪数）
 *             numkind 设置类型
 *             data:数据 
 *             databytenum：data数据的字节数
 *                          (不应溢出需刷新的结构体变量字节数)
 * @return 
 *********************************************************************/
void Refresh_chrg_EE_Money_data(uint8_t port,uint8_t numkind, uint8_t *data,uint8_t databytenum)
{
	CHG_DATA_T *pChgGunData = &g_chgData[port];
	
	if(port>=GUN_NUM_ANPEI) 
	   return;
	  
	  if (numkind == No_transaction_log_num)
			memcpy(&chrg_EE_Money[port].transaction_log_num, data, databytenum);
	  else if (numkind == No_anpei_card_number)
		memcpy(&chrg_EE_Money[port].anpei_card_number, data, databytenum);
	  else if (numkind == No_start_way)
		memcpy(&chrg_EE_Money[port].start_waykind, data, databytenum);	
	  else if (numkind == No_controlfee)
		  memcpy(&chrg_EE_Money[port].controlfee, data, databytenum);
	  else if (numkind == No_start_time)
		  memcpy(&chrg_EE_Money[port].start_time, data, databytenum);
	  else if (numkind == No_end_time)
		  memcpy(&chrg_EE_Money[port].end_time, data, databytenum);
	  else if (numkind == No_total_EE)
		  memcpy(&chrg_EE_Money[port].total_EE, data, databytenum);
	  else if (numkind == No_total_sharpEE)
		  memcpy(&chrg_EE_Money[port].total_sharpEE, data, databytenum);
	  else if (numkind == No_total_peakEE)
		  memcpy(&chrg_EE_Money[port].total_peakEE, data, databytenum);
	  else if (numkind == No_total_flatEE)
		  memcpy(&chrg_EE_Money[port].total_flatEE, data, databytenum);
	  else if (numkind == No_total_valleyEE)
		  memcpy(&chrg_EE_Money[port].total_valleyEE, data, databytenum);
	  else if (numkind == No_total_chrg_time)
		  memcpy(&chrg_EE_Money[port].total_chrg_time, data, databytenum);
	//   else if (numkind == No_total_chrg_EEmomey)
	// 	  memcpy(&chrg_EE_Money[port].total_chrg_EEmomey, data, databytenum);
	//   else if (numkind == No_total_chrg_severmomey)
	// 	  memcpy(&chrg_EE_Money[port].total_chrg_severmomey, data, databytenum);
  
}

/********************************************************************
 * @brief 	   订单上传结束后 或 开始启动充电时 清空 chrg_EE_Money[port]数据                 
 * @param[in] 
 * @return 
 *********************************************************************/
void Clear_chrg_EE_Money_data(uint8_t u8Port)
{
	if(u8Port>=GUN_NUM_ANPEI) return;
	
	memset(&chrg_EE_Money[u8Port], 0, sizeof(chrg_EE_Money_struct));

}


/*****************费率保存和读取*******************************************************************************/
/*
 *static uint8_t ANPEI_STO_Erase(...)
 *static uint8_t ANPEI_STO_Write (...)
 *static uint8_t ANPEI_STO_Read(...)
*/
/*
 *void save_rateB2_model_anpei(...)
 *uint16_t  Serach_billingmodel_ID(...)
 *uint8_t Comapare_content_billingModel(...)
*/
/*
 *void save_rateB47_model_anpei(...)
 *void  Read_rateB47_model_anpei(...)
*/
/********************************************************************
 * @brief 擦除  写入 读取    
 *********************************************************************/
//擦除
static uint8_t ANPEI_STO_Erase(uint32_t u32Dest)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr);
	
	return flag;
}
//写入
static uint8_t ANPEI_STO_Write(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreWriteRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}
//读取
static uint8_t ANPEI_STO_Read(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreReadRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}
/********************************************************************
 * @brief 	   B2基础(48时段)费率保存，目前不使用                     
 * @param[in]         
 * @return 
 *********************************************************************/
void save_rateB2_model_anpei(void *pRateM)
{
 return;
}
/********************************************************************
 * @brief 	   查询当前ID属于B47或B2同ID的费率                   
 * @param[in]   uport  枪号 (进入此函数已经保证该数值<GUN_NUM_ANPEI)
 *             needsearchID 需要查询的ID
 * @return      
 		  0x4700 B47类型的第一套   anpei_feeModel_save.FeemodelB47save_data[0]
		  0x4701 B47类型的第二套   anpei_feeModel_save.FeemodelB47save_data[1]
		  0x0200 B2类型的第一套   目前不开发
		  0x0201 B2类型的第二套   目前不开发
	 	  0xFFFF 未查到
 *********************************************************************/
uint16_t  Serach_billingmodel_ID(uint8_t uport,uint8_t *needsearchID)
{
    uint16_t resultresult = 0x4700+B47_A;//0X4700
	for (uint8_t i = 0; i < 8; i++)
	{		
		if (needsearchID[i] != anpei_feeModel_save[uport].FeemodelB47save_data[B47_A].billing_model[i])
		{
			resultresult= 0x4700+B47_B ;//0X4701
			for(uint8_t k=0;k<8;k++)
			{
				if (needsearchID[k] != anpei_feeModel_save[uport].FeemodelB47save_data[B47_B].billing_model[k])
				{
					resultresult = 0xFFFF;
					break;
				}
			}
			break;
		}
	}

	return resultresult;
}
/********************************************************************
 * @brief Comapare_content_billingModel 比较B47费率内容是否相等的相关函数   
 *        compare_Fee_data（Comapare_content_billingModel中的过程函数，功能类似）             
 * @param[in]  FeeModelB47 *a *b :需要比较的B47费率结构体指针
 * @return    0 不一样 ;1 一样
 *********************************************************************/
uint8_t compare_Fee_data(const Fee_data *a, const Fee_data *b) 
{
    if (a->Serial_number != b->Serial_number) return 0;
    if (a->Serial_rate != b->Serial_rate) return 0;
    if (memcmp(a->rate_start, b->rate_start, sizeof(a->rate_start)) != 0) return 0;
    if (memcmp(a->rate_end, b->rate_end, sizeof(a->rate_end)) != 0) return 0;
    if (memcmp(a->ele_fee, b->ele_fee, sizeof(a->ele_fee)) != 0) return 0;
    if (memcmp(a->ser_fee, b->ser_fee, sizeof(a->ser_fee)) != 0) return 0;
    return 1;
}
uint8_t Comapare_content_billingModel(FeeModelB47 *a,FeeModelB47 *b)
{
	if (memcmp(a->billing_model, b->billing_model, sizeof(a->billing_model)) != 0) return 0;
    if (memcmp(a->start_time, b->start_time, sizeof(a->start_time)) != 0) return 0;
    if (memcmp(a->end_time, b->end_time, sizeof(a->end_time)) != 0) return 0;
    if (memcmp(a->workstate, b->workstate, sizeof(a->workstate)) != 0) return 0;
    if (a->time_allnum != b->time_allnum) return 0;

    for (int i = 0; i < 12; i++) {
        if (!compare_Fee_data(&a->B47modeldata[i], &b->B47modeldata[i])) return 0;
    }

	return 1;
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
void save_rateB47_model_anpei(FeeModelB47 *pRateM,uint8_t uport,uint8_t length)
{
	uint8_t SameID_UnContentFlag = 0;
	uint8_t NeedCoverSameID_UnContent_No = 0;
	
	//获取当前正在运行的费率ID是哪一套
	//只会在刚开始没有任何费率模型时 NowNomber=0XFF，一旦存在费率模型，则必会返回0X00或0X01
    uint8_t NowNomber=Now_billingmodel_and_num[uport][8];

	//1.判断需要存储的费率模型跟当前两套费率ID比较，是否已经存在同ID
	 uint16_t NeedSaveNomber=Serach_billingmodel_ID(uport,pRateM->billing_model);
	 NeedSaveNomber=NeedSaveNomber&0xFF;
 
	if(NeedSaveNomber==B47_A||NeedSaveNomber==B47_B)
	{
		//2.具体内容比较 可能存在同ID但不同内容，则覆盖
		if(0==Comapare_content_billingModel(pRateM,&anpei_feeModel_save[uport].FeemodelB47save_data[NeedSaveNomber]))
		{
			//同ID不同内容,则覆盖FeemodelB47save_data[Nomber]里数据
			memset(&anpei_feeModel_save[uport].FeemodelB47save_data[NeedSaveNomber],0,sizeof(FeeModelB47));
            memcpy(&anpei_feeModel_save[uport].FeemodelB47save_data[NeedSaveNomber],pRateM,length);

			anpei_feeModel_save[uport].RecentUpdates_Nomber=NeedSaveNomber;
		}
		else //同ID同内容 
		{
			printf(".........Bill......NotNeedSave.............\r\n");
          return;//不需要重复保存

		}
   
	}
	else //新费率
	{
		//查找正在运行的费率是属于当前FeemodelB47save_data[未知]，更新另一套[]

          if(NowNomber==0xFF) 
		  {
			memset(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_A],0,sizeof(FeeModelB47));
			memset(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_B],0,sizeof(FeeModelB47));

            memcpy(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_A],pRateM,length);

			anpei_feeModel_save[uport].RecentUpdates_Nomber=B47_A;

		  }
		  else if(NowNomber==B47_A) //更新另一套
		  {
			memset(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_B],0,sizeof(FeeModelB47));
            memcpy(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_B],pRateM,length);

			anpei_feeModel_save[uport].RecentUpdates_Nomber=B47_B;
		  }
		  else if(NowNomber==B47_B)
		  {
            memset(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_A],0,sizeof(FeeModelB47));
            memcpy(&anpei_feeModel_save[uport].FeemodelB47save_data[B47_A],pRateM,length);

			anpei_feeModel_save[uport].RecentUpdates_Nomber=B47_A;

		  }
		

	}

	  
	 //将上述更新的anpei_feeModel_save[uport]存储到flash
      uint32_t u32Dest = EXT_FLASH_RATE_MODEL_ADDR;
      ANPEI_STO_Erase(u32Dest);
      ANPEI_STO_Write(u32Dest, &anpei_feeModel_save, 2*sizeof(FeeModel_Save_truct));
       
	  
       printf(".........Bill......Save.............\r\n");

	for(uint8_t i = 0; i < GUN_NUM_ANPEI; i++)
	{   
		B49_upjudge_flag[i]=1;//需要判断是否需要上报费率
		//如果没有充电,立即刷新计费模型,
		if(TRUE != g_cost_ctrl.start_chrg[i])
		   Refresh_NowbillModel(i);
	}



		
}
/********************************************************************
 * @brief 	   读取当前桩内4套的B47费率(双枪各两套 2*2=4)
 * @param[in]	 
 * @return 	   
 *********************************************************************/	
void Read_rateB47_model_anpei(void)
{
	memset(&anpei_feeModel_save, 0, 2*sizeof(FeeModel_Save_truct));
	
	uint32_t u32Dest = ANPEI_FLASH_RATE_MODEL_ADDR ;
	ANPEI_STO_Read(u32Dest, &anpei_feeModel_save, 2*sizeof(FeeModel_Save_truct));
    
}


/*****************费率更新策略*******************************************************************************/
/*
*uint16_t get_Rate_anpei_Priod(...)
*uint16_t Now_model_get_RatePriod(...)
*bool Refresh_NowbillModel(...)
*/
/*
*static void get_Rate_anpei_Priod(...)
*static void CP56Time2a_to_Time(...)
*static void Time_to_CP56Time2a(...)
*/
/*
*static int Anpei_is_leap_year(...) 
*static uint32_t Anpei_days_since_epoch(...) 
*static uint32_t Anpei_time_to_seconds(...)
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
 * @brief 	   判断是否为闰年
 *********************************************************************/	
static int Anpei_is_leap_year(int year) 
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
/********************************************************************
 * @brief 	 计算从 1970 年 1 月 1 日到指定日期的秒数
 *********************************************************************/	 
static uint32_t Anpei_days_since_epoch(int year, int month, int day) 
{
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t days = 0;

    // 计算从 1970 年到指定年份的天数
    for (int y = 1970; y < year; y++) {
        days += 365 + Anpei_is_leap_year(y);
    }

    // 计算从 1 月到指定月份的天数
    for (int m = 1; m < month; m++) {
        days += days_in_month[m - 1];
        if (m == 2 && Anpei_is_leap_year(year)) {
            days += 1; // 闰年 2 月多一天
        }
    }

    // 加上指定日期的天数
    days += day - 1;

    return days * 86400; // 每天 86400 秒
}

/********************************************************************
 * @brief 	  将时间数组转换为总秒数
 *********************************************************************/	
static uint32_t Anpei_time_to_seconds(U8 *timebill) 
{
    int year = (timebill[0] << 8) + timebill[1];
    int month = timebill[2];
    int day = timebill[3];
    int hour = timebill[4];
    int minute = timebill[5];
    int second = timebill[6];

    uint32_t total_seconds = Anpei_days_since_epoch(year, month, day);
    total_seconds += hour * 3600;    // 每小时 3600 秒
    total_seconds += minute * 60;    // 每分钟 60 秒
    total_seconds += second;

    return total_seconds;
}
/********************************************************************
 * @brief 	 置位/清除桩费率故障
 *********************************************************************/	
uint8_t FeemodelFalut_flg[GUN_NUM_MAX_ANPEI]={0};
void Set_FeemodelREE(uint8_t uport)
{
	dev_setErrExsit(uport,eErr_feemodelErr, __LINE__);
 	FeemodelFalut_flg[uport]=1;
}
 void Clear_FeemodelREE(uint8_t uport)
 {
	dev_clrErrExsit(uport,eErr_feemodelErr, __LINE__);
	FeemodelFalut_flg[uport]=0;

 }

 uint8_t FeeModel_errIixst(uint8_t uport)
 {
	return dev_getErrExsit(uport,eErr_feemodelErr);

 }

/********************************************************************
 * @brief 	 判断当前费率表真实有效
 * @param[in]	 (年H 年L 月 日 时 分 秒)
 * @return 	   
 * true false
 * 
 *********************************************************************/	
 bool ANPEI_Is_FeeModel_Valid( FeeModelB47 *FeeModelB47val)
 {
     if(FeeModelB47val->time_allnum > 12|| FeeModelB47val->time_allnum < 1) //计费模型数量
	   return false;

	 if(FeeModelB47val->start_time[6] >99) //年 2099
	   return false;   

	if(FeeModelB47val->start_time[5] >12||FeeModelB47val->start_time[5]==0) //月
	   return false;   

	if(FeeModelB47val->start_time[4] >31||FeeModelB47val->start_time[4]==0) //日
	   return false; 

	if(FeeModelB47val->start_time[3] >24) //时
	   return false; 

	if(FeeModelB47val->start_time[2] >60) //分
	   return false; 
	
	for(uint8_t i=0;i<FeeModelB47val->time_allnum;i++)
	{
	  if(FeeModelB47val->B47modeldata[i].Serial_rate>8||FeeModelB47val->B47modeldata[i].Serial_rate<1)
	   return false;

	}
	   

	   return true;
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
bool Refresh_NowbillModel(uint8_t port)
{
    //统一将费率时间段格式（毫秒L 毫秒H 分 时 日 月 年）转换成 (年H 年L 月 日 时 分 秒)
	uint8_t timebill_0[7] = {0};//(年H 年L 月 日 时 分 秒)
	uint8_t timebill_1[7] = {0};
    uint32_t SS=0;
	bool Value_enable[2]= {0}; //真实有效
	//A套
	CP56Time2a_to_Time(anpei_feeModel_save[port].FeemodelB47save_data[B47_A].start_time,timebill_0);
    Value_enable[B47_A]=ANPEI_Is_FeeModel_Valid(&anpei_feeModel_save[port].FeemodelB47save_data[B47_A]); //判断A数据是否有效
	 //B套
	CP56Time2a_to_Time(anpei_feeModel_save[port].FeemodelB47save_data[B47_B].start_time,timebill_1);
	Value_enable[B47_B]=ANPEI_Is_FeeModel_Valid(&anpei_feeModel_save[port].FeemodelB47save_data[B47_B]);//判断B数据是否有效

	
	uint8_t Update_local_Nomber=0;//理论上当前时间的费率属于 anpei_feeModel_save.FeemodelB47save_data[]的哪套下标
   
    if(false==Value_enable[B47_A]&&false==Value_enable[B47_B])//说明不存在费率表
	 {
		printf("not......search...billModel\r\n");
		return false;
	 }	     
	  else if(true==Value_enable[B47_A]&&false==Value_enable[B47_B]) //说明只有A费率表有效 
	    {
			//默认当前运行的费率ID只为这一套
			Update_local_Nomber=B47_A;
		}
		else if(false==Value_enable[B47_A]&&true==Value_enable[B47_B]) //说明只有rateB47save_data[1]费率表有效 
	    {
			Update_local_Nomber=B47_B;
		}
		else //两套均存在
		{

           //比较两套费率先后的费率的切换时间-转换成自 1970 年 1 月 1 日以来的总秒数
		   //秒数大----切换时间靠后
           uint32_t total_seconds_A = Anpei_time_to_seconds(timebill_0);
		   uint32_t total_seconds_B = Anpei_time_to_seconds(timebill_1);
           //获取当前时间 -转换成自 1970 年 1 月 1 日以来的总秒数
		   uint8_t timeOrgin[7] = {0};
		   getRunTimeYYMDHMS(timeOrgin); 
		   uint32_t total_seconds_Now=Anpei_time_to_seconds(timeOrgin);
           
		   //需要判断最近更新的费率的切换时间是否在另一套之前
		   //例如： 当前 24年12月 ，未更新前正在运行的是B，是24年11月开始切换；
		   // 新下发A是24年10月开始切换，切换时间早于B，此时应该选用A套费率 
		   if(B47_A==anpei_feeModel_save[port].RecentUpdates_Nomber)
		   {
                 if(total_seconds_A<total_seconds_B) 
				 {
					Update_local_Nomber=B47_A;

				 }
				 else if(total_seconds_A==total_seconds_B)//两套切换时间相同
				 {
					//后下发的优先
					Update_local_Nomber=B47_A;					
				 }
				 else if(total_seconds_A>total_seconds_B)
				 {
					if(total_seconds_Now>=total_seconds_A)//当前时间在rateB47save_data[0]的切换时间之后
					{
					  Update_local_Nomber=B47_A;
					} 
					else
					{
					  Update_local_Nomber=B47_B;
					}

				 }

		   }   

		   if(B47_B==anpei_feeModel_save[port].RecentUpdates_Nomber)
		    {
                
                if(total_seconds_B<total_seconds_A) 
				 {
					Update_local_Nomber=B47_B;

				 }
				 else if(total_seconds_A==total_seconds_B)//两套切换时间相同
				 {
					//后下发的优先
					Update_local_Nomber=B47_B;					
				 }
				 else if(total_seconds_B>total_seconds_A)
				 {
					if(total_seconds_Now>=total_seconds_B)//当前时间在rateB47save_data[1]的切换时间之后
					{
					  Update_local_Nomber=B47_B;
					} 
					else
					{
					  Update_local_Nomber=B47_A;
					}

				 }

		     }

		
		}

	   //更新理论上当前费率ID
		memcpy(&Now_billingmodel_and_num[port],&anpei_feeModel_save[port].FeemodelB47save_data[Update_local_Nomber].billing_model,8);
        Now_billingmodel_and_num[port][8]=Update_local_Nomber;
		Now_billingmodel_and_num[port][9]=anpei_feeModel_save[port].FeemodelB47save_data[Update_local_Nomber].time_allnum;

		 //打印Now_billingmodel_and_num[]的ID:[0]-[8] 
		printf(".......RefreshbillModelSUccess....port\r\n");

         return true;
}
/********************************************************************
 * @brief 	 返回当前正在运行的费率
 * @param[in]	 
 * @return 	  
 * 
 *********************************************************************/	
bool Read_rate_model_anpei(uint8_t port,uint8_t *pRate)
{
	if(false==Refresh_NowbillModel(port))
	return false;
	else
	 {	
		memcpy(pRate,&anpei_feeModel_save[port].RecentUpdates_Nomber,2*sizeof(FeeModel_Save_truct));

	 }
	 return true;

}

/********************************************************************
 * @brief 	 根据时间获取当前时间段所在的费率段序号
 * @param[in]	 port:枪口号
 *                a_or_b：套数 0-A套 1-B套
 *                hour:时 minute:分
 * @return 	   (resultNomber<<8)+X;
 *        //低字节 ratePeriod  0~3 尖峰平谷  4尖-  5峰-  6平- 7谷-
          //高字节 当前时段的序号数1~n
 *********************************************************************/	
uint16_t Now_model_get_RatePriod(uint8_t port,uint8_t a_or_b,uint8_t hour,uint8_t minute)
{
     uint8_t resultJFPG=0;
	 uint8_t resultNomber=0;
     uint8_t MaxNum=anpei_feeModel_save[port].FeemodelB47save_data[a_or_b].time_allnum;//时段数量

	 FeeModelB47 *pFeeMode_data=&anpei_feeModel_save[port].FeemodelB47save_data[a_or_b];

		 for(int i=0;i<MaxNum;i++)
		 {
             uint8_t StartBCD_HH=pFeeMode_data->B47modeldata[i].rate_start[1];//此处BCD码
			 uint8_t StartBCD_MM=pFeeMode_data->B47modeldata[i].rate_start[0];
			 uint8_t EndBCD_HH=pFeeMode_data->B47modeldata[i].rate_end[1];
			 uint8_t EndBCD_MM=pFeeMode_data->B47modeldata[i].rate_end[0];

			 uint8_t Start_HH=((StartBCD_HH>>4)&0x0f)*10+(StartBCD_HH&0xf);
			 uint8_t Start_MM= ((StartBCD_MM>>4)&0x0f)*10+(StartBCD_MM&0xf);
			 uint8_t End_HH= ((EndBCD_HH>>4)&0x0f)*10+(EndBCD_HH&0xf);
			 uint8_t End_MM=((EndBCD_MM>>4)&0x0f)*10+(EndBCD_MM&0xf);
           

		 
			  uint16_t timeInMinutes = hour * 60 + minute;
			  uint16_t startTimeInMinutes = Start_HH * 60 + Start_MM;
			  uint16_t endTimeInMinutes = End_HH * 60 + End_MM;

	          if (timeInMinutes >= startTimeInMinutes && timeInMinutes <= endTimeInMinutes)
			  {        resultNomber=i;
					
					     resultJFPG=pFeeMode_data->B47modeldata[i].Serial_rate;
                        break;

			  }
				 
			   
		 }
		 resultNomber=resultNomber+1;//(1~N)
       //此处 resultJFPG 1-4 尖峰平谷 5深谷 6尖扩展 7峰扩展 8平扩展
	   //转换成         0~3 尖峰平谷  4尖-  5峰-  6平-    7谷-
		 if(resultJFPG==1) return (resultNomber<<8)+0;
           else if(resultJFPG==2)return (resultNomber<<8)+1;
		   else if(resultJFPG==3)return (resultNomber<<8)+2;
		   else if(resultJFPG==4)return (resultNomber<<8)+3;
		   else if(resultJFPG==5)return (resultNomber<<8)+7;
		   else if(resultJFPG==6)return (resultNomber<<8)+4;
		   else if(resultJFPG==7)return (resultNomber<<8)+5;
		   else if(resultJFPG==8)return (resultNomber<<8)+6;
		   else
		     return 0;
	
	
}

/********************************************************************
 * @brief 	 返回当前的时段所处的费率的段号
 * @param[in]	 port:枪口号
 * @return 	ratePeriod_Nomber
 *        //低字节  0~3 尖峰平谷  4尖-  5峰-  6平- 7谷-
          //高字节 当前时段的序号数1~n
 *********************************************************************/	
uint16_t get_Rate_anpei_Priod(uint8_t u8Port)
{

	RATE_MODEL_T *prate = &g_cost_ctrl.rate_model[u8Port];

  //1.匹配当前运行的费率ID 是 A套 还是 B套 ，肯定有值
    uint8_t AorB= Now_billingmodel_and_num[u8Port][8];
           if(AorB==0xff)return 0; //说明没有时段费率，返回0

  //2.获取当前 时和分 匹配是该费率的哪一段
  uint8_t timeOrgin[7] = {0}; 
  getRunTimeYYMDHMS(timeOrgin); //(年H 年L 月 日 时 分 秒) 
  uint8_t Now_hour=timeOrgin[4];
  uint8_t Now_minute=timeOrgin[5];

  //高字节 当前时段的序号数1~n
  //低字节 ratePeriod  0~3 尖峰平谷  4尖-  5峰-  6平-    7谷-
  uint16_t ratePeriod_Nomber = Now_model_get_RatePriod(u8Port,AorB,Now_hour,Now_minute);
  
  //3.更新 RATE_MODEL_T 结构体中 price48_16u[] 和 sever48_16u[] ,elec_loss_ratio_32u默认0
  uint32_t Fee_value=0;
  //0.00001 电费
  Fee_value=fourUint8ToUint32(anpei_feeModel_save[u8Port].FeemodelB47save_data[AorB].B47modeldata[(ratePeriod_Nomber>>8)-1].ele_fee);
  prate->price48_16u[ratePeriod_Nomber&0xff] = Fee_value;//

  //0.00001 服务费
  Fee_value=fourUint8ToUint32(anpei_feeModel_save[u8Port].FeemodelB47save_data[AorB].B47modeldata[(ratePeriod_Nomber>>8)-1].ser_fee);
  prate->sever48_16u[ratePeriod_Nomber&0xff] = Fee_value;//
  
  prate->elec_loss_ratio_32u=0; 

  return (ratePeriod_Nomber);
}

/*****************更新存储的记录 **********************************************************************/
/*
*void anpei_packChgRecord(...)
*void ANpeiUpChargeRecordUpDealOffline(...)
*uint8_t ANpeiUpChargeStorageDeal(...)
*void anpei_packChgRecord_init(...)
*/
/********************************************************************
 * @brief 	 成功启动后 更新存储的记录结构体，需注意RecordB53 最多256个字节
 * @param[in]	 u8Port:枪口号
 * @return 	g_chgData[u8Port].DealRecord.ChgRecord //用于间断存储 和记录上传
 *********************************************************************/	
 void anpei_packChgRecord(uint8_t u8Port, RecordB53 *pRecord)
  {
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    
	memcpy(pRecord->transaction_log_num,chrg_EE_Money[u8Port].transaction_log_num,16);
	pRecord->FeeModel_timenum=Now_billingmodel_and_num[u8Port][9];
    
	uint8_t nomber=Now_billingmodel_and_num[u8Port][8];//A套 还是 B套
	uint8_t num=Now_billingmodel_and_num[u8Port][9];//时段个数
	//memcpy(pRecord->Fee_B53data,  anpei_feeModel_save[u8Port].FeemodelB47save_data[nomber].B47modeldata, Now_billingmodel_and_num[u8Port][9]*sizeof(Fee_data));
	uint64_t value_U64=0;
  for(uint8_t i=0;i<num;i++)
   {
    pRecord->Fee_B53data[i].time_serrnumber=anpei_feeModel_save[u8Port].FeemodelB47save_data[nomber].B47modeldata[i].Serial_number;
	pRecord->Fee_B53data[i].time_kind=anpei_feeModel_save[u8Port].FeemodelB47save_data[nomber].B47modeldata[i].Serial_rate;

    
	value_U64=pcostdata->PeriodElePower[i]/10;
	pRecord->Fee_B53data[i].chrg_totalpower[0]=value_U64&0Xff;
	pRecord->Fee_B53data[i].chrg_totalpower[1]=(value_U64>>8)&0Xff;
	pRecord->Fee_B53data[i].chrg_totalpower[2]=(value_U64>>16)&0Xff;

	value_U64=pcostdata->PeriodEleMoney[i]/100;
	pRecord->Fee_B53data[i].chrg_totalmoney[0]=value_U64&0Xff;
	pRecord->Fee_B53data[i].chrg_totalmoney[1]=(value_U64>>8)&0Xff;
	pRecord->Fee_B53data[i].chrg_totalmoney[2]=(value_U64>>16)&0Xff;

	value_U64=pcostdata->PeriodSerMoney[i]/100;
	pRecord->Fee_B53data[i].chrg_servemomey[0]=value_U64&0Xff;
	pRecord->Fee_B53data[i].chrg_servemomey[1]=(value_U64>>8)&0Xff;
	pRecord->Fee_B53data[i].chrg_servemomey[2]=(value_U64>>16)&0Xff;
   }
	
	//在 ChargingRecordUpdateScan()会实时更新
	//需要转成CP56Time2a格式
	uint8_t timeCP56Time[7]={0};
	Time_to_CP56Time2a(pChgGunData->chrg_start_time,timeCP56Time);
	memcpy(pRecord->chrg_start_time,timeCP56Time,7);

	Time_to_CP56Time2a(pChgGunData->chrg_stop_time,timeCP56Time);
	memcpy(pRecord->chrg_stop_time, timeCP56Time,7);
   
	

	//充电时间
	uint8_t time_min_arryvalue[2]={0};
	uint32_t  time_s= GetPile_ChgTimer(u8Port);
	Uint16ToTwoUint8(time_min_arryvalue,(time_s)/60);
    memcpy(pRecord->chrg_chrg_totaltime, time_min_arryvalue,2);

	uint32_t SUM_power=0;
	uint32_t SUM_sver_money=0;
	uint32_t SUM_chrg_money=0;


   for(uint8_t i=0;i<num;i++)
   { 
	
	SUM_chrg_money=SUM_chrg_money+pcostdata->PeriodEleMoney[i]/100;
	SUM_sver_money=SUM_sver_money+pcostdata->PeriodSerMoney[i]/100;
	SUM_power=SUM_power+pcostdata->PeriodElePower[i]/10;

   }

   pRecord->chrg_total_money[0]=SUM_chrg_money&0Xff;
   pRecord->chrg_total_money[1]=(SUM_chrg_money>>8)&0Xff;
   pRecord->chrg_total_money[2]=(SUM_chrg_money>>16)&0Xff;

   pRecord->sver_total_money[0]=SUM_sver_money&0Xff;
   pRecord->sver_total_money[1]=(SUM_sver_money>>8)&0Xff;
   pRecord->sver_total_money[2]=(SUM_sver_money>>16)&0Xff;

   pRecord->total_total_power[0]=SUM_power&0Xff;
   pRecord->total_total_power[1]=(SUM_power>>8)&0Xff;
   pRecord->total_total_power[2]=(SUM_power>>16)&0Xff;

	// memcpy(pRecord->chrg_total_money, chrg_EE_Money[u8Port].total_chrg_EEmomey,3);
	// memcpy(pRecord->sver_total_money, chrg_EE_Money[u8Port].total_chrg_severmomey,3);

	// memcpy(pRecord->total_total_power, chrg_EE_Money[u8Port].total_EE,3);

	//在 ChargingRecordUpdateScan()会实时更新pChgGunData->total_start_elec/total_stop_elec
	uint8_t arry[4]={0};
	uint32ToFourUint8(arry,(pChgGunData->total_start_elec)/10);
	memcpy(pRecord->total_start_power,arry ,4);


	// uint32ToFourUint8(arry,(pChgGunData->total_stop_elec)/10);
	// memcpy(pRecord->total_stop_power, arry ,4);
	U32 total_stop_elec_anpei=(pChgGunData->total_start_elec)/10+SUM_power;
	uint32ToFourUint8(arry,total_stop_elec_anpei);
	memcpy(pRecord->total_stop_power, arry ,4);

	pRecord->soc_start[0]=0;
	pRecord->soc_start[1]=0;
	pRecord->soc_end[0]=0;
	pRecord->soc_end[1]=0;

	//平台交易流水后16 位编码，用做在线交易记录中的物理卡号
	memcpy(pRecord->Logic_card_number, &chrg_EE_Money[u8Port].transaction_log_num[0],8);
	//电动汽车唯一标识
	memset(pRecord->Car_onlycode,0,32);

   //停止原因 在Update_stopReason（）实时更新，需要根据协议继续划分 
    uint8_t stop_reason_UP=pChgGunData->DealRecord.PileStopReason;

    uint16_t AnPei_stopreason=0;

 
    //B53停止原因
	//是充电桩启动过程中的停止存在故障 100-143
	if(eChargeState_Waiting==GetPile_gun_state(u8Port)||eChargeState_Starting==GetPile_gun_state(u8Port))
	{

	if(1 == dev_getErrExsit(u8Port, eErr_EmergencyStop))AnPei_stopreason=102;  //急停
		else if(1== dev_getErrExsit(u8Port, eErr_InputOverVol)) AnPei_stopreason=109;  //过流
		else if(1== dev_getErrExsit(u8Port, eErr_OutputOverCurr)) AnPei_stopreason=109;  //过压
		else if(1== dev_getErrExsit(u8Port,  eErr_AphaseInputLessVol)) AnPei_stopreason=109;  //欠压
		else if(1== dev_getErrExsit(u8Port, eErr_CpVoltAbnor)) AnPei_stopreason=100;  //CP异常
		else if(1== dev_getErrExsit(u8Port, eErr_CpGroundFault)) AnPei_stopreason=100;  //cp接地
		else if(1== dev_getErrExsit(u8Port, eErr_GunOverTempErr)) AnPei_stopreason=111;  //枪过温
		else if(1== dev_getErrExsit(u8Port, eErr_EnvOverTempWarn)) AnPei_stopreason=110;  //环境过温
		else if(1== dev_getErrExsit(u8Port, eErr_JcqMaloperation)) AnPei_stopreason=107;  //交流输出接触器误动拒动
		else if(1== dev_getErrExsit(u8Port, eErr_JcqSynechiaFault)) AnPei_stopreason=108;  //交流输出接触器粘连
		else if(1== dev_getErrExsit(u8Port, eErr_PEBreakFault)) AnPei_stopreason=113; // PE接地故障
		else if(1== dev_getErrExsit(u8Port, eErr_LeakageCurrErr)) AnPei_stopreason=106; //漏电流故障
		else if(1== dev_getErrExsit(u8Port, eErr_DiodeStop)) AnPei_stopreason=50; //二极管
		else if(1== dev_getErrExsit(u8Port, eErr_ShortCircleErro)) AnPei_stopreason=126; //短路保护
		else if(1== dev_getErrExsit(u8Port, eErr_GunDisConnErr)) AnPei_stopreason=100; //充电中控制导引故障
		else if(1== dev_getErrExsit(u8Port, eSrc_ManualCtrlStop))AnPei_stopreason=104; //主动控制停止
		else
		  AnPei_stopreason=139;//其他故障代替

	}
	else if(eChargeState_Charging==GetPile_gun_state(u8Port)||eChargeState_PauseB==GetPile_gun_state(u8Port))
	{
		AnPei_stopreason=312;

	}
	else  
	{
		//充电中停止
		//只需要列举在Update_stopReason（）中提到的即可
		 if(stop_reason_UP==Pile_Stop_Reason_Leak)AnPei_stopreason=21;//漏电
		  else if(stop_reason_UP==Pile_Stop_Reason_EStop)AnPei_stopreason=17;//急停
		  else if(stop_reason_UP==Pile_Stop_Reason_CPGnd)AnPei_stopreason=14;//CP接地,充电中控制导引故障 
		  else if(stop_reason_UP==Pile_Stop_Reason_CPErro)AnPei_stopreason=14;//CP异常,充电中控制导引故障 
		  else if(stop_reason_UP==Pile_Stop_Reason_VolOver)AnPei_stopreason=24;//总充电电压异常
		  else if(stop_reason_UP==Pile_Stop_Reason_VolUnder)AnPei_stopreason=24;//总充电电压异常
		  else if(stop_reason_UP==Pile_Stop_Reason_CrtUnder)AnPei_stopreason=24;//总充电电流异常
		  else if(stop_reason_UP==Pile_Stop_Reason_RlySyn)AnPei_stopreason=23;//接触器粘连
		  else if(stop_reason_UP==Pile_Stop_Reason_AirTempOver)AnPei_stopreason=25;//温度异常
		  else if(stop_reason_UP==Pile_Stop_Reason_GunTempOver)AnPei_stopreason=25;//温度异常
		  else if(stop_reason_UP==Pile_Stop_Reason_PlugTempOver)AnPei_stopreason=25;//温度异常
		  else if(stop_reason_UP==Pile_Stop_Reason_CarOk)AnPei_stopreason=1;//充电完成
		  else if(stop_reason_UP==Pile_Stop_Reason_GunBreak)AnPei_stopreason=1;//按充电中拔枪按充满来计算
		  else if(stop_reason_UP==Pile_Stop_Reason_StartDiode)AnPei_stopreason=50;//二极管
		  else if(stop_reason_UP==Pile_Stop_Reason_APP)AnPei_stopreason=3;//APP停止
		  else if(stop_reason_UP==Pile_Stop_Reason_OverBalance)AnPei_stopreason=301;	//金额不够
		  else if(stop_reason_UP==Pile_Stop_Reason_S2TimeOut)AnPei_stopreason= 51;	//原320车端长期未反应,修改为51 BMS 正常终止充电
		  else if(stop_reason_UP==Pile_Stop_Reason_Ele)AnPei_stopreason= 321;//电表数据异常
		  else if(stop_reason_UP==Pile_Stop_Reason_EleCommFault)AnPei_stopreason= 326;//电表通信中断
		  else if(stop_reason_UP==Pile_Stop_Reason_RlyRfs)AnPei_stopreason= 22;//据动
		  else if(stop_reason_UP==Pile_Stop_Reason_StopKey)AnPei_stopreason=2;//按键
		  else if(stop_reason_UP==Pile_Stop_Reason_MaxTime)AnPei_stopreason=206;//充电桩处于暂停
		  else if(stop_reason_UP== Pile_Stop_Reason_PEGnd) AnPei_stopreason=113; // PE接地故障
		else 
		AnPei_stopreason=54;


	}
   
    if(AnPei_stopreason>=100)
	{
		pRecord->stop_reason[0]=((((AnPei_stopreason%100)/10)<<4))+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=AnPei_stopreason/100;
	}
	else if(AnPei_stopreason<100)
	{
		pRecord->stop_reason[0]=((AnPei_stopreason/10)<<4)+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=0;
	}
	else if(AnPei_stopreason>=1000)
	{

		pRecord->stop_reason[0]=((AnPei_stopreason/10)<<4)+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=((AnPei_stopreason/1000)<<4)+((AnPei_stopreason%100)/10);
	}

	// pRecord->stop_reason[0]=AnPei_stopreason&0xff;  
	// pRecord->stop_reason[1]=(AnPei_stopreason>> 8) & 0x00ff;

 }
 /********************************************************************
 * @brief 	  充电阶段前初始化数据 
 * @param[in]	u8Port ，
 *              RecordB53 上传记录的结构体
 * @return 	
 *********************************************************************/	
 void anpei_packChgRecord_init(uint8_t u8Port,RecordB53 *pRecord)
 {
		memset(pRecord->transaction_log_num,0,sizeof(RecordB53));	

 }
  /********************************************************************
 * @brief 	 充电启动失败初始化指定数据
 * @param[in]	u8Port ，
 * 				transaction_log 交易流水号
 *              stopenreason：充电启动失败原因
 *              RecordB53 上传记录的结构体
 * @return 	
 *********************************************************************/	
 void anpei_packChgRecord_startFailinitData(uint8_t u8Port,uint8_t *transaction_log ,uint8_t stopenreason, RecordB53 *pRecord)
 {
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    
	memcpy(pRecord->transaction_log_num,transaction_log,16);

	
	uint8_t nomber=0;//A套 还是 B套
	uint8_t num=0;//时段个数

	if(Now_billingmodel_and_num[u8Port][9]==0xff)//不存在费率
	{
		pRecord->FeeModel_timenum=1;
		  nomber=1; //A套 还是 B套
	      num=1;    //时段个数

	}
	else
	{
		   pRecord->FeeModel_timenum=Now_billingmodel_and_num[u8Port][9];
		  nomber=Now_billingmodel_and_num[u8Port][8];//A套 还是 B套
		  num=Now_billingmodel_and_num[u8Port][9];//时段个数
	}

	  for(uint8_t i=0;i<num;i++)
	   {
		pRecord->Fee_B53data[i].time_serrnumber=anpei_feeModel_save[u8Port].FeemodelB47save_data[nomber].B47modeldata[i].Serial_number;
		pRecord->Fee_B53data[i].time_kind=anpei_feeModel_save[u8Port].FeemodelB47save_data[nomber].B47modeldata[i].Serial_rate;
		pRecord->Fee_B53data[i].chrg_totalpower[0]=0;
		pRecord->Fee_B53data[i].chrg_totalpower[1]=0;
		pRecord->Fee_B53data[i].chrg_totalpower[2]=0;
		pRecord->Fee_B53data[i].chrg_totalmoney[0]=0;
		pRecord->Fee_B53data[i].chrg_totalmoney[1]=0;
		pRecord->Fee_B53data[i].chrg_totalmoney[2]=0;
		pRecord->Fee_B53data[i].chrg_servemomey[0]=0;
		pRecord->Fee_B53data[i].chrg_servemomey[1]=0;
		pRecord->Fee_B53data[i].chrg_servemomey[2]=0;
	   }
   
	
	//在 ChargingRecordUpdateScan()会实时更新
	//需要转成CP56Time2a格式
	uint8_t timeCP56Time[7]={0};
	uint8_t timeTime[7]={0};//正常格式
	getRunTimeYYMDHMS(timeTime); 
	Time_to_CP56Time2a(timeTime,timeCP56Time);
	memcpy(pRecord->chrg_start_time,timeCP56Time,7);

	//Time_to_CP56Time2a(timeTime,timeCP56Time);
	memcpy(pRecord->chrg_stop_time, timeCP56Time,7);
   
	//充电时间
	uint8_t arryvalue[4]={0};
    memcpy(pRecord->chrg_chrg_totaltime, arryvalue,2);
	memcpy(pRecord->chrg_total_money, arryvalue,3);
	memcpy(pRecord->sver_total_money, arryvalue,3);
	memcpy(pRecord->total_total_power, arryvalue,3);
	memcpy(pRecord->total_start_power,arryvalue ,4); 
	memcpy(pRecord->total_stop_power, arryvalue ,4);

	pRecord->soc_start[0]=0;
	pRecord->soc_start[1]=0;
	pRecord->soc_end[0]=0;
	pRecord->soc_end[1]=0;

	//平台交易流水后16 位编码，用做在线交易记录中的物理卡号
	memcpy(pRecord->Logic_card_number, &transaction_log[0],8);
	//电动汽车唯一标识
	memset(pRecord->Car_onlycode,0,32);

  

    uint16_t AnPei_stopreason=235;
	if(stopenreason==eUP_Start_Fail_DevErr_ANPEI||stopenreason==eUP_Start_Fail_Offline_ANPEI)
	{
		//JJUNIVE 200-236
	if(1 == dev_getErrExsit(u8Port, eErr_EmergencyStop))AnPei_stopreason=208;  	//急停
		else if(1== dev_getErrExsit(u8Port, eErr_InputOverVol)) AnPei_stopreason=232;  //过流
		else if(1== dev_getErrExsit(u8Port, eErr_OutputOverCurr)) AnPei_stopreason=230;  //过压
		else if(1== dev_getErrExsit(u8Port,  eErr_AphaseInputLessVol)) AnPei_stopreason=231;  //欠压
		else if(1== dev_getErrExsit(u8Port, eErr_CpVoltAbnor)) AnPei_stopreason=100;  //CP异常
		else if(1== dev_getErrExsit(u8Port, eErr_CpGroundFault)) AnPei_stopreason=100;  //cp接地
		else if(1== dev_getErrExsit(u8Port, eErr_GunOverTempErr)) AnPei_stopreason=213;  //枪过温
		else if(1== dev_getErrExsit(u8Port, eErr_EnvOverTempWarn)) AnPei_stopreason=212;  //环境过温
		else if(1== dev_getErrExsit(u8Port, eErr_JcqMaloperation)) AnPei_stopreason=221;  //交流输出接触器误动拒动
		else if(1== dev_getErrExsit(u8Port, eErr_JcqSynechiaFault)) AnPei_stopreason=222;  //交流输出接触器粘连
		else if(1== dev_getErrExsit(u8Port, eErr_PEBreakFault)) AnPei_stopreason=113; // PE接地故障
		else if(1== dev_getErrExsit(u8Port, eErr_LeakageCurrErr)) AnPei_stopreason=106; //漏电流故障
		else if(1== dev_getErrExsit(u8Port, eErr_DiodeStop)) AnPei_stopreason=50; //二极管
		else if(1== dev_getErrExsit(u8Port, eErr_ShortCircleErro)) AnPei_stopreason=233; //短路保护
		else
		  AnPei_stopreason=235;//其他故障代替

	}
	else if(stopenreason==eUP_Start_Fail_Working_ANPEI) //枪已在充电
	{

		AnPei_stopreason=205;//充电桩处于“工作”状态

	}
	else if(stopenreason==eUP_Start_Fail_NoConn_ANPEI) //未插枪
	{

		AnPei_stopreason=207;//充电桩与车辆处于未连接状态

	}
	else if(stopenreason==eUP_Start_Fail_Reconnect_ANPEI)//超时不可启动
	{

		AnPei_stopreason=206;//充电桩处于“暂停”状态

	}
	else 
	{

		AnPei_stopreason=235;

	}
	
 
	if(AnPei_stopreason>=100)
	{
		pRecord->stop_reason[0]=((((AnPei_stopreason%100)/10)<<4))+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=AnPei_stopreason/100;
	}
	else if(AnPei_stopreason<100)
	{
		pRecord->stop_reason[0]=((AnPei_stopreason/10)<<4)+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=0;
	}
	else if(AnPei_stopreason>=1000)
	{

		pRecord->stop_reason[0]=((AnPei_stopreason/10)<<4)+(AnPei_stopreason%10);
		pRecord->stop_reason[1]=((AnPei_stopreason/1000)<<4)+((AnPei_stopreason%100)/10);
	}

	// pRecord->stop_reason[0]=AnPei_stopreason&0xff;  
	// pRecord->stop_reason[1]=(AnPei_stopreason>> 8) & 0x00ff;


 }
 /********************************************************************
 * @brief 	 读取最新的记录并判断是否上传成功
 * @param[in]	
 * @return 	
 *********************************************************************/	
 void ANpeiUpChargeRecordUpDealOffline(void)
 {
	RecordB53 tempRecord;
	 memset(&tempRecord,0,sizeof(RecordB53));
    U32 u32i = 0, temp = 0;

	for (uint8_t i = 0; i < GUN_NUM_ANPEI; i++ ) {
		uint8_t uGun = i;

		uint8_t ret = UpChargeRecordUpDealOffline(i);
        if (ret == FALSE) {
            continue;
        }

        uint8_t cmd = ANPEI_S_onlineEnd_ChgInfB53;
    
		//正在上报时不查记录
		if(SEND_ENABLE_ON == GetSendEnable(uGun, cmd)) {
			printf("ANPEIUpChargeRecordUpDealOffline gun = %d SEND_ENABLE_ON\r\n", uGun);
			continue;
		}

	    RecordB53 *UpRecord = &g_chgData[uGun].DealRecord.ChgRecord.AnpeiChgRecord;
		CHG_DATA_T *pChgGunData = &g_chgData[uGun];


		memcpy(&tempRecord, UpRecord,  sizeof(RecordB53));
		
		uint16_t StopVal= (tempRecord.stop_reason[1]<<8)+tempRecord.stop_reason[0];
		printf("ANPEIUpChargeRecordUpDealOffline gun = %d stop_reason =  %d\r\n", uGun, StopVal);

		if(StopVal==0)return;
		if(tempRecord.FeeModel_timenum>12||tempRecord.FeeModel_timenum==0)return;
		if(StopVal!=0xffff)
		{
		 	SetSendEnable(uGun, cmd, SEND_ENABLE_ON);
		    Send_Immediately(uGun, cmd);
			pChgGunData->upDealCnt = 0;
		}
	
    	
	}

    return ;

 }
/********************************************************************
 * @brief 	写入记录，在收到平台下发的B14 后调用
 * @param[in] 	u8Port 枪号
 *              *deal 记录的数据
 *              len  记录数据长度（默认直接写是0xFF即可） 	   
 *********************************************************************/	
 uint8_t ANpeiUpChargeStorageDeal(uint8_t u8Port, void *deal, uint8_t len)
{
    DealData_write(u8Port, (uint8_t *)&deal, sizeof(PlatDealRecord));
    return TRUE;
}


/****************枪状态**************************************************************************/

/********************************************************************
 * @brief 	更新枪状态
 * @param[in] 	u8Port 枪号
 *              
 *********************************************************************/	
void ANPEIUpGunStateCheck(uint8_t u8Port)
{
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	static uint8_t gun_state[GUN_NUM_MAX_ANPEI] = {0};
	static uint8_t gun_conn_state[GUN_NUM_MAX_ANPEI] = {0};
	uint8_t report_flag[GUN_NUM_MAX_ANPEI] = {0};

	if (logic_get_gun_charging(u8Port))
	{
		pUpGunData->up_gun_state = eUP_Gun_State_Work_anpei;
	}
	else
	{
		if (TRUE == dev_getErrState(u8Port)||0!=FeeModel_errIixst(u8Port)) //
		{
			pUpGunData->up_gun_state = eUP_Gun_State_Err_anpei;
		}
		else
		{
			if (TRUE == GetPile_gun_connect(u8Port))
			{
				if (eChargeState_StopFinish == logic_get_gun_state(u8Port))
					pUpGunData->up_gun_state = eUP_Gun_State_Finish_anpei;
				else
				{
					if (chrg_EE_Money[u8Port].start_waykind == 6) // 定时状态必须是已插枪
						pUpGunData->up_gun_state = eUP_Gun_State_Fixtime_anpei;
					else
						pUpGunData->up_gun_state = eUP_Gun_State_Conn_anpei;
				}
			}
			else
				pUpGunData->up_gun_state = eUP_Gun_State_Idle_anpei;
		}
	}

	if (gun_state[u8Port] != pUpGunData->up_gun_state)
	{
		gun_state[u8Port] = pUpGunData->up_gun_state;
		report_flag[u8Port] = TRUE;
	}

	if (gun_conn_state[u8Port] != GetPile_gun_connect(u8Port))
	{
		gun_conn_state[u8Port] = GetPile_gun_connect(u8Port);
		report_flag[u8Port] = TRUE;
	}

	if (TRUE == report_flag[u8Port]&&B1_first_aftre_online_flag==1)//已经登录，确保首次发送B1信息是重新登录时，解决上线首次会跳到这一步发送
	{
		SetSendEnable(u8Port, ANPEI_S_RealData, SEND_ENABLE_ON);
		Send_Immediately(u8Port, ANPEI_S_RealData);
	}

	return;
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
uint8_t anpei_monitor_charge_start(uint8_t u8Port, uint8_t *up_fail_reason, uint8_t trade_flag, uint8_t *pCardNo, uint8_t *pTrdNum, uint32_t *BlMoney,uint8_t *starttime,uint8_t *endtime)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
   GN_PLATMOD  *ltcpCharge = &sg_platmod;

   //启动失败回复B53 未回B54
   SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_OFF);
	//开始充电，清零停止原因
	stopPileCharge(u8Port, Pile_Stop_Reason_None);

	//一次充电完成后不允许再次开启充电，需要重新插拔枪
	// if(ltcpCharge->gun[u8Port].gunRtInfo.gun_ChrgSta == eChargeState_StopFinish)
	// {
	// 	up_fail_reason[0] = eUP_Start_Fail_Reconnect_ANPEI;
	// 	return FALSE;
	// }

	if (ltcpCharge->gun[u8Port].gunRtInfo.u_platChrgsta.bit.charging_1b) {
		up_fail_reason[0] = eUP_Start_Fail_Working_ANPEI;
		return FALSE;
	}

	if(eMonitorState_Auth == monitor_get_MonitorState(u8Port))//
	{
		up_fail_reason[0] = eUP_Start_Fail_Working_ANPEI;
		return FALSE;
	}

	if(eMonitorState_Forbid == monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Offline_ANPEI;
		return FALSE;
	}
	
	if(eMonitorState_UpState == monitor_get_MonitorState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_Offline_ANPEI;
		return FALSE;
	}

	if(eMonitorState_Service != monitor_get_MonitorState(u8Port)) //
	{
		up_fail_reason[0] = eUP_Start_Fail_Working_ANPEI;
		return FALSE;
	}
	
	if(TRUE == dev_getErrState(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_DevErr_ANPEI;
		return FALSE;
	}
	
	if(TRUE != GetPile_gun_connect(u8Port))
	{
		up_fail_reason[0] = eUP_Start_Fail_NoConn_ANPEI;
		return FALSE;
	}

	//判断当前是否有计费模型(B2或者B47类型)，
	if(false==Refresh_NowbillModel(u8Port))
	{
      
        up_fail_reason[0] = eUP_Start_Fail_DevErr_ANPEI;
        printf("monitor_charge_start---计费模型异常\r\n");
		return FALSE;  
	}


	getRunTimeYYMDHMS(pChgGunData->chrg_start_time);

	// if(TRUE == logic_charge_start(u8Port))
	{
		memset(pChgGunData->transaction_log_num, 0, GNDATA_TRDNUM_LEN);
		pChgGunData->sum_balance = 0;
        pChgGunData->ExistChargeDeal = 1;   //订单开始
		


	    pChgGunData->trade_flag = trade_flag;//交易标识

		//更新记录数据结构体 pChgGunData 和 chrg_EE_Money_struct()数据
		uint8_t val[1]={0};
		val[0]=trade_flag;
		Refresh_chrg_EE_Money_data(u8Port,No_start_way, val,1);	

		if(NULL != pCardNo)
		{	 	
		 memcpy(pChgGunData->Auth_card_number, pCardNo, 8);
		 Refresh_chrg_EE_Money_data(u8Port,No_anpei_card_number, pChgGunData->Auth_card_number,8);
		}		

		if(NULL != pTrdNum)
		{
            memcpy(pChgGunData->transaction_log_num, pTrdNum, GNDATA_TRDNUM_LEN);
			Refresh_chrg_EE_Money_data(u8Port,No_transaction_log_num,pChgGunData->transaction_log_num,16 );
		}
			
		if(NULL != BlMoney)
		{
            pChgGunData->sum_balance = BlMoney[0];
			uint8_t arry[4]={0};
			uint32ToFourUint8(arry,pChgGunData->sum_balance);
			Refresh_chrg_EE_Money_data(u8Port,No_controlfee,arry,4);

		}

		if (NULL!=starttime) // 若是定时启动更新开始和结束时间
		{
			Refresh_chrg_EE_Money_data(u8Port, No_start_time, starttime, 7);
		}

		if(NULL!=endtime)
		{
			Refresh_chrg_EE_Money_data(u8Port, No_end_time, endtime, 7);

		}

		return TRUE;
	}
	

}


/*********安培协议1.41收发**************************************************************************/
//注意: 桩号3100000000010019----------报文里是反位 1900010000000031

/***********************************************************************
*@brief 协议帧
【上行】：68 19 00 01 00 00 00 00 31 00 00
【下行】：68 01 19 00 01 00 00 00 00 31 00 00
*************************************************************************/

/********************************************************************
 * @brief 	   比较平台下发的桩编号（8个字节存储的16位）跟桩内编号（16个字节存储的16位）比较
 * @param[in]   *number1：0X31...0X19； 
 *              *numb2：平台返回的 0X19...0X31
 * @return 	   相同返1，不同返0
 *********************************************************************/	
// 桩号：3100000000010019----------报文里是1900010000000031
bool Compare_DeviceNumber_pile_plat(uint8_t *number1, uint8_t *number2)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (number1[i] != number2[7 - i])
			return false;
	}

	return true;
}

// 协议帧发 //680119000100000000310000
uint16_t send_Identification_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};

	uint8_t *data = (uint8_t *)pdata;
	uint16_t datalen = 0;

	data[datalen++] = 0x68;
	data[datalen++] = 0x01;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[datalen++] = plieNumber[7-k];
	}

	data[datalen++] = 0;
	data[datalen++] = 0;

	return datalen;
}

void send_Identification_anpei_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_Identification))
	{
		SetRecvEnable(u8Port, ANPEI_R_Identification, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_Identification, Get_Systick());
	}

	return;
}

// 协议帧收 //680119000100000000310000
uint8_t recv_login_data_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvIdenf;

	if (u8Port >= GUN_NUM_MAX_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 2, sizeof(ANPEI_Recv_Identification));

	return TRUE;
}

void recv_login_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_Identification *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvIdenf;

	uint8_t plieNumber[8] = {0};
	AnpeiGet_PlatNumberBCD(plieNumber);
	// 判断跟桩编号是否一致
 	if (true == Compare_DeviceNumber_pile_plat(plieNumber, (uint8_t *)pRecvIdenf))
 	{
		SetSendEnable(u8Port, ANPEI_S_Identification, SEND_ENABLE_OFF);

 	}

	 ANPEI_Recv_TimeSyn *pRecvSYn = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvTimeSyn;
	 pRecvSYn->first_flag=0;
	 ANPEI_Recv_SIMinf_up *PRecv_siminf = &g_ProtocolDCB.pANPEIRecvData[0].strRecvSIMinf_up;
	 PRecv_siminf->receiveflag=0;
    
	return;
}

/*****************************************************************
 * @brief  U帧
【上行】：68 04 07 00 00 00
【下行】：68 04 0B 00 00 00
******************************************************************/
// U帧发送 680407000000
uint16_t send_U_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;
	uint16_t i = 0;
	data[i++] = 0x68;
	data[i++] = 0x04;
	data[i++] = 0x07;
	data[i++] = 0x00;
	data[i++] = 0;
	data[i++] = 0;
	return i;
}

void send_U_anpei_Succ(uint8_t u8Port)
{

	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_U))
	{
		SetRecvEnable(u8Port, ANPEI_R_U, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_U, Get_Systick());
	}

	return;
}
// U帧认证应答解析
uint8_t recv_U_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_U *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvU;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 2, sizeof(ANPEI_Recv_U));

	return TRUE;
}

void recv_U_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_U *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvU;

	if (0x0B == pRecvIdenf->val)
	{
		SetSendEnable(u8Port, ANPEI_S_U, SEND_ENABLE_OFF);
	}

	return;
}

/*****************************************************************
 * @brief  心跳帧发送
【上行】：68 04 43 00 00 00
【下行】：68 04 83 00 00 00
******************************************************************/
// 心跳帧发送 68 04 43 00 00 00
uint16_t send_Heart_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = 0;

	data[data_len++] = 0x68;
	data[data_len++] = 0x04;

	data[data_len++] = 0x43;
	data[data_len++] = 0x00;
	data[data_len++] = 0;
	data[data_len++] = 0;

	return data_len;
}

void send_Heart_anpei_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_Heart))
	{
		SetRecvEnable(u8Port, ANPEI_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_Heart, Get_Systick());
	}

	return;
}
// 心跳认证应答解析
uint8_t recv_R_Heart_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_Heart *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvHeart;

	if (u8Port >= GUN_NUM_MAX_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 2, sizeof(ANPEI_Recv_Heart));

	return TRUE;
}

void recv_R_Heart_anpei_Succ(uint8_t u8Port)
{
    PlatHeartTickRefresh();

	Set_PlatConnectSta(eOnline_Heart);

	dev_clrErrExsit_all(eErr_PlatformOffline, __LINE__);

	if(RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_Heart))
	{
		SetRecvEnable(u8Port, ANPEI_R_Heart, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_Heart, Get_Systick());
	}

	return;
}

/*****************************************************************
*@brief  对时 
首次上线对时-->校时-->根据时间刷新当前理论费率表->上传SIM卡信息B31	   
******************************************************************/
// 对时收
void Cp56time2a_Set_TimeANPEI(cp56timeanpei *pCp56)
{
	tm_struct SysTime;
	tm_struct *pSysTime = &SysTime;

	pSysTime->yearH = 20;
	pSysTime->yearL = pCp56->Year;
	pSysTime->month = pCp56->Month;
	pSysTime->day = pCp56->Date;
	pSysTime->hour = pCp56->Hour;
	pSysTime->minute = pCp56->Minute;
	pSysTime->second = twoUint8ToUint16(pCp56->MilliSec) / 1000;

	setCurrentRunTime((uint8_t *)pSysTime);

	return;
}

uint8_t recv_clocksyn_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvTimeSyn;

	if (u8Port >= GUN_NUM_MAX_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 14, 7);


	return TRUE;
}
void recv_clocksyn_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_TimeSyn *pRecvTimeSyn = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvTimeSyn;

	Cp56time2a_Set_TimeANPEI(&pRecvTimeSyn->cur_time);

	SetSendEnable(u8Port, ANPEI_S_clocksyn, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_clocksyn);

	return;
}
//	对时发 68 13 00 00 00 00 67 00  07  00 00 00 00 00   FF B9 0D 0B 1D 02 14
uint16_t send_clocksyn_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	ANPEI_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvTimeSyn;

	uint8_t *data = (uint8_t *)pdata;

	uint16_t data_len = 0;
	uint8_t new_data[] = {0x68, 0x13, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,0x00};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	data[data_len++] = pRecvIdenf->cur_time.MilliSec[0];
	data[data_len++] = pRecvIdenf->cur_time.MilliSec[1];
	data[data_len++] = pRecvIdenf->cur_time.Minute;
	data[data_len++] = pRecvIdenf->cur_time.Hour;
	data[data_len++] = pRecvIdenf->cur_time.Date;
	data[data_len++] = pRecvIdenf->cur_time.Month;
	data[data_len++] = pRecvIdenf->cur_time.Year;

	return data_len;
}

void send_clocksyn_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_TimeSyn *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvTimeSyn;
    

	SetSendEnable(u8Port, ANPEI_S_clocksyn, SEND_ENABLE_OFF);
	if (0 == pRecvIdenf->first_flag) // 首次上线对时
	{
		pRecvIdenf->first_flag = 1;
		for(uint8_t i=0;i<GUN_NUM;i++)//首次上线对时 后刷新费率
		{
			if(true==  Refresh_NowbillModel(i))
		    {
			   printf("First online refresh..port:%d.billModel,\r\n",i);

			}
			else
			 {
				//置位桩费率故障
                Set_FeemodelREE(i);
				printf("First online fail refresh..port:%d.billModel,case feeModel Err \r\n",i);
			 }


		}

		SetSendEnable(u8Port, ANPEI_S_SIMInfAck, SEND_ENABLE_ON);
		Send_Immediately(u8Port, ANPEI_S_SIMInfAck);
	}

	return;
}


/*****************************************************************
*@brief  B1充电过程实时监测数据 
        空闲2min 发送一次;
		充电30s 发送一次;
		状态变换发送一次(非协议里提及的);
******************************************************************/
uint16_t send_RealData_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{

	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;

	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	uint16_t data_len = 0;
	uint8_t new_data[] = {
		0x68,
		0xA6,
		0x00,
		0x00,
		0x00,
		0x00,
		0x86,
		0x00,
		0x06,
		0x00,
		0x00,
	};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7 - k];
	}

	data[data_len++] = u8Port; // 充电接口标识

	if (TRUE == GetPile_gun_connect(u8Port))
		data[data_len++] = 0x01; // 连接确认开关状态
	else
		data[data_len++] = 0x00;

	// 工作状态BCD码 2Byte
	if (pUpGunData->up_gun_state == eUP_Gun_State_Offline_anpei)
	{
		data[data_len++] = 0;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		data[data_len++] = 0x01;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Idle_anpei)
	{
		data[data_len++] = 0x02;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Work_anpei)
	{
		data[data_len++] = 0x03;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_reservation_anpei)
	{
		data[data_len++] = 0x07; // 预约是针对带地锁的停车位之类
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Conn_anpei) // 待机
	{
		data[data_len++] = 0x02;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Finish_anpei)
	{
		data[data_len++] = 0x09;
		data[data_len++] = 0;
	}
	else if (pUpGunData->up_gun_state == eUP_Gun_State_Fixtime_anpei) // 定时充电
	{
		data[data_len++] = 0x10;
		data[data_len++] = 0;
	}
	else
	{
		data[data_len++] = 0x0;
		data[data_len++] = 0;
	}

	for (uint8_t i = 0; i < 21; i++)
		data[data_len++] = 0; // 序号5-16 共21字节

	uint32_t tempval = 0;
	// 充电输出电压
	tempval = GetPile_ChgOutVol(u8Port, 1);
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 充电输出电流

	tempval = GetPile_ChgOutCur(u8Port, 2);
	// JJUNIVETEST
		// if(eChargeState_Charging==GetPile_gun_state(u8Port))tempval=3200;
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 输出继电器状态
	if (pUpGunData->up_gun_state == eUP_Gun_State_Work_anpei)
		data[data_len++] = 1;
	else
		data[data_len++] = 0;

	data[data_len++] = 0; //
	data[data_len++] = 0; //

	data[data_len++] = 0; // 单体电池最高电压
	data[data_len++] = 0;

	data[data_len++] = 0; // 单体电池最低电压
	data[data_len++] = 0;

	// 有功总电量

	data[data_len++] = chrg_EE_Money[u8Port].total_EE[0];
	data[data_len++] = chrg_EE_Money[u8Port].total_EE[1];
	data[data_len++] = chrg_EE_Money[u8Port].total_EE[2];
	data[data_len++] = chrg_EE_Money[u8Port].total_EE[3];

	// 尖
	data[data_len++] = chrg_EE_Money[u8Port].total_sharpEE[0];
	data[data_len++] = chrg_EE_Money[u8Port].total_sharpEE[1];
	data[data_len++] = chrg_EE_Money[u8Port].total_sharpEE[2];
	data[data_len++] = chrg_EE_Money[u8Port].total_sharpEE[3];
	// 峰
	data[data_len++] = chrg_EE_Money[u8Port].total_peakEE[0];
	data[data_len++] = chrg_EE_Money[u8Port].total_peakEE[1];
	data[data_len++] = chrg_EE_Money[u8Port].total_peakEE[2];
	data[data_len++] = chrg_EE_Money[u8Port].total_peakEE[3];

	// 平
	data[data_len++] = chrg_EE_Money[u8Port].total_flatEE[0];
	data[data_len++] = chrg_EE_Money[u8Port].total_flatEE[1];
	data[data_len++] = chrg_EE_Money[u8Port].total_flatEE[2];
	data[data_len++] = chrg_EE_Money[u8Port].total_flatEE[3];
	// 谷
	data[data_len++] = chrg_EE_Money[u8Port].total_valleyEE[0];
	data[data_len++] = chrg_EE_Money[u8Port].total_valleyEE[1];
	data[data_len++] = chrg_EE_Money[u8Port].total_valleyEE[2];
	data[data_len++] = chrg_EE_Money[u8Port].total_valleyEE[3];

	// soc
	data[data_len++] = 0; //
	data[data_len++] = 0; //

	// 累计充电时间
	uint8_t time_min_arryvalue[2] = {0};
	uint32_t time_s = GetPile_ChgTimer(u8Port);
	Uint16ToTwoUint8(time_min_arryvalue, (time_s) / 60);

	data[data_len++] = time_min_arryvalue[0];
	data[data_len++] = time_min_arryvalue[1];

	// 电动汽车唯一标识 32字节
	for (uint8_t i = 0; i < 32; i++)
		data[data_len++] = 0;

	data[data_len++] = 0;
	data[data_len++] = 0;
	data[data_len++] = 0;

	// 充电桩急停
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		if (1 == dev_getErrExsit(u8Port, eErr_EmergencyStop))
			data[data_len++] = 1;
		else
			data[data_len++] = 0;
	}
	else
		data[data_len++] = 0;

	// 直流侧开关跳闸/熔断器熔断
	data[data_len++] = 0;
	// 充电机过温告警
	data[data_len++] = 0;
	// 交流输入异常
	data[data_len++] = 0;
	// 电表通信故障
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		if (1 == dev_getErrExsit(u8Port, eErr_MeterCommErr))
			data[data_len++] = 1;
		else
			data[data_len++] = 0;
	}
	else
		data[data_len++] = 0;
	// 缺相保护
	data[data_len++] = 0;
	// 反接保护
	data[data_len++] = 0;

	// 电压越限告警
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		if (1 == dev_getErrExsit(u8Port, eErr_InputOverVol))
			data[data_len++] = 1;
		else
			data[data_len++] = 0;
	}
	else
		data[data_len++] = 0;
	// 电流越限告警
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		if (1 == dev_getErrExsit(u8Port, eErr_OutputOverCurr))
			data[data_len++] = 1;
		else
			data[data_len++] = 0;
	}
	else
		data[data_len++] = 0;
	// 风扇故障
	data[data_len++] = 0;
	// 温度传感器故障
	data[data_len++] = 0;
	// 电池组过温告警
	data[data_len++] = 0;
	// 电池单体过压
	data[data_len++] = 0;
	// 电池单体欠压
	data[data_len++] = 0;
	// 电池单体过温
	data[data_len++] = 0;
	// 电池单体欠温
	data[data_len++] = 0;
	// 集中器与桩通信故障
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)//费率故障
	{
		if (1 == FeeModel_errIixst(u8Port))
			data[data_len++] = 1;
		else
			data[data_len++] = 0;
	}
	else
		data[data_len++] = 0;

	// 充电监控单元故障
	data[data_len++] = 0;

	COST_GUN_DATA *pcostdata = &g_cost_ctrl.strCostGunData[u8Port];
	// 充电费 4
	uint32_t value1= pcostdata->allEleMoney/100;
	data[data_len++] = (value1 & 0x00ff); ;
	data[data_len++] = ((value1 >> 8) & 0x00ff);
	data[data_len++] = ((value1 >> 16) & 0x00ff);
	data[data_len++] =  ((value1 >> 24) & 0x00ff);

	// 服务费 4
	uint32_t value2= pcostdata->allServerMoney / 100;
	data[data_len++] = (value2 & 0x00ff); ;
	data[data_len++] = ((value2 >> 8) & 0x00ff);
	data[data_len++] = ((value2 >> 16) & 0x00ff);
	data[data_len++] =  ((value2 >> 24) & 0x00ff);

	// 剩余时长 4
	data[data_len++] = 0;
	data[data_len++] = 0;  
	data[data_len++] = 0;
	data[data_len++] = 0;

	// 订单号 16

	memcpy(&data[data_len], chrg_EE_Money[u8Port].transaction_log_num, 16);
	data_len = data_len + 16;

	U8 err = 0;
	// 其他故障代码 8

	// 保证传输时报文跟状态一致性，由于监测轮询的时间差 会出现桩是故障，但有下述没有故障这种情况
	if (pUpGunData->up_gun_state == eUP_Gun_State_Err_anpei)
	{
		// bit0-7
		if (1 == dev_getErrExsit(u8Port, eErr_ReaderCommErr))
			data[data_len++] = 0x02;
		else
			data[data_len++] = 0;
		// bit8-15
		data[data_len++] = 0;

		// bit16-23
		err = 0;
		if (1 == dev_getErrExsit(u8Port, eErr_ShortCircleErro))
			err = err | (1 << 1);
		else
			err = err & ~(1 << 1);

		if (1 == dev_getErrExsit(u8Port, eErr_OutputOverCurr))
			err = err | (1 << 2);
		else
			err = err & ~(1 << 2);

		if (0 == dev_getErrExsit(u8Port, eErr_CpVoltAbnor) &&0 == dev_getErrExsit(u8Port, eErr_CpGroundFault) && 0== dev_getErrExsit(u8Port, eErr_GunDisConnErr))
			err = err & ~(1 << 6);
		else
			err = err | (1 << 6);

		data[data_len++] = err;

		// bit24-31
		if (1 == dev_getErrExsit(u8Port, eErr_GunOverTempErr))
			data[data_len++] = 0x8;
		else
			data[data_len++] = 0;

		// bit32-39
		err = 0;
		if (1 == dev_getErrExsit(u8Port, eErr_EnvOverTempWarn))
			err = err | (1 << 1);
		else
			err = err & ~(1 << 1);

		if (1 == dev_getErrExsit(u8Port, eErr_GunOverTempWarn))
			err = err | (1 << 2);
		else
			err = err & ~(1 << 2);

		for (uint8_t i = 0; i < eErr_Num; i++)
		{ // 剔除所有可上报的故障
			if (i != eErr_EmergencyStop && i != eErr_MeterCommErr \
				&& i != eErr_InputOverVol && i != eErr_OutputOverCurr \
				&& i != eErr_ReaderCommErr && i != eErr_ShortCircleErro \
				&& i != eErr_CpVoltAbnor && i != eErr_CpGroundFault \
				&& i != eErr_GunOverTempErr && i != eErr_EnvOverTempWarn \
				&& i != eErr_GunOverTempWarn && i != eErr_EnvOverTempWarn \
				&& i != eErr_JcqSynechiaFault && i != eErr_GunDisConnErr&&i!=eErr_JcqMaloperation)
			{
				if (1 == dev_getErrExsit(u8Port, i)) // 其他故障
				{
					err = err | (1 << 3);
					break;
				}
			}
		}

		if (1 == dev_getErrExsit(u8Port, eErr_JcqMaloperation))
			err = err | (1 << 4);
		else
			err = err & ~(1 << 4);

		if (1 == dev_getErrExsit(u8Port, eErr_JcqSynechiaFault))
			err = err | (1 << 5);
		else
			err = err & ~(1 << 5);

		data[data_len++] = err;

		// bit40-63
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
	}
	else
	{
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
	}

	return data_len;
}

void send_RealData_anpei_Succ(uint8_t u8Port)
{
	//B1报文结束
	//SetSendEnable(u8Port, ANPEI_S_RealData, SEND_ENABLE_OFF);

   
	return;
}


/********************************************************************
 * @brief 	   
 * B2 下发计费模型下行数据 
 * B3. 下发计费模型结果数据（基础） 
 *         平台不下发B2类型费率，此处暂不处理
 *********************************************************************/	
// uint8_t B2errflag = 0;
uint8_t recv_Rate_SETB2_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	// uint8_t u8Port = GUN_A;
	// ANPEI_Recv_Rate_ModelB2 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB2;

	// if (u8Port >= GUN_NUM_MAX_ANPEI)
	// 	return FALSE;

	// gun[0] = u8Port;

	// memcpy(pRecvIdenf, r_data, sizeof(ANPEI_Recv_Rate_ModelB2));

	return TRUE;
}

void recv_Rate_SETB2_anpei_Succ(uint8_t u8Port)
{
	// ANPEI_Recv_Rate_ModelB2 *pRecvRateModel = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB2;

	// // 验证费率的正确性
	// B2errflag = 0;
	// for (uint8_t i = 0; i < 48; i++)
	// {
	// 	if (pRecvRateModel->billing_modelB2.segmentation_rate[i] > 4)
	// 	{
	// 		B2errflag = 1;
	// 		break;
	// 	}
	// }

	// if (1 != B2errflag)
	// 	save_rateB2_model_anpei(pRecvRateModel); // 保存费率

	// SetSendEnable(u8Port, ANPEI_S_Rate_SETAskB3, SEND_ENABLE_ON);
	// Send_Immediately(u8Port, ANPEI_S_Rate_SETAskB3);

	return;
}

//	B3. 下发计费模型结果数据（基础） 681E 00 00 00 00 82 00 07 00 00 00 00 00 06 1900010000000031 B4DB9B3B0000000000
uint16_t send_Rate_SETAskB3_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	// uint8_t plieNumber[8] = {0};
	// uint8_t *data = (uint8_t *)pdata;
	// ANPEI_Recv_Rate_ModelB2 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB2;

	uint16_t data_len = 0;
	// uint8_t new_data[] = {0x68, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};
	// size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	// memcpy(&data[data_len], new_data, new_data_len);
	// data_len += new_data_len;

	// AnpeiGet_PlatNumberBCD(plieNumber);
	// for (uint8_t k = 0; k < 8; k++)
	// {
	// 	data[data_len++] = plieNumber[7-k];
	// }

	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[0];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[1];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[2];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[3];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[4];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[5];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[6];
	// data[data_len++] = pRecvIdenf->billing_modelB2.billing_model[7];

	// if (B2errflag)
	// 	data[data_len++] = 1;
	// else
	// 	data[data_len++] = 0;

	return data_len;
}

void send_Rate_SETAskB3_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_Rate_SETAskB3, SEND_ENABLE_OFF);
	return;
}


/********************************************************************
 * @brief 	    
 * B4 充电启停控制命令下发下行数据（扫码充电）
 * B5 充电启停控制命令结果确认（扫码充电）
 *    充电启动失败时，上传一条费用为0 的交易记录。并注明停止失败原因
 *    充电桩到达预约充电/定时充电的开始时间时，充电桩上报控制命令是3 和2。
 *********************************************************************/	
//按照B4格式组报文，用于定时启动时内部下发命令，见B25
void Creat_B4_start_endCMD(uint8_t *pilenumber,uint8_t port,uint8_t Startcmd,uint8_t start_condition,uint8_t start_way,uint8_t *datacontrol,uint8_t *numberCus,uint8_t *ordenumber )
{
	if(port>GUN_NUM_ANPEI)return;

	ANPEI_Recv_StartEnd_Charge *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[port].strRecvStartEndCharge;

	memcpy(pRecvIdenf->device_number, pilenumber, 8);//桩编号
	pRecvIdenf->Interface_mark=port;//接口标识

	pRecvIdenf->cntrol_cmd=Startcmd;//控制命令
	pRecvIdenf->start_condition=start_condition;//启动充电条件
	pRecvIdenf->start_way=start_way;//启动充电方式
	memcpy(pRecvIdenf->start_Controldata, datacontrol, 4);//启动充电控制数据
	memcpy(pRecvIdenf->user_number, numberCus, 8);//用户编号
	memcpy(pRecvIdenf->transaction_log_num, numberCus, 16);//订单号

}

// B4 充电启停控制命令下发下行数据（扫码充电）
uint8_t recv_StartEnd_Chg_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	ANPEI_Recv_StartEnd_Charge *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvStartEndCharge;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data, sizeof(ANPEI_Recv_StartEnd_Charge));

	return TRUE;
}

void recv_StartEnd_Chg_anpei_Succ(uint8_t u8Port)
{

	ANPEI_Recv_StartEnd_Charge *pRecvStartCharge = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvStartEndCharge;
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
	
	uint32_t sum_balance = 0;
   
   if(pRecvStartCharge->start_way==3)
	sum_balance = fourUint8ToUint32(pRecvStartCharge->start_Controldata)*100;//下发的是整数
    else if(pRecvStartCharge->start_way==4)//充满
	  sum_balance=0xFFFFFFFF;

	 monitor_set_MonitorState(u8Port, eMonitorState_Service);
     
	// 开始充电
	if (1 == pRecvStartCharge->cntrol_cmd)
	{
		//收到启动命令先清空后续充电过程中的实时记录的结构体
		Clear_chrg_EE_Money_data(u8Port);   
		anpei_packChgRecord_init(u8Port,&UpRecord->AnpeiChgRecord); 
		if (TRUE == anpei_monitor_charge_start(u8Port,
										 &pUpGunData->up_start_fail_reason,
										 eUP_Start_Style_App_ANPEI,
										 NULL,
										 pRecvStartCharge->transaction_log_num,
										 &sum_balance,
										 NULL,NULL))
		{
			pUpGunData->up_start_ret = UP_RESULT_SUCC;
			pUpGunData->up_start_fail_reason = eUP_Start_Fail_NULL_ANPEI;
			fgv_CtrlStartCharge(u8Port);
			Set_powerinit(u8Port); // 初始化功率设置值

			printf("----------B4--------------start success--------------\r\n");
		}
		else
		{
			pUpGunData->up_start_ret = UP_RESULT_FAIL;
			fgv_CtrlStopCharge(u8Port);
			Set_powerinit(u8Port);
			printf("-----------B4-------------start fail--------------\r\n");
		}

	   
	
	}
	else
	{
		// 停止充电
		stopPileCharge(u8Port, Pile_Stop_Reason_APP);

		pUpGunData->up_stop_ret = UP_RESULT_SUCC;
		pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL_ANPEI;
     
		printf("--------------B4----------end charging--------------\r\n");
		
	}
    

  
	SetSendEnable(u8Port, ANPEI_S_StartEnd_ChgAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_StartEnd_ChgAsk);

	

	return;
	
}

//	B5. 充电启停控制命令结果确认（扫码充电）
// 68 31 00 00 00 00 8500070000000000151900010000000031 000000000180BB330E1D021462313016511429022020000000000000
uint16_t send_StartEnd_ChgAsk_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;
	ANPEI_Recv_StartEnd_Charge *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvStartEndCharge;
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
   
	uint16_t data_len = 0;

	uint8_t new_data[] = {0x68, 0x31, 0x00, 0x00, 0x00, 0x00, 0x85, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 成功标志 1
	if (pUpGunData->up_start_ret == UP_RESULT_FAIL)
	{
		data[data_len++] = 0x01;
	}
	else
	{
		//启动成功	
      data[data_len++] = 0x00;
       
	}
	


	// 启动充电失败原因 2
	if (pUpGunData->up_start_ret == UP_RESULT_FAIL)
	{
		data[data_len++] = 0x00;

		if (pUpGunData->up_start_fail_reason == eUP_Start_Fail_Working_ANPEI)
			data[data_len++] = 0x01;
		else if (pUpGunData->up_start_fail_reason == eUP_Start_Fail_DevErr_ANPEI)
			data[data_len++] = 0x02;
		else
			data[data_len++] = 0x03;
	}
	else
	{
		//启动成功
		
		data[data_len++] = 0;
		data[data_len++] = 0;
         
	}

	// 控制命令 1
	data[data_len++] = pRecvIdenf->cntrol_cmd;


	// 控制时间 7
	U8 timeOrgin[7] = {0};
	getRunTimeYYMDHMS(timeOrgin); // 年H 年L 月 日 时 分 秒
    
	U8 CP56tim[7] = {0};
	Time_to_CP56Time2a(timeOrgin,CP56tim);
	memcpy(&data[data_len], CP56tim, 7);
	data_len = data_len + 7;

	// 订单号 16
	memcpy(&data[data_len], pRecvIdenf->transaction_log_num, 16);
	data_len = data_len + 16;

	return data_len;
}

void send_StartEnd_ChgAsk_anpei_Succ(uint8_t u8Port)
{
  ANPEI_Recv_StartEnd_Charge *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvStartEndCharge;
  up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
  union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;


	SetSendEnable(u8Port, ANPEI_S_StartEnd_ChgAsk, SEND_ENABLE_OFF);

	if(0==pRecvIdenf->cntrol_cmd)
	{
	  printf("--------------B53----------end charging--upb53------------\r\n");
      
	}

	//充电启动失败时，上传一条费用为0 的交易记录。并注明停止失败原因
	// if(pUpGunData->up_start_ret == UP_RESULT_FAIL)
	// {
	// 	anpei_packChgRecord_startFailinitData(u8Port,pRecvIdenf->transaction_log_num , pUpGunData->up_start_fail_reason, &UpRecord->AnpeiChgRecord);
	// 	printf(".......Start fail....... creat....message..... \r\n");
	// 	SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_ON);
	// 	Send_Immediately(u8Port, ANPEI_S_onlineEnd_ChgInfB53);
	// }

	return;
}

/********************************************************************
 * @brief 	 B6. 刷卡鉴权上行（在线刷卡充电）
 *           B7刷卡鉴权下行（在线刷卡充电）
 *********************************************************************/	
//B6. 刷卡鉴权上行（在线刷卡充电）
uint16_t send_Cardinf_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;

	// CHG_DATA_T *pChgGunData = &g_chgData[u8Port];
	//    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	uint16_t data_len = 0;
	uint8_t new_data[] = {0x68, 0x52, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1
	data[data_len++] = u8Port;

	// 物理卡号 8byte
	uint8_t cardnumber[8] = {0};
	GetPile_ChgCarNumber(u8Port, cardnumber);
	memcpy(&data[data_len], cardnumber, 8);
	data_len += 8;

	// 密码 16
	memset(&data[data_len], 0, 16);
	data_len += 16;

	// 卡余额 4
	memset(&data[data_len], 0, 4);
	data_len += 4;

	// 电动汽车唯一标识 32
	memset(&data[data_len], 0, 32);
	data_len += 32;

	return data_len;
}
void send_Cardinf_anpei_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_CardinfAck))
	{
		SetRecvEnable(u8Port, ANPEI_R_CardinfAck, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_CardinfAck, Get_Systick());
	}

	return;
}

// B7刷卡鉴权下行（在线刷卡充电）
uint8_t recv_CardinfAck_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	ANPEI_Recv_Card_inf *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_inf;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 57, sizeof(ANPEI_Recv_Card_inf));

	return TRUE;
}

void recv_CardinfAck_anpei_Succ(uint8_t u8Port)
{

	ANPEI_Recv_Card_inf *pRecvStartCharge = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_inf;

	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	uint32_t sum_balance = 0;
	sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);

	SetSendEnable(u8Port, ANPEI_S_Cardinf, SEND_ENABLE_OFF);

	if (1 == pRecvStartCharge->check_card && sum_balance)
	{

		SetSendEnable(u8Port, ANPEI_S_CardStart_Chg, SEND_ENABLE_ON); // 发送B10信息
		Send_Immediately(u8Port, ANPEI_S_CardStart_Chg);
	}
}
/********************************************************************
 * @brief B10. 启动通知上报（在线刷卡充电/在线vin码充电）
          B11 启动通知下行（在线刷卡充电/在线vin码充电）
 *********************************************************************/	
//	B10. 启动通知上报（在线刷卡充电/在线vin码充电）
uint16_t send_CardStart_Chg_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;

	// CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	uint16_t data_len = 0;
	uint8_t new_data[] = {0x68, 0x57, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 物理卡号 8byte
	uint8_t cardnumber[8] = {0};
	GetPile_ChgCarNumber(u8Port, cardnumber);
	memcpy(&data[data_len], cardnumber, 8);
	data_len += 8;

	// 密码 16
	memset(&data[data_len], 0, 16);
	data_len += 16;

	// 卡余额 4
	memset(&data[data_len], 0, 4);
	data_len += 4;

	// 电动汽车唯一标识 32
	memset(&data[data_len], 0, 32);
	data_len += 32;

	// 启动充电控制方式
	data[data_len++] = 4;
	// 启动充电控制数据
	data[data_len++] = 0;

	return data_len;
}
void send_CardStart_Chg_anpei_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_CardStart_ChgAck))
	{
		SetRecvEnable(u8Port, ANPEI_R_CardStart_ChgAck, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_CardStart_ChgAck, Get_Systick());
	}

	return;
}

// B11 启动通知下行（在线刷卡充电/在线vin码充电）
uint8_t recv_CardStart_ChgAck_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	ANPEI_Recv_Card_start *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_start;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, sizeof(ANPEI_Recv_Card_start));

	return TRUE;
}


void recv_CardStart_ChgAck_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_Card_start *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_start;
	ANPEI_Recv_Card_inf *pRecvStartCharge = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_inf;
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	uint32_t Card_sum_balance = fourUint8ToUint32(pRecvStartCharge->account_balance);
 
     monitor_set_MonitorState(u8Port, eMonitorState_Service);

	// 开始充电
	Clear_chrg_EE_Money_data(u8Port);   
	anpei_packChgRecord_init(u8Port,&UpRecord->AnpeiChgRecord); 

	if (1 == pRecvIdenf->result)
	{
		if (TRUE == anpei_monitor_charge_start(u8Port,
										 &pUpGunData->up_start_fail_reason,
										 eUP_Start_Style_CardOnline_ANPEI,
										 NULL,
										 pRecvIdenf->transaction_log_num,
										 &Card_sum_balance,
										 NULL,NULL))
		{
			pUpGunData->up_start_ret = UP_RESULT_SUCC;
			pUpGunData->up_start_fail_reason = eUP_Start_Fail_NULL_ANPEI;
			fgv_CtrlStartCharge(u8Port);
			Set_powerinit(u8Port);
		}
		else
		{
			pUpGunData->up_start_ret = UP_RESULT_FAIL;
			fgv_CtrlStopCharge(u8Port);
			Set_powerinit(u8Port);
		}

	}

    SetSendEnable(u8Port, ANPEI_S_CardStart_Chg, SEND_ENABLE_OFF);
	return;
}


/********************************************************************
 * @brief 	 B12. 在线情况下停止充电时上传记录数据（基础）停用
             B13 在线交易包下行数据（基础）    停用
 *********************************************************************/	
//	B12. 在线情况下停止充电时上传记录数据（基础）
uint16_t send_onlineEnd_ChgInfB12_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	// uint8_t plieNumber[8] = {0};
	// uint8_t *data = (uint8_t *)pdata;

	//   up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	uint16_t data_len = 0;
	// uint8_t new_data[] = {0x68, 0xB1, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
	// size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	// memcpy(&data[data_len], new_data, new_data_len);
	// data_len += new_data_len;

	// AnpeiGet_PlatNumberBCD(plieNumber);
	// for (uint8_t k = 0; k < 8; k++)
	// {
	// 	data[data_len++] = plieNumber[7-k];
	// }

	// //	 充电接口标识 1
	// data[data_len++] = u8Port;

	// RecordB12 *pB12Idenf = &RecordB12Save;

	// memcpy(&data[data_len], pB12Idenf->transaction_log_num, 16);
	// data_len = data_len + 16;

	// // 物理卡号 8byte
	// memcpy(&data[data_len], pB12Idenf->Logic_card_number, 8);
	// data_len = data_len + 8;

	// // 开始时间BIN 码  7
	// memcpy(&data[data_len], pB12Idenf->chrg_start_time, 7);
	// data_len += 7;

	// // 结束时间BIN 码 7
	// memcpy(&data[data_len], pB12Idenf->chrg_stop_time, 7);
	// data_len += 7;

	// // 累计充电时间BIN 2
	// memcpy(&data[data_len], pB12Idenf->chrg_chrg_totaltime, 2);
	// data_len += 2;

	// // 尖起 4
	// memcpy(&data[data_len], pB12Idenf->sharp_start_power, 4);
	// data_len += 4;
	// // 尖止 4
	// memcpy(&data[data_len], pB12Idenf->sharp_stop_power, 4);
	// data_len += 4;
	// // 峰起
	// memcpy(&data[data_len], pB12Idenf->peak_start_power, 4);
	// data_len += 4;
	// // 峰止
	// memcpy(&data[data_len], pB12Idenf->peak_stop_power, 4);
	// data_len += 4;
	// // 平起
	// memcpy(&data[data_len], pB12Idenf->flat_start_power, 4);
	// data_len += 4;
	// // 平止
	// memcpy(&data[data_len], pB12Idenf->flat_stop_power, 4);
	// data_len += 4;
	// // 谷起
	// memcpy(&data[data_len], pB12Idenf->valley_start_power, 4);
	// data_len += 4;
	// // 谷止
	// memcpy(&data[data_len], pB12Idenf->valley_stop_power, 4);
	// data_len += 4;

	// // 尖电
	// memcpy(&data[data_len], pB12Idenf->sharp_total_power, 4);
	// data_len += 4;
	// // 峰电
	// memcpy(&data[data_len], pB12Idenf->peak_total_power, 4);
	// data_len += 4;
	// // 平电
	// memcpy(&data[data_len], pB12Idenf->flat_total_power, 4);
	// data_len += 4;
	// // 谷电
	// memcpy(&data[data_len], pB12Idenf->valley_total_power, 4);
	// data_len += 4;

	// // 总
	// memcpy(&data[data_len], pB12Idenf->total_total_power, 4);
	// data_len += 4;
	// // 总起示
	// memcpy(&data[data_len], pB12Idenf->total_start_power, 4);
	// data_len += 4;

	// // 总止 4
	// memcpy(&data[data_len], pB12Idenf->total_stop_power, 4);
	// data_len += 4;

	// // 充电前SOC 2
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// // 结束后SOC  2
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// // 电动汽车唯一标  32
	// for (uint8_t i = 0; i < 32; i++)
	// 	data[data_len++] = 0;

	// // 停止充电原  2
	// memcpy(&data[data_len], pB12Idenf->stop_reason, 2);
	// data_len += 2;

	// // 充电费 4
	// memcpy(&data[data_len], pB12Idenf->chrg_total_money, 4);
	// data_len += 4;

	// // 服务费 4
	// memcpy(&data[data_len], pB12Idenf->sver_total_money, 4);
	// data_len += 4;
	// // 车牌号 8
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// data[data_len++] = 0;
	// // 确认车牌号功能是否开启 1
	// data[data_len++] = 0;

	return data_len;
}
void send_onlineEnd_ChgInfB12_anpei_Succ(uint8_t u8Port)
{
	// if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_onlineEnd_ChgInfAckB13))
	// {
	// 	SetRecvEnable(u8Port, ANPEI_R_onlineEnd_ChgInfAckB13, RECV_ENABLE_ON);
	// 	SetRecvTick(u8Port, ANPEI_R_onlineEnd_ChgInfAckB13, Get_Systick());
	// }

	// RecordB12 *pB12Idenf = &RecordB12Save;

	// pB12Idenf->UPB12_cnt++;

	// if (pB12Idenf->UPB12_cnt >= 10)
	// {
	// 	// 上报10次未回复，停止上报
	// 	if (SEND_ENABLE_ON == GetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB12))
	// 	{
	// 		SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB12, SEND_ENABLE_OFF);
	// 	}
	// }
	// else if (pB12Idenf->UPB12_cnt == 1)
	// {
	// 	pB12Idenf->ExistChargeDealB12 = 0;
	// }

	return;
}

// B13 在线交易包下行数据（基础）
uint8_t recv_onlineEnd_ChgInfAckB13_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	// uint8_t u8Port = GUN_A;
	// ANPEI_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvOnlinetrans_ask;

	// if (u8Port >= GUN_NUM_MAX_ANPEI)
	// 	return FALSE;

	// gun[0] = u8Port;

	// memcpy(pRecvIdenf, r_data + 9, sizeof(ANPEI_Recv_Online_ask));

	return TRUE;
}

void recv_onlineEnd_ChgInfAckB13_anpei_Succ(uint8_t u8Port)
{
	// ANPEI_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvOnlinetrans_ask;

	// SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB12, SEND_ENABLE_OFF);

	// if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_ChgDeduction_Inf))
	// {
	// 	SetRecvEnable(u8Port, ANPEI_R_ChgDeduction_Inf, RECV_ENABLE_ON);
	// 	SetRecvTick(u8Port, ANPEI_R_ChgDeduction_Inf, Get_Systick());
	// }

	return;
}

/********************************************************************
 * @brief 	   B14 充电扣款后下行数据（基础） 
上传交易B53记录后--平台回复B54 ,手机端结算完成后 平台回复B14
记录结构体数据停止原因写0XFFFF 表示已经上传完毕，后续开机读取判断是否为0xFF;
 *********************************************************************/	
// B14 充电扣款后下行数据（基础） 
uint8_t recv_ChgDeduction_Inf_anpei(uint8_t *r_data, int len, uint8_t *gun)
{//收到数据就表示已经结算完成
	return TRUE;
}

void recv_ChgDeduction_Inf_anpei_Succ(uint8_t u8Port)
{
	

	return;
}

/********************************************************************
 * @brief 	B15. 离线交易上线后上传交易记录数据（基础）
            B16  离线交易包下行数据（基础）
			该部分是针对离线刷卡启动才会有的数据，不涉及该功能，不做处理
 *********************************************************************/	
//	B15. 离线交易上线后上传交易记录数据（基础） 
uint16_t send_offlineEnd_ChgInfB15_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	return data_len;
}
void send_offlineEnd_ChgInfB15_anpei_Succ(uint8_t u8Port)
{
	return;
}

// B16  离线交易包下行数据（基础）
uint8_t recv_offlineEnd_ChgInfAckB16_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	return true;
}

void recv_offlineEnd_ChgInfAckB16_anpei_Succ(uint8_t u8Port)
{
	return;
}
/********************************************************************
 * @brief 	B23 远程升级启动（扩展） ftp升级
            B24 远程升级启动命令接收结果（扩展）
 *********************************************************************/	
uint8_t recv_RemoteUpgrade_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_RemUp_Cmd *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRemUp_Cmd;

	if (u8Port >= GUN_NUM_MAX_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data, sizeof(ANPEI_Recv_RemUp_Cmd));
	return TRUE;
}

void recv_RemoteUpgrade_anpei_Succ(uint8_t u8Port)
{
	char recip[16] = {0};
	up_gun_data_ctrl *ptcp_data = &g_ProtocolDCB.strUpGunData[u8Port];
	ANPEI_Recv_RemUp_Cmd *pRecvFtp = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRemUp_Cmd;

	SetSendEnable(u8Port, ANPEI_S_RemoteUpgradeAck, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_RemoteUpgradeAck); // B24

	ptcp_data->up_update_ret = 0;
	g_ProtocolDCB.PlatTask.updata_flag = E_Update_Ftp;		  // 升级
	g_ProtocolDCB.PlatTask.updata_delay_tick = Get_Systick(); // 超时时间

	// 如：C0 02 03 04 转换成IP 为192.2.3.4
	sprintf(recip, "%d.%d.%d.%d", pRecvFtp->update_ip[3], pRecvFtp->update_ip[2], pRecvFtp->update_ip[1], pRecvFtp->update_ip[0]);
    
    uint16_t u16Port = pRecvFtp->update_com[1] << 8 | pRecvFtp->update_com[0];
    g_PileUpdateInterface(recip, u16Port, (char *)pRecvFtp->update_username, (char *)pRecvFtp->update_password, 
                            (char *)pRecvFtp->update_file_path, (char *)pRecvFtp->update_file_name);

	return;
}
//	B24. 远程升级启动命令接收结果（扩展）
uint16_t send_RemoteUpgradeAck_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};
	charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;

	uint8_t new_data[] = {0x68, 0x16, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}
	data[data_len++] = 0x00;

	return data_len;
}

void send_RemoteUpgradeAck_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_RemoteUpgradeAck, SEND_ENABLE_OFF);
	return;
}
/********************************************************************************
 * @brief 	B25. 预约/定时命令下行数据（扩展）        
            B26. 桩回复预约/定时结果上行数据（扩展）
		预约是指预约停车位，目前未开发针对预约的内容	
		（非协议里提及）：插枪状态定时后，定时时间到之前，若拔枪采取自动取消定时状态
		                 拔枪状态后，若收到取消定时命令，依旧返回成功
 *********************************************************************************/
 /******************************************************** 
 * @brief 	   时间判断 年H 年L 月 日 时 分 秒  ; 	   
 *********************************************************/	
bool comapre_time(uint8_t *nowtime, uint8_t *fixtime)
{
	for(uint8_t i=0;i<7;i++)
	{
       if(nowtime[i]!=fixtime[i])
              return false;

	}
   return true;
	
}

/********************************************************************
 * @brief 	  定时任务
 * @param[in]	 
 * @return 	   
 *********************************************************************/	
uint8_t flixState[GUN_NUM_MAX_ANPEI]={0,0};//0无 1：成功开启定时 2定时充电中 3结束定时
void ANPEI_fix_Revers_timeTask(void)
{
	// U8 timeOrgin[7] = {0};

   
	// for(uint8_t u8Port=0;u8Port<GUN_NUM_ANPEI; u8Port++) 
	// {
	// 	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;

	// 	if(flixState[u8Port]==1)
	// 	{ 

	// 		getRunTimeYYMDHMS(timeOrgin); // 年H 年L 月 日 时 分 秒

	// 	   if(true == comapre_time(timeOrgin, chrg_EE_Money[u8Port].start_time))//开始启动
	// 		 {
	// 			flixState[u8Port]=0;
	// 			//开始启动前再次判断是否有故障
	// 			if(TRUE == dev_getErrState(u8Port)) //有故障 停止启动 结束订单，组故障报文 上传信息
	// 			{
	// 				anpei_packChgRecord_startFailinitData(u8Port,chrg_EE_Money[u8Port].transaction_log_num , eUP_Start_Fail_DevErr_ANPEI, &UpRecord->AnpeiChgRecord);
	// 				printf(".......Start fail..... BEFORE....getto.fixtime.... \r\n");
	// 				SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_ON);
	// 				Send_Immediately(u8Port, ANPEI_S_onlineEnd_ChgInfB53);


	// 			}
	// 			else //没有故障继续启动
	// 			{
	// 			  flixState[u8Port]=2;//定时充电中
    //               fgv_CtrlStartCharge(u8Port);
	// 		      Set_powerinit(u8Port); // 初始化功率设置值
	// 		      printf("--------------fixtime----start-------\r\n");
					
	// 			}

	  
	// 		 }

	// 		 if(chrg_EE_Money[u8Port].start_waykind==7&&false == GetPile_gun_connect(u8Port))//定时开始前强行拔枪
	// 		 {
	// 			flixState[u8Port]=0;
	// 			//组报文 上传信息
	// 			printf(".......Start fail..... BEFORE....getto.fixtime...gunoff... \r\n");
	// 			anpei_packChgRecord_startFailinitData(u8Port,chrg_EE_Money[u8Port].transaction_log_num , eUP_Start_Fail_NoConn_ANPEI, &UpRecord->AnpeiChgRecord);
	// 			SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_ON);
	// 			Send_Immediately(u8Port, ANPEI_S_onlineEnd_ChgInfB53);
	// 		 }
		 
	  
	  
	// 	}
        
	

	// 	if(flixState[u8Port]==2) //定时充电中
	// 	{
	// 		//判断是否到达指定时间
	// 		getRunTimeYYMDHMS(timeOrgin); 
	// 		if(true == comapre_time(timeOrgin, chrg_EE_Money[u8Port].end_time))
	// 		{

	// 			flixState[u8Port]=0;
	// 			stopPileCharge(u8Port, Pile_Stop_Reason_APP);
	// 			printf("--------------fixtime--runtime--end-------\r\n");

	// 		}
	// 		else //未到达
	// 		{
	// 			//桩启动方式发生改变，说明由于其他原因在未到达停止时间终止了
	// 			//（例：中途故障停止充电，上报结束后记录后，chrg_EE_Money结构体被清0）
	// 			if(chrg_EE_Money[u8Port].start_waykind!=7)
	// 			 {
	// 				flixState[u8Port]=0;
	// 			 }

	// 		}


	// 	}

	// 	if(flixState[u8Port]==3) //结束定时
	// 	{
			

	// 		//判断是否已经在运行
	// 		if(flixState[u8Port]==2)//是 停止充电 上传信息
	// 		{
				
	// 			stopPileCharge(u8Port, Pile_Stop_Reason_APP);
	// 			printf("------------fixtime----end-------\r\n");

	// 		}
	// 		else //没有运行 直接结束记录的信息即可
	// 		{				
	// 			Clear_chrg_EE_Money_data(u8Port);   
	// 			anpei_packChgRecord_init(u8Port,&UpRecord->AnpeiChgRecord); 
	// 		}

	// 		flixState[u8Port]=0;

	// 	}


	// }
  
}
//B25.  定时命令下行数据（扩展）
uint8_t recv_FixtimeCmd_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	ANPEI_Recv_fixtime_cmd *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_fixtime_cmd;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, sizeof(ANPEI_Recv_fixtime_cmd));

	return TRUE;
}

void recv_FixtimeCmd_anpei_Succ(uint8_t u8Port)
{

	//分析能否正常定时
	ANPEI_Recv_fixtime_cmd *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_fixtime_cmd;
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;
	
	

	//  monitor_set_MonitorState(u8Port, eMonitorState_Service);

    //  uint32_t sum_balance =0;
	// sum_balance= twoUint8ToUint16(pRecvIdenf->chargefee);

	// //报文为CP56格式需转正常格式
    // uint8_t fixstarttime[7]={0};//正常格式
	// uint8_t fixendtime[7]={0};
	// CP56Time2a_to_Time(pRecvIdenf->starttime,fixstarttime);
	// CP56Time2a_to_Time(pRecvIdenf->endtime,fixendtime);
	// // 开始定时充电
	// if (2 == pRecvIdenf->state)
	// {
	// 	//收到启动命令先清空后续充电过程中的实时记录的结构体
	// 	Clear_chrg_EE_Money_data(u8Port);   
	// 	anpei_packChgRecord_init(u8Port,&UpRecord->AnpeiChgRecord); 
	// 	if (TRUE == anpei_monitor_charge_start(u8Port,
	// 									 &pUpGunData->up_start_fail_reason,
	// 									 eUP_Start_Style_Fixtime_ANPEI,
	// 									 NULL,
	// 									 pRecvIdenf->transaction_log_num,
	// 									 &sum_balance,
	// 									 &fixstarttime[0],&fixendtime[0]))
	// 	{
	// 		pUpGunData->up_start_ret = UP_RESULT_SUCC;
	// 		pUpGunData->up_start_fail_reason = eUP_Start_Fail_NULL_ANPEI;	
	// 		flixState[u8Port]=1;	
	
	// 		printf("----------B25----------fixtime----judge success-------\r\n");
	// 	}
	// 	else
	// 	{
	// 		pUpGunData->up_start_ret = UP_RESULT_FAIL;
	// 		fgv_CtrlStopCharge(u8Port);
	// 		Set_powerinit(u8Port);
	// 		printf("-----------B25---------fixtime----judeg fail-----------\r\n");
	// 	}
	
	// }
	// else if(3==pRecvIdenf->state)//结束定时,即停止充电
	// {
	// 	flixState[u8Port]=2;	

	// 	pUpGunData->up_stop_ret = UP_RESULT_SUCC;
	// 	pUpGunData->up_stop_fail_reason = eUP_Stop_Fail_NULL_ANPEI;
     
	// 	printf("-------------B25------fixtime----end--------------\r\n");
		
	// }
	// else
	//  {

	// 	return;//不开发预约功能，其他直接return
	//  }
    

	SetSendEnable(u8Port, ANPEI_S_FixtimeCmdAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_FixtimeCmdAsk);

	return;
}

//	B26. 桩回复预约/定时结果上行数据
//  uint8_t fixtimeflag=0;//1 定时成功  2定时错误 3取消定时
uint16_t send_FixtimeCmdAsk_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}
	//	 充电接口标识 1
	data[data_len++] = u8Port;

	// 账户
	ANPEI_Recv_fixtime_cmd *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvCard_fixtime_cmd;
	memcpy(&data[data_len], pRecvIdenf->accountNumber, 8);
	data_len += 8;

	// 交易流水号 16byte
	memcpy(&data[data_len], pRecvIdenf->transaction_log_num, 16);
	data_len += 16;

	// 状态 1
	data[data_len++] = pRecvIdenf->state;
    
	//结果+原因 2byte
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	// if (pRecvIdenf->state == 2) // 开始定时
	// {
	// 	if (pUpGunData->up_start_fail_reason == eUP_Start_Fail_DevErr_ANPEI)
	// 	{
	// 		data[data_len++] = 1; // 结果 1 失败
	// 		data[data_len++] = 1; //
	// 	}
	// 	else if (pUpGunData->up_start_fail_reason == eUP_Start_Fail_NULL_ANPEI) 
	// 	{			
	// 		data[data_len++] = 0; // 结果 0 成功
	// 		data[data_len++] = 0; // 原因 0
	// 	}
	// 	else
	// 	{		
	// 		data[data_len++] = 1; // 结果 1失败
	// 		data[data_len++] = 9; //9 其他原因
	// 	}
	// }
	// else if (pRecvIdenf->state == 3) // 结束定时
	// {
	// 	data[data_len++] = 0;
	// 	data[data_len++] = 0;
	// }
	// else // 预约均回复失败
	{
		data[data_len++] = 1;
		data[data_len++] = 9;
	}

	return data_len;
}

void send_FixtimeCmdAsk_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_FixtimeCmdAsk, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	   B32   请求终端数据下行数据（扩展） 
 *             B31. SIM卡信息上行数据（扩展） 
 *********************************************************************/	
// B32 请求终端数据下行数据（扩展） 
uint8_t recv_NeedSIMInf_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	ANPEI_Recv_SIMinf_up *PRecv_siminf = &g_ProtocolDCB.pANPEIRecvData[0].strRecvSIMinf_up;
    
	return TRUE;
}

void recv_NeedSIMInf_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_SIMInfAck, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_SIMInfAck);
	return;
}
//	B31. SIM卡信息上行数据（扩展）
uint16_t send_SIMInfAck_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{

	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};
	charge_record *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.GnChgRecord;

	uint8_t new_data[] = {0x68, 0x34, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	// sim卡号 20
	char cSimID[20] = {0};
	GetNet_Comm_SimID((uint8_t *)cSimID, 20);

	for (uint8_t i = 0; i < 20; i++)
		data[data_len++] = cSimID[i];

	// 手机号 11
	for (uint8_t i = 0; i < 11; i++)
		data[data_len++] = 0;

	return data_len;
}
void send_SIMInfAck_anpei_Succ(uint8_t u8Port)
{
	CHG_DATA_T *pChgGunData = &g_chgData[u8Port];//上线登录后最后一步主动上传SIM信息，后续若有未上传记录，此处应上传
    up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
    ANPEI_Recv_SIMinf_up *PRecv_siminf = &g_ProtocolDCB.pANPEIRecvData[0].strRecvSIMinf_up;
	SetSendEnable(u8Port, ANPEI_S_SIMInfAck, SEND_ENABLE_OFF);


    
	if(PRecv_siminf->receiveflag==0)//说明是上线登录时候上传SIM信息
    {
		PRecv_siminf->receiveflag=1;//
		
		printf("JJUNIVE:-----------------First UPSIM--------------------------\r\n");

      if(pUpGunData->up_gun_state!=eUP_Gun_State_Work_anpei)//未上传并且桩未充电
	    {
			ANpeiUpChargeRecordUpDealOffline();  
	  }
       B1_first_aftre_online_flag=1;
	  SetSendEnable(u8Port, ANPEI_S_RealData, SEND_ENABLE_ON);//打开发送B1
  	  Send_Immediately(u8Port,ANPEI_S_RealData);

	  SetSendEnable(u8Port, ANPEI_S_Heart, SEND_ENABLE_ON);
	  Send_Immediately(u8Port,ANPEI_S_Heart);
	


	}
   


	return;
}

/********************************************************************
 * @brief B33	 充电功率控制下行（扩展）
          B34.   充电功率控制上行（扩展） 
 *********************************************************************/	
// 充电功率控制下行（扩展） B33
uint32_t ANPEIGet_devpow(void) // 获取桩默认功率
{
	return sg_platmod.pileCfgInfo.pow_limit;
}

void ANPEISet_devpow(void) // 获取桩默认功率
{
	return;
}

// 初始化运行时的功率值
void Set_powerinit(uint8_t u8Port)
{
    if (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] == 0) {
	    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] = ANPEIGet_devpow() * 100; // 没有平台默认功率时，取桩默认功率
    }
    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic] = 0;
    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl] = 0;
}

uint8_t recv_PowerCon_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_Powercontrol *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvPowercontrol;

	if (u8Port >= GUN_NUM_MAX_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8 + 1, sizeof(ANPEI_Recv_Powercontrol));

	return TRUE;
}


// 初始化运行时的功率值
void powerInit(uint8_t u8Port)
{
    if (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] == 0) {
	    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] = ANPEIGet_devpow() * 100; // 没有平台默认功率时，取桩默认功率
    }
    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic] = 0;
    stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl] = 0;
}


uint32_t GetUpdatePlatSetPower(uint8_t u8Port)
{
    #define DEFAULT_POWER   700
    //A默认功率  B动态功率  C控制功率
    uint32_t Avalue = stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault];
    uint32_t Bvalue = stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic];
    uint32_t Cvalue = stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl];
    
    stuGunPowerCtr[u8Port].Powerkind = E_platPowerDefault;

   //都不存在返回700
     if ((0 == Bvalue) && (0 == Avalue) && (0 == Cvalue)) {
         return DEFAULT_POWER;
     }
	 //平台离线返回默认功率
	if((eSocket_Online!=GPRS_Getonlineflag(eSocket_GPRS1))||(eOnline_Heart!=Get_PlatConnectSta()))
	return Avalue;

    //没有BC或者，则返回A
    if (Avalue && (0 == Bvalue) && (0 == Cvalue)) {
        return Avalue;
    }
    //AC同时存在，B不存在，C>A，返回A
    //AC同时存在，B不存在，C<A，返回C
    if ((0 == Bvalue) && ((Avalue) && (Cvalue))) {
        if (Avalue > Cvalue) {
            stuGunPowerCtr[u8Port].Powerkind = E_platPowerControl;
            return Cvalue;
        } else {
            return Avalue;
        }
    }
    //B存在，C不存在，返回B
    if ((Bvalue > 0) && (0 == Cvalue)) {
        stuGunPowerCtr[u8Port].Powerkind = E_platPowerDynamic;
        return Bvalue;
    }
    return DEFAULT_POWER;
}
//timer:单位s
void SetPlatSetPower(uint8_t u8Port, uint8_t kind, uint32_t power, uint32_t timer)
{
    printf("SetPlatSetPower: %d %d %d\r\n", u8Port, kind, power);
	if(kind==0||kind>4)return;
    stuGunPowerCtr[u8Port].CtrlPower[kind] = power;
    if (E_platPowerDynamic == kind) {
        stuGunPowerCtr[u8Port].DynamicTime = NOWTICK;
    }
    
    if (E_platPowerControl == kind) {
        stuGunPowerCtr[u8Port].timepre = timer*100;
    } else {
        stuGunPowerCtr[u8Port].timepre = eTick_15S;
    }
    //写入A板执行
    stuGunPowerCtr[u8Port].ActPower = GetUpdatePlatSetPower(u8Port);

    if (stuGunPowerCtr[u8Port].Powerkind == E_platPowerControl) {
	    SetSendEnable(u8Port, ANPEI_S_ChgPowerCon_inf, SEND_ENABLE_ON);
    } else {
	    SetSendEnable(u8Port, ANPEI_S_ChgPowerCon_inf, SEND_ENABLE_OFF);
    }
	printf("SetPlatSetEndPower: %d %d %d\r\n", u8Port, kind, stuGunPowerCtr[u8Port].ActPower);
    fgv_CtrlChargeCrt(u8Port, stuGunPowerCtr[u8Port].ActPower);
}

//需要实时监测
void ANPEI_refreshpowertoPile()
{
    static uint8_t pre_gunState[GUN_NUM_MAX] = {0};
    //B存在，15秒判断没有持续更新B，清零
    for (int i = 0; i < GUN_NUM_MAX; i++) {
        //离线判断
		//if(0) powerInit(i);  
        //充电结束，BC清零;开始充电
        uint8_t gunState = GetPile_gun_state(i);
        if (pre_gunState[i] != gunState) {
            if (gunState == eChargeState_StopFinish) {
                powerInit(i);    // 初始化功率设置值
            } else if (gunState == eChargeState_Waiting) {
                powerInit(i);    // 初始化功率设置值
            }
            pre_gunState[i] = gunState;
        }

        if (E_platPowerDynamic != stuGunPowerCtr[i].Powerkind) {
            continue;
        }
        if (JudgeTimeOutMs(stuGunPowerCtr[i].DynamicTime, eTick_15S) == TRUE) {
            SetPlatSetPower(i, E_platPowerDynamic, 0, 0);
        }
        
    }
}

uint8_t Refresh_PowerCon_flag=0;//0:失败 1：成功
void recv_PowerCon_anpei_Succ(uint8_t u8Port)
{

	ANPEI_Recv_Powercontrol *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvPowercontrol;
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];
	// 更新功率
	uint32_t powerVual = fourUint8ToUint32(pRecvIdenf->powerVual);
	uint16_t timeuppower = twoUint8ToUint16(pRecvIdenf->timeuppower);

	SetSendEnable(u8Port, ANPEI_S_PowerConASK, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_PowerConASK);

	//功率在正常范围内
	Refresh_PowerCon_flag=0;
   if(powerVual>=132&&powerVual<=700)
    {
          if(pRecvIdenf->kind==2||pRecvIdenf->kind==3)//动态和控制需要保证在充电中
		    {
               if(pUpGunData->up_gun_state == eUP_Gun_State_Work_anpei)
			   {
				Refresh_PowerCon_flag=1;
				SetPlatSetPower(u8Port, pRecvIdenf->kind, powerVual, timeuppower);

			   }
			}
			else if(pRecvIdenf->kind==1)//默认功率不需要
			{
				Refresh_PowerCon_flag=1;
				SetPlatSetPower(u8Port, pRecvIdenf->kind, powerVual, timeuppower);

				
			}

	}
    

	return;
}
//	B34. 充电功率控制上行（扩展）
uint16_t send_PowerConASK_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 时间戳
	ANPEI_Recv_Powercontrol *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvPowercontrol;
	data[data_len++] = pRecvIdenf->timepower[0];
	data[data_len++] = pRecvIdenf->timepower[1];
	data[data_len++] = pRecvIdenf->timepower[2];
	data[data_len++] = pRecvIdenf->timepower[3];
	data[data_len++] = pRecvIdenf->timepower[4];
	data[data_len++] = pRecvIdenf->timepower[5];
	data[data_len++] = pRecvIdenf->timepower[6];

	// 成功标志
	if(Refresh_PowerCon_flag)
	{
		Refresh_PowerCon_flag=0;
		data[data_len++] = 0;
	}
    else
	 data[data_len++] = 1;

	return data_len;
}
void send_PowerConASK_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_PowerConASK, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	 B35  计费模型召测下行数据（扩展）  
             B36  计费模型召测上行数据        不启用
 *********************************************************************/	
// 计费模型召测下行数据（扩展）B35
uint8_t recv_FeeModelB35_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = GUN_A;
	ANPEI_Recv_FeeModelUPB35 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvFeeModelUPB35;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, 7);

	return TRUE;

}

void recv_FeeModelB35_anpei_Succ(uint8_t u8Port)
{

	// SetSendEnable(u8Port, ANPEI_S_FeeModelB36Ask, SEND_ENABLE_ON);
	// Send_Immediately(u8Port, ANPEI_S_FeeModelB36Ask);

	return;
}

//	B36. 计费模型召测上行数据
uint16_t send_FeeModelB36Ask_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	// uint8_t *data = (uint8_t *)pdata;
	// uint8_t plieNumber[8] = {0};
	// ANPEI_Recv_FeeModelUPB35 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvFeeModelUPB35;

	// uint8_t new_data[] = {0x68, 0x72, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D};
	// size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	// memcpy(&data[data_len], new_data, new_data_len);
	// data_len += new_data_len;

	// AnpeiGet_PlatNumberBCD(plieNumber);
	// for (uint8_t k = 0; k < 8; k++)
	// {
	// 	data[data_len++] = plieNumber[7-k];
	// }

	// data[data_len++] = pRecvIdenf->CP56time[0];
	// data[data_len++] = pRecvIdenf->CP56time[1];
	// data[data_len++] = pRecvIdenf->CP56time[2];
	// data[data_len++] = pRecvIdenf->CP56time[3];
	// data[data_len++] = pRecvIdenf->CP56time[4];
	// data[data_len++] = pRecvIdenf->CP56time[5];
	// data[data_len++] = pRecvIdenf->CP56time[6];

	// // 判断当前运行的费率是否是B2类型的费率
	// uint8_t flag = 0; //
	// if (Now_billingmodel_and_num[u8Port][8] == 0x2)
	// 	flag = 1;
	// else
	// 	flag = 0; // 没有B2类型费率

	// if (0 == flag)
	// {
	// 	for (uint8_t i = 0; i < 85; i++)
	// 		data[data_len++] = 0;

	// 	data[data_len++] = 1; // 失败
	// 	return data_len;
	// }

	// 根据ID查询是哪套B2费率
	// uint8_t Nomber = 0;
	// for (uint8_t i = 0; i < 8; i++)
	// {
	// 	if (Now_billingmodel_and_num[u8Port][i] != FeemodelB2save_data[0].billing_model[i])
	// 	{
	// 		Nomber = 0xFF;
	// 		break;
	// 	}
	// }

	// if (Nomber == 0xFF)
	// {
	// 	Nomber = 1;
	// 	for (uint8_t i = 0; i < 8; i++)
	// 	{
	// 		if (Now_billingmodel_and_num[u8Port][i] != FeemodelB2save_data[1].billing_model[i])
	// 		{
	// 			Nomber = 0xFF;
	// 			break;
	// 		}
	// 	}
	// }

	// 若还是不存在费率ID //默认是不会存在B2类型费率
	// if (1)
	// {
	// 	for (uint8_t i = 0; i < 85; i++)
	// 		data[data_len++] = 0;

	// 	data[data_len++] = 1; // 失败
	// 	return data_len;
	// }

	
	// // 计费模型生效时间 7
	// memcpy(&data[data_len], FeemodelB2save_data[Nomber].start_time, 7);
	// data_len = data_len + 7;
	// // 计费模型失效时间 7
	// memcpy(&data[data_len], FeemodelB2save_data[Nomber].end_time, 7);
	// data_len = data_len + 7;

	// // 计量类型 2
	// memcpy(&data[data_len], FeemodelB2save_data[Nomber].Meterkind, 2);
	// data_len = data_len + 2;

	// // 48时段
	// memcpy(&data[data_len], FeemodelB2save_data[Nomber].segmentation_rate, 48);
	// data_len = data_len + 48;

	// // 费率数
	// data[data_len++] = 4;

	// memcpy(&data[data_len], FeemodelB2save_data[Nomber].sharp_ele_fee, 20);
	// data_len = data_len + 20;

	// data[data_len++] = 0; //

	return data_len;
}

void send_FeeModelB36Ask_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_FeeModelB36Ask, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	 B37. 充电中车辆监测数据   //不需要
 *********************************************************************/	
//	B37. 充电中车辆监测数据
uint16_t send_ChgCarInf_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	return data_len;
}
void send_ChgCarInf_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_ChgCarInf, SEND_ENABLE_OFF);
	return;
}
/********************************************************************
 * @brief 	B38. 零点示值上报(扩展) 
  
 *********************************************************************/	
//B38. 零点示值上报(扩展) 
// 读取数据用于累加，仅开机读取
void Init_data_to_Add(void)
{
	for(uint8_t i=0;i<GUN_NUM_MAX_ANPEI;i++)
	{
     memset(UP_0000_clock_data[i].total_00_valleyEE,0,4);
	 memset(UP_0000_clock_data[i].total_00_valleyEE,0,4);
	 memset(UP_0000_clock_data[i].total_00_valleyEE,0,4);
	 memset(UP_0000_clock_data[i].total_00_valleyEE,0,4);
	 memset(UP_0000_clock_data[i].total_00_valleyEE,0,4);
	}

}
//保存
void save_data_to_Add(void)
{
	//在充电中时变换大于0.001存一次
    static uint32_t EEdata_last[GUN_NUM_MAX_ANPEI][5]={0};//上一次 总 尖 峰 平 谷 充电量
	uint32_t EEdata_totalnow[GUN_NUM_MAX_ANPEI][5]={0};   //现在   总 尖 峰 平 谷 充电量
    uint8_t arryval[4]={0};
	uint32_t Value=0;
	for(uint8_t i=0;i<GUN_NUM_ANPEI;i++)
	{

        if(0==fourUint8ToUint32(chrg_EE_Money[i].total_EE))
		{
            EEdata_last[i][0]=0;
			EEdata_last[i][1]=0;
			EEdata_last[i][2]=0;
			EEdata_last[i][3]=0;
			EEdata_last[i][4]=0;
			continue;
		}
		   


		EEdata_totalnow[i][0]=fourUint8ToUint32(chrg_EE_Money[i].total_EE);
		EEdata_totalnow[i][1]=fourUint8ToUint32(chrg_EE_Money[i].total_sharpEE);
		EEdata_totalnow[i][2]=fourUint8ToUint32(chrg_EE_Money[i].total_peakEE);
		EEdata_totalnow[i][3]=fourUint8ToUint32(chrg_EE_Money[i].total_flatEE);
		EEdata_totalnow[i][4]=fourUint8ToUint32(chrg_EE_Money[i].total_valleyEE);

		if(EEdata_totalnow[i][0]>EEdata_last[i][0]&&(EEdata_totalnow[i][0]-EEdata_last[i][0])>=1)
		  {           
               //更新数值
			   //总=上一次存储值+前后充电量的差值
			   Value=fourUint8ToUint32(UP_0000_clock_data[i].total_00_EE)+(EEdata_totalnow[i][0]-EEdata_last[i][0]);
               uint32ToFourUint8(arryval,Value);
               memcpy(UP_0000_clock_data[i].total_00_EE,arryval,4);

               //尖
			   Value=fourUint8ToUint32(UP_0000_clock_data[i].total_00_sharpEE)+(EEdata_totalnow[i][1]-EEdata_last[i][1]);
               uint32ToFourUint8(arryval,Value);
               memcpy(UP_0000_clock_data[i].total_00_sharpEE,arryval,4);

			   //峰
			   Value=fourUint8ToUint32(UP_0000_clock_data[i].total_00_peakEE)+(EEdata_totalnow[i][2]-EEdata_last[i][2]);
               uint32ToFourUint8(arryval,Value);
               memcpy(UP_0000_clock_data[i].total_00_peakEE,arryval,4);

			   //平
			   Value=fourUint8ToUint32(UP_0000_clock_data[i].total_00_flatEE)+(EEdata_totalnow[i][3]-EEdata_last[i][3]);
               uint32ToFourUint8(arryval,Value);
               memcpy(UP_0000_clock_data[i].total_00_flatEE,arryval,4);

			   //谷
			   Value=fourUint8ToUint32(UP_0000_clock_data[i].total_00_valleyEE)+(EEdata_totalnow[i][4]-EEdata_last[i][4]);
               uint32ToFourUint8(arryval,Value);
               memcpy(UP_0000_clock_data[i].total_00_valleyEE,arryval,4);


			   //更新上一次值
			   EEdata_last[i][0]=fourUint8ToUint32(chrg_EE_Money[i].total_EE);
			   EEdata_last[i][1]=fourUint8ToUint32(chrg_EE_Money[i].total_sharpEE);
			   EEdata_last[i][2]=fourUint8ToUint32(chrg_EE_Money[i].total_peakEE);
			   EEdata_last[i][3]=fourUint8ToUint32(chrg_EE_Money[i].total_flatEE);
			   EEdata_last[i][4]=fourUint8ToUint32(chrg_EE_Money[i].total_valleyEE);
			  

		  }



	}


	
}

//判断是否到达24点
void ANPEI_24clock_UP(void)
{
	static uint8_t clock2400_UPflag[GUN_NUM_MAX_ANPEI]={0};//当天是否已经上传 0未 1已经

   //判断是否到0:0:0
	uint8_t timeOrgin[7] = {0};//(年H 年L 月 日 时 分 秒)
	getRunTimeYYMDHMS(timeOrgin); 
	if(timeOrgin[1]>=25&&timeOrgin[4]==0&&timeOrgin[5]==0) //年份大于25年，至少保证已经校时
	{
        if(timeOrgin[6]>=0)
		  {

			for(uint8_t i=0;i<GUN_NUM_ANPEI;i++)
			{
				if(!dev_getErrExsit(i, eErr_PlatformOffline)&&clock2400_UPflag[i]!=1)//平台在线&未上传
				{
					clock2400_UPflag[i]=1;
					SetSendEnable(i, ANPEI_S_2400Inf, SEND_ENABLE_ON);
			        Send_Immediately(i, ANPEI_S_2400Inf);

				}
				
				
			}
			

		
		  }


	}
	else//不管是否上报完成过0:01后就清零
	{
		clock2400_UPflag[0]=0;
		clock2400_UPflag[1]=0;

	}
	


}
//开机读取一次 充电中每1s存储一次 到点上传
void ANPEI_24clock_Deal(void)
{
   static uint8_t readflag=0;
   static uint32_t periotimes=0;  
   if(0==readflag)
     {
		readflag=1;
		periotimes=NOWTICK;
        Init_data_to_Add();//开机初始化一下
	 }

     
    if(JudgeTimeOutMs(periotimes, eTick_1S) == TRUE) 
	{
		save_data_to_Add();//保存数据用于累加
		periotimes=NOWTICK;
	}

	ANPEI_24clock_UP();

}
//B38 上报
uint16_t send_2400Inf_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}
	//	 充电接口标识 1

	data[data_len++] = u8Port;
	// 0点尖  4
	memcpy(&data[data_len], UP_0000_clock_data[u8Port].total_00_sharpEE, 4);
	data_len = data_len + 4;

	// 0点 峰  4
	memcpy(&data[data_len], UP_0000_clock_data[u8Port].total_00_peakEE, 4);
	data_len = data_len + 4;
	// 0点 平  4
	memcpy(&data[data_len], UP_0000_clock_data[u8Port].total_00_flatEE, 4);
	data_len = data_len + 4;
	// 0点 谷 4
	memcpy(&data[data_len], UP_0000_clock_data[u8Port].total_00_valleyEE, 4);
	data_len = data_len + 4;
	// 0点总 4
	memcpy(&data[data_len], UP_0000_clock_data[u8Port].total_00_EE, 4);
	data_len = data_len + 4;

	return data_len;
}
void send_2400Inf_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_2400Inf, SEND_ENABLE_OFF);
	return;
}
/********************************************************************
 * @brief 	 B40. 平台ftp服务器地址上行（扩展）
             B39 下发ftp服务器帧 
 *********************************************************************/	
//	B40. 平台ftp服务器地址上行（扩展）
uint16_t send_ftpInfAsk_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x16, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	data[data_len++] = 0;

	return data_len;
}
void send_ftpInfAsk_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_ftpInfAsk, SEND_ENABLE_OFF);

    SetSendEnable(u8Port, ANPEI_S_U, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_U);
	return;
}

// B39 下发ftp服务器帧 
uint8_t recv_ftpInf_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = GUN_A;
	ANPEI_Recv_Update_ftp *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvUpdata;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data, sizeof(ANPEI_Recv_Update_ftp));

	return TRUE;
}

void recv_ftpInf_Succ_anpei(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_ftpInfAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_ftpInfAsk);

	return;
}
/********************************************************************
 * @brief B45. 充电功率召测下行（扩展）
           B46. 充电功率召测上行（扩展）  
 *********************************************************************/	
// B45. 充电功率召测下行（扩展）
uint8_t recv_PowerVal_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
    uint8_t u8Port = r_data[8];

	gun[0] = u8Port;


	return TRUE;
}

void recv_PowerVal_Succ_anpei(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_PowerValAsk, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_PowerValAsk);

	return;
}
// B46. 充电功率召测上行（扩展）
uint16_t send_PowerValAsk_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x23, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1

	data[data_len++] = u8Port;
	// 控制功率 4
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl]) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl] >> 8) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl] >> 16) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerControl] >> 24) & 0xFF;
	// 默认功率 4
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault]) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] >> 8) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] >> 16) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDefault] >> 24) & 0xFF;
	// 动态功率 4
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic]) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic] >> 8) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic] >> 16) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].CtrlPower[E_platPowerDynamic] >> 24) & 0xFF;
	// 成功标志
	data[data_len++] = 0;

	return data_len;
}
void send_PowerValAsk_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_PowerValAsk, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	B47.下发计费模型下行数据—分时服务费
            B48.下发计费模型上行数据—分时服务费
 *********************************************************************/	
// 费率下发B47
uint8_t B47errflag = 0;
uint8_t recv_Rate_SETB47_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	ANPEI_Recv_Rate_ModelB47 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB47;

	if (u8Port >= GUN_NUM)
		return FALSE;

	gun[0] = u8Port;

	B47errflag=0;

	memset(pRecvIdenf,0,sizeof(ANPEI_Recv_Rate_ModelB47));
    memcpy(&pRecvIdenf->device_number, r_data, len);//此处长度不定,不用sizeof(结构体); 


   //判断理论长度跟实际接收的对不对
   uint8_t  num=pRecvIdenf->billing_modelB47.time_allnum;//收到的时段数量
	uint8_t theoretical_length =8+1+8+7+7+2+1+num*sizeof(Fee_data);//理论长度
     
	if(len==theoretical_length)//一样，正常新版本数据
		printf(".........receive...Newbilmode.........\r\n");
	else //与理论数值不同。
	     {
			printf(".........receive...errbilmode.........\r\n");
			B47errflag=1;//给1			
		 }	


	    return TRUE;
}

void recv_Rate_SETB47_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_Rate_ModelB47 *pRecvRateModel = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB47;
	uint8_t  num=pRecvRateModel->billing_modelB47.time_allnum;

	if(B47errflag!=1)
		{			
			for (uint8_t i = 0; i < num; i++)
			{
			 if (pRecvRateModel->billing_modelB47.B47modeldata[i].Serial_rate > 8)
			 {
				 B47errflag = 1;
				 break;
			 }
		   }

		}
	

	if (1 != B47errflag) //费率正常
	{
		uint8_t length=8+7+7+2+1+num*sizeof(Fee_data); //FeeModelB47 实际用到的长度 billing_model 8个字节+ start_time 7个字节 以此类推
		save_rateB47_model_anpei(&pRecvRateModel->billing_modelB47,u8Port,length); // 保存费率B47//
		//清除费率故障
		 Clear_FeemodelREE(u8Port);
	}
		

	SetSendEnable(u8Port, ANPEI_S_Rate_SETAskB48, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_Rate_SETAskB48);

	return;
}

//	B48. 下发计费模型上行数据—分时服务费
uint16_t send_Rate_SETAskB48_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;
	ANPEI_Recv_Rate_ModelB47 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB47;



	uint16_t data_len = 0;

	uint8_t new_data[] = {0x68, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 计费模型ID
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[0];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[1];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[2];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[3];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[4];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[5];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[6];
	data[data_len++] = pRecvIdenf->billing_modelB47.billing_model[7];

	if (B47errflag)
		data[data_len++] = 1;
	else
		data[data_len++] = 0;

	return data_len;
}
void send_Rate_SETAskB48_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_Rate_ModelB47 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateModelB47;

	//由于B47收到的字节长度不定，避免下次数据重叠，此处在回送后清空数据
    memset(&pRecvIdenf->billing_modelB47, 0, sizeof(pRecvIdenf->billing_modelB47));

	SetSendEnable(u8Port, ANPEI_S_Rate_SETAskB48, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	B49. 计费模型切换生效上行—分时服务费
            B50  计费模型切换生效下行—分时服务费 
 *********************************************************************/	
//判断是否到达切换点
void ANPEI_FEEModel_Refresh(void)
{
bool Value_enable[2]= {0}; //真实有效

   for(uint8_t i=0;i<GUN_NUM;i++)
   {
	 //两套费率均存在且有效
	  	Value_enable[B47_A]=ANPEI_Is_FeeModel_Valid(&anpei_feeModel_save[i].FeemodelB47save_data[B47_A]);
	    Value_enable[B47_B]=ANPEI_Is_FeeModel_Valid(&anpei_feeModel_save[i].FeemodelB47save_data[B47_B]);       
	   if(true==Value_enable[B47_A]&&true==Value_enable[B47_B])
	   {                          
	           //最近上报的 跟 最近更新的费率不一致
				if(Rate_Swtich_datamodel_and_num[i][8]!=anpei_feeModel_save[i].RecentUpdates_Nomber)
	              {
                         //判断是否达到切换点,直接判断时间是否超过 
						 uint8_t _Nomber = anpei_feeModel_save[i].RecentUpdates_Nomber;
					     uint8_t timebill[7] = {0};                                       //(年H 年L 月 日 时 分 秒)
						 uint8_t timeOrgin[7] = {0};
						 CP56Time2a_to_Time(anpei_feeModel_save[i].FeemodelB47save_data[_Nomber].start_time,timebill);
						 uint32_t total_seconds = Anpei_time_to_seconds(timebill); 
						 
						 getRunTimeYYMDHMS(timeOrgin); 
						 //确保时间正常>2000年（未校准前为1979年）
						if(timeOrgin[0]!=20)return;
						uint32_t total_seconds_Now=Anpei_time_to_seconds(timeOrgin);//当前秒数
                        
						if(total_seconds_Now>=total_seconds)
						{
                            //开机阶段 最近上报的费率是第 0xFF套 ，说明开机后时间就已经大于最新费率的切换点，
							//说明在切换后断电重启了。
							if(Rate_Swtich_datamodel_and_num[i][8]==0xff)
							{
								//更新值即可，不上报
								memcpy(&Rate_Swtich_datamodel_and_num[i][0], anpei_feeModel_save[i].FeemodelB47save_data[_Nomber].billing_model, 8); 
                                Rate_Swtich_datamodel_and_num[i][8]=_Nomber;
							    Rate_Swtich_datamodel_and_num[i][9]=anpei_feeModel_save[i].FeemodelB47save_data[_Nomber].time_allnum;

								continue; //结束这次循环，继续下一次
							}

							//非充电中需顺便更新当前及之后应运行的费率
							uint8_t state = GetPile_gun_state(i);
							if( state==eChargeState_Idle|| state==eChargeState_Waiting|| state==eChargeState_StopFinish)
							   Refresh_NowbillModel(i);
                            
							//充电中保持旧费率，更新应当发送给平台的切换的费率  
							memcpy(&Rate_Swtich_datamodel_and_num[i][0], anpei_feeModel_save[i].FeemodelB47save_data[_Nomber].billing_model, 8); 
                            Rate_Swtich_datamodel_and_num[i][8]=_Nomber;
							Rate_Swtich_datamodel_and_num[i][9]=anpei_feeModel_save[i].FeemodelB47save_data[_Nomber].time_allnum;

							SetSendEnable(i, ANPEI_S_Rate_Swtich, SEND_ENABLE_ON);
							Send_Immediately(i, ANPEI_S_Rate_Swtich);
						}
				
					  
	              }



         
	    }
	 
   }



}
//	B49. 计费模型切换生效上行—分时服务费
uint16_t send_Rate_Swtich_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;

	uint8_t new_data[] = {0x68, 0x26, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}
	//	 充电接口标识 1

	data[data_len++] = u8Port;

	// 计费模型ID
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][0];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][1];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][2];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][3];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][4];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][5];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][6];
	data[data_len++] = Rate_Swtich_datamodel_and_num[u8Port][7];

	// 根据ID查询是哪套B47费率
	uint8_t Nomber = Rate_Swtich_datamodel_and_num[u8Port][8];
			 
	if (Nomber == 0xFF)
	{
		// 切换时间
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;
		data[data_len++] = 0;

		// 切换结果
		data[data_len++] = 1; // 失败
	}
	// 切换时间
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[0];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[1];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[2];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[3];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[4];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[5];
	data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time[6];

	// 切换结果
	data[data_len++] = 0;
	return data_len;
}
void send_Rate_Swtich_anpei_Succ(uint8_t u8Port)
{
	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_Rate_SwtichAsk))
	{
		SetRecvEnable(u8Port, ANPEI_R_Rate_SwtichAsk, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_Rate_SwtichAsk, Get_Systick());
	}

	return;
}

// B50  计费模型切换生效下行—分时服务费
uint8_t recv_Rate_SwtichAsk_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	ANPEI_Recv_Rate_switchB50 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvRateswitchB50;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data, sizeof(ANPEI_Recv_Rate_switchB50));

	return TRUE;
}

void recv_Rate_SwtichAsk_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_Rate_Swtich, SEND_ENABLE_OFF);

	return;
}

/********************************************************************
 * @brief 	B51 计费模型召测下行数据-分时服务费
            B52. 计费模型召测上行数据—分时服务费  //长度不定
 *********************************************************************/	
// B51 计费模型召测下行数据-分时服务费
uint8_t recv_FeeModelB51_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	uint8_t u8Port = r_data[8];
	ANPEI_Recv_FeeModelUPB51 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvFeeModelUPB51;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, 8);

	return TRUE;
}

void recv_FeeModelB51_anpei_Succ(uint8_t u8Port)
{
	SetSendEnable(u8Port, ANPEI_S_FeeModelB52Ask, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_FeeModelB52Ask);
	return;
}


//	B52. 计费模型召测上行数据—分时服务费  //长度不定
uint16_t send_FeeModelB52Ask_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};
	ANPEI_Recv_FeeModelUPB51 *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvFeeModelUPB51;

	uint8_t new_data[] = {0x68, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	// 接口标识
	data[data_len++] = pRecvIdenf->intreface;

	// 时间戳
	data[data_len++] = pRecvIdenf->CP56time[0];
	data[data_len++] = pRecvIdenf->CP56time[1];
	data[data_len++] = pRecvIdenf->CP56time[2];
	data[data_len++] = pRecvIdenf->CP56time[3];
	data[data_len++] = pRecvIdenf->CP56time[4];
	data[data_len++] = pRecvIdenf->CP56time[5];
	data[data_len++] = pRecvIdenf->CP56time[6];

	uint8_t state=GetPile_gun_state(u8Port);
   if(eChargeState_Idle==state||eChargeState_Waiting==state||eChargeState_StopFinish==state)//非充电状态刷新费率
	  Refresh_NowbillModel(u8Port);//刷新下费率
           
	// 根据ID查询是哪套B47费率
	uint8_t Nomber = Now_billingmodel_and_num[u8Port][8];


	// 若还是不存在费率ID
	if (Nomber == 0xff)
	{
		for (uint8_t i = 0; i < (8 + 7 + 7 + 1 + 13); i++)
			data[data_len++] = 0;

		data[data_len++] = 1; // 失败

		// 因为长度不定 这一步修改传送的长度(默认是0x9D，实际需要根据实际修改)
		data[1] = 13 + 8 + 1 + 7 + (8 + 7 + 7 + 1 + 13) + 1;

		return data_len;
	}

	// ID
	memcpy(&data[data_len], anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].billing_model, 8);
	data_len = data_len + 8;

	// 计费模型生效时间 7
	memcpy(&data[data_len], anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].start_time, 7);
	data_len = data_len + 7;
	// 计费模型失效时间 7
	memcpy(&data[data_len], anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].end_time, 7);
	data_len = data_len + 7;

	// 时段数量
	uint8_t numval = 0;
	numval = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].time_allnum; //Now_billingmodel_and_num[u8Port][9]也可以
	data[data_len++] = numval;

	// 时段序号1~ numval；
	for (uint8_t i = 0; i < numval; i++)
	{
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].Serial_number;
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].Serial_rate;

		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].rate_start[0];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].rate_start[1];

		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].rate_end[0];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].rate_end[1];

		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ele_fee[0];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ele_fee[1];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ele_fee[2];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ele_fee[3];

		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ser_fee[0];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ser_fee[1];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ser_fee[2];
		data[data_len++] = anpei_feeModel_save[u8Port].FeemodelB47save_data[Nomber].B47modeldata[i].ser_fee[3];
	}

	// 成功
	data[data_len++] = 0; //

	// 因为长度不定 这一步修改传送的长度(默认是0x9D，实际需要根据实际修改)
	data[1] = 13 + 8 + 1 + 7 + (8 + 7 + 7 + 1 + 14 * numval) + 1;

	return data_len;
}
void send_FeeModelB52Ask_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_FeeModelB52Ask, SEND_ENABLE_OFF);
	return;
}

/********************************************************************
 * @brief 	B53. 在线情况下停止充电上传分时交易明细数据 //////长度
            B54  在线分时明细交易包下行数据   
 *********************************************************************/	
//	B53. 在线情况下停止充电上传分时交易明细数据 //////长度
void ANPEI_DealUpdate_Cmd(uint8_t u8Port)
{
	printf("........... send..B53... \r\n");
	SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_ON);
	Send_Immediately(u8Port, ANPEI_S_onlineEnd_ChgInfB53);

}
uint16_t send_onlineEnd_ChgInfB53_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	union ChargeRecordUnion *UpRecord = &g_chgData[u8Port].DealRecord.ChgRecord;

	uint16_t data_len = 0;
	uint8_t *data = (uint8_t *)pdata;
	uint8_t plieNumber[8] = {0};

	uint8_t new_data[] = {0x68, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}

	// 接口标识
	data[data_len++] = u8Port;

	// 交易流水号 16
    memcpy(&data[data_len], UpRecord->AnpeiChgRecord.transaction_log_num, 16);
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.transaction_log_num, 16);
	data_len = data_len + 16;

	// 时段个数 1
	uint8_t num = 0;
	num = UpRecord->AnpeiChgRecord.FeeModel_timenum;
	data[data_len++] = num;

	// 时段1-num数据
	for (uint8_t i = 0; i < num; i++)
	{
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].time_serrnumber;
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].time_kind;

		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalpower[0];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalpower[1];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalpower[2];

		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalmoney[0];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalmoney[1];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_totalmoney[2];

		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_servemomey[0];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_servemomey[1];
		data[data_len++] = UpRecord->AnpeiChgRecord.Fee_B53data[i].chrg_servemomey[2];
	}
	// 充电开始时间 7
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.chrg_start_time, 7);
	data_len = data_len + 7;
	// 充电结束时间 7
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.chrg_stop_time, 7);
	data_len = data_len + 7;
	// 累计充电时间 2
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.chrg_chrg_totaltime, 2);
	data_len = data_len + 2;
	// 充电费 3
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.chrg_total_money, 3);
	data_len = data_len + 3;
	// 服务费 3
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.sver_total_money, 3);
	data_len = data_len + 3;
	// 总电量 3
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.total_total_power, 3);
	data_len = data_len + 3;

	// 总起示值 4
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.total_start_power, 4);
	data_len = data_len + 4;

	// 总止示值 4
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.total_stop_power, 4);
	data_len = data_len + 4;
	// 充电前 S0C 2
	data[data_len++] = 0;
	data[data_len++] = 0;
	// 结束后 SoC 2
	data[data_len++] = 0;
	data[data_len++] = 0;
	// 物理卡号 8
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.Logic_card_number, 8);
	data_len = data_len + 8;
	// 电动汽车唯一标识 32
	for (uint8_t i = 0; i < 32; i++)
		data[data_len++] = 0;

	// 停止充电原因 2
	memcpy(&data[data_len], UpRecord->AnpeiChgRecord.stop_reason, 2);

	data_len = data_len + 2;

	data[1] = 13 + 8 + 1 + 16 + 1 + num * 11 + 7 + 7 + 2 + 3 + 3 + 3 + 4 + 4 + 2 + 2 + 8 + 32 + 2;

	return data_len;
}
void send_onlineEnd_ChgInfB53_anpei_Succ(uint8_t u8Port)
{
   CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (RECV_ENABLE_ON != GetRecvEnable(u8Port, ANPEI_R_onlineEnd_ChgInfAckB54))
	{
		SetRecvEnable(u8Port, ANPEI_R_onlineEnd_ChgInfAckB54, RECV_ENABLE_ON);
		SetRecvTick(u8Port, ANPEI_R_onlineEnd_ChgInfAckB54, Get_Systick());
	}


	pChgGunData->upDealCnt++;

	if (pChgGunData->upDealCnt >= 10)
	{
		// 上报10次未回复，停止上报,重连
		if (SEND_ENABLE_ON == GetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53))
		{
			SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_OFF);
		}
	}
	else if (pChgGunData->upDealCnt == 1)
	{
		pChgGunData->ExistChargeDeal = 0;
	}

	return;
}

// B54  在线分时明细交易包下行数据
uint8_t recv_onlineEnd_ChgInfAckB54_anpei(uint8_t *r_data, int len, uint8_t *gun)
{
	uint8_t u8Port = r_data[8];
	ANPEI_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvOnlinetrans_ask;

	if (u8Port >= GUN_NUM_ANPEI)
		return FALSE;

	gun[0] = u8Port;

	memcpy(pRecvIdenf, r_data + 8, sizeof(ANPEI_Recv_Online_ask));

	return TRUE;
}

void recv_onlineEnd_ChgInfAckB54_anpei_Succ(uint8_t u8Port)
{
	ANPEI_Recv_Online_ask *pRecvIdenf = &g_ProtocolDCB.pANPEIRecvData[u8Port].strRecvOnlinetrans_ask;
	RecordB53 *pRecord = &g_chgData[u8Port].DealRecord.ChgRecord.AnpeiChgRecord;

	// if (0x01 != pRecvIdenf->resultask)
	// {
		// 清除安培的实时上传的信息的结构体 JJUNIVE
		Clear_chrg_EE_Money_data(u8Port);
		SetSendEnable(u8Port, ANPEI_S_onlineEnd_ChgInfB53, SEND_ENABLE_OFF);

        updatePileStopReason(u8Port, Pile_Stop_Reason_Finish);
        
		ANpeiUpChargeStorageDeal(u8Port, (void *)pRecord, 0xFF);
	// }

	return;
}

/********************************************************************
 * @brief 	 B55 离线情况下停止充电上传分时交易明细数据  //不做
             B56 离线分时明细交易包下行数据
 *********************************************************************/	
// B55. 离线情况下停止充电上传分时交易明细数据  //不做

uint16_t send_offlineEnd_ChgInfB55_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	return data_len;
}
void send_offlineEnd_ChgInfB55_anpei_Succ(uint8_t u8Port)
{

	SetSendEnable(u8Port, ANPEI_S_offlineEnd_ChgInfB55, SEND_ENABLE_OFF);
	return;
}

// B56 离线分时明细交易包下行数据
uint8_t recv_offlineEnd_ChgInfAckB56_anpei(uint8_t *r_data, int len, uint8_t *gun)
{

	return TRUE;
}

void recv_offlineEnd_ChgInfAckB56_anpei_Succ(uint8_t u8Port)
{
	return;
}

/********************************************************************
 * @brief 	B57. 充电功率控制过程中的扩展实时状态
 *********************************************************************/	
//	B57. 充电功率控制过程中的扩展实时状态
// ANPEI_S_ChgPowerCon_inf
uint16_t send_ChgPowerCon_inf_anpei(uint8_t u8Port, void *pdata, uint16_t inlen)
{
	uint16_t data_len = 0;

	uint8_t plieNumber[8] = {0};
	uint8_t *data = (uint8_t *)pdata;
	uint8_t new_data[] = {0x68, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29};
	size_t new_data_len = sizeof(new_data) / sizeof(new_data[0]);
	memcpy(&data[data_len], new_data, new_data_len);
	data_len += new_data_len;

	AnpeiGet_PlatNumberBCD(plieNumber);
	for (uint8_t k = 0; k < 8; k++)
	{
		data[data_len++] = plieNumber[7-k];
	}
	//	 充电接口标识 1
	data[data_len++] = u8Port;

	// 时间戳 7
	U8 timeOrgin[7] = {0};
	getRunTimeYYMDHMS(timeOrgin); // 年H 年L 月 日 时 分 秒
    U8 Cp56time[7]={0};
	Time_to_CP56Time2a(timeOrgin,Cp56time);

	data[data_len++] = Cp56time[0];
	data[data_len++] = Cp56time[1];
	data[data_len++] = Cp56time[2];
	data[data_len++] = Cp56time[3];
	data[data_len++] = Cp56time[4];
	data[data_len++] = Cp56time[5];
	data[data_len++] = Cp56time[6];

	// 充电输出电压
	uint32_t tempval = 0;
	tempval = GetPile_ChgOutVol(u8Port, 1);
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 充电输出电流
	tempval = GetPile_ChgOutCur(u8Port, 2);
	data[data_len++] = tempval & 0x00ff;
	data[data_len++] = ((tempval >> 8) & 0x00ff);

	// 是否连接电池 1
	data[data_len++] = 0;
	// 充电功率 2

	data[data_len++] =  (stuGunPowerCtr[u8Port].ActPower) & 0xFF;
	data[data_len++] = (stuGunPowerCtr[u8Port].ActPower >> 8) & 0xFF;
	// soc 2
	data[data_len++] = 0;
	data[data_len++] = 0;

	// 电压需求 2
	data[data_len++] = 0;
	data[data_len++] = 0;
	// 电流需求 2
	data[data_len++] = 0;
	data[data_len++] = 0;

	return data_len;
}
void send_ChgPowerCon_inf_anpei_Succ(uint8_t u8Port)
{
	
	return;
}
/********************************************************************
 * @brief 	 接收等待时间判断处理
 * @return   false 未超时  true 超时     
 *********************************************************************/	
uint8_t ANPEIUpCtrlRecvTimer(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	int32_t start_tick = GetRecvTick(u8Port, cmd);

	if ((start_tick < 0) || (0xffffffff == Cyc))
		return FALSE;

	if (ANPEI_R_Heart == cmd)
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
uint8_t UpCtrlSendCycAnpei(uint8_t u8Port, uint32_t cmd, uint32_t Cyc)
{
	uint32_t start_tick = GetSendTick(u8Port, cmd);
	uint8_t u8SendImmdFlag = GetSendImmdFlag(u8Port, cmd);
	up_gun_data_ctrl *pUpGunData = &g_ProtocolDCB.strUpGunData[u8Port];

	if (TRUE == u8SendImmdFlag)
		return TRUE;

	if (ANPEI_S_RealData == cmd )
	{
		if (eUP_Gun_State_Work_anpei == pUpGunData->up_gun_state)
			Cyc = eTick_30S;
	}

	if (ANPEI_S_ChgPowerCon_inf == cmd)
	{
		if (eUP_Gun_State_Work_anpei == pUpGunData->up_gun_state)
            Cyc = stuGunPowerCtr[u8Port].timepre;
	}

	if (JudgeTimeOutMs(start_tick, Cyc))
		return TRUE;

	return FALSE;
}






/***************************发送系列函数********************************************************************************/
typedef uint8_t (*PSendCyc)(uint8_t u8Port, uint32_t cmd, uint32_t Cyc);
typedef uint16_t (*PSend)(uint8_t u8Port, void *pBuf, uint16_t u32BufSize);
typedef void (*PSendSucc)(uint8_t u8Port);

typedef struct
{
	uint32_t cmd;
	uint8_t FType;
	uint32_t cyc;
	PSendCyc pSendCyc;
	PSend pSend;
	PSendSucc pSendSucc;
} ANPEI_Send_ctrl;

// #define  ANPEI_SEND_IMMD 0

const ANPEI_Send_ctrl StrANPEISendCtrl[] = {
	{ANPEI_S_Identification, UP_S_FRAME_SELF, eTick_30S, UpCtrlSendCycAnpei, send_Identification_anpei, send_Identification_anpei_Succ}, //
	{ANPEI_S_U, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_U_anpei, send_U_anpei_Succ},
	{ANPEI_S_Heart, UP_S_FRAME_SELF, eTick_30S, UpCtrlSendCycAnpei, send_Heart_anpei, send_Heart_anpei_Succ}, //
	{ANPEI_S_clocksyn, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_clocksyn_anpei, send_clocksyn_anpei_Succ},

	{ANPEI_S_RealData, UP_S_FRAME_SELF, eTick_60S * 2, UpCtrlSendCycAnpei, send_RealData_anpei, send_RealData_anpei_Succ},
	{ANPEI_S_Rate_SETAskB3, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_Rate_SETAskB3_anpei, send_Rate_SETAskB3_anpei_Succ},
	{ANPEI_S_StartEnd_ChgAsk, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_StartEnd_ChgAsk_anpei, send_StartEnd_ChgAsk_anpei_Succ},
	{ANPEI_S_Cardinf, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_Cardinf_anpei, send_Cardinf_anpei_Succ},
	{ANPEI_S_CardStart_Chg, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_CardStart_Chg_anpei, send_CardStart_Chg_anpei_Succ},

	{ANPEI_S_onlineEnd_ChgInfB12, UP_S_FRAME_SELF, eTick_60S, UpCtrlSendCycAnpei, send_onlineEnd_ChgInfB12_anpei, send_onlineEnd_ChgInfB12_anpei_Succ},
	{ANPEI_S_offlineEnd_ChgInfB15, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_offlineEnd_ChgInfB15_anpei, send_offlineEnd_ChgInfB15_anpei_Succ},
	{ANPEI_S_RemoteUpgradeAck, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_RemoteUpgradeAck_anpei, send_RemoteUpgradeAck_anpei_Succ},
	{ANPEI_S_FixtimeCmdAsk, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_FixtimeCmdAsk_anpei, send_FixtimeCmdAsk_anpei_Succ},
	{ANPEI_S_SIMInfAck, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_SIMInfAck_anpei, send_SIMInfAck_anpei_Succ},
	{ANPEI_S_PowerConASK, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_PowerConASK_anpei, send_PowerConASK_anpei_Succ},
	{ANPEI_S_FeeModelB36Ask, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_FeeModelB36Ask_anpei, send_FeeModelB36Ask_anpei_Succ},
	{ANPEI_S_ChgCarInf, UP_S_FRAME_SELF, eTick_30S, UpCtrlSendCycAnpei, send_ChgCarInf_anpei, send_ChgCarInf_anpei_Succ},
	{ANPEI_S_2400Inf, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_2400Inf_anpei, send_2400Inf_anpei_Succ},
	{ANPEI_S_ftpInfAsk, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_ftpInfAsk_anpei, send_ftpInfAsk_anpei_Succ},
	{ANPEI_S_PowerValAsk, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_PowerValAsk_anpei, send_PowerValAsk_anpei_Succ},
	{ANPEI_S_Rate_SETAskB48, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_Rate_SETAskB48_anpei, send_Rate_SETAskB48_anpei_Succ},
	{ANPEI_S_Rate_Swtich, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_Rate_Swtich_anpei, send_Rate_Swtich_anpei_Succ},
	{ANPEI_S_FeeModelB52Ask, UP_S_FRAME_ACK, eTick_15S, UpCtrlSendCycAnpei, send_FeeModelB52Ask_anpei, send_FeeModelB52Ask_anpei_Succ},
	{ANPEI_S_onlineEnd_ChgInfB53, UP_S_FRAME_SELF, eTick_60S, UpCtrlSendCycAnpei, send_onlineEnd_ChgInfB53_anpei, send_onlineEnd_ChgInfB53_anpei_Succ},
	{ANPEI_S_offlineEnd_ChgInfB55, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_offlineEnd_ChgInfB55_anpei, send_offlineEnd_ChgInfB55_anpei_Succ},
	{ANPEI_S_ChgPowerCon_inf, UP_S_FRAME_SELF, eTick_15S, UpCtrlSendCycAnpei, send_ChgPowerCon_inf_anpei, send_ChgPowerCon_inf_anpei_Succ},

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
} ANPEI_Recv_ctrl;

const ANPEI_Recv_ctrl StrANPEIRecvCtrl[] = {

	{ANPEI_R_Identification, eTick_30S, ANPEIUpCtrlRecvTimer, recv_login_data_anpei, recv_login_anpei_Succ},		 //
	{ANPEI_R_U, eTick_10S, ANPEIUpCtrlRecvTimer, recv_U_anpei, recv_U_anpei_Succ},									 //
	{ANPEI_R_Heart, eTick_30S*5, ANPEIUpCtrlRecvTimer, recv_R_Heart_anpei, recv_R_Heart_anpei_Succ},					 //
	{ANPEI_R_clocksyn, 0xFFFFFFF, ANPEIUpCtrlRecvTimer, recv_clocksyn_anpei, recv_clocksyn_anpei_Succ},				 //
	{ANPEI_R_Rate_SETB2, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_Rate_SETB2_anpei, recv_Rate_SETB2_anpei_Succ},		 //
	{ANPEI_R_StartEnd_Chg, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_StartEnd_Chg_anpei, recv_StartEnd_Chg_anpei_Succ}, //
																													 //
	{ANPEI_R_CardinfAck, eTick_30S, ANPEIUpCtrlRecvTimer, recv_CardinfAck_anpei, recv_CardinfAck_anpei_Succ},
	//
	{ANPEI_R_CardStart_ChgAck, eTick_30S, ANPEIUpCtrlRecvTimer, recv_CardStart_ChgAck_anpei, recv_CardStart_ChgAck_anpei_Succ}, //
	{ANPEI_R_onlineEnd_ChgInfAckB13, eTick_30S, ANPEIUpCtrlRecvTimer, recv_onlineEnd_ChgInfAckB13_anpei, recv_onlineEnd_ChgInfAckB13_anpei_Succ},
	//
	{ANPEI_R_ChgDeduction_Inf, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_ChgDeduction_Inf_anpei, recv_ChgDeduction_Inf_anpei_Succ}, //
	{ANPEI_R_offlineEnd_ChgInfAckB16, eTick_30S, ANPEIUpCtrlRecvTimer, recv_offlineEnd_ChgInfAckB16_anpei, recv_offlineEnd_ChgInfAckB16_anpei_Succ},
	{ANPEI_R_RemoteUpgrade, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_RemoteUpgrade_anpei, recv_RemoteUpgrade_anpei_Succ}, //
	{ANPEI_R_FixtimeCmd, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_FixtimeCmd_anpei, recv_FixtimeCmd_anpei_Succ},
	{ANPEI_R_NeedSIMInf, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_NeedSIMInf_anpei, recv_NeedSIMInf_anpei_Succ}, //
	{ANPEI_R_PowerCon, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_PowerCon_anpei, recv_PowerCon_anpei_Succ},
	{ANPEI_R_FeeModelB35, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_FeeModelB35_anpei, recv_FeeModelB35_anpei_Succ},
	{ANPEI_R_ftpInf, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_ftpInf_anpei, recv_ftpInf_Succ_anpei},
	{ANPEI_R_PowerVal, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_PowerVal_anpei, recv_PowerVal_Succ_anpei},
	{ANPEI_R_Rate_SETB47, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_Rate_SETB47_anpei, recv_Rate_SETB47_anpei_Succ},
	{ANPEI_R_Rate_SwtichAsk, eTick_30S, ANPEIUpCtrlRecvTimer, recv_Rate_SwtichAsk_anpei, recv_Rate_SwtichAsk_anpei_Succ},
	{ANPEI_R_FeeModelB51, 0xffffffff, ANPEIUpCtrlRecvTimer, recv_FeeModelB51_anpei, recv_FeeModelB51_anpei_Succ},
	{ANPEI_R_onlineEnd_ChgInfAckB54, eTick_30S, ANPEIUpCtrlRecvTimer, recv_onlineEnd_ChgInfAckB54_anpei, recv_onlineEnd_ChgInfAckB54_anpei_Succ},
	{ANPEI_R_offlineEnd_ChgInfAckB56, eTick_30S, ANPEIUpCtrlRecvTimer, recv_offlineEnd_ChgInfAckB56_anpei, recv_offlineEnd_ChgInfAckB56_anpei_Succ},

};


/********************************************************************
 * @brief 	 启动认证
 * @return   
 *********************************************************************/	
void ANPEIUpLogin(void)
{
	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;

	if (!Comm_getIpSuces(eSocket_GPRS1))
	{
		return;
	}

	if (eOnline_Off == Get_PlatConnectSta())
	{
		Set_PlatConnectSta(eOnline_Start);

		SetSendEnable(GUN_A, ANPEI_S_Identification, SEND_ENABLE_ON);
		Send_Immediately(GUN_A, ANPEI_S_Identification);
	}
}

uint8_t ANPEIUpChargeRecordUpDeal(void)
{
	return FALSE;
}

void ANPEIUpCtrlTaskDeal(void) // 任务状态处理
{
	uint8_t i = 0;

	ANPEIUpLogin();

	for (i = 0; i < GUN_NUM_ANPEI; i++)
	{
		ANPEIUpGunStateCheck(i);
        
        anpeiCostUpdate(i);     //订单实时更新
	}

	ANPEIUpChargeRecordUpDeal();

	return;
}

/********************************************************************
 * @brief 	 发送任务处理
 * @return   
 *********************************************************************/	
static uint16_t ANPEIUpCtrlSendDeal(void *pBuf, uint32_t u32BufSize)
{
	const ANPEI_Send_ctrl *pANPEISendCtrl = NULL;
	uint32_t u32i;
	uint16_t outLen = 0;
	uint8_t i = 0;

	uint8_t *pData = (uint8_t *)pBuf;
	for (i = 0; i < GUN_NUM_ANPEI; i++)  
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrANPEISendCtrl); u32i++)
		{
			pANPEISendCtrl = &StrANPEISendCtrl[u32i];

			if (SEND_ENABLE_ON != GetSendEnable(i, pANPEISendCtrl->cmd))
				continue;

			if (TRUE == pANPEISendCtrl->pSendCyc(i, pANPEISendCtrl->cmd, pANPEISendCtrl->cyc))
			{

				if ((outLen = pANPEISendCtrl->pSend(i, pData, u32BufSize)) > 0)
				{

					pANPEISendCtrl->pSendSucc(i);
					SetSendTick(i, pANPEISendCtrl->cmd, Get_Systick());
					SetSendFlag(i, pANPEISendCtrl->cmd, SEND_FLAG_YES);
					SetSendImmdFlag(i, pANPEISendCtrl->cmd, FALSE);

					UPRINT("\r\nUpProtocol --> GUN: %d, SendDealcmd: %x \r\n", i, pANPEISendCtrl->cmd);
					return outLen;
				}
			}
		}
	}

	return outLen;
}


static void ANPEIUpSendDeal(void)
{
	uint8_t pbuf[TCP_DATA_LEN_MAX] = {0};
	uint16_t outLen = 0;

	if (eSocket_Online != GPRS_Getonlineflag(eSocket_GPRS1))
		return;

	outLen = ANPEIUpCtrlSendDeal(pbuf, sizeof(pbuf));

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
static uint8_t Deal_ANSYfromRec(uint8_t len, uint8_t TypeIDE, uint8_t control, uint8_t recordKind, int recLen)
{
	uint8_t cmd = 0;
	// 根据inlen, TypeIDE 和 Cot 判断是哪个cmd
	// 协议帧，U帧，心跳帧为特殊格式
	if (len == 0x01 && recLen == 12)					 //
		cmd = ANPEI_R_Identification;					 // 认证应答 例如 68 01 1900010000000031 0000
	else if (len == 4 && recLen == 6 && control == 0x0B) // U帧应答 68 04 0B000000
		cmd = ANPEI_R_U;
	else if (len == 4 && recLen == 6 && control == 0x83) // 心跳帧应答 680483000000
		cmd = ANPEI_R_Heart;
	else
	{
		if (TypeIDE == 103)
			cmd = ANPEI_R_clocksyn;
		else if (TypeIDE == 130)
		{
			if (recordKind == 2)
				cmd = ANPEI_R_onlineEnd_ChgInfAckB13;
			else if (recordKind == 3)
				cmd = ANPEI_R_offlineEnd_ChgInfAckB16;
		}
		else if (TypeIDE == 133)
		{
			if (recordKind == 5)
				cmd = ANPEI_R_Rate_SETB2;
			else if (recordKind == 21)
				cmd = ANPEI_R_StartEnd_Chg;
			else if (recordKind == 2)
				cmd = ANPEI_R_CardinfAck;
			else if (recordKind == 12)
				cmd = ANPEI_R_CardStart_ChgAck;
			else if (recordKind == 3)
				cmd = ANPEI_R_ChgDeduction_Inf;
			else if (recordKind == 15)
				cmd = ANPEI_R_RemoteUpgrade;
			else if (recordKind == 54)
				cmd = ANPEI_R_FixtimeCmd;
			else if (recordKind == 57)
				cmd = ANPEI_R_NeedSIMInf;
			else if (recordKind == 58)
				cmd = ANPEI_R_PowerCon;
			else if (recordKind == 59)
				cmd = ANPEI_R_FeeModelB35;
			else if (recordKind == 60)
				cmd = ANPEI_R_ftpInf;
			else if (recordKind == 63)
				cmd = ANPEI_R_PowerVal;
			else if (recordKind == 64)
				cmd = ANPEI_R_Rate_SETB47;
			else if (recordKind == 65)
				cmd = ANPEI_R_Rate_SwtichAsk;
			else if (recordKind == 66)
				cmd = ANPEI_R_FeeModelB51;
			else if (recordKind == 67)
				cmd = ANPEI_R_onlineEnd_ChgInfAckB54;
			else if (recordKind == 68)
				cmd = ANPEI_R_offlineEnd_ChgInfAckB56;
		}
	}

	return cmd;
}
//命令对应接收报文处理
void ANPEIUpCtrlRecvDeal(ANPEI_HEAD_T *pHead, uint8_t cmd, void *pindata, uint16_t inlen)
{
	const ANPEI_Recv_ctrl *pANPEIRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t port = 0;

	for (u32i = 0; u32i < ARRAY_SIZE(StrANPEIRecvCtrl); u32i++)
	{
		pANPEIRecvCtrl = &StrANPEIRecvCtrl[u32i];
		//
		if (cmd == pANPEIRecvCtrl->cmd)
		{
			if (TRUE == pANPEIRecvCtrl->pRecv(pindata, inlen, &port))
			{
				pANPEIRecvCtrl->pRecvSucc(port);

				SetRecvTick(port, cmd, Get_Systick());

				UPRINT("\r\nUpProtocol --> GUN: %d, RecvDealcmd: 0x%x \r\n", port, cmd);
				break;
			}
		}
	}

	return;
}
// 判断tcp接收到的所有数据是否合法
static int ANPEI_Tcp_Read_Data_Check(uint8_t *r_data)
{
	if (r_data[0] != 0x68)
	{
		printf("Check head erro  0x%x\r\n", r_data[0]);
		return -1;
	}

	return 0;
}

void ANPEIfrom_buffer_data(U8 *recv_buf, U16 *len)
{
	U16 read_len = 0;

	PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, recv_buf, len, TCP_DATA_LEN_MAX);

	if (recv_buf[0] == 0x68)
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



void ANPEIPackConnectHandle(U8 *recv_buf, int totalLen)
{
	int surplusLen = totalLen;
	int currentIndex = 0;
	ANPEI_HEAD_T *pHead = NULL;
	uint8_t ANPEICmd = 0;

	while (surplusLen)
	{
		pHead = (ANPEI_HEAD_T *)(recv_buf + currentIndex);
        
		 int packLen = 0;
		
		if(pHead->len==0x01)//只有协议帧的第二个字不是表示长度，单独做处理
		{          
          packLen=12;
		}
		else
		  packLen = pHead->len + 2;

		// 防止乱数据导致程序死掉
		if (packLen > surplusLen)
		{
			return;
		}
		surplusLen = surplusLen - packLen;

		// printf("PackConnectHandle: %d   %d %d\r\n", totalLen, surplusLen, packLen);

		if (ANPEI_Tcp_Read_Data_Check(recv_buf + currentIndex) < 0)
		{
			return;
		}

		hex_dump("tcp_recv_data", recv_buf + currentIndex, packLen);

		ANPEICmd = Deal_ANSYfromRec(pHead->len, pHead->TypeIDE, pHead->control[0], pHead->recordKind, packLen);
        if(ANPEICmd!=0)
		{
		  if (ANPEICmd >= 0xF1 && ANPEICmd <= 0xF8) // 这部分协议非正常格式，需要额外处理
			ANPEIUpCtrlRecvDeal(pHead, ANPEICmd, recv_buf + currentIndex, packLen);
		  else 
			ANPEIUpCtrlRecvDeal(pHead, ANPEICmd, recv_buf + currentIndex + sizeof(ANPEI_HEAD_T), packLen-sizeof(ANPEI_HEAD_T)); // packlen:		
		}
		

		currentIndex = currentIndex + packLen;
	}
}

void ANPEIUpRecvDeal(void)
{
	uint8_t from_tcp_data[TCP_DATA_LEN_MAX]={0};
	U16 r_len = 0;
	ANPEI_HEAD_T *pHead = NULL;

	// PalRecvPop(eDataID_1, eDataType_TCP, NULL, NULL, from_tcp_data, (U16 *)&r_len, TCP_DATA_LEN_MAX);

	ANPEIfrom_buffer_data(from_tcp_data, &r_len);
	if (r_len == 0)
		return;
	if (r_len > TCP_DATA_LEN_MAX)
	{
		printf("\r\nprotocol--> recv buf full ! ");
		return;
	}
	// 粘包处理
	ANPEIPackConnectHandle(from_tcp_data, r_len);

	return;
}
/********************************************************************
 * @brief 	 接收超时处理
 * @return   
 *********************************************************************/	
void ANPEIRecvOutTimeDeal(uint8_t u8Port, uint32_t cmd)
{
  CHG_DATA_T *pChgGunData = &g_chgData[u8Port];

	if (ANPEI_R_Heart == cmd)
	{
		DB_UpOfflineDeal();
	}

	if(ANPEI_R_onlineEnd_ChgInfAckB54==cmd) 
	{
       if(pChgGunData->upDealCnt>=10)
	   {
		pChgGunData->upDealCnt=0;
	   }
	     
	}

   if(ANPEI_R_CardStart_ChgAck == cmd)
	{
		SetSendEnable(u8Port, ANPEI_S_CardStart_Chg, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, ANPEI_R_CardStart_ChgAck, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);
	}
   
	if(ANPEI_R_CardinfAck==cmd)
	{
		SetSendEnable(u8Port, ANPEI_S_Cardinf, SEND_ENABLE_OFF);
		SetRecvEnable(u8Port, ANPEI_R_CardinfAck, RECV_ENABLE_OFF);
		monitor_set_MonitorState(u8Port, eMonitorState_Service);
        g_CardInfoClear(u8Port);

	}

	return;
}

void ANPEIUpCtrlRecvOutTime(void)
{
	const ANPEI_Recv_ctrl *pANPEIRecvCtrl = NULL;
	uint32_t u32i;
	uint8_t i = 0;

	for (i = 0; i < GUN_NUM_ANPEI; i++)
	{
		for (u32i = 0; u32i < ARRAY_SIZE(StrANPEIRecvCtrl); u32i++)
		{
			pANPEIRecvCtrl = &StrANPEIRecvCtrl[u32i];

			if (RECV_ENABLE_ON != GetRecvEnable(i, pANPEIRecvCtrl->cmd))
				continue;

			if (TRUE == pANPEIRecvCtrl->pRecvTimer(i, pANPEIRecvCtrl->cmd, pANPEIRecvCtrl->timer))
			{
				ANPEIRecvOutTimeDeal(i, pANPEIRecvCtrl->cmd);
			}
		}
	}
	return;
}



/********************************************************************
 * @brief 	主函数任务
 * @return   
 *********************************************************************/	

void ANPEIUpProtocolDeal(void)
{
    static uint8_t first_Open_feeAndbill_dataflag=0;//首次上电开机
	static uint8_t first_Open_wait_powrefresh=0;//首次上电等待默认功率更新

	ProtocolDCB *pProtocolDCB = &g_ProtocolDCB;
    
      if(0==first_Open_feeAndbill_dataflag)
	  {
        first_Open_feeAndbill_dataflag=1;
		Read_rateB47_model_anpei();//开机从指定位置读取费率并更新出当前应该所在的费率ID
		for(uint8_t i=0;i<GUN_NUM_ANPEI;i++)
		{
			Clear_chrg_EE_Money_data(i);
		}	
	  }

	  if (0 == first_Open_wait_powrefresh)
	  {
		  if (0 != ANPEIGet_devpow()) // 已经更新了默认值再去更新
		  {
			  first_Open_wait_powrefresh = 1;

			  for (uint8_t k = 0; k< GUN_NUM_ANPEI; k++)
			  {
				  powerInit(k);
			  }
		  }
	  }

	if (NULL == pProtocolDCB->pANPEIRecvData)
		return;

	ANPEIUpCtrlTaskDeal();	  // 任务状态处理
	ANPEIUpRecvDeal();		  // 接收处理
	ANPEIUpSendDeal();		  // 发送处理
	ANPEIUpCtrlRecvOutTime(); // 超时处理

	//ANPEI_fix_Revers_timeTask(); // 定时/预约处理
	ANPEI_refreshpowertoPile();	 // 功率刷新处理
   // ANPEI_24clock_Deal();//零点数据处理：开机读取 充电中1s存储 零点上报
	ANPEI_FEEModel_Refresh();//费率表切换上报处理

	return;
}