/******************************************************************************
* File Name          : Asw_VoltCurHandleConfig.c
* Description        : Code for VoltCurHandle
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/11/12      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Asw_VoltCurHandleConfig.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/


/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
  
const AswVoltCurHandleConfig_Struct c_AswVoltCurHandleConfigTable[AswVoltCur_EvtCnt] = 
{
    {AswVoltCur_OV_Set, AswVoltCur_MaxEqu, ASWVOLTCUR_CFG_OV_SET_THR, ASWVOLTCUR_CFG_OV_SET_FILTER_COUNT, TRUE,  eErr_AphaseInputOverVol},
    {AswVoltCur_OV_Clr, AswVoltCur_MinEqu, ASWVOLTCUR_CFG_OV_CLR_THR, ASWVOLTCUR_CFG_OV_CLR_FILTER_COUNT, FALSE, eErr_AphaseInputOverVol},
    {AswVoltCur_UV_Set, AswVoltCur_MinEqu, ASWVOLTCUR_CFG_UV_SET_THR, ASWVOLTCUR_CFG_UV_SET_FILTER_COUNT, TRUE,  eErr_AphaseInputLessVol},
    {AswVoltCur_UV_Clr, AswVoltCur_MaxEqu, ASWVOLTCUR_CFG_UV_CLR_THR, ASWVOLTCUR_CFG_UV_CLR_FILTER_COUNT, FALSE, eErr_AphaseInputLessVol},
    {AswVoltCur_OC_Set, AswVoltCur_MaxEqu, ASWVOLTCUR_CFG_OC_SET_THR, ASWVOLTCUR_CFG_OC_SET_FILTER_COUNT, TRUE,  eErr_OutputOverCurr},
    {AswVoltCur_OC_Clr, AswVoltCur_Min,    ASWVOLTCUR_CFG_OC_CLR_THR, ASWVOLTCUR_CFG_OC_CLR_FILTER_COUNT, FALSE, eErr_OutputOverCurr},
};


/*******************************************************************************
*    Function Source Code
*******************************************************************************/

  




















