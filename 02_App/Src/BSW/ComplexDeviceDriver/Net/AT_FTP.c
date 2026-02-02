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
#include "FrameQueue.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/
typedef struct
{
    uint32_t ufsFlashFreeSpace;
    uint8_t fileHandle;
}ATFTPPrivate_Struct;



/*******************************************************************************
*    Enum Definition
*******************************************************************************/



/*******************************************************************************
*    Typedef Definition
*******************************************************************************/




/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t ATFTP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID);


static uint8_t ATFTP_RecvOKACK(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSSpace(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
static uint8_t ATFTP_RecvUFSOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);

static uint16_t ATFTP_PackUfsOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
const ATCmdDescribtor_Struct c_stFTPATCmdDescribtor[eATFTPCmd_Count] =
{
    [eATFTPCmd_UFSDeleteFile] =
    { "AT+QFDEL=\"*\"\r\n",         "AT+QFDEL",         3,      5000,      3000,  TRUE, "删除UFS文件",
      NULL,                         ATFTP_RecvOKACK,                                ATFTP_FailHandle},

    [eATFTPCmd_UFSQuerySpace] =
    { "AT+QFLDS=\"UFS\"\r\n",       "+QFLDS: ",         3,      5000,      3000,  TRUE, "查询UFS剩余空间",
      NULL,                         ATFTP_RecvUFSSpace,                             ATFTP_FailHandle},

    [eATFTPCmd_UFSOpen] =
    { "AT+QFOPEN=\"[FILE]\",[MODE]\r\n", "+QFOPEN:",    3,      5000,      3000,  TRUE, "打开/新建文件",
      ATFTP_PackUfsOpen,            ATFTP_RecvUFSOpen,                              ATFTP_FailHandle},
};




/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t ATFTP_FailHandle(uint8_t socketIndex, void * socketPara, uint8_t atTaskID)
{
    return TRUE;
}

static uint16_t ATFTP_PackUfsOpen(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;

    nATLen = Common_ReplaceStr(pData, nATLen, "[FILE]", pFTPPara->fileName, strlen(pFTPPara->fileName), "D3_A32FB.log");

    if (pFTPPara->eMode == eCddNetMFtpMode_Download)
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[MODE]", 2, 2);
    }
    else
    {
        nATLen = Common_ReplaceNum(pData, nATLen, "[MODE]", 1, 1);
    }

    return nATLen;
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

static uint8_t ATFTP_RecvUFSSpace(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketPara;
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
            ret = TRUE;
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
            ret = TRUE;

            if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
            {

            }
            else
            {
                
            }
        }
    }

    return ret;
}


static void ATFTP_SocketStateMange(uint8_t socketIndex, CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl, CddNetMFtpPara_Struct *pFtpPara)
{



}

void ATFTP_StateHandle(uint8_t socketIndex, void *socketCtrl)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = (CddDrvEG800AKSocketCtrl_Struct *)socketCtrl;
    CddNetMFtpPara_Struct *pFTPPara = (CddNetMFtpPara_Struct *)pSocketCtrl->specificPara;

    if (pSocketCtrl->eSocketState == eCddNetMSocketState_Init)
    {
        if (pSocketCtrl->usedFlag == TRUE && CddDrvEG800AK_GetModuleState() == eCddNetMModuleState_Work)
        {
            CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSDeleteFile);
            CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSQuerySpace);

            if (pFTPPara->eMode == eCddNetMFtpMode_Upload)
            {
                CddDrvEG800AK_AddCmd(socketIndex, eATFTPCmd_UFSOpen);
            }

            memset(pSocketCtrl->user_data, 0, sizeof(pSocketCtrl->user_data));
            pSocketCtrl->eSocketState = eCddNetMSocketState_Connecting;
        }
    }
    else
    {
        ATFTP_SocketStateMange(socketIndex, pSocketCtrl, pFTPPara);
    }
}

void ATFTP_CloseSocket(void *socketCtrl)
{




}
















