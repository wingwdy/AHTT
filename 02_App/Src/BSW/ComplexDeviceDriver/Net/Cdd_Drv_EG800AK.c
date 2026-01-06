/******************************************************************************
* File Name          : Cdd_Drv_EG800AK.c
* Description        : Code for EG800AK Driver
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
#include "Cdd_Drv_EG800AK.h"
#include "Cdd_Drv_EG800AKConfig.h"
#include "AT_Describtor.h"
#include "AT_Module.h"
#include "Asw_ErrorHandle.h"
/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/



/**********************CDDDRV_EG800AK******************************************
*    Macro Definition
*******************************************************************************/
#define CDDDRV_EG800AK_CTRL_STEP0         0
#define CDDDRV_EG800AK_CTRL_STEP1         1
#define CDDDRV_EG800AK_CTRL_STEP2         2
#define CDDDRV_EG800AK_CTRL_STEP3         3   
#define CDDDRV_EG800AK_CTRL_STEP4         4
#define CDDDRV_EG800AK_CTRL_STEPEND       9

/* 用0xFF指向模块AT, 0~ 后面表示具体的socketIndex */
#define CDDDRV_EG800AK_MODULE_SOCKET      0xFF 

/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
CddDrvEG800AKCtrl_Struct g_stCddDrvEG800AKCtrl = { 0 };


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint8_t CddDrvEG800AK_PowerOn(void);
static const ATCmdDescribtor_Struct *CddDrvEG800AK_GetATDescribtor(CddNetMSocketType_Enum eSocketType, uint8_t cmd);
static void CddDrvEG800AK_SwitchNextSocket(void);
static void CddDrvEg800AK_UrcDecode(uint8_t *pData, uint16_t dataLen);

static void CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Enum eAbnormalType);

static void CddDrvEG800AK_DelAllSocket(void);
static void CddDrvEG800AK_CloseAllSocket(void);
static uint8_t CddDrvEG800AK_CheckAllSocketCloseFinish(void);
static void CddDrvEG800AK_StartModuleCfg(void);
static void CddDrvEG800AK_ClearAllSocketCmd(void);
static void CddDrvEG800AK_AbnormalHandle(void);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t CddDrvEG800AK_PowerOn(void)
{
    uint8_t result = GLOBAL_OPT_STATE_PROCESS;

    switch (g_stCddDrvEG800AKCtrl.powerOnStep)
    {
        case CDDDRV_EG800AK_CTRL_STEP0:
        {
            CDDDRV_EG800AK_CFG_PwrOff();
            g_stCddDrvEG800AKCtrl.powerCtrlStartTick = Common_GetSystick();
            g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP1;
            CDDDRV_EG800AK_CFG_LogPrint("4G Module EG800AK PowerOff!\n\n");
            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP1:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWEROFF_HOLD_TIME))
            {
                CDDDRV_EG800AK_CFG_PwrOn();
                g_stCddDrvEG800AKCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP2;
                CDDDRV_EG800AK_CFG_LogPrint("\n4G Module EG800AK PowerOn!!!\n\n");
            }

            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP2:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWERON_HOLD_TIME))
            {
                CDDDRV_EG800AK_CFG_PwrKeyOff();
                g_stCddDrvEG800AKCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP3;
                CDDDRV_EG800AK_CFG_LogPrint("\n4G Module EG800AK PowerKeyOff!!!\n\n");
            }

            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP3:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWERKEY_OFF_HOLD_TIME))
            {
                CDDDRV_EG800AK_CFG_PwrKeyOn();
                g_stCddDrvEG800AKCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP4;
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK PowerKeyOn!!!\n\n");
            }

            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP4:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWERKEY_ON_HOLD_TIME))
            {
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEPEND;
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK Start Finish!!!\n\n");
            }

            break;
        }

        case CDDDRV_EG800AK_CTRL_STEPEND:
        {
            result = GLOBAL_OPT_STATE_SUCCESS;
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}
static const ATCmdDescribtor_Struct *CddDrvEG800AK_GetATDescribtor(CddNetMSocketType_Enum eSocketType, uint8_t cmd)
{
    const CddDrvEG800AKATConfig_Struct *pATTablePtr = &c_stCddDrvEG800AKATConfigTable[eSocketType];
    const ATCmdDescribtor_Struct *pATDescribtor = NULL;

    if (pATTablePtr != NULL)
    {
        pATDescribtor = &pATTablePtr->pATCmdDescribtorTable[cmd];

        if (strlen(pATDescribtor->cAT) == 0)
        {
            pATDescribtor = NULL;
        }
    }

    return pATDescribtor;
}

static void CddDrvEG800AK_SwitchNextSocket(void)
{
    CddNetMSocketType_Enum eSocketType = eCddNetMSocketType_Null;
    uint8_t socketIndex = g_stCddDrvEG800AKCtrl.currentTaskSocketIndex;
    CddDrvEG800AKATCtrl_Struct *pAtCtrl = NULL;
    const ATCmdDescribtor_Struct *pCmdDescribtor = NULL;
    uint8_t findTimes = 0;

    while (findTimes < (CDDDRV_EG800AK_CFG_SOCKET_COUNT + 1))
    {
        if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
        {
            socketIndex = 0;
            eSocketType = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].eSocketType;
            pAtCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].stSocketAtCtrl;
        }
        else
        {
            if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
            {
                socketIndex++;
            
                if (socketIndex == CDDDRV_EG800AK_CFG_SOCKET_COUNT)
                {
                    socketIndex = CDDDRV_EG800AK_MODULE_SOCKET;
                    eSocketType = eCddNetMSocketType_Null;
                    pAtCtrl = &g_stCddDrvEG800AKCtrl.stModuleAtCtrl;
                }
                else
                {
                    eSocketType = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].eSocketType;
                    pAtCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].stSocketAtCtrl;
                }
            }
        }

        findTimes++;

        if ((pAtCtrl != NULL) && (pAtCtrl->readyFlag == TRUE) && (pAtCtrl->atTaskArray[0] != 0))
        {
            pCmdDescribtor = CddDrvEG800AK_GetATDescribtor(eSocketType, pAtCtrl->atTaskArray[0]);

            if (pCmdDescribtor != NULL)
            {
                g_stCddDrvEG800AKCtrl.currentTaskCmd = pAtCtrl->atTaskArray[0];
                g_stCddDrvEG800AKCtrl.currentTaskSocketIndex = socketIndex;
                g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = pCmdDescribtor;
                pAtCtrl->readyFlag = FALSE;
                pAtCtrl->atTryCount++;
                pAtCtrl->atWaitTickStart = Common_GetSystick();
                break;
            }
        }
    }
}

static uint16_t CddDrvEG800AK_PackATFrame(uint8_t socketIndex, uint8_t *txBuf, const ATCmdDescribtor_Struct *pTaskCmdDescribtor)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint16_t index = 0;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    }

    if (strlen(pTaskCmdDescribtor->cAT) > 0)
    {
        strcpy((char *)&txBuf[index], pTaskCmdDescribtor->cAT);
        index += strlen(pTaskCmdDescribtor->cAT);
    }

    if (pTaskCmdDescribtor->pFuncPackAT != NULL)
    {
        index = pTaskCmdDescribtor->pFuncPackAT(socketIndex, pSocketCtrl, txBuf, index);
    }

    return index;
}

static void CddDrvEG800AK_ATTaskSendHandle(uint8_t *txBuf)
{
    const ATCmdDescribtor_Struct * pATCmdDescribtor = g_stCddDrvEG800AKCtrl.currentTaskATDescribtor;
    uint16_t txLen = 0;

    if (g_stCddDrvEG800AKCtrl.transparentMode == TRUE)
    {
        if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.transparentModeStartTick, CDDDRV_EG800AK_CFG_TRANSPARENT_TIMEOUT))
        {
            g_stCddDrvEG800AKCtrl.transparentMode = FALSE;
            g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP0;
        }
    }
    else
    {
        if (g_stCddDrvEG800AKCtrl.currentTaskCmd == 0)
        {
            g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP0;
        }

        switch (g_stCddDrvEG800AKCtrl.cmdTaskStep)
        {
            case CDDDRV_EG800AK_CTRL_STEP0:
            {
                CddDrvEG800AK_SwitchNextSocket();

                if (g_stCddDrvEG800AKCtrl.currentTaskCmd != 0)
                {
                    g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP1;
                }

                break;
            }

            case CDDDRV_EG800AK_CTRL_STEP1:
            {
                txLen = CddDrvEG800AK_PackATFrame(g_stCddDrvEG800AKCtrl.currentTaskSocketIndex, txBuf, pATCmdDescribtor);

                if (txLen > 0)
                {
                    CDDDRV_EG800AK_CFG_WriteData(txBuf, txLen);
                    txBuf[txLen] = 0;
                    CDDDRV_EG800AK_CFG_LogPrint("[4G %s-->Tx]:\n%s\n", pATCmdDescribtor->cMeanings, txBuf);
                }

                g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP2;
                g_stCddDrvEG800AKCtrl.waitAtAckTickStart = Common_GetSystick();
                break;
            }

            case CDDDRV_EG800AK_CTRL_STEP2:
            {
                if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.waitAtAckTickStart, pATCmdDescribtor->maxAckTimeout))
                {
                    g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
                    g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
                    g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP0;
                    break;
                }
            }
            default:
            {
                break;
            }
        }
    }
}


static void CddDrvEG800AK_ATTaskTimeoutDetect(uint8_t socketIndex, CddNetMSocketType_Enum eSocketType, CddDrvEG800AKATCtrl_Struct *pAtCtrl)
{
    const ATCmdDescribtor_Struct *pCmdDescribtor = NULL;

    if (pAtCtrl->readyFlag == FALSE && pAtCtrl->atTaskArray[0] != 0)
    {
        pCmdDescribtor = CddDrvEG800AK_GetATDescribtor(eSocketType, pAtCtrl->atTaskArray[0]);

        if (pCmdDescribtor != NULL)
        {
            if (pAtCtrl->atTryCount == 0)
            {
                pAtCtrl->readyFlag = TRUE;
            }
            else
            {
                if (Common_JudgeTimeoutMs(pAtCtrl->atWaitTickStart, pCmdDescribtor->waitTimeout))
                {
                    if (pCmdDescribtor->maxTryCnt == 1)
                    {
                        CddDrvEG800AK_DeleteCmd(socketIndex);
                    }
                    else
                    {
                        if (pAtCtrl->atTryCount >= pCmdDescribtor->maxTryCnt)
                        {
                            CddDrvEG800AK_DeleteCmd(socketIndex);

                            if (pCmdDescribtor->pFuncFailHandle != NULL)
                            {
                                pCmdDescribtor->pFuncFailHandle(socketIndex, pAtCtrl, pAtCtrl->atTaskArray[0]);
                            }
                        }
                        else
                        {
                            pAtCtrl->readyFlag = TRUE;
                        }
                    }
                }
            }
        }
    }
}

static void CddDrvEG800AK_ATTaskRecvHandle(uint8_t *recvbuf)
{
    const ATCmdDescribtor_Struct *pATCmdDescribtor = g_stCddDrvEG800AKCtrl.currentTaskATDescribtor;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = g_stCddDrvEG800AKCtrl.currentTaskSocketIndex;
    uint8_t taskCmd = g_stCddDrvEG800AKCtrl.currentTaskCmd;
    uint16_t dataLen = 0;
    char *pDest = NULL;
    uint8_t bAnswerOK = FALSE;

    CDDDRV_EG800AK_CFG_ReadData(recvbuf, dataLen);

    if (dataLen > 0)
    {
        recvbuf[dataLen] = 0;
        CDDDRV_EG800AK_CFG_LogPrint("[4G-->Rx]:\n%s\n", recvbuf);
        
        if (g_stCddDrvEG800AKCtrl.currentTaskCmd != 0)
        {
            pDest = (char *)Common_SearchData(recvbuf, dataLen, pATCmdDescribtor->cATAnswer, strlen(pATCmdDescribtor->cATAnswer));
            
            if (pDest != NULL)
            {
                if (NULL != pATCmdDescribtor->pFuncRecvHandle)
                {
                    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
                    {
                        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
                    }

                    bAnswerOK = pATCmdDescribtor->pFuncRecvHandle(socketIndex, pSocketCtrl, (uint8_t *)pDest, dataLen - (uint16_t)((uint32_t)pDest - (uint32_t)recvbuf));
                }
                else
                {
                    bAnswerOK = TRUE;
                }

                if (bAnswerOK == TRUE)
                {
                    CddDrvEG800AK_DeleteCmd(g_stCddDrvEG800AKCtrl.currentTaskSocketIndex);
                }
            }
        }

        if (pDest == NULL)
        {
            CddDrvEg800AK_UrcDecode(recvbuf, dataLen);
        }
    }
}

static void CddDrvEg800AK_UrcDecode(uint8_t *pData, uint16_t dataLen)
{

}

static void CddDrvEG800AK_CmdTaskHandle(void)
{
    uint8_t cacheBuff[CDDDRV_EG800AK_CFG_BUFF_SIZE] = {0};
    uint8_t socketIndex = CDDDRV_EG800AK_MODULE_SOCKET;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;

    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        CddDrvEG800AK_ATTaskTimeoutDetect(socketIndex, eCddNetMSocketType_Null, &g_stCddDrvEG800AKCtrl.stModuleAtCtrl);
    }

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
        CddDrvEG800AK_ATTaskTimeoutDetect(socketIndex, pSocketCtrl->eSocketType, &pSocketCtrl->stSocketAtCtrl);
    }

    CddDrvEG800AK_ATTaskSendHandle(cacheBuff);
    CddDrvEG800AK_ATTaskRecvHandle(cacheBuff);
}

static void CddDrvEG800AK_DelAllSocket(void)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

        if (pSocketCtrl->usedFlag == TRUE)
        {
            if (pSocketCtrl->socketCloseHandle != NULL)
            {
                pSocketCtrl->socketCloseHandle(pSocketCtrl);
            }

            pSocketCtrl->usedFlag = FALSE;
        }
    }
}

static void CddDrvEG800AK_CloseAllSocket(void)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

        if (pSocketCtrl->socketCloseHandle != NULL)
        {
            pSocketCtrl->socketCloseHandle(pSocketCtrl);
        }
    }
}

static uint8_t CddDrvEG800AK_CheckAllSocketCloseFinish(void)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;
    uint8_t ret = TRUE;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

        if (pSocketCtrl->eSocketState != eCddNetMSocketState_WaitReconnect && 
            pSocketCtrl->eSocketState != eCddNetMSocketState_Init)
        {
            ret = FALSE;
            break;
        }
    }

    return ret;
}

static void CddDrvEG800AK_StartModuleCfg(void)
{
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryModule);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QuerySimRecognizeStatus);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryIccid);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCsq);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNtpClk);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCGREG);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCOPS);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNetWorkInfo);
}

static void CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Enum eAbnormalType)
{
    CddDrvEG800AKAbnormalHandle_Enum tempAbnormalType;
    
    if (eAbnormalType == CddDrvEG800AKAbnormalHandle_CFun)
    {
        if (g_stCddDrvEG800AKCtrl.eLastAbnormalHandleType == CddDrvEG800AKAbnormalHandle_CFun)
        {
            tempAbnormalType = CddDrvEG800AKAbnormalHandle_Reboot;
        }
        else
        {
            tempAbnormalType = CddDrvEG800AKAbnormalHandle_CFun;
        }
    }
    else
    {
        tempAbnormalType = CddDrvEG800AKAbnormalHandle_Reboot;
    }

    if (tempAbnormalType != g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType)
    {
        g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType = tempAbnormalType;
        g_stCddDrvEG800AKCtrl.abNormalHandleStep = CDDDRV_EG800AK_CTRL_STEP0;
        CddDrvEG800AK_ClearAllSocketCmd();
    }
}

static void CddDrvEG800AK_AbnormalHandle(void)
{
    switch (g_stCddDrvEG800AKCtrl.abNormalHandleStep)
    {
        case CDDDRV_EG800AK_CTRL_STEP0:
        {
            CddNetM_SwitchPhyChannel(CDD_NETM_CFG_DEV_4G);
            CddDrvEG800AK_CloseAllSocket();
            g_stCddDrvEG800AKCtrl.eLastAbnormalHandleType = g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType;
            g_stCddDrvEG800AKCtrl.abNormalHandleStep = CDDDRV_EG800AK_CTRL_STEP1;
            break;
        }
        case CDDDRV_EG800AK_CTRL_STEP1:
        {
            if (TRUE == CddDrvEG800AK_CheckAllSocketCloseFinish())
            {
                if (g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType == CddDrvEG800AKAbnormalHandle_CFun)
                {
                    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN0);
                    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN1);
                    g_stCddDrvEG800AKCtrl.abNormalHandleStep = CDDDRV_EG800AK_CTRL_STEP2;
                }
                else
                {
                    CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Init);
                }
            }

            break;
        }
        case CDDDRV_EG800AK_CTRL_STEP2:
        {
            if (g_stCddDrvEG800AKCtrl.stModuleAtCtrl.atTaskArray[0] == 0)
            {
                CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Cfg);
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

static void CddDrvEG800AK_ClearAllSocketCmd(void)
{
    uint8_t socketIndex = 0;

    CddDrvEG800AK_ClearSocketCmd(CDDDRV_EG800AK_MODULE_SOCKET);

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        CddDrvEG800AK_ClearSocketCmd(socketIndex);
    }
}

uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd)
{
    CddDrvEG800AKATCtrl_Struct *pAtCtrl = NULL;
    const CddDrvEG800AKATConfig_Struct *pATTablePtr = NULL;
    CddNetMSocketType_Enum eSocketType = eCddNetMSocketType_Null;
    uint8_t addResult = FALSE;
    uint8_t index = 0;
    
    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        pAtCtrl = &g_stCddDrvEG800AKCtrl.stModuleAtCtrl;
        eSocketType = eCddNetMSocketType_Null;
    }
    else
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            if (g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].usedFlag == TRUE)
            {
                pAtCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].stSocketAtCtrl;
                eSocketType = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].eSocketType;
            }
        }
    }

    if (pAtCtrl != NULL)
    {
        pATTablePtr = &c_stCddDrvEG800AKATConfigTable[eSocketType];

        if (cmd < pATTablePtr->cmdTaskCount)
        {
            for (index = 0; index < CDDDRV_EG800AK_CFG_AT_TASK_COUNT; index++)
            {
                if (pAtCtrl->atTaskArray[index] == cmd)
                {
                    break;
                }
                else if (pAtCtrl->atTaskArray[index] == 0)
                {
                    pAtCtrl->atTaskArray[index] = cmd;
                    addResult = TRUE;
                    break;
                }
                else
                {}
            }
        }
    }

    return addResult;
}

void CddDrvEG800AK_DeleteCmd(uint8_t socketIndex)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    CddDrvEG800AKATCtrl_Struct *pAtCtrl = NULL;
    
    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        pAtCtrl = &g_stCddDrvEG800AKCtrl.stModuleAtCtrl;
    }
    else
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            if (g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].usedFlag == TRUE)
            {
                pAtCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].stSocketAtCtrl;;
            }
        }
    }
    
    if (pAtCtrl != NULL)
    {
        memmove(&pAtCtrl->atTaskArray[0], &pAtCtrl->atTaskArray[1], sizeof(uint8_t) * (CDDDRV_EG800AK_CFG_AT_TASK_COUNT - 1));
        memset(&pAtCtrl->atTaskArray[CDDDRV_EG800AK_CFG_AT_TASK_COUNT - 1], 0x00, sizeof(uint8_t));

        pAtCtrl->atTryCount = 0;
        pAtCtrl->readyFlag = FALSE;

        if (socketIndex == g_stCddDrvEG800AKCtrl.currentTaskSocketIndex)
        {
            g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
            g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
        }
    }
}

void CddDrvEG800AK_ClearSocketCmd(uint8_t socketIndex)
{
    CddDrvEG800AKATCtrl_Struct *pAtCtrl = NULL;
    
    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        pAtCtrl = &g_stCddDrvEG800AKCtrl.stModuleAtCtrl;
    }
    else
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pAtCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].stSocketAtCtrl;
        }
    }
    
    if (pAtCtrl != NULL)
    {
        memset(pAtCtrl->atTaskArray, 0x00, sizeof(uint8_t) * CDDDRV_EG800AK_CFG_AT_TASK_COUNT);

        if (g_stCddDrvEG800AKCtrl.currentTaskSocketIndex == socketIndex)
        {
            g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
            g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
        }
    }
}


void CddDrvEG800AK_SetModuleState(CddNetMModuleState_Enum eModuleState)
{
    if (eModuleState != g_stCddDrvEG800AKCtrl.eModuleState)
    {
        if (eModuleState == eCddNetMModuleState_Init)
        {
            g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP0;
        }
        else if (eModuleState == eCddNetMModuleState_Cfg)
        {
            CddDrvEG800AK_StartModuleCfg();
        }
        else
        {}

        g_stCddDrvEG800AKCtrl.eModuleState = eModuleState;
    }
}

CddNetMModuleState_Enum CddDrvEG800AK_GetModuleState(void)
{
    return g_stCddDrvEG800AKCtrl.eModuleState;
}

void CddDrvEG800AK_MainFunction(void)
{
    if (g_stCddDrvEG800AKCtrl.eModuleState == eCddNetMModuleState_Init)
    {
        if (GLOBAL_OPT_STATE_SUCCESS == CddDrvEG800AK_PowerOn())
        {
            CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Cfg);
        }
    }
    else
    {
        if (g_stCddDrvEG800AKCtrl.eModuleState == eCddNetMModuleState_AbNormal)
        {
            CddDrvEG800AK_AbnormalHandle();
        }
        else if (g_stCddDrvEG800AKCtrl.eModuleState == eCddNetMModuleState_Work)
        {

        }
        else
        {}
        
        CddDrvEG800AK_CmdTaskHandle();
    }
}






























