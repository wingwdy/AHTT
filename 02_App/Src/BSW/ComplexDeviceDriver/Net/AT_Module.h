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
#ifndef AT_MODULE_H_
#define AT_MODULE_H_

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
    eATModuleCmd_Null,
    eATModuleCmd_ATE0,                              /* 关闭回显 */
    eATModuleCmd_QueryModule,                       /* 识别模块 */
    eATModuleCmd_QuerySimRecognizeStatus,           /* sim识别状态查询 */
    eATModuleCmd_QueryIccid,                        /* sim卡iccid查询 */
    eATModuleCmd_QueryCsq,                          /* 信号强度查询 */
    eATModuleCmd_QueryNtpClk,                       /* 查询NTP时间 */
    eATModuleCmd_QueryCGREG,                        /* PS服务网络连接状态查询 */
    eATModuleCmd_QueryCOPS,                         /* 查询运营商 */
    eATModuleCmd_QueryNetWorkInfo,                  /* 查询网络信息 */
    eATModuleCmd_ConfigAPN,                         /* 配置APN */
    eATModuleCmd_ActivePDP,                         /* 激活PDP */
    eATModuleCmd_QueryPDPState,                     /* 查询PDP状态 */
    eATModuleCmd_SetCFUN0,                          /* 设置最小功能模式 */
    eATModuleCmd_SetCFUN1,                          /* 设置全功能模式 */
    eATModuleCmd_QueryCount,
}ATModuleCmd_Enum;





/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const ATCmdDescribtor_Struct c_stModuleATCmdDescribtor[eATModuleCmd_QueryCount];
extern const ATUrcDescribtor_Struct c_stATUrcDescribtor[14];
/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif





















