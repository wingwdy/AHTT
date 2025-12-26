/******************************************************************************
* File Name          : template_Config.h
* Description        : Code for xxxxxxxxxxx
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
#ifndef CDD_4G_ATM_CONFIG_H_
#define CDD_4G_ATM_CONFIG_H_

/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Common.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDD4G_ATM_CFG_TX_AT_MAXLEN                    80
#define CDD4G_ATM_CFG_EXPECT_AT_ANSWER_LEN            36

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
	char cAT[CDD4G_ATM_CFG_TX_AT_MAXLEN];
    uint16_t (*pFuncPackAT)(uint8_t socketID, uint8_t *pData, uint16_t dataLen);
	char cATAnswer[CDD4G_ATM_CFG_EXPECT_AT_ANSWER_LEN];
	uint8_t (*pFuncRecvHandle)(uint8_t socketID, uint8_t *pData, uint16_t dataLen);						
	uint8_t (*pFuncFailHandle)(uint8_t socketID, uint8_t atTaskID); 
	uint8_t maxTryCnt;
	uint32_t waitTimeout;
	uint8_t atTaskID;												
	char *cMeanings;						
}Cdd4GATMTaskConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif





















