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
#include "AT_FTP.h"
#include "Cdd_NetM.h"
#include "Cdd_Drv_EG800AK.h"
#include "SS_Snapshot.h"
#include "SS_Ucm.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint32_t fileHandle;
    uint8_t fileHandleValidFlag;                /* 已经获取到文件句柄 */
    uint8_t abnormalCloseFlag;                  /* 异常关闭 */

    uint8_t waitFtpConnectOkFlag;
    uint32_t waitFtpConnectOkTickStart;         /* 等待FTP建立连接开始tick */
    uint32_t reconnectInterval;                 /* 重连间隔 */
    uint32_t cycleDetectSocketStateTickStart;
    uint32_t ufsFlashFreeSpace;

    uint32_t totalWriteSize;
    uint32_t upLoadFileTickStart;               /* 上传文件开始计时 */
    uint16_t curWriteSize;
    uint8_t fileWriteSuccFlag;                  /* 文件已经写入完成（限上传） */
    uint8_t uploadFileTickStartFlag;            /* 上传文件开始标记 */

    uint32_t downLoadFileTickStart;             /* 下载文件开始计时 */
    uint8_t downLoadFileTickStartFlag;          /* 下载文件开始标记*/
    uint32_t totalReadSize;
}ATFTPPrivate_Struct;



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void ATFTP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState);

static uint8_t ATFTP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID);

static uint8_t ATFTP_RecvOKACK(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSSpace(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSRead(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);

static uint8_t ATFTP_RecvOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvFTPState(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvFTPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvFTPSwithPath(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);

static uint16_t ATFTP_PackUfsOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackUfsWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackUfsClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackAccount(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackFileType(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackPath(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackUpload(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackDownload(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackUfsSeek(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
static uint16_t ATFTP_PackUfsRead(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stFTPATCmdDescribtor[eATFTPCmd_Count] =
{
    [eATFTPCmd_UFSDeleteFile] =
    { "AT+QFDEL=\"*\"\r\n",              "OK",                        3,      10000,     5000,    TRUE, "删除UFS文件",
      NULL,                               NULL,                                          ATFTP_FailHandle },

    [eATFTPCmd_UFSQuerySpace] =
    { "AT+QFLDS=\"UFS\"\r\n",            "+QFLDS: ",                  3,      5000,      3000,    TRUE, "查询UFS剩余空间",
      NULL,                              ATFTP_RecvUFSSpace,                             ATFTP_FailHandle },

    [eATFTPCmd_UFSOpen] =
    { "AT+QFOPEN=\"[FILE]\",[MODE]\r\n", "+QFOPEN:",                  3,      5000,      3000,    TRUE, "打开/新建文件",
      ATFTP_PackUfsOpen,                 ATFTP_RecvUFSOpen,                              ATFTP_FailHandle },

    [eATFTPCmd_UFSSeek] =
    { "AT+QFSEEK=[HANDLE],[OFFSET],0\r\n",   "+QFSEEK=",              3,      5000,      5000,    TRUE, "设置UFS文件指针",
      ATFTP_PackUfsSeek,                 ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_UFSRead] =
    { "AT+QFREAD=[HANDLE],[LEN]\r\n",    "CONNECT",                  3,      5000,      5000,    TRUE, "读UFS文件",
      ATFTP_PackUfsRead,                 ATFTP_RecvUFSRead,                              ATFTP_FailHandle },

    [eATFTPCmd_UFSWrite] =
    { "AT+QFWRITE=[HANDLE],[SIZE]\r\n",   "CONNECT",                  3,      5000,      5000,    TRUE, "写UFS文件",
      ATFTP_PackUfsWrite,                ATFTP_RecvUFSWrite,                             ATFTP_FailHandle },

    [eATFTPCmd_UFSClose] =
    { "AT+QFCLOSE=[HANDLE]\r\n",         "+QFCLOSE",                  3,      5000,      5000,    TRUE, "关闭UFS文件",
      ATFTP_PackUfsClose,                ATFTP_RecvUFSClose,                             ATFTP_FailHandle },

    [eATFTPCmd_FTPConfigContext] =
    { "AT+QFTPCFG=\"contextid\",1\r\n",  "contextid",                 3,      5000,      5000,    TRUE, "配置FTP PDP上下文",
      NULL,                              ATFTP_RecvOKACK,                                 ATFTP_FailHandle },

    [eATFTPCmd_FTPConfigPSW] =
    { "AT+QFTPCFG=\"account\",\"[NAME]\",\"[PSW]\"\r\n",  "account",  3,      5000,      5000,    TRUE, "配置FTP用户名密码",
      ATFTP_PackAccount,                 ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPConfigFileType] =
    { "AT+QFTPCFG=\"filetype\",[FILETYPE]\r\n",   "filetype",         3,      5000,      5000,    TRUE, "配置FTP文件类型",
      ATFTP_PackFileType,                ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPConfigTransferMode] =
    { "AT+QFTPCFG=\"transmode\",1\r\n",  "transmode",                 3,      5000,      5000,    TRUE, "配置FTP传输模式",
      NULL,                              ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPConfigTimeout] =
    { "AT+QFTPCFG=\"rsptimeout\",90\r\n","rsptimeout",                3,      5000,      5000,    TRUE, "配置FTP超时时间",
      NULL,                              ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPOpen] =
    { "AT+QFTPOPEN=\"[FIP]\",[FPORT]\r\n",  "+QFTPOPEN=",             3,      20000,     5000,    TRUE,  "Open FTP连接",
      ATFTP_PackOpen,                    ATFTP_RecvOpen,                                 ATFTP_FailHandle },

    [eATFTPCmd_FTPState] =
    { "AT+QFTPSTAT\r\n",                 "+QFTPSTAT:",                3,      5000,      5000,    FALSE, "查询FTP连接状态",
      NULL,                              ATFTP_RecvFTPState,                             ATFTP_FailHandle },
  
    [eATFTPCmd_FTPSwithPath] =
    { "AT+QFTPCWD=\"[PATH]\"\r\n",       "+QFTPCWD:",                 3,      5000,      5000,    TRUE, "设置FTP路径",
      ATFTP_PackPath,                    ATFTP_RecvFTPSwithPath,                         ATFTP_FailHandle },

    [eATFTPCmd_FTPUpload] =
    { "AT+QFTPPUT=\"[FILE1]\",\"UFS:[FILE2]\",0\r\n",   "+QFTPPUT=",  3,      5000,      5000,    TRUE, "上传文件",
      ATFTP_PackUpload,                  ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPDownload] =
    { "AT+QFTPGET=\"[FILE]\",\"UFS:file\"\r\n",   "+QFTPGET=",    3,      5000,      5000,    TRUE, "下载文件",
      ATFTP_PackDownload,                ATFTP_RecvOKACK,                                ATFTP_FailHandle },

    [eATFTPCmd_FTPClose] =
    { "AT+QFTPCLOSE\r\n",                 "+QFTPCLOSE",               3,      5000,      5000,    TRUE, "关闭FTP连接",
      NULL,                              ATFTP_RecvFTPClose,                             ATFTP_FailHandle },   
};

static uint8_t g_uploadFileBuf[ATFTP_UPLOAD_SINGLE_PACK_MAX_SIZE] = {0};
/* 
    创建这个变量是因为FTP的AT命令应答没有socket的概念，但是我们又需要这个socketIndex，所以内部记住 
    正常情况就是2
*/
static uint8_t g_socketIndex = 2; 

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void ATFTP_SetSocketState(uint8_t socketIndex, void *socketPara, CddNetMSocketState_Enum eSocketState)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (eSocketState != pSocketCtrl->eSocketState)
        {
            pSocketCtrl->eSocketState = eSocketState;

            if (eSocketState == eCddNetMSocketState_Connecting)
            {
                pPrivate->waitFtpConnectOkFlag = FALSE;
            }
            else if (eSocketState == eCddNetMSocketState_ConnectOK)
            {
                pPrivate->downLoadFileTickStart = FALSE;
                pPrivate->uploadFileTickStartFlag = FALSE;
                CDDDRV_EG800AK_CFG_LogPrint("[socket: %d]FTP服务器建立连接成功!\r\n", socketIndex);
                CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPSwithPath);
            }
            else if (eSocketState == eCddNetMSocketState_WaitReconnect)
            {
                pPrivate->reconnectInterval = 0;
            }
        }
    }
}

static uint8_t ATFTP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t ret = FALSE;

    if (atTaskID == eATFTPCmd_FTPClose)
    {
        ATFTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    }
    else
    {
        if (atTaskID == eATFTPCmd_UFSClose)
        {
            pPrivate->fileHandleValidFlag = FALSE;
        }
        
        pPrivate->abnormalCloseFlag = TRUE;
        ATFTP_CloseSocket(pSocketCtrl);
        ret = TRUE;
    }

    return ret;
}

static uint16_t ATFTP_PackUfsOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    char fileName[CDD_NETM_CFG_FTP_FILENAME_LEN + 1] = {0};
    uint8_t packIndex = 0;
    uint8_t *pTemp = NULL;

    if (pFTPPara->eMode == eCddNetMFtpMode_Download)
    {
        nATLen = Common_ReplaceStr(pData, nATLen, "[FILE]", "UFS:file", strlen("UFS:file"), "UFS:file");
    }
    else
    {
        nATLen = Common_ReplaceStr(pData, nATLen, "[FILE]", pFTPPara->fileName, strlen(pFTPPara->fileName), "D3_A32FB.log");
    }

    if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[MODE]", 1, 1);
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[MODE]", 2, 2);
    }

    return nATLen;
}

static uint16_t ATFTP_PackUfsSeek(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint16_t readLen = 0;
    uint32_t readOffset = 0;

    SSUcm_GetReadLenAndOffSet(&readLen, &readOffset);
    
    if (readLen != 0)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[HANDLE]", pPrivate->fileHandle, 1);
        nATLen = Common_ReplaceNum(pData, nATLen, "[OFFSET]", readOffset, 0);
    }
    
    return nATLen;
}

static uint16_t ATFTP_PackUfsRead(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint16_t readLen = 0;
    uint32_t readOffset = 0;

    SSUcm_GetReadLenAndOffSet(&readLen, &readOffset);
    
    if (readLen != 0)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[HANDLE]", pPrivate->fileHandle, 1);
        nATLen = Common_ReplaceNum(pData, nATLen, "[LEN]", readLen, 0);
    }

    return nATLen;
}

static uint16_t ATFTP_PackUfsWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    
    pPrivate->curWriteSize = 0;

    SSSnapshot_PreviewReadItem(ATFTP_UPLOAD_SINGLE_PACK_MAX_SIZE, &pPrivate->curWriteSize);

    if (pPrivate->curWriteSize > 0)
    {
        if (pPrivate->totalWriteSize + pPrivate->curWriteSize > ATFTP_UFS_FILE_MIN_SIZE)
        {
            pPrivate->fileWriteSuccFlag = TRUE;
            SSSnapshot_StopReadItem();
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_UFSClose);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigContext);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigPSW);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigFileType);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTransferMode);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTimeout);
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPOpen);
            nATLen = 0;
        }
        else
        {
            nATLen = Common_ReplaceNum(pData, nATLen, "[HANDLE]", pPrivate->fileHandle, 1);
            nATLen = Common_ReplaceNum(pData, nATLen, "[SIZE]", pPrivate->curWriteSize, 0);
        }
    }
    else
    {
        pPrivate->fileWriteSuccFlag = TRUE;
        SSSnapshot_StopReadItem();
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_UFSClose);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigContext);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigPSW);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigFileType);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTransferMode);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTimeout);
        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPOpen);
        nATLen = 0;
    }

    return nATLen;
}

static uint16_t ATFTP_PackUfsClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceNum(pData, nATLen, "[HANDLE]", pPrivate->fileHandle, 1);
    return nATLen;
}

static uint16_t ATFTP_PackAccount(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceStr(pData, nATLen, "[NAME]", pFTPPara->user, strlen(pFTPPara->user), ATFTP_DEFAULT_USER_NAME);
    nATLen = Common_ReplaceStr(pData, nATLen, "[PSW]", pFTPPara->passwd, strlen(pFTPPara->passwd), ATFTP_DEFAULT_USER_PSW);
    return nATLen;
}

static uint16_t ATFTP_PackFileType(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    if (pFTPPara->eFileFormat == eCddNetMFileType_BIN)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[FILETYPE]", 0, 0);
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[FILETYPE]", 1, 0);
    }

    return nATLen;
}

static uint16_t ATFTP_PackOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceStr(pData, nATLen, "[FIP]", pFTPPara->ip, strlen(pFTPPara->ip), ATFTP_DEFAULT_USER_IP);
    nATLen = Common_ReplaceNum(pData, nATLen, "[FPORT]", pFTPPara->port, ATFTP_DEFAULT_USER_PORT);
    return nATLen;
}

static uint16_t ATFTP_PackPath(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceStr(pData, nATLen, "[PATH]", pFTPPara->path, strlen(pFTPPara->path), ATFTP_DEFAULT_USER_PATH);
    return nATLen;
}

static uint16_t ATFTP_PackUpload(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    nATLen = Common_ReplaceStr(pData, nATLen, "[FILE1]", pFTPPara->fileName, strlen(pFTPPara->fileName), ATFTP_DEFAULT_USER_FILE);
    nATLen = Common_ReplaceStr(pData, nATLen, "[FILE2]", pFTPPara->fileName, strlen(pFTPPara->fileName), ATFTP_DEFAULT_USER_FILE);
    
    pPrivate->uploadFileTickStartFlag = TRUE;
    pPrivate->upLoadFileTickStart = Common_GetSystick();
    return nATLen;
}

static uint16_t ATFTP_PackDownload(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    char fileName[CDD_NETM_CFG_FTP_FILENAME_LEN + 1] = {0};
    uint8_t packIndex = 0;
    uint8_t *pTemp = NULL;

    SSUcm_GetPackIndex(&packIndex);

    if (strlen(pFTPPara->fileName) == 0)
    {
        snprintf(fileName, sizeof(fileName), "%s%d.bin", ATFTP_DEFAULT_USER_FILE, packIndex);
    }
    else
    {
        pTemp = Common_SearchData((uint8_t *)pFTPPara->fileName, strlen(pFTPPara->fileName), ".bin", 4);

        if (pTemp != NULL)
        {
            memcpy(fileName, pFTPPara->fileName, (uint32_t)pTemp - (uint32_t)pFTPPara->fileName);
        }

        snprintf(fileName, sizeof(fileName), "%s%d.bin", fileName, packIndex);
    }

    nATLen = Common_ReplaceStr(pData, nATLen, "[FILE]", fileName, strlen(fileName), fileName);

    pPrivate->downLoadFileTickStartFlag = TRUE;
    pPrivate->downLoadFileTickStart = Common_GetSystick();
    return nATLen;
}

void ATFTP_UrcRecvWrite(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &pModulePara->stSocketCtrl[g_socketIndex];   
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data; 
    uint8_t *pTemp = NULL;
    int32_t totalSize, writeSize;
    uint8_t flag = FALSE;

    pTemp = Common_SearchData(pData, dataLen, "+QFWRITE:", strlen("+QFWRITE:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFWRITE: %d,%d\r\n", &writeSize, &totalSize))
        {
            if (pPrivate->curWriteSize == writeSize && pPrivate->totalWriteSize == totalSize)
            {
                CddDrvEG800AK_AddCmd(g_socketIndex, eATFTPCmd_UFSWrite);
                flag = TRUE;
            }
        }
    }

    if (flag == FALSE)
    {
        SSSnapshot_StopReadItem();
        CddNetM_DeleteLink(eCddNetMPlatType_File);
    }

    CddDrvEG800AK_ExitTransparentMode();
}

void ATFTP_UrcRecvOpen(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &pModulePara->stSocketCtrl[g_socketIndex];   
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data; 
    int32_t err1, err2;
    uint8_t *pTemp = NULL;

    pTemp = Common_SearchData(pData, dataLen, "+QFTPOPEN:", strlen("+QFTPOPEN:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFTPOPEN: %d,%d\r\n", &err1, &err2))
        {
            if (err1 == 0 && err2 == 0)
            {
                if (pPrivate->waitFtpConnectOkFlag == TRUE)
                {
                    pPrivate->waitFtpConnectOkFlag = FALSE;
                    ATFTP_SetSocketState(g_socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("FTP连接失败，err1: %d, err2: %d !\r\n", err1, err2);
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
        }
    }
}

void ATFTP_UrcRecvPut(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
   CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &pModulePara->stSocketCtrl[g_socketIndex];   
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data; 
    int32_t err1, transferLen = 0;
    uint8_t *pTemp = NULL;

    pTemp = Common_SearchData(pData, dataLen, "+QFTPPUT::", strlen("+QFTPPUT:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFTPPUT: %d,%d\r\n", &err1, &transferLen))
        {
            if (err1 == 0)
            {
                if (pPrivate->uploadFileTickStartFlag == TRUE)
                {
                    pPrivate->uploadFileTickStartFlag = FALSE;
                    CddNetM_DeleteLink(eCddNetMPlatType_File);
                    CDDDRV_EG800AK_CFG_LogPrint("FTP上传文件成功, 文件长度： %d !\r\n",transferLen);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("FTP上传文件失败，err1: %d, err2: %d !\r\n", err1, transferLen);
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
        }
    }
}

void ATFTP_UrcRecvGet(uint8_t *pData, void * modulePara, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &pModulePara->stSocketCtrl[g_socketIndex];   
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data; 
    int32_t err1, transferLen = 0;
    uint8_t *pTemp = NULL;

    pTemp = Common_SearchData(pData, dataLen, "+QFTPGET:", strlen("+QFTPGET:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFTPGET: %d,%d\r\n", &err1, &transferLen))
        {
            if (err1 == 0)
            {
                if (pPrivate->downLoadFileTickStartFlag == TRUE)
                {
                    CddDrvEG800AK_AddCmd(g_socketIndex, eATFTPCmd_UFSOpen);
                    pPrivate->downLoadFileTickStartFlag = FALSE;
                    pPrivate->totalReadSize = transferLen;
                    CDDDRV_EG800AK_CFG_LogPrint("FTP下载文件成功, 文件长度： %d !\r\n",transferLen);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("FTP下载文件失败, err1: %d, err2: %d !\r\n", err1, transferLen);
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
        }
    }
}

static uint8_t ATFTP_RecvOKACK(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
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

static uint8_t ATFTP_RecvOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;

    pTemp = Common_SearchData(pData, dataLen, "OK", strlen("OK"));
    
    if (pTemp != NULL)
    {
        pPrivate->waitFtpConnectOkFlag = TRUE;
        pPrivate->waitFtpConnectOkTickStart = Common_GetSystick();
        pPrivate->cycleDetectSocketStateTickStart = Common_GetSystick();
        ret = TRUE;
    }

    return ret;
}

static uint8_t ATFTP_RecvUFSSpace(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    int32_t freeSize = 0, totalSize = 0;

    pTemp = Common_SearchData(pData, dataLen, "+QFLDS: ", strlen("+QFLDS: "));

    if (pTemp != NULL)
    { 
        if (2 == sscanf((char*)pTemp, "+QFLDS: %d,%d\r\n", &freeSize, &totalSize))
        {
            pPrivate->ufsFlashFreeSpace = freeSize;

            if (pPrivate->ufsFlashFreeSpace > ATFTP_UFS_FILE_MIN_SIZE)
            {
                if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
                {
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSOpen);
                }
                else
                {
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigContext);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigPSW);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigFileType);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTransferMode);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTimeout);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPOpen);
                }

                ret = TRUE;
            }
            else
            {
                if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
                {
                    SSSnapshot_StopReadItem();
                }
                else
                {
                    SSUcm_SetResult(eSSUcmResult_ModuleNoEnoughSpace);
                }

                CddNetM_DeleteLink(eCddNetMPlatType_File);
            }
        }
    }

    return ret;
}

static uint8_t ATFTP_RecvUFSOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint8_t ret = FALSE;
    int32_t fileHandle = 0;

    pTemp = Common_SearchData(pData, dataLen, "+QFOPEN: ", strlen("+QFOPEN: "));

    if (pTemp != NULL)
    { 
        if (1 == sscanf((char*)pTemp, "+QFOPEN: %d\r\n", &fileHandle))
        {
            pPrivate->fileHandle = fileHandle;
            pPrivate->fileHandleValidFlag = TRUE;
            ret = TRUE;

            if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
            {
                if (pPrivate->fileWriteSuccFlag == TRUE)
                {
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigContext);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigPSW);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigFileType);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTransferMode);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTimeout);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPOpen);
                }
                else
                {
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSWrite);
                }
            }
            else
            {
                CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSSeek);
                CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSRead);
            }
        }
    }

    return ret;
}

static uint8_t ATFTP_RecvUFSWrite(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint16_t bufLen = 0;

    SSSnapshot_ReadItem(g_uploadFileBuf, ATFTP_UPLOAD_SINGLE_PACK_MAX_SIZE, &bufLen);

    if (bufLen > 0 && bufLen == pPrivate->curWriteSize)
    {
        CDDDRVEG800AK_CFG_WriteData(g_uploadFileBuf, bufLen, pSocketCtrl);
        pPrivate->totalWriteSize += bufLen;
    }
    else
    {
        SSSnapshot_StopReadItem();
        CddNetM_DeleteLink(eCddNetMPlatType_File);
    }

    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Send);
    return TRUE;
}

static uint8_t ATFTP_RecvUFSRead(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    uint16_t bufLen = 0;
    uint8_t ret = FALSE;
    uint16_t readLen = 0;
    uint32_t readOffset = 0;

    pTemp = Common_SearchData(pData, dataLen, "CONNECT ", strlen("CONNECT "));

    if (pTemp != NULL)
    {
        pTemp += strlen("CONNECT ");

        pTemp = Common_SearchData(pTemp, 7, "\r\n", strlen("\r\n"));

        if (pTemp != NULL)
        {
            SSUcm_GetReadLenAndOffSet(&readLen, &readOffset);

            if (readLen != 0)
            {
                if ((readLen + strlen("CONNECT ") + strlen("\r\n")) < dataLen)
                {
                    pTemp += strlen("\r\n");

                    if (TRUE == SSUcm_FileDataHandle(pTemp, readLen))
                    {
                        ATFTP_CloseSocket(pSocketCtrl);
                        ret = FALSE;
                    }
                    else
                    {
                        CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSSeek);
                        CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSRead);
                        ret = TRUE;
                    }
                }
                else
                {
                    CddDrvEG800AK_EnterTransparentMode(socketIndex, eCddDrvEG800AKDirection_Recv);
                    ret = TRUE;
                }
            }
        }
    }
    
    return ret;
}

static uint8_t ATFTP_RecvUFSClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->fileHandleValidFlag = FALSE;
    return TRUE;
}


static uint8_t ATFTP_RecvFTPState(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    uint8_t *pTemp = NULL;
    int32_t res = 0;
    int32_t connectState = 0;

    pTemp = Common_SearchData(pData, dataLen, "+QFTPSTAT:", strlen("+QFTPSTAT:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFTPSTAT: %d,%d\r\n", &res, &connectState))
        {
            /* 1-已连接，空闲， 2-正在传输数据 */
            if (connectState == 1 || connectState == 2)
            {
                if (pPrivate->waitFtpConnectOkFlag == TRUE)
                {
                    pPrivate->waitFtpConnectOkFlag = FALSE;
                    ATFTP_SetSocketState(socketIndex, pSocketCtrl, eCddNetMSocketState_ConnectOK);
                }
            }
        }
    }

    return TRUE;
}

static uint8_t ATFTP_RecvFTPSwithPath(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;
    int32_t err1, err2;
    uint8_t *pTemp = NULL;
    uint8_t ret = TRUE;

    pTemp = Common_SearchData(pData, dataLen, "+QFTPCWD:", strlen("+QFTPCWD:"));

    if (pTemp != NULL)
    {
        if (2 == sscanf((char*)pTemp, "+QFTPCWD: %d,%d\r\n", &err1, &err2))
        {
            if (err1 == 0 && err2 == 0)
            {
                if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
                {
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPUpload);
                }
                else
                {
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPDownload);
                }
            }
            else
            {
                CDDDRV_EG800AK_CFG_LogPrint("FTP切换路径失败, err1: %d, err2: %d !\r\n", err1, err2);
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
                ret = FALSE;
            }
        }
    }

    return ret;
}

static uint8_t ATFTP_RecvFTPClose(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;

    ATFTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_WaitReconnect);
    return TRUE;
}

static void ATFTP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMFtpPara_Struct *pFtpPara)
{
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Connecting)
    {
        if (pPrivate->waitFtpConnectOkFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->waitFtpConnectOkTickStart, ATFTP_WAIT_IPOPEN_TIMEOUT))
            {
                pPrivate->waitFtpConnectOkTickStart = Common_GetSystick();
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
            else if (Common_JudgeTimeoutMs(pPrivate->cycleDetectSocketStateTickStart, ATFTP_DECTECT_STATE_PERIOD))
            { 
                pPrivate->cycleDetectSocketStateTickStart = Common_GetSystick();
                CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPState);
            }
            else
            {}
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_ConnectOK)
    {
        if (pPrivate->uploadFileTickStartFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->upLoadFileTickStart, ATFTP_UPLOAD_FILE_TIMEOUT))
            {
                pPrivate->uploadFileTickStartFlag = FALSE;
                pPrivate->upLoadFileTickStart = Common_GetSystick();
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
        }

        if(pPrivate->downLoadFileTickStartFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(pPrivate->downLoadFileTickStart, ATFTP_DOWNLOAD_FILE_TIMEOUT))
            {
                pPrivate->downLoadFileTickStartFlag = FALSE;
                pPrivate->downLoadFileTickStart = Common_GetSystick();
                pPrivate->abnormalCloseFlag = TRUE;
                ATFTP_CloseSocket(pSocketCtrl);
            }
        }
    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_Abnormal)
    {

    }
    else if (pSocketCtrl->eSocketState == eCddNetMSocketState_WaitReconnect)
    {
        if (pPrivate->abnormalCloseFlag == TRUE)
        {
            if (pPrivate->reconnectInterval == 0)
            {
                pSocketCtrl->reconectTimes++;

                if (pSocketCtrl->reconectTimes >= CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES)
                {
                    CddNetM_DeleteLink(eCddNetMPlatType_File);

                    if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
                    {
                        SSSnapshot_StopReadItem();
                    }
                    else
                    {
                        SSUcm_SetResult(eSSUcmResult_GetFileErr);
                    }
                }

                if (pSocketCtrl->socketDisconnectCallback != NULL)
                {
                    pSocketCtrl->socketDisconnectCallback();
                }

                pPrivate->reconnectInterval = CDDDRV_EG800AK_CFG_RECONECT_TIMEOUT(1);
                pSocketCtrl->disconectTickStart = Common_GetSystick();
                CDDDRV_EG800AK_CFG_LogPrint("[socket: %d] %d ms 后进行第 %d 次 重新连接!\r\n", socketIndex, pPrivate->reconnectInterval, pSocketCtrl->reconectTimes);
            }
            else
            {
                if (Common_JudgeTimeoutMs(pSocketCtrl->disconectTickStart, pPrivate->reconnectInterval))
                {
                    pPrivate->abnormalCloseFlag = FALSE;
                    ATFTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Init);
                }
            }
        }
        else
        {
            ATFTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Init);
        }
    }
    else
    {}
}

void ATFTP_ReavTransparentData(void *modulePara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKCtrl_Struct *pModulePara = (CddDrvEG800AKCtrl_Struct *)modulePara;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &pModulePara->stSocketCtrl[g_socketIndex];
    uint16_t readLen = 0;
    uint32_t readOffset = 0;

    SSUcm_GetReadLenAndOffSet(&readLen, &readOffset);

    if (readLen != 0 && readLen <= dataLen)
    {
        if (TRUE == SSUcm_FileDataHandle(pData, readLen))
        {
            ATFTP_CloseSocket(pSocketCtrl);
        }
        else
        {
            CddDrvEG800AK_AddCmd(g_socketIndex, eATFTPCmd_UFSSeek);
            CddDrvEG800AK_AddCmd(g_socketIndex, eATFTPCmd_UFSRead);
        }
    }
    else
    {
        SSUcm_SetResult(eSSUcmResult_UnexpectedError);
    }
}

void ATFTP_StateHandle(uint8_t socketIndex, void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE && CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work)
        {
            g_socketIndex = socketIndex;
            
            if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
            {
                if (pPrivate->fileWriteSuccFlag == TRUE)
                {
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigContext);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigPSW);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigFileType);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTransferMode);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPConfigTimeout);
                    CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPOpen);
                }
                else
                {
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSDeleteFile);
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSQuerySpace);
                }
            }
            else
            {
                if (pPrivate->fileHandleValidFlag == TRUE)
                {
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSSeek);
                }
                else
                {
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSDeleteFile);
                    CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSQuerySpace);
                }
            }

            ATFTP_SetSocketState(g_socketIndex, pSocketCtrl, eCddNetMSocketState_Connecting);
        }
    }
    else
    {
        ATFTP_SocketStateMange(socketIndex, pSocketCtrl, pFTPPara);
    }
}

void ATFTP_CloseSocket(void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    ATFTPPrivate_Struct *pPrivate = (ATFTPPrivate_Struct *)pSocketCtrl->user_data;

    pPrivate->waitFtpConnectOkFlag = FALSE;
    
    if (pSocketCtrl->eSocketState != eCddNetMSocketState_Init &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_Abnormal &&
        pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect)
    {
        CddDrvEG800AK_ClearSocketCmd(pSocketCtrl->socketIndex);

        if (pPrivate->fileHandleValidFlag == TRUE)
        {
            CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_UFSClose);
        }

        CddDrvEG800AK_AddCmd(pSocketCtrl->socketIndex, eATFTPCmd_FTPClose);
        ATFTP_SetSocketState(pSocketCtrl->socketIndex, pSocketCtrl, eCddNetMSocketState_Abnormal);
    }
}
















