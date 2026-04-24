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
static uint16_t CddDrvEg800AK_UrcDecode(uint8_t *pData, void * modulePara, uint16_t dataLen, uint8_t printEnable);

static void CddDrvEG800AK_CloseAllSocket(void);
static uint8_t CddDrvEG800AK_CheckAllSocketCloseFinish(void);
static void CddDrvEG800AK_StartModuleCfg(void);
static void CddDrvEG800AK_ClearAllSocketCmd(void);
static void CddDrvEG800AK_AbnormalHandle(void);
static uint8_t CddDrvEG800AK_FindFreeSocket(uint8_t *pSocketIndex);
static void CddDrvEG800AK_SocketDisconnectCallback(void);
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
            CDDDRV_EG800AK_CFG_LogPrint("4G Module EG800AK PowerOff!\r\n");
            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP1:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWEROFF_HOLD_TIME))
            {
                CDDDRV_EG800AK_CFG_PwrOn();
                g_stCddDrvEG800AKCtrl.powerCtrlStartTick = Common_GetSystick();
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEP2;
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK PowerOn!!!\r\n");
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
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK PowerKeyOff!!!\r\n");
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
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK PowerKeyOn!!!\r\n");
            }

            break;
        }

        case CDDDRV_EG800AK_CTRL_STEP4:
        {
            if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.powerCtrlStartTick, CDDDRV_EG800AK_CFG_POWERKEY_ON_HOLD_TIME))
            {
                g_stCddDrvEG800AKCtrl.powerOnStep = CDDDRV_EG800AK_CTRL_STEPEND;
                CDDDRV_EG800AK_CFG_LogPrint("\r\n4G Module EG800AK Start Finish!!!\r\n");
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
    const CddDrvEG800AKSocketConfig_Struct *pATTablePtr = &c_stCddDrvEG800AKSocketConfigTable[eSocketType];
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
        if (pSocketCtrl != NULL)
        {
            index = pTaskCmdDescribtor->pFuncPackAT(socketIndex, pSocketCtrl, txBuf, index);
        }
        else
        {
            index = pTaskCmdDescribtor->pFuncPackAT(socketIndex, &g_stCddDrvEG800AKCtrl, txBuf, index);
        }
    }

    return index;
}

static void CddDrvEG800AK_ATTaskSendHandle(uint8_t *txBuf)
{
    const ATCmdDescribtor_Struct * pATCmdDescribtor = g_stCddDrvEG800AKCtrl.currentTaskATDescribtor;
    uint16_t txLen = 0;

    if (CddDrvEG800AK_CheckTransparentMode() == TRUE)
    {
        if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.transparentModeStartTick, CDDDRV_EG800AK_CFG_TRANSPARENT_TIMEOUT))
        {
            CddDrvEG800AK_ExitTransparentMode();
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
                    if (pATCmdDescribtor->printFlag == TRUE)
                    {
                        CDDDRV_EG800AK_CFG_LogPrint("[4G %s-->Tx]:\r\n%s\r\n", pATCmdDescribtor->cMeanings, txBuf);
                    }

                    g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP2;
                    g_stCddDrvEG800AKCtrl.waitAtAckTickStart = Common_GetSystick();
                }
                else
                {
                    /* 紧急撤回一个发送 */
                    CddDrvEG800AK_DeleteCmd(g_stCddDrvEG800AKCtrl.currentTaskSocketIndex);
                    g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
                    g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
                    g_stCddDrvEG800AKCtrl.cmdTaskStep = CDDDRV_EG800AK_CTRL_STEP0;
                }

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


static void CddDrvEG800AK_ATTaskTimeoutDetect(uint8_t socketIndex, CddNetMSocketType_Enum eSocketType, void *para, CddDrvEG800AKATCtrl_Struct *pAtCtrl)
{
    const ATCmdDescribtor_Struct *pCmdDescribtor = NULL;
    uint8_t failHandleRet;

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
                            failHandleRet = FALSE;

                            if (pCmdDescribtor->pFuncFailHandle != NULL)
                            {
                                /* 当返回值为TRUE, 说明错误处理已经进行了cmd清除，并且添加了新的cmd */
                                failHandleRet = pCmdDescribtor->pFuncFailHandle(socketIndex, para, pAtCtrl->atTaskArray[0]);
                            }

                            if (failHandleRet == FALSE)
                            {
                                CddDrvEG800AK_DeleteCmd(socketIndex);
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
    uint16_t dataLen = 0;
    char *pDest = NULL;
    char *pTail = NULL;
    uint8_t bAnswerOK = FALSE;
    static uint16_t lastReadLen = 0;
    uint16_t dealLen = 0;
    uint16_t preLen = 0;
    uint8_t *pEnd = NULL;
    uint16_t postLen = 0;
    uint16_t remainLen = 0;
    uint8_t printFlag = TRUE;

    CDDDRV_EG800AK_CFG_ReadData(recvbuf, dataLen, lastReadLen);

    if (dataLen > 0)
    {
        g_stCddDrvEG800AKCtrl.noCommTickStart = Common_GetSystick();

        recvbuf[dataLen] = 0;

        if (CddDrvEG800AK_CheckTransparentMode() == TRUE &&
            g_stCddDrvEG800AKCtrl.eTransparentDirection == eCddDrvEG800AKDirection_Recv)
        {
            pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[g_stCddDrvEG800AKCtrl.transparentSocketIndex];

            if (c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].recvTransparentData != NULL)
            {
                c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].recvTransparentData(&g_stCddDrvEG800AKCtrl, recvbuf, dataLen);
            }

            CddDrvEG800AK_ExitTransparentMode();
        }
        else
        {
            remainLen = dataLen;
            postLen = remainLen;
            preLen = 0;

            if (g_stCddDrvEG800AKCtrl.currentTaskCmd != 0)
            {
                pDest = (char *)Common_SearchData(recvbuf, remainLen, pATCmdDescribtor->cATAnswerHead, strlen(pATCmdDescribtor->cATAnswerHead));

                if (pDest != NULL)
                {
                    if (pATCmdDescribtor->printFlag == TRUE)
                    {
                        CDDDRV_EG800AK_CFG_LogPrint("[4G-->Rx]:\n%s\n", recvbuf);
                    }

                    if (NULL != pATCmdDescribtor->pFuncRecvHandle)
                    {
                        postLen = remainLen - (uint16_t)((uint32_t)pDest - (uint32_t)recvbuf);

                        if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
                        {
                            pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
                            bAnswerOK = pATCmdDescribtor->pFuncRecvHandle(socketIndex, pSocketCtrl, (uint8_t *)pDest, postLen, &dealLen);
                        }
                        else
                        {
                            bAnswerOK = pATCmdDescribtor->pFuncRecvHandle(socketIndex, &g_stCddDrvEG800AKCtrl, (uint8_t *)pDest, postLen, &dealLen);
                        }

                        postLen -= dealLen;
                    }
                    else
                    {
                        bAnswerOK = TRUE;
                        postLen -= strlen(pATCmdDescribtor->cATAnswerHead);
                    }

                    /* 接收处理函数内部未处理dealLen */
                    if (dealLen == 0)
                    {
                        if (bAnswerOK == TRUE && pATCmdDescribtor->cATAnswerTail != NULL)
                        {
                            pTail = (char *)Common_SearchData(recvbuf + (remainLen - postLen), postLen, pATCmdDescribtor->cATAnswerTail, strlen(pATCmdDescribtor->cATAnswerTail));

                            if (pTail != NULL)
                            {
                                postLen = remainLen - (uint16_t)((uint32_t)pTail - (uint32_t)recvbuf) - strlen(pATCmdDescribtor->cATAnswerTail);
                            }
                        }
                    }

                    preLen = (uint16_t)((uint32_t)pDest - (uint32_t)recvbuf);
                    pEnd = recvbuf + (remainLen - postLen);

                    if (postLen > 0)
                    {
                        memmove(recvbuf + preLen, pEnd, postLen);
                    }

                    remainLen = preLen + postLen;
                    recvbuf[remainLen] = '\0';
                }

                if (bAnswerOK == TRUE)
                {
                    CddDrvEG800AK_DeleteCmd(g_stCddDrvEG800AKCtrl.currentTaskSocketIndex);
                }
            }
            
            while (remainLen > 0)
            {
                dealLen = CddDrvEg800AK_UrcDecode(recvbuf, &g_stCddDrvEG800AKCtrl, remainLen, printFlag);

                if (dealLen == 0)
                    break;

                printFlag = FALSE;
                remainLen -= dealLen;
                
                if (remainLen > 0)
                {
                    memmove(pData, &pData[dealLen], remainLen);
                }
            }
        }
    }
}


static uint16_t CddDrvEg800AK_UrcDecode(uint8_t *pData, void * modulePara, uint16_t dataLen, uint8_t printEnable)
{                                                                                                                              
    const ATUrcDescribtor_Struct *pUrcDescribtor = NULL;
    uint16_t usedLen = 0;
    uint16_t remainLen = 0;
    uint16_t matchPos = 0;
    uint16_t minMatchPos = 0xffff;
    uint16_t dropLen = 0;
    uint8_t matchIndex = 0xff;
    uint8_t findFlag = FALSE;
    uint8_t *pMatch = NULL;
    uint8_t index = 0;
    char *pSuffix = NULL;
    uint16_t dealLen = 0;

    /* 查找最早出现的URC */
    for (index = 0; index < ARRAY_SIZE(c_stATUrcDescribtor); index++)
    {
        pUrcDescribtor = &c_stATUrcDescribtor[index];
        if (strlen(pUrcDescribtor->cUrc) > 0)
        {
            pMatch = Common_SearchData(pData, dataLen, (uint8_t *)pUrcDescribtor->cUrc, strlen(pUrcDescribtor->cUrc));
            if (pMatch != NULL)
            {
                matchPos = (uint16_t)(pMatch - pData);
                if (findFlag == FALSE || matchPos < minMatchPos)
                {
                    findFlag = TRUE;
                    minMatchPos = matchPos;
                    matchIndex = index;
                    if (minMatchPos == 0)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (findFlag == TRUE)
    {
        pUrcDescribtor = &c_stATUrcDescribtor[matchIndex];
        if (printEnable == TRUE && pUrcDescribtor->printFlag == TRUE)
        {                                                            
            CDDDRV_EG800AK_CFG_LogPrint("[4G-->Rx]URC:\r\n%s\r\n", &pData[minMatchPos]);                                               
        }

        /* 如果函数内部会计算dealLen, 不需要配置cSuffix */
        if (pUrcDescribtor->cSuffix != NULL)
        {
            pSuffix = strstr((char *)&pData[minMatchPos], pUrcDescribtor->cSuffix);
            if (pSuffix != NULL)
            {
                usedLen = (uint16_t)(pSuffix - (char *)&pData[minMatchPos]) + strlen(pUrcDescribtor->cSuffix);
            }
            else
            {
                /* 没有找到后缀, 则认为是错误的URC */
                findFlag = FALSE;
            }
        }

        if (findFlag == TRUE && pUrcDescribtor->pFuncRecvHandle != NULL)
        {
            dealLen = pUrcDescribtor->pFuncRecvHandle(&pData[minMatchPos], modulePara,
                        (pUrcDescribtor->cSuffix != NULL) ? usedLen : (dataLen - minMatchPos));
            
            /* 如果函数内部会计算dealLen, 则usedLen = dealLen */
            if (pUrcDescribtor->cSuffix == NULL)
            {
                usedLen = dealLen;
            }
        }

        if (usedLen > 0 && usedLen <= (dataLen - minMatchPos))
        {
            dropLen = minMatchPos + usedLen;
        }
    }

    return dropLen;
}

static void CddDrvEG800AK_SocketStateHandle(void)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

        if (pSocketCtrl->stateHandle != NULL)
        {
            pSocketCtrl->stateHandle(pSocketCtrl->socketIndex, pSocketCtrl);
        }
    }
}

static void CddDrvEG800AK_CmdTaskHandle(void)
{
    uint8_t cacheBuff[CDDDRV_EG800AK_CFG_BUFF_SIZE] = {0};
    uint8_t socketIndex = CDDDRV_EG800AK_MODULE_SOCKET;
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;

    if (socketIndex == CDDDRV_EG800AK_MODULE_SOCKET)
    {
        CddDrvEG800AK_ATTaskTimeoutDetect(socketIndex, eCddNetMSocketType_Null, &g_stCddDrvEG800AKCtrl, &g_stCddDrvEG800AKCtrl.stModuleAtCtrl);
    }

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
        CddDrvEG800AK_ATTaskTimeoutDetect(socketIndex, pSocketCtrl->eSocketType, pSocketCtrl, &pSocketCtrl->stSocketAtCtrl);
    }

    CddDrvEG800AK_ATTaskSendHandle(cacheBuff);
    CddDrvEG800AK_ATTaskRecvHandle(cacheBuff);
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

static void CddDrvEG800AK_noCommTimeoutDetect(void)
{ 
    if (Common_JudgeTimeoutMs(g_stCddDrvEG800AKCtrl.noCommTickStart ,CDDDRV_EG800AK_CFG_NO_COMM_TIMEOUT))
    {
        g_stCddDrvEG800AKCtrl.noCommTickStart = Common_GetSystick();
        CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
        CddDrvEG800AK_SetAbnormalType(eCddDrvEG800AKAbnormalHandle_Reboot);
        CDDDRV_EG800AK_CFG_LogPrint("4G 模组无数据通信超时 %d ms\r\n!", CDDDRV_EG800AK_CFG_NO_COMM_TIMEOUT);    
    }
}

static void CddDrvEG800AK_StartModuleCfg(void)
{
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_ATE0);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryModule);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QuerySimRecognizeStatus);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryIccid);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCsq);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNtpClk);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCGREG);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryCOPS);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryNetWorkInfo);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_ConfigAPN);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_ActivePDP);
    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_QueryPDPState);
}

static uint8_t CddDrvEG800AK_FindFreeSocket(uint8_t *pSocketIndex)
{
	CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
	uint8_t socketIndex = 0;
	uint8_t ret = FALSE;

	for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
	{
		pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

		if (pSocketCtrl->usedFlag == FALSE)
		{
            pSocketIndex[0] = socketIndex;
            ret = TRUE;
            break;
		}
	}

	return ret;
}

void CddDrvEG800AK_EnterTransparentMode(uint8_t socketIndex, CddDrvEG800AKDirection_Enum eTransparentDirection)
{ 
    g_stCddDrvEG800AKCtrl.transparentMode = TRUE;
    g_stCddDrvEG800AKCtrl.transparentModeStartTick = Common_GetSystick();
    g_stCddDrvEG800AKCtrl.eTransparentDirection = eTransparentDirection;
    g_stCddDrvEG800AKCtrl.transparentSocketIndex = socketIndex;
}

void CddDrvEG800AK_ExitTransparentMode(void)
{ 
    g_stCddDrvEG800AKCtrl.transparentMode = FALSE;
}

uint8_t CddDrvEG800AK_CheckTransparentMode(void)
{
    return g_stCddDrvEG800AKCtrl.transparentMode;
}

void CddDrvEG800AK_SetSocketDisconnect(uint8_t socketIndex)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (pSocketCtrl->usedFlag == TRUE)
        {
            if (pSocketCtrl->socketCloseHandle != NULL)
            {
                pSocketCtrl->socketCloseHandle(pSocketCtrl);
            }
        }
    }
}

CddNetMSocketState_Enum CddDrvEG800AK_GetSocketState(uint8_t socketIndex)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
    CddNetMSocketState_Enum eSocketState = eCddNetMSocketState_Init;

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (pSocketCtrl->usedFlag == TRUE)
        {
            eSocketState = pSocketCtrl->eSocketState;
        }
    }

    return eSocketState;
}

void CddDrvEG800AK_DelAllSocket(void)
{
    uint8_t socketIndex = 0;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        CddDrvEG800AK_DelSingleSocket(socketIndex);
    }
}

void CddDrvEG800AK_DelSingleSocket(uint8_t socketIndex)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
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

void CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Enum eAbnormalType)
{
    CddDrvEG800AKAbnormalHandle_Enum tempAbnormalType;
    
    if (eAbnormalType == eCddDrvEG800AKAbnormalHandle_CFun)
    {
        if (g_stCddDrvEG800AKCtrl.eLastAbnormalHandleType == eCddDrvEG800AKAbnormalHandle_CFun)
        {
            tempAbnormalType = eCddDrvEG800AKAbnormalHandle_Reboot;
        }
        else
        {
            tempAbnormalType = eCddDrvEG800AKAbnormalHandle_CFun;
        }
    }
    else
    {
        tempAbnormalType = eCddDrvEG800AKAbnormalHandle_Reboot;
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
                if (g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType == eCddDrvEG800AKAbnormalHandle_CFun)
                {
                    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN0);
                    CddDrvEG800AK_AddCmd(CDDDRV_EG800AK_MODULE_SOCKET, eATModuleCmd_SetCFUN1);
                    g_stCddDrvEG800AKCtrl.abNormalHandleStep = CDDDRV_EG800AK_CTRL_STEP2;
                }
                else
                {
                    g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType = eCddDrvEG800AKAbnormalHandle_Null;
                    CddDrvEG800AK_SetModuleState(eCddNetMModuleState_Init);
                }
            }

            break;
        }
        case CDDDRV_EG800AK_CTRL_STEP2:
        {
            if (g_stCddDrvEG800AKCtrl.stModuleAtCtrl.atTaskArray[0] == 0)
            {
                g_stCddDrvEG800AKCtrl.eCurrentAbnormalHandleType = eCddDrvEG800AKAbnormalHandle_Null;
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

void CddDrvEG800AK_GetIccid(char *pICCID)
{
    if (pICCID != NULL)
    {
        memcpy(pICCID, g_stCddDrvEG800AKCtrl.stModuleInfo.iccid, CDDDRV_EG800AK_CFG_ICCID_LEN);
    }
}

uint8_t CddDrvEG800AK_GetCsq(void)
{
    return g_stCddDrvEG800AKCtrl.stModuleInfo.csq;
}

CddNetMOperator_Enum CddDrvEG800AK_GetOperatorType(void)
{
    return g_stCddDrvEG800AKCtrl.stModuleInfo.eOperatorType;
}

void CddDrvEG800AK_GetModuleTypeInfo(char *pModuleType, uint16_t readLen)
{
    uint16_t copyLen = strlen(CDDDRV_EG800AK_CFG_MODULE_TYPE);

    if (pModuleType != NULL && readLen != 0)
    {
        copyLen = copyLen > readLen ? readLen : copyLen;
        snprintf(pModuleType, copyLen, "%s", CDDDRV_EG800AK_CFG_MODULE_TYPE);
    }
}

void CddDrvEG800AK_UpdateIpPort(uint8_t socketIndex, char *pIp, uint16_t port)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (pSocketCtrl->usedFlag == TRUE)
        {
            if (c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].updateIpPort != NULL)
            {
                c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].updateIpPort(pSocketCtrl, pIp, port);
            }
        }
    }
}

void CddDrvEG800AK_UpdateMqttUserNamePassword(uint8_t socketIndex, char *pUserName, char *pPassword)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

    if (socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT)
    {
        if (pSocketCtrl->usedFlag == TRUE)
        {
            if (c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].updateMqttUserNamePassword != NULL)
            {
                c_stCddDrvEG800AKSocketConfigTable[pSocketCtrl->eSocketType].updateMqttUserNamePassword(pSocketCtrl, pUserName, pPassword);
            }
        }
    }
}

static void CddDrvEG800AK_SocketDisconnectCallback(void)
{
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;
    uint8_t flag = TRUE;
    uint8_t exsistSocketFlag = FALSE;

    for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];

        if (pSocketCtrl->usedFlag == TRUE)
        {
            exsistSocketFlag = TRUE;
            if (pSocketCtrl->reconectTimes < CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES)
            {
                flag = FALSE;
                break;
            }
        }
    }

    if (flag == TRUE && exsistSocketFlag == TRUE)
    {
        for (socketIndex = 0; socketIndex < CDDDRV_EG800AK_CFG_SOCKET_COUNT; socketIndex++)
        {
            pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
            pSocketCtrl->reconectTimes = 0;
        }

        CddDrvEG800AK_SetModuleState(eCddNetMModuleState_AbNormal);
        CddDrvEG800AK_SetAbnormalType(eCddDrvEG800AKAbnormalHandle_CFun);
    }
}

uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd)
{
    CddDrvEG800AKATCtrl_Struct *pAtCtrl = NULL;
    const CddDrvEG800AKSocketConfig_Struct *pATTablePtr = NULL;
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
        pATTablePtr = &c_stCddDrvEG800AKSocketConfigTable[eSocketType];

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

        pAtCtrl->atTryCount = 0;
        pAtCtrl->readyFlag = FALSE;

        if (g_stCddDrvEG800AKCtrl.currentTaskSocketIndex == socketIndex)
        {
            g_stCddDrvEG800AKCtrl.currentTaskCmd = 0;
            g_stCddDrvEG800AKCtrl.currentTaskATDescribtor = NULL;
        }
    }
}

uint8_t CddDrvEG800AK_CreatSocket(CddNetMSocketType_Enum socketType, CddNetMSocketPara_Union *pSocketPara, uint8_t *pSocketIndex, CddNetMPlatType_Enum ePlatType)
{
    const CddDrvEG800AKSocketConfig_Struct *pSocketConfig = &c_stCddDrvEG800AKSocketConfigTable[socketType];
    CddDrvEG800AKSocketCtrl_Struct *pSocketCtrl = NULL;
    uint8_t socketIndex = 0;
    uint8_t ret = FALSE;

    PARA_ASSERT_RET(pSocketPara != NULL && pSocketIndex != NULL, FALSE);
    PARA_ASSERT_RET(socketType < eCddNetMSocketType_Count, FALSE);

    if (TRUE == CddDrvEG800AK_FindFreeSocket(&socketIndex))
    {
        pSocketCtrl = &g_stCddDrvEG800AKCtrl.stSocketCtrl[socketIndex];
        memset(pSocketCtrl, 0x00, sizeof(CddDrvEG800AKSocketCtrl_Struct));
        pSocketCtrl->usedFlag = TRUE;
        pSocketCtrl->socketIndex = socketIndex;
        pSocketIndex[0] = socketIndex;
        pSocketCtrl->eSocketType = socketType;
        pSocketCtrl->ePlatType = ePlatType;
        pSocketCtrl->specificPara = pSocketPara;
        pSocketCtrl->socketDisconnectCallback = CddDrvEG800AK_SocketDisconnectCallback;
        pSocketCtrl->stateHandle = pSocketConfig->stateHandle;
        pSocketCtrl->socketCloseHandle = pSocketConfig->socketCloseHandle;
        ret = TRUE;
    }

    return ret;
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
            /* 暂时没考虑好，还需要添加什么处理 */
        }
        else
        {}
        
        CddDrvEG800AK_CmdTaskHandle();

        CddDrvEG800AK_SocketStateHandle();
    }

    CddDrvEG800AK_noCommTimeoutDetect();
}






























