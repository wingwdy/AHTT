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
typedef enum
{
    CddDrvEG800AKAbnormalHandle_Null,
    CddDrvEG800AKAbnormalHandle_Reboot,
    CddDrvEG800AKAbnormalHandle_CFun,
}CddDrvEG800AKAbnormalHandle_Enum;

//服务商
typedef enum 
{
	CddDrvSimOperator_Null,
	CddDrvSimOperator_CMCC,					/* 移动 */
	CddDrvSimOperator_CUCC,					/* 联通 */
	CddDrvSimOperator_CTCC,					/* 电信 */
	CddDrvSimOperator_Other,				    /* 其他 */
}CddDrvSimOperator_Enum;


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
    uint8_t usedFlag;
    uint8_t socketIndex;

    CddNetMSocketState_Enum eSocketState;
    CddNetMSocketType_Enum eSocketType;
    CddNetMPlatType_Enum ePlatType;

    uint32_t disconectTickStart;
    uint8_t reconectTimes;
    CddDrvEG800AKATCtrl_Struct stSocketAtCtrl;

    uint8_t user_data[32];
    void *specificPara;

 	void (*stateHandle)(void *socketCtrl);
    void (*socketDisconnectCallback)(void *socketCtrl);
    void (*socketCloseHandle)(void *socketCtrl);
} CddDrvEG800AKSocketCtrl_Struct;

typedef struct
{
    char iccid[CDDDRV_EG800AK_CFG_ICCID_LEN + 1];
    uint8_t csq;
    CddDrvSimOperator_Enum eOperatorType;
}CddDrvEG800AKInfo_Struct;

typedef struct
{
    uint8_t powerOnStep;                    /*: 重启控制步骤 */
    uint32_t powerCtrlStartTick;            /*: 重启控制时间 */

    uint8_t cmdTaskStep;                    /*: cmd 处理步骤 */

    CddDrvEG800AKAbnormalHandle_Enum eCurrentAbnormalHandleType;
    CddDrvEG800AKAbnormalHandle_Enum eLastAbnormalHandleType;

    uint8_t abNormalHandleStep;             /*: 异常处理步骤 */

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


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddDrvEG800AK_MainFunction(void);
uint8_t CddDrvEG800AK_CreatSocket(CddNetMSocketType_Enum socketType, CddNetMSocketPara_Union *pSocketPara, 
    uint8_t *pSocketIndex, CddNetMPlatType_Enum ePlatType);
/* 模块内使用 */
void CddDrvEG800AK_SetModuleState(CddNetMModuleState_Enum eModuleState);
CddNetMModuleState_Enum CddDrvEG800AK_GetModuleState(void);
uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd);
void CddDrvEG800AK_DeleteCmd(uint8_t socketIndex);
void CddDrvEG800AK_ClearSocketCmd(uint8_t socketIndex);
void CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Enum eAbnormalType);

#endif






















