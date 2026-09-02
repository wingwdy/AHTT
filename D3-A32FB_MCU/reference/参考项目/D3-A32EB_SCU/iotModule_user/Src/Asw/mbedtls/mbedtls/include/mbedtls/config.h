#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

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
#define MBEDTLS_SHA256_C


//读pem证书
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_FS_IO

//填充方式，V15或V21两种模式必须二选一
//#define MBEDTLS_PKCS1_V21
#define MBEDTLS_PKCS1_V15

//不使用平台默认熵源，mbedtls在windows和linux下已实现熵源
#define MBEDTLS_NO_PLATFORM_ENTROPY
//#include "check_config.h"

#endif
