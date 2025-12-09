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
#include "Asw_Charge.h"

#include "DS_Console.h"

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
static void Task_10msA(void *arg);
static void Task_10msB(void *arg);
static void Task_100ms(void *arg);
static void Task_Debug20ms(void *arg);




/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static portTask_CtrBlk  g_stTaskCtrBlkTable[] =
{
    {"App10msA",        Task_10msA,          NULL,         512,   6 } ,
    {"App10msB",        Task_10msB,          NULL,         512,   7 } ,
    {"App100ms",        Task_100ms,          NULL,         512,   5 } ,
    {"DebugTask",       Task_Debug20ms,      NULL,         512,   4 } ,
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


static void Task_10msA(void *arg)
{
    while (1)
    {
        CddRelay_MainFunction();
        CddCP_MainFunction();
        AswEVSE_MainFunction();
        AswCharge_MainFunction();
        vTaskDelay(10);
    }
}

static void Task_10msB(void *arg)
{
    while (1)
    {
        CddRcd_MainFunction();
        CddPE_MainFunction();
        vTaskDelay(10);
    }
}

static void Task_100ms(void *arg)
{
    while (1)
    {
 //       McalIf_Test();
        CddSensor_MainFunction();
        vTaskDelay(1000);
    }
}

static void Task_Debug20ms(void *arg)
{
    while (1)
    {
        DSConsole_MainFunction();
        vTaskDelay(10);
    }
}





























