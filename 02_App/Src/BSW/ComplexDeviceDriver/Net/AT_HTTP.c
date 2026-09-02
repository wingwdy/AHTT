/******************************************************************************
* File Name          : AT_HTTP.c
* Description        : HTTP AT command module
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/06/15      V1.0.0      hzb        支持GET/GETEX/POST
*
*******************************************************************************/

/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "AT_HTTP.h"
#include "Cdd_NetM.h"
#include "Cdd_Drv_EG800AK.h"
#include "SS_Ucm.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define ATHTTP_CFG_DebugPrint(fmt, ...)      CDDDRV_EG800AK_CFG_DebugPrint(fmt, ##__VA_ARGS__)

/* 分段GET最大重试 */
#define ATHTTP_SEGGET_MAX_RETRY        3
#define ATHTTP_SEGGET_RETRY_DELAY      2000
#define ATHTTP_REQUEST_TIMEOUT         (30 * 1000)  /* GET/POST请求等待URC响应的超时 */
/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint32_t reconnectInterval;             /* 重连间隔 */
    uint8_t  httpRequestPendingFlag;        /* 已发出请求,等待响应到达 */
    uint32_t httpRequestStartTick;          /* 发起请求时间戳 */
    uint32_t readContentLen;                /* HTTPREAD读取body长度 */
    uint8_t  segGetRetryCount;              /* 分段GET重试计数 */
    uint8_t  pendingRetryFlag;              /* 重试标志 */
    uint32_t pendingRetryTick;              /* 重试起始时间戳 */
} ATHTTPPrivate_Struct;

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void ATHTTP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState);
static void ATHTTP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMHttpPara_Struct *pHttpPara);
static uint8_t ATHTTP_RecvGET(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATHTTP_RecvUrlData(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
static uint8_t ATHTTP_RecvRead(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stHTTPATCmdDescribtor[eATHTTPCmd_Count] =
{
    [eATHTTPCmd_CFG_ContextId] =
    { "AT+QHTTPCFG=\"contextid\",1\r\n",                "OK\r\n",         NULL,       3,  3000,  3000,  TRUE, "HTTP配置contextid",
    NULL,                                               NULL,                           ATHTTP_FailHandle },

    [eATHTTPCmd_CFG_RequestHeader] =
    { "AT+QHTTPCFG=\"requestheader\",0\r\n",            "OK\r\n",         NULL,       3,  3000,  3000,  TRUE, "HTTP配置请求头(关闭)",
    NULL,                                               NULL,                           ATHTTP_FailHandle },

    [eATHTTPCmd_CFG_ResponseHeader] =
    { "AT+QHTTPCFG=\"responseheader\",[ENABLE]\r\n",    "OK\r\n",         NULL,       3,  3000,  3000,  TRUE, "HTTP配置响应头",
    ATHTTP_PackResHeader,                               NULL,                           ATHTTP_FailHandle },

    [eATHTTPCmd_CFG_ContentType] =
    { "AT+QHTTPCFG=\"contenttype\",[TYPE]\r\n",         "OK\r\n",         NULL,       3,  3000,  3000,  TRUE, "HTTP配置ContentType",
    ATHTTP_PackContextType,                             NULL,                           ATHTTP_FailHandle },

    [eATHTTPCmd_CFG_RSPOUT] =
    { "AT+QHTTPCFG=\"rspout/auto\",0\r\n",              "OK\r\n",         NULL,       3,  3000,  3000,  TRUE, "HTTP配置自动输出",
    NULL,                                               NULL,                           ATHTTP_FailHandle },

    [eATHTTPCmd_SET_URL] =
    { "AT+QHTTPURL=[LEN],60\r\n",                       "CONNECT",        NULL,       3, 10000,  5000,  TRUE, "HTTP设置URL",
    ATHTTP_PackURL,                                     ATHTTP_RecvUrlData,            ATHTTP_FailHandle },

    [eATHTTPCmd_Method_GET] =
    { "AT+QHTTPGET=60\r\n",                             "OK\r\n",         NULL,       3, 30000, 10000,  TRUE, "HTTP GET",
    NULL,                                               ATHTTP_RecvGET,                 ATHTTP_FailHandle },

    [eATHTTPCmd_Method_GETEX] =
    { "AT+QHTTPGETEX=60,[POS],[LEN]\r\n",              "OK\r\n",         NULL,       3, 30000, 10000,  TRUE, "HTTP GETEX分段",
    ATHTTP_PackGetEx,                                   ATHTTP_RecvGET,                 ATHTTP_FailHandle },

    [eATHTTPCmd_Method_POST] =
    { "AT+QHTTPPOST=[LEN],30,60\r\n",                   "CONNECT",        NULL,       3, 30000, 10000,  TRUE, "HTTP POST",
    ATHTTP_PackBody,                                    ATHTTP_RecvPOST,                ATHTTP_FailHandle },
        
    [eATHTTPCmd_READ] =
    { "AT+QHTTPREAD=60\r\n",                            "CONNECT",        NULL,       1, 15000, 15000,  FALSE, "HTTP读取响应",
    NULL,                                               ATHTTP_RecvRead,                ATHTTP_FailHandle },
};

static uint8_t s_u8HttpSocketIndex = 0xff;
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void ATHTTP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (eSocketState != pSocketCtrl->eSocketState)
        {
            pSocketCtrl->eSocketState = eSocketState;

            if (eSocketState == eCddNetMSocketState_Connecting)
            {
                pPrivate->httpRequestPendingFlag = FALSE;
            }
            else if (eSocketState == eCddNetMSocketState_ConnectOK)
            {
                pSocketCtrl->reconectTimes = 0;
                ATHTTP_CFG_DebugPrint("[socket: %d] HTTP连接建立成功!\r\n", socketIndex);
            }
            else if (eSocketState == eCddNetMSocketState_WaitReconnect)
            {
                pPrivate->reconnectInterval = 0;
            }
        }
    }
}

static uint8_t ATHTTP_SegGetRetry(CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl)
{
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t ret = FALSE;

    if (pPrivate->segGetRetryCount < ATHTTP_SEGGET_MAX_RETRY)
    {
        pPrivate->segGetRetryCount++;
        pPrivate->pendingRetryFlag = TRUE;
        pPrivate->pendingRetryTick = Common_GetSystick();
        ret = TRUE;
    }
    else
    {
        ATHTTP_CloseSocket(pSocketCtrl);
    }

    return ret;
}

static void ATHTTP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMHttpPara_Struct *pHttpPara)
{
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    if (pPrivate->pendingRetryFlag == TRUE)
    {
        if (Common_JudgeTimeoutMs(pPrivate->pendingRetryTick, ATHTTP_SEGGET_RETRY_DELAY) == TRUE)
        {
            pPrivate->pendingRetryFlag = FALSE;
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_Method_GETEX);
            ATHTTP_CFG_DebugPrint("[socket: %d] HTTP GETEX重试\r\n", socketIndex);
        }
    }

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Connecting)
    {
        if (pPrivate->httpRequestPendingFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->httpRequestStartTick, ATHTTP_WAIT_CONNECT_TIMEOUT))
            {
                ATHTTP_CloseSocket(pSocketCtrl);
                ATHTTP_CFG_DebugPrint("[socket: %d] HTTP连接超时!\r\n", socketIndex);
            }
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
    {
        if (pPrivate->httpRequestPendingFlag == TRUE &&
            Common_JudgeTimeoutMs(pPrivate->httpRequestStartTick, ATHTTP_REQUEST_TIMEOUT))
        {
            if (pHttpPara->type == eCddNetMHttpType_SegGET)
            {
                ATHTTP_CFG_DebugPrint("[socket: %d] HTTP GETEX无响应, 第%d次重试\r\n",
                    socketIndex, pPrivate->segGetRetryCount + 1);
                if (ATHTTP_SegGetRetry(pSocketCtrl))
                {
                    pPrivate->httpRequestPendingFlag = FALSE; /* 清零避免下个周期继续判定超时重复触发 */
                }
            }
            else
            {
                ATHTTP_CloseSocket(pSocketCtrl);
                ATHTTP_CFG_DebugPrint("[socket: %d] HTTP请求超时, 关闭socket\r\n", socketIndex);
            }
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_Abnormal)
    {
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_WaitReconnect)
    {
        if (pPrivate->reconnectInterval == 0)
        {
            pSocketCtrl->reconectTimes++;

            if (pSocketCtrl->reconectTimes >= CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES)
            {
                pSocketCtrl->reconectTimes = CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES;
                CddNetM_DeleteLink(pSocketCtrl->ePlatType);
            }

            if (pSocketCtrl->socketDisconnectCallback != NULL)
            {
                pSocketCtrl->socketDisconnectCallback();
            }

            CDDDRV_EG800AK_CFG_RECONECT_TIMEOUT(pSocketCtrl->reconectTimes, pPrivate->reconnectInterval);
            pSocketCtrl->disconectTickStart = Common_GetSystick();
            ATHTTP_CFG_DebugPrint("[socket: %d] %d ms 后进行第 %d 次重新连接!\r\n",
                socketIndex, pPrivate->reconnectInterval, pSocketCtrl->reconectTimes);
        }
        else
        {
            if (Common_JudgeTimeoutMs(pSocketCtrl->disconectTickStart, pPrivate->reconnectInterval))
            {
                ATHTTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Init);
            }
        }
    }
}

void ATHTTP_StateHandle(uint8_t socketIndex, void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE &&
            CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work)
        {
            s_u8HttpSocketIndex = socketIndex;
            memset(pSocketCtrl->user_data, 0, sizeof(pSocketCtrl->user_data));
            ATHTTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Connecting);

            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_CFG_ContextId);
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_CFG_RequestHeader);
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_CFG_ResponseHeader);
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_CFG_ContentType);
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_CFG_RSPOUT);
            CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_SET_URL);

            if (pHttpPara->type == eCddNetMHttpType_GET)
            {
                CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_Method_GET);
            }
            else if (pHttpPara->type == eCddNetMHttpType_SegGET)
            {
                CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_Method_GETEX);
            }
            else if (pHttpPara->type == eCddNetMHttpType_POST)
            {
                CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_Method_POST);
            }
        }
    }
    else
    {
        ATHTTP_SocketStateMange(socketIndex, pSocketCtrl, pHttpPara);
    }
}

void ATHTTP_CloseSocket(void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->httpRequestPendingFlag = FALSE;
    pPrivate->pendingRetryFlag = FALSE;
    pPrivate->segGetRetryCount = 0;

    if (pSocketCtrl->eSocketState != eCddNetMSocketState_Init &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_Abnormal &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect)
    {
        CddDrvEG800AK_ClearSocketCmd(pSocketCtrl->socketIndex);
        ATHTTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    }

    s_u8HttpSocketIndex = 0xff;
}

uint8_t ATHTTP_FailHandle(uint8_t socketIndex, void *socketPara, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATHTTP_CloseSocket(pSocketCtrl);

    return TRUE;
}

/* ============================== Pack ============================== */

uint16_t ATHTTP_PackResHeader(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;
    uint8_t enable = 0;//(pHttpPara->type == eCddNetMHttpType_POST) ? 0 : 1;
    nATLen = Common_ReplaceNum(pData, nATLen, "[ENABLE]", enable, enable);

    return nATLen;
}

uint16_t ATHTTP_PackContextType(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;
    uint8_t type = (pHttpPara->type == eCddNetMHttpType_POST) ? eATHTTP_ContentType_AppXWWW : eATHTTP_ContentType_AppOctet;
    nATLen = Common_ReplaceNum(pData, nATLen, "[TYPE]", type, type);

    return nATLen;
}

uint16_t ATHTTP_PackURL(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;
    nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", pHttpPara->urlLen, pHttpPara->urlLen);

    return nATLen;
}

uint16_t ATHTTP_PackBody(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;
    nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", pHttpPara->bodyLen, pHttpPara->bodyLen);

    return nATLen;
}

uint16_t ATHTTP_PackGetEx(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;

    nATLen = Common_ReplaceNum(pData, nATLen, "[POS]", pHttpPara->segPos, 0);
    nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", pHttpPara->segLen, 0);

    return nATLen;
}

/* ============================== Recv ============================== */

static uint8_t ATHTTP_RecvGET(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->httpRequestPendingFlag = TRUE;
    pPrivate->httpRequestStartTick = Common_GetSystick();

    return TRUE;
}

static uint8_t ATHTTP_RecvUrlData(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;

    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Send);
    CDDDRV_EG800AK_CFG_WriteData((uint8_t *)pHttpPara->url, pHttpPara->urlLen);
    CddDrvEG800AK_ExitTransparentMode();

    return TRUE;
}

uint8_t ATHTTP_RecvPOST(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;

    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Send);
    CDDDRV_EG800AK_CFG_WriteData((uint8_t *)pHttpPara->body, pHttpPara->bodyLen);
    CddDrvEG800AK_ExitTransparentMode();

    pPrivate->httpRequestPendingFlag = TRUE;
    pPrivate->httpRequestStartTick = Common_GetSystick();

    return TRUE;
}

static uint8_t ATHTTP_RecvRead(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMHttpPara_Struct *pHttpPara = (CddNetMHttpPara_Struct *)pSocketCtrl->specificPara;
    ATHTTPPrivate_Struct *pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pBody;
    uint16_t bodyLen;
    uint32_t contentLen;

    contentLen = pPrivate->readContentLen;

    pBody = (uint8_t *)strstr((char *)pData, "CONNECT\r\n");
    if (pBody == NULL)
    {
        return FALSE;
    }
    pBody += 9;

    if ((uint32_t)(pBody - pData) + contentLen > (uint32_t)dataLen)
    {/* 数据不完整，主动重新请求当前帧 */
        ATHTTP_CFG_DebugPrint("HTTP READ数据不完整: expected=%u, actual=%u, 第%d次重试\r\n",
                (uint32_t)(pBody - pData) + contentLen, dataLen, pPrivate->segGetRetryCount);
        *pDealLen = dataLen;
        pPrivate->readContentLen = 0;
        if (pHttpPara->type == eCddNetMHttpType_SegGET)
        {
            ATHTTP_SegGetRetry(pSocketCtrl);
        }
        else
        {
            ATHTTP_CloseSocket(pSocketCtrl);
        }
        return TRUE;
    }
    bodyLen = (uint16_t)contentLen;

    if (bodyLen > CDD_NETM_CFG_HTTP_BODY_LEN)
    {
        bodyLen = CDD_NETM_CFG_HTTP_BODY_LEN;
    }
    memcpy(pHttpPara->body, pBody, bodyLen);
    pHttpPara->body[bodyLen] = '\0';
    pHttpPara->bodyLen = bodyLen;
    pHttpPara->dataReady = TRUE;

    *pDealLen = (uint16_t)(pBody - pData + bodyLen + strlen("\r\nOK\r\n"));
    pPrivate->segGetRetryCount = 0;
    pPrivate->readContentLen = 0;

    ATHTTP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);

    if (bodyLen > 0)
    {
        if (pHttpPara->type == eCddNetMHttpType_SegGET)
        {/* 目前HTTP分段请求默认为OTA下载, 后续有其他场景,再做优化 */
            SSUcm_FileDataHandle((uint8_t *)pHttpPara->body, bodyLen);

            if (SSUcm_GetWorkState() < eSSUcmWorkState_Finish)
            {
                uint16_t readLen;
                uint32_t readOffset;
                SSUcm_GetReadLenAndOffSet(&readLen, &readOffset);
                pHttpPara->segPos = SSUcm_GetFileOffset();
                pHttpPara->segLen = readLen;
                CddDrvEG800AK_AddCmd(socketIndex, eATHTTPCmd_Method_GETEX);
            }
        }
    }

    return TRUE;
}

/* ============================== URC ============================== */
uint32_t ATHTTP_UrcQHTTPPost(uint8_t *pData, void *modulePara, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl;
    ATHTTPPrivate_Struct *pPrivate;
    int32_t err = 0, httprspcode = 0, contentLen = 0;

    if (s_u8HttpSocketIndex >= CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        return 0;
    }

    if (3 != sscanf((char *)pData, "+QHTTPPOST: %d,%d,%d", &err, &httprspcode, &contentLen))
    {
        return 0;
    }

    pSocketCtrl = &pModulePara->stSocketCtrl[s_u8HttpSocketIndex];
    pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    if (err != 0 || httprspcode != 200)
    {
        ATHTTP_CFG_DebugPrint("HTTP POST失败, err=%d, code=%d\r\n", (int)err, (int)httprspcode);
        pPrivate->httpRequestPendingFlag = FALSE;
        ATHTTP_CloseSocket(pSocketCtrl);
        return 0;
    }

    pPrivate->readContentLen = (uint32_t)contentLen;
    /* 发起http  read读数据 */
    CddDrvEG800AK_AddCmd(s_u8HttpSocketIndex, eATHTTPCmd_READ);

    return 0;
}

uint32_t ATHTTP_UrcQHTTPGet(uint8_t *pData, void *modulePara, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl;
    ATHTTPPrivate_Struct *pPrivate;
    int32_t err = 0, httprspcode = 0, contentLen = 0;

    if (s_u8HttpSocketIndex >= CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        return 0;
    }

    if (3 != sscanf((char *)pData, "+QHTTPGET: %d,%d,%d", &err, &httprspcode, &contentLen))
    {
        /* URC解析失败(数据可能损坏), 触发GETEX重试以恢复下载 */
        ATHTTP_CFG_DebugPrint("HTTP GET URC解析失败, dataLen=%d\r\n", dataLen);
        pSocketCtrl = &pModulePara->stSocketCtrl[s_u8HttpSocketIndex];
        pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;
        ATHTTP_SegGetRetry(pSocketCtrl);
        return 0;
    }

    pSocketCtrl = &pModulePara->stSocketCtrl[s_u8HttpSocketIndex];
    pPrivate = (ATHTTPPrivate_Struct *)pSocketCtrl->user_data;

    if ((err != 0) || (httprspcode != 200 && httprspcode != 206))
    {
        /* 5xx为服务器临时故障(502/503/504网关问题/服务过载/超时), 触发重试;
         * 4xx为客户端错误(URL/权限等), 重试无意义, 直接关闭 */
        uint8_t shouldRetry = (err == 0 && httprspcode >= 500 && httprspcode < 600);
        ATHTTP_CFG_DebugPrint("HTTP GET服务器临时故障, code=%d, 第%d次重试\r\n", httprspcode, pPrivate->segGetRetryCount);
        if (shouldRetry)
        {
            ATHTTP_SegGetRetry(pSocketCtrl);
        }
        else
        {
            ATHTTP_CloseSocket(pSocketCtrl);
        }
        return 0;
    }

    pPrivate->readContentLen = (uint32_t)contentLen;
    CddDrvEG800AK_AddCmd(s_u8HttpSocketIndex, eATHTTPCmd_READ);

    return 0;
}

uint32_t ATHTTP_UrcQHTTPRead(uint8_t *pData, void *modulePara, uint16_t dataLen)
{
    int32_t err = 0;

    if (1 == sscanf((char *)pData, "+QHTTPREAD: %d", &err))
    {
        if (err != 0)
        {
            ATHTTP_CFG_DebugPrint("HTTP READ失败, err=%d\r\n", (int)err);
        }
    }

    return 0;
}
