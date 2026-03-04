#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include "myMalloc.h"

#define MBEDTLS_ERROR_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C
#define MBEDTLS_RSA_C
#define MBEDTLS_AES_C
#define MBEDTLS_MD_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_GENPRIME
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_PK_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_STD_CALLOC    myCalloc
#define MBEDTLS_PLATFORM_STD_FREE      myFree

/* AES优化配置 */ 
 #define MBEDTLS_AES_FEWER_TABLES
 #define MBEDTLS_AES_ROM_TABLES

/* 只启用CBC模式 */
#define MBEDTLS_CIPHER_MODE_CBC

/* 启用填充模式 */
#define MBEDTLS_CIPHER_PADDING_PKCS7
#define MBEDTLS_CIPHER_PADDING_ZEROS

/* 读pem证书 */
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_FS_IO

/* 填充方式，V15或V21两种模式必须二选一 */
#define MBEDTLS_PKCS1_V21
#define MBEDTLS_PKCS1_V15
/* 不使用平台默认熵源，mbedtls在windows和linux下已实现熵源 */
#define MBEDTLS_NO_PLATFORM_ENTROPY
#endif
