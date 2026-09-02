#ifndef __COMMON_H__
#define __COMMON_H__

#include "globals.h"


#define MALLOC(size)			(malloc(size))
#define FREE(pointer)			do{free(pointer); pointer = NULL;}while(0);

#define UPRINT(...)  printf(__VA_ARGS__)
#define UPTBUF(...)  hex_dump(__VA_ARGS__)

uint32_t random(void);


typedef struct
{
    uint8_t             MilliSec[2]; // 毫秒
    uint8_t             Minute : 6;  // 分
    uint8_t             res1   : 2;                    
    uint8_t             Hour   : 5;  // 时
    uint8_t             res2   : 3;                    
    uint8_t             Date   : 5;  // 日
    uint8_t             Day    : 3;  // 周                   
    uint8_t             Month  : 4;  // 月
    uint8_t             res4   : 4;                    
    uint8_t             Year   : 7;  // 年
    uint8_t             res5   : 1;
}cp56time2a;


//=======================================================================
void AsciiPToBCD(char *pdata_Ascii, char *pdata_bcd, U32 lenth);

void BINToAscii(char *pdata_Ascii, char *pdata_bcd, uint32_t lenth);

void BINToBCD(U8 *bcd, U8 *bin, U32 len);

void reverse(void *pIndata, void *pOutData, U16 len);

uint16_t twoUint8ToUint16(U8 *pu8);

void Uint16ToTwoUint8(U8 *pU8, U16 u16CurValue);

void Uint16ToTwoUint8LH(U8 *pU8, U16 u16CurValue);

void uint32ToFourUint8(U8 *pu8, U32 CurValue);
void uint32ToFourUint8LH(U8 *pu8, U32 CurValue);

void uint32ToTwoUint8(U8 *pu8, U32 CurValue);

uint32_t fourUint8ToUint32(U8 *pu8);

uint32_t fourUint8ToUint32LH(U8 *pu8);

void BCDToBin(U8 *bin, U8 *bcd, U32 len);

U8 U8BcdToBin(U8 u8Bcd);

U8 GetBitFlag(void *pflag, U32 u32num);
void SetBitFlag(void *pflag, U32 u32num);
void ClrBitFlag(void *pflag, U32 u32num);


//cp56time2a转换
void Bin_to_Cp56time2a(uint8_t *pTime, cp56time2a *pCp56);
void Bcd_to_Cp56time2a(uint8_t *pTime, cp56time2a *pCp56);
void Cp56time2a_to_Bin(uint8_t *pTime, cp56time2a *pCp56);
void Bcd_to_Cp56time2a(uint8_t *pTime, cp56time2a *pCp56);  //wjy


void string_split_to_int(uint8_t *buf, char *string, int len);


void String2bin(U8* dest, const char* src, int destLength);
/*************************************************************
* 名称：
* 功能：在一串数据中查找数据，并返回数据的起始地址
* 入口参数：找到返回首指针。没有找到返回NULL
* 出口参数：
* 作者：
* 编制日期：
**************************************************************/
U8* SearchData(U8 *pData, U16 DataLen, void *pString, U16 StringLen);
//////////////////////////////////////////////////////////////////////////
//函数名：		ReplaceStr
//功能描述：	替代字符串
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U32 ReplaceStr(U8* pData, U32 nDataLen, char* cDestStr, void* pReplace, U32 nReplaceLen, char* pDefault);

//////////////////////////////////////////////////////////////////////////
//函数名：		ReplaceNum
//功能描述：	替代数字
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U32 ReplaceNum(U8* pData, U32 nDataLen, char* cDestStr, U32 u32Replace, U32 u32Default);

uint8_t u8GeneralAddition(void *pData, uint32_t len);

uint8_t CheckDataPack(void *pData, uint32_t stuctLen);

void PackData(void *pData1, uint32_t stuctLen);

uint32_t CheckSum(uint8_t *pData, uint32_t len);

uint32_t getSince1970StampSys(void);
uint64_t getSince1970StampSysMs(void);
uint32_t getSince1970StampTime(uint8_t *pTime);
uint64_t getSince1970StampTimeMs(uint8_t *pTime);
void Set1970SecToSysTime(uint32_t u32Stamp);
#endif
