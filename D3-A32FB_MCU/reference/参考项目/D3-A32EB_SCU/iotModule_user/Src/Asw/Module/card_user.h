#ifndef __IC_CAR_H
#define __IC_CAR_H

/***读取区域***/
#define NULL_BLOCK      0x00
#define NUM_BLOCK       0x04    //卡号
#define MONEY_BLOCK     0x05    //金额
#define SUPER_BLOCK     0x06    //解锁/限制超级卡
#define PREPAID_BLOCK   0x08    //预扣金额/单价

//达客云区域
#define DaKeYun_NUM_BLOCK              0xC  //卡号


/* 刷卡操作流程 */
#define WORK_START        0x00
#define SEARCH_SN_END     0x01
#define READ_ID_END       0x02
#define READ_MONEY_END    0x03
#define WRITE_MONEY_START 0x04
#define CUTS_MONEY_START  0x05
#define PREPAID_END       0x06
#define SUPER_CARD_END    0x07
#define SAME_CARD_END     0x08
#define WORK_END          0x09

//卡操作位定义
typedef struct
{
  	uint8_t ExistCard_ok_1b    :1;//存在卡，1存在 0不存在
	uint8_t ValidCard_ok_2b    :2;//卡号  :1,合法卡 2,非法卡
	uint8_t CardRevs_5b    	   :5;//预留
} _16BIT_IC_OPT_T;

typedef struct
{
	uint8_t type[2];      //卡类型
	_16BIT_IC_OPT_T opt_sts;//卡操作状态
	uint8_t step;         //操作步骤
	uint8_t seri[4];      //卡序号
	uint8_t nums[16];       //卡号         高低
	// uint8_t money_8u[16];   //卡金额        低高
	//	uint8_t prepaid_8u[16]; //预扣金额及单价缓存 低高
	// uint8_t super_8u[16];   //超级卡信息缓存 低高
	//	uint8_t write_money_8u[16]; //写金额 低高
	// uint8_t invalid_flg;//无效卡标识:8,表示超级卡 9,表示绑定卡取消提醒

	uint8_t SeriNumbers[4];			//物理卡号
	uint8_t LogicNumbers[16];		//逻辑卡号
	
	uint32_t ExistCardTimer;      		//ic卡存在时间，超时使用
	uint32_t NoneCardTimer;      		//ic卡拿开时间，超时使用
	//....
}IC_T;


/******外部调用*******/
//extern IC_T ic_g;

/*卡信息*/
extern IC_T *Get_IC_Msg(uint8_t ch);
/*是否存在有效卡*/
uint8_t Get_IC_ValidCard(uint8_t ch);
/*是否存在卡*/
uint8_t Get_IC_ExistCard(uint8_t ch);


extern uint8_t Get_InvalidCard_Flg(void);
extern void IC_Handle_Main(void);

void IcDealTask(void);

#endif


