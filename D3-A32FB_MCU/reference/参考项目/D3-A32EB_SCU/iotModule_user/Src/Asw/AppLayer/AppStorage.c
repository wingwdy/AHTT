/**********************************************************

**********************************************************/
#include "AppStorage.h"
#include "mbsDataUpdate.h"
#include "AppDealFlash.h"
#include "AppMidDataTrans.h"
#include "DisQRCode.h"
#include "screenUart.h"

uint8_t W25QXX_Read(void *pSrc, uint32_t u32Dest, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreReadRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}

uint8_t W25QXX_Write_safety(void *pSrc, uint32_t u32Dest, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreWriteRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}
uint8_t W25QXX_Erase_Sector(uint32_t u32Dest)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr);
	
	return flag;
}


/*===============================================*/
//参数定义
//SYSTEM_PARAM g_SystemParam;
//CHARGE_PARAM g_ChargeParam;
UP_PLAT_PARAM g_PlatParam;
//UPDATA_PARA g_UpdataParam;

/*===============================================*/
static uint8_t STO_Read(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreReadRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}

static uint8_t STO_Write(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreWriteRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}
static uint8_t STO_Erase(uint32_t u32Dest)
{
	uint8_t flag = RESET;
	uint32_t addr = u32Dest;
	
	flag = fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr);
	
	return flag;
}




//平台参数
PlatCfgInfo g_pltCfgInfo;
//升级参数
stu_UpdataCfg_t g_pstuUpdataCfg;
//二维码信息
stu_QrCodeInfo_t g_QrCodeInfo;

PlatCfgInfo *fgv_GetPlatCfgInfo()
{
	return &g_pltCfgInfo;
}

//获取当前卡类型
uint8_t get_ChgParam_Card_type(void)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
	
	return pst_cfgInfo->PltMainCardType;
}
//获取当前平台类型
uint8_t get_ChgParam_plat_type(void)
{
	PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;
	
	return pst_cfgInfo->PltMainType;
}
//初始化平台配置参数
static void Init_platParam(PlatCfgInfo *initPlat)
{
	memcpy(initPlat->PltAuxiliaryIp, "pmgmt.gongniu.cn", strlen("pmgmt.gongniu.cn"));
	memcpy(initPlat->PltMainIp, "pile.gongniu.cn", strlen("pile.gongniu.cn"));
	initPlat->PltAuxiliaryPort = 45113;
	initPlat->PltMainPort = 5455;
	initPlat->PltMainCardType = CARD_GN;
	initPlat->PltMainType = ePlatType_GN;
}
//读取平台配置参数
static uint8_t load_platParam(PlatCfgInfo *pst_info)
{
	uint32_t u32Dest = 0;
	PlatCfgInfo strFSysePara;

	memset(&strFSysePara, 0, sizeof(PlatCfgInfo));
	
	u32Dest = EXT_FLASH_PLAT_PARAM_ADDR;
	STO_Read(u32Dest, &strFSysePara, sizeof(PlatCfgInfo));
	
	if (FALSE == CheckDataPack(&strFSysePara, sizeof(PlatCfgInfo))) 
	{
		return FALSE;
	}
	
	memcpy(pst_info, &strFSysePara, sizeof(PlatCfgInfo));
	return TRUE;
}
uint8_t Set_platParam(PlatCfgInfo *pst_info)
{
	uint32_t u32Dest = 0;
	PlatCfgInfo strFChargePara;

	memset(&strFChargePara, 0, sizeof(PlatCfgInfo));
	memcpy(&strFChargePara, pst_info, sizeof(PlatCfgInfo));
	
	/*保存*/
	PackData((void*)(&strFChargePara), sizeof(PlatCfgInfo));
	u32Dest = EXT_FLASH_PLAT_PARAM_ADDR;
	
	STO_Erase(u32Dest);
	STO_Write(u32Dest, &strFChargePara, sizeof(PlatCfgInfo));

	return TRUE;
}


//读取平台配置参数
static uint8_t load_UpdataParam(stu_UpdataCfg_t *pst_info)
{
	uint32_t u32Dest = 0;
	stu_UpdataCfg_t strFSysePara;

	memset(&strFSysePara, 0, sizeof(stu_UpdataCfg_t));
	
	u32Dest = EXT_FLASH_OTA_PARAM_ADDR;
	STO_Read(u32Dest, &strFSysePara, sizeof(stu_UpdataCfg_t));
	
	
	memcpy(pst_info, &strFSysePara, sizeof(stu_UpdataCfg_t));
	return TRUE;
}
uint8_t Set_updataParam(stu_UpdataCfg_t *pst_info)
{
	uint32_t u32Dest = 0;
	stu_UpdataCfg_t strFChargePara;
	uint16_t u16Len = sizeof(stu_UpdataCfg_t);

	memset(&strFChargePara, 0, u16Len);
	memcpy(&strFChargePara, pst_info, u16Len);
	
	u32Dest = EXT_FLASH_OTA_PARAM_ADDR;
	
	STO_Erase(u32Dest);
	STO_Write(u32Dest, (uint8_t *)&strFChargePara, u16Len);

	return TRUE;
}


//读取二维码信息
uint8_t load_qrCodeInfo(stu_QrCodeInfo_t *Qr_info)
{
	uint32_t u32Dest = 0;
	stu_QrCodeInfo_t strFQrCodeInfo;

	memset(&strFQrCodeInfo, 0, sizeof(stu_QrCodeInfo_t));
	
	u32Dest = EXT_FLASH_QRCODE_ADDR;
	STO_Read(u32Dest, &strFQrCodeInfo, sizeof(stu_QrCodeInfo_t));
	
	if (FALSE == CheckDataPack(&strFQrCodeInfo, sizeof(stu_QrCodeInfo_t))) 
	{
		return FALSE;
	}
	
	memcpy(Qr_info, &strFQrCodeInfo, sizeof(stu_QrCodeInfo_t));
	return TRUE;
}
//存储二维码信息
uint8_t Set_qrCodeInfo(stu_QrCodeInfo_t *Qr_info)
{
	uint32_t u32Dest = 0;
	stu_QrCodeInfo_t strFQrCodeInfo;

	memset(&strFQrCodeInfo, 0, sizeof(stu_QrCodeInfo_t));
	memcpy(&strFQrCodeInfo, Qr_info, sizeof(stu_QrCodeInfo_t));
	
	/*保存*/
	PackData((void*)(&strFQrCodeInfo), sizeof(stu_QrCodeInfo_t));
	u32Dest = EXT_FLASH_QRCODE_ADDR;
	
	STO_Erase(u32Dest);
	STO_Write(u32Dest, &strFQrCodeInfo, sizeof(stu_QrCodeInfo_t));

	return TRUE;
}
void storage_qrCodeInfoStr(char *input)
{
    for (uint8_t i = 0; i < GUN_NUM_MAX; i++) {
        memset(&g_QrCodeInfo.qrcodeInfo[i][0], '\0', sizeof(g_QrCodeInfo.qrcodeInfo[i]));
        SpecialCharQrcodeStringConfig(&g_QrCodeInfo.qrcodeInfo[i][0], input);
    }
    Set_qrCodeInfo(&g_QrCodeInfo);      //存储
    
    PlatDevNumberChange(2);     //屏幕显示
}
/************************************************************************************************************************
 * 功能：平台配置二维码
 * u8Port = 0表示多把枪需要同时更新
 * u8Port = 1表示1枪需要更新
 * u8Port = 2表示2枪需要更新，以此类推
 * *input为二维码字符串
 ***********************************************************************************************************************/
void storage_PlatQRCodeInfoStr(uint8_t u8Port, char *input)
{
    if (u8Port > GUN_NUM_MAX) {
        return;
    }
    if (u8Port == 0) {
        for (uint8_t i = 0; i < GUN_NUM_MAX; i++) {
            memset(&g_QrCodeInfo.qrcodeInfo[i][0], '\0', sizeof(g_QrCodeInfo.qrcodeInfo[i]));
            NormalCharQrcodeStringConfig(&g_QrCodeInfo.qrcodeInfo[i][0], input);
        }
    } else{
        uint8_t qrN = u8Port - 1;
        memset(&g_QrCodeInfo.qrcodeInfo[qrN][0], '\0', sizeof(g_QrCodeInfo.qrcodeInfo[qrN]));
        NormalCharQrcodeStringConfig(&g_QrCodeInfo.qrcodeInfo[qrN][0], input);
    }
    Set_qrCodeInfo(&g_QrCodeInfo);      //存储
    
    PlatDevNumberChange(2);     //屏幕显示
}
//初始化二维码信息
void Update_qrCodeInfo()
{
    #define QRCODE_GN_STRING    "https://evse.gongniu.cn/car.html?qRCode="
    #define QRCODE_YKC_STRING    "http://www.ykccn.com/MPAGE/index.html?pNum="
	#define QRCODE_AHTT_STRING	 "https://www.ahttcd.cn/towercharge/open?equipmentid=[pile]"
    #define QRCODE_NULL_STRING    ""

    char headStr[100] = {"\0"};
    if (g_pltCfgInfo.PltMainType == ePlatType_GN) {
        strcpy(headStr,QRCODE_GN_STRING);
    } else if (g_pltCfgInfo.PltMainType == ePlatType_YKC) {
        strcpy(headStr,QRCODE_YKC_STRING);
    } else if (g_pltCfgInfo.PltMainType == ePlatType_AHTT){
        strcpy(headStr,QRCODE_AHTT_STRING);
	} else {
        strcpy(headStr,QRCODE_NULL_STRING);
    }

    storage_qrCodeInfoStr(headStr);         //更新存储信息
}

/* 在标品迭代过程中，前期二维码是由软件直接填充完整的（前缀+桩号+枪号），此时flash里存储的二维码里是不含[plle]和[gun 
   后续调整了一下策略，在二维码的前缀里追加[plle]和[gun，及其他相关策略，最终完成二维码的显示，但是代码整合导致一些bug
   导致屏幕二维码显示没有桩号，本质的原因是前缀里不带[pile]，故加一些弥补的措施，处理已经烧了老版本程序的板子，如果有问题，再重新默认一下*/
uint8_t Check_qrCodeInfoValid(void)
{
    uint8_t ret = TRUE;
    const char *pile_pattern = "[pile]";
    const char *gun_pattern = "[gun:";
    /* 不管几把枪， 对于0号枪而言，只要有1把枪的规则对，那整体就不会错 */
    const char *pile_pos = strstr((char *)&g_QrCodeInfo.qrcodeInfo[0][0], pile_pattern);
    const char *gun_pos = strstr((char *)&g_QrCodeInfo.qrcodeInfo[0][0], gun_pattern);

    if (g_pltCfgInfo.PltMainType == ePlatType_AHTT)
    {
        if (pile_pos == NULL)
        {
            ret = FALSE;
        }
    }
    else if (g_pltCfgInfo.PltMainType == ePlatType_GN)
    {
        if ((pile_pos == NULL) || (gun_pos == NULL))
        {
            ret = FALSE;
        }
    }
    else if (g_pltCfgInfo.PltMainType == ePlatType_YKC)
    {
        if ((pile_pos == NULL) || (gun_pos == NULL))
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}


//每个运营平台特有参数
uint8_t load_EEOP_Param(uint8_t *data, uint16_t u16Len)
{
    if (u16Len > 250) {
        printf("erro: load_EEOP_Param len = %d\r\n", u16Len);
        return FALSE;
    }
    uint8_t tmpData[256] = {0};
    
	uint32_t u32Dest = EXT_FLASH_EEOP_ADDR;
	
	STO_Read(u32Dest, tmpData, u16Len + 4);

	if (FALSE == CheckDataPack(tmpData, u16Len + 4)) 
	{
        printf("erro: load_EEOP_Param check\r\n");
		return FALSE;
	}
    
    memcpy(data, &tmpData[4], u16Len);

	return TRUE;
}
uint8_t Set_EEOP_Param(uint8_t *data, uint16_t u16Len)
{
    if (u16Len > 250) {
        printf("erro: Set_EEOP_Param len = %d\r\n", u16Len);
        return FALSE;
    }
    uint8_t tmpData[256] = {0};
	uint32_t u32Dest = EXT_FLASH_EEOP_ADDR;
    memcpy(&tmpData[4], data, u16Len);
    
	/*保存*/
	PackData((void*)(tmpData), u16Len + 4);

	STO_Erase(u32Dest);
	STO_Write(u32Dest, tmpData, u16Len + 4);

	return TRUE;
}
uint8_t Clear_EEOP_Param()
{
	STO_Erase(EXT_FLASH_EEOP_ADDR);                 //平台参数清除
	STO_Erase(EXT_FLASH_RATE_MODEL_ADDR);           //计费模型参数清除，切换平台需要清零
	return TRUE;
}




static void OdnStringChange(char *dest, char *src, uint8_t destMaxLen)
{
    //将src桩号赋值给dest
    uint8_t destlen = strlen(dest);
    uint8_t srclen = strlen(src);
    
    uint8_t cpyLen = (srclen >= (destMaxLen - 1)) ? (destMaxLen-1) : srclen;

    if ((destlen <= 4) && (srclen > 4)) {
        strncpy(dest, src, cpyLen);
		printf("OdnStringChange cpy finish. %d %d  %d %d\r\n", destlen, srclen, destMaxLen, cpyLen);
    }
}
static void OldVersionHandle()
{
    PlatCfgInfo *pCfgInfo = &g_pltCfgInfo;
    uint8_t len = strlen(pCfgInfo->fixDeviceNumber);
    uint8_t ret = memcmp(pCfgInfo->PltAuxiliaryIp, "pmgmt.gongniu.cn", sizeof("pmgmt.gongniu.cn")); //相同返回0
    if ((len > 4) && (ret == 0) && (pCfgInfo->PltAuxiliaryPort == 45113)) {
        printf("OldVersion Normal %d\r\n", len);
        return;
    }
    printf("OldVersion info:%d %d %d\r\n", len, ret, pCfgInfo->PltAuxiliaryPort);

    //10006版本的运维平台连接默认值有问题,需要覆盖掉
	memcpy(pCfgInfo->PltAuxiliaryIp, "pmgmt.gongniu.cn", strlen("pmgmt.gongniu.cn"));
	pCfgInfo->PltAuxiliaryPort = 45113;

    uint8_t len1 = strlen(pCfgInfo->char16fixDeviceNumber);
    if (len1 >= 16) {
        pCfgInfo->char16fixDeviceNumber[15] = '\0';
    }
    if ((len1 >= 6) && (len1 < 16)) {
        memcpy(pCfgInfo->fixDeviceNumber, pCfgInfo->char16fixDeviceNumber, sizeof(pCfgInfo->char16fixDeviceNumber));    //使用旧资产码
    } else {
        OdnStringChange(pCfgInfo->fixDeviceNumber,pCfgInfo->pltDeviceNumber, 32);        //针对旧桩没设置资产码的桩来说
    }
	Set_platParam(pCfgInfo);
}

//读取所有配置参数
void load_AllParam(void)
{
	uint8_t result = FALSE;

	result = load_platParam(&g_pltCfgInfo);
	if (result == FALSE) {
		Init_platParam(&g_pltCfgInfo);
		Set_platParam(&g_pltCfgInfo);
	}
    OldVersionHandle(); //仅为了处理市场老版本
	load_UpdataParam(&g_pstuUpdataCfg);

    result = load_qrCodeInfo(&g_QrCodeInfo);

    if (result == FALSE) 
	{
        Update_qrCodeInfo();
    }
	else if (FALSE == Check_qrCodeInfoValid())
	{
		Update_qrCodeInfo();
	}
	else
	{}

	//寻找离存储起始位置
	Start_Find_Deal_Addr();	//debug第一次跑飞，单独上电没问题，怀疑寄存器问题
}


//平台下发时设置计费模型
void Save_rate_model(void *pRate, int len)
{
	uint32_t u32Dest = 0;
	RATE_MODEL_PARA strRateModelPara;
    if (len >= RATE_MODEL_MAX_LEN) {
        printf("Save_rate_model len error\r\n");
    }

	memset(&strRateModelPara, 0, sizeof(RATE_MODEL_PARA));
	memcpy(&strRateModelPara.RateModelData, pRate, len);
	PackData((void*)(&strRateModelPara), sizeof(RATE_MODEL_PARA));

	u32Dest = EXT_FLASH_RATE_MODEL_ADDR;
	STO_Erase(u32Dest);
	STO_Write(u32Dest, &strRateModelPara, sizeof(RATE_MODEL_PARA));
}

//平台下发时设置计费模型
uint8_t Read_rate_model(void *pRate, int len)
{
	uint32_t u32Dest = 0;
	RATE_MODEL_PARA strRateModelPara;
    if (len > RATE_MODEL_MAX_LEN) {
        printf("Read_rate_model len error\r\n");
		return FALSE;
    }
    
	memset(&strRateModelPara, 0, sizeof(RATE_MODEL_PARA));
	
	u32Dest = EXT_FLASH_RATE_MODEL_ADDR ;
	STO_Read(u32Dest, &strRateModelPara, sizeof(RATE_MODEL_PARA));
    
	if (FALSE == CheckDataPack(&strRateModelPara, sizeof(RATE_MODEL_PARA))) 
	{
        printf("Read_rate_model Check error\r\n");
		return FALSE;
	}
    memcpy((uint8_t *)pRate, strRateModelPara.RateModelData, len);
    return TRUE;
}


uint8_t STO_OTA_Write(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = EXT_FLASH_ADDR_OTA_DOWNLOAD + u32Dest;
	
	flag = fgu8_AppInfoStoreWriteRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}

uint8_t STO_OTA_Read(uint32_t u32Dest, void *pSrc, uint32_t size)
{
	uint8_t flag = RESET;
	uint32_t addr = EXT_FLASH_ADDR_OTA_DOWNLOAD + u32Dest;
	
	flag = fgu8_AppInfoStoreReadRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_ADDRESS_TYPE, addr, pSrc, size);
	
	return flag;
}


uint8_t STO_W_ChgRcd(void *pRcdData, uint32_t RcdLen, uint32_t index)
{
	uint32_t u32Dest = 0;
	CHARGE_RECORD strFlashRecord;
	
	if(0 == RcdLen || RcdLen > STO_CHARGE_RCD_SIZE)
		return RESET;
	
	if(index < 1) index = 1;
	
	memset(&strFlashRecord, 0, sizeof(CHARGE_RECORD));
	memcpy(&strFlashRecord.strRcd, (uint8_t*)pRcdData, RcdLen);
	
	/*保存*/
	PackData((void*)(&strFlashRecord), sizeof(CHARGE_RECORD));
	u32Dest = STODM_CHARGE_RECORD;
	u32Dest += ((index - 1) % CHG_RCD_MAX_NUM) * sizeof(CHARGE_RECORD);
	STO_Write(u32Dest, &strFlashRecord, sizeof(CHARGE_RECORD));
	
	return SET;
}

uint8_t STO_R_ChgRcd(void *pRcdData, uint32_t RcdLen, uint32_t index)
{
	uint32_t u32Dest = 0;
	CHARGE_RECORD strFlashRecord;
	
	if(0 == RcdLen || RcdLen > STO_CHARGE_RCD_SIZE)
		return RESET;
	
	if(index < 1) index = 1;
	
	memset(&strFlashRecord, 0, sizeof(CHARGE_RECORD));
	
	u32Dest = STODM_CHARGE_RECORD;
	u32Dest += (((index - 1) % CHG_RCD_MAX_NUM) * sizeof(CHARGE_RECORD));
	STO_Read(u32Dest, &strFlashRecord, sizeof(CHARGE_RECORD));
	
	if (RESET == CheckDataPack(&strFlashRecord, sizeof(CHARGE_RECORD))) 
	{
		return RESET;
	}
	
	memcpy((U8*)pRcdData, &strFlashRecord.strRcd, RcdLen);
	return SET;
}
/*
u8 STO_R_chgRcd_Cnt(void *pRcdData, u16 RcdLen, u32 index, u32 cnt)
{
	u32 u32Dest = 0;
	CHARGE_RECORD strFlashRecord;
	
	if(0 == RcdLen || RcdLen > FLA_CHARGE_RCD_SIZE)
		return FALSE;
	
	if(index < 1) index = 1;
	
	memset(&strFlashRecord, 0, sizeof(CHARGE_RECORD));
	
	u32Dest = SPIDM_CHARGE_RECORD ;
	u32Dest += (((index - 1) % CHG_RCD_MAX_NUM) * sizeof(CHARGE_RECORD));
	STO_Read(&strFlashRecord, (void *)u32Dest, sizeof(CHARGE_RECORD));
	
	if (FALSE == CheckDataPack(&strFlashRecord, sizeof(CHARGE_RECORD))) 
	{
		return FALSE;
	}
	
	memcpy((u8*)pRcdData, &strFlashRecord.strRcd, RcdLen);
	return TRUE;
}*/

uint8_t STO_W_SampleChgRcd(S_CHARGE_RECORD *pSRecord)
{
	uint32_t u32Dest = 0;
	SAMPLE_RECORD strSamplFlashRecord;

	memset(&strSamplFlashRecord, 0, sizeof(SAMPLE_RECORD));
	memcpy(&strSamplFlashRecord.strSampleRecord, pSRecord, sizeof(S_CHARGE_RECORD));
	
	/*保存*/
	PackData((void*)(&strSamplFlashRecord), sizeof(SAMPLE_RECORD));
	u32Dest = STODM_SAMPLE_RECORD;
	STO_Write(u32Dest, &strSamplFlashRecord, sizeof(SAMPLE_RECORD));
	
	return SET;
}

uint8_t STO_R_SampleChgRcd(S_CHARGE_RECORD *pSRecord)
{
	uint32_t u32Dest = 0;
	SAMPLE_RECORD strSamplFlashRecord;
	
	memset(&strSamplFlashRecord, 0, sizeof(SAMPLE_RECORD));
	
	u32Dest = STODM_SAMPLE_RECORD;
	STO_Read(u32Dest, &strSamplFlashRecord, sizeof(SAMPLE_RECORD));
	
	if (RESET == CheckDataPack(&strSamplFlashRecord, sizeof(SAMPLE_RECORD))) 
	{
//		memset(pSRecord, 0, sizeof(S_CHARGE_RECORD));
		return RESET;
	}
	
	memcpy(pSRecord, &strSamplFlashRecord.strSampleRecord, sizeof(S_CHARGE_RECORD));
	return SET;
}

//存故障信息包，非一条故障
uint8_t STO_W_EveRcdPack(F_EVE_RECORD *pFRecord, uint32_t index, uint16_t cnt)
{
	uint32_t u32Dest = 0;
//	u32 len = 0;
	
	if((NULL == pFRecord) || (0 == cnt))
		return RESET;
	
	/*保存*/
	u32Dest = STODM_EVE_RECORD;
	u32Dest += (index % CHG_RCD_MAX_NUM) * sizeof(F_EVE_RECORD);
	
	STO_Write(u32Dest, pFRecord, sizeof(F_EVE_RECORD)*cnt);
	
	return SET;
}

uint8_t STO_R_EveRcd(EVE_RECORD *pRecord, uint32_t index)
{
	uint32_t u32Dest = 0;
	F_EVE_RECORD strFlashRecord;
	
	if(index < 1) index = 1;
	
	memset(&strFlashRecord, 0, sizeof(F_EVE_RECORD));
	
	u32Dest = STODM_EVE_RECORD;
	u32Dest += (((index - 1) % CHG_RCD_MAX_NUM) * sizeof(F_EVE_RECORD));
	STO_Read(u32Dest, &strFlashRecord, sizeof(F_EVE_RECORD));
	
	if (RESET == CheckDataPack(&strFlashRecord, sizeof(F_EVE_RECORD))) 
	{
		return RESET;
	}
	
	memcpy(pRecord, &strFlashRecord.strErrRcd, sizeof(EVE_RECORD));
	return SET;
}

uint8_t STO_W_SampleEveRcd(S_EVE_RECORD *pSRecord)
{
	uint32_t u32Dest = 0;
	SAMPLE_EVERECORD strSamplFlashRecord;
	
	memset(&strSamplFlashRecord, 0, sizeof(SAMPLE_EVERECORD));
	memcpy(&strSamplFlashRecord.strSampleEveRecord, pSRecord, sizeof(S_EVE_RECORD));
	
	/*保存*/
	PackData((void*)(&strSamplFlashRecord), sizeof(SAMPLE_EVERECORD));
	u32Dest = STODM_SAMPLE_EVERECORD;
	STO_Write(u32Dest, &strSamplFlashRecord, sizeof(SAMPLE_EVERECORD));
	
	return SET;
}

uint8_t STO_R_SampleEveRcd(S_EVE_RECORD *pSRecord)
{
	uint32_t u32Dest = 0;
	SAMPLE_EVERECORD strSamplFlashRecord;
	
	memset(&strSamplFlashRecord, 0, sizeof(SAMPLE_EVERECORD));
	
	u32Dest = STODM_SAMPLE_EVERECORD;
	STO_Read(u32Dest, &strSamplFlashRecord, sizeof(SAMPLE_EVERECORD));
	
	if (RESET == CheckDataPack(&strSamplFlashRecord, sizeof(SAMPLE_EVERECORD))) 
	{
//		memset(pSRecord, 0, sizeof(S_CHARGE_RECORD));
		return RESET;
	}
	
	memcpy(pSRecord, &strSamplFlashRecord.strSampleEveRecord, sizeof(S_EVE_RECORD));
	return SET;
}

uint8_t STO_W_QRPara(uint8_t u8Port, void *pQR, uint16_t len)
{
	uint32_t u32Dest = 0;
	QR_PARA strQRPara;
	
	if(0 == len || len > STO_QR_DATA_SIZE || NULL == pQR)
		return RESET;
	
	memset(&strQRPara, 0, sizeof(QR_PARA));
	memcpy(strQRPara.strQRData.QR_data[u8Port], (uint8_t*)pQR, len);
	
	/*保存*/
	PackData((void*)(&strQRPara), sizeof(QR_PARA));
	u32Dest = STODM_RATE_PARAM;
	STO_Write(u32Dest, &strQRPara, sizeof(QR_PARA));
	
	return SET;
}

uint8_t STO_R_QRPara(uint8_t u8Port, void *pQR, uint16_t len)
{
	uint32_t u32Dest = 0;
	QR_PARA strQRPara;
	
	if(0 == len || len > STO_QR_DATA_SIZE || NULL == pQR)
		return RESET;
	
	memset(&strQRPara, 0, sizeof(QR_PARA));
	
	u32Dest = STODM_RATE_PARAM;
	STO_Read(u32Dest, (void*)(&strQRPara), sizeof(QR_PARA));
	
	if (RESET == CheckDataPack(&strQRPara, sizeof(QR_PARA))) 
	{
		return RESET;
	}
	
	memcpy((uint8_t*)pQR, strQRPara.strQRData.QR_data[u8Port], len);
	
	return SET;
}

uint8_t STO_W_SystemParam(SYSTEM_PARAM *pSysPara)
{
	uint32_t u32Dest = 0;
	F_SYSTEM_PARAM strFSysPara;

	memset(&strFSysPara, 0, sizeof(F_SYSTEM_PARAM));
	memcpy(&strFSysPara.strSysPara, pSysPara, sizeof(SYSTEM_PARAM));
	
	/*保存*/
	PackData((void*)(&strFSysPara), sizeof(F_SYSTEM_PARAM));
	u32Dest = STODM_SYSTEMPARA;
	STO_Write(u32Dest, &strFSysPara, sizeof(F_SYSTEM_PARAM));

	return SET;
}

uint8_t STO_R_SystemParam(SYSTEM_PARAM *pSysPara)
{
	uint32_t u32Dest = 0;
	F_SYSTEM_PARAM strFSysePara;

	memset(&strFSysePara, 0, sizeof(F_SYSTEM_PARAM));
	
	u32Dest = STODM_SYSTEMPARA;
	STO_Read(u32Dest, &strFSysePara, sizeof(F_SYSTEM_PARAM));
	
	if (RESET == CheckDataPack(&strFSysePara, sizeof(F_SYSTEM_PARAM))) 
	{
		return RESET;
	}
	
	memcpy(pSysPara, &strFSysePara.strSysPara, sizeof(SYSTEM_PARAM));
	return SET;
}

uint8_t STO_W_ChargeParam(CHARGE_PARAM *pChgPara)
{
	uint32_t u32Dest = 0;
	F_CHARGE_PARAM strFChargePara;

	memset(&strFChargePara, 0, sizeof(F_CHARGE_PARAM));
	memcpy(&strFChargePara.strChargePara, pChgPara, sizeof(CHARGE_PARAM));
	
	/*保存*/
	PackData((void*)(&strFChargePara), sizeof(F_CHARGE_PARAM));
	u32Dest = STODM_CHARGE_PARAM;
	STO_Write(u32Dest, &strFChargePara, sizeof(F_CHARGE_PARAM));

	return SET;
}

uint8_t STO_R_ChargeParam(CHARGE_PARAM *pChgPara)
{
	uint32_t u32Dest = 0;
	F_CHARGE_PARAM strFChargePara;

	memset(&strFChargePara, 0, sizeof(F_CHARGE_PARAM));
	
	u32Dest = STODM_CHARGE_PARAM;
	STO_Read(u32Dest, &strFChargePara, sizeof(F_CHARGE_PARAM));

	if (RESET == CheckDataPack(&strFChargePara, sizeof(F_CHARGE_PARAM))) 
	{
		return RESET;
	}
	
	memcpy(pChgPara, &strFChargePara.strChargePara, sizeof(CHARGE_PARAM));
	return SET;
}


//升级参数只用了一个字节没有申请全局变量,以后参数定义多了可以申请全局变量
uint8_t STO_W_Updata(UPDATA_PARA *pUpdataPara)
{
	uint32_t u32Dest = 0;
	F_UPDATA_PARA strFUpdataPara;
	
	memset(&strFUpdataPara, 0, sizeof(F_UPDATA_PARA));
	memcpy(&strFUpdataPara.strUpdataPara, pUpdataPara, sizeof(UPDATA_PARA));
	
	/*保存*/
	PackData((void*)(&strFUpdataPara), sizeof(F_UPDATA_PARA));
	u32Dest = STODM_UPDATA_PARA;
	STO_Write(u32Dest, &strFUpdataPara, sizeof(F_UPDATA_PARA));
	
	return SET;
}

uint8_t STO_R_Updata(UPDATA_PARA *pUpdataPara)
{
	uint32_t u32Dest = 0;
	F_UPDATA_PARA strFUpdataPara;
	
	memset(&strFUpdataPara, 0, sizeof(F_UPDATA_PARA));
	
	u32Dest = STODM_UPDATA_PARA ;
	STO_Read(u32Dest, &strFUpdataPara, sizeof(F_UPDATA_PARA));
	
	if (RESET == CheckDataPack(&strFUpdataPara, sizeof(F_UPDATA_PARA))) 
	{
		return RESET;
	}
	
	memcpy(pUpdataPara, &strFUpdataPara.strUpdataPara, sizeof(UPDATA_PARA));
	return SET;
}


static uint8_t STO_Poweron_Check(void)
{
	uint8_t flag = SET;
	
	//参数
	if((STO_UPDATA_PARA_LEN != sizeof(UPDATA_PARA))
		|| (STO_SYSTEM_PARA_LEN != sizeof(SYSTEM_PARAM))
		|| (STO_CHARGE_PARA_LEN != sizeof(CHARGE_PARAM)))
	{
		flag = RESET;
	}
	
	//升级地址大于用户数据地址
	if(STODM_END >= STO_USER_BACKUP_DATA_ADDR)
	{
		flag = RESET;
	}
	
	if(SECTOR_SIZE != STODM_SEC_BLOCK)
	{
		flag = RESET;
	}
	
	if(SECTOR_SIZE*2 != STODM_FIRST_SEC_ADDR)
	{
		flag = RESET;
	}
	
	return flag;
}



void STO_EraseBillFlash(void)
{
	for (uint32_t i=BASE_INFO_START_ADDR; i < 0xfffff; i = i+0x1000) {
        W25QXX_Erase_Sector(i);
    }
}



//========================================================

