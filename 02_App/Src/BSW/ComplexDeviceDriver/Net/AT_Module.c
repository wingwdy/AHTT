/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "AT_Module.h"
#include "Cdd_Drv_EG800AK.h"
#include "AT_TCP.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define AT_MODULE_APN_YD              "CMIOT"
#define AT_MODULE_APN_LT              "3GNET"
#define AT_MODULE_APN_DX              "CTNET"

/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t ATModule_PackConfigAPN(uint8_t socketIndex, void * modulePara, uint8_t *pData, uint16_t nATLen);

static uint8_t ATModule_RecvSimStatus(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvCGREG(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvIccid(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvCSQ(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvCOPS(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvOKACK(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvPDPState(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen);

static void ATModule_FailHandle(uint8_t socketID, void * modulePara, uint8_t atTaskID);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stModuleATCmdDescribtor[] =
{
    [eATModuleCmd_QueryModule] =
    { "ATI\r\n",                               "ATI",             3,          5000,     3000,  TRUE, "识别模块",
        NULL,                                   NULL,                         ATModule_FailHandle},

    [eATModuleCmd_SetSimStatusReportEnable] =  
    { "AT+QSIMSTAT=1\r\n",                     "AT+QSIMSTAT=1",   3,          5000,     3000,  TRUE, "sim卡状态上报使能",
        NULL,                                   NULL,                         NULL},

    [eATModuleCmd_QuerySimStatus] =  
    { "AT+QSIMSTAT?\r\n",                      "+QSIMSTAT:",      3,          5000,     3000,  TRUE, "sim卡状态查询",
        NULL,                                   ATModule_RecvSimStatus,       NULL},

    [eATModuleCmd_QuerySimRecognizeStatus] =  
    { "AT+CPIN?\r\n",                          "+CPIN: READY",    10,          3000,     1000,  TRUE, "sim识别状态查询",
    NULL,                                      NULL,                          NULL},

    [eATModuleCmd_QueryIccid] =  
    { "AT+QCCID\r\n",                          "+QCCID: ",         3,          10000,    3000,  TRUE, "sim卡iccid查询",
    NULL,                                      ATModule_RecvIccid,            ATModule_FailHandle},

    [eATModuleCmd_QueryCsq] =  
    { "AT+CSQ\r\n",                            "+CSQ:",           3,          5000,     3000,  TRUE, "查询信号强度",
    NULL,                                      ATModule_RecvCSQ,              NULL},

    [eATModuleCmd_QueryNtpClk] =  
    { "AT+QNTP=1,\"ntp1.aliyun.com\"\r\n",    "+QNTP:",           3,          5000,     3000,  TRUE, "查询NTP时间",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryCGREG] =  
    { "AT+CGREG?\r\n",                        "+CGREG:",          3,          10000,    3000,  TRUE, "PS服务网络连接状态查询",
    NULL,                                      ATModule_RecvCGREG,            ATModule_FailHandle},

    [eATModuleCmd_QueryCOPS] =  
    { "AT+COPS?\r\n",                         "+COPS:",           10,         3000,     3000,  TRUE, "查询运营商",
    NULL,                                      ATModule_RecvCOPS,             ATModule_FailHandle},

    [eATModuleCmd_QueryNetWorkInfo] =  
    { "AT+QNWINFO\r\n",                        "+QNWINFO:",       3,          10000,    3000,  TRUE, "查询网络信息",
    NULL,                                      NULL,                          ATModule_FailHandle},

    [eATModuleCmd_ConfigAPN] =  
    { "AT+QICSGP=1,1,\"[APN]\",\"\",\"\",0\r\n", "+QICSGP",       3,          10000,    3000,  TRUE, "配置APN",
    ATModule_PackConfigAPN,                    ATModule_RecvOKACK,            ATModule_FailHandle},

    [eATModuleCmd_ActivePDP] =  
    { "AT+QIACT=1\r\n",                        "+QIACT",          3,          10000,    3000,  TRUE, "激活PDP",
    NULL,                                      NULL,                          ATModule_FailHandle},
    
    [eATModuleCmd_QueryPDPState] =  
    { "AT+QIACT?\r\n",                         "+QIACT",          3,          10000,    3000,  TRUE, "查询PDP状态",
    NULL,                                      ATModule_RecvPDPState,         ATModule_FailHandle},

    [eATModuleCmd_SetCFUN0] =  
    { "AT+CFUN=0\r\n",                         "+CFUN=0",         3,          10000,    3000,  TRUE, "设置最小功能模式",
    NULL,                                      NULL,                ATModule_FailHandle},

    [eATModuleCmd_SetCFUN1] =  
    { "AT+CFUN=1\r\n",                         "+CFUN=1",         3,          10000,    3000,  TRUE, "设置全功能模式",
    NULL,                                      NULL,                ATModule_FailHandle},
};

const ATUrcDescribtor_Struct c_stATUrcDescribtor[5] =
{
    [0] = { "+QIOPEN:",             ATTCP_UrcQIPOpen,    TRUE,    "建立连接"},
    [1] = { "SEND OK",              ATTCP_UrcSendOK,     FALSE,   "数据发送成功"},
    [2] = { "+QNTP:",               NULL,                TRUE,    "网络时间同步"},
    [3] = { "+QIURC: \"closed\"",   ATTCP_UrcClose,      TRUE,    "断开连接"},
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint16_t ATModule_PackConfigAPN(uint8_t socketIndex, void * modulePara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
	char cApn[24] = {0};

	//替代APN字符
	if(CddDrvSimOperator_CMCC == pModulePara->stModuleInfo.eOperatorType)
	{
		memcpy(cApn, AT_MODULE_APN_YD, strlen(AT_MODULE_APN_YD));
	}
	else if(CddDrvSimOperator_CUCC == pModulePara->stModuleInfo.eOperatorType)
	{
		memcpy(cApn, AT_MODULE_APN_LT, strlen(AT_MODULE_APN_LT));
	}
	else if(CddDrvSimOperator_CTCC == pModulePara->stModuleInfo.eOperatorType)
	{
		memcpy(cApn, AT_MODULE_APN_DX, strlen(AT_MODULE_APN_DX));
	}
    else
    {}

	nATLen = Common_ReplaceStr(pData, nATLen, "[APN]", cApn, strlen(cApn), "CMNET");
	return nATLen;
}

static void ATModule_FailHandle(uint8_t socketID, void * modulePara, uint8_t atTaskID)
{
    if (atTaskID >= eATModuleCmd_QueryCGREG && atTaskID < eATModuleCmd_QueryCount)
    {
        CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
        CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_CFun);
    }
    else
    {
        CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
        CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Reboot);
    }
}

static uint8_t ATModule_RecvSimStatus(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
    uint8_t *pTemp = NULL;
    int32_t status = 0;
    uint8_t ret = FALSE;

    sscanf((char*)pData, ",%d/r", &status);

    if (status == 1)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t ATModule_RecvIccid(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    uint8_t ret = FALSE;
    uint8_t minLen = CDDDRV_EG800AK_CFG_ICCID_LEN + strlen("+QCCID: ") + 2;
    uint8_t *pStart = NULL;

    if (dataLen >= minLen && pModulePara != NULL)
    {
        pStart = Common_SearchData(pData, dataLen, "+QCCID: ", strlen("+QCCID: "));

        if (pStart != NULL)
        {
            pStart += strlen("+QCCID: ");

            if (pStart[CDDDRV_EG800AK_CFG_ICCID_LEN] == '\r')
            {
                memcpy(pModulePara->stModuleInfo.iccid, pStart, CDDDRV_EG800AK_CFG_ICCID_LEN);
                ret = TRUE;
            }
        }
    }

    return ret;
}

static uint8_t ATModule_RecvCSQ(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    uint8_t ret = FALSE;
    uint8_t minLen = + strlen("+CSQ: ");
    uint8_t *pStart = NULL;
    int32_t csq = 0;

    if (dataLen >= minLen && pModulePara != NULL)
    {
        pStart = Common_SearchData(pData, dataLen, "+CSQ: ", strlen("+CSQ: "));

        if (pStart != NULL)
        {
            sscanf((char*)pStart, "+CSQ: %d,", &csq);
            pModulePara->stModuleInfo.csq = csq;
            ret = TRUE;
        }
    }

    return ret;
}


static uint8_t ATModule_RecvCGREG(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
    int32_t en = 0;
    int32_t status = 0;
    uint8_t ret = FALSE;

    sscanf((char*)pData, "+CGREG:%d,%d", &en, &status);

    /* 1: 已注册，归属地网络； 5: 已注册，漫游状态 */
    if (status == 1 || status == 5)
    {
        ret = TRUE;
    }

    return ret;
}

static uint8_t ATModule_RecvCOPS(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
	CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
	uint8_t ret = FALSE;
	uint8_t *pTemp = NULL;

	pTemp = Common_SearchData(pData, dataLen, "+COPS:", strlen("+COPS:"));
	if (NULL != pTemp)
	{
		if(NULL != Common_SearchData(pData, dataLen, "MOBILE", strlen("MOBILE")))
		{
			pModulePara->stModuleInfo.eOperatorType = CddDrvSimOperator_CMCC;
			ret = TRUE;
		}
		else if(NULL != Common_SearchData(pData, dataLen, "UNICOM", strlen("UNICOM")))
		{
			pModulePara->stModuleInfo.eOperatorType = CddDrvSimOperator_CUCC;
			ret = TRUE;
		}
		else if(NULL != Common_SearchData(pData, dataLen, "UNICOM", strlen("UNICOM")))
		{
			pModulePara->stModuleInfo.eOperatorType = CddDrvSimOperator_CUCC;
			ret = TRUE;
		}
		else if(NULL != Common_SearchData(pData, dataLen, "CT", strlen("CT")))
		{
			pModulePara->stModuleInfo.eOperatorType = CddDrvSimOperator_CTCC;
			ret = TRUE;
		}
	}
	
	return ret;
}

static uint8_t ATModule_RecvOKACK(uint8_t socketID, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    pTemp = Common_SearchData(pData, dataLen, "OK", strlen("OK"));
    
    if (pTemp != NULL)
    {
        ret = TRUE;
    }

    return ret;
}




static uint8_t ATModule_RecvPDPState(uint8_t socketID, void * modulePara, uint8_t *pData, uint16_t dataLen)
{
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    int32_t activeState = 0;

    pTemp = Common_SearchData(pData, dataLen, "+QIACT: ", strlen("+QIACT: "));

    if (pTemp != NULL)
    {
        sscanf((char*)pTemp, "+QIACT: %d,", &activeState);

        if (activeState == 1)
        {
            CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Work);
            ret = TRUE;
        }
    }

    return ret;
}














