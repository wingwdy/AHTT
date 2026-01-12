/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
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
#include "Asw_PlatM.h"
#include "Cdd_NetM.h"
#include "FrameQueue.h"
#include "Asw_IotProtoGNM.h"

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
IotGNCtx_Struct *pIotGNCtx = NULL;


/*******************************************************************************
*    Function Source Code
*******************************************************************************/




void IotGN_FillLinkPara(CddNetMSocketPara_Union *pLinkPara)
{
    MSNvmPlatParam_Struct *pParam = AswPlatM_GetPlatParamPtr();

    if (pLinkPara != NULL && pIotGNCtx != NULL)
    {
        strcpy(pLinkPara->stTcpPara.ip, pParam->platMainIp);
        pLinkPara->stTcpPara.port = pParam->platMainPort;
        FrameQueue_Creat(eFrameQueueType_TCP, 3072, 3072, &pIotGNCtx->frameQueueChannelID);
        pLinkPara->stTcpPara.frameQueueChannelID = pIotGNCtx->frameQueueChannelID;
    }
}

void IotGN_InitMemory(void)
{
    pIotGNCtx = (IotGNCtx_Struct *)malloc(sizeof(IotGNCtx_Struct));
    if (pIotGNCtx != NULL)
    {
        memset(pIotGNCtx, 0, sizeof(IotGNCtx_Struct));
    }
}

void IotGN_MainFunction(void)
{
    switch (pIotGNCtx->eWorkState)
    {
        case eIOTGNWorkState_Init:
        {
            break;
        }
        case eIOTGNWorkState_Offline:
        {
            break;
        }
        case eIOTGNWorkState_Login:
        {
            break;
        }
        case eIOTGNWorkState_Normal:
        {
            break;
        }
        default:
        {
            pIotGNCtx->eWorkState = eIOTGNWorkState_Init;
        }
    }
}























