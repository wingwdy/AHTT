#ifndef __APP_OTA_H_
#define __APP_OTA_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
//#include <ctype.h>

#include "bsp_flash.h"
#include "bsp_w25qxx.h"

/**/
#ifndef NULL
#define NULL	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef VALID
#define VALID	0x55
#endif

#ifndef INVALID
#define INVALID	0xAA
#endif

#ifndef U32
#define U32 uint32_t
#endif

#ifndef U16
#define U16 uint16_t
#endif

#ifndef U8
#define U8  uint8_t
#endif

#ifndef S32
#define S32 int32_t
#endif

#ifndef S16
#define S16 int16_t
#endif

#ifndef S8
#define S8 int8_t
#endif

#define SET_VALID_FLAG(flag, validflag)		(((U32)flag & 0x00ffffff) | (((U32)(validflag & 0xff)) << 24))
#define GET_VALID_FLAG(flag)				(flag >> 24)
#define SET_CHECK_SUM(flag, checksum)		((((U32)flag) & 0xffffff00) | checksum)
#define SET_LEN(flag, len)					((((U32)flag) & 0xff0000ff) | ((len)<<8))
#define GET_CHECK_SUM(flag)					(flag&0xff)
#define GETET_LEN(flag)						((flag&0xffffff)>>8)

#define FPOS(type, field) 					((uint32_t) &(( type *) 0)-> field )

#ifdef GD32E50X_HD
#define FLASH_PAGE_SIZE 	0x0800
#else
#define FLASH_PAGE_SIZE 	0x0400
#endif


/*参数结构体长度固定，防止升级后参数改变*/
#define STO_UPDATA_PARA_LEN	  		256	  	//

/*ota*/
#define APP_MAX_FRAME_CNT       	448	        //程序最大空间，单位K
#define OTA_FRAME_SIZE        		1024		//下载一帧长度1024
#define APP_MAX_SIZE        		(OTA_FRAME_SIZE*APP_MAX_FRAME_CNT)	//
#define OTA_DOWNLOAD_ADRR 			((uint32_t)(1 * 448 * 1024))//升级区
#define OTA_BACKUP_ADRR 			((uint32_t)0)//备份区

//运行区地址
#define APP_FLASH_BASE				0x08000000
#define APP_FLASH_OFFSET 			0x4000
#define APP_START_ADDRESS           (APP_FLASH_BASE + APP_FLASH_OFFSET)

//参数等数据存储于前16个扇区，升级参数存储于参数存储的最后一个扇区，其他在一开始的位置，整个数据参数存储占用64k
#define	CFG_RES_BODY_SIZE					123	//预留配置体大小

enum
{
    eCheck_Null,						//
    eCheck_Error,						//
    eCheck_Right,						//
};

//升级状态 boot中会使用 勿轻易更改
//升级流程
typedef enum
{
    eOTA_Boot_Idle,						//
    eOTA_Boot_FileCheck,				//下载完成
    eOTA_Boot_Backup,					//备份完成-
    eOTA_Boot_Copy,						//复制完成
    eOTA_Boot_RollBack,					//回滚
    eOTA_Boot_Jump,						//跳转
}BOOT_OTA_E;

typedef struct
{
	uint32_t	CsIndex;
	uint32_t	BackupIndex;
	uint32_t	CopyIndex;
	uint32_t	RollbackIndex;
    uint32_t	u32Cs;      		// 结算累加和
//	uint32_t	tatolCopyCnt;
	uint8_t		FileCheckResult;
	
	//升级文件信息,过程中不可变
    uint32_t	binFileLen;      	// bin文件大小
    uint32_t	u32FileCs;       	// 程序文件累加和校验
    uint32_t	totalPackCnt;    	// 总包数 一包长度OTA_CUT_PACK_SIZE
    uint32_t	tatolFrameCnt;	 	// 总帧数 一帧长度OTA_FRAME_SIZE
    uint32_t	finalFrameLen;	 	// 最后一帧长度
	
}OTA_CTX_T;

typedef struct
{
    uint8_t       flagUp1[4];         // 升级论证标志1 0x12345678      
    char          rtuType[16];        // 终端类型
    uint8_t       softVer[4];         // 软件版本号
    uint8_t       softDate[4];        // 软件日期
    uint8_t       binFileLen[4];      // bin文件大小 程序大小不包括打包头
    uint8_t       flagUp2[4];         // 升级论证标志2 0x87654321
    uint8_t       cs32[4];            // 程序文件累加和校验
    uint8_t       crc_16[2];          // 从flagUp1到flagUp2的crc16校验
    uint8_t       rvsd[86];           // 预留
}OTA_HEAD_T;

#define OTA_HEAD_LEN      sizeof(OTA_HEAD_T)



typedef struct _UPDATA_PARA{
    uint8_t UpFlag;
    uint8_t u8release[123];
}UPDATA_PARA;

typedef struct _F_UPDATA_PARA{
    uint8_t u8CtrlWord[4];
	UPDATA_PARA strUpdataPara;
	
}F_UPDATA_PARA;


//!!!!!所有数据都使用u8定义内存对齐,版本迭代要保证参数不变
typedef struct __SPI_DATA_MAP__
{
	/*前4096参数不能丢不能乱*/
	uint8_t					u8CtrlWord[4];
	uint8_t					u8ValidFlag;						//
	uint8_t					u8Res1[251];						//预留
	//256
	F_UPDATA_PARA			strFupDataPara;						//
    
}STO_DATA_MAP;

#define STODM_ctrlWord			FPOS(STO_DATA_MAP, u8CtrlWord)		
#define STODM_UPDATA_PARA		FPOS(STO_DATA_MAP, strFupDataPara)

#define STO_USER_BACKUP_DATA_ADDR ((uint32_t)OTA_BACKUP_ADRR)	//升级备份区
#define STO_USER_UPDATA_DATA_ADDR ((uint32_t)OTA_DOWNLOAD_ADRR)	//升级区
//=============================================

//body
typedef struct
{
	BOOT_OTA_E	eBootOtaState;          //升级状态
    uint32_t verBL;                     //bootloader软件版本        
    uint8_t au8_Res[CFG_RES_BODY_SIZE]; //
} stu_Cfg_t;

extern stu_Cfg_t g_pstuCfg;


/*****************************************************************************
*
*****************************************************************************/

//==============================================================
//ota初始化
uint8_t ota_Init(void);

void ota_task(void);

uint8_t flash_check(uint32_t ucVer);
void sflv_BootUpdateJumpToAppManage(void);

//======================================================

#endif

