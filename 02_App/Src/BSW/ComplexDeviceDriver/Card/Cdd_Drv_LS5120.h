/******************************************************************************
* File Name          : Cdd_Drv_BL0942.h
* Description        : Code for Code for the device driver for BL0942
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
******************************************************************************/
#ifndef CDD_DRV_LS5120_H_
#define CDD_DRV_LS5120_H_


/******************************************************************************
*    Include
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD_DRV_LS5120_CHIP_VER 0x82

#define PCD_IDLE                0x00 /*取消当前命令*/
#define PCD_AUTHENT             0x0E /*验证密钥*/
#define PCD_RECEIVE             0x08 /*接收数据*/
#define PCD_TRANSMIT            0x04 /*发送数据*/
#define PCD_TRANSCEIVE          0x0C /*发送并接收数据*/
#define PCD_RESETPHASE          0x0F /*复位*/
#define PCD_CALCCRC             0x03 /*CRC计算*/

#define PICC_REQIDL             0x26 /*寻天线区内未进入休眠状态*/
#define PICC_REQALL             0x52 /*寻天线区内全部卡*/
#define PICC_ANTICOLL1          0x93 /*防冲撞1*/
#define PICC_ANTICOLL2          0x95 /*防冲撞2*/
#define PICC_AUTHENT1A          0x60 /*验证A密钥*/
#define PICC_AUTHENT1B          0x61 /*验证B密钥*/
#define PICC_READ               0x30 /*读块*/
#define PICC_WRITE              0xA0 /*写块*/
#define PICC_DECREMENT          0xC0 /*扣款*/
#define PICC_INCREMENT          0xC1 /*充值*/
#define PICC_RESTORE            0xC2 /*调块数据到缓冲区*/
#define PICC_TRANSFER           0xB0 /*保存缓冲区中数据*/
#define PICC_HALT               0x50 /*休眠*/

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Header File Inclusion
******************************************************************************/


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MAX_RECV_LEN            18
#define MAX_FIFO_LEN            64

/* 寄存器定义 */
/* Page0 指令和状态 */
#define Reserved00              0x00    /*保留*/
#define CommandReg              0x01    /*启动和停止指令*/
#define ComIEnReg               0x02    /*使能和禁用中断请求控制位*/
#define DivlEnReg               0x03    /*使能和禁用中断请求控制位*/
#define ComIrqReg               0x04    /*中断请求位*/
#define DivIrqReg               0x05    /*中断请求位*/
#define ErrorReg                0x06    /*显示上一个指令执行的错误状态*/
#define Status1Reg              0x07    /*通信状态位*/
#define Status2Reg              0x08    /*接收器和发送器状态位*/
#define FIFODataReg             0x09    /*FIFO缓冲区输入和输出*/
#define FIFOLevelReg            0x0A    /*FIFO缓冲区已存储字节的数量*/
#define WaterLevelReg           0x0B    /*FIFO缓冲区溢出和空警告*/
#define ControlReg              0x0C    /*各种控制寄存器*/
#define BitFramingReg           0x0D    /*面向位的帧的调节*/
#define CollReg                 0x0E    /*检查产生位冲突的第一个位的地址*/
#define Reserved0F              0x0F    /*保留*/

/* Page1 指令*/
#define Reserved10              0x10    /*保留*/
#define ModeReg                 0x11    /*定义发送和接收通用模式的设置*/
#define TxModeReg               0x12    /*定义发送过程的数据传输速率和结构*/
#define RxModeReg               0x13    /*定义接收过程中的数据传输速率和结构*/
#define TxControlReg            0x14    /*控制天线驱动器管脚TX1和TX2的逻辑特性*/
#define TxASKReg                0x15    /*控制发送调整的设置*/
#define TxSelReg                0x16    /*选择天线驱动器的内部信号源*/
#define RxSelReg                0x17    /*选择内部接收器的设置*/
#define RxThresholdReg          0x18    /*选择位解码器的阈值*/
#define DemodReg                0x19    /*定义解调器的设置*/
#define Reserved1A              0x1A    /*保留*/
#define Reserved1B              0x1B    /*保留*/
#define TxWaitReg               0x1C    /*控制通信时发送的等待时间*/
#define ParityReg               0x1D    /*设置奇偶校验位*/
#define Reserved1E              0x1E    /*保留*/
#define SerialSpeedReg          0x1F    /*选择串行UART接口的速率*/

/* Page2 配置 */
#define Reserved20              0x20    /*保留*/
#define CRCResultRegMSB         0x21    /*显示CRC计算的MSB值*/
#define CRCResultRegLSB         0x22    /*显示CRC计算的LSB值*/
#define Reserved23              0x23    /*保留*/
#define ModWidthReg             0x24    /*控制调制宽度的设置*/
#define Reserved25              0x25    /*保留*/
#define RFCfgReg                0x26    /*接收器增益的配置*/
#define GsNReg                  0x27    /*选择天线驱动器管脚TX1和TX2的调整电导*/
#define CWGsPReg                0x28    /*定义p-driver无调制的输出电导*/
#define ModGsPReg               0x29    /*定义p-driver经过调制的输出电导*/
#define TModeReg                0x2A    /*内部定时器的设置*/
#define TPrescalerReg           0x2B    /*内部定时器的设置*/
#define TReloadRegMSB           0x2C    /*定义16位定时器的重载值*/
#define TReloadRegLSB           0x2D    /*定义16位定时器的重载值*/
#define TCounterValRegMSB       0x2E    /*显示16定时器的当前值*/
#define TCounterValRegLSB       0x2F    /*显示16定时器的当前值*/

/* Page3 测试寄存器 */
#define Reserved30              0x30    /*保留*/
#define TestSel1Reg             0x31    /*通用测试信号的配置*/
#define TestSel2Reg             0x32    /*通用测试信号的配置和PRBS控制*/
#define TestPinEnReg            0x33    /*使能D1-D7的输出驱动器*/
#define TestPinValueReg         0x34    /*定义管脚D1-D7用作I/O总线时的值*/
#define TestBusReg              0x35    /*显示内部测试总线的状态*/
#define AutoTestReg             0x36    /*控制数字自检*/
#define VersionReg              0x37    /*显示软件版本*/
#define AnalogTestReg           0x38    /*控制管脚AUX1和AUX2*/
#define TestDAC1Reg             0x39    /*定义TestDAC1的测试值*/
#define TestDAC2Reg             0x3A    /*定义TestDAC2的测试值*/
#define TestADCReg              0x3B    /*显示ADC中I和Q通道的值*/
#define Reserved3C              0x3C    /*保留*/
#define Reserved3D              0x3D    /*保留*/
#define Reserved3E              0x3E    /*保留*/
#define Reserved3F              0x3F    /*保留*/

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
extern uint8_t CddDrvLS5120_PcdAuthKeyA(uint8_t addr, const uint8_t *pKey, const uint8_t *pUid);
extern uint8_t CddDrvLS5120_PcdReadSector(uint8_t addr, uint8_t *pDataOut);
extern uint8_t CddDrvLS5120_PcdRequest(uint8_t reqCode, uint8_t *pTagType);
extern uint8_t CddDrvLS5120_PcdAntiCollision(uint8_t *pOutUid, uint8_t *pUidLenOut);
extern uint8_t CddDrvLS5120_PcdSelect(uint8_t *pUid);
extern void CddDrvLS5120_Init(void);
extern void CddDrvLS5120_SoftwareReset(void);
extern void CddDrvLS5120_HardwareResetStart(void);
extern void CddDrvLS5120_HardwareResetEnd(void);
extern uint8_t CddDrvLS5120_VersionCheck(void);

#endif /* CDD_DRV_LS5120_H_ */

