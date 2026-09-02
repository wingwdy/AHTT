#ifndef __SCREENUART_H
#define __SCREENUART_H

#include <stdint.h>
#include <string.h>
#include "AppHeaderSummary.h"


#define CHRG_FINISH_HOLE_TIME   3000        //充电过程中强制拔枪，充电完成界面需要保持3s

typedef enum {
  SCRDEVSTA_Normal = 0x0,
  SCRDEVSTA_Fault,
  SCRDEVSTA_Update,
  SCRDEVSTA_Debug = 0x09,
  SCRDEVSTA_MAX,
} E_SCRDEVSTA_LIST;

typedef enum {
  SCRCMD_PileInfo = 0x01,
  SCRCMD_PileRtInfo,
  SCRCMD_GunRtSta,
  SCRCMD_GunCharging,
  SCRCMD_GunQrCode = 0x06,
  SCRCMD_GunCard,
  SCRCMD_GunDebug,
  SCRCMD_REBOOT,
  SCRCMD_PileCalTime,
  SCRCMD_MAX,
} E_SCRCMD_LIST;


typedef enum {
  UISTA_STANDBY = 0x0,
  UISTA_WAITSTART,
  UISTA_STARTING,
  UISTA_CHARGING,
  UISTA_PAUSE,
  UISTA_STOPPING,
  UISTA_CHARGFINISH,
  UISTA_FAULT = 10,
  UISTA_UPDATE,
  UISTA_DEBUG,
  UISTA_MAX,
} E_UISTA_LIST;

//收发控制
typedef struct __SCR_SEND_CTRL__
{
	int32_t CycTimer;					  //周期计时器
	uint8_t u8SendEnable;				//发送使能
	uint8_t u8SendCnt;					//发送标记
}SCR_SEND_CTRL;
 

typedef struct
{
    uint8_t gunMode;    //1单枪，2双枪
    uint8_t operator;   //1： 中国移动 2： 中国联通 3： 中国电信 4： LAN
    uint8_t standard;   //1： 国标 2： 企标
    uint8_t facMode;    //1： 场内模式 2： 场外模式
    char ICCID[20];    //ICCID
} Screen_PileData;

typedef struct
{
    uint8_t gunSta;   //0： 待机 1： 等待启动 2： 启动中 3： 充电中 4： 暂停充电 5： 停止中 6： 充电完成

    uint8_t UISta;   //0： 待机 1： 等待启动 2： 启动中 3： 充电中 4： 暂停充电 5： 停止中 6： 充电完成，10故障，11软件升级，12调试中

    uint32_t fault;
} Screen_GunData;

typedef struct
{
    Screen_PileData pileInfo;
    Screen_GunData gunData[GUN_NUM_MAX];

    SCR_SEND_CTRL scrSendCtrl[GUN_NUM_MAX][SCRCMD_MAX];
} Screen_data_ctrl;



Screen_data_ctrl *GetScrMainStruct(void);

void RecvScreenCmdHandle(uint8_t *recvBuf, uint8_t len);
void ScrnMainTask(void);


void PlatDevNumberChange(uint8_t list);

#endif

