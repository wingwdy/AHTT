
/******************************************************************************
* File Name          : Cdd_PEConfig.h
* Description        : Code for PEConfig
 ------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      shenjc    初版创建
*
******************************************************************************/
#ifndef CDD_PE_CONFIG_H_
#define CDD_PE_CONFIG_H_
/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Adc.h"
#include "Cdd_ModeM.h"
/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDPE_CFG_ADC_BUFF_NUM 	    	                        (MCALADC_ADC0_SAMPLE_CNT)    /*ADC sample buffer number */

#define CDDPE_CFG_PE_UNCONN_MIN_VOLT                            (700u)
#define CDDPE_CFG_PE_UNCONN_MAX_VOLT                            (1200u)
#define CDDPE_CFG_PE_CHECK_FILTER_TIME                          (5000u)

/* 火零反接的电压值 */
#define CDDPE_CFG_LN_REVERSE_MIN_VOLT                           (1500u)
#define CDDPE_CFG_LN_REVERSE_MAX_VOLT                           (2300u)

#define CDDPE_CFG_LN_REVERSE_FILTER_TIME                        (3000u)
#define CDDPE_CFG_LN_REVERSE_CHECK_MAXTIME                      (5000u)


#define CDDPE_CFG_IsQBStandardMode()                            (CddModeM_IsGBMode() == FALSE)
#define CDDPE_CFG_AswErrHandle_PileSetErrCallback(errType)      AswErrhandle_SetErrExsitCallback(0, errType)
#define CDDPE_CFG_AswErrHandle_PileResetErrCallback(errType)    AswErrhandle_ResetErrExsitCallback(0, errType)


/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/

/******************************************************************************
*    Global variables Declaration
******************************************************************************/



/******************************************************************************
*    Global Function Prototypes
******************************************************************************/


#endif /* CDD_PE_CONFIG_H_ */

