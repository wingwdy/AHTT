/******************************************************************************
* File Name          : SS_Csm.h
* Description        : Header file for the Implementation of the Cryptographic Service Manager
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

#ifndef SS_CSM_H
#define SS_CSM_H


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "SS_CsmConfig.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eSSCsmRet_Ok = 0,                 /* 操作成功 */
    eSSCsmRet_NotInitialized,         /* 模块未初始化 */
    eSSCsmRet_InvalidParam,           /* 参数无效 */
    eSSCsmRet_KeyNotFound,            /* 密钥未找到 */
    eSSCsmRet_AlgorithmNotSupported,  /* 算法不支持 */
    eSSCsmRet_OperationFailed,        /* 操作失败 */
    eSSCsmRet_BufferTooSmall          /* 缓冲区太小 */
}SSCsmRet_Enum;

typedef enum
{
    eSSCsmAlg_AesCbc = 0,            /* AES CBC 模式 */
    eSSCsmAlg_Rsa                    /* RSA 算法 */
}SSCsmAlgorithm_Enum;

typedef enum
{
    eSSCsmKeyType_Symmetric = 0,    /* 对称密钥 */
    eSSCsmKeyType_Asymmetric        /* 非对称密钥 */
}SSCsmKeyType_Enum;

typedef enum
{
    eSSCsmMode_Encrypt = 0,           /* 加密模式 */
    eSSCsmMode_Decrypt                /* 解密模式 */
}SSCsmOperationMode_Enum;

typedef enum
{
    eSSCsmPadding_NoPadding = 0,      /* 无填充 */
    eSSCsmPadding_Pkcs7,              /* PKCS#7填充 */
    eSSCsmPadding_Pkcs5,              /* PKCS#5填充 */
    eSSCsmPadding_ZeroPadding         /* 零填充 */
}SSCsmPaddingMode_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


typedef struct
{
    SSCsmAlgorithm_Enum eAlgorithm;     /* 使用的算法 */
    uint8_t keyHandle;                  /* 密钥句柄 */
    SSCsmOperationMode_Enum eMode;      /* 操作模式 */
    SSCsmPaddingMode_Enum ePaddingMode; /* 填充模式 */
    uint8_t *iv;                        /* 初始化向量（如需要） */
    uint8_t ivLength;                   /* 初始化向量长度 */
}SSCsmContext_Struct;

typedef struct
{
    SSCsmKeyType_Enum eKeyType;         /* 密钥类型 */
    SSCsmAlgorithm_Enum eAlgorithm;     /* 算法类型 */
    SSCsmPaddingMode_Enum ePaddingMode; /* 填充模式 */
    const uint8_t *pKeyData;            /* 密钥数据 */
    uint16_t keyDataLength;             /* 密钥数据长度 */
    const uint8_t *pIv;                 /* 初始化向量（如需要） */
    uint8_t ivLength;                   /* 初始化向量长度 */
}SSCsmKeyImportParams_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void SSCsm_InitMemory(void);
SSCsmRet_Enum SSCsm_Encrypt(uint8_t keyHandle, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength, const uint8_t *pIv, uint8_t ivLength);
SSCsmRet_Enum SSCsm_Decrypt(uint8_t keyHandle, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength, const uint8_t *pIv, uint8_t ivLength);
SSCsmRet_Enum SSCsm_ImportKey(const SSCsmKeyImportParams_Struct *pParams, uint8_t *pKeyHandle);
SSCsmRet_Enum SSCsm_GenerateAndEncryptKey(uint8_t keyHandle, uint8_t *pEncryptedKey, uint32_t *pEncryptedKeyLength, uint8_t *pRandomKey, uint32_t *pRandomKeyLength);
void SSCsm_MainFunction(void);

#endif /* SS_CSM_H */