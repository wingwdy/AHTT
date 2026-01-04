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
static void CddDrvEG800AK_SwitchNextSocket(void);

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
    const ATCmdDescribtor_Struct *pCmdDescribtor = NULL;
    uint8_t curTaskID = 0;
    uint8_t findTimes = 0;

    while (findTimes < (CDDDRV_EG800AK_CFG_SOCKET_COUNT + 1))
    {
        if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
        {
            socketIndex = 0;
            eSocketType = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].eSocketType;
            curTaskID = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].socketATTaskArray[0];
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
                    curTaskID = g_stCddDrvEG800AKCtrl.moduleATTaskArray[0];
                }
                else
                {
                    eSocketType = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].eSocketType;
                    curTaskID = g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex].socketATTaskArray[0];
                }
            }
        }

        findTimes++;

        if (curTaskID != 0)
        {
            pCmdDescribtor = CddDrvEG800AK_GetATDescribtor(eSocketType, curTaskID);

            if (pCmdDescribtor != NULL)
            {
                g_stCddDrvEG800AKCtrl.currentTaskCmd = curTaskID;
                g_stCddDrvEG800AKCtrl.currentTaskSocketIndex = socketIndex;
                g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = pCmdDescribtor;
                g_stCddDrvEG800AKCtrl.atTryCount = 0;
                break;
            }
        }
    }
}


  
static uint16_t CddDrvEG800AK_PackATFrame(uint8_t socketIndex, uint8_t *txBuf, const ATCmdDescribtor_Struct *pTaskCmdDescribtor)
{
    uint16_t index = 0;

    if (strlen(pTaskCmdDescribtor->cAT) > 0)
    {
        strcpy((char *)&txBuf[index], pTaskCmdDescribtor->cAT);
        index += strlen(pTaskCmdDescribtor->cAT);
    }

    if (pTaskCmdDescribtor->pFuncPackAT != NULL)
    {
        index += pTaskCmdDescribtor->pFuncPackAT(socketIndex, &txBuf[index]);
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
                g_stCddDrvEG800AKCtrl.atTryCount++;
                g_stCddDrvEG800AKCtrl.atWaitTickStart = Common_GetSystick();
                break;
            }

            case CDDDRV_EG800AK_CTRL_STEP2:
            {
                if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.atWaitTickStart, pATCmdDescribtor->waitTimeout))
                {
                    if (pATCmdDescribtor->maxTryCnt == 1)
                    {
                        CddDrvEG800AK_DeleteCmd();
                        break;
                    }
                    else
                    {
                        if (g_stCddDrvEG800AKCtrl.atTryCount >= pATCmdDescribtor->maxTryCnt)
                        {
                            CddDrvEG800AK_DeleteCmd();
                            break;
                        }
                    }

                    g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP1;
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

static uint8_t CddDrvEg800AK_UrcDecode(uint8_t *pData, uint16_t dataLen)
{
    return FALSE;
}

static void CddDrvEG800AK_ATTaskRecvHandle(uint8_t *recvbuf)
{
    const ATCmdDescribtor_Struct *pATCmdDescribtor = g_stCddDrvEG800AKCtrl.currentTaskATDescribtor;
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
                    bAnswerOK = pATCmdDescribtor->pFuncRecvHandle(socketIndex, (uint8_t *)pDest, dataLen - (uint16_t)((uint32_t)pDest - (uint32_t)recvbuf));
                }
                else
                {
                    bAnswerOK = TRUE;
                }
            }
        }

        if (pDest == NULL)
        {
            bAnswerOK = CddDrvEg800AK_UrcDecode(recvbuf, dataLen);
        }

        if (bAnswerOK == TRUE)
        {
            CddDrvEG800AK_DeleteCmd();
        }
    }
}

static void CddDrvEG800AK_CmdTaskHandle(void)
{
    uint8_t cacheBuff[CDDDRV_EG800AK_CFG_BUFF_SIZE] = {0};

    CddDrvEG800AK_ATTaskSendHandle(cacheBuff);
    CddDrvEG800AK_ATTaskRecvHandle(cacheBuff);
}

static void CddDrvEG800AK_StartModuleCfg(void)
{
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryModule);
#if (CDDDRV_EG800AK_CFG_CPIN_DETECT_HW_SUPPORT == TRUE)
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetSimStatusReportEnable);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QuerySimStatus);
#endif
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QuerySimRecognizeStatus);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryIccid);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCsq);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNtpClk);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCREG);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCOPS);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNetWorkInfo);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN0);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN1);
}

uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    uint8_t *pTaskArray = NULL;
    const CddDrvEG800AKATConfig_Struct *pATTablePtr = NULL;
    CddNetMSocketType_Enum eSocketType = eCddNetMSocketType_Null;
    uint8_t addResult = FALSE;
    uint8_t index = 0;
    
    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        pTaskArray = g_stCddDrvEG800AKCtrl.moduleATTaskArray;
        eSocketType = eCddNetMSocketType_Null;
    }
    else
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            if (pSocketCtrl->useFlag == TRUE)
            {
                pTaskArray = pSocketCtrl->socketATTaskArray;
                eSocketType = pSocketCtrl->eSocketType;
            }
        }
    }

    if (pTaskArray != NULL)
    {
        pATTablePtr = &c_stCddDrvEG800AKATConfigTable[eSocketType];

        if (cmd < pATTablePtr->cmdTaskCount)
        {
            for (index = 0; index < CDDDRV_EG800AK_CFG_AT_TASK_COUNT; index++)
            {
                if (pTaskArray[index] == cmd)
                {
                    break;
                }
                else if (pTaskArray[index] == 0)
                {
                    pTaskArray[index] = cmd;
                    addResult = TRUE;
                    break;
                }
            }
        }
    }

    return addResult;
}

void CddDrvEG800AK_DeleteCmd(void)
{
    uint8_t socketIndex = g_stCddDrvEG800AKCtrl.currentTaskSocketIndex;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    uint8_t *pTaskArray = NULL;
    
    if (g_stCddDrvEG800AKCtrl.currentTaskCmd != 0)
    {
        if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
        {
            pTaskArray = g_stCddDrvEG800AKCtrl.moduleATTaskArray;
        }
        else
        {
            if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
            {
                if (pSocketCtrl->useFlag == TRUE)
                {
                    pTaskArray = pSocketCtrl->socketATTaskArray;
                }
            }
        }
    }

    if (pTaskArray != NULL)
    {
        memmove(&pTaskArray[0], &pTaskArray[1], sizeof(uint8_t) * (CDDDRV_EG800AK_CFG_AT_TASK_COUNT - 1));
        memset(&pTaskArray[CDDDRV_EG800AK_CFG_AT_TASK_COUNT - 1], 0x00, sizeof(uint8_t));
        g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
        g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
    }
}




void CddDrvEG800AK_ClearSocketCmd(uint8_t socketIndex)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    uint8_t *pTaskArray = NULL;
    
    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        pTaskArray = g_stCddDrvEG800AKCtrl.moduleATTaskArray;
    }
    else
    {
        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
        {
            pTaskArray = pSocketCtrl->socketATTaskArray;
        }
    }
    
    if (pTaskArray != NULL)
    {
        memset(pTaskArray, 0x00, sizeof(uint8_t) * CDDDRV_EG800AK_CFG_AT_TASK_COUNT);
    }
}

void CddDrvEG800AK_SetModuleState(CddNetMModuleState_Enum eModuleState)
{
    if (eModuleState != g_stCddDrvEG800AKCtrl.eModuleState)
    {
        g_stCddDrvEG800AKCtrl.eModuleState = eModuleState;
    }
}


void CddDrvEG800AK_MainFunction(void)
{
    static uint8_t initFlag = FALSE;

    switch (g_stCddDrvEG800AKCtrl.eModuleState)
    {
        case eCddNetMModuleState_Init:
        {
            if (GLOBAL_OPT_STATE_SUCCESS == CddDrvEG800AK_PowerOn())
            {
                CddDrvEG800AK_StartModuleCfg();
                CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Cfg);
            }

            break;
        }
        case eCddNetMModuleState_Cfg:
        {
            break;
        }
        case eCddNetMModuleState_Work:
        {
            break;
        }
        case eCddNetMModuleState_AbNormal:
        {
 

            break;
        }
        default:
        {
            break;
        }
    }

    if (g_stCddDrvEG800AKCtrl.eModuleState != eCddNetMModuleState_Init)
    {
        CddDrvEG800AK_CmdTaskHandle();
    }
}






























