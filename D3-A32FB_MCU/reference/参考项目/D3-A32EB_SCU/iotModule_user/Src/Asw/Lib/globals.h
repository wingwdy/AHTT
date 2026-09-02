#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

//#include "gd32e50x.h"
//#include "systick.h"

/**********宏观变量**********/

/*枪的最大个数*/
#define GUN_NUM_MAX		2

#define GUN_A    		0
#define GUN_B    		1
#define GUN_ALL    		GUN_NUM_MAX

/**/
#ifndef NULL
#define NULL	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef VALID
#define VALID	0x55
#endif

#ifndef INVALID
#define INVALID	0xAA
#endif

#ifndef U32
#define U32 uint32_t
#endif

#ifndef U16
#define U16 uint16_t
#endif

#ifndef U8
#define U8  uint8_t
#endif

#ifndef S32
#define S32 int32_t
#endif

#ifndef S16
#define S16 int16_t
#endif

#ifndef S8
#define S8 int8_t
#endif

#define FCNT(array) (sizeof(array) / sizeof(array[0]))
#define FPOS(type, field) ((U32) &(( type *) 0)-> field )

#define SET_VALID_FLAG(flag, validflag)		(((U32)flag & 0x00ffffff) | (((U32)(validflag & 0xff)) << 24))
#define GET_VALID_FLAG(flag)				(flag >> 24)
#define SET_CHECK_SUM(flag, checksum)		((((U32)flag) & 0xffffff00) | checksum)
#define SET_LEN(flag, len)					((((U32)flag) & 0xff0000ff) | ((len)<<8))
#define GET_CHECK_SUM(flag)					(flag&0xff)
#define GETET_LEN(flag)						((flag&0xffffff)>>8)

typedef enum
{
    eTick_0ms           = 0,
    eTick_5ms           = 5,
    eTick_10ms          = 10,
    eTick_20ms          = 20,
    eTick_30ms          = 30,
    eTick_50ms          = 50,       
    eTick_100ms         = 100,
    eTick_200ms         = 200,
    eTick_300ms         = 300,
    eTick_500ms         = 500,
    eTick_1S            = 1000,
    eTick_2S            = 2000,
    eTick_3S            = 3000,
    eTick_5S            = 5000,
    eTick_6S            = 6000,
    eTick_8S            = 8000,
    eTick_10S           = 10000,
    eTick_15S           = 15000,
    eTick_20S           = 20000,
	eTick_30S           = 30000,
	eTick_40S           = 40000,
    eTick_60S           = 60000,
    eTick_180S          = 180000,
    eTick_1H            = 3600000,
}TIMEOUT_E;


#endif
