#include "DisQRCode.h"

/*********************************************************************************************** 
配置二维码链接，特殊字符[pile][gun:xx]
桩号替换字符串[pile], 枪号按照[gun:%d]中的格式去替换
没有特殊字符自动按照默认添加的，比如尾部跟随9500002003000101
格式不做固定是因为有些平台会有枪号一位或者两位的情况；
[gun:xx]：xx为%02d时，枪号跟随01
[gun:xx]：xx为%d时，枪号跟随1

实际使用中，还有可能枪号为00，比如电瓶车，[gun:00]可以直接替换00
受串口配置缓存大小限制，最大只能输入128字节字符串，再多屏幕也无法正常显示

关于平台配置：
平台下发枪号为0表示只发限定字符串，需要补充桩号
平台下发枪号为实际枪号，则表示全部以平台为主，不需要补充桩号等信息
********************************************************************************************** */
#define QRCODE_DATA_LEN_MAX    200          //二维码长度

/*********************************************************************************************** 
 * 下面为串口二维码配置函数
********************************************************************************************** */
// 辅助函数：查找子串的位置
const char* find_substring(const char *input, const char *pattern) {
    return strstr(input, pattern);
}

// 辅助函数：计算替换字符串的长度
size_t calculate_replacement_length(const char *input, const char *old_substring, const char *new_substring) {
    size_t len_old = strlen(old_substring);
    size_t len_new = strlen(new_substring);
    size_t len_input = strlen(input);
    size_t count = 0;

    // 计算 old_substring 在 input 中出现的次数
    const char *pos = input;
    while ((pos = find_substring(pos, old_substring)) != NULL) {
        count++;
        pos += len_old;
    }

    // 计算新的字符串长度
    return len_input + (len_new - len_old) * count;
}

void strcatPileGun(char *result,  const char *input, const char *pileNum, uint8_t gun, char *gunFormat)
{
    // 如果没有找到 [pile]，直接在字符串末尾追加 pileNum 和 gun 格式化后的字符串
    size_t len_input = strlen(input);

    size_t len_pileNum = strlen(pileNum);
    char gun_str[8]; // 足够大的缓冲区来存储 gun 的字符串表示
    snprintf(gun_str, sizeof(gun_str), "%02d", gun);
    size_t len_gun_str = strlen(gun_str);

    if (len_input + len_pileNum + len_gun_str >= QRCODE_DATA_LEN_MAX) {
        printf("Result buffer is too small\n");
        return;
    }

    // 构建新字符串
    strcpy(result, input);
    strcat(result, pileNum);
    strcat(result, gun_str);
}

// 按照位置替换字符串
void replace_by_position(char *result, const char *input, size_t start, size_t length, const char *new_substring) {
    size_t len_input = strlen(input);
    size_t len_new = strlen(new_substring);
    size_t len_result = len_input - length + len_new;

    if (len_result >= QRCODE_DATA_LEN_MAX) {
        printf("Result buffer is too small\n");
        return;
    }
    // 拷贝起始部分
    memcpy(result, input, start);

    // 拷贝新字符串
    memcpy(result + start, new_substring, len_new);

    // 拷贝剩余部分
    memcpy(result + start + len_new, input + start + length, len_input - start - length);

    result[len_result] = '\0';
}

// 辅助函数：处理 [pile] 和 [gun:] 的替换逻辑
static uint8_t handle_replacement(char *output, const char *input, const char *pileNum, uint8_t gun) {
    const char *pile_pattern = "[pile]";
    const char *gun_pattern = "[gun:";
    char temp_result[QRCODE_DATA_LEN_MAX] = {0};

    const char *pile_pos = find_substring(input, pile_pattern);
    const char *gun_pos = find_substring(input, gun_pattern);
    if ((pile_pos == NULL) && (gun_pos == NULL)) {
        // 如果没有找到 [pile] 和 [gun:]，直接在字符串末尾追加 pileNum 和 gun 格式化后的默认字符串
        // strcatPileGun(output, input, pileNum, gun, "%02d");
        strcpy(output, input);
        return 1;
    }

    if (pile_pos) {
        // 替换 [pile]
        size_t pile_len = strlen(pile_pattern);
        replace_by_position(temp_result, input, pile_pos - input, pile_len, pileNum);
    } else {
        // 如果没有找到 [pile]，直接复制输入字符串
        strcpy(temp_result, input);
    }

    strcpy(output, temp_result);

    gun_pos = find_substring(temp_result, gun_pattern);
    if (gun_pos) {
        // 找到 [gun:] 后面的格式化字符串
        const char *gun_format_start = gun_pos + strlen(gun_pattern);
        const char *gun_format_end = strchr(gun_format_start, ']');
        if (gun_format_end == NULL) {
            // 如果没有找到 ']'，直接返回输入字符串的副本
            return 1;
        }

        // 提取 [gun:] 后面的格式化字符串
        size_t len_gun_format = gun_format_end - gun_format_start;
        #define lenMax  5
        char gun_format[lenMax + 1];
        memcpy(gun_format, gun_format_start, len_gun_format);
        gun_format[len_gun_format] = '\0';

        // 格式化 gun 字符串
        char gun_str[16]; // 足够大的缓冲区来存储 gun 的字符串表示
        snprintf(gun_str, sizeof(gun_str), gun_format, gun);

        // 计算 [gun:] 及其格式化字符串的总长度
        size_t gun_len = gun_format_end - gun_pos + 1;

        // 替换 [gun:] 及其格式化字符串
        replace_by_position(output, temp_result, gun_pos - temp_result, gun_len, gun_str);

        return 1;
    }
    return 1;
}

// 主函数：DisQRcodeReplace
uint8_t DisQRcodeReplace(char *output, const char *input, const char *pileNum, uint8_t gun) {
    return handle_replacement(output, input, pileNum, gun);
}



/*****************************************************************************************************************
* 二维码配置：type:0无需补充替换字符，1需要拼接替换特殊字符
* 配置二维码链接，特殊字符[pile][gun:xx]
* 可以用作初始化，串口配置，平台配置
*****************************************************************************************************************/
static void QrcodeStringConfig(uint8_t type, char *output, char *input)
{
    strcpy(output, input);
    if (type == 1) {
        strcat(output, "[pile][gun:%02d]");
    }
}



/*****************************************************************************************************************
* 外部调用
*****************************************************************************************************************/

//串口二维码配置、初始化配置可调用
void SpecialCharQrcodeStringConfig(char *output, char *input)
{
    //查找是否存在[pile],不存在自动补[pile][gun:%02d]
    const char *pile_pattern = "[pile]";
    const char *gun_pattern = "[gun:";
    const char *pile_pos = find_substring(input, pile_pattern);
    const char *gun_pos = find_substring(input, gun_pattern);
    if ((pile_pos == NULL) && (gun_pos == NULL)) {
        QrcodeStringConfig(1, output, input);
    } else {
        QrcodeStringConfig(0, output, input);
    }
    printf("Qrcode:%s\r\n", output);
}


//平台配置
void NormalCharQrcodeStringConfig(char *output, char *input)
{
    QrcodeStringConfig(0, output, input);
    printf("Plat Qrcode:%s\r\n", output);
}

