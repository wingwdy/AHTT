

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "mbedtls/entropy_poll.h"
#include "mbedtls/rsa.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"
#include "mbedtls/base64.h"

#include "rsaOwn.h"
#include "common.h"
//添加熵源
int get_clock_for_entropy( void *data,unsigned char *output, size_t len, size_t *olen ) {
    time_t now_time;
    // time(&now_time);
    unsigned long timer = now_time;
    ((void) data);
    *olen = 0;

    if (len < sizeof(unsigned long))
        return (0);

    memcpy(output, &timer, sizeof(unsigned long));
    *olen = sizeof(unsigned long);
    return 0;
}

void print_mpi(const mbedtls_mpi *mpi) {
    unsigned char buf[1024];
    size_t len;

    // 获取 MPI 的实际大小（以字节为单位）
    len = mbedtls_mpi_size(mpi);

    // 将 MPI 转换为二进制格式
    mbedtls_mpi_write_binary(mpi, buf, len);

    // 打印二进制数据
    for (size_t i = 0; i < len; i++) {
        printf("%02X", buf[i]);
    }
    printf("\n");
}

int encrypt_and_decrypt_data(unsigned char* rsa_key,unsigned char* output,unsigned char* random_key){
    //个性化初始值：用于初始化伪随机数生成器，可设置为任意值
    const char *personalization = "Fr789jj-ikrkjfjs@";
    // unsigned char base64_der_pub_key;
    unsigned char base64_der_pub_key[128] ;


    memcpy(base64_der_pub_key, rsa_key, RSA_KEY_LEN);
    
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
                                (unsigned char *)base64_der_pub_key, RSA_KEY_LEN);
    if (ret != 0) {
        printf("Failed to decode Base64: -0x%04X\n", -ret);
        goto exit;
    }

    // 解析 DER 编码的公钥
    ret = mbedtls_pk_parse_public_key(&pk_ctx, der_pub_key, der_len);
    if (ret != 0) {
        printf("Failed to parse public key: -0x%04X\n", -ret);
        goto exit;
    }

    //添加熵源,若在嵌入式平台，需添加熵源
    mbedtls_entropy_add_source(&entropy,get_clock_for_entropy,NULL,MBEDTLS_ENTROPY_MIN_PLATFORM,MBEDTLS_ENTROPY_SOURCE_STRONG);
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)personalization,strlen(personalization));

    if (ret != 0) {
        printf("Failed to seed the random generator: -0x%04X\n", -ret);
        goto exit;
    }


     size_t olen = 0;
    // 生成随机密钥A
    ret = mbedtls_ctr_drbg_random(&ctr_drbg, random_key, KEY_LEN);
    if (ret != 0) {
        printf("Failed to generate the random key: -0x%04X\n", -ret);
        goto exit;
    }
    hex_dump("----------key----------", random_key, KEY_LEN);

     // 使用公钥加密数据-随机
     ret = mbedtls_pk_encrypt(&pk_ctx, random_key, KEY_LEN, encrypt_output, 
                                &olen, sizeof(encrypt_output), 
                                mbedtls_ctr_drbg_random, &ctr_drbg);
     if (ret != 0) {
         printf("Encryption failed: -0x%04X\n", -ret);
         goto exit;
     }
 

    //Base64编码
     unsigned char base64_data[89]; // 512位RSA加密后64字节，Base64编码后长度固定为88字节
     size_t base64_len = 0;
     ret = mbedtls_base64_encode(base64_data, sizeof(base64_data), &base64_len,
                                 encrypt_output, olen);
     if (ret != 0) {
         printf("Base64 encoding fails: -0x%04X\n", -ret);
         goto exit;
     }
     // 添加字符串终止符
     if (base64_len >= sizeof(base64_data)) {
         printf("Error :Base64 buffer overflow\n");
         goto exit;
     }
    // base64_data[base64_len] = '\0'; 

    memcpy(output, base64_data, base64_len);

    //Base64编码
     unsigned char base64_output[89]; // 512位RSA加密后64字节，Base64编码后长度固定为88字节
     ret = mbedtls_base64_encode(base64_output, sizeof(base64_output), &base64_len,
                                 encrypt_output, olen);
     if (ret != 0) {
         printf("Base64 encoding fails: -0x%04X\n", -ret);
         goto exit;
     }
     // 添加字符串终止符
     if (base64_len >= sizeof(base64_output)) {
         printf("Error :Base64 buffer overflow\n");
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




