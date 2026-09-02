#include "ota_Interface.h"
#include "CommInterface.h"
#include "RouteHeaderSummary.h"
#include "protocol_ctrl.h"
#include "AppStorage.h"
#include "mbsMaster.h"


//=========================================================
OtaDCB g_OtaDCB;
UPDATA_PARA g_UpdataParam;
static uint8_t g_pMbsData[256];
//=========================================================
uint16_t ota_crc16(void* pInBuf, uint32_t nLen)
{
    uint8_t *pBuf = (uint8_t *)pInBuf;
    uint32_t i, j;
    uint16_t wTemp = 0, wFlag = 0;
	
    for (i = 0; i < nLen; i++)
    {
        wTemp ^= *(pBuf + i) << 8;
        for (j = 0; j < 8; j++)
        {
            wFlag = wTemp & 0x8000;
            wTemp <<= 1;
            if (wFlag)
            {
                wTemp ^= 0x1021;
            }
        }
    }
	
    return wTemp;
}

static uint8_t ota_head_crc(OTA_HEAD_T *pOtaHead)
{
    uint16_t crc = 0;
	uint32_t len = fourUint8ToUint32(pOtaHead->binFileLen);
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	crc = ota_crc16((void *)pOtaHead->flagUp1, FPOS(OTA_HEAD_T, cs32));
	printf("\r\ncrc = 0x%x\r\n", crc);
	
	crc = ota_crc16((void *)pOtaHead->flagUp1, FPOS(OTA_HEAD_T, crc_16));
	printf("crc = 0x%x\r\n", crc);
	
	if((len < OTA_FRAME_SIZE) || (len > 512*OTA_FRAME_SIZE)
		|| (crc != twoUint8ToUint16(pOtaHead->crc_16)))
	{
		//失败
		pOtaDCB->u8TryCnt = OTA_RETRY_CNT;
		LogPrintf(LVL_LOG_ERR, "OTA file head crc err! cal:%d  file:0x%x,0x%x  %d", crc, pOtaHead->crc_16[0], pOtaHead->crc_16[1], twoUint8ToUint16(pOtaHead->crc_16));
		return FALSE;
	}
	
	return TRUE;
}


//文件长度在1k以内未做处理
uint8_t ota_head_check(uint8_t *pOtaData)
{
    OTA_HEAD_T *pOtaHead = (OTA_HEAD_T*)pOtaData;
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	// hex_dump("ota_head_check:", pOtaData, 64);
	//包头校验
	if(TRUE != ota_head_crc(pOtaHead)) return FALSE;
	
	//原始文件长度
	pOtaDCB->u32FileSize = fourUint8ToUint32(pOtaHead->binFileLen);
	
	//总长度=原始文件长度+包头
	pOtaDCB->u32FileSize += OTA_HEAD_LEN;
	pOtaDCB->u32FileAddCheck = fourUint8ToUint32(pOtaHead->cs32);
	pOtaDCB->u8PackIndex = 0; 
	pOtaDCB->FileType = eFileType_CcuApp;
	pOtaDCB->u32FinalFrameLen = pOtaDCB->u32FileSize%OTA_FRAME_SIZE;
	pOtaDCB->u32FinalFrameLen = (0 == pOtaDCB->u32FinalFrameLen) ? OTA_FRAME_SIZE : pOtaDCB->u32FinalFrameLen;

	
	
	//总包数
	pOtaDCB->u8PackCnt = (pOtaDCB->u32FileSize+OTA_CUT_PACK_SIZE-1) / OTA_CUT_PACK_SIZE;
	//总帧数
	pOtaDCB->u32FrameCnt = (pOtaDCB->u32FileSize+OTA_FRAME_SIZE-1) / OTA_FRAME_SIZE;
	
	return TRUE;
}

void ota_run_check(void)
{
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;
	
	if(eOTA_Boot_RollBack != pstuCfg->eBootOtaState)
		return;
	
	//升级后正常运行30S
	if(TRUE != JudgeTimeOutMs(0, eTick_15S))
		return;
	
	printf("ota_run_check 15s finish...ok!\r\n");
	
	pstuCfg->eBootOtaState = eOTA_Boot_Idle;
	
	Set_updataParam(pstuCfg);
	
	return;
}

void OtaInit(void)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	memset(pOtaDCB, 0, sizeof(OtaDCB));
	pOtaDCB->u32ReadLen = OTA_FRAME_SIZE;
	
	return;
}

static void Ota_ClrDownloadSpace(void)
{
	uint32_t u32EraseAddr = EXT_FLASH_ADDR_OTA_DOWNLOAD; 
	
	//清flash
	while(u32EraseAddr < (EXT_FLASH_LEN_OTA_FILESIZE+EXT_FLASH_ADDR_OTA_DOWNLOAD))
	{
		fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_BLOCK_ERASE_TYPE, u32EraseAddr);
		u32EraseAddr += EXT_FLASH_BLOCK_SIZE;
	}
	
	return;
}

void OtaSetState(eDownloadState state)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	
	pOtaDCB->MOtaState = state;

	pOtaDCB->Mtimeouttick = Get_Systick();

	printf("OtaSetState stae = %d\r\n", state);
}

uint8_t OtaGetUpdatingFlag()
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	
	if(eOtaSta_Idle != pOtaDCB->MOtaState)
		return 1;
	return 0;
}


uint16_t OtaStep(void)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	uint16_t u16Step = 0;
	
	u16Step = (pOtaDCB->u32FrameIndex*100) / pOtaDCB->u32FrameCnt;
	return u16Step;
}

void OtaStart_http(void)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	memset(pOtaDCB, 0, sizeof(OtaDCB));
	pOtaDCB->u32ReadLen = OTA_FRAME_SIZE;
	
	Ota_ClrDownloadSpace();
	
	Comm_HttpsStart();
	
	return;
}

void OtaStart_tcp()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	memset(pOtaDCB, 0, sizeof(OtaDCB));
	pOtaDCB->u32ReadLen = OTA_FRAME_SIZE;
	
	Ota_ClrDownloadSpace();
	
	tcpSwitchFtp(eSocket_GPRS1);

	return;
}

void Ota_m_start()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	OtaSetState(eOtaSta_Start);
	
	pOtaDCB->MStartTimeouttick = Get_Systick();
	
	return;
}

void Ota_m_stop()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	memset(pOtaDCB, 0, sizeof(OtaDCB));
	
	return;
}


void ota_download_finish(void)
{
	OtaDCB *pOtaDCB = &g_OtaDCB;
	
	if(eOtaSta_Reboot != pOtaDCB->MOtaState)
		return;	

	//3s后复位
	if(TRUE != JudgeTimeOutMs(pOtaDCB->u32ResetTick, eTick_3S))
		return;
	
	OtaSetState(eOtaSta_Idle);
	
	fgv_AppSoftwareReset();
	
	return;
}


//下载完成后部分标志位清零
void ota_downFinish_clear()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	pOtaDCB->u32FrameIndex = 0;	
	pOtaDCB->u8PackIndex = 0;
}

//可外部调用，存储ota数据
void ota_file_storage(uint8_t *pOtaFileData, uint16_t len)
{

	OtaDCB* pOtaDCB = &g_OtaDCB;
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;
	
	if(0 == pOtaDCB->u32FrameIndex)
	{
		printf("\r\nOtaStart_tcp\r\n");
		if(TRUE != ota_head_check(pOtaFileData)) {
			OtaSetState(eOtaSta_FtpFinish);
			return;
		}
		Ota_m_start();
	}

	printf("\r\nfile: %d  %d  %d  %d\r\n", pOtaDCB->u32FrameIndex, pOtaDCB->u32FrameCnt, pOtaDCB->u32ReadLen, pOtaDCB->u32ReadOffDest);

	STO_OTA_Write(pOtaDCB->u32FrameIndex*OTA_FRAME_SIZE, pOtaFileData, pOtaDCB->u32ReadLen);

	//存储后帧序号自增
	pOtaDCB->u32FrameIndex++;

	//存储完成自增包序号
	if(pOtaDCB->u32FrameIndex >= pOtaDCB->u32FrameCnt){
		OtaSetState(eOtaSta_FtpFinish);
		ota_downFinish_clear();

		printf("\r\nota_file_storage: ftp eOtaSta_FtpFinish\r\n");
		return;
	} else if(0 != pOtaDCB->u32FrameIndex && 0 == pOtaDCB->u32FrameIndex%OTA_PACK_FRAME_CNT)
	{
		pOtaDCB->u8PackIndex++;
	}
	
	//字节数偏移，一帧偏移1024，累加
	pOtaDCB->u32ReadOffDest = (pOtaDCB->u32FrameIndex%OTA_PACK_FRAME_CNT)*OTA_FRAME_SIZE;

	if(pOtaDCB->u32FrameIndex+1 == pOtaDCB->u32FrameCnt)
		pOtaDCB->u32ReadLen = pOtaDCB->u32FinalFrameLen;
	else
		pOtaDCB->u32ReadLen = OTA_FRAME_SIZE;
	
	pOtaDCB->u32ReadLen = pOtaDCB->u32ReadLen > OTA_FRAME_SIZE ? OTA_FRAME_SIZE:pOtaDCB->u32ReadLen;

}

//可外部调用，获取是否需要取下一包，64k一包
uint8_t ota_get_nextFile()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	static uint8_t pre_packIndex = 0;
	if (pre_packIndex != pOtaDCB->u8PackIndex) {
		pre_packIndex = pOtaDCB->u8PackIndex;
		return 1;
	}
	return 0;
}





typedef struct _MbsTransFile{
	uint32_t startTime;	//传输开始时间

	uint8_t mbsAddr;	//地址码
	uint8_t funcode;	//功能码
	uint16_t addcheck;	//程序和校验
	uint32_t totalBytes;//总字节数
	uint16_t totalPack;	//总包数
	uint16_t crtPack;	//当前索引包
	uint16_t crtDataLen;//当前数据长度
} MbsTransFile;

#define TRANS_FILE_LEN	240
MbsTransFile mbsFile;
MbsTransFile *pMbsTF = &mbsFile;

void OtaStart_UpdateObj(uint8_t obj)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	
	pOtaDCB->UpdateObj = obj;

	memset(pMbsTF, 0, sizeof(MbsTransFile));
	
	return;
}


void MbsFileTransFinish()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;
	// pOtaDCB->MOtaState = eOtaSta_TransFinish;
	OtaSetState(eOtaSta_Idle);
	pOtaDCB->UpdateObj = eUpdateObj_Null;
}


//根据读取文件更新mbs传输文件数据头
static void fsv_setFileMbsHead()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	uint16_t addCheck;

	pMbsTF->startTime = Get_Systick();	//超时其实时间，传输超时5分钟

	pMbsTF->mbsAddr = 0;
	pMbsTF->funcode = 0x15;
	pMbsTF->totalBytes = pOtaDCB->u32FileSize;
	pMbsTF->addcheck = pOtaDCB->u32FileAddCheck;

	//索引从0开始
	if (pOtaDCB->u32FileSize % TRANS_FILE_LEN) {
		pMbsTF->totalPack = pOtaDCB->u32FileSize / TRANS_FILE_LEN + 1;
	} else {
		pMbsTF->totalPack = pOtaDCB->u32FileSize / TRANS_FILE_LEN;
	}
}


uint16_t fsv_fileMbsTransPack(uint8_t *MbsFileData, uint8_t *pOtaFileData, uint16_t len)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;

	pMbsTF->crtDataLen = len;

	memcpy(MbsFileData, &mbsFile.mbsAddr, 14);
	memcpy(&MbsFileData[14], pOtaFileData, len);
	
	return len + 14;
}

void fsv_otaFileTrans(uint8_t *MbsFileData, uint16_t *len)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;

	uint8_t pOtaFileData[256];

	uint16_t readLen = 0;

	//读取文件，开始传输
	if (pMbsTF->crtPack == (pMbsTF->totalPack - 1)) {
		readLen = pMbsTF->totalBytes % TRANS_FILE_LEN;
	} else {
		readLen = TRANS_FILE_LEN;
	}

	printf("\r\nfsv_otaFileTrans %d ctrPack = %d  totalPack = %d\r\n", NOWTICK, pMbsTF->crtPack, pMbsTF->totalPack);
	STO_OTA_Read(pMbsTF->crtPack*TRANS_FILE_LEN, pOtaFileData, readLen);
	*len = fsv_fileMbsTransPack(MbsFileData, pOtaFileData, readLen);
}

void fgv_otaFileRecvCheck(uint8_t *MbsFileData)
{
	if (MbsFileData[0] != 0) {
		return;
	}
	if (MbsFileData[1] != 0x16) {
		return;
	}
	
	//应答数据索引错误
	uint16_t index = 0;
	memcpy(&index, MbsFileData + 6, 2);
	if (index != pMbsTF->crtPack) {
		printf("\r\nRecvCheck erro. index erro...  recvIndex = %d, sendIndex = %d.", index, pMbsTF->crtPack);
		return;
	}

	//传输完成
	if (pMbsTF->crtPack >= (pMbsTF->totalPack - 1)) {
		MbsFileTransFinish();
	}

	if (MbsFileData[8] != 0) {
		printf("\r\nRecvCheck erro. check erro...  check:%d\r\n", MbsFileData[8]);
		return;
	}
	
	printf("\r\nRecvCheck Ok.  crtPack = %d.", pMbsTF->crtPack);
	pMbsTF->crtPack++;
}

uint8_t fgv_getUpdataTrans(uint8_t obj)
{
	OtaDCB* pOtaDCB = &g_OtaDCB;

	if ((pOtaDCB->MOtaState == eOtaSta_Trans)
		&& (obj == pOtaDCB->UpdateObj)) {
			return 1;
	}
	return 0;
}

uint8_t fgv_getUpdataObj()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;

	return pOtaDCB->UpdateObj;
}


//对于从服务器下载来的文件进行检查
void fsv_downloadFileCheck()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	if (pOtaDCB->MOtaState != eOtaSta_FtpFinish) {
		return;
	}

	uint8_t *pOtaFileData = g_pMbsData;
	
	STO_OTA_Read(0, pOtaFileData, OTA_HEAD_LEN);
	if(TRUE != ota_head_check(pOtaFileData)) {
		printf("\r\nfsv_downloadFileCheck head faild");
		MbsFileTransFinish();
		return;
	}
	
	OTA_HEAD_T *pOtaHead = (OTA_HEAD_T*)pOtaFileData;
	//根据升级文件头分辨升级对象
	if((strcmp(pOtaHead->binFileName, "appA") == 0)
	|| strcmp(pOtaHead->binFileName, "bootA") == 0) {
		OtaStart_UpdateObj(eUpdateObj_A);
	} else if((strcmp(pOtaHead->binFileName, "appB") == 0)
	|| strcmp(pOtaHead->binFileName, "bootB") == 0) {
		OtaStart_UpdateObj(eUpdateObj_B);
	} else if((strcmp(pOtaHead->binFileName, "appC") == 0)
	|| strcmp(pOtaHead->binFileName, "bootC") == 0) {
		OtaStart_UpdateObj(eUpdateObj_C);
	} else {
		printf("\r\nfsv_downloadFileCheck faild.  binFileName = %s", pOtaHead->binFileName);
		return;
	}
	

	//自己升级需要将标志位写入，其他升级开始传输文件
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;
	if (eUpdateObj_A == pOtaDCB->UpdateObj) {
		printf("\r\nfsv_downloadFileCheck start trans....\r\n");
		OtaSetState(eOtaSta_Trans);
		fsv_setFileMbsHead();		//开始传输之前mbs前缀数据更新
	} else {
		printf("\r\nfsv_downloadFileCheck restart....\r\n");
		pstuCfg->eBootOtaState = eOTA_Boot_FileCheck;
		Set_updataParam(pstuCfg);
		OtaSetState(eOtaSta_Reboot);
		pOtaDCB->u32ResetTick = Get_Systick();
	}
}

//ota传输
void fsv_transFileCheck()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	stu_UpdataCfg_t *pstuCfg = &g_pstuUpdataCfg;

	uint8_t *bufSend = g_pMbsData;

	uint16_t lenSend = 0;

	if (pOtaDCB->MOtaState != eOtaSta_Trans) {
		return;
	}

	static uint32_t start_tick = 0;
	//100ms传输一次
	if(TRUE != JudgeTimeOutMs(start_tick, eTick_200ms))
		return;
	start_tick = Get_Systick();

	//传输超时关闭升级
	if (JudgeTimeOutMs(pMbsTF->startTime, 5 * eTick_60S)) {
		printf("\r\nfsv_otaFileTrans TimeOut\r\n");
		MbsFileTransFinish();
	}

	fsv_otaFileTrans(bufSend, &lenSend);
	
	if (eUpdateObj_A == pOtaDCB->UpdateObj) {
		MbsMasteSendData(bufSend, lenSend);
	} else if (eUpdateObj_B == pOtaDCB->UpdateObj) {
		MbsMasteSendData(bufSend, lenSend);
	}
}
static void fsv_OtaTimeoutScan()
{
	OtaDCB* pOtaDCB = &g_OtaDCB;
	//5min后超时退出
	
	if (pOtaDCB->MOtaState) {
		//传输超时关闭升级
		if (JudgeTimeOutMs(pOtaDCB->Mtimeouttick, UPDATE_TIMEOUT_MS)) {
			printf("\r\nfsv_OtaTimeout 10min\r\n");
			MbsFileTransFinish();
			//重启两个主板
			Reboot_System(0);
		}
	}
}

void fgv_otaMain()
{
	fsv_downloadFileCheck();		//从ftp下载的问题进行头校验
	fsv_transFileCheck();			//给其他主板传输文件
	ota_download_finish();			//重启升级
	fsv_OtaTimeoutScan();			//超时机制
}