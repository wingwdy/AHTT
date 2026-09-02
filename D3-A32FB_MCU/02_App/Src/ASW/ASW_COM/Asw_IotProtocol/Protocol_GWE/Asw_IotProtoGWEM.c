/******************************************************************************
* File Name          : Asw_IotProtoGWEM.c
* Description        : 国网e充电协议主模块
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/05/22      V1.0.0      hzb        初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_IotProtoGWEM.h"
#include "Asw_IotProtoGWERecv.h"
#include "Asw_IotProtoGWESend.h"
#include "Asw_PlatM.h"
#include "Asw_Monitor.h"
#include "Asw_ChargeIf.h"
#include "SS_TM.h"
#include "Cdd_NetM.h"
#include "Cdd_Drv_EG800AK.h"
#include "Cdd_CP.h"
#include "FrameQueue.h"
#include "myMalloc.h"
#include "mbedtls/md.h"
#include "Asw_ErrorHandle.h"
#include "AT_HTTP.h"
#include "SS_Ucm.h"
#include "cJSON.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOTGWE_SIGN_TIMESTAMP                       "2524608000000"     /* 固定时间戳, 参考 SDK dev_sign_mqtt.c */

/* 调试开关: 使用写死三元组直连MQTT, 跳过HTTP注册流程 */
//#define IOTGWE_IS_DEBUG
#ifdef IOTGWE_IS_DEBUG
#define IOTGWE_DEV_DEVICENAME                   "971779950057669436878948"
#define IOTGWE_DEV_PRODUCTKEY                   "a1377lKmcTh"
#define IOTGWE_DEV_DEVICESECRET                 "eec08e408cb660f835dc913fcfb0c5bc"
#define IOTGWE_DEV_MQTT_IP                      "47.114.190.246"
#define IOTGWE_DEV_MQTT_PORT                    30183
#endif

/* HTTP注册API地址 */
#define IOTGWE_REG_BASE_URL_TEST                "http://121.196.185.161:8080/access/pile"                   /* 测试环境 */
#define IOTGWE_REG_BASE_URL_PROD                "http://10.111.186.1:11901/asset-web-api/registerService"   /* 正式环境 */

#define IOTGWE_HTTP_GET_REGISTER_CODE_SUFFIX    "/getRegisterCode"
#define IOTGWE_HTTP_GET_CERTIFICATE_INFO_SUFFIX "/getCertificateInfo"

#define IOTGWE_ENV_REGVASE_URL(envFlag)         (((envFlag) == 0) ? IOTGWE_REG_BASE_URL_PROD : IOTGWE_REG_BASE_URL_TEST)

#define IOTGWE_OTAPROGRESS_DEFAULT              (-128)
#define IOTGWE_REGDEV_RETRYMAX                  3
#define IOTGWE_ORDERCHECK_WAIT_TIMEOUT_MS       (15000U)    /* 交易记录上报等待应答确认超时时间,unit:ms */
/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum{
    
    eIotGWERegDevStep_Start = 0,
    eIotGWERegDevStep_GetRegCode,
    eIotGWERegDevStep_GetCertInfo,
    eIotGWERegDevStep_CreatLink,
    eIotGWERegDevStep_Login,

}IotGWERegDevStep_Enum;
/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
IotGWECtx_Struct *pIotGWECtx = NULL;
static CddNetMSocketPara_Union s_stNetMSocketPara;
/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CommonSendCtrl_Struct* IotGWE_GetSendCtrl(uint8_t port, uint16_t cmd);
static CommonRecvCtrl_Struct* IotGWE_GetRecvCtrl(uint8_t port, uint16_t cmd);
static void IotGWE_WSInitHandle(void);
static void IotGWE_WSRegisterHandle(void);
static void IotGWE_WSOfflineHandle(void);
static void IotGWE_WSLoginHandle(void);
static void IotGWE_WSNormalHandle(void);
static void IotGWE_CycleDetect(void);
static void IotGWE_CycleDetectFeemodelRetry(uint32_t sysTick);
static void IotGWE_CycleDetectUnreportedOrder(void);
static void IotGWE_CycleDetectTimeSync(void);
static void IotGWE_CycleDetectOutMeter(void);
static void IotGWE_CycleDetectMeterRecordUpload(void);
static void IotGWE_CycleDetectFaultRecordUpload(void);
static void IotGWE_CycleDetectTradeRecordUpload(void);
static void IotGWE_CycleDetectTradeRecordNvmUpload(void);
static void IotGWE_CycleDetectRunLogUpload(void);
static void IotGWE_CycleDetectOfflineCharging(void);
static void IotGWE_DealOrderlyCharging(void);
static void IotGWE_MqttConnectCallback(uint8_t connectResult, uint8_t *pCredential);
static void IotGWE_CalcMqttAuth(char *pOutUsername, uint8_t usernameSize, char *pOutPassword, uint16_t passwordSize);
static void IotGWE_OTAProgress(void);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
/**
 ******************************************************************************
 * @brief  获取发送命令控制块
 * @return 命令控制块指针
 ******************************************************************************
 */
static CommonSendCtrl_Struct* IotGWE_GetSendCtrl(uint8_t port, uint16_t cmd)
{
    CommonSendCtrl_Struct* pSendCtrl = NULL;

    switch (cmd)
    {
        case IOT_GWE_CMD_FWINFO_REQ:                    pSendCtrl = &pIotGWECtx->stSendCtrl[port][0];   break;
        case IOT_GWE_CMD_FIRMWARE_INFO_REQ:             pSendCtrl = &pIotGWECtx->stSendCtrl[port][1];   break;
        case IOT_GWE_CMD_VER_INFO_REQ:                  pSendCtrl = &pIotGWECtx->stSendCtrl[port][2];   break;
        case IOT_GWE_CMD_DEVMDU_INFO_REQ:               pSendCtrl = &pIotGWECtx->stSendCtrl[port][3];   break;
        case IOT_GWE_CMD_ASK_DEV_CONFIG_REQ:            pSendCtrl = &pIotGWECtx->stSendCtrl[port][4];   break;
        case IOT_GWE_CMD_ASK_FEEMODEL_REQ:              pSendCtrl = &pIotGWECtx->stSendCtrl[port][5];   break;
        case IOT_GWE_CMD_TIME_SYNC_RESULT_REQ:          pSendCtrl = &pIotGWECtx->stSendCtrl[port][6];   break;
        case IOT_GWE_CMD_START_CHA_RES_REQ:             pSendCtrl = &pIotGWECtx->stSendCtrl[port][7];   break;
        case IOT_GWE_CMD_START_CHARGE_AUTH_REQ:         pSendCtrl = &pIotGWECtx->stSendCtrl[port][8];   break;
        case IOT_GWE_CMD_STOP_CHA_RES_REQ:              pSendCtrl = &pIotGWECtx->stSendCtrl[port][9];   break;
        case IOT_GWE_CMD_ORDER_TW_UPDATE_REQ:           pSendCtrl = &pIotGWECtx->stSendCtrl[port][10];  break;
        case IOT_GWE_CMD_TOTAL_FAULT_REQ:               pSendCtrl = &pIotGWECtx->stSendCtrl[port][11];  break;
        case IOT_GWE_CMD_AC_ST_CH_REQ:                  pSendCtrl = &pIotGWECtx->stSendCtrl[port][12];  break;
        case IOT_GWE_CMD_AC_CAR_CON_CH_REQ:             pSendCtrl = &pIotGWECtx->stSendCtrl[port][13];  break;
        case IOT_GWE_CMD_LOG_QUERY_RESULT_REQ:          pSendCtrl = &pIotGWECtx->stSendCtrl[port][14];  break;
        case IOT_GWE_CMD_DEV_MAINTAIN_RET_REQ:          pSendCtrl = &pIotGWECtx->stSendCtrl[port][15];  break;
        case IOT_GWE_CMD_PROPERTY_ACPILE_REQ:           pSendCtrl = &pIotGWECtx->stSendCtrl[port][16];  break;
        case IOT_GWE_CMD_PROPERTY_AC_WORK_REQ:          pSendCtrl = &pIotGWECtx->stSendCtrl[port][17];  break;
        case IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ:       pSendCtrl = &pIotGWECtx->stSendCtrl[port][18];  break;
        case IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ:      pSendCtrl = &pIotGWECtx->stSendCtrl[port][19];  break;
        case IOT_GWE_CMD_CONF_UPDATE_SRV_REPLY:         pSendCtrl = &pIotGWECtx->stSendCtrl[port][20];  break;
        case IOT_GWE_CMD_CONF_GET_SRV_REPLY:            pSendCtrl = &pIotGWECtx->stSendCtrl[port][21];  break;
        case IOT_GWE_CMD_FUN_CONF_UPDATE_SRV_REPLY:     pSendCtrl = &pIotGWECtx->stSendCtrl[port][22];  break;
        case IOT_GWE_CMD_FUN_CONF_GET_SRV_REPLY:        pSendCtrl = &pIotGWECtx->stSendCtrl[port][23];  break;
        case IOT_GWE_CMD_FEE_MODEL_UPDATE_SRV_REPLY:    pSendCtrl = &pIotGWECtx->stSendCtrl[port][24];  break;
        case IOT_GWE_CMD_FEE_MODEL_QUERY_SRV_REPLY:     pSendCtrl = &pIotGWECtx->stSendCtrl[port][25];  break;
        case IOT_GWE_CMD_START_CHARGE_SRV_REPLY:        pSendCtrl = &pIotGWECtx->stSendCtrl[port][26];  break;
        case IOT_GWE_CMD_STOP_CHARGE_SRV_REPLY:         pSendCtrl = &pIotGWECtx->stSendCtrl[port][27];  break;
        case IOT_GWE_CMD_ORDER_CHECK_SRV_REPLY:         pSendCtrl = &pIotGWECtx->stSendCtrl[port][28];  break;
        case IOT_GWE_CMD_QUE_DATA_SRV_REPLY:            pSendCtrl = &pIotGWECtx->stSendCtrl[port][29];  break;
        case IOT_GWE_CMD_DEV_MAINTAIN_CTRL_SRV_REPLY:   pSendCtrl = &pIotGWECtx->stSendCtrl[port][30];  break;
        case IOT_GWE_CMD_DEV_MAINTAIN_QUERY_SRV_REPLY:  pSendCtrl = &pIotGWECtx->stSendCtrl[port][31];  break;
        case IOT_GWE_CMD_TRADE_RECORD_ASK_SRV_REPLY:    pSendCtrl = &pIotGWECtx->stSendCtrl[port][32];  break;
        case IOT_GWE_CMD_METER_RECORD_ASK_SRV_REPLY:    pSendCtrl = &pIotGWECtx->stSendCtrl[port][33];  break;
        case IOT_GWE_CMD_TIME_SYNC_SRV_REPLY:           pSendCtrl = &pIotGWECtx->stSendCtrl[port][34];  break;
        case IOT_GWE_CMD_AC_ORDERLY_CHARGE_SRV_REPLY:   pSendCtrl = &pIotGWECtx->stSendCtrl[port][35];  break;
        case IOT_GWE_CMD_RSV_CHARGE_SRV_REPLY:          pSendCtrl = &pIotGWECtx->stSendCtrl[port][36];  break;
        case IOT_GWE_CMD_OTA_PROGRESS_REQ:              pSendCtrl = &pIotGWECtx->stSendCtrl[port][37];  break;
        default: break;
    }

    return pSendCtrl;
}

/**
 ******************************************************************************
 * @brief  获取接收命令控制块
 * @return 命令控制块指针
 ******************************************************************************
 */
static CommonRecvCtrl_Struct* IotGWE_GetRecvCtrl(uint8_t port, uint16_t cmd)
{
    CommonRecvCtrl_Struct* pRecvCtrl = NULL;

    switch (cmd)
    {
        case IOT_GWE_CMD_FIRMWARE_INFO_RSP:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][0];   break;
        case IOT_GWE_CMD_VER_INFO_RSP:              pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][1];   break;
        case IOT_GWE_CMD_DEVMDU_INFO_RSP:           pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][2];   break;
        case IOT_GWE_CMD_DEV_CONFIG_RSP:            pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][3];   break;
        case IOT_GWE_CMD_FEEMODEL_RSP:              pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][4];   break;
        case IOT_GWE_CMD_TIME_SYNC_RSP:             pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][5];   break;
        case IOT_GWE_CMD_START_CHA_RES_RSP:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][6];   break;
        case IOT_GWE_CMD_START_CHARGE_AUTH_RSP:     pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][7];   break;
        case IOT_GWE_CMD_STOP_CHA_RES_RSP:          pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][8];   break;
        case IOT_GWE_CMD_ORDER_TW_UPDATE_RSP:       pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][9];   break;
        case IOT_GWE_CMD_TOTAL_FAULT_RSP:           pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][10];  break;
        case IOT_GWE_CMD_AC_ST_CH_RSP:              pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][11];  break;
        case IOT_GWE_CMD_AC_CAR_CON_CH_RSP:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][12];  break;
        case IOT_GWE_CMD_LOG_QUERY_RESULT_RSP:      pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][13];  break;
        case IOT_GWE_CMD_DEV_MAINTAIN_RET_RSP:      pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][14];  break;
        case IOT_GWE_CMD_PROPERTY_ACPILE_RSP:       pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][15];  break;
        case IOT_GWE_CMD_PROPERTY_AC_WORK_RSP:      pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][16];  break;
        case IOT_GWE_CMD_PROPERTY_AC_NONWORK_RSP:   pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][17];  break;
        case IOT_GWE_CMD_PROPERTY_AC_OUTMETER_RSP:  pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][18];  break;
        case IOT_GWE_SRV_CONF_UPDATE:               pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][19];  break;
        case IOT_GWE_SRV_CONF_GET:                  pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][20];  break;
        case IOT_GWE_SRV_FUN_CONF_UPDATE:           pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][21];  break;
        case IOT_GWE_SRV_FUN_CONF_GET:              pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][22];  break;
        case IOT_GWE_SRV_FEE_MODEL_UPDATE:          pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][23];  break;
        case IOT_GWE_SRV_FEE_MODEL_QUERY:           pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][24];  break;
        case IOT_GWE_SRV_START_CHARGE:              pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][25];  break;
        case IOT_GWE_SRV_STOP_CHARGE:               pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][26];  break;
        case IOT_GWE_SRV_ORDER_CHECK:               pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][27];  break;
        case IOT_GWE_SRV_QUE_DATA:                  pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][28];  break;
        case IOT_GWE_SRV_DEV_MAINTAIN_CTRL:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][29];  break;
        case IOT_GWE_SRV_DEV_MAINTAIN_QUERY:        pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][30];  break;
        case IOT_GWE_SRV_TRADE_RECORD_ASK:          pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][31];  break;
        case IOT_GWE_SRV_METER_RECORD_ASK:          pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][32];  break;
        case IOT_GWE_SRV_TIME_SYNC:                 pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][33];  break;
        case IOT_GWE_SRV_AC_ORDERLY_CHARGE:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][34];  break;
        case IOT_GWE_SRV_RSV_CHARGE:                pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][35];  break;
        case IOT_GWE_CMD_PROPERTY_SET_RECV:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][36];  break;
        case IOT_GWE_CMD_OTA_UPGRADE_RECV:          pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][37];  break;
        case IOT_GWE_CMD_OTA_FIRMWARE_REPLY_RECV:   pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][38];  break;
        case IOT_GWE_CMD_NTP_RESPONSE_RECV:         pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][39];  break;
        case IOT_GWE_CMD_DEV_CONFG_UPDATE_SRV_RECV: pRecvCtrl = &pIotGWECtx->stRecvCtrl[port][40];  break;
        default: break;
    }

    return pRecvCtrl;
}

/**
 ******************************************************************************
 * @brief  平台初始化处理
 ******************************************************************************
 */
static void IotGWE_WSInitHandle(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    uint8_t needWrite = FALSE;

    if (pPlatInfo->equipParamReportCycle == 0)
    {
        pPlatInfo->equipParamReportCycle = IOTGWE_CFG_EQUIP_PARAM_FREQ_DEFAULT;
        needWrite = TRUE;
    }

    if (pPlatInfo->gunElecReportCycle == 0)
    {
        pPlatInfo->gunElecReportCycle = IOTGWE_CFG_GUN_ELEC_FREQ_DEFAULT;
        needWrite = TRUE;
    }

    if (pPlatInfo->nonElecReportCycle == 0)
    {
        pPlatInfo->nonElecReportCycle = IOTGWE_CFG_NON_ELEC_FREQ_DEFAULT;
        needWrite = TRUE;
    }

    if (pPlatInfo->faultWarningsCycle == 0)
    {
        pPlatInfo->faultWarningsCycle = IOTGWE_CFG_FAULT_WARNINGS_DEFAULT;
        needWrite = TRUE;
    }

    if (pPlatInfo->offlineChaLen == 0)
    {
        pPlatInfo->offlineChaLen = IOTGWE_CFG_OFFLIN_CHALEN_DEFAULT;
        needWrite = TRUE;
    }

    if (needWrite)
    {
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
    }

#ifdef IOTGWE_IS_DEBUG
    /* 调试模式: 跳过注册(使用默认三元组) */
    pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
#else
    /* 生产模式: 检查是否已有凭据, 无则进入HTTP注册流程 */
    if (pPlatInfo->credentialSaveFlag == TRUE || strlen(pParam->platPileDn) == 0)
    {
        pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
    }
    else
    {
        pIotGWECtx->eWorkState = eIOTGWEWorkState_Register;
        pIotGWECtx->regStep = 0;
        pIotGWECtx->regRetryCount = 0;
    }
#endif
}

/**
 ******************************************************************************
 * @brief  注册设备请求
 * @param[in] url    URL字符串
 * @param[in] body   POST body字符串
 * @param[out] pRsp  响应数据
 * @return TRUE=请求完成, FALSE=进行中或失败
 ******************************************************************************
 */
static uint8_t IotGWE_RegDevRequest(const char *url, const char *body, CddNetMHttpPara_Struct **ppRsp)
{
    CddNetMHttpPara_Struct *pH = CddNetM_GetHttpPara(eCddNetMPlatType_O);
    uint8_t ret = FALSE;

    do {

        if (pH == NULL)
        {/* 未创建HTTP, 则创建 */
            memset(&s_stNetMSocketPara, 0, sizeof(s_stNetMSocketPara));

            s_stNetMSocketPara.stHttpPara.type    = eCddNetMHttpType_POST;
            s_stNetMSocketPara.stHttpPara.urlLen  = (uint16_t)strlen(url);
            s_stNetMSocketPara.stHttpPara.bodyLen = (uint16_t)strlen(body);
            if (s_stNetMSocketPara.stHttpPara.urlLen >= CDD_NETM_CFG_HTTP_URL_LEN)
            {
                s_stNetMSocketPara.stHttpPara.urlLen = CDD_NETM_CFG_HTTP_URL_LEN  - 1;
            }

            if (s_stNetMSocketPara.stHttpPara.bodyLen >= CDD_NETM_CFG_HTTP_BODY_LEN)
            {
                s_stNetMSocketPara.stHttpPara.bodyLen = CDD_NETM_CFG_HTTP_BODY_LEN - 1;
            }

            memcpy(s_stNetMSocketPara.stHttpPara.url,  url,  s_stNetMSocketPara.stHttpPara.urlLen);
            memcpy(s_stNetMSocketPara.stHttpPara.body, body, s_stNetMSocketPara.stHttpPara.bodyLen);
            CddNetM_CreatLink(eCddNetMSocketType_HTTP, s_stNetMSocketPara, eCddNetMPlatType_O);

            break;
        }

        if (pH->dataReady)
        {/* 收到POST应答数据 */
            pH->dataReady = FALSE;
            *ppRsp = pH;
            ret = TRUE;
            break;
        }

    } while (0);

    return ret;
}

/**
 ******************************************************************************
 * @brief  注册超时检查
 * @return TRUE=超时 FALSE=未超时
 ******************************************************************************
 */
static void IotGWE_RegTimeoutCheck(uint32_t now, const char *pStepName)
{
    if (pIotGWECtx->regWaitTick == 0)
    {
        pIotGWECtx->regWaitTick = now;
    }
    else if (Common_JudgeTimeoutMs(pIotGWECtx->regWaitTick, IOTGWE_REGDEV_TIMEOUT_MS))
    {
        IOTGWE_CFG_DebugPrint("[GWE] %s timeout\r\n", pStepName);
        pIotGWECtx->regRetryCount++;
        pIotGWECtx->regWaitTick = 0;
        CddNetM_DeleteLink(eCddNetMPlatType_O);
    }
    else 
    {}
}

/**
 ******************************************************************************
 * @brief  平台注册处理: HTTP获取三元组
 ******************************************************************************
 */
static void IotGWE_WSRegisterHandle(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    CddNetMHttpPara_Struct *pH = NULL;
    uint8_t httpDone = FALSE;
    uint32_t now = Common_GetSystick();
    char url[128], body[128];
    cJSON *root = NULL, *code = NULL, *data = NULL;
    cJSON *pk = NULL, *dn = NULL, *ds = NULL;

    do {

        if (pIotGWECtx->regStep == 0 && pPlatInfo->credentialSaveFlag == TRUE)
        {
            pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
            break;
        }

        if (strlen(pParam->platPileDn) == 0)
        {
            pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
            break;
        }

        switch (pIotGWECtx->regStep)
        {
            case eIotGWERegDevStep_Start:
                pIotGWECtx->regStep = eIotGWERegDevStep_GetRegCode;
                pIotGWECtx->regRetryCount = 0;
                pIotGWECtx->regWaitTick = 0;
                IOTGWE_CFG_DebugPrint("[GWE] Register start, deviceCode=%s\r\n", pParam->platPileDn);
                break;

            case eIotGWERegDevStep_GetRegCode:  /* 注册码getRegisterCode */
            {
                snprintf(url,  sizeof(url),  "%s%s", pIotGWECtx->regBaseUrl, IOTGWE_HTTP_GET_REGISTER_CODE_SUFFIX);
                snprintf(body, sizeof(body), "deviceCode=%s", pParam->platPileDn);

                httpDone = IotGWE_RegDevRequest(url, body, &pH);

                if (httpDone && pH != NULL)
                {
                    root = cJSON_Parse(pH->body);
                    if (root != NULL)
                    {
                        code = cJSON_GetObjectItem(root, "code");
                        if (code != NULL && code->valueint == 200)
                        {
                            data = cJSON_GetObjectItem(root, "data");
                            if (data != NULL)
                            {
                                cJSON *regCode = cJSON_GetObjectItem(data, "registerCode");
                                if (regCode != NULL && regCode->valuestring != NULL)
                                {
                                    strncpy(pIotGWECtx->registerCode, regCode->valuestring, sizeof(pIotGWECtx->registerCode) - 1);
                                    IOTGWE_CFG_InfoPrint("[GWE] registerCode obtained: %s\r\n", pIotGWECtx->registerCode);
                                    pIotGWECtx->regStep = eIotGWERegDevStep_GetCertInfo;
                                    pIotGWECtx->regRetryCount = 0;
                                }
                            }
                        }
                        cJSON_Delete(root);
                    }

                    CddNetM_DeleteLink(eCddNetMPlatType_O);
                    pIotGWECtx->regWaitTick = 0;

                    if (pIotGWECtx->regStep != eIotGWERegDevStep_GetCertInfo)
                    {
                        IOTGWE_CFG_DebugPrint("[GWE] registerCode parse failed, rsp=%s\r\n", pH->body);
                        pIotGWECtx->regRetryCount++;
                    }
                }
                else
                {
                    IotGWE_RegTimeoutCheck(now, "getRegisterCode");
                }

                if (pIotGWECtx->regRetryCount >= IOTGWE_REGDEV_RETRYMAX)
                {
                    IOTGWE_CFG_DebugPrint("[GWE] getRegisterCode max retries, exit\r\n");
                    pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
                }
                break;
            }

            case eIotGWERegDevStep_GetCertInfo:  /* 获取三元组getCertificateInfo */
            {
                snprintf(url,  sizeof(url),  "%s%s", pIotGWECtx->regBaseUrl, IOTGWE_HTTP_GET_CERTIFICATE_INFO_SUFFIX);
                snprintf(body, sizeof(body), "deviceCode=%s&registerCode=%s&gunNo=0", pParam->platPileDn, pIotGWECtx->registerCode);

                httpDone = IotGWE_RegDevRequest(url, body, &pH);

                if (httpDone && pH != NULL)
                {
                    root = cJSON_Parse(pH->body);
                    if (root != NULL)
                    {
                        code = cJSON_GetObjectItem(root, "code");
                        if (code != NULL && code->valueint == 200)
                        {
                            data = cJSON_GetObjectItem(root, "data");
                            if (data != NULL)
                            {
                                pk = cJSON_GetObjectItem(data, "productKey");
                                dn = cJSON_GetObjectItem(data, "deviceName");
                                ds = cJSON_GetObjectItem(data, "deviceSecret");
                                if (pk && pk->valuestring && dn && dn->valuestring && ds && ds->valuestring)
                                {
                                    strncpy(pPlatInfo->cProductKey,   pk->valuestring, sizeof(pPlatInfo->cProductKey)   - 1);
                                    strncpy(pPlatInfo->cDeviceName,   dn->valuestring, sizeof(pPlatInfo->cDeviceName)   - 1);
                                    strncpy(pPlatInfo->cDeviceSecret, ds->valuestring, sizeof(pPlatInfo->cDeviceSecret) - 1);
                                    pPlatInfo->credentialSaveFlag = TRUE;
                                    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
                                    IOTGWE_CFG_InfoPrint("[GWE] Triple obtained: pk=%s, dn=%s\r\n", pPlatInfo->cProductKey, pPlatInfo->cDeviceName);
                                    pIotGWECtx->regStep = eIotGWERegDevStep_CreatLink;
                                    pIotGWECtx->regRetryCount = 0;
                                }
                            }
                        }
                        cJSON_Delete(root);
                    }

                    CddNetM_DeleteLink(eCddNetMPlatType_O);
                    pIotGWECtx->regWaitTick = 0;

                    if (pIotGWECtx->regStep != eIotGWERegDevStep_CreatLink)
                    {
                        IOTGWE_CFG_DebugPrint("[GWE] Triple parse failed, rsp=%s\r\n", pH->body);
                        pIotGWECtx->regRetryCount++;
                    }
                }
                else
                {
                    IotGWE_RegTimeoutCheck(now, "getCertificateInfo");
                }

                if (pIotGWECtx->regRetryCount >= IOTGWE_REGDEV_RETRYMAX)
                {
                    IOTGWE_CFG_DebugPrint("[GWE] getCertificateInfo max retries, failed\r\n");
                    pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
                }
                break;
            }

            case eIotGWERegDevStep_CreatLink:  /* 创建MQTT连接 */
                memset(&s_stNetMSocketPara, 0, sizeof(s_stNetMSocketPara));
                IotGWE_FillLinkPara(&s_stNetMSocketPara);
                CddNetM_CreatLink(eCddNetMSocketType_MQTT, s_stNetMSocketPara, eCddNetMPlatType_O);
                pIotGWECtx->regStep = eIotGWERegDevStep_Login;
                break;

            case eIotGWERegDevStep_Login:  /* 进入离线 */
                IOTGWE_CFG_InfoPrint("[GWE] Register completed, will conn mqtt\r\n");
                pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;
                break;

            default:
                pIotGWECtx->regStep = eIotGWERegDevStep_Start;
                break;
        }

    } while (0);

}

/**
 ******************************************************************************
 * @brief  平台离线处理
 ******************************************************************************
 */
static void IotGWE_WSOfflineHandle(void)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint8_t port = 0;
    char userName[CDD_NETM_CFG_MQTT_USER_NAME_LEN + 1] = {0};
    char password[CDD_NETM_CFG_MQTT_PASSWORD_LEN + 1] = {0};

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {/* 离线清除缓存 */
        memset(&pIotGWECtx->stProtoData.stRecvData[port].offlineClearData, 0x00, sizeof(IotGWEDataOfflineClr_Struct));
    }

    pIotGWECtx->loginSucc = FALSE;
    pIotGWECtx->queueBusyFlag = FALSE;
    pIotGWECtx->waitQueueIdleTick = 0;

    pIotGWECtx->sendIndex = 0;
    pIotGWECtx->sendPort = 0;
    pIotGWECtx->reqSeq = 0;
    pIotGWECtx->curMsgId = 0;

    memset(pIotGWECtx->stSendCtrl, 0x00, sizeof(pIotGWECtx->stSendCtrl));
    memset(pIotGWECtx->stRecvCtrl, 0x00, sizeof(pIotGWECtx->stRecvCtrl));
    FrameQueue_Reset(pIotGWECtx->frameQueueChannelID);
#ifdef IOTGWE_IS_DEBUG
    /* 调试用:  设置默认三元组 */
    strcpy(pPrivateParam->stGWEParam.platinfo.cProductKey, IOTGWE_DEV_PRODUCTKEY);
    strcpy(pPrivateParam->stGWEParam.platinfo.cDeviceName,   IOTGWE_DEV_DEVICENAME);
    strcpy(pPrivateParam->stGWEParam.platinfo.cDeviceSecret, IOTGWE_DEV_DEVICESECRET);
    pPrivateParam->stGWEParam.platinfo.credentialSaveFlag = TRUE;
    MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
#endif
    memcpy(pIotGWECtx->productKey,  pPrivateParam->stGWEParam.platinfo.cProductKey,  MSNVM_GWE_PRODUCT_KEY_LEN);
    memcpy(pIotGWECtx->deviceName,  pPrivateParam->stGWEParam.platinfo.cDeviceName,   MSNVM_GWE_DEVICE_NAME_LEN);
    memcpy(pIotGWECtx->deviceSecret,pPrivateParam->stGWEParam.platinfo.cDeviceSecret, MSNVM_GWE_DEVICE_SECRET_LEN);

    /* 更新MQTT用户名和密钥 */
    if (pPrivateParam->stGWEParam.platinfo.credentialSaveFlag == TRUE)
    {
        IotGWE_CalcMqttAuth(userName, sizeof(userName), password, sizeof(password));
        CddNetM_UpdateMqttUserNamePassword(eCddNetMPlatType_O, userName, password);
    }

    AswErrhandle_SetErrExsitCallback(0, eErr_PlatformOffline);
    pIotGWECtx->eWorkState = eIOTGWEWorkState_Login;
}

/**
 ******************************************************************************
 * @brief  平台登录处理
 ******************************************************************************
 */
static void IotGWE_WSLoginHandle(void)
{
    uint8_t port = 0;

    if (TRUE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        pIotGWECtx->eWorkState = eIOTGWEWorkState_Normal;

        /* 从NVM恢复计费模型到Monitor */
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            IotGWE_TransformBillMode(port, AswMonitor_GetCurUsedBillModePtr(port));
        }

        /* 上线推送固件版本到OTA通道, 平台据此判定OTA成功 */
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_FWINFO_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_FWINFO_REQ, TRUE);

        /* 上线时上报: 固件, 版本, 设备组件信息 */
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_FIRMWARE_INFO_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_FIRMWARE_INFO_REQ, TRUE);

        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_VER_INFO_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_VER_INFO_REQ, TRUE);

        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_DEVMDU_INFO_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_DEVMDU_INFO_REQ, TRUE);

        /* 上线请求设备配置: 立即请求(之后每个10秒重试2次) */
        pIotGWECtx->devConfigReceived = FALSE;
        pIotGWECtx->devConfigReqCnt = 1;
        pIotGWECtx->lastDevConfigReqTick = Common_GetSystick();
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_DEV_CONFIG_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_DEV_CONFIG_REQ, TRUE);

        /* 上线请求计费模型: 延时10秒上报(之后每隔10s重试, 重试4次后每2h再尝试) */
        pIotGWECtx->feeModelReceived = FALSE;
        pIotGWECtx->feeModelReqCnt = 0;
        pIotGWECtx->lastFeeModelReqTick = Common_GetSystick();

        /* 上线主动上报枪状态和CP连接状态 */
        pIotGWECtx->loginFirstReport = TRUE;

        /* 上线触发首次对时 */
        pIotGWECtx->timeSyncFlag = 0;
        pIotGWECtx->timeSyncReqCnt = 0;
        pIotGWECtx->timeSyncReqTick = 0;

        /* 清空交易记录上下文 */
        pIotGWECtx->orderReportAwaitTick = 0;
        pIotGWECtx->reportingPreTradeNo[0] = '\0';
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            pIotGWECtx->reportUseRuntime[port] = FALSE;
        }

        /* 上报(未同步)历史订单 */
        pIotGWECtx->tradeRecordUploadFlag = TRUE;
    }
}

/**
 ******************************************************************************
 * @brief  连接中循环检测 - 周期性数据上报
 ******************************************************************************
 */
static void IotGWE_CycleDetect(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    uint32_t sysTick = Common_GetSystick();
    uint8_t port = 0;
    uint8_t isCharging = FALSE;
    uint8_t curGunConn = 0;
    CddCPVolState_Enum curCP = eCddCPVolState_12V;
    uint8_t errChanged = FALSE;
    uint32_t curVersion = 0;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        isCharging = (AswChargeIf_GetChargeState(port) == ASWCHARGEIF_WORKSTATE_CHARGING);

        /* 设备属性 */
        if (pPlatInfo->equipParamReportCycle > 0 &&
            IotGWE_ReportCycleCheck(port, IOT_GWE_CMD_PROPERTY_ACPILE_REQ, pPlatInfo->equipParamReportCycle * 1000))
        {
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_ACPILE_REQ, TRUE);
            Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_ACPILE_REQ, sysTick);
        }

        if (isCharging)
        {/* 充电中: 上报充电实时属性 + 电表属性 */
#ifdef IOTGWE_CFG_ACWORK_FAST_REPORT
            /* 启动后1分钟内每5s快速上报, 之后按gunElecFreq周期 */
            if (pIotGWECtx->acWorkFastFlag[port] == TRUE)
            {
                if (!Common_JudgeTimeoutMs(pIotGWECtx->acWorkFastStartTick[port], IOTGWE_ACWORK_FAST_DURATION_MS))
                {
                    if (IotGWE_ReportCycleCheck(port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, IOTGWE_ACWORK_FAST_CYCLE_MS))
                    {
                        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, TRUE);
                        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, sysTick);
                    }
                }
                else
                {
                    pIotGWECtx->acWorkFastFlag[port] = FALSE;
                }
            }
            else
#endif
            if (pPlatInfo->gunElecReportCycle > 0 &&
                IotGWE_ReportCycleCheck(port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, pPlatInfo->gunElecReportCycle * 1000))
            {
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, TRUE);
                Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, sysTick);
            }
        }
        else
        {/* 非充电中: 上报非充电属性 */
            if (pPlatInfo->nonElecReportCycle > 0 &&
                IotGWE_ReportCycleCheck(port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, pPlatInfo->nonElecReportCycle * 1000))
            {
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, TRUE);
                Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, sysTick);
            }
        }
    }

    /* 2. 故障告警上报 */
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curVersion = AswErrHandle_GetErrStatusVersion(port);
        if (curVersion != pIotGWECtx->lastErrVersion[port])
        {
            pIotGWECtx->lastErrVersion[port] = curVersion;
            errChanged = TRUE;
        }
    }
    if (errChanged && IotGWE_CheckErrStatus())
    {
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_TOTAL_FAULT_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_TOTAL_FAULT_REQ, TRUE);
    }

    if (pPlatInfo->faultWarningsCycle > 0 &&
        IotGWE_ReportCycleCheck(0, IOT_GWE_CMD_TOTAL_FAULT_REQ, pPlatInfo->faultWarningsCycle * 1000))
    {
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_TOTAL_FAULT_REQ, TRUE);
        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_TOTAL_FAULT_REQ, sysTick);
    }

    /* 3. 枪状态和CP连接状态变化检测 */
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        curGunConn = AswChargeIf_CheckGunConnected(port) ? 1 : 0;
        curCP = CddCP_GetVolState(port);

        if (pIotGWECtx->loginFirstReport)
        {/* 上线首次上报 */
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_ST_CH_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_ST_CH_REQ, TRUE);

            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_CAR_CON_CH_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_CAR_CON_CH_REQ, TRUE);
        }
        else
        {
            if (curGunConn != pIotGWECtx->prevGunConnected[port])
            {/* 检测枪连接变化 */
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_ST_CH_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_ST_CH_REQ, TRUE);
            }

            if ((uint8_t)curCP != pIotGWECtx->prevCPState[port])
            {/* 检测CP电压变化 */
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_CAR_CON_CH_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_AC_CAR_CON_CH_REQ, TRUE);
            }
        }

        pIotGWECtx->prevGunConnected[port] = curGunConn;
        pIotGWECtx->prevCPState[port] = (uint8_t)curCP;
    }

    /* 4. 配置/请求重试
     *     策略: 登录立即发(cnt=1), 未收到则每10s重试, 最多重试2次(cnt=2,3共3次) */
    if (!pIotGWECtx->devConfigReceived &&
        pIotGWECtx->devConfigReqCnt < 3 &&
        Common_JudgeTimeoutMs(pIotGWECtx->lastDevConfigReqTick, 10 * 1000))
    {
        pIotGWECtx->devConfigReqCnt++;
        pIotGWECtx->lastDevConfigReqTick = sysTick;
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_DEV_CONFIG_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_DEV_CONFIG_REQ, TRUE);
        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_DEV_CONFIG_REQ, sysTick);
    }

    /* 5. 计费模型请求重试 */
    IotGWE_CycleDetectFeemodelRetry(sysTick);

    /* 6. 未上报交易记录检测 */
    IotGWE_CycleDetectUnreportedOrder();

    /* 7. 对时检测 */
    IotGWE_CycleDetectTimeSync();

    /* 8. 电表底值整点上报   */
    IotGWE_CycleDetectOutMeter();

    /* 9. 电表底值逐帧上传检测 (召测/日志查询触发) */
    IotGWE_CycleDetectMeterRecordUpload();

    /* 10. 故障告警逐帧上传检测 */
    IotGWE_CycleDetectFaultRecordUpload();

    /* 11. 交易记录逐帧上传(日志召测 logQueryEvt) */
    IotGWE_CycleDetectTradeRecordUpload();

    /* 12. 交易记录逐条上传(交易记录召测(全部记录), orderTwUpdateEvt) */
    IotGWE_CycleDetectTradeRecordNvmUpload();

    /* 13. 运行日志分片逐帧上传检测 */
    IotGWE_CycleDetectRunLogUpload();

    /* 14. 有序充电功率调节 */
    IotGWE_DealOrderlyCharging();

    /* 15. OTA进度上报 */
    IotGWE_OTAProgress();

    pIotGWECtx->loginFirstReport = FALSE;
}

/**
 ******************************************************************************
 * @brief  同步时间:
 *         1) 每次上线对时一次
 *         2) 运行中每24小时对时一次
 *         3) 对时失败后每10秒重试, 3次不成功上报对时故障
 *         4) 故障后每2小时对时一次, 成功消除故障
 *         5) 充电中不允许对时
 ******************************************************************************
 */
static void IotGWE_CycleDetectTimeSync(void)
{
    uint32_t sysTick = Common_GetSystick();
    uint32_t retryInterval;
    uint8_t port;
    uint8_t isCharge = FALSE;
    char topic[128];

    do {
        /* 充电中不允许对时 */
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (AswMonitor_IsOrderIdle(port) != TRUE)
            {
                isCharge = TRUE;
                break;
            }
        }

        if (isCharge == TRUE)
            break;

        /* 确定重试间隔 */
        if (pIotGWECtx->timeSyncReqCnt > 3)
        {
            retryInterval = 2 * 3600 * 1000;    /* 故障后每2小时 */
        }
        else if (pIotGWECtx->timeSyncFlag == 0)
        {
            retryInterval = 10 * 1000;          /* 失败后每10秒 */
        }
        else
        {
            retryInterval = 24 * 3600 * 1000;   /* 成功后每24小时 */
        }

        if (Common_JudgeTimeoutMs(pIotGWECtx->timeSyncReqTick, retryInterval) == FALSE)
        {
            break;
        }

        /* 发送NTP请求 */
        snprintf(topic, sizeof(topic), IOT_GWE_PUB_NTP_REQUEST, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        FrameQueue_PushTx(pIotGWECtx->frameQueueChannelID, topic, strlen(topic), (uint8_t *)"{}", sizeof("{}") - 1, 1);

        pIotGWECtx->timeSyncReqTick = sysTick;
        pIotGWECtx->timeSyncReqCnt++;

        IOTGWE_CFG_DebugPrint("[GWE] NTP request send, cnt=%u\r\n", pIotGWECtx->timeSyncReqCnt);

    } while (0);
}

/**
 ******************************************************************************
 * @brief  电表底值整点上报检测
 *         策略: 每30s检查一次, 整点(minute==0)触发, lastHour防重复
 ******************************************************************************
 */
static void IotGWE_CycleDetectOutMeter(void)
{
    uint32_t ts;
    uint8_t min, hour;
    uint8_t port;
    MSNvmMeterRecord_Struct rec;
    uint32_t sysTick = Common_GetSystick();

    do {

        if (!Common_JudgeTimeoutMs(pIotGWECtx->acOutMeterTick, 30000))
        {
            break;
        }

        pIotGWECtx->acOutMeterTick = sysTick;

        ts = SSTM_GetSecTimestamp();
        if (ts < 1577808000u)
        {/* 2020-01-01 00:00:00 UTC+8, 时间未同步则不启动 */
            break;
        }

        min = (ts / 60) % 60;
        if (min != 0)
        {/* 不是整点 */
            break;
        }

        hour = (ts / 3600) % 24;
        if (hour == pIotGWECtx->acOutMeterLastHour)
        {/* 如果之前该整点已上报过了 */
            break;
        }

        pIotGWECtx->acOutMeterLastHour = hour;

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ, TRUE);
            Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_OUTMETER_REQ, sysTick);

            /* 存储电表底值记录到 TSDB */
            MSNvmOrderInfo_Struct *pOrder = AswMonitor_GerOrderDataPtr(port);
            AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);

            memset(&rec, 0, sizeof(rec));
            rec.gunNo = port + 1;
            rec.acqTime = ts;
            rec.sumMeter = AswChargeIf_GetMeterEnergyVal(port);
            if (pOrder != NULL)
            {
                strncpy(rec.lastTrade, pOrder->platOrderInfo.stGWEOrderInfo.preTradeNo, sizeof(rec.lastTrade) - 1);
            }
            if (pChargeData != NULL)
            {
                rec.elec = pChargeData->totalEnergy;
            }
            MSNvm_InsertNewRecord(eMSNvmBlockID_MeterRecord, (uint8_t *)&rec, sizeof(rec));
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  电表底值逐帧上传检测 (召测/日志查询触发)
 ******************************************************************************
 */
static void IotGWE_CycleDetectMeterRecordUpload(void)
{
    uint8_t port;
    uint8_t sendFlag, sendEnable;

    do {

        if (pIotGWECtx->meterRecordUploadActive != TRUE)
        {
            break;
        }

        port = pIotGWECtx->meterRecordUploadPort;
        if (port >= SYSCFG_CFG_GUN_NUM)
        {
            break;
        }

        sendFlag = Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);
        sendEnable = Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);

        if (sendFlag == FALSE && sendEnable == FALSE)
        {
            IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

            if (pIotGWECtx->meterRecordCursorTime == 0)
            {/* 无更多记录, 上传完成 */
                pIotGWECtx->meterRecordUploadActive = FALSE;
                pOfflineClr->logQueryEvtSum = 0;
                pOfflineClr->logQueryEvtNo = 0;
                IOTGWE_CFG_DebugPrint("[GWE] meterRecord upload completed\r\n");
            }
            else
            {/* 触发下一帧上报 */
                pOfflineClr->logQueryEvtNo++;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  故障告警逐帧上传检测 (日志查询触发)
 ******************************************************************************
 */
static void IotGWE_CycleDetectFaultRecordUpload(void)
{
    uint8_t port;
    uint8_t sendFlag, sendEnable;

    do {

        if (pIotGWECtx->faultRecordUploadActive != TRUE)
        {
            break;
        }

        port = pIotGWECtx->faultRecordUploadPort;
        if (port >= SYSCFG_CFG_GUN_NUM)
        {
            break;
        }

        sendFlag = Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);
        sendEnable = Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);

        if (sendFlag == FALSE && sendEnable == FALSE)
        {
            IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

            if (pIotGWECtx->faultRecordCursorTime == 0)
            {/* 无更多记录, 上传完成 */
                pIotGWECtx->faultRecordUploadActive = FALSE;
                pOfflineClr->logQueryEvtSum = 0;
                pOfflineClr->logQueryEvtNo = 0;
                IOTGWE_CFG_DebugPrint("[GWE] faultRecord upload completed\r\n");
            }
            else
            {/* 触发下一帧上报 */
                pOfflineClr->logQueryEvtNo++;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  交易记录逐帧上传检测 (日志查询触发)
 ******************************************************************************
 */
static void IotGWE_CycleDetectTradeRecordUpload(void)
{
    uint8_t port;
    uint8_t sendFlag, sendEnable;

    do {

        if (pIotGWECtx->tradeRecordUploadActive != TRUE)
        {
            break;
        }

        port = pIotGWECtx->tradeRecordUploadPort;
        if (port >= SYSCFG_CFG_GUN_NUM)
        {
            break;
        }

        sendFlag = Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);
        sendEnable = Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);

        if (sendFlag == FALSE && sendEnable == FALSE)
        {
            IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

            if (pIotGWECtx->tradeRecordCursorTime == 0)
            {/* 无更多记录, 上传完成 */
                pIotGWECtx->tradeRecordUploadActive = FALSE;
                pOfflineClr->logQueryEvtSum = 0;
                pOfflineClr->logQueryEvtNo = 0;
                IOTGWE_CFG_DebugPrint("[GWE] tradeRecord upload completed\r\n");
            }
            else
            {/* 触发下一帧上报 */
                pOfflineClr->logQueryEvtNo++;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  交易记录召测全部记录逐条上传 (askType=12, orderTwUpdateEvt)
 *         按时间范围从 startTs 向上迭代
 ******************************************************************************
 */
static void IotGWE_CycleDetectTradeRecordNvmUpload(void)
{
    uint8_t port;
    uint8_t sendFlag, sendEnable;

    do {

        if (pIotGWECtx->tradeRecordAskAllActive != TRUE)
        {
            break;
        }

        port = pIotGWECtx->tradeRecordAskAllPort;
        if (port >= SYSCFG_CFG_GUN_NUM)
        {
            break;
        }

        /* 检查是否有正在发送的 */
        sendFlag = Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ);
        sendEnable = Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ);
        if (sendFlag == TRUE || sendEnable == TRUE)
        {
            break;
        }

        if (pIotGWECtx->orderReportAwaitTick != 0)
        {
            if (Common_JudgeTimeoutMs(pIotGWECtx->orderReportAwaitTick, IOTGWE_ORDERCHECK_WAIT_TIMEOUT_MS))
            {/* 上传交易记录确认应答超时 */
                pIotGWECtx->orderReportAwaitTick = 0;
                pIotGWECtx->tradeRecordAskAllActive = FALSE;
            }
            break;
        }

        /* 寻找下一条记录 */
        {
            MSNvmOrderInfo_Struct rec;
            uint32_t cursor = pIotGWECtx->tradeRecordAskAllCursor;
            uint32_t latestIdx = MSNvm_QueryRecordLatestTime(eMSNvmBlockID_OrderRecord);
            uint8_t  found = FALSE;
            uint32_t idx;

            for (idx = 1; idx <= latestIdx; idx++)
            {
                if (eGlobalRet_OK != MSNvm_QueryRecordByTime(eMSNvmBlockID_OrderRecord, (uint8_t *)&rec, sizeof(rec), idx))
                    continue;

                uint32_t ts = rec.platOrderInfo.stGWEOrderInfo.startTime;
                if (ts >= cursor && ts <= pIotGWECtx->tradeRecordAskAllStopTs)
                {/* 本条订单记录的开始时间在查询范围内 */
                    pIotGWECtx->tradeRecordAskAllCursor = ts + 1;/* 左边界右移, 不然之前的记录会一直匹配！*/
                    pIotGWECtx->reportingRecordTime = idx;
                    found = TRUE;
                    break;
                }
            }

            if (found == FALSE)
            {/* 遍历完成 */
                pIotGWECtx->tradeRecordAskAllActive = FALSE;
                IOTGWE_CFG_DebugPrint("[GWE] tradeRecordAskAll upload completed\r\n");
                break;
            }
        }

        pIotGWECtx->reportUseRuntime[port] = FALSE;
        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);

    } while (0);
}

/**
 ******************************************************************************
 * @brief  运行日志分片逐帧上传检测 (日志查询触发)
 ******************************************************************************
 */
static void IotGWE_CycleDetectRunLogUpload(void)
{
    uint8_t port = 0;
    uint8_t sendFlag, sendEnable;

    do {

        if (pIotGWECtx->runLogUploadActive != TRUE)
        {
            break;
        }

        if (port >= SYSCFG_CFG_GUN_NUM)
        {
            break;
        }

        sendFlag = Common_GetSendFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);
        sendEnable = Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ);

        if (sendFlag == FALSE && sendEnable == FALSE)
        {
            IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;

            if (pIotGWECtx->runLogCursorIdx == 0)
            {/* 无更多记录或达到100KB上限, 上传完成 */
                pIotGWECtx->runLogUploadActive = FALSE;
                pOfflineClr->logQueryEvtSum = 0;
                pOfflineClr->logQueryEvtNo = 0;
                IOTGWE_CFG_DebugPrint("[GWE] runLog upload completed\r\n");
            }
            else
            {/* 触发下一帧上报 */
                pOfflineClr->logQueryEvtNo++;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_LOG_QUERY_RESULT_REQ, TRUE);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  离线充电超时检测
 ******************************************************************************
 */
static void IotGWE_CycleDetectOfflineCharging(void)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint32_t offlinChaLen = pPrivateParam->stGWEParam.platinfo.offlineChaLen;
    uint8_t port;

    do {

        if (offlinChaLen == 0 || pIotGWECtx->offlineStartTick == 0)
        {
            break;
        }

        if (!Common_JudgeTimeoutMs(pIotGWECtx->offlineStartTick, offlinChaLen * 60 * 1000))
        {
            break;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (AswMonitor_IsOrderIdle(port) != TRUE)
            {
                AswErrhandle_SetErrExsitCallback(port, eSrc_MannulStop);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  有序充电功率调节
 *         根据当前时间匹配策略时段, 调节充电功率
 ******************************************************************************
 */
static void IotGWE_DealOrderlyCharging(void)
{
    uint8_t port;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (pIotGWECtx->OrderlyChargeFlg[port] == FALSE)
        {
            continue;
        }

        IotGWEOrderlyChargeStrategy_Struct *pStrategy = &pIotGWECtx->sOrderlyCharge[port];

        /* 空闲或故障时清除策略, 恢复默认功率 */
        if (AswMonitor_IsOrderIdle(port) == TRUE)
        {
            if (pIotGWECtx->lastOrderlyChargeKw[port] != IOT_GWE_PROILEPOWER_DEFAULT)
            {
                AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, SYSCFG_CFG_MAX_OUTPUT_POWER);
                pIotGWECtx->lastOrderlyChargeKw[port] = IOT_GWE_PROILEPOWER_DEFAULT;
                IOTGWE_CFG_DebugPrint("[GWE] DealOrderly: gun%u idle, reset to 7.0kW\r\n", port + 1);
            }

            pIotGWECtx->OrderlyChargeFlg[port] = FALSE;
            memset(pStrategy, 0, sizeof(IotGWEOrderlyChargeStrategy_Struct));
            continue;
        }

        if (pStrategy->num == 0)
        {
            continue;
        }

        /* 获取当前北京时间 HHMM (系统时间为北京时间) */
        uint32_t ts = SSTM_GetSecTimestamp();
        CommonDateTime_Struct dt;
        Common_TimestampToDateTime(ts, &dt);
        uint16_t nowHHMM = (uint16_t)dt.hour * 100 + dt.minute;

        /* 匹配时间段 */
        uint8_t matchIdx = 0xFF;
        uint8_t i;
        for (i = 0; i < pStrategy->num; i++)
        {
            uint16_t startHHMM, endHHMM;

            startHHMM = (uint16_t)(pStrategy->validTime[i][0] - '0') * 1000 +
                        (uint16_t)(pStrategy->validTime[i][1] - '0') * 100 +
                        (uint16_t)(pStrategy->validTime[i][2] - '0') * 10 +
                        (uint16_t)(pStrategy->validTime[i][3] - '0');

            if ((i + 1) < pStrategy->num)
            {
                endHHMM = (uint16_t)(pStrategy->validTime[i + 1][0] - '0') * 1000 +
                          (uint16_t)(pStrategy->validTime[i + 1][1] - '0') * 100 +
                          (uint16_t)(pStrategy->validTime[i + 1][2] - '0') * 10 +
                          (uint16_t)(pStrategy->validTime[i + 1][3] - '0');
            }
            else
            {
                endHHMM = 2400;
            }

            if (nowHHMM >= startHHMM && nowHHMM < endHHMM)
            {
                matchIdx = i;
                break;
            }
        }

        uint16_t targetKw;
        if (matchIdx != 0xFF)
        {
            targetKw = pStrategy->kw[matchIdx];
        }
        else
        {
            targetKw = IOT_GWE_PROILEPOWER_DEFAULT;  /* 不在任何策略时段内, 恢复到7.0kW */
        }

        if (targetKw != pIotGWECtx->lastOrderlyChargeKw[port])
        {/* 调节功率输出 */
            AswChargeIf_AdjustOutputCurrent(port, ASWCHARGEIF_ADJUST_POWER_ABSOLUTE, IOT_GWE_KW_TO_W(targetKw));
            pIotGWECtx->lastOrderlyChargeKw[port] = targetKw;
            IOTGWE_CFG_InfoPrint("[GWE] DealOrderly: gun%u, target=%u.%ukW, HHMM=%u\r\n", port + 1, targetKw / 10, targetKw % 10, nowHHMM);
        }
    }
}

/**
 ******************************************************************************
 * @brief  计费模型请求重试
 *         策略: <=4每10秒, >4每2h
 * @param[in] sysTick 当前系统tick
 ******************************************************************************
 */
static void IotGWE_CycleDetectFeemodelRetry(uint32_t sysTick)
{
    uint32_t interval;

    do {

        if (pIotGWECtx->feeModelReceived)
            break;

        interval = (pIotGWECtx->feeModelReqCnt <= 4) ? (10 * 1000) : (2 * 3600 * 1000);

        if (Common_JudgeTimeoutMs(pIotGWECtx->lastFeeModelReqTick, interval) == FALSE)
            break;

        pIotGWECtx->lastFeeModelReqTick = sysTick;
        if (pIotGWECtx->feeModelReqCnt <= 4)
        {
            pIotGWECtx->feeModelReqCnt++;
        }

        Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_FEEMODEL_REQ, TRUE);
        Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_FEEMODEL_REQ, TRUE);
        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_ASK_FEEMODEL_REQ, sysTick);

    } while (0);
}

/**
 ******************************************************************************
 * @brief  交易记录召测上传
 ******************************************************************************
 */
static void IotGWE_CycleDetectUnreportedOrder(void)
{
    uint8_t port = 0;
    uint8_t sending = FALSE;
    MSNvmOrderInfo_Struct stOrderInfo;
    uint32_t time = 0;

    do {

        if (pIotGWECtx->tradeRecordUploadFlag != TRUE)
        {
            break;
        }

        /* 检查是否有正在上报的交易记录(避免重复触发) */
        sending = FALSE;
        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ) ||
                Common_GetRecvTimerEnable(pIotGWECtx->pFuncRecvCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_RSP))
            {
                sending = TRUE;
                break;
            }
        }

        if (sending == TRUE)
        {
            break;
        }

        if (pIotGWECtx->orderReportAwaitTick != 0)
        {
            if (Common_JudgeTimeoutMs(pIotGWECtx->orderReportAwaitTick, IOTGWE_ORDERCHECK_WAIT_TIMEOUT_MS))
            {/* 上笔交易记录上报应答确认超时, 则放弃本次上报, 并清空在报上下文 */
                pIotGWECtx->orderReportAwaitTick = 0;
                pIotGWECtx->reportingPreTradeNo[0] = '\0';
                for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
                {
                    pIotGWECtx->reportUseRuntime[port] = FALSE;
                }
                pIotGWECtx->tradeRecordUploadFlag = FALSE;
            }
            break;
        }

        /* 取出下一条未上报记录 */
        if (MSNvm_QueryUnreportedRecordCount(eMSNvmBlockID_OrderRecord) == 0)
        {
            pIotGWECtx->tradeRecordUploadFlag = FALSE;
            break;
        }

        if (eGlobalRet_OK == MSNvm_QueryLatestUnreportedRecord(eMSNvmBlockID_OrderRecord,
            (uint8_t *)&stOrderInfo, sizeof(MSNvmOrderInfo_Struct), &time))
        {
            port = stOrderInfo.port;

            if (port < SYSCFG_CFG_GUN_NUM &&
                stOrderInfo.protocolType == eAswPlatType_GWE &&
                stOrderInfo.orderSaveState == ASWMONITOR_ORDER_SAVE_STOP)
            {
                pIotGWECtx->reportingRecordTime = time;
                pIotGWECtx->reportUseRuntime[port] = FALSE;
                strncpy(pIotGWECtx->reportingPreTradeNo, stOrderInfo.platOrderInfo.stGWEOrderInfo.preTradeNo, sizeof(pIotGWECtx->reportingPreTradeNo) - 1);
                pIotGWECtx->orderReportAwaitTick = Common_GetSystick();
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
            }
            else
            {
                MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OrderRecord, time);
            }
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  OTA进度处理
 ******************************************************************************
 */
static void IotGWE_OTAProgress(void)
{
    if (pIotGWECtx->otaState == eIotGWEOTAState_Starting)
    {
        if (SSUcm_IsUpdating() == TRUE)
        {
            int8_t curProgress = (int8_t)SSUcm_GetProgress();
            if (curProgress != pIotGWECtx->lastOtaProgress)
            {
                pIotGWECtx->lastOtaProgress = curProgress;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_OTA_PROGRESS_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_OTA_PROGRESS_REQ, TRUE);
            }
        }
        else
        {
            SSUcmResult_Enum result = SSUcm_GetResult();
            if (result != eSSUcmResult_None)
            {/* ota结束 */
                pIotGWECtx->otaEndResult = (uint8_t)result;
                pIotGWECtx->lastOtaProgress = IOTGWE_OTAPROGRESS_DEFAULT;
                Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_OTA_PROGRESS_REQ, TRUE);
                Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, 0, IOT_GWE_CMD_OTA_PROGRESS_REQ, TRUE);
                pIotGWECtx->otaState = eIotGWEOTAState_Idle;
            }
        }
    }
}

/**
 ******************************************************************************
 * @brief  平台连接中处理
 ******************************************************************************
 */
static void IotGWE_WSNormalHandle(void)
{
    if (FALSE == CddNetM_CheckLinkConnectOK(eCddNetMPlatType_O))
    {
        IotGWE_WSOfflineHandle();
    }
    else
    {
        pIotGWECtx->offlineStartTick = 0;

        if (pIotGWECtx->loginSucc == TRUE)
        {
            IotGWE_CycleDetect();
        }

        IotGWE_UpCtrlSendDeal();

        IotGWE_UpCtrlRecvDeal();

        IotGWE_TimeoutDetect();
    }
}

/**
 ******************************************************************************
 * @brief  MQTT连接成功回调
 ******************************************************************************
 */
static void IotGWE_MqttConnectCallback(uint8_t connectResult, uint8_t *pCredential)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    uint8_t flag = FALSE;

    if (connectResult == TRUE)
    {
        pIotGWECtx->loginSucc = TRUE;
        flag = pPlatInfo->credentialSaveFlag;
        AswErrhandle_ResetErrExsitCallback(0, eErr_PlatformOffline);
        IOTGWE_CFG_DebugPrint("[国网e充电] MQTT连接成功, 设备注册完成\r\n");
    }
    else
    {
        IOTGWE_CFG_DebugPrint("[国网e充电] MQTT连接失败\r\n");
    }

    if (pCredential)
    {
        *pCredential = flag;
    }
}

/**
 ******************************************************************************
 * @brief  使用三元组实时计算MQTT认证用户名和密码
 *         参考: eng/dev_sign/dev_sign_mqtt.c::IOT_Sign_MQTT()
 *         username = deviceName&productKey
 *         password = uppercase hex(HMAC-SHA256(deviceSecret, signSource))
 *         signSource = clientId{device_id}deviceName{name}productKey{key}timestamp{ts}
 ******************************************************************************
 */
static void IotGWE_CalcMqttAuth(char *pOutUsername, uint8_t usernameSize,
                                    char *pOutPassword, uint16_t passwordSize)
{

    char deviceId[65] = {0};
    char signSource[256] = {0};
    uint8_t signHex[32] = {0};
    uint8_t i;

    snprintf(deviceId, sizeof(deviceId), "%s.%s", pIotGWECtx->productKey, pIotGWECtx->deviceName);
    snprintf(pOutUsername, usernameSize, "%s&%s", pIotGWECtx->deviceName, pIotGWECtx->productKey);
    snprintf(signSource, sizeof(signSource), "clientId%sdeviceName%sproductKey%stimestamp%s",
            deviceId, pIotGWECtx->deviceName, pIotGWECtx->productKey, IOTGWE_SIGN_TIMESTAMP);

    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        (const unsigned char *)pIotGWECtx->deviceSecret, strlen(pIotGWECtx->deviceSecret),
        (const unsigned char *)signSource, strlen(signSource), signHex);

    for (i = 0; i < 32; i++)
    {
        snprintf(pOutPassword + (i * 2), 3, "%02X", signHex[i]);
    }
}

/**
 ******************************************************************************
 * @brief  填充MQTT连接参数: ip, port, topics, 队列ID
 ******************************************************************************
 */
uint8_t IotGWE_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    uint8_t ret = FALSE;

    if (pLinkPara != NULL && pIotGWECtx != NULL)
    {
#ifdef IOTGWE_IS_DEBUG
        /* 调试模式: 先将三元组写入NVM并同步到context, 确保后续MQTT认证和Topic订阅使用正确凭证 */
        strcpy(pPrivateParam->stGWEParam.platinfo.cProductKey, IOTGWE_DEV_PRODUCTKEY);
        strcpy(pPrivateParam->stGWEParam.platinfo.cDeviceName,   IOTGWE_DEV_DEVICENAME);
        strcpy(pPrivateParam->stGWEParam.platinfo.cDeviceSecret, IOTGWE_DEV_DEVICESECRET);
        pPrivateParam->stGWEParam.platinfo.credentialSaveFlag = TRUE;
        MSNvm_WriteParaBlock(eMSNvmBlockID_PlatPrivateParam, (uint8_t *)pPrivateParam, sizeof(MSNvmPlatPrivateParam_Union));
#endif
        /* 从NVM同步凭证 */
        memcpy(pIotGWECtx->productKey,  pPrivateParam->stGWEParam.platinfo.cProductKey,   MSNVM_GWE_PRODUCT_KEY_LEN);
        memcpy(pIotGWECtx->deviceName,  pPrivateParam->stGWEParam.platinfo.cDeviceName,   MSNVM_GWE_DEVICE_NAME_LEN);
        memcpy(pIotGWECtx->deviceSecret,pPrivateParam->stGWEParam.platinfo.cDeviceSecret, MSNVM_GWE_DEVICE_SECRET_LEN);

#ifdef IOTGWE_IS_DEBUG
        strcpy(pLinkPara->stMqttPara.ip, IOTGWE_DEV_MQTT_IP);
        pLinkPara->stMqttPara.port = IOTGWE_DEV_MQTT_PORT;
        ret = TRUE;
#else
        if (pPrivateParam->stGWEParam.platinfo.credentialSaveFlag == TRUE)
        {
            strncpy(pLinkPara->stMqttPara.ip, pParam->platMainIp, sizeof(pLinkPara->stMqttPara.ip));
            pLinkPara->stMqttPara.port = pParam->platMainPort;
            ret = TRUE;
        }
#endif

        pLinkPara->stMqttPara.eVersion = eCddNetMMqttVersion_V3_1_1;
        pLinkPara->stMqttPara.keepAliveTime = IOTGWE_HEATBEAT_PERIOD;

        if (pPrivateParam->stGWEParam.platinfo.credentialSaveFlag == TRUE)
        {
            IotGWE_CalcMqttAuth(pLinkPara->stMqttPara.userName, CDD_NETM_CFG_MQTT_USER_NAME_LEN,
                                pLinkPara->stMqttPara.password, CDD_NETM_CFG_MQTT_PASSWORD_LEN);
        }

        /* MQTT clientId 格式: {pk}.{dn}|timestamp=...,_v=sdk-c-...,securemode=3,signmethod=hmacsha256,lan=C,gw=0,ext=0| */
#ifdef IOTGWE_IS_DEBUG
        snprintf(pLinkPara->stMqttPara.pid, sizeof(pLinkPara->stMqttPara.pid),
            "%s.%s|timestamp=%s,_v=sdk-c-1.1.10,securemode=3,signmethod=hmacsha256,lan=C,gw=0,ext=0|",
            pIotGWECtx->productKey, pIotGWECtx->deviceName, IOTGWE_SIGN_TIMESTAMP);
#else
        snprintf(pLinkPara->stMqttPara.pid, sizeof(pLinkPara->stMqttPara.pid),
            "%s.%s|timestamp=%s,_v=sdk-c-1.1.10,securemode=8,signmethod=hmacsha256,lan=C,gw=0,ext=0|",
            pIotGWECtx->productKey, pIotGWECtx->deviceName, IOTGWE_SIGN_TIMESTAMP);
#endif
        /* 订阅主题Topic */
        pLinkPara->stMqttPara.topicCount = 0;
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_OTA_UPGRADE, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_OTA_FIRMWARE_REPLY, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_NTP_RESPONSE, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_PROPERTY_POST_REPLY, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_PROPERTY_SET, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_SERVICE, pIotGWECtx->productKey, pIotGWECtx->deviceName);
        snprintf(pLinkPara->stMqttPara.topic[pLinkPara->stMqttPara.topicCount++], CDD_NETM_CFG_MQTT_TOPIC_LEN + 1,
                IOT_GWE_SUB_EVENT_REALY, pIotGWECtx->productKey, pIotGWECtx->deviceName);

        pLinkPara->stMqttPara.pFuncMqttConnectCallback = IotGWE_MqttConnectCallback;

        FrameQueue_Creat(eFrameQueueType_MQTT, 3072, 3072, &pIotGWECtx->frameQueueChannelID);
        pLinkPara->stMqttPara.frameQueueChannelID = pIotGWECtx->frameQueueChannelID;
    }

    return ret;
}

/**
 ******************************************************************************
 * @brief  设置平台离线
 ******************************************************************************
 */
void IotGWE_OfflineHandle(void)
{
    uint8_t port;

    CddNetM_SetLinkDisconnect(eCddNetMPlatType_O);
    pIotGWECtx->loginSucc = FALSE;
    pIotGWECtx->eWorkState = eIOTGWEWorkState_Offline;

    /* 记录离线时刻, 用于离线充电超时控制 */
    if (pIotGWECtx->offlineStartTick == 0)
    {
        pIotGWECtx->offlineStartTick = Common_GetSystick();
    }

    pIotGWECtx->tradeRecordUploadFlag = FALSE;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pIotGWECtx->reportUseRuntime[port] = FALSE;
    }

    pIotGWECtx->orderReportAwaitTick = 0;
    pIotGWECtx->reportingPreTradeNo[0] = '\0';

    pIotGWECtx->meterRecordUploadActive = FALSE;
    pIotGWECtx->meterRecordCursorTime = 0;

    pIotGWECtx->faultRecordUploadActive = FALSE;
    pIotGWECtx->faultRecordCursorTime = 0;

    pIotGWECtx->tradeRecordUploadActive = FALSE;
    pIotGWECtx->tradeRecordCursorTime = 0;

    pIotGWECtx->tradeRecordAskAllActive = FALSE;
    pIotGWECtx->tradeRecordAskAllCursor = 0;

    pIotGWECtx->runLogUploadActive = FALSE;
    pIotGWECtx->runLogCursorIdx = 0;
    pIotGWECtx->runLogByteOffset = 0;
    pIotGWECtx->runLogSentBytes = 0;

    pIotGWECtx->acOutMeterTick = 0;
    pIotGWECtx->acOutMeterLastHour = 0xFF;

    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pIotGWECtx->lastErrVersion[port] = 0;
    }
    IotGWE_ResetErrStatus();

    /* 清除有序充电策略, 恢复默认功率 */
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        pIotGWECtx->OrderlyChargeFlg[port] = FALSE;
        pIotGWECtx->lastOrderlyChargeKw[port] = IOT_GWE_PROILEPOWER_DEFAULT;
        memset(&pIotGWECtx->sOrderlyCharge[port], 0, sizeof(IotGWEOrderlyChargeStrategy_Struct));
    }
}

/**
 ******************************************************************************
 * @brief  计费模型转换: NVM -> Monitor运行时格式
 ******************************************************************************
 */
void IotGWE_TransformBillMode(uint8_t port, AswMonitorBillMode_Struct *pBillMode)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEParamBillMode_Struct *pSrc = &pPrivateParam->stGWEParam.stBillMode;
    uint8_t i = 0;
    uint8_t count = 0;

    do {

        if (pBillMode == NULL)
        {
            break;
        }

        if (pSrc->validFlag == 0)
        {
            break;
        }

        if (pSrc->periodCount == 0)
        {
            break;
        }

        memset(pBillMode, 0, sizeof(AswMonitorBillMode_Struct));
        memcpy(pBillMode->billModeID, pSrc->billModeID, sizeof(pBillMode->billModeID));
        pBillMode->billmodeType = ASWMONITOR_BILLMODE_TYPE_MULT;
        pBillMode->validFlag = 1;

        count = pSrc->periodCount;
        pBillMode->rateCount = count;
        pBillMode->periodCount = count;

        for (i = 0; i < count; i++)
        {
            pBillMode->startTime[i][0] = pSrc->startTime[i][0];
            pBillMode->startTime[i][1] = pSrc->startTime[i][1];

            if (i + 1 < count)
            {
                pBillMode->stopTime[i][0] = pSrc->startTime[i + 1][0];
                pBillMode->stopTime[i][1] = pSrc->startTime[i + 1][1];
            }
            else
            {
                pBillMode->stopTime[i][0] = 24;
                pBillMode->stopTime[i][1] = 0;
            }

            pBillMode->totalPrice[i]     = pSrc->elecPrice[i] + pSrc->servPrice[i];
            pBillMode->rateElecPrice[i]  = pSrc->elecPrice[i];
            pBillMode->rateSeverPrice[i] = pSrc->servPrice[i];
            pBillMode->periodRate[i]     = i;
            pBillMode->elecLossRate = 0;
        }

    } while (0);
}

/**
 ******************************************************************************
 * @brief  计算跨越点电量: 将时段电量分配到15分钟时隙
 * @param  pChargeData   充电实时数据
 * @param  pBillMode     当前计费模型
 * @param  pointsElect   [out] 96×2字节跨越点电量数组
 * @param  pStartPoint   [out] 起始点标识(1-96)
 * @param  pCrossPoints  [out] 跨越点数
 ******************************************************************************
 */
void IotGWE_ComputePointsElect(AswMonitorChargeData_Struct *pChargeData, AswMonitorBillMode_Struct *pBillMode,
                                uint8_t (*pointsElect)[2], uint8_t *pStartPoint, uint8_t *pCrossPoints)
{
    CommonDateTime_Struct beginDt;
    uint16_t beginMin, absoluteEndMin;
    uint16_t beginSlot, endSlotAbs;
    uint32_t chargeDurationSec;
    uint16_t chargeDurationMin;
    uint32_t beginSecondOfDay, endSecondOfDay;
    uint8_t i;
    uint16_t k;

    if (pChargeData == NULL || pBillMode == NULL || pointsElect == NULL
        || pStartPoint == NULL || pCrossPoints == NULL
        || pChargeData->chargeStartTime == 0)
    {
        if (pStartPoint != NULL) *pStartPoint = 0;
        if (pCrossPoints != NULL) *pCrossPoints = 0;
        return;
    }

    Common_TimestampToDateTime(pChargeData->chargeStartTime, &beginDt);
    beginMin = (uint16_t)beginDt.hour * 60 + beginDt.minute;
    beginSecondOfDay = (uint32_t)beginDt.hour * 3600U + beginDt.minute * 60U + beginDt.second;

    chargeDurationSec = pChargeData->chargeStopTime - pChargeData->chargeStartTime;
    chargeDurationMin = (uint16_t)((chargeDurationSec + 59U) / 60U);/* 避免充电时长小于1分钟 */

    absoluteEndMin = beginMin + chargeDurationMin;
    endSecondOfDay = beginSecondOfDay + chargeDurationSec;

    beginSlot  = beginMin / 15U;
    endSlotAbs = absoluteEndMin / 15U;

    if ((endSecondOfDay % 900U) == 0 && endSlotAbs > beginSlot)
    {
        endSlotAbs--;
    }
    /* 计算起始点和跨越时段数 */
    *pStartPoint = (uint8_t)(beginSlot + 1U);
    *pCrossPoints = (uint8_t)(endSlotAbs - beginSlot + 1U);
    if (*pCrossPoints > 96U)
    {
        *pCrossPoints = 96U;
    }

    memset(pointsElect, 0, 96U * 2U);

    for (i = 0; i < pBillMode->periodCount && i < ASWMONITOR_BILLMODE_PERIOD_COUNT; i++)
    {
        if (pChargeData->periodValidFlag[i] != TRUE)
            continue;

        uint16_t segStartMin = (uint16_t)pBillMode->startTime[i][0] * 60 + pBillMode->startTime[i][1];
        uint16_t segStopMin  = (uint16_t)pBillMode->stopTime[i][0] * 60 + pBillMode->stopTime[i][1];
        if (segStopMin == 0)
        {
            segStopMin = 24U * 60U;
        }

        uint16_t maxDay = absoluteEndMin / (24U * 60U);
        uint16_t dayOffset;

        for (dayOffset = 0; dayOffset <= maxDay; dayOffset++)
        {/* 支持跨天叠加 */
            uint16_t dayBegin = dayOffset * 24U * 60U;
            uint16_t segStartAbs = dayBegin + segStartMin;
            uint16_t segStopAbs  = dayBegin + segStopMin;

            uint16_t overlapStart = (beginMin > segStartAbs) ? beginMin : segStartAbs;
            uint16_t overlapEnd   = (absoluteEndMin < segStopAbs) ? absoluteEndMin : segStopAbs;

            if (overlapEnd <= overlapStart)
                continue;

            uint16_t totalOverlapMin = overlapEnd - overlapStart;
            uint32_t totalAllocated = 0U;
            uint8_t  lastCoveredIdx = 0U;

            /* 计算充电最后一个15分钟槽位: 用于后面吃余数 */
            uint16_t lastOverlapMin = (absoluteEndMin < segStopAbs) ? (absoluteEndMin - 1U) : (segStopAbs - 1U);
            if (lastOverlapMin >= segStartAbs)
            {
                lastCoveredIdx = (uint8_t)((lastOverlapMin / 15U) % 96U);
            }

            for (k = segStartMin / 15U; k < segStopMin / 15U && k < 96U; k++)
            {
                uint16_t slotStartAbs = dayBegin + (uint16_t)k * 15U;
                uint16_t slotEndAbs   = dayBegin + (uint16_t)(k + 1U) * 15U;

                if (slotStartAbs >= absoluteEndMin || slotEndAbs <= beginMin)
                    continue;

                uint16_t slotOverlapStart = (overlapStart > slotStartAbs) ? overlapStart : slotStartAbs;
                uint16_t slotOverlapEnd   = (overlapEnd   < slotEndAbs)   ? overlapEnd   : slotEndAbs;
                uint16_t slotOverlapMin = slotOverlapEnd - slotOverlapStart;

                uint8_t idx = (uint8_t)(k % 96U);
                uint32_t slotEnergy;

                if (idx == lastCoveredIdx)
                {
                    slotEnergy = (uint32_t)pChargeData->periodElePower[i] - totalAllocated;
                }
                else
                {
                    slotEnergy = (uint32_t)pChargeData->periodElePower[i] * slotOverlapMin / totalOverlapMin;
                    totalAllocated += slotEnergy;
                }

                uint32_t existing = Common_TwoUint8ToUint16(pointsElect[idx]);
                uint32_t acc = existing + slotEnergy;
                if (acc > 65535UL)
                {
                    acc = 65535UL;
                }
                Common_Uint32ToTwoUint8(pointsElect[idx], acc);
            }
        }
    }
}

/**
 ******************************************************************************
 * @brief  打包充电记录: Monitor运行时 → NVM订单结构体
 *         分START/STOP阶段填充MSNvmGWEOrderInfo_Struct
 ******************************************************************************
 */
void IotGWE_PackChargeRecord(uint8_t port, MSNvmOrderInfo_Struct *pOrderData, uint8_t orderSaveReason)
{
    MSNvmPlatPrivateParam_Union *pPrivateParam = AswPlatM_GetPlatPrivateParamPtr();
    MSNvmGWEPlatInfo_Struct *pPlatInfo = &pPrivateParam->stGWEParam.platinfo;
    AswMonitorBillMode_Struct *pBillMode = AswMonitor_GetCurUsedBillModePtr(port);
    AswMonitorChargeData_Struct *pChargeData = AswMonitor_GetChargeDataPtr(port);
    MSNvmGWEOrderInfo_Struct *pGWEOrder = &pOrderData->platOrderInfo.stGWEOrderInfo;
    uint8_t i = 0;
    CommonDateTime_Struct dt;
    uint16_t startMinOfDay = 0;

    if (orderSaveReason == ASWMONITOR_ORDER_SAVE_START)
    {
        pIotGWECtx->reportUseRuntime[port] = FALSE;
        pGWEOrder->sumStart = pChargeData->startMeterVal;
        pOrderData->port = port;
        pOrderData->protocolType = eAswPlatType_GWE;
        pOrderData->orderLen = sizeof(MSNvmGWEOrderInfo_Struct);
        pGWEOrder->stopReason = IOTGWE_STOPREASON_POWEROFF_MARK;
        /* 计算起始点标识(开始时段) */
        Common_TimestampToDateTime(pChargeData->chargeStartTime, &dt);
        startMinOfDay = (uint16_t)dt.hour * 60 + dt.minute;
        pGWEOrder->startPoint = startMinOfDay / 15 + 1;

#ifdef IOTGWE_CFG_ACWORK_FAST_REPORT
        /* 充电启动后快速上报模式: 1分钟内每5s上报acGunRunItyData */
        pIotGWECtx->acWorkFastFlag[port] = TRUE;
        pIotGWECtx->acWorkFastStartTick[port] = Common_GetSystick();
        Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_WORK_REQ, 0);
#endif
    }
    else if (orderSaveReason == ASWMONITOR_ORDER_SAVE_STOP)
    {
        pGWEOrder->stopReason = (uint8_t)pChargeData->eChargeStopReason;

        if (pIotGWECtx->loginSucc == TRUE)
        {
            pIotGWECtx->reportUseRuntime[port] = TRUE;
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_ORDER_TW_UPDATE_REQ, TRUE);

            /* 同步上报停止充电结果 */
            IotGWEDataOfflineClr_Struct *pOfflineClr = &pIotGWECtx->stProtoData.stRecvData[port].offlineClearData;
            if (pOfflineClr->stopResultCode == 0)
            {/* 桩端主动停止 */
                pOfflineClr->stopResult = eIotGWEStartResult_Success;
                pOfflineClr->stopResultCode = IotGWE_MapStopReason(pChargeData->eChargeStopReason);
                pOfflineClr->stopFailReson = 0;
            }
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_STOP_CHA_RES_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_STOP_CHA_RES_REQ, TRUE);

            /* 立即上报非充电属性, 实时更新平台工作状态 */
            Common_SetSendEnable(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, TRUE);
            Common_SetSendImmdFlag(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, TRUE);
            Common_SetSendTick(pIotGWECtx->pFuncSendCtrl, port, IOT_GWE_CMD_PROPERTY_AC_NONWORK_REQ, Common_GetSystick());
        }
    }
    else
    {}

    /* 充电过程中更新 */
    pGWEOrder->endTime = pChargeData->chargeStopTime;
    pGWEOrder->sumEnd = pChargeData->stopMeterVal;

    /* 时段电量, 电费,服务费 */
    {
        uint8_t periodCount = pPrivateParam->stGWEParam.stBillMode.periodCount;

        pGWEOrder->timeNum = periodCount;
        for (i = 0; i < periodCount; i++)
        {
            Common_Uint32ToThreeUint8(pGWEOrder->partElect[i],  pChargeData->periodElePower[i]);
            Common_Uint32ToThreeUint8(pGWEOrder->chargeFee[i],  pChargeData->periodEleMoney[i]);
            Common_Uint32ToThreeUint8(pGWEOrder->serviceFee[i], pChargeData->periodSerMoney[i]);
        }

        pGWEOrder->totalElec      = pChargeData->totalEnergy;
        pGWEOrder->totalPowerCost = pChargeData->totalElecMoney;
        pGWEOrder->totalServCost  = pChargeData->totalServeMoney;
    }

    /* 跨越点电量及跨越点数 */
    IotGWE_ComputePointsElect(pChargeData, pBillMode, pGWEOrder->pointsElect, &pGWEOrder->startPoint, &pGWEOrder->crossPoints);
}

/**
 ******************************************************************************
 * @brief  交易记录转换: NVM订单 -> 协议上报二进制格式(用于上报到运维平台)
 * @note   其实国网e充电是专网,连不上运维平台的, 这里只是为了框架完整性
 ******************************************************************************
 */
void IotGWE_TransformChargeRecord(MSNvmPlatOrderInfo_Union *pFlashRecord, uint8_t *pProtocolRecord, uint16_t *pRecordLen)
{
    if (pFlashRecord != NULL && pProtocolRecord != NULL && pRecordLen != NULL)
    {
        memcpy(pProtocolRecord, &pFlashRecord->stGWEOrderInfo, sizeof(MSNvmGWEOrderInfo_Struct));
        pRecordLen[0] = sizeof(MSNvmGWEOrderInfo_Struct);
    }
}

/**
 ******************************************************************************
 * @brief  恢复出厂设置时初始化GWE私有参数
 ******************************************************************************
 */
void IotGWE_SetPrivateParam(MSNvmPlatPrivateParam_Union *pPrivateParam)
{
    if (pPrivateParam)
    {
        pPrivateParam->stGWEParam.platinfo.credentialSaveFlag = FALSE;

        pPrivateParam->stGWEParam.platinfo.equipParamReportCycle = IOTGWE_CFG_EQUIP_PARAM_FREQ_DEFAULT;
        pPrivateParam->stGWEParam.platinfo.gunElecReportCycle    = IOTGWE_CFG_GUN_ELEC_FREQ_DEFAULT;
        pPrivateParam->stGWEParam.platinfo.nonElecReportCycle    = IOTGWE_CFG_NON_ELEC_FREQ_DEFAULT;
        pPrivateParam->stGWEParam.platinfo.faultWarningsCycle    = IOTGWE_CFG_FAULT_WARNINGS_DEFAULT;
        pPrivateParam->stGWEParam.platinfo.offlineChaLen         = IOTGWE_CFG_OFFLIN_CHALEN_DEFAULT;
    }
}

/**
 ******************************************************************************
 * @brief  初始化内存变量
 ******************************************************************************
 */
void IotGWE_InitMemory(void)
{
    uint8_t envFlag = 0;

    pIotGWECtx = (IotGWECtx_Struct *)myMalloc(sizeof(IotGWECtx_Struct));
    if (pIotGWECtx != NULL)
    {
        memset(pIotGWECtx, 0, sizeof(IotGWECtx_Struct));

        pIotGWECtx->acOutMeterLastHour = 0xFF;  /* 防止上电首小时(0点)误判重复 */
        pIotGWECtx->pFuncSendCtrl = IotGWE_GetSendCtrl;
        pIotGWECtx->pFuncRecvCtrl = IotGWE_GetRecvCtrl;

        AswPlatM_GetEnvFlag(&envFlag);
        pIotGWECtx->regBaseUrl = IOTGWE_ENV_REGVASE_URL(envFlag);
    }    
}

/**
 ******************************************************************************
 * @brief  工作状态状态机
 ******************************************************************************
 */
static void IotGWE_WortStateFSM(void)
{
    switch (pIotGWECtx->eWorkState)
    {
        case eIOTGWEWorkState_Init:
        {
            IotGWE_WSInitHandle();
            break;
        }
        case eIOTGWEWorkState_Register:
        {
            IotGWE_WSRegisterHandle();
            break;
        }
        case eIOTGWEWorkState_Offline:
        {
            IotGWE_WSOfflineHandle();
            break;
        }
        case eIOTGWEWorkState_Login:
        {
            IotGWE_WSLoginHandle();
            break;
        }
        case eIOTGWEWorkState_Normal:
        {
            IotGWE_WSNormalHandle();
            break;
        }
        default:
        {
            pIotGWECtx->eWorkState = eIOTGWEWorkState_Init;
        }
    }
}

/**
 ******************************************************************************
 * @brief  主循环: 根据连接状态执行相应处理
 ******************************************************************************
 */
void IotGWE_MainFunction(void)
{
    IotGWE_WortStateFSM();

    if (pIotGWECtx->eWorkState != eIOTGWEWorkState_Normal)
    {
        IotGWE_CycleDetectOfflineCharging();
    }
}

