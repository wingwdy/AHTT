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
    eCddDrvEG800AKAbnormalHandle_Null,
    eCddDrvEG800AKAbnormalHandle_Reboot,
    eCddDrvEG800AKAbnormalHandle_CFun,
}CddDrvEG800AKAbnormalHandle_Enum;

typedef enum
{
    eCddDrvEG800AKDirection_Send,
    eCddDrvEG800AKDirection_Recv,
}CddDrvEG800AKDirection_Enum;

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t readyFlag;  /* FALSE-表示当前有任务在执行， TRUE-表示可以执行切换到下一个sokcet了*/
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

    uint32_t periodDetectDataSendTick;

    uint8_t user_data[64];
    void *specificPara;

 	void (*stateHandle)(uint8_t socketIndex, void *socketCtrl);
    void (*socketDisconnectCallback)(void);
    void (*socketCloseHandle)(void *socketCtrl);
    void (*updateMqttUserNamePassword)(uint8_t socketIndex, char *pUserName, char *pPassword);
    void (*updateIpPort)(uint8_t socketIndex, char *pIp, uint16_t port);
} CddDrvEG800AKSocketCtrl_Struct;

typedef struct
{
    char iccid[CDDDRV_EG800AK_CFG_ICCID_LEN + 1];
    char imei[CDDDRV_EG800AK_CFG_IMEI_LEN + 1];
    uint8_t csq;
    CddNetMOperator_Enum eOperatorType;
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
    CddDrvEG800AKDirection_Enum eTransparentDirection; /* 透传方向 */              
    uint32_t transparentModeStartTick;      /* 透传模式起始时间 */
    uint8_t transparentSocketIndex;

    CddDrvEG800AKInfo_Struct stModuleInfo;  /* 模块信息 */

    CddNetMModuleState_Enum eModuleState;

    uint32_t noCommTickStart;               /* 无数据通信计数器 */

    uint8_t currentTaskSocketIndex;
    uint8_t currentTaskCmd;
    uint32_t waitAtAckTickStart;
    const ATCmdDescribtor_Struct *currentTaskATDescribtor;

    CddDrvEG800AKATCtrl_Struct stModuleAtCtrl;

    uint8_t simNetFlag;                    /*: sim卡标志: 0-公网卡, 1-专网卡 */
    CddDrvEG800AKSocketCtrl_Struct stSocketCtrl[CDDDRV_EG800AK_CFG_SOCKET_COUNT];
}CddDrvEG800AKCtrl_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CddDrvEG800AK_MainFunction(void);
CddNetMModuleState_Enum CddDrvEG800AK_GetModuleState(void);
uint8_t CddDrvEG800AK_CreatSocket(CddNetMSocketType_Enum socketType, CddNetMSocketPara_Union *pSocketPara, 
    uint8_t *pSocketIndex, CddNetMPlatType_Enum ePlatType);
void CddDrvEG800AK_DelAllSocket(void);
void CddDrvEG800AK_SetSocketDisconnect(uint8_t socketIndex);
CddNetMSocketState_Enum CddDrvEG800AK_GetSocketState(uint8_t socketIndex);
void CddDrvEG800AK_DelSingleSocket(uint8_t socketIndex);
void CddDrvEG800AK_GetIccid(char *pICCID);
void CddDrvEG800AK_GetImei(char *pIMEI);
uint8_t CddDrvEG800AK_GetCsq(void);
CddNetMOperator_Enum CddDrvEG800AK_GetOperatorType(void);
void CddDrvEG800AK_GetModuleTypeInfo(char *pModuleType, uint16_t readLen);
void CddDrvEG800AK_SetSimNet(uint8_t simNet);
/* 模块内使用 */
void CddDrvEG800AK_SetModuleState(CddNetMModuleState_Enum eModuleState);
void CddDrvEG800AK_EnterTransparentMode(uint8_t socketIndex, CddDrvEG800AKDirection_Enum eTransparentDirection);
void CddDrvEG800AK_ExitTransparentMode(void);
uint8_t CddDrvEG800AK_CheckTransparentMode(void);
uint8_t CddDrvEG800AK_AddCmd(uint8_t socketIndex, uint8_t cmd);
void CddDrvEG800AK_DeleteCmd(uint8_t socketIndex);
void CddDrvEG800AK_ClearSocketCmd(uint8_t socketIndex);
void CddDrvEG800AK_SetAbnormalType(CddDrvEG800AKAbnormalHandle_Enum eAbnormalType);
void CddDrvEG800AK_UpdateIpPort(uint8_t socketIndex, char *pIp, uint16_t port);
void CddDrvEG800AK_UpdateMqttUserNamePassword(uint8_t socketIndex, char *pUserName, char *pPassword);
#endif






















