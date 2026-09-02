#ifndef __APP_DEALFLASH_H__
#define __APP_DEALFLASH_H__

#include "AppHeaderSummary.h"

/*****************************************************************************
 * flash存储交易记录基本信息
 * 分区第0个block给存储一些不常改变的信息，1个block
 * 分区第1-8个block给存储交易记录的起止地址，7个block
 * 剩余block存储交易记录大数据组，120个block
*****************************************************************************/
#define DEAL_DATA_HEAD_SIZE       3     //交易记录数据长度
#define DEAL_DATA_ALL_SIZE         DEAL_RECORD_MAXLEN    //每次存储字节占用
#define DEAL_DATA_VALID_SIZE      (DEAL_RECORD_MAXLEN - DEAL_DATA_HEAD_SIZE)

//每把枪的存储空间，单位K
#define DEAL_ALL_SPACE            (64 * 1024)

//前两个sector开始存储大数据的一些信息，擦除最小单位为扇区擦除
//单枪占用128K，8k起止地址存储；120k账单存储；多枪往后添加
#define BASE_INFO_START_ADDR      0xD0000
#define BASE_INFO_STOP_ADDR       0xD1000
#define DEAL_INFO_STOP_ADDR       (BASE_INFO_START_ADDR + DEAL_ALL_SPACE)

//从第3个sector开始存储大数据，1sector = 4k数据
#define START_ADDR    BASE_INFO_STOP_ADDR

#define DEAL_ADDR_CHECK    0x87654321   //存储交易记录当前正在存储位置时加简单校验

typedef struct _flash_data{
  uint8_t head[2];
  uint8_t check;          //校验和,仅包括数据部分
  uint8_t data[1];      //数据
} flash_data;
typedef struct _flash_info{
  //存储基础信息的起始位置，可以在此段flash中寻找需要的数据
  uint32_t p_addr;    //基础信息起始位置
  uint32_t c_addr;    //当前使用到这个地址，写的时候直接从此地址写入，开机找寻此地址
  uint32_t deal_start_addr;    //交易记录存储的有效位置起始值
  uint32_t deal_stop_addr;    //交易记录存储的有效位置终止值，也就是当前存储的位置
  uint32_t deal_next_addr;    //最后一个存储的地方
  uint32_t unread_num;    //未处理的条数
} flash_info;


void Start_Find_Deal_Addr(void);
void DealData_write(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite);
int DealData_Read(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite, int lastN);
void DealData_Clear(void);

#endif



