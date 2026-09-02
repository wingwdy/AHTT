#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// AES 的 S 盒
static const uint8_t s_box[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

// 逆向 S 盒
static const uint8_t inv_s_box[256] = {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};


// 轮常数
static const uint8_t rcon[11] = {
    0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

// 列混淆的固定矩阵
static const uint8_t mix_columns_matrix[4][4] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
};

// 逆向列混淆的固定矩阵
static const uint8_t inv_mix_columns_matrix[4][4] = {
    {0x0E, 0x0B, 0x0D, 0x09},
    {0x09, 0x0E, 0x0B, 0x0D},
    {0x0D, 0x09, 0x0E, 0x0B},
    {0x0B, 0x0D, 0x09, 0x0E}
};

// 密钥扩展
void key_expansion(const uint8_t *key, uint8_t *round_keys) {
    uint8_t temp[4];
    memcpy(round_keys, key, 16);

    for (int i = 4; i < 44; i++) {
        memcpy(temp, round_keys + (i - 1) * 4, 4);

        if (i % 4 == 0) {
            // 字节循环
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;

            // 字节替换
            for (int j = 0; j < 4; j++) {
                temp[j] = s_box[temp[j]];
            }

            // 轮常数异或
            temp[0] ^= rcon[i / 4];
        }

        for (int j = 0; j < 4; j++) {
            round_keys[i * 4 + j] = round_keys[(i - 4) * 4 + j] ^ temp[j];
        }
    }
}

// 字节替换
void sub_bytes(uint8_t state[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = s_box[state[i][j]];
        }
    }
}

// 逆向字节替换
void inv_sub_bytes(uint8_t state[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = inv_s_box[state[i][j]];
        }
    }
}

// 行移位
void shift_rows(uint8_t state[4][4]) {
    uint8_t temp;

    // 第 1 行左移 1 字节
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    // 第 2 行左移 2 字节
    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    // 第 3 行左移 3 字节
    temp = state[3][0];
    state[3][0] = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = temp;
}

// 逆向行移位
void inv_shift_rows(uint8_t state[4][4]) {
    uint8_t temp;

    // 第 1 行右移 1 字节
    temp = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = temp;

    // 第 2 行右移 2 字节
    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    // 第 3 行右移 3 字节
    temp = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = temp;
}
// 有限域上的乘法
uint8_t gf_mul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        uint8_t carry = a & 0x80;
        a <<= 1;
        if (carry) a ^= 0x1B; // 0x1B 是 AES 的不可约多项式 x^8 + x^4 + x^3 + x + 1
        b >>= 1;
    }
    return p;
}

// 列混淆
void mix_columns(uint8_t state[4][4]) {
    uint8_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[0] = state[0][i];
        temp[1] = state[1][i];
        temp[2] = state[2][i];
        temp[3] = state[3][i];

        state[0][i] = gf_mul(0x02, temp[0]) ^ gf_mul(0x03, temp[1]) ^ temp[2] ^ temp[3];
        state[1][i] = temp[0] ^ gf_mul(0x02, temp[1]) ^ gf_mul(0x03, temp[2]) ^ temp[3];
        state[2][i] = temp[0] ^ temp[1] ^ gf_mul(0x02, temp[2]) ^ gf_mul(0x03, temp[3]);
        state[3][i] = gf_mul(0x03, temp[0]) ^ temp[1] ^ temp[2] ^ gf_mul(0x02, temp[3]);
    }
}

// 逆向列混淆
void inv_mix_columns(uint8_t state[4][4]) {
    uint8_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[0] = state[0][i];
        temp[1] = state[1][i];
        temp[2] = state[2][i];
        temp[3] = state[3][i];

        state[0][i] = gf_mul(0x0E, temp[0]) ^ gf_mul(0x0B, temp[1]) ^ gf_mul(0x0D, temp[2]) ^ gf_mul(0x09, temp[3]);
        state[1][i] = gf_mul(0x09, temp[0]) ^ gf_mul(0x0E, temp[1]) ^ gf_mul(0x0B, temp[2]) ^ gf_mul(0x0D, temp[3]);
        state[2][i] = gf_mul(0x0D, temp[0]) ^ gf_mul(0x09, temp[1]) ^ gf_mul(0x0E, temp[2]) ^ gf_mul(0x0B, temp[3]);
        state[3][i] = gf_mul(0x0B, temp[0]) ^ gf_mul(0x0D, temp[1]) ^ gf_mul(0x09, temp[2]) ^ gf_mul(0x0E, temp[3]);
    }
}

// 轮密钥加
void add_round_key(uint8_t state[4][4], const uint8_t *round_key) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] ^= round_key[i * 4 + j];
        }
    }
}

// AES 加密
void aes_encrypt(const uint8_t *input, const uint8_t *round_keys, uint8_t *output) {
    uint8_t state[4][4];

    // 将输入数据加载到状态矩阵中
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[j][i] = input[i * 4 + j];
        }
    }

    // 初始轮密钥加
    add_round_key(state, round_keys);

    // 9 轮完整加密
    for (int round = 1; round < 10; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, round_keys + round * 16);
    }

    // 最后一轮
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, round_keys + 160);

    // 将状态矩阵写回输出
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            output[i * 4 + j] = state[j][i];
        }
    }
}

// AES 解密
void aes_decrypt(const uint8_t *input, const uint8_t *round_keys, uint8_t *output) {
    uint8_t state[4][4];

    // 将输入数据加载到状态矩阵中
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[j][i] = input[i * 4 + j];
        }
    }

    // 初始轮密钥加
    add_round_key(state, round_keys + 160);

    // 9 轮完整解密
    for (int round = 9; round > 0; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, round_keys + round * 16);
        inv_mix_columns(state);
    }

    // 最后一轮
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, round_keys);

    // 将状态矩阵写回输出
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            output[i * 4 + j] = state[j][i];
        }
    }
}

// PKCS5 填充
void pkcs5_padding(uint8_t *data, size_t *len) {
    size_t block_size = 16;
    size_t padding_len = block_size - (*len % block_size);
    memset(data + *len, padding_len, padding_len);
    *len += padding_len;
}

// PKCS5 去除填充
void pkcs5_unpadding(uint8_t *data, size_t *len) {
    size_t padding_len = data[*len - 1];
    *len -= padding_len;
}

// AES-CBC 加密
void aes_cbc_encrypt(const uint8_t *input, size_t len, const uint8_t *key, uint8_t *output) {
    uint8_t round_keys[176];
    key_expansion(key, round_keys);

    uint8_t iv[16];
    memcpy(iv, key, 16); // 初始向量与密钥一致

    size_t padded_len = len;
    uint8_t *padded_input = (uint8_t *)malloc(padded_len * 4);
    if (padded_input == NULL) {
        printf("faile\r\n");
        return;
    }
    
    // uint8_t padded_input[32] = {0};
    memcpy(padded_input, input, len);

    pkcs5_padding(padded_input, &padded_len);

    for (size_t i = 0; i < padded_len; i += 16) {
        for (int j = 0; j < 16; j++) {
            padded_input[i + j] ^= iv[j];
        }
        aes_encrypt(padded_input + i, round_keys, output + i);
        memcpy(iv, output + i, 16);
    }

    // free(padded_input);
}

// AES-CBC 解密
void aes_cbc_decrypt(const uint8_t *input, size_t len, const uint8_t *key, uint8_t *output) {
    uint8_t round_keys[176];
    key_expansion(key, round_keys);

    uint8_t iv[16];
    memcpy(iv, key, 16); // 初始向量与密钥一致

    size_t padded_len = len; // 使用填充后的长度
    for (size_t i = 0; i < len; i += 16) {
        aes_decrypt(input + i, round_keys, output + i);
        for (int j = 0; j < 16; j++) {
            output[i + j] ^= iv[j];
        }
        memcpy(iv, input + i, 16);
    }

    pkcs5_unpadding(output, &padded_len);
}

int main() {
    // 密钥和初始向量
    uint8_t key[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    // uint8_t key[8] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6};

    // 明文
    uint8_t plaintext[] = "hello";
    size_t plaintext_len = strlen((char *)plaintext);
    printf("plaintext_len: %d\n", plaintext_len);

    // 加密
    uint8_t ciphertext[128] = {0};
    aes_cbc_encrypt(plaintext, plaintext_len, key, ciphertext);

    // 解密
    uint8_t decrypted[128] = {0};
    size_t decrypted_len = plaintext_len; // 使用填充后的长度
    aes_cbc_decrypt(ciphertext, plaintext_len + (16 - plaintext_len % 16), key, decrypted);

    // 输出原始明文
    printf("Plaintext: %s\n", plaintext);

    // 输出加密密文
    printf("ciphertext: %d \n", plaintext_len);
    for (size_t i = 0; i < plaintext_len + (16 - plaintext_len % 16); i++) {
        printf("%02X", ciphertext[i]);
    }
    printf("\n");

    //输出解密后的明文
    printf("Decrypted: %.*s\n", (int)decrypted_len, decrypted); // 使用正确的解密长度
    
    for (size_t i = 0; i < 32; i++) {
        printf("%02X", decrypted[i]);
    }
    printf("\n");
    // printf("Decrypted: %s\n", decrypted);

    return 0;
}
