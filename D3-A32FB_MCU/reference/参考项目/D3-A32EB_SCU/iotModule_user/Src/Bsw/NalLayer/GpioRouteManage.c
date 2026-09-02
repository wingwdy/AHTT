#include "GpioRouteManage.h"

STRU_DO_APP_FUNC_ROUTE_MANAGE DoAppRouteTable[APP_MAX_DO_NUMBER] =
{
	{
        .AppDoName = APP_LED_RUN_DO,
        .PinNum = E_DIO_PIN44,
        .SetValue = E_SET_VALUE_HIGH,
        .ReSetValue = E_SET_VALUE_LOW,
        .InitValue = E_SET_VALUE_LOW 
    },
    // {
    //     .AppDoName = APP_BLE_RST_DO,
    //     .PinNum = E_DIO_PIN34,
    //     .SetValue = E_SET_VALUE_HIGH,
    //     .ReSetValue = E_SET_VALUE_LOW,
    //     .InitValue = E_SET_VALUE_LOW
    // },
    {
        .AppDoName = APP_LTE_PWR_ENABLE_DO,
        .PinNum = E_DIO_PIN15,
        .SetValue = E_SET_VALUE_HIGH,
        .ReSetValue = E_SET_VALUE_LOW,
        .InitValue = E_SET_VALUE_LOW
    },
    { .AppDoName = APP_LTE_PWR_KEY_DO, .PinNum = E_DIO_PIN40,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_LTE_RESET_DO,    .PinNum = E_DIO_PIN39,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },

    
    //NFC1
    { .AppDoName = APP_NFC1_RST_DO,    .PinNum = E_DIO_PIN37,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_NFC1_NSS_DO,    .PinNum = E_DIO_PIN33,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_NFC1_CLK_DO,    .PinNum = E_DIO_PIN34,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
   { .AppDoName = APP_NFC1_MOSI_DO,    .PinNum = E_DIO_PIN36,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    //NFC2
    { .AppDoName = APP_NFC2_RST_DO,    .PinNum = E_DIO_PIN2,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_NFC2_NSS_DO,    .PinNum = E_DIO_PIN62,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_NFC2_CLK_DO,    .PinNum = E_DIO_PIN61,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },
    { .AppDoName = APP_NFC2_MOSI_DO,    .PinNum = E_DIO_PIN58,    .SetValue = E_SET_VALUE_HIGH,  .ReSetValue = E_SET_VALUE_LOW,  .InitValue = E_SET_VALUE_LOW },

};

STRU_DI_APP_FUNC_ROUTE_MANAGE DiAppRouteTable[APP_MAX_DI_NUMBER] =
{
    { APP_NET_STATUS_DI,      E_DIO_PIN28,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },
    { APP_LTE_STATUS_DI,      E_DIO_PIN29,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },
	{ APP_NET_MODE_DI,        E_DIO_PIN27,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },
    
	{ APP_NFC1_IRQ_DI,        E_DIO_PIN38,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },
	{ APP_NFC1_MISO_DI,        E_DIO_PIN35,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },

	{ APP_NFC2_IRQ_DI,        E_DIO_PIN3,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },
	{ APP_NFC2_MISO_DI,        E_DIO_PIN59,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },

    { APP_SIM_STATUS_DI,      E_DIO_PIN38,  E_SET_VALUE_HIGH,  E_SET_VALUE_LOW },

};

//
void fgv_AppGpioInit(void)
{
    E_APP_DO_LIST   GpioDoIndex = APP_FIRST_DO;
    E_APP_DI_LIST   GpioDiIndex = APP_FIRST_DI;

    /* 使能AFIO时钟,禁用JTAG RST功能 */
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    for(GpioDoIndex = APP_FIRST_DO; GpioDoIndex < APP_MAX_DO_NUMBER; GpioDoIndex++)
    {
        fgv_GpioFuncInit(DoAppRouteTable[GpioDoIndex].PinNum, DIGITAL_OUTPUT_TYPE, DoAppRouteTable[GpioDoIndex].InitValue, UNVALID_ALTERNATE_VALUE);
    }
	
    for(GpioDiIndex = APP_FIRST_DI; GpioDiIndex < APP_MAX_DI_NUMBER; GpioDiIndex++)
    {
        fgv_GpioFuncInit(DiAppRouteTable[GpioDiIndex].PinNum, DIGITAL_INPUT_TYPE, E_SET_VALUE_LOW, UNVALID_ALTERNATE_VALUE);
    }
}

void fgv_DoWriteRoute(E_APP_DO_LIST AppIndex, uint8_t WriteValue)
{
    if(SET == WriteValue)
    {
        fgv_DoFuncValueWrite(DoAppRouteTable[AppIndex].PinNum,DoAppRouteTable[AppIndex].SetValue);
    }else
    {
        fgv_DoFuncValueWrite(DoAppRouteTable[AppIndex].PinNum,DoAppRouteTable[AppIndex].ReSetValue);
    }
}

uint8_t fgu1_DoReadRoute(E_APP_DO_LIST AppDoIndex)
{
    if(fgu1_DoFuncValueRead(DoAppRouteTable[AppDoIndex].PinNum) == DoAppRouteTable[AppDoIndex].SetValue)
    {
        return SET;
    }else
    {
        return RESET;
    }
}

uint8_t fgu1_DiReadRoute(E_APP_DI_LIST AppDiIndex)
{
    if(fgu1_DiFuncValueRead(DiAppRouteTable[AppDiIndex].PinNum) == DiAppRouteTable[AppDiIndex].SetValue)
    {
        return SET;
    }else
    {
        return RESET;
    }
}
