/******************************************************************************
* File Name          : SS_CsmConfig.h
* Description        : Configuration file for the Cryptographic Service Manager
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2026/03/02      V1.0.0      chenls    初版创建
*
*******************************************************************************/

#ifndef SS_CSM_CONFIG_H
#define SS_CSM_CONFIG_H


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "DS_LogM.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define SSCSM_CFG_MAX_KEYS                           2     /* 最大密钥数量 */
#define SSCSM_CFG_MAX_CONTEXTS                       8     /* 最大上下文数量 */
#define SSCSM_CFG_MAX_BUFFER_SIZE                    128   /* 最大缓冲区大小 */

/* 功能开关 */
#define SSCSM_CFG_SUPPORT_AES_CBC                    TRUE  /* 支持 AES CBC 模式 */
#define SSCSM_CFG_SUPPORT_RSA                        TRUE  /* 支持 RSA 算法 */

/* 填充模式开关 */
#define SSCSM_CFG_SUPPORT_PKCS7_PADDING              TRUE  /* 支持 PKCS#7 填充 */
#define SSCSM_CFG_SUPPORT_ZERO_PADDING               TRUE  /* 支持零填充 */
#define SSCSM_CFG_SUPPORT_PKCS1_V15_PADDING          TRUE  /* 支持 PKCS#1 v1.5 填充 */
#define SSCSM_CFG_SUPPORT_PKCS1_V21_PADDING          FALSE /* 支持 PKCS#1 v2.1 填充 */

#define SSCSM_CFG_LogPrint(fmt, ...)                 DSLOGM_Debug(DSLogMModule_System, fmt, ##__VA_ARGS__)
#define SSCSM_CFG_ErrorPrint(fmt, ...)               DSLOGM_Error(DSLogMModule_System, fmt, ##__VA_ARGS__)


/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/


/*******************************************************************************
*    Function Source Code
*******************************************************************************/


#endif /* SS_CSM_CONFIG_H */