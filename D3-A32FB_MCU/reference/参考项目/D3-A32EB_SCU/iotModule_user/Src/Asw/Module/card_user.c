/***********************************************************************************
 * 文 件 名  : card_user.c
 * 版 本 号  : V1.0
 * 负 责 人  : WEEN
 * 创建日期  : 2021-9-16
 * 文件描述  : 读卡器驱动函数
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  : 
 * 其    他  : 
 * 修改日志  : 初版
***********************************************************************************/
#include "string.h"
#include "common.h"
#include "card_user.h"
#include "md5.h"
#include "card_user.h"
#include "rf_card.h"


#include "cmsis_os2.h"
#include "AppHeaderSummary.h"
#include "FreeRTOS.h"
#include "task.h"


#define CardUserPintf(fmt,args...)	\
		do {								\
            debug(fmt ,##args); 	\
		} while(0)


static IC_T ic_g_array[GUN_NUM_MAX];


// proximity
static uint8_t password_a_g8u[6];//密钥A
static uint8_t password_b_g8u[6];//密钥B

const uint8_t super_binding[] = {"binding"};
static uint8_t data_buf[32] = {0x00};//flash读写缓存
static uint8_t num0_arry[8] = {0x00};//0数组,用于判定存储区域是否存储了有效卡

static uint8_t Secret_A_Dakeyun[6] = {0xa0, 0x21, 0x06, 0x30, 0x17, 0x5f};



/*****************************************************************************
 * 函 数 名  : Get_Charge_Msg
 * 负 责 人  : WEEN
 * 创建日期  : 2021年5月1日
 * 函数功能  : 获取充电信息函数
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  : called by needs
 * 其    它  :   
*****************************************************************************/
IC_T *Get_IC_Msg(uint8_t ch)
{
  return &ic_g_array[ch];
}


/*****************************************************************************
 * 是否存在有效卡
*****************************************************************************/
uint8_t Get_IC_ValidCard(uint8_t ch)
{
  return ic_g_array[ch].opt_sts.ValidCard_ok_2b;
}


/*****************************************************************************
 * 是否识别到IC卡
*****************************************************************************/
uint8_t Get_IC_ExistCard(uint8_t ch)
{
  return ic_g_array[ch].opt_sts.ExistCard_ok_1b;
}



/*************************************************************
  Function:    CMP_para_1
  Description: 数字比较处理
  Calls:       
  Called By:   
  Input:       
  Output:     
  Return:     
  Others:      
*************************************************************/
uint8_t CMP_para_1(uint8_t* pt1,uint8_t* pt2,uint8_t length)
{
  uint8_t i = 0;
  for(i = 0;i < length;i ++)
  {
    if(pt1[i] != pt2[i])
      return 0;
  }
  return 1;
}
/*************************************************************
  Function:    Bull_Check
  Description: 读取卡号校验码函数
  Calls:       
  Called By:   
  Input:       
  Output:   
  Return:      
  Others:      无
*************************************************************/
static uint8_t Bull_Check(uint8_t *pdata1_l8u,uint8_t *pdata2_l8u)
{
  uint8_t i;
  uint8_t check_result1_l8u = 0,check_result2_l8u = 0,check_result_l8u;
  for(i=0;i<4;i++)
  {
    check_result1_l8u ^= pdata1_l8u[i];
    check_result2_l8u ^= pdata2_l8u[i+1];
  }
  check_result_l8u = check_result1_l8u ^ check_result2_l8u;
  if(check_result_l8u == pdata2_l8u[0])
   return 1;
  else
   return 0;
}
/*************************************************************
  Function:    Secret_Key_A
  Description: local
  Calls:
  Called By:  local
  Input:       NULL
  Output:      NULL
  Return:      NULL
  Others:      
*************************************************************/
static void SecretKeyA(uint8_t *result, uint8_t *seri)
{
	uint8_t i;
	uint8_t md5_l8u[24];
	uint8_t secret_check_num_l8u[8];
	uint8_t string_A[24] = "bullevse666_user";
	
	unsigned char decrypt_result[16];//MD5结果存储数组

	for(i = 0;i<4;i++)
	{ 
		if((seri[i]>>4) < 10)
			md5_l8u[i * 2] = (seri[i]>>4) + 0x30;
		else
			md5_l8u[i * 2] = ((seri[i]>>4)-10) + 0x41;

		if((seri[i] & 0x0f) < 10)
			md5_l8u[i * 2 + 1] = (seri[i] & 0x0f) + 0x30;
		else
			md5_l8u[i * 2 + 1] = ((seri[i] & 0x0f)-10) + 0x41;
	}
	memcpy(&md5_l8u[8],string_A,16);
	MD5_API(decrypt_result, md5_l8u,24);
	for(i=0;i<8;i++)
		secret_check_num_l8u[i] = decrypt_result[i] ^ decrypt_result[15-i];
		 
	result[0] =  secret_check_num_l8u[0] ^ secret_check_num_l8u[7];
	result[1] =  secret_check_num_l8u[1] ^ secret_check_num_l8u[6];
	memcpy(&result[2],&secret_check_num_l8u[2],4);
}
/*************************************************************
  Function:    Secret_Key_B
  Description: 
  Calls:
  Called By:   local
  Input:       NULL
  Output:      NULL
  Return:      NULL
  Others:      
*************************************************************/
static void SecretKeyB(uint8_t *result, uint8_t *seri)
{
	uint8_t i;
	uint8_t string_B[20] = "bullevse666_admin888";
	uint8_t md5_l8u[28];
	uint8_t secret_check_num_l8u[8];

	unsigned char decrypt_result[16];//MD5结果存储数组

	for( i = 0;i<4;i++)
	{ 
		if((seri[i]>>4) < 10)
			md5_l8u[i * 2] = (seri[i]>>4) + 0x30;
		else
			md5_l8u[i * 2] = ((seri[i]>>4)-10) + 0x41;

		if((seri[i] & 0x0f) < 10)
			md5_l8u[i * 2 + 1] = (seri[i] & 0x0f) + 0x30;
		else
			md5_l8u[i * 2 + 1] = ((seri[i] & 0x0f)-10) + 0x41;
	}
	memcpy(&md5_l8u[8],string_B,20);
	MD5_API(decrypt_result, md5_l8u,28);
	for(i=0;i<8;i++)
		secret_check_num_l8u[i] = decrypt_result[i] ^ decrypt_result[15-i];

	password_b_g8u[0] = secret_check_num_l8u[0] ^ secret_check_num_l8u[7];
	result[1] = secret_check_num_l8u[1] ^ secret_check_num_l8u[6];
	memcpy(&result[2],&secret_check_num_l8u[2],4);
}


/*************************************************************
  Function:    SecretKeyGN
  Description: local
  Calls:
  Called By:  local
  Input:       NULL
  Output:      NULL
  Return:      NULL
  Others:      
*************************************************************/
static void SecretKeyGN(uint8_t AorB, uint8_t *seri)
{
	uint8_t i;
	uint8_t md5_l8u[28];
	uint8_t secret_check_num_l8u[8];
	uint8_t string_A[24] = "bullevse666_user";
	uint8_t string_B[20] = "bullevse666_admin888";
	uint8_t string[28] = "";
	uint8_t md5Len = 0;
	uint8_t *password = password_a_g8u;

	unsigned char decrypt_result[16];//MD5结果存储数组


	if (AorB == 1) {
		md5Len = 24;
		memcpy(string, string_A, 24);
		password = password_a_g8u;
	} else {
		md5Len = 28;
		memcpy(string, string_B, 20);
		password = password_b_g8u;
	}

	for(i = 0;i<4;i++)
	{ 
		if((seri[i]>>4) < 10)
			md5_l8u[i * 2] = (seri[i]>>4) + 0x30;
		else
			md5_l8u[i * 2] = ((seri[i]>>4)-10) + 0x41;

		if((seri[i] & 0x0f) < 10)
			md5_l8u[i * 2 + 1] = (seri[i] & 0x0f) + 0x30;
		else
			md5_l8u[i * 2 + 1] = ((seri[i] & 0x0f)-10) + 0x41;
	}
	memcpy(&md5_l8u[8],string,20);
	MD5_API(decrypt_result, md5_l8u, md5Len);

	for(i=0;i<8;i++)
		secret_check_num_l8u[i] = decrypt_result[i] ^ decrypt_result[15-i];
		 
	password[0] =  secret_check_num_l8u[0] ^ secret_check_num_l8u[7];
	password[1] =  secret_check_num_l8u[1] ^ secret_check_num_l8u[6];
	memcpy(&password[2],&secret_check_num_l8u[2],4);
}


//二次确认卡
uint8_t fsv_Card_SecConfirm(uint8_t ch, uint8_t *InPhysicalNum)
{
	uint8_t SecPhysicalNum[4];

	if(PcdAnticoll(ch, SecPhysicalNum) !=  MI_OK)//二次读卡序号,判断是否为同一张卡一直在感应区
	{
		return 0;
	}
	
	if(CMP_para_1(SecPhysicalNum, InPhysicalNum, 4))
	{
		return 1;
	}

	return 0;
}


//寻找卡, 并读取物理卡号
uint8_t fsv_Card_Search(uint8_t ch, uint8_t *OutCardType, uint8_t *OutPhysicalNum)
{
	if (PcdReadCard(ch, OutPhysicalNum, OutCardType) != MI_OK)
	{
		return 0;
	}
	//激活卡操作
	if(PcdSelect(ch, OutPhysicalNum) != MI_OK)      //选定卡
	{
		return 0;
	}

	return 1;
}


//读取逻辑卡号
uint8_t fsv_Card_ReadLogicNum(uint8_t ch, uint8_t *OutLogicNum, uint8_t *InSeri, uint8_t InBlock, uint8_t *InPassword)
{
	if(PcdAuthState(ch, PICC_AUTHENT1A, InBlock, InPassword, InSeri) != MI_OK)  //验证密钥
	{
		return 0;
	}

	if(PcdRead(ch, InBlock, OutLogicNum) != MI_OK) //读取卡号
	{
		return 0;
	}

	return 1;
}




//读取金额
uint8_t fsv_Card_ReadMoney(uint8_t ch, uint8_t *OutMoney, uint8_t *InSeri, uint8_t InBlock, uint8_t *InPassword)
{
	if(PcdAuthState(ch, PICC_AUTHENT1A, InBlock, InPassword, InSeri) != MI_OK)  //验证密钥
	{
		return 0;
	}
	if(PcdRead(ch, InBlock, OutMoney) != MI_OK)//读取金额
	{
		return 0;
	}
	return 1;
}

//写入金额
uint8_t fsv_Card_WriteMoney(uint8_t ch, uint8_t *InMoney, uint8_t *InSeri, uint8_t InBlock, uint8_t *InPassword)
{
	if(PcdAuthState(ch, PICC_AUTHENT1A, InBlock, InPassword, InSeri) != MI_OK)  //验证密钥
	{
		return 0;
	}
	
	if(PcdWrite(ch, InBlock, InMoney) != MI_OK)
	{
		return 0;
	}

	return 1;
}


//读取预扣金额及单价
uint8_t fsv_Card_ReadPrepaid(uint8_t ch, uint8_t *OutpPrepaid, uint8_t *InSeri, uint8_t InBlock, uint8_t *InPassword)
{
	if(PcdAuthState(ch, PICC_AUTHENT1B, InBlock, InPassword, InSeri) != MI_OK)  //验证密钥
	{
		return 0;
	}
	if(PcdRead(ch, InBlock, OutpPrepaid) != MI_OK)//读取预扣金额及单价
	{
		return 0;
	}
	return 1;
}


//卡离开之后将数据清零
void fsv_cardClearFlag(uint8_t ch)
{
	IC_T *ic_g = &ic_g_array[ch];

	memset(&ic_g->opt_sts, 0, 1);
	memset(ic_g->seri,0,4);
	memset(ic_g->nums,0,16);
}

void IC_Deal(uint8_t ch)
{
	uint8_t block = NUM_BLOCK;
	uint8_t password[6] = {0};
	
	uint8_t u8PortCard = ch;
	uint8_t u8Autennae = 0;     //天线开关，0表示全部打开
	//双枪刷卡AB枪调换下，随着硬件设计
    if(GUN_NUM == 2) {
		u8PortCard = (ch == 0) ? 1 : 0;
        u8Autennae = u8PortCard;
	}

	IC_T *ic_g = &ic_g_array[ch];
    
	//找卡，无卡返回
	if (fsv_Card_Search(u8PortCard, ic_g->type, ic_g->SeriNumbers)) {
		if (ic_g->opt_sts.ExistCard_ok_1b == 0) {
			//有卡标识
			ic_g->ExistCardTimer = NOWTICK;
			ic_g->opt_sts.ExistCard_ok_1b = 1;
			CardUserPintf("Exist Card: 0x%x%x%x%x\r\n", ic_g->SeriNumbers[0], ic_g->SeriNumbers[1], ic_g->SeriNumbers[2], ic_g->SeriNumbers[3]);
		}
		ic_g->ExistCardTimer = NOWTICK;
	}

		//持续200ms表示卡存在
	if (JudgeTimeOutMs(ic_g->ExistCardTimer, 200)) {
		
		if (ic_g->opt_sts.ExistCard_ok_1b == 1) {
			CardUserPintf("Remove Card\r\n");
			fsv_cardClearFlag(ch);
		}
	}

	//如果不存在卡，返回
	if (ic_g->opt_sts.ExistCard_ok_1b == 0) {
		return;
	}


	//公牛卡
	uint8_t platType = get_ChgParam_Card_type();
	if (platType == CARD_GN) {
		SecretKeyA(password_a_g8u, ic_g->SeriNumbers); 
		// SecretKeyB(password_b_g8u, ic_g->SeriNumbers);
        memcpy(password, password_a_g8u, 6);
	} else if (platType == CARD_DKY) {
        memcpy(password, Secret_A_Dakeyun, 6);
	} else if (platType == CARD_YKC) {
		ic_g->opt_sts.ValidCard_ok_2b = 1;
	} else if (platType == CARD_AHTT) {
		ic_g->opt_sts.ValidCard_ok_2b = 1;
	} else if (platType == CARD_HNCT) {
		ic_g->opt_sts.ValidCard_ok_2b = 1;
	}

	//读取逻辑卡号
	if (fsv_Card_ReadLogicNum(u8PortCard, ic_g->LogicNumbers, ic_g->SeriNumbers, block, password)) {
		
		if (ic_g->opt_sts.ValidCard_ok_2b == 0) {
			CardUserPintf("ValidCard: ");
			for(int i = 8;  i < 16; i++) {
				CardUserPintf("%02x", ic_g->LogicNumbers[i]);
			}
			CardUserPintf("\r\n");
		}

		ic_g->opt_sts.ValidCard_ok_2b = 1;//有效卡置位
	}
}

void IcDealTask()
{
	RF_Card_Init();
	
	while (1) {

		for (int i = 0; i < GUN_NUM; i++) {
			IC_Deal(i);
		}

		osDelay(100);
	}
}
