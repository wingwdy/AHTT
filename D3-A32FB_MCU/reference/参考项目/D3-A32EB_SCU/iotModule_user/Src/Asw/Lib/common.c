#include "common.h"
#include "AppRunTime.h"

//===========================================================

#define RANDOM_MAX	0XFFFFFFFE
#define RANDOM_MIN	0
uint32_t random()
{
	U32 value;
	U32 seed = Get_Systick();
	srand(seed);
	value = rand() % (RANDOM_MAX + 1 - RANDOM_MIN) + RANDOM_MIN;
	return value;
}

/*************************************************************
  Function:    AsciiPToBCD
  Description: ASCII正转BCD
  Calls:       无
  Called By:   
  Input:       unsigned char *pdata_Ascii assii码数组
               unsigned char *pdata_bcd   bcd码数组
               unsigned char lenth        assii码长度
  Output:      无
  Return:
  Others:      无
*************************************************************/
void AsciiPToBCD(char *pdata_Ascii, char *pdata_bcd, uint32_t lenth)
{
	unsigned char i,tmp_h,tmp_l,j;
	for(i = 0;i < lenth/2;i++)
	{
		tmp_h = *(pdata_Ascii + 2*i);
		tmp_l = *(pdata_Ascii + 2*i +1);
		
		if(tmp_h < '0' || tmp_h > 'f' ||
			tmp_l < '0' || tmp_l > 'f')
		{
			for(j=i;j<lenth/2;j++)
			{
				*pdata_bcd ++ = 0;
			}
	     	break;
    	}
    	if(tmp_h  <= '9' && tmp_l <= '9')
      		*pdata_bcd ++ =  (tmp_h - '0') << 4 | (tmp_l - '0');
    	else if(tmp_h  > '9' && tmp_l <= '9')
      		*pdata_bcd ++ =  (tmp_h - 'a' + 10) << 4 | (tmp_l - '0');
    	else if(tmp_h  <= '9' && tmp_l > '9')
      		*pdata_bcd ++ =  (tmp_h - '0') << 4 | (tmp_l - 'a' + 10);
    	else if(tmp_h  > '9' && tmp_l > '9')
      		*pdata_bcd ++ =  (tmp_h - 'a' + 10) << 4 | (tmp_l - 'a' + 10);
	}
}

/*************************************************************
  Function:    BCDToAsciiP
  Calls:       无
  Called By:   
  Input:       unsigned char *pdata_Ascii assii码数组
               unsigned char *pdata_bcd   bcd码数组
               unsigned char lenth        assii码长度
  Output:      无
  Return:
  Others:      无
*************************************************************/
void BINToAscii(char *pdata_Ascii, char *pdata_bcd, uint32_t lenth)
{
    for (int i = 0; i < lenth; i++) {
        sprintf(pdata_Ascii, "%02x", pdata_bcd[i]);
        pdata_Ascii = pdata_Ascii + 2;
    }
}

void BINToBCD(U8 *bcd, U8 *bin, U32 len)
{
    //设备编码之类的BCD转成BIN，传输的时候全部按照BIN进行传输
    for (int i = 0; i < len; i++) {
        bcd[i] = bin[i] / 10 * 16 + bin[i] % 10;
    }
}

//顺序颠倒
void reverse(void *pIndata, void *pOutData, U16 len)
{
    U8 *pIn = (U8 *)pIndata;
    U8 *pOut = (U8 *)pOutData;
	
    for (U16 index = 0; index < len; index++)
    {
        pOut[index] = pIn[len - index - 1];
    }

    return;
}

U16 twoUint8ToUint16(U8 *pu8)
{
    uint8_t temp1, temp2;
    temp1 = *pu8++;
    temp2 = *pu8++;
    return ((temp2 << 8) + temp1);
}

void Uint16ToTwoUint8(U8 *pU8, U16 u16CurValue)
{
    *pU8++ = (u16CurValue & 0x00ff);
    *pU8++ = ((u16CurValue >> 8) & 0x00ff);

    return;
}

void Uint16ToTwoUint8LH(U8 *pU8, U16 u16CurValue)
{
    *pU8++ = ((u16CurValue >> 8) & 0x00ff);
    *pU8++ = (u16CurValue & 0x00ff);

    return;
}

void uint32ToFourUint8(U8 *pu8, U32 CurValue)
{
    *pu8++ = (CurValue & 0x00ff);
    *pu8++ = ((CurValue >> 8) & 0x00ff);
    *pu8++ = ((CurValue >> 16) & 0x00ff);
    *pu8++ = ((CurValue >> 24) & 0x00ff);
}

void uint32ToFourUint8LH(U8 *pu8, U32 CurValue)
{
    *pu8++ = ((CurValue >> 24) & 0x00ff);
    *pu8++ = ((CurValue >> 16) & 0x00ff);
    *pu8++ = ((CurValue >> 8) & 0x00ff);
    *pu8++ = (CurValue & 0x00ff);
}

void  uint32ToTwoUint8(U8 *pu8, U32 CurValue)
{
	*pu8++ = (CurValue&0x00ff);
	*pu8++ = ((CurValue>>8)&0x00ff);
}

uint32_t fourUint8ToUint32(U8 *pu8)
{
    uint32_t temp1, temp2, temp3, temp4;
    temp1 = *pu8++;
    temp2 = *pu8++;
    temp3 = *pu8++;
    temp4 = *pu8++;
    return ((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1);
}

uint32_t fourUint8ToUint32LH(U8 *pu8)
{
    uint32_t temp1, temp2, temp3, temp4;
    temp4 = *pu8++;
    temp3 = *pu8++;
    temp2 = *pu8++;
    temp1 = *pu8++;
    return ((temp4 << 24) + (temp3 << 16) + (temp2 << 8) + temp1);
}

void BCDToBin(U8 *bin, U8 *bcd, U32 len)
{
    //设备编码之类的BIN转成BCD,解析的时候用
    for (int i = 0; i < len; i++) {
        bin[i] = (bcd[i] / 16) *10 + bcd[i] % 16;
    }
}

U8 U8BcdToBin(U8 u8Bcd)
{
    return ((u8Bcd & 0xf0)>>4)*10+(u8Bcd & 0x0f);
}

U8 GetBitFlag(void *pflag, U32 u32num)
{
	U8 *p = (U8 *)pflag;

	if((p[u32num >> 3] & (1 << (u32num & 0x07))) != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void SetBitFlag(void *pflag, U32 u32num)
{
    U8 *p = (U8 *)pflag;

    p[u32num >> 3] |= (1 << (u32num & 0x07));

    return ;
}

void ClrBitFlag(void *pflag, U32 u32num)
{
    U8 *p = (U8 *)pflag;

    p[u32num >> 3] &= (~(1 << (u32num & 0x07)));

    return ;
}


/*************************************************************
* 名称：
* 功能：在一串数据中查找数据，并返回数据的起始地址
* 入口参数：找到返回首指针。没有找到返回NULL
* 出口参数：
* 作者：
* 编制日期：
**************************************************************/
U8* SearchData(U8 *pData, U16 DataLen, void *pString, U16 StringLen)
{
	while(DataLen >= StringLen)
	{
		if (0 == memcmp(pData, pString, StringLen))
		{
			return pData;
		}
		pData++;
		DataLen--;
	}

	return (NULL);
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ReplaceStr
//功能描述：	替代字符串
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U32 ReplaceStr(U8* pData, U32 nDataLen, char* cDestStr, void* pReplace, U32 nReplaceLen, char* pDefault)
{
	U8 *pDest = NULL;
	char cTailStr[100];
	void *pCopyData = pReplace;
	U32 nCopyLen = nReplaceLen;

	memset(cTailStr, 0x00, sizeof(cTailStr));

	pDest = SearchData(pData, nDataLen, cDestStr, strlen(cDestStr));
	if (NULL != pDest)
	{
		//保存尾巴
		strcpy(cTailStr, (char*)(pDest + strlen(cDestStr)));

		//默认
		if (0 == nCopyLen)
		{
			pCopyData = pDefault;
			nCopyLen = strlen(pDefault);
		}

		memcpy(pDest, pCopyData, nCopyLen);
		pDest += nCopyLen;

		//加上尾巴
		strcpy((char*)pDest, cTailStr);

		//重新计算长度
		nDataLen = nDataLen - strlen(cDestStr) + nCopyLen;
	}

	return nDataLen;
}

//////////////////////////////////////////////////////////////////////////
//函数名：		ReplaceNum
//功能描述：	替代数字
//入口参数：	无
//函数返回值：	无
//////////////////////////////////////////////////////////////////////////
U32 ReplaceNum(U8* pData, U32 nDataLen, char* cDestStr, U32 u32Replace, U32 u32Default)
{
	char cReplace[24];
	char cDefault[24];

	if (0 == u32Replace)
	{
		strcpy(cReplace, "");
	}
	else
	{
		sprintf(cReplace, "%d", u32Replace);
	}

	sprintf(cDefault, "%d", u32Default);

	return ReplaceStr(pData, nDataLen, cDestStr, cReplace, strlen(cReplace), cDefault);
}

uint8_t u8GeneralAddition(void *pData, uint32_t len)
{
	uint8_t u8Value = 0;
	uint8_t *pCurPointer = (uint8_t *)pData;
	
	for (; len != 0; len--)
	{
		u8Value += (*pCurPointer);
		pCurPointer++;
	}
	
	return (u8Value);
}

uint8_t CheckDataPack(void *pData, uint32_t stuctLen)
{
	uint32_t ctrlWord;
	uint8_t* pSrc = (uint8_t*)pData ;
	
	if (NULL == pSrc && stuctLen == 0) {
		return FALSE;
	}
	
	ctrlWord = fourUint8ToUint32(pSrc);
	
	//标志或数据长度不对
	if ((GET_VALID_FLAG(ctrlWord) != VALID) || (GETET_LEN(ctrlWord) != (stuctLen -4)))
	{
		return FALSE;
	}
	
	if (GET_CHECK_SUM(ctrlWord) == u8GeneralAddition(pSrc+4 ,stuctLen-4)) 
	{
		return TRUE;
	}
	
	return FALSE;
}

void PackData(void *pData1, uint32_t stuctLen)
{
	uint32_t ctrlWord = 0;
	uint8_t *pData = (uint8_t*)pData1;
	uint8_t cs;
	
	if (NULL == pData && stuctLen == 0) {
		return;
	}
	cs = u8GeneralAddition(pData+4, stuctLen-4);
	ctrlWord = SET_CHECK_SUM(ctrlWord, cs);
	ctrlWord = SET_VALID_FLAG(ctrlWord, VALID);
	ctrlWord = SET_LEN(ctrlWord, (stuctLen-4));
	uint32ToFourUint8(pData,ctrlWord);
	
	return ;
}

uint32_t CheckSum(uint8_t *pData, uint32_t len)
{
    uint32_t cs = 0;
    uint8_t *pCur = (uint8_t *)pData;
	
    for (; len != 0; len--)
    {
        cs += (*pCur);
        pCur++;
    }
	
    return (cs);
}

/*************************************************************************************
 * 关于cp56time2a
**************************************************************************************/
//pTime，年年月日时分秒，20250101121212
void Bin_to_Cp56time2a(uint8_t *pTime, cp56time2a *pCp56)
{		
	pCp56->Year = pTime[0];
	pCp56->Month = pTime[1];
	pCp56->Date = pTime[2];
	pCp56->Hour = pTime[3];
	pCp56->Minute = pTime[4];
	Uint16ToTwoUint8(pCp56->MilliSec, pTime[5]*1000);
	return;
}

void Bcd_to_Cp56time2a(uint8_t *pTime, cp56time2a *pCp56)
{
	pCp56->Year = U8BcdToBin(pTime[0]);
	pCp56->Month = U8BcdToBin(pTime[1]);
	pCp56->Date = U8BcdToBin(pTime[2]);
	pCp56->Hour = U8BcdToBin(pTime[3]);
	pCp56->Minute = U8BcdToBin(pTime[4]);
	Uint16ToTwoUint8(pCp56->MilliSec, U8BcdToBin(pTime[5])*1000);
	return;
}
void Cp56time2a_to_Bin(uint8_t *pTime, cp56time2a *pCp56)
{		
	pTime[0] = pCp56->Year;
	pTime[1] = pCp56->Month;
	pTime[2] = pCp56->Date;
	pTime[3] = pCp56->Hour;
	pTime[4] = pCp56->Minute;
	pTime[5] = twoUint8ToUint16(pCp56->MilliSec)/1000;
	return;
}


static const uint16_t month_days_table[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
 
static const uint16_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//        日期结构体对象
typedef struct {
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t min;
    uint16_t sec;
}date_time_t;

static uint16_t fml_leap_year(uint16_t year)
{
    return (((year % 4 == 0)&&(year % 100 != 0)) || (year % 400 == 0));
}
 
 

// @brief        日期转时间戳
// @param        [in] date 日期值
// @retval       =0 成功
// @retval       非0 失败，以及失败原因

static uint32_t time_to_stamp(date_time_t *pdate)
{
    static  uint32_t dax = 0;
    static  uint32_t day_count = 0;
    uint16_t leap_year_count = 0;
    uint16_t i;
	
    // 计算闰年数
    for (i = 1970; i < pdate->year; i++)
    {
        if (fml_leap_year(i))
        {
            leap_year_count++;
        }
    }
	
    // 计算年的总天数
    day_count = leap_year_count * 366 + (pdate->year - 1970 - leap_year_count) * 365;
	
    // 累加计算当年所有月的天数
    for (i = 1; i < pdate->month; i++)
    {
        if ((2 == i) && (fml_leap_year(pdate->year)))
        {
            day_count += 29;
        }
        else
        {
            day_count += month_days_table[i];
        }
    }
	
    // 累加计算当月的天数
    day_count += (pdate->day - 1);
	
    dax = (uint32_t)(day_count * 86400) + (uint32_t)((uint32_t)pdate->hour * 3600) + (uint32_t)((uint32_t)pdate->min * 60) + (uint32_t)pdate->sec;
	
    /* 北京时间补偿 */
	dax = dax - 8*60*60;
	
    return dax;
}

// @brief        时间戳转化为日期
// @param        [in] date 日期值
// @retval       =0 成功
// @retval       非0 失败，以及失败原因
static uint32_t stamp_to_time(uint32_t timep, date_time_t *date)
{
    uint32_t days = 0;
    uint32_t rem = 0;
    uint16_t month = 0;
	
	
    /* 北京时间补偿 */
//    timep = timep + 8*60*60;
	
    // 计算天数
    days = (uint32_t)(timep / 86400);
    rem = (uint32_t)(timep % 86400);
	
    // 计算年份
    uint16_t year;
    for (year = 1970; ; ++year)
    {
        uint16_t leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
        uint16_t ydays = leap ? 366 : 365;
        if (days < ydays)
        {
            break;
        }
        days -= ydays;
    }
    date->year  =  year;
	
    // 计算月份
    for (month = 0; month < 12; month++)
    {
        uint16_t mdays = days_in_month[month];
        if (month == 1 && ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        {
            mdays = 29;
        }
        if (days < mdays)
        {
            break;
        }
        days -= mdays;
    }
    date->month = month;
    date->month += 1;
 
    // 计算日期
    date->day = days + 1;
 
    // 计算时间
    date->hour = rem / 3600;
    rem %= 3600;
    date->min = rem / 60;
    date->sec = rem % 60;
 
    return 0;
}

uint32_t getSince1970StampSys(void)
{
	uint8_t u8Time[9] = {0};
	uint32_t u32Second = 0;
	date_time_t strDate;
	
	getRunTime(u8Time);
	memset(&strDate, 0, sizeof(date_time_t));
	strDate.year = u8Time[0]*100 + u8Time[1];
	strDate.month = u8Time[2];
	strDate.day = u8Time[3];
	strDate.hour = u8Time[4];
	strDate.min = u8Time[5];
	strDate.sec = u8Time[6];
	
	u32Second = time_to_stamp(&strDate);
	
    return u32Second;
}

uint64_t getSince1970StampSysMs(void)
{
	uint8_t u8Time[9] = {0};
	uint32_t u32Second = 0;
	date_time_t strDate;
	uint64_t u64Ms = 0;
	
	getRunTime(u8Time);
	memset(&strDate, 0, sizeof(date_time_t));
	strDate.year = u8Time[0]*100 + u8Time[1];
	strDate.month = u8Time[2];
	strDate.day = u8Time[3];
	strDate.hour = u8Time[4];
	strDate.min = u8Time[5];
	strDate.sec = u8Time[6];
	
	u32Second = time_to_stamp(&strDate);

	u64Ms = (((uint64_t)u32Second)*1000) + 0;
    return u64Ms;
}

//年前两位，年后两位，月日时分秒
uint32_t getSince1970StampTime(uint8_t *pTime)
{
	uint32_t u32Second = 0;
	date_time_t strDate;
	
	memset(&strDate, 0, sizeof(date_time_t));
	strDate.year = pTime[0]*100 + pTime[1];
	strDate.month = pTime[2];
	strDate.day = pTime[3];
	strDate.hour = pTime[4];
	strDate.min = pTime[5];
	strDate.sec = pTime[6];
	
	u32Second = time_to_stamp(&strDate);
	
    return u32Second;
}

uint64_t getSince1970StampTimeMs(uint8_t *pTime)
{
	uint32_t u32Second = 0;
	date_time_t strDate;
	uint64_t u64Ms = 0;
	
	memset(&strDate, 0, sizeof(date_time_t));
	strDate.year = pTime[0]*100 + pTime[1];
	strDate.month = pTime[2];
	strDate.day = pTime[3];
	strDate.hour = pTime[4];
	strDate.min = pTime[5];
	strDate.sec = pTime[6];
	
	u32Second = time_to_stamp(&strDate);

	u64Ms = (((uint64_t)u32Second)*1000) + 0;
    return u64Ms;
}

void Set1970SecToSysTime(uint32_t u32Stamp)
{
	date_time_t strDate;
	uint8_t u8Time[8] = {0};
	
	memset(&strDate, 0, sizeof(date_time_t));
	stamp_to_time(u32Stamp, &strDate);
	
	u8Time[0] = strDate.year/100;
	u8Time[1] = strDate.year%100;
	u8Time[2] = strDate.month;
	u8Time[3] = strDate.day;
	u8Time[4] = strDate.hour;
	u8Time[5] = strDate.min;
	u8Time[6] = strDate.sec;
	
	setRunTime1(u8Time, 0, 3);
	
    return;
}


void string_split_to_int(uint8_t *buf, char *string, int len)
{
    int i = 0, j;
	char cp_string[16] = {0};
	memcpy(cp_string, string, strlen(string));
    char *p = strtok(cp_string, ".");
    while (p) {
        buf[i] = atoi(p);
        ++i;
        if (i >= len) {
            break;
        }
        p = strtok(NULL, ".");
    }
	//向后移动len-i位,前面补0
	int move = len - i;
	if (move) {
		uint8_t mid;
		for (j = len - 1; j > move - 1; j--) {
			mid = buf[j - move];
			buf[j] = mid;
		}
		for (j = 0; j < move; j++) {
			buf[j] = 0;
		}
	}
}


static U8 isNumericString(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9') {
            return 0; // 非数字字符，返回0
        }
        i++;
    }
    
    return 1; // 字符串中仅包含数字，返回1
}
void String2bin(U8* dest, const char* src, int destLength) {
	int t_strLen = strlen(src);
	int count = 0;
	//包含数字之外字符
	if (!isNumericString(src)) {
		return;
	}
	//奇数长度无法转换
	if (t_strLen % 2) {
		return;
	}

	// 遍历字符串并将两个字符一组转换为uint8_t类型的数组元素
    for (int i = 0; i < t_strLen; i += 2) {
        char substr[3]; // 存放两个字符的子串（包括结束符）
        strncpy(substr, src + i, 2); // 截取两个字符的子串
        substr[2] = '\0'; // 添加字符串结束符
        
        // 将子串转换为整数并存储到数组中
        dest[count++] = (uint8_t) strtol(substr, NULL, 10);
        
        if (count >= destLength) {
            break; // 达到数组大小限制，退出循环
        }
    }
}