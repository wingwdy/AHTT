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
*2025/10/10      V1.0.0      Chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "Common.h"
#include "Mcal_If.h"

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
*    Global variables Declaration
*******************************************************************************/
static void Task_Test(void *arg);

static portTask_CtrBlk  g_stTaskCtrBlkTable[] =
{
	{"TestTaskHandle", Task_Test,  NULL, 1536,  6},
};

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/



/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void TaskStartMain(void)
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

#if 0
	    if(xResult != pdTRUE)
        {
            log_debug("[%s] task create error\n", pTaskCtr->cTaskName);
        }
#endif
	}
}

static void Task_Test(void *arg)
{
	while (1)
	{
		vTaskDelay(1000);
		McalIf_Test();
	}
}

































