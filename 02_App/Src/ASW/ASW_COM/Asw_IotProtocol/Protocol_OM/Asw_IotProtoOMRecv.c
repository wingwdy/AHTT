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
#include "Asw_IotProtoOMM.h"
#include "Asw_IotProtoOMRecv.h"
#include "FrameQueue.h"
#include "Asw_Monitor.h"
#include "SS_Ucm.h"
#include "Asw_PlatM.h"
#include "Cdd_ModeM.h"
#include "SS_Snapshot.h"
/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define IOT_OM_RecvGunNoTransform(inputPort, outputPort)    do{ \
                                                                if (inputPort > SYSCFG_CFG_GUN_NUM)\
                                                                {\
                                                                    outputPort = 0;\
                                                                }\
                                                                else\
                                                                {\
                                                                    outputPort = (inputPort - 1);\
                                                                }\
                                                            }while(0)


/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t IotOM_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvCallNetModuleInfo(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvSetQrcode(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvSetForbid(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvReportForBidStateRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvUpdate(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvRemoteQueryParam(uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvRemoteSetParam(uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvRemoteQuerySetParam(uint8_t *port, uint8_t *r_data, uint16_t len);
static uint8_t IotOM_RecvReadLocalFile(uint8_t *port, uint8_t *r_data, uint16_t len);
static const IotOMRecvCtrl_Struct* IotOM_GetRecvCtrlPtr(uint16_t cmd);
static IotOMFrameHead_Struct *IotOM_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen);
static void IotOM_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
extern IotOMCtx_Struct *pIotOMCtx;

static const IotOMRecvCtrl_Struct c_stIotOMRecvctrlTable[IOT_OM_CMD_RECV_COUNT] = 
{
    [0] = 
    {
        .cmd = IOT_OM_CMD_LOGIN_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvLoginRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 10,
        .matchCmd = IOT_OM_CMD_LOGIN_REQ,
        .printFlag = TRUE,
        .cMeaning = "登陆应答",
    },

    [1] = 
    {
        .cmd = IOT_OM_CMD_HEARTBEAT_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvHeartBeatRsp,
        .maxTimeout = 30 * 1000,
        .maxTryCnt = 3,
        .matchCmd = IOT_OM_CMD_HEARTBEAT_REQ,
        .printFlag = FALSE,
        .cMeaning = "心跳应答",
    },

    [2] = 
    {
        .cmd = IOT_OM_CMD_CALL_NETMODULE_INFO,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvCallNetModuleInfo,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_CALL_NETMODULE_INFO_RSP,
        .printFlag = TRUE,
        .cMeaning = "请求网络模块信息",
    },

    [3] = 
    {
        .cmd = IOT_OM_CMD_CALL_REALDATA,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvCallRealData,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_CALL_REALDATA_ACK,
        .printFlag = TRUE,
        .cMeaning = "召测实时数据",
    },

    [4] = 
    {
        .cmd = IOT_OM_CMD_SET_QRCODE,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvSetQrcode,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_SET_QRCODE_RSP,
        .printFlag = TRUE,
        .cMeaning = "设置二维码",
    },

    [5] = 
    {
        .cmd = IOT_OM_CMD_REBOOT,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvSetReboot,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_REBOOT_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程重启",
    },

    [6] = 
    {
        .cmd = IOT_OM_CMD_SET_FORBID,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvSetForbid,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_SET_FORBID_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程锁机",
    },

    [7] = 
    {
        .cmd = IOT_OM_CMD_REPORT_FORBID_STATE_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvReportForBidStateRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 29,
        .matchCmd = IOT_OM_CMD_REPORT_FORBID_STATE,
        .printFlag = TRUE,
        .cMeaning = "远程锁机状态上报应答",
    },

    [8] = 
    {
        .cmd = IOT_OM_CMD_UPDATE,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvUpdate,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_UPDATE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程更新",
    },

    [9] = 
    {
        .cmd = IOT_OM_CMD_ORDER_RECORD_RSP,
        .cmdType = IOT_OM_CMDTYPE_RESPONSE,
        .pRecvParse = IotOM_RecvOrderRecordRsp,
        .maxTimeout = 10 * 1000,
        .maxTryCnt = 10,
        .matchCmd = IOT_OM_CMD_ORDER_RECORD,
        .printFlag = TRUE,
        .cMeaning = "订单上报应答",
    },

    [10] = 
    {
        .cmd = IOT_OM_CMD_REMOTE_QUERY_SET_PARAM,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvRemoteQuerySetParam,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_REMOTE_QUERY_SET_PARAM_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程设置查询参数",
    },

    [11] = 
    {
        .cmd = IOT_OM_CMD_CALL_READ_LOCALFILE,
        .cmdType = IOT_OM_CMDTYPE_REQUSET,
        .pRecvParse = IotOM_RecvReadLocalFile,
        .maxTimeout = 0,
        .maxTryCnt = 1,
        .matchCmd = IOT_OM_CMD_CALL_READ_LOCALFILE_RSP,
        .printFlag = TRUE,
        .cMeaning = "远程读取桩本地文件",
    },
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t IotOM_RecvLoginRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    if (pRecvData[index] == 0x00)
    {
        Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_HEARTBEAT_REQ, TRUE);
        Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_SEND_NETMODULE_INFO, TRUE);

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_REPORT_REALDATA, TRUE);            
            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, 0, IOT_OM_CMD_REPORT_METERVAL, TRUE);
            pIotOMCtx->realDataReportTick[gunNo] = Common_GetSystick();
            pIotOMCtx->meterValReportTick[gunNo] = Common_GetSystick();
        }

        pIotOMCtx->loginSucc = TRUE;
        IOTOM_CFG_InfoPrint("[运维平台]登陆成功!\r\n");
    }
    else
    {
        index++;
        IOTOM_CFG_InfoPrint("[运维平台]登陆失败，失败原因：%d!\r\n", pRecvData[index]);
        IotOM_OfflineHandle();
    }

    return TRUE;
}

static uint8_t IotOM_RecvHeartBeatRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    return TRUE;
}

static uint8_t IotOM_RecvCallNetModuleInfo(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    return TRUE;
}

static uint8_t IotOM_RecvCallRealData(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;

    IOT_OM_RecvGunNoTransform(pRecvData[index], port[0]);
    return TRUE;
}

static uint8_t IotOM_RecvSetQrcode(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    uint8_t temp = 0;
    MSNvmDrcode_Struct qrParam = { 0 };
    
    IOT_OM_RecvGunNoTransform(pRecvData[index], port[0]);
    index++;

    if (port[0] == 0)
    {
        memcpy(qrParam.qrcode, &pRecvData[index], 200);
        MSNvm_WriteParaBlock(eMSNvmBlockID_Gun0Qrcode, (uint8_t *)&qrParam, sizeof(MSNvmDrcode_Struct));
        IOTOM_CFG_InfoPrint("[枪：%d]设置的二维码内容：%.200s\r\n", port[0], &pRecvData[index]);
    }

    return TRUE;
}

static uint8_t IotOM_RecvSetReboot(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;
    uint8_t gunNo = 0;

    if (pRecvData[index] == 0x01)
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_Immediate); 
        pIotOMCtx->stProtoData[0].setRebootResult = 0x01;
    }
    else
    {
        AswMonitor_SetReboot(eAswMonitorRebootType_WaitIdle);

        pIotOMCtx->stProtoData[0].setRebootResult = 0x01;

        for (gunNo = 0; gunNo < SYSCFG_CFG_GUN_NUM; gunNo++)
        {
            if (TRUE != AswMonitor_IsOrderIdle(gunNo))
            {
                pIotOMCtx->stProtoData[0].setRebootResult = 0x02;
                break;
            }
        }
    }

    return TRUE;
}

static uint8_t IotOM_RecvSetForbid(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32;
    uint8_t *pRecvData = r_data;

    if (pRecvData[index] == 0x01)
    {
        AswMonitor_SetForbidState(TRUE, pRecvData[index + 1]);
        pIotOMCtx->stProtoData[0].setForbidStateResult = 0x02;
        pIotOMCtx->stProtoData[0].setForbidStateFailReason = 0x00;
        pIotOMCtx->stProtoData[0].setUnforbidStateResult = 0x00;
        pIotOMCtx->stProtoData[0].setUnforbidStateFailReason = 0x00;

    }
    else if (pRecvData[index] == 0x02)
    {
        AswMonitor_SetForbidState(FALSE, pRecvData[index + 2]);
        pIotOMCtx->stProtoData[0].setUnforbidStateResult = 0x02;
        pIotOMCtx->stProtoData[0].setUnforbidStateFailReason = 0x00;
        pIotOMCtx->stProtoData[0].setForbidStateResult = 0x00;
        pIotOMCtx->stProtoData[0].setForbidStateFailReason = 0x00;
    }
    else
    {
        pIotOMCtx->stProtoData[0].setForbidStateResult = 0x00;
        pIotOMCtx->stProtoData[0].setForbidStateFailReason = 0x00;
        pIotOMCtx->stProtoData[0].setUnforbidStateResult = 0x00;
        pIotOMCtx->stProtoData[0].setUnforbidStateFailReason = 0x00;        
    }

    return TRUE;
}

static uint8_t IotOM_RecvReportForBidStateRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    if (pIotOMCtx->sendForbidStateFlag == TRUE)
    {
        pIotOMCtx->sendForbidStateCount++;
    }
    
    return TRUE;
}

static uint8_t IotOM_RecvUpdate(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t index = 32 + 1 + 2;
    uint8_t *pRecvData = r_data;
    uint32_t timeout = 0;
    char path[33] = {0};
    CddNetMSocketPara_Union stSocketPara = {0};

    if (TRUE == SSUcm_IsOngoging())
    {
        pIotOMCtx->stProtoData[0].setUpdateResult = 0x01;
    }
    else
    {
        stSocketPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;
        stSocketPara.stFtpPara.eMode = eCddNetMFtpMode_Download;

        memcpy(stSocketPara.stFtpPara.ip, &pRecvData[index], 16);
        index += 16;
        memcpy(&stSocketPara.stFtpPara.port, &pRecvData[index], 2);
        index += 2;

        memcpy(stSocketPara.stFtpPara.user, &pRecvData[index], 16);
        index += 16;
        memcpy(stSocketPara.stFtpPara.passwd, &pRecvData[index], 16);
        index += 16;
        memcpy(path, &pRecvData[index], 32);
        index += 32;
        Common_ExtractPathAndFileName(path, stSocketPara.stFtpPara.path, sizeof(stSocketPara.stFtpPara.path), 
        stSocketPara.stFtpPara.fileName, sizeof(stSocketPara.stFtpPara.fileName));   

        index += 1;

        timeout = pRecvData[index++] * 60 * 1000;

        if (TRUE == SSUcm_CheckUpdateCondition())
        {
            if (TRUE == SSUcm_CheckUpdateCondition())
            {
                pIotOMCtx->stProtoData[0].setUpdateResult = 0x00;
            }
            else
            {
                pIotOMCtx->stProtoData[0].setUpdateResult = 0x01;
            }
        }
        
        SSUcm_GetResult();
        SSUcm_ReqStartOTA(&stSocketPara, eSSUcmChannelType_FTP, eSSUcmExcuteMode_WaitIdle, timeout);
        pIotOMCtx->stProtoData[0].recvUpdateFlag = TRUE;
    }

    return TRUE;
}

static uint8_t IotOM_RecvOrderRecordRsp(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OmOrderRecord, pIotOMCtx->time);
    IOTOM_CFG_InfoPrint("订单上报成功!\r\n");
    return TRUE;
}

static uint8_t IotOM_RecvRemoteQueryParam(uint8_t *r_data, uint16_t len)
{
    uint8_t result = TRUE;
    uint16_t dataIndex = 32 + 1;
    uint8_t *pRecvData = r_data;
    uint8_t paramCount = 0;
    uint8_t paramIndex = 0;
    uint8_t key[17] = {0};

    /* 读取参数个数 */
    paramCount = pRecvData[dataIndex++];

    /* 清除之前的查询状态 */
    pIotOMCtx->stProtoData[0].queryParamFlag = 0;

    if (paramCount > 0)
    {
        /* 处理每个参数 */
        for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
        {
            /* 读取参数key */
            memcpy(key, &pRecvData[dataIndex], 16);
            dataIndex += 16;

            /* 根据key处理查询逻辑 */
            if (strcmp((const char *)key, "platDn") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 0);
            }
            else if (strcmp((const char *)key, "platType") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 1);
            }
            else if (strcmp((const char *)key, "ipAddr") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 2);    
            }
            else if (strcmp((const char *)key, "port") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 3);    
            }
            else if (strcmp((const char *)key, "cardType") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 4);    
            }
            else if (strcmp((const char *)key, "devOperator") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 5);    
            }
            else if (strcmp((const char *)key, "iv") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 6);    
            }
            else if (strcmp((const char *)key, "cipherKey") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 7);    
            }
            else if (strcmp((const char *)key, "productKey") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 8);    
            }
            else if (strcmp((const char *)key, "token") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 9);    
            }
            else if (strcmp((const char *)key, "productSecret") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 10);    
            }
            else if (strcmp((const char *)key, "workmode") == 0)
            {
                Common_SetBitFlag(&pIotOMCtx->stProtoData[0].queryParamFlag, 11);    
            }
            else
            {
                result = FALSE;
                IOTOM_CFG_InfoPrint("查询参数: %s (未知参数)\r\n", key);
            }
        }
    }
    else
    {
        result = FALSE;
    }
 
    return result;
}

static uint8_t IotOM_RecvRemoteSetParam(uint8_t *r_data, uint16_t len)
{
    uint8_t result = TRUE;
    uint16_t dataIndex = 32 + 1;
    uint8_t *pRecvData = r_data;
    uint8_t paramCount = 0;
    uint8_t paramIndex = 0;
    uint8_t key[17] = {0}; 
    uint8_t valueLen = 0;
    uint8_t handleResult = TRUE;

    /* 设置参数个数 */
    paramCount = pRecvData[dataIndex++];

    if (paramCount > 0)
    {
        /* 处理每个参数 */
        for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
        {
            /* 读取参数值长度 */
            valueLen = pRecvData[dataIndex++];
            
            /* 读取参数key */
            memcpy(key, &pRecvData[dataIndex], 16);
            dataIndex += 16;

            handleResult = TRUE;

            /* 根据key处理设置逻辑 */
            if (strcmp((const char *)key, "platDn") == 0)
            {
                handleResult = AswPlatM_SetPileDn((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "platType") == 0)
            {
                handleResult = AswPlatM_SetPlatType((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "ipAddr") == 0)
            {
                handleResult = AswPlatM_SetPlatMainIp((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "port") == 0)
            {
                handleResult = AswPlatM_SetPlatMainPort(*(uint16_t *)&pRecvData[dataIndex]);
            }
            else if (strcmp((const char *)key, "cardType") == 0)
            {
                handleResult = AswPlatM_SetPlatCardType((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "devOperator") == 0)
            {
                handleResult = AswPlatM_SetDevOperator((char *)&pRecvData[dataIndex], valueLen);   
            }
            else if (strcmp((const char *)key, "iv") == 0)
            {
                handleResult = AswPlatM_SetIv((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "cipherKey") == 0)
            {
                 handleResult = AswPlatM_SetCipherKey((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "productKey") == 0)
            {
                handleResult = AswPlatM_SetProductKey((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "token") == 0)
            {
                handleResult = AswPlatM_SetToken((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "productSecret") == 0)
            {
                handleResult = AswPlatM_SetProductSecret((char *)&pRecvData[dataIndex], valueLen);
            }
            else if (strcmp((const char *)key, "workmode") == 0)
            {
                if (pRecvData[dataIndex] == 0x01)
                {
                    CddModeM_ExitGBMode();
                }
                else
                {
                    CddModeM_EnterGBMode();
                }

                handleResult = TRUE;
            }
            else
            {
                result = FALSE;
                IOTOM_CFG_InfoPrint("设置参数: %s (未知参数)\r\n", key);
            }

            if (handleResult == FALSE)
            {
                result = FALSE;
            }

            dataIndex += valueLen;
        }
    }
    else
    {
        result = FALSE;
    }

    AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);
    return result;
}

static uint8_t IotOM_RecvRemoteQuerySetParam(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    uint8_t optResult = TRUE;

    /* 查询参数 */
    if (r_data[32] == 0x00)
    {
        optResult = IotOM_RecvRemoteQueryParam(r_data, len);
        pIotOMCtx->stProtoData[0].optParamAction = 0x00;
        pIotOMCtx->stProtoData[0].optParamResult = optResult ? 0x00 : 0x01;
    }
    /* 设置参数 */
    else  
    {
        optResult = IotOM_RecvRemoteSetParam(r_data, len);
        pIotOMCtx->stProtoData[0].optParamAction = 0x01;
        pIotOMCtx->stProtoData[0].optParamResult = optResult ? 0x00 : 0x01;
    }
    
    return TRUE;
}

static uint8_t IotOM_RecvReadLocalFile(uint8_t *port, uint8_t *r_data, uint16_t len)
{
    IotOMFrameReadLocalFile_Struct *pReq = (IotOMFrameReadLocalFile_Struct *)r_data;
    CddNetMSocketPara_Union stSocketPara = {0};
    uint8_t ret = FALSE;
    uint8_t result = eIotOMReadLocalFileResult_TOOLARGE;

    do
    {
        if (len < sizeof(IotOMFrameReadLocalFile_Struct))
        {
            IOTOM_CFG_InfoPrint("[%s] param is invaild\r\n", __FUNCTION__);
            break;
        }

        if (Common_MemEqual(pReq->ip, 0x0, sizeof(pReq->ip))
            || Common_MemEqual(pReq->user, 0x0, sizeof(pReq->user))
            || Common_MemEqual(pReq->passwd, 0x0, sizeof(pReq->passwd)) 
            || Common_MemEqual(pReq->remotePath, 0x0, sizeof(pReq->remotePath)))
        {
            break;
        }

        if (TRUE == SSUcm_IsUpdating())
        {/* 升级中, 不允许文件上传 */
            break;
        }

        stSocketPara.stFtpPara.eMode = eCddNetMFtpMode_Upload;
        stSocketPara.stFtpPara.eFileFormat = eCddNetMFileType_BIN;
        stSocketPara.stFtpPara.port = pReq->port;
        memcpy(stSocketPara.stFtpPara.ip,     pReq->ip,     sizeof(pReq->ip));
        memcpy(stSocketPara.stFtpPara.user,   pReq->user,   sizeof(pReq->user));
        memcpy(stSocketPara.stFtpPara.passwd, pReq->passwd, sizeof(pReq->passwd));

        uint16_t pathLen = sizeof(stSocketPara.stFtpPara.path) - 1; // 协议长度 > path长度 
        memcpy(stSocketPara.stFtpPara.path, pReq->remotePath, pathLen);
        stSocketPara.stFtpPara.path[pathLen] = '\0';

        if (SSSnapshot_ExportAllItems(&stSocketPara) != TRUE)
        {
            result = eIotOMReadLocalFileResult_NULL;
            break;
        }

        result = eIotOMReadLocalFileResult_OK;
        ret = TRUE;

    } while(0);

    pIotOMCtx->stProtoData[0].readLocalFileResult = result;

    return ret;
}

static const IotOMRecvCtrl_Struct* IotOM_GetRecvCtrlPtr(uint16_t cmd)
{
    const IotOMRecvCtrl_Struct* pCtrl = NULL;
    uint8_t index = 0;

    for (index = 0; index < IOT_OM_CMD_RECV_COUNT; index++) 
    {
        if (c_stIotOMRecvctrlTable[index].cmd == cmd)
        {
            pCtrl =  &c_stIotOMRecvctrlTable[index];
            break;
        }
    }

    return pCtrl;
}

static IotOMFrameHead_Struct *IotOM_FindValidFrameLen(uint8_t *pData, uint16_t dataLen, uint16_t *dealLen)
{
    uint8_t *pStart = pData;
    uint8_t *pRecvCrc = NULL;
    uint16_t remainLen = dataLen;
    IotOMFrameHead_Struct *pHead = NULL;
    uint16_t calcCrc16, recvCrc16;
    uint16_t frameLen = 0;

    while (remainLen > (sizeof(IotOMFrameHead_Struct) + 2))
    {
        pHead = (IotOMFrameHead_Struct *)pStart;

        if ((pHead->head[0] == IOT_OM_HEAD1) && (pHead->head[1] == IOT_OM_HEAD2))
        { 
            frameLen = Common_TwoUint8ToUint16(pHead->dataLen);

            if (frameLen > (sizeof(IotOMFrameHead_Struct) + 2))
            {
                calcCrc16 = Common_CalcCRC16((uint8_t *)pHead, frameLen - 2);
                pRecvCrc = (uint8_t *)pHead + frameLen - 2;
                recvCrc16 = pRecvCrc [1] | (pRecvCrc [0] << 8);

                if (calcCrc16 == recvCrc16)
                {
                    dealLen[0] = ((uint32_t)pHead - (uint32_t)pData) + frameLen;
                    break;
                }
            }
        }

        pStart++;
        remainLen--;
        dealLen[0]++;
    }

    return pHead;
}

static void IotOM_DecodeData(uint8_t *pData, uint16_t dataLen, uint16_t topicLen, uint8_t *pTopic, uint16_t *dealLen)
{
    const IotOMRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    IotOMFrameHead_Struct *pFrameHead = IotOM_FindValidFrameLen(pData, dataLen, dealLen);
    uint8_t port = 0;
    uint16_t frameLen = 0;

    if (pFrameHead != NULL)
    {
        pCmdRecvCtrl = IotOM_GetRecvCtrlPtr(pFrameHead->cmd);

        if (pCmdRecvCtrl != NULL)
        {
            if (pCmdRecvCtrl->pRecvParse != NULL)
            {
                frameLen = Common_TwoUint8ToUint16(pFrameHead->dataLen);
                if (TRUE == pCmdRecvCtrl->pRecvParse(&port, (uint8_t *)pFrameHead + sizeof(IotOMFrameHead_Struct), frameLen - sizeof(IotOMFrameHead_Struct) - 2))
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTOM_CFG_DebugPrint("[枪：%d]接收[cmd: %02X, %s][%d]\r\n", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }

                    if (pCmdRecvCtrl->cmdType == IOT_OM_CMDTYPE_RESPONSE)
                    {
                        Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pFrameHead->cmd, FALSE);
                        Common_ClearRptCount(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                    }
                    else
                    {
                        if (pCmdRecvCtrl->matchCmd != IOT_OM_CMD_NULL)
                        {
                            Common_SetRecvSeq(pIotOMCtx->pFuncRecvCtrl, port, pFrameHead->cmd, Common_TwoUint8ToUint16(pFrameHead->seq));
                            Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                        }
                    }

                    if (pCmdRecvCtrl->matchCmd != IOT_OM_CMD_NULL)
                    {
                        Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    if (pCmdRecvCtrl->printFlag)
                    {
                        IOTOM_CFG_InfoPrint("[枪：%d]接收[cmd: %02X, %s][%d] 处理失败!\r\n", port, pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, frameLen);
                        DSLogM_HexOutput((uint8_t *)pFrameHead, frameLen);
                    }
                }
            }
        }
    }
}

void IotOM_UpCtrlRecvDeal(void)
{
    FrameQueue_ProcessRxData(pIotOMCtx->frameQueueChannelID, IotOM_DecodeData);
}

void IotOM_TimeoutDetect(void)
{
    const IotOMRecvCtrl_Struct *pCmdRecvCtrl = NULL;
    uint8_t index = 0;
    uint8_t port = 0;
    uint8_t timeoutCount = 0;

    for (index = 0; index < IOT_OM_CMD_RECV_COUNT; index++)
    {
        pCmdRecvCtrl = &c_stIotOMRecvctrlTable[index];

        if (pCmdRecvCtrl->cmdType != IOT_OM_CMDTYPE_RESPONSE)
        {
            continue;
        }

        for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
        {
            if (Common_GetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd) != TRUE)
            {
                 continue;
            }

            if (Common_JudgeTimeoutMs(Common_GetRecvTick(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd), pCmdRecvCtrl->maxTimeout) == TRUE)
            {
                Common_SetRptCount(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                timeoutCount = Common_GetRptCount(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);

                IOTOM_CFG_InfoPrint("[cmd:0x%02X %s] 接收超时第 %d 次, 超时时间：%d ms\r\n", pCmdRecvCtrl->cmd, pCmdRecvCtrl->cMeaning, timeoutCount, pCmdRecvCtrl->maxTimeout);

                if (timeoutCount >= pCmdRecvCtrl->maxTryCnt)
                {
                    if (pCmdRecvCtrl->cmd == IOT_OM_CMD_HEARTBEAT_RSP || pCmdRecvCtrl->cmd == IOT_OM_CMD_LOGIN_RSP)
                    {
                        IotOM_OfflineHandle();
                    }
                    else
                    {
                        if (pCmdRecvCtrl->cmd == IOT_OM_CMD_ORDER_RECORD_RSP)
                        {
                            IOTOM_CFG_InfoPrint("订单上报失败，删除记录!\r\n");
                            MSNvm_SetRecordReportSuccess(eMSNvmBlockID_OmOrderRecord, pIotOMCtx->time);
                        }

                        Common_ClearRptCount(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd);
                        Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                        Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                    }
                }
                else
                {
                    Common_SetSendEnable(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetSendImmdFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, TRUE);
                    Common_SetRecvTimerEnable(pIotOMCtx->pFuncRecvCtrl, port, pCmdRecvCtrl->cmd, FALSE);
                    Common_SetSendFlag(pIotOMCtx->pFuncSendCtrl, port, pCmdRecvCtrl->matchCmd, FALSE);
                }
            }
        }
    }
}