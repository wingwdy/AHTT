/******************************************************************************
* File Name          : Cdd_Net.c
* Description        : Code for xxxxxxxxxxx
*                      架构设计预留以太网的可能性
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_NetM.h"



/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
	eCddNetMWorkState_Init,
	eCddNetMWorkState_ChooseNet,
	eCddNetMWorkState_NetWorking,
	eCddNetMWorkState_SwitchNet
}CddNetMWorkState_Enum;





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
    uint8_t useFlag;                            /* 表示该链路是否使用 */
    uint8_t validFlag;                          /* 表示该链路是否创建 */
    CddNetMPlatType_Enum ePlatType;  
    CddNetMSocketPara_Union stSocketPara;
}CddNetMLinkPara_Struct;


typedef struct
{
    CddNetMWorkState_Enum eWorkState;
    uint8_t curNetDev;
    uint8_t curNetDevChooseSuccess;    
    uint32_t switchNetTickStart;
    CddNetMLinkPara_Struct NetMLinkPara[CDD_NETM_DEV_COUNT][CCDD_NETM_CFG_LINK_COUNT];
}CddNetMCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static CddNetMCtx_Struct g_stCddNetMCtx;

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t CddNetM_IsFileLinkExist(void)
{
    uint8_t index = 0U;
    uint8_t fileLinkExist = FALSE;
    CddNetMLinkPara_Struct *pLinkPara = NULL;
    
    for(index = 0U; index < CCDD_NETM_CFG_LINK_COUNT; index++)
    {
        pLinkPara = &g_stCddNetMCtx.NetMLinkPara[CDD_NETM_DEV_DEFAULT][index];

        if((TRUE == pLinkPara->validFlag) &&
           (eCddNetMPlatType_File == pLinkPara->ePlatType))
        {
            fileLinkExist = TRUE;
            break;
        }
    }
    
    return fileLinkExist;
}
static CddNetMLinkPara_Struct* CddNetM_FindFreeLink(void)
{
    uint8_t index = 0U;
    CddNetMLinkPara_Struct* pRetVal = NULL;
    CddNetMLinkPara_Struct *pLinkPara = NULL;
    
    for(index = 0U; index < CCDD_NETM_CFG_LINK_COUNT; index++)
    {
        pLinkPara = &g_stCddNetMCtx.NetMLinkPara[CDD_NETM_DEV_DEFAULT][index];

        if(FALSE == pLinkPara->validFlag)
        {
            pRetVal = pLinkPara;
            break; 
        }
    }
    
    return pRetVal;    
}

GlobalRet_Enum CddNetM_CreatLink(CddNetMSocketType_Enum eSocketType, CddNetMSocketPara_Union socketPara, CddNetMPlatType_Enum ePlatType)
{
    GlobalRet_Enum retVal = eGlobalRet_OK;
    CddNetMLinkPara_Struct* pLinkPara = NULL;

    PARA_ASSERT_RET(eSocketType < eCddNetMSocketType_Count, eGlobalRet_ParaInvalid);

    if (ePlatType == eCddNetMPlatType_File && TRUE == CddNetM_IsFileLinkExist())
    {
        retVal = eGlobalRet_Error;
    }
    else
    {
        pLinkPara = CddNetM_FindFreeLink();

        if (NULL != pLinkPara)
        {
            pLinkPara->validFlag = TRUE;
            pLinkPara->ePlatType = ePlatType;
            pLinkPara->stSocketPara = socketPara;
        }
        else
        {
            retVal = eGlobalRet_NotEnoughChannel;
        }
    }

    return retVal;
}

static void CddNetM_WorkStateManage(void)
{
    switch (g_stCddNetMCtx.eWorkState)
    {
        case eCddNetMWorkState_Init:
        {
            g_stCddNetMCtx.eWorkState = eCddNetMWorkState_ChooseNet;
            break;
        }

        case eCddNetMWorkState_ChooseNet:
        {
            break;
        }
        case eCddNetMWorkState_NetWorking:
        {
            break;
        }
        case eCddNetMWorkState_SwitchNet:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}



















