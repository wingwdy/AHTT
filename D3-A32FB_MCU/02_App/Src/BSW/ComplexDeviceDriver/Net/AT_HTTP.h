/******************************************************************************
* File Name          : AT_HTTP.h
* Description        : HTTP AT command module
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/06/15      V1.0.0      hzb        初版创建
*2026/06/17      V1.0.1      hzb        适配MQTT框架,支持GET/GETEX/POST
*
******************************************************************************/
#ifndef AT_HTTP_H_
#define AT_HTTP_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "AT_Describtor.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define ATHTTP_WAIT_CONNECT_TIMEOUT         (60000U)

/******************************************************************************
*    Enum Definition
******************************************************************************/
/* http body数据类型 */
typedef enum
{
    eATHTTP_ContentType_AppXWWW = 0,        /* application/x-www-form-urlencoded */
    eATHTTP_ContentType_TextPlain,          /* text/plain */
    eATHTTP_ContentType_AppOctet,           /* application/octet-stream */
    eATHTTP_ContentType_MultilFormData,     /* multipart/form-data */
    eATHTTP_ContentType_AppJson,            /* application/json */
    eATHTTP_ContentType_ImageJpeg,          /* image/jpeg */

}ATHTTP_ContentType_Enum;

typedef enum
{
    eATHTTPCmd_NULL,
    eATHTTPCmd_CFG_ContextId,
    eATHTTPCmd_CFG_RequestHeader,
    eATHTTPCmd_CFG_ResponseHeader,
    eATHTTPCmd_CFG_ContentType,
    eATHTTPCmd_CFG_RSPOUT,
    eATHTTPCmd_SET_URL,
    eATHTTPCmd_Method_GET,
    eATHTTPCmd_Method_GETEX,
    eATHTTPCmd_Method_POST,
    eATHTTPCmd_READ,

    eATHTTPCmd_Count,
} ATHTTPCmd_Enum;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const ATCmdDescribtor_Struct c_stHTTPATCmdDescribtor[eATHTTPCmd_Count];

/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void ATHTTP_StateHandle(uint8_t socketIndex, void *socketCtrl);
void ATHTTP_CloseSocket(void *socketCtrl);
uint16_t ATHTTP_PackResHeader(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
uint16_t ATHTTP_PackContextType(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
uint16_t ATHTTP_PackURL(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
uint16_t ATHTTP_PackBody(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
uint16_t ATHTTP_PackGetEx(uint8_t socketIndex, void *socketPara, uint8_t *pData, uint16_t nATLen);
uint8_t ATHTTP_RecvPOST(uint8_t socketID, void *socketPara, uint8_t *pData, uint16_t dataLen, uint16_t *pDealLen);
uint8_t ATHTTP_FailHandle(uint8_t socketID, void *socketPara, uint8_t atTaskID);
uint32_t ATHTTP_UrcQHTTPPost(uint8_t *pData, void *modulePara, uint16_t dataLen);
uint32_t ATHTTP_UrcQHTTPGet(uint8_t *pData, void *modulePara, uint16_t dataLen);
uint32_t ATHTTP_UrcQHTTPRead(uint8_t *pData, void *modulePara, uint16_t dataLen);

#endif /* AT_HTTP_H_ */
