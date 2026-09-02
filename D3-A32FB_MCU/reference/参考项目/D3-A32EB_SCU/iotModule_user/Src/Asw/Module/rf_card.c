/***********************************************************************************
 * 文 件 名  : rf_card.c
 * 版 本 号  : V1.0
 * 负 责 人  : WEEN
 * 创建日期  : 2021-9-16
 * 文件描述  : 读卡器驱动函数
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  : 
 * 其    他  : 
 * 修改日志  : 初版
***********************************************************************************/
#include "string.h"
#include <stdint.h>
#include "rf_card.h"
#include "FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"


static uint32_t NFC_RST_GPIO[2][2] = {{NFC1_RST_GPIO, NFC1_RST_PIN}, {NFC2_RST_GPIO, NFC2_RST_PIN}};
static uint32_t NFC_IRQ_GPIO[2][2] = {{NFC1_IRQ_GPIO, NFC1_IRQ_PIN}, {NFC2_IRQ_GPIO, NFC2_IRQ_PIN}};
static uint32_t NFC_NSS_GPIO[2][2] = {{NFC1_NSS_GPIO, NFC1_NSS_PIN}, {NFC2_NSS_GPIO, NFC2_NSS_PIN}};
static uint32_t NFC_CLK_GPIO[2][2] = {{NFC1_CLK_GPIO, NFC1_CLK_PIN}, {NFC2_CLK_GPIO, NFC2_CLK_PIN}};
static uint32_t NFC_MISO_GPIO[2][2] = {{NFC1_MISO_GPIO, NFC1_MISO_PIN}, {NFC2_MISO_GPIO, NFC2_MISO_PIN}};
static uint32_t NFC_MOSI_GPIO[2][2] = {{NFC1_MOSI_GPIO, NFC1_MOSI_PIN}, {NFC2_MOSI_GPIO, NFC2_MOSI_PIN}};


#define RC522_RST_H(x)        gpio_bit_set(NFC_RST_GPIO[x][0], NFC_RST_GPIO[x][1])
#define RC522_RST_L(x)        gpio_bit_reset(NFC_RST_GPIO[x][0], NFC_RST_GPIO[x][1])
#define RC522_SPI_NSS_H(x)    gpio_bit_set(NFC_NSS_GPIO[x][0], NFC_NSS_GPIO[x][1])
#define RC522_SPI_NSS_L(x)    gpio_bit_reset(NFC_NSS_GPIO[x][0], NFC_NSS_GPIO[x][1])
#define RC522_SPI_SCLK_H(x)   gpio_bit_set(NFC_CLK_GPIO[x][0], NFC_CLK_GPIO[x][1])
#define RC522_SPI_SCLK_L(x)   gpio_bit_reset(NFC_CLK_GPIO[x][0], NFC_CLK_GPIO[x][1])
#define RC522_SPI_MOSI_H(x)   gpio_bit_set(NFC_MOSI_GPIO[x][0], NFC_MOSI_GPIO[x][1])
#define RC522_SPI_MOSI_L(x)   gpio_bit_reset(NFC_MOSI_GPIO[x][0], NFC_MOSI_GPIO[x][1])

#define RC522_SPI_MISO_HL(x)  gpio_input_bit_get(NFC_MISO_GPIO[x][0], NFC_MISO_GPIO[x][1])
#define RC522_IRQ_HL(x)       gpio_input_bit_get(NFC_IRQ_GPIO[x][0], NFC_IRQ_GPIO[x][1])



static uint8_t card_com_flag;//读卡器通讯标识:1,异常
/********************************************************************* 
* Spi Slave rate lower 10Mbit/s      Main 64M  
*********************************************************************/
static inline void delay_for_slave(void)
{
    for (uint8_t i = 0; i < 10; i++) {
    __NOP();__NOP();__NOP();__NOP();__NOP();
    }
//  asm("nop");asm("nop");asm("nop");
//	asm("nop");asm("nop");asm("nop");
//	asm("nop");asm("nop");asm("nop");
//	asm("nop");asm("nop");asm("nop");
}
/*****************************************************************************
 * 函 数 名  : Get_Card_Com_Flag
 * 负 责 人  : WEEN
 * 创建日期  : 2021年9月26日
 * 函数功能  : 获取读卡器通讯标识函数
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  : called by needs
 * 其    它  :   
*****************************************************************************/
uint8_t Get_Card_Com_Flag(void)
{
  return card_com_flag;
}
/********************************************************************* 
* RC522 复位延迟时间 大于100ns
*********************************************************************/
void delay_for_rst(void)
{
  unsigned int i= 500;//900
  while(i--)
  {
    __NOP();__NOP();__NOP();__NOP();__NOP();
    __NOP();__NOP();__NOP();__NOP();__NOP();
    __NOP();__NOP();__NOP();__NOP();__NOP();
    __NOP();__NOP();__NOP();__NOP();__NOP();
  }
}
void RC522_SPI_MOSI(uint8_t nfcI, uint8_t i)
//__inline void RC522_SPI_MOSI(uint8_t i)
{
  if(i)
    RC522_SPI_MOSI_H(nfcI);
  else
    RC522_SPI_MOSI_L(nfcI);
}
/***************************************************************
*  向SPI器件写入一个字节数据
****************************************************************/
void RC522_SPI_WriteByte(uint8_t nfcI, uint8_t data) 
{
	char i;
	for(i=8;i>0;i--)
	{
		RC522_SPI_MOSI(nfcI, (data&0x80));
		RC522_SPI_SCLK_H(nfcI);
		data <<= 1;
		delay_for_slave();
		RC522_SPI_SCLK_L(nfcI);
		delay_for_slave();
	} 
}
/*****************************************************************
*  从SPI器件读出一个字节数据
******************************************************************/
uint8_t RC522_SPI_ReadByte(uint8_t nfcI)
{
  uint8_t data=0;
	char i;
//  for(i=0;i<8;i++)
//  {
//    RC522_SPI_SCLK_L;
//    delay_for_slave();
//    RC522_SPI_SCLK_H;
//    data = (data <<1) | RC522_SPI_MISO_HL;
//    delay_for_slave();
//  }
	for(i=0;i<8;i++)
  {
		data <<= 1;
    RC522_SPI_SCLK_L(nfcI);
    delay_for_slave();
		if(RC522_SPI_MISO_HL(nfcI))
			data |= 0x01;
		RC522_SPI_SCLK_H(nfcI);
    delay_for_slave();
  }
  return data;
}
/*******************************************************************
*功    能：读RC522寄存器
*参数说明：Address[IN]:寄存器地址
*返    回：读出的值
*******************************************************************/
unsigned char ReadRawRC(uint8_t i, unsigned char Address)
{
	unsigned char ucAddr;
	unsigned char ucResult=0;

	RC522_SPI_SCLK_L(i);
	RC522_SPI_NSS_L(i);

	ucAddr = ((Address<<1)&0x7E)|0x80;
	RC522_SPI_WriteByte(i, ucAddr);
	ucResult = RC522_SPI_ReadByte(i);

	RC522_SPI_NSS_H(i);
	RC522_SPI_SCLK_H(i);
	return ucResult;
}
/*******************************************************************
*功    能：写RC522寄存器
*参数说明：Address[IN]:寄存器地址
*          value[IN]:写入的值
********************************************************************/
void WriteRawRC(uint8_t i, unsigned char Address, unsigned char value)
{  
    unsigned char ucAddr;

    RC522_SPI_SCLK_L(i);
    RC522_SPI_NSS_L(i);
     
    ucAddr = ((Address<<1)&0x7E);
    RC522_SPI_WriteByte(i, ucAddr);
    RC522_SPI_WriteByte(i, value);
    
    RC522_SPI_NSS_H(i);
    RC522_SPI_SCLK_H(i);  
}
/******************************************************************
*功    能：置RC522寄存器位
*参数说明：reg[IN]:寄存器地址
*          mask[IN]:置位值
******************************************************************/
void SetBitMask(uint8_t i, unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(i, reg);
    WriteRawRC(i, reg,tmp | mask);  // set bit mask
}
/****************************************************************
*功    能：清RC522寄存器位
*参数说明：reg[IN]:寄存器地址
*          mask[IN]:清位值
****************************************************************/
void ClearBitMask(uint8_t i, unsigned char reg,unsigned char mask)  
{
	char tmp = 0x0;
	tmp = ReadRawRC(i, reg);
	WriteRawRC(i, reg, tmp & ~mask);  // clear bit mask
}


void PcdAntennaOnIndex(uint8_t nfcI, uint8_t index)
{
   unsigned char i;
    ClearBitMask(nfcI, TxControlReg, 0x03);
    //全部开启
    if (index == 0) {
        index = 3;
    }
    i = ReadRawRC(nfcI, TxControlReg);
    if (!(i & index))
    {
        SetBitMask(nfcI, TxControlReg, index);
    }
}

/*****************************************************************
*功    能：通过RC522和ISO14443卡通讯
*参数说明：Command[IN]:RC522命令字
*          pInData[IN]:通过RC522发送到卡片的数据
*          InLenByte[IN]:发送数据的字节长度
*          pOutData[OUT]:接收到的卡片返回数据
*          *pOutLenBit[OUT]:返回数据的位长度
*******************************************************************/
signed char PcdComMF522(uint8_t nfcI, 
                  unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    signed char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char n,lastBits;
    unsigned int i;
  
    static unsigned char err_cnt=0;
    
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    WriteRawRC(nfcI, ComIEnReg,irqEn|0x80);
    ClearBitMask(nfcI, ComIrqReg,0x80);
    WriteRawRC(nfcI, CommandReg,PCD_IDLE);
    SetBitMask(nfcI, FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
      WriteRawRC(nfcI, FIFODataReg, pInData[i]);    
    }
    WriteRawRC(nfcI, CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    
      SetBitMask(nfcI, BitFramingReg,0x80);  
    }
    // 根据时钟频率调整，操作M1卡最大等待时间25ms; 定时器24ms 定时器操作
    i = 1500;//2000
    do 
    {
       n = ReadRawRC(nfcI, ComIrqReg);  
       i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));//600->210  213定时器超时溢出 15ms  不超时  2000 -> 750
    
    ClearBitMask(nfcI, BitFramingReg,0x80);
	      
    //if (i!=0)
    //{    
         if(!(ReadRawRC(nfcI, ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {  status = MI_NOTAGERR;   }
						 else if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(nfcI, FIFOLevelReg);
              	lastBits = ReadRawRC(nfcI, ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(nfcI, FIFODataReg);    }  
            }
            err_cnt = 0;
            card_com_flag = 0;
         }
         else
         {   
					 status = MI_ERR;
           if(err_cnt++ > 25)
           {
             RF_Card_Init();
             err_cnt = 0;
             card_com_flag = 1;
             return status;
           }
				 }
   //}
   
   SetBitMask(nfcI, ControlReg,0x80);           // stop timer now
   WriteRawRC(nfcI, CommandReg,PCD_IDLE); 
   return status;
}

/*******************************************************************
*用RC522计算CRC16函数
********************************************************************/
void CalulateCRC(uint8_t nfcI, unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
  unsigned char i,n;
  unsigned int cnt;
  ClearBitMask(nfcI, DivIrqReg,0x04);
  WriteRawRC(nfcI, CommandReg,PCD_IDLE);
  SetBitMask(nfcI, FIFOLevelReg,0x80);
  for (i=0; i<len; i++)
  {   WriteRawRC(nfcI, FIFODataReg, *(pIndata+i));   }
  WriteRawRC(nfcI, CommandReg, PCD_CALCCRC);
  // 根据时钟频率调整  0xFF      //时间调整
  cnt = 0x200;
  do 
  {
    n = ReadRawRC(nfcI, DivIrqReg);
    cnt--;
  }
  while ((cnt!=0) && !(n&0x04));
  pOutData[0] = ReadRawRC(nfcI, CRCResultRegL);
  pOutData[1] = ReadRawRC(nfcI, CRCResultRegM);
}

/**************************************************************************
*功    能：复位RC522        100nS复位信号
*返    回: 成功返回MI_OK
***************************************************************************/
char PcdReset(int i)
{
  RC522_RST_H(i);
  delay_for_rst();
  RC522_RST_L(i);
  delay_for_rst();
  RC522_RST_H(i);
  delay_for_rst();
  WriteRawRC(i, CommandReg,PCD_RESETPHASE);
  delay_for_rst();
    
  WriteRawRC(i, ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
  WriteRawRC(i, TReloadRegL,30);           
  WriteRawRC(i, TReloadRegH,0);
  WriteRawRC(i, TModeReg,0x8D);           //分频到2000     15ms
  WriteRawRC(i, TPrescalerReg,0x3E);
  WriteRawRC(i, TxAutoReg,0x40);
  
  WriteRawRC(i, ControlReg,0x10);
	
  return MI_OK;
}
/*******************************************************************
*关闭天线
********************************************************************/
void PcdAntennaOff(uint8_t nfcI)
{
  ClearBitMask(nfcI, TxControlReg, 0x03);
}

/********************************************************************
*功    能：寻卡
*参数说明: req_code[IN]:寻卡方式
*                0x52 = 寻感应区内所有符合14443A标准的卡  
*                0x26 = 寻未进入休眠状态的卡
*          pTagType[OUT]：卡片类型代码
*                0x4400 = Mifare_UltraLight
*                0x0400 = Mifare_One(S50)
*                0x0200 = Mifare_One(S70)
*                0x0800 = Mifare_Pro(X)
*                0x4403 = Mifare_DESFire
*返    回: 成功返回MI_OK
*********************************************************************/
signed char PcdRequest(uint8_t nfcI, unsigned char req_code,unsigned char *pTagType)
{
	signed char status;  
	unsigned int  unLen;
  
	unsigned char ucInCom = req_code; 

	unsigned char ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(nfcI, Status2Reg,0x08);
	WriteRawRC(nfcI, BitFramingReg,0x07);
	SetBitMask(nfcI, TxControlReg,0x03);

	status = PcdComMF522(nfcI, PCD_TRANSCEIVE,&ucInCom,1,ucComMF522Buf,&unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];//04
		*(pTagType+1) = ucComMF522Buf[1];//00
	}
	else
		status = MI_ERR;

	return status;
}

/****************************************************************************
*功    能：防冲撞
*参数说明: pSnr[OUT]:卡片序列号，4字节
*返    回: 成功返回MI_OK
*****************************************************************************/  
signed char PcdAnticoll(uint8_t nfcI, unsigned char *pSnr)
{
  signed char status;
  unsigned char i,snr_check=0;
  unsigned int  unLen;
  
	unsigned char ucInCom[2] = {PICC_ANTICOLL1, 0x20}; 

  unsigned char ucComMF522Buf[MAXRLEN]; 

  ClearBitMask(nfcI, Status2Reg,0x08);
  WriteRawRC(nfcI, BitFramingReg,0x00);
  ClearBitMask(nfcI, CollReg,0x80);

  status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucInCom,2,ucComMF522Buf,&unLen);
  if (status == MI_OK)
  {
    for (i=0; i<4; i++)
    {   
      *(pSnr+i)  = ucComMF522Buf[i];
      snr_check ^= ucComMF522Buf[i];
    }
    if (snr_check != ucComMF522Buf[i])
    {   status = MI_ERR;    }
  } 
  SetBitMask(nfcI, CollReg,0x80);
    
	return status;
}


/********************************************************************
*功    能：寻卡
*参数说明: req_code[IN]:寻卡方式
*                0x52 = 寻感应区内所有符合14443A标准的卡  
*                0x26 = 寻未进入休眠状态的卡
*          pTagType[OUT]：卡片类型代码
*                0x4400 = Mifare_UltraLight
*                0x0400 = Mifare_One(S50)
*                0x0200 = Mifare_One(S70)
*                0x0800 = Mifare_Pro(X)
*                0x4403 = Mifare_DESFire
*返    回: 成功返回MI_OK
*********************************************************************/
signed char PcdReadCard(uint8_t nfcI, unsigned char *Seri,unsigned char *pTagType)
{
	signed char status;  
	unsigned int  unLen;
  
	unsigned char ucInCom = PICC_REQALL; 

	unsigned char ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(nfcI, Status2Reg,0x08);
	WriteRawRC(nfcI, BitFramingReg,0x07);
	// SetBitMask(nfcI, TxControlReg,0x03);

	status = PcdComMF522(nfcI, PCD_TRANSCEIVE, &ucInCom, 1, ucComMF522Buf, &unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];//04
		*(pTagType+1) = ucComMF522Buf[1];//00

      PcdAnticoll(nfcI, Seri);
	}
	else
		status = MI_ERR;

	return status;
}

/**********************************************************************
*功    能：选定卡片
*参数说明: pSnr[IN]:卡片序列号，4字节
*返    回: 成功返回MI_OK
***********************************************************************/
signed char PcdSelect(uint8_t nfcI, unsigned char *pSnr)
{
   signed char status;
   unsigned char i;
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN];
    
   ucComMF522Buf[0] = PICC_ANTICOLL1;
   ucComMF522Buf[1] = 0x70;
   ucComMF522Buf[6] = 0;
   for (i=0; i<4; i++)
   {
    ucComMF522Buf[i+2] = *(pSnr+i);
    ucComMF522Buf[6]  ^= *(pSnr+i);
   }
   CalulateCRC(nfcI, ucComMF522Buf,7,&ucComMF522Buf[7]);
   ClearBitMask(nfcI, Status2Reg,0x08);

   status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen); 
   if ((status == MI_OK) && (unLen == 0x18))
   {   status = MI_OK;  }
   else
   {   status = MI_ERR;    }

   return status;
}

/*****************************************************************************
*功    能：验证卡片密码
*参数说明: auth_mode[IN]: 密码验证模式
*                0x60 = 验证A密钥
*                0x61 = 验证B密钥 
*          addr：块地址//-->扇区认证
*          pKey[IN]：密码
*          pSnr[IN]：卡片序列号，4字节
*返    回: 成功返回MI_OK
*****************************************************************************/               
signed char PcdAuthState(uint8_t nfcI, unsigned char auth_mode,unsigned char addr,const unsigned char *pKey,unsigned char *pSnr)
{
   signed char status;
   unsigned int  unLen;
   unsigned char i,ucComMF522Buf[MAXRLEN]; 

   ucComMF522Buf[0] = auth_mode;
   ucComMF522Buf[1] = addr;
   for (i=0; i<6; i++)
   {    ucComMF522Buf[i+2] = *(pKey+i);   }
   for (i=0; i<4; i++)
   {    ucComMF522Buf[i+8] = *(pSnr+i);   }
   // memcpy(&ucComMF522Buf[2], pKey, 6); 
   // memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
   status = PcdComMF522(nfcI, PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);//0x01 认证出错
   if ((status != MI_OK) || (!(ReadRawRC(nfcI, Status2Reg) & 0x08)))
   {   status = MI_ERR;   }
    
   return status;
}

/**************************************************************************
*设置RC522的工作方式 
***************************************************************************/
signed char PcdConfigISOType(uint8_t nfcI, unsigned char type)
{
   if (type == 'A')//ISO14443_A    RC5xxx系列仅支持单模式
   { 
     ClearBitMask(nfcI, Status2Reg,0x08);
     WriteRawRC(nfcI, ModeReg,0x3D);     //3F
     WriteRawRC(nfcI, TxSelReg,0x10);
     WriteRawRC(nfcI, RxSelReg,0x86);    //84 
     WriteRawRC(nfcI, RFCfgReg,0x7F);    //4F    //RxGain

     WriteRawRC(nfcI, TReloadRegL,0x39);// ox30--25ms   0x1C    15ms
     WriteRawRC(nfcI, TReloadRegH,0);
     WriteRawRC(nfcI, TModeReg,0x86);//0x8D
     WriteRawRC(nfcI, TPrescalerReg,0x9F);//0x3E
     
     osDelay(10);
   }
   else{ return -1; }
   
   return MI_OK;
}
/*****************************************************************************
*功    能：读取M1卡一块数据
*参数说明: addr：块地址
*         pData[OUT]：读出的数据，16字节
*返    回: 成功返回MI_OK
******************************************************************************/ 
signed char PcdRead(uint8_t nfcI, unsigned char addr,unsigned char *pData)
{
   signed char status;
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 

   ucComMF522Buf[0] = PICC_READ;
   ucComMF522Buf[1] = addr;
   CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);
   
   status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
   if ((status == MI_OK) && (unLen == 0x90))//18 bytes = 16data + 2CRC
   {
     memcpy(pData, ucComMF522Buf, 16);
     CalulateCRC(nfcI, pData,16,ucComMF522Buf);
     if((ucComMF522Buf[0] != ucComMF522Buf[16]) || (ucComMF522Buf[1] != ucComMF522Buf[17]))
     {
       status = MI_ERR;
     }
   }
   else
   {   status = MI_ERR;   }
    
   return status;
}
/****************************************************************************
*功    能：写数据到M1卡一块
*参数说明: addr：块地址
*          pData[IN]：写入的数据，16字节
*返    回: 成功返回MI_OK
*****************************************************************************/                  
signed char PcdWrite(uint8_t nfcI, unsigned char addr,unsigned char *pData)
{
   signed char status;
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN];//i； 
    
   ucComMF522Buf[0] = PICC_WRITE;
   ucComMF522Buf[1] = addr;
   CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);
  
   status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
   if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
   {   status = MI_ERR;   }
        
   if (status == MI_OK)
   {
     memcpy(ucComMF522Buf, pData, 16);
     CalulateCRC(nfcI, ucComMF522Buf,16,&ucComMF522Buf[16]);
     status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
     if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
     {   status = MI_ERR;   }
   }
    
   return status;
}

/*****************************************************************************
*功    能：扣款和充值
*参数说明: dd_mode[IN]：命令字
*          0xC0 = 扣款
*          0xC1 = 充值
*          addr：钱包地址
*          pValue[IN]：4字节增(减)值，低位在前
*返    回: 成功返回MI_OK
*****************************************************************************/                 
signed char PcdValue(uint8_t nfcI, unsigned char dd_mode,unsigned char addr,unsigned char *pValue)
{
    signed char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);   //

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
       // memcpy(ucComMF522Buf, pValue, 4);
        for (i=0; i<4; i++)
        {    ucComMF522Buf[i] = *(pValue+i);   }
        CalulateCRC(nfcI, ucComMF522Buf,4,&ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_ERR)           //此处允许
        {    status = MI_OK;    }
    }
    
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]); 
   
        status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
		
    return status;
}

/*************************************************************************
*功    能：备份钱包
*参数说明: sourceaddr[IN]：源地址
*          goaladdr[IN]：目标地址
*返    回: 成功返回MI_OK
***************************************************************************/
signed char PcdBakValue(uint8_t nfcI, unsigned char sourceaddr, unsigned char goaladdr)
{
    signed char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
    
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(nfcI, ucComMF522Buf,4,&ucComMF522Buf[4]);
 
        status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_ERR)
        {    status = MI_OK;    }
    }
    
    if (status != MI_OK)
    {    return MI_ERR;   }
    
    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    return status;
}

/**********************************************************************
*功    能：命令卡片进入休眠状态
*返    回: 成功返回MI_OK
***********************************************************************/
signed char PcdHalt(uint8_t nfcI)
{
	signed char status;
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 

	ucComMF522Buf[0] = PICC_HALT;
	ucComMF522Buf[1] = 0;
	CalulateCRC(nfcI, ucComMF522Buf,2,&ucComMF522Buf[2]);

	status = PcdComMF522(nfcI, PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

	return status;//MI_OK;
}

/***********************************
* @Name  RC522_isConnected()
* @Brief 检查RC522模块是否连接
* @Retun 1 - 连接成功
*        0 - 连接失败
************************************/
signed char Pcd_isConnected(uint8_t nfcI)
{
	uint8_t version = ReadRawRC(nfcI, VersionReg);
	if(version == 0x91)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*****************************************************************************
 * 函 数 名  : RF_Card_Init
 * 负 责 人  : WEEN
 * 创建日期  : 2021年9月16日
 * 函数功能  : 读卡器初始化
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 四线SPI模拟
*****************************************************************************/
void RF_Card_Init(void)
{
  for(int i = 0; i < GUN_NUM_MAX; i++) {
    gpio_init(NFC_RST_GPIO[i][0], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NFC_RST_GPIO[i][1]);
    gpio_init(NFC_NSS_GPIO[i][0], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NFC_NSS_GPIO[i][1]);
    gpio_init(NFC_CLK_GPIO[i][0], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NFC_CLK_GPIO[i][1]);
    gpio_init(NFC_MOSI_GPIO[i][0], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, NFC_MOSI_GPIO[i][1]);
    
    gpio_init(NFC_MISO_GPIO[i][0], GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, NFC_MISO_GPIO[i][1]);
    gpio_init(NFC_IRQ_GPIO[i][0], GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, NFC_IRQ_GPIO[i][1]);

    gpio_bit_reset(NFC_RST_GPIO[i][0], NFC_RST_GPIO[i][1]);
    gpio_bit_reset(NFC_NSS_GPIO[i][0], NFC_NSS_GPIO[i][1]);
    gpio_bit_reset(NFC_CLK_GPIO[i][0], NFC_CLK_GPIO[i][1]);
    gpio_bit_reset(NFC_MOSI_GPIO[i][0], NFC_MOSI_GPIO[i][1]);

    /*Init RC522 pin */
    RC522_SPI_MOSI_L(i);
    RC522_SPI_SCLK_H(i);
    RC522_SPI_NSS_H(i);
    RC522_RST_H(i);

    PcdReset(i);
    PcdAntennaOff(i);
    PcdAntennaOnIndex(i, 0);

    osDelay(5);

    PcdConfigISOType(i, 'A');
  }

}

