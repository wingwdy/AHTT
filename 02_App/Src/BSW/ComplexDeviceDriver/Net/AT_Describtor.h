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


/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
	char *cAT;
	char *cATAnswer;
	uint8_t maxTryCnt;
	uint32_t waitTimeout;
	uint32_t maxAckTimeout;
	uint8_t printFlag;
	char *cMeanings;
	uint16_t (*pFuncPackAT)(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t nATLen);
	uint8_t (*pFuncRecvHandle)(uint8_t socketIndex, void * socketPara, uint8_t *pData, uint16_t dataLen);
	uint8_t (*pFuncFailHandle)(uint8_t socketIndex, void * socketPara, uint8_t atTaskID);
}ATCmdDescribtor_Struct;

typedef struct
{
	char *cUrc;
	void (*pFuncRecvHandle)(uint8_t *pData, void * modulePara, uint16_t dataLen);
	uint8_t printFlag;
	char *cMeanings;
}ATUrcDescribtor_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif





















