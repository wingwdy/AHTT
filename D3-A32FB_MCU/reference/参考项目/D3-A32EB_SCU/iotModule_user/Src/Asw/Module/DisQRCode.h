#include "stdio.h"
#include "stdint.h"
#include "string.h"

// 主函数：replace_or_append
uint8_t DisQRcodeReplace(char *output, const char *input, const char *pileNum, uint8_t gun);

void SpecialCharQrcodeStringConfig(char *output, char *input);
void NormalCharQrcodeStringConfig(char *output, char *input);