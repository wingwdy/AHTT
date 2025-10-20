
/******************************************************************************
* File Name          : Mcal_Port_Config.h
* Description        : Code for Pin-level configuration module for hardware
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      Chenls    初版创建
*
******************************************************************************/

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Port.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define  McalPort_CFG_PinLISTArray     \
{\
    { eMcalPortPinChanel_PA0_ENTC,            RCU_GPIOA,  GPIOA,  GPIO_MODE_AIN,          GPIO_OSPEED_50MHZ,  GPIO_PIN_0,   RESET  },\
    { eMcalPortPinChanel_PA1_RunLed,          RCU_GPIOA,  GPIOA,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_1,   RESET  },\
    { eMcalPortPinChanel_PA2_BL0942Tx,        RCU_GPIOA,  GPIOA,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_2,   RESET  },\
    { eMcalPortPinChanel_PA3_BL0942Rx,        RCU_GPIOA,  GPIOA,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_3,   RESET  },\
    { eMcalPortPinChanel_PA4_Reserve,         RCU_GPIOA,  GPIOA,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_4,   RESET  },\
    { eMcalPortPinChanel_PA5_CPADC,           RCU_GPIOA,  GPIOA,  GPIO_MODE_AIN,          GPIO_OSPEED_50MHZ,  GPIO_PIN_5,   RESET  },\
    { eMcalPortPinChanel_PA6_CPPWMOut,        RCU_GPIOA,  GPIOA,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_6,   RESET  },\
    { eMcalPortPinChanel_PA7_GunNtc,          RCU_GPIOA,  GPIOA,  GPIO_MODE_AIN,          GPIO_OSPEED_50MHZ,  GPIO_PIN_7,   RESET  },\
    { eMcalPortPinChanel_PA8_ShortCircuitEn,  RCU_GPIOA,  GPIOA,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_8,   RESET  },\
    { eMcalPortPinChanel_PA9_CaliMeterTx,     RCU_GPIOA,  GPIOA,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_9,   RESET  },\
    { eMcalPortPinChanel_PA10_CaliMeterRx,    RCU_GPIOA,  GPIOA,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_10,  RESET  },\
    { eMcalPortPinChanel_PA11_DebugTx,        RCU_GPIOA,  GPIOA,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_11,  RESET  },\
    { eMcalPortPinChanel_PA12_DebugRx,        RCU_GPIOA,  GPIOA,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_12,  RESET  },\
    { eMcalPortPinChanel_PA15_Reserve,        RCU_GPIOA,  GPIOA,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_15,  RESET  },\
    \
    { eMcalPortPinChanel_PB0_RelayEn,         RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_0,   RESET  },\
    { eMcalPortPinChanel_PB1_RCDTrip,         RCU_GPIOB,  GPIOB,  GPIO_MODE_IPU,          GPIO_OSPEED_50MHZ,  GPIO_PIN_1,   RESET  },\
    { eMcalPortPinChanel_PB2_RCDZero,         RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_2,   RESET  },\
    { eMcalPortPinChanel_PB3_NorFlashSpiSCK,  RCU_GPIOB,  GPIOB,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_3,   RESET  },\
    { eMcalPortPinChanel_PB4_NorFlashSpiMISO, RCU_GPIOB,  GPIOB,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_4,   RESET  },\
    { eMcalPortPinChanel_PB5_NorFlashSpiMOSI, RCU_GPIOB,  GPIOB,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_5,   RESET  },\
    { eMcalPortPinChanel_PB6_Reserve,         RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_6,   RESET  },\
    { eMcalPortPinChanel_PB7_Reserve,         RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_7,   SET    },\
    { eMcalPortPinChanel_PB8_NorFlashSpiCS,   RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_8,   RESET  },\
    { eMcalPortPinChanel_PB9_LTEReset,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_9,   SET    },\
    { eMcalPortPinChanel_PB10_RCDRms,         RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_10,  RESET  },\
    { eMcalPortPinChanel_PB11_RCDTest,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_11,  RESET  },\
    { eMcalPortPinChanel_PB12_Reserve,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_12,  RESET  },\
    { eMcalPortPinChanel_PB13_Reserve,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_13,  RESET  },\
    { eMcalPortPinChanel_PB14_Reserve,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_14,  RESET  },\
    { eMcalPortPinChanel_PB15_Reserve,        RCU_GPIOB,  GPIOB,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_15,  RESET  },\
    \
    { eMcalPortPinChanel_PC0_FCTPin,          RCU_GPIOC,  GPIOC,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_0,   RESET  },\
    { eMcalPortPinChanel_PC1_Reserve,         RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_1,   RESET  },\
    { eMcalPortPinChanel_PC2_SC_CHK,          RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_2,   RESET  },\
    { eMcalPortPinChanel_PC3_Reserve,         RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_3,   RESET  },\
    { eMcalPortPinChanel_PC4_Reserve,         RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_4,   RESET  },\
    { eMcalPortPinChanel_PC5_PEIn,            RCU_GPIOC,  GPIOC,  GPIO_MODE_AIN,          GPIO_OSPEED_50MHZ,  GPIO_PIN_5,   RESET  },\
    { eMcalPortPinChanel_PC6_RelayAdhesion,   RCU_GPIOC,  GPIOC,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_6,   RESET  },\
    { eMcalPortPinChanel_PC7_OutBack1,        RCU_GPIOC,  GPIOC,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_7,   RESET  },\
    { eMcalPortPinChanel_PC8_ICLed,           RCU_GPIOC,  GPIOC,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_8,   RESET  },\
    { eMcalPortPinChanel_PC9_CaliMeterEn,     RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_9,   RESET  },\
    { eMcalPortPinChanel_PC10_NFCReset,       RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_10,  SET    },\
    { eMcalPortPinChanel_PC11_NFCCS,          RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_11,  SET    },\
    { eMcalPortPinChanel_PC12_4GTX,           RCU_GPIOC,  GPIOC,  GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  GPIO_PIN_12,  RESET  },\
    { eMcalPortPinChanel_PC13_Reserve,        RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_13,  RESET  },\
    { eMcalPortPinChanel_PC14_4GPwrKeyEn,     RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_14,  SET    },\
    { eMcalPortPinChanel_PC15_4GPwrEn,        RCU_GPIOC,  GPIOC,  GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  GPIO_PIN_14,  SET    },\
    \
    { eMcalPortPinChanel_PD2_4GRX,            RCU_GPIOD,  GPIOD,  GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  GPIO_PIN_2,   RESET  },\
}

/******************************************************************************
*    Enum Definition
******************************************************************************/


/******************************************************************************
*    Typedef Definition
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
























