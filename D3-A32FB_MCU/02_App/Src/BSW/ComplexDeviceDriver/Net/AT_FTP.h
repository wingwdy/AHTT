/******************************************************************************
* File Name          : template_Config.h
* Description        : Code for xxxxxxxxxxx
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef AT_FTP_H_
#define AT_FTP_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "AT_Describtor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ATFTP_UPLOAD_SINGLE_PACK_MAX_SIZE       (1024U)
#define ATFTP_UFS_FILE_MIN_SIZE                 (100 * 1024U)

#define ATFTP_WAIT_IPOPEN_TIMEOUT               (90000U)
#define ATFTP_UPLOAD_FILE_TIMEOUT               (60000U)
#define ATFTP_DOWNLOAD_FILE_TIMEOUT             (60000U)

#define ATFTP_DECTECT_STATE_PERIOD              (3000U)

#define ATFTP_DEFAULT_USER_NAME                 "gn_ftp_fw_reader"
#define ATFTP_DEFAULT_USER_PSW                  "d2aa28ee9a8693db"
#define ATFTP_DEFAULT_USER_IP                   "fw.ftp.gongniu.cn"
#define ATFTP_DEFAULT_USER_PORT                 21
#define ATFTP_DEFAULT_USER_PATH                 "/AC_pile/D3_A32FB/"
#define ATFTP_DEFAULT_USER_FILE                 "D3_A32FB_DEFAULT"
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eATFTPCmd_Null,
    
    eATFTPCmd_UFSDeleteFile,
    eATFTPCmd_UFSQuerySpace,
    eATFTPCmd_UFSOpen,
    eATFTPCmd_UFSSeek,
    eATFTPCmd_UFSRead,
    eATFTPCmd_UFSWrite,
    eATFTPCmd_UFSClose,

    eATFTPCmd_FTPConfigContext,
    eATFTPCmd_FTPConfigPSW,
    eATFTPCmd_FTPConfigFileType,
    eATFTPCmd_FTPConfigTransferMode,
    eATFTPCmd_FTPConfigTimeout,
    eATFTPCmd_FTPOpen,
    eATFTPCmd_FTPState,
    eATFTPCmd_FTPSwithPath,
    eATFTPCmd_FTPUpload,
    eATFTPCmd_FTPDownload,
    eATFTPCmd_FTPClose,

    eATFTPCmd_Count,
}ATFTPCmd_Enum;
/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const ATCmdDescribtor_Struct c_stFTPATCmdDescribtor[eATFTPCmd_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void ATFTP_StateHandle(uint8_t socketIndex, void *socketCtrl);
void ATFTP_CloseSocket(void *socketCtrl);
uint32_t ATFTP_UrcRecvWrite(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATFTP_UrcRecvOpen(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATFTP_UrcRecvPut(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint32_t ATFTP_UrcRecvGet(uint8_t *pData, void * modulePara, uint16_t dataLen);
uint8_t ATFTP_ReavTransparentData(void *modulePara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
#endif





















