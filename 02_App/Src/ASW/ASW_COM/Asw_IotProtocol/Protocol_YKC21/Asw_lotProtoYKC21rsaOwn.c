

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "mbedtls/entropy_poll.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"
#include "mbedtls/base64.h"

#include "Asw_lotProtoYKC21rsaOwn.h"
#include "common.h"

// 添加熵源
int get_clock_for_entropy(void *data, unsigned char *output, size_t len, size_t *olen)
{
    time_t now_time;
    // time(&now_time);
    unsigned long timer = now_time;
    ((void)data);
    *olen = 0;

    if (len < sizeof(unsigned long))
        return (0);

    memcpy(output, &timer, sizeof(unsigned long));
    *olen = sizeof(unsigned long);
    return 0;
}

int encrypt_and_decrypt_data(unsigned char *rsa_key, unsigned char *output, unsigned char *random_key)
{
    // 个性化初始值：用于初始化伪随机数生成器，可设置为任意值
    const char *personalization = "Fr789jj-ikrkjfjs@";
    // unsigned char base64_der_pub_key;
    unsigned char base64_der_pub_key[128];

    memcpy(base64_der_pub_key, rsa_key, IOTYKC21_CFG_RSA_KEY_LEN);

    unsigned char encrypt_output[128]; // 根据实际情况调整大小

    int ret;
    mbedtls_pk_context pk_ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char der_pub_key[256]; // 确保足够大以容纳解码后的数据
    size_t der_len = 0;

    // // 初始化公钥上下文
    // mbedtls_pk_init(&pk_ctx);
    // 初始化公钥上下文、熵源和CTR_DRBG
    mbedtls_pk_init(&pk_ctx);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // 解码 Base64 数据
    ret = mbedtls_base64_decode(der_pub_key, sizeof(der_pub_key), &der_len,
                                (unsigned char *)base64_der_pub_key, IOTYKC21_CFG_RSA_KEY_LEN);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Failed to decode Base64: -0x%04X\n", -ret);
        goto exit;
    }

    // 解析 DER 编码的公钥
    ret = mbedtls_pk_parse_public_key(&pk_ctx, der_pub_key, der_len);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Failed to parse public key: -0x%04X\n", -ret);
        goto exit;
    }

    // 添加熵源,若在嵌入式平台，需添加熵源
    mbedtls_entropy_add_source(&entropy, get_clock_for_entropy, NULL, MBEDTLS_ENTROPY_MIN_PLATFORM, MBEDTLS_ENTROPY_SOURCE_STRONG);
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)personalization, strlen(personalization));

    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Failed to seed the random generator: -0x%04X\n", -ret);
        goto exit;
    }

    size_t olen = 0;
    // 生成随机密钥A
    ret = mbedtls_ctr_drbg_random(&ctr_drbg, random_key, KEY_LEN);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Failed to generate the random key: -0x%04X\n", -ret);
        goto exit;
    }

    // 使用公钥加密数据-随机

    ret = mbedtls_pk_encrypt(&pk_ctx, random_key, KEY_LEN, encrypt_output,
                             &olen, sizeof(encrypt_output),
                             mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Encryption failed: -0x%04X\n", -ret);
        goto exit;
    }

    // Base64编码
    unsigned char base64_data[89]; // 512位RSA加密后64字节，Base64编码后长度固定为88字节
    size_t base64_len = 0;
    ret = mbedtls_base64_encode(base64_data, sizeof(base64_data), &base64_len,
                                encrypt_output, olen);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Base64 encoding fails: -0x%04X\n", -ret);
        goto exit;
    }
    // 添加字符串终止符
    if (base64_len >= sizeof(base64_data))
    {
        IOTYKC21_CFG_LogPrint("Error :Base64 buffer overflow\n");
        goto exit;
    }
    // base64_data[base64_len] = '\0';

    memcpy(output, base64_data, base64_len);

    // Base64编码
    unsigned char base64_output[89]; // 512位RSA加密后64字节，Base64编码后长度固定为88字节
    ret = mbedtls_base64_encode(base64_output, sizeof(base64_output), &base64_len,
                                encrypt_output, olen);
    if (ret != 0)
    {
        IOTYKC21_CFG_LogPrint("Base64 encoding fails: -0x%04X\n", -ret);
        goto exit;
    }
    // 添加字符串终止符
    if (base64_len >= sizeof(base64_output))
    {
        IOTYKC21_CFG_LogPrint("Error :Base64 buffer overflow\n");
        goto exit;
    }

    memcpy(output, base64_output, base64_len);

exit:
    // 清理公钥上下文
    mbedtls_pk_free(&pk_ctx);

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}


 
 
// 配置参数
#define RSA_KEY_BITS 512                   // RSA密钥长度
#define SYM_KEY_LEN 16                     // 对称密钥长度（16字节）
#define ENCRYPTED_LEN 88                   // 最终加密后数据长度（88字节）
#define RSA_ENCRYPT_LEN (RSA_KEY_BITS / 8) // RSA原始加密结果长度（64字节）
#define RSA_KEY_BYTES 128                  // 已知RSA密钥总长度（128字节）

// 88字节封装格式（同前序，保证总长度）
typedef struct
{
    uint32_t magic;                                       // 魔数（4字节，0x52534145）
    uint16_t sym_key_len;                                 // 对称密钥长度（2字节）
    uint16_t rsa_encrypt_len;                             // RSA加密结果长度（2字节）
    uint8_t rsa_data[RSA_ENCRYPT_LEN];                    // RSA加密数据（64字节）
    uint8_t padding[ENCRYPTED_LEN - 8 - RSA_ENCRYPT_LEN]; // 填充（16字节）
} EncryptedKeyPackage;

// 字节序转换（主机序→网络序）
static uint16_t htons(uint16_t val)
{
    return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
}

static uint32_t htonl(uint32_t val)
{
    return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) |
           ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

// 从128字节原始数据加载RSA-512公钥（假设格式：前64字节=模数n，后64字节=指数e（仅低2字节有效））
// 注：若你的128字节密钥格式不同，需调整此函数的解析逻辑
int load_rsa_public_key(mbedtls_rsa_context *rsa, const uint8_t *rsa_key_128b)
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
    // 若你的e是完整64字节，直接用64字节即可
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

int encrypt_and_decrypt_data00(unsigned char *rsa_key, unsigned char *output, unsigned char *random_key)
{
    int ret;
    // 初始化mbedtls上下文
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_known_key_encrypt_88byte";

    // ========== 已知密钥（替换为你实际的密钥数据） ==========

    // 2. 已知的16字节对称密钥（替换为你的真实对称密钥）
    uint8_t sym_key[SYM_KEY_LEN] = {
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
        0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};
    // ========== 密钥结束 ==========
   memcpy(random_key, sym_key, 16);
    // 缓冲区定义
    uint8_t rsa_encrypted[RSA_ENCRYPT_LEN] = {0}; // RSA原始加密结果（64字节）
    uint8_t final_encrypted[ENCRYPTED_LEN] = {0}; // 最终88字节结果
    EncryptedKeyPackage *pkg = (EncryptedKeyPackage *)final_encrypted;

    // 初始化mbedtls组件
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // 1. 初始化随机数生成器（RSA加密需要随机填充）
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const uint8_t *)pers, strlen(pers));
    if (ret != 0)
        IOTYKC21_CFG_LogPrint("mbedtls_ctr_drbg_seed: 0x%08X\n", ret);

    // 2. 加载已知的128字节RSA-512公钥
    IOTYKC21_CFG_LogPrint("Loading known 128-byte RSA-%d public key...\n", RSA_KEY_BITS);
    ret = load_rsa_public_key(&rsa, rsa_key);
    if (ret != 0)
        IOTYKC21_CFG_LogPrint("load_rsa_public_key: 0x%08X\n", ret);

    // 3. 用RSA公钥加密16字节对称密钥
    IOTYKC21_CFG_LogPrint("Encrypting known 16-byte symmetric key...\n");
    ret = mbedtls_rsa_pkcs1_encrypt(&rsa, mbedtls_ctr_drbg_random, &ctr_drbg,
                                    MBEDTLS_RSA_PUBLIC, SYM_KEY_LEN, sym_key,
                                    rsa_encrypted);
    if (ret != 0)
        IOTYKC21_CFG_LogPrint("mbedtls_rsa_pkcs1_encrypt: 0x%08X\n", ret);

    // 4. 封装为88字节格式
    IOTYKC21_CFG_LogPrint("Packaging to %d bytes...\n", ENCRYPTED_LEN);
    memset(final_encrypted, 0, ENCRYPTED_LEN);
    pkg->magic = htonl(0x52534145);                        // 魔数RSAE
    pkg->sym_key_len = htons(SYM_KEY_LEN);                 // 对称密钥长度
    pkg->rsa_encrypt_len = htons(RSA_ENCRYPT_LEN);         // RSA加密长度
    memcpy(pkg->rsa_data, rsa_encrypted, RSA_ENCRYPT_LEN); // 拷贝RSA密文
    memset(pkg->padding, 0xFF, sizeof(pkg->padding));      // 填充0xFF

    // 输出验证
    // IOTYKC21_CFG_LogPrint("\n=== Known Symmetric Key (16 bytes) ===\n");
    //  DSLogM_HexOutput((uint8_t *)sym_key, 16);
    

    // IOTYKC21_CFG_LogPrint("\n\n=== RSA Encrypted Data (64 bytes) ===\n");
    //  DSLogM_HexOutput((uint8_t *)rsa_encrypted, RSA_ENCRYPT_LEN);
    
    // IOTYKC21_CFG_LogPrint("\n=== Final Encrypted Data (88 bytes) ===\n");
    //  DSLogM_HexOutput((uint8_t *)final_encrypted, ENCRYPTED_LEN);
     memcpy(output, final_encrypted, 88);

exit:
    // 清理资源
    mbedtls_rsa_free(&rsa);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret == 0 ? 0 : -1;
}