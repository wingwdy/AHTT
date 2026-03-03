/******************************************************************************
* File Name          : SS_Csm.c
* Description        : Code for the Implementation of the Cryptographic Service Manager
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


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "SS_Csm.h"
#include "SS_CsmConfig.h"
#include "DS_LogM.h"
#include "Common.h"
#include "mbedtls/cipher.h"
#include "mbedtls/rsa.h"
#include "mbedtls/base64.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/bignum.h"



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
    uint8_t          used;            /* 是否使用 */
    SSCsmKeyType_Enum eKeyType;         /* 密钥类型 */
    SSCsmAlgorithm_Enum eAlgorithm;     /* 算法类型 */
    SSCsmPaddingMode_Enum ePaddingMode; /* 填充模式 */
    uint16_t         keyBitLength;    /* 密钥长度（位） */
    uint8_t          keyData[SSCSM_CFG_MAX_BUFFER_SIZE]; /* 密钥数据 */
    uint16_t         keyDataLength;   /* 密钥数据长度 */
    uint8_t          iv[16];          /* 初始化向量 */
    uint8_t          ivLength;        /* 初始化向量长度 */
}SSCsmKeyInfo_Struct;

typedef struct
{
    uint8_t              initialized;          /* 初始化标志 */
    uint8_t              keyCount;             /* 当前密钥数量 */
    SSCsmKeyInfo_Struct  keys[SSCSM_CFG_MAX_KEYS]; /* 密钥信息数组 */
}SSCsmInternalCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SSCsmInternalCtx_Struct g_stSSCsmCtx = {0};


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static SSCsmRet_Enum SSCsm_IsAlgorithmSupported(SSCsmAlgorithm_Enum eAlgorithm);
static SSCsmRet_Enum SSCsm_IsKeyValid(uint8_t keyHandle);
static int SSCsm_SetPaddingMode(mbedtls_cipher_context_t *ctx, SSCsmPaddingMode_Enum ePaddingMode);
static SSCsmRet_Enum SSCsm_AesCbcCrypt(const SSCsmContext_Struct *pContext, const uint8_t *pInput, uint32_t inputLength, uint8_t *pOutput, uint32_t *pOutputLength, mbedtls_operation_t operation);
static SSCsmRet_Enum SSCsm_AesCbcEncrypt(const SSCsmContext_Struct *pContext, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength);
static SSCsmRet_Enum SSCsm_AesCbcDecrypt(const SSCsmContext_Struct *pContext, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength);
static int SSCsm_LoadRsaPublicKey(mbedtls_rsa_context *rsa, const uint8_t *rsa_key_128b);
static int SSCsm_GenerateRandomKey(uint8_t *key, size_t key_len);
static SSCsmRet_Enum SSCsm_RsaCrypt(const SSCsmContext_Struct *pContext, const uint8_t *pInput, uint32_t inputLength, uint8_t *pOutput, uint32_t *pOutputLength, int operation);
static SSCsmRet_Enum SSCsm_RsaEncrypt(const SSCsmContext_Struct *pContext, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength);
static SSCsmRet_Enum SSCsm_RsaDecrypt(const SSCsmContext_Struct *pContext, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

static SSCsmRet_Enum SSCsm_IsAlgorithmSupported(SSCsmAlgorithm_Enum eAlgorithm)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_AlgorithmNotSupported;
    
    switch (eAlgorithm)
    {
        case eSSCsmAlg_AesCbc:
            eStatus = eSSCsmRet_Ok;
            break;
        case eSSCsmAlg_Rsa:
            eStatus = eSSCsmRet_Ok;
            break;
        default:
            eStatus = eSSCsmRet_AlgorithmNotSupported;
            break;
    }
    
    return eStatus;
}

static SSCsmRet_Enum SSCsm_IsKeyValid(uint8_t keyHandle)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_KeyNotFound;
    
    if (keyHandle < SSCSM_CFG_MAX_KEYS && g_stSSCsmCtx.keys[keyHandle].used)
    {
        eStatus = eSSCsmRet_Ok;
    }
    
    return eStatus;
}

/* 设置填充模式的封装函数 */
static int SSCsm_SetPaddingMode(mbedtls_cipher_context_t *ctx, SSCsmPaddingMode_Enum ePaddingMode)
{
    int ret = MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE;
    
    switch (ePaddingMode)
    {
        case eSSCsmPadding_NoPadding:
            ret = mbedtls_cipher_set_padding_mode(ctx, MBEDTLS_PADDING_NONE);
            break;
        case eSSCsmPadding_Pkcs7:
        case eSSCsmPadding_Pkcs5: /* PKCS#5 与 PKCS#7 在 AES 中是相同的 */
            ret = mbedtls_cipher_set_padding_mode(ctx, MBEDTLS_PADDING_PKCS7);
            break;
        case eSSCsmPadding_ZeroPadding:
            ret = mbedtls_cipher_set_padding_mode(ctx, MBEDTLS_PADDING_ZEROS);
            break;
        default:
            ret = MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE;
            break;
    }
    
    return ret;
}

/* AES CBC 通用加密/解密函数 - 使用mbedtls cipher库实现 */
static SSCsmRet_Enum SSCsm_AesCbcCrypt(const SSCsmContext_Struct *pContext, const uint8_t *pInput, uint32_t inputLength, uint8_t *pOutput, uint32_t *pOutputLength, mbedtls_operation_t operation)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    SSCsmKeyInfo_Struct *pKey = &g_stSSCsmCtx.keys[pContext->keyHandle];
    uint32_t blockSize = 16; /* AES 块大小为16字节 */
    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *cipher_info;
    int ret;
    uint8_t iv[16];
    size_t olen;
    
    /* 检查IV */
    if (pContext->iv == NULL || pContext->ivLength != blockSize)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    /* 检查输入长度是否为块大小的整数倍（无填充模式） */
    else if (pContext->ePaddingMode == eSSCsmPadding_NoPadding && inputLength % blockSize != 0)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    /* 检查输出缓冲区大小 */
    else if (*pOutputLength < inputLength)
    {
        *pOutputLength = inputLength;
        eStatus = eSSCsmRet_BufferTooSmall;
    }
    else
    {
        /* 初始化cipher上下文 */
        mbedtls_cipher_init(&ctx);
        
        /* 获取cipher信息 */
        cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC);
        if (cipher_info == NULL)
        {
            eStatus = eSSCsmRet_OperationFailed;
        }
        else
        {
            /* 设置cipher */
            ret = mbedtls_cipher_setup(&ctx, cipher_info);
            if (ret != 0)
            {
                eStatus = eSSCsmRet_OperationFailed;
            }
            else
            {
                /* 设置填充模式 */
                ret = SSCsm_SetPaddingMode(&ctx, pContext->ePaddingMode);
                if (ret != 0)
                {
                    eStatus = eSSCsmRet_OperationFailed;
                }
                else
                {
                    /* 设置密钥 */
                    ret = mbedtls_cipher_setkey(&ctx, pKey->keyData, pKey->keyBitLength, operation);
                    if (ret != 0)
                    {
                        eStatus = eSSCsmRet_OperationFailed;
                    }
                    else
                    {
                        /* 复制IV */
                        memcpy(iv, pContext->iv, blockSize);
                        
                        /* 执行AES CBC加密/解密 */
                        ret = mbedtls_cipher_crypt(&ctx, iv, blockSize, pInput, inputLength, pOutput, &olen);
                        if (ret != 0)
                        {
                            eStatus = eSSCsmRet_OperationFailed;
                        }
                        else
                        {
                            *pOutputLength = (uint32_t)olen;
                        }
                    }
                }
            }
        }
        
        /* 释放cipher上下文 */
        mbedtls_cipher_free(&ctx);
    }
    
    return eStatus;
}

/* AES CBC 加密函数 - 使用mbedtls cipher库实现 */
static SSCsmRet_Enum SSCsm_AesCbcEncrypt(const SSCsmContext_Struct *pContext, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength)
{
    return SSCsm_AesCbcCrypt(pContext, pPlaintext, plaintextLength, pCiphertext, pCiphertextLength, MBEDTLS_ENCRYPT);
}

/* AES CBC 解密函数 - 使用mbedtls cipher库实现 */
static SSCsmRet_Enum SSCsm_AesCbcDecrypt(const SSCsmContext_Struct *pContext, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength)
{
    return SSCsm_AesCbcCrypt(pContext, pCiphertext, ciphertextLength, pPlaintext, pPlaintextLength, MBEDTLS_DECRYPT);
}

/* RSA 通用加密/解密函数 - 使用mbedtls RSA库实现 */
static SSCsmRet_Enum SSCsm_RsaCrypt(const SSCsmContext_Struct *pContext, const uint8_t *pInput, uint32_t inputLength, uint8_t *pOutput, uint32_t *pOutputLength, int operation)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    SSCsmKeyInfo_Struct *pKey = &g_stSSCsmCtx.keys[pContext->keyHandle];
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    int ret;
    size_t olen;
    const char *personalization = "CSM_RSA_Operation";
    
    /* 检查输入参数 */
    if (pInput == NULL || pOutput == NULL || pOutputLength == NULL)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    /* 检查输入长度是否为RSA密钥长度（解密时） */
    else if (operation == MBEDTLS_RSA_PRIVATE && inputLength != pKey->keyBitLength / 8)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    /* 检查输出缓冲区大小 */
    else if (*pOutputLength < pKey->keyBitLength / 8)
    {
        *pOutputLength = pKey->keyBitLength / 8;
        eStatus = eSSCsmRet_BufferTooSmall;
    }
    else
    {
        /* 初始化RSA上下文 */
        mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        
        /* 初始化随机数生成器 */
        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 
                                    (const unsigned char *)personalization, 
                                    strlen(personalization));
        if (ret != 0)
        {
            eStatus = eSSCsmRet_OperationFailed;
        }
        else
        {
            /* 从128字节格式加载RSA-512公钥 */
            ret = SSCsm_LoadRsaPublicKey(&rsa, pKey->keyData);
            if (ret != 0)
            {
                eStatus = eSSCsmRet_OperationFailed;
            }
            else
            {
                /* 执行RSA加密/解密 */
                if (operation == MBEDTLS_RSA_PUBLIC)
                {
                    ret = mbedtls_rsa_pkcs1_encrypt(&rsa, mbedtls_ctr_drbg_random, &ctr_drbg, 
                                                  MBEDTLS_RSA_PUBLIC, inputLength, pInput, pOutput);
                    if (ret != 0)
                    {
                        eStatus = eSSCsmRet_OperationFailed;
                    }
                    else
                    {
                        *pOutputLength = pKey->keyBitLength / 8;
                    }
                }
                else if (operation == MBEDTLS_RSA_PRIVATE)
                {
                    ret = mbedtls_rsa_pkcs1_decrypt(&rsa, mbedtls_ctr_drbg_random, &ctr_drbg, 
                                                  MBEDTLS_RSA_PRIVATE, &olen, pInput, pOutput, *pOutputLength);
                    if (ret != 0)
                    {
                        eStatus = eSSCsmRet_OperationFailed;
                    }
                    else
                    {
                        *pOutputLength = (uint32_t)olen;
                    }
                }
                else
                {
                    eStatus = eSSCsmRet_InvalidParam;
                }
            }
        }
        
        /* 释放RSA上下文 */
        mbedtls_rsa_free(&rsa);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }
    
    return eStatus;
}

/* RSA 加密函数 - 使用mbedtls RSA库实现 */
static SSCsmRet_Enum SSCsm_RsaEncrypt(const SSCsmContext_Struct *pContext, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength)
{
    return SSCsm_RsaCrypt(pContext, pPlaintext, plaintextLength, pCiphertext, pCiphertextLength, MBEDTLS_RSA_PUBLIC);
}

/* RSA 解密函数 - 使用mbedtls RSA库实现 */
static SSCsmRet_Enum SSCsm_RsaDecrypt(const SSCsmContext_Struct *pContext, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength)
{
    return SSCsm_RsaCrypt(pContext, pCiphertext, ciphertextLength, pPlaintext, pPlaintextLength, MBEDTLS_RSA_PRIVATE);
}

// 字节序转换（主机序→网络序）
static uint16_t SSCsm_htons(uint16_t val)
{
    return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
}

static uint32_t SSCsm_htonl(uint32_t val)
{
    return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) |
           ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

// 从128字节原始数据加载RSA-512公钥（前64字节=模数n，后64字节=指数e）
static int SSCsm_LoadRsaPublicKey(mbedtls_rsa_context *rsa, const uint8_t *rsa_key_128b)
{
    int ret;
    mbedtls_mpi n, e; // 模数、指数

    mbedtls_mpi_init(&n);
    mbedtls_mpi_init(&e);

    // 1. 加载模数n（前64字节，512位）
    ret = mbedtls_mpi_read_binary(&n, rsa_key_128b, 64);
    if (ret != 0)
        return ret;

    // 2. 加载指数e（后64字节中取低2字节，通常e=65537=0x010001）
    ret = mbedtls_mpi_read_binary(&e, rsa_key_128b + 64, 2);
    if (ret != 0)
        goto exit_mpi;

    // 3. 设置RSA公钥参数
    ret = mbedtls_rsa_import(rsa, &n, &e, NULL, NULL, NULL);
    if (ret != 0)
        goto exit_mpi;

    // 4. 验证RSA参数有效性
    ret = mbedtls_rsa_check_pubkey(rsa);

 exit_mpi:
    mbedtls_mpi_free(&n);
    mbedtls_mpi_free(&e);
    return ret;
}

// 生成随机密钥（16字节）
static int SSCsm_GenerateRandomKey(uint8_t *key, size_t key_len)
{
    int ret;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *personalization = "CSM_Random_Key_Generator";

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // 初始化随机数生成器
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 
                                (const unsigned char *)personalization, 
                                strlen(personalization));
    if (ret != 0)
        goto exit;

    // 生成随机密钥
    ret = mbedtls_ctr_drbg_random(&ctr_drbg, key, key_len);

 exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return ret;
}

void SSCsm_InitMemory(void)
{
    /* 初始化内部上下文 */
    memset(&g_stSSCsmCtx, 0, sizeof(SSCsmInternalCtx_Struct));
    g_stSSCsmCtx.initialized = TRUE;
}

SSCsmRet_Enum SSCsm_Encrypt(uint8_t keyHandle, const uint8_t *pPlaintext, uint32_t plaintextLength, uint8_t *pCiphertext, uint32_t *pCiphertextLength, const uint8_t *pIv, uint8_t ivLength)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    SSCsmKeyInfo_Struct *pKey = &g_stSSCsmCtx.keys[keyHandle];
    SSCsmContext_Struct stContext;
    
    /* 验证参数 */
    if (pPlaintext == NULL || pCiphertext == NULL || pCiphertextLength == NULL)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    else if (!g_stSSCsmCtx.initialized)
    {
        eStatus = eSSCsmRet_NotInitialized;
    }
    else
    {
        /* 检查密钥是否存在 */
        eStatus = SSCsm_IsKeyValid(keyHandle);
        if (eStatus == eSSCsmRet_Ok)
        {
            /* 构建上下文 */
            stContext.eAlgorithm = pKey->eAlgorithm;
            stContext.keyHandle = keyHandle;
            stContext.eMode = eSSCsmMode_Encrypt;
            stContext.ePaddingMode = pKey->ePaddingMode;
            stContext.iv = (uint8_t *)pIv;
            stContext.ivLength = ivLength;
            
            /* 支持AES CBC和RSA模式 */
            if (pKey->eAlgorithm == eSSCsmAlg_AesCbc)
            {
                eStatus = SSCsm_AesCbcEncrypt(&stContext, pPlaintext, plaintextLength, pCiphertext, pCiphertextLength);
            }
            else if (pKey->eAlgorithm == eSSCsmAlg_Rsa)
            {
                eStatus = SSCsm_RsaEncrypt(&stContext, pPlaintext, plaintextLength, pCiphertext, pCiphertextLength);
            }
            else
            {
                eStatus = eSSCsmRet_AlgorithmNotSupported;
            }
        }
    }
    
    return eStatus;
}

SSCsmRet_Enum SSCsm_Decrypt(uint8_t keyHandle, const uint8_t *pCiphertext, uint32_t ciphertextLength, uint8_t *pPlaintext, uint32_t *pPlaintextLength, const uint8_t *pIv, uint8_t ivLength)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    SSCsmKeyInfo_Struct *pKey = &g_stSSCsmCtx.keys[keyHandle];
    SSCsmContext_Struct stContext;
    
    /* 验证参数 */
    if (pCiphertext == NULL || pPlaintext == NULL || pPlaintextLength == NULL)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    else if (!g_stSSCsmCtx.initialized)
    {
        eStatus = eSSCsmRet_NotInitialized;
    }
    else
    {
        /* 检查密钥是否存在 */
        eStatus = SSCsm_IsKeyValid(keyHandle);
        if (eStatus == eSSCsmRet_Ok)
        {
            /* 构建上下文 */
            stContext.eAlgorithm = pKey->eAlgorithm;
            stContext.keyHandle = keyHandle;
            stContext.eMode = eSSCsmMode_Decrypt;
            stContext.ePaddingMode = pKey->ePaddingMode;
            stContext.iv = (uint8_t *)pIv;
            stContext.ivLength = ivLength;
            
            /* 支持AES CBC和RSA模式 */
            if (pKey->eAlgorithm == eSSCsmAlg_AesCbc)
            {
                eStatus = SSCsm_AesCbcDecrypt(&stContext, pCiphertext, ciphertextLength, pPlaintext, pPlaintextLength);
            }
            else if (pKey->eAlgorithm == eSSCsmAlg_Rsa)
            {
                eStatus = SSCsm_RsaDecrypt(&stContext, pCiphertext, ciphertextLength, pPlaintext, pPlaintextLength);
            }
            else
            {
                eStatus = eSSCsmRet_AlgorithmNotSupported;
            }
        }
    }
    
    return eStatus;
}



SSCsmRet_Enum SSCsm_ImportKey(const SSCsmKeyImportParams_Struct *pParams, uint8_t *pKeyHandle)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    uint8_t keyIndex;
    uint8_t keyFound = FALSE;
    
    /* 验证参数 */
    if (pParams == NULL || pParams->pKeyData == NULL || pKeyHandle == NULL)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    else if (!g_stSSCsmCtx.initialized)
    {
        eStatus = eSSCsmRet_NotInitialized;
    }
    else
    {
        /* 检查算法支持 */
        eStatus = SSCsm_IsAlgorithmSupported(pParams->eAlgorithm);
        if (eStatus == eSSCsmRet_Ok)
        {
            /* 检查密钥数据长度 */
            if (pParams->keyDataLength > SSCSM_CFG_MAX_BUFFER_SIZE)
            {
                eStatus = eSSCsmRet_BufferTooSmall;
            }
            /* 检查IV长度 */
            else if (pParams->pIv != NULL && pParams->ivLength > 16)
            {
                eStatus = eSSCsmRet_InvalidParam;
            }
            else
            {
                /* 寻找可用的密钥槽 */
                for (keyIndex = 0; keyIndex < SSCSM_CFG_MAX_KEYS; keyIndex++)
                {
                    if (!g_stSSCsmCtx.keys[keyIndex].used)
                    {
                        /* 标记为使用 */
                        g_stSSCsmCtx.keys[keyIndex].used = TRUE;
                        g_stSSCsmCtx.keys[keyIndex].eKeyType = pParams->eKeyType;
                        g_stSSCsmCtx.keys[keyIndex].eAlgorithm = pParams->eAlgorithm;
                        g_stSSCsmCtx.keys[keyIndex].ePaddingMode = pParams->ePaddingMode;
                        g_stSSCsmCtx.keys[keyIndex].keyBitLength = pParams->keyDataLength * 8; /* 转换为位 */
                        g_stSSCsmCtx.keys[keyIndex].keyDataLength = pParams->keyDataLength;
                        
                        /* 复制密钥数据 */
                        memcpy(g_stSSCsmCtx.keys[keyIndex].keyData, pParams->pKeyData, pParams->keyDataLength);
                        
                        /* 复制IV数据 */
                        if (pParams->pIv != NULL && pParams->ivLength > 0)
                        {
                            memcpy(g_stSSCsmCtx.keys[keyIndex].iv, pParams->pIv, pParams->ivLength);
                            g_stSSCsmCtx.keys[keyIndex].ivLength = pParams->ivLength;
                        }
                        else
                        {
                            memset(g_stSSCsmCtx.keys[keyIndex].iv, 0, 16);
                            g_stSSCsmCtx.keys[keyIndex].ivLength = 0;
                        }
                        
                        /* 更新密钥数量 */
                        g_stSSCsmCtx.keyCount++;
                        
                        /* 返回密钥句柄 */
                        *pKeyHandle = (uint8_t)keyIndex;
                        keyFound = TRUE;
                        break;
                    }
                }
                
                if (!keyFound)
                {
                    eStatus = eSSCsmRet_OperationFailed;
                }
            }
        }
    }
    
    return eStatus;
}

SSCsmRet_Enum SSCsm_GenerateAndEncryptKey(uint8_t keyHandle, uint8_t *pEncryptedKey, uint32_t *pEncryptedKeyLength, uint8_t *pRandomKey, uint32_t *pRandomKeyLength)
{
    SSCsmRet_Enum eStatus = eSSCsmRet_Ok;
    SSCsmKeyInfo_Struct *pKey = &g_stSSCsmCtx.keys[keyHandle];
    uint8_t random_key[16]; // 16字节随机密钥
    uint8_t rsa_encrypted[64]; // RSA-512加密结果（64字节）
    uint8_t base64_encoded[89]; // Base64编码结果（88字节）
    size_t rsa_encrypted_len;
    size_t base64_len;
    int ret;
    
    /* 检查参数 */
    if (pEncryptedKey == NULL || pEncryptedKeyLength == NULL || 
        pRandomKey == NULL || pRandomKeyLength == NULL)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    /* 检查密钥是否有效 */
    else if ((eStatus = SSCsm_IsKeyValid(keyHandle)) != eSSCsmRet_Ok)
    {
        // 密钥无效，eStatus已经设置
    }
    /* 检查算法是否为RSA */
    else if (pKey->eAlgorithm != eSSCsmAlg_Rsa)
    {
        eStatus = eSSCsmRet_AlgorithmNotSupported;
    }
    /* 检查RSA密钥长度是否为512位 */
    else if (pKey->keyBitLength != 512)
    {
        eStatus = eSSCsmRet_InvalidParam;
    }
    else
    {
        /* 生成16字节随机密钥 */
        ret = SSCsm_GenerateRandomKey(random_key, 16);
        if (ret != 0)
        {
            eStatus = eSSCsmRet_OperationFailed;
        }
        else
        {
            /* 构建RSA加密上下文 */
            SSCsmContext_Struct stContext;
            stContext.eAlgorithm = eSSCsmAlg_Rsa;
            stContext.keyHandle = keyHandle;
            stContext.eMode = eSSCsmMode_Encrypt;
            stContext.ePaddingMode = eSSCsmPadding_Pkcs7;
            stContext.iv = NULL;
            stContext.ivLength = 0;
            
            /* 使用RSA公钥加密随机密钥 */
            rsa_encrypted_len = sizeof(rsa_encrypted);
            eStatus = SSCsm_RsaEncrypt(&stContext, random_key, 16, rsa_encrypted, &rsa_encrypted_len);
            if (eStatus == eSSCsmRet_Ok)
            {
                /* 对加密结果进行Base64编码 */
                ret = mbedtls_base64_encode(base64_encoded, sizeof(base64_encoded), &base64_len, 
                                           rsa_encrypted, rsa_encrypted_len);
                if (ret != 0)
                {
                    eStatus = eSSCsmRet_OperationFailed;
                }
                else
                {
                    /* 检查输出缓冲区大小 */
                    if (*pEncryptedKeyLength < base64_len)
                    {
                        *pEncryptedKeyLength = base64_len;
                        eStatus = eSSCsmRet_BufferTooSmall;
                    }
                    else
                    {
                        /* 复制结果 */
                        memcpy(pEncryptedKey, base64_encoded, base64_len);
                        *pEncryptedKeyLength = (uint32_t)base64_len;
                        
                        /* 复制随机密钥 */
                        if (*pRandomKeyLength < 16)
                        {
                            *pRandomKeyLength = 16;
                            eStatus = eSSCsmRet_BufferTooSmall;
                        }
                        else
                        {
                            memcpy(pRandomKey, random_key, 16);
                            *pRandomKeyLength = 16;
                        }
                    }
                }
            }
        }
    }
    
    return eStatus;
}


void SSCsm_MainFunction(void)
{
    /* 检查模块是否初始化 */
    if (!g_stSSCsmCtx.initialized)
    {
        return;
    }
  
    /* TODO: 实现定期维护任务，如密钥轮换、状态检查等 */
}