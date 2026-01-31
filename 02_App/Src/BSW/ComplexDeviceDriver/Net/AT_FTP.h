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


/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eATFTPCmd_Null,
    eATFTPCmd_UFSDeleteFile,
    eATFTPCmd_UFSQuerySpace,

    eATFTPCmd_FTPConfigContext,
    eATFTPCmd_FTPConfigPSW,
    eATFTPCmd_FTPConfigFileType,
    eATFTPCmd_FTPConfigTimeout,
    eATFTPCmd_FTPOpen,
    eATFTPCmd_FTPSwithPath,
    eATFTPCmd_FTPDownload,
    eATFTPCmd_FTPClose,

    eATFTPCmd_UFSOpen,
    eATFTPCmd_UFSSeek,
    eATFTPCmd_UFSRead,

    eATFtbCmd_UFSConfigTimeout,
    eATFTPCmd_UFSWrite,

    eATFTPCmd_UFSClose,
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

#endif





















