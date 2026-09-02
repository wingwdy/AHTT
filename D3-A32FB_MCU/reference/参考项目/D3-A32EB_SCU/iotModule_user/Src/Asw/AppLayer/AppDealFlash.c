/**********************************************************
  File name:       flash_cycle	          
  Author:                       
  Version:              
  Date:                    
  Description: 循环存储
  交易记录信息循环存储，起始地址和终止地址用来表示交易有效记录，不在此范围的表示已经成功上传并且被删除
  交易记录信息存储在第3扇区到flash结尾
  删除记录时更新起始地址即可
  起始地址和终止地址保存在第1、2扇区，由于可能存储比较频繁，所以也需要循环存储
  存储方式为8个字节一组，前4字节为数据，后4字节为地址（此地址相当于对数据的一个定位信息）
  Others:		                      
  Function List:	                
  History:
**********************************************************/
#include "AppDealFlash.h"
#include "AppStorage.h"

flash_info flash_g[GUN_NUM_MAX];
flash_data flash_d_g;

#define deal_flash_printf(fmt,args...)	\
		do {								\
            debug(fmt ,##args); 	\
            debug("\r\n");      			\
		} while(0)


/******************************************************************************
 *  uint8_t calc_checksum(uint8_t const *p_data, int32_t data_len)
 *
 * 计算校验和的算法，校验和占用1个字节。
 *
 * p_data  待校验数据的首地址
 * data_len  数据长度
 *
 * checksum, 0x00 ~ 0xFF
 */
uint8_t calc_checksum(uint8_t const *p_data, int32_t data_len)
{
    uint8_t sum = 0;
    while (data_len--) {
      sum += *p_data++;
    }
    return sum;
}

//ucStartAddr为存储基础信息时候的标识，data为地址信息，4字节，一共八字节，方便存储
static void Deal_Addr_Write(uint8_t u8Port, uint32_t ucddrCheck, uint32_t addr)
{
    flash_info *pFlash = &flash_g[u8Port];
    uint32_t addrOffset = u8Port * DEAL_ALL_SPACE;

  //地址不合法，返回
  if ((pFlash->c_addr > BASE_INFO_STOP_ADDR + addrOffset)
      || (pFlash->c_addr < BASE_INFO_START_ADDR + addrOffset)) {
        pFlash->c_addr = BASE_INFO_START_ADDR + addrOffset;
  }
  deal_flash_printf("AddrSta:0x%x 0x%x\r\n", pFlash->c_addr, addr);
  
  //根据地址判断是否需要删除哪一个扇区
  if (pFlash->c_addr == (BASE_INFO_START_ADDR + addrOffset)) {
    W25QXX_Erase_Sector(BASE_INFO_START_ADDR + addrOffset);
    deal_flash_printf("AddrStation Erease...\r\n");
  }

  //存储数据，4字节地址信息存储在4字节数据之后
  W25QXX_Write_safety((uint8_t *)&addr, pFlash->c_addr, 4);
  pFlash->c_addr += 4;
  //存储地址便于寻找
  W25QXX_Write_safety((uint8_t *)&ucddrCheck, pFlash->c_addr, 4);
  pFlash->c_addr += 4;

  if (pFlash->c_addr >= (BASE_INFO_STOP_ADDR + addrOffset)) {
    pFlash->c_addr = BASE_INFO_START_ADDR + addrOffset;
  }

}
#define IS_IN_RANGE(value, min, max) (((value) >= (min)) && ((value) < (max)))

//读取存储信息
static int Deal_Addr_Read(uint8_t u8Port, uint32_t ucddrCheck, uint32_t *Addr)
{
    flash_info *pFlash = &flash_g[u8Port];
    uint32_t addrOffset = u8Port * DEAL_ALL_SPACE;

    uint8_t addrStep = 8;
    uint32_t empty_addr = 0;
    //由于数据存储都是八个子节的前四个字节，后四个字节为存储地址
    uint32_t find_addr = 0;
    // 找寻地址范围
    uint32_t len_addr = (BASE_INFO_STOP_ADDR - BASE_INFO_START_ADDR) / addrStep;

    uint8_t abnmEnable = 0;
    uint32_t temp_addrData = 0;     //校验数据
    uint32_t temp_addr = 0;         //订单地址
    //第一扇区存储完成正在存储第二扇区或者刚开始存储第一扇区, flash_g.p_addr == 0
    for (int i = 0; i < len_addr; i++) {
        find_addr = BASE_INFO_START_ADDR + addrOffset + addrStep * i;

        W25QXX_Read((uint8_t *)&temp_addr , find_addr, 4);
        W25QXX_Read((uint8_t *)&temp_addrData, find_addr + 4, 4);

        // printf("Deal:0x%x 0x%x 0x%x\r\n", find_addr, temp_addr, temp_addrData);

        if ((temp_addr == 0xFFFFFFFF) && (temp_addrData == 0xFFFFFFFF)) {
            abnmEnable = 1;
            // break;
        } else if (temp_addrData == ucddrCheck) {
            abnmEnable = 0;
            pFlash->c_addr = find_addr;
            W25QXX_Read((uint8_t *)Addr, find_addr, 4);
        } else {
            printf("Deal addr erro 0x%x\r\n", temp_addrData);
            return -1;
        }

        if (i == (len_addr - 1)) 
        {
            //已经写满了
            if (abnmEnable == 0) 
            {
                printf("addr is full, start Init\r\n");
                return -1;
            }
        }
    }

    if (!IS_IN_RANGE(*Addr, START_ADDR + addrOffset, DEAL_INFO_STOP_ADDR + addrOffset)) {
        printf("DealInfoAddr range erro %d\r\n", *Addr);
        //不在正常范围内
        return -1;
    }
    if (!IS_IN_RANGE(pFlash->c_addr, BASE_INFO_START_ADDR + addrOffset, BASE_INFO_STOP_ADDR + addrOffset)) {
        printf("DealAddr range erro %d\r\n", pFlash->c_addr);
        //不在正常范围内
        return -1;
    }
    return 0;
}

//开机寻找交易地址存储区域的起止地址，方便联网上传未处理数据，顺便初始化交易记录存储包头0x15,0x12
static void _Start_Find_Deal_Addr(uint8_t u8Port)
{
    flash_info *pFlash = &flash_g[u8Port];
    uint32_t addrOffset = u8Port * DEAL_ALL_SPACE;

  flash_d_g.head[0] = 0x15;
  flash_d_g.head[1] = 0x12;

  //开机需要寻找到falsh存储交易记录地址信息区域找到目前存储到什么位置，有效区域开始位置
  uint32_t flash_addr;
  if (Deal_Addr_Read(u8Port, DEAL_ADDR_CHECK, &flash_addr) == 0) {
    pFlash->deal_stop_addr = flash_addr + DEAL_DATA_ALL_SIZE;
    pFlash->c_addr = pFlash->c_addr + 8;
  } else {
    pFlash->deal_stop_addr = START_ADDR + addrOffset;
    pFlash->c_addr = BASE_INFO_START_ADDR + addrOffset;
  }
  deal_flash_printf("Gun:%d\r\n", u8Port);
  deal_flash_printf("find NextAddrStation: 0x%x\r\n", pFlash->c_addr);
  deal_flash_printf("find Nextaddr: 0x%x\r\n", pFlash->deal_stop_addr);
}

void Start_Find_Deal_Addr()
{
    for (uint8_t i = 0; i < GUN_NUM_MAX; i++) {
        flash_info *pFlash = &flash_g[i];

        _Start_Find_Deal_Addr(i);
    }
}

//超过扇区会尽进行擦除
void DealData_Safety_write(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    flash_info *pFlash = &flash_g[u8Port];
    uint32_t addrOffset = u8Port * DEAL_ALL_SPACE;

    uint32_t currentAddr = pFlash->deal_stop_addr;
    uint8_t need_delete = 0;
    uint32_t data;
    //如果超过扇区，需要擦除下一个扇区
    if (!(currentAddr % SECTOR_SIZE)) {
      //一轮存储完成，新一轮存储
      if (currentAddr >= (DEAL_INFO_STOP_ADDR + addrOffset)) {
        currentAddr = START_ADDR + addrOffset;
        deal_flash_printf("GUN%d nextAddr > STOP_ADDR\r\n", u8Port);
      }
      W25QXX_Read((uint8_t *)&data, currentAddr, 4);
      if (data != 0xFFFFFFFF) {
        //扇区删除
        need_delete = 1;
      }
    }
    //删除block之后，如果未读交易记录位置在被删除的范围之内，则需要更新交易位置有效起始位
    if (need_delete) {
      //是否需要更新有效起始位
    __set_FAULTMASK(1);//关闭总中断
      // W25QXX_Erase_Sector(currentAddr / SECTOR_SIZE);
      W25QXX_Erase_Sector(currentAddr);
    __set_FAULTMASK(0);//开启总中断
      deal_flash_printf("GUN%d W25QXX_Erase_Sector: 0x%x  %d\r\n",u8Port, currentAddr, currentAddr / SECTOR_SIZE); 
    }
    deal_flash_printf("GUN%d DealWriterAddr: 0x%x\r\n",u8Port, currentAddr);
    W25QXX_Write_safety(pBuffer, currentAddr, NumByteToWrite);
    
    //存储flash_g.deal_stop_addr,开机可查询
    Deal_Addr_Write(u8Port, DEAL_ADDR_CHECK, currentAddr);
    //   deal_flash_printf("GUN%d DealData_write success\r\n", u8Port);

    //写完之后地址更新，以及需要存储，防止断电重启之后找不到
    pFlash->deal_stop_addr = currentAddr + DEAL_DATA_ALL_SIZE;
}


//循环进行存储，已经存储的页尽心标记，读取之后也尽心标记，相当于删除数据
//已经读取并删除的时候需要将起始地址更新
//存储数据包括头和定长数据，检测没有存储的page来存储，
void DealData_write(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
  if (NumByteToWrite > DEAL_DATA_VALID_SIZE) {
    NumByteToWrite = DEAL_DATA_VALID_SIZE;
  }
  flash_info *pFlash = &flash_g[u8Port];
  flash_data *frm = NULL;
  uint8_t send_buf[DEAL_DATA_ALL_SIZE] = {0};
  frm = (flash_data*)send_buf;
  frm->head[0] = flash_d_g.head[0];
  frm->head[1] = flash_d_g.head[1];
  uint16_t h_len = 3;   //2字节包头，加1字节校验
  uint16_t t_len = h_len + NumByteToWrite; //总长度
  //计算数据校验
  frm->check = calc_checksum(pBuffer, NumByteToWrite);
  memcpy(frm->data, pBuffer, NumByteToWrite);
  DealData_Safety_write(u8Port, (uint8_t *)frm, t_len); //数据写入完成之后存储地址
}


void DealData_Safety_Read(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite, int lastN)
{
    flash_info *pFlash = &flash_g[u8Port];
    uint32_t addrOffset = u8Port * DEAL_ALL_SPACE;

    //当前地址往前读取第lastN条
    uint32_t offset = lastN * DEAL_DATA_ALL_SIZE;
    uint32_t lastAddr = pFlash->deal_stop_addr - offset;
    //如果超过扇区，需要擦除下一个扇区
    if (lastAddr < (START_ADDR + addrOffset)) {
      lastAddr = (DEAL_INFO_STOP_ADDR + addrOffset) - offset;
    } 

    deal_flash_printf("GUN%d DealDataRead: 0x%x\r\n", u8Port, lastAddr);
    W25QXX_Read(pBuffer, lastAddr, NumByteToWrite);
}


int DealData_Read(uint8_t u8Port, uint8_t* pBuffer, uint16_t NumByteToWrite, int lastN)
{
  flash_data *frm = NULL;
  uint8_t recv_buf[DEAL_DATA_ALL_SIZE] = {0};
  frm = (flash_data*)recv_buf;
  //读取一定长度的信息,读取数据和包头校验
  DealData_Safety_Read(u8Port, (uint8_t *)frm, NumByteToWrite + DEAL_DATA_HEAD_SIZE, lastN);
  if ((frm->head[0] != flash_d_g.head[0]) || (frm->head[1] != flash_d_g.head[1])) {
    deal_flash_printf("flash head erro %d  %d\r\n", frm->head[0], frm->head[1]);
    return -1;
  }
  uint8_t check = calc_checksum(&frm->data[0], NumByteToWrite);
  if (check != frm->check) {
    deal_flash_printf("flash check erro %d  %d\r\n", check, pBuffer[2]);
    return -1;
  }
  memcpy(pBuffer, &frm->data[0], NumByteToWrite);
  deal_flash_printf("DealData_Read success\r\n");
  return 0;
}

void DealData_Clear(void)
{
	fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_BLOCK_ERASE_TYPE, BASE_INFO_START_ADDR);
	fgu8_AppInfoStoreEraseRoute(APP_STORE_MEDIUM_SPI_FLASH, E_APP_BLOCK_ERASE_TYPE, DEAL_INFO_STOP_ADDR);
}
