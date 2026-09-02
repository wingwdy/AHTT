#include "AppInputCfg.h"
#include "maths.h"
#include "stdint.h"
#include <stdio.h>
#include "SIM900A.h"
#include "AppStorage.h"
#include "mbsDataUpdate.h"
#include "mbsMaster.h"
#include "screenUart.h"
#include "DisQRCode.h"
#include "AppMidDataTrans.h"
#include "iot_YKC_Protocol_CodeV2_1.h"
#include "AppDealFlash.h"

inputCmdBuf g_inBuf;
inputCmdBuf *pst_inBuf = &g_inBuf;


#define InputCfgPintf(fmt,args...)	\
		do {								\
            debug(fmt ,##args); 	\
            debug("\r\n");      			\
		} while(0)




PlatCfgInfo *pst_cfgInfo = &g_pltCfgInfo;


typedef struct _platCardItem {
	const char pltChar[8];
	PLAT_TYPE pltType;
	CARD_TYPE cardType;
} platCardItem;

//需要按pltType顺序排列
static platCardItem platCardItem_Map[] =
{
	{"gn",            ePlatType_GN,			    CARD_GN},
	{"gn+",           ePlatType_GNP,		    CARD_GN},
	{"ykc",           ePlatType_YKC, 		    CARD_YKC},
	{"ykc2.1",        ePlatType_YKC_V2, 	    CARD_YKC},
	{"gwykc",         ePlatType_gwYKC, 		    CARD_YKC},//JJUNIVE 国网e充电平台
	{"anpei",  		  ePlatType_ANPEI,          CARD_GN},//JJUNIVE
	{"tt",            ePlatType_TOWER, 		    CARD_GN},
	{"dd",            ePlatType_DD, 		    CARD_GN},
	{"dky",    		  ePlatType_GNP,	        CARD_DKY},
	{"ahtt",          ePlatType_AHTT, 		    CARD_AHTT},//JXY
	{"hnct",          ePlatType_HaiNCT, 		CARD_HNCT},//MXY
	{"wjy",           ePlatType_WJY, 			CARD_GN},  //WDY
    
	{"js",			  ePlatType_JSIOT,		    CARD_GN},		// xw 202409
	{"ahchg",         ePlatType_AHCHG,	   	    CARD_GN},
	{"xjb",           ePlatType_XJB, 	   	    CARD_GN},
	{"nfdw",          ePlatType_NFDW, 	   	    CARD_GN},
	{"xxcd",          ePlatType_XXCD, 	   	    CARD_GN},
	{"ptxny",         ePlatType_PTXNY, 	   	    CARD_GN},
	{"fyjg",          ePlatType_FYJG,	   	    CARD_GN},
	{"ahllcd",        ePlatType_AHLLCD,	   	    CARD_GN},
	{"bsc",           ePlatType_BSC, 	   	    CARD_GN},
};

void CmdHandle_SetPlat(char *pstr, const char *cmd)
{
    char format[32];
    char platStr[8];
    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, platStr);

	platCardItem *pt_platCard = NULL;

	for (int i = 0; i < ARRAY_SIZE(platCardItem_Map); i++) {
		pt_platCard = &platCardItem_Map[i];
		
		if (0 == strncasecmp(platStr, pt_platCard->pltChar, sizeof(pt_platCard->pltChar) - 1)) {
            
            if (pst_cfgInfo->PltMainType != pt_platCard->pltType) {
                Clear_EEOP_Param(); //清除平台特有flash中的数据，避免数据混用
				DealData_Clear(); // 清除交易记录				
            }

    		sprintf(format, "\r\nok! set plat %s\r\n", pt_platCard->pltChar);
            Set_PlatType(pt_platCard->pltType);
			printf("%s", format);
			break;
		} 
	}
}
void CmdHandle_SetCard(char *pstr, const char *cmd)
{
    char format[32];
    char platStr[8];
    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, platStr);

	platCardItem *pt_platCard = NULL;

	for (int i = 0; i < ARRAY_SIZE(platCardItem_Map); i++) {
		pt_platCard = &platCardItem_Map[i];
		
		if (0 == strncasecmp(platStr, pt_platCard->pltChar, sizeof(pt_platCard->pltChar) - 1)) {
			pst_cfgInfo->PltMainCardType = pt_platCard->cardType;
    		sprintf(format, "\r\nok! set card %s\r\n", pt_platCard->pltChar);
			printf("%s", format);
            
            PlatDevNumberChange(1);
			break;
		} 
	}
}


static void SetDnString(char *dest, char *src, uint8_t destMaxLen)
{
    //将src桩号赋值给dest
    uint8_t destlen = strlen(dest);
    uint8_t srclen = strlen(src);

    uint8_t cpyLen = (srclen >= (destMaxLen - 1)) ? (destMaxLen-1) : srclen;

    if ((destlen <= 4) && (srclen > 4)) {
        strncpy(dest, src, cpyLen);
		printf("SetDnString cpy finish. %d %d  %d %d\r\n", destlen, srclen, destMaxLen, cpyLen);
    }
}
void CmdHandle_SetOdn(char *pstr, const char *cmd)
{
    char format[64];
	uint8_t len = strlen(pstr) - strlen(cmd);
	if (len >= 30) {
		printf("cmd erro. device number is too long\r\n");
		return;
	}
    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, pst_cfgInfo->fixDeviceNumber);
	// sscanf(pstr, format, pst_cfgInfo->pltDeviceNumber);
	
    SetDnString(pst_cfgInfo->pltDeviceNumber, pst_cfgInfo->fixDeviceNumber, PLAT_NUMBER_LEN);

	printf("ok! odn = %s\r\n", pst_cfgInfo->fixDeviceNumber);
	printf("ok! dn = %s\r\n", pst_cfgInfo->pltDeviceNumber);

    PlatDevNumberChange(1);
    PlatDevNumberChange(2);
}

uint8_t PlatDNCheck(uint8_t len)
{
    if ((pst_cfgInfo->PltMainType == ePlatType_GN)
      ||(pst_cfgInfo->PltMainType == ePlatType_GNP)) {
        if (len > 14) {
            printf("cmd erro. len = %d. GN dn is too long\r\n", len);
            return 1;
        }
    }
    return 0;
}
void CmdHandle_SetDn(char *pstr, const char *cmd)
{
    char format[64];
	uint8_t len = strlen(pstr) - strlen(cmd);
	if (len > 30) {
		printf("cmd erro. len = %d dn is too long\r\n", len);
		return;
	}
    //校验异常
    if (PlatDNCheck(len)) {
        return;
    }
    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, pst_cfgInfo->pltDeviceNumber);
	printf("ok! dn = %s\r\n", pst_cfgInfo->pltDeviceNumber);

    SetDnString(pst_cfgInfo->fixDeviceNumber, pst_cfgInfo->pltDeviceNumber, FIX_NUMBER_LEN);

    PlatDevNumberChange(2);
}

int countCharOccurrences(char *str, char ch) {
    int count = 0;
    while (*str) {
        if (*str == ch) {
            count++;
        }
        str++;
    }
    return count;
}

void ipOrDomainChoose(char *substring)
{
	//set para ip:evse.gongniu.cn 15455
	//set para ip:evse.gongniu.cn,15455
	//支持以上两种格式
	
	int cntComma = countCharOccurrences(substring, ',');
	int cntSpace = countCharOccurrences(substring, ' ');
	if ((cntComma != 1) && (cntSpace != 1)) {
		printf("The command parsing failure...\r\n");
		printf("> ");
		return;
	}

	char *foundComma = strchr(substring, ',');
	if (foundComma != NULL) {
        int index = foundComma - substring; // 计算分隔符在原字符串中的索引位置
        substring[index] = ' ';
	}
}
void CmdHandle_SetMainIp(char *pstr, const char *cmd)
{
	//*pstr为输入字符串，*cmd为前缀部分字符串
	char *substring = pstr + strlen(cmd);

	ipOrDomainChoose(substring);

    // sprintf(format, "%%s %%d.", cmd);
	sscanf(substring, "%s %hd", pst_cfgInfo->PltMainIp, &pst_cfgInfo->PltMainPort);
	printf("ok! MainIp = %s, MainPort = %d\r\n", pst_cfgInfo->PltMainIp, pst_cfgInfo->PltMainPort);
    
    PlatDevNumberChange(1);
}
void CmdHandle_SetMainPort(char *pstr, const char *cmd)
{
    char format[32];
    sprintf(format, "%s%%hd", cmd);
	sscanf(pstr, format, &pst_cfgInfo->PltMainPort);
	printf("ok! MainPort = %d\r\n", pst_cfgInfo->PltMainPort);
}


void CmdHandle_SetMntrIp(char *pstr, const char *cmd)
{
	//*pstr为输入字符串，*cmd为前缀部分字符串
	char *substring = pstr + strlen(cmd);
	ipOrDomainChoose(substring);

	sscanf(substring, "%s %hd", pst_cfgInfo->PltAuxiliaryIp, &pst_cfgInfo->PltAuxiliaryPort);
	printf("ok! MonitorIP = %s, MonitorPort = %d\r\n", pst_cfgInfo->PltAuxiliaryIp, pst_cfgInfo->PltAuxiliaryPort);
}
void CmdHandle_SetMntrPort(char *pstr, const char *cmd)
{
    char format[32];
    sprintf(format, "%s%%d", cmd);
	sscanf(pstr, format, &pst_cfgInfo->PltAuxiliaryPort);
	printf("ok! MonitorPort = %d\r\n", pst_cfgInfo->PltAuxiliaryPort);
}

void CmdHandle_SetQrcode(char *pstr, const char *cmd)
{
    char qrcodeStr[200];
    char format[32];
    sprintf(format, "%s%%s", cmd);
	sscanf(pstr, format, qrcodeStr);
	printf("ok! qrcode = %s\r\n", qrcodeStr);
    //设置
    storage_qrCodeInfoStr(qrcodeStr);
}
void CmdHandle_SetTime(char *pstr, const char *cmd)
{
    char format[32];
    sprintf(format, "%s%%d", cmd);
    uint32_t l_time = 0;
	sscanf(pstr, format, &l_time);

	printf("ok! time = %d\r\n", l_time);

	setCurrentRunTimeStamp(l_time);
}

void CmdHandle_TestMode(char *pstr, const char *cmd)
{
    //底板需要进入厂内模式
    fgv_SetPileCfgFac(1);
	printf("ok! test mode\r\n");
}
void CmdHandle_QrcodeInit(char *pstr, const char *cmd)
{
    //二维码更新
    Update_qrCodeInfo();
	printf("ok! qrcode init\r\n");
}

//云快充2.1 rsa公钥设置
void CmdHandle_SetRsaKey(char *pstr, const char *cmd)
{
    char format[32] = {0};
    char key[128 + 1] = { 0 };
	uint8_t len = strlen(pstr) - strlen(cmd);
	if (len > 128) {	//rsa key长度128
		printf("CmdHandle_SetRsaKey erro. len = %d RsaKey is too long\r\n", len);
		return;
	}

    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, key);		
	YKC21_WriteRsaKey(&key[0]);	
	printf("ok! rsa key = %s\r\n", key);
}

//云快充2.1 token设置
void CmdHandle_SetToken(char *pstr, const char *cmd)
{
    char format[32];
    char token[14];
	uint8_t len = strlen(pstr) - strlen(cmd);
	if (len > 14) {		//token 长度14
		printf("CmdHandle_SetToken erro. len = %d token is too long\r\n", len);
		return;
	}

    sprintf(format, "%s %%s", cmd);
	sscanf(pstr, format, token);			
	YKC21_WriteToken(&token[0]);	
	printf("ok! token = %s\r\n", token);
}
void Get_CurrentPlatTypeName(char *name)
{
    if (pst_cfgInfo->PltMainType < ARRAY_SIZE(platCardItem_Map)) {
        //避免表格顺序不一致问题
	    platCardItem *pt_platCard = NULL;
        uint8_t indexPlat = 0;
        for (int i = 0; i < ARRAY_SIZE(platCardItem_Map); i++) {
            pt_platCard = &platCardItem_Map[i];
            if (pt_platCard->pltType == pst_cfgInfo->PltMainType) {
                indexPlat = i;
                break;
            }
        }
        memcpy(name, platCardItem_Map[indexPlat].pltChar, strlen(platCardItem_Map[indexPlat].pltChar));
	} else {
		InputCfgPintf("Info----plat Type: out of range. %d", pst_cfgInfo->PltMainType);  
    }
}
void Get_CurrentCardTypeName(char *name)
{
    if (pst_cfgInfo->PltMainCardType < ARRAY_SIZE(platCardItem_Map)) {
        //避免表格顺序不一致问题
	    platCardItem *pt_platCard = NULL;
        uint8_t indexPlat = 0;
        for (int i = 0; i < ARRAY_SIZE(platCardItem_Map); i++) {
            pt_platCard = &platCardItem_Map[i];
            if (pt_platCard->cardType == pst_cfgInfo->PltMainCardType) {
                indexPlat = i;
                break;
            }
        }
        memcpy(name, platCardItem_Map[indexPlat].pltChar, strlen(platCardItem_Map[indexPlat].pltChar));
	} else {
		InputCfgPintf("Info----Card Type: out of range. %d", pst_cfgInfo->PltMainCardType);  

    }
}

void CmdHandle_PrintfAll()
{
	static GN_PLATMOD  *InputMod = &sg_platmod;	//为了获取A板版本

	InputCfgPintf("\r\n"); 
	
	InputCfgPintf("Info----A Software Version: %d.%d.%d.%d", InputMod->pileInf.soft_ver[0], InputMod->pileInf.soft_ver[1], InputMod->pileInf.soft_ver[2], InputMod->pileInf.soft_ver[3]); 
	InputCfgPintf("Info----A Hardware Version: %d.%d.%d.%d", InputMod->pileInf.hw_version[0], InputMod->pileInf.hw_version[1], InputMod->pileInf.hw_version[2], InputMod->pileInf.hw_version[3]); 
	InputCfgPintf("Info----A Protocal Version: %d.%d.%d.%d\r\n", InputMod->pileInf.protocol_ver[0], InputMod->pileInf.protocol_ver[1], InputMod->pileInf.protocol_ver[2], InputMod->pileInf.protocol_ver[3]); 

	InputCfgPintf("Info----B Software Version: %s", SOFTWARE_VERSION); 
	InputCfgPintf("Info----B Hardware Version: %s", HARDWARE_VERSION); 
	InputCfgPintf("Info----B Mbs Protocal Version: %s", MBSPROTOCOL_VERSION);
	InputCfgPintf("Info----B Protocal Version: %s\r\n", PROTOCOL_VERSION); 
	
	InputCfgPintf("Info----Pile sole dn: %s", pst_cfgInfo->fixDeviceNumber); 
	InputCfgPintf("Info----Plat Dn: %s", pst_cfgInfo->pltDeviceNumber); 
	InputCfgPintf("Info----Auxiliary Ip: %s", pst_cfgInfo->PltAuxiliaryIp); 
	InputCfgPintf("Info----Auxiliary Port: %d", pst_cfgInfo->PltAuxiliaryPort); 
	InputCfgPintf("Info----PltMainIp: %s", pst_cfgInfo->PltMainIp); 
	InputCfgPintf("Info----PltMainPort: %d", pst_cfgInfo->PltMainPort); 

    char platName[16] = {0};
    Get_CurrentPlatTypeName(platName);
	InputCfgPintf("Info----Plat Type: %s", platName); 
    memset(platName, 0, sizeof(platName));
    Get_CurrentCardTypeName(platName);
	InputCfgPintf("Info----Card Type: %s", platName); 

	InputCfgPintf("\r\n"); 
}


void CmdHandle_Else(char *pstr)
{
	if (0 == strcasecmp(pstr, "get all")) {
		CmdHandle_PrintfAll();
	}
	else if (0 == strcasecmp(pstr, "get dn")) {
		printf("\r\nok! dn:%s.\r\n", pst_cfgInfo->pltDeviceNumber); 
	}
	else if (0 == strcasecmp(pstr, "get odn")) {

		printf("\r\nok! dn:%s.\r\n", pst_cfgInfo->fixDeviceNumber); 
	}
	else if (0 == strcasecmp(pstr, "reboot a")) {
		Reboot_System(1);
	}
	else if (0 == strcasecmp(pstr, "reboot b")) {
		Reboot_System(2);
	}
	else if (0 == strcasecmp(pstr, "reboot")) {
		Reboot_System(0);
	}
	else if (0 == strcasecmp(pstr, "reset charge")) {
		fgv_CtrlPileOpr(E_DEV_CTRL_CMD_FACRST);
		printf("\r\nok! reset charge\r\n"); 
	}
	else if (0 == strncasecmp(pstr, "ef ", sizeof("ef ") - 1)) {
		U8 t = 3;
		sscanf(pstr, "ef %hhd",&t);
		printf("\r\nef %d  finish!\r\n", t);
	} 
	else if (0 == strcasecmp(pstr, "update a")) {
		printf("\r\nok! update a start \r\n");
		
		char destFilename[16] = {0};
		strcat(destFilename, FTP_FILENAME);
		strcat(destFilename, "_A");
		SIM900FtpInfoSetip(FTP_USER_IP, FTP_USER_PORT);
		SIM900FtpInfoSetUserName(FTP_USER_NAME, FTP_USER_PSW);
		SIM900FtpInfoSetPath(FTP_FILDER_PATH,  destFilename);

		OtaStart_tcp();
		OtaStart_UpdateObj(eUpdateObj_A);
	}
	else if (0 == strcasecmp(pstr, "update b")) {
		printf("\r\nok! update b start \r\n");
		
		char destFilename[16] = {0};
		strcat(destFilename, FTP_FILENAME);
		strcat(destFilename, "_B");
		SIM900FtpInfoSetip(FTP_USER_IP, FTP_USER_PORT);
		SIM900FtpInfoSetUserName(FTP_USER_NAME, FTP_USER_PSW);
		SIM900FtpInfoSetPath(FTP_FILDER_PATH,  destFilename);

		OtaStart_tcp();
		OtaStart_UpdateObj(eUpdateObj_B);
	}
	else if (0 == strcasecmp(pstr, "update c")) {
		printf("\r\nok! update c start \r\n");

		char destFilename[16] = {0};
		strcat(destFilename, FTP_FILENAME);
		strcat(destFilename, "_C");
		SIM900FtpInfoSetip(FTP_USER_IP, FTP_USER_PORT);
		SIM900FtpInfoSetUserName(FTP_USER_NAME, FTP_USER_PSW);
		SIM900FtpInfoSetPath(FTP_FILDER_PATH,  destFilename);

		OtaStart_tcp();
		OtaStart_UpdateObj(eUpdateObj_C);
	}
	else if (0 == strncasecmp(pstr, "printf ", sizeof("printf ") - 1)) {
		U8 t = 0;
		sscanf(pstr, "printf %hhd",&t);
		hex_dump_printf(t);
		printf("\r\nprintf %d !!\r\n", t);
	}
    else if (0 == strncasecmp(pstr, "log ", sizeof("log ") - 1)) {
		U8 t = 0;
		sscanf(pstr, "log %hhd",&t);
		log_printf(t);
		printf("\r\nlog %d !!\r\n", t);
	}
	else if (0 == strncasecmp(pstr, "load on ", sizeof("load on ") - 1)) {
		U16 t = 0;
		sscanf(pstr, "load on %hd",&t);
		printf("\r\nok! load on %d\r\n", t);
	}
	else if (0 == strncasecmp(pstr, "start", sizeof("start") - 1)) {
		U8 t = 0;
		sscanf(pstr, "start %hhd",&t);
		printf("\r\nok! gun %d start charging...\r\n", t);
		fgv_CtrlStartCharge(t);
	}
	else if(0 == strncasecmp(pstr, "resetbillflash", sizeof("resetbillflash") - 1)){
        STO_EraseBillFlash();
		printf("\r\nok! resetbillflash ...\r\n");
	}
	else {
		printf("The command parsing failure...\r\n");
		printf("> ");
	}
}


typedef void (*CmdHandle)(char *pstr, const char *cmd);
typedef void (*CmdExecute)(void);

typedef struct _InputItem {
	const char inputChar[32];				//串口输入信息
	CmdHandle FHandle;				//串口输入信息处理
	CmdExecute FExecute;			//串口输入信息处理完成后执行
} InputItem;

static InputItem InputItem_Map[] =
{
	{"set odn ",            	CmdHandle_SetOdn,			NULL		},
	{"set dn ",            		CmdHandle_SetDn,			NULL		},
	{"set plat ",            	CmdHandle_SetPlat, 			NULL		},
	{"set card ",            	CmdHandle_SetCard, 			NULL		},
	{"set para ip:",            CmdHandle_SetMainIp,		NULL		},
	{"set para domain:",        CmdHandle_SetMainIp,		NULL		},
	{"set para port:",        	CmdHandle_SetMainPort,		NULL		},
	{"set mntr ip:",        	CmdHandle_SetMntrIp,		NULL		},
	{"set mntr port:",       	CmdHandle_SetMntrPort,		NULL		},
	{"set qrcode:",        	    CmdHandle_SetQrcode,		NULL		},
	{"set rsa key:",        	CmdHandle_SetRsaKey,		NULL		},
	{"set token:",        		CmdHandle_SetToken,			NULL		},
    {"set time ",            	CmdHandle_SetTime,			NULL		},
	{"test mode",            	CmdHandle_TestMode,			NULL		},
	{"qrcodeinit",            	CmdHandle_QrcodeInit, 		NULL		},
};

static void InputItemCmdMain(char *buf)
{
	InputItem *pt_input = NULL;

	char *pstr = buf;

	for (int i = 0; i < ARRAY_SIZE(InputItem_Map); i++) {
		pt_input = &InputItem_Map[i];

		if (0 != strncasecmp(pstr, pt_input->inputChar, strlen(pt_input->inputChar) - 1)) {
			continue;
		}
		//找到相同字符，进行命令处理
		if (pt_input->FHandle != NULL) {
			pt_input->FHandle(pstr, pt_input->inputChar);
		}
		if (pt_input->FExecute != NULL) {
			pt_input->FExecute();
		}
		Set_platParam(pst_cfgInfo);
		return;
	}
	CmdHandle_Else(pstr);
}


void deletInputCmdChar()
{
	if (pst_inBuf->u8_InputCnt) {
		pst_inBuf->u8_InputCmdBuf[pst_inBuf->u8_InputCnt] = 0;
		pst_inBuf->u8_InputCnt = pst_inBuf->u8_InputCnt - 1;;
	}
}
void pushInputCmd(uint8_t *inBuf, uint16_t len)
{
	if (len > INPUTBUF_LEN) {
		pst_inBuf->u8_InputCnt = 0;
		return;
	}
	for (int i = 0; i < len; i++) {
		switch (inBuf[i]) {
			case '\r' :
			case '\n' :		/* Enter */
				printf("\r\n");
				InputItemCmdMain((char *)pst_inBuf->u8_InputCmdBuf);
				pst_inBuf->u8_InputCnt = 0;
				memset((char *)pst_inBuf->u8_InputCmdBuf, '\0', sizeof(pst_inBuf->u8_InputCmdBuf));
				continue;
			case 0x08 :	/* ^H - backspace */
				printf("\b \b");	/* backspace and delete */
				deletInputCmdChar();
				continue;
			default:
				printf("%c", inBuf[i]);
				break;	
		}

		pst_inBuf->u8_InputCmdBuf[pst_inBuf->u8_InputCnt] = inBuf[i];
		pst_inBuf->u8_InputCnt = pst_inBuf->u8_InputCnt + 1;
	}
	// memcpy(pst_inBuf->u8_InputCmdBuf[pst_inBuf->u8_InputCnt], inBuf, len);
	// pst_inBuf->u8_InputCnt = pst_inBuf->u8_InputCnt + len;
}


// void fetchInputCmd(uint8_t *outBuf, uint16_t len)
// {
// 	int len2 = pst_inBuf->u8_OutputCnt + len - INPUTBUF_LEN;
// 	if (len2 > 0) {
// 		memcpy(outBuf, pst_inBuf->u8_InputCmdBuf[pst_inBuf->u8_OutputCnt], INPUTBUF_LEN - pst_inBuf->u8_OutputCnt);
// 		pst_inBuf->u8_OutputCnt = 0;
// 	}
// 	memcpy(outBuf, pst_inBuf->u8_InputCmdBuf[pst_inBuf->u8_OutputCnt], len2);
// 	pst_inBuf->u8_OutputCnt = pst_inBuf->u8_OutputCnt + len2;
// }
