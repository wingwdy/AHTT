/******************************************************************************
* File Name          : portTask.c
* Description        : Code for task creat
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
#include "Asw_EVSE.h"

#include "Cdd_CP.h"
#include "Cdd_Relay.h"
#include "Cdd_Rcd.h"
#include "Cdd_Sensor.h"
#include "Cdd_PE.h"

#include "Mcal_If.h"

#include "FreeRTOS.h"
#include "task.h"
#include "Common.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/




/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
	const char * const cTaskName;
	TaskFunction_t pxTaskCode;
	void   *arg;
	const configSTACK_DEPTH_TYPE usStackDepth;
	UBaseType_t uxPriority;
    TaskHandle_t taskHandle;
}portTask_CtrBlk;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void Task_10ms(void *arg);
static void Task_100ms(void *arg);




/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static portTask_CtrBlk  g_stTaskCtrBlkTable[] =
{
    {"App10ms",        Task_10ms,          NULL,         512,   6 } ,
    {"App100ms",       Task_100ms,         NULL,         512,   6 } ,
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void portTask_CreatAllTask(void)
{
	uint8_t index = 0;
	portTask_CtrBlk *pTaskCtr = NULL;
    BaseType_t xResult = pdFALSE;

	for (index = 0; index < ARRAY_SIZE(g_stTaskCtrBlkTable); index++)
	{
		pTaskCtr = &g_stTaskCtrBlkTable[index];

		xResult = xTaskCreate(pTaskCtr->pxTaskCode,
                              pTaskCtr->cTaskName,
                              pTaskCtr->usStackDepth,
                              pTaskCtr->arg,
                              pTaskCtr->uxPriority,
                              &pTaskCtr->taskHandle);

	    if(xResult != pdTRUE)
        {
//          log_debug("[%s] task create error\n", pTaskCtr->cTaskName);
        }
	}
}


static void Task_10ms(void *arg)
{
    while (1)
    {
        CddRelay_MainFunction();
        CddCP_MainFunction();
        CddRcd_MainFunction();
        CddPE_MainFunction();

        AswEVSE_MainFunction();
        vTaskDelay(10);
    }
}

static void Task_100ms(void *arg)
{
    while (1)
    {
        CddSensor_MainFunction();
        vTaskDelay(1000);
    }
}































