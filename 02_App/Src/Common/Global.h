/******************************************************************************
* File Name          : Global.h
* Description        : Code for Global Definition
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

#ifndef Global_H_
#define Global_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "stdint.h"


/******************************************************************************
*    Macro Definition
******************************************************************************/
#define GLOBAL_OPT_STATE_IDLE                          (0U)
#define GLOBAL_OPT_STATE_PROCESS                       (1U)
#define GLOBAL_OPT_STATE_SUCCESS                       (2U)
#define GLOBAL_OPT_STATE_FAIL                          (3U)

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ARRAY_SIZE(x)               sizeof(x) / sizeof(x[0])
#define PARA_ASSERT(x)              do \
                                    {}while(x != TRUE)

#define PARA_ASSERT_RET(x, ret)     do \
                                    {\
                                        if (x != TRUE)\
                                        {\
                                            return ret;\
                                        }\
                                    }while(0)

/******************************************************************************
*    Enum Definition
******************************************************************************/
typedef enum
{
    eGlobalRet_OK,
    eGlobalRet_Error,
    eGlobalRet_NotEnoughChannel,
    eGlobalRet_FIFONotFull,
    eGlobalRet_ParaInvalid,
    eGlobalRet_NotInit,
    eGlobalRet_NotEnoughBuf,
    eGlobalRet_NotEnoughData,
    eGlobalRet_UnexpectedError,
}GlobalRet_Enum;



/******************************************************************************
*    Typedef Definition
******************************************************************************/


/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* Global_H_ */
























