#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include <stdint.h>
#include "mbsDataUpdate.h"

//软件版本号，最大99，按照打通平台递增，第一个基本不改动，第二个按照平台递增，第三个功能迭代增加bug修复，第四个为测试list，每个增加后面清零
#define HARDWARE_VERSION    "1.0.0.1"
#define SOFTWARE_VERSION    "1.7.0.0"
#define MBSPROTOCOL_VERSION "1.0.0.5"
//两个协议版本同步更改
#define PROTOCOL_VERSION    "1.0.0.5"
#define PROTOCOL_VERSION_UP    10005        //版本域,tcp协议版本

/*枪的个数*/
#define GUN_NUM   (fgv_GetPileCfgGunNum())

//单枪标志，为1时需要处理单枪为B枪的所有东西
#define SINGLE_GUN_REV   ((GUN_NUM == 1) ? 1 : 0)

//FTP部分
#define FTP_FILDER_PATH   "/AC_pile/D3-A32EB/"
#define FTP_USER_NAME     "gn_ftp_fw_reader"
#define FTP_USER_PSW      "d2aa28ee9a8693db"
#define FTP_USER_IP       "fw.ftp.gongniu.cn"
#define FTP_USER_PORT      21

#define FTP_FILENAME      "D3-A32EB"        //升级ftp文件从0开始

//有屏无屏不做区分，因为没有可靠信号来判断有屏无屏
#define DEVICE_MODEL_CODE_ONE      "D3-A32EB"         //单枪型号
#define DEVICE_MODEL_CODE_TWO      "D3-A32EB2"        //双枪型号


#define UPDATE_TIMEOUT_MS   (10*eTick_60S)     //升级超时时间，文件太大，加长到十分钟；单位ms

#define DEAL_RECORD_MAXLEN   1024               //账单存储长度

// #define FTP_FILENAME      "maxy503Test.bin"

//GD25Q80EEIGR flash大小1M
//擦除单位4K
//存储参数用64K；预留16个sector用来存储所需参数
//升级需要用到512K；前256备份区，后256升级数据区域
//预留64K
//预留128K
//后面256K用来存储充电记录，循环存储

#define EXT_FLASH_PAGE_SIZE    			    0x00000100 //256B
#define EXT_FLASH_SECTOR_SIZE   		    0x00001000 //4K
#define EXT_FLASH_BLOCK_SIZE   		    	0x00010000 //64K

//OTA INFO
#define EXT_FLASH_ADDR_OTA_INFO             0x00000000 //外部flash,升级信息存放地址

//OTA FILE
#define EXT_FLASH_ADDR_OTA_DOWNLOAD	            0x00010000	//外部flash,升级下载区起始地址
#define EXT_FLASH_LEN_OTA_FILESIZE	            0x00070000	//外部flash,升级下载区大小


//参数存储addr
//参数等数据存储于前16个扇区，升级参数存储于参数存储的最后一个扇区，其他在一开始的位置，整个数据参数存储占用64k
#define EXT_FLASH_OTA_PARAM_ADDR 			((uint32_t)0xF000)//升级参数存储区域
#define EXT_FLASH_PLAT_PARAM_ADDR 			((uint32_t)0x0000)//平台参数存储区域
#define EXT_FLASH_RATE_MODEL_ADDR 			((uint32_t)0x1000)//计费模型存储区域
#define EXT_FLASH_QRCODE_ADDR 			    ((uint32_t)0x2000)//二维码存储区域
#define EXT_FLASH_EEOP_ADDR 			    ((uint32_t)0x4000)//各个平台需要的参数存储，可以复用，切换平台需要清零
#define EXT_FLASH_ELSE_PARAM_ADDR 			((uint32_t)0x5000)//其他从此地址开始


//
#define EXT_FLASH_ADDR_SETDATA_PARAM        0x00080000 //配置参数起始地址
#define EXT_FLASH_LEN_SETDATA_PARAM         0x00001000
#define EXT_FLASH_ADDR_PLAT_PARAM        	0x00081000 //平台参数起始地址
#define EXT_FLASH_LEN_PLAT_PARAM         	0x00001000

#define EXT_FLASH_ADDR_SETDATA_BACKUP       0x00081000 //配置参数起始地址（备份）
#define EXT_FLASH_LEN_SETDATA_PARAM         0x00001000

//计量0x00082000~0x00087000

//
#define EXT_FLASH_ADDR_HIS_LOG	            0x00088000 //历史数据
#define EXT_FLASH_LEN_HIS_LOG	            0x00040000

typedef struct
{
	unsigned int W:11;//应用版本号0-999
	unsigned int Z:7;//修订号0-99
	unsigned int Y:7;//次版本号0-99
	unsigned int X:7;//主版本号0-99
}bfVersion;

//设备信息结构定义
typedef struct
{
	uint32_t	Head[2];		//消息头(产品)	            0x01234567	0x89ABCDEF
	uint32_t	Ver;			//消息版本			        0x00000002
	uint32_t	Len;			//消息长度			        0x0000002C
	uint32_t	ModNum;			//设备编码(软件料号)	     0x00000000
	char	    ModName[32];	//设备名称		            "bull.acpile.d3-d32a.scu"//厂商标识.产品名称.产品型号.模块名称
    bfVersion   HardVer;		//硬件版本			        {1,0,0,0}
	bfVersion   SoftVer;		//软件版本			        {1,0,0,1}
	uint32_t	ChkSum;			//校验和		            0x00000000
} stuDevInfo;

extern const stuDevInfo gc_DevInfo;

//所有线程优先级枚举
enum eTaskPriority
{
    //数字越小优先级越低
    PRIO_USED_MIN = 8,
    PRIO_IMPLOG,
    PRIO_INIT = PRIO_USED_MIN,
    PRIO_DETECT,    //检测各线程需要使用的栈大小
    PRIO_CARD,
    PRIO_RUN_LOG,
    PRIO_EVENT,
    PRIO_NET_COMM,
    PRIO_IOT_COMM,
	PRIO_CCU_COMM_SND,
    PRIO_CCU_COMM_RCV,
    PRIO_RGB_LED,
    PRIO_SCREEN,
    PRIO_PROTOCOL,
    PRIO_LCD_COMM,
    PRIO_CONTROL,
    PRIO_IWDG,
};


#ifdef __cplusplus
extern "C"
{
#endif

void main_var_init();

void Reboot_System(uint8_t n);


#ifdef __cplusplus
}
#endif

#endif
