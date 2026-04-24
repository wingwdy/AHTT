/******************************************************************************
* File Name          : Cdd_CardMConfig.h
* Description        : Code for Configuration of Card interface
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
#ifndef CDD_CARDM_CONFIG_H_
#define CDD_CARDM_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"
#include "Global.h"
#include "DS_LogM.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDCARDM_CFG_SWIPECARD_INTERVAL_TICK    	(300u)  /* 刷卡间隔 */
#define CDDCARDM_CFG_SWIPECARD_PAUSE_TICK       	(5000u) /* 刷卡暂停时间 */
#define CDDCARDM_CFG_SWIPECARD_FAULT_TICK           (500u)  /* 刷卡器故障检测时间间隔 */
#define CDDCARDM_CFG_SWIPECARD_FAULT_COUNT          (10u)   /* 刷卡故障累计次数*/

#define CDDCARDM_CFG_LogPrint(fmt, ...)             DSLOGM_Debug(DSLogMModule_CardM, fmt, ##__VA_ARGS__)
#define CDDCARDM_CFG_RunLogPrint(fmt, ...)          DSLOGM_Info(DSLogMModule_CardM, fmt, ##__VA_ARGS__)
/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct
{
    void (*pFunInitMemery)(void);
    int8_t (*pFunGetCardSn)(uint8_t *pCardSn);

} CddCardMConfigStruct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* CDD_CARDM_CONFIG_H_ */



