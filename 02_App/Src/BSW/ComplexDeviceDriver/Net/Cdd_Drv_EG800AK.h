/******************************************************************************
* File Name          : template.h
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
#ifndef CDD_DRV_EG800AK_H_
#define CDD_DRV_EG800AK_H_


/******************************************************************************
*    Include Files
******************************************************************************/
#include "Common.h"
#include "Cdd_NetM.h"
#include "Cdd_Drv_EG800AKConfig.h"


/******************************************************************************
*    Header File Inclusion
******************************************************************************/


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
    uint8_t readyFlag;
    uint8_t  atTryCount;
    uint32_t atWaitTickStart;
    uint8_t atTaskArray[CDDDRV_EG800AK_CFG_AT_TASK_COUNT];
}CddDrvEG800AKATCtrl_Struct;

typedef struct
{
    uint8_t useFlag;
    uint8_t socketIndex;

    CddNetMSocketState_Enum eSocketState;
    CddNetMSocketType_Enum eSocketType;
    CddNetMPlatType_Enum ePlatType;

    uint32_t disconectTickStart;
    uint8_t reconectTimes;
    CddDrvEG800AKATCtrl_Struct stSocketAtCtrl;

 	void (*stateHandle)(void *socketCtrl);
    void (*socketDisconnectCallback)(void *socketCtrl);
    void (*socketCloseHandle)(void *socketCtrl);
} CddDrvEG800AKSocketCtrl_Struct;

typedef struct
{
    char iccid[CDDDRV_EG800AK_CFG_ICCID_LEN + 1];

}CddDrvEG800AKInfo_Struct;

typedef struct
{
    uint8_t powerOnStep;                    /*: 重启控制步骤 */
    uint32_t powerCtrlStartTick;            /*: 重启控制时间 */

    uint8_t cmdTaskStep;                    /*: cmd 处理步骤 */

    uint8_t transparentMode;                /* TRUE: 透传模式 */
    uint32_t transparentModeStartTick;      /* 透传模式起始时间 */

    CddDrvEG800AKInfo_Struct stModuleInfo;  /* 模块信息 */

    CddNetMModuleState_Enum eModuleState;

    uint8_t currentTaskSocketIndex;
    uint8_t currentTaskCmd;
    uint32_t waitAtAckTickStart;
    const ATCmdDescribtor_Struct *currentTaskATDescribtor;

    CddDrvEG800AKATCtrl_Struct stModuleAtCtrl;
    
    CddDrvEG800AKSocketCtrl_Struct stSocketCtrl[CDDDRV_EG800AK_CFG_SOCKET_COUNT];
}CddDrvEG800AKCtrl_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
/* 限模块内使用， 禁止其它模块直接访问 */
extern CddDrvEG800AKCtrl_Struct g_stCddDrvEG800AKCtrl;


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddDrvEG800AK_InitMemory(void);
void CddDrvEG800AK_MainFunction(void);

/* 模块内使用 */
void CddDrvEG800AK_SetModuleState(CddNetMModuleState_Enum eModuleState);
uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd);
void CddDrvEG800AK_DeleteCmd(uint8_t socketIndex);
void CddDrvEG800AK_ClearSocketCmd(uint8_t socketIndex);
#endif






















