/******************************************************************************
* File Name          : Mcal_Port.h
* Description        : Code for Pin-level configuration module for hardware
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
******************************************************************************/
#ifndef MCAL_PORT_H_
#define MCAL_PORT_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h" 

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define MCALPORT_PIN_HIGH                           SET
#define MCALPORT_PIN_LOW                            RESET
/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eMcalPortPinChanel_PA0_ENTC,                    /* 环境温度检测 */             
    eMcalPortPinChanel_PA1_RunLed,                  /* 运行灯控制 */ 
    eMcalPortPinChanel_PA2_BL0942Tx,                /* 计量芯片0942串口发送 */ 
    eMcalPortPinChanel_PA3_BL0942Rx,                /* 计量芯片0942串口接收 */ 
    eMcalPortPinChanel_PA4_Reserve,                 /* 预留 */ 
    eMcalPortPinChanel_PA5_CPADC,                   /* CP电压采样 */ 
    eMcalPortPinChanel_PA6_CPPWMOut,                /* CP PWM输出 */ 
    eMcalPortPinChanel_PA7_GunNtc,                  /* 枪温采样 */ 
    eMcalPortPinChanel_PA8_ShortCutEn,              /* 短路检测使能 */ 
    eMcalPortPinChanel_PA9_CaliMeterTx,             /* 校表485口，发送 */
    eMcalPortPinChanel_PA10_CaliMeterRx,            /* 校表485口，接收 */
    eMcalPortPinChanel_PA11_DebugTx,                /* 调试口，发送 */
    eMcalPortPinChanel_PA12_DebugRx,                /* 调试口，接收 */
    eMcalPortPinChanel_PA15_Reserve,                /* 预留 */ 

    eMcalPortPinChanel_PB0_RelayEn,                 /* 继电器控制口 */
    eMcalPortPinChanel_PB1_RCDTrip,                 /* RCD trip 漏电反馈 */
    eMcalPortPinChanel_PB2_RCDZero,                 /* RCD Zero 校0口 */
    eMcalPortPinChanel_PB3_NorFlashSpiSCK,          /* NorFlash的spi时钟线 */
    eMcalPortPinChanel_PB4_NorFlashSpiMISO,         /* NorFlash的spi接收*/  
    eMcalPortPinChanel_PB5_NorFlashSpiMOSI,         /* NorFlash的spi发送*/  
    eMcalPortPinChanel_PB6_Reserve,                 /* 预留 */ 
    eMcalPortPinChanel_PB7_Reserve,                 /* 预留 */ 
    eMcalPortPinChanel_PB8_NorFlashSpiCS,           /* NorFlash的spi片选教CS */
    eMcalPortPinChanel_PB9_4GReset,                 /* 4G模组的复位脚 */  
    eMcalPortPinChanel_PB10_RCDRms,                 /* RCD RMS */  
    eMcalPortPinChanel_PB11_RCDTest,                /* RCD TEST */ 
    eMcalPortPinChanel_PB12_Reserve,                /* 预留 */ 
    eMcalPortPinChanel_PB13_Reserve,                /* 预留 */ 
    eMcalPortPinChanel_PB14_Reserve,                /* 预留 */ 
    eMcalPortPinChanel_PB15_Reserve,                /* 预留 */ 

    eMcalPortPinChanel_PC0_FCTPin,                  /* FCT测试脚 */ 
    eMcalPortPinChanel_PC1_Reserve,                 /* 预留 */ 
    eMcalPortPinChanel_PC2_SC_CHK,                  /* 短路检测反馈 */
    eMcalPortPinChanel_PC3_Reserve,                 /* 预留 */
    eMcalPortPinChanel_PC4_Reserve,                 /* 预留 */
    eMcalPortPinChanel_PC5_PEIn,                    /* PE反馈 */
    eMcalPortPinChanel_PC6_RelayAdhesion,           /* 继电器粘连检测 */
    eMcalPortPinChanel_PC7_OutBack1,                /* 继电器反馈信号，用于误动拒动检测 */
    eMcalPortPinChanel_PC8_ICLed,                   /* 灯板控制，单总线 */   
    eMcalPortPinChanel_PC9_CaliMeterEn,             /* 校表485口，驱动口 */
    eMcalPortPinChanel_PC10_NFCReset,               /* NFC复位脚 */
    eMcalPortPinChanel_PC11_NFCCS,                  /* NFC片选脚 */
    eMcalPortPinChanel_PC12_4GTX,                   /* 4G发送脚 */
    eMcalPortPinChanel_PC13_Reserve,                /* 预留 */
    eMcalPortPinChanel_PC14_4GPwrKeyEn,             /* 4G开机脚 */
    eMcalPortPinChanel_PC15_4GPwrEn,                /* 4G电源控制 */
    
    eMcalPortPinChanel_PD2_4GRX,                    /* 4G接收脚 */
    eMcalPortPinChanel_Count,
}McalPortPinChanel_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void McalPort_Init(void);
void McalPort_ResetPin(McalPortPinChanel_Enum ePinChannel);
void McalPort_SetPin(McalPortPinChanel_Enum ePinChannel);
uint8_t McalPort_GetPin(McalPortPinChanel_Enum ePinChannel);
void McalPort_WritePin(McalPortPinChanel_Enum ePinChannel, uint8_t pinVal);
void MalPort_TogglePin(McalPortPinChanel_Enum ePinChannel);
#endif /* MCAL_PORT_H_ */


















