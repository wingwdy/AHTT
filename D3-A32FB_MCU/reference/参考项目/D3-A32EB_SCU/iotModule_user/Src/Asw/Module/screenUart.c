/***********************************************************************************
 * 文 件 名  : screenUart.c
 * 版 本 号  : V1.0
 * 负 责 人  : maxy
 * 创建日期  : 2024-7-1
 * 文件描述  : 
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  : 
 * 其    他  : 
 * 修改日志  : 初版
***********************************************************************************/
#include <stdlib.h>
#include "globals.h"
#include "screenUart.h"
#include "ProtoLayerHeaderSummary.h"
#include "AppMidDataTrans.h"
#include "card_user.h"
#include "cmsis_os2.h"
#include "iot_GN_Protocol_Code.h"
#include "AppInputCfg.h"
#include "DisQRCode.h"

static uint8_t g_ScrExsit = 0;    //屏幕是否存在

static void UIRebootInit();

#define QRCODE_GN_STRING    "https://evse.gongniu.cn/car.html?qRCode="

// #define scrnPintf(fmt,args...)	\
// 		do {								\
//             debug(fmt ,##args); 	\
// 		} while(0)
#define scrnPintf(fmt,args...)


Screen_data_ctrl gScr_data;

static uint8_t g_sendDataBuf[256];

Screen_data_ctrl *GetScrMainStruct()
{
    return &gScr_data;
}

uint8_t ScrPileInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 1： 单枪模式 2： 双枪模式
    Data[1]： 1： 中国移动 2： 中国联通 3： 中国电信 4： LAN
    Data[2]： 1： 国标 2： 企标
    Data[3]： 1： 场内模式 2： 场外模式
    Data[4]~Data[14]： 软件版本号(ascii 码， 格式 xx.xx.xx.xx)
    Data[15]~Data[25]： 硬件版本号(ascii 码， 格式 xx.xx.xx.xx)
    Data[26]~Data[36]： HMI 软件版本号(ascii 码， 格式 xx.xx.xx.xx)
    Data[37]~Data[47]： 4G 软件版本号(ascii 码， 格式 xx.xx.xx.xx)
    Data[48]~Data[67]： ICCID(ascii 码)
    Data[68]~Data[83]： Server 地址(ascii 码)
    Data[84]~Data[85]： 端口
    Data[86]~Data[93]： 平台类型(ascii 码)
    Data[94]~Data[101]： 卡类型(ascii 码)
    Data[102]~Data[117]： 桩资产码(ascii 码), 16 个字节， (debug 界面)
    */
    Screen_data_ctrl *pScrData = &gScr_data;

    buf[0] = GUN_NUM;

    buf[1] = pScrData->pileInfo.operator;
    buf[2] = pScrData->pileInfo.standard;
    buf[3] = pScrData->pileInfo.facMode;
    
    GetPile_StrSoftVer((char *)&buf[4]);
    memcpy(&buf[15], HARDWARE_VERSION, 11);
    memcpy(&buf[26], SOFTWARE_VERSION, 11);

    GetNet_Comm_SimID(&buf[48], 20);

    char ser[PLAT_DNS_LEN] = {0};
    uint16_t port = 0;
    Get_PlatIServer(ser, &port);
    uint8_t l_strlenDns = strlen(ser);
    uint8_t l_displayLen = (l_strlenDns > 18) ? 10 : (l_strlenDns / 2);
    sprintf((char *)&buf[68], "***%s", &ser[l_strlenDns - l_displayLen]);

    Uint16ToTwoUint8LH(&buf[84], port);

    Get_CurrentPlatTypeName((char *)&buf[86]);
    Get_CurrentCardTypeName((char *)&buf[94]);
    
    Get_DevNumberString((char *)&buf[102]);

    return 118;
}
uint8_t ScrPileRtInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 信号强度（0~4）
    Data[1]：信号强度dB值(-128~127), 1表示不可测, 不需要显示
    Data[2]：联网步骤（0~99），变位上送
    */
    buf[0] = GetNet_SignalLevel();

    buf[1] = GetNet_SignaldBm();
    
    buf[2] = Comm_getNetStep(eSocket_GPRS1);

    return 3;
}
uint8_t ScrGunRtInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 1： A 枪头； 2： B 枪头
    Data[1]： 0： 未插枪 1： 已插枪
    Data[2]： 0： 待机 1： 等待启动 2： 启动中 3： 充电中 4： 暂停充电 5： 停止中 6： 充电完成
            10故障 11：软件升级 12：调试
            注： “软件升级” 时， 可以忽略 Data[0-1]
    Data[3] 预留
    Data[4-7] 0： 无故障
    Bit 位表示（0 否 1 是） ， 低位到高位顺序
    Bit0： 漏电故障
    Bit1： 计量异常
    Bit2： 急停按钮故障
    Bit3： 桩过温故障
    Bit4： 枪过温故障
    Bit5-Bit6： 1-继电器黏连， 2-继电器输出异常；
    Bit7： cp 异常
    Bit8： 过流
    Bit9-Bit10： 1 过压,2 欠压
    */
    Screen_data_ctrl *pScrData = &gScr_data;

    buf[0] = u8Port + 1;
    buf[1] = GetPile_gun_connect(u8Port);
    buf[2] = pScrData->gunData[u8Port].UISta;
    buf[3] = pScrData->gunData[u8Port].gunSta;

    uint32ToFourUint8LH(&buf[4], pScrData->gunData[u8Port].fault);

    return 8;
    
}
uint8_t ScrGunChargingInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 1： A 枪头； 2： B 枪头
    Data[1-3]： 充电时间， 时、 分、 秒各占 1 字节
    Data[4-7]： 充电电量， 单位 0.001kw
    Data[8-9]： 充电电压， 单位 0.01V
    Data[10-11]： 充电电流， 单位 0.01A
    Data[12-15]： 充电金额， 单位 0.01 元
    Data[16-19]： 结算前卡余额， 单位 0.01 元
    Data[20]： 充电温度， 单位 ℃， 范围【-128~128】
    Data[21-24]： 开始 UTC 时间戳， 1717058806 对应北京时间 2024-05-30 16:46:46
    Data[25-28]： 结束 UTC 时间戳， 1717058806 对应北京时间 2024-05-30 16:46:46
    */

    buf[0] = u8Port + 1;

    #define SCR_MAXTIME (100 * 60 * 60 - 1) //屏幕上最大显示原因

    uint32_t l_timer = monitor_getChgTimer(u8Port);
    if (l_timer > SCR_MAXTIME) {
        l_timer = SCR_MAXTIME;
    }
    buf[1] = l_timer / 3600;
    buf[2] = l_timer % 3600 / 60;
    buf[3] = l_timer % 60;

    uint32_t scrMax = (monitor_getChgTotalEnergy(u8Port) + 5) / 10;
    if (scrMax > 999999) {
        scrMax = 999999;
    }
    uint32ToFourUint8LH(&buf[4], scrMax);
    
    Uint16ToTwoUint8LH(&buf[8], GetPile_ChgOutVol(u8Port, 2));

    Uint16ToTwoUint8LH(&buf[10], GetPile_ChgOutCur(u8Port, 2));
    
    scrMax = GetPile_ChgTotalMoneyDisplay(u8Port);
    if (scrMax > 99999) {
        scrMax = 99999;
    }
    uint32ToFourUint8LH(&buf[12], scrMax);

    scrMax = GetPile_AccountBalance(u8Port);
    if (scrMax > 9999999) {
        scrMax = 9999999;
    }
    uint32ToFourUint8LH(&buf[16], scrMax);

    buf[20] = GetPile_GunTem(u8Port);
    
    uint32_t l_startStamp = 0;
    uint32_t l_stopStamp = 0;
    GetPile_ChargeBgEndTime(u8Port, &l_startStamp, &l_stopStamp);
    uint32ToFourUint8LH(&buf[21], l_startStamp);
    uint32ToFourUint8LH(&buf[25], l_stopStamp);

    return 29;
}
uint8_t ScrQrcodeInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 1： A 枪头； 2： B 枪头
    Data[1]~Data[32]： 桩连接平台码(ascii 码), 32 个字节， (主界面)
    Data[33]~Data[N]： 二维码(ascii 码)
    */
    buf[0] = u8Port + 1;

    char devStr[PLAT_NUMBER_LEN + 1] = {0};
    char gunStr[3] = {0};
    char qrStr[200] = {0};

    Get_PlatNumberString(devStr);
    
    DisQRcodeReplace(qrStr, &g_QrCodeInfo.qrcodeInfo[u8Port][0], devStr, buf[0]);

    strcat((char *)&buf[33], qrStr);

    return 233;
}

uint8_t ScrCardInfoData(uint8_t u8Port, uint8_t *buf)
{
	// IC_T *pPlat_ic = Get_IC_Msg(u8Port);
    /* 
    Data[0]： 1： A 枪头； 2： B 枪头
    Data[1]~Data[N]： 卡号(ascii 码)
    */
    buf[0] = u8Port + 1;
    
    uint8_t card_number[GNDATA_CARD_LEN] = {0};

	GetPile_ChgCarNumber(u8Port, card_number);

	BINToAscii((char *)&buf[1], (char *)card_number, GNDATA_CARD_LEN);
	// BINToAscii((char *)&buf[1], (char *)&pPlat_ic->LogicNumbers[8], 16);

    return 17;
}

uint8_t ScrGunDebugRtInfoData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0]： 1： A 枪头； 2： B 枪头
    Data[1-2]： Cp Value, 单位 0.1V
    Data[3-4]： Pwm Duty, 单位 0.1%
    Data[5]： Pwm Out, 0 表示不输出， 1 表示输出
    Data[7]： 继电器输出 0 表示不输出， 1 表示输出
    Data[8-9]： 充电电压， 单位 0.1V
    Data[10-11]： 充电电流， 单位 0.1A
    Data[12]： 充电温度， 单位 ℃， 范围【-128~128】
    */
   
    buf[0] = u8Port + 1;

    Uint16ToTwoUint8LH(&buf[1], GetPile_GunRtCpValue(u8Port) / 10);
    Uint16ToTwoUint8LH(&buf[3], GetPile_GunChargingPwm(u8Port));
    buf[5] = GetPile_GunPwnEn(u8Port);
    buf[6] = GetPile_GunRelayOut(u8Port);
    
    Uint16ToTwoUint8LH(&buf[7], (GetPile_ChgOutVol(u8Port, 1)));

    Uint16ToTwoUint8LH(&buf[9], (GetPile_ChgOutCur(u8Port, 1)));

    buf[11] = GetPile_GunTem(u8Port);

    return 12;
}

uint8_t ScrPileCalTimeData(uint8_t u8Port, uint8_t *buf)
{
    /* 
    Data[0-3]： UTC 时间戳， 1717058806 对应北京时间 2024-05-30 16:46:46
    */
    uint32ToFourUint8LH(&buf[0], getRunTimeS());

    scrnPintf("set scr time:%d\r\n", getRunTimeS());

    return 4;
}


static SCR_SEND_CTRL* GetScrSendCtrl(uint8_t u8Port, E_SCRCMD_LIST cmd)
{
    if (cmd >= SCRCMD_MAX) {
        return NULL;
    } else {
        return &gScr_data.scrSendCtrl[u8Port][cmd];
    }
}

void scrn_SetUploadFlag(uint8_t u8Port, E_SCRCMD_LIST cmd, uint8_t start)
{
    SCR_SEND_CTRL *pSendCtrl = NULL;
    
    pSendCtrl = GetScrSendCtrl(u8Port, cmd);
    if(NULL == pSendCtrl)
        return;

    pSendCtrl->u8SendEnable = start;
    // pSendCtrl->CycTimer =  NOWTICK;
    if (start) {
        pSendCtrl->u8SendCnt = 0;
    }
}

void scrn_SendImmediately(uint8_t u8Port, E_SCRCMD_LIST cmd)
{
    SCR_SEND_CTRL *pSendCtrl = NULL;
    
    // scrnPintf("scrn_SendImmediately %d: %d  %d\r\n", NOWTICK, u8Port, cmd);

    pSendCtrl = GetScrSendCtrl(u8Port, cmd);
    if(NULL == pSendCtrl)
        return;

    pSendCtrl->u8SendEnable = 1;
    //发送计时
    pSendCtrl->CycTimer =  0;
}

//打包可以直接发送的数据
uint8_t ScrPackSendBuf(uint8_t *sendBuf, uint8_t cmd, uint8_t *Buf, uint8_t len)
{
    sendBuf[0] = 0x55;
    sendBuf[1] = 0xEE;
    sendBuf[2] = len;
    sendBuf[3] = cmd;
    memcpy(&sendBuf[4], Buf, len);
    //和校验
    uint16_t check = 0;
    for (int i = 0; i < (len+4); i++) {
    check += sendBuf[i];
    }

    sendBuf[len+4] = (check >> 8);
    sendBuf[len+5] = check;

    return len + 6;
}



typedef uint8_t (*PSendCondition)(uint8_t u8Port, uint8_t *buf);
typedef uint8_t (*PSendScreenData)(uint8_t u8Port, uint8_t *buf);


typedef struct
{
    E_SCRCMD_LIST		  cmd;
    uint32_t     	interval;   //发送间隔
    uint8_t     	rspFlag;    //0表示持续发送，其他表示无应答发送次数上限，应答完成停止发送
    PSendCondition 			pSendCondition;
    // PSendScreenData 		pRecvSucc;
} Screen_Data_Send;


const Screen_Data_Send Screen_SendData_Map[] = {
    {SCRCMD_PileInfo		, eTick_3S      ,	3,  ScrPileInfoData          },
    {SCRCMD_PileRtInfo 		, eTick_1S	    ,	0,  ScrPileRtInfoData        },
    {SCRCMD_PileCalTime 	, eTick_10S	    ,	0,  ScrPileCalTimeData    },

    {SCRCMD_GunRtSta 		, eTick_3S	    ,	1,  ScrGunRtInfoData         },
    {SCRCMD_GunCharging 	, eTick_300ms	,	1,  ScrGunChargingInfoData   },
    {SCRCMD_GunQrCode 		, eTick_3S	    ,	3,  ScrQrcodeInfoData        },
    {SCRCMD_GunCard 		, eTick_3S	    ,	3,  ScrCardInfoData          },
    {SCRCMD_GunDebug 		, eTick_500ms	,	0,  ScrGunDebugRtInfoData    },
};

void Screen_SendData_Map_Handle()
{
    const Screen_Data_Send *pSendData = NULL;


    for (int u32i = 0; u32i < ARRAY_SIZE(Screen_SendData_Map); u32i++)
    {
        pSendData = &Screen_SendData_Map[u32i];

        for (int gun = 0; gun < GUN_NUM; gun++) {
              
            SCR_SEND_CTRL *pSendCtrl = GetScrSendCtrl(gun, pSendData->cmd);

            if (pSendCtrl->u8SendEnable == 0) {
                continue;
            }
            if (!JudgeTimeOutMs(pSendCtrl->CycTimer, pSendData->interval)) {
                continue;
            }
            //发送计时
            pSendCtrl->CycTimer =  NOWTICK;

            if (pSendData->pSendCondition != NULL) {

                UartDCB *pUartDCB = GetUartDCB(UART_HMI);

                memset(pUartDCB->pTxBuffer, 0, pUartDCB->u16TxBufSize);

                uint8_t bufLen = pSendData->pSendCondition(gun, pUartDCB->pTxBuffer+4);

                uint8_t sendLen = ScrPackSendBuf(pUartDCB->pTxBuffer, pSendData->cmd, pUartDCB->pTxBuffer+4, bufLen);
                
                if (sendLen > 255) {
                    continue;
                }
                if (pSendData->cmd != SCRCMD_PileRtInfo) {
                    scrnPintf ("send  tick:%d: cmd:%d, gun:%d ", NOWTICK, pSendData->cmd, gun);
                    for (int i = 0; i < sendLen; i++) {
                        scrnPintf("%02x ", pUartDCB->pTxBuffer[i]);
                    }
                    scrnPintf ("\r\n");
                }
                osDelay(5);     //需要延时进行传输，否则太快容易粘包

                UartSendData(UART_HMI, sendLen);
                

                if (pSendData->rspFlag) {
                    pSendCtrl->u8SendCnt++;
                    if (pSendCtrl->u8SendCnt >= pSendData->rspFlag) {
                        scrn_SetUploadFlag(gun, pSendData->cmd, 0);
                    }
                }

                continue;
                // return;
            }
        }
    }
    
}


void RecvScreenVaildCmdHandle(uint8_t *recvBuf, uint8_t len)
{   
    scrnPintf ("recv: %d ", NOWTICK);
    for (int i = 0; i < len; i++) {
        scrnPintf ("%02x ", recvBuf[i]);
    }
    scrnPintf ("\r\n");

    //接收到数据，数据验证合法后，进行处理 
    //和校验
    uint16_t check = 0;
    for (int i = 0; i < (len - 2); i++) {
      check += recvBuf[i];
    }
    if (check != (recvBuf[len - 2] << 8 | recvBuf[len - 1])) {
      printf("HMI recv addCheck erro\r\n");
      return;
    }

    g_ScrExsit = 1; //屏幕启动会进行主动上送数据，这时候可以判断屏幕存在

    uint8_t hmiCmd = recvBuf[3];

    if (hmiCmd == SCRCMD_REBOOT) {
        printf ("scrn reboot\r\n");
        UIRebootInit();
        return;
    }
    
    const Screen_Data_Send *pSendData = NULL;

    for (int u32i = 0; u32i < ARRAY_SIZE(Screen_SendData_Map); u32i++)
    {
        pSendData = &Screen_SendData_Map[u32i];
        if (pSendData->rspFlag == 0) {
            continue;
        }
        if (hmiCmd != pSendData->cmd) {
            continue;
        }

        uint8_t u8Port = 0;
        if ((hmiCmd == SCRCMD_GunRtSta)
        || (hmiCmd == SCRCMD_GunQrCode)
        || (hmiCmd == SCRCMD_GunCard)) {
            u8Port = recvBuf[4] - 1;

            if (u8Port > GUN_NUM) {
                return;
            }
        }
        SCR_SEND_CTRL *pSendCtrl = GetScrSendCtrl(u8Port, pSendData->cmd);

        if(NULL == pSendCtrl)
            return;

        pSendCtrl->u8SendEnable = 0;

        break;
    }

}

//接收到的屏幕数据处理
void RecvScreenCmdHandle(uint8_t *recvBuf, uint8_t len)
{
    //粘包处理
    int16_t h_len = len;
    uint8_t each_len = 0;
    uint8_t crt_len = 0;
    while (h_len)
    {
        if ((recvBuf[crt_len] != 0x55) || (recvBuf[crt_len+1] != 0xEE)) {
            printf("HMI recv head erro len = %d\r\n", len);
            return;
        }
        each_len = recvBuf[crt_len+2] + 6;

        //有效数据处理
        RecvScreenVaildCmdHandle(&recvBuf[crt_len], each_len);

        h_len -= each_len;
        if (h_len < 6) {
            return;
        }

        crt_len += each_len;
    }

}

//充电桩数据刷新并更新屏幕
void ScrDataUpdate_PileCheck()
{
    Screen_data_ctrl *pScrData = &gScr_data;
    Screen_PileData *pScrPileData = &pScrData->pileInfo; //桩信息，变为上送

    pScrPileData->gunMode = GetPile_CfgGunNum();
    pScrPileData->operator = GetNet_Comm_Operator();
    if ((pScrPileData->operator == 0) || (pScrPileData->operator >= 4)) {
        pScrPileData->operator = 1;
    }

    if (GetPile_CfgNationalStandard()) {
        pScrPileData->standard = 1;
    } else {
        pScrPileData->standard = 2;
    }

    if (GetPile_CfgOffLinChrg()) {
        pScrPileData->facMode = 1;
    } else {
        pScrPileData->facMode = 2;
    }

    GetNet_Comm_SimID((uint8_t *)pScrPileData->ICCID, 20);
    
    //检测桩状态变化上传给屏幕
    static Screen_PileData l_PreScrPileData;
    SCR_SEND_CTRL *pSendCtrl = NULL;
    if (memcmp(&l_PreScrPileData, pScrPileData, sizeof(Screen_PileData))) {

        scrn_SendImmediately(0, SCRCMD_PileInfo);

        memcpy(&l_PreScrPileData, pScrPileData, sizeof(Screen_PileData));
    } 
}
void debugUITimeoutStandby()
{
    //debug界面下待机超过1分钟自动退出到待机界面
    Screen_data_ctrl *pScrData = &gScr_data;
    
    uint8_t devIdle = GetPile_Idlet();
    static uint32_t debugAndIdleTime = 0;
   
    for (uint8_t i = 0; i < GUN_NUM; i++) {
        Screen_GunData *pScrGunData = &pScrData->gunData[i]; //枪信息，变为上送
        
        if (pScrGunData->UISta != UISTA_DEBUG) {
            debugAndIdleTime = NOWTICK;
            break;
        }
        if (!devIdle) {  //开始计时
            debugAndIdleTime = NOWTICK;
        }
        if (JudgeTimeOutMs(debugAndIdleTime, 30000) == TRUE) {
            pScrGunData->UISta = UISTA_STANDBY;
            printf("debugUITimeoutStandby\r\n");
        }
    }
}
void ScrUIStaUpdate()
{
    Screen_data_ctrl *pScrData = &gScr_data;

    static uint8_t preGunSta[2] = {0};
    static uint8_t needHoldFinishUI[2] = {0};
    static uint32_t needWaitTime[2] = {0};

    for (uint8_t i = 0; i < GUN_NUM; i++) {

        Screen_GunData *pScrGunData = &pScrData->gunData[i]; //枪信息，变为上送

        pScrGunData->fault = GetPile_ErrInfo(i);
        pScrGunData->gunSta = GetPile_gun_state(i);
        
        if (OtaGetUpdatingFlag()) {
            pScrGunData->UISta = UISTA_UPDATE; //升级
        } else if (pScrGunData->UISta == UISTA_DEBUG) {
            continue;
        } else if (GetPile_ErrState(i)) {
            pScrGunData->UISta = UISTA_FAULT;  //故障
        } else {
            //充电中拔枪直接等待一定时间再显示待机界面
            if (((preGunSta[i] >= eChargeState_Starting) && (preGunSta[i] < eChargeState_StopFinish) )
            && (pScrGunData->gunSta == eChargeState_Idle)) {
                needHoldFinishUI[i] = 1;
                needWaitTime[i] = NOWTICK;
            } else if (preGunSta[i] != pScrGunData->gunSta) {
                needHoldFinishUI[i] = 0;
            }

            if (needHoldFinishUI[i]) {
                pScrGunData->UISta = UISTA_CHARGFINISH;
                if (JudgeTimeOutMs(needWaitTime[i], CHRG_FINISH_HOLE_TIME)) {
                    needHoldFinishUI[i] = 0;
                }
            } else {
                pScrGunData->UISta = pScrGunData->gunSta;
            }

        }
        preGunSta[i] = pScrGunData->gunSta;
    }

}


//充电枪数据更新，检查是否产生变化需要上报，并刷新屏幕
void ScrDataUpdate_GunCheck()
{
    Screen_data_ctrl *pScrData = &gScr_data;
    static Screen_GunData l_PreGunData[GUN_NUM_MAX] = {0};
    static uint8_t l_PreGunSta[GUN_NUM_MAX] = {0};
    static uint8_t l_PreUISta[GUN_NUM_MAX] = {0};
    static uint8_t StopStateDelayCnt = 0;
    ScrUIStaUpdate();

    for (uint8_t i = 0; i < GUN_NUM; i++) {

        Screen_GunData *pScrGunData = &pScrData->gunData[i]; //枪信息，变为上送
        Screen_GunData *pPreScrGunData = &l_PreGunData[i];  //枪信息，变为上送
        
        //实时数据屏显
        if ((pScrGunData->gunSta >= eChargeState_Starting)
            && (pScrGunData->gunSta <= eChargeState_Stoping)) {
                scrn_SetUploadFlag(i, SCRCMD_GunCharging, 1);
        }
        //充电完成状态，屏幕的充电金额(电费+服务费)需等待1秒钟以上，确保数据已刷新
        if (pScrGunData->gunSta == eChargeState_StopFinish) {
            if (pPreScrGunData->gunSta != eChargeState_StopFinish)
            {
                StopStateDelayCnt = 5;
                scrn_SetUploadFlag(i, SCRCMD_GunCharging, 1);
            }
            if(StopStateDelayCnt)
            {
                StopStateDelayCnt --;
                scrn_SetUploadFlag(i, SCRCMD_GunCharging, 1);
            }
        }
        if (memcmp(pPreScrGunData, pScrGunData, sizeof(Screen_GunData))) {
            scrn_SendImmediately(i, SCRCMD_GunRtSta);
        } else {
            continue;
        }
        
        //检测枪状态变化上传给屏幕，故障恢复显示
        if (pPreScrGunData->gunSta != pScrGunData->gunSta) {
            if (pScrGunData->gunSta == eChargeState_StopFinish) {
                scrn_SetUploadFlag(i, SCRCMD_GunCharging, 1);
            }
        }
        if (pPreScrGunData->UISta != pScrGunData->UISta) {
            if (pScrGunData->UISta == UISTA_DEBUG) {
                scrn_SetUploadFlag(i, SCRCMD_GunDebug, 1);
            } else {
                scrn_SetUploadFlag(i, SCRCMD_GunDebug, 0);
            }
        }
        memcpy(pPreScrGunData, pScrGunData, sizeof(Screen_GunData));
    }
}

//刷卡是否变化，如有变化传输屏幕显示
void ScrDataUpdate_CardCheck()
{
    static uint8_t preIc[2] ={0};
    for (uint8_t i = 0; i < GUN_NUM; i++) {
        //充电后不传输卡号
        if (GetPile_gun_state(i) >= eChargeState_Starting) {
            return;
        }
        //卡号
	    IC_T *pPlat_ic = Get_IC_Msg(i);
        if (preIc[i] == 0 && pPlat_ic->opt_sts.ValidCard_ok_2b == 1) {
            scrn_SendImmediately(i, SCRCMD_GunCard);
            preIc[i] = 1;
        } else if (pPlat_ic->opt_sts.ValidCard_ok_2b == 0) {
            preIc[i] = 0;
        }
    }
}

void ScrDataUpdate_CardOrder()
{
    static uint8_t s_preNeedTrans[2] ={0};
    uint8_t s_needTrans[2] ={0};
    for (uint8_t i = 0; i < GUN_NUM; i++) {
        if (Get_OrderTradeFlag(i) == 2) {
            s_needTrans[i] = 1;
        } else {
            s_needTrans[i] = 0;
        }
        if (s_preNeedTrans[i] == s_needTrans[i]) {
            continue;
        }
        if (s_needTrans[i]) {
            scrn_SendImmediately(i, SCRCMD_GunCard);
        }
        s_preNeedTrans[i] = s_needTrans[i];
    }
}

//联网步骤是否变化，如有变化传输屏幕显示
void ScrDataUpdate_NetCheck()
{
    //500ms检测一次
    static uint32_t l_detectTime = 0;
    if (JudgeTimeOutMs(l_detectTime, eTick_200ms) == FALSE) {
        return;
    }
    l_detectTime = NOWTICK;

    //联网步骤变化立即上传
    uint8_t netStep = Comm_getNetStep(eSocket_GPRS1);
    static uint8_t preNetStep = 0;
    if (netStep != preNetStep) {
        scrn_SendImmediately(0, SCRCMD_PileRtInfo);
        preNetStep = netStep;
    }
}
//桩码发生变化，实时更新
void PlatDevNumberChange(uint8_t list)
{
    if (list == 1) {
        scrn_SendImmediately(0, SCRCMD_PileInfo);
    } else if (list == 2) {
        for (uint8_t i = 0; i < GUN_NUM_MAX; i++) {
        scrn_SetUploadFlag(i, SCRCMD_GunQrCode, 1);
        }
    }
}
//开机任务初始化
void UIRebootInit()
{
    scrn_SendImmediately(0, SCRCMD_PileInfo);
    scrn_SendImmediately(0, SCRCMD_PileRtInfo);
    scrn_SendImmediately(0, SCRCMD_PileCalTime);
    for (uint8_t i = 0; i < GUN_NUM_MAX; i++) {
        scrn_SendImmediately(i, SCRCMD_GunRtSta);
        scrn_SendImmediately(i, SCRCMD_GunQrCode);
    }
}

void ScrnMainTask()
{	
    UIRebootInit();

    while (1) {
        if ((g_ScrExsit == 0) && (NOWTICK > eTick_10S)) {
            osDelay(eTick_1S);
            continue;
        }
        //数据更新，并检查是否需要上传屏幕显示
        ScrDataUpdate_PileCheck();
        ScrDataUpdate_GunCheck();
        // ScrDataUpdate_CardCheck();
        ScrDataUpdate_CardOrder();
        ScrDataUpdate_NetCheck();

        debugUITimeoutStandby();    //debug界面待机时1分钟自动退出到待机界面

        //数据传输处理
        Screen_SendData_Map_Handle();

        osDelay(eTick_20ms);
    }

}