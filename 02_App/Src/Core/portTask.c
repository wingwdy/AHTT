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
#include "Asw_LedEvent.h"
#include "Asw_TempHandle.h"
#include "Asw_VoltCurHandle.h"

#include "Asw_PlatM.h"
#include "Asw_Monitor.h"

#include "DS_Console.h"
#include "SS_Tm.h"
#include "SS_Snapshot.h"
#include "SS_WdgM.h"
#include "SS_Ucm.h"


#include "Cdd_CP.h"
#include "Cdd_Relay.h"
#include "Cdd_Rcd.h"
#include "Cdd_Sensor.h"
#include "Cdd_PE.h"
#include "Cdd_MeterM.h"
#include "Cdd_LedM.h"
#include "Cdd_NetM.h"
#include "Cdd_ModeM.h"
#include "Cdd_CardM.h"

#include "Mcal_If.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "Common.h"


/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define PORTTASK_CFG_LogPrint(fmt, ...)          DSLOGM_Debug(DSLogMModule_System, fmt, ##__VA_ARGS__)

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
    uint8_t wdgId;
}portTask_CtrBlk;


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static void Task_10msA(void *arg);
static void Task_10msB(void *arg);
static void Task_100msA(void *arg);
static void Task_20msA(void *arg);
static void Task_20msB(void *arg);
static void Task_1SecA(void *arg);
static void Task_Wdg100ms(void *arg);

/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static portTask_CtrBlk  g_stTaskCtrBlkTable[] =
{
    {"App10msA",        Task_10msA,          &g_stTaskCtrBlkTable[0],         384,   6 } ,
    {"App10msB",        Task_10msB,          &g_stTaskCtrBlkTable[1],         384,   7 } ,
    {"App20msA",        Task_20msA,          &g_stTaskCtrBlkTable[2],         256,   4 } ,
    {"App20msB",        Task_20msB,          &g_stTaskCtrBlkTable[3],         2048,  5 } ,
    {"App100ms",        Task_100msA,         &g_stTaskCtrBlkTable[4],         512,   5 } ,
    {"App1Sec",         Task_1SecA,          &g_stTaskCtrBlkTable[5],         256,   4 } ,
    {"Wdg100ms",        Task_Wdg100ms,       &g_stTaskCtrBlkTable[6],         128,   8 } ,
};

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
void portTask_ShowStackInfo(void)
{
	uint8_t index = 0;
	portTask_CtrBlk *pTaskCtr = NULL;
    BaseType_t xResult = pdFALSE;
    uint32_t uxHighWaterMark = 0;

    PORTTASK_CFG_LogPrint("---------------------------------堆栈信息------------------------------------\r\n");

    PORTTASK_CFG_LogPrint("freeRTOS 总堆栈空间: %d K 字节, 剩余空间: %d K 字节\r\n", SYSCFG_CFG_OS_HEAP_SIZE / 1024, xPortGetFreeHeapSize() / 1024);
  
	for (index = 0; index < ARRAY_SIZE(g_stTaskCtrBlkTable); index++)
	{
		pTaskCtr = &g_stTaskCtrBlkTable[index];

        uxHighWaterMark = uxTaskGetStackHighWaterMark(pTaskCtr->taskHandle);
        PORTTASK_CFG_LogPrint("线程名: %s,\t\t总分配: %4d 字节, \t剩余: %4d 字节, \t空置率: %d%% \r\n", 
            pTaskCtr->cTaskName, pTaskCtr->usStackDepth * 4, uxHighWaterMark * 4, uxHighWaterMark * 100 / pTaskCtr->usStackDepth);
    }

    PORTTASK_CFG_LogPrint("------------------------------------------------------------------------------\r\n");
}

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
            PORTTASK_CFG_LogPrint("%s: Task create failed!!\r\n", pTaskCtr->cTaskName);
        }
        else
        {
            SSWdgM_RegisterModule(pTaskCtr->cTaskName, 5000, &pTaskCtr->wdgId);
        }
	}
}

static void Task_10msA(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;
    
    while (1)
    {
        CddCP_MainFunction();
        AswEVSE_MainFunction();
        AswCharge_MainFunction();
        CddRelay_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(10);
    }
}

static void Task_10msB(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;

    while (1)
    {
        CddRcd_MainFunction();
        CddPE_MainFunction();
        CddCardM_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(10);
    }
}

static void Task_20msA(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;
    while (1)
    {
        AswLedEvent_MainFunction();
        CddLedM_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(20);
    }
}

static void Task_20msB(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;

    while (1)
    {
        CddNetM_MainFunction();
        AswPlatM_MainFunction();
        SSSnapshot_MainFunction();
        DSConsole_MainFunction();
        SSUcm_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(20);
    }
}

static void Task_100msA(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;

    while (1)
    {
        CddMeterM_MainFunction();
        CddModeM_MainFunction();
        CddSensor_MainFunction();
        AswMonitor_MainFunction();
        AswVoltCurHandle_MainFunction();
        AswTempHandle_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(100);
    }
}

static void Task_1SecA(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;

    while (1)
    {
        SSTM_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(1000);
    }
}

static void Task_Wdg100ms(void *arg)
{
    portTask_CtrBlk *pTaskCtr = (portTask_CtrBlk *)arg;

    while (1)
    {
        SSWdgM_MainFunction();
        SSWdgM_Checkin(pTaskCtr->wdgId);
        vTaskDelay(100);
    }
}





























