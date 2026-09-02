#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "AppHeaderSummary.h"
#include "ProtoLayerHeaderSummary.h"
#include "AppInputCfg.h"
#include "key.h"
#include "screenUart.h"


// #define mbsMasterPintf(fmt,args...)	\
// 		do {								\
//             debug(fmt ,##args); 	\
// 		} while(0)

#define mbsMasterPintf(fmt,args...)

#define Usart_MbsMaster	E_UART1_INDEX


static uint8_t g_mbsCommSta = 0; //0正常，1异常
enum {
	eNormal,
	eAbnormal
};

void fsv_SetMbsCommSta(uint8_t sta)
{
	g_mbsCommSta = sta;
}
uint8_t fgv_GetMbsCommSta()
{
	return g_mbsCommSta;
}

Stru_MbsMasterSta ls_MbsMasterSta;

static GN_PLATMOD  *lpm = &sg_platmod;	//用于本文件中

#define MBS_SEND_OFF	0
#define MBS_SEND_ON		1

typedef bool (*ModbusRegRecvFunc)(uint8_t ch,uint8_t *pData);
typedef struct {
	E_MbsCmd				CommCmd;
	uint32_t				TimeOutTick;
}Stru_MbsItrcInfo;

typedef struct {
	uint8_t enable;
	uint32_t sendTick;
}Stru_MbsItrcInfoSta;


Stru_MbsItrcInfoSta g_MbsPileItrcInfoSta[E_PileInfoMax];
Stru_MbsItrcInfoSta g_MbsGunItrcInfoSta[GUN_NUM_MAX][E_InteractionMaxCmd - E_PileInfoMax];


//mbs停止或开始发送
void fsv_cmdIfEnable(uint8_t ch, uint8_t cmd, uint8_t enable)
{
	//mbs交互状态
	Stru_MbsItrcInfoSta *lp_MbsItrcInfoStaObj = NULL;
	uint8_t lCmd = cmd;

	if ((cmd == E_PileInfoMax) || (cmd >= E_InteractionMaxCmd)) {
		printf("fsv_cmdIfEnable InVain: %d\r\n", cmd);
		return;
	}
	//桩
	if (cmd < E_PileInfoMax) {
		lp_MbsItrcInfoStaObj = &g_MbsPileItrcInfoSta[lCmd];
		lCmd = cmd;
	}
	//枪
	if (cmd > E_PileInfoMax) {
		lCmd = cmd - E_PileInfoMax - 1;
		lp_MbsItrcInfoStaObj = &g_MbsGunItrcInfoSta[ch][lCmd];
	}
	// printf("fsv_cmdIfEnable: %d %d %d\r\n", ch, cmd, enable);
	lp_MbsItrcInfoStaObj->enable = enable;
}

//返回可操作结构体
Stru_MbsItrcInfoSta *fsvp_getItrcInfoSta(uint8_t ch, uint8_t cmd)
{
	//mbs交互状态
	Stru_MbsItrcInfoSta *lp_MbsItrcInfoStaObj = NULL;
	uint8_t lCmd = cmd;

	if ((cmd == E_PileInfoMax) || (cmd >= E_InteractionMaxCmd)) {
		return lp_MbsItrcInfoStaObj;
	}
	//桩
	if (cmd < E_PileInfoMax) {
		lp_MbsItrcInfoStaObj = &g_MbsPileItrcInfoSta[lCmd];
	}
	//枪
	if (cmd > E_PileInfoMax) {
		lCmd = cmd - E_PileInfoMax - 1;
		lp_MbsItrcInfoStaObj = &g_MbsGunItrcInfoSta[ch][lCmd];
	}
	return lp_MbsItrcInfoStaObj;
}


//接收数据进行解析到结构体中
static bool RPileInfo(uint8_t ch,uint8_t *pInData){
	uint8_t len = 0;
	memcpy(lpm->pileInf.soft_ver, pInData + len, 4);
	len = len + 4;
	memcpy(lpm->pileInf.hw_version, pInData + len, 4);
	len = len + 4;
	memcpy(lpm->pileInf.protocol_ver, pInData + len, 4);
	len = len + 4;

	lpm->pileInf.dev_type = pInData[len];
	len = len + 1;
	lpm->pileInf.IotMod = pInData[len];
	len = len + 1;

	printf("RPileInfo ch = %d\r\n", ch);

    CmdHandle_PrintfAll();

    PlatDevNumberChange(1);

	return TRUE;
}
//接收数据进行解析到结构体中
static bool RPileInfoHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_ReadCmd_PileInfo, MBS_SEND_OFF);
    printf("RPileInfoHandle ch = %d\r\n", ch);
	return TRUE;
}

static bool RPileRtInfo(uint8_t ch,uint8_t *pInData){
	uint8_t len = 0;
	lpm->pile_rt_inf.pile_ack = pInData[len];
	len = len + 2;
	
	lpm->pile_rt_inf.pile_sta = pInData[len];
	lpm->pile_rt_inf.key = pInData[len + 1];
	len = len + 2;

	lpm->pile_rt_inf.inVoltageA = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->pile_rt_inf.inVoltageB = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->pile_rt_inf.inVoltageC = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->pile_rt_inf.pileTemp = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->pile_rt_inf.stopKey = pInData[len];

	return TRUE;
}
static bool RPileRtInfoHandle(uint8_t ch){
	if (!lpm->pile_rt_inf.pile_ack) {
		return FALSE;
	}
    
    printf("230 pile ack change\r\n");

	fsv_cmdIfEnable(ch, E_ReadCmd_PileInfo, MBS_SEND_ON);
	fsv_cmdIfEnable(ch, E_ReadCmd_PileCfgInfo, MBS_SEND_ON);
	fsv_cmdIfEnable(ch, E_WriteCmd_PileCfgAck, MBS_SEND_ON);
	return TRUE;
}


static bool WPileCfgInfo(uint8_t ch,uint8_t *pOutData){
    // memcpy(pOutData, &lpm->pileCfgInfo.gunNum, sizeof(GN_PILE_CFG_INF));
	pOutData[0] = lpm->pileCfgInfo.gunNum;
	pOutData[1] = 0;

	pOutData[2] = lpm->pileCfgInfo.fct_cfg.bits << 8;
	pOutData[3] = lpm->pileCfgInfo.fct_cfg.bits;
    printf("WPileCfgInfo: 0x%x  0x%x", pOutData[2], pOutData[3]);
	return TRUE;
}
static bool WPileCfgInfoHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_WriteCmd_PileCfgInfo, MBS_SEND_OFF);
	return TRUE;
}


static bool WPileCfgAck(uint8_t ch,uint8_t *pOutData){
	lpm->pile_rt_inf.pile_ack = 0;
	pOutData[0] = lpm->pile_rt_inf.pile_ack;
	pOutData[1] = lpm->pile_rt_inf.revs1;
	
	return TRUE;
}
static bool WPileCfgAckHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_WriteCmd_PileCfgAck, MBS_SEND_OFF);
	return TRUE;
}

static bool WPileTime(uint8_t ch,uint8_t *pOutData){
	getRunTimeYYMDHMS(lpm->pileCtrlInf.pileTime);

	printf("\r\nWPileTime: %d%d-%02d-%02d %02d:%02d:%02d\r\n\r\n", lpm->pileCtrlInf.pileTime[0], lpm->pileCtrlInf.pileTime[1], lpm->pileCtrlInf.pileTime[2], 
	lpm->pileCtrlInf.pileTime[3], lpm->pileCtrlInf.pileTime[4], lpm->pileCtrlInf.pileTime[5], lpm->pileCtrlInf.pileTime[6]);

	memcpy(pOutData, lpm->pileCtrlInf.pileTime, 8);
	return TRUE;
}
static bool WPileTimeHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_WriteCmd_TimeSync, MBS_SEND_OFF);
    printf("WPileTimeHandle off\r\n");
	return TRUE;
}
static bool WPileCtrlCmd(uint8_t ch,uint8_t *pOutData){
	pOutData[0] = lpm->pileCtrlInf.ctrlCmd;
	pOutData[1] = lpm->pileCtrlInf.ctrlRvs;
	
	fsv_cmdIfEnable(0, E_WriteCmd_CtrlDev, MBS_SEND_OFF);

	return TRUE;
}
static bool WPileCtrlCmdHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_WriteCmd_CtrlDev, MBS_SEND_OFF);
	return TRUE;
}
static bool RPileCfgInfo(uint8_t ch,uint8_t *pInData){
	uint8_t len = 0;
	lpm->pileCfgInfo.gunNum = pInData[len];
	len = len + 2;

	lpm->pileCfgInfo.fct_cfg.bits = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.pow_limit = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.max_voltage = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.min_voltage = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.max_current = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.target_current = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.max_gun_temp = pInData[len];
	lpm->pileCfgInfo.min_gun_temp = pInData[len+1];
	len = len + 2;
	
	lpm->pileCfgInfo.max_pile_temp = pInData[len];
	lpm->pileCfgInfo.min_pile_temp = pInData[len+1];
	len = len + 2;

	lpm->pileCfgInfo.warning_vol = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->pileCfgInfo.warning_cur = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	printf("RPileCfgInfo ch = %d\r\n", ch);
	return TRUE;
}

static bool RPileCfgInfoHandle(uint8_t ch){
	fsv_cmdIfEnable(0, E_ReadCmd_PileCfgInfo, MBS_SEND_OFF);
	printf("RPileCfgInfoHandle ch = %d\r\n", ch);
	return TRUE;
}

static bool RGunRtInfo(uint8_t ch,uint8_t *pInData){
	uint8_t len = 0;
	memcpy(&lpm->gun[ch].gunRtInfo.gunAck, pInData, 6);
	len = len + 6;
	
	lpm->gun[ch].gunRtInfo.cpValue = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->gun[ch].gunRtInfo.u_platChrgsta.bits = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;
	lpm->gun[ch].gunRtInfo.plugTemp = pInData[len];
	len = len + 2;
	lpm->gun[ch].gunRtInfo.gunWarn.bits = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->gun[ch].gunRtInfo.hardfault.bits = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;

	lpm->gun[ch].gunRtInfo.pileStopReason = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;



    len = 96;

	memcpy(&lpm->gun[ch].chrgingInfo.start_time, pInData+len, 8);
	len = len + 8;

	memcpy(&lpm->gun[ch].chrgingInfo.stop_time, pInData+len, 8);
	len = len + 8;
	
	lpm->gun[ch].chrgingInfo.start_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;
	
	lpm->gun[ch].chrgingInfo.stop_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;
	
	lpm->gun[ch].chrgingInfo.charge_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;

	lpm->gun[ch].chrgingInfo.charge_time = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;

	lpm->gun[ch].chrgingInfo.out_vol = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->gun[ch].chrgingInfo.out_cur = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->gun[ch].chrgingInfo.pwm = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;


	mbsMasterPintf("\r\nRtInfo = %d %d\r\n", ch, NOWTICK);
	return TRUE;
}

static bool WGunCtrlCharge(uint8_t ch,uint8_t *pOutData){
	uint8_t len = 0;
	pOutData[len] = lpm->gun[ch].cardCmd.cmd;
	pOutData[len+1] =lpm->gun[ch].cardCmd.stopReason;
	printf("\r\nWGunCtrlCharge  ch = %d   cmd = %d\r\n", ch, pOutData[len]);
	//控制完成后清零
	lpm->gun[ch].cardCmd.cmd = 0;
	return TRUE;
}
static bool WGunCtrlChargeHandle(uint8_t ch){
	fsv_cmdIfEnable(ch, E_WriteCmd_GunCtrlCmd, MBS_SEND_OFF);
	return TRUE;
}


static bool WGunSetCrtkAskInfo(uint8_t ch,uint8_t *pOutData){
	pOutData[0] = lpm->gun[ch].cardCmd.setPower >> 8;
	pOutData[1] =lpm->gun[ch].cardCmd.setPower & 0xFF;

	return TRUE;
}
static bool WGunSetCrtkInfoHandle(uint8_t ch){
	fsv_cmdIfEnable(ch, E_WriteCmd_GunCrtInfo, MBS_SEND_OFF);
	return TRUE;
}

static bool RGunChrgingInfo(uint8_t ch,uint8_t *pInData){
	uint8_t len = 0;
	memcpy(&lpm->gun[ch].chrgingInfo.start_time, pInData, 8);
	len = len + 8;

	memcpy(&lpm->gun[ch].chrgingInfo.stop_time, pInData+len, 8);
	len = len + 8;
	
	lpm->gun[ch].chrgingInfo.start_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;
	
	lpm->gun[ch].chrgingInfo.stop_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;
	
	lpm->gun[ch].chrgingInfo.charge_ele = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;

	lpm->gun[ch].chrgingInfo.charge_time = pInData[len] << 24 | pInData[len + 1] << 16 | pInData[len + 2] << 8 | pInData[len + 3];
	len = len + 4;

	lpm->gun[ch].chrgingInfo.out_vol = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->gun[ch].chrgingInfo.out_cur = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	lpm->gun[ch].chrgingInfo.pwm = pInData[len] << 8 | pInData[len + 1];
	len = len + 2;

	mbsMasterPintf("\r\nChrgingInfo = %d %d\r\n", ch, NOWTICK);

	return TRUE;
}

static bool EnableFlag(uint8_t ch){
	return TRUE;
}
static bool DisableFlag(uint8_t ch){
	return FALSE;
}
static bool ChargingFlag(uint8_t ch){
	if (lpm->gun[ch].gunRtInfo.u_platChrgsta.bit.charging_1b) {
		return TRUE;
	}
	return FALSE;
}

Stru_MbsInteraction  Stru_MbsPileInteraction_Map[] = {
	//写入
	{0,   200,		E_WriteCmd_PileCfgInfo, 	MODBUS_WRITE_MULTIPLE_REGS, 	PILE_CONFIG_OFFSET_ADR,		2,		WPileCfgInfo,	NULL,		WPileCfgInfoHandle	}, 
	{1,   200,		E_WriteCmd_PileCfgAck, 		MODBUS_WRITE_SINGLE_REG, 		PILE_RT_INFO_OFFSET_ADR,	1,		WPileCfgAck,	NULL,		WPileCfgAckHandle	}, 
	{0,   200,		E_WriteCmd_TimeSync, 		MODBUS_WRITE_MULTIPLE_REGS, 	PILE_CTRL_OFFSET_ADR,		4,		WPileTime,		NULL,		WPileTimeHandle		}, 
	{0,   200,		E_WriteCmd_CtrlDev, 		MODBUS_WRITE_SINGLE_REG, 		PILE_CTRL_OFFSET_ADR+4,		1,		WPileCtrlCmd,	NULL,		WPileCtrlCmdHandle	}, 

	//读取
	{1,    200,		E_ReadCmd_PileInfo,    		MODBUS_READ_REG, 				PILE_INF_OFFSET_ADR,		8,		NULL,			RPileInfo,		RPileInfoHandle	}, 
	{1,    100,		E_ReadCmd_PileRtInfo, 		MODBUS_READ_REG, 				PILE_RT_INFO_OFFSET_ADR,	7,		NULL,			RPileRtInfo,	RPileRtInfoHandle}, 
	{1,    200,		E_ReadCmd_PileCfgInfo, 		MODBUS_READ_REG, 				PILE_CONFIG_OFFSET_ADR,		11,		NULL,			RPileCfgInfo,	RPileCfgInfoHandle}, 

};

Stru_MbsInteraction  Stru_MbsGunInteraction_Map[] = {
	//写入
	{0,    200,		E_WriteCmd_GunCtrlCmd,    		MODBUS_WRITE_SINGLE_REG, 		GUN_CTRL_OFFSET_ADR,		1,		WGunCtrlCharge,				NULL,		WGunCtrlChargeHandle	}, 
	{0,    200,		E_WriteCmd_GunCrtInfo,    	    MODBUS_WRITE_SINGLE_REG, 	    GUN_CTRL_OFFSET_ADR+0x01,	1,		WGunSetCrtkAskInfo,		    NULL,		WGunSetCrtkInfoHandle	}, 

	//读取
	{1,    60,		E_ReadCmd_GunRtInfo,    		MODBUS_READ_REG, 				GUN_RT_INF_OFFSET_ADR,		0x50,		NULL,			RGunRtInfo,		NULL}, 
	// {0,    100,		E_ReadCmd_GunChrgingInfo,    	MODBUS_READ_REG, 				GUN_CHARGE_OFFSET_ADR,		0x13,	NULL,			RGunChrgingInfo,		NULL}, 
};

//开始充电
static uint8_t S_start_flg[GUN_NUM_MAX]={0};
void fgv_CtrlStartCharge(uint8_t ch)
{
	if (ch >= GUN_NUM) {
		printf("\r\nGun %d StartCharge Erro\r\n", ch);
		return;
	}
	S_start_flg[ch]=1;
	lpm->gun[ch].cardCmd.cmd = E_PLATMOD_CMD_START;
	fsv_cmdIfEnable(ch, E_WriteCmd_GunCtrlCmd, MBS_SEND_ON);
	printf("\r\n %d fgv_CtrlStartCharge\r\n", ch);
}

//停止充电
void fgv_CtrlStopCharge(uint8_t ch)
{
	if (ch >= GUN_NUM) {
		printf("\r\nGun %d StppCharge Erro\r\n", ch);
		return;
	}
    if (logic_get_gun_pwmEnable(ch) == 0) {
        return;
    }
	lpm->gun[ch].cardCmd.cmd = E_PLATMOD_CMD_STOP;
	fsv_cmdIfEnable(ch, E_WriteCmd_GunCtrlCmd, MBS_SEND_ON);
	printf("\r\nfgv_CtrlStopCharge\r\n");
}
//获取是否下发启动命令
uint8_t fgv_get_gun_startCmd(uint8_t ch)
{
	return S_start_flg[ch];
}
//清除下发的启动标志位
void fgv_clr_gun_startCmd(uint8_t ch)
{
	S_start_flg[ch]=0;
}

//功率设置
void fgv_CtrlChargeCrt(uint8_t ch, uint16_t power)
{
	if (ch >= GUN_NUM) {
		printf("\r\nGun %d set crt Erro\r\n", ch);
		return;
	}
	lpm->gun[ch].cardCmd.setPower = power;
	fsv_cmdIfEnable(ch, E_WriteCmd_GunCrtInfo, MBS_SEND_ON);
}

//设置需要进入厂内模式
void fgv_SetPileCfgFac(uint8_t mode)
{
    lpm->pileCfgInfo.fct_cfg.bit.cgfNeedFacMode = mode;
	fsv_cmdIfEnable(0, E_WriteCmd_PileCfgInfo, MBS_SEND_ON);
}
//设置设备为即插即充
void fgv_SetPileCfgOffLinChrg(uint8_t mode)
{
    lpm->pileCfgInfo.fct_cfg.bit.cgfPlugChrg = mode;
	fsv_cmdIfEnable(0, E_WriteCmd_PileCfgInfo, MBS_SEND_ON);
}

//给控制板下发时间
void fgv_PileSetTime()
{
	fsv_cmdIfEnable(0, E_WriteCmd_TimeSync, MBS_SEND_ON);
	// printf("\r\n set pile time\r\n");
}

//控制充电桩重启、恢复出场设置等操作
void fgv_CtrlPileOpr(uint8_t cmd)
{
	lpm->pileCtrlInf.ctrlCmd = cmd;
	fsv_cmdIfEnable(0, E_WriteCmd_CtrlDev, MBS_SEND_ON);
	printf("\r\n fgv_CtrlPileOpr cmd = %d\r\n", cmd);
}



static uint16_t Fun_MdbMasterReadReg(Stru_MbsInteraction *MbsSend, uint8_t *pOutDat)
{
	pOutDat[0] = LNLSB(MbsSend->num_8u);
	pOutDat[1] = LLSB(MbsSend->num_8u);
	return 2;
}
	
static uint16_t Fun_MdbMasterWriteSingleReg(uint8_t ch, Stru_MbsInteraction *MbsSend, uint8_t *pOutDat)
{
	if (MbsSend->pileRegSendFunc == NULL) {
		return 0;
	}
	MbsSend->pileRegSendFunc(ch, pOutDat);
	return 2;
}
static uint16_t Fun_MdbMasterWriteMultyReg(uint8_t ch, Stru_MbsInteraction *MbsSend, uint8_t *pOutDat)
{
	if (MbsSend->pileRegSendFunc == NULL) {
		return 0;
	}
	pOutDat[0] = LNLSB(MbsSend->num_8u);
	pOutDat[1] = LLSB(MbsSend->num_8u);
	pOutDat[2] = MbsSend->num_8u * 2;
	MbsSend->pileRegSendFunc(ch, pOutDat+3);
	return (3 + MbsSend->num_8u * 2);
}

static uint16_t Fun_MdbMasterSendReg(uint8_t ch, Stru_MbsInteraction *MbsSend, uint8_t *pOutDat)
{
	uint16_t t_retLenU16 = 0;
	//主机下发读写操作
	pOutDat[0] = LNLSB(MbsSend->beginRegAddr_u16);
	pOutDat[1] = LLSB(MbsSend->beginRegAddr_u16);

	switch (MbsSend->funCode) {
		case MODBUS_READ_REG:
			t_retLenU16 = Fun_MdbMasterReadReg(MbsSend, pOutDat+2);
			break;
		case MODBUS_WRITE_SINGLE_REG:
			t_retLenU16 = Fun_MdbMasterWriteSingleReg(ch, MbsSend, pOutDat+2);
			break;
		case MODBUS_WRITE_MULTIPLE_REGS:
			t_retLenU16 = Fun_MdbMasterWriteMultyReg(ch, MbsSend, pOutDat+2);
			break;
		default:
			break;
	}
	if (t_retLenU16 == 0) {
		return t_retLenU16;
	}
	return (t_retLenU16 + 2);
}







/* 写入需要交互的命令 */
void fgv_MbsInteractionCmd(uint8_t ch, uint8_t cmd)
{
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;

	if (lp_sta->Sta != 0) {
		return;
	}
	if ((cmd >= E_ReadCmd_PileInfo) && (cmd < E_PileInfoMax)) {
		lp_sta->SlaveAddr = 0;
	} else {
		lp_sta->SlaveAddr = ch + 1;
	}
	lp_sta->CommCmd = cmd;
	lp_sta->GunNum = ch;
	lp_sta->Sta = eMbsSend;
	lp_sta->SendTick = NOWTICK;	
}


static void fsv_MbsPileRecvTHread(uint8_t funCode, uint8_t *buf)
{	
	uint8_t l_PileMapLen = ARRAY_SIZE(Stru_MbsPileInteraction_Map);
	Stru_MbsInteraction *lp_PileObj = NULL;
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;

	for(int i = 0; i < l_PileMapLen; i++) {
		lp_PileObj = &Stru_MbsPileInteraction_Map[i];
		
		if (funCode != lp_PileObj->funCode) {
			continue;
		}
		if (lp_sta->CommCmd != lp_PileObj->CommCmd) {
			continue;
		}
		
		if (lp_PileObj->pileExecuteFunc != NULL) {
			lp_PileObj->pileExecuteFunc(0);
		}

		if (lp_PileObj->pileRegHandleFunc == NULL) {
			return;
		}
		//长度错误
		if (lp_PileObj->num_8u * 2 != buf[0]) {
            printf("MbsPileRcv num erro\r\n");
			return;
		}
		lp_PileObj->pileRegHandleFunc(0, buf + 1);
		
		return;
	}
}
static void fsv_MbsGunRecvTHread(uint8_t sAddr, uint8_t funCode, uint8_t *buf)
{	
	uint8_t l_GunMapLen = ARRAY_SIZE(Stru_MbsGunInteraction_Map);
	Stru_MbsInteraction *lp_GunObj = NULL;
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;

	for(int i = 0; i < l_GunMapLen; i++) {
		lp_GunObj = &Stru_MbsGunInteraction_Map[i];
		
		if (funCode != lp_GunObj->funCode) {
			continue;
		}
		if (lp_sta->CommCmd != lp_GunObj->CommCmd) {
			continue;
		}
		
        if (sAddr == 0) {
            return;
        }
        if (sAddr > GUN_NUM) {
            return;
        }

		if (lp_GunObj->pileExecuteFunc != NULL) {
			lp_GunObj->pileExecuteFunc(sAddr-1);
		}

		if (lp_GunObj->pileRegHandleFunc == NULL) {
			return;
		}
		//长度错误
		if (lp_GunObj->num_8u * 2 != buf[0]) {
            printf("MbsGunRcv num erro\r\n");
			return;
		}
		lp_GunObj->pileRegHandleFunc(sAddr-1, buf + 1);

		return;
	}
}

STRU_MBUS_COMM_CONTEXT sg_stuMbsMasterCommCtx;
static STRU_MBUS_COMM_CONTEXT *pt_MbsMasterCommCtx = &sg_stuMbsMasterCommCtx;

static void fsv_MbsRecvTHread()
{	
	ModbusFrame *frm_recv_l = NULL;
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;

	if (JudgeTimeOutMs(lp_sta->SendTick, eTick_3S)) {
		lp_sta->Sta = eMbsStanby;
		fsv_SetMbsCommSta(eAbnormal);
	}

	frm_recv_l = MbsComm_ParsePack(pt_MbsMasterCommCtx);
	if (frm_recv_l == NULL) {
		return;
	}
    
	//升级时接收处理
	fgv_otaFileRecvCheck(&frm_recv_l->dev);

	lp_sta->Sta = eMbsStanby;
	fsv_SetMbsCommSta(eNormal);
    
	if (frm_recv_l->func & 0x80) {
		//异常信息
		return;
	}
	//桩数据
	if (frm_recv_l->dev == 0) {
		fsv_MbsPileRecvTHread(frm_recv_l->func, frm_recv_l->buf);
	} else {
		fsv_MbsGunRecvTHread(frm_recv_l->dev, frm_recv_l->func, frm_recv_l->buf);
	}
}

static uint8_t fsv_MbsGetChCmd(uint8_t *ch, uint8_t *cmd)
{
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;

	uint8_t l_PileMapLen = ARRAY_SIZE(Stru_MbsPileInteraction_Map);
	uint8_t l_GunMapLen = ARRAY_SIZE(Stru_MbsGunInteraction_Map);

	//mbs交互状态
	Stru_MbsItrcInfoSta *lp_MbsItrcInfoStaObj = NULL;

	Stru_MbsInteraction *lp_Obj = NULL;
	
	
	//桩
	for(int i = 0; i < l_PileMapLen; i++) {

		lp_Obj = &Stru_MbsPileInteraction_Map[i];

		lp_MbsItrcInfoStaObj = fsvp_getItrcInfoSta(0, lp_Obj->CommCmd);

		if (lp_MbsItrcInfoStaObj == NULL) {
			continue;
		}

		if (lp_MbsItrcInfoStaObj->enable == MBS_SEND_OFF) {
			continue;
		}

		if (JudgeTimeOutMs(lp_MbsItrcInfoStaObj->sendTick, lp_Obj->TimeOutTick) == FALSE) {
			continue;
		}
		mbsMasterPintf("\r\nfsv_MbsGetChCmd Pile = %d %d\r\n", lp_Obj->CommCmd, NOWTICK);
		//发送数据
		lp_MbsItrcInfoStaObj->sendTick = NOWTICK;
		*ch = 0;
		*cmd = lp_Obj->CommCmd;
		return 1;
	}


	//枪
	for (int gun = 0; gun < GUN_NUM; gun++) {

		for(int i = 0; i < l_GunMapLen; i++) {

			lp_Obj = &Stru_MbsGunInteraction_Map[i];

			lp_MbsItrcInfoStaObj = fsvp_getItrcInfoSta(gun, lp_Obj->CommCmd);

			if (lp_MbsItrcInfoStaObj == NULL) {
				continue;
			}
			
			if (lp_MbsItrcInfoStaObj->enable == MBS_SEND_OFF) {
				continue;
			}

			if (JudgeTimeOutMs(lp_MbsItrcInfoStaObj->sendTick, lp_Obj->TimeOutTick) == FALSE) {
				continue;
			}
			mbsMasterPintf("\r\nfsv_MbsGetChCmd gun = %d  %d  %d\r\n", gun, lp_Obj->CommCmd, NOWTICK);
			//发送数据
			lp_MbsItrcInfoStaObj->sendTick = NOWTICK;
			*ch = gun;
			*cmd = lp_Obj->CommCmd;
			return 1;
		}
	}
	return 0;
}




void fsv_MbsMasterHandle()
{
	// THREAD_CYCLE(10);
  	uint16_t  length = 0;
	ModbusFrame *frm_send_l = (ModbusFrame *)pt_MbsMasterCommCtx->m_aucSendBuf;
	Stru_MbsMasterSta *lp_sta = &ls_MbsMasterSta;
	Stru_MbsInteraction *lp_Obj = NULL;
	uint8_t l_PileMapLen = ARRAY_SIZE(Stru_MbsPileInteraction_Map);
	uint8_t l_GunMapLen = ARRAY_SIZE(Stru_MbsGunInteraction_Map);
	int i = 0;

	if (lp_sta->Sta == eMbsWaitRecv) {
		return;
	}
	
	uint8_t ch, cmd;
	//寻找需要执行的命令
	uint8_t l_rslt = fsv_MbsGetChCmd(&ch, &cmd);
	if (l_rslt == 0) {
		return;
	}
	//命令处理下
	fgv_MbsInteractionCmd(ch, cmd);

	if (lp_sta->CommCmd >= E_InteractionMaxCmd) {
		lp_sta->Sta = eMbsStanby;
		return;
	}

	if (lp_sta->SlaveAddr == 0) {
		for(i = 0; i < l_PileMapLen; i++) {
			lp_Obj = &Stru_MbsPileInteraction_Map[i];
			if (lp_sta->CommCmd == lp_Obj->CommCmd) {
				break;
			}
		}
	} else {
		for(i = 0; i < l_GunMapLen; i++) {
			lp_Obj = &Stru_MbsGunInteraction_Map[i];
			if (lp_sta->CommCmd == lp_Obj->CommCmd) {
				break;
			}
		}
	}
	
	frm_send_l->dev = lp_sta->SlaveAddr;
	frm_send_l->func = lp_Obj->funCode;
	length = Fun_MdbMasterSendReg(ch, lp_Obj, frm_send_l->buf);
	if (length == 0) {
		return;
	}
	length += 2;	//功能码和设备地址
    
	lp_sta->Sta = eMbsWaitRecv;

	MbsComm_SendPack (pt_MbsMasterCommCtx, (uint8_t *)frm_send_l, length);
}

//初始化是否开启读取底板信息
static void MbsPileInteractionInit()
{
	uint8_t l_PileMapLen = ARRAY_SIZE(Stru_MbsPileInteraction_Map);
	uint8_t l_GunMapLen = ARRAY_SIZE(Stru_MbsGunInteraction_Map);

	Stru_MbsInteraction *lp_PileObj = NULL;

	for(int i = 0; i < l_PileMapLen; i++) {
		lp_PileObj = &Stru_MbsPileInteraction_Map[i];

		if (lp_PileObj->excuteType) {
			fsv_cmdIfEnable(0, lp_PileObj->CommCmd, MBS_SEND_ON);
		}
	}
	
	for (int gun = 0; gun < GUN_NUM_MAX; gun++) {
		for(int i = 0; i < l_GunMapLen; i++) {
			lp_PileObj = &Stru_MbsGunInteraction_Map[i];

			if (lp_PileObj->excuteType) {
				fsv_cmdIfEnable(gun, lp_PileObj->CommCmd, MBS_SEND_ON);
			}
		}
	}
}

static void MbsIfChargingEnable()
{
	uint8_t chargeSta = 0;
	static uint8_t pre_chargeSta[GUN_NUM_MAX] = {0};

	// LogPrintf(LVL_LOG_NONE, "fsv_cmdIfEnable324234234\r\n");
	for (int ch = 0; ch < GUN_NUM; ch++) {

		chargeSta = logic_get_gun_Stanby(ch);

		if (pre_chargeSta[ch] == chargeSta) {
			continue;
		}
		pre_chargeSta[ch] = chargeSta;

		if (chargeSta == 0) {
			fsv_cmdIfEnable(ch, E_ReadCmd_GunChrgingInfo, MBS_SEND_ON);
		} else {
			fsv_cmdIfEnable(ch, E_ReadCmd_GunChrgingInfo, MBS_SEND_OFF);
		}
	}
}

void MbsMasterMod_Init(void)
{
	/* 实时更新数据*/
	// fgv_Modbus_ModDatInit();

    memset(&sg_stuMbsMasterCommCtx, 0, sizeof(STRU_MBUS_COMM_CONTEXT));
    if(pt_MbsMasterCommCtx)
    {
		MbsCommModuleInit(pt_MbsMasterCommCtx, Usart_MbsMaster);
    }
	MbsPileInteractionInit();
}

void MbsMasteSendData(uint8_t *MbsFileData, uint16_t len)
{
	MbsComm_SendPack (pt_MbsMasterCommCtx, MbsFileData, len);
}

void MbsMasterModSendTaskMain(void)
{
	while (1) {
		osDelay(10);
		//A板正在传输升级数据
		if (fgv_getUpdataTrans(eUpdateObj_A)) {
			continue;
		}
		// MbsIfChargingEnable();
		fsv_MbsMasterHandle();
	}
}
void MbsMasterModRecvTaskMain(void)
{
	while (1) {
		osDelay(5);
        Key_Main();
		fsv_MbsRecvTHread();
		//A板正在传输升级数据
		if (fgv_getUpdataTrans(eUpdateObj_A)) {
			continue;
		}
	}
} 

