#ifndef	__GPIO_ROUTE_MANAGE_H_
#define	__GPIO_ROUTE_MANAGE_H_


/* Includes-----------------------------------------------------------------------------------*/
#include "FunctionalHeaderSummary.h"
#include "FuncExternHeaderSummary.h"

#define UNVALID_ALTERNATE_VALUE 	0xFF

typedef enum
{
    APP_FIRST_PWM = 0,
    APP_MAX_PWM_NUMBER = APP_FIRST_PWM,
}E_APP_PWM_LIST;

typedef enum
{
    APP_FIRST_DO = 0,
    APP_LED_RUN_DO = 0,
    // APP_BLE_RST_DO,
    APP_LTE_PWR_ENABLE_DO,
    APP_LTE_PWR_KEY_DO,
    APP_LTE_RESET_DO,
    
    //NFC1
    APP_NFC1_RST_DO,
    APP_NFC1_NSS_DO,
    APP_NFC1_CLK_DO,
    APP_NFC1_MOSI_DO,
    //NFC2
    APP_NFC2_RST_DO,
    APP_NFC2_NSS_DO,
    APP_NFC2_CLK_DO,
    APP_NFC2_MOSI_DO,

    APP_MAX_DO_NUMBER,
}E_APP_DO_LIST;

typedef enum
{
    APP_FIRST_DI = 0,
	APP_NET_STATUS_DI = 0,
    APP_LTE_STATUS_DI,
    APP_NET_MODE_DI,

    
    //NFC1
    APP_NFC1_IRQ_DI,
    APP_NFC1_MISO_DI,
    //NFC2
    APP_NFC2_IRQ_DI,
    APP_NFC2_MISO_DI,
    
	APP_SIM_STATUS_DI,

    APP_MAX_DI_NUMBER,
}E_APP_DI_LIST;

typedef enum
{
    E_DO_NOT_EXCUTE = 0,
    E_DO_EXCUTE,
}E_DO_EXCUTE_LIST;

typedef enum
{
    E_DI_EXCUTE = 0,
    E_DI_NOT_EXCUTE,
}E_DI_STATE_LIST;

typedef struct
{
    E_APP_DO_LIST           AppDoName;
    E_DIO_RESOURCE_MANAGE   PinNum;     //this name, meanwhile, is the name of index in the table of DoResourceTable
    E_BIT_VALUE_LIST        SetValue;
    E_BIT_VALUE_LIST        ReSetValue;
    E_BIT_VALUE_LIST        InitValue;
}STRU_DO_APP_FUNC_ROUTE_MANAGE;

typedef struct
{
    E_APP_DI_LIST           AppDiName;
    E_DIO_RESOURCE_MANAGE   PinNum;     //this name, meanwhile, is the name of index in the table of DoResourceTable
    E_BIT_VALUE_LIST        SetValue;
    E_BIT_VALUE_LIST        ReSetValue;
}STRU_DI_APP_FUNC_ROUTE_MANAGE;

typedef struct
{
    E_APP_PWM_LIST          AppPwmName;
    E_DIO_RESOURCE_MANAGE   PinNum;     //this name, meanwhile, is the name of index in the table of DoResourceTable
    uint8_t                 AlternateValue;
}STRU_PWM_APP_FUNC_ROUTE_MANAGE;

#ifdef __cplusplus
 extern "C" {
#endif
void fgv_AppGpioInit(void);
void fgv_DoWriteRoute(E_APP_DO_LIST AppIndex,uint8_t WriteValue);
uint8_t fgu1_DoReadRoute(E_APP_DO_LIST AppDoIndex);
uint8_t fgu1_DiReadRoute(E_APP_DI_LIST AppDiIndex);

#ifdef __cplusplus
}
#endif

#endif /*__GPIO_ROUTE_MANAGE_H_*/
