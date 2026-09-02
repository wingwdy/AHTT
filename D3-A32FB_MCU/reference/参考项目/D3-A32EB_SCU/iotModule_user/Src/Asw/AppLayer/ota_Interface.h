#ifndef __TASK_OTA_INTERFACE_H
#define __TASK_OTA_INTERFACE_H

#include "AppHeaderSummary.h"

typedef enum {
    eUpdateObj_Null, 
    eUpdateObj_A,   //充电板
    eUpdateObj_B,   //网络板
    eUpdateObj_C,   //网络板
} eUpdateObj;


//下载状态
typedef enum {
    eOtaSta_Idle     	= 0,	//
    eOtaSta_Start,    			//开始从ftp获取文件
    eOtaSta_FtpFinish,    		//ftp下载文件完成
    eOtaSta_Trans,		        //modbus传输文件中
    eOtaSta_TransFinish,    	//mbs传输文件完成
    eOtaSta_Reboot,    		    //重启进行升级
}eDownloadState;

typedef enum {
    eFileType_None     	= 0,		//
    eFileType_CcuApp,    			//
    eFileType_CcuBoot,    			//
    eFileType_PcuApp,    			//
    eFileType_PcuBoot,    			//
    eFileType_PduApp,    			//
    eFileType_PduBoot,    			//
    eFileType_CardApp,    			//
    eFileType_CardBoot,    			//
}eFileType;

//升级过程
#define OTA_CRC_NULL           	0
#define OTA_CRC_ERR           	1		//包头校验错误
#define OTA_CRC_OK         		2
//尝试次数
#define OTA_RETRY_CNT           5

//
#define OTA_PACK_FRAME_CNT      64		//一包64帧

#define OTA_CUT_PACK_SIZE       (OTA_PACK_FRAME_CNT*OTA_FRAME_SIZE)	//一包64k

#define FILE_NAME_SIZE        	64



//升级状态 boot中会使用 勿轻易更改
//升级流程
typedef enum
{
    eOTA_Boot_Idle,						//
    eOTA_Boot_FileCheck,				//下载完成
    eOTA_Boot_Trans,					//传输完成-
    eOTA_Boot_Backup,					//备份完成-
    eOTA_Boot_Copy,						//复制完成
    eOTA_Boot_RollBack,					//回滚
    eOTA_Boot_Jump,						//跳转
}BOOT_OTA_E;


//===============================================

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
    char          binFileName[16];    // 升级文件名称，可根据名称判断是否选择升级app/boot, Aapp表示升级A应用程序，Aboot表示升级boot程序，其他不升级
    uint8_t       rvsd[70];           // 预留
}OTA_HEAD_T;

#define OTA_HEAD_LEN      sizeof(OTA_HEAD_T)


typedef struct __OTA_DCB__
{
    eUpdateObj UpdateObj;
	eDownloadState DownloadState;
	eFileType FileType;
	U8	u8TryCnt;										//重试次数
	//文件
	U32	u32FileHandle;									//文件句柄
	U32 u32FileSize;									//文件大小
	U32	u32FileAddCheck;								//文件校验
	//分包
	U8	u8PackCnt;										//包数量
	U8	u8PackIndex;									//包序号
	U8	u8PackSize;										//包大小
	//分帧
	U32	u32FrameCnt;									//帧数量
	U32	u32FrameIndex;									//帧序号
	U32	u32FrameLen;									//包序号
	U32	u32FinalFrameLen;								//最后一帧长度
	//读
	U32 u32ReadOffDest;									//ftp文件偏移
	U32 u32ReadLen;										//ftp读取长度

    
	//升级过程参数-modbus
	uint32_t	u32ResetTick;        //升级完成重启时间
    uint8_t		MOtaState;    		 //
	uint32_t	Mtimeouttick;
	uint32_t	MStartTimeouttick;		//没开始升级的超时
	uint32_t	MRestarttick;

}OtaDCB;

extern OtaDCB g_OtaDCB;

//===============================================
uint16_t ota_crc16(void* pInBuf, uint32_t nLen);

uint8_t ota_head_check(uint8_t *pOtaData);

void ota_run_check(void);

void OtaInit(void);

void OtaStart(void);
void OtaStart_tcp(void);

void ota_file_storage(uint8_t *pOtaFileData, uint16_t len);
uint8_t ota_get_nextFile(void);

//升级进度
uint16_t OtaStep(void);
//void Ota_ClrDownloadSpace(void);

//升级开始
void OtaStart_http(void);

void set_ota_finish(void);

void ota_download_finish(void);

//是否升级中
uint8_t OtaGetUpdatingFlag(void);

void OtaSetState(eDownloadState state);

uint8_t fgv_getUpdataTrans(uint8_t obj);
uint8_t fgv_getUpdataObj(void);
void OtaStart_UpdateObj(uint8_t obj);
uint16_t fgv_otaFileTrans(uint8_t *MbsFileData, uint16_t *len);
void fgv_otaFileRecvCheck(uint8_t *MbsFileData);

void fgv_otaMain(void);


void Ota_m_stop(void);
#endif

