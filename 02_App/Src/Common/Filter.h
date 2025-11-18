/******************************************************************************
* File Name          : Common.h
* Description        : Code for Common function
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
#ifndef Filter_H_
#define Filter_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define FILTER_FIFO_IO_POINT_COUNT        (10U)     /* 对于IO的FIFO, 每个FIFO可存储的最大点数 */

#define FILTER_FIFO_ANALOG_POINT_COUNT    (10U)     /* 对于模拟量的FIFO, 每个FIFO可存储的最大点数 */

#define FILTER_PROFILE1_MAXVALUE_16       (0xFFFF)

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eFilterIOChannel_RCD,
    eFilterIOChannel_Count,
}FilterIOChannel_Enum;

typedef enum
{
    eFilterAnalogChannel_CP,
    eFilterAnalogChannel_EnvNtc,
    eFilterAnalogChannel_GunNtc,
    eFilterAnalogChannel_ShortCut,
    eFilterAnalogChannel_PE,
    eFilterAnalogChannel_Count,
}FilterAnalogChannel_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t status;
    uint8_t lastStatus;
    uint16_t validStatus;
    uint16_t filterCount;
} FilterProfile1_Struct;

/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
GlobalRet_Enum Filter_IO_CreatFIFO(FilterIOChannel_Enum eCH, uint8_t pointCount, uint8_t initVal);
GlobalRet_Enum Filter_IO_InsertFIFO(FilterIOChannel_Enum eCH, uint8_t ioVal);
GlobalRet_Enum Filter_IO_GetVal(FilterIOChannel_Enum eCH, uint8_t *pIoVal);
uint8_t Filter_Profile1(FilterProfile1_Struct *pFilterStatus, uint16_t filterCnt);



#endif /* Filter_H_ */






















