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
#include "Cdd_NetMConfig.h"
#include "string.h"

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
    uint8_t usedFlag;                            /* 表示该链路是否使用 */
    uint8_t validFlag;                          /* 表示该链路是否创建 */
    uint8_t socketIndex;
    CddNetMSocketType_Enum eSocketType;
    CddNetMPlatType_Enum ePlatType;  
    CddNetMSocketPara_Union stSocketPara;
}CddNetMLinkPara_Struct;


typedef struct
{
    CddNetMWorkState_Enum eWorkState;
    uint8_t curNetDev;
    uint8_t curNetDevChooseSuccess;    
    uint32_t switchNetTickStart;
    CddNetMLinkPara_Struct stLinkPara[CDD_NETM_CFG_DEV_COUNT][CCDD_NETM_CFG_LINK_COUNT];
}CddNetMCtx_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddNetMCtx_Struct g_stCddNetMCtx;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t CddNetM_IsFileLinkExist(void);
static CddNetMLinkPara_Struct* CddNetM_FindFreeLink(void);
static void CddNetM_WorkStateManage(void);
static void CddNetM_CheckSocketCreate(void);

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
        pLinkPara = &g_stCddNetMCtx.stLinkPara[CDD_NETM_CFG_DEV_4G][index];

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
        pLinkPara = &g_stCddNetMCtx.stLinkPara[CDD_NETM_CFG_DEV_4G][index];

        if(FALSE == pLinkPara->validFlag)
        {
            pRetVal = pLinkPara;
            break; 
        }
    }
    
    return pRetVal;    
}

static void CddNetM_CheckSocketCreate(void)
{
    CddNetMLinkPara_Struct *pLinkPata = NULL;
    uint8_t index = 0U;

    if (g_stCddNetMCtx.curNetDev == CDD_NETM_CFG_DEV_4G)
    {
		if (c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_4G].getModuleState != NULL &&
			c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_4G].getModuleState() == eCddNetMModuleState_Work)
		{
			for (index = 0; index < CCDD_NETM_CFG_LINK_COUNT; index++)
			{
				pLinkPata = &g_stCddNetMCtx.stLinkPara[CDD_NETM_CFG_DEV_4G][index];

				if (pLinkPata->usedFlag == FALSE && pLinkPata->validFlag == TRUE)
				{
					if (c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_4G].creatSocket != NULL)
					{
						c_NetMModuleOpsTable[CDD_NETM_CFG_DEV_4G].creatSocket(pLinkPata->eSocketType, &pLinkPata->stSocketPara, 
							&pLinkPata->socketIndex, pLinkPata->ePlatType);
						pLinkPata->usedFlag = TRUE;
					}
				}
			}
		}
    }

    /* 预留以太网的创建逻辑，无需求，暂不实现 */
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
            /* 暂未实现网络切换逻辑，后续有需求再实现 */
            g_stCddNetMCtx.curNetDev = CDD_NETM_CFG_DEV_4G;
            g_stCddNetMCtx.curNetDevChooseSuccess = TRUE;
            CddNetM_CheckSocketCreate();
            g_stCddNetMCtx.eWorkState = eCddNetMWorkState_NetWorking;
            break;
        }
        case eCddNetMWorkState_NetWorking:
        {
            CddNetM_CheckSocketCreate();
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

void CddNetM_DelSingleLink(CddNetMPlatType_Enum ePlatType)
{



    

}

void CddNetM_SetLinkDisconnect(CddNetMPlatType_Enum ePlatType)
{




}

uint8_t CddNetM_CheckLinkConnectOK(CddNetMPlatType_Enum ePlatType)
{
	CddNetMLinkPara_Struct *pChannelDCB = NULL;
	uint8_t curNetDev = g_stCddNetMCtx.curNetDev;
	uint8_t ret = FALSE;
	uint8_t index = 0; 

	if (g_stCddNetMCtx.curNetDevChooseSuccess == TRUE)
	{
        for (index = 0; index < CCDD_NETM_CFG_LINK_COUNT; index++)
        {
            pChannelDCB = &g_stCddNetMCtx.stLinkPara[curNetDev][index];
            
            if (pChannelDCB->ePlatType == ePlatType)
            {
                if (c_NetMModuleOpsTable[curNetDev].getSocketState != NULL)
                {
                    if (c_NetMModuleOpsTable[curNetDev].getSocketState(pChannelDCB->socketIndex) == eCddNetMSocketState_ConnectOK)
                    {
                        ret = TRUE;
                    }
                }

                break;
            }
        }
	}
	
	return ret;
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
            pLinkPara->eSocketType = eSocketType;
            pLinkPara->stSocketPara = socketPara;
        }
        else
        {
            retVal = eGlobalRet_NotEnoughChannel;
        }
    }

    return retVal;
}



void CddNetM_SwitchPhyChannel(uint8_t moduleDev)
{


}

void CddNetM_MainFunction(void)
{
    uint8_t netDev = 0U;

    CddNetM_WorkStateManage();

    for (netDev = 0; netDev < CDD_NETM_CFG_DEV_COUNT; netDev++)
    {
        if (c_NetMModuleOpsTable[netDev].pFuncMainFunction != NULL)
        {
            c_NetMModuleOpsTable[netDev].pFuncMainFunction();
        }
    }
}

















