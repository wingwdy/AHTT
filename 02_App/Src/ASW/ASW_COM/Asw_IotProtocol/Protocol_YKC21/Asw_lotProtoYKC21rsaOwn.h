
#ifndef __IOTYKC21RSA_H_
#define __IOTYKC21RSA_H_

#include "Asw_IotProtoYKC21M.h"

#define KEY_LEN 16

int encrypt_and_decrypt_data(unsigned char* key,unsigned char* output,unsigned char* random_key);

#endif
