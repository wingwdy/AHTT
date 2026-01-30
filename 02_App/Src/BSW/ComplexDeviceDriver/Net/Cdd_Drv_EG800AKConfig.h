/******************************************************************************
* File Name          : template_Config.h
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
#ifndef CDD_DRV_EG800AK_CONFIG_H_
#define CDD_DRV_EG800AK_CONFIG_H_


/******************************************************************************
*    Header File Inclusion
******************************************************************************/
#include "Mcal_Port.h"
#include "Mcal_Uart.h"
#include "AT_Describtor.h"
#include "Cdd_NetM.h"
#include "DS_LogM.h"
#include "FreeRTOS.h"
#include "task.h"

/******************************************************************************
*    Macro Definition
******************************************************************************/
#define CDDDRV_EG800AK_CFG_PwrOff()                     McalPort_SetPin(eMcalPortPinChanel_PC15_4GPwrEn)
#define CDDDRV_EG800AK_CFG_PwrOn()                      McalPort_ResetPin(eMcalPortPinChanel_PC15_4GPwrEn)
#define CDDDRV_EG800AK_CFG_PwrKeyOff()                  McalPort_SetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn)
#define CDDDRV_EG800AK_CFG_PwrKeyOn()                   McalPort_ResetPin(eMcalPortPinChanel_PC14_4GPwrKeyEn)

#define CDDDRV_EG800AK_CFG_WriteData(data, len)         McalUart_WriteData(eMcalUartChanel_4G, data, len)

#define CDDDRV_EG800AK_CFG_ReadData(data, len, lastReadLen)     do\
                                                                {\
                                                                    while (eGlobalRet_OK  == McalUart_CheckDataLen(eMcalUartChanel_4G, &dataLen))\
                                                                    {\
                                                                        if (dataLen != lastReadLen)\
                                                                        {\
                                                                            lastReadLen = dataLen;\
                                                                            vTaskDelay(1);\
                                                                            continue;\
                                                                        }\
                                                                        McalUart_ReadData(eMcalUartChanel_4G, recvbuf, dataLen);\
                                                                        break;\
                                                                    }\
                                                                    lastReadLen = 0;\
                                                                } while(0)

#define CDDDRV_EG800AK_CFG_LogPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_4G, fmt, ##__VA_ARGS__)

#define CDDDRV_EG800AK_CFG_POWEROFF_HOLD_TIME           500           
#define CDDDRV_EG800AK_CFG_POWERON_HOLD_TIME            1000
#define CDDDRV_EG800AK_CFG_POWERKEY_OFF_HOLD_TIME       2000
#define CDDDRV_EG800AK_CFG_POWERKEY_ON_HOLD_TIME        3500

#define CDDDRV_EG800AK_CFG_TRANSPARENT_TIMEOUT          3000

#define CDDDRV_EG800AK_CFG_NO_COMM_TIMEOUT              (3 * 60000)

#define CDDDRV_EG800AK_CFG_SOCKET_COUNT                 3

#define CDDDRV_EG800AK_CFG_AT_TASK_COUNT                20

#define CDDDRV_EG800AK_CFG_BUFF_SIZE                    3072

#define CDDDRV_EG800AK_CFG_ICCID_LEN                    20

#define CDDDRV_EG800AK_CFG_RECONECT_TIMEOUT(x)          (10000 * x)
#define CDDDRV_EG800AK_CFG_RECONECT_MAX_TIMES           5

#define CDDDRV_EG800AK_CFG_MODULE_TYPE                  "EG800AK"

/******************************************************************************
*    Enum Definition
******************************************************************************/

/******************************************************************************
*    Typedef Definition
******************************************************************************/
typedef struct 
{
    uint8_t cmdTaskCount;
    const ATCmdDescribtor_Struct *pATCmdDescribtorTable;
 	void (*stateHandle)(uint8_t socketIndex, void *socketCtrl);
    void (*socketCloseHandle)(void *socketCtrl);
}CddDrvEG800AKSocketConfig_Struct;


/******************************************************************************
*    Global variables Declaration
******************************************************************************/
extern const CddDrvEG800AKSocketConfig_Struct c_stCddDrvEG800AKSocketConfigTable[eCddNetMSocketType_Count];


/******************************************************************************
*    Global Function Prototypes
******************************************************************************/
void CDDDRVEG800AK_CFG_WriteData(uint8_t *pData, uint16_t len, void *userData);
#endif /* CDD_DRV_EG800AK_CONFIG_H_ */























