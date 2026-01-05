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
static uint8_t ATModule_RecvSimStatus(uint8_t socketID, uint8_t *pData, uint16_t dataLen);
static uint8_t ATModule_RecvCGREG(uint8_t socketID, uint8_t *pData, uint16_t dataLen);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stModuleATCmdDescribtor[] =
{
    [eATModuleCmd_QueryModule] =  
    { "ATI\r\n",                               "ATI",             3,          5000,     3000,  "识别模块",
        NULL,                                   NULL,                         NULL},
    [eATModuleCmd_SetSimStatusReportEnable] =  
    { "AT+QSIMSTAT=1\r\n",                     "AT+QSIMSTAT=1",   3,          5000,     3000,  "sim卡状态上报使能",
        NULL,                                   NULL,                         NULL},
    [eATModuleCmd_QuerySimStatus] =  
    { "AT+QSIMSTAT?\r\n",                      "+QSIMSTAT:",      3,          5000,     3000,  "sim卡状态查询",
        NULL,                                   ATModule_RecvSimStatus,       NULL},

    [eATModuleCmd_QuerySimRecognizeStatus] =  
    { "AT+CPIN?\r\n",                          "+CPIN: READY",    3,          10000,    3000,  "sim识别状态查询",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryIccid] =  
    { "AT+QCCID\r\n",                          "+QCCID:",         3,          10000,    3000,  "sim卡iccid查询",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryCsq] =  
    { "AT+CSQ\r\n",                            "+CSQ:",           3,          5000,     3000,  "查询信号强度",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryNtpClk] =  
    { "AT+QNTP=1,\"ntp1.aliyun.com\"\r\n",    "+QNTP:",           3,          5000,     3000,  "查询NTP时间",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryCGREG] =  
    { "AT+CGREG?\r\n",                        "+CGREG:",          3,          10000,    3000,  "PS服务网络连接状态查询",
    NULL,                                      ATModule_RecvCGREG,        NULL},

    [eATModuleCmd_QueryCOPS] =  
    { "AT+COPS?\r\n",                         "+COPS:",           10,         3000,     3000,  "查询运营商",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_QueryNetWorkInfo] =  
    { "AT+QNWINFO\r\n",                        "+QNWINFO:",       3,          10000,    3000,  "查询网络信息",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_SetCFUN0] =  
    { "AT+CFUN=0\r\n",                         "+CFUN=0",         3,          10000,    3000,  "设置最小功能模式",
    NULL,                                      NULL,                NULL},

    [eATModuleCmd_SetCFUN1] =  
    { "AT+CFUN=1\r\n",                         "+CFUN=1",         3,          10000,    3000,  "设置全功能模式",
    NULL,                                      NULL,                NULL},
};

const ATUrcDescribtor_Struct c_stATCmdDescribtor[1] =
{


};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t ATModule_RecvSimStatus(uint8_t socketID, uint8_t *pData, uint16_t dataLen)
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

#include "Asw_ErrorHandle.h"

static uint8_t ATModule_RecvCGREG(uint8_t socketID, uint8_t *pData, uint16_t dataLen)
{
    int32_t en = 0;
    int32_t status = 0;
    uint8_t ret = FALSE;

    sscanf((char*)pData, "+CGREG:%d,%d", &en, &status);

    /* 1: 已注册，归属地网络； 5: 已注册，漫游状态 */
    if (status == 1 || status == 5)
    {
        ret = TRUE;
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
    }

    return ret;
}























