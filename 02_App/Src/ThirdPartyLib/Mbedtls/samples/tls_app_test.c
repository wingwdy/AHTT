/*
 * File      : tls_app_test.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date          Author          Notes
 * 2018-01-22    chenyong     first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef TEST
#include <mbedtls/aes.h>

#define CRYPTO_RC4_KEY_SIZE                    16
#define CRYPTO_AES_KEY_128BITS                 128
#define CRYPTO_AES_KEY_256BITS                 256
#define CRYPTO_AES_PADDING_NOPADDING           0
#define CRYPTO_AES_PADDING_ZEROPADDING         1
#define CRYPTO_AES_PADDING_PKCS5PADDING        2

static int __aes_cbc_encrypt(uint8_t *data, int len,
                             const uint8_t *key, const uint8_t *iv,
                             int key_size, int pad_type)
{
	int i, length;
	uint8_t ivcpy[16];
	uint8_t pad = 0;
	mbedtls_aes_context ctx;

	length = (len + 0x0f) & (~0x0f);

	if (pad_type == CRYPTO_AES_PADDING_NOPADDING && (len & 0x0f)) {
		return -1;
	}

	if ((len & 0x0f) == 0 && pad_type == CRYPTO_AES_PADDING_PKCS5PADDING) {
		length += 16;
	}

	if (pad_type == CRYPTO_AES_PADDING_PKCS5PADDING) {
		pad = length - len;
	}

	for (i = 0; i < length - len; i++) {
		data[len + i] = pad;
	}

	len = length;

	memcpy(ivcpy, iv, 16);

	mbedtls_aes_init(&ctx);
	mbedtls_aes_setkey_enc(&ctx, key, key_size);
	while (length > 0) {
		for (i = 0; i < 16; i++) {
			data[i] ^= ivcpy[i];
		}
		mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, data, data);
		memcpy(ivcpy, data, 16);
		data += 16;
		length -= 16;
	}
	mbedtls_aes_free(&ctx);

	return len;
}

static int __aes_cbc_decrypt(uint8_t *data, int len,
                             const uint8_t *key, const uint8_t *iv,
                             int key_size, int pad_type)
{
	int i, length;
	uint8_t ivcpy[16];
	uint8_t *ptr = data;
	uint8_t pad;
	mbedtls_aes_context ctx;

	if (len & 0xf) {
		return -1;
	}

	length = len;

	memcpy(ivcpy, iv, 16);

	mbedtls_aes_init(&ctx);
	mbedtls_aes_setkey_dec(&ctx, key, key_size);

	while (length > 0) {
		uint8_t tmp[16];

		memcpy(tmp, data, 16);
		mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, data, data);
		for (i = 0; i < 16; i++) {
			data[i] ^= ivcpy[i];
		}
		memcpy(ivcpy, tmp, 16);
		data += 16;
		length -= 16;
	}

	mbedtls_aes_free(&ctx);

	if (pad_type == CRYPTO_AES_PADDING_NOPADDING || \
	        pad_type == CRYPTO_AES_PADDING_ZEROPADDING) {
		return len;
	}

	pad = ptr[len - 1];
	if (pad < 1 || pad > 16) {
		return -1;
	}

	for (i = 1; i <= pad; i++) {
		if (ptr[len - i] != pad) {
			return -1;
		}
		ptr[len - i] = 0;
	}

	return len - pad;
}

int crypto_aes_cbc_encrypt(const uint8_t *input, int len,
                           uint8_t *output, const uint8_t *key, const uint8_t *iv,
                           int key_size, int pad_type)
{
	if (key_size != CRYPTO_AES_KEY_128BITS && \
	        key_size != CRYPTO_AES_KEY_256BITS) {
		return -1;
	}

	if (pad_type != CRYPTO_AES_PADDING_NOPADDING && \
	        pad_type != CRYPTO_AES_PADDING_ZEROPADDING && \
	        pad_type != CRYPTO_AES_PADDING_PKCS5PADDING) {
		return -1;
	}

	memcpy(output, input, len);

	return __aes_cbc_encrypt(output, len, key, iv, key_size, pad_type);
}

int crypto_aes_cbc_decrypt(const uint8_t *input, int len,
                           uint8_t *output, const uint8_t *key, const uint8_t *iv,
                           int key_size, int pad_type)
{
	if (key_size != CRYPTO_AES_KEY_128BITS && \
	        key_size != CRYPTO_AES_KEY_256BITS) {
		return -1;
	}

	if (pad_type != CRYPTO_AES_PADDING_NOPADDING && \
	        pad_type != CRYPTO_AES_PADDING_ZEROPADDING && \
	        pad_type != CRYPTO_AES_PADDING_PKCS5PADDING) {
		return -1;
	}

	memcpy(output, input, len);

	return __aes_cbc_decrypt(output, len, key, iv, key_size, pad_type);
}

int crypto_aes_ecb_encrypt(const uint8_t *input, int len,
                           uint8_t *output, const uint8_t *key, int keybits)
{
	int i, cnt;
	mbedtls_aes_context ctx;

	if (len % 16) {
		return -1;
	}

	cnt = len / 16;

	if (keybits != CRYPTO_AES_KEY_128BITS && \
	        keybits != CRYPTO_AES_KEY_256BITS) {
		return -1;
	}

	mbedtls_aes_init(&ctx);
	mbedtls_aes_setkey_enc(&ctx, key, keybits);
	for (i = 0; i < cnt; i++)
		mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, \
		                      input + (i << 4), output + (i << 4));

	return len;
}

int crypto_aes_ecb_decrypt(const uint8_t *input, int len,
                           uint8_t *output, const uint8_t *key, int keybits)
{
	int i, cnt;
	mbedtls_aes_context ctx;

	if (len % 16) {
		return -1;
	}

	cnt = len / 16;

	if (keybits != CRYPTO_AES_KEY_128BITS && \
	        keybits != CRYPTO_AES_KEY_256BITS) {
		return -1;
	}

	mbedtls_aes_init(&ctx);
	mbedtls_aes_setkey_dec(&ctx, key, keybits);
	for (i = 0; i < cnt; i++)
		mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, \
		                      input + (i << 4), output + (i << 4));

	return len;
}

#include "bsp.h"

int mbedtls_aes_test(void)
{
	debug("AES-CBC\n");
	int i;
	mbedtls_aes_context aes_ctx;
	//密钥数值
	unsigned char key[16] = {'c', 'b', 'c', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3', '4'};
	//iv
	unsigned char iv[16];
	//明文空间
	unsigned char plain[64] = "hello_world1234";
	//解密后明文的空间
	unsigned char dec_plain[64] = {0};
	//密文空间
	unsigned char cipher[64] = {0};

	mbedtls_aes_init(&aes_ctx);
	mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
	for (i = 0; i < 16; i++) {
		iv[i] = 0x01;
	}
	debug("orig data: %s\n", plain);
	debug("key: %s\n", key);
	mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, 64, iv, plain, cipher);
	debug("result: ");
	for (int loop = 0; loop < 64; loop++) {
		debug("%02x", cipher[loop]);
	}
	debug("\n");

	//设置解密密钥
	mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
	for (i = 0; i < 16; i++) {
		iv[i] = 0x01;
	}
	mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, 64, iv, cipher, dec_plain);
	debug("decrypt: %s\n", dec_plain);
	mbedtls_aes_free(&aes_ctx);
	return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(mbedtls_aes_test, aes_test, mbedtls aes test);

#endif
