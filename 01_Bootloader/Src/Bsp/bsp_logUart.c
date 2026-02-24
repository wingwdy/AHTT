#include "bsp_logUart.h"
#include "bsp_delay.h"
#include <stdio.h>
#include "stdarg.h"



#define USART_GPIO_PORT 	GPIOA
#define USART_TxPin 		GPIO_PIN_11
#define USART_RxPin 		GPIO_PIN_12
#define USART_COM 			USART5

void printf_USART_Config()
{
	// USART_InitPara USART_InitStructure;

    /* 使能 USART 和相应的时钟 */
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

	rcu_periph_clock_enable(RCU_USART5);
	rcu_periph_clock_enable(RCU_GPIOA);

    /* 配置 USART 引脚为复用功能 */
    gpio_init(USART_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, USART_TxPin);
    gpio_init(USART_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USART_RxPin);

    gpio_afio_port_config(AFIO_PA11_USART5_CFG, ENABLE);
    gpio_afio_port_config(AFIO_PA12_USART5_CFG, ENABLE);

    /* 配置 USART 参数 */
	usart_deinit(USART_COM);
	usart_baudrate_set(USART_COM, 115200);
	usart_word_length_set(USART_COM, USART_WL_8BIT);
	usart_stop_bit_set(USART_COM, USART_STB_1BIT);
	usart_parity_config(USART_COM, USART_PM_NONE);
	usart_receive_config(USART_COM, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART_COM, USART_TRANSMIT_ENABLE);

    /* 使能 USART */
    usart_enable(USART_COM);
}
int fputc(int ch, FILE *f)
{
	usart_data_transmit(USART_COM, (uint8_t)ch);
	while (RESET == usart5_flag_get(USART_COM, USART5_FLAG_TBE)) {
		;
	}
	return ch;
}



//打印单个字符
void print_ch(const char ch)
{
//这里实现你的串口发送单个字符的函数
  // LPUART_WriteBlocking(LPUART0, (uint8_t *)&ch, 1);
}

//打印整数，不明白的可以网上查查，怎么回事，print_int()又调用了print_int()
void print_int(int dec)
{
    if(dec < 0)
    {
        print_ch('-');
        dec = -dec;
    }
    if(dec / 10)
        print_int(dec / 10);
    print_ch(dec%10 + '0');
}
//转换成十六进制

static void get_hex(uint8_t hex)
{
    const uint8_t ascii_zero = 48;
    const uint8_t ascii_a = 65;

    if ((hex >= 0) && (hex <= 9))
    {
        print_ch(hex + ascii_zero);
    }
    if ((hex >= 10) && (hex <= 15))
    {
        print_ch(hex - 10 + ascii_a);
    }
}
//以十六进制格式输出
void print_hex(uint32_t hex)
{
    if(hex / 16)
        print_hex(hex/16);
    get_hex(hex%16);
}

//打印字符串
void print_str(const char *ptr)
{
    while(*ptr)
    {
        print_ch(*ptr);
        ptr++;
    }
}

//打印浮点
void print_float(const float flt)
{
    int tmpint = (int)flt;
    int tmpflt = (int)(100000 * (flt - tmpint));
    if(tmpflt % 10 >= 5)
    {
        tmpflt = tmpflt / 10 + 1;
    }
    else
    {
        tmpflt = tmpflt / 10;
    }
    print_int(tmpint);
    print_ch('.');
    print_int(tmpflt);

}

//带格式打印，
void my_printf(const char *format,...)
{
    va_list ap;
    va_start(ap,format);
    while(*format)
    {
        if(*format != '%')
        {
            print_ch(*format);
            format++;
        }
        else
        {
            format++;
            switch(*format)
            {
            case 'c':
            {
                char valch = va_arg(ap,int);
                print_ch(valch);
                format++;
                break;
            }
            case 'd':
            {
                int valint = va_arg(ap,int);
                print_int(valint);
                format++;
                break;
            }
            case 's':
            {
                char *valstr = va_arg(ap,char *);
                print_str(valstr);
                format++;
                break;
            }
            case 'f':
            {
                float valflt = va_arg(ap,double);
                print_float(valflt);
                format++;
                break;
            }
            case 'x':
            case 'X':
            {
                int valhex = va_arg(ap,int);

                if(((uint32_t)valhex)<16)
                {
                    print_ch('0');
                }
                print_hex((uint32_t)valhex);
                format++;
                break;
            }
            default:
            {
                print_ch(*format);
                format++;
            }
            }
        }
    }
    va_end(ap);
}
