
#ifndef __SHA256_H_
#define __SHA256_H_

#include "AppHeaderSummary.h"

typedef struct {
    unsigned char	hash[32];	// Changed by RKW, unsigned char becomes uint8_t
    unsigned int	buffer[16];	// Changed by RKW, unsigned long becomes uint32_t
    unsigned int	state[8];	// Changed by RKW, unsinged long becomes uint32_t
    unsigned char		length[8];	// Changed by RKW, unsigned char becomes uint8_t
} sha256;
	
void sha256_get(unsigned char hash[32],
                const unsigned char *message,
                int length);

#endif /* __SHA256_H_ */
