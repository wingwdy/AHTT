#ifndef	__RESOURCE_SUMMARY_DESCRIBE_H_
#define	__RESOURCE_SUMMARY_DESCRIBE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "string.h"


#if defined(GD32E230)
#include "gd32e23x.h"
#elif defined(GD32E50X)
#include "gd32e50x.h"
#endif

#if defined(GD32E230)
#define SPI_TOTAL_TABLE_NUMBER      2
#define UART_TOTAL_TABLE_NUMBER     2
#elif defined(GD32E50X)
#define SPI_TOTAL_TABLE_NUMBER      3
#define UART_TOTAL_TABLE_NUMBER     6
#endif

typedef enum
{
    E_DIO_FIRST_INDEX = 0,
#if defined(GD32E230)//GD32E230Cx

#elif defined (GD32E50X)//GD32E503Rx
    // E_DIO_PIN1,
    E_DIO_PIN2 = 0,
    E_DIO_PIN3,
    E_DIO_PIN4,
    // E_DIO_PIN5,
    // E_DIO_PIN6,
    // E_DIO_PIN7,
    E_DIO_PIN8,
    E_DIO_PIN9,
    E_DIO_PIN10,
    E_DIO_PIN11,
    // E_DIO_PIN12,
    // E_DIO_PIN13,
    E_DIO_PIN14,
    E_DIO_PIN15,
    E_DIO_PIN16,
    E_DIO_PIN17,
    // E_DIO_PIN18,
    // E_DIO_PIN19,
    E_DIO_PIN20,
    E_DIO_PIN21,
    E_DIO_PIN22,
    E_DIO_PIN23,
    E_DIO_PIN24,
    E_DIO_PIN25,
    E_DIO_PIN26,
    E_DIO_PIN27,
    E_DIO_PIN28,
    E_DIO_PIN29,
    E_DIO_PIN30,
    // E_DIO_PIN31,
    // E_DIO_PIN32,
    E_DIO_PIN33,
    E_DIO_PIN34,
    E_DIO_PIN35,
    E_DIO_PIN36,
    E_DIO_PIN37,
    E_DIO_PIN38,
    E_DIO_PIN39,
    E_DIO_PIN40,
    E_DIO_PIN41,
    E_DIO_PIN42,
    E_DIO_PIN43,
    E_DIO_PIN44,
    E_DIO_PIN45,
    E_DIO_PIN46,
    // E_DIO_PIN47,
    // E_DIO_PIN48,
    E_DIO_PIN49,
    E_DIO_PIN50,
    E_DIO_PIN51,
    E_DIO_PIN52,
    E_DIO_PIN53,
    E_DIO_PIN54,
    E_DIO_PIN55,
    E_DIO_PIN56,
    E_DIO_PIN57,
    E_DIO_PIN58,
    E_DIO_PIN59,
    // E_DIO_PIN60,
    E_DIO_PIN61,
    E_DIO_PIN62,
    // E_DIO_PIN63,
    // E_DIO_PIN64,
#endif
    E_MAX_DIO_NUMBER,
}E_DIO_RESOURCE_MANAGE;

typedef enum
{
    DIGITAL_NO_TYPE = 0,
    DIGITAL_OUTPUT_TYPE,
    DIGITAL_INPUT_TYPE,
    DIGITAL_PWM_OUTPUT_TYPE,
}E_GPIO_TYPE_LIST;

typedef enum
{
    E_SET_VALUE_LOW = 0,
    E_SET_VALUE_HIGH,
}E_BIT_VALUE_LIST;

void fgv_ResourceSoftwareReset(void);

#endif /*__RESOURCE_SUMMARY_DESCRIBE_H_*/
