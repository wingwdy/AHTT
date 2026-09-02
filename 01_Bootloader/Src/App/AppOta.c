#include "AppOta.h"

#define EXT_FLASH_OTA_PARAM_ADDR 			((448 + 448) * 1024)//升级参数存储区域

/***********************************************************************************
 * 流程基于eBootOtaState的状态
 * eOTA_Boot_Idle：
 * eOTA_Boot_FileCheck：有文件需要进行升级，进行校验判断升级文件完整性，以及判断升级类型
 * eOTA_Boot_Backup：文件无误后，进行旧可执行性文件备份存储，防止新的升级文件异常
 * eOTA_Boot_Copy：将升级文件拷贝到运行区，开始执行，置位rollback
 * eOTA_Boot_RollBack：如果运行新文件正常，置位idle,如有异常，重启退回，执行rollback
 * eOTA_Boot_Jump：可以跳转到app
***********************************************************************************/

static uint8_t ota_data_buf[OTA_FRAME_SIZE] = {0};
static OTA_CTX_T g_OtaCtx;
stu_Cfg_t g_pstuCfg;

static void Uint32ToFourUint8(uint8_t *pu8, uint32_t CurValue)
{
    *pu8++ = (CurValue & 0x00ff);
    *pu8++ = ((CurValue >> 8) & 0x00ff);
    *pu8++ = ((CurValue >> 16) & 0x00ff);
    *pu8++ = ((CurValue >> 24) & 0x00ff);
}

static uint32_t fourUint8ToUint32(uint8_t *pu8)
{
    uint32_t temp1, temp2, temp3, temp4;
	
    temp1 = *pu8++;
    temp2 = *pu8++;
    temp3 = *pu8++;
    temp4 = *pu8++;
    return ((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1);
}

static uint16_t twoUint8ToUint16(uint8_t *pu8)
{
    uint8_t temp1, temp2;
    temp1 = *pu8++;
    temp2 = *pu8++;
    return ((temp2 << 8) + temp1);
}

static uint8_t u8GeneralAddition(void *pData, uint32_t len)
{
	uint8_t u8Value = 0;
	uint8_t *pCurPointer = (uint8_t *)pData;
	
	for (; len != 0; len--)
	{
		u8Value += (*pCurPointer);
		pCurPointer++;
	}
	
	return (u8Value);
}

static void PackData(void *pData1, uint32_t stuctLen)
{
	uint32_t ctrlWord = 0;
	uint8_t *pData = (uint8_t*)pData1;
	uint8_t cs;
	
	if (NULL == pData && stuctLen == 0) {
		return;
	}
	cs = u8GeneralAddition(pData+4, stuctLen-4);
	ctrlWord = SET_CHECK_SUM(ctrlWord, cs);
	ctrlWord = SET_VALID_FLAG(ctrlWord, VALID);
	ctrlWord = SET_LEN(ctrlWord,stuctLen-4);
	Uint32ToFourUint8(pData,ctrlWord);
	
	return ;
}

static uint8_t CheckDataPack(void *pData, uint32_t stuctLen)
{
	uint32_t ctrlWord;
	uint8_t* pSrc = (uint8_t*)pData ;
	
	if (NULL == pSrc && stuctLen == 0) {
		return RESET;
	}
	
	ctrlWord = fourUint8ToUint32(pSrc);
	
	//标志或数据长度不对
	if ((GET_VALID_FLAG(ctrlWord) != VALID) || (GETET_LEN(ctrlWord) != (stuctLen -4)))
	{
		return RESET;
	}
	
	if (GET_CHECK_SUM(ctrlWord) == u8GeneralAddition(pSrc+4 ,stuctLen-4)) 
	{
		return SET;
	}
	
	return RESET;
}

static uint16_t ota_crc16(void* pInBuf, uint32_t nLen)
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

static uint32_t CheckSum(uint8_t *pData, uint32_t len)
{
    uint32_t cs = 0;
    uint8_t *pCur = (uint8_t *)pData;
	
    for (; len != 0; len--)
    {
        cs += (*pCur);
        pCur++;
    }
	
    return (cs);
}

/*******************************************************
*
*
*
*******************************************************/
static uint8_t STO_Read(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = bsp_flash_read(addr, pSrc, size);
	
	return flag;
}

static uint8_t STO_Write(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	bsp_flash_sector_erase(u32Dest);
	flag = bsp_flash_write_page(u32Dest, pSrc, size);
	
	return flag;
}


uint8_t STO_OTA_Read_Download(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = STO_USER_UPDATA_DATA_ADDR + u32Dest;
	
	printf("Boot---STO_OTA_Read_Download addr = 0x%x\r\n", u32Dest);

	flag = bsp_flash_read(addr, pSrc, size);
	
	return flag;
}


uint8_t STO_OTA_Write_Backup(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = STO_USER_BACKUP_DATA_ADDR + u32Dest;

	uint16_t pageSize = 256;
	uint32_t pageAddr = addr;
	uint32_t pageOffset = 0;

	for (int i = 0; i < (size / 256); i++) {
		//最后一包
		if ( i == (size / 256) - 1) {
			if (size % 256) {
				pageSize = size % 256;
			}
		}
		flag = bsp_flash_write_page(pageAddr, pSrc+pageOffset, pageSize);
		pageAddr = pageAddr + pageSize;
		pageOffset = pageOffset + pageSize;
	}
	
	return flag;
}
uint8_t STO_OTA_Read_Backup(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = STO_USER_BACKUP_DATA_ADDR + u32Dest;
	
	flag = bsp_flash_read(addr, pSrc, size);
	
	return flag;
}
uint8_t STO_OTA_Erase_Backup()
{
	uint8_t flag = RESET;
	uint32_t addr = STO_USER_BACKUP_DATA_ADDR;
	
	for (int i = 0; i < APP_MAX_FRAME_CNT / 4; i++) {
		flag = bsp_flash_sector_erase(addr);
		addr = addr + 4 * 1024;
	}
	return flag;
}


//清除片内flash内容
fmc_state_enum clear_Flash(uint32_t PackLenth)
{
	fmc_state_enum status;
	uint8_t pages = PackLenth / FLASH_PAGE_SIZE;
	uint8_t i = 0;
	
	fmc_unlock();
	for(i = 0; i <= pages; i++)
	{
		status = fmc_page_erase(APP_START_ADDRESS + FLASH_PAGE_SIZE * i);
		if (status != FMC_READY)
		{
			return status;
		}
	}
	
	return FMC_READY;
}

/*
 *@brief 向片内flash写入1个page内容
*/
fmc_state_enum update_file_downloading(uint32_t Addr, uint32_t *pBuf, uint32_t len)
{
	fmc_state_enum status;
	uint16_t i = 0;
	for (i = 0; i < len/4; i++)
	{
//		feedDog();
		status = fmc_word_program(Addr + APP_START_ADDRESS, pBuf[i]);
		if (status != FMC_READY)
		{
			return status;
		}
		Addr += 4;
	}
	
	return FMC_READY;
}

uint8_t STO_OTA_Write_Internal(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	uint32_t InData;
	// fmc_unlock();
	update_file_downloading(u32Dest, pSrc, size);
	return flag;
}

uint8_t STO_OTA_Read_Internal(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;

	uint32_t OutData;
	
	for (int i = 0; i < size; i+=4) {
		addr = i + u32Dest + APP_START_ADDRESS;
		OutData=(*(__IO uint32_t*)(addr));
		memcpy(pSrc+i, &OutData, 4);
	}
	
//	flag = fgu1_StoreDataRead(APP_STORE_MEDIUM_INTERNAL_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}



uint8_t flash_check(uint32_t ucVer)
{
	stu_Cfg_t *pstuCfg = &g_pstuCfg;

	uint32_t UniqueId = 0;
	
	bsp_flash_read_id(&UniqueId);

	STO_Read(EXT_FLASH_OTA_PARAM_ADDR, pstuCfg , sizeof(stu_Cfg_t));

	if(pstuCfg->verBL == ucVer)//BL已初始化Flash通过,BL版本已写入外部Flash
    {
        return 0;		   
    }
	else//排除第一次初始化的情况
	{
		pstuCfg->verBL = ucVer;
		STO_Write(EXT_FLASH_OTA_PARAM_ADDR, pstuCfg , sizeof(stu_Cfg_t));
		pstuCfg->verBL = 0;
		STO_Read(EXT_FLASH_OTA_PARAM_ADDR, pstuCfg , sizeof(stu_Cfg_t));
		if(pstuCfg->verBL == ucVer)//BL已初始化Flash通过,BL版本已写入外部Flash
		{
			return 0;
		}
	}
	return 1;
}


uint8_t ota_Init(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	
	printf("Boot---ota_Init eBootOtaState = %d\r\n", pstuCfg->eBootOtaState);
	// STO_Read(EXT_FLASH_OTA_PARAM_ADDR, pstuCfg , sizeof(stu_Cfg_t));
	
	if(eOTA_Boot_Idle == pstuCfg->eBootOtaState)
	{
		pstuCfg->eBootOtaState = eOTA_Boot_Jump;
	}
	else if(pstuCfg->eBootOtaState > eOTA_Boot_Jump)
	{
		pstuCfg->eBootOtaState = eOTA_Boot_Jump;
	}


	memset(pOtaCtx, 0, sizeof(OTA_CTX_T));
	
	return SET;
}

static uint8_t ota_head_crc(OTA_HEAD_T *pOtaHead)
{
    uint16_t crc = 0;
	uint32_t len = fourUint8ToUint32(pOtaHead->binFileLen);
	
	crc = ota_crc16((void *)pOtaHead->flagUp1, FPOS(OTA_HEAD_T, crc_16));
	
	if((len < OTA_FRAME_SIZE) || (len > APP_MAX_SIZE)
		|| (crc != twoUint8ToUint16(pOtaHead->crc_16)))
	{		
		return RESET;
	}
	
	return SET;
}

static uint8_t ota_head_check(uint8_t *pOtaData)
{
    OTA_HEAD_T *pOtaHead = (OTA_HEAD_T*)pOtaData;
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	
	//包头校验
	if(SET != ota_head_crc(pOtaHead)) return RESET;
	
	pOtaCtx->binFileLen = fourUint8ToUint32(pOtaHead->binFileLen) + OTA_HEAD_LEN;
	pOtaCtx->u32FileCs = fourUint8ToUint32(pOtaHead->cs32);
	pOtaCtx->finalFrameLen = pOtaCtx->binFileLen%OTA_FRAME_SIZE;
	pOtaCtx->tatolFrameCnt = (pOtaCtx->binFileLen+OTA_FRAME_SIZE-1) / OTA_FRAME_SIZE;

	if (pOtaCtx->finalFrameLen == 0)
	{
		pOtaCtx->finalFrameLen = OTA_FRAME_SIZE;
	}
		
//	pOtaCtx->tatolCopyCnt = (fourUint8ToUint32(pOtaHead->binFileLen)+OTA_FRAME_SIZE-1) / OTA_FRAME_SIZE;
	
	return SET;
}

static void ota_file_check(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	uint8_t *pData = ota_data_buf;
	
	if(eOTA_Boot_FileCheck != pstuCfg->eBootOtaState)
		return;

	memset(pData, 0, OTA_FRAME_SIZE);
	
	if(0 == pOtaCtx->CsIndex)
	{
		printf("Boot---ota_file_check start\r\n");
	}

	STO_OTA_Read_Download(pOtaCtx->CsIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
	
	if(0 == pOtaCtx->CsIndex)
	{
		if(SET != ota_head_check(pData))
	    {
	    	pstuCfg->eBootOtaState = eOTA_Boot_Jump;
			pOtaCtx->FileCheckResult = eCheck_Error;
			printf("Boot---ota_file_check head erro\r\n");
	        return;
	    }
	}
	
	//第一帧就是尾帧的不处理，程序不可能这么小
	if(0 == pOtaCtx->CsIndex)
	{
		pOtaCtx->u32Cs += CheckSum(&pData[OTA_HEAD_LEN], OTA_FRAME_SIZE-OTA_HEAD_LEN);
	}
	else if((pOtaCtx->CsIndex+1) >= pOtaCtx->tatolFrameCnt)
	{
		pOtaCtx->u32Cs += CheckSum(pData, pOtaCtx->finalFrameLen);
	}
	else
	{
		pOtaCtx->u32Cs += CheckSum(pData, OTA_FRAME_SIZE);
	}
	
	pOtaCtx->CsIndex++;
	if(pOtaCtx->CsIndex >= pOtaCtx->tatolFrameCnt)
	{
		if(pOtaCtx->u32Cs != pOtaCtx->u32FileCs)
		{
			pstuCfg->eBootOtaState = eOTA_Boot_Jump;
			pOtaCtx->FileCheckResult = eCheck_Error;
			printf("Boot---ota_file_check addCheck erro\r\n");
			return;
		}
		
		pOtaCtx->BackupIndex = 0;
		pstuCfg->eBootOtaState = eOTA_Boot_Backup;
		// pOtaCtx->CopyIndex = 0;
		// pstuCfg->eBootOtaState = eOTA_Boot_Copy;
		
		pOtaCtx->FileCheckResult = eCheck_Right;
	}
	
	return;
}

static void ota_file_backup(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	uint8_t *pData = ota_data_buf;
	
	if(eOTA_Boot_Backup != pstuCfg->eBootOtaState)
		return;
	
	if (pOtaCtx->BackupIndex == 0) {
		//清除片外备份flash
		STO_OTA_Erase_Backup();
		printf("Boot---STO_OTA_Erase_Backup\r\n");
	}
	printf("Boot---ota_file_backup: BackupIndex = %d\r\n", pOtaCtx->BackupIndex);

	memset(pData, 0, OTA_FRAME_SIZE);
	
	STO_OTA_Read_Internal(pOtaCtx->BackupIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
	// printf("STO_OTA_Read_Internal: 0x%x  0x%x  0x%x  0x%x  0x%x \r\n", pData[0], pData[1], pData[2], pData[3], pData[4]);
	STO_OTA_Write_Backup(pOtaCtx->BackupIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
	
	pOtaCtx->BackupIndex++;
	if(pOtaCtx->BackupIndex >= APP_MAX_FRAME_CNT)
	{
		pOtaCtx->CopyIndex = 0;
		pstuCfg->eBootOtaState = eOTA_Boot_Copy;
	}
	return;
}

//static uint8_t u8cmp[128] = {0};
//uint8_t  uflag = 0;

static void ota_file_copy(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	uint8_t *pData = ota_data_buf;
	
	if(eOTA_Boot_Copy != pstuCfg->eBootOtaState)
		return;

	if (pOtaCtx->CopyIndex >= APP_MAX_FRAME_CNT) {
		return;
	}
	if (pOtaCtx->CopyIndex == 0) {
		clear_Flash(APP_MAX_FRAME_CNT * 1024);
		printf("Boot---ota_file_copy clear\r\n");
	}

	printf("Boot---ota_file_copy: CopyIndex = %d\r\n", pOtaCtx->CopyIndex);

	memset(pData, 0xFF, OTA_FRAME_SIZE);
	
	if(0 == pOtaCtx->CopyIndex)
	{
		STO_OTA_Read_Download(pOtaCtx->CopyIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
		STO_OTA_Write_Internal(0, pData + OTA_HEAD_LEN, OTA_FRAME_SIZE-OTA_HEAD_LEN);
	}
	else if((pOtaCtx->CopyIndex+1) >= pOtaCtx->tatolFrameCnt)
	{
		STO_OTA_Read_Download(pOtaCtx->CopyIndex*OTA_FRAME_SIZE, pData, pOtaCtx->finalFrameLen);
		STO_OTA_Write_Internal(pOtaCtx->CopyIndex*OTA_FRAME_SIZE-OTA_HEAD_LEN, pData, OTA_FRAME_SIZE);
	}
	else
	{
		STO_OTA_Read_Download(pOtaCtx->CopyIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
		STO_OTA_Write_Internal(pOtaCtx->CopyIndex*OTA_FRAME_SIZE-OTA_HEAD_LEN, pData, OTA_FRAME_SIZE);
	}
	
//	memset(u8cmp, 0xff, 128);
//	if(0xff == pData[0])
//	{
//		uflag = 0;
//	}
	pOtaCtx->CopyIndex++;
	if(pOtaCtx->CopyIndex >= pOtaCtx->tatolFrameCnt)
	{
		pstuCfg->eBootOtaState = eOTA_Boot_Jump;
	}
	return;
}

static void ota_file_rollback(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	uint8_t *pData = ota_data_buf;
	
	if(eOTA_Boot_RollBack != pstuCfg->eBootOtaState)
		return;
	if (pOtaCtx->RollbackIndex == 0) {
		clear_Flash(APP_MAX_FRAME_CNT * 1024);
	}
	
	printf("Boot---ota_file_rollback: %d\r\n", pOtaCtx->RollbackIndex);

	memset(pData, 0, OTA_FRAME_SIZE);
	
	STO_OTA_Read_Backup(pOtaCtx->RollbackIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
	STO_OTA_Write_Internal(pOtaCtx->RollbackIndex*OTA_FRAME_SIZE, pData, OTA_FRAME_SIZE);
	
	pOtaCtx->RollbackIndex++;
	if(pOtaCtx->RollbackIndex >= APP_MAX_FRAME_CNT)
	{
		pstuCfg->eBootOtaState = eOTA_Boot_Jump;
		pOtaCtx->FileCheckResult = eCheck_Right;
	}
	return;
}

uint8_t ota_file_jump(void)
{
    OTA_CTX_T *pOtaCtx = &g_OtaCtx;
	stu_Cfg_t *pstuCfg = &g_pstuCfg;
	
	if(eOTA_Boot_Jump != pstuCfg->eBootOtaState)
		return RESET;
	
	printf("Boot---ota_file_jump: eBootOtaState = %d, FileCheckResult = %d\r\n", pstuCfg->eBootOtaState, pOtaCtx->FileCheckResult);

	if(eCheck_Right != pOtaCtx->FileCheckResult) {
		pstuCfg->eBootOtaState = eOTA_Boot_Idle;
		STO_Write(EXT_FLASH_OTA_PARAM_ADDR, pstuCfg, sizeof(stu_Cfg_t));
		return SET;
	}

	//程序升级完成，避免新程序错误，回滚准备
	stu_Cfg_t l_pstuCfg;
	memcpy(&l_pstuCfg, pstuCfg, sizeof(stu_Cfg_t));
	l_pstuCfg.eBootOtaState = eOTA_Boot_RollBack;
	
	STO_Write(EXT_FLASH_OTA_PARAM_ADDR, &l_pstuCfg, sizeof(stu_Cfg_t));

	return SET;
}



void ota_task(void)
{
	ota_file_check();		//将flash里的程序读出进行校验，失败直接进入应用程序，校验成功进入backup
	ota_file_backup();		//从片内flash取出程序存入外部flash备份区, 再进入copy
	ota_file_copy();		//从外部flash升级区取出数据放入片内flash应用程序部分
	ota_file_rollback();	//从外部flash备份区取出数据放入片内flash应用程序部分
	return;
}

typedef void (*fun)(void);             
fun AppStart;
uint32_t JumpAddress_sp = 0;
static void jumpApp(void)
{
    uint32_t JumpAddress;
	
	JumpAddress_sp = (*(__IO uint32_t *)(APP_START_ADDRESS));

    if ( ((*(__IO uint32_t *)(APP_START_ADDRESS)) & 0x2FFE0000) ==
         0x20000000 )    
    {
        __disable_irq();

        JumpAddress = *(volatile uint32_t *)(APP_START_ADDRESS + 4);//转到后半段的Reset_Handler函数
        AppStart = (fun)JumpAddress;
        __set_MSP(*(volatile uint32_t *)(APP_START_ADDRESS));
        AppStart();                                 
    }
}

// typedef void (*app_func_t)(void);
// static void jumpApp(void)
// {
//     uint32_t sp = *(__IO uint32_t *)APP_START_ADDRESS;
//     uint32_t pc = *(__IO uint32_t *)(APP_START_ADDRESS + 4);

//     // 检查 SP 是否在 SRAM 范围（GD32E503 有 64KB/96KB SRAM，起始 0x20000000）
//     if ((sp & 0x2FFF0000) != 0x20000000) {
//         return;
//     }

//     // 1. 禁用中断
//     __disable_irq();
//     __DSB();
//     __ISB();

//     // 2. 清除所有挂起的中断
//     for (int i = 0; i < 8; i++) {
//         NVIC->ICPR[i] = 0xFFFFFFFF;
//     }

//     // 3. 复位 SysTick（Bootloader 可能开启了）
//     SysTick->CTRL = 0;
//     SysTick->LOAD = 0;
//     SysTick->VAL = 0;

//     // 4. 设置向量表（关键！）
//     SCB->VTOR = APP_START_ADDRESS;
//     __DSB();
//     __ISB();

//     // 5. 可选：关闭 Flash 预取指
//     FMC_WS &= ~FMC_WS_PFEN;
//     __DSB();

//     // 6. 设置主栈指针并跳转
//     __set_MSP(sp);
//     __DSB();
//     __ISB();

//     ((app_func_t)pc)();
    
//     // 不应该执行到这里
//     while(1);
// }

void sflv_BootUpdateJumpToAppManage(void)
{    
	if(SET == ota_file_jump())
	{
		jumpApp();
	}
}



