#ifndef __AES128_CBC_H_
#define __AES128_CBC_H_

#include "AppHeaderSummary.h"

void aes_cbc_encrypt(const uint8_t *input, size_t len, const uint8_t *key, uint8_t *output);
void key_expansion(const uint8_t *key, uint8_t *round_keys) ;
int aes_test() ;


#endif